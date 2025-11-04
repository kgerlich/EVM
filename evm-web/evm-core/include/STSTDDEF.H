////////////////////////////////////////////////////////////////////////////////
// NAME: 			ststddef.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:	Header file
//						standard defines for 68K simulator for Windows
////////////////////////////////////////////////////////////////////////////////

#ifndef STSTDDEF_H_
#define STSTDDEF_H_

#include <stdint.h>
#include <stdbool.h>

/* Type definitions for cross-platform compatibility */
typedef unsigned char BYTE;
typedef unsigned char UCHAR;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef long LONG;
typedef int BOOL;
typedef char CHAR;
typedef short SHORT;
typedef int INT;

#define TRUE 1
#define FALSE 0
/* NULL is defined by system headers - do not redefine */
/* #define NULL 0 */

/* CPU state structure */
typedef struct {
    struct { LONG a[8]; } aregs;       /* address registers */
    struct { LONG d[8]; } dregs;       /* data registers */
    LONG pc;                            /* program counter */
    struct { USHORT sr; } sregs;       /* status register */
    LONG usp, ssp, msp;                 /* stack pointers */

    /* Control registers */
    UCHAR sfc;                          /* Source Function Code */
    UCHAR dfc;                          /* Destination Function Code */
    LONG cacr;                          /* Cache Control Register */
    LONG vbr;                           /* Vector Base Register */
} CPU;

/* Work structure for instruction calculations */
struct tag_work {
    LONG source;
    LONG destination;
    LONG result;
};

// debugging (no longer valid in WASM - commented out)
// #define DEBUGGER asm int 3;

/* data size defines (for addressing modes, not types) */
#define SIZE_BYTE  0	/* size is byte */
#define SIZE_WORD  1	/* size is word */
#define SIZE_DWORD 2	/* size is double word */

/* data access mode defines */
#define READ  0 /* command is read */
#define WRITE 1 /* command is write */

/* bit field defines */
#define ONE_TO_ZERO 	(0x0003) /* opcode field bits 1-0 */
#define TWO_TO_ZERO 	(0x0007) /* opcode field bits 2-0 */
#define FOUR_TO_THREE 	(0x0018) /* opcode field bits 4-3 */
#define EIGHT_TO_SIX 	(0x01C0) /* opcode field bits 8-6 */
#define EIGHT_TO_FOUR 	(0x01F0) /* opcode field bits 8-4 */
#define EIGHT_TO_THREE 	(0x01F8) /* opcode field bits 8-3 */

/* status bit defines */
#define CARRY 		(0x01)
#define OVERFLOW 	(0x02)
#define ZERO 		(0x04)
#define NEGATIVE 	(0x08)
#define EXTEND 		(0x10)

/* instruction caching macro (optimization hint - currently a no-op) */
#define CACHEFUNCTION(p)

#endif /* STSTDDEF_H_ */
