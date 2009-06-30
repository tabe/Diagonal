#ifndef DIAGONAL_TRIE_H
#define DIAGONAL_TRIE_H

typedef struct diag_trie_bc_s {
	diag_ssize_t base;
	diag_ssize_t check;
} diag_trie_bc_t;

typedef struct diag_trie_s {
	diag_size_t size;
	diag_trie_bc_t bc[];
} diag_trie_t;

DIAG_C_DECL_BEGIN

extern diag_trie_t *diag_trie_new(void);

extern void diag_trie_destroy(diag_trie_t *trie);

extern int diag_trie_traverse(diag_trie_t *trie, diag_size_t length, const uint8_t *seq, int insert);

DIAG_C_DECL_END

#endif /* DIAGONAL_TRIE_H */
