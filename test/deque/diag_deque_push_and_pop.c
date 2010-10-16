#include "test.h"

#include "diagonal.h"
#include "diagonal/deque.h"

int
main()
{
	struct diag_deque *deque;
	struct diag_deque_elem *elem;

	deque = diag_deque_new();
	assert(!deque->last);
	assert(deque->length == 0);

	elem = diag_deque_pop(deque);
	assert(!elem);
	assert(deque->length == 0);
	elem = diag_deque_push(deque, (uintptr_t)1);
	assert(elem);
	assert(deque->last == elem);
	assert(deque->length == 1);
	elem = diag_deque_push(deque, (uintptr_t)2);
	assert(elem);
	assert(deque->last == elem);
	assert(deque->length == 2);
	elem = diag_deque_pop(deque);
	assert(elem);
	assert((int)elem->attr == 2);
	assert(deque->length == 1);
	diag_free(elem);
	elem = diag_deque_pop(deque);
	assert(elem);
	assert((int)elem->attr == 1);
	assert(deque->length == 0);
	assert(!deque->last);
	diag_free(elem);

	diag_deque_destroy(deque);

	return EXIT_SUCCESS;
}
