////////////////////////////////////////////////////////////////////////////////
// NAME: 			s19.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				04/11/1997
// DESCRIPTION:
//						load a S19 file
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "..\evmsim\stdll.h"
#include "evmrom.h"


// S19 record structure
typedef struct
{
	char mark;
   short type;
   short size;
   long address;
   short data;
   char name[64];
}S19;

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
// NAME:				void PUTdirectdword(unsigned long address,long data)
//
// DESCRIPTION:	write a byte into memory
//
// PARAMETERS:    unsigned long address: logical address to write to
//						long data: data to write
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void PUTdirectdword(unsigned long address,long data)
{
	*((long*)(pgRom+address))=data;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				BOOL Load_S19(char *Filename)
//
// DESCRIPTION:   load an S19 file
//
// PARAMETERS:    char* Filename: file to load
//
// RETURNS:       BOOL: SUCCESS/FAILURE = TRUE/SUCCESS
//
////////////////////////////////////////////////////////////////////////////////
BOOL Load_S19(char *Filename)
{
FILE* in;
char temp[256];
S19 s19;
short i;

	if((in=fopen(Filename,"r"))==NULL)return FALSE;
	while(fgets(temp,256,in))
	{
		sscanf(temp,"%c%1X%02X",&(s19.mark),&(s19.type),&(s19.size));
		if(s19.mark=='S') // begins with S mark
		{
			switch(s19.type)
			{
				case 0:  // S0
					for(i=8;i<8+s19.size;i+=2)
					{
						sscanf(&temp[i],"%02x",&(s19.data));
						s19.name[(i-8)>>1]=s19.data;
					}
					s19.name[(i-8)>>1]='\0';
					break;
				case 1:  // S1
					sscanf(&temp[4],"%04lx",&(s19.address));
					for(i=8;i<8+((s19.size-1)<<1);i+=2)
					{
						sscanf(&temp[i],"%02x",&(s19.data));
						PUTdirectbyte(s19.address+((i-8)>>1),s19.data);
					}
					break;
				case 2:  // S2
					sscanf(&temp[4],"%06lx",&(s19.address));
					for(i=10;i<10+((s19.size-2)<<1);i+=2)
					{
						sscanf(&temp[i],"%02x",&(s19.data));
						PUTdirectbyte(s19.address+((i-10)>>1),s19.data);
					}
					break;
				case 3:  // S3
					sscanf(&temp[4],"%08lx",&(s19.address));
					for(i=12;i<12+((s19.size-4)<<1);i+=2)
					{
						sscanf(&temp[i],"%02x",&(s19.data));
						PUTdirectbyte(s19.address+((i-12)>>1),s19.data);
					}
					break;
				}
			}
			else
			{
				fclose(in);
				return FALSE;
			}
		}
		fclose(in);
		return TRUE;
}
