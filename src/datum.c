/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/datum.h"

struct diag_datum *diag_datum_create(uintptr_t id, intptr_t value)
{
	struct diag_datum *datum;

	datum = diag_malloc(sizeof(*datum));
	datum->tag = 0;
	datum->id = id;
	datum->value = value;
	return datum;
}

struct diag_customized_datum *
diag_customized_datum_create(uintptr_t id, intptr_t value,
			     diag_datum_finalizer finalize)
{
	struct diag_customized_datum *datum;

	datum = diag_malloc(sizeof(*datum));
	datum->tag = DIAG_TAG_CUSTOMIZED;
	datum->id = id;
	datum->value = value;
	datum->finalize = finalize;
	return datum;
}

void
diag_datum_destroy(struct diag_datum *datum)
{
	if (!datum) return;
	if (DIAG_DATUM_CUSTOMIZED_P(datum)) {
		struct diag_customized_datum *customized_datum;

		customized_datum = (struct diag_customized_datum *)datum;
		customized_datum->finalize(datum);
	}
	diag_free(datum);
}
