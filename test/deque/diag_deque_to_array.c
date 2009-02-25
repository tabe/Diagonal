#include "test.h"

#include "diagonal.h"
#include "diagonal/deque.h"

#define LENGTH 10000

int
main()
{
	diag_deque_t *deque;
	diag_deque_elem_t *elem;
	int i;

	deque = diag_deque_new();

	for (i = 0; i < LENGTH; i++) {
		diag_deque_push(deque, (void *)i);
	}
	assert(deque->length == LENGTH);
	i = 0;
	DIAG_DEQUE_FOR_EACH(deque, elem) {
		assert(i == (int)elem->attr);
		i++;
	}
	assert(i == LENGTH);

	diag_deque_destroy(deque);

	return 0;
}
