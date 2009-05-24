#include "test.h"

#include "diagonal.h"
#include "diagonal/metric.h"

int
main()
{
	assert(-1 == diag_elevenshtein_chars("", "", 0));
	assert(0 == diag_elevenshtein_chars("", "", 1));
	assert(0 == diag_elevenshtein_chars("The metric", "The metric", 1));
	assert(3 == diag_elevenshtein_chars("foo", "", 4));
	assert(-1 == diag_elevenshtein_chars("foo", "", 2));
	assert(3 == diag_elevenshtein_chars("", "bar", 4));
	assert(-1 == diag_elevenshtein_chars("", "bar", 2));
	assert(3 == diag_elevenshtein_chars("foo", "foobar", 4));
	assert(-1 == diag_elevenshtein_chars("foo", "foobar", 3));
	assert(3 == diag_elevenshtein_chars("foobar", "bar", 4));
	assert(-1 == diag_elevenshtein_chars("foobar", "bar", 3));
	assert(3 == diag_elevenshtein_chars("Monday", "Friday", 100));
	assert(-1 == diag_elevenshtein_chars("Monday", "Friday", 2));
	assert(5 == diag_elevenshtein_chars("Wednesday", "Sunday", 19));
	assert(-1 == diag_elevenshtein_chars("Wednesday", "Sunday", 5));
	assert(3 == diag_elevenshtein_chars("kitten", "sitting", 20));
	assert(-1 == diag_elevenshtein_chars("kitten", "sitting", 1));
	return EXIT_SUCCESS;
}
