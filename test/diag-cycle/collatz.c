/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	uint64_t x;
	int r = fscanf(stdin, "%" SCNu64, &x);
	if (r != 1) {
		fprintf(stderr, "failed to read integer\n");
		return EXIT_FAILURE;
	}
	if (x == 0) {
		fprintf(stderr, "0 given\n");
		return EXIT_FAILURE;
	}
	if (x%2 == 0) {
		x /= 2;
	} else {
		if (x >= UINT64_MAX/3) {
			fprintf(stderr, "overflow\n");
			return EXIT_FAILURE;
		}
		x *= 3;
		x++;
		if (x == 0) {
			fprintf(stderr, "overflow\n");
			return EXIT_FAILURE;
		}
	}
	(void)printf("%"  PRIu64 "\n", x);
	return EXIT_SUCCESS;
}
