/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_DATASET_H
#define DIAGONAL_DATASET_H

/*
 * The dataset structure
 * A dataset is a set of data.
 * A user can
 * - add entries to the set
 * - remove existing entries from the set
 * - sort entries by a given function
 */

struct diag_dataset {
	size_t size;
	struct diag_datum *(*at)(size_t, struct diag_dataset *);
	intptr_t attic;
};

typedef struct diag_datum *(*diag_dataset_at_t)(size_t, struct diag_dataset *);

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_dataset *diag_dataset_create(diag_dataset_at_t,
						       intptr_t);

DIAG_FUNCTION struct diag_datum *diag_dataset_at(struct diag_dataset *, size_t);

DIAG_FUNCTION void diag_dataset_destroy(struct diag_dataset *);

DIAG_C_DECL_END

#endif
