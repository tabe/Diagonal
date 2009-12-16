#include "test.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/hash.h"

int
main()
{
	uint8_t arr[84] = "This is a sentence of length 84 for testing function `diag_rolling_hash32_collect'.";
	uint32_t *result;
	diag_size_t length, i;
	diag_rolling_hash32_t *rh;

	rh = diag_rolling_hash32_new_rabin_karp(arr, 84, 3, 101);
	result = diag_rolling_hash32_collect(rh, &length);
	ASSERT_EQ_UINT32(82, length);
	for (i = 0; i < length; i++) {
		printf("%d: ", i);
		ASSERT_EQ_UINT32(diag_hash32_rabin_karp(arr + i, 3, 101), result[i]);
		printf("\n");
	}
	diag_free(result);
	diag_rolling_hash32_destroy(rh);
	return EXIT_SUCCESS;
}
