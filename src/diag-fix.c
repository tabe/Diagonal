#include "config.h"

#include <assert.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"

#define BUFFER_LENGTH 1024

static void
usage(void)
{
	diag_info("diag-fix command [operand ...]");
}

static void
execute_program(char *const argv[], int ifd[2], int ofd[2])
{
	int r;

	close(ifd[1]);
	close(ofd[0]);
	if (fcntl(ifd[0], F_SETFD, FD_CLOEXEC) < 0) {
		_Exit(EXIT_FAILURE);
	}
	r = dup2(ifd[0], STDIN_FILENO);
	if (r < 0) {
		_Exit(EXIT_FAILURE);
	}
	if (fcntl(ofd[1], F_SETFD, FD_CLOEXEC) < 0) {
		_Exit(EXIT_FAILURE);
	}
	r = dup2(ofd[1], STDOUT_FILENO);
	if (r < 0) {
		_Exit(EXIT_FAILURE);
	}
	if (execvp(argv[0], argv) == -1) {
		_Exit(EXIT_FAILURE);
	}
}

int
main(int argc, char *argv[])
{
	int c, d, r, status;
	pid_t pid1, pid2;
	int ifd1[2], ifd2[2];
	int ofd1[2], ofd2[2];
	char *m1, *m2;
	size_t s, s1, s2;
	ssize_t ss;

	while ( (c = getopt(argc, argv, "+Vh")) >=0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		default:
			break;
		}
	}
	if (!argv[optind]) {
		usage();
		exit(EXIT_FAILURE);
	}

	/* 1st */
	r = pipe(ifd1);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}
	r = pipe(ofd1);
	if (r < 0) {
		close(ifd1[0]);
		close(ifd1[1]);
		exit(EXIT_FAILURE);
	}
	pid1 = fork();
	if (pid1 < 0) {
		exit(EXIT_FAILURE);
	} else if (pid1 == 0) { /* child */
		execute_program(argv+optind, ifd1, ofd1);
	} else { /* parent */
		close(ofd1[1]);
		close(ifd1[0]);
		s1 = BUFFER_LENGTH;
		m1 = (char *)diag_malloc(s1);
		s = 0;
		while (read(STDIN_FILENO, (void *)m1+s, 1) > 0) {
			write(ifd1[1], (void *)m1+s, 1);
			if (++s == s1) {
				s1 += BUFFER_LENGTH;
				m1 = diag_realloc(m1, s1);
			}
		}
		s1 = s;
		m1 = (char *)diag_realloc((void *)m1, s1);
		close(ifd1[1]);
	}

#define RUN(x, y) do {													\
		waitpid(pid##y, &status, 0);									\
		if (!WIFEXITED(status)) {										\
			exit(EXIT_FAILURE);											\
		}																\
		if (WEXITSTATUS(status) != EXIT_SUCCESS) {						\
			exit(WEXITSTATUS(status));									\
		}																\
		r = pipe(ifd##x);												\
		if (r < 0) {													\
			close(ofd##y[0]);											\
			close(ofd##y[1]);											\
			exit(EXIT_FAILURE);											\
		}																\
		r = pipe(ofd##x);												\
		if (r < 0) {													\
			close(ofd##y[0]);											\
			close(ofd##y[1]);											\
			close(ifd##x[0]);											\
			close(ifd##x[1]);											\
			exit(EXIT_FAILURE);											\
		}																\
		pid##x = fork();												\
		if (pid##x < 0) {												\
			exit(EXIT_FAILURE);											\
		} else if (pid##x == 0) { /* child */							\
			execute_program(argv+optind, ifd##x, ofd##x);				\
		} else { /* parent */											\
			close(ofd##x[1]);											\
			close(ifd##x[0]);											\
			s##x = BUFFER_LENGTH;										\
			m##x = (char *)diag_malloc(s##x);							\
			s = 0;														\
			while (read(ofd##y[0], (void *)m##x+s, 1) > 0) {			\
				if (write(ifd##x[1], (void *)m##x+s, 1) < 1) {			\
					exit(EXIT_FAILURE);									\
				}														\
				if (++s == s##x) {										\
					s##x += BUFFER_LENGTH;								\
					m##x = (char *)diag_realloc((void *)m##x, s##x);	\
				}														\
			}															\
			close(ifd##x[1]);											\
			s##x = s;													\
			m##x = (char *)diag_realloc((void *)m##x, s##x);			\
			close(ofd##y[0]);											\
			if (s##x == s##y) {											\
				d = 0;													\
				for (s = 0; s < s##x; s++) {							\
					if (m##x[s] != m##y[s]) {							\
						d = 1;											\
						break;											\
					}													\
				}														\
				if (!d) {												\
					kill(pid##x, SIGINT);								\
					waitpid(pid##x, NULL, 0);							\
					ss = write(STDOUT_FILENO, (void *)m##x, s##x);		\
					if (ss < 0 || (size_t)ss < s##x) {					\
						exit(EXIT_FAILURE);								\
					}													\
					exit(EXIT_SUCCESS);									\
				}														\
			}															\
		}																\
	} while (0)

 loop:
	/* 2nd */
	RUN(2, 1);
	/* 3rd */
	RUN(1, 2);
	goto loop;

	return EXIT_SUCCESS;
}