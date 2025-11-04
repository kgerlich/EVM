////////////////////////////////////////////////////////////////////////////////
// NAME: 			stmem.h
// COMPILER:		Borland C++ 5.0 
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						declarations for module stmem.c
////////////////////////////////////////////////////////////////////////////////

// memory setup
void SetupMapperArrays(void);
int CheckPeriAddr(unsigned long address);

// memory read routines

// read BYTE (8bit) from address
extern char _fastcall GETbyte(unsigned long address);
// read WORD (16bit) from address
extern short _fastcall GETword(unsigned long address);
// read DWORD (32bit) from address
extern long _fastcall GETdword(unsigned long address);

// memory write routines

// write BYTE (8bit) data to address
extern void _fastcall PUTbyte(unsigned long address,char data);
// write WORD (16bit)data to address
extern void _fastcall PUTword(unsigned long address,short data);
// write DWORD (32bit)data to address
extern void _fastcall PUTdword(unsigned long address,long data);


