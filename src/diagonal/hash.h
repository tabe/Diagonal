/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_HASH_H
#define DIAGONAL_HASH_H

struct diag_rollinghash32 {
	uint32_t value;
	const uint8_t *data;
	const uint8_t *head;
	size_t size;
	size_t s_window;
	void *attr;
	uint32_t (*init)(struct diag_rollinghash32 *);
	uint32_t (*roll)(struct diag_rollinghash32 *);
};

struct diag_rollinghash64 {
	uint64_t value;
	const uint8_t *data;
	const uint8_t *head;
	size_t size;
	size_t s_window;
	void *attr;
	uint64_t (*init)(struct diag_rollinghash64 *);
	uint64_t (*roll)(struct diag_rollinghash64 *);
};

DIAG_C_DECL_BEGIN

/*
 * Return the 32-bit Rabin-Karp hash value of `data' of length `size' with
 * `base'.
 */
DIAG_FUNCTION uint32_t diag_hash32_rabin_karp(const uint8_t *data, size_t size,
				       uint32_t base);
/*
 * Return the 64-bit Rabin-Karp hash value of `data' of length `size' with
 * `base'.
 */
DIAG_FUNCTION uint64_t diag_hash64_rabin_karp(const uint8_t *data, size_t size,
				       uint64_t base);

#ifdef HAVE_ARPA_INET_H
/*
 * Return the Adler-32 hash value of `data' of length `size'.
 */
DIAG_FUNCTION uint32_t diag_hash32_adler32(const uint8_t *data, size_t size);
#endif

/*
 * Return new opaque data to calculate 32-bit Rabin-Karp hash values, with
 * `base', for each window of size `s_window' of `data' of length `size'.
 */
DIAG_FUNCTION struct diag_rollinghash32 *
diag_rollinghash32_new_rabin_karp(const uint8_t *data, size_t size,
				  size_t s_window, uint32_t base);
/*
 * Finalize and free `rh'.
 */
DIAG_FUNCTION void diag_rollinghash32_destroy(struct diag_rollinghash32 *rh);
/*
 * Calculate and return the array of 32-bit rolling hash values, which length
 * will be stored at `length'.
 */
DIAG_FUNCTION uint32_t *diag_rollinghash32_collect(struct diag_rollinghash32 *rh,
					    size_t *length);

DIAG_C_DECL_END

#endif
