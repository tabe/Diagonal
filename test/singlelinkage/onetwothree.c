/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/dataset.h"
#include "diagonal/datum.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/rbtree.h"
#include "diagonal/singlelinkage.h"

static intptr_t f(intptr_t a, intptr_t b)
{
	const struct diag_datum *x, *y;
	x = (const struct diag_datum *)a;
	y = (const struct diag_datum *)b;
	return (intptr_t)(x->value - y->value);
}

static struct diag_datum *at(size_t i, struct diag_dataset *ds)
{
	struct diag_datum *arr = (struct diag_datum *)ds->attic;
	struct diag_datum *d = diag_malloc(sizeof(*d));
	memcpy(d, arr + i, sizeof(*d));
	return d;
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

	ds = diag_dataset_create(at, (intptr_t)d);

	ds->size = 1;
	sl = diag_singlelinkage_create(ds, f, DIAG_CMP_IMMEDIATE);
	r = diag_singlelinkage_analyze(sl);
	ASSERT_EQ_INT(-1, r);
	diag_singlelinkage_destroy(sl);

	ds->size = 2;
	sl = diag_singlelinkage_create(ds, f, DIAG_CMP_IMMEDIATE);
	r = diag_singlelinkage_analyze(sl);
	ASSERT_EQ_INT(0, r);
	ASSERT_EQ_SIZE(2, sl->t->length);
	diag_singlelinkage_destroy(sl);

	ds->size = 3;
	sl = diag_singlelinkage_create(ds, f, DIAG_CMP_IMMEDIATE);
	r = diag_singlelinkage_analyze(sl);
	ASSERT_EQ_INT(0, r);
	ASSERT_EQ_SIZE(4, sl->t->length);
	diag_singlelinkage_destroy(sl);

	diag_dataset_destroy(ds);

	return EXIT_SUCCESS;
}
