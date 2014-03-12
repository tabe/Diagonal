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
#include "diagonal/qselect.h"

static int compare_double(const void *x, const void *y)
{
	const double *a = x;
	const double *b = y;
	if (*a == *b) return 0;
	if (*a < *b) return -1;
	return 1;
}

static void usage(void)
{
	diag_printf("diag-medi");
}

int main(int argc, char *argv[])
{
	int c, r = EXIT_FAILURE;

	diag_init();

	while ( (c = getopt(argc, argv, "+Vh")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
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

	struct diag_port *iport = diag_port_new_stdin();
	if (!iport) {
		return EXIT_FAILURE;
	}
	struct diag_line_context *context = diag_line_context_new(iport);
	if (!context) {
		diag_port_destroy(iport);
		return EXIT_FAILURE;
	}
	enum diag_line_error_e e;
	size_t size;
	char *line;
	size_t n = 0;
	size_t capacity = 32;
	double *data = diag_calloc(capacity, sizeof(double));
	while ( (e = diag_line_read(context, &size, &line)) == DIAG_LINE_ERROR_OK) {
		char *nptr = line;
		char *end;
		errno = 0;
		double v = strtod(nptr, &end);
		if (v == 0 && line == end) {
			diag_error("failed to convert to number");
			diag_free(line);
			goto done;
		}
		if (v == HUGE_VAL || v == -HUGE_VAL) {
			diag_error("found overflow");
			diag_free(line);
			goto done;
		}
		if (v == 0 && errno == ERANGE) {
			diag_error("found underflow");
			diag_free(line);
			goto done;
		}
		diag_free(line);
		if (capacity <= n) {
			capacity *= 2;
			data = diag_realloc(data, capacity * sizeof(double));
		}
		data[n++] = v;
	}
	if (n == 0) {
		diag_error("no input");
		goto done;
	}
	double result;
	if (n%2 == 0) {
		double *copied = diag_calloc(n, sizeof(double));
		memcpy(copied, data, n * sizeof(double));
		double *p = diag_qselect(data, n, sizeof(double), compare_double, n/2-1);
		assert(p);
		double *q = diag_qselect(copied, n, sizeof(double), compare_double, n/2);
		assert(q);
		result = (*p + *q)/2;
		diag_free(copied);
	} else {
		double *p = diag_qselect(data, n, sizeof(double), compare_double, n/2);
		assert(p);
		result = *p;
	}
	/* print result */
	printf("%g\n", result);
	r = EXIT_SUCCESS;

 done:
	diag_free(data);
	diag_line_context_destroy(context);
	diag_port_destroy(iport);
	return r;
}
