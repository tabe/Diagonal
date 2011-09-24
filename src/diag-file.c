/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/datum.h"
#include "diagonal/dataset.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/set.h"
#include "diagonal/singlelinkage.h"
#include "diagonal/private/filesystem.h"

static uintptr_t hamming(intptr_t a, intptr_t b)
{
	const struct diag_datum *x, *y;
	size_t i, smin, smax;
	uintptr_t d = 0;

	x = (const struct diag_datum *)a;
	y = (const struct diag_datum *)b;
	if (x->tag < y->tag) {
		smin = (size_t)(x->tag>>1);
		smax = (size_t)(y->tag>>1);
	} else {
		smin = (size_t)(y->tag>>1);
		smax = (size_t)(x->tag>>1);
	}
	for (i = 0; i < smin; i++) {
		if (((char *)x->value)[i] != ((char *)y->value)[i]) d++;
	}
	return d + (smax - smin);
}

static uintptr_t levenshtein(intptr_t x, intptr_t y)
{
	size_t d = 0, i, j, lx, ly, *cur, *new, *tmp;
	const struct diag_datum *dx, *dy;
	char cx, cy;

	dx = (const struct diag_datum *)x;
	dy = (const struct diag_datum *)y;
	assert(dx && dy);
	lx = (size_t)(dx->tag>>1);
	ly = (size_t)(dy->tag>>1);
	if (lx == 0) return ly;
	if (ly == 0) return lx;
	if (lx < ly) {
		const struct diag_datum *dz = dx;
		size_t lz = lx;
		dx = dy;
		dy = dz;
		lx = ly;
		ly = lz;
	}
	cur = diag_calloc((ly + 1)*2, sizeof(*cur));
	new = cur + ly + 1;
	for (j = 1; j <= ly; j++) cur[j] = j;
	for (i = 0; i < lx; i++) {
		new[0] = cur[0] + 1;
		for (j = 0; j < ly; j++) {
			size_t k, a, b, c;
			k = j + 1;
			a = new[j] + 1;
			b = cur[k] + 1;
			cx = ((char *)dx->value)[i];
			cy = ((char *)dy->value)[j];
			c = cur[j] + ((cx == cy) ? 0 : 1);
			d = (a < b) ? a : b;
			d = (c < d) ? c : d;
			new[k] = d;
		}
		tmp = cur;
		cur = new;
		new = tmp;
	}
	diag_free(cur < new ? cur : new);
	return d;
}

static struct {
	const char *name;
	diag_metric_t metric;
	diag_cmp_t cmp;
} metrics[] = {
	{"hamming",     hamming,     DIAG_CMP_IMMEDIATE},
	{"levenshtein", levenshtein, DIAG_CMP_IMMEDIATE}
};

#define NUM_METRICS (sizeof(metrics)/sizeof(metrics[0]))

static void usage(void)
{
	diag_printf("diag-file [-m metric] [-i intial] [-f final] [-1] path [...]");
}

static void finalize(struct diag_datum *d)
{
	size_t len;

	len = (size_t)(d->tag>>1);
	munmap((char *)d->value, len);
}

static struct diag_datum *at(size_t i, struct diag_dataset *ds)
{
	struct diag_customized_datum *d;
	char **entries, *p = NULL;
	size_t len;

	entries = (char **)ds->attic;
	len = diag_mmap_file(entries[i], &p);
	d = diag_customized_datum_create((uintptr_t)entries[i],
					 (intptr_t)p,
					 finalize);
	d->tag |= (uint64_t)len<<1;
	return (struct diag_datum *)d;
}

int main(int argc, char *argv[])
{
	int c, initial = 0, final = 0, one = 0;
	diag_metric_t metric = hamming;
	diag_cmp_t cmp = DIAG_CMP_IMMEDIATE;
	struct diag_dataset *ds;
	struct diag_singlelinkage *sl;
	struct diag_deque_elem *elem;
	struct diag_set *clusters, *cluster;
	char **entries;
	size_t i, j, num_entries, *num_leaves;

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "Vf:hi:m:1")) >= 0) {
		unsigned int found;
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'f':
			final = atoi(optarg);
			if (final <= 0) {
				usage();
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			initial = atoi(optarg);
			if (initial <= 0) {
				usage();
				exit(EXIT_FAILURE);
			}
			break;
		case 'm':
			found = 0;
			for (i = 0; i < NUM_METRICS; i++) {
				if (strcmp(metrics[i].name, optarg) == 0) {
					metric = metrics[i].metric;
					cmp = metrics[i].cmp;
					found = 1;
					break;
				}
			}
			if (!found) {
				printf("available metrics:\n");
				for (i = 0; i < NUM_METRICS; i++) {
					printf(" %s\n", metrics[i].name);
				}
				exit(EXIT_FAILURE);
			}
			break;
		case '1':
			one = 1;
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}

	entries = diag_paths(&argv[optind], &num_entries);
	if (!entries) {
		exit(EXIT_FAILURE);
	}
	printf("number of entries: %zd\n", num_entries);
	ds = diag_dataset_create(at, (intptr_t)entries);
	ds->size = num_entries;
	sl = diag_singlelinkage_create(ds, metric, cmp);
	sl->initial = (size_t)initial;
	sl->final = (size_t)final;
	if (diag_singlelinkage_analyze(sl) != 0) {
		exit(EXIT_FAILURE);
	}
	clusters = diag_set_create(NULL);
	while ( (elem = diag_deque_pop(sl->t)) ) {
		int found = 0;
		struct diag_couple *p = (struct diag_couple *)elem->attr;
		for (i = 0; i < clusters->size; i++) {
			cluster = (struct diag_set *)clusters->arr[i];
			if (diag_set_contains(cluster, p->j)) {
				found = 1;
				diag_set_insert(cluster, p->i);
				break;
			}
			if (diag_set_contains(cluster, p->i)) {
				found = 1;
				diag_set_insert(cluster, p->j);
				break;
			}
		}
		if (!found) {
			cluster = diag_set_create(NULL);
			diag_set_insert(cluster, p->i);
			diag_set_insert(cluster, p->j);
			diag_set_insert(clusters, (intptr_t)cluster);
		}
		diag_free(p);
		diag_free(elem);
	}
	/* count leaves for each cluster */
	if (!one) {
		num_leaves = diag_calloc(clusters->size, sizeof(size_t));
		for (i = 0; i < clusters->size; i++) {
			cluster = (struct diag_set *)clusters->arr[i];
			for (j = 0; j < cluster->size; j++) {
				size_t k = (size_t)cluster->arr[j];
				if (k < ds->size && ++num_leaves[i] > 1) {
					break;
				}
			}
		}
	}
	for (i = 0; i < clusters->size; i++) {
		cluster = (struct diag_set *)clusters->arr[i];
		if (one || num_leaves[i] > 1) {
			printf("= cluster %zd:\n", i);
			for (j = 0; j < cluster->size; j++) {
				size_t k = (size_t)cluster->arr[j];
				if (k < ds->size) printf("%s\n", entries[k]);
			}
		}
		diag_set_destroy(cluster);
	}
	if (!one) diag_free(num_leaves);
	diag_set_destroy(clusters);
	diag_singlelinkage_destroy(sl);
	diag_dataset_destroy(ds);
	for (i = 0; i < num_entries; i++) {
		free(entries[i]); /* free memory given by strdup() */
	}
	diag_free(entries);
	return EXIT_SUCCESS;
}
