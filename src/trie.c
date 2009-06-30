#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/trie.h"

#define CODE_NONE 0

#define CODE(b) ((int)(b)+1)

static void
extend_if_necessary(diag_trie_t *trie, diag_size_t s)
{
	diag_size_t t;

	assert(trie);
	if ( (t = trie->size) <= s) {
		trie = (diag_trie_t *)diag_realloc(trie, sizeof(diag_trie_t) + sizeof(diag_trie_bc_t) * ((s) + 1));
		trie->size = s + 1;
		do {
			trie->bc[t].base = trie->bc[t].check = CODE_NONE;
		} while (t++ < s);
	}
}

static void
set_base(diag_trie_t *trie, diag_size_t s, diag_ssize_t base)
{
	assert(trie);
	extend_if_necessary(trie, s);
	trie->bc[s].base = base;
}

static diag_ssize_t
get_base(diag_trie_t *trie, diag_size_t s)
{
	assert(trie);
	extend_if_necessary(trie, s);
	return trie->bc[s].base;
}

static void
set_check(diag_trie_t *trie, diag_size_t s, diag_ssize_t check)
{
	assert(trie);
	extend_if_necessary(trie, s);
	trie->bc[s].check = check;
}

static diag_ssize_t
get_check(diag_trie_t *trie, diag_size_t s)
{
	assert(trie);
	extend_if_necessary(trie, s);
	return trie->bc[s].check;
}

static void
relocate(diag_trie_t *trie, diag_ssize_t s, diag_size_t b)
{
	int c, d;
	diag_ssize_t t, u;

	assert(trie && s >= 0 && ((diag_size_t)s < trie->size));
	for (c = 1; c <= 256; c++) {
		t = get_base(trie, s) + c;
		if ((diag_size_t)t >= trie->size) {
			break;
		}
		if (get_check(trie, t) == s) {
			set_check(trie, b + c, s);
			set_base(trie, b + c, trie->bc[t].base);
			for (d = 1; d <= 256; d++) {
				u = get_base(trie, t) + d;
				if ((diag_size_t)u >= trie->size) {
					break;
				}
				if (get_check(trie, u) == t) {
					set_check(trie, u, b + c);
				}
			}
			set_check(trie, t, CODE_NONE);
		}
	}
	set_base(trie, s, b);
}

static diag_size_t
add_transition(diag_trie_t *trie, diag_size_t s, uint8_t b)
{
	diag_size_t t;

	assert(trie);
	t = get_base(trie, s) + CODE(b);
	if (get_check(trie, t) != CODE_NONE) {
		relocate(trie, s, trie->size);
		t = get_base(trie, s) + CODE(b);
	}
	set_check(trie, t, s);
	return t;
}

diag_trie_t *
diag_trie_new(void)
{
	diag_trie_t *trie;

	trie = (diag_trie_t *)diag_malloc(sizeof(diag_trie_t) + sizeof(diag_trie_bc_t));
	trie->size = 1;
	trie->bc[0].base  = CODE_NONE;
	trie->bc[0].check = CODE_NONE;
	return trie;
}

void
diag_trie_destroy(diag_trie_t *trie)
{
	if (trie) {
		diag_free(trie);
	}
}

int
diag_trie_traverse(diag_trie_t *trie, diag_size_t length, const uint8_t *seq, int insert)
{
	diag_ssize_t s, t;
	diag_size_t i;
	int result = 1;

	assert(trie && seq);
	s = 0;
	for (i = 0; i < length; i++) {
		if (i == trie->size) {
			result = 0;
			break;
		}
		t = trie->bc[s].base + CODE(seq[i]);
		if (trie->bc[t].check == s) {
			s = t;
		} else {
			result = 0;
			break;
		}
	}
	if (result == 0 && insert) {
		do {
			s = add_transition(trie, s, seq[i++]);
		} while (i < length);
	}
	return result;
}
