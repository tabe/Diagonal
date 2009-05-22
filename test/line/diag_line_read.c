#include "test.h"

#include "diagonal.h"
#include "diagonal/line.h"

#define ASSERT_NO_ERROR(context)							\
	if (DIAG_LINE_HAS_ERROR(context)) {						\
		printf("unintented error: %d\n", (context)->error);	\
		diag_line_context_destroy(context);					\
		diag_port_destroy(port);							\
		close(fd);											\
		exit(EXIT_FAILURE);											\
	}

int
main()
{
	int fd;
	diag_port_t *port;
	diag_line_context_t *context;
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

	context = diag_line_read(context, &size, &line);
	ASSERT_NO_ERROR(context);
	ASSERT_EQ_SIZE(17, size);
	ASSERT_EQ_STRING("#include \"test.h\"", line);
	diag_free(line);

	context = diag_line_read(context, &size, &line);
	ASSERT_NO_ERROR(context);
	ASSERT_EQ_SIZE(0, size);
	ASSERT_EQ_STRING("", line);
	diag_free(line);

	context = diag_line_read(context, &size, &line);
	ASSERT_NO_ERROR(context);
	ASSERT_EQ_SIZE(21, size);
	ASSERT_EQ_STRING("#include \"diagonal.h\"", line);
	diag_free(line);

	diag_line_context_destroy(context);
	diag_port_destroy(port);
	close(fd);
	return EXIT_SUCCESS;
}
