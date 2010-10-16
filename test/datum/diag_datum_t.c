#include "test.h"

#include "diagonal.h"

int
main()
{
	struct diag_datum *datum;
	char *value;

#define VALUE "struct diag_datum"

	datum = diag_datum_new(NULL);
	DIAG_DATUM_SET_IMMEDIATE(datum, 1);
	assert(DIAG_DATUM_IMMEDIATE_P(datum));
	assert(!DIAG_DATUM_CHARS_P(datum));
	assert(DIAG_DATUM_GET_IMMEDIATE(datum) == 1);

	datum = diag_datum_new(VALUE);
	datum->tag = DIAG_TAG_CHARS;
	assert(!DIAG_DATUM_IMMEDIATE_P(datum));
	assert(DIAG_DATUM_CHARS_P(datum));
	assert(DIAG_DATUM_ASCIZ_P(datum));
	assert(!DIAG_DATUM_TBFRE_P(datum));
	diag_datum_destroy(datum);

	value = diag_malloc(sizeof(VALUE));
	strcpy(value, VALUE);
	datum = diag_datum_new(value);
	datum->tag = DIAG_TAG_CHARS|DIAG_TAG_TBFRE;
	assert(!DIAG_DATUM_IMMEDIATE_P(datum));
	assert(DIAG_DATUM_CHARS_P(datum));
	assert(DIAG_DATUM_ASCIZ_P(datum));
	assert(DIAG_DATUM_TBFRE_P(datum));
	diag_datum_destroy(datum);

#undef VALUE

	return 0;
}
