/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_DATUM_H
#define DIAGONAL_DATUM_H

/*
 * The datum structure
 * This is for representing a profile of interests.
 * Its instance consists of three members:
 * - tag,
 * - id, and
 * - value.
 * The `tag' contains information to deal with the resource of the datum.
 * The `id' assure the uniqueness in the domain of data.
 * The `value' is an opaque pointer referring to the data's main body.
 *
 * For example, in the domain of a local filesystem, a regular file has
 * its content as its main body. You can also specify each one by its path.
 * So in that case we name its content `value' and its path `id'.
 * Then what is `tag'?
 * To answer the question we should check how to manage the resource of
 * the file, e.g., its file descriptor or its mapped memory.
 * If you want to release the resource at the end of execution, mark as
 * "to be freed" in the `tag' and register a function to do that.
 */

DIAG_C_DECL_BEGIN

enum {
	DIAG_TAG_CUSTOMIZED = 0x01,
};

#define DIAG_DATUM_HEAD				\
	uint64_t tag;				\
	uintptr_t id;				\
	void *value

struct diag_datum {
	DIAG_DATUM_HEAD;
};

struct diag_customized_datum {
	DIAG_DATUM_HEAD;
	void (*finalize)(struct diag_datum *);
};

typedef void (*diag_datum_finalizer)(struct diag_datum *);

#define DIAG_DATUM_CUSTOMIZED_P(datum) ((datum)->tag & DIAG_TAG_CUSTOMIZED)

DIAG_FUNCTION struct diag_datum *diag_datum_create(uintptr_t, void *);

DIAG_FUNCTION struct diag_customized_datum *
diag_customized_datum_create(uintptr_t, void *, diag_datum_finalizer);

DIAG_FUNCTION void diag_datum_destroy(struct diag_datum *datum);

DIAG_C_DECL_END

#endif
