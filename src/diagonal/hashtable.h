/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_HASHTABLE_H
#define DIAGONAL_HASHTABLE_H

struct diag_hashtable {
	diag_size_t size;
	struct diag_trie *trie;
	int mutable;
};

DIAG_C_DECL_BEGIN

extern struct diag_hashtable *diag_hashtable_new_eq(diag_size_t size);

extern void diag_hashtable_destroy(struct diag_hashtable *ht);

extern diag_size_t diag_hashtable_size(struct diag_hashtable *ht);

extern diag_object_t diag_hashtable_ref(struct diag_hashtable *ht, diag_object_t key, diag_object_t alt);

extern void diag_hashtable_set(struct diag_hashtable *ht, diag_object_t key, diag_object_t value);

extern void diag_hashtable_delete(struct diag_hashtable *ht, diag_object_t key);

extern int diag_hashtable_contains(struct diag_hashtable *ht, diag_object_t key);

extern struct diag_hashtable *diag_hashtable_copy(struct diag_hashtable *ht, int mutable);

extern void diag_hashtable_clear(struct diag_hashtable *ht, ssize_t k);

extern struct diag_vector *diag_hashtable_keys(struct diag_hashtable *ht);

extern void diag_hashtable_entries(struct diag_hashtable *ht, struct diag_vector **keys, struct diag_vector **values);

extern int diag_hashtable_mutable(struct diag_hashtable *ht);

DIAG_C_DECL_END

#endif /* DIAGONAL_HASHTABLE_H */
