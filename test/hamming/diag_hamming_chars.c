/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/metric.h"

#define DIAG_HAMMING_CHARS(x, y) \
	diag_hamming_chars((intptr_t)x, (intptr_t)y)

int main()
{
	assert(0 == DIAG_HAMMING_CHARS("", ""));
	assert(0 == DIAG_HAMMING_CHARS("The metric", "The metric"));
	assert(3 == DIAG_HAMMING_CHARS("foo", ""));
	assert(3 == DIAG_HAMMING_CHARS("", "bar"));
	assert(3 == DIAG_HAMMING_CHARS("Monday", "Friday"));
	assert(9 == DIAG_HAMMING_CHARS("Wednesday", "Sunday"));
	assert(3 == DIAG_HAMMING_CHARS("kitten", "sitting"));
	return EXIT_SUCCESS;
}
