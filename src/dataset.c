/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/dataset.h"

struct diag_dataset *diag_dataset_create(size_t num_data)
{
	struct diag_dataset *ds;
	size_t s;

	s = sizeof(*ds) + num_data * sizeof(struct diag_datum *);
	ds = diag_malloc(s);
	ds->num_data = num_data;
	return ds;
}

void diag_dataset_destroy(struct diag_dataset *ds)
{
	if (!ds) return;
	diag_free(ds);
}
