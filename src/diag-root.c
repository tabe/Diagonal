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
#include "diagonal/vector.h"
#include "diagonal/private/system.h"
#include "diagonal/private/temporary-file.h"

static int read_real(const char *file, double *r)
{
	struct diag_port *port = diag_port_new_path(file, "r");
	if (!port) {
		/* TODO */
		return 0;
	}
	uint8_t buf[256];
	memset(buf, 0, sizeof(buf));
	int i = diag_port_read_bytes(port, sizeof(buf), buf);
	if (i < 0) {
		/* TODO */
		diag_port_destroy(port);
		return 0;
	}
	if (port->i_pos == 0) {
		/* TODO */
		diag_port_destroy(port);
		return 0;
	}
	*r = atof((char *)buf);
	diag_port_destroy(port);
	return 1;
}

static int write_real(double r, char **file)
{
	uint8_t buf[256];
	memset(buf, 0, sizeof(buf));
	int s = snprintf((char *)buf, 256, "%g", r);
	if (s <= 0 || s >= 256) {
		return 0;
	}
	struct diag_temporary_file *tf = diag_temporary_file_new();
	if (!tf) {
		return 0;
	}
	int i = diag_port_write_bytes(tf->port, s, buf);
	if (i < 0) {
		diag_temporary_file_destroy(tf);
		return 0;
	}
	if (tf->port->o_pos == 0) {
		diag_temporary_file_destroy(tf);
		return 0;
	}
	*file = diag_strdup(tf->path);
	diag_temporary_file_destroy(tf);
	return 1;
}

static int evaluate(char **argv, const char *input, double *val)
{
	struct diag_command *cmd = diag_command_new(argv, NULL,
						    input, NULL, NULL);
	if (!cmd) {
		return EXIT_FAILURE;
	}
	int r = EXIT_FAILURE;
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
	assert(cmd->out);
	assert(cmd->err);
	diag_remove(cmd->err);
	if (!read_real(cmd->out, val)) {
		diag_remove(cmd->out);
		goto done;
	}
	diag_remove(cmd->out);
	r = EXIT_SUCCESS;
 done:
	diag_command_destroy(cmd);
	return r;
}

static void usage(void)
{
	diag_printf("diag-root [-n num_of_iteration] -g guess0 -g guess1 ..."
		    " command [operand ...]");
}

int main(int argc, char *argv[])
{
	int c;
	int num_of_iter = 0;
	int r = EXIT_FAILURE;
	char *g = NULL;
	struct diag_vector *fv = diag_vector_create(0);

	diag_init();

	while ( (c = getopt(argc, argv, "+Vg:hn:")) >=0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'g':
			g = diag_strdup(optarg);
			diag_vector_push_back(fv, (intptr_t)g);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'n':
			num_of_iter = atoi(optarg);
			if (num_of_iter <= 0) {
				diag_fatal("non-positive num_of_iteration");
			}
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
	size_t k = diag_vector_length(fv);
	if (k < 2) {
		diag_fatal("give me 2 or more guesses via -g option");
	}

	char *input;
	double *xp;
	double *rp;
	struct diag_vector *xv = diag_vector_create(0);
	struct diag_vector *rv = diag_vector_create(0);
	size_t i;
	for (i = 0; i < k; i++) {
		input = (char *)diag_vector_ref(fv, i);
		assert(input);

		xp = diag_malloc(sizeof(*xp));
		if (!read_real(input, xp)) {
			/* TODO */
			diag_free(xp);
			goto done0;
		}
		diag_vector_push_back(xv, (intptr_t)xp);

		rp = diag_malloc(sizeof(*rp));
		r = evaluate(argv + optind, input, rp);
		if (r != EXIT_SUCCESS) {
			diag_free(rp);
			goto done0;
		}
		if (*rp == 0) {
			printf("%g\n", *xp);
			/* TODO */
			diag_free(rp);
			r = EXIT_SUCCESS;
			goto done0;
		}
		diag_vector_push_back(rv, (intptr_t)rp);
	}
	assert(k == diag_vector_length(xv));
	assert(k == diag_vector_length(rv));

	char *file = NULL;
	double x_n2 = *(double *)diag_vector_ref(xv, k-2);
	double x_n1 = *(double *)diag_vector_ref(xv, k-1);
	double r_n2 = *(double *)diag_vector_ref(rv, k-2);
	double r_n1 = *(double *)diag_vector_ref(rv, k-1);
	double x_n;
	double q;

	int n = 0;
 run:
	q = r_n1 - r_n2;
	if (q == 0) {
		diag_error("failed due to div-by-zero");
		goto done0;
	}
	x_n = (x_n2*r_n1 - x_n1*r_n2)/q;
	if (!write_real(x_n, &file)) {
		goto done0;
	}
	double r_n;
	r = evaluate(argv + optind, file, &r_n);
	if (r != EXIT_SUCCESS) {
		goto done1;
	}
	if (r_n != 0) {
		if (num_of_iter > 0 && ++n == num_of_iter) {
			diag_error("the number of iteration exceeded");
			goto done1;
		}
		diag_remove(file);
		diag_free(file);
		x_n2 = x_n1;
		x_n1 = x_n;
		r_n2 = r_n1;
		r_n1 = r_n;
		goto run;
	}
	printf("%g\n", x_n);
	r = EXIT_SUCCESS;

 done1:
	if (file) {
		diag_remove(file);
		diag_free(file);
	}
 done0:
	diag_vector_destroy(fv);
	diag_vector_destroy(xv);
	diag_vector_destroy(rv);
	return r;
}
