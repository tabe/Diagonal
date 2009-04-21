#ifndef DIAGONAL_RBTREE_H
#define DIAGONAL_RBTREE_H

typedef void *diag_rbtree_key_t;

typedef int (*diag_rbtree_cmp_t)(diag_rbtree_key_t x, diag_rbtree_key_t y);

typedef void (*diag_rbtree_callback_t)(diag_rbtree_key_t key, void *attr);

#define DIAG_RBTREE_IMMEDIATE ((diag_rbtree_cmp_t)NULL)

typedef struct diag_rbtree_node_s {
	diag_rbtree_key_t key;
	void *attr;
	char color;
	struct diag_rbtree_node_s *parent;
	struct diag_rbtree_node_s *left;
	struct diag_rbtree_node_s *right;
} diag_rbtree_node_t;

typedef struct diag_rbtree_s {
	diag_rbtree_node_t *root;
	diag_rbtree_cmp_t cmp;
	diag_size_t num_nodes;
} diag_rbtree_t;

DIAG_C_DECL_BEGIN

extern diag_rbtree_t *diag_rbtree_new(diag_rbtree_cmp_t cmp);

extern void diag_rbtree_destroy(diag_rbtree_t *tree);

extern diag_rbtree_node_t *diag_rbtree_node_new(diag_rbtree_key_t key, void *attr);

extern void diag_rbtree_node_destroy(diag_rbtree_node_t *node);

extern void diag_rbtree_insert(diag_rbtree_t *tree, diag_rbtree_node_t *node);

extern void diag_rbtree_delete(diag_rbtree_t *tree, diag_rbtree_node_t *node);

extern int diag_rbtree_search(diag_rbtree_t *tree, diag_rbtree_key_t key, diag_rbtree_node_t **found);

extern diag_rbtree_node_t *diag_rbtree_minimum(const diag_rbtree_t *tree);
extern diag_rbtree_node_t *diag_rbtree_maximum(const diag_rbtree_t *tree);

extern diag_rbtree_node_t *diag_rbtree_predecessor(const diag_rbtree_node_t *node);
extern diag_rbtree_node_t *diag_rbtree_successor(const diag_rbtree_node_t *node);

extern void diag_rbtree_for_each(const diag_rbtree_t *tree, diag_rbtree_callback_t callback);

DIAG_C_DECL_END

#endif
