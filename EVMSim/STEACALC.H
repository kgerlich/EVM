////////////////////////////////////////////////////////////////////////////////
// NAME: 			steacalc.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						declarations for module steacalc.c
////////////////////////////////////////////////////////////////////////////////

// addressing modes
extern long DRD(char,char,long,char); // data register direct
extern long ARD(char,char,long,char); // address register direct
extern long ARI(char,char,long,char); // address register indirect
extern long ARIPI(char,char,long,char);  // address register indirect post increment
extern long ARIPD(char,char,long,char);  // address register indirect pre decrement
extern long ARID(char,char,long,char);  // address register indirect with displacement
extern long ARII(char,char,long,char);  // address register indirect indexed
extern long MISC(char,char,long,char);  // miscellaneous addressing modes (immediate etc.)


