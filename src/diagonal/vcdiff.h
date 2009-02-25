#ifndef DIAGONAL_VCDIFF_H
#define DIAGONAL_VCDIFF_H

typedef struct {
	uint32_t length;
	uint32_t s_window;
	uint8_t indicator;
	uint32_t length_data;
	uint32_t length_inst;
	uint32_t length_addr;
	uint8_t *data;
	uint8_t *inst;
	uint8_t *addr;
} diag_vcdiff_delta_t;

enum {
	DIAG_VCD_DATACOMP = 0x1,
	DIAG_VCD_INSTCOMP = 0x2,
	DIAG_VCD_ADDRCOMP = 0x4,
};

#define DIAG_VCDIFF_DATACOMPP(delta)  ((delta)->indicator & DIAG_VCD_DATACOMP)
#define DIAG_VCDIFF_INSTCOMPP(delta)  ((delta)->indicator & DIAG_VCD_INSTCOMP)
#define DIAG_VCDIFF_ADDRCOMPP(delta)  ((delta)->indicator & DIAG_VCD_ADDRCOMP)

typedef struct {
	uint8_t indicator;
	uint32_t length_source_segment;
	uint32_t position_source_segment;
	diag_vcdiff_delta_t *delta;
} diag_vcdiff_window_t;

enum {
	DIAG_VCD_SOURCE = 0x1,
	DIAG_VCD_TARGET = 0x2,
};

#define DIAG_VCDIFF_SOURCEP(window) ((window)->indicator & DIAG_VCD_SOURCE)
#define DIAG_VCDIFF_TARGETP(window) ((window)->indicator & DIAG_VCD_TARGET)

typedef struct {
	uint8_t version;
	uint8_t indicator;
	uint8_t compressor_id;
	uint32_t length_code_table_data;
	uint8_t *code_table_data;
	uint32_t num_windows;
	diag_vcdiff_window_t **windows;
} diag_vcdiff_t;

enum {
	DIAG_VCD_DECOMPRESS = 0x1,
	DIAG_VCD_CODETABLE  = 0x2,
};

#define DIAG_VCDIFF_DECOMPRESSP(vcdiff) ((vcdiff)->indicator & DIAG_VCD_DECOMPRESS)
#define DIAG_VCDIFF_CODETABLEP(vcdiff)  ((vcdiff)->indicator & DIAG_VCD_CODETABLE)

typedef struct {
    uint32_t *near;     /* array of size s_near */
	uint8_t s_near;
	uint32_t next_slot; /* the circular index for near */
	uint32_t *same;     /* array of size s_same * 256 */
	uint8_t s_same;
} diag_vcdiff_cache_t;

#define DIAG_VCDIFF_S_NEAR_DEFAULT 4
#define DIAG_VCDIFF_S_SAME_DEFAULT 3

enum {
	DIAG_VCD_SELF = 0,
	DIAG_VCD_HERE = 1,
};

typedef struct {
	uint8_t inst1;
	uint8_t size1;
	uint8_t mode1;
	uint8_t inst2;
	uint8_t size2;
	uint8_t mode2;
} diag_vcdiff_code_t;

enum {
	DIAG_VCD_NOOP = 0,
	DIAG_VCD_ADD  = 1,
	DIAG_VCD_RUN  = 2,
	DIAG_VCD_COPY = 3,
};

typedef struct {
	uint8_t s_near;
	uint8_t s_same;
	diag_vcdiff_code_t *entries;
} diag_vcdiff_code_table_t;

typedef struct diag_vcdiff_context_s {
	uint8_t compatibility;
	diag_port_t *port;
} diag_vcdiff_context_t;

#define DIAG_VCDIFF_COMPATIBLEP(context) (context)->compatibility

extern diag_vcdiff_context_t *diag_vcdiff_context_new_fp(FILE *fp);
extern diag_vcdiff_context_t *diag_vcdiff_context_new_fd(int fd);
extern diag_vcdiff_context_t *diag_vcdiff_context_new_bm(uint8_t *head, uint32_t size);
extern diag_vcdiff_context_t *diag_vcdiff_context_new_path(const char *path);

extern void diag_vcdiff_context_destroy(diag_vcdiff_context_t *context);

extern diag_vcdiff_t *diag_vcdiff_read(diag_vcdiff_context_t *context);

typedef struct diag_vcdiff_vm_s {
	uint32_t s_source;
	uint8_t *source;
	uint32_t s_target;
	uint8_t *target;
	diag_vcdiff_cache_t *cache;
	diag_vcdiff_code_table_t *code_table;
	jmp_buf env;
	const uint8_t *src;
	const uint8_t *src_s;
	uint8_t *data;
	const uint8_t *data_s;
	uint8_t *inst;
	const uint8_t *inst_s;
	uint8_t *addr;
	const uint8_t *addr_s;
	void (*close)(struct diag_vcdiff_vm_s *vm);
	void (*error)(struct diag_vcdiff_vm_s *vm, const char *message, ...);
} diag_vcdiff_vm_t;

extern diag_vcdiff_vm_t *diag_vcdiff_vm_new(uint32_t s_source, uint8_t *source);
extern diag_vcdiff_vm_t *diag_vcdiff_vm_new_path(const char *path);
extern void diag_vcdiff_vm_destroy(diag_vcdiff_vm_t *vm);

extern uint8_t *diag_vcdiff_decode(diag_vcdiff_vm_t *vm, diag_vcdiff_t *vcdiff);

extern void diag_vcdiff_destroy(diag_vcdiff_t *vcdiff);

#endif
