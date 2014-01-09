/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/port.h"
#include "diagonal/private/system.h"

static const int BUFFER_LENGTH = 4096;

static void usage(void)
{
	diag_printf("diag-cycle command [operand ...]");
}

int main(int argc, char *argv[])
{
	int c;

	diag_init();

	while ( (c = getopt(argc, argv, "+Vh")) >=0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		default:
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}

	struct diag_port *op0 = diag_port_new_path("diagonal0.out", "wb");
	if (!op0) {
		exit(EXIT_FAILURE);
	}
	struct diag_port *ip = diag_port_new_fd(STDIN_FILENO, DIAG_PORT_INPUT);
	assert(ip);
	ssize_t s = diag_port_copy(ip, op0);
	diag_port_destroy(ip);
	diag_port_destroy(op0);
	if (s < 0) {
		exit(EXIT_FAILURE);
	}

	char *in = diag_strdup("diagonal0.out");
	struct diag_command *cmd;

 run:
	cmd = diag_command_new(argv+optind, NULL, in, NULL, NULL);
	if (!cmd) {
		exit(EXIT_FAILURE);
	}
	struct diag_process *p = diag_run_program(cmd);
	if (!p) {
		diag_command_destroy(cmd);
		exit(EXIT_FAILURE);
	}
	diag_process_wait(p);
	int status = p->status;
	diag_process_destroy(p);
	if (status != 0) {
		diag_command_destroy(cmd);
		exit(p->status);
	}

	struct diag_port *pp;
	struct diag_port *cp;
	pp = diag_port_new_path(in, "rb");
	if (!pp) {
		diag_command_destroy(cmd);
		exit(EXIT_FAILURE);
	}
	cp = diag_port_new_path(cmd->out, "rb");
	if (!cp) {
		diag_port_destroy(pp);
		diag_command_destroy(cmd);
		exit(EXIT_FAILURE);
	}
	int r = diag_port_diff(pp, cp);
	diag_port_destroy(cp);
	diag_port_destroy(pp);
	if (r == 1) {
		diag_free(in);
		in = diag_strdup(cmd->out);
		diag_command_destroy(cmd);
		goto run;
	}
	if (r == -1) {
		exit(EXIT_FAILURE);
	}

	cp = diag_port_new_path(in, "rb");
	if (!cp) {
		diag_free(in);
		exit(EXIT_FAILURE);
	}
	diag_free(in);
	struct diag_port *op = diag_port_new_fd(STDOUT_FILENO, DIAG_PORT_OUTPUT);
	assert(op);
	s = diag_port_copy(cp, op);
	diag_port_destroy(cp);
	diag_port_destroy(op);
	if (s < 0) {
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}
