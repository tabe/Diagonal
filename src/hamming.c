/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/datum.h"
#include "diagonal/metric.h"

uintptr_t diag_hamming_chars(intptr_t a, intptr_t b)
{
	const char *x, *y;
	register uintptr_t d = 0;

	x = (const char *)a;
	y = (const char *)b;
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

intptr_t diag_ehamming_chars(intptr_t a, intptr_t b, uintptr_t e)
{
	const char *x, *y;
	size_t lx, ly, dxy;
	register uintptr_t d;

	x = (const char *)a;
	y = (const char *)b;
	assert(x && y);
	if (e == 0) return -1;
	if (x == y) return 0;
	lx = strlen(x);
	ly = strlen(y);
	dxy = (lx < ly) ? ly - lx : lx - ly;
	if (dxy >= (size_t)e) return -1;
	d = (uintptr_t)dxy;
	do {
		if (*x == '\0' || *y == '\0') return d;
		if (*x++ != *y++) d++;
	} while (d < e);
	return -1;
}
