/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/private/memory.h"

/* API */

size_t diag_memory_to_lines(size_t s, const char *src,
			    char **dst, char ***lines)
{
	size_t i, k = 0, ls;
	char *p, *q = NULL, **lp;

	assert(src && dst && lines);
	if (s == 0) return 0;
	p = diag_malloc(s + 1);
	memcpy(p, src, s);
	p[s] = '\0';
	*dst = p;
	ls = (s >> 1) + 2;
	lp = diag_calloc(ls, sizeof(*lp));
	*lines = lp;
	for (i = 0; i < s; i++) {
		if (!p[i]) {
			if (q) {
				lp[k++] = q;
				q = NULL;
			}
		} else if (p[i] == '\n' || p[i] == '\r') {
			p[i] = '\0';
			if (q) {
				lp[k++] = q;
				q = NULL;
			}
		} else if (!q) {
			q = p + i;
		}
	}
	if (q) lp[k++] = q;
	if (!k) {
		diag_free(*lines);
		diag_free(*dst);
		return 0;
	}
	assert(k < ls);
	lp[k] = NULL;
	return k;
}
