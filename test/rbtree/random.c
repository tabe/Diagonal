/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include <time.h>
#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/rbtree.h"

int main(void)
{
	struct diag_rbtree *tree;
	struct diag_rbtree_node *node;
	size_t s;
	int n, i, r;

	srand((unsigned int)time(NULL));
	tree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);

	/* determine the initial number */
	do {
		n = rand();
	} while (n > 100000);
	printf("initial number of nodes: %d\n", n);

	for (i=0; i<n; i++) {
		node = diag_rbtree_node_new((intptr_t)i, (intptr_t)NULL);
		s = diag_rbtree_insert(tree, node);
		assert((int)s == i+1);
	}

	/* delete or insert some of nodes */
	for (i=0; i<n; i++) {
		if (rand()%2 == 0) {
			r = diag_rbtree_search(tree, (intptr_t)i, &node);
			assert(r == DIAG_SUCCESS);
			diag_rbtree_delete(tree, node);
		} else {
			node = diag_rbtree_node_new((intptr_t)rand(), (intptr_t)NULL);
			diag_rbtree_insert(tree, node);
		}
	}

	while (tree->root) diag_rbtree_delete(tree, tree->root);
	diag_rbtree_destroy(tree);
	return EXIT_SUCCESS;
}
