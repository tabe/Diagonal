/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_METRIC_H
#define DIAGONAL_METRIC_H

typedef uintptr_t (*diag_metric_t)(intptr_t, intptr_t);
typedef intptr_t (*diag_emetric_t)(intptr_t, intptr_t, uintptr_t);

DIAG_C_DECL_BEGIN

DIAG_FUNCTION uintptr_t diag_hamming_chars(intptr_t, intptr_t);
DIAG_FUNCTION intptr_t diag_ehamming_chars(intptr_t, intptr_t, uintptr_t);

DIAG_FUNCTION uintptr_t diag_levenshtein_chars(intptr_t, intptr_t);
DIAG_FUNCTION intptr_t diag_elevenshtein_chars(intptr_t, intptr_t, uintptr_t);

DIAG_C_DECL_END

#endif
