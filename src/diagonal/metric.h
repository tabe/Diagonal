/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_METRIC_H
#define DIAGONAL_METRIC_H

typedef unsigned int diag_distance_t;
typedef signed int diag_sdistance_t;

typedef diag_distance_t (*diag_metric_t)(const struct diag_datum *d1, const struct diag_datum *d2);
typedef diag_distance_t (*diag_metric_chars_t)(const char *x, const char *y);
typedef diag_sdistance_t (*diag_emetric_chars_t)(const char *x, const char *y, diag_distance_t e);

DIAG_C_DECL_BEGIN

DIAG_FUNCTION diag_distance_t diag_hamming_chars(const char *x, const char *y);
DIAG_FUNCTION diag_sdistance_t diag_ehamming_chars(const char *x, const char *y, diag_distance_t e);

DIAG_FUNCTION diag_distance_t diag_levenshtein_chars(const char *x, const char *y);
DIAG_FUNCTION diag_sdistance_t diag_elevenshtein_chars(const char *x, const char *y, diag_distance_t e);

DIAG_C_DECL_END

#endif
