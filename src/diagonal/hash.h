#ifndef DIAGONAL_HASH_H
#define DIAGONAL_HASH_H

struct diag_rolling_hash32 {
	uint32_t value;
	const uint8_t *data;
	const uint8_t *head;
	diag_size_t size;
	diag_size_t s_window;
	void *attr;
	uint32_t (*init)(struct diag_rolling_hash32 *);
	uint32_t (*roll)(struct diag_rolling_hash32 *);
};

struct diag_rolling_hash64 {
	uint64_t value;
	const uint8_t *data;
	const uint8_t *head;
	diag_size_t size;
	diag_size_t s_window;
	void *attr;
	uint64_t (*init)(struct diag_rolling_hash64 *);
	uint64_t (*roll)(struct diag_rolling_hash64 *);
};

DIAG_C_DECL_BEGIN

extern uint32_t diag_hash32_rabin_karp(const uint8_t *data, diag_size_t size, uint32_t base);
extern uint64_t diag_hash64_rabin_karp(const uint8_t *data, diag_size_t size, uint64_t base);

extern struct diag_rolling_hash32 *diag_rolling_hash32_new(const uint8_t *data, diag_size_t size, diag_size_t s_window);
extern struct diag_rolling_hash32 *diag_rolling_hash32_new_rabin_karp(const uint8_t *data, diag_size_t size, diag_size_t s_window, uint32_t base);
extern void diag_rolling_hash32_destroy(struct diag_rolling_hash32 *rh);
extern uint32_t *diag_rolling_hash32_collect(struct diag_rolling_hash32 *rh, diag_size_t *length);

DIAG_C_DECL_END

#endif
