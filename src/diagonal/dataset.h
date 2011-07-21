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
	size_t num_data;
	struct diag_datum *data[];
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_dataset *diag_dataset_create(size_t);

DIAG_FUNCTION void diag_dataset_destroy(struct diag_dataset *);

DIAG_C_DECL_END

#endif
