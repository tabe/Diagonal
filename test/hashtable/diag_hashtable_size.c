/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

int main(void)
{
	struct diag_hashtable *ht;

	ht = diag_hashtable_new_eq(0);
	assert(diag_hashtable_size(ht) == 0);
	diag_hashtable_destroy(ht);
	return EXIT_SUCCESS;
}
