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
#include "diagonal/datum.h"
#include "diagonal/dataset.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/rbtree.h"
#include "diagonal/set.h"
#include "diagonal/singlelinkage.h"

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
	char *name;
	diag_metric_t metric;
} metrics[] = {
	{"hamming",     hamming},
	{"levenshtein", levenshtein}
};

#define NUM_METRICS (sizeof(metrics)/sizeof(metrics[0]))

static void usage(void)
{
	diag_printf("diag-file [-m metric] [-i intial] [-f final] path [...]");
}

static int scan_directory(struct diag_rbtree *tree, int i, const char *path)
{
	struct diag_rbtree_node *node;
	int result, slen;
	DIR *dir;
	char *name;
	size_t len;

	assert(tree && path);
	if ( (dir = opendir(path)) == NULL) return -1;
	for (;;) {
		struct dirent *ent = readdir(dir);
		if (!ent) {
			result = (errno) ? -1 : 0;
			break;
		}
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
			continue;
		}
#ifdef NAME_MAX
		len = strlen(path) + NAME_MAX + 2;
#else
		len = strlen(path) + MAXNAMLEN + 2;
#endif
		name = diag_calloc(len, sizeof(*name));
		slen = snprintf(name, len, "%s/%s", path, ent->d_name);
		if (slen < 0 || (int)len <= slen)  {
			diag_free(name);
			result = -1;
			break;
		}
		if (ent->d_type == DT_DIR) {
			if (scan_directory(tree, i + 1, name) == -1) {
				diag_free(name);
				result = -1;
				break;
			}
			continue;
		}
		node = diag_rbtree_node_new((uintptr_t)i, (uintptr_t)name);
		diag_rbtree_insert(tree, node);
	}
	closedir(dir);
	return result;
}

static struct diag_rbtree *map_paths(char **paths)
{
	struct diag_rbtree *tree;
	struct diag_rbtree_node *node;
	const char *path;
	struct stat st;
	char *name;
	size_t i = 0;

	assert(paths);
	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	while ( (path = paths[i++]) ) {
		if (stat(path, &st) == -1) goto fail;
		if (S_ISDIR(st.st_mode)) {
			if (scan_directory(tree, 1, path) == -1) goto fail;
			continue;
		}
		if ( (name = strdup(path)) == NULL) goto fail;
		node = diag_rbtree_node_new((uintptr_t)0, (uintptr_t)name);
		diag_rbtree_insert(tree, node);
	}
	return tree;
 fail:
	diag_rbtree_destroy(tree);
	return NULL;
}

static char **serialize_entries(const struct diag_rbtree *tree,
				size_t *num_entries)
{
	struct diag_rbtree_node *node;
	char **e = NULL;
	size_t i = 0;

	assert(tree);
	if (tree->num_nodes == 0) goto done;
	e = diag_calloc(tree->num_nodes, sizeof(*e));
	node = diag_rbtree_minimum(tree);
	assert(node);
	do {
		e[i++] = (char *)node->attr;
	} while ( (node = diag_rbtree_successor(node)) );
	assert(i == tree->num_nodes);
 done:
	*num_entries = i;
	return e;
}

static size_t mmap_file(const char *path, char **p) {
	int fd;
	struct stat st;
	size_t len;

	if ( (fd = open(path, O_RDONLY)) < 0) diag_fatal("could not open file");
	if (fstat(fd, &st) < 0) {
		close(fd);
		diag_fatal("could not stat file");
	}
	if ( (len = st.st_size) == 0) {
		close(fd);
		goto done;
	}
	*p = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (*p == MAP_FAILED) diag_fatal("could not map file: %s", strerror(errno));
 done:
	return len;
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
	len = mmap_file(entries[i], &p);
	d = diag_customized_datum_create((uintptr_t)entries[i],
					 (intptr_t)p,
					 finalize);
	d->tag |= (uint64_t)len<<1;
	return (struct diag_datum *)d;
}

int main(int argc, char *argv[])
{
	int c, initial = 0, final = 0;
	diag_metric_t metric = hamming;
	struct diag_rbtree *tree;
	struct diag_dataset *ds;
	struct diag_singlelinkage *sl;
	struct diag_deque_elem *elem;
	struct diag_set *clusters, *cluster;
	char **entries;
	size_t i, j, num_entries;

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "Vf:hi:m:t:1")) >= 0) {
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
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}

	tree = map_paths(&argv[optind]);
	if (!tree) {
		exit(EXIT_FAILURE);
	}
	entries = serialize_entries(tree, &num_entries);
	if (!entries) {
		exit(EXIT_FAILURE);
	}
	printf("number of entries: %d\n", num_entries);
	ds = diag_dataset_create(at, (intptr_t)entries);
	ds->size = num_entries;
	sl = diag_singlelinkage_create(ds, metric);
	sl->initial = (size_t)initial;
	sl->final = (size_t)final;
	if (diag_singlelinkage_analyze(sl) != 0) {
		exit(EXIT_FAILURE);
	}
	clusters = diag_set_create(NULL);
	while ( (elem = diag_deque_pop(sl->t)) ) {
		int found = 0;
		struct diag_pair *p = (struct diag_pair *)elem->attr;
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
	for (i = 0; i < clusters->size; i++) {
		printf("= cluster %d:\n", i);
		cluster = (struct diag_set *)clusters->arr[i];
		for (j = 0; j < cluster->size; j++) {
			size_t k = (size_t)cluster->arr[j];
			if (k < ds->size) printf("%s\n", entries[k]);
		}
		diag_set_destroy(cluster);
	}
	diag_set_destroy(clusters);
	diag_singlelinkage_destroy(sl);
	diag_dataset_destroy(ds);
	diag_rbtree_destroy(tree);
	for (i = 0; i < num_entries; i++) {
		free(entries[i]); /* free memory given by strdup() */
	}
	diag_free(entries);
	return EXIT_SUCCESS;
}
