/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "test.h"

#include "diagonal.h"
#include "diagonal/private/system.h"

static char *ARGV[] = {
	"date",
	"+%Y%m%d"
};

int main(void)
{
	struct diag_command *c = diag_command_new(ARGV, NULL, NULL, NULL, NULL);
	ASSERT_NOT_NULL(c);
	struct diag_process *p = diag_run_program(c);
	ASSERT_NOT_NULL(p);
	diag_process_wait(p);
	ASSERT_EQ_INT(0, p->status);
	diag_process_destroy(p);
	remove(c->out);
	remove(c->err);
	diag_command_destroy(c);
	return EXIT_SUCCESS;
}
