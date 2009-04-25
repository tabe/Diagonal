#ifndef DIAGONAL_LINE_H
#define DIAGONAL_LINE_H

#include <diagonal/port.h>

#define DIAG_LINE_BUFSIZE 8192

enum diag_line_error_e {
	DIAG_LINE_ERROR_EOF = 1,
	DIAG_LINE_ERROR_UNKNOWN,
};

typedef struct diag_line_context_s {
	diag_port_t *port;
	char *buf;
	size_t bufsize;
	size_t head;
	size_t sentinel;
	enum diag_line_error_e error;
} diag_line_context_t;

#define DIAG_LINE_HAS_ERROR(context) ((context)->error > 0)

DIAG_C_DECL_BEGIN

extern diag_line_context_t *diag_line_context_new(diag_port_t *port);
extern void diag_line_context_destroy(diag_line_context_t *context);

extern diag_line_context_t *diag_line_read(diag_line_context_t *context, size_t *sizep, char **linep);

DIAG_C_DECL_END

#endif
