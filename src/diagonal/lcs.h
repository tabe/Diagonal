/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_LCS_H
#define DIAGONAL_LCS_H

/*
 * The longest common subsequence problem
 */
struct diag_lcs {
	const struct diag_vector *vx, *vy;
	int (*eq)(intptr_t, intptr_t);
};

typedef int (*diag_lcs_eq_t)(intptr_t, intptr_t);

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_lcs *diag_lcs_create(const struct diag_vector *vx,
					       const struct diag_vector *vy,
					       diag_lcs_eq_t eq);

DIAG_FUNCTION void diag_lcs_destroy(struct diag_lcs *lcs);

DIAG_FUNCTION int diag_lcs_compute(struct diag_lcs *lcs,
				   struct diag_vector **ses);

DIAG_C_DECL_END

#endif
