/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_IMF_H
#define DIAGONAL_IMF_H

#define DIAG_IMF_LINE_LIMIT 998
#define DIAG_IMF_LINE_CLIMIT 78

struct diag_imf_header_field {
	char *name;
	char *body;
};

struct diag_imf {
	struct diag_imf_header_field **header_fields;
	char *body;
};

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

DIAG_C_DECL_BEGIN

typedef diag_distance_t (*diag_metric_imf_t)(const struct diag_imf *x, const struct diag_imf *y);

extern void diag_imf_raise_error(enum diag_imf_error e);

extern int diag_imf_parse(char *s, struct diag_imf **imfp, unsigned int option);

extern void diag_imf_destroy(struct diag_imf *imf);

extern diag_distance_t diag_hamming_imf(const struct diag_imf *x, const struct diag_imf *y);

DIAG_C_DECL_END

#endif
