/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_H
#define DIAGONAL_H

#define DIAGONAL_VERSION_MAJOR 0
#define DIAGONAL_VERSION_MINOR 0
#define DIAGONAL_VERSION_PATCH 1

#include <stdarg.h>

#ifdef __cplusplus
#define DIAG_C_DECL_BEGIN extern "C" {
#define DIAG_C_DECL_END   }
#else
#define DIAG_C_DECL_BEGIN
#define DIAG_C_DECL_END
#endif

#define DIAG_FUNCTION extern

#define DIAG_SUCCESS 1
#define DIAG_FAILURE 0

DIAG_C_DECL_BEGIN

DIAG_FUNCTION void diag_init(void);

DIAG_FUNCTION void diag_print_version(void);

DIAG_FUNCTION void diag_fatal(const char *message, ...);
DIAG_FUNCTION void diag_verror(const char *message, va_list ap);
DIAG_FUNCTION void diag_error(const char *message, ...);
DIAG_FUNCTION void diag_vwarn(const char *message, va_list ap);
DIAG_FUNCTION void diag_warn(const char *message, ...);
DIAG_FUNCTION void diag_vinfo(const char *message, va_list ap);
DIAG_FUNCTION void diag_info(const char *message, ...);
DIAG_FUNCTION void diag_vprintf(const char *message, va_list ap);
DIAG_FUNCTION void diag_printf(const char *message, ...);

DIAG_FUNCTION void *diag_malloc(size_t size);
DIAG_FUNCTION void *diag_calloc(size_t nmemb, size_t size);
DIAG_FUNCTION void *diag_realloc(void *ptr, size_t size);
/**
 * Free the resource which `ptr' refers to.
 * Do nothing if `ptr' is NULL.
 */
DIAG_FUNCTION void diag_free(void *ptr);

DIAG_FUNCTION char *diag_strdup(const char *s);

DIAG_C_DECL_END

#endif
