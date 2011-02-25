/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/trie.h"

int
main()
{
	struct diag_trie *trie;

	trie = diag_trie_new();
	assert(trie->size == 1);
	diag_trie_destroy(trie);
	return EXIT_SUCCESS;
}
