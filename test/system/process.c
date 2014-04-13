/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/private/system.h"

int main(void)
{
	char *argv[3];
	argv[0] = diag_strdup("date");
	argv[1] = diag_strdup("+%Y%m%d");
	argv[2] = NULL;
	struct diag_command *c = diag_command_new(argv, NULL, NULL, NULL, NULL);
	ASSERT_NOT_NULL(c);
	struct diag_process *p = diag_run_program(c);
	ASSERT_NOT_NULL(p);
	diag_process_wait(p);
	ASSERT_EQ_INT(0, p->status);
	diag_process_destroy(p);
	diag_remove(c->out);
	diag_remove(c->err);
	diag_command_destroy(c);
	diag_free(argv[0]);
	diag_free(argv[1]);
	return EXIT_SUCCESS;
}
