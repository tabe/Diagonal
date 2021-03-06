/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_HASHTABLE_H
#define DIAGONAL_HASHTABLE_H

struct diag_hashtable {
	size_t size;
	struct diag_trie *trie;
	int mutable;
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_hashtable *diag_hashtable_new_eq(size_t size);

DIAG_FUNCTION void diag_hashtable_destroy(struct diag_hashtable *ht);

DIAG_FUNCTION size_t diag_hashtable_size(struct diag_hashtable *ht);

DIAG_FUNCTION uintptr_t diag_hashtable_ref(struct diag_hashtable *ht, uintptr_t key, uintptr_t alt);

DIAG_FUNCTION void diag_hashtable_set(struct diag_hashtable *ht, uintptr_t key, uintptr_t value);

DIAG_FUNCTION void diag_hashtable_delete(struct diag_hashtable *ht, uintptr_t key);

DIAG_FUNCTION int diag_hashtable_contains(struct diag_hashtable *ht, uintptr_t key);

DIAG_FUNCTION struct diag_hashtable *diag_hashtable_copy(struct diag_hashtable *ht, int mutable);

DIAG_FUNCTION void diag_hashtable_clear(struct diag_hashtable *ht, ssize_t k);

DIAG_FUNCTION struct diag_vector *diag_hashtable_keys(struct diag_hashtable *ht);

DIAG_FUNCTION void diag_hashtable_entries(struct diag_hashtable *ht, struct diag_vector **keys, struct diag_vector **values);

DIAG_FUNCTION int diag_hashtable_mutable(struct diag_hashtable *ht);

DIAG_C_DECL_END

#endif /* DIAGONAL_HASHTABLE_H */
