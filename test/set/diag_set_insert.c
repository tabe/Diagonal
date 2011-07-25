/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/set.h"

int main(void)
{
	struct diag_set *set;
	const intptr_t *r;

	set = diag_set_create(NULL);
	ASSERT_EQ_SIZE(0, set->size);
	r = diag_set_insert(set, 2);
	ASSERT_FALSE(r);
	ASSERT_EQ_SIZE(1, set->size);
	r = diag_set_insert(set, 2);
	ASSERT_NOT_NULL(r);
	ASSERT_EQ_SIZE(1, set->size);
	r = diag_set_insert(set, 3);
	ASSERT_FALSE(r);
	ASSERT_EQ_SIZE(2, set->size);
	r = diag_set_insert(set, 5);
	ASSERT_FALSE(r);
	ASSERT_EQ_SIZE(3, set->size);
	r = diag_set_insert(set, 7);
	ASSERT_FALSE(r);
	ASSERT_EQ_SIZE(4, set->size);
	r = diag_set_insert(set, 11);
	ASSERT_FALSE(r);
	ASSERT_EQ_SIZE(5, set->size);
	r = diag_set_insert(set, 5);
	ASSERT_NOT_NULL(r);
	ASSERT_EQ_SIZE(5, set->size);
	diag_set_destroy(set);
	return EXIT_SUCCESS;
}
