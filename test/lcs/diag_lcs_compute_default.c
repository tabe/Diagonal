/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/pair.h"
#include "diagonal/list.h"
#include "diagonal/vector.h"
#include "diagonal/lcs.h"

#define DIAG_VECTOR_SET(v, i, e) diag_vector_set(v, i, (intptr_t)e)

int main()
{
	struct diag_vector *vx, *vy, *ses;
	struct diag_lcs *lcs;

	/* "" vs. "" */
	vx = diag_vector_create(0);
	vy = diag_vector_create(0);
	lcs = diag_lcs_create(vx, vy, NULL);
	ASSERT_EQ_INT(0, diag_lcs_compute(lcs, &ses));
	ASSERT_EQ_SIZE(0, diag_vector_length(ses));
	diag_vector_destroy(ses);
	diag_lcs_destroy(lcs);
	diag_vector_destroy(vy);
	diag_vector_destroy(vx);

	/* "a" vs. "" */
	vx = diag_vector_create(1);
	DIAG_VECTOR_SET(vx, 0, 'a');
	vy = diag_vector_create(0);
	lcs = diag_lcs_create(vx, vy, NULL);
	ASSERT_EQ_INT(1, diag_lcs_compute(lcs, &ses));
	ASSERT_EQ_SIZE(1, diag_vector_length(ses));
	ASSERT_EQ_INT(-1, diag_vector_ref(ses, 0));
	diag_vector_destroy(ses);
	diag_lcs_destroy(lcs);
	diag_vector_destroy(vy);
	diag_vector_destroy(vx);

	/* "" vs. "a" */
	vx = diag_vector_create(0);
	vy = diag_vector_create(1);
	DIAG_VECTOR_SET(vy, 0, 'a');
	lcs = diag_lcs_create(vx, vy, NULL);
	ASSERT_EQ_INT(1, diag_lcs_compute(lcs, &ses));
	ASSERT_EQ_SIZE(1, diag_vector_length(ses));
	ASSERT_EQ_INT(1, diag_vector_ref(ses, 0));
	diag_vector_destroy(ses);
	diag_lcs_destroy(lcs);
	diag_vector_destroy(vy);
	diag_vector_destroy(vx);

	/* "abde2f" vs. "0ab1cfg" */
	vx = diag_vector_create(6);
	DIAG_VECTOR_SET(vx, 0, 'a');
	DIAG_VECTOR_SET(vx, 1, 'b');
	DIAG_VECTOR_SET(vx, 2, 'd');
	DIAG_VECTOR_SET(vx, 3, 'e');
	DIAG_VECTOR_SET(vx, 4, '2');
	DIAG_VECTOR_SET(vx, 5, 'f');
	vy = diag_vector_create(7);
	DIAG_VECTOR_SET(vy, 0, '0');
	DIAG_VECTOR_SET(vy, 1, 'a');
	DIAG_VECTOR_SET(vy, 2, 'b');
	DIAG_VECTOR_SET(vy, 3, '1');
	DIAG_VECTOR_SET(vy, 4, 'c');
	DIAG_VECTOR_SET(vy, 5, 'f');
	DIAG_VECTOR_SET(vy, 6, 'g');
	lcs = diag_lcs_create(vx, vy, NULL);
	ASSERT_EQ_INT(7, diag_lcs_compute(lcs, &ses));
	ASSERT_EQ_INT(1, diag_vector_ref(ses, 0));
	ASSERT_EQ_INT(0, diag_vector_ref(ses, 1));
	ASSERT_EQ_INT(0, diag_vector_ref(ses, 2));
	ASSERT_EQ_INT(1, diag_vector_ref(ses, 3));
	ASSERT_EQ_INT(1, diag_vector_ref(ses, 4));
	ASSERT_EQ_INT(-1, diag_vector_ref(ses, 5));
	ASSERT_EQ_INT(-1, diag_vector_ref(ses, 6));
	ASSERT_EQ_INT(-1, diag_vector_ref(ses, 7));
	ASSERT_EQ_INT(0, diag_vector_ref(ses, 8));
	ASSERT_EQ_INT(1, diag_vector_ref(ses, 9));
	diag_vector_destroy(ses);
	diag_lcs_destroy(lcs);
	diag_vector_destroy(vy);
	diag_vector_destroy(vx);
	return EXIT_SUCCESS;
}
