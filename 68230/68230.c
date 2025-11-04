////////////////////////////////////////////////////////////////////////////////
// NAME: 			68230.c (DLL)
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:   simulate a 68230 PI/T (Parallel Interface/Timer)
////////////////////////////////////////////////////////////////////////////////
#define STRICT  // strict type checking
#define WIN32_LEAN_AND_MEAN  // only basic windows stuff
#include "windows-compat.h" // portable Windows API compatibility
#include <stdio.h>  // standard IO
#include <stdlib.h>
#include <string.h> // strings
#include <commdlg.h> // common dialogs (CHOOSEFONT)
#include "port.h"
#include "writes.h"
#include "..\evmsim\stdll.h" // the includes needed for DLL linking

////////////////////////////////////////////////////////////////////////////////
// declare _DLLHDR header
// CPU address space:	0x800000-0x800035
// PRIORITY 		  :	NORMAL
// CACHEABLE		  :	can NOT be cached
////////////////////////////////////////////////////////////////////////////////
MAKE_HDR(0x00800000L,0x0036L,SIM_NORMAL_PRIORITY,FALSE);


////////////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////////////
DWORD TimerThread(DWORD lpdwParam);

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////

// 68230 registers
BYTE PGCR,PSRR,PADDR,PBDDR,PCDDR,PIVR,PACR,PBCR,PADR,PBDR,PAAR,PBAR,PCDR,PSR,
	  TCR,TIVR,TSR,HLINES;
BYTE OldPADR,OldPBDR,OldPCDR,OldPADDR,OldPBDDR,OldPCDDR;

BOOL bPIVRwritten; // PIVR has not been written since RESET

DWORD dwThreadId,dwThrdParam;
HANDLE hTimerThread=0; // handle to timer thread
BOOL bTimerRun=FALSE;
DWORD nTicks=0,nOldTicks=0;
short nIPL;

// provide all sizes access to counter/ counter preload registers
union
{
	DWORD CPR; // long word access
	struct // byte access
	{
		BYTE CPRL,CPRM,CPRH,dummy;
	}b;
}CP;
union
{
	DWORD CNTR; // long word access
	struct   // byte access
	{
		BYTE CNTRL,CNTRM,CNTRH,dummy;
	}b;
}CNT;

