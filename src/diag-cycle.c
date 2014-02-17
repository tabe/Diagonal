/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/cmp.h"
#include "diagonal/deque.h"
#include "diagonal/hash.h"
#include "diagonal/port.h"
#include "diagonal/rbtree.h"
#include "diagonal/vector.h"
#include "diagonal/private/filesystem.h"
#include "diagonal/private/system.h"
#include "diagonal/private/temporary-file.h"

#define BASE 15485863

static int hash(const char *file, uint32_t *hp)
{
	struct diag_mmap *mm = diag_mmap_file(file, DIAG_MMAP_RO);
	if (!mm) return 0;
	uint32_t h = diag_hash32_rabin_karp(mm->addr, mm->size, BASE);
	diag_munmap(mm);
	*hp = h;
	return 1;
}

static void insert(size_t n, uint32_t h, struct diag_rbtree *tree)
{
	struct diag_deque *q = diag_deque_new();
	(void)diag_deque_push(q, (intptr_t)n);
	struct diag_rbtree_node *node;
	node = diag_rbtree_node_new((uintptr_t)h, (uintptr_t)q);
	(void)diag_rbtree_insert(tree, node);
}

static int find_or_insert(size_t n, uint32_t h,
			    struct diag_rbtree *tree,
			    struct diag_deque **qp)
{
	struct diag_rbtree_node *node = NULL;
	int found = diag_rbtree_search(tree, h, &node);
	if (found) {
		assert(node);
		assert(node->attr);
		*qp = (struct diag_deque *)node->attr;
		return found;
	}
	insert(n, h, tree);
	return 0;
}

static void free_entries(uintptr_t attr, void *data)
{
	(void)data;
	struct diag_deque *dq = (struct diag_deque *)attr;
	diag_deque_destroy(dq);
}

static void usage(void)
{
	diag_printf("diag-cycle [-i input] [-o output] command [operand ...]");
}

int main(int argc, char *argv[])
{
	int c;
	int r = EXIT_FAILURE;
	intptr_t leave_output = 0;
	char *in = NULL;
	char *dir = NULL;

	diag_init();

	while ( (c = getopt(argc, argv, "+Vhi:o:")) >=0) {
		switch (c) {
		case 'V':
			diag_print_version();
			exit(EXIT_SUCCESS);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			in = optarg;
			break;
		case 'o':
			leave_output = 1;
			dir = diag_strdup(optarg);
			diag_assert_directory(dir);
			break;
		case ':':
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

	struct diag_rbtree *tree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);

	struct diag_deque *q = diag_deque_new();
	struct diag_deque_elem *e;

	if (!in) {
		struct diag_temporary_file *tf = diag_temporary_file_new();
		if (!tf) {
			goto done;
		}
		in = diag_strdup(tf->path);
		diag_deque_push(q, (intptr_t)in);
		struct diag_port *ip = diag_port_new_stdin();
		assert(ip);
		ssize_t s = diag_port_copy(ip, tf->port);
		diag_port_destroy(ip);
		diag_temporary_file_destroy(tf);
		if (s < 0) {
			goto done;
		}
	}

	struct diag_vector *v = diag_vector_create(1);
	diag_vector_set(v, 0, (intptr_t)in);

	size_t n = 0;

	uint32_t h;
	int hashed;
	hashed = hash(in, &h);
	if (!hashed) {
		goto done;
	}
	insert(n, h, tree);

	char *file;
	struct diag_command *cmd;
 run:
	file = (char *)diag_vector_ref(v, n++);
	assert(file);
	cmd = diag_command_new(argv+optind, dir, file, NULL, NULL);
	if (!cmd) {
		goto done;
	}
	struct diag_process *p = diag_run_program(cmd);
	if (!p) {
		goto done;
	}
	diag_process_wait(p);
	int status = p->status;
	diag_process_destroy(p);
	if (status != 0) {
		r = status;
		goto done;
	}

	file = (char *)diag_strdup(cmd->out);
	diag_vector_push_back(v, (intptr_t)file);
	diag_deque_push(q, (intptr_t)file);
	diag_deque_push(q, (intptr_t)diag_strdup(cmd->err));

	hashed = hash(cmd->out, &h);
	if (!hashed) {
		goto done;
	}
	struct diag_deque *dq;
	int found = find_or_insert(n, h, tree, &dq);
	if (!found) {
		diag_command_destroy(cmd);
		goto run;
	}

	struct diag_deque_elem *dqe;
	DIAG_DEQUE_FOR_EACH(dq, dqe) {
		struct diag_port *pp;
		struct diag_port *cp;
		size_t k = (size_t)dqe->attr;
		file = (char *)diag_vector_ref(v, k);
		pp = diag_port_new_path(file, "rb");
		if (!pp) {
			goto done;
		}
		cp = diag_port_new_path(cmd->out, "rb");
		if (!cp) {
			diag_port_destroy(pp);
			goto done;
		}
		int d = diag_port_diff(pp, cp);
		diag_port_destroy(cp);
		diag_port_destroy(pp);
		if (d == 1) {
			diag_deque_push(dq, (intptr_t)n);
			diag_command_destroy(cmd);
			goto run;
		}
		if (d == -1) {
			goto done;
		}

		diag_printf("%ld %ld\n", k, n-k);
		if (!leave_output) {
			DIAG_DEQUE_FOR_EACH(q, e) {
				file = (char *)e->attr;
				assert(file);
				remove(file);
			}
		}
		r = EXIT_SUCCESS;
		goto done;
	}
	diag_error("inconsistency with hashing or reading file: %s", cmd->out);

 done:
	diag_command_destroy(cmd);
	diag_vector_destroy(v);
	diag_rbtree_for_each_attr(tree, free_entries, NULL);
	diag_rbtree_destroy(tree);
	DIAG_DEQUE_FOR_EACH(q, e) {
		diag_free((void *)e->attr);
	}
	diag_deque_destroy(q);
	diag_free(dir);
	return r;
}
