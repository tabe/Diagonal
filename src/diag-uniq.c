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
#include "diagonal/port.h"
#include "diagonal/private/filesystem.h"

static void usage(void)
{
	diag_printf("diag-uniq [-c count] [-o output] file");
}

int main(int argc, char *argv[])
{
	diag_init();

	int c;
	long long count = 0;
	size_t size;
	const char *output = NULL;
	char *buf, *p, *q;
	struct diag_port *port;

	if (argc < 2) {
		usage();
		exit(EXIT_FAILURE);
	}
	while ( (c = getopt(argc, argv, "Vc:ho:")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'c':
			count = atoll(optarg);
			if (count <= 0) diag_fatal("count should be positive");
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'o':
			output = optarg;
			break;
		case '?':
			usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}
	if (count == 0) {
		diag_fatal("please specify the count with the -c option");
	}
	struct diag_mmap *mm = diag_mmap_file(argv[optind], DIAG_MMAP_RO);
	buf = mm->addr;
	size = mm->size;
	if (output) {
		port = diag_port_new_path(output, "wb");
	} else {
		port = diag_port_new_fp(stdout, DIAG_PORT_OUTPUT);
	}
	if (size == 0) {
		goto done;
	} else if (size <= (size_t)count) {
		port->write_bytes(port, size, (const uint8_t *)buf);
		goto done;
	}
	lldiv_t d = lldiv((long long)size, count);
	long long n = d.quot;
	long long r = d.rem;
	port->write_bytes(port, (size_t)count, (const uint8_t *)buf);
	p = buf;
	q = buf + count;
	long long i;
	for (i = 1; i < n; i++) {
		if (memcmp(p, q, (size_t)count) != 0) {
			port->write_bytes(port, (size_t)count, (const uint8_t *)q);
		}
		p = q;
		q += count;
	}
	if (r > 0) port->write_bytes(port, (size_t)r, (const uint8_t *)p);
 done:
	diag_port_destroy(port);
	diag_munmap(mm);
	return EXIT_SUCCESS;
}
