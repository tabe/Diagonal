/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_CLUSTER_H
#define DIAGONAL_CLUSTER_H

struct diag_cluster {
	diag_size_t num_data;
	struct diag_datum *data[];
};

enum diag_delta_type {
	DIAG_DELTA_REPLACE,
	DIAG_DELTA_INSERT,
	DIAG_DELTA_DELETE,
	DIAG_DELTA_APPEND,
	DIAG_DELTA_TRIM,
};

struct diag_delta {
	enum diag_delta_type type;
	diag_size_t index;
	void *value;
};

struct diag_code {
	struct diag_cluster *cluster;
	diag_size_t num_deltas;
	struct diag_delta *deltas[];
};

typedef struct diag_code *(*diag_encoder_t)(struct diag_cluster *cluster, struct diag_datum *datum);
typedef struct diag_datum *(*diag_decoder_t)(struct diag_code *code);

struct diag_analysis {
	diag_size_t num_data;
	struct diag_datum **data;
	diag_metric_t metric;
	diag_encoder_t encoder;
	diag_decoder_t decoder;
	diag_size_t num_clusters;
	struct diag_cluster **clusters;
	struct diag_code *codes[];
};

/* greedy agglomerative clustering */
struct diag_gac {
	struct diag_cluster *(*agglomerator)(diag_size_t num_data, struct diag_datum **data, diag_metric_t metric);
};

DIAG_C_DECL_BEGIN

extern struct diag_cluster *diag_cluster_new(diag_size_t num_data);
extern void diag_cluster_destroy(struct diag_cluster *cluster);

extern struct diag_delta *diag_delta_new(enum diag_delta_type type);
extern void diag_delta_destroy(struct diag_delta *delta);

extern struct diag_code *diag_code_new(struct diag_cluster *cluster, diag_size_t num_deltas);
extern void diag_code_destroy(struct diag_code *datum);

extern struct diag_analysis *diag_analysis_new(diag_size_t num_data, struct diag_datum **data);
extern void diag_analysis_destroy(struct diag_analysis *analysis);

extern struct diag_code *diag_encode(struct diag_analysis *analysis);
extern struct diag_datum **diag_decode(struct diag_analysis *analysis);

extern struct diag_code *diag_delta_hamming_chars(struct diag_cluster *cluster, const char *x, const char *y);

DIAG_C_DECL_END

#endif
