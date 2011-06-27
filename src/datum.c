/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/datum.h"

struct diag_datum *
diag_datum_new(void *value)
{
	struct diag_datum *datum;

	datum = diag_malloc(sizeof(struct diag_datum));
	datum->value = value;
	datum->id.number = 0;
	return datum;
}

void
diag_datum_destroy(struct diag_datum *datum)
{
	if (!datum) return;
	if (DIAG_DATUM_TBFRE_P(datum)) diag_free(datum->value);
	diag_free(datum);
}
