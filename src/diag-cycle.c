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
#include "diagonal/deque.h"
#include "diagonal/port.h"
#include "diagonal/private/filesystem.h"
#include "diagonal/private/system.h"

static void usage(void)
{
	diag_printf("diag-cycle [-i input] [-o output] command [operand ...]");
}

int main(int argc, char *argv[])
{
	int c;
	int r = EXIT_FAILURE;
	int leave_output = 0;
	char *in = NULL;
	char *dir = NULL;

	diag_init();

	while ( (c = getopt(argc, argv, "+Vhi:o:")) >=0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			in = diag_strdup(optarg);
			break;
		case 'o':
			leave_output = 1;
			dir = diag_strdup(optarg);
			diag_assert_directory(dir);
			break;
		case ':':
		case '?':
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}

	struct diag_deque *q = diag_deque_new();
	struct diag_deque_elem *e;

	if (!in) {
		struct diag_port *op0 = diag_port_new_path("diagonal0.out", "wb");
		if (!op0) {
			goto done;
		}
		struct diag_port *ip = diag_port_new_stdin();
		assert(ip);
		ssize_t s = diag_port_copy(ip, op0);
		diag_port_destroy(ip);
		diag_port_destroy(op0);
		if (s < 0) {
			goto done;
		}

		in = diag_strdup("diagonal0.out");
		diag_deque_push((intptr_t)diag_strdup("diagonal0.out"));
	}

	struct diag_command *cmd;

 run:
	cmd = diag_command_new(argv+optind, dir, in, NULL, NULL);
	if (!cmd) {
		goto done;
	}
	struct diag_process *p = diag_run_program(cmd);
	if (!p) {
		diag_command_destroy(cmd);
		goto done;
	}
	diag_process_wait(p);
	int status = p->status;
	diag_process_destroy(p);
	if (status != 0) {
		diag_command_destroy(cmd);
		r = status;
		goto done;
	}

	diag_deque_push(q, (intptr_t)diag_strdup(cmd->out));
	diag_deque_push(q, (intptr_t)diag_strdup(cmd->err));

	struct diag_port *pp;
	struct diag_port *cp;
	pp = diag_port_new_path(in, "rb");
	if (!pp) {
		diag_command_destroy(cmd);
		goto done;
	}
	cp = diag_port_new_path(cmd->out, "rb");
	if (!cp) {
		diag_port_destroy(pp);
		diag_command_destroy(cmd);
		goto done;
	}
	int d = diag_port_diff(pp, cp);
	diag_port_destroy(cp);
	diag_port_destroy(pp);
	if (d == 1) {
		diag_free(in);
		in = diag_strdup(cmd->out);
		diag_command_destroy(cmd);
		goto run;
	}
	diag_command_destroy(cmd);
	if (d == -1) {
		goto done;
	}

	cp = diag_port_new_path(in, "rb");
	if (!cp) {
		goto done;
	}
	struct diag_port *op = diag_port_new_stdout();
	assert(op);
	ssize_t s = diag_port_copy(cp, op);
	diag_port_destroy(cp);
	diag_port_destroy(op);
	if (s < 0) {
		goto done;
	}
	if (!leave_output) {
		DIAG_DEQUE_FOR_EACH(q, e) {
			remove((char *)e->attr);
		}
	}
	r = EXIT_SUCCESS;

 done:
	DIAG_DEQUE_FOR_EACH(q, e) {
		diag_free((void *)e->attr);
	}
	diag_deque_destroy(q);
	diag_free(in);
	diag_free(dir);
	return r;
}
