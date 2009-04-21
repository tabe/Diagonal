#ifndef DIAGONAL_CLUSTER_H
#define DIAGONAL_CLUSTER_H

typedef struct {
	diag_size_t num_data;
	diag_datum_t *data[];
} diag_cluster_t;

enum diag_delta_type {
	DIAG_DELTA_REPLACE,
	DIAG_DELTA_INSERT,
	DIAG_DELTA_DELETE,
	DIAG_DELTA_APPEND,
	DIAG_DELTA_TRIM,
};

typedef struct {
	enum diag_delta_type type;
	diag_size_t index;
	void *value;
} diag_delta_t;

typedef struct {
	diag_cluster_t *cluster;
	diag_size_t num_deltas;
	diag_delta_t *deltas[];
} diag_code_t;

typedef diag_code_t *(*diag_encoder_t)(diag_cluster_t *cluster, diag_datum_t *datum);
typedef diag_datum_t *(*diag_decoder_t)(diag_code_t *code);

typedef struct {
	diag_size_t num_data;
	diag_datum_t **data;
	diag_metric_t metric;
	diag_encoder_t encoder;
	diag_decoder_t decoder;
	diag_size_t num_clusters;
	diag_cluster_t **clusters;
	diag_code_t *codes[];
} diag_analysis_t;

/* greedy agglomerative clustering */
typedef struct {
	diag_cluster_t *(*agglomerator)(diag_size_t num_data, diag_datum_t **data, diag_metric_t metric);
} diag_gac_t;

DIAG_C_DECL_BEGIN

extern diag_cluster_t *diag_cluster_new(diag_size_t num_data);
extern void diag_cluster_destroy(diag_cluster_t *cluster);

extern diag_delta_t *diag_delta_new(enum diag_delta_type type);
extern void diag_delta_destroy(diag_delta_t *delta);

extern diag_code_t *diag_code_new(diag_cluster_t *cluster, diag_size_t num_deltas);
extern void diag_code_destroy(diag_code_t *datum);

extern diag_analysis_t *diag_analysis_new(diag_size_t num_data, diag_datum_t **data);
extern void diag_analysis_destroy(diag_analysis_t *analysis);

extern diag_code_t *diag_encode(diag_analysis_t *analysis);
extern diag_datum_t **diag_decode(diag_analysis_t *analysis);

extern diag_code_t *diag_delta_hamming_chars(diag_cluster_t *cluster, const char *x, const char *y);

DIAG_C_DECL_END

#endif
