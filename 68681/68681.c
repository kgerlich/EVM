////////////////////////////////////////////////////////////////////////////////
// NAME: 			68681.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				5/1997
// DESCRIPTION:
//						simulate a 68681 DUART (Dual Serial Interface)
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "..\evmsim\stdll.h"
#include "vt100.h"
#include "updown.h"


////////////////////////////////////////////////////////////////////////////////
// declare _DLLHDR header
// CPU address space:	0xA00000-0xA0001F
// PRIORITY 		  :	NORMAL
// CACHEABLE		  :	can NOT be cached (obsolete)
////////////////////////////////////////////////////////////////////////////////
MAKE_HDR(0x00A00000L,0x00000020L,SIM_NORMAL_PRIORITY,FALSE);

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HWND hParentWnd;
char *gszAppName="QUME VT101";
BOOL bReadRBA=FALSE;

// register set
BYTE MR1A,MR2A,SRA,CSRA,CRA,RBA,TBA,IPCR,ACR,ISR,IMR,CTUR,CTLR,
	  MR1B,MR2B,SRB,CSRB,CRB,RBB,TBB,IVR,IPU,OPCR,OPR;
BYTE CMSB,CLSB;

// memory mapping of
// DUART write registers
BYTE *DUART_W_regs[]={NULL,&MR1A, // $A00001 mode register A
							 NULL,&CSRA, // $A00003 clock select register
							 NULL,&CRA,  // $A00005 command register A
							 NULL,&TBA,  // $A00007 transmitter buffer A
							 NULL,&ACR,  // $A00009 auxiliary control register
							 NULL,&IMR,  // $A0000B interrupt mask register
							 NULL,&CTUR, // $A0000D counter/timer uppper register
							 NULL,&CTLR, // $A0000F counter/timer lower register
							 NULL,&MR1B, // $A00011 mode register B
							 NULL,&CSRB, // $A00013 clock select register B
							 NULL,&CRB,  // $A00015 command register B
							 NULL,&TBB,  // $A00017 transmitter buffer B
							 NULL,&IVR,  // $A00019 interrupt vector register
							 NULL,&OPCR, // $A0001B output port config register
							 NULL,&OPR,  // $A0001D bit set command
							 NULL,&OPR}; // $A0001F bit clear command

// memory mapping of
// DUART read registers
BYTE *DUART_R_regs[]={NULL,&MR1A, // $A00001 mode register A
							 NULL,&SRA,  // $A00003 status register A
							 NULL,NULL,  // $A00005 nothing here
							 NULL,&RBA,  // $A00007 receiver buffer A
							 NULL,&IPCR, // $A00009 input port change register
							 NULL,&ISR,  // $A0000B interrupt status register
							 NULL,&CMSB, // $A0000D current MSB of counter
							 NULL,&CLSB, // $A0000F current LSB of counter
							 NULL,&MR1B, // $A00011 mode register B
							 NULL,&SRB,  // $A00013 status register B
							 NULL,NULL,  // $A00015 nothing here
							 NULL,&RBB,  // $A00017 receiver buffer B
							 NULL,&IVR,  // $A00019 interrupt vector register
							 NULL,&IPU,  // $A0001B input port unlatched
							 NULL,NULL,  // $A0001D nothing here
							 NULL,NULL}; // $A0001F nothing here



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
int index=address&0x0000001F; // get index of register by masking out

	if(DUART_R_regs[index]!=NULL) // is there anything on this address
	{
		switch(index) // use index to get register selected
		{
			case 3:
				bReadRBA=TRUE; // RBA has been read
				break;
			case 7: // RBA offset 7
				if(bReadRBA)  // has RBA been read ?
				{
					bReadRBA=FALSE;
					SRA&=0xfe;  // clear RXRDY
				}
				break;
			case 0x13: // SRA offset 0x13
				if(nUpdownMode!=0)UpdownLoad();
				break;
			case 0x17: // RBB offset 0x17
				SRB&=0xfe;  // clear RXRDY
				break;
		}
		return *DUART_R_regs[index]; // read from the selected register
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
int index=address&0x0000001F; // get index of register by masking

	if(DUART_W_regs[index]!=NULL) // is there anything
	{
		switch(index) // use index to get selected register
		{
			case 7: // TBA offset 7
				SRA&=~0x04; // clear TXRDY
				SimulateTerminal((BYTE)data);
				break;
			case 0x17: // TBB offset 0x17
				SRB&=~0x04; // clear TXRDY
				SimulateUpdown(data);
				break;
			default:  // default to do nothing
				break;
		}
		*DUART_W_regs[index]=(BYTE)data; // write to the selected register
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
	SRA|=0x04;  // set TXRDY channel A
	SRB|=0x04;  // set TXRDY and RXRDY channel B

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
BOOL Setup()
{
	hParentWnd=DLLHDR.hWnd;
	SetupTerminal();
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
	ExitTerminal();
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
	ResetUpdown();
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
	ResetUpdown();
}
