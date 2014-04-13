/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/hash.h"

int main(void)
{
	uint8_t arr[83] = "This is a sentence of length 83 for testing function `diag_rollinghash32_collect'.";
	uint32_t *result;
	size_t length, i;
	struct diag_rollinghash32 *rh;

	rh = diag_rollinghash32_new_rabin_karp(arr, 1, 1, 107);
	result = diag_rollinghash32_collect(rh, &length);
	ASSERT_EQ_UINT32(1, length);
	ASSERT_EQ_UINT32(diag_hash32_rabin_karp(arr, 1, 107), result[0]);
	diag_free(result);
	diag_rollinghash32_destroy(rh);

	rh = diag_rollinghash32_new_rabin_karp(arr, 83, 3, 101);
	result = diag_rollinghash32_collect(rh, &length);
	ASSERT_EQ_UINT32(81, length);
	for (i = 0; i < length; i++) {
		printf("%zu: ", i);
		ASSERT_EQ_UINT32(diag_hash32_rabin_karp(arr + i, 3, 101), result[i]);
		printf("\n");
	}
	diag_free(result);
	diag_rollinghash32_destroy(rh);

	return EXIT_SUCCESS;
}
