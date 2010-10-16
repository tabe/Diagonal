#ifndef DIAGONAL_VECTOR_H
#define DIAGONAL_VECTOR_H

struct diag_vector {
	diag_size_t length;
	diag_object_t elements[];
};

DIAG_C_DECL_BEGIN

extern struct diag_vector *diag_vector_new(diag_size_t length);

extern void diag_vector_destroy(struct diag_vector *v);

extern diag_size_t diag_vector_length(struct diag_vector *v);

extern diag_object_t diag_vector_ref(struct diag_vector *v, diag_size_t k);

extern void diag_vector_set(struct diag_vector *v, diag_size_t k, diag_object_t e);

extern void diag_vector_fill(struct diag_vector *v, diag_object_t fill);

DIAG_C_DECL_END

#endif /* DIAGONAL_VECTOR_H */
