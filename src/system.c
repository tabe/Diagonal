/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && defined(__MINGW32__)
#include <direct.h>
#include <process.h>
#include <windows.h>
#elif defined(HAVE_UNISTD_H)
#include <unistd.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#else
#error "no available system utilities"
#endif

#include "diagonal.h"
#include "diagonal/private/system.h"

#define PATH_LENGTH 1024

static void build_path(const char *dir, int pid, const char *ext, char *path)
{
	assert(path);
	int len = sprintf(path, "%s/diagonal%d.%s", dir, (int)pid, ext);
	if (len < 0) diag_fatal("fail to build path");
	if (PATH_LENGTH <= len) diag_fatal("exceed PATH_LENGTH");
}

static int contains_space(const char *s)
{
	size_t i;
	for (i = 0; s[i]; i++) {
		if (isspace((int)s[i])) return 1;
	}
	return 0;
}

/* API */

struct diag_command *diag_command_new(char **argv,
				      const char *dir,
				      const char *in,
				      const char *out,
				      const char *err)
{
	struct diag_command *command = diag_malloc(sizeof(*command));
	int argc;
	for (argc = 0; argv[argc]; argc++) ;
	command->argv = diag_calloc(argc + 1, sizeof(char *));
	int i;
	for (i = 0; i < argc; i++) {
		command->argv[i] = diag_strdup(argv[i]);
	}
	command->argv[argc] = NULL; /* NULL-terminated */
	command->file = argv[0];

	if (dir) {
		command->dir = diag_strdup(dir);
	} else {
		command->dir = diag_malloc(PATH_LENGTH);
		char *p;
#if defined(_WIN32) && defined(__MINGW32__)
		p = _getcwd(command->dir, PATH_LENGTH);
#elif defined(HAVE_UNISTD_H)
		p = getcwd(command->dir, PATH_LENGTH);
#endif
		if (!p) {
			perror(NULL);
			diag_fatal("failed to get current working directory");
		}
	}

	if (in) {
		command->in = diag_strdup(in);
	} else {
		command->in = NULL;
	}
	if (out) {
		command->out = diag_strdup(out);
	} else {
		command->out = NULL;
	}
	if (err) {
		command->err = diag_strdup(err);
	} else {
		command->err = NULL;
	}
	return command;
}

void diag_command_destroy(struct diag_command *command)
{
	if (!command) return;
	int i;
	for (i = 0; command->argv[i]; i++) {
		diag_free(command->argv[i]);
	}
	diag_free(command->argv);
	diag_free(command->dir);
	diag_free(command->in);
	diag_free(command->out);
	diag_free(command->err);
	diag_free(command);
}

char *diag_get_command_line(char **argv)
{
	assert(argv);

	char *line = diag_malloc(32768);
	char *lp = line;
	int i;
	for (i = 0; argv[i]; i++) {
		const char *s = argv[i];
		int slen = (int)strlen(s);
		if (lp + slen >= line + 32768) {
			diag_free(line);
			return NULL;
		}
		if (i > 0) {
			*lp++ = ' ';
		}
		if (contains_space(s)) {
			*lp++ = '"';
			strcpy(lp, s);
			lp += slen;
			*lp++ = '"';
		} else {
			strcpy(lp, s);
			lp += slen;
		}
	}
	*lp = '\0';
	return line;
}

#if defined(_WIN32) && defined(__MINGW32__)

static int local_pid = 100;

struct diag_process *diag_run_program(struct diag_command *command)
{
	assert(command);

	if (!command->out) {
		command->out = diag_malloc(PATH_LENGTH);
		build_path(command->dir, local_pid, "out", command->out);
	}
	if (!command->err) {
		command->err = diag_malloc(PATH_LENGTH);
		build_path(command->dir, local_pid, "err", command->err);
	}
	local_pid++;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	HANDLE ih = NULL;
	if (command->in) {
		ih = CreateFile(command->in,
				GENERIC_READ,
				FILE_SHARE_READ,
				&sa,
				OPEN_EXISTING,
				0,
				NULL);
		if (ih == INVALID_HANDLE_VALUE) {
			diag_error("failed to open file: %s: 0x%x",
				   command->in,
				   (unsigned int)GetLastError());
			return NULL;
		}
	}

	HANDLE oh = CreateFile(command->out,
			       GENERIC_WRITE,
			       FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			       &sa,
			       CREATE_NEW,
			       FILE_ATTRIBUTE_NORMAL,
			       NULL);
	if (oh == INVALID_HANDLE_VALUE) {
		diag_error("failed to open file: %s: 0x%x",
			   command->out,
			   (unsigned int)GetLastError());
		return NULL;
	}

