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
#include "diagonal/deque.h"
#include "diagonal/rbtree.h"
#include "diagonal/metric.h"
#include "diagonal/imf.h"
#include "diagonal/cluster.h"

#define THRESHOLD 10

struct metric_option_s {
	char *name;
	diag_metric_chars_t metric;
} METRICS[] = {
	{"hamming",     diag_hamming_chars},
	{"levenshtein", diag_levenshtein_chars},
};

#define NUM_METRICS (sizeof(METRICS)/sizeof(struct metric_option_s))

static void
usage(void)
{
	diag_info("diag-enc [-m metric] [-t threshold] [-1] file");
}

static void *
map_file(const char *path, diag_rbtree_t *tree, size_t *plen)
{
	int fd, r;
	struct stat st;
	size_t len;
	char *p, *q;

	assert(path && tree);
	fd = open(path, O_RDONLY);
	if (fd < 0) diag_fatal("could not open file");
	r = fstat(fd, &st);
	if (r < 0) diag_fatal("counld not stat file");
	*plen = len = st.st_size;
	p = q = (char *)mmap(NULL, len + 1, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (p == MAP_FAILED) diag_fatal("could not map file");
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
	return (void *)p;
}

static char **
serialize_entries(const diag_rbtree_t *tree, unsigned int *num_entries)
{
	diag_rbtree_node_t *node;
	char **e;
	unsigned int i = 0;

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
aggregate_combinations(char **entries, unsigned int num_entries, diag_metric_chars_t metric)
{
	diag_rbtree_t *comb;
	unsigned int i, j;

	if (num_entries == 0) return NULL;
	comb = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	for (i = 0; i < num_entries; i++) {
		for (j = i + 1; j < num_entries; j++) {
			diag_rbtree_node_t *node;
			diag_rbtree_key_t k;
			unsigned int *p;

			p = (unsigned int *)diag_calloc(2, sizeof(unsigned int));
			p[0] = i + 1;
			p[1] = j + 1;
			k = (diag_rbtree_key_t)(*metric)((char *)entries[i], (char *)entries[j]);
			node = diag_rbtree_node_new(k, (void *)p);
			diag_rbtree_insert(comb, node);
		}
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
display_clusters(diag_analysis_t *analysis, int one)
{
	diag_size_t i;

	for (i = 0; i < analysis->num_clusters; i++) {
		diag_cluster_t *cluster;
		diag_size_t j;

		cluster = analysis->clusters[i];
		switch (cluster->num_data) {
		case 1:
			if (!one) break;
			/* FALLTHROUGH */
		default:
			for (j = 0; j < cluster->num_data; j++) {
				printf("%03d|%s\n", (unsigned int)cluster->data[j]->id.number, (char *)cluster->data[j]->value);
			}
			printf("\n");
			break;
		}
	}
}

static diag_distance_t
metric_log(const diag_datum_t *d1, const diag_datum_t *d2)
{
	return diag_hamming_chars((const char *)d1->value, (const char *)d2->value);
}

static diag_code_t *
encode_log(diag_cluster_t *cluster, diag_datum_t *datum)
{
	diag_code_t *code;

	assert(cluster && datum);
	code = diag_delta_hamming_chars(cluster, (const char *)cluster->data[0]->value, (const char *)datum->value);
	return code;
}

static void
process_cluster_data(diag_datum_t **data, unsigned int num_data, const unsigned int *parent, diag_size_t i, diag_deque_t *d)
{
	diag_size_t j;

	for (j = 1; j <= num_data; j++) {
		if (j != i && parent[j] == i) {
			diag_deque_push(d, (void *)data[j-1]);
			process_cluster_data(data, num_data, parent, j, d);
		}
	}
}

static diag_analysis_t *
analyze(char **entries, diag_size_t num_entries, const diag_size_t *parent)
{
	diag_analysis_t *analysis;
	diag_datum_t **data;
	diag_deque_t *deque;
	diag_deque_elem_t *elem;
	diag_size_t i;

	data = (diag_datum_t **)diag_calloc((size_t)num_entries, sizeof(diag_datum_t *));
	for (i = 0; i < num_entries; i++) {
		data[i] = diag_datum_new((void *)entries[i]);
		data[i]->id.number = i;
	}
	analysis = diag_analysis_new(num_entries, data);
	analysis->metric = metric_log;
	analysis->encoder = encode_log;

	deque = diag_deque_new();
	for (i = 1; i <= num_entries; i++) {
		if (parent[i] == 0) {
			diag_deque_t *d;

			d = diag_deque_new();
			diag_deque_push(d, (void *)data[i-1]);
			process_cluster_data(data, num_entries, parent, i, d);
			diag_deque_push(deque, (void *)d);
		}
	}
	analysis->num_clusters = deque->length;
	analysis->clusters = (diag_cluster_t **)diag_calloc((size_t)deque->length, sizeof(diag_cluster_t *));
	i = 0;
	DIAG_DEQUE_FOR_EACH(deque, elem) {
		diag_cluster_t *c;
		diag_deque_t *d;
		diag_deque_elem_t *e;
		diag_size_t j;

		d = (diag_deque_t *)elem->attr;
		c = diag_cluster_new(d->length);
		j = 0;
		DIAG_DEQUE_FOR_EACH(d, e) {
			diag_datum_t *da = (diag_datum_t *)e->attr;
			c->data[j++] = da;
		}
		analysis->clusters[i++] = c;
		diag_deque_destroy(d);
	}
	diag_deque_destroy(deque);

	return analysis;
}

static void
display_codes(diag_analysis_t *analysis)
{
	diag_size_t i;

	for (i = 0; i < analysis->num_data; i++) {
		diag_code_t *code;
		diag_size_t j;

		code = analysis->codes[i];
		if (!code) continue;
		if (!code->cluster) continue;
		if (!code->cluster->data[0]) continue;
		printf("[%s]\n", (char *)code->cluster->data[0]->value);
		for (j = 0; j < code->num_deltas; j++) {
			diag_delta_t *d;

			d = code->deltas[j];
			switch (d->type) {
			case DIAG_DELTA_REPLACE:
				printf("\t[REPLACE:%d:%c]\n", d->index, *((char *)d->value));
				break;
			case DIAG_DELTA_APPEND:
				printf("\t[APPEND:%s]\n", (char *)d->value);
				break;
			case DIAG_DELTA_TRIM:
				printf("\t[TRIM:%d]\n", d->index);
				break;
			case DIAG_DELTA_INSERT:
				printf("\t[INSERT:%d]\n", d->index);
				break;
			case DIAG_DELTA_DELETE:
				printf("\t[DELETE:%d]\n", d->index);
				break;
			default:
				diag_fatal("unknown delta type: %d", d->type);
				break;
			}
		}
		printf("\n");
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
	diag_metric_chars_t metric = diag_levenshtein_chars;
	size_t len;
	void *p;
	diag_rbtree_t *tree, *comb;
	char **entries;
	unsigned int num_entries, *parent, *occur;

	if (argc < 2) {
		usage();
		exit(1);
	}
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
	if (!argv[optind]) {
		usage();
		exit(1);
	}

	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	p = map_file(argv[optind], tree, &len);
	entries = serialize_entries(tree, &num_entries);
	diag_rbtree_destroy(tree);

	comb = aggregate_combinations(entries, num_entries, metric);
	parent = process_equivalence_relations(comb, num_entries, t, (one) ? NULL : &occur);
	diag_rbtree_for_each(comb, free_attr);
	diag_rbtree_destroy(comb);
	if (parent) {
		diag_analysis_t *analysis;

		analysis = analyze(entries, num_entries, parent);
/* 		display_clusters(analysis, one); */
		diag_encode(analysis);
		display_codes(analysis);
		diag_analysis_destroy(analysis);
	}
	diag_free(parent);
	diag_free(occur);
	diag_free(entries);
	munmap(p, len + 1);
	return 0;
}
