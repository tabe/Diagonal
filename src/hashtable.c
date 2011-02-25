/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

struct diag_hashtable *
diag_hashtable_new_eq(diag_size_t size)
{
	struct diag_hashtable *ht;

	ht = diag_malloc(sizeof(struct diag_hashtable));
	ht->size = size;
	ht->mutable = 1;
	return ht;
}

void
diag_hashtable_destroy(struct diag_hashtable *ht)
{
	diag_free(ht);
}

diag_size_t
diag_hashtable_size(struct diag_hashtable *ht)
{
	assert(ht);
	return ht->size;
}

diag_object_t 
diag_hashtable_ref(struct diag_hashtable *ht, diag_object_t key, diag_object_t alt)
{
	assert(ht && key && alt);
	return alt;
}

void
diag_hashtable_set(struct diag_hashtable *ht, diag_object_t key, diag_object_t value)
{
	assert(ht && key && value);
}

void
diag_hashtable_delete(struct diag_hashtable *ht, diag_object_t key)
{
	assert(ht && key);
}

int
diag_hashtable_contains(struct diag_hashtable *ht, diag_object_t key)
{
	assert(ht && key);
	return 0;
}

struct diag_hashtable *
diag_hashtable_copy(struct diag_hashtable *ht, int mutable)
{
	struct diag_hashtable *copy;

	assert(ht);
	copy = diag_malloc(sizeof(struct diag_hashtable));
	memcpy(copy, ht, sizeof(struct diag_hashtable));
	copy->mutable = mutable;
	return copy;
}

void
diag_hashtable_clear(struct diag_hashtable *ht, ssize_t k)
{
	assert(ht);
	if (k >= 0) {
		
	}
}

struct diag_vector *
diag_hashtable_keys(struct diag_hashtable *ht)
{
	struct diag_vector *v;

	assert(ht);
	v = diag_vector_new(ht->size);
	return v;
}

void
diag_hashtable_entries(struct diag_hashtable *ht, struct diag_vector **keys, struct diag_vector **values)
{
	struct diag_vector *kv, *vv;

	assert(ht && keys && values);
	kv = diag_vector_new(ht->size);
	vv = diag_vector_new(ht->size);
	*keys   = kv;
	*values = vv;
}

int
diag_hashtable_mutable(struct diag_hashtable *ht)
{
	assert(ht);
	return ht->mutable;
}
