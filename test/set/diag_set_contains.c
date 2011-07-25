/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/set.h"

int main(void)
{
	struct diag_set *set;
	int r;

	set = diag_set_create(NULL);
	diag_set_insert(set, 2);
	diag_set_insert(set, 3);
	diag_set_insert(set, 5);
	diag_set_insert(set, 7);
	diag_set_insert(set, 11);
	r = diag_set_contains(set, 1);
	ASSERT_EQ_INT(0, r);
	r = diag_set_contains(set, 2);
	ASSERT_EQ_INT(1, r);
	r = diag_set_contains(set, 3);
	ASSERT_EQ_INT(1, r);
	r = diag_set_contains(set, 4);
	ASSERT_EQ_INT(0, r);
	r = diag_set_contains(set, 5);
	ASSERT_EQ_INT(1, r);
	r = diag_set_contains(set, 6);
	ASSERT_EQ_INT(0, r);
	r = diag_set_contains(set, 7);
	ASSERT_EQ_INT(1, r);
	r = diag_set_contains(set, 8);
	ASSERT_EQ_INT(0, r);
	r = diag_set_contains(set, 9);
	ASSERT_EQ_INT(0, r);
	r = diag_set_contains(set, 10);
	ASSERT_EQ_INT(0, r);
	r = diag_set_contains(set, 11);
	ASSERT_EQ_INT(1, r);

	r = diag_set_erase(set, 11);
	r = diag_set_contains(set, 11);
	ASSERT_EQ_INT(0, r);

	r = diag_set_erase(set, 5);
	r = diag_set_contains(set, 5);
	ASSERT_EQ_INT(0, r);

	r = diag_set_erase(set, 2);
	r = diag_set_contains(set, 2);
	ASSERT_EQ_INT(0, r);

	diag_set_destroy(set);
	return EXIT_SUCCESS;
}
