////////////////////////////////////////////////////////////////////////////////
// NAME: 			updown.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						simulate a data transfer host
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "68681.h"
#include "file.h"
#include "printer.h"

// submodes
#define MESSAGE 0
#define NAME 1
#define RESPONDNAME 2
#define LOADSTARTS 3
#define ACKNOWLEDGE 4
#define DATA 5
#define ENDE 6
// main modes
#define NORMAL 0
#define UPLOAD 1
#define DOWNLOAD 2
#define PRINTER  3

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
short nUpdownMode=NORMAL,nSubMode=NAME;
long nCount=0;
char MsgLoad[256]="File to load: ",
	  MsgSave[256]="File to save: ",
	  MsgLoadStarts[256]="     loading... ",
	  MsgSaveStarts[256]="     saving... ",
	  szFilename[256];
HFILE hFile;
BOOL bLoadFinished;
HGLOBAL hPrintBuf;
char *pPrintBuf;

////////////////////////////////////////////////////////////////////////////////
// NAME:				void ResetUpdown(void)
//
// DESCRIPTION:   reset up/download simulation
//
// PARAMETERS: 	none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void ResetUpdown(void)
{
	nUpdownMode=NORMAL;
	nSubMode=NAME;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void SimulateUpdown(long data)
//
// DESCRIPTION:   receives data from 68681 simulation and acts upon it
//
// PARAMETERS:    long data: received data
//
// RETURNS:			none
//
////////////////////////////////////////////////////////////////////////////////
void SimulateUpdown(long data)
{
char temp;

	data=(unsigned char)data;
	if(nUpdownMode==NORMAL)
	{
		switch(data)
		{
			case 0x02: // STX
				nUpdownMode=PRINTER;
				hPrintBuf=GlobalAlloc(GHND,0x10000);
				pPrintBuf=GlobalLock(hPrintBuf);
				nCount=0;
				break;
			case 0x07: //BEL
				RBB=0x07;
				SRB|=0x01;
				break;
			case 'B': // Download
			case 'L': // Download
				nUpdownMode=DOWNLOAD;
				nSubMode=MESSAGE;
				nCount=0;
				break;
			case 'a': // Upload
			case 'S': // Download
				nUpdownMode=UPLOAD;
				nSubMode=MESSAGE;
				nCount=0;
				break;
		}
	}
	else if(nUpdownMode==PRINTER)
	{
		switch(data)
		{
			case 0x03: // ETX
				DoPrint(pPrintBuf,nCount);
				GlobalUnlock(hPrintBuf);
				GlobalFree(hPrintBuf);
				nUpdownMode=NORMAL;
				break;
			default:
				pPrintBuf[nCount]=(char)data;
				nCount++;
				break;
		}
	}
	else if(nUpdownMode==UPLOAD)
	{
		switch(nSubMode)
		{
			case ENDE:
				switch(data)
				{
					case 0x03: // ETX
						_lclose(hFile);
						bLoadFinished=TRUE;
						break;
					case 0x0a:
						temp=0x0d;
						_lwrite(hFile,&temp,1L);
						temp=(char)data;
						_lwrite(hFile,&temp,1L);
						break;
					default:
						temp=(char)data;
						_lwrite(hFile,&temp,1L);
						break;
				}
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void UpdownLoad(void)
//
// DESCRIPTION:   sends data to 68681 simulation
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void UpdownLoad(void)
{
int nBytesRead;

	switch(nUpdownMode)
	{
		case DOWNLOAD:
			switch(nSubMode)
			{
				case MESSAGE:
					RBB=MsgLoad[nCount];
					if(MsgLoad[nCount]==0)
					{
						nSubMode=NAME;
						nCount=0;
						if(OpenAFile(hParentWnd,
										(LPSTR)"Open a file",
										(LPSTR)szFilename,
					(LPSTR)"Any files (*.*)\0*.*\0Text Dateien (*.txt)\0*.txt\0\0",
										(LPSTR)"txt")!=(DWORD)(-1))
						{
							hFile=_lopen(szFilename,OF_READ);
						}
						else strcpy(szFilename,"File error");
						strupr(szFilename);
						RBA=szFilename[nCount++];
						SRA|=0x01;
					}
					else nCount++;
					SRB|=0x01;
					break;
				case NAME:
					RBA=szFilename[nCount];
					if(szFilename[nCount]==0)
					{
						RBA='\r';
						nSubMode=RESPONDNAME;
						nCount=0;
					}
					else nCount++;
					SRA|=0x01;
					break;
				case RESPONDNAME:
					RBB=szFilename[nCount];
					if(szFilename[nCount]==0)
					{
						nSubMode=LOADSTARTS;
						nCount=0;
					}
					else nCount++;
					SRB|=0x01;
					break;
				case LOADSTARTS:
					RBB=MsgLoadStarts[nCount];
					if(MsgLoadStarts[nCount]==0)
					{
						nSubMode=DATA;
						nCount=0;
					}
					else nCount++;
					SRB|=0x01;
					break;
				case DATA:
					nBytesRead=_lread(hFile,&RBB,1);
					if(RBB=='\r')
					{
						_lread(hFile,&RBB,1);
						RBB=0;
					}
					if(nBytesRead!=1)
					{
						nSubMode=ENDE;
						nCount=0;
						_lclose(hFile);
					}
					SRB|=0x01;
					break;
				case ENDE:
					RBB=0x03;
					SRB|=0x01;
					nSubMode=NAME;
					nUpdownMode=NORMAL;
					break;
			}
			break;
		case UPLOAD:
			switch(nSubMode)
			{
				case MESSAGE:
					RBB=MsgSave[nCount];
					if(MsgSave[nCount]==0)
					{
						nSubMode=NAME;
						nCount=0;
						SaveAFile(hParentWnd,(LPSTR)"Save a file",(LPSTR)szFilename,
						(LPSTR)"Any files (*.*)\0*.*\0Text Dateien (*.txt)\0*.txt\0\0",
						(LPSTR)"txt");
						if((hFile=_lopen(szFilename,OF_WRITE))==HFILE_ERROR)
						{
							hFile=_lcreat(szFilename,0);
						}
						strupr(szFilename);
						RBA=szFilename[nCount++];
						SRA|=0x01;
						bLoadFinished=FALSE;
					}
					else nCount++;
					SRB|=0x01;
					break;
				case NAME:
					RBA=szFilename[nCount];
					if(szFilename[nCount]==0)
					{
						RBA='\r';
						nSubMode=RESPONDNAME;
						nCount=0;
					}
					else nCount++;
					SRA|=0x01;
					break;
				case RESPONDNAME:
					RBB=szFilename[nCount];
					if(szFilename[nCount]==0)
					{
						nSubMode=LOADSTARTS;
						nCount=0;
					}
					else nCount++;
					SRB|=0x01;
					break;
				case LOADSTARTS:
					RBB=MsgSaveStarts[nCount];
					if(MsgSaveStarts[nCount]==0)
					{
						nSubMode=ACKNOWLEDGE;
						nCount=0;
					}
					else nCount++;
					SRB|=0x01;
					bLoadFinished=FALSE;
					break;
				case ACKNOWLEDGE:
					RBB='u';
					SRB|=0x01;
					nSubMode=ENDE;
					nCount=0;
					break;
				case ENDE:
					if(bLoadFinished==TRUE)
					{
						nSubMode=NAME;
						nUpdownMode=NORMAL;
					}
					break;
			}
			break;
	}
}
