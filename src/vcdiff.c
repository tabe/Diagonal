/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/bytevector.h"
#include "diagonal/cmp.h"
#include "diagonal/deque.h"
#include "diagonal/rbtree.h"
#include "diagonal/port.h"
#include "diagonal/hash.h"
#include "diagonal/vcdiff.h"

static int
integer_read(struct diag_vcdiff_context *context, uint32_t *i)
{
	struct diag_port *port;
	uint8_t b;
	uint32_t t = 0;
	int r;

	assert(context && i);
	port = context->port;
	while ( (r = port->read_byte(port, &b)) > 0) {
		if (b >> 7) {
			t = (t << 7) | (b & ~((uint8_t)0x80));
			continue;
		} else {
			*i = (t << 7) | b;
			return r;
		}
	}
	return r;
}

static int
code_table_data_read(struct diag_vcdiff_context *context, struct diag_vcdiff *vcdiff)
{
	struct diag_port *port;
	uint32_t length;
	uint8_t *data;
	int r;

	assert(context && vcdiff);
	if ( (r = integer_read(context, &length)) <= 0) {
		return r;
	}
	data = diag_calloc((size_t)length, sizeof(uint8_t));
	port = context->port;
	if ( (r = port->read_bytes(port, (size_t)length, data)) <= 0) {
		diag_free(data);
		return r;
	}
	vcdiff->length_code_table_data = length;
	vcdiff->code_table_data = data;
	return r;
}

static int
delta_read(struct diag_vcdiff_context *context, struct diag_vcdiff_window *window)
{
	struct diag_port *port;
	struct diag_vcdiff_delta *delta;
	uint8_t b, *data, *inst, *addr;
	uint32_t i;
	int r;

	assert(context && window);
	delta = diag_malloc(sizeof(struct diag_vcdiff_delta));
	if ( (r = integer_read(context, &i)) <= 0) goto bail;
	delta->length = i;
	if ( (r = integer_read(context, &i)) <= 0) goto bail;
	delta->s_window = i;
	port = context->port;
	if ( (r = port->read_byte(port, &b)) <= 0) goto bail;
	delta->indicator = b;
	if ( (r = integer_read(context, &i)) <= 0) goto bail;
	delta->length_data = i;
	if ( (r = integer_read(context, &i)) <= 0) goto bail;
	delta->length_inst = i;
	if ( (r = integer_read(context, &i)) <= 0) goto bail;
	delta->length_addr = i;
	data = diag_calloc((size_t)(delta->length_data), sizeof(uint8_t));
	inst = diag_calloc((size_t)(delta->length_inst), sizeof(uint8_t));
	addr = diag_calloc((size_t)(delta->length_addr), sizeof(uint8_t));
	if ( (r = port->read_bytes(port, (size_t)(delta->length_data), data)) <= 0) goto clear;
	if ( (r = port->read_bytes(port, (size_t)(delta->length_inst), inst)) <= 0) goto clear;
	if ( (r = port->read_bytes(port, (size_t)(delta->length_addr), addr)) <= 0) goto clear;
	delta->data = data;
	delta->inst = inst;
	delta->addr = addr;
	window->delta = delta;
	return r;

 clear:
	diag_free(data);
	diag_free(inst);
	diag_free(addr);
 bail:
	diag_free(delta);
	return r;
}

static void
cache_init(struct diag_vcdiff_cache *cache, uint8_t s_near, uint8_t s_same)
{
	register uint32_t i;

	assert(cache);
	cache->next_slot = 0;
	cache->s_near = s_near;
	cache->near = diag_calloc(s_near, sizeof(uint32_t));
	for (i = 0; i < s_near; i++) {
		cache->near[i] = 0;
	}
	cache->s_same = s_same;
	cache->same = diag_calloc(s_same * 256, sizeof(uint32_t));
	for (i = 0; i < (uint32_t)s_same * 256; i++) {
		cache->same[i] = 0;
	}
}

static void
cache_update(struct diag_vcdiff_cache *cache, uint32_t addr)
{
	assert(cache);
	if (cache->s_near > 0) {
		cache->near[cache->next_slot] = addr;
		cache->next_slot = (cache->next_slot + 1) % cache->s_near;
	}
	if (cache->s_same > 0) {
		cache->same[addr % (cache->s_same * 256)] = addr;
	}
}

