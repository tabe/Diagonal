#include "test.h"

#include "diagonal.h"
#include "diagonal/rbtree.h"

int
main()
{
	diag_rbtree_t *tree;
	diag_rbtree_node_t *node;
	int keys[] = {1, 6, 8, 11, 13, 15, 17, 22, 25, 27, 0};
	int i, r;

	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	for (i = 0; keys[i] > 0; i++) {
		node = diag_rbtree_node_new((diag_rbtree_key_t)keys[i], NULL);
		diag_rbtree_insert(tree, node);
	}
	for (i = 0; keys[i] > 0; i++) {
		r = diag_rbtree_search(tree, (diag_rbtree_key_t)keys[i], &node);
		assert(r == DIAG_SUCCESS);
		assert(keys[i] == (int)node->key);
	}
	r = diag_rbtree_search(tree, (diag_rbtree_key_t)50, &node);
	assert(r == DIAG_FAILURE);

	node = diag_rbtree_minimum(tree);
	assert(1 == (int)node->key);

	for (i = 1; keys[i] > 0; i++) {
		node = diag_rbtree_successor(node);
		assert(keys[i] == (int)node->key);
	}
	node = diag_rbtree_successor(node);
	assert(!node);

	node = diag_rbtree_maximum(tree);
	assert(27 == (int)node->key);

	i--;
	while (--i > 0) {
		node = diag_rbtree_predecessor(node);
		assert(keys[i] == (int)node->key);
	}

	while (tree->root) diag_rbtree_delete(tree, tree->root);
	diag_rbtree_destroy(tree);
	return 0;
}