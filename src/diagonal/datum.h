/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef DIAGONAL_DATUM_H
#define DIAGONAL_DATUM_H

DIAG_C_DECL_BEGIN

enum {
	DIAG_TAG_CHARS = 1,
	DIAG_TAG_IMF,

	DIAG_TAG_TBFRE = 0x01<<8, /* "to be freed" */
	DIAG_TAG_SIZE  = 0x02<<8,
	DIAG_TAG_IDNUM = 0x01<<16,
	DIAG_TAG_IDNAM = 0x02<<16,
};

struct diag_datum {
	uint64_t tag;
	void *value;
	union {
		uint32_t number;
		char *name;
	} id;
};

#define DIAG_DATUM_IMMEDIATE_P(datum) (!(uint32_t)((datum)->tag))
#define DIAG_DATUM_CHARS_P(datum) ((datum)->tag & DIAG_TAG_CHARS)
#define DIAG_DATUM_TBFRE_P(datum) ((datum)->tag & DIAG_TAG_TBFRE)
#define DIAG_DATUM_SIZE_P(datum) ((datum)->tag & DIAG_TAG_SIZE)
#define DIAG_DATUM_ASCIZ_P(datum) \
	(DIAG_DATUM_CHARS_P(datum) && !DIAG_DATUM_SIZE_P(datum))
#define DIAG_DATUM_SIZE(datum) ((uint32_t)((datum)->tag>>(sizeof(uint32_t)*8)))
#define DIAG_DATUM_SET_IMMEDIATE(datum, x) ((datum)->tag = (uint64_t)(x)<<(sizeof(uint32_t)*8))
#define DIAG_DATUM_GET_IMMEDIATE(datum) DIAG_DATUM_SIZE(datum)

DIAG_FUNCTION struct diag_datum *diag_datum_new(void *value);
DIAG_FUNCTION void diag_datum_destroy(struct diag_datum *datum);

DIAG_C_DECL_END

#endif
