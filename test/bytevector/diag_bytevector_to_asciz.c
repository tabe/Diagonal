#include "test.h"

#include "diagonal.h"
#include "diagonal/bytevector.h"

int
main(void)
{
	diag_bytevector_t *bv;
	char *s;

	bv = diag_bytevector_new_path(__FILE__);
	ASSERT_NOT_NULL(bv);
	s = diag_bytevector_to_asciz(bv);
	ASSERT_EQ_UINT(bv->size, strlen(s));
	s[8] = '\0';
	ASSERT_EQ_STRING("#include", s);
	diag_bytevector_destroy(bv);
	return EXIT_SUCCESS;
}
