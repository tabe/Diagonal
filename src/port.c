/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/port.h"

static const int BUFFER_LENGTH = 4096;

static int
read_byte_fd(struct diag_port *port, uint8_t *i)
{
	int r;

	assert(port && i);
	r = (int)read(port->stream.fd, (void *)i, 1);
	if (r > 0) port->i_pos += r;
	if (r == -1) diag_error("read error: %s", strerror(errno));
	return r;
}

static int
read_bytes_fd(struct diag_port *port, size_t size, uint8_t *buf)
{
	ssize_t s;

	assert(port && buf);
	if (size == 0) return 1; /* nothing to do */
	if ( (size_t)(s = read(port->stream.fd, (void *)buf, size)) == size) {
		port->i_pos += size;
		return 1;
	} else if (s >= 0) {
		port->i_pos += (size_t)s;
		return 0;
	} else {
		diag_error("read error: %s", strerror(errno));
		return -1;
	}
}

static int
write_byte_fd(struct diag_port *port, uint8_t i)
{
	int r;

	assert(port);
 retry:
	r = (int)write(port->stream.fd, (const void *)&i, 1);
	if (r > 0) port->o_pos += r;
	if (r == -1) {
		if (errno == EINTR) goto retry;
		diag_error("write error: %s", strerror(errno));
	}
	return r;
}

static int
write_bytes_fd(struct diag_port *port, size_t size, const uint8_t *buf)
{
	ssize_t s;

	assert(port && buf);
	if (size == 0) return 1; /* nothing to do */
 retry:
	if ( (size_t)(s = write(port->stream.fd, (const void *)buf, size)) == size) {
		port->o_pos += size;
		return 1;
	} else if (s >= 0) {
		port->o_pos += (size_t)s;
		return 0;
	} else {
		if (errno == EINTR) goto retry;
		diag_error("write error: %s", strerror(errno));
		return -1;
	}
}

static int
read_byte_fp(struct diag_port *port, uint8_t *i)
{
	FILE *fp;
	size_t s;

	assert(port && i);
	fp = port->stream.fp;
	s = fread(i, sizeof(uint8_t), 1, fp);
	port->i_pos += s;
	if (s < 1) {
		if (feof(fp)) return 0;
		diag_error("read error");
		return -1;
	}
	return 1;
}

static int
read_bytes_fp(struct diag_port *port, size_t size, uint8_t *buf)
{
	FILE *fp;
	size_t s;

	assert(port && buf);
	if (size == 0) return 1; /* nothing to do */
	fp = port->stream.fp;
	s = fread(buf, sizeof(uint8_t), size, fp);
	port->i_pos += s;
	if (s < size) {
		if (feof(fp)) return 0;
		diag_error("read error");
		return -1;
	}
	return 1;
}

static int
write_byte_fp(struct diag_port *port, uint8_t i)
{
	FILE *fp;
	size_t s;

	assert(port);
	fp = port->stream.fp;
	s = fwrite((const void *)&i, sizeof(uint8_t), 1, fp);
	port->o_pos += s;
	if (s < 1) {
		if (feof(fp)) return 0;
		diag_error("write error");
		return -1;
	}
	return 1;
}

static int
write_bytes_fp(struct diag_port *port, size_t size, const uint8_t *buf)
{
	FILE *fp;
	size_t s;

	assert(port && buf);
	if (size == 0) return 1; /* nothing to do */
	fp = port->stream.fp;
	s = fwrite((const void *)buf, sizeof(uint8_t), size, fp);
	port->o_pos += s;
	if (s < size) {
		if (feof(fp)) return 0;
		diag_error("write error");
		return -1;
	}
	return 1;
}

static void
close_fp(struct diag_port *port)
{
	assert(port);
	(void)fclose(port->stream.fp);
}

static int
read_byte_bm(struct diag_port *port, uint8_t *i)
{
	assert(port && i);
	if (port->i_pos < port->stream.bm.size) {
		*i = port->stream.bm.head[port->i_pos++];
		return 1;
	}
	return 0;
}

static int
read_bytes_bm(struct diag_port *port, size_t size, uint8_t *buf)
{
	assert(port && buf);
	size_t len = port->stream.bm.size - port->i_pos;
	if (size <= len) {
		(void)memmove(buf, port->stream.bm.head + port->i_pos, size);
		port->i_pos += size;
		return 1;
	} else {
		(void)memmove(buf, port->stream.bm.head + port->i_pos, len);
		port->i_pos += len;
		return 0;
	}
	return -1;
}

static int
write_byte_bm(struct diag_port *port, uint8_t i)
{
	assert(port);
	if (port->o_pos < port->stream.bm.size) {
		port->stream.bm.head[port->o_pos++] = i;
		return 1;
	}
	return 0;
}

static int
write_bytes_bm(struct diag_port *port, size_t size, const uint8_t *buf)
{
	assert(port && buf);
	if (port->o_pos + size <= port->stream.bm.size) {
		(void)memmove((void *)port->stream.bm.head + port->o_pos, (const void *)buf, size);
		port->o_pos += size;
		return 1;
	}
	return 0;
}

struct diag_port *
diag_port_new_fd(int fd, uint8_t flags)
{
	struct diag_port *port;

	port = diag_malloc(sizeof(struct diag_port));
	port->tag = DIAG_PORT_FD|flags;
	port->stream.fd = fd;
	if (DIAG_PORT_INPUT_P(port)) {
		port->read_byte = read_byte_fd;
		port->read_bytes = read_bytes_fd;
		port->i_pos = 0;
	}
	if (DIAG_PORT_OUTPUT_P(port)) {
		port->write_byte = write_byte_fd;
		port->write_bytes = write_bytes_fd;
		port->o_pos = 0;
	}
	port->close = NULL;
	return port;
}

