/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/metric.h"

#define DIAG_LEVENSHTEIN_CHARS(x, y) \
	diag_levenshtein_chars((intptr_t)x, (intptr_t)y)

int main()
{
	assert(0 == DIAG_LEVENSHTEIN_CHARS("", ""));
	assert(0 == DIAG_LEVENSHTEIN_CHARS("The metric", "The metric"));
	assert(3 == DIAG_LEVENSHTEIN_CHARS("foo", ""));
	assert(3 == DIAG_LEVENSHTEIN_CHARS("", "bar"));
	assert(3 == DIAG_LEVENSHTEIN_CHARS("foo", "foobar"));
	assert(3 == DIAG_LEVENSHTEIN_CHARS("foobar", "bar"));
	assert(3 == DIAG_LEVENSHTEIN_CHARS("Monday", "Friday"));
	assert(5 == DIAG_LEVENSHTEIN_CHARS("Wednesday", "Sunday"));
	assert(3 == DIAG_LEVENSHTEIN_CHARS("kitten", "sitting"));
	return EXIT_SUCCESS;
}
