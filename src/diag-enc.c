/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
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
#include "diagonal/deque.h"
#include "diagonal/line.h"
#include "diagonal/metric.h"
#include "diagonal/port.h"
#include "diagonal/rbtree.h"
#include "diagonal/cluster.h"

#define THRESHOLD 10

struct metric_option_s {
	const char *name;
	diag_metric_t metric;
} METRICS[] = {
	{"hamming",     diag_hamming_chars},
	{"levenshtein", diag_levenshtein_chars},
};

#define NUM_METRICS (sizeof(METRICS)/sizeof(struct metric_option_s))

static void
usage(void)
{
	diag_printf("diag-enc [-m metric] [-t threshold] [-1] file");
}

static void map_file(const char *path, struct diag_rbtree *tree)
{
	struct diag_port *port;
	char *p;

	assert(tree);
	if (path) {
		port = diag_port_new_path(path, "r");
		if (!port) diag_fatal("could not open file: %s", path);
	} else {
		port = diag_port_new_fp(stdin, DIAG_PORT_INPUT);
	}
	size_t n = 0;
	struct diag_line_context *context = diag_line_context_new(port);
	struct diag_rbtree_node *node;
	enum diag_line_error_e e;
	for (;;) {
		e = diag_line_read(context, NULL, &p);
		if (e != DIAG_LINE_ERROR_OK) break;
		node = diag_rbtree_node_new((intptr_t)n++, (intptr_t)p);
		diag_rbtree_insert(tree, node);
	}
	diag_line_context_destroy(context);
	diag_port_destroy(port);
}

static char **
serialize_entries(const struct diag_rbtree *tree, size_t *num_entries)
{
	struct diag_rbtree_node *node;
	char **e;
	size_t i = 0;

	if (tree->num_nodes == 0) {
		*num_entries = 0;
		return NULL;
	}
	e = diag_calloc(tree->num_nodes, sizeof(char *));
	node = diag_rbtree_minimum(tree);
	assert(node);
	do {
		e[i++] = (char *)node->attr;
	} while ( (node = diag_rbtree_successor(node)) );
	assert(i == tree->num_nodes);
	*num_entries = i;
	return e;
}

static struct diag_rbtree *
aggregate_combinations(char **entries, size_t num_entries, diag_metric_t metric)
{
	struct diag_rbtree *comb;
	unsigned int i, j;

	if (num_entries == 0) return NULL;
	comb = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	for (i = 0; i < num_entries; i++) {
		for (j = i + 1; j < num_entries; j++) {
			struct diag_rbtree_node *node;
			intptr_t k;
			unsigned int *p;

			p = diag_calloc(2, sizeof(unsigned int));
			p[0] = i + 1;
			p[1] = j + 1;
			k = metric((intptr_t)entries[i], (intptr_t)entries[j]);
			node = diag_rbtree_node_new(k, (intptr_t)p);
			diag_rbtree_insert(comb, node);
		}
	}
	return comb;
}

