/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/vector.h"

struct diag_vector *
diag_vector_create(size_t length)
{
	struct diag_vector *v;
	size_t s;

	s = sizeof(*v) + sizeof(intptr_t) * (size_t)length;
	/* TODO: check the overflow */
	v = diag_malloc(s);
	v->length = length;
	return v;
}

void
diag_vector_destroy(struct diag_vector *v)
{
	diag_free(v);
}


size_t
diag_vector_length(struct diag_vector *v)
{
	assert(v);
	return v->length;
}

intptr_t
diag_vector_ref(struct diag_vector *v, size_t k)
{
	if (k >= v->length) {
		diag_error("exceed vector length %ld: %ld", v->length, k);
	}
	return v->elements[k];
}

void
diag_vector_set(struct diag_vector *v, size_t k, intptr_t e)
{
	assert(v && e);
	if (k >= v->length) {
		diag_error("exceed vector length %ld: %ld", v->length, k);
	}
	v->elements[k] = e;
}

void
diag_vector_fill(struct diag_vector *v, intptr_t fill)
{
	size_t k;

	assert(v && fill);
	for (k = 0; k < v->length; k++) {
		v->elements[k] = fill;
	}
}
