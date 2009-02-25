#include "config.h"

#include <assert.h>
#include <errno.h>
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
		int result = (int)(expr);										\
		if (result) break;												\
		printf("%s:%d: " #expr " is expected to be true, but false\n", __FILE__, __LINE__); \
		exit(1);														\
	} while (0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_NOT_NULL(expr) do {										\
		void *result = (void *)(expr);									\
		if (result != NULL) break;										\
		printf("%s:%d: expected " #expr " != NULL, but NULL\n", __FILE__, __LINE__); \
		exit(1);														\
	} while (0)

#define ASSERT_EQ_(type, expected, expr)  do {							\
		type actual = (type)(expr);										\
		if ((type)expected == actual) break;							\
		printf("%s:%d: %d is expected, but %d\n", __FILE__, __LINE__, (type)expected, actual); \
		exit(1);														\
	} while (0)

#define ASSERT_EQ_INT(expected, expr) ASSERT_EQ_(int, expected, expr)
#define ASSERT_EQ_UINT8(expected, expr) ASSERT_EQ_(uint8_t, expected, expr)
#define ASSERT_EQ_SIZE(expected, expr) ASSERT_EQ_(diag_size_t, expected, expr)
