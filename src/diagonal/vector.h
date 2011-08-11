/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_VECTOR_H
#define DIAGONAL_VECTOR_H

/*
 * The vector structure
 * This is for representing fixed-length vectors.
 */
struct diag_vector {
	size_t length;
	intptr_t elements[];
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_vector *diag_vector_create(size_t length);

DIAG_FUNCTION void diag_vector_destroy(struct diag_vector *v);

DIAG_FUNCTION size_t diag_vector_length(const struct diag_vector *v);

DIAG_FUNCTION intptr_t diag_vector_ref(const struct diag_vector *v, size_t k);

DIAG_FUNCTION void diag_vector_set(struct diag_vector *v, size_t k, intptr_t e);

DIAG_FUNCTION void diag_vector_fill(struct diag_vector *v, intptr_t fill);

DIAG_FUNCTION struct diag_vector *diag_vector_copy(const struct diag_vector *v);

DIAG_FUNCTION struct diag_vector *
diag_vector_copy_from(const struct diag_vector *v, size_t start);

DIAG_C_DECL_END

#endif /* DIAGONAL_VECTOR_H */
