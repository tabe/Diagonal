#include "test.h"

#include "diagonal.h"
#include "diagonal/bytevector.h"

#define SIZE 10

int
main(void)
{
	diag_bytevector_t *bv;
	uint8_t *data;

	data = (uint8_t *)diag_malloc(SIZE);
	bv = diag_bytevector_new_heap(SIZE, data);
	ASSERT_NOT_NULL(bv);
	ASSERT_EQ_UINT(SIZE, bv->size);
	ASSERT_TRUE(data == bv->data);
	diag_bytevector_destroy(bv);
	return EXIT_SUCCESS;
}
