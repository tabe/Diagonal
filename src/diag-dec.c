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
#include "diagonal/port.h"
#include "diagonal/vcdiff.h"

static void
usage(void)
{
	diag_info("diag-dec [-s source] [-t target] [path]");
}

int
main(int argc, char *argv[])
{
	int c;
	const char *path_source = NULL;
	const char *path_target = NULL;
	diag_vcdiff_t *vcdiff;
	diag_vcdiff_context_t *context;
	diag_vcdiff_vm_t *vm;

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
		ssize_t s;

		if (path_target) {
			fd = open(path_target, O_CREAT|O_WRONLY|O_TRUNC, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
			if (fd < 0) {
				perror(strerror(errno));
				return 1;
			}
		} else {
			fd = STDOUT_FILENO;
		}
		s = write(fd, vm->target, vm->s_target);
		if (s < 0) {
			perror(strerror(errno));
			return 1;
		}
		close(fd);
	} else {
		diag_fatal("could not decode vcdiff");
	}
	diag_vcdiff_vm_destroy(vm);
	diag_vcdiff_destroy(vcdiff);
	diag_vcdiff_context_destroy(context);
	return EXIT_SUCCESS;
}
