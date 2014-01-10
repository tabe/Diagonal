/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_PRIVATE_SYSTEM_H
#define DIAGONAL_PRIVATE_SYSTEM_H

struct diag_command {
	const char *file;
	char **argv;
	char *dir;
	char *in;
	char *out;
	char *err;
};

struct diag_process {
	intptr_t pid;
	int status;
};

DIAG_C_DECL_BEGIN

/*
 * `argv' must be NULL-terminated.
 */
DIAG_FUNCTION struct diag_command *diag_command_new(char **argv,
						    const char *dir,
						    const char *in,
						    const char *out,
						    const char *err);

DIAG_FUNCTION void diag_command_destroy(struct diag_command *);

DIAG_FUNCTION char *diag_get_command_line(char **argv);

DIAG_FUNCTION struct diag_process *diag_run_program(struct diag_command *);

DIAG_FUNCTION void diag_process_wait(struct diag_process *);

DIAG_FUNCTION void diag_process_destroy(struct diag_process *);

DIAG_C_DECL_END

#endif
