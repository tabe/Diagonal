/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/deque.h"

int
main()
{
	struct diag_deque *head, *tail;
	struct diag_deque_elem *elem;
	uintptr_t i;
	unsigned int len;

	head = diag_deque_new();
	tail = diag_deque_new();

	/* both are empty */
	len = diag_deque_append(head, tail);
	ASSERT_EQ_UINT(0, len);
	ASSERT_EQ_UINT(0, head->length);
	ASSERT_EQ_UINT(0, tail->length);

	/* head is trivial */
	i = 0;
	while (i < 29) {
		diag_deque_push(tail, i++);
	}
	len = diag_deque_append(head, tail);
	ASSERT_EQ_UINT(29, head->length);
	ASSERT_EQ_UINT(0, tail->length);
	i = 0;
	while ( (elem = diag_deque_shift(head)) ) {
		ASSERT_EQ_UINTPTR(i++, elem->attr);
	}
	ASSERT_EQ_UINTPTR(29, i);

	/* tail is trivial */
	i = 0;
	while (i < 31) {
		diag_deque_push(head, i++);
	}
	len = diag_deque_append(head, tail);
	ASSERT_EQ_UINT(31, head->length);
	ASSERT_EQ_UINT(0, tail->length);
	i = 0;
	while ( (elem = diag_deque_shift(head)) ) {
		ASSERT_EQ_UINTPTR(i++, elem->attr);
	}
	ASSERT_EQ_UINTPTR(31, i);

	/* general */
	i = 0;
	while (i < 100) {
		diag_deque_push(head, i++);
	}
	while (i < 300) {
		diag_deque_push(tail, i++);
	}
	len = diag_deque_append(head, tail);
	ASSERT_EQ_UINT(300, len);
	ASSERT_EQ_UINT(300, head->length);
	ASSERT_EQ_UINT(0, tail->length);
	i = 0;
	while ( (elem = diag_deque_shift(head)) ) {
		ASSERT_EQ_UINTPTR(i++, elem->attr);
	}
	ASSERT_EQ_UINTPTR(300, i);

	diag_deque_destroy(head);
	diag_deque_destroy(tail);
	return EXIT_SUCCESS;
}
