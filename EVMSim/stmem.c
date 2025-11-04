////////////////////////////////////////////////////////////////////////////////
// NAME: 			stmem.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						high level memory access for 68K simulator for Windows
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN //schlank und rank HAHA!!
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "stmain.h"  // main program
#include "stcom.h"   // CPU core
#include "stexep.h"  // exception handling

#define MAPPERSIZE (16*1024) // 16*1024 entries in table = 16MB à 1kB pages
////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
// function pointer array to READ() procs of plugins
unsigned long (*fpReadProc[MAPPERSIZE])(unsigned long address,short size);
// function pointer array to WRITE() procs of plugins
void (*fpWriteProc[MAPPERSIZE])(unsigned long address,long data,short size);

////////////////////////////////////////////////////////////////////////////////
// NAME:				int CheckPeriAddr(unsigned long address)
//
// DESCRIPTION:	check to which attached module address belongs
//
// PARAMETERS:    unsigned long address: address to check
//
// RETURNS:       int: index into array fpReadProc[] and fpWriteProc[]
//
////////////////////////////////////////////////////////////////////////////////
int CheckPeriAddr(unsigned long address)
{
int index;

	for(index=0;pDllHdr[index]!=0;index++)
	{
		if(address>=pDllHdr[index]->nAddress &&
			 address<(pDllHdr[index]->nAddress+pDllHdr[index]->nSize))return index;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void SetupMapperArrays(void)
//
// DESCRIPTION:	create two arrays containing READ and WRITE procedure addresses
// 					for fast access to the attached module procedures
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void SetupMapperArrays(void)
{
short i,index;

	for(i=0;i<(MAPPERSIZE);i++) // for each entry
	{
		// any module for that address?
		if((index=CheckPeriAddr(i*(MAPPERSIZE/16)))!=-1)
		{
			// fill in function pointer arrays
			fpReadProc[i]=(void*)pDllHdr[index]->ReadProc;
			fpWriteProc[i]=(void*)pDllHdr[index]->WriteProc;
		}
		// no module there
		else
		{
			// no address
			fpReadProc[i]=NULL;
			fpWriteProc[i]=NULL;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:          char   GETbyte(unsigned long address)
//
// DESCRIPTION:	return a BYTE (8bit) from memory
//
// PARAMETERS:    unsigned long address: address of data in memory
//
// RETURNS:       char: data from memory
//
////////////////////////////////////////////////////////////////////////////////
char _fastcall GETbyte(unsigned long address)
{
	address&=0x00FFFFFFL;
	// did we hit any peripheral device?
	if(fpReadProc[address>>10]) // (address)/1024=entry in table
	{
		return (BYTE)fpReadProc[address>>10](address,0);
	}
	else bus_err(); // since we would not receive an acknowledge in real life
						 // we do a BUS ERROR
	return 0L;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:          char   GETword(unsigned long address)
//
// DESCRIPTION:	return a WORD (16bit) from memory
//
// PARAMETERS:    unsigned long address: address of data in memory
//
// RETURNS:       short: data from memory
//
////////////////////////////////////////////////////////////////////////////////
short _fastcall GETword(unsigned long address)
{
	address&=0x00FFFFFFL;
	if(fpReadProc[address>>10]) // (address)/1024=entry in table
	{
		return (short)(unsigned long)fpReadProc[address>>10](address,1);
	}
	else bus_err();  // since we would not receive an acknowledge in real life
						  // we do a BUS ERROR
	return 0L;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:          long   GETdword(unsigned long address)
//
// DESCRIPTION:	return a DWORD (32bit) from memory
//
// PARAMETERS:    unsigned long address: address of data in memory
//
// RETURNS:       long: data from memory
//
////////////////////////////////////////////////////////////////////////////////
long _fastcall GETdword(unsigned long address)
{
	address&=0x00FFFFFFL;
	if(fpReadProc[address>>10]) // (address)/1024=entry in table
	{
		return fpReadProc[address>>10](address,2);
	}
	else bus_err();  // same old game -> BUS ERROR
	return 0L;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				void PUTbyte(unsigned long address,char data)
//
// DESCRIPTION:   write a BYTE (8bit) to memory
//
// PARAMETERS:    unsigned long address: address of data in memory
//						char data:				  data to write
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall PUTbyte(unsigned long address,char data)
{
	address&=0x00FFFFFFL;
	if(fpWriteProc[address>>10]) // (address)/1024=entry in table
	{
		fpWriteProc[address>>10](address,data,0);
	}
	else bus_err(); // stepped into memory hole?
						 // then there's just one goal -> BUS ERROR
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void PUTword(unsigned long address,short data)
//
// DESCRIPTION:   write a BYTE (8bit) to memory
//
// PARAMETERS:    unsigned long address: address of data in memory
//						short data:				  data to write
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall PUTword(unsigned long address,short data)
{
	address&=0x00FFFFFFL;
	if(fpWriteProc[address>>10]) // (address)/1024=entry in table
	{
		fpWriteProc[address>>10](address,data,1);
	}
	else bus_err(); // sorry, mister!!
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void PUTword(unsigned long address,short data)
//
// DESCRIPTION:   write a DWORD (32bit) to memory
//
// PARAMETERS:    unsigned long address: address of data in memory
//						long data:				  data to write
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void _fastcall PUTdword(unsigned long address,long data)
{
	address&=0x00FFFFFFL;
	if(fpWriteProc[address>>10]) // (address)/1024=entry in table
	{
		fpWriteProc[address>>10](address,data,2);
	}
	else bus_err(); // all the same, end of game
}



