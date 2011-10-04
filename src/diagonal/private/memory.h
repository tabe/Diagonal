/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_PRIVATE_MEMORY_H
#define DIAGONAL_PRIVATE_MEMORY_H

DIAG_C_DECL_BEGIN

DIAG_FUNCTION size_t diag_memory_to_lines(size_t s, const char *src,
					  char **dst, char ***lines);

DIAG_C_DECL_END

#endif
