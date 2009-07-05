#ifndef DIAGONAL_OBJECT_H
#define DIAGONAL_OBJECT_H

typedef intptr_t diag_object_t;

#define DIAG_OBJECT(obj) ((diag_object_t)(obj))

/*
 * location:
 *  pppppppp pppppppp pppppppp pppppp00
 *
 * immediate:
 *  nnnnnnnn nnnnnnnn nnnnnnnn nnnnnnn1 [fixnum]
 *  cccccccc cccccccc cccccccc ccccc010 [character]
 *  00000000 00000000 00000000 00000110 [boolean false]
 *  00000000 00000000 00000000 00001110 [boolean true]
 *  00000000 00000000 00000000 00010110 [empty list]
 *  00000000 00000000 00000000 00011110 [eof]
 */

#define DIAG_LOCATION_P(obj) ((DIAG_OBJECT(obj) & 3) == 0)

#define DIAG_FIXNUM_P(obj) ((DIAG_OBJECT(obj) & 1) == 1)

#define DIAG_IMMEDIATE_P(obj) ((DIAG_OBJECT(obj) & 7) == 6)

#define DIAG_CHARACTER_P(obj) ((DIAG_OBJECT(obj) & 7) == 2)

#define DIAG_MAKE_IMMEDIATE(x) DIAG_OBJECT(6|(x<<3))

#define DIAG_FALSE DIAG_MAKE_IMMEDIATE(0)
#define DIAG_TRUE  DIAG_MAKE_IMMEDIATE(1)
#define DIAG_NIL   DIAG_MAKE_IMMEDIATE(2)
#define DIAG_EOF   DIAG_MAKE_IMMEDIATE(3)

/*
 * tag:
 *  pppppppp pppppppp pppppppp pppppp00 [pair]
 *  pppppppp pppppppp pppppppp pppppp01 [symbol]
 *  pppppppp pppppppp pppppppp pppppp10 [string]
 *  ssssssss ssssssss ssssssss ssssi011 [vector] i: immediate
 *  -------- -------- -------- ----0111 [procedure]
 *  -------- -------- -------- ---01111 [hashtable]
 */

#define DIAG_OBJECT_STUB uintptr_t tag

#endif /* DIAGONAL_OBJECT_H */
