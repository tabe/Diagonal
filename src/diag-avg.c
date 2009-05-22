#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
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
#include "diagonal/deque.h"
#include "diagonal/port.h"
#include "diagonal/rbtree.h"

#define NUMBER_OF_TRIALS 5
#define PATH_LENGTH 256
#define BUFFER_LENGTH 256

#define DECIMAL_P(x) ( ('0' <= (x) && (x) <= '9') || (x) == '.')

static void
usage(void)
{
	diag_info("diag-avg [-n num_of_trials] [-o path] command [operand ...]");
}

static int
skip_parameter(diag_port_t *port, diag_deque_t *head, double *vp, uint8_t *xp)
{
	diag_deque_t *tail;
	uint8_t x;

	assert(port && tail && vp && xp);
	tail = diag_deque_new();

#define COUPLE_TO_VALUE() do {					\
		char *s, *r;							\
		diag_deque_elem_t *e;					\
		unsigned int len, i = 0;				\
												\
		len = head->length + tail->length;		\
		s = (char *)diag_malloc(len + 1);		\
		DIAG_DEQUE_FOR_EACH(head, e) {			\
			s[i++] = (char)e->attr;				\
		}										\
		DIAG_DEQUE_FOR_EACH(tail, e) {			\
			s[i++] = (char)e->attr;				\
		}										\
		s[len] = '\0';							\
		*vp = strtod(s, &r);					\
		diag_free(s);							\
	} while (0)

	while (port->read_byte(port, xp)) {
		x = *xp;
		if (DECIMAL_P(x)) {
			diag_deque_push(tail, (uintptr_t)x);
			continue;
		}
		COUPLE_TO_VALUE();
		diag_deque_destroy(tail);
		return 1;
	}
	COUPLE_TO_VALUE();
	diag_deque_destroy(tail);
	return 0;
}

static void
run_files(char **paths, int n, int fd)
{
	diag_port_t **ports, *port;
	uint8_t i, *x;
	int d, cont;
	diag_deque_t *q, *head;
	diag_deque_elem_t *e;
	double value, *values;
	char *buf;

	ports = (diag_port_t **)diag_calloc(n, sizeof(diag_port_t *));
	for (i = 0; i < n; i++) {
		ports[i] = diag_port_new_path(paths[i], "rb");
	}
	port = ports[0];
	x = (uint8_t *)diag_malloc(n);
	q = diag_deque_new();
	head = diag_deque_new();
	cont = 0;
	values = (double *)diag_calloc(n, sizeof(double));
	buf = (char *)diag_malloc(BUFFER_LENGTH);
	while ( cont || port->read_byte(port, x) > 0 ) {
		if (DECIMAL_P(*x)) {
			diag_deque_push(head, (uintptr_t)*x);
		} else {
			while ( (e = diag_deque_shift(head)) ) {
				diag_deque_push(q, e->attr);
				diag_free(e);
			}
			diag_deque_push(q, (uintptr_t)*x);
		}
		d = 0;
		for (i = 1; i < n; i++) {
			if ( cont || ports[i]->read_byte(ports[i], x+i) > 0 ) {
				if (*x != x[i]) {
					if (DECIMAL_P(*x) && DECIMAL_P(x[i])) {
						d = 1;
					} else {
						diag_fatal("unexpected mismatch: %c: %c", *x, x[i]);
					}
				}
			} else {
				diag_fatal("unexpected eof");
			}
		}
		if (d) {
			cont = skip_parameter(port, head, values, x);
			for (i = 1; i < n; i++) {
				if (cont != skip_parameter(ports[i], head, values+i, x+i)) {
					diag_fatal("unexpected eof");
				}
			}
			value = 0;
			for (i = 0; i < n; i++) {
				value += values[i];
			}
			value /= n;
			sprintf(buf, "%f", value);
			for (i = 0; buf[i]; i++) {
				diag_deque_push(q, (uintptr_t)buf[i]);
			}
			while ( (e = diag_deque_shift(head)) ) diag_free(e);
		} else {
			cont = 0;
		}
	}
	while ( (e = diag_deque_shift(q)) ) {
		write(fd, &e->attr, 1);
		diag_free(e);
	}
	diag_free(buf);
	diag_free(values);
	diag_deque_destroy(head);
	diag_deque_destroy(q);
	diag_free(x);
	for (i = 0; i < n; i++) {
		diag_port_destroy(ports[i]);
	}
	diag_free(ports);
}

static void
collect_children(diag_rbtree_t *ptree)
{
	diag_rbtree_node_t *pnode;
	pid_t pid;

	assert(ptree);
	while ( (pnode = diag_rbtree_minimum(ptree)) ) {
		pid = (pid_t)pnode->key;
		waitpid(pid, NULL, 0);
		diag_rbtree_delete(ptree, pnode);
	}
}

