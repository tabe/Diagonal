/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_TRIE_H
#define DIAGONAL_TRIE_H

struct diag_trie_bc {
	ssize_t base;
	ssize_t check;
};

struct diag_trie {
	ssize_t size;
	struct diag_trie_bc bc[];
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_trie *diag_trie_new(void);

DIAG_FUNCTION void diag_trie_destroy(struct diag_trie *trie);

DIAG_FUNCTION int diag_trie_traverse(struct diag_trie *trie, ssize_t length, const uint8_t *seq, struct diag_trie **insert);

DIAG_C_DECL_END

#endif /* DIAGONAL_TRIE_H */
