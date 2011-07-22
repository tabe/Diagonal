/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/metric.h"

#define DIAG_ELEVENSHTEIN_CHARS(x, y, e) \
	diag_elevenshtein_chars((intptr_t)x, (intptr_t)y, e)

int main()
{
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("", "", 0));
	assert(0 == DIAG_ELEVENSHTEIN_CHARS("", "", 1));
	assert(0 == DIAG_ELEVENSHTEIN_CHARS("The metric", "The metric", 1));
	assert(3 == DIAG_ELEVENSHTEIN_CHARS("foo", "", 4));
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("foo", "", 2));
	assert(3 == DIAG_ELEVENSHTEIN_CHARS("", "bar", 4));
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("", "bar", 2));
	assert(3 == DIAG_ELEVENSHTEIN_CHARS("foo", "foobar", 4));
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("foo", "foobar", 3));
	assert(3 == DIAG_ELEVENSHTEIN_CHARS("foobar", "bar", 4));
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("foobar", "bar", 3));
	assert(3 == DIAG_ELEVENSHTEIN_CHARS("Monday", "Friday", 100));
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("Monday", "Friday", 2));
	assert(5 == DIAG_ELEVENSHTEIN_CHARS("Wednesday", "Sunday", 19));
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("Wednesday", "Sunday", 5));
	assert(3 == DIAG_ELEVENSHTEIN_CHARS("kitten", "sitting", 20));
	assert(-1 == DIAG_ELEVENSHTEIN_CHARS("kitten", "sitting", 1));
	return EXIT_SUCCESS;
}
