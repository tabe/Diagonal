/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/hash.h"

int main(void)
{
#ifdef HAVE_ARPA_INET_H
	uint8_t arr[128];

	ASSERT_EQ_UINT32(htonl(1), diag_hash32_adler32(arr, 0));

	arr[0] = 'A';
	arr[1] = 'B';
	arr[2] = 'C';
	arr[3] = 'x';
	arr[4] = 'y';
	arr[5] = 'z';
	ASSERT_EQ_UINT32(htonl((1718 << 16)|562), diag_hash32_adler32(arr, 6));
#endif

	return EXIT_SUCCESS;
}
