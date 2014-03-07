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
#include "diagonal/private/system.h"

static const int NUMBER_OF_TRIALS = 5;

static void usage(void)
{
	diag_printf("diag-repeat [-I interval] [-e code] [-n num] command [operand ...]");
}

int main(int argc, char *argv[])
{
	int c, i, n = NUMBER_OF_TRIALS;
	int interval = 0;
	int early_exit = 0;
	int exit_code = 0;

	diag_init();

	while ( (c = getopt(argc, argv, "+I:Vehn:s")) >= 0) {
		switch (c) {
		case 'I':
			interval = atoi(optarg);
			if (interval < 0) {
				diag_fatal("non-negative integer expected, but %d", interval);
			}
			break;
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'e':
			early_exit = 1;
			exit_code = atoi(optarg);
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
	assert(interval >= 0);
	char **cmd = argv + optind;
	intptr_t p;
	int r = EXIT_SUCCESS;
	for (i = 0; i < n; i++) {
		if (i > 0) diag_sleep(interval);
		p = diag_run_agent(cmd);
		(void)diag_wait_agent(1, &p, &r);
		if (early_exit && r == exit_code) {
			return r;
		}
	}
	return r;
}
