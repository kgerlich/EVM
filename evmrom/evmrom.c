////////////////////////////////////////////////////////////////////////////////
// NAME: 			evmrom.c (DLL)
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						simulate the EVM rom arena
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "..\evmsim\stdll.h"
#include "s19.h"
#include "binary.h"

////////////////////////////////////////////////////////////////////////////////
// declare _DLLHDR header
// CPU address space:	0x000000-0x00FFFF
// PRIORITY 		  :	HIGH
// CACHEABLE		  :	can be cached
////////////////////////////////////////////////////////////////////////////////
MAKE_HDR(0x00000000L,0x00010000L,SIM_HIGHEST_PRIORITY,TRUE);


////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HGLOBAL hgRom=0; // handle to ROM memory
BYTE *pgRom=0;	// pointer to ROM memory
char szSystemFilename[64]; // system file name
									//	from EVMROM.INI (normally PS20.S19)

// union for endian conversion (MC680x0 -> i80x86)
typedef union
{
	long l;
	struct
	{
		short l,h;
	}w;
	struct
	{
		char ll,lh,ul,uh;
	}b;
}endian;

endian endian1,endian2;

////////////////////////////////////////////////////////////////////////////////
// NAME:				unsigned long Read(long address,short size)
//
// DESCRIPTION:   (required function)
//                return value from peripheral memory
//
// PARAMETERS:    long address:	location of data
//                short size:    size of access (BYTE,WORD,DWORD)
//
// RETURNS:       unsigned long: return data
//
////////////////////////////////////////////////////////////////////////////////

unsigned long Read(long address,short size)
{
	address&=(~DLLHDR.nAddress); // mask out irrelevant address bits
	switch(size) // access size?
	{
		case 0: // BYTE
			return (char)(*(pgRom+address));
		case 1: // WORD
			endian1.w.l=*(short*)(pgRom+address); // endian conversion
			endian2.b.ll=endian1.b.lh;
			endian2.b.lh=endian1.b.ll;
			return (short)endian2.w.l;
		case 2: // DWORD
			endian1.l=*(long*)(pgRom+address); // endian conversion
			endian2.b.ll=endian1.b.uh;
			endian2.b.lh=endian1.b.ul;
			endian2.b.ul=endian1.b.lh;
			endian2.b.uh=endian1.b.ll;
			return endian2.l;
	}
	return 0L;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void Write(long address,long data,short size)
//
// DESCRIPTION:   (required function)
//                write to peripheral memory
//
// PARAMETERS:    long address:	location of data
//						long data:     data to write
//                short size:    size of access (BYTE,WORD,DWORD)
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////

void Write(long address,long data,short size)
{
	// no function, since ROM write access doesn't do anything
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				BOOL Setup(void)
//
// DESCRIPTION:   (required function)
//						setup module
//
// PARAMETERS:    none
//
// RETURNS:       BOOL: success/failure = TRUE/FALSE
//
////////////////////////////////////////////////////////////////////////////////

BOOL Setup(void)
{
int i;

	// allocate global simulation memory
	// on error display message box and exit
	if((hgRom=GlobalAlloc(GHND,DLLHDR.nSize))==0)
	{
		return FALSE; // return failure
	}
	// on error display message box and exit
	if((pgRom=(BYTE *)GlobalLock(hgRom))==NULL)
	{
		return FALSE; // return failure
	}
	// get system filename from INI file
	GetPrivateProfileString("EVMROM","SYSTEM","",
						szSystemFilename,sizeof(szSystemFilename),".\\evmrom.ini");
	// isolate extension
	for(i=0;szSystemFilename[i]!=0 && szSystemFilename[i]!='.';i++);
	if(strcmp(&szSystemFilename[i+1],"s19")==0) // it's an S19 record file
	{
		return Load_S19(szSystemFilename); // load it
	}
/*	else if(strcmp(&szSystemFilename[i+1],"bin")==0) // it's a binary file
	{
		return Load_Bin(szSystemFilename); // load it
	}*/

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void Exit(void)
//
// DESCRIPTION:   (required function)
//						shutdown module
//						called at global simulation exit only
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void Exit(void)
{
	// have we allocated simulation memory
	if(hgRom)
	{
		GlobalUnlock(hgRom); // unlock the mem
		GlobalFree(hgRom);   // free the mem
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void Reset(void)
//
// DESCRIPTION:   (required function)
//						reset module
//						called at startup and from RESET opcode
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////

void Reset(void)
{
	// does nothing
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void Simulate(void)
//
// DESCRIPTION:   (required function)
//						synchronized module action
//						called AFTER every executed opcode
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void Simulate(void)
{
	// does nothing
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void Init(void)
//
// DESCRIPTION:   (required function)
//						initialize module
//						called at startup after call to Setup() when all plugin
//						module have already been loaded
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void Init(void)
{
	// does nothing
}

