#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/object.h"
#include "diagonal/vector.h"

diag_vector_t *
diag_vector_new(diag_size_t length)
{
	diag_vector_t *v;
	size_t s;

	s = sizeof(diag_vector_t) + sizeof(diag_object_t) * (size_t)length;
	/* TODO: check the overflow */
	v = diag_malloc(s);
	v->length = length;
	return v;
}

void
diag_vector_destroy(diag_vector_t *v)
{
	diag_free(v);
}


diag_size_t
diag_vector_length(diag_vector_t *v)
{
	assert(v);
	return v->length;
}

diag_object_t 
diag_vector_ref(diag_vector_t *v, diag_size_t k)
{
	if (k >= v->length) {
		diag_error("exceed vector length %ld: %ld", v->length, k);
	}
	return v->elements[k];
}

void
diag_vector_set(diag_vector_t *v, diag_size_t k, diag_object_t e)
{
	assert(v && e);
	if (k >= v->length) {
		diag_error("exceed vector length %ld: %ld", v->length, k);
	}
	v->elements[k] = e;
}

void
diag_vector_fill(diag_vector_t *v, diag_object_t fill)
{
	diag_size_t k;

	assert(v && fill);
	for (k = 0; k < v->length; k++) {
		v->elements[k] = fill;
	}
}
