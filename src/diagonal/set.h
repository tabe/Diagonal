/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_SET_H
#define DIAGONAL_SET_H

struct diag_set {
	int (*cmp)(const void *, const void *);
	size_t size;
	intptr_t *arr;
};

typedef int (*diag_set_cmp_t)(const void *, const void *);

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_set *diag_set_create(diag_set_cmp_t);

DIAG_FUNCTION int diag_set_contains(struct diag_set *, intptr_t);

DIAG_FUNCTION const intptr_t *diag_set_insert(struct diag_set *, intptr_t);

DIAG_FUNCTION int diag_set_erase(struct diag_set *, intptr_t);

DIAG_FUNCTION void diag_set_destroy(struct diag_set *);

DIAG_C_DECL_END

#endif /* DIAGONAL_SET_H */
