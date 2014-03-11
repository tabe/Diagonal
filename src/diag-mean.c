/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/line.h"
#include "diagonal/port.h"

static void usage(void)
{
	diag_printf("diag-mean [-c num_of_columns]");
}

int main(int argc, char *argv[])
{
	int c;
	long i, num_of_columns = 1;
	int r = EXIT_FAILURE;

	diag_init();

	while ( (c = getopt(argc, argv, "+Vc:h")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'c':
			num_of_columns = atol(optarg);
			if (num_of_columns <= 0) {
				diag_fatal("non-positive number of columns: %s", optarg);
			}
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case ':':
		case '?':
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}

	assert(num_of_columns >= 1);
	struct diag_port *iport = diag_port_new_stdin();
	if (!iport) {
		return EXIT_FAILURE;
	}
	struct diag_line_context *context = diag_line_context_new(iport);
	if (!context) {
		return EXIT_FAILURE;
	}
	enum diag_line_error_e e;
	size_t size;
	char *line;
	size_t n = 0;
	double *sum = diag_calloc(num_of_columns, sizeof(double));
	while ( (e = diag_line_read(context, &size, &line)) == DIAG_LINE_ERROR_OK) {
		n++;
		char *nptr = line;
		char *end;
		for (i = 0; i < num_of_columns; i++) {
			if (nptr >= line + size) {
				diag_error("missing column at line %d", n);
				goto done;
			}
			errno = 0;
			double v = strtod(nptr, &end);
			if (v == 0 && line == end) {
				diag_error("failed to convert to number");
				goto done;
			}
			if (v == HUGE_VAL || v == -HUGE_VAL) {
				diag_error("found overflow");
				goto done;
			}
			if (v == 0 && errno == ERANGE) {
				diag_error("found underflow");
				goto done;
			}
			sum[i] += v;
			nptr = end;
		}
	}
	if (n == 0) {
		diag_error("no input");
		goto done;
	}
	/* print result */
	printf("%g", sum[0]/n);
	for (i = 1; i < num_of_columns; i++) {
		printf(" %g", sum[i]/n);
	}
	putchar('\n');
	r = EXIT_SUCCESS;

 done:
	diag_free(sum);
	diag_line_context_destroy(context);
	diag_port_destroy(iport);
	return r;
}
