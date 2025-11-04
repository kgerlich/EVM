////////////////////////////////////////////////////////////////////////////////
// NAME: 			macros.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						macros for flag manipulation
////////////////////////////////////////////////////////////////////////////////
#define CARRY1    cpu.sregs.ccr=cpu.sregs.ccr|0x01
#define CARRY0    cpu.sregs.ccr=cpu.sregs.ccr&(~0x01)

#define ZERO1     cpu.sregs.ccr=cpu.sregs.ccr|0x04
#define ZERO0	  cpu.sregs.ccr=cpu.sregs.ccr&(~0x04)

#define OVER1     cpu.sregs.ccr=cpu.sregs.ccr|0x02
#define OVER0	  cpu.sregs.ccr=cpu.sregs.ccr&(~0x02)

#define XTEND1    cpu.sregs.ccr=cpu.sregs.ccr|0x10
#define XTEND0   cpu.sregs.ccr=cpu.sregs.ccr&(~0x10)

#define NEG1   	  cpu.sregs.ccr=cpu.sregs.ccr|0x08
#define NEG0     cpu.sregs.ccr=cpu.sregs.ccr&(~0x08)

