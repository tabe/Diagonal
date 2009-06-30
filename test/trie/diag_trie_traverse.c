#include "test.h"

#include "diagonal.h"
#include "diagonal/trie.h"

int
main()
{
	diag_trie_t *trie;
	int r;

	trie = diag_trie_new();
	r = diag_trie_traverse(trie, 4, (uint8_t *)"abc", 0);
	assert(!r && trie->size == 1);
	r = diag_trie_traverse(trie, 4, (uint8_t *)"abc", 1);
	assert(!r && trie->size == 101);
	diag_trie_destroy(trie);
	return EXIT_SUCCESS;
}
