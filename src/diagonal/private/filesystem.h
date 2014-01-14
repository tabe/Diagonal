/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_PRIVATE_FILESYSTEM_H
#define DIAGONAL_PRIVATE_FILESYSTEM_H

#define DIAG_MMAP_HEAD				\
	void *addr;				\
	size_t size

struct diag_mmap {
	DIAG_MMAP_HEAD;
};

enum diag_mmap_mode {
	DIAG_MMAP_RO = 0,
	DIAG_MMAP_COW
};

DIAG_C_DECL_BEGIN

/*
 * Scream and die unless `path' is an existing directory.
 */
DIAG_FUNCTION void diag_assert_directory(const char *path);

/*
 * Return 1 if `path' is a directory, 0 otherwise.
 * Return -1 in case of error.
 */
DIAG_FUNCTION int diag_is_directory(const char *path);

DIAG_FUNCTION char **diag_paths(char **paths, size_t *);

DIAG_FUNCTION struct diag_mmap *diag_mmap_file(const char *path, enum diag_mmap_mode mode);

DIAG_FUNCTION void diag_munmap(struct diag_mmap *);

DIAG_FUNCTION size_t diag_file_to_lines(const char *path,
					char **dst, char ***lines);

DIAG_C_DECL_END

#endif
