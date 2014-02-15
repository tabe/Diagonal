/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/vector.h"

int main(void)
{
	struct diag_vector *v;

	v = diag_vector_create(0);
	assert(diag_vector_length(v) == 0);
	diag_vector_destroy(v);

	v = diag_vector_create(1);
	assert(diag_vector_length(v) == 1);
	diag_vector_destroy(v);

	return EXIT_SUCCESS;
}
