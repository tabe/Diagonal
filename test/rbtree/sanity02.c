#include "test.h"

#include "diagonal.h"
#include "diagonal/rbtree.h"

int
main()
{
	diag_rbtree_t *tree;
	diag_rbtree_node_t *node;
	int r;

	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	assert(!tree->root);
	assert(tree->num_nodes == 0);

	node = diag_rbtree_node_new((diag_rbtree_key_t)1, (diag_rbtree_attr_t)NULL);
	diag_rbtree_insert(tree, node);
	assert(tree->num_nodes == 1);
	assert(tree->root == node);
	assert(node->color == 'b');

	node = diag_rbtree_node_new((diag_rbtree_key_t)2, (diag_rbtree_attr_t)NULL);
	diag_rbtree_insert(tree, node);
	assert(tree->num_nodes == 2);
	assert(tree->root->right == node);
	assert(node->color == 'r');

	r = diag_rbtree_search(tree, (diag_rbtree_key_t)1, &node);
	assert(r == DIAG_SUCCESS);
	assert(node == tree->root);
	r = diag_rbtree_search(tree, (diag_rbtree_key_t)2, &node);
	assert(r == DIAG_SUCCESS);
	assert(node == tree->root->right);
	r = diag_rbtree_search(tree, (diag_rbtree_key_t)1024, (diag_rbtree_attr_t)NULL);
	assert(r == DIAG_FAILURE);

	diag_rbtree_delete(tree, tree->root);
	assert(tree->num_nodes == 1);
	assert(tree->root);
	assert(tree->root->color == 'b');
	assert(!tree->root->left);
	assert(!tree->root->right);
	diag_rbtree_delete(tree, tree->root);
	assert(tree->num_nodes == 0);
	assert(!tree->root);

	diag_rbtree_destroy(tree);
	return 0;
}
