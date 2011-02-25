/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/hash.h"

int
main()
{
	uint8_t arr[128];

	ASSERT_EQ_UINT32(0, diag_hash32_rabin_karp(arr, 0, 0));
	ASSERT_EQ_UINT32(0, diag_hash32_rabin_karp(arr, 0, 1));
	ASSERT_EQ_UINT32(0, diag_hash32_rabin_karp(arr, 0, 2));

	arr[0] = 1;
	arr[1] = 2;
	arr[2] = 3;
	ASSERT_EQ_UINT32(10406, diag_hash32_rabin_karp(arr, 3, 101));

	return EXIT_SUCCESS;
}
