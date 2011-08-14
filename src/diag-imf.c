/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/datum.h"
#include "diagonal/metric.h"
#include "diagonal/rbtree.h"
#include "diagonal/imf.h"
#include "diagonal/private/filesystem.h"

#define THRESHOLD 100

struct metric_option_s {
	char *name;
	diag_metric_t metric;
} METRICS[] = {
	{"hamming", diag_hamming_imf},
};

#define NUM_METRICS (sizeof(METRICS)/sizeof(struct metric_option_s))

static void
usage(void)
{
	diag_printf("diag-imf [-m metric] [-t threshold] [-1] path ...");
}

#define MMAP_IMF(path, p, len) do {					\
		int fd;							\
		struct stat st;						\
									\
		if ( (fd = open(path, O_RDONLY)) < 0) diag_fatal("could not open file"); \
		if (fstat(fd, &st) < 0) {				\
			close(fd);					\
			diag_fatal("could not stat file");		\
		}							\
		(len) = st.st_size;					\
		(p) = (char *)mmap(NULL, (len), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0); \
		close(fd);						\
		if ((p) == MAP_FAILED) diag_fatal("could not map file"); \
	} while (0)

static struct diag_rbtree *
aggregate_combinations(char **entries, size_t num_entries, diag_metric_t metric)
{
	struct diag_rbtree *comb;
	size_t i, j;

	if (num_entries == 0) return NULL;
	comb = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	for (i = 0; i < num_entries; i++) {
		char *px;
		size_t lx;
		struct diag_imf *imfx;
		int r;

		MMAP_IMF((char *)entries[i], px, lx);
		r = diag_imf_parse(px, &imfx, 1);
		for (j = i + 1; j < num_entries; j++) {
			struct diag_rbtree_node *node;
			uintptr_t k;
			unsigned int *pair;
			char *py;
			size_t ly;
			struct diag_imf *imfy;

			pair = diag_calloc(2, sizeof(unsigned int));
			pair[0] = i + 1;
			pair[1] = j + 1;
			if (r < 0) {
				k = (uintptr_t)(lx + ly);
			} else {
				MMAP_IMF((char *)entries[j], py, ly);
				if (diag_imf_parse(py, &imfy, 1) < 0) {
					k = (uintptr_t)(lx + ly);
				} else {
					k = metric((intptr_t)imfx, (intptr_t)imfy);
					diag_imf_destroy(imfy);
				}
				munmap(py, ly);
			}
			node = diag_rbtree_node_new(k, (uintptr_t)pair);
			diag_rbtree_insert(comb, node);
		}
		if (r >= 0) diag_imf_destroy(imfx);
		munmap(px, lx);
	}
	return comb;
}

static unsigned int *
process_equivalence_relations(const struct diag_rbtree *comb, size_t num_entries, int t, unsigned int **occur)
{
	struct diag_rbtree_node *node;
	unsigned int *p, i = 0;

	if (num_entries == 0) return NULL;
	p = diag_calloc(num_entries + 1, sizeof(unsigned int));
	if (occur) *occur = diag_calloc(num_entries + 1, sizeof(unsigned int));
	node = diag_rbtree_minimum(comb);
	do {
		unsigned int k = (unsigned int)node->key;

		if ((int)k < t) {
			unsigned int x, y;

			i++;
			x = ((unsigned int *)node->attr)[0];
			y = ((unsigned int *)node->attr)[1];
			if (occur) (*occur)[x] = (*occur)[y] = 1;
			while (p[x] > 0) x = p[x];
			while (p[y] > 0) y = p[y];
			if (x != y) p[x] = y;
		} else {
			break;
		}
	} while ( (node = diag_rbtree_successor(node)) );
	return p;
}

static void
display_imf(size_t i, char *path)
{
	char *p;
	size_t len;
	struct diag_imf *imf;

	printf("%03zd| %s\n", i, path);
	MMAP_IMF(path, p, len);
	if (diag_imf_parse(p, &imf, 1) < 0) {
		printf("--- (parse error)\n");
	} else {
		char *n;

		n = imf->body;
		while (*n) {
			switch (*n) {
			case '\r':
			case '\n':
				*n = '\0';
				goto output;
			default:
				n++;
				break;
			}
		}
	output:
		printf("--- %s\n", imf->body);
		diag_imf_destroy(imf);
	}
	munmap(p, len);
}

static void
display_group_members(char **entries, size_t num_entries, const unsigned int *parent, size_t i)
{
	size_t j;

	for (j = 1; j <= num_entries; j++) {
		if (j != i && parent[j] == i) {
			display_imf(j, entries[j-1]);
			display_group_members(entries, num_entries, parent, j);
		}
	}
}

static void
display_groups(char **entries, size_t num_entries, const unsigned int *parent, unsigned int *occur)
{
	size_t i;

	for (i = 1; i <= num_entries; i++) {
		if ((!occur || occur[i]) && parent[i] == 0) {
			display_imf(i, entries[i-1]);
			display_group_members(entries, num_entries, parent, i);
			printf("\n");
		}
	}
}

int
main(int argc, char *argv[])
{
	int c, t = THRESHOLD, one = 0;
	diag_metric_t metric = diag_hamming_imf;
	struct diag_rbtree *comb;
	char **entries;
	unsigned int *parent, *occur;
	size_t i, num_entries;

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "Vhm:t:1")) >= 0) {
		unsigned int found;
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'm':
			found = 0;
			for (i = 0; i < NUM_METRICS; i++) {
				if (strcmp(METRICS[i].name, optarg) == 0) {
					metric = METRICS[i].metric;
					found = 1;
					break;
				}
			}
			if (!found) {
				printf("available metrics:\n");
				for (i = 0; i < NUM_METRICS; i++) {
					printf(" %s\n", METRICS[i].name);
				}
				exit(EXIT_FAILURE);
			}
			break;
		case 't':
			t = atoi(optarg);
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
	comb = aggregate_combinations(entries, num_entries, metric);
	parent = process_equivalence_relations(comb, num_entries, t, (one) ? NULL : &occur);
	diag_rbtree_for_each_attr(comb, (diag_rbtree_callback_attr_t)diag_free);
	diag_rbtree_destroy(comb);
	if (parent) display_groups(entries, num_entries, parent, (one) ? NULL : occur);
	diag_free(parent);
	diag_free(occur);
	for (i = 0; i < num_entries; i++) {
		free(entries[i]); /* free memory given by strdup() */
	}
	diag_free(entries);
	return EXIT_SUCCESS;
}
