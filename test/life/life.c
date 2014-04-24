/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_input(size_t height, size_t width, char *bbox)
{
	size_t i;
	size_t j;
	int c;
	for (i=0;i<height;i++) {
		/* read a row */
		if (fread(bbox+i*width, width, 1, stdin) != 1) {
			fprintf(stderr, "failed to read row %lu\n", i);
			return 0;
		}

		/* validate content of the row */
		for (j=0;j<width;j++) {
			char b = bbox[i*width+j];
			if (b != '.' && b != 'X') {
				fprintf(stderr, "invalid row: ");
				fwrite(bbox+i*width, width, 1, stderr);
				fputc('\n', stderr);
				return 0;
			}
		}

		/* skip the rest of the line */
		do {
			c = fgetc(stdin);
			if (c == EOF) {
				if (i == height-1) {
					return 1;
				}
				fprintf(stderr, "failed to read row %lu\n", i);
				return 0;
			}
		} while (c != '\r' && c != '\n');
		do {
			c = fgetc(stdin);
			if (c == EOF) {
				if (i == height-1) {
					return 1;
				}
				fprintf(stderr, "failed to read row %lu\n", i);
				return 0;
			}
		} while (c == '\r' || c == '\n');
		if (ungetc(c, stdin) == EOF) {
			fprintf(stderr, "failed to ungetc at row %lu\n", i);
			return 0;
		}
	}
	return 1;
}

static void usage(void)
{
	fprintf(stderr, "usage: life WIDTH HEIGHT\n");
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		usage();
		return EXIT_FAILURE;
	}

	long h = atol(argv[1]);
	if (h <= 0) {
		fprintf(stderr, "invalid height: %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	long w = atol(argv[2]);
	if (w <= 0) {
		fprintf(stderr, "invalid width: %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	size_t height = (size_t)h;
	size_t width = (size_t)w;

	char *bbox = calloc((size_t)height, (size_t)width);
	if (!bbox) {
		fprintf(stderr, "failed to calloc\n");
		return EXIT_FAILURE;
	}

	int r = EXIT_FAILURE;
	if (!read_input(height, width, bbox)) {
		goto bail;
	}

	size_t i;
	size_t j;
	int c;
	for (i=0;i<height;i++) {
		for (j=0;j<width;j++) {

			int n = 0;
			if (0 < i && 0 < j && bbox[(i-1)*width+(j-1)] == 'X') n++;
			if (0 < i && bbox[(i-1)*width+j] == 'X') n++;
			if (0 < i && j+1 < width && bbox[(i-1)*width+(j+1)] == 'X') n++;
			if (0 < j && bbox[i*width+(j-1)] == 'X') n++;
			if (j+1 < width && bbox[i*width+(j+1)] == 'X') n++;
			if (i+1 < height && 0 < j && bbox[(i+1)*width+(j-1)] == 'X') n++;
			if (i+1 < height && bbox[(i+1)*width+j] == 'X') n++;
			if (i+1 < height && j+1 < width && bbox[(i+1)*width+(j+1)] == 'X') n++;

			if (bbox[i*width+j] == 'X') { /* "alive" */
				c = (n == 2 || n == 3) ? 'X' : '.';
			} else { /* "dead" */
				c = (n == 3) ? 'X' : '.';
			}
			if (putchar(c) == EOF) {
				fprintf(stderr, "an error on writing\n");
				goto bail;
			}
		}
		if (putchar('\n') == EOF) {
			fprintf(stderr, "an error on writing\n");
			goto bail;
		}
	}
	r = EXIT_SUCCESS;

 bail:
	free(bbox);
	return r;
}
