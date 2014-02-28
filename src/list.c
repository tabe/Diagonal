/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/pair.h"
#include "diagonal/vector.h"
#include "diagonal/list.h"

/* API */

void diag_list_destroy(struct diag_pair *list)
{
	struct diag_pair *p;

	while (list) {
		p = (struct diag_pair *)list->cdr;
		diag_pair_destroy(list);
		list = p;
	}
}

size_t diag_list_length(const struct diag_pair *list)
{
	size_t s;

	for (s = 0; list; s++) {
		list = (const struct diag_pair *)list->cdr;
	}
	return s;
}

intptr_t diag_list_ref(const struct diag_pair *list, size_t i)
{
	if (!list) diag_fatal("list is null");
	while (i-- > 0) {
		list = (const struct diag_pair *)list->cdr;
		if (!list) diag_fatal("exceed list length");
	}
	return list->car;
}

struct diag_pair *diag_list_reverse(const struct diag_pair *list)
{
	struct diag_pair *p = NULL;

	while (list) {
		p = diag_pair_create(list->car, (intptr_t)p);
		list = (struct diag_pair *)list->cdr;
	}
	return p;
}

struct diag_vector *diag_list_to_vector(const struct diag_pair *list)
{
	struct diag_vector *v;
	size_t length, i;

	length = diag_list_length(list);
	v = diag_vector_create(length);
	for (i = 0; list; i++) {
		diag_vector_set(v, i, list->car);
		list = (struct diag_pair *)list->cdr;
	}
	return v;
}
