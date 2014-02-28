/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/port.h"
#include "diagonal/private/filesystem.h"
#include "diagonal/private/system.h"
#include "diagonal/private/temporary-file.h"

static void usage(void)
{
	diag_printf("diag-fix [-i input] command [operand ...]");
}

int main(int argc, char *argv[])
{
	int c;
	int r = EXIT_FAILURE;
	char *in = NULL;

	diag_init();

	while ( (c = getopt(argc, argv, "+Vhi:")) >=0) {
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
			in = optarg;
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

	char *tfin = NULL;
	if (!in) {
		struct diag_temporary_file *tf = diag_temporary_file_new();
		if (!tf) {
			return EXIT_FAILURE;
		}
		tfin = diag_strdup(tf->path);
		struct diag_port *ip = diag_port_new_stdin();
		assert(ip);
		ssize_t s = diag_port_copy(ip, tf->port);
		diag_port_destroy(ip);
		diag_temporary_file_destroy(tf);
		if (s < 0) {
			assert(tfin);
			diag_remove(tfin);
			diag_free(tfin);
			return EXIT_FAILURE;
		}
	}

	size_t n = 0;
	char *file = in ? in : tfin;
	struct diag_command *cmd;
 run:
	cmd = diag_command_new(argv + optind, NULL, file, NULL, NULL);
	if (!cmd) {
		goto done;
	}
	struct diag_process *p = diag_run_program(cmd);
	if (!p) {
		goto done;
	}
	diag_process_wait(p);
	int status = p->status;
	diag_process_destroy(p);
	if (status != 0) {
		r = status;
		goto done;
	}

	assert(cmd->err);
	diag_remove(cmd->err);

	struct diag_port *ip;
	struct diag_port *op;
	ip = diag_port_new_path(file, "rb");
	if (!ip) {
		goto done;
	}
	op = diag_port_new_path(cmd->out, "rb");
	if (!op) {
		diag_port_destroy(ip);
		goto done;
	}
	int d = diag_port_diff(ip, op);
	diag_port_destroy(op);
	diag_port_destroy(ip);
	if (d == -1) {
		goto done;
	}
	if (d == 1) {
		if (n) {
			diag_remove(file);
			diag_free(file);
		} else if (tfin) {
			diag_remove(tfin);
			diag_free(tfin);
			tfin = NULL;
		}
		file = diag_strdup(cmd->out);
		diag_command_destroy(cmd);
		n++;
		goto run;
	}

	ip = diag_port_new_path(file, "rb");
	if (!ip) {
		goto done;
	}
	op = diag_port_new_stdout();
	assert(op);
	ssize_t s = diag_port_copy(ip, op);
	diag_port_destroy(op);
	diag_port_destroy(ip);
	if (s < 0) {
		goto done;
	}

	diag_remove(cmd->out);
	if (n) {
		diag_remove(file);
		diag_free(file);
	}

	r = EXIT_SUCCESS;

 done:
	diag_command_destroy(cmd);
	if (tfin) {
		diag_remove(tfin);
		diag_free(tfin);
	}
	return r;
}
