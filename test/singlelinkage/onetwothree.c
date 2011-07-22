/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/dataset.h"
#include "diagonal/datum.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/rbtree.h"
#include "diagonal/singlelinkage.h"

static uintptr_t f(intptr_t a, intptr_t b)
{
	const struct diag_datum *x, *y;
	x = (const struct diag_datum *)a;
	y = (const struct diag_datum *)b;
	return (intptr_t)x->value - (intptr_t)y->value;
}

static struct diag_datum d[] = {
	{0, (uintptr_t)'a', (intptr_t)1},
	{0, (uintptr_t)'b', (intptr_t)2},
	{0, (uintptr_t)'c', (intptr_t)3}
};

int main(void)
{
	struct diag_dataset *ds;
	struct diag_singlelinkage *sl;
	int r;

	ds = diag_dataset_create(1);
	ds->data[0] = &d[0];
	sl = diag_singlelinkage_create(ds, f);
	r = diag_singlelinkage_analyze(sl);
	ASSERT_EQ_INT(-1, r);
	diag_singlelinkage_destroy(sl);
	diag_dataset_destroy(ds);

	ds = diag_dataset_create(2);
	ds->data[0] = &d[0];
	ds->data[1] = &d[1];
	sl = diag_singlelinkage_create(ds, f);
	r = diag_singlelinkage_analyze(sl);
	ASSERT_EQ_INT(0, r);
	ASSERT_EQ_SIZE(2, sl->t->length);
	diag_singlelinkage_destroy(sl);
	diag_dataset_destroy(ds);

	ds = diag_dataset_create(3);
	ds->data[0] = &d[0];
	ds->data[1] = &d[1];
	ds->data[2] = &d[2];
	sl = diag_singlelinkage_create(ds, f);
	r = diag_singlelinkage_analyze(sl);
	ASSERT_EQ_INT(0, r);
	ASSERT_EQ_SIZE(4, sl->t->length);
	diag_singlelinkage_destroy(sl);
	diag_dataset_destroy(ds);

	return EXIT_SUCCESS;
}
