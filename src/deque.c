#include "config.h"

#include <assert.h>
#include <stdint.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/deque.h"

diag_deque_t *
diag_deque_new(void)
{
	diag_deque_t *deque;

	deque = (diag_deque_t *)diag_malloc(sizeof(diag_deque_t));
	deque->first = NULL;
	deque->last = NULL;
	deque->length = 0;
	return deque;
}

void
diag_deque_destroy(diag_deque_t *deque)
{
	if (deque) {
		diag_deque_elem_t *elem = deque->first;
		while (elem) {
			diag_deque_elem_t *tmp = elem;
			elem = tmp->next;
			diag_free(tmp);
		}
		diag_free(deque);
	}
}

#define DIAG_DEQUE_ADD(end1, end2, link1, link2) do {					\
		diag_deque_elem_t *tmp;											\
																		\
		assert(deque);													\
		tmp = deque->end1;												\
		elem = (diag_deque_elem_t *)diag_malloc(sizeof(diag_deque_elem_t));	\
		elem->attr = attr;												\
		elem->link1 = NULL;												\
		elem->link2 = tmp;												\
		if (deque->length == 0) {										\
			deque->end2 = elem;											\
		} else {														\
			assert(tmp);												\
			tmp->link1 = elem;											\
		}																\
		deque->end1 = elem;												\
		deque->length++;												\
	} while (0)

#define DIAG_DEQUE_REMOVE(end1, end2, link1, link2) do {	\
		diag_deque_elem_t *tmp;								\
															\
		assert(deque);										\
		elem = deque->end1;									\
		switch (deque->length) {							\
		case 0:												\
			assert(!elem);									\
			break;											\
		case 1:												\
			assert(elem && elem == deque->end2);			\
			deque->end1 = deque->end2 = NULL;				\
			deque->length--;								\
			break;											\
		default:											\
			assert(elem && elem->link2);					\
			tmp = elem->link2;								\
			tmp->link1 = NULL;								\
			deque->end1 = tmp;								\
			deque->length--;								\
			break;											\
		}													\
	} while (0)

diag_deque_elem_t *
diag_deque_shift(diag_deque_t *deque)
{
	diag_deque_elem_t *elem;

	DIAG_DEQUE_REMOVE(first, last, prev, next);
	return elem;
}

diag_deque_elem_t *
diag_deque_unshift(diag_deque_t *deque, uintptr_t attr)
{
	diag_deque_elem_t *elem;

	DIAG_DEQUE_ADD(first, last, prev, next);
	return elem;
}

diag_deque_elem_t *
diag_deque_push(diag_deque_t *deque, uintptr_t attr)
{
	diag_deque_elem_t *elem;

	DIAG_DEQUE_ADD(last, first, next, prev);
	return elem;
}

diag_deque_elem_t *
diag_deque_pop(diag_deque_t *deque)
{
	diag_deque_elem_t *elem;
	
	DIAG_DEQUE_REMOVE(last, first, next, prev);
	return elem;
}
