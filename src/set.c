/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/set.h"

static int compare(const void *x, const void *y)
{
	const intptr_t *a, *b;
	a = (const intptr_t *)x;
	b = (const intptr_t *)y;
	return (*a > *b) - (*a < *b);
}

/* API */

struct diag_set *diag_set_create(diag_set_cmp_t cmp)
{
	struct diag_set *set;

	set = diag_malloc(sizeof(*set));
	set->cmp = cmp;
	set->size = 0;
	set->arr = NULL;
	return set;
}

int diag_set_contains(struct diag_set *set, intptr_t x)
{
	diag_set_cmp_t cmp;
	intptr_t *r;

	if (set->size == 0) return 0;
	cmp = (set->cmp) ? set->cmp : compare;
	r = bsearch(&x, set->arr, set->size, sizeof(x), cmp);
	return r ? 1 : 0;
}

const intptr_t *diag_set_insert(struct diag_set *set, intptr_t x)
{
	diag_set_cmp_t cmp;
	intptr_t *r;
	size_t i;

	if (set->size == 0) {
		set->size++;
		set->arr = diag_malloc(sizeof(x));
		set->arr[0] = x;
		return NULL;
	}
	cmp = (set->cmp) ? set->cmp : compare;
	r = bsearch(&x, set->arr, set->size, sizeof(x), cmp);
	if (!r) {
		i = set->size++;
		set->arr = diag_realloc(set->arr, set->size * sizeof(x));
		set->arr[i] = x;
		qsort(set->arr, set->size, sizeof(x), cmp);
		return NULL;
	}
	return r;
}

int diag_set_erase(struct diag_set *set, intptr_t x)
{
	diag_set_cmp_t cmp;
	intptr_t *r;
	size_t s;

	if (set->size == 0) return 0;
	cmp = (set->cmp) ? set->cmp : compare;
	r = bsearch(&x, set->arr, set->size, sizeof(x), cmp);
	if (!r) return 0;
	set->size--;
	s = (size_t)((set->arr + set->size) - r);
	if (s > 0) memmove(r, r + 1, s * sizeof(x));
	return 1;
}

void diag_set_destroy(struct diag_set *set)
{
	if (!set) return;
	diag_free(set->arr);
	diag_free(set);
}
