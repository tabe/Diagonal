#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/metric.h"

diag_distance_t
diag_hamming_chars(const char *x, const char *y)
{
	register diag_distance_t d = 0;

	assert(x && y);
	if (x == y) return 0;
	for (;;) {
		if (*x == '\0') {
			d += strlen(y);
			break;
		}
		if (*y == '\0') {
			d += strlen(x);
			break;
		}
		if (*x++ != *y++) d++;
	}
	return d;
}
