/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/pair.h"
#include "diagonal/list.h"

#define CONS(x, y) diag_pair_create((intptr_t)x, (intptr_t)y)

int main()
{
	struct diag_pair *list = NULL;

	ASSERT_EQ_SIZE(0, diag_list_length(list));
	list = CONS('a', list);
	ASSERT_EQ_SIZE(1, diag_list_length(list));
	list = CONS('b', list);
	ASSERT_EQ_SIZE(2, diag_list_length(list));
	list = CONS('c', list);
	ASSERT_EQ_SIZE(3, diag_list_length(list));
	diag_list_destroy(list);
	return EXIT_SUCCESS;
}
