/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_BYTEVECTOR_H
#define DIAGONAL_BYTEVECTOR_H

#define DIAG_BYTEVECTOR_HEAD						\
	/* number of bytes in vector */					\
	size_t size;							\
	/* array of bytes */						\
	uint8_t *data;							\
	/* callback invoked with this bytevector itself for finalization */ \
	void (*finalize)(struct diag_bytevector *)

struct diag_bytevector {
	DIAG_BYTEVECTOR_HEAD;
};

typedef void (*diag_bytevector_finalizer_t)(struct diag_bytevector *);

DIAG_C_DECL_BEGIN

/*
 * Return new diag_bytevector having `data' of size `size'.
 * `data' will be freed in case of finalization.
 */
DIAG_FUNCTION struct diag_bytevector *diag_bytevector_new_heap(size_t size, uint8_t *data);
/*
 * Return new diag_bytevector having the content of file of path `path'.
 * The data is mounted as read-only, and will be unmounted in case of
 * finalization.
 */
DIAG_FUNCTION struct diag_bytevector *diag_bytevector_new_path(const char *path);
/*
 * Return a copy of `bv'.
 * It consists of bytes of length (size of `bv') + 1 and is null-terminated.
 * The client code is responsible to free it after use.
 */
DIAG_FUNCTION char *diag_bytevector_to_asciz(const struct diag_bytevector *bv);
/*
 * Finalize and free `bv'.
 */
DIAG_FUNCTION void diag_bytevector_destroy(struct diag_bytevector *bv);

DIAG_C_DECL_END

#endif
