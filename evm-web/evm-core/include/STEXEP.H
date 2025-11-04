////////////////////////////////////////////////////////////////////////////////
// NAME: 			stexep.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						declarations for module stexep.c
////////////////////////////////////////////////////////////////////////////////

// bus error processing
extern void bus_err(void);
// address error processing
extern void addr_err(void);
// privilege violation processing
extern void priv_viol(void);
// divide by zero exception processing
extern void div_by_zero(void);
// tracing
extern void single_step(void);
// illegal opcode
extern void illegal(void);
// check for hardware interrupt
extern void CheckForInt(void);
// line A exep
extern void emulatelinea(void);
// line F exep
extern void emulatelinef(void);
// process unknown opcodes
extern void Unknown(short opcode);

