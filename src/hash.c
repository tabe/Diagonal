/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/hash.h"

uint32_t
diag_hash32_rabin_karp(const uint8_t *data, diag_size_t size, uint32_t base)
{
	register diag_size_t i;
	register uint32_t v = 0;

	assert(data);
	for (i = 0; i < size; i++) {
		v *= base;
		v += data[i];
	}
	return v;
}

uint64_t
diag_hash64_rabin_karp(const uint8_t *data, diag_size_t size, uint64_t base)
{
	register diag_size_t i;
	register uint64_t v = 0;

	assert(data);
	for (i = 0; i < size; i++) {
		v *= base;
		v += data[i];
	}
	return v;
}

struct rabin_karp_attr32 {
	uint32_t base;
	uint32_t factor;
};

static uint32_t
rabin_karp_init32(struct diag_rollinghash32 *rh)
{
	register diag_size_t i;
	uint32_t base, factor = 1;

	assert(rh);
	base = ((struct rabin_karp_attr32 *)rh->attr)->base;
	for (i = 1; i < rh->s_window; i++) {
		factor *= base;
	}
	((struct rabin_karp_attr32 *)rh->attr)->factor = factor;
	rh->value = diag_hash32_rabin_karp(rh->data, rh->s_window, base);
	rh->head = rh->data + rh->s_window;
	return rh->value;
}

static uint32_t
rabin_karp_roll32(struct diag_rollinghash32 *rh)
{
	register uint32_t base, factor;
	const uint8_t *tail;

	assert(rh);
	tail = rh->head - rh->s_window;
	assert(tail >= rh->data);
	base = ((struct rabin_karp_attr32 *)rh->attr)->base;
	factor = ((struct rabin_karp_attr32 *)rh->attr)->factor;
	rh->value -= *tail * factor;
	rh->value *= base;
	rh->value += *rh->head++;
	return rh->value;
}

struct diag_rollinghash32 *
diag_rollinghash32_new(const uint8_t *data, diag_size_t size, diag_size_t s_window)
{
	struct diag_rollinghash32 *rh;

	assert(data);
	assert(size > 0);
	assert(s_window > 0);
	assert(size >= s_window);
	rh = diag_malloc(sizeof(*rh));
	rh->value = 0;
	rh->data = rh->head = data;
	rh->size = size;
	rh->s_window = s_window;
	rh->attr = NULL;
	rh->init = NULL;
	rh->roll = NULL;
	return rh;
}

struct diag_rollinghash32 *
diag_rollinghash32_new_rabin_karp(const uint8_t *data, diag_size_t size, diag_size_t s_window, uint32_t base)
{
	struct diag_rollinghash32 *rh;
	struct rabin_karp_attr32 *attr;

	assert(base > 0);
	rh = diag_rollinghash32_new(data, size, s_window);
	attr = diag_malloc(sizeof(*attr));
	attr->base = base;
	rh->attr = attr;
	rh->init = rabin_karp_init32;
	rh->roll = rabin_karp_roll32;
	return rh;
}

void
diag_rollinghash32_destroy(struct diag_rollinghash32 *rh)
{
	if (!rh) return;
	diag_free(rh->attr);
	diag_free(rh);
}

uint32_t *
diag_rollinghash32_collect(struct diag_rollinghash32 *rh, diag_size_t *length)
{
	register diag_size_t len, i;
	uint32_t *arr;

	assert(rh);
	assert(length);
	assert(rh->size >= rh->s_window);
	*length = len = rh->size - rh->s_window + 1;
	assert(len > 0);
	arr = diag_calloc((size_t)len, sizeof(uint32_t));
	arr[0] = rh->init(rh);
	for (i = 1; i < len; i++) {
		arr[i] = rh->roll(rh);
	}
	return arr;
}
