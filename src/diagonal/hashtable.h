#ifndef DIAGONAL_HASHTABLE_H
#define DIAGONAL_HASHTABLE_H

typedef uintptr_t diag_hashtable_key_t;
typedef uintptr_t diag_hashtable_value_t;

typedef struct diag_hashtable_s {
	diag_size_t size;
	diag_trie_t *trie;
	int mutable;
} diag_hashtable_t;

DIAG_C_DECL_BEGIN

extern diag_hashtable_t *diag_hashtable_new_eq(diag_size_t size);

extern void diag_hashtable_destroy(diag_hashtable_t *ht);

extern diag_size_t diag_hashtable_size(diag_hashtable_t *ht);

extern diag_hashtable_value_t diag_hashtable_ref(diag_hashtable_t *ht, diag_hashtable_key_t key, diag_hashtable_value_t alt);

extern void diag_hashtable_set(diag_hashtable_t *ht, diag_hashtable_key_t key, diag_hashtable_value_t value);

extern void diag_hashtable_delete(diag_hashtable_t *ht, diag_hashtable_key_t key);

extern int diag_hashtable_contains(diag_hashtable_t *ht, diag_hashtable_key_t key);

extern diag_hashtable_t *diag_hashtable_copy(diag_hashtable_t *ht, int mutable);

extern void diag_hashtable_clear(diag_hashtable_t *ht, diag_ssize_t k);

extern diag_vector_t *diag_hashtable_keys(diag_hashtable_t *ht);

extern void diag_hashtable_entries(diag_hashtable_t *ht, diag_vector_t **keys, diag_vector_t **values);

extern int diag_hashtable_mutable(diag_hashtable_t *ht);

DIAG_C_DECL_END

#endif /* DIAGONAL_HASHTABLE_H */
