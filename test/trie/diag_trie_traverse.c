/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/trie.h"

#define AB   ((const uint8_t *)"ab")
#define ABC  ((const uint8_t *)"abc")
#define ABCD ((const uint8_t *)"abcd")
#define ABE  ((const uint8_t *)"abe")

static void
dump(struct diag_trie *trie)
{
	ssize_t i, b, c;

	printf("-- size %zd\n", trie->size);
	for (i = 0; i < trie->size; i++) {
		b = trie->bc[i].base;
		c = trie->bc[i].check;
		if ((b|c) != 0) {
			printf("%03zd: %03zd|%03zd\n", i, b, c);
		}
	}
}

int
main()
{
	struct diag_trie *trie, *next;
	int r;

	trie = diag_trie_new();
	r = diag_trie_traverse(trie, 4, ABC, 0);
	assert(!r && trie->size == 1);

	r = diag_trie_traverse(trie, 4, ABC, &next);
	dump(next);
	assert(!r);
	trie = next;
	r = diag_trie_traverse(trie, 4, ABC, 0);
	assert(r);
	r = diag_trie_traverse(trie, 3, ABC, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 1, AB, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 2, AB, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 3, AB, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 4, ABCD, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 5, ABCD, 0);
	assert(!r);

	r = diag_trie_traverse(trie, 4, ABE, &next);
	dump(next);
	assert(!r);
	trie = next;
	r = diag_trie_traverse(trie, 4, ABC, 0);
	assert(r);
	r = diag_trie_traverse(trie, 4, ABE, 0);
	assert(r);
	r = diag_trie_traverse(trie, 3, ABE, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 2, ABE, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 1, ABE, 0);
	assert(!r);

	r = diag_trie_traverse(trie, 2, AB, &next);
	dump(next);
	assert(!r);
	trie = next;
	r = diag_trie_traverse(trie, 4, ABE, 0);
	assert(r);
	r = diag_trie_traverse(trie, 3, ABE, 0);
	assert(!r);
	r = diag_trie_traverse(trie, 2, ABE, 0);
	assert(r);
	r = diag_trie_traverse(trie, 1, ABE, 0);
	assert(!r);

	diag_trie_destroy(trie);
	return EXIT_SUCCESS;
}
