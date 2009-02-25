#ifndef DIAGONAL_METRIC_H
#define DIAGONAL_METRIC_H

typedef unsigned int diag_distance_t;

typedef diag_distance_t (*diag_metric_t)(const diag_datum_t *d1, const diag_datum_t *d2);
typedef diag_distance_t (*diag_metric_chars_t)(const char *x, const char *y);

extern diag_distance_t diag_hamming_chars(const char *x, const char *y);

extern diag_distance_t diag_levenshtein_chars(const char *x, const char *y);

#endif
