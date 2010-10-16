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

enum diag_command_index_e {
	DIAG_COMMAND_AVG,
	DIAG_COMMAND_DEC,
	DIAG_COMMAND_ENC,
	DIAG_COMMAND_FIX,
	DIAG_COMMAND_IMF,
	DIAG_COMMAND_LINE,
};

static const char *commands[] = {
	"avg",
	"dec",
	"enc",
	"fix",
	"imf",
	"line",
};

#define NUM_OF_COMMANDS (sizeof(commands)/sizeof(commands[0]))

struct diag_command_variation {
	unsigned int i;
	char *name;
} command_variations[] = {
	{DIAG_COMMAND_AVG, "average"},
	{DIAG_COMMAND_AVG, "avg"},
	{DIAG_COMMAND_DEC, "dec"},
	{DIAG_COMMAND_DEC, "decode"},
	{DIAG_COMMAND_ENC, "enc"},
	{DIAG_COMMAND_ENC, "encode"},
	{DIAG_COMMAND_FIX, "fix"},
	{DIAG_COMMAND_IMF, "imf"},
	{DIAG_COMMAND_LINE, "line"},
};

#define NUM_OF_COMMAND_VARIATIONS (sizeof(command_variations)/sizeof(command_variations[0]))

static int
cmpcmd(const void *x, const void *y)
{
	struct diag_command_variation *cx = (struct diag_command_variation *)x;
	struct diag_command_variation *cy = (struct diag_command_variation *)y;
	return strcmp(cx->name, cy->name);
}

static void
usage(void)
{
	diag_printf("diag [-l] <command> ...");
}

static void
print_commands(void)
{
	unsigned int i, j, k = 0;

	diag_printf("available commands are:");
	for (i = 0; i < NUM_OF_COMMANDS; i++) {
		for (j = k; k < NUM_OF_COMMAND_VARIATIONS && command_variations[k].i == i; k++) ;
		if (k - j == 1) {
			diag_printf("  %s", commands[i]);
		} else if (strcmp(commands[i], command_variations[j].name) == 0) {
			diag_printf("  %-8s(%s)", command_variations[j+1].name, commands[i]);
		} else {
			diag_printf("  %-8s(%s)", command_variations[j].name, commands[i]);
		}
	}
}

int
main(int argc, char *argv[])
{
	char c, *e;
	char *path1, *path2, *dir, *base;
	const char *cmd;
	struct diag_command_variation cv, *found;
	int elen;

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "+Vhl")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'l':
			print_commands();
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
	cv.name = argv[optind];
	found = (struct diag_command_variation *)bsearch(&cv, (const void *)command_variations, NUM_OF_COMMAND_VARIATIONS, sizeof(struct diag_command_variation), cmpcmd);
	if (!found) {
		usage();
		exit(EXIT_FAILURE);
	}
	assert(found->i < NUM_OF_COMMANDS);
	cmd = commands[found->i];

	assert(argv[0]);
	path1 = strdup(argv[0]);
	if (!path1) {
		diag_fatal("could not duplicate path");
	}
	path2 = strdup(argv[0]);
	if (!path2) {
		diag_fatal("could not duplicate path");
	}
	dir = dirname(path1);
	base = basename(path2);
	e = diag_malloc(DIAG_EXECUTABLE_PATH_MAX + 1);
	if (strcmp(base, argv[0]) == 0) {
		elen = snprintf(e, DIAG_EXECUTABLE_PATH_MAX, "diag-%s", cmd);
	} else {
		elen = snprintf(e, DIAG_EXECUTABLE_PATH_MAX, "%s/diag-%s", dir, cmd);
	}
	if (elen < 0) {
		diag_fatal("failed to construct command path");
	} else if (DIAG_EXECUTABLE_PATH_MAX <= elen) {
		diag_fatal("exceed DIAG_EXECUTABLE_PATH_MAX");
	}
	free(path1);
	free(path2);
	if (execvp(e, &argv[optind]) == -1) {
		diag_free(e);
		diag_fatal("failed to exec");
	}
	return EXIT_SUCCESS;
}
