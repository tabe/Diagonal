/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/rbtree.h"

static void
count(intptr_t attr, void *data)
{
	(void)data;

	static intptr_t i = 0;

	assert(attr == i);
	i++;
}

int main(void)
{
	struct diag_rbtree *tree;
	struct diag_rbtree_node *node;
	intptr_t i;

	tree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	for (i = 0; i < 100; i++) {
		node = diag_rbtree_node_new(i, i);
		diag_rbtree_insert(tree, node);
	}
	diag_rbtree_for_each_attr(tree, count, NULL);
	diag_rbtree_destroy(tree);
	return EXIT_SUCCESS;
}
