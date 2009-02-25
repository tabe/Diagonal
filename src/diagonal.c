#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"

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
	_Exit(1);
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

void
diag_free(void *ptr)
{
	if (ptr) free(ptr);
}

diag_datum_t *
diag_datum_new(void *value)
{
	diag_datum_t *datum;

	datum = (diag_datum_t *)diag_malloc(sizeof(diag_datum_t));
	datum->value = value;
	datum->id.number = 0;
	return datum;
}

void
diag_datum_destroy(diag_datum_t *datum)
{
	if (!datum) return;
	if (DIAG_DATUM_TBFRE_P(datum)) diag_free(datum->value);
	diag_free(datum);
}