static void
cache_clean(struct diag_vcdiff_cache *cache)
{
	assert(cache && cache->near && cache->same);
	diag_free(cache->near);
	cache->near = NULL;
	diag_free(cache->same);
	cache->same = NULL;
}

static void
cache_destroy(struct diag_vcdiff_cache *cache)
{
	if (!cache) return;
	diag_free(cache->near);
	diag_free(cache->same);
	diag_free(cache);
}

#if 0
uint32_t
addr_encode(struct diag_vcdiff_cache *cache, uint32_t addr, uint32_t here, uint32_t *mode)
{
	uint32_t i, d, bestd, bestm;

	assert(cache);
	/* Attempt to find the address mode that yields the
	 * smallest integer value for "d", the encoded address
	 * value, thereby minimizing the encoded size of the
	 * address. */
	bestd = addr;
	bestm = DIAG_VCD_SELF;
	if ( (d = here - addr) < bestd) {
		bestd = d;
		bestm = DIAG_VCD_HERE;
	}
	for (i = 0; i < cache->s_near; i++) {
		if ( (d = addr - cache->near[i]) >= 0 && d < bestd ) {
			bestd = d;
			bestm = i + 2;
		}
	}
	if (cache->s_same > 0 && cache->same[d = addr%(cache->s_same*256)] == addr) {
		bestd = d%256;
		bestm = cache->s_near + 2 + d/256;
	}
	cache_update(cache, addr);
	*mode = bestm; /* this returns the address encoding mode */
	return bestd; /* this returns the encoded address */
}
#endif

static const struct diag_vcdiff_code default_code_table_entries[256] = {
	/* line 1: index 0 */
	{DIAG_VCD_RUN,  0, 0, DIAG_VCD_NOOP, 0, 0},
	/* line 2: index [1,18] */
	{DIAG_VCD_ADD, 0, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 1, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 2, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 3, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 4, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 5, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 6, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 7, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 8, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 9, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 10, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 11, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 12, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 13, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 14, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 15, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 16, 0, DIAG_VCD_NOOP, 0, 0},
	{DIAG_VCD_ADD, 17, 0, DIAG_VCD_NOOP, 0, 0},
#define LINE(mode)\
	{DIAG_VCD_COPY, 0, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 4, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 5, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 6, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 7, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 8, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 9, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 10, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 11, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 12, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 13, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 14, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 15, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 16, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 17, mode, DIAG_VCD_NOOP, 0, 0},\
	{DIAG_VCD_COPY, 18, mode, DIAG_VCD_NOOP, 0, 0}
	/* line 3: index [19,34] */
	LINE(0),
	/* line 4: index [35,50] */
	LINE(1),
	/* line 5: index [51,66] */
	LINE(2),
	/* line 6: index [67,82] */
	LINE(3),
	/* line 7: index [83,98] */
	LINE(4),
	/* line 8: index [99,114] */
	LINE(5),
	/* line 9: index [115,130] */
	LINE(6),
	/* line 10: index [131,146] */
	LINE(7),
	/* line 11: index [147,162] */
	LINE(8),
#undef LINE
#define LINE(mode)\
	{DIAG_VCD_ADD, 1, 0, DIAG_VCD_COPY, 4, mode},\
	{DIAG_VCD_ADD, 1, 0, DIAG_VCD_COPY, 5, mode},\
	{DIAG_VCD_ADD, 1, 0, DIAG_VCD_COPY, 6, mode},\
	{DIAG_VCD_ADD, 2, 0, DIAG_VCD_COPY, 4, mode},\
	{DIAG_VCD_ADD, 2, 0, DIAG_VCD_COPY, 5, mode},\
	{DIAG_VCD_ADD, 2, 0, DIAG_VCD_COPY, 6, mode},\
	{DIAG_VCD_ADD, 3, 0, DIAG_VCD_COPY, 4, mode},\
	{DIAG_VCD_ADD, 3, 0, DIAG_VCD_COPY, 5, mode},\
	{DIAG_VCD_ADD, 3, 0, DIAG_VCD_COPY, 6, mode},\
	{DIAG_VCD_ADD, 4, 0, DIAG_VCD_COPY, 4, mode},\
	{DIAG_VCD_ADD, 4, 0, DIAG_VCD_COPY, 5, mode},\
	{DIAG_VCD_ADD, 4, 0, DIAG_VCD_COPY, 6, mode}
	/* line 12: index [163,174] */
	LINE(0),
	/* line 13: index [175,186] */
	LINE(1),
	/* line 14: index [187,198] */
	LINE(2),
	/* line 15: index [199,210] */
	LINE(3),
	/* line 16: index [211,222] */
	LINE(4),
	/* line 17: index [223,234] */
	LINE(5),
#undef LINE
#define LINE(mode)\
	{DIAG_VCD_ADD, 1, 0, DIAG_VCD_COPY, 4, mode},\
	{DIAG_VCD_ADD, 2, 0, DIAG_VCD_COPY, 4, mode},\
	{DIAG_VCD_ADD, 3, 0, DIAG_VCD_COPY, 4, mode},\
	{DIAG_VCD_ADD, 4, 0, DIAG_VCD_COPY, 4, mode}
	/* line 18: index [235,238] */
	LINE(6),
	/* line 19: index [239,242] */
	LINE(7),
	/* line 20: index [243,246] */
	LINE(8),
#undef LINE
	/* line 21: index [247,255] */
	{DIAG_VCD_COPY, 4, 0, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 1, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 2, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 3, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 4, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 5, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 6, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 7, DIAG_VCD_ADD, 1, 0},
	{DIAG_VCD_COPY, 4, 8, DIAG_VCD_ADD, 1, 0},
};

