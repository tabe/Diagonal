#ifndef DIAGONAL_DEQUE_H
#define DIAGONAL_DEQUE_H

struct diag_deque_elem {
	uintptr_t attr;
	struct diag_deque_elem *prev;
	struct diag_deque_elem *next;
};

struct diag_deque {
	struct diag_deque_elem *first;
	struct diag_deque_elem *last;
	unsigned int length;
};

#define DIAG_DEQUE_FOR_EACH(deque, elem) for ((elem) = (deque)->first; (elem); (elem) = (elem)->next)

#define DIAG_DEQUE_TO_ARRAY(deque, type, array) do {					\
		unsigned int i = 0;												\
		struct diag_deque_elem *elem;										\
		array = (type *)diag_calloc((size_t)(deque)->length, sizeof(type)); \
		DIAG_DEQUE_FOR_EACH(deque, elem) {								\
			array[i++] = (type)elem->attr;								\
		}																\
	} while (0)

DIAG_C_DECL_BEGIN

extern struct diag_deque *diag_deque_new(void);
extern void diag_deque_destroy(struct diag_deque *deque);

extern struct diag_deque_elem *diag_deque_push(struct diag_deque *deque, uintptr_t attr);
extern struct diag_deque_elem *diag_deque_pop(struct diag_deque *deque);

extern struct diag_deque_elem *diag_deque_shift(struct diag_deque *deque);
extern struct diag_deque_elem *diag_deque_unshift(struct diag_deque *deque, uintptr_t attr);

extern unsigned int diag_deque_append(struct diag_deque *head, struct diag_deque *tail);

DIAG_C_DECL_END

#endif
