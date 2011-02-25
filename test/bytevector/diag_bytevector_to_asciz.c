/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/bytevector.h"

int
main(void)
{
	struct diag_bytevector *bv;
	char *s;

	bv = diag_bytevector_new_path(__FILE__);
	ASSERT_NOT_NULL(bv);
	s = diag_bytevector_to_asciz(bv);
	ASSERT_EQ_UINT(bv->size, strlen(s));
	s[8] = '\0';
	ASSERT_EQ_STRING("/* -*- M", s);
	diag_free(s);
	diag_bytevector_destroy(bv);
	return EXIT_SUCCESS;
}
