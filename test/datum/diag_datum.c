/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/datum.h"

static void finalize(struct diag_datum *datum)
{
	diag_free(datum->value);
}

int main()
{
	struct diag_datum *datum;
	char *value;

#define VALUE "struct diag_datum"

	datum = diag_datum_create(1, VALUE);
	assert(!DIAG_DATUM_CUSTOMIZED_P(datum));
	diag_datum_destroy(datum);

	value = diag_malloc(sizeof(VALUE));
	strcpy(value, VALUE);
	datum = (struct diag_datum *)diag_customized_datum_create(2, value, finalize);
	assert(DIAG_DATUM_CUSTOMIZED_P(datum));
	diag_datum_destroy(datum);

#undef VALUE

	return 0;
}
