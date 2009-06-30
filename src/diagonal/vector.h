#ifndef DIAGONAL_VECTOR_H
#define DIAGONAL_VECTOR_H

typedef uintptr_t diag_vector_element_t;

typedef struct diag_vector_s {
	diag_size_t length;
	diag_vector_element_t elements[];
} diag_vector_t;

DIAG_C_DECL_BEGIN

extern diag_vector_t *diag_vector_new(diag_size_t length);

extern void diag_vector_destroy(diag_vector_t *v);

extern diag_size_t diag_vector_length(diag_vector_t *v);

extern diag_vector_element_t diag_vector_ref(diag_vector_t *v, diag_size_t k);

extern void diag_vector_set(diag_vector_t *v, diag_size_t k, diag_vector_element_t e);

extern void diag_vector_fill(diag_vector_t *v, diag_vector_element_t fill);

DIAG_C_DECL_END

#endif /* DIAGONAL_VECTOR_H */
