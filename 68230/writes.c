////////////////////////////////////////////////////////////////////////////////
// NAME: 			writes.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				5/1997
// DESCRIPTION:   write accesses to PI/T
////////////////////////////////////////////////////////////////////////////////
#define STRICT  // strict type checking
#define WIN32_LEAN_AND_MEAN  // only basic windows stuff
#include "windows-compat.h" // portable Windows API compatibility
#include <stdio.h>  // standard IO
#include "..\evmsim\stdll.h"  // DLLHDR
#include "68230.h"

////////////////////////////////////////////////////////////////////////////////
// NAME:				void WritePADR(BYTE data)
//
// DESCRIPTION:   write to PADR according to mode
//
// PARAMETERS:		BYTE data: data to write
//
// RETURNS:			none
//
// REMARKS:			only MODE 0 SUBMODE 1X implemented
//
////////////////////////////////////////////////////////////////////////////////
void WritePADR(BYTE data)
{
	switch((PGCR>>6)&0x03) // which port mode is selected
	{
		case 0: // mode 0: unidirectional 8-bit
			switch((PACR>>6)&0x03) // which port A submode is selected
			{
				case 2: // submode 1X
				case 3:
					PADR=(BYTE)data&(PADDR);
					break;
			}
			break;
		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void WritePBDR(BYTE data)
//
// DESCRIPTION:   write to PBDR according to mode
//
// PARAMETERS:		BYTE data: data to write
//
// RETURNS:			none
//
// REMARKS:			only MODE 0 SUBMODE 1X implemented
//
////////////////////////////////////////////////////////////////////////////////
void WritePBDR(BYTE data)
{
	switch((PGCR>>6)&0x03) // which port mode is selected
	{
		case 0: // mode 0: unidirectional 8-bit
			switch((PBCR>>6)&0x03) // which port B submode is selected
			{
				case 2:
				case 3:
					PBDR=(BYTE)data&(PBDDR);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void WritePCDR(BYTE data)
//
// DESCRIPTION:   write to PCDR according to mode
//
// PARAMETERS:		BYTE data: data to write
//
// RETURNS:			none
//
////////////////////////////////////////////////////////////////////////////////
void WritePCDR(BYTE data)
{
	PCDR=(BYTE)data&(PCDDR);
}
