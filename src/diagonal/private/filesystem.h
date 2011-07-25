/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_PRIVATE_FILESYSTEM_H
#define DIAGONAL_PRIVATE_FILESYSTEM_H

DIAG_C_DECL_BEGIN

DIAG_FUNCTION char **diag_paths(char **paths, size_t *);

DIAG_FUNCTION size_t diag_mmap_file(const char *, char **);

DIAG_C_DECL_END

#endif
