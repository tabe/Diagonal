/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/deque.h"
#include "diagonal/trie.h"

#define CODE_NONE 0
#define CODE_EOF  1
#define CODE_MAX  257

#define CODE(b) (((ssize_t)(b))+2)

struct trie_children {
	ssize_t base;
	struct diag_deque *deque;
};

struct trie_child {
	ssize_t code;
	ssize_t dst;
	struct trie_children *children;
};

#define SET_BASE(trie, s, b)  (trie)->bc[s].base = (b)
#define GET_BASE(trie, s)     (trie)->bc[s].base
#define SET_CHECK(trie, s, c) (trie)->bc[s].check = (c)
#define GET_CHECK(trie, s)    (trie)->bc[s].check

static struct diag_trie *
extend_if_necessary(struct diag_trie *trie, ssize_t s)
{
	ssize_t t;

	assert(trie);
	if ( (t = trie->size) <= s) {
		trie = (struct diag_trie *)diag_realloc(trie, sizeof(struct diag_trie) + sizeof(struct diag_trie_bc) * (s + 1));
		trie->size = s + 1;
		do {
			SET_BASE(trie, t, CODE_NONE);
			SET_CHECK(trie, t, CODE_NONE);
		} while (t++ < s);
	}
	return trie;
}

static struct diag_trie *
fetch_base(struct diag_trie *trie, ssize_t s, ssize_t *basep)
{
	assert(trie);
	trie = extend_if_necessary(trie, s);
	*basep = GET_BASE(trie, s);
	return trie;
}

static struct diag_trie *
set_check(struct diag_trie *trie, ssize_t s, ssize_t check)
{
	assert(trie);
	trie = extend_if_necessary(trie, s);
	SET_CHECK(trie, s, check);
	return trie;
}

static struct diag_trie *
fetch_check(struct diag_trie *trie, ssize_t s, ssize_t *checkp)
{
	assert(trie);
	trie = extend_if_necessary(trie, s);
	*checkp = GET_CHECK(trie, s);
	return trie;
}

static struct trie_children *
transitions(struct diag_trie *trie, ssize_t s)
{
	ssize_t t, c;
	struct trie_children *children;
	struct trie_child *child;

	assert(trie && s < trie->size);
	children = diag_malloc(sizeof(*children));
	children->deque = diag_deque_new();
	children->base = GET_BASE(trie, s);
	for (c = CODE_EOF; c <= CODE_MAX && ( (t = children->base + c) < trie->size); c++) {
		if (GET_CHECK(trie, t) == s) {
			child = diag_malloc(sizeof(*child));
			child->dst  = t;
			child->code = c;
			child->children = NULL;
			diag_deque_push(children->deque, (uintptr_t)child);
		}
	}
	return children;
}

static struct diag_trie *
relocate(struct diag_trie *trie, ssize_t s, ssize_t b)
{
	struct trie_children *children;
	struct trie_child *child, *gchild;
	struct diag_deque_elem *e_child, *e_gchild;

	assert(trie && s < trie->size);
	children = transitions(trie, s);
	DIAG_DEQUE_FOR_EACH(children->deque, e_child) {
		child = (struct trie_child *)e_child->attr;
		child->children = transitions(trie, child->dst);
	}
	DIAG_DEQUE_FOR_EACH(children->deque, e_child) {
		child = (struct trie_child *)e_child->attr;
		trie = set_check(trie, b + child->code, s);
		SET_BASE(trie, b + child->code, child->children->base);
		DIAG_DEQUE_FOR_EACH(child->children->deque, e_gchild) {
			gchild = (struct trie_child *)e_gchild->attr;
			SET_CHECK(trie, gchild->dst, b + child->code);
			diag_free(gchild);
		}
		diag_deque_destroy(child->children->deque);
		diag_free(child->children);
		SET_CHECK(trie, child->dst, CODE_NONE);
		diag_free(child);
	}
	diag_deque_destroy(children->deque);
	diag_free(children);
	SET_BASE(trie, s, b);
	return trie;
}

static struct diag_trie *
add_transition(struct diag_trie *trie, ssize_t s, ssize_t c, ssize_t *tp)
{
	ssize_t base, check, t;

	assert(trie);
	trie = fetch_base(trie, s, &base);
	t = base + c;
	trie = fetch_check(trie, t, &check);
	if (check != CODE_NONE) {
		for (base = 0; (t = base + c) < trie->size; base++) {
			if ( GET_BASE(trie, base) == CODE_NONE
				 && GET_BASE(trie, t) == CODE_NONE
				 && GET_CHECK(trie, t) == CODE_NONE ) {
				break;
			}
		}
		trie = relocate(trie, s, base);
	}
	SET_CHECK(trie, t, s);
	if (tp) *tp = t;
	return trie;
}

struct diag_trie *
diag_trie_new(void)
{
	struct diag_trie *trie;

	trie = diag_malloc(sizeof(struct diag_trie) + sizeof(struct diag_trie_bc));
	trie->size = 1;
	SET_BASE(trie, 0, CODE_NONE);
	SET_CHECK(trie, 0, CODE_NONE);
	return trie;
}

void
diag_trie_destroy(struct diag_trie *trie)
{
	diag_free(trie);
}

int
diag_trie_traverse(struct diag_trie *trie, ssize_t length, const uint8_t *seq, struct diag_trie **insert)
{
	ssize_t s, t, i;

	assert(trie && seq);
	s = 0;
	for (i = 0; i < length; i++) {
		assert(s < trie->size);
		t = GET_BASE(trie, s) + CODE(seq[i]);
		if (t >= trie->size) {
			goto bail;
		} else if (GET_CHECK(trie, t) == s) {
			s = t;
		} else {
			goto bail;
		}
	}
	assert(s < trie->size);
	t = GET_BASE(trie, s) + CODE_EOF;
	if (t >= trie->size) {
		/* nothing to do */
	} else if (GET_CHECK(trie, t) == s) {
		return 1;
	}
 bail:
	if (insert) {
		while (i < length) {
			trie = add_transition(trie, s, CODE(seq[i]), &t);
			s = t;
			i++;
		}
		*insert = add_transition(trie, s, CODE_EOF, NULL);
	}
	return 0;
}
