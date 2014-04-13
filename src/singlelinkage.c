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
#include "diagonal/cmp.h"
#include "diagonal/dataset.h"
#include "diagonal/datum.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/rbtree.h"
#include "diagonal/singlelinkage.h"

static void free_couple(intptr_t attr, void *data)
{
	(void)data;
	struct diag_couple *p = (struct diag_couple *)attr;
	diag_free(p);
}

/* API */

struct diag_singlelinkage *diag_singlelinkage_create(struct diag_dataset *ds,
						     diag_metric_t f,
						     diag_cmp_t cmp)
{
	struct diag_singlelinkage *sl;

	assert(ds);
	sl = diag_malloc(sizeof(*sl));
	sl->ds = ds;
	sl->f = f;
	sl->cmp = cmp;
	sl->initial = sl->final = 0;
	sl->m = NULL;
	sl->t = NULL;
	return sl;
}

int diag_singlelinkage_analyze(struct diag_singlelinkage *sl)
{
	struct diag_datum *di, *dj;
	struct diag_rbtree *tree;
	struct diag_rbtree_node *cur_node, *nxt_node, *tmp_node, *node;
	struct diag_couple *p;
	size_t i, j, k, n, initial = 0, final;
	intptr_t x, car, cdr;
	int r = 0;

	assert(sl);
	n = final = sl->ds->size;
	if (n == 0) {
		return -1;
	} else if (n == 1) {
		return -1;
	}
	sl->m = diag_rbtree_create(sl->cmp);
	/* calculate metric for each couple of data */
	for (i = 0; i < n - 1; i++) {
		di = diag_dataset_at(sl->ds, i);
		for (j = i + 1; j < n; j++) {
			dj = diag_dataset_at(sl->ds, j);
			x = sl->f((intptr_t)di, (intptr_t)dj);
			p = diag_malloc(sizeof(*p));
			p->i = i;
			p->car = (intptr_t)di->id;
			p->j = j;
			p->cdr = (intptr_t)dj->id;
			tmp_node = diag_rbtree_node_new(x, (intptr_t)p);
			diag_rbtree_insert(sl->m, tmp_node);
			diag_datum_destroy(dj);
		}
		diag_datum_destroy(di);
	}
	/* construct binary tree by starting with the minimum */
	sl->t = diag_deque_new();
	do {
		cur_node = diag_rbtree_minimum(sl->m);
		assert(cur_node);
		p = (struct diag_couple *)cur_node->attr;
		i = p->i;
		j = p->j;
		car = p->car;
		cdr = p->cdr;
		nxt_node = diag_rbtree_successor(cur_node);
		if (!nxt_node) goto push;
		tree = diag_rbtree_create(sl->cmp);
		do {
			tmp_node = nxt_node;
			p = (struct diag_couple *)tmp_node->attr;
			if (p->i == i || p->i == j) {
				k = p->j;
				if (diag_rbtree_search(tree, (intptr_t)k, &node)) {
					diag_rbtree_delete(tree, node);
					nxt_node = diag_rbtree_successor(tmp_node);
					diag_rbtree_delete(sl->m, tmp_node);
				} else {
					p->i = n;
					node = diag_rbtree_node_new((intptr_t)k, (intptr_t)NULL);
					diag_rbtree_insert(tree, node);
					nxt_node = diag_rbtree_successor(tmp_node);
				}
				continue;
			}
			if (p->j == i || p->j == j) {
				k = p->i;
				if (diag_rbtree_search(tree, (intptr_t)k, &node)) {
					diag_rbtree_delete(tree, node);
					nxt_node = diag_rbtree_successor(tmp_node);
					diag_rbtree_delete(sl->m, tmp_node);
				} else {
					p->j = n;
					node = diag_rbtree_node_new((intptr_t)k, (intptr_t)NULL);
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
		p->i = i;
		p->car = car;
		p->j = n;
		p->cdr = (intptr_t)NULL;
		diag_deque_unshift(sl->t, (intptr_t)p);
		p = diag_malloc(sizeof(*p));
		p->i = j;
		p->car = cdr;
		p->j = n;
		p->cdr = (intptr_t)NULL;
		diag_deque_unshift(sl->t, (intptr_t)p);
		n++;
		if (sl->initial > 0 && sl->initial == ++initial) break;
		if (sl->final > 0 && sl->final == --final) break;
		free_couple(cur_node->attr, NULL);
	} while ( diag_rbtree_delete(sl->m, cur_node) > 0 );
	diag_rbtree_for_each_attr(sl->m, free_couple, NULL);
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
