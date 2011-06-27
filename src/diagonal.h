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

DIAG_C_DECL_BEGIN

#define DIAG_FUNCTION extern

#define DIAG_SUCCESS 1
#define DIAG_FAILURE 0

typedef uint32_t diag_size_t;

enum {
	DIAG_TAG_CHARS = 1,
	DIAG_TAG_IMF,

	DIAG_TAG_TBFRE = 0x01<<8, /* "to be freed" */
	DIAG_TAG_SIZE  = 0x02<<8,
	DIAG_TAG_IDNUM = 0x01<<16,
	DIAG_TAG_IDNAM = 0x02<<16,
};

struct diag_datum {
	uint64_t tag;
	void *value;
	union {
		diag_size_t number;
		char *name;
	} id;
};

#define DIAG_DATUM_IMMEDIATE_P(datum) (!(diag_size_t)((datum)->tag))
#define DIAG_DATUM_CHARS_P(datum) ((datum)->tag & DIAG_TAG_CHARS)
#define DIAG_DATUM_TBFRE_P(datum) ((datum)->tag & DIAG_TAG_TBFRE)
#define DIAG_DATUM_SIZE_P(datum) ((datum)->tag & DIAG_TAG_SIZE)
#define DIAG_DATUM_ASCIZ_P(datum) \
	(DIAG_DATUM_CHARS_P(datum) && !DIAG_DATUM_SIZE_P(datum))
#define DIAG_DATUM_SIZE(datum) ((diag_size_t)((datum)->tag>>(sizeof(diag_size_t)*8)))
#define DIAG_DATUM_SET_IMMEDIATE(datum, x) ((datum)->tag = (uint64_t)(x)<<(sizeof(diag_size_t)*8))
#define DIAG_DATUM_GET_IMMEDIATE(datum) DIAG_DATUM_SIZE(datum)

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

DIAG_FUNCTION struct diag_datum *diag_datum_new(void *value);
DIAG_FUNCTION void diag_datum_destroy(struct diag_datum *datum);

DIAG_C_DECL_END

#endif
