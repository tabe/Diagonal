/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/vector.h"

int main()
{
	struct diag_vector *v, *w;

	v = diag_vector_create(3);
	diag_vector_set(v, 0, (intptr_t)'a');
	diag_vector_set(v, 1, (intptr_t)'b');
	diag_vector_set(v, 2, (intptr_t)'c');

	w = diag_vector_copy(v);
	ASSERT_EQ_SIZE(3, diag_vector_length(w));
	ASSERT_EQ_CHAR('a', diag_vector_ref(w, 0));
	ASSERT_EQ_CHAR('b', diag_vector_ref(w, 1));
	ASSERT_EQ_CHAR('c', diag_vector_ref(w, 2));
	diag_vector_destroy(w);

	diag_vector_destroy(v);
	return EXIT_SUCCESS;
}
