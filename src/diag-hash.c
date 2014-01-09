/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/hash.h"
#include "diagonal/port.h"
#include "diagonal/private/filesystem.h"

static void usage(void)
{
	diag_printf("diag-hash [-b base] [-o output] [-s] [-w window] file");
}

static int cmp(const void *x, const void *y)
{
	const uint32_t *a, *b;
	a = (const uint32_t *)x;
	b = (const uint32_t *)y;
	return *a - *b;
}

int main(int argc, char *argv[])
{
	int c, ordered = 0;
	uint32_t base = 107;
	size_t len, s_window = 10;
	const char *output = NULL;
	struct diag_port *port;
	struct diag_rollinghash32 *rh;
	uint32_t *result;

	diag_init();

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "Vb:ho:sw:")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'b':
			base = (uint32_t)atoi(optarg);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'o':
			output = optarg;
			break;
		case 's':
			ordered = 1;
			break;
		case 'w':
			s_window = (size_t)atoi(optarg);
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}
	struct diag_mmap *mm = diag_mmap_file(argv[optind], DIAG_MMAP_RO);
	if (!mm) diag_fatal("could not map file: %s", argv[optind]);
	if (mm->size == 0) {
		diag_munmap(mm);
		diag_fatal("file is empty: %s", argv[optind]);
	}
	rh = diag_rollinghash32_new_rabin_karp((const uint8_t *)mm->addr, mm->size, s_window, base);
	result = diag_rollinghash32_collect(rh, &len);
	if (ordered) {
		qsort(result, len, sizeof(*result), cmp);
	}
	if (output) {
		port = diag_port_new_path(output, "wb");
	} else {
		port = diag_port_new_fp(stdout, DIAG_PORT_OUTPUT);
	}
	port->write_bytes(port, len * sizeof(*result), (const uint8_t *)result);
	diag_port_destroy(port);
	diag_free(result);
	diag_rollinghash32_destroy(rh);
	diag_munmap(mm);
	return EXIT_SUCCESS;
}
