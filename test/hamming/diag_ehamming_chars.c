/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/metric.h"

#define DIAG_EHAMMING_CHARS(x, y, e) \
	diag_ehamming_chars((intptr_t)x, (intptr_t)y, e)

int main()
{
	assert(-1 == DIAG_EHAMMING_CHARS("", "", 0));
	assert(0 == DIAG_EHAMMING_CHARS("", "", 1));
	assert(0 == DIAG_EHAMMING_CHARS("The metric", "The metric", 1));
	assert(3 == DIAG_EHAMMING_CHARS("foo", "", 4));
	assert(-1 == DIAG_EHAMMING_CHARS("foo", "", 2));
	assert(3 == DIAG_EHAMMING_CHARS("", "bar", 5));
	assert(-1 == DIAG_EHAMMING_CHARS("", "bar", 1));
	assert(3 == DIAG_EHAMMING_CHARS("Monday", "Friday", 4));
	assert(-1 == DIAG_EHAMMING_CHARS("Monday", "Friday", 2));
	assert(9 == DIAG_EHAMMING_CHARS("Wednesday", "Sunday", 100));
	assert(-1 == DIAG_EHAMMING_CHARS("Wednesday", "Sunday", 9));
	assert(3 == DIAG_EHAMMING_CHARS("kitten", "sitting", 19));
	assert(-1 == DIAG_EHAMMING_CHARS("kitten", "sitting", 1));
	return EXIT_SUCCESS;
}
