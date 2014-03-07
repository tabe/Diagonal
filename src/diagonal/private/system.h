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

/*
 * Run a command specified with `argv' as an agent.
 */
DIAG_FUNCTION intptr_t diag_run_agent(char **argv);

/*
 * Wait until one of `n' numbers of `agents' exits.
 * Return the index of the exiting agent in case of success.
 * If `code' is non-NULL, fill it with the exit status.
 * Return -1 if an error occurs.
 */
DIAG_FUNCTION int diag_wait_agent(int n, const intptr_t *agents, int *code);

/*
 * Suspend the calling thread until `interval' seconds have elapsed.
 * Return immediately if `interval` is non-positive.
 */
DIAG_FUNCTION void diag_sleep(int interval);

DIAG_C_DECL_END

#endif
