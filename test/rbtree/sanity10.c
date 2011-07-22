/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/rbtree.h"

int
main()
{
	struct diag_rbtree *tree;
	struct diag_rbtree_node *node;
	uintptr_t keys[] = {1, 6, 8, 11, 13, 15, 17, 22, 25, 27, 0};
	int i, r;

	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	for (i = 0; keys[i] > 0; i++) {
		node = diag_rbtree_node_new(keys[i], (uintptr_t)NULL);
		diag_rbtree_insert(tree, node);
	}
	for (i = 0; keys[i] > 0; i++) {
		r = diag_rbtree_search(tree, keys[i], &node);
		assert(r == DIAG_SUCCESS);
		assert(keys[i] == node->key);
	}
	r = diag_rbtree_search(tree, (uintptr_t)50, &node);
	assert(r == DIAG_FAILURE);

	node = diag_rbtree_minimum(tree);
	assert(1 == (int)node->key);

	for (i = 1; keys[i] > 0; i++) {
		node = diag_rbtree_successor(node);
		assert(keys[i] == node->key);
	}
	node = diag_rbtree_successor(node);
	assert(!node);

	node = diag_rbtree_maximum(tree);
	assert(27 == (int)node->key);

	i--;
	while (--i > 0) {
		node = diag_rbtree_predecessor(node);
		assert(keys[i] == node->key);
	}

	while (tree->root) diag_rbtree_delete(tree, tree->root);
	diag_rbtree_destroy(tree);
	return EXIT_SUCCESS;
}
