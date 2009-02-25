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

diag_cluster_t *
diag_cluster_new(diag_size_t num_data)
{
	diag_cluster_t *cluster;
	size_t size;

	size = (size_t)(sizeof(diag_cluster_t)+num_data*sizeof(diag_datum_t *));
	cluster = (diag_cluster_t *)diag_malloc(size);
	cluster->num_data = num_data;
	return cluster;
}

void
diag_cluster_destroy(diag_cluster_t *cluster)
{
	if (cluster) {
		diag_free(cluster);
	}
}

diag_analysis_t *
diag_analysis_new(diag_size_t num_data, diag_datum_t **data)
{
	diag_analysis_t *analysis;
	size_t size;

	size = (size_t)(sizeof(diag_analysis_t)+num_data*sizeof(diag_code_t *));
	analysis = (diag_analysis_t *)diag_malloc(size);
	analysis->num_data = num_data;
	analysis->data = data;
	analysis->metric = NULL;
	analysis->encoder = NULL;
	analysis->decoder = NULL;
	analysis->clusters = NULL;
	return analysis;
}

void
diag_analysis_destroy(diag_analysis_t *analysis)
{
	if (analysis) {
		diag_free(analysis);
	}
}

diag_code_t *
diag_code_new(diag_cluster_t *cluster, diag_size_t num_deltas)
{
	diag_code_t *code;
	size_t size;

	size = (size_t)(sizeof(diag_code_t)+num_deltas*sizeof(diag_delta_t *));
	code = (diag_code_t *)diag_malloc(size);
	code->cluster = cluster;
	code->num_deltas = num_deltas;
	return code;
}

void
diag_code_destroy(diag_code_t *datum)
{
	if (datum) {
		diag_free(datum);
	}
}

diag_delta_t *
diag_delta_new(enum diag_delta_type type)
{
	diag_delta_t *delta;

	delta = (diag_delta_t *)diag_malloc(sizeof(diag_delta_t));
	delta->type = type;
	return delta;
}

void
diag_delta_destroy(diag_delta_t *delta)
{
	if (delta) {
		diag_free(delta);
	}
}

diag_code_t *
diag_delta_hamming_chars(diag_cluster_t *cluster, const char *x, const char *y)
{
	diag_deque_t *deque;
	diag_deque_elem_t *elem;
	diag_delta_t *d;
	diag_code_t *code;
	diag_size_t i = 0;

	assert(x && y);
	if (x == y) return NULL;
	deque = diag_deque_new();
	for (;; i++, x++, y++) {
		if (*x == '\0') {
			if (*y != '\0') {
				d = diag_delta_new(DIAG_DELTA_APPEND);
				d->value = (void *)y;
				diag_deque_push(deque, (void *)d);
			}
			break;
		}
		if (*y == '\0') {
			d = diag_delta_new(DIAG_DELTA_TRIM);
			d->index = i;
			diag_deque_push(deque, (void *)d);
			break;
		}
		if (*x != *y) {
			d = diag_delta_new(DIAG_DELTA_REPLACE);
			d->index = i;
			d->value = (void *)y;
			diag_deque_push(deque, (void *)d);
		}
	}
	code = diag_code_new(cluster, deque->length);
	i = 0;
	DIAG_DEQUE_FOR_EACH(deque, elem) {
		d = (diag_delta_t *)elem->attr;
		code->deltas[i++] = d;
	}
	assert(i == deque->length);
	diag_deque_destroy(deque);
	return code;
}
