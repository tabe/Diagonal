/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/rbtree.h"

#define REDP(n) ((n)->color == 'r')
#define BLACKP(n) ((n)->color == 'b')

#define TURN_RED(n) ((n)->color = 'r')
#define TURN_BLACK(n) do {if (n) ((n)->color = 'b');} while (0)
#define COPY_COLOR(src, dst) ((src)->color = (dst) ? (dst)->color : 'b')

#define LEFTP(n) (n == n->parent->left)
#define RIGHTP(n) (n == n->parent->right)
#define SIBLING(n) (LEFTP(n) ? n->parent->right : n->parent->left)

static int
compare(uintptr_t x, uintptr_t y)
{
	return (x > y) - (x < y);
}

static struct diag_rbtree_node *
leftmost(struct diag_rbtree_node *n)
{
	assert(n);
	while (n->left) n = n->left;
	return n;
}

static struct diag_rbtree_node *
rightmost(struct diag_rbtree_node *n)
{
	assert(n);
	while (n->right) n = n->right;
	return n;
}

static void
destroy_subtree(struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *left, *right;

	assert(n);
	left = n->left;
	right = n->right;
	diag_free(n);
	n = NULL;
	if (left) destroy_subtree(left);
	if (right) destroy_subtree(right);
}

static void
replace(struct diag_rbtree *t, struct diag_rbtree_node *n, struct diag_rbtree_node *by)
{
	assert(n);
	if (!n->parent) {
		t->root = by;
	} else if (LEFTP(n)) {
		n->parent->left = by;
	} else {
		n->parent->right = by;
	}
	if (by) by->parent = n->parent;
}

static void
rotate_left(struct diag_rbtree *t, struct diag_rbtree_node *b)
{
	struct diag_rbtree_node *c, *d;

	assert(t && b && b->right);
	d = b->right;
	c = d->left;
	replace(t, b, d);
	d->left = b;
	b->parent = d;
	b->right = c;
	if (c) c->parent = b;
}

static void
rotate_right(struct diag_rbtree *t, struct diag_rbtree_node *d)
{
	struct diag_rbtree_node *b, *c;

	assert(t && d && d->left);
	b = d->left;
	c = b->right;
	replace(t, d, b);
	b->right = d;
	d->parent = b;
	d->left = c;
	if (c) c->parent = d;
}

