/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/pair.h"
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
	if (!list) diag_error("list is null");
	while (i-- > 0) {
		list = (const struct diag_pair *)list->cdr;
		if (!list) diag_error("exceed list length");
	}
	return list->car;
}

struct diag_pair *diag_list_reverse(struct diag_pair *list)
{
	struct diag_pair *p = NULL, *tmp;

	while (list) {
		p = diag_pair_create(list->car, (intptr_t)p);
		tmp = (struct diag_pair *)list->cdr;
		diag_pair_destroy(list);
		list = tmp;
	}
	return p;
}
