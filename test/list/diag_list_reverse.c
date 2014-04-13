/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/pair.h"
#include "diagonal/list.h"

#define CONS(x, y) diag_pair_create((intptr_t)x, (intptr_t)y)

int main(void)
{
	struct diag_pair *list = NULL, *r;

	ASSERT_FALSE(diag_list_reverse(list));
	list = CONS('a', list);
	list = CONS('b', list);
	list = CONS('c', list);
	r = diag_list_reverse(list);
	diag_list_destroy(list);
	ASSERT_EQ_SIZE(3, diag_list_length(r));
	ASSERT_EQ_CHAR('a', diag_list_ref(r, 0));
	ASSERT_EQ_CHAR('b', diag_list_ref(r, 1));
	ASSERT_EQ_CHAR('c', diag_list_ref(r, 2));
	diag_list_destroy(r);
	return EXIT_SUCCESS;
}
