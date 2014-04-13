/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/port.h"
#include "diagonal/line.h"

#define ASSERT_NO_ERROR()						\
	if (e != DIAG_LINE_ERROR_OK) {					\
		printf("unintented error: %d\n", e);			\
		diag_line_context_destroy(context);			\
		diag_port_destroy(port);				\
		close(fd);						\
		exit(EXIT_FAILURE);					\
	}

int main(void)
{
	int fd;
	struct diag_port *port;
	struct diag_line_context *context;
	enum diag_line_error_e e;
	size_t size;
	char *line;

	fd = open(__FILE__, O_RDONLY);
	if (fd < 0) {
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	port = diag_port_new_fd(fd, DIAG_PORT_INPUT);
	ASSERT_NOT_NULL(port);
	context = diag_line_context_new(port);
	ASSERT_NOT_NULL(context);

	e = diag_line_read(context, &size, &line);
	ASSERT_NO_ERROR();
	ASSERT_EQ_SIZE(75, size);
	ASSERT_EQ_STRING("/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */", line);
	diag_free(line);

	e = diag_line_read(context, &size, &line);
	ASSERT_NO_ERROR();
	ASSERT_EQ_SIZE(17, size);
	ASSERT_EQ_STRING("#include \"test.h\"", line);
	diag_free(line);

	e = diag_line_read(context, &size, &line);
	ASSERT_NO_ERROR();
	ASSERT_EQ_SIZE(0, size);
	ASSERT_EQ_STRING("", line);
	diag_free(line);

	diag_line_context_destroy(context);
	diag_port_destroy(port);
	close(fd);
	return EXIT_SUCCESS;
}
