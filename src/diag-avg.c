/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#define PATH_LENGTH 1024
#define BUFFER_LENGTH 256

#define DECIMAL_P(x) ( ('0' <= (x) && (x) <= '9') || (x) == '.')

static void
usage(void)
{
	diag_printf("diag-avg [-n num_of_trials] [-o path] command [operand ...]");
}

enum diag_avg_param_type {
	DIAG_AVG_PARAM_LONG   = 1,
	DIAG_AVG_PARAM_DOUBLE = 1<<1,
};

struct diag_avg_param {
	enum diag_avg_param_type type;
	union {
		long int l;
		double d;
	} value;
	unsigned int field_width;
	int precision;
};

static enum diag_avg_param_type
couple_to_param(struct diag_deque *head, struct diag_deque *tail, struct diag_avg_param *param)
{
	char c, *s, *r;
	struct diag_deque_elem *e;
	unsigned int len;
	int precision = -1;
	int i = 0;
	enum diag_avg_param_type type = DIAG_AVG_PARAM_LONG;

	len = head->length + tail->length;
	s = diag_malloc(len + 1);

#define FILLUP(q) do {							\
		DIAG_DEQUE_FOR_EACH(q, e) {				\
			c = (char)e->attr;				\
			switch (c) {					\
			case '.':					\
				type = DIAG_AVG_PARAM_DOUBLE;		\
				precision = 0;				\
				break;					\
			default:					\
				if (precision >= 0) precision++;	\
				break;					\
			}						\
			s[i++] = c;					\
		}							\
	} while (0)

	FILLUP(head);
	FILLUP(tail);
	s[len] = '\0';
	param->type = type;
	param->field_width = len;
	param->precision = precision;
	switch (type) {
	case DIAG_AVG_PARAM_LONG:
		param->value.l = atol(s);
		break;
	default:
		param->value.d = strtod(s, &r);
		break;
	}
	diag_free(s);
	return type;
}

static int
read_parameter(struct diag_port *port, struct diag_deque *head, struct diag_avg_param *param, uint8_t *xp)
{
	struct diag_deque *tail;
	uint8_t x;
	int r = 0;

	assert(port && head && param && xp);
	tail = diag_deque_new();
	do {
		x = *xp;
		if (DECIMAL_P(x)) {
			diag_deque_push(tail, (uintptr_t)x);
		} else {
			r = 1;
			break;
		}
	} while (port->read_byte(port, xp));
	couple_to_param(head, tail, param);
	diag_deque_destroy(tail);
	return r;
}

static long int
average_long(int n, struct diag_avg_param *param, char *buf)
{
	struct diag_avg_param *p;
	long int l = 0;
	int i;

	assert(n > 0 && param);
	for (i = 0; i < n; i++) {
		p = param+i;
		assert(p->type == DIAG_AVG_PARAM_LONG);
		l += p->value.l;
	}
	l /= n;
	sprintf(buf, "%ld", l);
	return l;
}

static double
average_double(int n, struct diag_avg_param *param, char *buf)
{
	struct diag_avg_param *p;
	double d = 0;
	unsigned int field_width = 0;
	int precision = 0;
	int i;

	assert(n > 0 && param);
	for (i = 0; i < n; i++) {
		p = param+i;
		switch (p->type) {
		case DIAG_AVG_PARAM_LONG:
			d += (double)p->value.l;
			break;
		default:
			d += p->value.d;
			if (field_width < p->field_width) field_width = p->field_width;
			if (precision < p->precision) precision = p->precision;
			break;
		}
	}
	d /= n;
	sprintf(buf, "%0*.*f", field_width, precision, d);
	return d;
}

static void
average_parameters(int n, struct diag_deque *q, struct diag_avg_param *param, char *buf)
{
	int i;
	unsigned int t = 0;

	assert(n > 0 && q && param && buf);
	for (i = 0; i < n; i++) {
		t |= (unsigned int)(param+i)->type;
	}
	if (t & DIAG_AVG_PARAM_DOUBLE) {
		average_double(n, param, buf);
	} else {
		average_long(n, param, buf);
	}
	for (i = 0; buf[i]; i++) {
		diag_deque_push(q, (uintptr_t)buf[i]);
	}
}

static void
run_files(char **paths, int n, int fd)
{
	struct diag_port **ports, *port, *fdport;
	uint8_t i, *x;
	int d, cont;
	struct diag_deque *q, *head;
	struct diag_deque_elem *e;
	struct diag_avg_param *param;
	char *buf;

	ports = diag_calloc(n, sizeof(struct diag_port *));
	for (i = 0; i < n; i++) {
		ports[i] = diag_port_new_path(paths[i], "rb");
	}
	port = ports[0];
	x = diag_malloc(n);
	q = diag_deque_new();
	head = diag_deque_new();
	cont = 0;
	param = diag_calloc(n, sizeof(struct diag_avg_param));
	buf = diag_malloc(BUFFER_LENGTH);
	while ( cont || port->read_byte(port, x) > 0 ) {
		if (DECIMAL_P(*x)) {
			diag_deque_push(head, (uintptr_t)*x);
		} else {
			diag_deque_append(q, head);
			diag_deque_push(q, (uintptr_t)*x);
		}
		d = 0;
		for (i = 1; i < n; i++) {
			if ( cont || ports[i]->read_byte(ports[i], x+i) > 0 ) {
				if (*x != x[i]) {
					if (DECIMAL_P(*x) && DECIMAL_P(x[i])) {
						d = 1;
					} else {
						diag_fatal("unexpected mismatch: '%c'[0] vs '%c'[%d]", *x, x[i], i);
					}
				}
			} else {
				diag_fatal("unexpected eof: %d", i);
			}
		}
		if (d) {
			assert(head->length > 0);
			(void)diag_deque_pop(head);
			cont = read_parameter(port, head, param, x);
			for (i = 1; i < n; i++) {
				if (cont != read_parameter(ports[i], head, param+i, x+i)) {
					diag_fatal("unexpected eof: %d", i);
				}
			}
			average_parameters(n, q, param, buf);
			while ( (e = diag_deque_shift(head)) ) diag_free(e);
		} else {
			cont = 0;
		}
	}
	fdport = diag_port_new_fd(fd, DIAG_PORT_OUTPUT);
	while ( (e = diag_deque_shift(q)) ) {
		fdport->write_byte(fdport, (uint8_t)e->attr);
		diag_free(e);
	}
	diag_port_destroy(fdport);
	diag_free(buf);
	diag_free(param);
	diag_deque_destroy(head);
	diag_deque_destroy(q);
	diag_free(x);
	for (i = 0; i < n; i++) {
		diag_port_destroy(ports[i]);
	}
	diag_free(ports);
}

