/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/port.h"

int
main()
{
	struct diag_port *port;
	uint8_t b;
#define BUFLEN 2
	uint8_t buf[BUFLEN];

	port = diag_port_new_path(__FILE__, "rb");
	ASSERT_NOT_NULL(port);
	ASSERT_TRUE(DIAG_PORT_INPUT_P(port));
	ASSERT_FALSE(DIAG_PORT_OUTPUT_P(port));
	ASSERT_TRUE(DIAG_PORT_FP_P(port));
	ASSERT_EQ_INT(1, port->read_byte(port, &b));
	ASSERT_EQ_UINT8(47, b); /* '/' */
	ASSERT_EQ_SIZE(1, port->i_pos);
	ASSERT_EQ_INT(1, port->read_bytes(port, BUFLEN, buf));
	ASSERT_EQ_UINT8(42, buf[0]); /* '*' */
	ASSERT_EQ_UINT8(32, buf[1]); /* ' ' */
	ASSERT_EQ_SIZE(3, port->i_pos);
	diag_port_destroy(port);
	return EXIT_SUCCESS;
}