BYTE *PIT_regs[]={NULL,&PGCR,  // array for 68230 register access
						NULL,&PSRR,
						NULL,&PADDR,
						NULL,&PBDDR,
						NULL,&PCDDR,
						NULL,&PIVR,
						NULL,&PACR,
						NULL,&PBCR,
						NULL,&PADR,
						NULL,&PBDR,
						NULL,&PAAR,
						NULL,&PBAR,
						NULL,&PCDR,
						NULL,&PSR,
						NULL,NULL,
						NULL,NULL,
						NULL,&TCR,
						NULL,&TIVR,
						NULL,NULL,
						NULL,&CP.b.CPRH,
						NULL,&CP.b.CPRM,
						NULL,&CP.b.CPRL,
						NULL,NULL,
						NULL,&CNT.b.CNTRH,
						NULL,&CNT.b.CNTRM,
						NULL,&CNT.b.CNTRL,
						NULL,&TSR};

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
short index=(short)(address&0x0000003F);
BYTE data;

	if(PIT_regs[index]!=NULL)
	{
		switch(index)
		{
			default:
				data=*PIT_regs[index]; // read from the selected register
				break;
		}
		return (unsigned long)data;
	}
	else return 0xff;
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
short index=(short)(address&0x0000003F);

	if(PIT_regs[index]!=NULL)
	{
		switch(index)
		{
			case 0x0b: // PIVR
				PIVR=(BYTE)data&0xFC; // only write upper 6 bits of vector number
				bPIVRwritten=TRUE;
				break;
			case 0x11: // PADR
				WritePADR((BYTE)data);
				DisplayPorts();
				break;
			case 0x13: // PBDR
				WritePBDR((BYTE)data);
				DisplayPorts();
				break;
			case 0x15: // PAAR
			case 0x17: // PBAR
						// no writes to these regs
				break;
			case 0x19: // PCDR
				WritePCDR((BYTE)data);
				DisplayPorts();
				break;
			case 0x1B: // PSR
				PSR&=~((BYTE)data&0x0F); // clear bits that are on in mask
				break;
			case 0x27: // CPRL: copy preload to counter value
				*PIT_regs[index]=(BYTE)data; // write to the selected register
				CNT.b.CNTRH=(BYTE)data;
				break;
			case 0x29: // CPRM: copy preload to counter value
				*PIT_regs[index]=(BYTE)data; // write to the selected register
				CNT.b.CNTRM=(BYTE)data;
				break;
			case 0x2B: // CPRH: copy preload to counter value
				*PIT_regs[index]=(BYTE)data; // write to the selected register
				CNT.b.CNTRL=(BYTE)data;
				break;
			case 0x35: //TSR: reset interrupt flag on write
				if(data&0x01)
				{
					data=0;
					*PIT_regs[index]=(BYTE)data; // write to the selected register
				}
				break;
			default:
				*PIT_regs[index]=(BYTE)data; // write to the selected register
		}
	}
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
	// process timer modes
	if(TCR&0x01)  // timer unlocked ?
	{
		switch((TCR>>5)&0x07) // switch modes
		{
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 6:
				break;
			case 5:
			case 7:
				if((nTicks-nOldTicks)>=CNT.CNTR)
				{
					// set ZDS bit in TCR if counter went zero and ZDS enabled
					TSR|=0x01;
					if(!(TCR&0x10)) // reload timer?
					{
						CNT.CNTR=CP.CPR;
					}
					else CNT.CNTR=0x00FFFFFF;
				}
				else CNT.CNTR-= (nTicks-nOldTicks);
				nOldTicks=nTicks;

				if(TSR&0x01)
				{
					DLLHDR.pConnect->ipl=(char)nIPL;
					DLLHDR.pConnect->VecNum=(TIVR);
					DLLHDR.pConnect->bNonAutoVector=TRUE;
				}
				break;
		}
	}
	if(!(PGCR&0xC0)) // mode 0
	{
		if(PGCR&0x20) // H34 enabled
		{
			if(!(PBCR&0x20) && PBCR&0x04) // H4 is input and H4 interrupt enabled
			{
				if(HLINES&0x08)
				{
					DLLHDR.pConnect->ipl=(char)nIPL;
					if(bPIVRwritten)DLLHDR.pConnect->VecNum=(PIVR&0xFE)+3;
					else DLLHDR.pConnect->VecNum=PIVR;
					DLLHDR.pConnect->bNonAutoVector=TRUE;
					HLINES&=~0x08;
					PSR|=0x88;
				}
			}
			if(PBCR&0x02) // H3 interrupt enabled
			{
				if(HLINES&0x04)
				{
					DLLHDR.pConnect->ipl=(char)nIPL;
					if(bPIVRwritten)DLLHDR.pConnect->VecNum=(PIVR&0xFE)+2;
					else DLLHDR.pConnect->VecNum=PIVR;
					DLLHDR.pConnect->bNonAutoVector=TRUE;
					HLINES&=~0x04;
					PSR|=0x44;
				}
			}
		}
		if(PGCR&0x10) // H12 enabled
		{
			if(!(PACR&0x20) && PACR&0x04) // H2 is input and H2 interrupt enabled
			{
				if(HLINES&0x02)
				{
					DLLHDR.pConnect->ipl=(char)nIPL;
					if(bPIVRwritten)DLLHDR.pConnect->VecNum=(PIVR&0xFE)+1;
					else DLLHDR.pConnect->VecNum=PIVR;
					DLLHDR.pConnect->bNonAutoVector=TRUE;
					HLINES&=~0x02;
					PSR|=0x22;
				}
			}
			if(PACR&0x02) // H1 interrupt enabled
			{
				if(HLINES&0x01)
				{
					DLLHDR.pConnect->ipl=(char)nIPL;
					if(bPIVRwritten)DLLHDR.pConnect->VecNum=(PIVR&0xFE);
					else DLLHDR.pConnect->VecNum=PIVR;
					DLLHDR.pConnect->bNonAutoVector=TRUE;
					HLINES&=~0x01;
					PSR|=0x11;
				}
			}
		}
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
	CreatePortsWindow((HINSTANCE)GetModuleHandle("68230.dll"),DLLHDR.hWnd);
	nIPL=GetPrivateProfileInt("HARDWARE","IPL",3,".\\68230.ini");

	// install timer tick thread
	bTimerRun=TRUE;
	hTimerThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TimerThread,
											&dwThrdParam,0,&dwThreadId);
	// FAILURE -> notify user
	if(!hTimerThread)
	{
		MessageBox(DLLHDR.hWnd,"Couldn't create tick timer!","68230.dll",MB_OK);
		return FALSE;
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
	// kill timer tick thread
	if(hTimerThread)
	{
		bTimerRun=FALSE;
		WaitForSingleObject(hTimerThread,1000);
		hTimerThread=0;
	}
	// destroy port dialog window
	DestroyPortDialog();
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
	// put all registers into a known state
	// port registers
	OldPADR=~PADR;
	OldPADDR=~PADDR;
	OldPBDR=~PBDR;
	OldPBDDR=~PBDDR;
	PGCR=0; // port general control reg
	PSRR=0; // port service request reg
	PADDR=0; // port A data direction reg
	PBDDR=0; // port B data direction reg
	PCDDR=0; // port C data direction reg
	PIVR=0x0f; // port interrupt vector reg
	bPIVRwritten=FALSE; // PIVR has not been written since RESET
	PACR=0;    // port A control reg
	PBCR=0;    // port B control reg
	PCDR=(BYTE)0xAA;//random(0x100); // port C direction reg (any value)
	PSR=0xaa; // port status reg (any value)
	// timer registers
	TCR=0;  // timer control reg
	TIVR=0x0f; // timer interrupt vector reg
	TSR=0; // timer status reg
	HLINES=0; // H1-4 lines

	// update window
	DisplayPorts();
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				DWORD TimerThread(DWORD lpdwParam)
//
// DESCRIPTION:   timer tick thread
//
// PARAMETERS:    DWORD lpdwParam: (not used)
//
// RETURNS:       DWORD: exit state of thread
//
////////////////////////////////////////////////////////////////////////////////
DWORD TimerThread(DWORD lpdwParam)
{
	while(bTimerRun)
	{
		if(TCR&0x01) // timer enables
		{
			if(TCR&0x02)nTicks+=6250*5; // no prescaler in timer (1 tick=160ns)
			else nTicks+=195*5; // prescaler in timer (1 tick=160ns*32=5ns)
			Sleep(1);
		}
		else Sleep(0); // not enabled -> give up time slice
	}
	return 0L;
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
	// put one time INITIALISATION after Setup() here
	// update window
	DisplayPorts();
}




