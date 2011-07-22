/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/dataset.h"

struct diag_dataset *diag_dataset_create(diag_dataset_at_t at, intptr_t attic)
{
	struct diag_dataset *ds;
	size_t s;

	s = sizeof(*ds);
	ds = diag_malloc(s);
	ds->at = at;
	ds->attic = attic;
	return ds;
}

struct diag_datum *diag_dataset_at(struct diag_dataset *ds, size_t i)
{
	assert(ds);
	return ds->at(i, ds);
}

void diag_dataset_destroy(struct diag_dataset *ds)
{
	if (!ds) return;
	diag_free(ds);
}
