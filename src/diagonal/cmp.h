/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_CMP_H
#define DIAGONAL_CMP_H

typedef int (*diag_cmp_t)(intptr_t, intptr_t);

#define DIAG_CMP_IMMEDIATE ((diag_cmp_t)NULL)

#endif
