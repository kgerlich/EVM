////////////////////////////////////////////////////////////////////////////////
// NAME: 			binary.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				04/11/1997
// DESCRIPTION:
//						load a binary file
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include "windows-compat.h"
#include <string.h>
#include <stdio.h>
#include "..\evmsim\stdll.h"
#include "evmrom.h"

////////////////////////////////////////////////////////////////////////////////
// NAME:				void PUTdirectbyte(unsigned long address,char data)
//
// DESCRIPTION:	write a byte into memory
//
// PARAMETERS:    unsigned long address: logical address to write to
//						char data: data to write
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void PUTdirectbyte(unsigned long address,char data)
{
	// check for address overrun, then write data to address
	if(address<(DLLHDR.nAddress+DLLHDR.nSize))*(pgRom+address)=data;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				BOOL Load_Bin(char* szFilename)
//
// DESCRIPTION:   load binary file
//
// PARAMETERS:    char* szFilename: which file to load
//
// RETURNS:			BOOL: SUCCESS/FAILURE = TRUE/FALSE
//
////////////////////////////////////////////////////////////////////////////////
BOOL Load_Bin(char* szFilename)
{
FILE* in;
unsigned long addr=0;
char temp;

	if((in=fopen(szFilename,"r+b"))==NULL)return FALSE;

	while(fread(&temp, 1 ,1, in))
	{
		PUTdirectbyte(addr,temp);
		addr++;
	}
	fclose(in);

	return TRUE;
}