static struct diag_vcdiff_code_table default_code_table = {
	DIAG_VCDIFF_S_NEAR_DEFAULT,
	DIAG_VCDIFF_S_SAME_DEFAULT,
	(struct diag_vcdiff_code *)default_code_table_entries
};

static struct diag_vcdiff_code *
code_table_entries_from_string(const uint8_t *str)
{
	struct diag_vcdiff_code *entries;
	int i, j = 0;

	assert(str);
	entries = diag_calloc(256, sizeof(struct diag_vcdiff_code));
	for (i = 0; i < 256; i++) {
		entries[i].inst1 = str[j++];
	}
	for (i = 0; i < 256; i++) {
		entries[i].inst2 = str[j++];
	}
	for (i = 0; i < 256; i++) {
		entries[i].size1 = str[j++];
	}
	for (i = 0; i < 256; i++) {
		entries[i].size2 = str[j++];
	}
	for (i = 0; i < 256; i++) {
		entries[i].mode1 = str[j++];
	}
	for (i = 0; i < 256; i++) {
		entries[i].mode2 = str[j++];
	}
	return entries;
}

#define CODE_TABLE_LENGTH 1536

static uint8_t *
code_table_entries_to_string(const struct diag_vcdiff_code *entries)
{
	uint8_t *str;
	int i, j = 0;

	assert(entries);
	str = diag_malloc(CODE_TABLE_LENGTH);
	for (i = 0; i < 256; i++) {
		str[j++] = entries[i].inst1;
	}
	for (i = 0; i < 256; i++) {
		str[j++] = entries[i].inst2;
	}
	for (i = 0; i < 256; i++) {
		str[j++] = entries[i].size1;
	}
	for (i = 0; i < 256; i++) {
		str[j++] = entries[i].size2;
	}
	for (i = 0; i < 256; i++) {
		str[j++] = entries[i].mode1;
	}
	for (i = 0; i < 256; i++) {
		str[j++] = entries[i].mode2;
	}
	return str;
}

static void
code_table_destroy(struct diag_vcdiff_code_table *code_table)
{
	if (!code_table) return;
	if (code_table == &default_code_table) return;
	diag_free(code_table->entries);
	diag_free(code_table);
}