static void insert1(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void insert2(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void insert3(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void insert4(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void insert5(struct diag_rbtree *t, struct diag_rbtree_node *n);

static void
insert1(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	assert(t && n);
	if (n->parent) {
		insert2(t, n);
	} else {
		TURN_BLACK(n);
		t->root = n;
	}
}

static void
insert2(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	assert(t && n && n->parent);
	if (BLACKP(n->parent)) {
		return;
	} else {
		insert3(t, n);
	}
}

static void
insert3(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *p, *g, *u;

	assert(t && n);
	p = n->parent;
	assert(p);
	g = p->parent;
	assert(g);
	u = (p == g->left) ? g->right : g->left;
	if (u && REDP(u)) {
		TURN_BLACK(p);
		TURN_BLACK(u);
		TURN_RED(g);
		insert1(t, g);
	} else {
		insert4(t, n);
	}
}

static void
insert4(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *p, *g;

	assert(t && n);
	p = n->parent;
	assert(p);
	g = p->parent;
	assert(g);
	if (n == p->right && p == g->left) {
		rotate_left(t, p);
		n = n->left;
	} else if (n == p->left && p == g->right) {
		rotate_right(t, p);
		n = n->right;
	}
	insert5(t, n);
}

static void
insert5(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *p, *g;

	assert(t && n);
	p = n->parent;
	assert(p);
	g = p->parent;
	assert(g);
	TURN_BLACK(p);
	TURN_RED(g);
	if (n == p->left && p == g->left) {
		rotate_right(t, g);
	} else {
		assert(n == p->right && p == g->right);
		rotate_left(t, g);
	}
}

static struct diag_rbtree_node *
inorder_predecessor(const struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *p;

	assert(n);
	if (n->left) {
		p = n->left;
		while (p->right) p = p->right;
		return p;
	}
	while (n->parent) {
		if (RIGHTP(n)) return n->parent;
		n = n->parent;
	}
	return NULL;
}

static struct diag_rbtree_node *
inorder_successor(const struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *p;

	assert(n);
	if (n->right) {
		p = n->right;
		while (p->left) p = p->left;
		return p;
	}
	while (n->parent) {
		if (LEFTP(n)) return n->parent;
		n = n->parent;
	}
	return NULL;
}

static void delete1(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void delete2(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void delete3(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void delete4(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void delete5(struct diag_rbtree *t, struct diag_rbtree_node *n);
static void delete6(struct diag_rbtree *t, struct diag_rbtree_node *n);

static void
delete1(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	assert(t && n && BLACKP(n));
	if (n->parent) delete2(t, n);
}

static void
delete2(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *s;

	assert(t && n && BLACKP(n) && n->parent);
	s = SIBLING(n);
	if (s && REDP(s)) {
		TURN_RED(n->parent);
		TURN_BLACK(s);
		if (LEFTP(n)) {
			rotate_left(t, n->parent);
		} else {
			rotate_right(t, n->parent);
		}
	}
	delete3(t, n);
}

static void
delete3(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *s;

	assert(t && n && BLACKP(n) && n->parent);
	s = SIBLING(n);
	if ( BLACKP(n->parent) &&
		 s && BLACKP(s) &&
		 (!s->left  || BLACKP(s->left)) &&
		 (!s->right || BLACKP(s->right)) ) {
		TURN_RED(s);
		delete1(t, n->parent);
	} else {
		delete4(t, n);
	}
}

static void
delete4(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *s;

	assert(t && n && BLACKP(n) && n->parent);
	s = SIBLING(n);
	if ( REDP(n->parent) &&
		 s && BLACKP(s) &&
		 (!s->left  || BLACKP(s->left)) &&
		 (!s->right || BLACKP(s->right)) ) {
		TURN_RED(s);
		TURN_BLACK(n->parent);
	} else {
		delete5(t, n);
	}
}

static void
delete5(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *s;

	assert(t && n && BLACKP(n) && n->parent);
	s = SIBLING(n);
	if (s && BLACKP(s)) {
		if ( LEFTP(n) &&
			 (!s->right || BLACKP(s->right)) &&
			 (s->left && REDP(s->left)) ) {
			TURN_RED(s);
			TURN_BLACK(s->left);
			rotate_right(t, s);
		} else if ( RIGHTP(n) &&
					(!s->left || BLACKP(s->left)) &&
					(s->right && REDP(s->right)) ) {
			TURN_RED(s);
			TURN_BLACK(s->right);
			rotate_left(t, s);
		}
	}
	delete6(t, n);
}

static void
delete6(struct diag_rbtree *t, struct diag_rbtree_node *n)
{
	struct diag_rbtree_node *s;

	assert(t && n && BLACKP(n) && n->parent);
	s = SIBLING(n);
	COPY_COLOR(n->parent, s);
	TURN_BLACK(n->parent);
 	if (LEFTP(n)) {
		if (s) {
			TURN_BLACK(s->right);
			rotate_left(t, n->parent);
		}
	} else {
		if (s) {
			TURN_BLACK(s->left);
			rotate_right(t, n->parent);
		}
	}
}

/* Public API */

struct diag_rbtree *
diag_rbtree_create(diag_cmp_t cmp)
{
	struct diag_rbtree *tree;
	tree = diag_malloc(sizeof(struct diag_rbtree));
	tree->root = NULL;
	tree->num_nodes = 0;
	tree->cmp = (cmp) ? cmp : compare;
	return tree;
}

void
diag_rbtree_destroy(struct diag_rbtree *tree)
{
	if (tree) {
		if (tree->root) destroy_subtree(tree->root);
		diag_free(tree);
		tree = NULL;
	}
}

struct diag_rbtree_node *
diag_rbtree_node_new(uintptr_t key, uintptr_t attr)
{
	struct diag_rbtree_node *node;
	node = diag_malloc(sizeof(struct diag_rbtree_node));
	node->key = key;
	node->color = '\0';
	node->parent = node->left = node->right = NULL;
	node->attr = attr;
	return node;
}

void
diag_rbtree_node_destroy(struct diag_rbtree_node *node)
{
	diag_free(node);
	node = NULL;
}

size_t
diag_rbtree_insert(struct diag_rbtree *tree, struct diag_rbtree_node *node)
{
	struct diag_rbtree_node *n;

	assert(tree && node);
	n = tree->root;
	if (!n) {
		TURN_BLACK(node);
		node->parent = NULL;
		tree->root = node;
		tree->num_nodes = 1;
		return tree->num_nodes;
	}

	for (;;) {
		if (tree->cmp(node->key, n->key) < 0) {
			if (n->left) {
				n = n->left;
				continue;
			} else {
				n->left = node;
				break;
			}
		} else {
			if (n->right) {
				n = n->right;
				continue;
			} else {
				n->right = node;
				break;
			}
		}
	}

	TURN_RED(node);
	node->parent = n;
	insert1(tree, node);
	tree->num_nodes++;
	return tree->num_nodes;
}

size_t
diag_rbtree_delete(struct diag_rbtree *tree, struct diag_rbtree_node *node)
{
	struct diag_rbtree_node *n, *c;

	assert(tree && node);
	while (node->left && node->right) {
		n = inorder_predecessor(node);
		assert(n);
		node->key = n->key;
		node->attr = n->attr;
		node = n;
	}
	c = node->left ? node->left : node->right; /* c may be NULL */
	replace(tree, node, c);
	if (BLACKP(node)) {
		if (c) {
			if (REDP(c)) {
				TURN_BLACK(c);
			} else {
				assert(BLACKP(c));
				delete1(tree, c);
			}
		}
	}
	diag_rbtree_node_destroy(node);
	tree->num_nodes--;
	return tree->num_nodes;
}

int
diag_rbtree_search(struct diag_rbtree *tree, uintptr_t key, struct diag_rbtree_node **found)
{
	struct diag_rbtree_node *n;

	assert(tree);
	n = tree->root;
	while (n) {
		int c = tree->cmp(key, n->key);
		if (c == 0) {
			if (found) *found = n;
			return DIAG_SUCCESS;
		}
		n = (c < 0) ? n->left : n->right;
	}
	return DIAG_FAILURE;
}

struct diag_rbtree_node *
diag_rbtree_minimum(const struct diag_rbtree *tree)
{
	assert(tree);
	return (tree->root) ? leftmost(tree->root) : NULL;
}

struct diag_rbtree_node *
diag_rbtree_maximum(const struct diag_rbtree *tree)
{
	assert(tree);
	return (tree->root) ? rightmost(tree->root) : NULL;
}

struct diag_rbtree_node *
diag_rbtree_predecessor(const struct diag_rbtree_node *node)
{
	return inorder_predecessor(node);
}

struct diag_rbtree_node *
diag_rbtree_successor(const struct diag_rbtree_node *node)
{
	return inorder_successor(node);
}

void
diag_rbtree_for_each(const struct diag_rbtree *tree, diag_rbtree_callback_t callback)
{
	struct diag_rbtree_node *node;

	node = diag_rbtree_minimum(tree);
	if (!node) return;
	do {
		callback(node->key, node->attr);
	} while ( (node = diag_rbtree_successor(node)) );
}

void
diag_rbtree_for_each_attr(const struct diag_rbtree *tree, diag_rbtree_callback_attr_t callback)
{
	struct diag_rbtree_node *node;

	node = diag_rbtree_minimum(tree);
	if (!node) return;
	do {
		callback(node->attr);
	} while ( (node = diag_rbtree_successor(node)) );
}
