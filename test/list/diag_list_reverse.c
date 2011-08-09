/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/pair.h"
#include "diagonal/list.h"

#define CONS(x, y) diag_pair_create((intptr_t)x, (intptr_t)y)

int main()
{
	struct diag_pair *list = NULL;

	ASSERT_FALSE(diag_list_reverse(list));
	list = CONS('a', list);
	list = CONS('b', list);
	list = CONS('c', list);
	list = diag_list_reverse(list);
	ASSERT_EQ_SIZE(3, diag_list_length(list));
	ASSERT_EQ_CHAR('a', diag_list_ref(list, 0));
	ASSERT_EQ_CHAR('b', diag_list_ref(list, 1));
	ASSERT_EQ_CHAR('c', diag_list_ref(list, 2));
	diag_list_destroy(list);
	return EXIT_SUCCESS;
}
