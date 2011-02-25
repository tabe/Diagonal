/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_BYTEVECTOR_H
#define DIAGONAL_BYTEVECTOR_H

struct diag_bytevector {
	diag_size_t size;
	uint8_t *data;
	void (*finalize)(struct diag_bytevector *);
};

typedef void (*diag_bytevector_finalizer_t)(struct diag_bytevector *);

DIAG_C_DECL_BEGIN

extern struct diag_bytevector *diag_bytevector_new_heap(diag_size_t size, uint8_t *data);
extern struct diag_bytevector *diag_bytevector_new_path(const char *path);

extern char *diag_bytevector_to_asciz(const struct diag_bytevector *bv);

extern void diag_bytevector_destroy(struct diag_bytevector *bv);

DIAG_C_DECL_END

#endif
