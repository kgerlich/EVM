////////////////////////////////////////////////////////////////////////////////
// NAME: 			stmem.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						high level memory access for 68K simulator for Windows
// MODIFIED: Updated to use new simulator module system instead of old DLL plugins
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include "simulator.h" // New simulator module interface
#include "STCOM.H"   // CPU core
#include "STEXEP.H"  // exception handling

// External simulator context (set by cpu_execute_opcode in cpu_core_new.c)
extern simulator_t *g_sim;

#define MAPPERSIZE (16*1024) // 16*1024 entries in table = 16MB � 1kB pages
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
char GETbyte(unsigned long address)
{
	address&=0x00FFFFFFL;
	if (g_sim == NULL) {
		bus_err();
		return 0L;
	}
	return (BYTE)simulator_read_memory(g_sim, address, 1);
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
short GETword(unsigned long address)
{
	address&=0x00FFFFFFL;
	if (g_sim == NULL) {
		bus_err();
		return 0L;
	}
	return (short)(unsigned long)simulator_read_memory(g_sim, address, 2);
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
long GETdword(unsigned long address)
{
	address&=0x00FFFFFFL;
	if (g_sim == NULL) {
		bus_err();
		return 0L;
	}
	return simulator_read_memory(g_sim, address, 4);
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
void PUTbyte(unsigned long address,char data)
{
	address&=0x00FFFFFFL;
	if (g_sim == NULL) {
		bus_err();
		return;
	}
	simulator_write_memory(g_sim, address, data, 1);
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
void PUTword(unsigned long address,short data)
{
	address&=0x00FFFFFFL;
	if (g_sim == NULL) {
		bus_err();
		return;
	}
	simulator_write_memory(g_sim, address, data, 2);
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
void PUTdword(unsigned long address,long data)
{
	address&=0x00FFFFFFL;
	if (g_sim == NULL) {
		bus_err();
		return;
	}
	simulator_write_memory(g_sim, address, data, 4);
}



