#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#include "diagonal/line.h"

#define THRESHOLD 10

struct metric_option_s {
	char *name;
	diag_emetric_chars_t metric;
} METRICS[] = {
	{"hamming",     diag_ehamming_chars},
	{"levenshtein", diag_elevenshtein_chars},
};

#define NUM_METRICS (sizeof(METRICS)/sizeof(struct metric_option_s))

static void
usage(void)
{
	diag_info("diag-line [-m metric] [-t threshold] [-1] [path]");
}

static void *
map_file(const char *path, diag_rbtree_t *tree, size_t *plen)
{
	int fd, r;
	struct stat st;
	size_t len;
	char *p, *q;

	assert(tree);
	if (path) {
		fd = open(path, O_RDONLY);
		if (fd < 0) diag_fatal("could not open file");
	} else {
		fd = STDIN_FILENO;
	}
	r = fstat(fd, &st);
	if (r < 0) diag_fatal("could not stat file");
	*plen = len = st.st_size;
	if (S_ISREG(st.st_mode) && len == 0) {
		diag_rbtree_destroy(tree);
		exit(0);
	}
	p = q = (char *)mmap(NULL, len + 1, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (p == MAP_FAILED) {
		diag_port_t *port;
		diag_line_context_t *context;
		diag_rbtree_node_t *node;
		size_t n = 0;

		port = diag_port_new_fd(fd, DIAG_PORT_INPUT);
		context = diag_line_context_new(port);
		for (;;) {
			context = diag_line_read(context, NULL, &q);
			if (DIAG_LINE_HAS_ERROR(context)) break;
			node = diag_rbtree_node_new((diag_rbtree_key_t)n++, (void *)q);
			diag_rbtree_insert(tree, node);
		}
		diag_line_context_destroy(context);
		diag_port_destroy(port);
		close(fd);
	} else {
		close(fd);
		*(p + len) = '\0';
		while (q < p + len) {
			diag_rbtree_node_t *node = diag_rbtree_node_new((diag_rbtree_key_t)(q - p), (void *)q);
			diag_rbtree_insert(tree, node);
			do {
				if (*q == '\n') {
					*q++ = '\0';
					break;
				}
			} while (++q < p + len);
		}
	}
	return (void *)p;
}

static char **
serialize_entries(const diag_rbtree_t *tree, unsigned int *num_entries)
{
	diag_rbtree_node_t *node;
	char **e;
	register unsigned int i = 0;

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

static diag_rbtree_t *
aggregate_combinations(char **entries, register unsigned int num_entries, diag_emetric_chars_t metric, int t)
{
	diag_rbtree_t *comb;
	register unsigned int i, j;
	diag_rbtree_node_t *node;
	diag_sdistance_t k;
	unsigned int *p;

	if (num_entries == 0) return NULL;
	comb = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	for (i = 0; i < num_entries; i++) {
		for (j = i + 1; j < num_entries; j++) {
			k = (*metric)((char *)entries[i], (char *)entries[j], (diag_distance_t)t);
			if (k >= 0) {
				p = (unsigned int *)diag_calloc(2, sizeof(unsigned int));
				p[0] = i + 1;
				p[1] = j + 1;
				node = diag_rbtree_node_new((diag_rbtree_key_t)k, (void *)p);
				diag_rbtree_insert(comb, node);
			}
		}
	}
	return comb;
}

static void
display_combinations(const diag_rbtree_t *comb, register int field_width)
{
	printf("%d\n", comb->num_nodes);
	if (comb->num_nodes > 0) {
		diag_rbtree_node_t *n;

		n = diag_rbtree_minimum(comb);
		do {
			printf("%0*d-%0*d: %d\n",
				   field_width, ((unsigned int *)n->attr)[0],
				   field_width, ((unsigned int *)n->attr)[1],
				   (unsigned int)n->key);
		} while ( (n = diag_rbtree_successor(n)) );
	}
}

static unsigned int *
process_equivalence_relations(const diag_rbtree_t *comb, unsigned int num_entries, unsigned int **occur)
{
	diag_rbtree_node_t *node;
	unsigned int *p;
	register unsigned int x, y;

	if (num_entries == 0) return NULL;
	p = (unsigned int *)diag_calloc(num_entries + 1, sizeof(unsigned int));
	if (occur) *occur = (unsigned int *)diag_calloc(num_entries + 1, sizeof(unsigned int));
	if (comb->num_nodes == 0) return p;
	node = diag_rbtree_minimum(comb);
	do {
		x = ((unsigned int *)node->attr)[0];
		y = ((unsigned int *)node->attr)[1];
		if (occur) (*occur)[x] = (*occur)[y] = 1;
		while (p[x] > 0) x = p[x];
		while (p[y] > 0) y = p[y];
		if (x != y) p[x] = y;
	} while ( (node = diag_rbtree_successor(node)) );
	return p;
}

static void
display_equivalence_relations(const unsigned int *parent, register unsigned int n, register int field_width)
{
	register unsigned int i;

	for (i = 1; i <= n; i++) {
		printf("%0*d -> %0*d\n", field_width, i, field_width, parent[i]);
	}
}

static void
display_group_members(char **entries, register unsigned int num_entries, const unsigned int *parent, register unsigned int i, register int field_width)
{
	register unsigned int j;

	for (j = 1; j <= num_entries; j++) {
		if (j != i && parent[j] == i) {
			printf("%*d %s\n", field_width, j, entries[j-1]);
			display_group_members(entries, num_entries, parent, j, field_width);
		}
	}
}

static void
display_groups(char **entries, register unsigned int num_entries, const unsigned int *parent, unsigned int *occur, register int field_width)
{
	register unsigned int i;

	for (i = 1; i <= num_entries; i++) {
		if ((!occur || occur[i]) && parent[i] == 0) {
			printf("%*d %s\n", field_width, i, entries[i-1]);
			display_group_members(entries, num_entries, parent, i, field_width);
			printf("\n");
		}
	}
}

static void
free_attr(diag_rbtree_key_t key, void *attr)
{
	diag_free(attr);
}

int
main(int argc, char *argv[])
{
	int c, t = THRESHOLD, one = 0;
	int field_width;
	diag_emetric_chars_t metric = diag_ehamming_chars;
	size_t len;
	void *p;
	diag_rbtree_t *tree, *comb;
	char **entries;
	unsigned int num_entries, *parent, *occur = NULL;

	while ( (c = getopt(argc, argv, "Vhm:t:1")) >= 0) {
		unsigned int i, found;
		switch (c) {
		case 'V':
			diag_print_version();
			exit(0);
			break;
		case 'h':
			usage();
			exit(0);
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
				exit(1);
			}
			break;
		case 't':
			t = atoi(optarg);
			break;
		case '1':
			one = 1;
			break;
		default:
			break;
		}
	}

	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	p = map_file(argv[optind], tree, &len);
	entries = serialize_entries(tree, &num_entries);
	diag_rbtree_destroy(tree);
	if (!entries) goto done;

	field_width = snprintf(NULL, 0, "%d", num_entries);
	comb = aggregate_combinations(entries, num_entries, metric, t);
#if 0
	display_combinations(comb, field_width);
#endif
	parent = process_equivalence_relations(comb, num_entries, (one) ? NULL : &occur);
	diag_rbtree_for_each(comb, free_attr);
	diag_rbtree_destroy(comb);
#if 0
	display_equivalence_relations(parent, num_entries, field_width);
#endif
	if (parent) display_groups(entries, num_entries, parent, (one) ? NULL : occur, field_width);
	diag_free(parent);
	diag_free(occur);
	diag_free(entries);
 done:
	if (p == MAP_FAILED) {
		
	} else {
		munmap(p, len + 1);
	}
	return 0;
}
