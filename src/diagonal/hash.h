#ifndef DIAGONAL_HASH_H
#define DIAGONAL_HASH_H

typedef struct diag_rolling_hash32_s {
	uint32_t value;
	const uint8_t *data;
	const uint8_t *head;
	diag_size_t size;
	diag_size_t s_window;
	void *attr;
	uint32_t (*init)(struct diag_rolling_hash32_s *);
	uint32_t (*roll)(struct diag_rolling_hash32_s *);
} diag_rolling_hash32_t;

typedef struct diag_rolling_hash64_s {
	uint64_t value;
	const uint8_t *data;
	const uint8_t *head;
	diag_size_t size;
	diag_size_t s_window;
	void *attr;
	uint64_t (*init)(struct diag_rolling_hash64_s *);
	uint64_t (*roll)(struct diag_rolling_hash64_s *);
} diag_rolling_hash64_t;

DIAG_C_DECL_BEGIN

extern uint32_t diag_hash32_rabin_karp(const uint8_t *data, diag_size_t size, uint32_t base);
extern uint64_t diag_hash64_rabin_karp(const uint8_t *data, diag_size_t size, uint64_t base);

extern diag_rolling_hash32_t *diag_rolling_hash32_new(const uint8_t *data, diag_size_t size, diag_size_t s_window);
extern diag_rolling_hash32_t *diag_rolling_hash32_new_rabin_karp(const uint8_t *data, diag_size_t size, diag_size_t s_window, uint32_t base);
extern void diag_rolling_hash32_destroy(diag_rolling_hash32_t *rh);
extern uint32_t *diag_rolling_hash32_collect(diag_rolling_hash32_t *rh, diag_size_t *length);

DIAG_C_DECL_END

#endif
