/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_LINE_H
#define DIAGONAL_LINE_H

#define DIAG_LINE_BUFSIZE 8192

enum diag_line_error_e {
	DIAG_LINE_ERROR_EOF = 1,
	DIAG_LINE_ERROR_UNKNOWN,
};

struct diag_line_context {
	struct diag_port *port;
	char *buf;
	size_t bufsize;
	size_t head;
	size_t sentinel;
	enum diag_line_error_e error;
};

#define DIAG_LINE_HAS_ERROR(context) ((context)->error > 0)

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_line_context *diag_line_context_new(struct diag_port *port);
DIAG_FUNCTION void diag_line_context_destroy(struct diag_line_context *context);

DIAG_FUNCTION struct diag_line_context *diag_line_read(struct diag_line_context *context, size_t *sizep, char **linep);

DIAG_C_DECL_END

#endif
