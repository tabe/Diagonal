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

intptr_t diag_levenshtein_chars(intptr_t a, intptr_t b)
{
	const char *x, *y;
	size_t d = 0;
	register unsigned int i, j;
	size_t lx, ly;
	size_t *cur, *new, *tmp;

	x = (const char *)a;
	y = (const char *)b;
	assert(x && y);
	/* trim the common initial sequence */
	while ( *x && *y && (*x == *y) ) x++, y++;
	lx = strlen(x);
	ly = strlen(y);
	if (lx == 0) return (intptr_t)ly;
	if (ly == 0) return (intptr_t)lx;
	if (lx < ly) {
		const char *z = x;
		size_t lz = lx;
		x = y;
		y = z;
		lx = ly;
		ly = lz;
	}
	/* trim the common final sequence */
	while ( ly > 0 && x[lx-1] == y[ly-1] ) lx--, ly--;
	if (ly == 0) return (intptr_t)lx;
	cur = diag_calloc((ly + 1)*2, sizeof(size_t));
	new = cur + ly + 1;
	for (j = 1; j <= ly; j++) cur[j] = j;
	for (i = 0; i < lx; i++) {
		new[0] = cur[0] + 1;
		for (j = 0; j < ly; j++) {
			size_t k, u, v, w;
			k = j + 1;
			u = new[j] + 1;
			v = cur[k] + 1;
			w = cur[j] + ((x[i] == y[j]) ? 0 : 1);
			d = (u < v) ? u : v;
			d = (w < d) ? w : d;
			new[k] = d;
		}
		tmp = cur;
		cur = new;
		new = tmp;
	}
	diag_free(cur < new ? cur : new);
	return (intptr_t)d;
}

intptr_t diag_elevenshtein_chars(intptr_t a, intptr_t b, intptr_t e)
{
	const char *x, *y;
	size_t lx, ly, dxy;
	intptr_t d;

	x = (const char *)a;
	y = (const char *)b;
	assert(x && y);
	if (e == 0) return -1;
	if (x == y) return 0;
	lx = strlen(x);
	ly = strlen(y);
	dxy = (lx < ly) ? ly - lx : lx - ly;
	if (dxy >= (size_t)e) return -1;
	d = diag_levenshtein_chars(a, b);
	return (d < e) ? d : -1;
}
