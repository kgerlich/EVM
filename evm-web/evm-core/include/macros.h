////////////////////////////////////////////////////////////////////////////////
// NAME: 			macros.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						macros for flag manipulation
// NOTE: CCR is lower 8 bits of SR in MC68020
////////////////////////////////////////////////////////////////////////////////
#define CARRY1    cpu.sregs.sr=cpu.sregs.sr|0x01
#define CARRY0    cpu.sregs.sr=cpu.sregs.sr&(~0x01)

#define ZERO1     cpu.sregs.sr=cpu.sregs.sr|0x04
#define ZERO0	  cpu.sregs.sr=cpu.sregs.sr&(~0x04)

#define OVER1     cpu.sregs.sr=cpu.sregs.sr|0x02
#define OVER0	  cpu.sregs.sr=cpu.sregs.sr&(~0x02)

#define XTEND1    cpu.sregs.sr=cpu.sregs.sr|0x10
#define XTEND0   cpu.sregs.sr=cpu.sregs.sr&(~0x10)

#define NEG1   	  cpu.sregs.sr=cpu.sregs.sr|0x08
#define NEG0     cpu.sregs.sr=cpu.sregs.sr&(~0x08)

