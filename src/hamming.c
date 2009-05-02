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

diag_sdistance_t
diag_ehamming_chars(const char *x, const char *y, diag_distance_t e)
{
	size_t lx, ly, dxy;
	register diag_distance_t d;

	assert(x && y);
	if (e == 0) return -1;
	if (x == y) return 0;
	lx = strlen(x);
	ly = strlen(y);
	dxy = (lx < ly) ? ly - lx : lx - ly;
	if (dxy >= (size_t)e) return -1;
	d = (diag_distance_t)dxy;
	do {
		if (*x == '\0' || *y == '\0') return d;
		if (*x++ != *y++) d++;
	} while (d < e);
	return -1;
}
