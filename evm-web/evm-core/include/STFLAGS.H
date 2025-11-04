////////////////////////////////////////////////////////////////////////////////
// NAME: 			stflags.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						declarations for module stflags.c
////////////////////////////////////////////////////////////////////////////////
#include "STSTDDEF.H"

/* Flag manipulation functions - implemented inline below */
static inline void setcarry(char flag) {
    extern CPU cpu;
    if (flag) cpu.sregs.sr |= 0x01;
    else cpu.sregs.sr &= ~0x01;
}

static inline void setzero(char flag) {
    extern CPU cpu;
    if (flag) cpu.sregs.sr |= 0x04;
    else cpu.sregs.sr &= ~0x04;
}

static inline void setover(char flag) {
    extern CPU cpu;
    if (flag) cpu.sregs.sr |= 0x02;
    else cpu.sregs.sr &= ~0x02;
}

static inline void setxtend(char flag) {
    extern CPU cpu;
    if (flag) cpu.sregs.sr |= 0x10;
    else cpu.sregs.sr &= ~0x10;
}

static inline void setneg(char flag) {
    extern CPU cpu;
    if (flag) cpu.sregs.sr |= 0x08;
    else cpu.sregs.sr &= ~0x08;
}

/* Generate carry flag from operation */
static inline char gen_carry(long s, long d, long r) {
    return (r < 0 && d >= 0 && s >= 0) || (r >= 0 && d < 0 && s < 0) ? 1 : 0;
}

/* Generate overflow flag from operation */
static inline char gen_over(long s, long d, long r) {
    return (r < 0 && d >= 0 && s >= 0) || (r >= 0 && d < 0 && s < 0) ? 1 : 0;
}

/* Macro wrappers for flag setting (for compatibility with original code) */
#define NEG0        setneg(0)
#define NEG1        setneg(1)
#define ZERO0       setzero(0)
#define ZERO1       setzero(1)
#define CARRY0      setcarry(0)
#define CARRY1      setcarry(1)
#define OVER0       setover(0)
#define OVER1       setover(1)
#define XTEND0      setxtend(0)
#define XTEND1      setxtend(1)
