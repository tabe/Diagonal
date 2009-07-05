#include "test.h"

#include "diagonal.h"
#include "diagonal/object.h"
#include "diagonal/vector.h"

int
main()
{
	diag_vector_t *ht;

	ht = diag_vector_new(0);
	assert(diag_vector_length(ht) == 0);
	diag_vector_destroy(ht);
	return EXIT_SUCCESS;
}