static int
code_table_decode(struct diag_vcdiff_vm *vm, struct diag_vcdiff *vcdiff)
{
	uint8_t *source;
	struct diag_vcdiff_code_table *code_table;
	struct diag_vcdiff_code *entries;
	struct diag_vcdiff_context *c;
	struct diag_vcdiff *v;
	struct diag_vcdiff_vm *w;

	assert(vm && vcdiff);
	if (vcdiff->length_code_table_data <= 2) {
		vm->error(vm, "code table data is too short");
		return 0;
	}
	if (!vcdiff->code_table_data) {
		vm->error(vm, "code table data is null");
		return 0;
	}
	c = diag_vcdiff_context_new_bm(vcdiff->code_table_data + 2, vcdiff->length_code_table_data - 2);
	c->compatibility = 1;
	v = diag_vcdiff_read(c);
	if (!v) {
		vm->error(vm, "could not read code table data");
		diag_vcdiff_context_destroy(c);
		return 0;
	}
	source = code_table_entries_to_string(default_code_table_entries);
	w = diag_vcdiff_vm_new(CODE_TABLE_LENGTH, source);
	if (!diag_vcdiff_decode(w, v)) {
		vm->error(vm, "could not decode code table");
		diag_vcdiff_vm_destroy(w);
		diag_vcdiff_destroy(v);
		diag_vcdiff_context_destroy(c);
		return 0;
	}
	if (w->s_target != CODE_TABLE_LENGTH) {
		vm->error(vm, "length of \"code\" must be %d", CODE_TABLE_LENGTH);
		diag_vcdiff_vm_destroy(w);
		diag_vcdiff_destroy(v);
		diag_vcdiff_context_destroy(c);
		return 0;
	}
	entries = code_table_entries_from_string(w->target);
	diag_vcdiff_vm_destroy(w);
	diag_vcdiff_destroy(v);
	diag_vcdiff_context_destroy(c);
	code_table = diag_malloc(sizeof(struct diag_vcdiff_code_table));
	code_table->s_near = vcdiff->code_table_data[0];
	code_table->s_same = vcdiff->code_table_data[1];
	code_table->entries = entries;
	vm->code_table = code_table;
	return 1;
}

#undef CODE_TABLE_LENGTH

struct diag_vcdiff_context *
diag_vcdiff_context_new_fp(FILE *fp)
{
	struct diag_vcdiff_context *context;

	context = diag_malloc(sizeof(struct diag_vcdiff_context));
	context->port = diag_port_new_fp(fp, DIAG_PORT_INPUT);
	return context;
}

struct diag_vcdiff_context *
diag_vcdiff_context_new_fd(int fd)
{
	struct diag_vcdiff_context *context;

	context = diag_malloc(sizeof(struct diag_vcdiff_context));
	context->port = diag_port_new_fd(fd, DIAG_PORT_INPUT);
	return context;
}

struct diag_vcdiff_context *
diag_vcdiff_context_new_bm(uint8_t *head, uint32_t size)
{
	struct diag_vcdiff_context *context;

	assert(head);
	context = diag_malloc(sizeof(struct diag_vcdiff_context));
	context->port = diag_port_new_bm(head, size, DIAG_PORT_INPUT);
	return context;
}

struct diag_vcdiff_context *
diag_vcdiff_context_new_path(const char *path)
{
	struct diag_port *port;
	struct diag_vcdiff_context *context;

	assert(path);
	port = diag_port_new_path(path, "rb");
	if (!port) return NULL;
	context = diag_malloc(sizeof(struct diag_vcdiff_context));
	context->port = port;
	return context;
}

void
diag_vcdiff_context_destroy(struct diag_vcdiff_context *context)
{
	if (!context) return;
	diag_port_destroy(context->port);
	diag_free(context);
}

struct diag_vcdiff *
diag_vcdiff_read(struct diag_vcdiff_context *context)
{
	struct diag_vcdiff *vcdiff;
	struct diag_port *port;
	struct diag_deque *deque;
	uint8_t b;
	int r;

	assert(context);
	port = context->port;
	if ( port->read_byte(port, &b) <= 0 || b != (uint8_t)0xD6 ||
		 port->read_byte(port, &b) <= 0 || b != (uint8_t)0xC3 ||
		 port->read_byte(port, &b) <= 0 || b != (uint8_t)0xC4 ) {
		return NULL;
	}
	vcdiff = diag_malloc(sizeof(struct diag_vcdiff));
	if ( port->read_byte(port, &b) <= 0 ||
		 (DIAG_VCDIFF_COMPATIBLEP(context) && b != (uint8_t)0x0) ) {
		goto bail;
	}
	vcdiff->version = b;
	if (port->read_byte(port, &b) <= 0) {
		goto bail;
	}
	vcdiff->indicator = b;
	if (DIAG_VCDIFF_DECOMPRESSP(vcdiff)) {
		if (port->read_byte(port, &b) <= 0) {
			goto bail;
		}
		vcdiff->compressor_id = b;
	}
	if (DIAG_VCDIFF_CODETABLEP(vcdiff)) {
		if (code_table_data_read(context, vcdiff) <= 0) {
			goto bail;
		}
	}
	deque = diag_deque_new();
	while ( (r = port->read_byte(port, &b)) > 0) {
		struct diag_vcdiff_window *window;

		if (DIAG_VCDIFF_COMPATIBLEP(context)) {
			if (b && (b & (b-1))) {
				diag_error("The Win_Indicator byte MUST NOT have more than one of the bits set");
				goto clear;
			}
		}
		window = diag_malloc(sizeof(struct diag_vcdiff_window));
		window->indicator = b;
		if ( DIAG_VCDIFF_SOURCEP(window) ||
			 DIAG_VCDIFF_TARGETP(window) ) {
			uint32_t len, pos;

			if ( integer_read(context, &len) <= 0 ||
				 integer_read(context, &pos) <= 0 ) {
				diag_free(window);
				goto clear;
			}
			window->length_source_segment   = len;
			window->position_source_segment = pos;
		}
		if (delta_read(context, window) <= 0) {
			diag_free(window);
			goto clear;
		}
		diag_deque_push(deque, (uintptr_t)window);
	}
	if (r < 0) {
		goto clear;
	}
	vcdiff->num_windows = (uint32_t)deque->length;
	DIAG_DEQUE_TO_ARRAY(deque, struct diag_vcdiff_window *, vcdiff->windows);
	diag_deque_destroy(deque);
	return vcdiff;

 clear:
	{
		struct diag_deque_elem *elem;

		DIAG_DEQUE_FOR_EACH(deque, elem) {
			diag_free((void *)elem->attr);
		}
		diag_deque_destroy(deque);
	}

 bail:
	diag_free(vcdiff);
	return NULL;
}

