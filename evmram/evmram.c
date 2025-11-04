////////////////////////////////////////////////////////////////////////////////
// NAME: 			evmram.c (DLL)
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1996
// DESCRIPTION:
//						simulate the EVM ram arena
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include "windows-compat.h"
#include <stdio.h>
#include <string.h>
#include "..\evmsim\stdll.h"

////////////////////////////////////////////////////////////////////////////////
// declare _DLLHDR header
// CPU address space:	0x400000-0x41FFFF
// PRIORITY 		  :	HIGH
// CACHEABLE		  :	can be cached
////////////////////////////////////////////////////////////////////////////////
MAKE_HDR(0x00400000L,0x00020000L,SIM_HIGHEST_PRIORITY,TRUE);

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HGLOBAL hgRam=0;	// handle to simulation RAM
BYTE *pgRam=0;    // pointer to locked simulation RAM

// union for endian conversion (MC680x0->INTEL80x86)
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
	address&=(~DLLHDR.nAddress); // mask out irrelevant bits of address
	if(size==0)
	{
		return *(pgRam+address);
	}
	else if(size==1)
	{
		ULONG result;
		_asm
		{
			mov ebx,pgRam
			add ebx,address
			movzx eax,WORD PTR [ebx]
			bswap eax
			shr eax,16
			mov [result],eax
		}
		return result;
	}
	else
	{
		ULONG result;
		_asm
		{
			mov ebx,pgRam
			add ebx,address
			mov eax,[ebx]
			bswap eax
			mov [result],eax
		}
		return result;
	}
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
	address&=(~DLLHDR.nAddress);
	switch(size)
	{
		case 0: // BYTE
			*(pgRam+address)=(BYTE)data;
			break;
      case 1: // WORD
			endian1.w.l=(short)data;   // do endian conversion
			endian2.b.ll=endian1.b.lh;
			endian2.b.lh=endian1.b.ll;
			*(short*)(pgRam+address)=endian2.w.l;
			break;
		case 2: // DWORD
			endian1.l=data;            // do endian conversion
			endian2.b.ll=endian1.b.uh;
			endian2.b.lh=endian1.b.ul;
			endian2.b.ul=endian1.b.lh;
			endian2.b.uh=endian1.b.ll;
			*(long*)(pgRam+address)=endian2.l;
      	break;
   }
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
	// allocate global simulation memory
	if((hgRam=GlobalAlloc(GHND,DLLHDR.nSize))==0) // on error return FAILURE
	{
		return FALSE; // return failure
	}
	// lock it down in linear memory
	if((pgRam=(BYTE *)GlobalLock(hgRam))==NULL) // on error return FAILURE
	{
		return FALSE; // return failure
	}
	return TRUE;
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
	// had we allocated simulation memory?
	if(hgRam)
	{
		GlobalUnlock(hgRam); // unlock the mem
		GlobalFree(hgRam);   // free the mem
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
	// does nothing here
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
	// does nothing here
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
	// does nothing here
}
