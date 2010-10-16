#ifndef DIAGONAL_RBTREE_H
#define DIAGONAL_RBTREE_H

typedef uintptr_t diag_rbtree_key_t;
typedef uintptr_t diag_rbtree_attr_t;

typedef int (*diag_rbtree_cmp_t)(diag_rbtree_key_t x, diag_rbtree_key_t y);

typedef void (*diag_rbtree_callback_t)(diag_rbtree_key_t key, diag_rbtree_attr_t attr);
typedef void (*diag_rbtree_callback_attr_t)(diag_rbtree_attr_t attr);

#define DIAG_RBTREE_IMMEDIATE ((diag_rbtree_cmp_t)NULL)

struct diag_rbtree_node {
	diag_rbtree_key_t key;
	diag_rbtree_attr_t attr;
	char color;
	struct diag_rbtree_node *parent;
	struct diag_rbtree_node *left;
	struct diag_rbtree_node *right;
};

struct diag_rbtree {
	struct diag_rbtree_node *root;
	diag_rbtree_cmp_t cmp;
	diag_size_t num_nodes;
};

DIAG_C_DECL_BEGIN

extern struct diag_rbtree *diag_rbtree_new(diag_rbtree_cmp_t cmp);

extern void diag_rbtree_destroy(struct diag_rbtree *tree);

extern struct diag_rbtree_node *diag_rbtree_node_new(diag_rbtree_key_t key, diag_rbtree_attr_t attr);

extern void diag_rbtree_node_destroy(struct diag_rbtree_node *node);

extern diag_size_t diag_rbtree_insert(struct diag_rbtree *tree, struct diag_rbtree_node *node);

extern diag_size_t diag_rbtree_delete(struct diag_rbtree *tree, struct diag_rbtree_node *node);

extern int diag_rbtree_search(struct diag_rbtree *tree, diag_rbtree_key_t key, struct diag_rbtree_node **found);

extern struct diag_rbtree_node *diag_rbtree_minimum(const struct diag_rbtree *tree);
extern struct diag_rbtree_node *diag_rbtree_maximum(const struct diag_rbtree *tree);

extern struct diag_rbtree_node *diag_rbtree_predecessor(const struct diag_rbtree_node *node);
extern struct diag_rbtree_node *diag_rbtree_successor(const struct diag_rbtree_node *node);

extern void diag_rbtree_for_each(const struct diag_rbtree *tree, diag_rbtree_callback_t callback);
extern void diag_rbtree_for_each_attr(const struct diag_rbtree *tree, diag_rbtree_callback_attr_t callback);

DIAG_C_DECL_END

#endif
