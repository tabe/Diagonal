/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/vector.h"

struct diag_vector *
diag_vector_create(size_t length)
{
	struct diag_vector *v;
	size_t c = 1;
	size_t s;

	while (c < length) c <<= 1;
	if (c >= SIZE_MAX / sizeof(*v->elements)) {
		diag_fatal("vector size overflow");
	}
	s = sizeof(*v->elements) * c;
	v = diag_malloc(sizeof(*v));
	v->capacity = c;
	v->length = length;
	v->elements = diag_malloc(s);
	return v;
}

void
diag_vector_destroy(struct diag_vector *v)
{
	if (!v) return;
	diag_free(v->elements);
	diag_free(v);
}


size_t
diag_vector_length(const struct diag_vector *v)
{
	assert(v);
	return v->length;
}

intptr_t
diag_vector_ref(const struct diag_vector *v, size_t k)
{
	assert(v);
	if (k >= v->length) {
		diag_error("exceed vector length %ld: %ld", v->length, k);
	}
	return v->elements[k];
}

void
diag_vector_set(struct diag_vector *v, size_t k, intptr_t e)
{
	assert(v);
	if (k >= v->length) {
		diag_error("exceed vector length %ld: %ld", v->length, k);
	}
	v->elements[k] = e;
}

void
diag_vector_fill(struct diag_vector *v, intptr_t fill)
{
	size_t k;

	assert(v);
	for (k = 0; k < v->length; k++) {
		v->elements[k] = fill;
	}
}

struct diag_vector *diag_vector_copy(const struct diag_vector *v)
{
	struct diag_vector *w;
	size_t length;

	length = v->length;
	w = diag_vector_create(length);
	if (length == 0) return w;
	memcpy(w->elements, v->elements, sizeof(*v->elements) * length);
	return w;
}

struct diag_vector *diag_vector_copy_from(const struct diag_vector *v,
					  size_t start)
{
	struct diag_vector *w;
	size_t s;

	if (start > v->length) return NULL;
	s = v->length - start;
	w = diag_vector_create(s);
	if (s == 0) return w;
	memcpy(w->elements, v->elements + start, sizeof(*v->elements) * s);
	return w;
}

void diag_vector_push_back(struct diag_vector *v, intptr_t e)
{
	assert(v);
	if (v->length == v->capacity) {
		v->capacity <<= 1;
		if (v->capacity >= SIZE_MAX / sizeof(*v->elements)) {
			diag_fatal("vector size overflow");
		}
		size_t s = v->capacity * sizeof(*v->elements);
		v->elements = diag_realloc(v->elements, s);
	}
	v->elements[v->length++] = e;
}

void diag_vector_pop_back(struct diag_vector *v)
{
	assert(v);
	v->length--;
}

void diag_vector_for_each(struct diag_vector *v,
			  diag_vector_callback_t callback,
			  void *data)
{
	assert(v);
	size_t i;
	for (i = 0; i < v->length; i++) {
		callback(i, v->elements[i], data);
	}
}
