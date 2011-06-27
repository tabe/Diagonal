/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_DEQUE_H
#define DIAGONAL_DEQUE_H

struct diag_deque_elem {
	intptr_t attr;
	struct diag_deque_elem *prev;
	struct diag_deque_elem *next;
};

struct diag_deque {
	struct diag_deque_elem *first;
	struct diag_deque_elem *last;
	diag_size_t length;
};

#define DIAG_DEQUE_FOR_EACH(deque, elem) \
	for ((elem) = (deque)->first; (elem); (elem) = (elem)->next)

#define DIAG_DEQUE_TO_ARRAY(deque, type, array) do {			\
		diag_size_t i = 0;					\
		struct diag_deque_elem *elem;				\
		array = (type *)diag_calloc((size_t)(deque)->length,	\
					    sizeof(type));		\
		DIAG_DEQUE_FOR_EACH(deque, elem) {			\
			array[i++] = (type)elem->attr;			\
		}							\
	} while (0)

DIAG_C_DECL_BEGIN

/*
 * Return new empty deque.
 */
DIAG_FUNCTION struct diag_deque *diag_deque_new(void);
/*
 * Finalize and free `deque'.
 * Do nothing if `deque' is NULL.
 */
DIAG_FUNCTION void diag_deque_destroy(struct diag_deque *deque);
/*
 * Append `attr' to `deque'.
 */
DIAG_FUNCTION struct diag_deque_elem *diag_deque_push(struct diag_deque *deque,
					       intptr_t attr);
/*
 * Drop the last element from `deque' and return it.
 * It is safe to call this function even if `deque' is empty.
 * NULL may be returned in such case.
 */
DIAG_FUNCTION struct diag_deque_elem *diag_deque_pop(struct diag_deque *deque);
/*
 * Drop the first element from `deque' and return it.
 * It is safe to call this function even if `deque' is empty.
 * NULL may be returned in such case.
 */
DIAG_FUNCTION struct diag_deque_elem *diag_deque_shift(struct diag_deque *deque);
/*
 * Prepend `attr' to `deque'.
 */
DIAG_FUNCTION struct diag_deque_elem *diag_deque_unshift(struct diag_deque *deque,
						  intptr_t attr);
/*
 * Join `head' and `tail' destructively.
 * Return the length of the resulting `head'.
 */
DIAG_FUNCTION diag_size_t diag_deque_append(struct diag_deque *head,
				     struct diag_deque *tail);

DIAG_C_DECL_END

#endif
