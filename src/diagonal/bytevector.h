#ifndef DIAGONAL_BYTEVECTOR_H
#define DIAGONAL_BYTEVECTOR_H

typedef struct diag_bytevector_s {
	diag_size_t size;
	uint8_t *data;
	void (*finalize)(struct diag_bytevector_s *);
} diag_bytevector_t;

typedef void (*diag_bytevector_finalizer_t)(diag_bytevector_t *);

DIAG_C_DECL_BEGIN

extern diag_bytevector_t *diag_bytevector_new_path(const char *path);

extern char *diag_bytevector_to_asciz(const diag_bytevector_t *bv);

extern void diag_bytevector_destroy(diag_bytevector_t *bv);

DIAG_C_DECL_END

#endif
