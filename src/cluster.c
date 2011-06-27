/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/cluster.h"

struct diag_cluster *
diag_cluster_new(size_t num_data)
{
	struct diag_cluster *cluster;
	size_t size;

	size = (size_t)(sizeof(struct diag_cluster)+num_data*sizeof(struct diag_datum *));
	cluster = diag_malloc(size);
	cluster->num_data = num_data;
	return cluster;
}

void
diag_cluster_destroy(struct diag_cluster *cluster)
{
	diag_free(cluster);
}

struct diag_analysis *
diag_analysis_new(size_t num_data, struct diag_datum **data)
{
	struct diag_analysis *analysis;
	size_t size;

	size = (size_t)(sizeof(struct diag_analysis)+num_data*sizeof(struct diag_code *));
	analysis = diag_malloc(size);
	analysis->num_data = num_data;
	analysis->data = data;
	analysis->metric = NULL;
	analysis->encoder = NULL;
	analysis->decoder = NULL;
	analysis->clusters = NULL;
	return analysis;
}

void
diag_analysis_destroy(struct diag_analysis *analysis)
{
	diag_free(analysis);
}

struct diag_code *
diag_code_new(struct diag_cluster *cluster, size_t num_deltas)
{
	struct diag_code *code;
	size_t size;

	size = (size_t)(sizeof(struct diag_code)+num_deltas*sizeof(struct diag_delta *));
	code = diag_malloc(size);
	code->cluster = cluster;
	code->num_deltas = num_deltas;
	return code;
}

void
diag_code_destroy(struct diag_code *datum)
{
	diag_free(datum);
}

struct diag_delta *
diag_delta_new(enum diag_delta_type type)
{
	struct diag_delta *delta;

	delta = diag_malloc(sizeof(struct diag_delta));
	delta->type = type;
	return delta;
}

void
diag_delta_destroy(struct diag_delta *delta)
{
	diag_free(delta);
}

struct diag_code *
diag_delta_hamming_chars(struct diag_cluster *cluster, const char *x, const char *y)
{
	struct diag_deque *deque;
	struct diag_deque_elem *elem;
	struct diag_delta *d;
	struct diag_code *code;
	size_t i = 0;

	assert(x && y);
	if (x == y) return NULL;
	deque = diag_deque_new();
	for (;; i++, x++, y++) {
		if (*x == '\0') {
			if (*y != '\0') {
				d = diag_delta_new(DIAG_DELTA_APPEND);
				d->value = (void *)y;
				diag_deque_push(deque, (uintptr_t)d);
			}
			break;
		}
		if (*y == '\0') {
			d = diag_delta_new(DIAG_DELTA_TRIM);
			d->index = i;
			diag_deque_push(deque, (uintptr_t)d);
			break;
		}
		if (*x != *y) {
			d = diag_delta_new(DIAG_DELTA_REPLACE);
			d->index = i;
			d->value = (void *)y;
			diag_deque_push(deque, (uintptr_t)d);
		}
	}
	code = diag_code_new(cluster, deque->length);
	i = 0;
	DIAG_DEQUE_FOR_EACH(deque, elem) {
		d = (struct diag_delta *)elem->attr;
		code->deltas[i++] = d;
	}
	assert(i == deque->length);
	diag_deque_destroy(deque);
	return code;
}
