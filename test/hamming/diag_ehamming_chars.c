/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/metric.h"

int
main()
{
	assert(-1 == diag_ehamming_chars("", "", 0));
	assert(0 == diag_ehamming_chars("", "", 1));
	assert(0 == diag_ehamming_chars("The metric", "The metric", 1));
	assert(3 == diag_ehamming_chars("foo", "", 4));
	assert(-1 == diag_ehamming_chars("foo", "", 2));
	assert(3 == diag_ehamming_chars("", "bar", 5));
	assert(-1 == diag_ehamming_chars("", "bar", 1));
	assert(3 == diag_ehamming_chars("Monday", "Friday", 4));
	assert(-1 == diag_ehamming_chars("Monday", "Friday", 2));
	assert(9 == diag_ehamming_chars("Wednesday", "Sunday", 100));
	assert(-1 == diag_ehamming_chars("Wednesday", "Sunday", 9));
	assert(3 == diag_ehamming_chars("kitten", "sitting", 19));
	assert(-1 == diag_ehamming_chars("kitten", "sitting", 1));
	return EXIT_SUCCESS;
}
