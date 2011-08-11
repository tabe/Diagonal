/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_LIST_H
#define DIAGONAL_LIST_H

/*
 * The list structure
 */

DIAG_C_DECL_BEGIN

DIAG_FUNCTION void diag_list_destroy(struct diag_pair *);

DIAG_FUNCTION size_t diag_list_length(const struct diag_pair *);

DIAG_FUNCTION intptr_t diag_list_ref(const struct diag_pair *, size_t);

DIAG_FUNCTION struct diag_pair *diag_list_reverse(const struct diag_pair *);

DIAG_FUNCTION struct diag_vector *diag_list_to_vector(const struct diag_pair *);

DIAG_C_DECL_END

#endif /* DIAGONAL_LIST_H */
