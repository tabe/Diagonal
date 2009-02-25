#include "test.h"

#include "diagonal.h"
#include "diagonal/metric.h"

int
main()
{
	assert(0 == diag_levenshtein_chars("", ""));
	assert(0 == diag_levenshtein_chars("The metric", "The metric"));
	assert(3 == diag_levenshtein_chars("foo", ""));
	assert(3 == diag_levenshtein_chars("", "bar"));
	assert(3 == diag_levenshtein_chars("Monday", "Friday"));
	assert(5 == diag_levenshtein_chars("Wednesday", "Sunday"));
	assert(3 == diag_levenshtein_chars("kitten", "sitting"));
	return 0;
}
