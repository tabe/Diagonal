#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"

#define DIAG_EXECUTABLE_PATH_MAX 1024

static const char *commands[] = {
	"avg",
	"dec",
	"enc",
	"fix",
	"imf",
	"line",
	NULL
};

static void
usage(void)
{
	diag_info("diag <command> ...");
}

int
main(int argc, char *argv[])
{
	char c, *path, *dir, *cmd;
	unsigned int i;

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
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
		default:
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}
	cmd = argv[optind];
	assert(argv[0]);
	path = strdup(argv[0]);
	if (!path) {
		diag_fatal("could not duplicate path");
	}
	for (i = 0; commands[i]; i++) {
		if (strcmp(cmd, commands[i]) == 0) {
			char *e;
			int elen;

			dir = dirname(path);
			e = (char *)diag_malloc(DIAG_EXECUTABLE_PATH_MAX + 1);
			if (strcmp(".", dir) == 0) {
				elen = snprintf(e, DIAG_EXECUTABLE_PATH_MAX, "diag-%s", commands[i]);
			} else {
				elen = snprintf(e, DIAG_EXECUTABLE_PATH_MAX, "%s/diag-%s", dir, commands[i]);
			}
			if (elen < 0) {
				diag_fatal("fail to construct command path");
			} else if (DIAG_EXECUTABLE_PATH_MAX <= elen) {
				diag_fatal("exceed DIAG_EXECUTABLE_PATH_MAX");
			}
			free(path);
			execvp(e, &argv[optind]);
		}
	}
	free(path);
	usage();
	return EXIT_FAILURE;
}
