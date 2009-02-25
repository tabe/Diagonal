#include "config.h"

#include <assert.h>
#include <stdint.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/rbtree.h"

#define REDP(n) ((n)->color == 'r')
#define BLACKP(n) ((n)->color == 'b')

#define TURN_RED(n) ((n)->color = 'r')
#define TURN_BLACK(n) ((n)->color = 'b')
#define COPY_COLOR(src, dst) ((src)->color = (dst)->color)

#define LEFTP(n) (n == n->parent->left)
#define RIGHTP(n) (n == n->parent->right)
#define SIBLING(n) (LEFTP(n) ? n->parent->right : n->parent->left)

static int
compare(diag_rbtree_key_t x, diag_rbtree_key_t y)
{
	return (x > y) - (x < y);
}

static diag_rbtree_node_t *
leftmost(diag_rbtree_node_t *n)
{
	assert(n);
	while (n->left) n = n->left;
	return n;
}

static diag_rbtree_node_t *
rightmost(diag_rbtree_node_t *n)
{
	assert(n);
	while (n->right) n = n->right;
	return n;
}

static void
destroy_subtree(diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *left, *right;

	assert(n);
	left = n->left;
	right = n->right;
	diag_free(n);
	n = NULL;
	if (left) destroy_subtree(left);
	if (right) destroy_subtree(right);
}

