#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/object.h"
#include "diagonal/vector.h"
#include "diagonal/trie.h"
#include "diagonal/hashtable.h"

diag_hashtable_t *
diag_hashtable_new_eq(diag_size_t size)
{
	diag_hashtable_t *ht;

	ht = diag_malloc(sizeof(diag_hashtable_t));
	ht->size = size;
	ht->mutable = 1;
	return ht;
}

void
diag_hashtable_destroy(diag_hashtable_t *ht)
{
	diag_free(ht);
}

diag_size_t
diag_hashtable_size(diag_hashtable_t *ht)
{
	assert(ht);
	return ht->size;
}

diag_object_t 
diag_hashtable_ref(diag_hashtable_t *ht, diag_object_t key, diag_object_t alt)
{
	assert(ht && key && alt);
	return alt;
}

void
diag_hashtable_set(diag_hashtable_t *ht, diag_object_t key, diag_object_t value)
{
	assert(ht && key && value);
}

void
diag_hashtable_delete(diag_hashtable_t *ht, diag_object_t key)
{
	assert(ht && key);
}

int
diag_hashtable_contains(diag_hashtable_t *ht, diag_object_t key)
{
	assert(ht && key);
	return 0;
}

diag_hashtable_t *
diag_hashtable_copy(diag_hashtable_t *ht, int mutable)
{
	diag_hashtable_t *copy;

	assert(ht);
	copy = diag_malloc(sizeof(diag_hashtable_t));
	memcpy(copy, ht, sizeof(diag_hashtable_t));
	copy->mutable = mutable;
	return copy;
}

void
diag_hashtable_clear(diag_hashtable_t *ht, diag_ssize_t k)
{
	assert(ht);
	if (k >= 0) {
		
	}
}

diag_vector_t *
diag_hashtable_keys(diag_hashtable_t *ht)
{
	diag_vector_t *v;

	assert(ht);
	v = diag_vector_new(ht->size);
	return v;
}

void
diag_hashtable_entries(diag_hashtable_t *ht, diag_vector_t **keys, diag_vector_t **values)
{
	diag_vector_t *kv, *vv;

	assert(ht && keys && values);
	kv = diag_vector_new(ht->size);
	vv = diag_vector_new(ht->size);
	*keys   = kv;
	*values = vv;
}

int
diag_hashtable_mutable(diag_hashtable_t *ht)
{
	assert(ht);
	return ht->mutable;
}
