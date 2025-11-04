////////////////////////////////////////////////////////////////////////////////
// NAME: 			port.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				5/1997
// DESCRIPTION:
//						create and process port window
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include "windows-compat.h"
#include <string.h>
#include <stdio.h>
#include "..\evmsim\stdll.h"
#include "68230.h"
#include "68230.rh" // resources header

////////////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////////////
void DisplayPorts(void); // display and update register window
// port dialog procedure
BOOL CALLBACK PortDlgProc(HWND hWnd,UINT iMessage,WORD wParam,long lParam);
DWORD PortThread(DWORD lpdwParam); // port dialog update thread
////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HWND hPortDlg;
DWORD dwPortThreadId,dwPortThrdParam;
HANDLE hPortThread=0; // handle to timer thread
BOOL bPortRun=FALSE,bDisplayPort=TRUE;

// IDs of address edits
short nPortA[]={IDC_PA7,
					 IDC_PA6,
					 IDC_PA5,
					 IDC_PA4,
					 IDC_PA3,
					 IDC_PA2,
					 IDC_PA1,
					 IDC_PA0};

short nPortDirA[]={IDC_PAD7,
					 IDC_PAD6,
					 IDC_PAD5,
					 IDC_PAD4,
					 IDC_PAD3,
					 IDC_PAD2,
					 IDC_PAD1,
					 IDC_PAD0};

short nPortB[]={IDC_PB7,
					 IDC_PB6,
					 IDC_PB5,
					 IDC_PB4,
					 IDC_PB3,
					 IDC_PB2,
					 IDC_PB1,
					 IDC_PB0};

short nPortDirB[]={IDC_PBD7,
					 IDC_PBD6,
					 IDC_PBD5,
					 IDC_PBD4,
					 IDC_PBD3,
					 IDC_PBD2,
					 IDC_PBD1,
					 IDC_PBD0};

short nPortC[]={IDC_PC7,
					 IDC_PC6,
					 IDC_PC5,
					 IDC_PC4,
					 IDC_PC3,
					 IDC_PC2,
					 IDC_PC1,
					 IDC_PC0};

short nPortDirC[]={IDC_PCD7,
					 IDC_PCD6,
					 IDC_PCD5,
					 IDC_PCD4,
					 IDC_PCD3,
					 IDC_PCD2,
					 IDC_PCD1,
					 IDC_PCD0};

short nHLines[4]={IDC_H1,IDC_H2,IDC_H3,IDC_H4};

////////////////////////////////////////////////////////////////////////////////
// NAME:				void CreatePortsWindow(HANDLE hInstance,HANDLE hParentWindow)
//
// DESCRIPTION:	create the port dialog
//
// PARAMETERS:		HANDLE hInstance: 		instance handle of module containing
//														dialog resource
//						HANDLE hParentWindow:   handle of parent window
//
// RETURNS:			none
//
////////////////////////////////////////////////////////////////////////////////
void CreatePortsWindow(HANDLE hInstance,HANDLE hParentWindow)
{
	hPortDlg=CreateDialog(hInstance,"PORT",hParentWindow,(DLGPROC)PortDlgProc);
	bPortRun=TRUE;
	hPortThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)PortThread,
											&dwPortThrdParam,0,&dwPortThreadId);
	if(!hPortThread)
	{
		MessageBox(DLLHDR.hWnd,"Couldn't create port display thread!",
								"68230.dll",MB_OK);
	}
	SetWindowText(hPortDlg,"MC68230 ports");
	ShowWindow(hPortDlg,SW_SHOW);
	SetWindowPos(hPortDlg,NULL,
						GetPrivateProfileInt("WINDOW","PORTSX",0,".\\68230.ini"),
						GetPrivateProfileInt("WINDOW","PORTSY",0,".\\68230.ini"),
						0,0,SWP_NOZORDER|SWP_NOSIZE);
	UpdateWindow(hPortDlg);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void DisplayPorts(void)
