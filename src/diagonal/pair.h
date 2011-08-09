/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_PAIR_H
#define DIAGONAL_PAIR_H

/*
 * The pair structure
 */
struct diag_pair {
	intptr_t car;
	intptr_t cdr;
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_pair *diag_pair_create(intptr_t car, intptr_t cdr);

DIAG_FUNCTION void diag_pair_destroy(struct diag_pair *p);

DIAG_C_DECL_END

#endif /* DIAGONAL_PAIR_H */
