////////////////////////////////////////////////////////////////////////////////
// NAME: 			stflags.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						68K flag manipulations
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "stcom.h"

////////////////////////////////////////////////////////////////////////////////
// NAME:				void setcarry(char flag)
//
// DESCRIPTION:   set/reset carry flag
//
// PARAMETERS:    char flag: 0 reset/ 1 set flag
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall setcarry(char flag)
{
	if(flag)cpu.sregs.ccr=cpu.sregs.ccr|0x01;
	else cpu.sregs.ccr=cpu.sregs.ccr&(~0x01);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void setzero(char flag)
//
// DESCRIPTION:   set/reset zero flag
//
// PARAMETERS:    char flag: 0 reset/ 1 set flag
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall setzero(char flag)
{
	if(flag)cpu.sregs.ccr=cpu.sregs.ccr|0x04;
	else cpu.sregs.ccr=cpu.sregs.ccr&(~0x04);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void setover(char flag)
//
// DESCRIPTION:   set/reset overflow flag
//
// PARAMETERS:    char flag: 0 reset/ 1 set flag
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall setover(char flag)
{
	if(flag)cpu.sregs.ccr=cpu.sregs.ccr|0x02;
	else cpu.sregs.ccr=cpu.sregs.ccr&(~0x02);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void setxtend(char flag)
//
// DESCRIPTION:   set/reset extend flag
//
// PARAMETERS:    char flag: 0 reset/ 1 set flag
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall setxtend(char flag)
{
	if(flag)cpu.sregs.ccr=cpu.sregs.ccr|0x10;
	else cpu.sregs.ccr=cpu.sregs.ccr&(~0x10);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void setneg(char flag)
//
// DESCRIPTION:   set/reset negative flag
//
// PARAMETERS:    char flag: 0 reset/ 1 set flag
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall setneg(char flag)
{
	if(flag)cpu.sregs.ccr=cpu.sregs.ccr|0x08;
	else cpu.sregs.ccr=cpu.sregs.ccr&(~0x08);
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				char gen_carry(long s,long d,long r)
//
// DESCRIPTION:   generate carry from source,destination and result
//
// PARAMETERS:    long s: source value
//						long d: destination value
//						long r: result value from source and destination
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
char _fastcall gen_carry(long s,long d,long r)
{
	if( (s<0 && !(d<0)) || (r<0 && !(d<0)) || (s<0 && r<0) )return 1;
	else return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				char gen_over(long s,long d,long r)
//
// DESCRIPTION:   generate overflow from source,destination and result
//
// PARAMETERS:    long s: source value
//						long d: destination value
//						long r: result value from source and destination
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
char _fastcall gen_over(long s,long d,long r)
{
	if( (!(s<0) && (d<0) && !(r<0)) || ((s<0) && !(d<0) && (r<0)) )return 1;
	else return 0;
}
