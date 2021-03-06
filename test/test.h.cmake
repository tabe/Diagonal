/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#cmakedefine DIAGONAL_TEST_DIR "@DIAGONAL_TEST_DIR@"

#define ASSERT_TRUE(expr) do {											\
		if (expr) break;												\
		printf("%s:%d: " #expr " is expected to be true, but false\n", __FILE__, __LINE__); \
		exit(EXIT_FAILURE);												\
	} while (0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_NOT_NULL(expr) do {										\
		if ((const void *)(expr) != NULL) break;										\
		printf("%s:%d: expected " #expr " != NULL, but NULL\n", __FILE__, __LINE__); \
		exit(EXIT_FAILURE);												\
	} while (0)

#define ASSERT_EQ_(type, slot, expected, expr) do {						\
		type actual = (type)(expr);										\
		if ((type)expected == actual) break;							\
		printf("%s:%d: " slot " is expected, but " slot "\n", __FILE__, __LINE__, (type)expected, actual); \
		exit(EXIT_FAILURE);												\
	} while (0)

#define ASSERT_EQ_CHAR(expected, expr)    ASSERT_EQ_(char, "%c", expected, expr)
#define ASSERT_EQ_INT(expected, expr)     ASSERT_EQ_(int, "%d", expected, expr)
#define ASSERT_EQ_UINT(expected, expr)    ASSERT_EQ_(unsigned int, "%u", expected, expr)
#define ASSERT_EQ_UINT8(expected, expr)   ASSERT_EQ_(uint8_t, "%u", expected, expr)
#define ASSERT_EQ_UINT32(expected, expr)  ASSERT_EQ_(uint32_t, "%" PRIu32, expected, expr)
#define ASSERT_EQ_UINT64(expected, expr)  ASSERT_EQ_(uint64_t, "%" PRIu64, expected, expr)
#define ASSERT_EQ_UINTPTR(expected, expr) ASSERT_EQ_(uintptr_t, "%" PRIuPTR, expected, expr)
#define ASSERT_EQ_SIZE(expected, expr)    ASSERT_EQ_(size_t, "%zu", expected, expr)

#define ASSERT_EQ_STRING(expected, expr) do {							\
		char *actual = (char *)(expr);									\
		if (strcmp(expected, actual) == 0) break;						\
		printf("%s:%d: %s is expected, but %s\n", __FILE__, __LINE__, expected, actual); \
		exit(EXIT_FAILURE);												\
	} while (0)
