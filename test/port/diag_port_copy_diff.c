/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include <time.h>

#include "diagonal.h"
#include "diagonal/port.h"

int main(void)
{
	srand((int)time(NULL));

	int len = abs(rand())%32767 + 1; /* RAND_MAX is at least 32767 */
	uint8_t *buf = diag_malloc(len);
	int i;
	for (i = 0; i < len; i++) {
		buf[i] = (uint8_t)rand();
	}
	struct diag_port *p0 = diag_port_new_bm(buf, len, DIAG_PORT_INPUT);
	assert(p0);

	struct diag_port *p1 = diag_port_new_path("0.txt", "wb");
	if (!p1) {
		diag_port_destroy(p0);
		diag_free(buf);
		return EXIT_FAILURE;
	}

	ssize_t s = diag_port_copy(p0, p1);
	if (s < 0) {
		diag_port_destroy(p1);
		diag_remove("0.txt");
		diag_port_destroy(p0);
		diag_free(buf);
		return EXIT_FAILURE;
	}
	ASSERT_EQ_INT(len, s);
	diag_port_destroy(p1);
	diag_port_destroy(p0);

	p0 = diag_port_new_bm(buf, len, DIAG_PORT_INPUT);
	assert(p0);
	struct diag_port *p2 = diag_port_new_path("0.txt", "rb");
	if (!p2) {
		diag_remove("0.txt");
		diag_port_destroy(p0);
		diag_free(buf);
		return EXIT_FAILURE;
	}
	int r = diag_port_diff(p0, p2);
	diag_remove("0.txt");
	diag_port_destroy(p2);
	diag_port_destroy(p0);
	diag_free(buf);
	return r;
}
