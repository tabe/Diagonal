/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_SINGLELINKAGE_H
#define DIAGONAL_SINGLELINKAGE_H

/*
 * Single-linkage method
 */

typedef uintptr_t (*diag_metric_t)(const struct diag_datum *,
				  const struct diag_datum *);

struct diag_proximity_entry {
	uintptr_t d;    /* distance */
	uintptr_t id1; /* id of 1st one */
	uintptr_t id2; /* id of 2nd one */
};

struct diag_pair {
	uintptr_t car;
	uintptr_t cdr;
};

struct diag_singlelinkage {
	struct diag_dataset *ds;
	diag_metric_t f;
	struct diag_rbtree *m;
	struct diag_deque *t;
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_singlelinkage *
diag_singlelinkage_create(struct diag_dataset *, diag_metric_t);

DIAG_FUNCTION int diag_singlelinkage_analyze(struct diag_singlelinkage *);

DIAG_FUNCTION void diag_singlelinkage_destroy(struct diag_singlelinkage *);

DIAG_C_DECL_END

#endif

