#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "diagonal.h"
#include "diagonal/metric.h"
#include "diagonal/cluster.h"

diag_code_t *
diag_encode(diag_analysis_t *analysis)
{
	diag_size_t k, i;

	assert(analysis && analysis->clusters && analysis->num_data > 0);
	k = 0;
	for (i = 0; i < analysis->num_clusters; i++) {
		diag_cluster_t *cluster;
		diag_size_t j;

		cluster = analysis->clusters[i];
		for (j = 0; j < cluster->num_data; j++) {
			assert(k < analysis->num_data);
			analysis->codes[k++] = analysis->encoder(cluster, cluster->data[j]);
		}
	}
	return analysis->codes[0];
}

diag_datum_t **
diag_decode(diag_analysis_t *analysis)
{
	diag_datum_t **data;
	diag_size_t i;

	assert(analysis && analysis->codes && analysis->num_data > 0 && !analysis->data);
	data = (diag_datum_t **)diag_calloc((size_t)analysis->num_data, sizeof(diag_datum_t *));
	for (i = 0; i < analysis->num_data; i++) {
		data[i] = analysis->decoder(analysis->codes[i]);
	}
	analysis->data = data;
	return data;
}
