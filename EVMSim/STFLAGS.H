////////////////////////////////////////////////////////////////////////////////
// NAME: 			stflags.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						declarations for module stflags.c
////////////////////////////////////////////////////////////////////////////////
extern void _fastcall setcarry(char flag); // set carry to boolean flag
extern void _fastcall setzero(char flag);	// set zero to boolean flag
extern void _fastcall setover(char flag);  // set overflow to boolean flag
extern void _fastcall setxtend(char flag); // set extend to boolean flag
extern void _fastcall setneg(char flag);   // set negative to boolean flag
// generate carry on source,destination,result
extern char _fastcall gen_carry(long s,long d,long r);
// generate overflow on source,destination,result
extern char _fastcall gen_over(long s,long d,long r);
