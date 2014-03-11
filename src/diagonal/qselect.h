/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_QSELECT_H
#define DIAGONAL_QSELECT_H

/*
 * Find (0-based) `k'th element in `nmemb' elements by quickselect.
 * Note that it is destructive i.e. rearranges given data in-place.
 */
DIAG_FUNCTION void *diag_qselect(void *base, size_t nmemb, size_t size,
				 int (*cmp)(const void *, const void *),
				 size_t k);

#endif
