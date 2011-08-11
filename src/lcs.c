/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "diagonal.h"
#include "diagonal/pair.h"
#include "diagonal/list.h"
#include "diagonal/set.h"
#include "diagonal/vector.h"
#include "diagonal/lcs.h"

static int eq_default(intptr_t x, intptr_t y)
{
	return x == y;
}

struct diag_onp {
	const struct diag_vector *a, *b;
	int (*eq)(intptr_t, intptr_t);
	int m, n, u;
	int *fp;
	intptr_t *path;
	struct diag_set *s;
};

static int fp_ref(int i, const struct diag_onp *onp)
{
	return onp->fp[i];
}

static void fp_set(int i, int v, struct diag_onp *onp)
{
	onp->fp[i] = v;
}

static intptr_t path_ref(int i, const struct diag_onp *onp)
{
	return onp->path[i];
}

static void path_set(int i, intptr_t v, struct diag_onp *onp)
{
	onp->path[i] = v;
}

static void path_push(int i, intptr_t v, struct diag_onp *onp)
{
	struct diag_pair *p;
	intptr_t w;

	w = path_ref(i, onp);
	p = diag_pair_create(v, w);
	diag_set_insert(onp->s, (intptr_t)p);
	path_set(i, (intptr_t)p, onp);
}

static void snake(int k, struct diag_onp *onp)
{
	int x, y, y1, y2;
	intptr_t v, vx, vy;

	y1 = fp_ref(k - 1, onp) + 1;
	y2 = fp_ref(k + 1, onp);
	y = (y1 < y2) ? y2 : y1; /* max(y1, y2) */
	if (y1 > y2) {
		v = path_ref(k - 1, onp);
		path_set(k, v, onp);
		path_push(k, onp->u, onp);
	} else {
		v = path_ref(k + 1, onp);
		path_set(k, v, onp);
		path_push(k, -(onp->u), onp);
	}
	x = y - k;
	while (x < onp->m && y < onp->n) {
		vx = diag_vector_ref(onp->a, x);
		vy = diag_vector_ref(onp->b, y);
		if (!onp->eq(vx, vy)) break;
		path_push(k, 0, onp);
		x++, y++;
	}
	fp_set(k, y, onp);
}

/* API */

struct diag_lcs *diag_lcs_create(const struct diag_vector *vx,
				 const struct diag_vector *vy,
				 diag_lcs_eq_t eq)
{
	struct diag_lcs *lcs;

	lcs = diag_malloc(sizeof(*lcs));
	lcs->vx = vx;
	lcs->vy = vy;
	lcs->eq = eq;
	return lcs;
}

void diag_lcs_destroy(struct diag_lcs *lcs)
{
	diag_free(lcs);
}

int diag_lcs_compute(struct diag_lcs *lcs, struct diag_vector **ses)
{
	struct diag_onp *onp;
	int *fp_base;
	intptr_t *path_base;
	size_t lx, ly;
	int i, k, m, n, mn3, d, p;
	struct diag_pair *s;

	onp = diag_malloc(sizeof(*onp));
	lx = diag_vector_length(lcs->vx);
	ly = diag_vector_length(lcs->vy);
	if (lx <= ly) {
		onp->m = m = (int)lx;
		onp->n = n = (int)ly;
		onp->a = lcs->vx;
		onp->b = lcs->vy;
		onp->u = 1;
	} else {
		onp->m = m = (int)ly;
		onp->n = n = (int)lx;
		onp->a = lcs->vy;
		onp->b = lcs->vx;
		onp->u = -1;
	}
	onp->eq = lcs->eq ? lcs->eq : eq_default;
	d = n - m;

	mn3 = m + n + 3;
	fp_base = diag_calloc(mn3, sizeof(*fp_base));
	path_base = diag_calloc(mn3, sizeof(*path_base));
	for (i = 0; i < mn3; i++) {
		fp_base[i] = -1;
		path_base[i] = (intptr_t)NULL;
	}
	onp->fp = fp_base + m + 1;
	onp->path = path_base + m + 1;
	onp->s = diag_set_create(NULL);

	p = 0;
	for (;;) {
		for (k = -p; k != d; k++) {
			snake(k, onp);
		}
		for (k = d + p; k != d; k--) {
			snake(k, onp);
		}
		snake(d, onp);
		if (fp_ref(d, onp) == n) break;
		p++;
	}

	s = (struct diag_pair *)path_ref(d, onp);
	s = diag_list_reverse(s);
	*ses = diag_list_to_vector((struct diag_pair *)s->cdr);
	diag_list_destroy(s);

	for (i = 0; i < (int)onp->s->size; i++) {
		diag_pair_destroy((struct diag_pair *)onp->s->arr[i]);
	}
	diag_set_destroy(onp->s);
	diag_free(path_base);
	diag_free(fp_base);
	diag_free(onp);
	return d + 2 * p;
}
