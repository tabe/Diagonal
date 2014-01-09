/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) && defined(__MINGW32__)
#include <fcntl.h>
#include <io.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"

void diag_init(void)
{
#if defined(_WIN32) && defined(__MINGW32__)
	int r;

	r = _setmode(_fileno(stdin), _O_BINARY);
	if (r == -1) {
		perror("failed to set mode");
	}
	r = _setmode(_fileno(stdout), _O_BINARY);
	if (r == -1) {
		perror("failed to set mode");
	}
#endif
}

void
diag_print_version(void)
{
	printf("%d.%d.%d\n", DIAGONAL_VERSION_MAJOR, DIAGONAL_VERSION_MINOR, DIAGONAL_VERSION_PATCH);
}

void
diag_fatal(const char *message, ...)
{
	va_list ap;

	assert(message);
	va_start(ap, message);
	vfprintf(stderr, message, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	_Exit(EXIT_FAILURE);
}

void
diag_verror(const char *message, va_list ap)
{
	assert(message);
	vfprintf(stderr, message, ap);
	fprintf(stderr, "\n");
}

void
diag_error(const char *message, ...)
{
	va_list ap;

	assert(message);
	va_start(ap, message);
	diag_verror(message, ap);
	va_end(ap);
}

void
diag_vwarn(const char *message, va_list ap)
{
	assert(message);
	vfprintf(stderr, message, ap);
	fprintf(stderr, "\n");
}

void
diag_warn(const char *message, ...)
{
	va_list ap;

	assert(message);
	va_start(ap, message);
	diag_vwarn(message, ap);
	va_end(ap);
}

void
diag_vinfo(const char *message, va_list ap)
{
	assert(message);
	vfprintf(stderr, message, ap);
	fprintf(stderr, "\n");
}

void
diag_info(const char *message, ...)
{
	va_list ap;

	assert(message);
	va_start(ap, message);
	diag_vinfo(message, ap);
	va_end(ap);
}

void
diag_vprintf(const char *message, va_list ap)
{
	assert(message);
	vfprintf(stdout, message, ap);
	fprintf(stdout, "\n");
}

void
diag_printf(const char *message, ...)
{
	va_list ap;

	assert(message);
	va_start(ap, message);
	diag_vprintf(message, ap);
	va_end(ap);
}

void *
diag_malloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
	if (!ptr) {
		diag_fatal("malloc failed");
	}
	return ptr;
}

void *
diag_calloc(size_t nmemb, size_t size)
{
	void *ptr;

	ptr = calloc(nmemb, size);
	if (!ptr) {
		diag_fatal("calloc failed");
	}
	return ptr;
}

void *
diag_realloc(void *ptr, size_t size)
{
	void *new_ptr;

	new_ptr = realloc(ptr, size);
	if (!new_ptr) {
		free(ptr);
		diag_fatal("realloc failed");
	}
	return new_ptr;
}

void
diag_free(void *ptr)
{
	free(ptr);
}

char *diag_strdup(const char *s)
{
	assert(s);
	size_t len = strlen(s);
	char *r = diag_malloc(len + 1);
	memcpy(r, s, len);
	r[len] = '\0'; /* NULL-terminated */
	return r;
}
