#ifndef DIAGONAL_DEQUE_H
#define DIAGONAL_DEQUE_H

typedef struct diag_deque_elem_s {
	uintptr_t attr;
	struct diag_deque_elem_s *prev;
	struct diag_deque_elem_s *next;
} diag_deque_elem_t;

typedef struct {
	diag_deque_elem_t *first;
	diag_deque_elem_t *last;
	unsigned int length;
} diag_deque_t;

#define DIAG_DEQUE_FOR_EACH(deque, elem) for ((elem) = (deque)->first; (elem); (elem) = (elem)->next)

#define DIAG_DEQUE_TO_ARRAY(deque, type, array) do {					\
		unsigned int i = 0;												\
		diag_deque_elem_t *elem;										\
		array = (type *)diag_calloc((size_t)(deque)->length, sizeof(type)); \
		DIAG_DEQUE_FOR_EACH(deque, elem) {								\
			array[i++] = (type)elem->attr;								\
		}																\
	} while (0)

DIAG_C_DECL_BEGIN

extern diag_deque_t *diag_deque_new(void);
extern void diag_deque_destroy(diag_deque_t *deque);

extern diag_deque_elem_t *diag_deque_push(diag_deque_t *deque, uintptr_t attr);
extern diag_deque_elem_t *diag_deque_pop(diag_deque_t *deque);

extern diag_deque_elem_t *diag_deque_shift(diag_deque_t *deque);
extern diag_deque_elem_t *diag_deque_unshift(diag_deque_t *deque, uintptr_t attr);

DIAG_C_DECL_END

#endif
