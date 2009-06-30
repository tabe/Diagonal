#include "test.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/vector.h"
#include "diagonal/trie.h"
#include "diagonal/hashtable.h"

int
main()
{
	diag_hashtable_t *ht;

	ht = diag_hashtable_new_eq(0);
	assert(diag_hashtable_size(ht) == 0);
	return EXIT_SUCCESS;
}
