/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
diag_levenshtein_chars(const char *x, const char *y)
{
	register diag_distance_t d = 0;
	register unsigned int i, j;
	register unsigned int lx, ly;
	diag_size_t *cur, *new, *tmp;

	assert(x && y);
	/* trim the common initial sequence */
	while ( *x && *y && (*x == *y) ) x++, y++;
	lx = strlen(x);
	ly = strlen(y);
	if (lx == 0) return ly;
	if (ly == 0) return lx;
	if (lx < ly) {
		const char *z = x;
		unsigned int lz = lx;
		x = y;
		y = z;
		lx = ly;
		ly = lz;
	}
	/* trim the common final sequence */
	while ( ly > 0 && x[lx-1] == y[ly-1] ) lx--, ly--;
	if (ly == 0) return lx;
	cur = diag_calloc((ly + 1)*2, sizeof(diag_size_t));
	new = cur + ly + 1;
	for (j = 1; j <= ly; j++) cur[j] = j;
	for (i = 0; i < lx; i++) {
		new[0] = cur[0] + 1;
		for (j = 0; j < ly; j++) {
			size_t k;
			diag_size_t a, b, c;
			k = j + 1;
			a = new[j] + 1;
			b = cur[k] + 1;
			c = cur[j] + ((x[i] == y[j]) ? 0 : 1);
			d = (a < b) ? a : b;
			d = (c < d) ? c : d;
			new[k] = d;
		}
		tmp = cur;
		cur = new;
		new = tmp;
	}
	diag_free(cur < new ? cur : new);
	return d;
}

diag_sdistance_t
diag_elevenshtein_chars(const char *x, const char *y, diag_distance_t e)
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
	d = diag_levenshtein_chars(x, y);
	return (d < e) ? (diag_sdistance_t)d : -1;
}