int
main(int argc, char *argv[])
{
	int c, i, n = NUMBER_OF_TRIALS;
	int leave_output = 0;
	pid_t pid;
	diag_rbtree_t *ptree;
	diag_rbtree_node_t *pnode;
	int status, *stable;
	char *dir = NULL;
	char **opaths, **epaths;

#define CHECK_DIR() do {								\
		struct stat st;									\
		if (!dir) {										\
			diag_fatal("could not allocate memory");	\
		}												\
		if (stat(dir, &st) < 0) {						\
			diag_fatal("could not stat %s", dir);		\
		}												\
		if (!S_ISDIR(st.st_mode)) {						\
			diag_fatal("%s is not a directory", dir);	\
		}												\
	} while (0)

	while ( (c = getopt(argc, argv, "Vhn:o:")) >= 0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'n':
			n = atoi(optarg);
			if (n < 0) {
				diag_fatal("non-negative integer expected, but %d", n);
			} else if (n == 0) {
				exit(EXIT_SUCCESS);
			}
			break;
		case 'o':
			leave_output = 1;
			dir = strdup(optarg);
			CHECK_DIR();
			break;
		default:
			break;
		}
	}

	assert(n > 0);
	ptree = diag_rbtree_new(DIAG_RBTREE_IMMEDIATE);
	opaths = (char **)diag_calloc(n << 1, sizeof(char *));
	epaths = opaths + n;
	if (!dir) {
		char *tmpdir;

		tmpdir = getenv("TMPDIR");
		dir = strdup((tmpdir) ? tmpdir : ".");
		CHECK_DIR();
	}

#define FAIL() do {								\
		collect_children(ptree);				\
		while (i--) {							\
			diag_free(opaths[i]);				\
			diag_free(epaths[i]);				\
		}										\
		diag_free(opaths);						\
		exit(EXIT_FAILURE);						\
	} while (0)

	for (i = 0; i < n; i++) {

#define BUILD_PATHS() do {												\
			opaths[i] = (char *)diag_calloc(PATH_LENGTH, 1);			\
			(void)sprintf(opaths[i], "%s/diagonal%d.out", dir, pid);	\
			epaths[i] = (char *)diag_calloc(PATH_LENGTH, 1);			\
			(void)sprintf(epaths[i], "%s/diagonal%d.err", dir, pid);	\
		} while (0)

#define FREE_PATHS() do {						\
			for (i = 0; i < n; i++) {			\
				if (!leave_output) {			\
					unlink(opaths[i]);			\
					unlink(epaths[i]);			\
				}								\
				diag_free(opaths[i]);			\
				diag_free(epaths[i]);			\
			}									\
			diag_free(opaths);					\
		} while (0)

		pid = fork();
		if (pid < 0) {
			FAIL();
		} else if (pid == 0) {
			pid = getpid();
			BUILD_PATHS();
			diag_free(dir);
			diag_rbtree_destroy(ptree);
			(void)freopen(opaths[i], "wb", stdout);
			(void)freopen(epaths[i], "wb", stderr);
			if (execvp(argv[optind], argv+optind) == -1) {
				_Exit(EXIT_FAILURE);
			}
		} else {
			pnode = diag_rbtree_node_new((diag_rbtree_key_t)pid, (diag_rbtree_attr_t)i);
			diag_rbtree_insert(ptree, pnode);
			BUILD_PATHS();
		}
	}
	diag_free(dir);

	stable = (int *)diag_calloc(n, sizeof(int));
	do {
		pid = waitpid((pid_t)-1, &status, WCONTINUED|WNOHANG|WUNTRACED);
		if (pid == (pid_t)0) {
			continue;
		} else if (pid == (pid_t)-1) {
			exit(EXIT_FAILURE);
		}

#define DETERMINE_STATUS() do {											\
			if (diag_rbtree_search(ptree, (diag_rbtree_key_t)pid, &pnode)) { \
				i = (int)pnode->attr;									\
				diag_rbtree_delete(ptree, pnode);						\
				stable[i] = status;										\
			} else {													\
				FAIL();													\
			}															\
		} while (0)

		if (WIFEXITED(status)) {
			DETERMINE_STATUS();
		} else if (WIFSIGNALED(status)) {
			DETERMINE_STATUS();
		} else if (WIFSTOPPED(status)) {
			
		} else if (WIFCONTINUED(status)) {
			
		}
	} while (diag_rbtree_minimum(ptree));

	run_files(opaths, n, STDOUT_FILENO);
	run_files(epaths, n, STDERR_FILENO);

	diag_free(stable);
	FREE_PATHS();
	diag_rbtree_destroy(ptree);
	return EXIT_SUCCESS;
}
