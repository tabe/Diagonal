/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	unsigned long int x;
	int r = fscanf(stdin, "%lu", &x);
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
		if (x >= ULONG_MAX/3) {
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
	(void)printf("%lu\n", x);
	return EXIT_SUCCESS;
}
