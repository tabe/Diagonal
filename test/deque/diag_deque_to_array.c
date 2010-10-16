#include "test.h"

#include "diagonal.h"
#include "diagonal/deque.h"

#define LENGTH 10000

int
main()
{
	struct diag_deque *deque;
	struct diag_deque_elem *elem;
	int i;

	deque = diag_deque_new();

	for (i = 0; i < LENGTH; i++) {
		diag_deque_push(deque, (uintptr_t)i);
	}
	assert(deque->length == LENGTH);
	i = 0;
	DIAG_DEQUE_FOR_EACH(deque, elem) {
		assert(i == (int)elem->attr);
		i++;
	}
	assert(i == LENGTH);

	diag_deque_destroy(deque);

	return EXIT_SUCCESS;
}
