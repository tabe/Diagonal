/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/port.h"

int main(void)
{
	struct diag_port *port;
	int fd;
	uint8_t b;
#define BUFLEN 2
	uint8_t buf[BUFLEN];

	fd = open(__FILE__, O_RDONLY);
	if (fd < 0) {
		perror(NULL);
		return 1;
	}
	port = diag_port_new_fd(fd, DIAG_PORT_INPUT);
	ASSERT_NOT_NULL(port);
	ASSERT_TRUE(DIAG_PORT_INPUT_P(port));
	ASSERT_FALSE(DIAG_PORT_OUTPUT_P(port));
	ASSERT_TRUE(DIAG_PORT_FD_P(port));
	ASSERT_EQ_INT(1, diag_port_read_byte(port, &b));
	ASSERT_EQ_UINT8(47, b); /* '/' */
	ASSERT_EQ_SIZE(1, port->i_pos);
	ASSERT_EQ_INT(1, diag_port_read_bytes(port, BUFLEN, buf));
	ASSERT_EQ_UINT8(42, buf[0]); /* '*' */
	ASSERT_EQ_UINT8(32, buf[1]); /* ' ' */
	ASSERT_EQ_SIZE(3, port->i_pos);
	diag_port_destroy(port);
	close(fd);

	port = diag_port_new_fd(STDOUT_FILENO, DIAG_PORT_OUTPUT);
	ASSERT_NOT_NULL(port);
	ASSERT_FALSE(DIAG_PORT_INPUT_P(port));
	ASSERT_TRUE(DIAG_PORT_OUTPUT_P(port));
	ASSERT_TRUE(DIAG_PORT_FD_P(port));
	ASSERT_EQ_INT(1, diag_port_write_byte(port, 'f'));
	ASSERT_EQ_SIZE(1, port->o_pos);
	ASSERT_EQ_INT(1, diag_port_write_bytes(port, 3, (const uint8_t *)"oo\n"));
	ASSERT_EQ_SIZE(4, port->o_pos);
	diag_port_destroy(port);
	return EXIT_SUCCESS;
}