//
// DESCRIPTION:   signal update and display port dialog
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void DisplayPorts(void)
{
	bDisplayPort=TRUE; // just signal need for update to port update thread
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void DestroyPortDialog(void)
//
// DESCRIPTION:   close port dialog window
//
// PARAMETERS:		none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void DestroyPortDialog(void)
{
char temp[64];
RECT rc;
POINT pt;

	if(hPortThread)
	{
		bPortRun=FALSE;
		WaitForSingleObject(hPortThread,1000);
		hPortThread=0;
	}
	GetWindowRect(hPortDlg,&rc);
	pt.x=rc.left; pt.y=rc.top;
	ScreenToClient(DLLHDR.hWnd,&pt);
	sprintf(temp,"%ld",pt.x);
	WritePrivateProfileString("WINDOW","PORTSX",temp,".\\68230.ini");
	sprintf(temp,"%ld",pt.y);
	WritePrivateProfileString("WINDOW","PORTSY",temp,".\\68230.ini");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				DWORD PortThread(DWORD lpdwParam)
//
// DESCRIPTION:	thread that updates the port dialog
//
// PARAMETERS:    DWORD lpdwParam: not used
//
// RETURNS: 	   DWORD: exit state of thread (not used)
//
// REMARKS:			this thread is needed to update the port
//						display in background, so the performance
//						of the simulation doesn't break down
//						the side effect is that the port windows
//						update is kind of slow compared to direct
//						updating
////////////////////////////////////////////////////////////////////////////////

DWORD PortThread(DWORD lpdwParam)
{
char temp[16];
int i;

	while(bPortRun)
	{
		if(bDisplayPort)
		{
			bDisplayPort=FALSE;
			// display values of ports as hex numbers
			// and as bit fields
			if(OldPADR!=PADR)
			{
				wsprintf(temp,"0x%02X",PADR);            // PADR
				SetDlgItemText(hPortDlg,IDC_PA,temp);
				for(i=0;i<8;i++)
				{
					SendDlgItemMessage(hPortDlg,nPortA[i],BM_SETCHECK,
											(PADR&(0x80>>i))?1:0,0L);
				}
				OldPADR=PADR;
			}
			if(OldPADDR!=PADDR)
			{
				wsprintf(temp,"0x%02X",PADDR);           // PADDR
				SetDlgItemText(hPortDlg,IDC_PAD,temp);
				for(i=0;i<8;i++)
				{
					EnableWindow(GetDlgItem(hPortDlg,nPortA[i]),
									(PADDR&(0x80>>i))?FALSE:TRUE);
					SendDlgItemMessage(hPortDlg,nPortDirA[i],
											 BM_SETCHECK,(PADDR&(0x80>>i))?1:0,0L);
				}
				OldPADDR=PADDR;
			}

			if(OldPBDR!=PBDR)
			{
				wsprintf(temp,"0x%02X",PBDR);            // PBDR
				SetDlgItemText(hPortDlg,IDC_PB,temp);
				for(i=0;i<8;i++)
				{
					SendDlgItemMessage(hPortDlg,nPortB[i],BM_SETCHECK,
											(PBDR&(0x80>>i))?1:0,0L);
				}
				OldPBDR=PBDR;
			}
			if(OldPBDDR!=PBDDR)
			{
				wsprintf(temp,"0x%02X",PBDDR);           // PBDDR
				SetDlgItemText(hPortDlg,IDC_PBD,temp);
				for(i=0;i<8;i++)
				{
					EnableWindow(GetDlgItem(hPortDlg,nPortB[i]),
									(PBDDR&(0x80>>i))?FALSE:TRUE);
					SendDlgItemMessage(hPortDlg,nPortDirB[i],
											BM_SETCHECK,(PBDDR&(0x80>>i))?1:0,0L);
				}
				OldPBDDR=PBDDR;
			}
			if(OldPCDR!=PCDR)
			{
				wsprintf(temp,"0x%02X",PCDR);            // PCDR
				SetDlgItemText(hPortDlg,IDC_PC,temp);
				for(i=0;i<8;i++)
				{
					SendDlgItemMessage(hPortDlg,nPortC[i],BM_SETCHECK,
											(PCDR&(0x80>>i))?1:0,0L);
				}
				OldPCDR=PCDR;
			}
			if(OldPCDDR!=PCDDR)
			{
				wsprintf(temp,"0x%02X",PCDDR);           // PCDDR
				SetDlgItemText(hPortDlg,IDC_PCD,temp);
				for(i=0;i<8;i++)
				{
					EnableWindow(GetDlgItem(hPortDlg,nPortC[i]),
									(PCDDR&(0x80>>i))?FALSE:TRUE);
					SendDlgItemMessage(hPortDlg,nPortDirC[i],BM_SETCHECK,
											(PCDDR&(0x80>>i))?1:0,0L);
				}
				OldPCDDR=PCDDR;
			}

			for(i=0;i<4;i++)
			{
				SendDlgItemMessage(hPortDlg,nHLines[i],BM_SETCHECK,
										(HLINES&(0x01<<i))?1:0,0L);
			}
		}
		else Sleep(10);
	}
	return 0L;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				void ToggleBit(BYTE* pb,int pos)
//
// DESCRIPTION:	toggle a bit in a byte
//
// PARAMETERS:    BYTE *pb: address of byte
//						int pos:  position of bit (from left,values 0-7)
//
// RETURNS: 		none
//
////////////////////////////////////////////////////////////////////////////////
void ToggleBit(BYTE* pb,int pos)
{
	*pb=*pb^(0x80>>pos);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				int SearchID(short nIDArray[],short nID)
//
// DESCRIPTION:	search a window control ID in an array and return index
//
// PARAMETERS:    short nIDArray[]: array to search in
//						short nID:        ID of control to search
//
// RETURNS:
//
////////////////////////////////////////////////////////////////////////////////
int SearchID(short nIDArray[],short nID)
{
int i;
	// search for handle of button that was clicked
	for(i=0;i<8 && nIDArray[i]!=nID;i++);
	if(i<8)return i;
	// not found
	return -1;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				BOOL CALLBACK PortDlgProc(HWND hWnd,UINT iMessage,
//															WORD wParam,long lParam)
//
// DESCRIPTION:	dialog procedure for port dialog
//
// PARAMETERS:		(standard WIN32 API)
//
// RETURNS:       (standard WIN32 API)
//
////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK PortDlgProc(HWND hWnd,UINT iMessage,WORD wParam,long lParam)
{
int i;

	// what message?
	switch(iMessage)
	{
		// command message
		case WM_COMMAND:
			switch(HIWORD(wParam))
			{
				// a button was clicked
				case BN_CLICKED:
					if((i=SearchID(nPortA,(short)LOWORD(wParam)))!=-1)
					{
						ToggleBit(&PADR,i);
						DisplayPorts();
					}
					else if((i=SearchID(nPortB,(short)LOWORD(wParam)))!=-1)
					{
						ToggleBit(&PBDR,i);
						DisplayPorts();
					}
					else if((i=SearchID(nPortC,(short)LOWORD(wParam)))!=-1)
					{
						ToggleBit(&PCDR,i);
						DisplayPorts();
					}
					else if(LOWORD(wParam)==IDC_H1)
					{
						HLINES|=0x01;
						DisplayPorts();
					}
					else if(LOWORD(wParam)==IDC_H2)
					{
						HLINES|=0x02;
						DisplayPorts();
					}
					else if(LOWORD(wParam)==IDC_H3)
					{
						HLINES|=0x04;
						DisplayPorts();
					}
					else if(LOWORD(wParam)==IDC_H4)
					{
						HLINES|=0x08;
						DisplayPorts();
					}
					break;
			}
			break;
	}
	return FALSE; // message not processed
}