static size_t *
process_equivalence_relations(const struct diag_rbtree *comb, size_t num_entries, int t, unsigned int **occur)
{
	struct diag_rbtree_node *node;
	size_t *p, i = 0;

	if (num_entries == 0) return NULL;
	p = diag_calloc(num_entries + 1, sizeof(*p));
	if (occur) *occur = diag_calloc(num_entries + 1, sizeof(**occur));
	node = diag_rbtree_minimum(comb);
	do {
		unsigned int k = (unsigned int)node->key;

		if ((int)k < t) {
			uintptr_t x, y;

			i++;
			x = ((uintptr_t *)node->attr)[0];
			y = ((uintptr_t *)node->attr)[1];
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

#if 0
static void
display_clusters(struct diag_analysis *analysis, int one)
{
	size_t i;

	for (i = 0; i < analysis->num_clusters; i++) {
		struct diag_cluster *cluster;
		size_t j;

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
#endif

static intptr_t metric_log(intptr_t x, intptr_t y)
{
	const struct diag_datum *d1, *d2;
	d1 = (const struct diag_datum *)x;
	d2 = (const struct diag_datum *)y;
	return diag_hamming_chars(d1->value, d2->value);
}

static struct diag_code *
encode_log(struct diag_cluster *cluster, struct diag_datum *datum)
{
	struct diag_code *code;

	assert(cluster && datum);
	code = diag_delta_hamming_chars(cluster, (const char *)cluster->data[0]->value, (const char *)datum->value);
	return code;
}

static void
process_cluster_data(struct diag_datum **data, size_t num_data, const size_t *parent, size_t i, struct diag_deque *d)
{
	size_t j;

	for (j = 1; j <= num_data; j++) {
		if (j != i && parent[j] == i) {
			diag_deque_push(d, (intptr_t)data[j-1]);
			process_cluster_data(data, num_data, parent, j, d);
		}
	}
}

static struct diag_analysis *
analyze(char **entries, size_t num_entries, const size_t *parent)
{
	struct diag_analysis *analysis;
	struct diag_datum **data;
	struct diag_deque *deque;
	struct diag_deque_elem *elem;
	size_t i;

	data = diag_calloc((size_t)num_entries, sizeof(struct diag_datum *));
	for (i = 0; i < num_entries; i++) {
		data[i] = diag_datum_create((uintptr_t)i, (intptr_t)entries[i]);
	}
	analysis = diag_analysis_new(num_entries, data);
	analysis->metric = metric_log;
	analysis->encoder = encode_log;

	deque = diag_deque_new();
	for (i = 1; i <= num_entries; i++) {
		if (parent[i] == 0) {
			struct diag_deque *d;

			d = diag_deque_new();
			diag_deque_push(d, (intptr_t)data[i-1]);
			process_cluster_data(data, num_entries, parent, i, d);
			diag_deque_push(deque, (intptr_t)d);
		}
	}
	analysis->num_clusters = deque->length;
	analysis->clusters = diag_calloc((size_t)deque->length, sizeof(struct diag_cluster *));
	i = 0;
	DIAG_DEQUE_FOR_EACH(deque, elem) {
		struct diag_cluster *c;
		struct diag_deque *d;
		struct diag_deque_elem *e;
		size_t j;

		d = (struct diag_deque *)elem->attr;
		c = diag_cluster_new(d->length);
		j = 0;
		DIAG_DEQUE_FOR_EACH(d, e) {
			struct diag_datum *da = (struct diag_datum *)e->attr;
			c->data[j++] = da;
		}
		analysis->clusters[i++] = c;
		diag_deque_destroy(d);
	}
	diag_deque_destroy(deque);

	return analysis;
}

static void
display_codes(struct diag_analysis *analysis)
{
	size_t i;

	for (i = 0; i < analysis->num_data; i++) {
		struct diag_code *code;
		size_t j;

		code = analysis->codes[i];
		if (!code) continue;
		if (!code->cluster) continue;
		if (!code->cluster->data[0]) continue;
		printf("[%s]\n", (char *)code->cluster->data[0]->value);
		for (j = 0; j < code->num_deltas; j++) {
			struct diag_delta *d;

			d = code->deltas[j];
			switch (d->type) {
			case DIAG_DELTA_REPLACE:
				printf("\t[REPLACE:%ld:%c]\n", (long int)d->index, *((const char *)d->value));
				break;
			case DIAG_DELTA_APPEND:
				printf("\t[APPEND:%s]\n", (const char *)d->value);
				break;
			case DIAG_DELTA_TRIM:
				printf("\t[TRIM:%ld]\n", (long int)d->index);
				break;
			case DIAG_DELTA_INSERT:
				printf("\t[INSERT:%ld]\n", (long int)d->index);
				break;
			case DIAG_DELTA_DELETE:
				printf("\t[DELETE:%ld]\n", (long int)d->index);
				break;
			default:
				diag_fatal("unknown delta type: %d", d->type);
				break;
			}
		}
		printf("\n");
	}
}

static void free_attr(intptr_t attr, void *data)
{
	(void)data;
	diag_free((void *)attr);
}

int
main(int argc, char *argv[])
{
	int c, t = THRESHOLD, one = 0;
	diag_metric_t metric = diag_levenshtein_chars;
	size_t num_entries, *parent;
	struct diag_rbtree *tree, *comb;
	char **entries;
	unsigned int *occur = NULL;

	diag_init();

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "Vhm:t:1")) >= 0) {
		unsigned int i, found;
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
		case ':':
		case '?':
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}

	tree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	map_file(argv[optind], tree);
	entries = serialize_entries(tree, &num_entries);
	diag_rbtree_destroy(tree);

	comb = aggregate_combinations(entries, num_entries, metric);
	parent = process_equivalence_relations(comb, num_entries, t, (one) ? NULL : &occur);
	diag_rbtree_for_each_attr(comb, free_attr, NULL);
	diag_rbtree_destroy(comb);
	if (parent) {
		struct diag_analysis *analysis;

		analysis = analyze(entries, num_entries, parent);
#if 0
 		display_clusters(analysis, one);
#endif
		diag_encode(analysis);
		display_codes(analysis);
		diag_analysis_destroy(analysis);
	}
	diag_free(parent);
	diag_free(occur);
	diag_free(entries);
	return EXIT_SUCCESS;
}
