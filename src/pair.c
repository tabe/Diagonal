/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/pair.h"

struct diag_pair *diag_pair_create(intptr_t car, intptr_t cdr)
{
	struct diag_pair *p;

	p = diag_malloc(sizeof(*p));
	p->car = car;
	p->cdr = cdr;
	return p;
}

void diag_pair_destroy(struct diag_pair *p)
{
	diag_free(p);
}
