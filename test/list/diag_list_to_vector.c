/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/pair.h"
#include "diagonal/vector.h"
#include "diagonal/list.h"

#define CONS(x, y) diag_pair_create((intptr_t)x, (intptr_t)y)

int main()
{
	struct diag_pair *list = NULL;
	struct diag_vector *v;

	ASSERT_FALSE(diag_list_reverse(list));
	list = CONS('c', list);
	list = CONS('b', list);
	list = CONS('a', list);
	v = diag_list_to_vector(list);
	ASSERT_EQ_SIZE(3, diag_vector_length(v));
	ASSERT_EQ_CHAR('a', diag_vector_ref(v, 0));
	ASSERT_EQ_CHAR('b', diag_vector_ref(v, 1));
	ASSERT_EQ_CHAR('c', diag_vector_ref(v, 2));
	diag_vector_destroy(v);
	return EXIT_SUCCESS;
}