	HANDLE eh = CreateFile(command->err,
			       GENERIC_WRITE,
			       FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			       &sa,
			       CREATE_NEW,
			       FILE_ATTRIBUTE_NORMAL,
			       NULL);
	if (eh == INVALID_HANDLE_VALUE) {
		diag_error("failed to open file: %s: 0x%x",
			   command->err,
			   (unsigned int)GetLastError());
		return NULL;
	}

	STARTUPINFO si;
	GetStartupInfo(&si);
	si.dwFlags |= STARTF_USESTDHANDLES;
	si.hStdInput = ih;
	si.hStdOutput = oh;
	si.hStdError = eh;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	char *line = diag_get_command_line(command->argv);

	BOOL b = CreateProcess(NULL,
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
		diag_error("could not create process: %s: 0x%x",
			   line,
			   (unsigned int)GetLastError());
		// should follow GetLastError()
		diag_free(line);
		if (ih) CloseHandle(ih);
		CloseHandle(oh);
		CloseHandle(eh);
		return NULL;
	}

	/* We do not want WaitForInputIdle() because there is no interaction
	   between the parent process and this child */

	diag_free(line);
	if (ih) CloseHandle(ih);
	CloseHandle(oh);
	CloseHandle(eh);

	CloseHandle(pi.hThread);

	struct diag_process *p = diag_malloc(sizeof(*p));
	p->pid = (intptr_t)pi.hProcess;
	return p;
}

void diag_process_wait(struct diag_process *process)
{
	assert(process);

	HANDLE h = (HANDLE)process->pid;
	WaitForSingleObject(h, INFINITE);
	DWORD code = 0;
	if (GetExitCodeProcess(h, &code) == 0) {
		diag_error("failed to get exit code: %x",
			   (unsigned int)GetLastError());
	}
	process->status = code;
	CloseHandle(h);
}

#elif defined(HAVE_UNISTD_H)

struct diag_process *diag_run_program(struct diag_command *command)
{
	assert(command);

	char *out;
	if (!command->out) {
		out = diag_malloc(PATH_LENGTH);
	}
	char *err;
	if (!command->err) {
		err = diag_malloc(PATH_LENGTH);
	}
	pid_t pid = fork();
	if (pid < 0) {
		perror(command->file);
		return NULL;
	}
	if (pid == 0) {
		pid = getpid();
		if (!command->out) {
			build_path(command->dir, pid, "out", out);
			command->out = out;
		}
		if (!command->err) {
			build_path(command->dir, pid, "err", err);
			command->err = err;
		}
		if (command->in) { /* redirect stdin only when specified */
			if (!freopen(command->in, "rb", stdin)) {
				perror(command->in);
				_Exit(EXIT_FAILURE);
			}
		}
		/* Both stdout and stderr should be always redirected */
		if (!freopen(command->out, "wb", stdout)) {
			perror(command->out);
			_Exit(EXIT_FAILURE);
		}
		if (!freopen(command->err, "wb", stderr)) {
			perror(command->err);
 			_Exit(EXIT_FAILURE);
		}
		if (execvp(command->file, command->argv) == -1) {
			perror(command->file);
			_Exit(EXIT_FAILURE);
		}
	}
	if (!command->out) {
		build_path(command->dir, pid, "out", out);
		command->out = out;
	}
	if (!command->err) {
		build_path(command->dir, pid, "err", err);
		command->err = err;
	}
	struct diag_process *p = diag_malloc(sizeof(*p));
	p->pid = (intptr_t)pid;
	p->status = 0;
	return p;
}

void diag_process_wait(struct diag_process *process)
{
	assert(process);

	int status;
	pid_t pid = waitpid((pid_t)process->pid, &status,
#ifdef WCONTINUED
			    WCONTINUED|
#endif
			    WUNTRACED);
	if (pid == (pid_t)0) {
		assert(0);
	} else if (pid == (pid_t)-1) {
		perror(NULL);
		diag_fatal("failed to wait process: %d", (int)process->pid);
	}
	if (WIFEXITED(status)) {
		process->status = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		process->status = -1;
	} else if (WIFSTOPPED(status)) {

#ifdef WIFCONTINUED
	} else if (WIFCONTINUED(status)) {

#endif
	}
}

#endif

void diag_process_destroy(struct diag_process *process)
{
	diag_free(process);
}
