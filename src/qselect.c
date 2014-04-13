/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/qselect.h"

/* Public API */

void *diag_qselect(void *base, size_t nmemb, size_t size,
		   int (*cmp)(const void *, const void *),
		   size_t k)
{
	assert(base);
	assert(nmemb > 0);
	assert(size > 0);

	if (nmemb <= k) return NULL;
	if (nmemb == 1) return base;

	int *rv = diag_calloc(nmemb, sizeof(int));
	size_t i;
	size_t n_lt;
	size_t n_gt;
	size_t n_eq;
	char *p;

 partition:
	assert(k < nmemb);
	if (nmemb == 1) goto done;

	n_lt = 0;
	n_gt = 0;
	p = (char *)base;
	for (i = 1; i < nmemb; i++) {
		int r = cmp(p, p + size * i);
		if (r < 0) n_lt++;
		if (0 < r) n_gt++;
		rv[i] = r;
	}
	n_eq = nmemb - n_lt - n_gt;
	if (k < n_lt) {
		size_t j = n_lt + 1;
		for (i = 1; i <= n_lt; i++) {
			if (rv[i] >= 0) {
				while (j < nmemb && rv[j] >= 0) {
					j++;
				}
				assert(j < nmemb);
				memcpy(p + size * i, p + size * j++, size);
			}
		}
		base = p + size;
		nmemb = n_lt;
		/* k is unchanged */
		goto partition;
	}
	if (k >= n_lt + n_eq) {
		size_t j = n_gt + 1;
		for (i = 1; i <= n_gt; i++) {
			if (rv[i] <= 0) {
				while (j < nmemb && rv[j] <= 0) {
					j++;
				}
				assert(j < nmemb);
				memcpy(p + size * i, p + size * j++, size);
			}
		}
		base = p + size;
		nmemb = n_gt;
		k -= n_lt + n_eq;
		goto partition;
	}

 done:
	diag_free(rv);
	return base;
}
