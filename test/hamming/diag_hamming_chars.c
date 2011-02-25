/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/metric.h"

int
main()
{
	assert(0 == diag_hamming_chars("", ""));
	assert(0 == diag_hamming_chars("The metric", "The metric"));
	assert(3 == diag_hamming_chars("foo", ""));
	assert(3 == diag_hamming_chars("", "bar"));
	assert(3 == diag_hamming_chars("Monday", "Friday"));
	assert(9 == diag_hamming_chars("Wednesday", "Sunday"));
	assert(3 == diag_hamming_chars("kitten", "sitting"));
	return EXIT_SUCCESS;
}
