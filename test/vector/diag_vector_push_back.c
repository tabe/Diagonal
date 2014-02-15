/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/vector.h"

int main(void)
{
	struct diag_vector *v;

	v = diag_vector_create(0);
	assert(v);
	diag_vector_push_back(v, (intptr_t)'a');
	diag_vector_push_back(v, (intptr_t)'b');
	diag_vector_push_back(v, (intptr_t)'c');
	ASSERT_EQ_SIZE(3, diag_vector_length(v));
	ASSERT_EQ_CHAR('a', diag_vector_ref(v, 0));
	ASSERT_EQ_CHAR('b', diag_vector_ref(v, 1));
	ASSERT_EQ_CHAR('c', diag_vector_ref(v, 2));
	diag_vector_destroy(v);

	return EXIT_SUCCESS;
}
