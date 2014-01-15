/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_PRIVATE_TEMPORARY_FILE_H
#define DIAGONAL_PRIVATE_TEMPORARY_FILE_H

struct diag_temporary_file {
	char *path;
	struct diag_port *port;
	intptr_t f;
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_temporary_file *diag_temporary_file_new(void);

DIAG_FUNCTION void diag_temporary_file_destroy(struct diag_temporary_file *);

DIAG_C_DECL_END

#endif