static void
vm_error(struct diag_vcdiff_vm *vm, const char *message, ...)
{
	va_list ap;

	assert(vm && message);
	va_start(ap, message);
	diag_verror(message, ap);
	va_end(ap);
	longjmp(vm->env, 1);
}

static struct diag_vcdiff_vm *
vm_new(struct diag_bytevector *source)
{
	struct diag_vcdiff_vm *vm;
	struct diag_vcdiff_cache *cache;

	cache = diag_malloc(sizeof(struct diag_vcdiff_cache));
	cache->near = cache->same = NULL;
	vm = diag_malloc(sizeof(struct diag_vcdiff_vm));
	vm->source = source;
	vm->s_target = 0;
	vm->target = NULL;
	vm->cache = cache;
	vm->code_table = NULL;
	vm->error = vm_error;
	return vm;
}

struct diag_vcdiff_vm *
diag_vcdiff_vm_new(uint32_t size, uint8_t *data)
{
	struct diag_bytevector *source;

	source = diag_bytevector_new_heap(size, data);
	return vm_new(source);
}

struct diag_vcdiff_vm *
diag_vcdiff_vm_new_path(const char *path)
{
	struct diag_bytevector *source;

	if (path) {
		source = diag_bytevector_new_path(path);
		return vm_new(source);
	} else {
		source = diag_bytevector_new_heap(0, NULL);
		return vm_new(source);
	}
}

void
diag_vcdiff_vm_destroy(struct diag_vcdiff_vm *vm)
{
	if (!vm) return;
	code_table_destroy(vm->code_table);
	cache_destroy(vm->cache);
	diag_free(vm->target);
	diag_bytevector_destroy(vm->source);
	diag_free(vm);
}

static uint8_t *
read_integer(uint8_t *buf, const uint8_t *sentinel, uint32_t *i)
{
	uint8_t b;
	uint32_t t = 0;

	assert(buf && i);
	while (buf < sentinel) {
		if ( (b = *buf++) >> 7) {
			t = (t << 7) | (b & ~((uint8_t)0x80));
			continue;
		} else {
			*i = (t << 7) | b;
			return buf;
		}
	}
	return NULL;
}

static uint32_t
vm_inst_read_size(struct diag_vcdiff_vm *vm)
{
	uint32_t size;

	assert(vm);
	vm->inst = read_integer(vm->inst, vm->inst_s, &size);
	if (!vm->inst) vm->error(vm, "exceed instructions and sizes section");
	return size;
}

static uint32_t
vm_addr_read_integer(struct diag_vcdiff_vm *vm)
{
	uint32_t i;

	assert(vm);
	vm->addr = read_integer(vm->addr, vm->addr_s, &i);
	if (!vm->addr) vm->error(vm, "exceed addresses section");
	return i;
}

