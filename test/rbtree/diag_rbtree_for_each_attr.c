#include "test.h"

#include "diagonal.h"
#include "diagonal/rbtree.h"

static void
count(uintptr_t attr)
{
	static uintptr_t i = 0;

	assert(attr == i);
	i++;
}

int
main()
{
	diag_rbtree_t *tree;
	diag_rbtree_node_t *node;
	uintptr_t i;

	tree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	for (i = 0; i < 100; i++) {
		node = diag_rbtree_node_new((diag_rbtree_key_t)i, i);
		diag_rbtree_insert(tree, node);
	}
	diag_rbtree_for_each_attr(tree, count);
	diag_rbtree_destroy(tree);
	return 0;
}