struct diag_port *
diag_port_new_fp(FILE *fp, uint8_t flags)
{
	struct diag_port *port;

	port = diag_malloc(sizeof(struct diag_port));
	port->tag = DIAG_PORT_FP|flags;
	port->stream.fp = fp;
	if (DIAG_PORT_INPUT_P(port)) {
		port->read_byte = read_byte_fp;
		port->read_bytes = read_bytes_fp;
		port->i_pos = 0;
	}
	if (DIAG_PORT_OUTPUT_P(port)) {
		port->write_byte = write_byte_fp;
		port->write_bytes = write_bytes_fp;
		port->o_pos = 0;
	}
	port->close = NULL;
	return port;
}

struct diag_port *
diag_port_new_bm(uint8_t *head, uint32_t size, uint8_t flags)
{
	struct diag_port *port;

	assert(head);
	port = diag_malloc(sizeof(struct diag_port));
	port->tag = DIAG_PORT_BM|flags;
	port->stream.bm.head = head;
	port->stream.bm.size = size;
	if (DIAG_PORT_INPUT_P(port)) {
		port->read_byte = read_byte_bm;
		port->read_bytes = read_bytes_bm;
		port->i_pos = 0;
	}
	if (DIAG_PORT_OUTPUT_P(port)) {
		port->write_byte = write_byte_bm;
		port->write_bytes = write_bytes_bm;
		port->o_pos = 0;
	}
	port->close = NULL;
	return port;
}

struct diag_port *
diag_port_new_path(const char *path, const char *mode)
{
	struct diag_port *port;
	FILE *fp;
	uint8_t flags = 0;

	assert(path && mode);
	fp = fopen(path, mode);
	if (!fp) {
		return NULL;
	}
	switch (mode[0]) {
	case 'r':
		flags |= DIAG_PORT_INPUT;
		if (mode[1] == '+') flags |= DIAG_PORT_OUTPUT;
		break;
	case 'w':
	case 'a':
		flags |= DIAG_PORT_OUTPUT;
		if (mode[1] == '+') flags |= DIAG_PORT_INPUT;
		break;
	default:
		fclose(fp);
		return NULL;
	}
	port = diag_port_new_fp(fp, flags);
	port->close = close_fp;
	return port;
}

struct diag_port *diag_port_new_stdin(void)
{
	return diag_port_new_fd(STDIN_FILENO, DIAG_PORT_INPUT);
}

struct diag_port *diag_port_new_stdout(void)
{
	return diag_port_new_fd(STDOUT_FILENO, DIAG_PORT_OUTPUT);
}

ssize_t diag_port_copy(struct diag_port *iport, struct diag_port *oport)
{
	ssize_t pos = 0;
	uint8_t *buf = diag_malloc(BUFFER_LENGTH);
	int r = iport->read_bytes(iport, BUFFER_LENGTH, buf);
	while (r > 0) {
		int s = oport->write_bytes(oport, BUFFER_LENGTH, buf);
		if (s < 0) {
			pos = s;
			goto done;
		}
		pos += BUFFER_LENGTH;
		r = iport->read_bytes(iport, BUFFER_LENGTH, buf);
	}
	if (r < 0) {
		pos = r;
		goto done;
	}
	size_t len = iport->i_pos - pos;
	if (len > 0) {
		int s = oport->write_bytes(oport, len, buf);
		if (s < 0) {
			pos = s;
			goto done;
		}
		pos += len;
	}
 done:
	diag_free(buf);
	return pos;
}

int diag_port_diff(struct diag_port *iport1, struct diag_port *iport2)
{
	int r = 0;
	int r1;
	int r2;
	uint8_t *buf = diag_malloc(BUFFER_LENGTH * 2);
	uint8_t *buf1 = buf;
	uint8_t *buf2 = buf + BUFFER_LENGTH;
	size_t pos1 = iport1->i_pos;
	size_t pos2 = iport2->i_pos;
	size_t len1;
	size_t len2;

 iter:
	r1 = iport1->read_bytes(iport1, BUFFER_LENGTH, buf1);
	if (r1 < 0) {
		r = r1;
		goto done;
	}
	r2 = iport2->read_bytes(iport2, BUFFER_LENGTH, buf2);
	if (r2 < 0) {
		r = r2;
		goto done;
	}
	if (r1 != r2) {
		r = 1;
		goto done;
	}
	if (r1 == 1) {
		assert(r2 == 1);
		if (memcmp(buf1, buf2, BUFFER_LENGTH) != 0) {
			r = 1;
			goto done;
		}
		pos1 = iport1->i_pos;
		pos2 = iport2->i_pos;
		goto iter;
	}
	assert(r1 == 0 && r2 == 0);
	len1 = iport1->i_pos - pos1;
	len2 = iport2->i_pos - pos2;
	if (len1 != len2) {
		r = 1;
		goto done;
	}
	if (len1 > 0) {
		assert(len2 > 0);
		if (memcmp(buf1, buf2, len1) != 0) {
			r = 1;
		}
	}

 done:
	diag_free(buf);
	return r;
}

void
diag_port_destroy(struct diag_port *port)
{
	if (!port) return;
	if (port->close) port->close(port);
	diag_free(port);
}
