/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && defined(__MINGW32__)
#include <process.h>
#include <windows.h>
#endif

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/private/system.h"

#define DIAG_EXECUTABLE_PATH_MAX 1024

enum diag_command_index_e {
	DIAG_COMMAND_CYCLE,
	DIAG_COMMAND_DEC,
	DIAG_COMMAND_ENC,
	DIAG_COMMAND_FILE,
	DIAG_COMMAND_FIX,
	DIAG_COMMAND_HASH,
	DIAG_COMMAND_LINE,
	DIAG_COMMAND_MEAN,
	DIAG_COMMAND_MEDI,
	DIAG_COMMAND_MODE,
	DIAG_COMMAND_POOL,
	DIAG_COMMAND_REP,
	DIAG_COMMAND_ROOT,
	DIAG_COMMAND_UNIQ
};

static const char *commands[] = {
	"cycle",
	"dec",
	"enc",
	"file",
	"fix",
	"hash",
	"line",
	"mean",
	"medi",
	"mode",
	"pool",
	"rep",
	"root",
	"uniq"
};

#define NUM_OF_COMMANDS (sizeof(commands)/sizeof(commands[0]))

struct diag_command_variation {
	unsigned int i;
	char *name;
} command_variations[] = {
	{DIAG_COMMAND_CYCLE, "cycle"},
	{DIAG_COMMAND_DEC, "dec"},
	{DIAG_COMMAND_DEC, "decode"},
	{DIAG_COMMAND_ENC, "enc"},
	{DIAG_COMMAND_ENC, "encode"},
	{DIAG_COMMAND_FILE, "file"},
	{DIAG_COMMAND_FIX, "fix"},
	{DIAG_COMMAND_HASH, "hash"},
	{DIAG_COMMAND_LINE, "line"},
	{DIAG_COMMAND_MEAN, "mean"},
	{DIAG_COMMAND_MEDI, "medi"},
	{DIAG_COMMAND_MEDI, "median"},
	{DIAG_COMMAND_MODE, "mode"},
	{DIAG_COMMAND_POOL, "pool"},
	{DIAG_COMMAND_REP, "rep"},
	{DIAG_COMMAND_REP, "repeat"},
	{DIAG_COMMAND_ROOT, "root"},
	{DIAG_COMMAND_UNIQ, "uniq"}
};

#define NUM_OF_COMMAND_VARIATIONS (sizeof(command_variations)/sizeof(command_variations[0]))

static int
cmpcmd(const void *x, const void *y)
{
	const struct diag_command_variation *cx = x;
	const struct diag_command_variation *cy = y;
	return strcmp(cx->name, cy->name);
}

static void
usage(void)
{
	diag_printf("diag <command> ...");
	diag_printf("diag -l");
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

#if defined(_WIN32) && defined(__MINGW32__)

static void run_command(char *e, char **argv) {
	if (strlen(e) + 4 > DIAG_EXECUTABLE_PATH_MAX) {
		diag_fatal("exceed DIAG_EXECUTABLE_PATH_MAX");
	}
	strcat(e, ".exe");
	argv[0] = e;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	STARTUPINFO si;
	GetStartupInfo(&si);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	char *line = diag_get_command_line(argv);

	BOOL b = CreateProcess(e,
			       line,
			       NULL,
			       NULL,
			       TRUE,
			       0,
			       NULL,
			       NULL,
			       &si,
			       &pi);
	if (!b) {
		diag_error("failed to create process: %s: %x",
			   line,
			   (unsigned int)GetLastError());
		// should follow GetLastError()
		diag_free(line);
		exit(EXIT_FAILURE);
	}

	/* We do not want WaitForInputIdle() because there is no interaction
	   between the parent process and this child */

	diag_free(line);
	CloseHandle(pi.hThread);

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD code = EXIT_FAILURE;
	if (GetExitCodeProcess(pi.hProcess, &code) == 0) {
		diag_error("failed to get exit code: %x",
			   (unsigned int)GetLastError());
	}
	CloseHandle(pi.hProcess);
	exit(code);
}

#elif defined(HAVE_UNISTD_H)

static void run_command(char *e, char **argv) {
	argv[0] = e;
	if (execvp(e, argv) == -1) {
		diag_free(e);
		diag_fatal("failed to exec");
	}
}

#endif

int
main(int argc, char *argv[])
{
	diag_init();

	int c;
	char *e;
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
	cv.name = argv[optind];
	found = bsearch(&cv, command_variations, NUM_OF_COMMAND_VARIATIONS,
			sizeof(struct diag_command_variation), cmpcmd);
	if (!found) {
		diag_fatal("unknown command: %s", cv.name);
	}
	assert(found->i < NUM_OF_COMMANDS);
	cmd = commands[found->i];

	assert(argv[0]);
	path1 = diag_strdup(argv[0]);
	if (!path1) {
		diag_fatal("could not duplicate path");
	}
	path2 = diag_strdup(argv[0]);
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
	diag_free(path1);
	diag_free(path2);

	run_command(e, &argv[optind]);
	return EXIT_SUCCESS;
}
