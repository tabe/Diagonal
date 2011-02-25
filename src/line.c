/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/line.h"

struct diag_line_context *
diag_line_context_new(struct diag_port *port)
{
	struct diag_line_context *context;

	assert(port);
	context = diag_malloc(sizeof(struct diag_line_context));
	context->port = port;
	context->bufsize = DIAG_LINE_BUFSIZE;
	if (DIAG_PORT_FD_P(port)) {
		struct stat st;
		if (fstat(port->stream.fd, &st) == 0 && st.st_blksize > 0) {
			context->bufsize = (size_t)st.st_blksize;
		}
	}
	context->buf = diag_malloc(context->bufsize);
	context->head = context->sentinel = 0;
	context->error = 0;
	return context;
}

void
diag_line_context_destroy(struct diag_line_context *context)
{
	if (context) {
		if (context->buf) diag_free(context->buf);
		diag_free(context);
	}
}

struct diag_line_context *
diag_line_read(struct diag_line_context *context, size_t *sizep, char **linep)
{
	size_t h, i_pos, size = 0;
	int s;
	struct diag_port *port;
	char *line = NULL;

	assert(context && linep);
	port = context->port;
	for (;;) {
		if (context->head >= context->sentinel) {
			context->head = 0;
			i_pos = port->i_pos;
			s = port->read_bytes(port, context->bufsize, (uint8_t *)context->buf);
			if (s == 1) {
				context->sentinel = context->bufsize;
			} else if (s == 0) {
				context->sentinel = port->i_pos - i_pos;
				if (context->sentinel == 0) {
					if (line) {
						line[size] = '\0';
						*linep = line;
						if (sizep) *sizep = size;
					} else {
						context->error = DIAG_LINE_ERROR_EOF;
					}
					return context;
				}
			} else {
				if (line) diag_free(line);
				context->error = DIAG_LINE_ERROR_UNKNOWN;
				return context;
			}
		}

		assert(context->head < context->sentinel);
		h = context->head;
		while (h < context->sentinel) {
			if (context->buf[h] == '\n') {
				s = h - context->head;
				line = diag_realloc(line, size+s+1);
				memcpy(line+size, context->buf+context->head, s);
				line[size+s] = '\0';
				*linep = line;
				if (sizep) *sizep = size+s;
				context->head = h+1;
				return context;
			}
			h++;
		}

		s = context->sentinel-context->head;
		line = diag_realloc(line, size+s+1);
		memcpy(line+size, context->buf+context->head, s);
		size += s;
		context->head = context->sentinel = 0;
	}
}
