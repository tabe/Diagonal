/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_RBTREE_H
#define DIAGONAL_RBTREE_H

typedef void (*diag_rbtree_callback_t)(intptr_t key, intptr_t attr, void *data);
typedef void (*diag_rbtree_callback_attr_t)(intptr_t attr, void *data);

struct diag_rbtree_node {
	intptr_t key;
	intptr_t attr;
	char color;
	struct diag_rbtree_node *parent;
	struct diag_rbtree_node *left;
	struct diag_rbtree_node *right;
};

struct diag_rbtree {
	struct diag_rbtree_node *root;
	diag_cmp_t cmp;
	size_t num_nodes;
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_rbtree *diag_rbtree_create(diag_cmp_t cmp);

DIAG_FUNCTION void diag_rbtree_destroy(struct diag_rbtree *tree);

DIAG_FUNCTION struct diag_rbtree_node *diag_rbtree_node_new(intptr_t key, intptr_t attr);

DIAG_FUNCTION void diag_rbtree_node_destroy(struct diag_rbtree_node *node);

/*
 * Insert `node' into `tree' and return the resulting number of nodes in `tree'.
 */
DIAG_FUNCTION size_t diag_rbtree_insert(struct diag_rbtree *tree,
					struct diag_rbtree_node *node);

/*
 * Delete `node' from `tree' and return the resulting number of nodes in `tree'.
 */
DIAG_FUNCTION size_t diag_rbtree_delete(struct diag_rbtree *tree,
					struct diag_rbtree_node *node);

/*
 * If `key' is found in `tree', return DIAG_SUCCESS with the resulting node
 * assigned to `found', otherwise return DIAG_FAILURE.
 */
DIAG_FUNCTION int diag_rbtree_search(struct diag_rbtree *tree, intptr_t key,
				     struct diag_rbtree_node **found);

DIAG_FUNCTION struct diag_rbtree_node *diag_rbtree_minimum(const struct diag_rbtree *tree);
DIAG_FUNCTION struct diag_rbtree_node *diag_rbtree_maximum(const struct diag_rbtree *tree);

DIAG_FUNCTION struct diag_rbtree_node *diag_rbtree_predecessor(const struct diag_rbtree_node *node);
DIAG_FUNCTION struct diag_rbtree_node *diag_rbtree_successor(const struct diag_rbtree_node *node);

DIAG_FUNCTION void diag_rbtree_for_each(const struct diag_rbtree *tree,
					diag_rbtree_callback_t callback,
					void *data);
DIAG_FUNCTION void diag_rbtree_for_each_attr(const struct diag_rbtree *tree,
					     diag_rbtree_callback_attr_t callback,
					     void *data);

DIAG_C_DECL_END

#endif
