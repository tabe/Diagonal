/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_METRIC_H
#define DIAGONAL_METRIC_H

#include <diagonal/datum.h>

typedef uintptr_t (*diag_metric_t)(const struct diag_datum *,
				   const struct diag_datum *);
typedef uintptr_t (*diag_metric_chars_t)(const char *x, const char *y);
typedef intptr_t (*diag_emetric_chars_t)(const char *x, const char *y,
					 uintptr_t e);

DIAG_C_DECL_BEGIN

DIAG_FUNCTION uintptr_t diag_hamming_chars(const char *x, const char *y);
DIAG_FUNCTION intptr_t diag_ehamming_chars(const char *x, const char *y, uintptr_t e);

DIAG_FUNCTION uintptr_t diag_levenshtein_chars(const char *x, const char *y);
DIAG_FUNCTION intptr_t diag_elevenshtein_chars(const char *x, const char *y, uintptr_t e);

DIAG_C_DECL_END

#endif
