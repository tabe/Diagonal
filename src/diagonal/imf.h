#ifndef DIAGONAL_IMF_H
#define DIAGONAL_IMF_H

#define DIAG_IMF_LINE_LIMIT 998
#define DIAG_IMF_LINE_CLIMIT 78

typedef struct {
	char *name;
	char *body;
} diag_imf_header_field_t;

typedef struct {
	diag_imf_header_field_t **header_fields;
	char *body;
} diag_imf_t;

enum diag_imf_error {
	DIAG_IMF_ERROR_INPUT,
	DIAG_IMF_ERROR_OUTPUT,
	DIAG_IMF_ERROR_UNEXPECTED_EOF,
	DIAG_IMF_ERROR_UNEXPECTED_WS,
	DIAG_IMF_ERROR_MISSING_CR,
	DIAG_IMF_ERROR_MISSING_LF,
	DIAG_IMF_ERROR_INVALID_HEADER_FIELD_NAME,
	DIAG_IMF_ERROR_MAX
};

enum diag_imf_option {
	DIAG_IMF_LF = 1,
};

typedef diag_distance_t (*diag_metric_imf_t)(const diag_imf_t *x, const diag_imf_t *y);

extern void diag_imf_raise_error(enum diag_imf_error e);

extern int diag_imf_parse(char *s, diag_imf_t **imf, unsigned int option);

extern void diag_imf_destroy(diag_imf_t *imf);

extern diag_distance_t diag_hamming_imf(const diag_imf_t *x, const diag_imf_t *y);

#endif
