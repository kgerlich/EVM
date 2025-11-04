////////////////////////////////////////////////////////////////////////////////
// NAME: 			stexep.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						68K CPU exception and interrupt processing
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include "windows-compat.h"
#include <stdio.h>
#include "STMAIN.H"
#include "STMEM.H"
#include "STSTDDEF.H"
#include "STCOM.H"
#include "STSTATE.H"

//******************************************************************************
//																										**
//              exception routines                                            **
//																										**
//******************************************************************************


////////////////////////////////////////////////////////////////////////////////
// NAME:				void bus_err(void)
//
// DESCRIPTION:   process bus error exception
//						caused by missing memory/peripheral device
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void bus_err(void)
{
	if(cpu.pc==pcbefore)
	{
		cpu.ssp-=24;				 		// create short bus cycle fault stack frame
		cpu.ssp-=2;
		PUTword(cpu.ssp,0xa008); 		// FORMAT $A + vector offset
	}
	else
	{
		cpu.ssp-=24;              // create long bus cycle fault stack frame
		cpu.ssp-=2;
		PUTword(cpu.ssp,0xb008); 	// FORMAT $B + vector offset
	}
	cpu.ssp-=4;         							// decrement a7 by one dword
	PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
	cpu.ssp-=2;         							// decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);  // save current status register to stack
	cpu.pc=GETdword((long)(0x08&0x0FFF)+cpu.vbr);   // get cpu.pc for privilege violation (vector #20)
	cpu.aregs.a[7]=cpu.ssp;                  // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000; // setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - BUS ERROR");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void addr_err(void)
//
// DESCRIPTION:   process address error exception
//						caused by uneven memory access
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void addr_err(void)
{
	if(cpu.pc==pcbefore)
	{
		cpu.ssp-=24;						 // create short bus cycle fault stack frame
		cpu.ssp-=2;
		PUTword(cpu.ssp,0xa00c);		 	// FORMAT $A + vector offset
	}
	else
	{
		cpu.ssp-=24;                    // create long bus cycle fault stack frame
		cpu.ssp-=2;
		PUTword(cpu.ssp,0xb00c);		// FORMAT $B + vector offset
	}
	cpu.ssp-=4;         							// decrement a7 by one word
	PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
	cpu.ssp-=2;         							// decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);         // save current status register to stack
	cpu.pc=GETdword((long)(0x0c&0x0FFF)+cpu.vbr);   // get cpu.pc for privilege violation (vector #20)
	cpu.aregs.a[7]=cpu.ssp;            // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000;  // setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - ADDRESS ERROR");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void priv_viol(void)
//
// DESCRIPTION:   process privilege violation error exception
//						caused by restricted operations like ORI.W #imm,SR
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void priv_viol(void)
{
	cpu.ssp-=2;
	PUTword(cpu.ssp,0x0020);                // FORMAT $0 + vector offset
	cpu.ssp-=4;         							// decrement a7 by one word
	PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
	cpu.ssp-=2;      		// decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);   // save current status register to stack
	cpu.pc=GETdword((long)(0x20&0x0FFF)+cpu.vbr);   // get cpu.pc for privilege violation (vector #20)
	cpu.aregs.a[7]=cpu.ssp;    // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000;// setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - PRIVILEGE VIOLATION");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void div_by_zero(void)
//
// DESCRIPTION:   process divide by zero error exception
//						caused by division by zero
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void div_by_zero(void)
{
	if((cpu.sregs.sr&0x2000)==0)        // set stack pointer according to mode
	{
		cpu.usp=cpu.aregs.a[7];
	}
	else cpu.ssp=cpu.aregs.a[7];
	cpu.ssp-=2;
	PUTword(cpu.ssp,0x2014);                // FORMAT $2 + vector offset #14
	cpu.ssp-=4;                             // decrement a7 by one word
	PUTdword(cpu.ssp,cpu.pc);                  // save current cpu.pc to stack
	cpu.ssp-=2;     // decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);   // save current status register to stack
	// get cpu.pc for divide by zero exception (vector #14)
	cpu.pc=GETdword((long)(0x14&0x0FFF)+cpu.vbr);
	cpu.aregs.a[7]=cpu.ssp;   // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000; // setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - DIVIDE BY ZERO");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void single_step(void)
//
// DESCRIPTION:   process single step (trace) exception
//						caused by setting TRACE bits in SR
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void single_step(void)
{
	cpu.ssp-=4;
	PUTdword(cpu.ssp,pcbefore);
	cpu.ssp-=2;
	PUTword(cpu.ssp,0x2024);                // FORMAT $A + vector offset #24
	cpu.ssp-=4;         							// decrement a7 by one word
	PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
	cpu.ssp-=2;         							// decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);         // save current status register to stack
	cpu.pc=GETdword((long)(0x24&0x0FFF)+cpu.vbr);   // get cpu.pc for trace (vector #24)
	cpu.aregs.a[7]=cpu.ssp;                // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000; // setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - TRACE");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void illegal(void)
//
// DESCRIPTION:   process illegal opcode exception
//						caused by illegal opcodes
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void illegal(void)
{
	cpu.ssp-=4;
	PUTdword(cpu.ssp,pcbefore);
	cpu.ssp-=2;
	PUTword(cpu.ssp,0x2010);                // FORMAT $A + vector offset #24
	cpu.ssp-=4;         							// decrement a7 by one word
	PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
	cpu.ssp-=2;         							// decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);           // save current status register to stack
	cpu.pc=GETdword((long)(0x24&0x0FFF)+cpu.vbr);   // get cpu.pc for trace (vector #24)
	cpu.aregs.a[7]=cpu.ssp;                       // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000;  // setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - ILLEGAL OPCODE");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void emulatelinef(void)
//
// DESCRIPTION:   process LINE F excetion
//						caused by opcodes $F000-$FFFF
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void emulatelinef(void)
{
	cpu.ssp-=2;
	PUTword(cpu.ssp,0x002c);                // FORMAT $0 + vector offset
	cpu.ssp-=4;         							// decrement a7 by one word
	PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
	cpu.ssp-=2;         							// decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);         // save current status register to stack
	cpu.pc=GETdword((long)(0x2c&0x0FFF)+cpu.vbr);   // get cpu.pc for line F (vector #2c)
	cpu.aregs.a[7]=cpu.ssp;                           // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000;// setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - LINE F");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void emulatelinea(void)
//
// DESCRIPTION:   process LINE A excetion
//						caused by opcodes $A000-$AFFF
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void emulatelinea(void)
{
	cpu.ssp-=2;
	PUTword(cpu.ssp,0x0028);                // FORMAT $0 + vector offset
	cpu.ssp-=4;         							// decrement a7 by one word
	PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
	cpu.ssp-=2;         							// decrement a7 by two words
	PUTword(cpu.ssp,cpu.sregs.sr);                    // save current status register to stack
	cpu.pc=GETdword((long)(0x28&0x0FFF)+cpu.vbr);   // get cpu.pc for line A (vector #2c)
	cpu.aregs.a[7]=cpu.ssp;          // setup stack for supervisor mode
	cpu.sregs.sr=(cpu.sregs.sr&0x07ff)|0x2000;  // setup status reg for interrupt processing
	SetStatusText(2,"68K Simulator for Windows - LINE A");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void Unknown(short opcode)
//
// DESCRIPTION:   process (non CPU) unknown opcode
//						caused by not implemented opcodes (may be somewhere)
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void Unknown(short opcode)
{
char temp[64];

	sprintf(temp,
	"Unknown instruction\nat PC=0x%lX OPCODE=0x%04X\nSimulation will be stopped.\nUse Reset to restart...",
		savepc,(unsigned short)opcode);
	MessageBox(ghWnd,temp,"ERROR",MB_OK|MB_ICONSTOP|MB_TOPMOST);
	SetStatusText(0,"Stopped...");
	bStopped=TRUE;
}

//******************************************************************************
//																										**
//              Interrupt routines                                            **
//																										**
//******************************************************************************

////////////////////////////////////////////////////////////////////////////////
// NAME:				void CheckForInt(void)
//
// DESCRIPTION:   check if there's an interrupt to process
//
// PARAMETERS:		none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void CheckForInt(void)
{
	// irq priority level > irq privilige level OR NMI (sConn_to_cpu.ipl=7)
	if(sConn_to_cpu.ipl>((cpu.sregs.sr>>8)&0x0007) || sConn_to_cpu.ipl==7)
	{
		bStopped=FALSE;
		nIRQs++;
		// NON autovector interrupt
		if(sConn_to_cpu.bNonAutoVector==TRUE)
		{
			if(sConn_to_cpu.VecNum!=0x0f)
			{
				// setup interrupt stack frame
				cpu.ssp-=2;
				PUTword(cpu.ssp,(sConn_to_cpu.VecNum*4));// FORMAT $0 + vector offset #$100
				cpu.ssp-=4;         		 		// decrement a7 by one word
				PUTdword(cpu.ssp,cpu.pc);	// save current cpu.pc to stack
				cpu.ssp-=2; // decrement a7 by two words
				PUTword(cpu.ssp,cpu.sregs.sr);   // save current status register to stack
				// get cpu.pc for trace (vector #$10+(sConn_to_cpu.VecNum*4))
				cpu.pc=GETdword((long)((sConn_to_cpu.VecNum*4)+cpu.vbr));
				cpu.aregs.a[7]=cpu.ssp; // setup stack for supervisor mode
				// setup status reg for interrupt processing
				cpu.sregs.sr=(cpu.sregs.sr&0x00ff)|0x2000|(sConn_to_cpu.ipl<<8);
				sConn_to_cpu.ipl=0;
				sConn_to_cpu.VecNum=0x0f;
				sConn_to_cpu.bNonAutoVector=FALSE;
			}
			else
			{
				// setup interrupt stack frame
				cpu.ssp-=2;
				PUTword(cpu.ssp,0x3c);                // FORMAT $0 + vector offset #$100
				cpu.ssp-=4;         							// decrement a7 by one word
				PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
				cpu.ssp-=2;         							// decrement a7 by two words
				PUTword(cpu.ssp,cpu.sregs.sr);   // save current status register to stack
				// get cpu.pc for trace (vector #$10+(sConn_to_cpu.VecNum*4))
				cpu.pc=GETdword((long)(0x3c)+cpu.vbr);
				cpu.aregs.a[7]=cpu.ssp;            // setup stack for supervisor mode
				// setup status reg for interrupt processing
				cpu.sregs.sr=(cpu.sregs.sr&0x00ff)|0x2000|(sConn_to_cpu.ipl<<8);
				sConn_to_cpu.ipl=0;
				sConn_to_cpu.VecNum=0x0f;
				sConn_to_cpu.bNonAutoVector=FALSE;
			}
		}
		// autovector interrupt
		else
		{
			// setup interrupt stack frame
			cpu.ssp-=2;
			PUTword(cpu.ssp,(sConn_to_cpu.ipl*4)+0x60);  // FORMAT $0 + vector offset #$64
			cpu.ssp-=4;         							// decrement a7 by one word
			PUTdword(cpu.ssp,cpu.pc); 						// save current cpu.pc to stack
			cpu.ssp-=2;         							// decrement a7 by two words
			PUTword(cpu.ssp,cpu.sregs.sr);                    // save current status register to stack
			// get cpu.pc for trace (vector #$60+(sConn_to_cpu.ipl*4))
			cpu.pc=GETdword((long)((sConn_to_cpu.ipl*4+0x60)+cpu.vbr));
			cpu.aregs.a[7]=cpu.ssp;                           // setup stack for supervisor mode
			// setup status reg for interrupt processing
			cpu.sregs.sr=(cpu.sregs.sr&0x00ff)|0x2000|(sConn_to_cpu.ipl<<8);
			sConn_to_cpu.ipl=0;
			sConn_to_cpu.bNonAutoVector=FALSE;
		}
	}
}


