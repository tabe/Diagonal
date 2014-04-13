/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static intptr_t hamming(intptr_t a, intptr_t b)
{
	const struct diag_datum *x, *y;
	size_t i, smin, smax;
	intptr_t d = 0;

	x = (const struct diag_datum *)a;
	y = (const struct diag_datum *)b;
	const struct diag_mmap *xmm = (const struct diag_mmap *)x->value;
	const struct diag_mmap *ymm = (const struct diag_mmap *)y->value;
	if (xmm->size < ymm->size) {
		smin = xmm->size;
		smax = ymm->size;
	} else {
		smin = ymm->size;
		smax = xmm->size;
	}
	for (i = 0; i < smin; i++) {
		if (((char *)xmm->addr)[i] != ((char *)ymm->addr)[i]) d++;
	}
	return d + (intptr_t)(smax - smin);
}

static intptr_t levenshtein(intptr_t x, intptr_t y)
{
	size_t d = 0, i, j, lx, ly, *cur, *new, *tmp;
	const struct diag_datum *dx, *dy;
	char cx, cy;

	dx = (const struct diag_datum *)x;
	dy = (const struct diag_datum *)y;
	assert(dx && dy);
	const struct diag_mmap *xmm = (const struct diag_mmap *)dx->value;
	const struct diag_mmap *ymm = (const struct diag_mmap *)dy->value;
	lx = xmm->size;
	ly = xmm->size;
	if (lx == 0) return (intptr_t)ly;
	if (ly == 0) return (intptr_t)lx;
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
			cx = ((char *)xmm->addr)[i];
			cy = ((char *)ymm->addr)[j];
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
	return (intptr_t)d;
}

static intptr_t hash32(intptr_t a, intptr_t b)
{
	const struct diag_datum *x, *y;
	size_t lx, ly;
	uint32_t *p, *pe, *q, *qe;
	intptr_t d = 0;

	x = (const struct diag_datum *)a;
	y = (const struct diag_datum *)b;
	const struct diag_mmap *xmm = (const struct diag_mmap *)x->value;
	const struct diag_mmap *ymm = (const struct diag_mmap *)y->value;
	lx = xmm->size>>2; /* number of 32 bits words */
	ly = ymm->size>>2; /* number of 32 bits words */
	p = (uint32_t *)xmm->addr;
	pe = p + lx;
	q = (uint32_t *)ymm->addr;
	qe = q + ly;
	while (p < pe && q < qe) {
		if (*p < *q) {
			p++, d++;
		} else if (*p > *q) {
			q++, d++;
		} else {
			p++, q++;
		}
	}
	return d;
}

static intptr_t hash32_rev(intptr_t a, intptr_t b)
{
	const struct diag_datum *x, *y;
	size_t lx, ly;
	uint32_t *p, *pe, *q, *qe;
	intptr_t d = 0;

	x = (const struct diag_datum *)a;
	y = (const struct diag_datum *)b;
	const struct diag_mmap *xmm = (const struct diag_mmap *)x->value;
	const struct diag_mmap *ymm = (const struct diag_mmap *)y->value;
	lx = xmm->size>>2; /* number of 32 bits words */
	ly = ymm->size>>2; /* number of 32 bits words */
	p = (uint32_t *)xmm->addr;
	pe = p + lx;
	q = (uint32_t *)ymm->addr;
	qe = q + ly;
	while (p < pe && q < qe) {
		if (*p < *q) {
			p++;
		} else if (*p > *q) {
			q++;
		} else {
			p++, q++, d++;
		}
	}
	return d;
}

static int cmp_imm_rev(intptr_t x, intptr_t y)
{
	return (x < y) - (x > y);
}

static struct {
	const char *name;
	diag_metric_t metric;
	diag_cmp_t cmp;
} metrics[] = {
	{"hamming",     hamming,     DIAG_CMP_IMMEDIATE},
	{"levenshtein", levenshtein, DIAG_CMP_IMMEDIATE},
	{"hash32",      hash32,      DIAG_CMP_IMMEDIATE},
	{"hash32_rev",  hash32_rev,  cmp_imm_rev}
};

#define NUM_METRICS (sizeof(metrics)/sizeof(metrics[0]))

static void usage(void)
{
	diag_printf("diag-file [-m metric] [-I intial] [-F final] [-1] file [...]");
	diag_printf("diag-file [-m metric] [-I intial] [-F final] [-1] -i input");
}

static void finalize(struct diag_datum *d)
{
	diag_munmap((struct diag_mmap *)d->value);
}

static struct diag_datum *at(size_t i, struct diag_dataset *ds)
{
	char **entries = (char **)ds->attic;
	struct diag_mmap *mm = diag_mmap_file(entries[i], DIAG_MMAP_RO);
	if (!mm) diag_fatal("could not map file: %s", entries[i]);
	struct diag_customized_datum *d = diag_customized_datum_create((uintptr_t)entries[i],
								       (intptr_t)mm,
								       finalize);
	return (struct diag_datum *)d;
}

int main(int argc, char *argv[])
{
	int c, initial = 0, final = 0, one = 0;
	char *input = NULL;
	diag_metric_t metric = hamming;
	diag_cmp_t cmp = DIAG_CMP_IMMEDIATE;
	struct diag_dataset *ds;
	struct diag_singlelinkage *sl;
	struct diag_deque_elem *elem;
	struct diag_set *clusters, *cluster;
	char **args = NULL, **entries, *dst = NULL;
	size_t i, j, num_entries, *num_leaves = NULL;

	diag_init();

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "F:I:Vhi:m:1")) >= 0) {
		unsigned int found;
		switch (c) {
		case 'F':
			final = atoi(optarg);
			if (final <= 0) {
				usage();
				exit(EXIT_FAILURE);
			}
			break;
		case 'I':
			initial = atoi(optarg);
			if (initial <= 0) {
				usage();
				exit(EXIT_FAILURE);
			}
			break;
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			input = optarg;
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
		case ':':
		case '?':
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}

	if (input) {
		size_t nl;
		nl = diag_file_to_lines(input, &dst, &args);
		if (!nl) {
			diag_fatal("no path listed in %s", input);
		}
	} else {
		    if (!argv[optind]) {
			    usage();
			    exit(EXIT_FAILURE);
		    }
		    args = &argv[optind];
	}
	entries = diag_paths(args, &num_entries);
	if (!entries) {
		exit(EXIT_FAILURE);
	}
	printf("number of entries: %ld\n", (long int)num_entries);
	fflush(stdout);
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
			if (diag_set_contains(cluster, (intptr_t)p->j)) {
				found = 1;
				diag_set_insert(cluster, (intptr_t)p->i);
				break;
			}
			if (diag_set_contains(cluster, (intptr_t)p->i)) {
				found = 1;
				diag_set_insert(cluster, (intptr_t)p->j);
				break;
			}
		}
		if (!found) {
			cluster = diag_set_create(NULL);
			diag_set_insert(cluster, (intptr_t)p->i);
			diag_set_insert(cluster, (intptr_t)p->j);
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
			printf("= cluster %ld:\n", (long int)i);
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
		diag_free(entries[i]);
	}
	diag_free(entries);
	if (input) {
		diag_free(args);
		diag_free(dst);
	}
	return EXIT_SUCCESS;
}
