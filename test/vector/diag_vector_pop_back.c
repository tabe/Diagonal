/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/vector.h"

int main(void)
{
	struct diag_vector *v;

	v = diag_vector_create(5);
	assert(v);
	diag_vector_fill(v, (intptr_t)100);
	diag_vector_pop_back(v);
	ASSERT_EQ_SIZE(4, diag_vector_length(v));
	diag_vector_pop_back(v);
	ASSERT_EQ_SIZE(3, diag_vector_length(v));
	diag_vector_pop_back(v);
	ASSERT_EQ_SIZE(2, diag_vector_length(v));
	diag_vector_pop_back(v);
	ASSERT_EQ_SIZE(1, diag_vector_length(v));
	diag_vector_pop_back(v);
	ASSERT_EQ_SIZE(0, diag_vector_length(v));
	diag_vector_destroy(v);

	return EXIT_SUCCESS;
}
