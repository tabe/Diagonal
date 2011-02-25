/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_PORT_H
#define DIAGONAL_PORT_H

enum {
	DIAG_PORT_INPUT  = 1,
	DIAG_PORT_OUTPUT = 1<<1,
	DIAG_PORT_BOTH   = 3,

	DIAG_PORT_FD     = 1<<2,
	DIAG_PORT_FP     = 2<<2,
	DIAG_PORT_BM     = 3<<2,
};

#define DIAG_PORT_INPUT_P(port)  ((port)->tag & DIAG_PORT_INPUT)
#define DIAG_PORT_OUTPUT_P(port) ((port)->tag & DIAG_PORT_OUTPUT)
#define DIAG_PORT_TYPE(port) ((port)->tag&(~DIAG_PORT_BOTH))
#define DIAG_PORT_FD_P(port) (DIAG_PORT_TYPE(port) == DIAG_PORT_FD)
#define DIAG_PORT_FP_P(port) (DIAG_PORT_TYPE(port) == DIAG_PORT_FP)
#define DIAG_PORT_BM_P(port) (DIAG_PORT_TYPE(port) == DIAG_PORT_BM)

struct diag_port {
	uint8_t tag;
	union {
		int fd;
		FILE *fp;
		struct {
			uint8_t *head;
			uint32_t size;
		} bm;
	} stream;
	size_t i_pos;
	size_t o_pos;
	int (*read_byte)(struct diag_port *port, uint8_t *i);
	int (*read_bytes)(struct diag_port *port, size_t size, uint8_t *buf);
	int (*write_byte)(struct diag_port *port, uint8_t i);
	int (*write_bytes)(struct diag_port *port, size_t size, const uint8_t *buf);
	void (*close)(struct diag_port *port);
};

DIAG_C_DECL_BEGIN

extern struct diag_port *diag_port_new_fd(int fd, uint8_t flags);
extern struct diag_port *diag_port_new_fp(FILE *fp, uint8_t flags);
extern struct diag_port *diag_port_new_bm(uint8_t *head, uint32_t size, uint8_t flags);
extern struct diag_port *diag_port_new_path(const char *path, const char *mode);

extern void diag_port_destroy(struct diag_port *port);

DIAG_C_DECL_END

#endif
