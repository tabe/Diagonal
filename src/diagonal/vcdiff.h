/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_VCDIFF_H
#define DIAGONAL_VCDIFF_H

struct diag_vcdiff_delta {
	uint32_t length;
	uint32_t s_window;
	uint8_t indicator;
	uint32_t length_data;
	uint32_t length_inst;
	uint32_t length_addr;
	uint8_t *data;
	uint8_t *inst;
	uint8_t *addr;
};

enum {
	DIAG_VCD_DATACOMP = 0x1,
	DIAG_VCD_INSTCOMP = 0x2,
	DIAG_VCD_ADDRCOMP = 0x4,
};

#define DIAG_VCDIFF_DATACOMPP(delta)  ((delta)->indicator & DIAG_VCD_DATACOMP)
#define DIAG_VCDIFF_INSTCOMPP(delta)  ((delta)->indicator & DIAG_VCD_INSTCOMP)
#define DIAG_VCDIFF_ADDRCOMPP(delta)  ((delta)->indicator & DIAG_VCD_ADDRCOMP)

struct diag_vcdiff_window {
	uint8_t indicator;
	uint32_t length_source_segment;
	uint32_t position_source_segment;
	struct diag_vcdiff_delta *delta;
};

enum {
	DIAG_VCD_SOURCE = 0x1,
	DIAG_VCD_TARGET = 0x2,
};

#define DIAG_VCDIFF_SOURCEP(window) ((window)->indicator & DIAG_VCD_SOURCE)
#define DIAG_VCDIFF_TARGETP(window) ((window)->indicator & DIAG_VCD_TARGET)

struct diag_vcdiff {
	uint8_t version;
	uint8_t indicator;
	uint8_t compressor_id;
	uint32_t length_code_table_data;
	uint8_t *code_table_data;
	uint32_t num_windows;
	struct diag_vcdiff_window **windows;
};

enum {
	DIAG_VCD_DECOMPRESS = 0x1,
	DIAG_VCD_CODETABLE  = 0x2,
};

#define DIAG_VCDIFF_DECOMPRESSP(vcdiff) ((vcdiff)->indicator & DIAG_VCD_DECOMPRESS)
#define DIAG_VCDIFF_CODETABLEP(vcdiff)  ((vcdiff)->indicator & DIAG_VCD_CODETABLE)

struct diag_vcdiff_cache {
    uint32_t *near;     /* array of size s_near */
	uint8_t s_near;
	uint32_t next_slot; /* the circular index for near */
	uint32_t *same;     /* array of size s_same * 256 */
	uint8_t s_same;
};

#define DIAG_VCDIFF_S_NEAR_DEFAULT 4
#define DIAG_VCDIFF_S_SAME_DEFAULT 3

enum {
	DIAG_VCD_SELF = 0,
	DIAG_VCD_HERE = 1,
};

struct diag_vcdiff_code {
	uint8_t inst1;
	uint8_t size1;
	uint8_t mode1;
	uint8_t inst2;
	uint8_t size2;
	uint8_t mode2;
};

enum {
	DIAG_VCD_NOOP = 0,
	DIAG_VCD_ADD  = 1,
	DIAG_VCD_RUN  = 2,
	DIAG_VCD_COPY = 3,
};

struct diag_vcdiff_code_table {
	uint8_t s_near;
	uint8_t s_same;
	struct diag_vcdiff_code *entries;
};

struct diag_vcdiff_context {
	uint8_t compatibility;
	struct diag_port *port;
};

#define DIAG_VCDIFF_COMPATIBLEP(context) (context)->compatibility

struct diag_vcdiff_vm {
	struct diag_bytevector *source;
	uint32_t s_target;
	uint8_t *target;
	struct diag_vcdiff_cache *cache;
	struct diag_vcdiff_code_table *code_table;
	jmp_buf env;
	const uint8_t *src;
	const uint8_t *src_s;
	uint8_t *data;
	const uint8_t *data_s;
	uint8_t *inst;
	const uint8_t *inst_s;
	uint8_t *addr;
	const uint8_t *addr_s;
	void (*close)(struct diag_vcdiff_vm *vm);
	void (*error)(struct diag_vcdiff_vm *vm, const char *message, ...);
};

struct diag_vcdiff_pcode {
	int inst;
	diag_size_t size;
	union {
		diag_size_t addr;
		uint8_t *data;
		uint8_t byte;
	} attr;
};

struct diag_vcdiff_script {
	const uint8_t *source;
	diag_size_t s_pcodes;
	struct diag_vcdiff_pcode *pcodes;
};

DIAG_C_DECL_BEGIN

DIAG_FUNCTION struct diag_vcdiff_context *diag_vcdiff_context_new_fp(FILE *fp);
DIAG_FUNCTION struct diag_vcdiff_context *diag_vcdiff_context_new_fd(int fd);
DIAG_FUNCTION struct diag_vcdiff_context *diag_vcdiff_context_new_bm(uint8_t *head, uint32_t size);
DIAG_FUNCTION struct diag_vcdiff_context *diag_vcdiff_context_new_path(const char *path);

DIAG_FUNCTION void diag_vcdiff_context_destroy(struct diag_vcdiff_context *context);

DIAG_FUNCTION struct diag_vcdiff *diag_vcdiff_read(struct diag_vcdiff_context *context);

DIAG_FUNCTION struct diag_vcdiff_vm *diag_vcdiff_vm_new(uint32_t s_source, uint8_t *source);
DIAG_FUNCTION struct diag_vcdiff_vm *diag_vcdiff_vm_new_path(const char *path);
DIAG_FUNCTION void diag_vcdiff_vm_destroy(struct diag_vcdiff_vm *vm);

DIAG_FUNCTION uint8_t *diag_vcdiff_decode(struct diag_vcdiff_vm *vm, struct diag_vcdiff *vcdiff);

DIAG_FUNCTION void diag_vcdiff_destroy(struct diag_vcdiff *vcdiff);

DIAG_FUNCTION void diag_vcdiff_script_destroy(struct diag_vcdiff_script *script);

DIAG_FUNCTION uint8_t *diag_vcdiff_expand(const struct diag_vcdiff_script *script, diag_size_t *size);
DIAG_FUNCTION struct diag_vcdiff_script *diag_vcdiff_contract(struct diag_rollinghash32 *rh);

DIAG_C_DECL_END

#endif
