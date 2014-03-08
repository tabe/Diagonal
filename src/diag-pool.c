/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/vector.h"
#include "diagonal/private/system.h"

static const int NUMBER_OF_PROCESSES = 5;

static void usage(void)
{
	diag_printf("diag-pool [-n num] command [operand ...]");
}

int main(int argc, char *argv[])
{
	int c, i, n = NUMBER_OF_PROCESSES;
	int r = EXIT_FAILURE;

	diag_init();

	while ( (c = getopt(argc, argv, "+Vhn:")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'n':
			n = atoi(optarg);
			if (n < 0) {
				diag_fatal("non-negative integer expected, but %d", n);
			} else if (n == 0) {
				exit(EXIT_SUCCESS);
			}
			break;
		case ':':
		case '?':
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (optind >= argc) {
		usage();
		exit(EXIT_FAILURE);
	}

	assert(n > 0);
	struct diag_vector *pv = diag_vector_create(n);
	if (!pv) {
		exit(EXIT_FAILURE);
	}
	char **cmd = argv + optind;
	intptr_t p;
	for (i = 0; i < n; i++) {
		p = diag_run_agent(cmd);
		diag_vector_set(pv, i, p);
	}
	for (;;) {
		i = diag_wait_agent(n, pv->elements, NULL);
		if (i < 0) {
			goto done;
		}
		p = diag_run_agent(cmd);
		diag_vector_set(pv, i, p);
	}
	r = EXIT_SUCCESS;

 done:
	diag_vector_destroy(pv);
	return r;
}