static uint8_t
vm_addr_read_byte(struct diag_vcdiff_vm *vm)
{
	assert(vm);
	if (vm->addr + 1 > vm->addr_s) vm->error(vm, "exceed addresses section");
	return *(vm->addr)++;
}

static uint32_t
vm_addr_decode(struct diag_vcdiff_vm *vm, uint32_t here, uint8_t mode)
{
	struct diag_vcdiff_cache *cache;
	uint32_t addr, m;

	assert(vm && vm->cache);
	cache = vm->cache;
	if (mode == DIAG_VCD_SELF) {
		addr = vm_addr_read_integer(vm);
	} else if (mode == DIAG_VCD_HERE) {
		addr = here - vm_addr_read_integer(vm);
	} else if ( mode >= 2 && (m = mode - 2) < cache->s_near ) { /* near cache */
		addr = cache->near[m] + vm_addr_read_integer(vm);
	} else { /* same cache */
		uint32_t i;
		m = mode - (2 + cache->s_near);
		i = m * 256 + vm_addr_read_byte(vm);
		if (i >= (uint32_t)cache->s_same * 256) vm->error(vm, "exceed same cache with index %d", i);
		addr = cache->same[i];
	}
	cache_update(cache, addr);
	return addr;
}

static uint32_t
vm_decode(struct diag_vcdiff_vm *vm, uint32_t here, uint8_t inst, uint32_t size, uint8_t mode)
{
	uint8_t b, i;
	uint32_t addr;

	switch (inst) {
	case DIAG_VCD_NOOP:
		/* nothing to do */
		break;
	case DIAG_VCD_ADD:
		if (here + size > vm->s_target) vm->error(vm, "exceed target data segment");
		if (vm->data + size > vm->data_s) vm->error(vm, "exceed data section");
		(void)memmove(vm->target + here, vm->data, size);
		vm->data += size;
		here += size;
		break;
	case DIAG_VCD_RUN:
		if (here + size > vm->s_target) vm->error(vm, "exceed target data segment");
		if (vm->data + 1 > vm->data_s) vm->error(vm, "exceed data section");
		b = *(vm->data)++;
		for (i = 0; i < size; i++) {
			vm->target[here + i] = b;
		}
		here += size;
		break;
	case DIAG_VCD_COPY:
		if (here + size > vm->s_target) vm->error(vm, "exceed target data segment");
		addr = vm_addr_decode(vm, here, mode);
		if (vm->src + addr + size > vm->src_s) vm->error(vm, "exceed source data segment");
		(void)memmove(vm->target + here, vm->src + addr, size);
		here += size;
		break;
	default:
		vm->error(vm, "unknown instruction");
		break;
	}
	return here;
}

