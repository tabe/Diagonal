/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_VECTOR_H
#define DIAGONAL_VECTOR_H

struct diag_vector {
	diag_size_t length;
	diag_object_t elements[];
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_vector *diag_vector_new(diag_size_t length);

DIAG_FUNCTION void diag_vector_destroy(struct diag_vector *v);

DIAG_FUNCTION diag_size_t diag_vector_length(struct diag_vector *v);

DIAG_FUNCTION diag_object_t diag_vector_ref(struct diag_vector *v, diag_size_t k);

DIAG_FUNCTION void diag_vector_set(struct diag_vector *v, diag_size_t k, diag_object_t e);

DIAG_FUNCTION void diag_vector_fill(struct diag_vector *v, diag_object_t fill);

DIAG_C_DECL_END

#endif /* DIAGONAL_VECTOR_H */
