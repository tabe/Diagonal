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
	ASSERT_EQ_SIZE(5, set->size);
	r = diag_set_erase(set, 1);
	ASSERT_EQ_INT(0, r);
	ASSERT_EQ_SIZE(5, set->size);
	r = diag_set_erase(set, 5);
	ASSERT_EQ_INT(1, r);
	ASSERT_EQ_SIZE(4, set->size);
	r = diag_set_erase(set, 5);
	ASSERT_EQ_INT(0, r);
	ASSERT_EQ_SIZE(4, set->size);
	r = diag_set_erase(set, 11);
	ASSERT_EQ_INT(1, r);
	ASSERT_EQ_SIZE(3, set->size);
	r = diag_set_erase(set, 7);
	ASSERT_EQ_INT(1, r);
	ASSERT_EQ_SIZE(2, set->size);
	r = diag_set_erase(set, 3);
	ASSERT_EQ_INT(1, r);
	ASSERT_EQ_SIZE(1, set->size);
	r = diag_set_erase(set, 2);
	ASSERT_EQ_INT(1, r);
	ASSERT_EQ_SIZE(0, set->size);
	diag_set_destroy(set);
	return EXIT_SUCCESS;
}
