/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/dataset.h"
#include "diagonal/datum.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/rbtree.h"
#include "diagonal/singlelinkage.h"

/* API */

struct diag_singlelinkage *diag_singlelinkage_create(struct diag_dataset *ds,
						     diag_metric_t f)
{
	struct diag_singlelinkage *sl;

	assert(ds);
	sl = diag_malloc(sizeof(*sl));
	sl->ds = ds;
	sl->f = f;
	sl->m = NULL;
	sl->t = NULL;
	return sl;
}

int diag_singlelinkage_analyze(struct diag_singlelinkage *sl)
{
	struct diag_datum *di, *dj;
	struct diag_rbtree *tree;
	struct diag_rbtree_node *cur_node, *nxt_node, *tmp_node, *node;
	struct diag_pair *p;
	size_t i, j, k, n;
	uintptr_t x;
	int r = 0;

	assert(sl);
	n = sl->ds->size;
	if (n == 0) {
		return -1;
	} else if (n == 1) {
		return -1;
	}
	sl->m = diag_rbtree_new(NULL);
	/* calculate metric for each pair of data */
	for (i = 0; i < n - 1; i++) {
		di = diag_dataset_at(sl->ds, i);
		for (j = i + 1; j < n; j++) {
			dj = diag_dataset_at(sl->ds, j);
			x = sl->f((intptr_t)di, (intptr_t)dj);
			p = diag_malloc(sizeof(*p));
			p->car = (uintptr_t)i;
			p->cdr = (uintptr_t)j;
			tmp_node = diag_rbtree_node_new(x, (uintptr_t)p);
			diag_rbtree_insert(sl->m, tmp_node);
		}
	}
	/* construct binary tree by starting with the minimum */
	sl->t = diag_deque_new();
	do {
		cur_node = diag_rbtree_minimum(sl->m);
		assert(cur_node);
		p = (struct diag_pair *)cur_node->attr;
		i = p->car;
		j = p->cdr;
		nxt_node = diag_rbtree_successor(cur_node);
		if (!nxt_node) goto push;
		tree = diag_rbtree_new(NULL);
		do {
			struct diag_rbtree_node *tmp_node = nxt_node;
			p = (struct diag_pair *)tmp_node->attr;
			if (p->car == i || p->car == j) {
				k = p->cdr;
				if (diag_rbtree_search(tree, (uintptr_t)k, &node)) {
					diag_rbtree_delete(tree, node);
					nxt_node = diag_rbtree_successor(tmp_node);
					diag_rbtree_delete(sl->m, tmp_node);
				} else {
					p->car = n;
					node = diag_rbtree_node_new((uintptr_t)k, (uintptr_t)NULL);
					diag_rbtree_insert(tree, node);
					nxt_node = diag_rbtree_successor(tmp_node);
				}
				continue;
			}
			if (p->cdr == i || p->cdr == j) {
				k = p->car;
				if (diag_rbtree_search(tree, (uintptr_t)k, &node)) {
					diag_rbtree_delete(tree, node);
					nxt_node = diag_rbtree_successor(tmp_node);
					diag_rbtree_delete(sl->m, tmp_node);
				} else {
					p->cdr = n;
					node = diag_rbtree_node_new((uintptr_t)k, (uintptr_t)NULL);
					diag_rbtree_insert(tree, node);
					nxt_node = diag_rbtree_successor(tmp_node);
				}
				continue;
			}
			nxt_node = diag_rbtree_successor(tmp_node);
		} while (nxt_node);
		assert(tree->num_nodes == 0);
		diag_rbtree_destroy(tree);
	push:
		p = diag_malloc(sizeof(*p));
		p->car = n;
		p->cdr = i;
		diag_deque_unshift(sl->t, (intptr_t)p);
		p = diag_malloc(sizeof(*p));
		p->car = n;
		p->cdr = j;
		diag_deque_unshift(sl->t, (intptr_t)p);
		n++;
	} while ( diag_rbtree_delete(sl->m, cur_node) > 0 );
	diag_rbtree_destroy(sl->m);
	sl->m = NULL;
	return r;
}

void diag_singlelinkage_destroy(struct diag_singlelinkage *sl)
{
	if (!sl) return;
	diag_deque_destroy(sl->t);
	diag_rbtree_destroy(sl->m);
	diag_free(sl);
}
