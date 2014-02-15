/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/vector.h"

static void callback(size_t i, intptr_t e, void *data)
{
	char *s = (char *)data;
	s[i] = (char)e;
}

int main(void)
{
	struct diag_vector *v;

	v = diag_vector_create(3);
	diag_vector_set(v, 0, (intptr_t)'a');
	diag_vector_set(v, 1, (intptr_t)'b');
	diag_vector_set(v, 2, (intptr_t)'c');

	char buf[4];
	diag_vector_for_each(v, callback, buf);
	buf[3] = '\0';
	ASSERT_EQ_STRING("abc", buf);

	diag_vector_destroy(v);
	return EXIT_SUCCESS;
}
