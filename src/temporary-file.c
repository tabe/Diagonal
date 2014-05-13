/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && defined(__MINGW32__)
#include <windows.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/port.h"
#include "diagonal/private/filesystem.h"
#include "diagonal/private/temporary-file.h"

#if defined(_WIN32) && defined(__MINGW32__)
static const DWORD PATH_LENGTH = MAX_PATH;
#else
static const size_t PATH_LENGTH = 1024;
#endif

static int get_temporary_directory(char *dir)
{
	assert(dir);

#if defined(_WIN32) && defined(__MINGW32__)
	DWORD r = GetTempPath(PATH_LENGTH, dir);
	if ( r > PATH_LENGTH || r == 0 ) {
		return -1;
	}
	return (int)strlen(dir);
#else
	static const char DEFAULT_DIR[] = "/tmp";

	char *v = getenv("TMPDIR");
	if (v) {
		size_t len = strlen(v);
		if (len <= PATH_LENGTH && diag_is_directory(v)) {
			strcpy(dir, v);
			while ( len > 0 && dir[len - 1] == '/' ) {
				dir[--len] = '\0';
			}
			return (int)len;
		}
	}
	if (diag_is_directory(DEFAULT_DIR)) {
		strcpy(dir, DEFAULT_DIR);
		return (int)strlen(dir);
	}
	return -1;
#endif
}

/* API */

struct diag_temporary_file *diag_temporary_file_new(void)
{
#if defined(_WIN32) && defined(__MINGW32__)
	char *dir = diag_malloc(PATH_LENGTH + 1);
	int dirlen = get_temporary_directory(dir);
	if (dirlen < 0) {
		/* TODO */
		diag_free(dir);
		return NULL;
	}
	char *path = diag_malloc(PATH_LENGTH + 1);
	UINT r = GetTempFileName(dir, "dia", 0, path);
	diag_free(dir);
	if (r == 0) {
		/* TODO */
		diag_free(path);
		return NULL;
	}
	FILE *fp = fopen(path, "wb+");
	if (!fp) {
		DeleteFile(path);
		diag_free(path);
		return NULL;
	}
	struct diag_temporary_file *tf = diag_malloc(sizeof(*tf));
	tf->path = path;
	tf->port = diag_port_new_fp(fp, DIAG_PORT_INPUT|DIAG_PORT_OUTPUT);
	tf->f = (intptr_t)fp;
	return tf;
#else
	static const char TEMPLATE[] = "diagonalXXXXXX";

	char *path = diag_malloc(PATH_LENGTH + 1);
	int dirlen = get_temporary_directory(path);
	if (dirlen < 0) {
		/* TODO */
		diag_free(path);
		return NULL;
	}
	assert(dirlen >= 0);
	size_t len1 = (size_t)dirlen + sizeof(TEMPLATE) + 1;
	path = diag_realloc(path, len1);
	path[dirlen] = '/';
	strcpy(path + dirlen + 1, TEMPLATE);
	mode_t old_mode = umask(S_IXUSR|S_IRWXG|S_IRWXO);
	int fd = mkstemp(path);
	(void)umask(old_mode);
	if (fd == -1) {
		perror(path);
		diag_free(path);
		return NULL;
	}
	struct diag_temporary_file *tf = diag_malloc(sizeof(*tf));
	tf->path = path;
	tf->port = diag_port_new_fd(fd, DIAG_PORT_INPUT|DIAG_PORT_OUTPUT);
	tf->f = fd;
	return tf;
#endif
}

void diag_temporary_file_destroy(struct diag_temporary_file *tf)
{
	if (!tf) return;
#if defined(_WIN32) && defined(__MINGW32__)
	fclose((FILE *)tf->f);
#else
	close((int)tf->f);
#endif
	diag_port_destroy(tf->port);
	diag_free(tf->path);
	diag_free(tf);
}
