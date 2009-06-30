#include "test.h"

#include "diagonal.h"
#include "diagonal/trie.h"

int
main()
{
	diag_trie_t *trie;

	trie = diag_trie_new();
	assert(trie->size == 1);
	diag_trie_destroy(trie);
	return EXIT_SUCCESS;
}