uint8_t *
diag_vcdiff_decode(struct diag_vcdiff_vm *vm, struct diag_vcdiff *vcdiff)
{
	uint32_t s_target = 0;
	uint32_t here = 0;
	uint32_t i;
	int status;

	assert(vm && vcdiff);
	if (DIAG_VCDIFF_CODETABLEP(vcdiff)) {
		if (!code_table_decode(vm, vcdiff)) return NULL;
	} else {
		vm->code_table = &default_code_table;
	}
	cache_init(vm->cache, vm->code_table->s_near, vm->code_table->s_same);
	for (i = 0; i < vcdiff->num_windows; i++) {
		if (vcdiff->windows[i] && vcdiff->windows[i]->delta) {
			s_target += vcdiff->windows[i]->delta->s_window;
		} else {
			cache_clean(vm->cache);
			code_table_destroy(vm->code_table);
			vm->code_table = NULL;
			return NULL;
		}
	}
	vm->s_target = s_target;
	vm->target = diag_calloc(s_target, sizeof(uint8_t));
	status = setjmp(vm->env);
	if (status != 0) goto bail;
	for (i = 0; i < vcdiff->num_windows; i++) {
		struct diag_vcdiff_window *window;
		struct diag_vcdiff_delta *delta;

		window = vcdiff->windows[i];
		assert(window->delta);
		delta = window->delta;
		if (DIAG_VCDIFF_SOURCEP(window)) {
			uint32_t end = window->position_source_segment + window->length_source_segment;
			if (end > vm->source->size) {
				vm->error(vm, "exceed source data segment: %d > %d", end, vm->source->size);
			}
			vm->src = vm->source->data + window->position_source_segment;
			vm->src_s = vm->src + window->length_source_segment;
		} else if (DIAG_VCDIFF_TARGETP(window)) {
			uint32_t end = window->position_source_segment + window->length_source_segment;
			if (end > vm->s_target) {
				vm->error(vm, "exceed target data segment: %d > %d", end, vm->s_target);
			}
			vm->src = vm->target + window->position_source_segment;
			vm->src_s = vm->src + window->length_source_segment;
		} else {
			uint32_t end = here + delta->s_window;
			if (end > vm->s_target) {
				vm->error(vm, "exceed target data segment: %d > %d", end, vm->s_target);
			}
			vm->src = vm->target + here;
			vm->src_s = vm->src + delta->s_window;
		}
		/* decompress data, inst, and addr if needed */
		vm->data   = delta->data;
		vm->data_s = delta->data + delta->length_data;
		vm->inst   = delta->inst;
		vm->inst_s = delta->inst + delta->length_inst;
		vm->addr   = delta->addr;
		vm->addr_s = delta->addr + delta->length_addr;
		while (vm->inst < vm->inst_s) {
			struct diag_vcdiff_code code;
			uint32_t size1, size2;
			uint8_t index;

			index = *(vm->inst)++;
			code = vm->code_table->entries[index];
			if (code.inst1 != DIAG_VCD_NOOP && code.size1 == 0) {
				size1 = vm_inst_read_size(vm);
			} else {
				size1 = code.size1;
			}
			if (code.inst2 != DIAG_VCD_NOOP && code.size2 == 0) {
				size2 = vm_inst_read_size(vm);
			} else {
				size2 = code.size2;
			}
			here = vm_decode(vm, here, code.inst1, size1, code.mode1);
			if (here > vm->s_target) vm->error(vm, "exceed target data segment: %d > %d", here, vm->s_target);
			here = vm_decode(vm, here, code.inst2, size2, code.mode2);
			if (here > vm->s_target) vm->error(vm, "exceed target data segment: %d > %d", here, vm->s_target);
		}
	}
	return vm->target;

 bail:
	vm->s_target = 0;
	diag_free(vm->target);
	vm->target = NULL;
	return NULL;
}

void
diag_vcdiff_destroy(struct diag_vcdiff *vcdiff)
{
	diag_free(vcdiff);
}

void
diag_vcdiff_script_destroy(struct diag_vcdiff_script *script)
{
	register size_t i;

	if (!script) return;
	for (i = 0; i < script->s_pcodes; i++) {
		switch (script->pcodes[i].inst) {
		case DIAG_VCD_ADD:
			diag_free(script->pcodes[i].attr.data);
			break;
		}
	}
	diag_free(script->pcodes);
	diag_free(script);
}

uint8_t *
diag_vcdiff_expand(const struct diag_vcdiff_script *script, size_t *size)
{
	register size_t i, n = 0, p, s;
	struct diag_vcdiff_pcode *pcode;
	const uint8_t *source;
	uint8_t *result;

	assert(script);
	assert(size);
	s = 0;
	for (p = 0; p < script->s_pcodes; p++) {
		s += script->pcodes[p].size;
		if (s < script->pcodes[p].size) {
			return NULL;
		}
	}
	*size = s;
	result = diag_calloc(s, sizeof(uint8_t));
	source = (script->source) ? script->source : result;
	for (p = 0; p < script->s_pcodes; p++) {
		pcode = script->pcodes + p;
		switch (pcode->inst) {
		case DIAG_VCD_NOOP:
			/* nothing to do */
			break;
		case DIAG_VCD_ADD:
			(void)memcpy(result + n, pcode->attr.data, pcode->size);
			n += pcode->size;
			break;
		case DIAG_VCD_RUN:
			for (i = 0; i < pcode->size; i++) {
				result[n++] = pcode->attr.byte;
			}
			break;
		case DIAG_VCD_COPY:
			/* memmove() is inadequate for sequential copy. */
			for (i = 0; i < pcode->size; i++) {
				result[n++] = source[pcode->attr.addr + i];
			}
			break;
		default:
			diag_error("unknown instruction: %d", pcode->inst);
			break;
		}
	}
	return result;
}

static struct diag_vcdiff_pcode *
pcode_copy_new(size_t size, size_t addr)
{
	struct diag_vcdiff_pcode *pcode;

	assert(size > 0);
	pcode = diag_malloc(sizeof(struct diag_vcdiff_pcode));
	pcode->inst = DIAG_VCD_COPY;
	pcode->size = size;
	pcode->attr.addr = addr;
	return pcode;
}

