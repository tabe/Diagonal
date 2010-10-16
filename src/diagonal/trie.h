#ifndef DIAGONAL_TRIE_H
#define DIAGONAL_TRIE_H

struct diag_trie_bc {
	diag_ssize_t base;
	diag_ssize_t check;
};

struct diag_trie {
	diag_ssize_t size;
	struct diag_trie_bc bc[];
};

DIAG_C_DECL_BEGIN

extern struct diag_trie *diag_trie_new(void);

extern void diag_trie_destroy(struct diag_trie *trie);

extern int diag_trie_traverse(struct diag_trie *trie, diag_ssize_t length, const uint8_t *seq, struct diag_trie **insert);

DIAG_C_DECL_END

#endif /* DIAGONAL_TRIE_H */
