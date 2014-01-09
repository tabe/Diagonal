/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/bytevector.h"
#include "diagonal/port.h"
#include "diagonal/hash.h"
#include "diagonal/vcdiff.h"

static void
usage(void)
{
	diag_printf("diag-dec [-s source] [-t target] [file]");
}

int
main(int argc, char *argv[])
{
	int c;
	const char *path_source = NULL;
	const char *path_target = NULL;
	struct diag_vcdiff *vcdiff;
	struct diag_vcdiff_context *context;
	struct diag_vcdiff_vm *vm;

	diag_init();

	while ( (c = getopt(argc, argv, "Vhs:t:")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 's':
			path_source = optarg;
			break;
		case 't':
			path_target = optarg;
			break;
		default:
			break;
		}
	}
	if (argv[optind]) {
		context = diag_vcdiff_context_new_path(argv[optind]);
	} else {
		context = diag_vcdiff_context_new_fd(STDIN_FILENO);
	}
	if (!context) {
		diag_fatal("could not make context");
	}
	context->compatibility = 1;
	vcdiff = diag_vcdiff_read(context);
	if (!vcdiff) {
		diag_fatal("could not read context");
	}
	vm = diag_vcdiff_vm_new_path(path_source);
	if (!vm) {
		diag_fatal("could not create vm");
	}
	if (diag_vcdiff_decode(vm, vcdiff)) {
		int fd;
		struct diag_port *port;
		ssize_t s;

		if (path_target) {
			fd = open(path_target, O_CREAT|O_WRONLY|O_TRUNC, S_IWUSR|S_IRUSR
#ifdef S_IRGRP
				  |S_IRGRP
#endif
#ifdef S_IROTH
				  |S_IROTH
#endif
				  );
			if (fd < 0) {
				perror(strerror(errno));
				exit(EXIT_FAILURE);
			}
		} else {
			fd = STDOUT_FILENO;
		}
		port = diag_port_new_fd(fd, DIAG_PORT_OUTPUT);
		s = port->write_bytes(port, vm->s_target, vm->target);
		diag_port_destroy(port);
		close(fd);
		if (s < 0) {
			exit(EXIT_FAILURE);
		}
	} else {
		diag_fatal("could not decode vcdiff");
	}
	diag_vcdiff_vm_destroy(vm);
	diag_vcdiff_destroy(vcdiff);
	diag_vcdiff_context_destroy(context);
	return EXIT_SUCCESS;
}
