/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/bytevector.h"
#include "diagonal/private/filesystem.h"

struct diag_bytevector_ex {
	DIAG_BYTEVECTOR_HEAD;
	/* extensions */
	struct diag_mmap *mm;
};

static void
bytevector_free(struct diag_bytevector *bv)
{
	assert(bv);
	diag_free(bv->data);
}

struct diag_bytevector *
diag_bytevector_new_heap(size_t size, uint8_t *data)
{
	struct diag_bytevector *bv;

	bv = diag_malloc(sizeof(struct diag_bytevector));
	bv->size = size;
	bv->data = data;
	bv->finalize = bytevector_free;
	return bv;
}

static void
bytevector_munmap(struct diag_bytevector *bv)
{
	assert(bv);
	struct diag_bytevector_ex *bve = (struct diag_bytevector_ex *)bv;
	diag_munmap(bve->mm);
}

struct diag_bytevector *
diag_bytevector_new_path(const char *path)
{
	assert(path);
	struct diag_mmap *mm = diag_mmap_file(path, DIAG_MMAP_RO);
	if (!mm) return NULL;
	struct diag_bytevector_ex *bve = diag_malloc(sizeof(*bve));
	bve->size = mm->size;
	bve->data = mm->addr;
	bve->finalize = bytevector_munmap;
	bve->mm = mm;
	return (struct diag_bytevector *)bve;
}

char *
diag_bytevector_to_asciz(const struct diag_bytevector *bv)
{
	char *s;

	assert(bv);
	s = diag_malloc(bv->size + 1);
	(void)memcpy(s, bv->data, bv->size);
	s[bv->size] = '\0';
	return s;
}

void
diag_bytevector_destroy(struct diag_bytevector *bv)
{
	if (!bv) return;
	if (bv->finalize) bv->finalize(bv);
	diag_free(bv);
}
