////////////////////////////////////////////////////////////////////////////////
// NAME: 			ststddef.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:	Header file
//						standard defines for 68K simulator for Windows
////////////////////////////////////////////////////////////////////////////////

// debugging
#define DEBUGGER asm int 3;   // assembler inline to cause a debugger trap

/* data size defines */
#define BYTE  0	/* size is byte */
#define WORD  1	/* size is word */
#define DWORD 2	/* size is double word */

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
