#include "test.h"

#include "diagonal.h"
#include "diagonal/port.h"

int
main()
{
	struct diag_port *port;
	uint8_t b;
#define BUFLEN 3
	uint8_t bm[BUFLEN+2] = "abcd";
	uint8_t obuf[BUFLEN] = "12";
	uint8_t ibuf[BUFLEN+1];

	port = diag_port_new_bm((uint8_t *)bm, BUFLEN+1, DIAG_PORT_BOTH);
	ASSERT_NOT_NULL(port);
	ASSERT_TRUE(DIAG_PORT_INPUT_P(port));
	ASSERT_TRUE(DIAG_PORT_OUTPUT_P(port));
	ASSERT_TRUE(DIAG_PORT_BM_P(port));

	ASSERT_EQ_INT(1, port->write_byte(port, (uint8_t)'0'));
	ASSERT_EQ_SIZE(1, port->o_pos);
	ASSERT_EQ_INT(1, port->read_byte(port, &b));
	ASSERT_EQ_UINT8('0', b);
	ASSERT_EQ_SIZE(1, port->i_pos);
	ASSERT_EQ_INT(1, port->write_bytes(port, BUFLEN-1, obuf));
	ASSERT_EQ_SIZE(3, port->o_pos);
	ASSERT_EQ_INT(1, port->read_bytes(port, BUFLEN, ibuf));
	ASSERT_EQ_UINT8('1', ibuf[0]);
	ASSERT_EQ_UINT8('2', ibuf[1]);
	ASSERT_EQ_UINT8('d', ibuf[2]);
	ASSERT_EQ_SIZE(4, port->i_pos);

	diag_port_destroy(port);
	return EXIT_SUCCESS;
}
