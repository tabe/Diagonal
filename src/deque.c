/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/deque.h"

struct diag_deque *
diag_deque_new(void)
{
	struct diag_deque *deque;

	deque = diag_malloc(sizeof(struct diag_deque));
	deque->first = NULL;
	deque->last = NULL;
	deque->length = 0;
	return deque;
}

void
diag_deque_destroy(struct diag_deque *deque)
{
	if (deque) {
		struct diag_deque_elem *elem = deque->first;
		while (elem) {
			struct diag_deque_elem *tmp = elem;
			elem = tmp->next;
			diag_free(tmp);
		}
		diag_free(deque);
	}
}

#define DIAG_DEQUE_ADD(end1, end2, link1, link2) do {			\
		struct diag_deque_elem *tmp;				\
									\
		assert(deque);						\
		tmp = deque->end1;					\
		elem = diag_malloc(sizeof(struct diag_deque_elem));	\
		elem->attr = attr;					\
		elem->link1 = NULL;					\
		elem->link2 = tmp;					\
		if (deque->length == 0) {				\
			deque->end2 = elem;				\
		} else {						\
			assert(tmp);					\
			tmp->link1 = elem;				\
		}							\
		deque->end1 = elem;					\
		deque->length++;					\
	} while (0)

#define DIAG_DEQUE_REMOVE(end1, end2, link1, link2) do {	\
		struct diag_deque_elem *tmp;			\
								\
		assert(deque);					\
		elem = deque->end1;				\
		switch (deque->length) {			\
		case 0:						\
			assert(!elem);				\
			break;					\
		case 1:						\
			assert(elem && elem == deque->end2);	\
			deque->end1 = deque->end2 = NULL;	\
			deque->length--;			\
			break;					\
		default:					\
			assert(elem && elem->link2);		\
			tmp = elem->link2;			\
			tmp->link1 = NULL;			\
			deque->end1 = tmp;			\
			deque->length--;			\
			break;					\
		}						\
	} while (0)

struct diag_deque_elem *
diag_deque_shift(struct diag_deque *deque)
{
	struct diag_deque_elem *elem;

	DIAG_DEQUE_REMOVE(first, last, prev, next);
	return elem;
}

struct diag_deque_elem *
diag_deque_unshift(struct diag_deque *deque, uintptr_t attr)
{
	struct diag_deque_elem *elem;

	DIAG_DEQUE_ADD(first, last, prev, next);
	return elem;
}

struct diag_deque_elem *
diag_deque_push(struct diag_deque *deque, uintptr_t attr)
{
	struct diag_deque_elem *elem;

	DIAG_DEQUE_ADD(last, first, next, prev);
	return elem;
}

struct diag_deque_elem *
diag_deque_pop(struct diag_deque *deque)
{
	struct diag_deque_elem *elem;

	DIAG_DEQUE_REMOVE(last, first, next, prev);
	return elem;
}

unsigned int
diag_deque_append(struct diag_deque *head, struct diag_deque *tail)
{
	unsigned int len;
	struct diag_deque_elem *x, *y;

	assert(head && tail);
	if (tail->length == 0) return head->length;
	if (head->length == 0) {
		head->first  = tail->first;
		head->last   = tail->last;
		head->length = tail->length;
	} else {
		len = head->length + tail->length;
		x = head->last;
		y = tail->first;
		assert(x && y);
		x->next = y;
		y->prev = x;
		head->length = len;
		head->last = tail->last;
	}
	tail->first = NULL;
	tail->last = NULL;
	tail->length = 0;
	return head->length;
}