static void
replace(diag_rbtree_t *t, diag_rbtree_node_t *n, diag_rbtree_node_t *by)
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
rotate_left(diag_rbtree_t *t, diag_rbtree_node_t *b)
{
	diag_rbtree_node_t *c, *d;

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
rotate_right(diag_rbtree_t *t, diag_rbtree_node_t *d)
{
	diag_rbtree_node_t *b, *c;

	assert(t && d && d->left);
	b = d->left;
	c = b->right;
	replace(t, d, b);
	b->right = d;
	d->parent = b;
	d->left = c;
	if (c) c->parent = d;
}

static void insert1(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void insert2(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void insert3(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void insert4(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void insert5(diag_rbtree_t *t, diag_rbtree_node_t *n);

static void
insert1(diag_rbtree_t *t, diag_rbtree_node_t *n)
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
insert2(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	assert(t && n && n->parent);
	if (BLACKP(n->parent)) {
		return;
	} else {
		insert3(t, n);
	}
}

static void
insert3(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *p, *g, *u;

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
insert4(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *p, *g;

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
insert5(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *p, *g;

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

static diag_rbtree_node_t *
inorder_predecessor(const diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *p;

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

static diag_rbtree_node_t *
inorder_successor(const diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *p;

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

static void delete1(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void delete2(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void delete3(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void delete4(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void delete5(diag_rbtree_t *t, diag_rbtree_node_t *n);
static void delete6(diag_rbtree_t *t, diag_rbtree_node_t *n);

static void
delete1(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	assert(t && n);
	if (n->parent) delete2(t, n);
}

static void
delete2(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *s;

	assert(t && n && n->parent);
	s = SIBLING(n);
	if (REDP(s)) {
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
delete3(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *s;

	assert(t && n && n->parent);
	s = SIBLING(n);
	if ( BLACKP(n->parent) &&
		 BLACKP(s) &&
		 BLACKP(s->left) &&
		 BLACKP(s->right) ) {
		TURN_RED(s);
		delete1(t, n->parent);
	} else {
		delete4(t, n);
	}
}

static void
delete4(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *s;

	assert(t && n && n->parent);
	s = SIBLING(n);
	if ( REDP(n->parent) &&
		 BLACKP(s) &&
		 BLACKP(s->left) &&
		 BLACKP(s->right) ) {
		TURN_RED(s);
		TURN_BLACK(n->parent);
	} else {
		delete5(t, n);
	}
}

static void
delete5(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *s;

	assert(t && n && n->parent);
	s = SIBLING(n);
	if  (BLACKP(s)) {
		if ( LEFTP(n) &&
			 BLACKP(s->right) &&
			 REDP(s->left) ) {
			TURN_RED(s);
			TURN_BLACK(s->left);
			rotate_right(t, s);
		} else if ( RIGHTP(n) &&
					BLACKP(s->left) &&
					REDP(s->right) ) {
			TURN_RED(s);
			TURN_BLACK(s->right);
			rotate_left(t, s);
		}
	}
	delete6(t, n);
}

static void
delete6(diag_rbtree_t *t, diag_rbtree_node_t *n)
{
	diag_rbtree_node_t *s;

	assert(t && n && n->parent);
	s = SIBLING(n);
	COPY_COLOR(n->parent, s);
	TURN_BLACK(n->parent);
 	if (LEFTP(n)) {
		TURN_BLACK(s->right);
		rotate_left(t, n->parent);
	} else {
		TURN_BLACK(s->left);
		rotate_right(t, n->parent);
	}
}

/* Public API */

diag_rbtree_t *
diag_rbtree_new(diag_rbtree_cmp_t cmp)
{
	diag_rbtree_t *tree;
	tree = (diag_rbtree_t *)diag_malloc(sizeof(diag_rbtree_t));
	tree->root = NULL;
	tree->num_nodes = 0;
	tree->cmp = (cmp) ? cmp : compare;
	return tree;
}

void
diag_rbtree_destroy(diag_rbtree_t *tree)
{
	if (tree) {
		if (tree->root) destroy_subtree(tree->root);
		diag_free(tree);
		tree = NULL;
	}
}

diag_rbtree_node_t *
diag_rbtree_node_new(diag_rbtree_key_t key, void *attr)
{
	diag_rbtree_node_t *node;
	node = (diag_rbtree_node_t *)diag_malloc(sizeof(diag_rbtree_node_t));
	node->key = key;
	node->color = '\0';
	node->parent = node->left = node->right = NULL;
	node->attr = attr;
	return node;
}

void
diag_rbtree_node_destroy(diag_rbtree_node_t *node)
{
	if (node) {
		diag_free(node);
		node = NULL;
	}
}

void
diag_rbtree_insert(diag_rbtree_t *tree, diag_rbtree_node_t *node)
{
	diag_rbtree_node_t *n;

	assert(tree && node);
	n = tree->root;
	if (!n) {
		TURN_BLACK(node);
		node->parent = NULL;
		tree->root = node;
		tree->num_nodes = 1;
		return;
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
}

void
diag_rbtree_delete(diag_rbtree_t *tree, diag_rbtree_node_t *node)
{
	diag_rbtree_node_t *n, *c;

	assert(tree && node);
	if (node->left && node->right) {
		n = inorder_predecessor(node);
		node->key = n->key;
		node->attr = n->attr;
	} else {
		n = node;
	}
	c = n->left ? n->left : n->right;
	replace(tree, n, c);
	if (!n || BLACKP(n)) {
		if (c) {
			if (REDP(c)) {
				TURN_BLACK(c);
			} else {
				delete1(tree, c);
			}
		}
	}
	diag_rbtree_node_destroy(n);
	tree->num_nodes--;
}

int
diag_rbtree_search(diag_rbtree_t *tree, diag_rbtree_key_t key, diag_rbtree_node_t **found)
{
	diag_rbtree_node_t *n;

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

diag_rbtree_node_t *
diag_rbtree_minimum(const diag_rbtree_t *tree)
{
	return (tree->root) ? leftmost(tree->root) : NULL;
}

diag_rbtree_node_t *
diag_rbtree_maximum(const diag_rbtree_t *tree)
{
	return (tree->root) ? rightmost(tree->root) : NULL;
}

diag_rbtree_node_t *
diag_rbtree_predecessor(const diag_rbtree_node_t *node)
{
	return inorder_predecessor(node);
}

diag_rbtree_node_t *
diag_rbtree_successor(const diag_rbtree_node_t *node)
{
	return inorder_successor(node);
}

void
diag_rbtree_for_each(const diag_rbtree_t *tree, diag_rbtree_callback_t callback)
{
	diag_rbtree_node_t *node;

	node = diag_rbtree_minimum(tree);
	if (!node) return;
	do {
		callback(node->key, node->attr);
	} while ( (node = diag_rbtree_successor(node)) );
}
