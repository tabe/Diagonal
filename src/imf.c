#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "diagonal.h"
#include "diagonal/deque.h"
#include "diagonal/metric.h"
#include "diagonal/imf.h"

static diag_imf_header_field_t *
diag_imf_header_field_new(char *name, char *body)
{
	diag_imf_header_field_t *header_field;

	assert(name && body);
	header_field = (diag_imf_header_field_t *)diag_malloc(sizeof(diag_imf_header_field_t));
	header_field->name = name;
	header_field->body = body;
	return header_field;
}

static char *
read_header_field_name(char *s, char **name)
{
	char c, *r;

	assert(s);
	r = s;
	while ( (c = *s++) ) {
		if (c == ':') {
			*(s-1) = '\0';
			*name = r;
			return s;
		} else if (c == ' ') {
			diag_imf_raise_error(DIAG_IMF_ERROR_UNEXPECTED_WS);
			return NULL;
		} else if (isprint(c)) {
			continue;
		} else {
			diag_imf_raise_error(DIAG_IMF_ERROR_INVALID_HEADER_FIELD_NAME);
			return NULL;
		}
	}
	diag_imf_raise_error(DIAG_IMF_ERROR_UNEXPECTED_EOF);
	return NULL;
}

static int
read_header_field(char *s, diag_imf_header_field_t **header_field, char **r, unsigned int option)
{
	char c, *name, *t;

	assert(s);
	if (s[0] == '\r' && s[1] == '\n') {
		*r = s+2;
		return 0;
	}
	if (option & DIAG_IMF_LF) {
		if (*s == '\n') {
			*r = s+1;
			return 0;
		}
	}
	s = t = read_header_field_name(s, &name);
	if (!t) return -1;
	while ( (c = *t++) ) {
		if (c == '\r') {
			if (*t++ == '\n') {
				if (*t == ' ' || *t == '\t') continue; /* unfolding */
				*(t-2) = '\0';
				*header_field = diag_imf_header_field_new(name, (s[0] == ' ') ? s + 1 : s);
				*r = t;
				return 1;
			}
			diag_imf_raise_error(DIAG_IMF_ERROR_MISSING_LF);
			return -1;
		} else if (c == '\n') {
			if (option & DIAG_IMF_LF) {
				if (*t == ' ' || *t == '\t') continue; /* unfolding */
				*(t-1) = '\0';
				*header_field = diag_imf_header_field_new(name, (s[0] == ' ') ? s + 1 : s);
				*r = t;
				return 1;
			}
			diag_imf_raise_error(DIAG_IMF_ERROR_MISSING_CR);
			return -1;
		} else {
			continue;
		}
	}
	diag_imf_raise_error(DIAG_IMF_ERROR_UNEXPECTED_EOF);
	return -1;
}

int
diag_imf_parse(char *s, diag_imf_t **imfp, unsigned int option)
{
	diag_imf_t *imf;
	diag_imf_header_field_t *header_field, **header_fields;
	diag_deque_t *deque;
	diag_deque_elem_t *elem;
	char *r;
	int x;

	assert(s && imfp);
	deque = diag_deque_new();
	while ( (x = read_header_field(s, &header_field, &r, option)) > 0) {
		diag_deque_push(deque, (void *)header_field);
		s = r;
	}
	if (x < 0) {
		diag_deque_destroy(deque);
		return x;
	}
	imf = (diag_imf_t *)diag_malloc(sizeof(diag_imf_t));
	header_fields = (diag_imf_header_field_t **)diag_calloc((size_t)deque->length + 1, sizeof(diag_imf_header_field_t *));
	imf->header_fields = header_fields;
	DIAG_DEQUE_FOR_EACH(deque, elem) {
		*header_fields++ = (diag_imf_header_field_t *)elem->attr;
	}
	diag_deque_destroy(deque);
	imf->body = r;
	*imfp = imf;
	return 0;
}

void
diag_imf_raise_error(enum diag_imf_error e)
{
	syslog(LOG_INFO, "imf parse error: %d", e);
}

void
diag_imf_destroy(diag_imf_t *imf)
{
	if (imf) {
		diag_imf_header_field_t **header_fields = imf->header_fields;
		diag_imf_header_field_t *header_field;

		while ( (header_field = *header_fields++) ) {
			diag_free(header_field);
		}
		diag_free(imf->header_fields);
		diag_free(imf);
	}
}

diag_distance_t
diag_hamming_imf(const diag_imf_t *x, const diag_imf_t *y)
{
	diag_distance_t d = 0;
	unsigned int i = 0;

	assert(x && y);
	if (x == y) return 0;
	for (;;) {
		if (!x->header_fields[i]) {
			while (y->header_fields[i]) {
				d += strlen(y->header_fields[i]->body);
				i++;
			}
			break;
		}
		if (!y->header_fields[i]) {
			while (x->header_fields[i]) {
				d += strlen(x->header_fields[i]->body);
				i++;
			}
			break;
		}
		if (strcmp(x->header_fields[i]->name, y->header_fields[i]->name) == 0) {
			d += diag_hamming_chars(x->header_fields[i]->body, y->header_fields[i]->body);
		} else {
			d += strlen(x->header_fields[i]->body);
			d += strlen(y->header_fields[i]->body);
		}
		i++;
	}
	return d;
}