static struct diag_vcdiff_pcode *
pcode_add_new(size_t size, const uint8_t *data)
{
	struct diag_vcdiff_pcode *pcode;
	uint8_t *d;

	assert(size > 0);
	pcode = diag_malloc(sizeof(struct diag_vcdiff_pcode));
	pcode->inst = DIAG_VCD_ADD;
	pcode->size = size;
	d = diag_calloc(size + 1, sizeof(uint8_t));
	(void)memcpy(d, data, size);
	d[size] = '\0';
	pcode->attr.data = d;
	return pcode;
}

static size_t
lookback(struct diag_rollinghash32 *rh, uint32_t *arr, size_t i, uint32_t h, struct diag_rbtree *tree)
{
	register size_t k, n;
	size_t m, head, tail;
	struct diag_vcdiff_pcode *p;
	struct diag_rbtree_node *node;

	/* check whether some hash value matches */
	for (k = 0; k < i / rh->s_window; k++) {
		if (h == arr[k]) {
			int matched = 1;
			/* check whether its substring actually matches */
			for (n = 0; n < rh->s_window; n++) {
				if (rh->data[i + n] != rh->data[k * rh->s_window + n]) {
					matched = 0;
					break;
				}
			}
			if (matched) {
				assert(i > k * rh->s_window);
				m = i - k * rh->s_window;

				head = i;
				while (--head >= m) {
					if (rh->data[head - m] != rh->data[head]) break;
				}

				tail = i + rh->s_window - 1;
				while (++tail < rh->size) {
					if (rh->data[tail - m] != rh->data[tail]) break;
				}

				p = pcode_copy_new(tail - head - 1, head - m + 1);
				node = diag_rbtree_node_new((uintptr_t)head + 1, (uintptr_t)p);
				diag_rbtree_insert(tree, node);

				while (++i < tail - rh->s_window + 1) {
					h = rh->roll(rh);
					if (i % rh->s_window == 0) {
						arr[i / rh->s_window] = h;
					}
				}
				return i;
			}
		}
	}
	return i + 1;
}

struct diag_vcdiff_script *
diag_vcdiff_contract(struct diag_rollinghash32 *rh)
{
	struct diag_vcdiff_script *script;
	struct diag_rbtree *tree;
	struct diag_rbtree_node *node, *n;
	register size_t s, i, a;
	uint32_t *arr, h;
	struct diag_vcdiff_pcode *p;

	assert(rh);
	assert(rh->size >= rh->s_window);
	s = rh->size / rh->s_window;
	if (s == 0) {
		return NULL;
	}

	tree = diag_rbtree_create(DIAG_CMP_IMMEDIATE);
	arr = diag_calloc((size_t)s, sizeof(uint32_t));
	arr[0] = rh->init(rh);
	for (i = 1; i < rh->size - rh->s_window + 1; i = lookback(rh, arr, i, h, tree)) {
		h = rh->roll(rh);
		if (i % rh->s_window == 0) {
			arr[i / rh->s_window] = h;
		}
	}
	diag_free(arr);

	a = 0;
	node = diag_rbtree_minimum(tree);
	while (node) {
		size_t b = (size_t)node->key;

		if (a < b) {
			p = pcode_add_new(b - a, rh->data + a);
			n = diag_rbtree_node_new((uintptr_t)a, (uintptr_t)p);
			diag_rbtree_insert(tree, n);
		}
		p = (struct diag_vcdiff_pcode *)node->attr;
		a = b + p->size;
		node = diag_rbtree_successor(node);
	}
	if (a < rh->size) {
		p = pcode_add_new(rh->size - a, rh->data + a);
		n = diag_rbtree_node_new((uintptr_t)a, (uintptr_t)p);
		diag_rbtree_insert(tree, n);
	}

	script = diag_malloc(sizeof(struct diag_vcdiff_script));
	script->source = NULL;
	script->s_pcodes = (size_t)tree->num_nodes;
	script->pcodes = diag_calloc(script->s_pcodes, sizeof(struct diag_vcdiff_pcode));
	node = diag_rbtree_minimum(tree);
	i = 0;
	do {
		script->pcodes[i++] = (*(struct diag_vcdiff_pcode *)node->attr);
	} while ( (node = diag_rbtree_successor(node)) );
	diag_rbtree_destroy(tree);
	return script;
}
