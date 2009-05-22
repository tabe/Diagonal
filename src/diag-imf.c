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
#include "diagonal/rbtree.h"
#include "diagonal/metric.h"
#include "diagonal/imf.h"

#define THRESHOLD 100

struct metric_option_s {
	char *name;
	diag_metric_imf_t metric;
} METRICS[] = {
	{"hamming",     diag_hamming_imf},
};

#define NUM_METRICS (sizeof(METRICS)/sizeof(struct metric_option_s))

static void
usage(void)
{
	diag_info("diag-imf [-m metric] [-t threshold] [-1] path ...");
}

static int
scan_directory(diag_rbtree_t *tree, int i, const char *path)
{
	int result;
	DIR *dir;

	assert(tree && path);
	if ( (dir = opendir(path)) == NULL) return -1;
	for (;;) {
		struct dirent *ent = readdir(dir);
		if (ent) {
			char *name;
			size_t len;
			int slen;

			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}
#ifdef NAME_MAX
			len = strlen(path) + NAME_MAX + 2;
#else
			len = strlen(path) + MAXNAMLEN + 2;
#endif
			name = (char *)diag_calloc(len, sizeof(char));
			slen = snprintf(name, len, "%s/%s", path, ent->d_name);
			if (slen < 0 || (int)len <= slen)  {
				diag_free(name);
				result = -1;
				goto done;
			}
			switch (ent->d_type) {
			case DT_DIR:
				if (scan_directory(tree, i + 1, name) == -1) {
					diag_free(name);
					result = -1;
					goto done;
				}
				break;
			default:
				{
					diag_rbtree_node_t *node = diag_rbtree_node_new((diag_rbtree_key_t)i, (diag_rbtree_attr_t)name);
					diag_rbtree_insert(tree, node);
				}
				break;
			}
		} else if (errno) {
			result = -1;
			goto done;
		} else {
			result = 0;
			goto done;
		}
	}
 done:
	closedir(dir);
	return result;
}

static diag_rbtree_t *
map_paths(char **paths)
{
	diag_rbtree_t *tree;
	unsigned int i = 0;

	assert(paths);
	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
#define FAIL(tree) {							\
		/* TODO */								\
		diag_rbtree_destroy(tree);				\
		return NULL;							\
}
	while (paths[i]) {
		const char *path = paths[i];
		char *name;
		struct stat st;
		diag_rbtree_node_t *node;

		if (stat(path, &st) == -1) FAIL(tree);
		if (S_ISDIR(st.st_mode)) {
			if (scan_directory(tree, 1, path) == -1) FAIL(tree);
		} else {
			if ( (name = strdup(path)) == NULL) FAIL(tree);
			node = diag_rbtree_node_new((diag_rbtree_key_t)0, (diag_rbtree_attr_t)name);
			diag_rbtree_insert(tree, node);
			break;
		}
		i++;
	}
	return tree;
}

static char **
serialize_entries(const diag_rbtree_t *tree, unsigned int *num_entries)
{
	diag_rbtree_node_t *node;
	char **e;
	unsigned int i = 0;

	assert(tree);
	if (tree->num_nodes == 0) {
		*num_entries = 0;
		return NULL;
	}
	e = (char **)diag_calloc(tree->num_nodes, sizeof(char *));
	node = diag_rbtree_minimum(tree);
	assert(node);
	do {
		e[i++] = (char *)node->attr;
	} while ( (node = diag_rbtree_successor(node)) );
	assert(i == (unsigned int)tree->num_nodes);
	*num_entries = i;
	return e;
}

#define MMAP_IMF(path, p, len) do {										\
		int fd;															\
		struct stat st;													\
																		\
		if ( (fd = open(path, O_RDONLY)) < 0) diag_fatal("could not open file"); \
		if (fstat(fd, &st) < 0) {										\
			close(fd);													\
			diag_fatal("could not stat file");							\
		}																\
		(len) = st.st_size;												\
		(p) = (char *)mmap(NULL, (len), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0); \
		close(fd);														\
		if ((p) == MAP_FAILED) diag_fatal("could not map file");		\
	} while (0)

static diag_rbtree_t *
aggregate_combinations(char **entries, unsigned int num_entries, diag_metric_imf_t metric)
{
	diag_rbtree_t *comb;
	unsigned int i, j;

	if (num_entries == 0) return NULL;
	comb = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	for (i = 0; i < num_entries; i++) {
		char *px;
		size_t lx;
		diag_imf_t *imfx;
		int r;

		MMAP_IMF((char *)entries[i], px, lx);
		r = diag_imf_parse(px, &imfx, 1);
		for (j = i + 1; j < num_entries; j++) {
			diag_rbtree_node_t *node;
			diag_rbtree_key_t k;
			unsigned int *pair;
			char *py;
			size_t ly;
			diag_imf_t *imfy;

			pair = (unsigned int *)diag_calloc(2, sizeof(unsigned int));
			pair[0] = i + 1;
			pair[1] = j + 1;
			if (r < 0) {
				k = (diag_rbtree_key_t)(lx + ly);
			} else {
				MMAP_IMF((char *)entries[j], py, ly);
				if (diag_imf_parse(py, &imfy, 1) < 0) {
					k = (diag_rbtree_key_t)(lx + ly);
				} else {
					k = (diag_rbtree_key_t)(*metric)(imfx, imfy);
					diag_imf_destroy(imfy);
				}
				munmap(py, ly);
			}
			node = diag_rbtree_node_new(k, (diag_rbtree_attr_t)pair);
			diag_rbtree_insert(comb, node);
		}
		if (r >= 0) diag_imf_destroy(imfx);
		munmap(px, lx);
	}
	return comb;
}

static unsigned int *
process_equivalence_relations(const diag_rbtree_t *comb, unsigned int num_entries, int t, unsigned int **occur)
{
	diag_rbtree_node_t *node;
	unsigned int *p, i = 0;

	if (num_entries == 0) return NULL;
	p = (unsigned int *)diag_calloc(num_entries + 1, sizeof(unsigned int));
	if (occur) *occur = (unsigned int *)diag_calloc(num_entries + 1, sizeof(unsigned int));
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
display_imf(unsigned int i, char *path)
{
	char *p;
	size_t len;
	diag_imf_t *imf;

	printf("%03d| %s\n", i, path);
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
display_group_members(char **entries, unsigned int num_entries, const unsigned int *parent, unsigned int i)
{
	unsigned int j;

	for (j = 1; j <= num_entries; j++) {
		if (j != i && parent[j] == i) {
			display_imf(j, entries[j-1]);
			display_group_members(entries, num_entries, parent, j);
		}
	}
}

static void
display_groups(char **entries, unsigned int num_entries, const unsigned int *parent, unsigned int *occur)
{
	unsigned int i;

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
	diag_metric_imf_t metric = diag_hamming_imf;
	diag_rbtree_t *tree, *comb;
	char **entries;
	unsigned int i, num_entries, *parent, *occur;

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

	tree = map_paths(&argv[optind]);
	if (!tree) {
		exit(EXIT_FAILURE);
	}
	entries = serialize_entries(tree, &num_entries);
	diag_rbtree_destroy(tree);
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
		diag_free(entries[i]);
	}
	diag_free(entries);
	return EXIT_SUCCESS;
}
