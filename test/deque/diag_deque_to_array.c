/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/deque.h"

#define LENGTH 10000

int main(void)
{
	struct diag_deque *deque;
	struct diag_deque_elem *elem;
	int i;

	deque = diag_deque_new();

	for (i = 0; i < LENGTH; i++) {
		diag_deque_push(deque, (intptr_t)i);
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
