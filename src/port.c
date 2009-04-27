#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
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
#include "diagonal/port.h"

static int
read_byte_fd(diag_port_t *port, uint8_t *i)
{
	int r;

	assert(port && i);
	r = (int)read(port->stream.fd, (void *)i, 1);
	if (r > 0) port->i_pos += r;
	if (r == -1) diag_error("read error: %s", strerror(errno));
	return r;
}

static int
read_bytes_fd(diag_port_t *port, size_t size, uint8_t *buf)
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
write_byte_fd(diag_port_t *port, uint8_t i)
{
	int r;

	assert(port);
	r = (int)write(port->stream.fd, (const void *)&i, 1);
	if (r > 0) port->o_pos += r;
	if (r == -1) diag_error("write error: %s", strerror(errno));
	return r;
}

static int
write_bytes_fd(diag_port_t *port, size_t size, const uint8_t *buf)
{
	ssize_t s;

	assert(port && buf);
	if (size == 0) return 1; /* nothing to do */
	if ( (size_t)(s = write(port->stream.fd, (const void *)buf, size)) == size) {
		port->o_pos += size;
		return 1;
	} else if (s >= 0) {
		port->o_pos += (size_t)s;
		return 0;
	} else {
		diag_error("write error: %s", strerror(errno));
		return -1;
	}
}

static int
read_byte_fp(diag_port_t *port, uint8_t *i)
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
read_bytes_fp(diag_port_t *port, size_t size, uint8_t *buf)
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
write_byte_fp(diag_port_t *port, uint8_t i)
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
write_bytes_fp(diag_port_t *port, size_t size, const uint8_t *buf)
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
close_fp(diag_port_t *port)
{
	assert(port);
	(void)fclose(port->stream.fp);
}

static int
read_byte_bm(diag_port_t *port, uint8_t *i)
{
	assert(port && i);
	if (port->i_pos < port->stream.bm.size) {
		*i = port->stream.bm.head[port->i_pos++];
		return 1;
	}
	return 0;
}

static int
read_bytes_bm(diag_port_t *port, size_t size, uint8_t *buf)
{
	assert(port && buf);
	if (port->i_pos + size <= port->stream.bm.size) {
		(void)memmove((void *)buf, (const void *)port->stream.bm.head + port->i_pos, size);
		port->i_pos += size;
		return 1;
	}
	return 0;
}

static int
write_byte_bm(diag_port_t *port, uint8_t i)
{
	assert(port);
	if (port->o_pos < port->stream.bm.size) {
		port->stream.bm.head[port->o_pos++] = i;
		return 1;
	}
	return 0;
}

static int
write_bytes_bm(diag_port_t *port, size_t size, const uint8_t *buf)
{
	assert(port && buf);
	if (port->o_pos + size <= port->stream.bm.size) {
		(void)memmove((void *)port->stream.bm.head + port->o_pos, (const void *)buf, size);
		port->o_pos += size;
		return 1;
	}
	return 0;
}

diag_port_t *
diag_port_new_fd(int fd, uint8_t flags)
{
	diag_port_t *port;

	port = (diag_port_t *)diag_malloc(sizeof(diag_port_t));
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

diag_port_t *
diag_port_new_fp(FILE *fp, uint8_t flags)
{
	diag_port_t *port;

	port = (diag_port_t *)diag_malloc(sizeof(diag_port_t));
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

diag_port_t *
diag_port_new_bm(uint8_t *head, uint32_t size, uint8_t flags)
{
	diag_port_t *port;

	assert(head);
	port = (diag_port_t *)diag_malloc(sizeof(diag_port_t));
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

diag_port_t *
diag_port_new_path(const char *path, const char *mode)
{
	diag_port_t *port;
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

void
diag_port_destroy(diag_port_t *port)
{
	if (!port) return;
	if (port->close) port->close(port);
	diag_free(port);
}