static void
collect_children(struct diag_rbtree *ptree)
{
	struct diag_rbtree_node *pnode;
	pid_t pid;

	assert(ptree);
	while ( (pnode = diag_rbtree_minimum(ptree)) ) {
		pid = (pid_t)pnode->key;
		waitpid(pid, NULL, 0);
		diag_rbtree_delete(ptree, pnode);
	}
}

static void
build_path(char **paths, int i, const char *format, char *dir, pid_t pid)
{
	int len;

	assert(paths && format);
	paths[i] = diag_malloc(PATH_LENGTH);
	len = sprintf(paths[i], format, dir, pid);
	if (len < 0) {
		diag_fatal("fail to build path");
	} else if (PATH_LENGTH <= len) {
		diag_fatal("exceed PATH_LENGTH");
	}
}

int
main(int argc, char *argv[])
{
	int c, i, n = NUMBER_OF_TRIALS;
	int leave_output = 0;
	pid_t pid;
	struct diag_rbtree *ptree;
	struct diag_rbtree_node *pnode;
	int status, *stable;
	char *dir = NULL;
	char **opaths, **epaths;

#define CHECK_DIR() do {						\
		struct stat st;						\
		if (!dir) {						\
			diag_fatal("could not allocate memory");	\
		}							\
		if (stat(dir, &st) < 0) {				\
			diag_fatal("could not stat %s", dir);		\
		}							\
		if (!S_ISDIR(st.st_mode)) {				\
			diag_fatal("%s is not a directory", dir);	\
		}							\
	} while (0)

	while ( (c = getopt(argc, argv, "+Vhn:o:")) >= 0) {
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
	opaths = diag_calloc(n << 1, sizeof(char *));
	epaths = opaths + n;
	if (!dir) {
		char *tmpdir;

		tmpdir = getenv("TMPDIR");
		dir = strdup((tmpdir) ? tmpdir : ".");
		CHECK_DIR();
	}

#define FAIL() do {				\
		collect_children(ptree);	\
		while (i--) {			\
			diag_free(opaths[i]);	\
			diag_free(epaths[i]);	\
		}				\
		diag_free(opaths);		\
		exit(EXIT_FAILURE);		\
	} while (0)

	for (i = 0; i < n; i++) {

#define BUILD_PATHS() do {						\
			build_path(opaths, i, "%s/diagonal%d.out", dir, pid); \
			build_path(epaths, i, "%s/diagonal%d.err", dir, pid); \
		} while (0)

#define FREE_PATHS() do {					\
			for (i = 0; i < n; i++) {		\
				if (!leave_output) {		\
					unlink(opaths[i]);	\
					unlink(epaths[i]);	\
				}				\
				diag_free(opaths[i]);		\
				diag_free(epaths[i]);		\
			}					\
			diag_free(opaths);			\
		} while (0)

		pid = fork();
		if (pid < 0) {
			FAIL();
		} else if (pid == 0) {
			pid = getpid();
			BUILD_PATHS();
			diag_free(dir);
			diag_rbtree_destroy(ptree);
			if (!freopen(opaths[i], "wb", stdout)) {
				_Exit(EXIT_FAILURE);
			}
			if (!freopen(epaths[i], "wb", stderr)) {
				_Exit(EXIT_FAILURE);
			}
			if (execvp(argv[optind], argv+optind) == -1) {
				_Exit(EXIT_FAILURE);
			}
		} else {
			pnode = diag_rbtree_node_new((uintptr_t)pid, (uintptr_t)i);
			diag_rbtree_insert(ptree, pnode);
			BUILD_PATHS();
		}
	}
	diag_free(dir);

	stable = diag_calloc(n, sizeof(int));
	do {
		pid = waitpid((pid_t)-1, &status, WCONTINUED|WNOHANG|WUNTRACED);
		if (pid == (pid_t)0) {
			continue;
		} else if (pid == (pid_t)-1) {
			exit(EXIT_FAILURE);
		}

#define DETERMINE_STATUS() do {						\
			if (diag_rbtree_search(ptree, (uintptr_t)pid, &pnode)) { \
				i = (int)pnode->attr;			\
				diag_rbtree_delete(ptree, pnode);	\
				stable[i] = status;			\
			} else {					\
				FAIL();					\
			}						\
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
