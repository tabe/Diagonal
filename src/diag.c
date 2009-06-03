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

struct diag_command_variation_s {
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
	struct diag_command_variation_s *cx = (struct diag_command_variation_s *)x;
	struct diag_command_variation_s *cy = (struct diag_command_variation_s *)y;
	return strcmp(cx->name, cy->name);
}

static void
usage(void)
{
	diag_info("diag <command> ...");
}

int
main(int argc, char *argv[])
{
	char c, *e, *path, *dir;
	const char *cmd;
	struct diag_command_variation_s cv, *found;
	int elen;

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
	cv.name = argv[optind];
	assert(argv[0]);
	path = strdup(argv[0]);
	if (!path) {
		diag_fatal("could not duplicate path");
	}
	found = (struct diag_command_variation_s *)bsearch(&cv, (const void *)command_variations, NUM_OF_COMMAND_VARIATIONS, sizeof(struct diag_command_variation_s), cmpcmd);
	if (!found) {
		usage();
		exit(EXIT_FAILURE);
	}
	assert(found->i < NUM_OF_COMMANDS);
	cmd = commands[found->i];
	dir = dirname(path);
	e = (char *)diag_malloc(DIAG_EXECUTABLE_PATH_MAX + 1);
	if (strcmp(dir, path) == 0) {
		elen = snprintf(e, DIAG_EXECUTABLE_PATH_MAX, "diag-%s", cmd);
	} else {
		elen = snprintf(e, DIAG_EXECUTABLE_PATH_MAX, "%s/diag-%s", dir, cmd);
	}
	if (elen < 0) {
		diag_fatal("failed to construct command path");
	} else if (DIAG_EXECUTABLE_PATH_MAX <= elen) {
		diag_fatal("exceed DIAG_EXECUTABLE_PATH_MAX");
	}
	free(path);
	if (execvp(e, &argv[optind]) == -1) {
		diag_free(e);
		diag_fatal("failed to exec");
	}
	return EXIT_SUCCESS;
}
