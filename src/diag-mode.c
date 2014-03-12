/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/line.h"
#include "diagonal/port.h"
#include "diagonal/rbtree.h"
#include "diagonal/vector.h"

static void free_vector(intptr_t attr, void *data)
{
	(void)data;

	struct diag_vector *vec = (struct diag_vector *)attr;
	diag_vector_destroy(vec);
}

static void transpose(intptr_t key, intptr_t attr, void *data)
{
	struct diag_rbtree *xtree = data;
	assert(xtree);
	struct diag_vector *indices = (struct diag_vector *)attr;
	assert(indices);
	size_t len = diag_vector_length(indices);
	struct diag_rbtree_node *node;
	struct diag_vector *keys;
	if (diag_rbtree_search(xtree, (intptr_t)len, &node) == DIAG_SUCCESS) {
		keys = (struct diag_vector *)node->attr;
		diag_vector_push_back(keys, key);
	} else {
		keys = diag_vector_create(1);
		diag_vector_set(keys, 0, key);
		node = diag_rbtree_node_new((intptr_t)len, (intptr_t)keys);
		(void)diag_rbtree_insert(xtree, node);
	}
}

static void usage(void)
{
	diag_printf("diag-mode [-e exponent]");
}

int main(int argc, char *argv[])
{
	int c, r = EXIT_FAILURE;
	int exponent = 0;

	diag_init();

	while ( (c = getopt(argc, argv, "+Ve:h")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'e':
			exponent = atoi(optarg);
			if (32 < exponent) {
				diag_fatal("exponent is too large: %s", optarg);
			}
			if (exponent < -32) {
				diag_fatal("exponent is too small: %s", optarg);
			}
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case ':':
		case '?':
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}

	struct diag_port *iport = diag_port_new_stdin();
	if (!iport) {
		return EXIT_FAILURE;
	}
	struct diag_line_context *context = diag_line_context_new(iport);
	if (!context) {
		diag_port_destroy(iport);
		return EXIT_FAILURE;
	}
	enum diag_line_error_e e;
	size_t size;
	char *line;
	size_t n = 0;
	size_t capacity = 32;
	double *data = diag_calloc(capacity, sizeof(double));
	struct diag_rbtree *tree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	struct diag_rbtree *xtree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	double coeff = pow((double)2, (double)exponent);
	struct diag_rbtree_node *node;
	struct diag_vector *indices;
	struct diag_vector *keys;
	while ( (e = diag_line_read(context, &size, &line)) == DIAG_LINE_ERROR_OK) {
		char *nptr = line;
		char *end;
		errno = 0;
		double v = strtod(nptr, &end);
		if (v == 0 && line == end) {
			diag_error("failed to convert to number");
			diag_free(line);
			goto done;
		}
		if (v == HUGE_VAL || v == -HUGE_VAL) {
			diag_error("found overflow");
			diag_free(line);
			goto done;
		}
		if (v == 0 && errno == ERANGE) {
			diag_error("found underflow");
			diag_free(line);
			goto done;
		}
		diag_free(line);
		if (capacity <= n) {
			capacity *= 2;
			data = diag_realloc(data, capacity * sizeof(double));
		}
		intptr_t key = (intptr_t)floor(v * coeff);
		if (diag_rbtree_search(tree, key, &node) == DIAG_SUCCESS) {
			indices = (struct diag_vector *)node->attr;
			diag_vector_push_back(indices, (intptr_t)n);
		} else {
			indices = diag_vector_create(1);
			diag_vector_set(indices, 0, (intptr_t)n);
			node = diag_rbtree_node_new(key, (intptr_t)indices);
			(void)diag_rbtree_insert(tree, node);
		}
		data[n++] = v;
	}
	if (n == 0) {
		diag_error("no input");
		goto done;
	}
	diag_rbtree_for_each(tree, transpose, xtree);
	node = diag_rbtree_maximum(xtree);
	keys = (struct diag_vector *)node->attr;
	size_t len = diag_vector_length(keys);
	size_t i;
	for (i = 0; i < len; i++) {
		intptr_t key = (intptr_t)diag_vector_ref(keys, i);
		int r = diag_rbtree_search(tree, key, &node);
		assert(r == DIAG_SUCCESS);
		indices = (struct diag_vector *)node->attr;
		size_t k = (size_t)diag_vector_ref(indices, 0);
		printf("%g\n", data[k]);
	}
	r = EXIT_SUCCESS;

 done:
	diag_free(data);
	diag_rbtree_for_each_attr(xtree, free_vector, NULL);
	diag_rbtree_destroy(xtree);
	diag_rbtree_for_each_attr(tree, free_vector, NULL);
	diag_rbtree_destroy(tree);
	diag_line_context_destroy(context);
	diag_port_destroy(iport);
	return r;
}
