/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/metric.h"
#include "diagonal/cluster.h"

struct diag_code *
diag_encode(struct diag_analysis *analysis)
{
	diag_size_t k, i;

	assert(analysis && analysis->clusters && analysis->num_data > 0);
	k = 0;
	for (i = 0; i < analysis->num_clusters; i++) {
		struct diag_cluster *cluster;
		diag_size_t j;

		cluster = analysis->clusters[i];
		for (j = 0; j < cluster->num_data; j++) {
			assert(k < analysis->num_data);
			analysis->codes[k++] = analysis->encoder(cluster, cluster->data[j]);
		}
	}
	return analysis->codes[0];
}

struct diag_datum **
diag_decode(struct diag_analysis *analysis)
{
	struct diag_datum **data;
	diag_size_t i;

	assert(analysis && analysis->codes && analysis->num_data > 0 && !analysis->data);
	data = diag_calloc((size_t)analysis->num_data, sizeof(struct diag_datum *));
	for (i = 0; i < analysis->num_data; i++) {
		data[i] = analysis->decoder(analysis->codes[i]);
	}
	analysis->data = data;
	return data;
}
