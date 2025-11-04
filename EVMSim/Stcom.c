////////////////////////////////////////////////////////////////////////////////
// NAME: 			stcom.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						decode and execute an 68K opcode
////////////////////////////////////////////////////////////////////////////////
#define STRICT // strict type checking
#define WIN32_LEAN_AND_MEAN  // use only the barest windows API
#include "windows-compat.h" // portable Windows API compatibility layer
#include <stdlib.h>  // include functions LDIV (structure LDIV_T, DV)
							// division operations
#include "STMAIN.H"  // includes of stmain.c
#include "STFLAGS.H" // MC68000 flag functions
#include "STMEM.H"   // memory access
#include "STEACALC.H" // calc effective address
#include "STEXEP.H"   // exception handling
#include "stdll.h"     // header for runtime dynamic linking of plugin modules
#include "ststartw.h"  // startup window header
#include "macros.h"    // macros for CPU status register operations
#include <math.h>

// this macro is the lead-in to every execution function
// and does nothing at the moment
// but could be used in dynamic compiling or debugging
#define CACHEFUNCTION(p)

// opcode jump table created by BUILD.EXE from SKRIPT.TXT
extern void (*Operation[])(short opcode);

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
BOOL bStopped=FALSE;	   // set by opcode STOP #XX to indicate
								// a stopped instruction processing
unsigned long nIRQs=0;  // count the occurrance of HW irqs

ULONG hits=0,misses=0;

// pointers to plugin _DLLHDRs
struct tag_DLLHDR* pDllHdr[64];

// structure for event connection between plugin and CPU struct
struct tag_CONN_TO_CPU sConn_to_cpu;


// union for opcode decoding
// divide opcode into bit fields
union
{
		unsigned short o;  // Opcode
		// | 15   14   13   12   11   10   9   8   7   6   5   4   3   2   1   0 |
		// | group             | regdest     | modedest  | modesrc   | regsrc    |
		struct
		{
			unsigned regsrc   : 3;   // 0x0007
			unsigned modesrc  : 3;   // 0x0038
			unsigned modedest : 3;   // 0x01C0
			unsigned regdest  : 3;   // 0x0E00
			unsigned group    : 4;   // 0xF000
		}general;
		struct
		{
			unsigned regsrc   : 3;   // 0x0007
			unsigned modesrc  : 3;   // 0x0038
			unsigned size     : 2;   // 0x00C0
			unsigned reserved : 4;   // 0x0F00
			unsigned group    : 4;   // 0xF000
		}special;
}of; // Opcode

// CPU structure
// the virtual CPUs register set
CPU cpu;

// internal working registers
struct tag_work
{
	long source,destination,result;
}work;

long savepc,spc;  // Temporary PC

long pcbefore; // PC before instruction
char AddressError=0;

// EA calculation function pointer array
long (*CommandMode[8])(char reg,char command,long destination,char size)=
		{DRD,ARD,ARI,ARIPI,ARIPD,ARID,ARII,MISC};

////////////////////////////////////////////////////////////////////////////////
// NAME:          void COM_[name](short opcode);
//
// DESCRIPTION:   execution functions for CPU core
// 					the following functions have one definition in common:
// 					their parameter always is:  short opcode:
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:	OR immediate to EA
////////////////////////////////////////////////////////////////////////////////

void COM_ori(short opcode)
{
	cpu.pc=cpu.pc+2; // increment PC to after opcode
	switch(of.special.size) // check size of operation (BYTE,WORD,DWORD)
	{
		case 0: // ORI.b //
			work.source=GETword(cpu.pc)&0x00FF; // get immediate byte
			cpu.pc=cpu.pc+2;                    // increment PC by two
			spc=cpu.pc;                         // save current PC because
															// CommandMode() might alter it
			// get destination value
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			0,of.special.size);
			cpu.pc=spc;  // restore saved PC
			// do binary OR of source and destination
			work.destination=(work.destination)|(work.source&0x000000FF);
			// write back result
			CommandMode[of.general.modesrc](of.general.regsrc,1,
													  work.destination,of.special.size);
			// generate flag status
			if((char)work.destination<0)NEG1; // NEG flag
			else NEG0;
			if((char)work.destination==0)ZERO1; // ZERO flag
			else ZERO0;
			break;
		case 1:
			work.source=GETword(cpu.pc); // get immediate word
			cpu.pc=cpu.pc+2;             // increment PC by two
			spc=cpu.pc;                  // save current PC because
												  // CommandMode() might alter it
			// get destination value
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																				0,of.special.size);
			cpu.pc=spc;   // restore saved PC
			// do binary OR of source and destination
			work.destination=(work.destination)|(work.source&0x0000FFFF);
			// write back result
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
														of.special.size);
			// generate flag status
			if((short)work.destination<0)NEG1;
			else NEG0;
			if((short)work.destination==0)ZERO1;
			else ZERO0;
			break;
		case 2:
			work.source=GETdword(cpu.pc);
			cpu.pc+=4;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			 0,of.special.size);
			cpu.pc=spc;
			work.destination=(work.destination)|(work.source);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
													  of.special.size);
			if(work.destination<0)NEG1;
			else NEG0;
			if(work.destination==0)ZERO1;
			else ZERO0;
			break;
	}
	// always clear OVERFLOW and CARRY flag
	OVER0;
	CARRY0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: AND immediate to EA
////////////////////////////////////////////////////////////////////////////////

void COM_andi(short opcode)
{
	cpu.pc=cpu.pc+2;
	switch(of.special.size)
	{
		case 0: // ANDI.b //
			work.source=GETword(cpu.pc)&0x00FF;
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			 0,of.special.size);
			cpu.pc=spc;
			work.destination=(work.destination)&(work.source|0xFFFFFF00L);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
													  of.special.size);
			if((char)work.destination<0)NEG1;
			else NEG0;
			if((char)work.destination==0)ZERO1;
			else ZERO0;
			break;
		case 1:
			work.source=GETword(cpu.pc);
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			 0,of.special.size);
			cpu.pc=spc;
			work.destination=(work.destination)&(work.source|0xFFFF0000L);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
													  of.special.size);
			if((short)work.destination<0)NEG1;
			else NEG0;
			if((short)work.destination==0)ZERO1;
			else ZERO0;
			break;
		case 2:
			work.source=GETdword(cpu.pc);
			cpu.pc=cpu.pc+4;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			 0,of.special.size);
			cpu.pc=spc;
			work.destination=(work.destination)&(work.source);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
													  of.special.size);
			if(work.destination<0)NEG1;
			else NEG0;
			if(work.destination==0)ZERO1;
			else ZERO0;
			break;
	}
	OVER0;
	CARRY0;
}


////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: SUB immediate to EA
////////////////////////////////////////////////////////////////////////////////

void COM_subi(short opcode)
{
	cpu.pc=cpu.pc+2;
	switch(of.special.size)
	{
		case 0: // SUBI.b
			work.source=GETword(cpu.pc)&0x00FF;
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																			 of.special.size);
			cpu.pc=spc;
			work.result=(work.destination)-(work.source&0x000000FF);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.result,
													  of.special.size);
			setcarry( gen_carry((char)work.source,(char)work.destination,
									  (char)work.result));
			setover( gen_over((char)work.source,(char)work.destination,
						(char)work.result) );
			if((char)work.result<0)NEG1;
			else NEG0;
			if((char)work.result==0)ZERO1;
			else ZERO0;
			break;
		case 1:
			work.source=GETword(cpu.pc);
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																			 of.special.size);
			cpu.pc=spc;
			work.result=(work.destination)-(work.source&0x0000FFFF);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.result,
													  of.special.size);
			setcarry( gen_carry((short)work.source,(short)work.destination,
									(short)work.result) );
			setover( gen_over((short)work.source,(short)work.destination,
						(short)work.result) );
			if((short)work.result<0)NEG1;
			else NEG0;
			if((short)work.result==0)ZERO1;
			else ZERO0;
			break;
		case 2:
			work.source=GETdword(cpu.pc);
			cpu.pc=cpu.pc+4;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																			 of.special.size);
			cpu.pc=spc;
			work.result=(work.destination)-(work.source);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.result,
														of.special.size);
			setcarry( gen_carry(work.source,work.destination,work.result) );
			setover( gen_over(work.source,work.destination,work.result) );
			if(work.result<0)NEG1;
			else NEG0;
			if(work.result==0)ZERO1;
			else ZERO0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: ADD immediate to EA
////////////////////////////////////////////////////////////////////////////////

void COM_addi(short opcode)
{
	cpu.pc=cpu.pc+2;
	switch(of.special.size)
	{
		case 0: // ADDI.b
			work.source=GETword(cpu.pc)&0x00FF;
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																			 of.special.size);
			cpu.pc=spc;
			work.result=(work.destination)+(work.source&0x000000FF);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.result,
														of.special.size);
			setcarry( gen_carry((char)work.source,(char)work.destination,
										(char)work.result) );
			setover( gen_over((char)work.source,(char)work.destination,
									(char)work.result) );
			if((char)work.result<0)NEG1;
			else NEG0;
			if((char)work.result==0)ZERO1;
			else ZERO0;
			break;
		case 1:
			work.source=GETword(cpu.pc);
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																			 of.special.size);
			cpu.pc=spc;
			work.result=(work.destination)+(work.source&0x0000FFFF);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.result,
														of.special.size);
			setcarry( gen_carry((short)work.source,(short)work.destination,
										(short)work.result) );
			setover( gen_over((short)work.source,(short)work.destination,
									(short)work.result) );
			if((short)work.result<0)NEG1;
			else NEG0;
			if((short)work.result==0)ZERO1;
			else ZERO0;
			break;
		case 2:
			work.source=GETdword(cpu.pc);
			cpu.pc=cpu.pc+4;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			  0,of.special.size);
			cpu.pc=spc;
			work.result=(work.destination)+(work.source);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.result,
																		of.special.size);
			setcarry( gen_carry(work.source,work.destination,work.result) );
			setover( gen_over(work.source,work.destination,work.result) );
			if(work.result<0)NEG1;
			else NEG0;
			if(work.result==0)ZERO1;
			else ZERO0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: EOR immediate to EA
////////////////////////////////////////////////////////////////////////////////

void COM_eori(short opcode)
{
	cpu.pc=cpu.pc+2;
	switch(of.special.size)
	{
		case 0: // EORI.b
			work.source=GETword(cpu.pc)&0x00FF;
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			0,of.special.size);
			cpu.pc=spc;
			work.destination=(work.destination)^(work.source&0x000000FF);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
														of.special.size);
			if((char)work.destination<0)NEG1;
			else NEG0;
			if((char)work.destination==0)ZERO1;
			else ZERO0;
			break;
		case 1:
			work.source=GETword(cpu.pc);
			cpu.pc=cpu.pc+2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																				of.special.size);
			cpu.pc=spc;
			work.destination=(work.destination)^(work.source&0x0000FFFF);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
														of.special.size);
			if((short)work.destination<0)NEG1;
			else NEG0;
			if((short)work.destination==0)ZERO1;
			else ZERO0;
			break;
		case 2:
			work.source=GETdword(cpu.pc);
			cpu.pc=cpu.pc+4;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																			of.special.size);
			cpu.pc=spc;
			work.destination=(work.destination)^(work.source);
			CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,
														of.special.size);
			if(work.destination<0)NEG1;
			else NEG0;
			if(work.destination==0)ZERO1;
			else ZERO0;
			break;
	}
	OVER0;
	CARRY0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: CMP immediate to EA
////////////////////////////////////////////////////////////////////////////////

void COM_cmpi(short opcode)
{
	switch(of.special.size)
	{
		case 0:	// CMPI.b
			cpu.pc+=2;
			work.source=(char)GETword(cpu.pc)&0x000000FF;
			cpu.pc+=2;
			work.destination=(char)CommandMode[of.general.modesrc](of.general.regsrc,
															0,0,of.special.size)&0x000000FF;
			work.result=work.destination-work.source;
			setcarry( gen_carry((char)work.source,(char)work.destination,
																	(char)work.result) );
			setover( gen_over((char)work.source,(char)work.destination,
																(char)work.result) );
			if((char)work.result<0)NEG1;
			else NEG0;
			if((char)work.result==0)ZERO1;
			else ZERO0;
			break;
		case 1: // CMPI.w
			cpu.pc=cpu.pc+2;
			work.source=GETword(cpu.pc);
			cpu.pc+=2;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,
																of.special.size)&0x0000FFFF;
			work.result=work.destination-work.source;
			setcarry( gen_carry((short)work.source,(short)work.destination,
																(short)work.result) );
			setover( gen_over((short)work.source,(short)work.destination,
																(short)work.result) );
			if((short)work.result<0)NEG1;
			else NEG0;
			if((short)work.result==0)ZERO1;
			else ZERO0;
			break;
		case 2: // CMPI.l //
			cpu.pc=cpu.pc+2;
			work.source=GETdword(cpu.pc);
			cpu.pc=cpu.pc+4;
			work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,
																			0,of.special.size);
			work.result=work.destination-work.source;
			setcarry( gen_carry(work.source,work.destination,work.result) );
			setover( gen_over(work.source,work.destination,work.result) );
			if(work.result<0)NEG1;
			else NEG0;
			if(work.result==0)ZERO1;
			else ZERO0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move multiple register to EA
////////////////////////////////////////////////////////////////////////////////
void COM_movemtoEA(short opcode)
{
static unsigned short offset,reglist,size,i;

	CACHEFUNCTION(COM_movemtoEA);
	cpu.pc+=4; // increment PC to after movem opcode + reglist field
	reglist=GETword(cpu.pc-2); // get reglist
	switch(of.general.modesrc) // mode field //
	{
		case 2: // ARI //
			// get work register a[]
			work.destination=cpu.aregs.a[of.general.regsrc];
			offset=0; // start with offset 0
			size=(opcode>>6)&0x0001;  // get size (word or dword)
			for(i=0;i<8;i++) // for each bit in reglist
			{
				if(reglist&0x0001) // bit is set, then store corresponding register
				{
					// size ?
					// store dword
					if(size)PUTdword(work.destination+4*offset,cpu.dregs.d[i]);
					// store word
					else PUTword(work.destination+2*offset,
									(unsigned short)cpu.dregs.d[i]);
					offset++;
				}
				reglist=reglist>>1;
			}
			for(i=0;i<8;i++)
			{
				if(reglist&0x0001)
				{
					if(size)PUTdword(work.destination+4*offset,cpu.aregs.a[i]);
					else PUTword(work.destination+2*offset,
											(unsigned short)cpu.aregs.a[i]);
					offset++;
				}
				reglist=reglist>>1;
			}
			break;
		case 4: // ARIPD //
			size=(opcode>>6)&0x0001;
			for(i=0;i<8;i++)
			{
				if(reglist&0x0001)
				{
					if(size)
					{
						cpu.aregs.a[of.general.regsrc]-=4;
						work.destination=cpu.aregs.a[of.general.regsrc];
						PUTdword(work.destination,cpu.aregs.a[7-i]);
					}
					else
					{
						cpu.aregs.a[of.general.regsrc]-=2;
						work.destination=cpu.aregs.a[of.general.regsrc];
						PUTword(work.destination,(unsigned short)cpu.aregs.a[7-i]);
					}
				}
				reglist=reglist>>1;
			}
			for(i=0;i<8;i++)
			{
				if(reglist&0x0001)
				{
					if(size)
					{
						cpu.aregs.a[of.general.regsrc]-=4;
						work.destination=cpu.aregs.a[of.general.regsrc];
						PUTdword(work.destination,cpu.dregs.d[7-i]);
					}
					else
					{
						cpu.aregs.a[of.general.regsrc]-=2;
						work.destination=cpu.aregs.a[of.general.regsrc];
						PUTword(work.destination,(unsigned short)cpu.dregs.d[7-i]);
					}
				}
				reglist=reglist>>1;
			}
			break;
		case 5: // ARID //
			work.destination=cpu.aregs.a[of.general.regsrc]+(long)GETword(cpu.pc);
			cpu.pc+=2;
			offset=0;
			size=(opcode>>6)&0x0001;
			for(i=0;i<8;i++)
			{
				if(reglist&0x0001)
				{
					if(size)PUTdword(work.destination+4*offset,cpu.aregs.a[i]);
					else PUTword(work.destination+2*offset,
										(unsigned short)cpu.aregs.a[i]);
					offset++;
				}
				reglist=reglist>>1;
			}
			for(i=0;i<8;i++)
			{
				if(reglist&0x0001)
				{
					if(size)PUTdword(work.destination+4*offset,cpu.dregs.d[i]);
					else PUTword(work.destination+2*offset,
										(unsigned short)cpu.dregs.d[i]);
					offset++;
				}
				reglist=reglist>>1;
			}
			break;
		case 6: // ARII //
			Unknown(opcode);
			break;
		case 7: // MISC addressing modes//
			switch(of.general.regsrc)
			{
				case 0:  // (XXX).w
					work.destination=(unsigned long)GETword(cpu.pc);
					offset=0;
					size=(opcode>>6)&0x0001;
					cpu.pc+=2;
					for(i=0;i<8;i++)
					{
						if(reglist&0x0001)
						{
							if(size)PUTdword(work.destination+4*offset,cpu.dregs.d[i]);
							else PUTword(work.destination+2*offset,
											(unsigned short)cpu.dregs.d[i]);
							offset++;
						}
						reglist=reglist>>1;
					}
					for(i=0;i<8;i++)
					{
						if(reglist&0x0001)
						{
							if(size)PUTdword(work.destination+4*offset,cpu.aregs.a[i]);
							else PUTword(work.destination+2*offset,
											(unsigned short)cpu.aregs.a[i]);
							offset++;
						}
						reglist=reglist>>1;
					}
					break;
				case 1:  // (XXX).l
					work.destination=GETdword(cpu.pc);
					cpu.pc+=4;
					offset=0;
					size=(opcode>>6)&0x0001;
					for(i=0;i<8;i++)
					{
						if(reglist&0x0001)
						{
							if(size)PUTdword(work.destination+4*offset,cpu.dregs.d[i]);
							else PUTword(work.destination+2*offset,
											(unsigned short)cpu.dregs.d[i]);
							offset++;
						}
						reglist=reglist>>1;
					}
					for(i=0;i<8;i++)
					{
						if(reglist&0x0001)
						{
							if(size)PUTdword(work.destination+4*offset,cpu.aregs.a[i]);
							else PUTword(work.destination+2*offset,
											(unsigned short)cpu.aregs.a[i]);
							offset++;
						}
						reglist=reglist>>1;
					}
					break;
				}
				break;
			default:
				Unknown(opcode);
				break;
		}
}

//////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: extend data at EA
//////////////////////////////////////////////////////////////////////////////////

void COM_ext(short opcode)
{
	CACHEFUNCTION(COM_ext);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 2:
			cpu.dregs.d[of.general.regsrc]=(cpu.dregs.d[of.general.regsrc]
						&0xFFFF0000L)|((short)((char)cpu.dregs.d[of.general.regsrc]));
			if(((short)(cpu.dregs.d[of.general.regsrc]&0x0000FFFF))==0)ZERO1;
			else ZERO0;
			if(((short)(cpu.dregs.d[of.general.regsrc]&0x0000FFFF))<0)NEG1;
			else NEG0;
			break;
		case 3:
			cpu.dregs.d[of.general.regsrc]=(long)((short)cpu.dregs.d[of.general.regsrc]);
			if(cpu.dregs.d[of.general.regsrc]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regsrc]<0)NEG1;
			else NEG0;
			break;
		case 7:
			cpu.dregs.d[of.general.regsrc]=(long)((char)cpu.dregs.d[of.general.regsrc]);
			if(cpu.dregs.d[of.general.regsrc]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regsrc]<0)NEG1;
			else NEG0;
			break;
	}
	CARRY0;
	OVER0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move to/from peripheral
////////////////////////////////////////////////////////////////////////////////

void COM_movep(short opcode)
{
short displacement;
	CACHEFUNCTION(COM_movep);
	cpu.pc+=4;
	displacement=GETword(cpu.pc-2);
	switch(of.general.modedest)
	{
		case 4: // word mem -> reg //
			cpu.dregs.d[of.general.regdest]=(cpu.dregs.d[of.general.regdest]&0xFFFF00FFL)
			+( ( GETbyte(cpu.aregs.a[of.general.regsrc]+(long)displacement) <<8)&0x0000FF00L);
			cpu.dregs.d[of.general.regdest]=(cpu.dregs.d[of.general.regdest]&
					0xFFFFFF00L)+(GETbyte( cpu.aregs.a[of.general.regsrc]+
						(long)displacement+2L)&0x000000FFL);
			break;
		case 5: // long mem -> reg //
			cpu.dregs.d[of.general.regdest]=(cpu.dregs.d[of.general.regdest]&0x00FFFFFFL)
			+( ( GETbyte(cpu.aregs.a[of.general.regsrc]+(long)displacement) <<8)&0xFF000000L);
			cpu.dregs.d[of.general.regdest]=(cpu.dregs.d[of.general.regdest]&0xFF00FFFFL)
			+( ( GETbyte(cpu.aregs.a[of.general.regsrc]+(long)displacement+2L) <<8)&0x00FF0000L);
			cpu.dregs.d[of.general.regdest]=(cpu.dregs.d[of.general.regdest]&0xFFFF00FFL)
			+( ( GETbyte(cpu.aregs.a[of.general.regsrc]+(long)displacement+4L) <<8)&0x0000FF00L);
			cpu.dregs.d[of.general.regdest]=(cpu.dregs.d[of.general.regdest]&0xFFFFFF00L)
			+(GETbyte( cpu.aregs.a[of.general.regsrc]+(long)displacement+6L)&0x000000FFL);
			break;
		case 6: // word reg -> mem //
			PUTbyte( cpu.aregs.a[of.general.regsrc]+(long)displacement,
						cpu.dregs.byted[of.general.regdest].dlh );
			PUTbyte( cpu.aregs.a[of.general.regsrc]+(long)displacement+2L,
						cpu.dregs.byted[of.general.regdest].dll);
			break;
		case 7: // long reg -> mem //
			PUTbyte( cpu.aregs.a[of.general.regsrc]+(long)displacement,
						cpu.dregs.byted[of.general.regdest].duh);
			PUTbyte( cpu.aregs.a[of.general.regsrc]+(long)displacement+2L,
						cpu.dregs.byted[of.general.regdest].dul);
			PUTbyte( cpu.aregs.a[of.general.regsrc]+(long)displacement+4L,
						cpu.dregs.byted[of.general.regdest].dlh );
			PUTbyte( cpu.aregs.a[of.general.regsrc]+(long)displacement+6L,
						cpu.dregs.byted[of.general.regdest].dll);
			break;
		default:
			Unknown(opcode);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: dynamic bit test
////////////////////////////////////////////////////////////////////////////////

void COM_dyntstbit(short opcode)
{
static char mod,size;

	CACHEFUNCTION(COM_dyntstbit);
	cpu.pc+=2;
	if(of.general.modesrc)
	{
		size=0;
		mod=8;
	}
	else
	{
		size=2;
		mod=32;
	}
	switch(of.special.size)
	{
		case 0:     // btst //
			work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,size);
			if(work.source&(1<<(cpu.dregs.d[of.general.regdest]%mod)))ZERO0;
			else ZERO1;
			break;
		case 1:     // bchg //
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,size);
			cpu.pc=spc;
			if(work.source&(1<<cpu.dregs.d[of.general.regdest]))
			{
				ZERO0;
				work.source=work.source&(~(1<<(cpu.dregs.d[of.general.regdest]%mod)));
			}
			else
			{
				ZERO1;
				work.source=work.source|(1<<(cpu.dregs.d[of.general.regdest]%mod));
			}
			CommandMode[of.general.modesrc]((of.general.regsrc),1,work.source,size);
			break;
		case 2:     // bclr //
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,size);
			cpu.pc=spc;
			if(work.source&(1<<(cpu.dregs.d[of.general.regdest]%mod)))ZERO0;
			else ZERO1;
			work.source=work.source&(~(1<<(cpu.dregs.d[of.general.regdest]%mod)));
			CommandMode[of.general.modesrc]((of.general.regsrc),1,work.source,size);
			break;
		case 3:     // bset //
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,size);
			cpu.pc=spc;
			if(work.source&(1<<(cpu.dregs.d[of.general.regdest]%mod)))ZERO0;
			else ZERO1;
			work.source=work.source|(1<<(cpu.dregs.d[of.general.regdest]%mod));
			CommandMode[of.general.modesrc]((of.general.regsrc),1,work.source,size);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: static bit test
////////////////////////////////////////////////////////////////////////////////

void COM_stattstbit(short opcode)
{
static char mod,size;

	CACHEFUNCTION(COM_stattstbit);
	cpu.pc+=2;
	if(of.general.modesrc)
	{
		size=0;
		mod=8;
	}
	else
	{
		size=2;
		mod=32;
	}
	switch(of.special.size)
	{
		case 0: // btst //
			work.source=(BYTE)GETword(cpu.pc);
			cpu.pc=cpu.pc+2;
			work.destination=CommandMode[of.general.modesrc]((of.general.regsrc),0,
																				0L,size);
			if(work.destination&(1<<(work.source%mod)))ZERO0;
			else ZERO1;
			break;
		case 1:  // bchg //
			work.source=(BYTE)GETword(cpu.pc);
			cpu.pc+=2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc]((of.general.regsrc),0,
																				0L,size);
			cpu.pc=spc;
			if(work.destination&(1<<(work.source%mod)))
			{
				ZERO0;
				work.destination=work.destination&~(1<<(work.source%mod));
			}
			else
			{
				ZERO1;
				work.destination=work.destination|(1<<(work.source%mod));
			}
			CommandMode[of.general.modesrc]((of.general.regsrc),1,
														work.destination,size);
			break;
		case 2: // bclr //
			work.source=(BYTE)GETword(cpu.pc);
			cpu.pc+=2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc]((of.general.regsrc),
																					0,0L,size);
			cpu.pc=spc;
			if(work.destination&(1<<(work.source%mod)))ZERO0;
			else ZERO1;
			work.destination=work.destination&~(1<<(work.source%mod));
			CommandMode[of.general.modesrc]((of.general.regsrc),1,work.destination,
																					size);
			break;
		case 3: // bset //
			work.source=(BYTE)GETword(cpu.pc);
			cpu.pc+=2;
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc]((of.general.regsrc),
																					0,0L,size);
			cpu.pc=spc;
			if(work.destination&(1<<(work.source%mod)))ZERO1;
			else ZERO0;
			work.destination=work.destination|(1<<(work.source%mod));
			CommandMode[of.general.modesrc]((of.general.regsrc),1,work.destination
																,size);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: load effective address
////////////////////////////////////////////////////////////////////////////////
// REMARKS:
// 			most of the PC indirect modes are not implemented, only (d16,PC) can
//				can be used
////////////////////////////////////////////////////////////////////////////////

void COM_lea(short opcode)
{
short extension;
long index,od,bd;

	CACHEFUNCTION(COM_lea);
	cpu.pc+=2;
	switch(of.general.modesrc)
	{
		case 2: // ARI
			cpu.aregs.a[of.general.regdest]=cpu.aregs.a[(of.general.regsrc)];
			break;
		case 5: // ARID
			cpu.aregs.a[of.general.regdest]=cpu.aregs.a[(of.general.regsrc)]+
														(long)GETword(cpu.pc);
			cpu.pc+=2;
			break;
		case 6: // ARII
			extension=GETword(cpu.pc);
			cpu.pc+=2;
			switch((extension&0x0100)>>8)
			{
				case 0: // brief format extension
					if(extension&0x8000) // (d8,Asrc,An.x*scale)
					{
						if(extension&0x0800) // long word
						{
							cpu.aregs.a[of.general.regdest]=cpu.aregs.a[(of.general.regsrc)]+
								(cpu.aregs.a[(extension>>12)&0x0007])*
									(1<<((extension>>9)&0x0003));
						}
						else
						{
							cpu.aregs.a[of.general.regdest]=cpu.aregs.a[(of.general.regsrc)]+
							 (long)((short)cpu.aregs.a[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
					}
					else // Dn
					{
						if(extension&0x0800) // long word
						{
							cpu.aregs.a[of.general.regdest]=cpu.aregs.a[(of.general.regsrc)]+
							  (cpu.dregs.d[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
						else
						{
							cpu.aregs.a[of.general.regdest]=cpu.aregs.a[(of.general.regsrc)]+
							 (long)((short)cpu.dregs.d[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
					}
					cpu.aregs.a[of.general.regdest]+=(long)(extension&0x00FF);
					break;
				case 1: // long format extension

					if(!(extension&0x0080)) // BD is not suppressed
					{
						cpu.aregs.a[of.general.regdest]=cpu.aregs.a[(of.general.regsrc)];
					}
					else cpu.aregs.a[of.general.regdest]=0;

					switch(extension&0x8800)
					{
						case 0x0000: // Dn.W
							index=(long)(short)cpu.dregs.d[(extension>>12)&0x0007];
							break;
						case 0x0800: // Dn.L
							index=cpu.dregs.d[(extension>>12)&0x0007];
							break;
						case 0x8000: // An.W
							index=(long)(short)cpu.aregs.a[(extension>>12)&0x0007];
							break;
						case 0x8800: // An.L
							index=cpu.aregs.a[(extension>>12)&0x0007];
							break;
					}

					// scale index
					switch((extension>>9)&0x0003)
					{
						case 0:
							break;
						case 1:
							index*=2;
							break;
						case 2:
							index*=4;
							break;
						case 3:
							index*=8;
							break;
					}

					switch((extension>>4)&0x0003) // BD size
					{
						case 0: // reserved
						case 1: // null displacement
							bd=0;
							break;
						case 2: // word displacement
							bd=(long)(short)GETword(cpu.pc);
							cpu.pc+=2;
							break;
						case 3: // long displacement
							bd=(long)GETdword(cpu.pc);
							cpu.pc+=4;
							break;
					}
					if(!(extension&0x0040)) // Index is not suppressed
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								cpu.aregs.a[of.general.regdest]+=index+bd;
								break;
							case 1: // indirect pre-indexed with null displacement
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+bd+index);
								break;
							case 2: // indirect pre-indexed with word displacement
								od=(long)GETword(cpu.pc);
								cpu.pc+=2;
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+index+bd)+od;
								break;
							case 3: // indirect pre-indexed with long displacement
								od=GETdword(cpu.pc);
								cpu.pc+=4;
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+index+bd)+od;
								break;
							case 4: // reserved
								break;
							case 5: // indirect post-indexed with null displacement
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+bd)+index;
								break;
							case 6: // indirect post-indexed with word displacement
								od=(long)GETword(cpu.pc);
								cpu.pc+=2;
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+bd)+index+od;
								break;
							case 7: // indirect post-indexed with long displacement
								od=GETdword(cpu.pc);
								cpu.pc+=4;
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+bd)+index+od;
								break;
						}
					}
					else
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								cpu.aregs.a[of.general.regdest]+=bd;
								break;
							case 1: // memory indirect with null displacement
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+bd);
								break;
							case 2: // memory indirect with word displacement
								od=(long)GETword(cpu.pc);
								cpu.pc+=2;
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+bd)+od;
								break;
							case 3: // memory indirect with long displacement
								od=GETdword(cpu.pc);
								cpu.pc+=4;
								cpu.aregs.a[of.general.regdest]=
									GETdword(cpu.aregs.a[of.general.regdest]+bd)+od;
								break;
							case 4: // reserved
							case 5:
							case 6:
							case 7:
								break;
						}
					}
					break;
			}
			break;
		case 7: // MISC (PC,Absolute)
			switch(of.general.regsrc)
			{
				case 0: // (XXX).w
					cpu.aregs.a[of.general.regdest]=(long)GETword(cpu.pc);
					cpu.pc+=2;
					break;
				case 1: // (XXX).l
					cpu.aregs.a[of.general.regdest]=GETdword(cpu.pc);
					cpu.pc+=4;
					break;
				case 2: // (d16,PC)
					cpu.aregs.a[of.general.regdest]=cpu.pc+(long)GETword(cpu.pc);
					cpu.pc+=2;
					break;
				default:
					Unknown(opcode);
					break;
			}
			break;
		default:
			Unknown(opcode);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: check against bounds
////////////////////////////////////////////////////////////////////////////////

void COM_chk(short opcode)
{
	Unknown(opcode);
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move SR to EA
////////////////////////////////////////////////////////////////////////////////

void COM_movefromSR(short opcode)
{
	CACHEFUNCTION(COM_movefromSR);
	// test if in supervisor modes
	if(cpu.sregs.sr&0x3000)
	{
		cpu.pc+=2;
		CommandMode[of.general.modesrc]((of.general.regsrc),1,cpu.sregs.sr,1);
	}
	else priv_viol();

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: negate with extend
////////////////////////////////////////////////////////////////////////////////

void COM_negx(short opcode)
{
	CACHEFUNCTION(COM_negx);
	cpu.pc+=2;
	spc=cpu.pc;
	work.destination=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,
																	of.special.size);
	cpu.pc=spc;
	work.destination=0-work.destination-(cpu.sregs.ccr&0x10);
	CommandMode[of.general.modesrc]((of.general.regsrc),1,work.destination,
														of.special.size);
	switch(of.special.size)
	{
		case 0:
			if((char)work.destination==0)
			{
				ZERO1;
				CARRY0;
				XTEND0;
			}
			else
			{
				ZERO0;
				CARRY1;
				XTEND1;
			}
			if((char)work.destination<0)NEG1;
			else NEG0;
		break;
		case 1:
			if((short)work.destination==0)
			{
				ZERO1;
				CARRY0;
				XTEND0;
			}
			else
			{
				ZERO0;
				CARRY1;
				XTEND1;
			}
			if((short)work.destination<0)NEG1;
			else NEG0;
		break;
		case 2:
			if(work.destination==0)
			{
				ZERO1;
				CARRY0;
				XTEND0;
			}
			else
			{
				ZERO0;
				CARRY1;
				XTEND1;
			}
			if(work.destination<0)NEG1;
			else NEG0;
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: clear EA
////////////////////////////////////////////////////////////////////////////////

void COM_clr(short opcode)
{
	CACHEFUNCTION(COM_clr);
	cpu.pc=cpu.pc+2;	// increment PC
	// write zero to destination
	work.destination=CommandMode[of.general.modesrc]((of.general.regsrc),1,0L,
							of.special.size);
	CARRY0; // flags
	OVER0;
	NEG0;
	ZERO1;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move to CCR
////////////////////////////////////////////////////////////////////////////////

void COM_movetoCCR(short opcode)
{
	CACHEFUNCTION(COM_movetoCCR);
	cpu.pc+=2;
	work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,1);
	cpu.sregs.ccr=(char)work.source;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: negate EA
////////////////////////////////////////////////////////////////////////////////

void COM_neg(short opcode)
{
	CACHEFUNCTION(COM_neg);
	cpu.pc+=2;
	spc=cpu.pc;
	work.destination=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,
																	of.special.size);
	cpu.pc=spc;
	work.destination=0-work.destination;
	CommandMode[of.general.modesrc]((of.general.regsrc),1,work.destination,
																of.special.size);
	switch(of.special.size)
	{
		case 0:
			if((char)work.destination==0)
			{
				ZERO1;
				CARRY0;
				XTEND0;
			}
			else
			{
				ZERO0;
				CARRY1;
				XTEND1;
			}
		break;
		case 1:
			if((short)work.destination==0)
			{
				ZERO1;
				CARRY0;
				XTEND0;
			}
			else
			{
				ZERO0;
				CARRY1;
				XTEND1;
			}
		break;
		case 2:
			if(work.destination==0)
			{
				ZERO1;
				CARRY0;
				XTEND0;
			}
			else
			{
				ZERO0;
				CARRY1;
				XTEND1;
			}
		break;
	}
	switch(of.special.size)
	{
		case 0:
			if((char)work.destination<0)NEG1;
			else NEG0;
			break;
		case 1:
			if((short)work.destination<0)NEG1;
			else NEG0;
			break;
		case 2:
			if(work.destination<0)NEG1;
			else NEG0;
			break;
	}

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move to SR (priviledged)
////////////////////////////////////////////////////////////////////////////////

void COM_movetoSR(short opcode)
{
	CACHEFUNCTION(COM_movetoSR);
	// test if in supervisor modes (master and supervisor)
	if(cpu.sregs.sr&0x3000)
	{
		cpu.pc+=2;
		work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,1);
		cpu.sregs.sr=(short)work.source;
		cpu.aregs.a[7]=cpu.usp;
	}
	else priv_viol();
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: not EA
////////////////////////////////////////////////////////////////////////////////

void COM_not(short opcode)
{
	CACHEFUNCTION(COM_not);
	cpu.pc+=2;
	spc=cpu.pc;
	work.source=~CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,
																		of.special.size);
	cpu.pc=spc;
	CommandMode[of.general.modesrc]((of.general.regsrc),1,work.source,
												of.special.size);
	CARRY0;
	OVER0;
	switch(of.special.size)
	{
		case 0:
			if((char)work.destination==0)ZERO1;
			else ZERO0;
			break;
		case 1:
			if((short)work.destination==0)ZERO1;
			else ZERO0;
			break;
		case 2:
			if(work.destination==0)ZERO1;
			else ZERO0;
			break;
	}
	switch(of.special.size)
	{
		case 0:
			if((char)work.destination<0)NEG1;
			else NEG0;
			break;
		case 1:
			if((short)work.destination<0)NEG1;
			else NEG0;
			break;
		case 2:
			if(work.destination<0)NEG1;
			else NEG0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: negate decimal with extend
////////////////////////////////////////////////////////////////////////////////

void COM_nbcd(short opcode)
{
char DecBorrow=0;
char source;
	cpu.pc+=2;
	spc=cpu.pc;
	work.source=(char)CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,0);
	source = work.source;
	cpu.pc=spc;
	__asm{
		xor eax,eax
		mov ecx,OFFSET work
		mov bl,byte ptr source
		sub al,bl
		das
		jnc  noneedborrow1
		add ah,1
	noneedborrow1:
		mov bl,cpu.sregs.ccr
		shr bl,4
		and bl,1
		sub al,bl
		das
		jnc  noneedborrow2
		add ah,1
	noneedborrow2:
		mov DecBorrow,ah
		mov byte ptr source,al
	}
	work.source = source;
	CommandMode[of.general.modesrc]((of.general.regsrc),1,work.source,0);
	if((char)work.source)ZERO0;
	if(DecBorrow)
	{
		CARRY1;
		XTEND1;
	}
	else
	{
		CARRY0;
		XTEND0;
	}

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: swap register halves
////////////////////////////////////////////////////////////////////////////////

void COM_swap(short opcode)
{
unsigned short high,low;

	CACHEFUNCTION(COM_swap);
	high=(cpu.dregs.wordd[(of.general.regsrc)].dh); // get high word temporary
	low=(cpu.dregs.wordd[(of.general.regsrc)].dl);  // get low word temporary
	cpu.dregs.wordd[(of.general.regsrc)].dl=high;   // put back in swapped order
	cpu.dregs.wordd[(of.general.regsrc)].dh=low;
	if(cpu.dregs.d[(of.general.regsrc)]<0)NEG1;     // test conditions
	else NEG0;
	if(low==0 && high==0)ZERO1;
	else ZERO0;
	cpu.pc=cpu.pc+2;
	OVER0;
	CARRY0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: push EA
////////////////////////////////////////////////////////////////////////////////

void COM_pea(short opcode)
{
short extension;
long ea, index,od,bd;

	CACHEFUNCTION(COM_pea);
	cpu.aregs.a[7]-=4;
	cpu.pc=cpu.pc+2;
	switch(of.general.modesrc)
	{
		// ARI
		case 2:
			PUTdword(cpu.aregs.a[7],cpu.aregs.a[(of.general.regsrc)]);
			break;
		// ARID
		case 5:
			PUTdword(cpu.aregs.a[7],cpu.aregs.a[(of.general.regsrc)]+
									(long)GETword(cpu.pc));
			cpu.pc+=2;
			break;
		// ARII
		case 6: // ARII
			extension=GETword(cpu.pc);
			cpu.pc+=2;
			switch((extension&0x0100)>>8)
			{
				case 0: // brief format extension
					if(extension&0x8000) // (d8,Asrc,An.x*scale)
					{
						if(extension&0x0800) // long word
						{
							ea=cpu.aregs.a[(of.general.regsrc)]+
							 (cpu.aregs.a[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
						else
						{
							ea=cpu.aregs.a[(of.general.regsrc)]+
							 (long)((short)cpu.aregs.a[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
					}
					else // Dn
					{
						if(extension&0x0800) // long word
						{
							ea=cpu.aregs.a[(of.general.regsrc)]+
							 (cpu.dregs.d[(extension>>12)&0x0007])*
							  (1<<((extension>>9)&0x0003));
						}
						else
						{
							ea=cpu.aregs.a[(of.general.regsrc)]+
							 (long)((short)cpu.dregs.d[(extension>>12)&0x0007])*
							  (1<<((extension>>9)&0x0003));
						}
					}
					ea+=(long)(extension&0x00FF);
					break;
				case 1: // long format extension

					if(!(extension&0x0080)) // BD is not suppressed
					{
						ea=cpu.aregs.a[(of.general.regsrc)];
					}
					else ea=0;

					switch(extension&0x8800)
					{
						case 0x0000: // Dn.W
							index=(long)(short)cpu.dregs.d[(extension>>12)&0x0007];
							break;
						case 0x0800: // Dn.L
							index=cpu.dregs.d[(extension>>12)&0x0007];
							break;
						case 0x8000: // An.W
							index=(long)(short)cpu.aregs.a[(extension>>12)&0x0007];
							break;
						case 0x8800: // An.L
							index=cpu.aregs.a[(extension>>12)&0x0007];
							break;
					}

					// scale index
					switch((extension>>9)&0x0003)
					{
						case 0:
							break;
						case 1:
							index*=2;
							break;
						case 2:
							index*=4;
							break;
						case 3:
							index*=8;
							break;
					}

					switch((extension>>4)&0x0003) // BD size
					{
						case 0: // reserved
						case 1: // null displacement
							bd=0;
							break;
						case 2: // word displacement
							bd=(long)(short)GETword(cpu.pc);
							cpu.pc+=2;
							break;
						case 3: // long displacement
							bd=(long)GETdword(cpu.pc);
							cpu.pc+=4;
							break;
					}
					if(!(extension&0x0040)) // Index is not suppressed
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								ea+=index+bd;
								break;
							case 1: // indirect pre-indexed with null displacement
								ea=GETdword(ea+bd+index);
								break;
							case 2: // indirect pre-indexed with word displacement
								od=(long)GETword(cpu.pc);
								cpu.pc+=2;
								ea=GETdword(ea+index+bd)+od;
								break;
							case 3: // indirect pre-indexed with long displacement
								od=GETdword(cpu.pc);
								cpu.pc+=4;
								ea=GETdword(ea+index+bd)+od;
								break;
							case 4: // reserved
								break;
							case 5: // indirect post-indexed with null displacement
								ea=GETdword(ea+bd)+index;
								break;
							case 6: // indirect post-indexed with word displacement
								od=(long)GETword(cpu.pc);
								cpu.pc+=2;
								ea=GETdword(ea+bd)+index+od;
								break;
							case 7: // indirect post-indexed with long displacement
								od=GETdword(cpu.pc);
								cpu.pc+=4;
								ea=GETdword(ea+bd)+index+od;
								break;
						}
					}
					else
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								ea+=bd;
								break;
							case 1: // memory indirect with null displacement
								ea=GETdword(ea+bd);
								break;
							case 2: // memory indirect with word displacement
								od=(long)GETword(cpu.pc);
								cpu.pc+=2;
								ea=GETdword(ea+bd)+od;
								break;
							case 3: // memory indirect with long displacement
								od=GETdword(cpu.pc);
								cpu.pc+=4;
								ea=GETdword(ea+bd)+od;
								break;
							case 4: // reserved
							case 5:
							case 6:
							case 7:
								break;
						}
					}
					break;
			}
			PUTdword(cpu.aregs.a[7],ea);
			break;
		case 7:
			switch(of.general.regsrc)
			{
				case 0:
					PUTdword(cpu.aregs.a[7],(long)GETword(cpu.pc));
					cpu.pc+=2;
					break;
				case 1:
					PUTdword(cpu.aregs.a[7],GETdword(cpu.pc));
					cpu.pc+=4;
					break;
				case 2:
					PUTdword(cpu.aregs.a[7],cpu.pc+(long)GETword(cpu.pc));
					cpu.pc+=2;
					break;
			}
			break;
		default:
			Unknown(opcode);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: test EA
////////////////////////////////////////////////////////////////////////////////

void COM_tst(short opcode)
{
	CACHEFUNCTION(COM_tst);
	cpu.pc+=2;
	work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,
																of.special.size);
	switch(of.special.size)
	{
		case 0:
			if((char)work.source==0)ZERO1;
			else ZERO0;
			if((char)work.source<0)NEG1;
			else NEG0;
			break;
		case 1:
			if((short)work.source==0)ZERO1;
			else ZERO0;
			if((short)work.source<0)NEG1;
			else NEG0;
			break;
		case 2:
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
	}
	CARRY0;
	OVER0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: illegal opcode trap
////////////////////////////////////////////////////////////////////////////////

void COM_illegal(short opcode)
{
	illegal();
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: test and set
////////////////////////////////////////////////////////////////////////////////

void COM_tas(short opcode)
{
	cpu.pc+=2;                    // increment PC by two
	spc=cpu.pc;           // save current PC because CommandMode() might alter it
	// get destination value
	work.destination=CommandMode[of.general.modesrc](of.general.regsrc,0,0,0);
	cpu.pc=spc;  // restore saved PC
	// generate flag status
	if((char)work.destination<0)NEG1; // NEG flag
	else NEG0;
	if((char)work.destination==0)ZERO1; // ZERO flag
	else ZERO0;
	// do binary OR of source and destination
	work.destination=(char)(work.destination)|0x80;
	// write back result
	CommandMode[of.general.modesrc](of.general.regsrc,1,work.destination,0);
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: divide (MC68020)
////////////////////////////////////////////////////////////////////////////////
// REMARKS:
//				64 bit division not tested
////////////////////////////////////////////////////////////////////////////////

void COM_div020(short opcode)
{
short extension;
ldiv_t dv;
long Dr,Dq;

	CACHEFUNCTION(COM_div020);
	cpu.pc+=2;
	extension=GETword(cpu.pc);
	cpu.pc+=2;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),
																0,0L,2);
	if(work.source)
	{
		switch((extension>>11)&0x0001)
		{
			case 0: // DIVU
				switch((extension>>10)&0x0001)
				{
					case 0: // 32 bit to Dq
						dv=ldiv((unsigned long)cpu.dregs.d[(extension>>12)&0x0007],
									(unsigned long)work.source);
						cpu.dregs.d[extension&0x0007]=(unsigned long)dv.rem;
						cpu.dregs.d[(extension>>12)&0x0007]=(unsigned long)dv.quot;
						if((unsigned long)cpu.dregs.d[(extension>>12)&0x0007]<
									(unsigned short)work.source)OVER1;
						else OVER0;
						break;
					case 1: // 64 bit in Dq:Dr
						Dr=cpu.dregs.d[extension&0x0007];
						Dq=cpu.dregs.d[(extension>>12)&0x0007];
						__asm{
							mov edx,dword ptr Dr // get upper 32 bits
							mov eax,dword ptr Dq // get lower 32 bits
							div dword ptr work.source // 64 bit divide
							mov Dq,eax
							mov Dr,edx
						}
						cpu.dregs.d[extension&0x0007]=Dr;
						cpu.dregs.d[(extension>>12)&0x0007]=Dq;
						break;
				}
				break;
			case 1: // DIVS
				switch((extension>>10)&0x0001)
				{
					case 0: // 32 bit to Dq
						dv=ldiv((unsigned long)cpu.dregs.d[(extension>>12)&0x0007],
														(unsigned long)work.source);
						cpu.dregs.d[extension&0x0007]=(unsigned long)dv.rem;
						cpu.dregs.d[(extension>>12)&0x0007]=(unsigned long)dv.quot;
						if((short)cpu.dregs.d[(extension>>12)&0x0007]<(short)work.source)OVER1;
						else OVER0;
						break;
					case 1: // 64 bit in Dq:Dr
						Dr=cpu.dregs.d[extension&0x0007];
						Dq=cpu.dregs.d[(extension>>12)&0x0007];
						__asm{
							mov edx,dword ptr Dr // get upper 32 bits
							mov eax,dword ptr Dq // get lower 32 bits
							div dword ptr work.source // 64 bit divide
							mov Dq,eax
							mov Dr,edx
						}
						cpu.dregs.d[extension&0x0007]=Dr;
						cpu.dregs.d[(extension>>12)&0x0007]=Dq;
						break;
				}
				break;
		}
		if(cpu.dregs.d[(extension>>12)&0x0007]==0)ZERO1;
		else ZERO0;
		if(cpu.dregs.d[(extension>>12)&0x0007]<0)NEG1;
		else NEG0;
	}
	else div_by_zero();
	CARRY0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: multiply (MC68020)
////////////////////////////////////////////////////////////////////////////////

void COM_mul020(short opcode)
{
static short extension;

	CACHEFUNCTION(COM_mul020);
	cpu.pc+=2;
	extension=GETword(cpu.pc);
	cpu.pc+=2;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
	switch((extension>>11)&0x0001)
	{
		case 0: // MULU
			switch((extension>>10)&0x0001)
			{
				case 0: // 32 bit to Di
					cpu.dregs.d[(extension>>12)&0x0007]=
					 (unsigned long)cpu.dregs.d[(extension>>12)&0x0007]*
						(unsigned long)work.source;
					if((unsigned long)cpu.dregs.d[(extension>>12)&0x0007]<
						(unsigned short)work.source)OVER1;
					else OVER0;
					break;
				case 1: // 64 bit in Di:Dh
					Unknown(opcode);
					break;
			}
			break;
		case 1: // MULS
			switch((extension>>10)&0x0001)
			{
				case 0: // 32 bit to Di
					cpu.dregs.d[(extension>>12)&0x0007]=
						(long)cpu.dregs.d[(extension>>12)&0x0007]*
						 (long)work.source;
					if((short)cpu.dregs.d[(extension>>12)&0x0007]<(short)work.source)OVER1;
					else OVER0;
					break;
				case 1: // 64 bit in Di:Dh
					Unknown(opcode);
					break;
			}
		break;
	}
	if(cpu.dregs.d[(extension>>12)&0x0007]==0)ZERO1;
	else ZERO0;
	if(cpu.dregs.d[(extension>>12)&0x0007]<0)NEG1;
	else NEG0;
	CARRY0;

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move multiple to registers
////////////////////////////////////////////////////////////////////////////////

void COM_movemtoreg(short opcode)
{
static unsigned short offset,reglist,size,i;

		CACHEFUNCTION(COM_movemtoreg);
		cpu.pc+=4;
		reglist=GETword(cpu.pc-2);
		switch(of.general.modesrc)
		{
			case 2: // ARI //
				work.destination=cpu.aregs.a[of.general.regsrc];
				offset=0;
				size=(opcode>>6)&0x0001;
				for(i=0;i<8;i++)
				{
					if(reglist&0x0001)
					{
						if(size)cpu.dregs.d[i]=GETdword(work.destination+4*offset);
						else cpu.dregs.d[i]=GETword(work.destination+2*offset);
						offset++;
					}
					reglist=reglist>>1;
				}
				for(i=0;i<8;i++)
				{
					if(reglist&0x0001)
					{
						if(size)cpu.aregs.a[i]=GETdword(work.destination+4*offset);
						else cpu.aregs.a[i]=GETword(work.destination+2*offset);
						offset++;
					}
					reglist=reglist>>1;
				}
				break;

			case 3: // ARIPI //
				size=(opcode>>6)&0x0001;
				for(i=0;i<8;i++)
				{
					if(reglist&0x0001)
					{
						if(size)
						{
							work.destination=cpu.aregs.a[of.general.regsrc];
							cpu.dregs.d[i]=GETdword(work.destination);
							cpu.aregs.a[of.general.regsrc]+=4;
						}
						else
						{
							work.destination=cpu.aregs.a[of.general.regsrc];
							cpu.dregs.d[i]=GETword(work.destination);
							cpu.aregs.a[of.general.regsrc]+=2;
						}
					}
					reglist=reglist>>1;
				}
				for(i=0;i<8;i++)
				{
					if(reglist&0x0001)
					{
						if(size)
						{
							work.destination=cpu.aregs.a[of.general.regsrc];
							cpu.aregs.a[i]=GETdword(work.destination);
							cpu.aregs.a[of.general.regsrc]+=4;
						}
						else
						{
							work.destination=cpu.aregs.a[of.general.regsrc];
							cpu.aregs.a[i]=GETword(work.destination);
							cpu.aregs.a[of.general.regsrc]+=2;
						}
					}
					reglist=reglist>>1;
				}
				break;
			case 5: // ARID //
				work.destination=cpu.aregs.a[of.general.regsrc]+(long)GETword(cpu.pc);
				cpu.pc+=2;
				offset=0;
				size=(opcode>>6)&0x0001;
				for(i=0;i<8;i++)
				{
					if(reglist&0x0001)
					{
						if(size)cpu.dregs.d[i]=GETdword(work.destination+4*offset);
						else cpu.dregs.d[i]=GETword(work.destination+2*offset);
						offset++;
					}
					reglist=reglist>>1;
				}
				for(i=0;i<8;i++)
				{
					if(reglist&0x0001)
					{
						if(size)cpu.aregs.a[i]=GETdword(work.destination+4*offset);
						else cpu.aregs.a[i]=GETword(work.destination+2*offset);
						offset++;
					}
					reglist=reglist>>1;
				}
				break;

			case 7:
				switch(of.general.regsrc)
				{
					case 0:
						work.destination=(unsigned long)GETword(cpu.pc);
						offset=0;
						size=(opcode>>6)&0x0001;
						cpu.pc+=2;
						for(i=0;i<8;i++)
						{
							if(reglist&0x0001)
							{
								if(size)cpu.dregs.d[i]=GETdword(work.destination+4*offset);
								else cpu.dregs.d[i]=(cpu.dregs.d[i]&0xffff0000L)|
											GETword(work.destination+2*offset);
								offset++;
							}
							reglist=reglist>>1;
						}
						for(i=0;i<8;i++)
						{
							if(reglist&0x0001)
							{
								if(size)cpu.aregs.a[i]=GETdword(work.destination+4*offset);
								else cpu.aregs.a[i]=(cpu.aregs.a[i]&0xFFFF0000L)|
									GETword(work.destination+2*offset);
								offset++;
							}
							reglist=reglist>>1;
						}
						break;
					case 1:
						work.destination=GETdword(cpu.pc);
						offset=0;
						size=(opcode>>6)&0x0001;
						cpu.pc+=4;
						for(i=0;i<8;i++)
						{
							if(reglist&0x0001)
							{
								if(size)cpu.dregs.d[i]=GETdword(work.destination+4*offset);
								else cpu.dregs.d[i]=(cpu.dregs.d[i]&0xFFFF0000L)|
										GETword(work.destination+2*offset);
								offset++;
							}
							reglist=reglist>>1;
						}
						for(i=0;i<8;i++)
						{
							if(reglist&0x0001)
							{
								if(size)cpu.aregs.a[i]=GETdword(work.destination+4*offset);
								else cpu.aregs.a[i]=(cpu.aregs.a[i]&0xFFFF0000L)|
									GETword(work.destination+2*offset);
								offset++;
							}
							reglist=reglist>>1;
						}
						break;
				}
				break;
			default:
				Unknown(opcode);
				break;
			}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: jump to subroutine
////////////////////////////////////////////////////////////////////////////////

void COM_jsr(short opcode)
{
short extension;
long ea,od,bd,index;

	CACHEFUNCTION(COM_jsr);
	cpu.pc+=2;
	switch(of.general.modesrc)
	{
		case 2: // ARI
			cpu.aregs.a[7]-=4; // decrement stack pointer
			PUTdword(cpu.aregs.a[7],cpu.pc); // save return address on stack
			cpu.pc=cpu.aregs.a[(of.general.regsrc)]; // set new PC
			break;
		case 5: // ARID
			cpu.aregs.a[7]-=4; // decrement stack pointer
			PUTdword(cpu.aregs.a[7],cpu.pc); // save return address on stack
			cpu.pc+=(long)GETword(cpu.pc);
			break;
		case 6: // ARII
			cpu.aregs.a[7]-=4; // decrement stack pointer
			extension=GETword(cpu.pc); // get extension word
			cpu.pc+=2; // increment PC
			switch((extension&0x0100)>>8) // 68000 or 68020 extension
			{
				case 0: // brief format extension
					if(extension&0x8000) // (d8,Asrc,An.x*scale)
					{
						if(extension&0x0800) // long word
						{
							cpu.pc=cpu.aregs.a[(of.general.regsrc)]+
							 (cpu.aregs.a[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
						else
						{
							cpu.pc=cpu.aregs.a[(of.general.regsrc)]+
							 (long)((short)cpu.aregs.a[(extension>>12)&0x0007])*
							  (1<<((extension>>9)&0x0003));
						}
					}
					else // Dn
					{
						if(extension&0x0800) // long word
						{
							cpu.pc=cpu.aregs.a[(of.general.regsrc)]+
							 (cpu.dregs.d[(extension>>12)&0x0007])*
							  (1<<((extension>>9)&0x0003));
						}
						else
						{
							cpu.pc=cpu.aregs.a[(of.general.regsrc)]+
							 (long)((short)cpu.dregs.d[(extension>>12)&0x0007])*
							  (1<<((extension>>9)&0x0003));
						}
					}
					cpu.pc+=(long)(extension&0x00FF);
					break;
				case 1: // long format extension
					if(!(extension&0x0080)) // Base is not suppressed
					{
						ea=cpu.aregs.a[(of.general.regsrc)];
					}
					else ea=0;

					switch(extension&0x8800)
					{
						case 0x0000: // Dn.W
							index=(long)(short)cpu.dregs.d[(extension>>12)&0x0007];
							break;
						case 0x0800: // Dn.L
							index=cpu.dregs.d[(extension>>12)&0x0007];
							break;
						case 0x8000: // An.W
							index=(long)(short)cpu.aregs.a[(extension>>12)&0x0007];
							break;
						case 0x8800: // An.L
							index=cpu.aregs.a[(extension>>12)&0x0007];
							break;
					}

					// scale index
					switch((extension>>9)&0x0003)
					{
						case 0:
							break;
						case 1:
							index*=2;
							break;
						case 2:
							index*=4;
							break;
						case 3:
							index*=8;
							break;
					}

					switch((extension>>4)&0x0003) // BD size
					{
						case 0: // reserved
						case 1: // null displacement
							bd=0;
							break;
						case 2: // word displacement
							bd=(long)(short)GETword(cpu.pc);
							cpu.pc+=2;
							break;
						case 3: // long displacement
							bd=(long)GETdword(cpu.pc);
							cpu.pc+=4;
							break;
					}
					if(!(extension&0x0040)) // Index is not suppressed
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								ea=ea+index+bd;
								break;
							case 1: // indirect pre-indexed with null displacement
								ea=GETdword(ea+bd+index);
								break;
							case 2: // indirect pre-indexed with word displacement
								od=(long)GETword(cpu.pc);
								ea=GETdword(ea+index+bd)+od;
								break;
							case 3: // indirect pre-indexed with long displacement
								od=GETdword(cpu.pc);
								ea=GETdword(ea+index+bd)+od;
								break;
							case 4: // reserved
								break;
							case 5: // indirect post-indexed with null displacement
								ea=GETdword(ea+bd)+index;
								break;
							case 6: // indirect post-indexed with word displacement
								od=(long)GETword(cpu.pc);
								ea=GETdword(ea+bd)+index+od;
								break;
							case 7: // indirect post-indexed with long displacement
								od=GETdword(cpu.pc);
								ea=GETdword(ea+bd)+index+od;
								break;
						}
					}
					else
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								ea=ea+bd;
								break;
							case 1: // memory indirect with null displacement
								ea=GETdword(ea+bd);
								break;
							case 2: // memory indirect with word displacement
								od=(long)GETword(cpu.pc);
								ea=GETdword(ea+bd)+od;
								break;
							case 3: // memory indirect with long displacement
								od=GETdword(cpu.pc);
								ea=GETdword(ea+bd)+od;
								break;
							case 4: // reserved
							case 5:
							case 6:
							case 7:
								break;
						}
					}
					break;
			}
			PUTdword(cpu.aregs.a[7],cpu.pc); // save return address on stack
			cpu.pc=ea; // set new PC
			break;
		case 7: // MISC
			switch(of.general.regsrc)
			{
				case 0: // (XXX).w
					cpu.aregs.a[7]-=4;
					PUTdword(cpu.aregs.a[7],cpu.pc+2);
					cpu.pc=(long)GETword(cpu.pc);
					break;
				case 1: // (XXX).l
					cpu.aregs.a[7]-=4;
					PUTdword(cpu.aregs.a[7],cpu.pc+4);
					cpu.pc=GETdword(cpu.pc);
					break;
				case 2: // (d16,PC)
					cpu.aregs.a[7]-=4;
					PUTdword(cpu.aregs.a[7],cpu.pc+2);
					cpu.pc=cpu.pc+(long)GETword(cpu.pc);
					break;
			}
			break;
		default:
			Unknown(opcode);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: jump to address
////////////////////////////////////////////////////////////////////////////////

void COM_jmp(short opcode)
{
	CACHEFUNCTION(COM_jmp);
	cpu.pc+=2;
	switch(of.general.modesrc)
	{
		case 2:
			cpu.pc=cpu.aregs.a[(of.general.regsrc)];
			break;
		case 5:
			cpu.pc=cpu.aregs.a[(of.general.regsrc)]+(long)GETword(cpu.pc);
			break;
		case 7:
			switch(of.general.regsrc)
			{
				case 0:
					cpu.pc=(long)GETword(cpu.pc);
					break;
				case 1:
					cpu.pc=GETdword(cpu.pc);
					break;
				case 2:
					cpu.pc=cpu.pc+(long)GETword(cpu.pc);
					cpu.pc+=2;
					break;
			}
			break;
		default:
			Unknown(opcode);
			break;
	}

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: link and allocate
////////////////////////////////////////////////////////////////////////////////

void COM_link(short opcode)
{
	CACHEFUNCTION(COM_link);
	cpu.pc+=2;
	cpu.aregs.a[7]-=4;
	PUTdword(cpu.aregs.a[7],cpu.aregs.a[of.general.regsrc]);
	cpu.aregs.a[of.general.regsrc]=cpu.aregs.a[7];
	cpu.aregs.a[7]=cpu.aregs.a[7]+(long)GETword(cpu.pc);
	cpu.pc+=2;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: trap
////////////////////////////////////////////////////////////////////////////////

void COM_trap(short opcode)
{
static short foofs;

	CACHEFUNCTION(COM_trap);
	cpu.pc+=2; // increment PC
	// CPU in which mode?
	if((cpu.sregs.sr&0x3000)==0)
	{
		cpu.usp=cpu.aregs.a[7]; // CPU in user mode
	}
	else
	{
		cpu.ssp=cpu.aregs.a[7]; // CPU in supervisor mode
	}
	foofs=(opcode&0x000f)*4+0x0080;
	cpu.ssp-=2;                     // push format/offset on stack
	PUTword(cpu.ssp,foofs);
	cpu.ssp-=4;                    // push PC on stack
	PUTdword(cpu.ssp,cpu.pc);
	cpu.ssp-=2;                    // push SR on stack
	PUTword(cpu.ssp,cpu.sregs.sr);
	// get new PC from vector
	cpu.pc=GETdword((long)(foofs&0x0FFF)+cpu.vbr)&0x00FFFFFFL;
	cpu.aregs.a[7]=cpu.ssp; // switch to supervisor stack
	cpu.sregs.sr=(cpu.sregs.sr|0x2000)&0x3fff; // switch CPU to supervisor mode
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: unlink and deallocate
////////////////////////////////////////////////////////////////////////////////

void COM_unlink(short opcode)
{
	CACHEFUNCTION(COM_unlink);
	cpu.pc+=2;
	cpu.aregs.a[7]=cpu.aregs.a[of.general.regsrc];
	cpu.aregs.a[of.general.regsrc]=GETdword(cpu.aregs.a[7]);
	cpu.aregs.a[7]+=4;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move to user stack pointer
////////////////////////////////////////////////////////////////////////////////
void COM_moveUSP(short opcode)
{
	CACHEFUNCTION(COM_moveUSP);
	// test if in supervisor modes
	if(cpu.sregs.sr&0x3000)
	{
		cpu.pc+=2;
		if(opcode&0x0008) // MOVE.l USP,An //
		{
			cpu.ssp=cpu.aregs.a[of.general.regsrc]=cpu.usp;
		}
		else // MOVE.l An,USP //
		{
			cpu.usp=cpu.aregs.a[of.general.regsrc];
		}
	}
	else priv_viol();
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: reset peripherals (priviledged instruction)
////////////////////////////////////////////////////////////////////////////////

void COM_reset(short opcode)
{
	CACHEFUNCTION(COM_reset);
	// test if in supervisor modes
	if(cpu.sregs.sr&0x3000)
	{
	int index;
		// call all plugin reset functions
		for(index=0;pDllHdr[index]!=0;index++)pDllHdr[index]->ResetProc();
		cpu.pc+=2; // increment PC
	}
	else priv_viol();
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: No OPeration
////////////////////////////////////////////////////////////////////////////////

void COM_nop(short opcode)
{
	CACHEFUNCTION(COM_nop);
	cpu.pc+=2;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: load SR and stop CPU
////////////////////////////////////////////////////////////////////////////////

void COM_stop(short opcode)
{
	CACHEFUNCTION(COM_stop);
	// test if in supervisor modes
	if(cpu.sregs.sr&0x3000) // test if in supervisor mode
	{
		cpu.pc+=4; // increment PC
		cpu.sregs.sr=GETword(cpu.pc-2); // load SR from immediate data
		bStopped=TRUE; // stop CPU
	}
	else priv_viol();
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: return from exception
////////////////////////////////////////////////////////////////////////////////

void COM_rte(short opcode)
{
	CACHEFUNCTION(COM_rte);
	// test if in supervisor modes
	if(cpu.sregs.sr&0x3000) // in supervisor mode?
	{
		cpu.sregs.sr=GETword(cpu.ssp);  // fetch SR from stack
		cpu.pc=GETdword(cpu.ssp+2);     // fetch PC from stack
		switch(GETword(cpu.ssp+6)&0xF000)  // test stack frame format
		{
			case 0:
				cpu.ssp+=8;
				break;
			case 0x2000:
				cpu.ssp+=12;
				break;
		}
		if(cpu.sregs.sr&0x3000)cpu.aregs.a[7]=cpu.ssp;
		else cpu.aregs.a[7]=cpu.usp;
	}
	else priv_viol();
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: return from subroutine
////////////////////////////////////////////////////////////////////////////////

void COM_rts(short opcode)
{
	CACHEFUNCTION(COM_rts);
	cpu.pc=GETdword(cpu.aregs.a[7]); // get PC from stack
	cpu.aregs.a[7]+=4;      // increment stack pointer
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: trap on overflow
////////////////////////////////////////////////////////////////////////////////

void COM_trapv(short opcode)
{
	Unknown(opcode);
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: return and restore
////////////////////////////////////////////////////////////////////////////////

void COM_rtr(short opcode)
{
	CACHEFUNCTION(COM_rtr);
	cpu.sregs.ccr=(char)GETword(cpu.aregs.a[7]);
	cpu.aregs.a[7]+=2;
	cpu.pc=GETdword(cpu.aregs.a[7]);
	cpu.aregs.a[7]+=4;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move special
////////////////////////////////////////////////////////////////////////////////

void COM_movec(short opcode)
{
short select,reg;

	CACHEFUNCTION(COM_movec);
	cpu.pc+=2;
	select=GETword(cpu.pc); // get the selection fields
	reg=(select>>12)&0x0007;
	cpu.pc+=2;
	switch(opcode&0x0001) // direction
	{
		case 0: // control register to general register
			switch((select>>15)&0x0001) // address or data reg
			{
				case 0: // data reg
					switch(select&0x0fff) // control reg
					{
						case 0: // cpu.sfc
							cpu.dregs.d[reg]=cpu.sfc;
							break;
						case 1: // cpu.dfc
							cpu.dregs.d[reg]=cpu.dfc;
							break;
						case 2: // cpu.cacr
							cpu.dregs.d[reg]=cpu.cacr;
							break;
						case 0x800: // cpu.usp
							cpu.dregs.d[reg]=cpu.usp;
							break;
						case 0x801: // cpu.vbr
							cpu.dregs.d[reg]=cpu.vbr;
							break;
						case 0x802: // cpu.cacr
							cpu.dregs.d[reg]=cpu.cacr;
							break;
						case 0x803: // cpu.msp
							cpu.dregs.d[reg]=cpu.msp;
							break;
						case 0x804: // isp
							cpu.dregs.d[reg]=cpu.ssp;
							break;
					}
					break;
				case 1: // address reg
					switch(select&0x0fff) // control reg
					{
						case 0: // cpu.sfc
							cpu.aregs.a[reg]=cpu.sfc;
							break;
						case 1: // cpu.dfc
							cpu.aregs.a[reg]=cpu.dfc;
							break;
						case 2: // cpu.cacr
							cpu.aregs.a[reg]=cpu.cacr;
							break;
						case 0x800: // cpu.usp
							cpu.aregs.a[reg]=cpu.usp;
							break;
						case 0x801: // cpu.vbr
							cpu.aregs.a[reg]=cpu.vbr;
							break;
						case 0x802: // cpu.cacr
							cpu.aregs.a[reg]=cpu.cacr;
							break;
						case 0x803: // cpu.msp
							cpu.aregs.a[reg]=cpu.msp;
							break;
						case 0x804: // isp
							cpu.aregs.a[reg]=cpu.ssp;
							break;
					}
					break;
			}
			break;
		case 1: // general register to control register
			switch((select>>15)&0x0001) // address or data reg
			{
				case 0: // data reg
					switch(select&0x0fff) // control reg
					{
						case 0: // cpu.sfc
							cpu.sfc=cpu.dregs.d[reg];
							break;
						case 1: // cpu.dfc
							cpu.dfc=cpu.dregs.d[reg];
							break;
						case 2: // cpu.cacr
							cpu.cacr=cpu.dregs.d[reg];
							break;
						case 0x800: // cpu.usp
							cpu.usp=cpu.dregs.d[reg];
							break;
						case 0x801: // cpu.vbr
							cpu.vbr=cpu.dregs.d[reg];
							break;
						case 0x802: // cpu.cacr
							cpu.cacr=cpu.dregs.d[reg];
							break;
						case 0x803: // cpu.msp
							cpu.msp=cpu.dregs.d[reg];
							break;
						case 0x804: // isp
							cpu.ssp=cpu.dregs.d[reg];
							break;
					}
					break;
				case 1: // address reg
					switch(select&0x0fff) // control reg
					{
						case 0: // cpu.sfc
							cpu.sfc=cpu.aregs.a[reg];
							break;
						case 1: // cpu.dfc
							cpu.dfc=cpu.aregs.a[reg];
							break;
						case 2: // cpu.cacr
							cpu.cacr=cpu.aregs.a[reg];
							break;
						case 0x800: // cpu.usp
							cpu.usp=cpu.aregs.a[reg];
							break;
						case 0x801: // cpu.vbr
							cpu.vbr=cpu.aregs.a[reg];
							break;
						case 0x802: // cpu.cacr
							cpu.cacr=cpu.aregs.a[reg];
							break;
						case 0x803: // cpu.msp
							cpu.msp=cpu.aregs.a[reg];
							break;
						case 0x804: // isp
							cpu.ssp=cpu.aregs.a[reg];
							break;
					}
					break;
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: BRanch Always
////////////////////////////////////////////////////////////////////////////////

void COM_bra(short opcode)
{
	CACHEFUNCTION(COM_bra);
	switch(opcode&0x00ff)
	{
		case 0:
			cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			break;
		case 0xFF:
			cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			break;
		default:
			cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch to SubRoutine
////////////////////////////////////////////////////////////////////////////////

void COM_bsr(short opcode)
{
	CACHEFUNCTION(COM_bsr);
	switch(opcode&0x00ff)
	{
		case 0:
			cpu.aregs.a[7]-=4;
			PUTdword(cpu.aregs.a[7],cpu.pc+4);
			cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			break;
		case 0xFF:
			cpu.aregs.a[7]-=4;
			PUTdword(cpu.aregs.a[7],cpu.pc+6);
			cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			break;
		default:
			cpu.aregs.a[7]-=4;
			PUTdword(cpu.aregs.a[7],cpu.pc+2);
			cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if HIgher
////////////////////////////////////////////////////////////////////////////////

void COM_bhi(short opcode)
{
	CACHEFUNCTION(COM_bhi);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x05) == 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x05) == 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x05) == 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Lower or Same
////////////////////////////////////////////////////////////////////////////////

void COM_bls(short opcode)
{
	CACHEFUNCTION(COM_bls);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x05) != 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x05) != 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x05) != 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Carry Set
////////////////////////////////////////////////////////////////////////////////

void COM_bcs(short opcode)
{
	CACHEFUNCTION(COM_bcs);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x01) != 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x01) != 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x01) != 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Carry Clear
////////////////////////////////////////////////////////////////////////////////

void COM_bcc(short opcode)
{
	CACHEFUNCTION(COM_bcc);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x01) == 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x01) == 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x01) == 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Not Equal
////////////////////////////////////////////////////////////////////////////////

void COM_bne(short opcode)
{
	CACHEFUNCTION(COM_bne);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x04) == 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x04) == 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x04) == 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if EQual
////////////////////////////////////////////////////////////////////////////////

void COM_beq(short opcode)
{
	CACHEFUNCTION(COM_beq);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x04) != 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x04) != 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x04) != 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if oVerflow Clear
////////////////////////////////////////////////////////////////////////////////

void COM_bvc(short opcode)
{
	CACHEFUNCTION(COM_bvc);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x02) == 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x02) == 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x02) == 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if oVerflow Set
////////////////////////////////////////////////////////////////////////////////

void COM_bvs(short opcode)
{
	CACHEFUNCTION(COM_bvs);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x02) != 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x02) != 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x02) != 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if PLus
////////////////////////////////////////////////////////////////////////////////

void COM_bpl(short opcode)
{
	CACHEFUNCTION(COM_bpl);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x08) == 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x08) == 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x08) == 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if MInus
////////////////////////////////////////////////////////////////////////////////

void COM_bmi(short opcode)
{
	CACHEFUNCTION(COM_bmi);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x08) != 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x08) != 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x08) != 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Greater Or Equal
////////////////////////////////////////////////////////////////////////////////

void COM_bge(short opcode)
{
	CACHEFUNCTION(COM_bge);
	switch(opcode&0x00ff)
	{
		case 0:
			if( ((cpu.sregs.ccr&0x0A)==0) || ((cpu.sregs.ccr&0x0A)==0x0A))
				cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( ((cpu.sregs.ccr&0x0A)==0) || ((cpu.sregs.ccr&0x0A)==0x0A))
				cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( ((cpu.sregs.ccr&0x0A)==0) || ((cpu.sregs.ccr&0x0A)==0x0A))
				cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Lower Than
////////////////////////////////////////////////////////////////////////////////

void COM_blt(short opcode)
{
	CACHEFUNCTION(COM_blt);
	switch(opcode&0x00ff)
	{
		case 0:
			if( ((cpu.sregs.ccr&0x0A)==0x08) || ((cpu.sregs.ccr&0x0A)==0x02))
				cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( ((cpu.sregs.ccr&0x0A)==0x08) || ((cpu.sregs.ccr&0x0A)==0x02))
				cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( ((cpu.sregs.ccr&0x0A)==0x08) || ((cpu.sregs.ccr&0x0A)==0x02))
				cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Greater Than
////////////////////////////////////////////////////////////////////////////////

void COM_bgt(short opcode)
{
	CACHEFUNCTION(COM_bgt);
	switch(opcode&0x00ff)
	{
		case 0:
			if( ((cpu.sregs.ccr&0x0e)==0x0A) || ((cpu.sregs.ccr&0x0e)==0))
				cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( ((cpu.sregs.ccr&0x0e)==0x0A) || ((cpu.sregs.ccr&0x0e)==0))
				cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( ((cpu.sregs.ccr&0x0e)==0x0A) || ((cpu.sregs.ccr&0x0e)==0))
				cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch if Lower or Equal
////////////////////////////////////////////////////////////////////////////////

void COM_ble(short opcode)
{
	CACHEFUNCTION(COM_ble);
	// if (Z || N&&!V || !N&&V) is TRUE
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.ccr&0x04)||
				 ((cpu.sregs.ccr&0x0a)==0x08)||
				 ((cpu.sregs.ccr&0x0a)==0x02))cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.ccr&0x04)||
				 ((cpu.sregs.ccr&0x0a)==0x08)||
				 ((cpu.sregs.ccr&0x0a)==0x02))cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.ccr&0x04)||
			 ((cpu.sregs.ccr&0x0a)==0x08) ||
			 ((cpu.sregs.ccr&0x0a)==0x02))cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Branch (false) / Branch never
////////////////////////////////////////////////////////////////////////////////

void COM_bf(short opcode)
{
	CACHEFUNCTION(COM_bf);
	switch(opcode&0x00ff)
	{
		case 0:
			cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			cpu.pc=cpu.pc+6;
			break;
		default:
			cpu.pc=cpu.pc+2;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: decrement and branch on condition
////////////////////////////////////////////////////////////////////////////////

void COM_dbcc(short opcode)
{
void (*BranchType[16])(short)={COM_bf,COM_bra,
				COM_bhi,COM_bls,COM_bcc,COM_bcs,
				COM_bne,COM_beq,COM_bvc,COM_bvs,
				COM_bpl,COM_bmi,COM_bge,COM_blt,
						COM_bgt,COM_ble};

	CACHEFUNCTION(COM_dbcc);
	cpu.dregs.wordd[of.general.regsrc].dl--;
	if(cpu.dregs.wordd[of.general.regsrc].dl!=-1)
		BranchType[((opcode>>8)&0x000F)](0);
	else cpu.pc=cpu.pc+4;
}

//////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Set conditionally
//////////////////////////////////////////////////////////////////////////////////

void COM_scc(short opcode)
{
	CACHEFUNCTION(COM_scc);
	switch((opcode&0x0f00)>>8)
	{
		case 0: // st //
			cpu.pc+=2;
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,0xffffffffL,0);
			break;
		case 1:  // sf //
			cpu.pc+=2;
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,0,0);
			break;
		case 6: // sne //
			cpu.pc+=2;
			if(cpu.sregs.ccr&0x04)
				CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,0xFFFFFFFFL,0);
			else CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,0,0);
			break;
		case 7: // sne //
			cpu.pc+=2;
			if(!(cpu.sregs.ccr&0x04))
				CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,0xFFFFFFFFL,0);
			else CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,0,0);
			break;
		default:
			Unknown(opcode);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: add quick
////////////////////////////////////////////////////////////////////////////////

void COM_addq(short opcode)
{
static char data,size;

	CACHEFUNCTION(COM_addq);
	cpu.pc+=2;
	size=(opcode>>6)&0x03;
	if((opcode&0x0E00)==0)data=8;
	else data=(of.general.regdest);
	spc=cpu.pc;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,size);
	cpu.pc=spc;
	if(size==0)work.result=(work.source&0xFFFFFF00L)|((char)work.source+data);
	else if(size==1)work.result=work.source+((unsigned short)data);
	else work.result=work.source+(unsigned long)data;
	CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.result,size);
	switch(size)
	{
		case 0:
			setcarry( gen_carry((char)work.source,(char)work.destination,(char)work.result) );
			setxtend( gen_carry((char)work.source,(char)work.destination,(char)work.result) );
			setover( gen_over((char)work.source,(char)work.destination,(char)work.result) );
			if((char)work.source==0)ZERO1;
			else ZERO0;
			if((char)work.source<0)NEG1;
			else NEG0;
			break;
		case 1:
			setcarry( gen_carry((short)work.source,(short)work.destination,(short)work.result) );
			setxtend( gen_carry((short)work.source,(short)work.destination,(short)work.result) );
			setover( gen_over((short)work.source,(short)work.destination,(short)work.result) );
			if((short)work.source==0)ZERO1;
			else ZERO0;
			if((short)work.source<0)NEG1;
			else NEG0;
			break;
		case 2:
			setcarry( gen_carry(work.source,work.destination,work.result) );
			setxtend( gen_carry(work.source,work.destination,work.result) );
			setover( gen_over(work.source,work.destination,work.result) );
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: subtract quick
////////////////////////////////////////////////////////////////////////////////

void COM_subq(short opcode)
{
static char data,size;

	CACHEFUNCTION(COM_subq);
	cpu.pc+=2;
	size=(opcode>>6)&0x03;
	if((opcode&0x0E00)==0)data=8;
	else data=(of.general.regdest);
	spc=cpu.pc;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,size);
	cpu.pc=spc;
	if(size==0)work.source=(work.source&0xFFFFFF00L)|((char)work.source-data);
	else if(size==1)work.source=(work.source&0xFFFF0000L)|((short)work.source-data);
	else work.source=work.source-(long)data;
	CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,size);
	switch(size)
	{
		case 0:
			setcarry( gen_carry((char)work.source,(char)work.destination,(char)work.result) );
			setxtend( gen_carry((char)work.source,(char)work.destination,(char)work.result) );
			setover( gen_over((char)work.source,(char)work.destination,(char)work.result) );
			if((char)work.source==0)ZERO1;
			else ZERO0;
			if((char)work.source<0)NEG1;
			else NEG0;
			break;
		case 1:
			setcarry( gen_carry((short)work.source,(short)work.destination,(short)work.result) );
			setxtend( gen_carry((short)work.source,(short)work.destination,(short)work.result) );
			setover( gen_over((short)work.source,(short)work.destination,(short)work.result) );
			if((short)work.source==0)ZERO1;
			else ZERO0;
			if((short)work.source<0)NEG1;
			else NEG0;
			break;
		case 2:
			setcarry( gen_carry(work.source,work.destination,work.result) );
			setxtend( gen_carry(work.source,work.destination,work.result) );
			setover( gen_over(work.source,work.destination,work.result) );
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: subtract
//////////////////////////////////////////////////////////////////////////////////

void COM_sub(short opcode)
{
	CACHEFUNCTION(COM_sub);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 0:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]-work.source;
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 1:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]-work.source;
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 2:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]-work.source;
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 3:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.aregs.a[of.general.regdest]=cpu.aregs.a[of.general.regdest]-work.source;
			if(cpu.aregs.a[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.aregs.a[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 4:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.pc=spc;
			work.source=work.source-cpu.dregs.d[of.general.regdest];
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,0);
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
		case 5:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.pc=spc;
			work.source=work.source-cpu.dregs.d[of.general.regdest];
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,1);
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
		case 6:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.pc=spc;
			work.source=work.source-cpu.dregs.d[of.general.regdest];
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,2);
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
		case 7:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.aregs.a[of.general.regdest]=cpu.aregs.a[of.general.regdest]-work.source;
			if(cpu.aregs.a[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.aregs.a[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: add
////////////////////////////////////////////////////////////////////////////////

void COM_add(short opcode)
{
	CACHEFUNCTION(COM_add);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 0:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.dregs.d[of.general.regdest]=(cpu.dregs.d[of.general.regdest])+
												(unsigned char)(work.source&0x000000ffL);
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 1:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.dregs.d[of.general.regdest]=
				(cpu.dregs.d[of.general.regdest]&0xFFFF0000L)|
					(((cpu.dregs.d[of.general.regdest])+
						(unsigned short)(work.source&0x0000ffff))&0x0000FFFFL);
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 2:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]+work.source;
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 3:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.aregs.a[of.general.regdest]=cpu.aregs.a[of.general.regdest]+
							(work.source&0x0000FFFFL);
			if(cpu.aregs.a[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.aregs.a[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 4:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.pc=spc;
			work.source=(work.source)+(cpu.dregs.d[of.general.regdest]&0x000000ffL);
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,0);
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
		case 5:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.pc=spc;
			work.source=(work.source)+(cpu.dregs.d[of.general.regdest]&0x0000FFFFL);
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,1);
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
		case 6:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.pc=spc;
			work.source=work.source+cpu.dregs.d[of.general.regdest];
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,2);
			if(work.source==0)ZERO1;
			else ZERO0;
			if(work.source<0)NEG1;
			else NEG0;
			break;
		case 7:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.aregs.a[of.general.regdest]=cpu.aregs.a[of.general.regdest]+work.source;
			if(cpu.aregs.a[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.aregs.a[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
	}

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: arithmetic shift
////////////////////////////////////////////////////////////////////////////////

void COM_asx(short opcode)
{
static short i;
static short shiftcount;
static char btemp,bmsb;
static short itemp,imsb;
static long ltemp,lmsb;


	CACHEFUNCTION(COM_asx);
	if((opcode&0x0ec0)==0x00c0)	// memory //
	{
		switch((opcode>>8)&0x0001)
		{
			case 0: // ASR mem
				cpu.pc+=2;
				spc=cpu.pc;
				work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc)
																			,0,0L,1);
				cpu.pc=spc;
				setcarry(work.source&0x0001);
				setxtend(work.source&0x0001);
				itemp=work.source;
				work.source=(work.source>>1)|(itemp&0x8000);
				CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,
														work.source,1);
				break;
			case 1: // ASL mem
				cpu.pc+=2;
				spc=cpu.pc;
				work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
				cpu.pc=spc;
				setcarry((work.source>>15)&0x0001);
				setxtend((work.source>>15)&0x0001);
				work.source=(work.source<<1)&0xfffe;
				CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,1);
				break;

		}
	}
	else // regs or immediate //
	{
		cpu.pc+=2;
		if(opcode&0x0020)
		{
			shiftcount=cpu.dregs.d[of.general.regdest]%64;
		}
		else
		{
			if((of.general.regdest)==0)shiftcount=8;
			else shiftcount=(of.general.regdest);
		}
		switch(((opcode&0x0100)>>8)) // direction //
		{
			case 0:
				switch(of.special.size)
				{
					case 0:
						btemp=(char)cpu.dregs.d[of.general.regsrc];
						if(btemp<0)bmsb=0x80;
						else bmsb=0;
						for(i=0;i<shiftcount;i++)
						{
							if(btemp&0x01)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							btemp=(btemp>>1)|bmsb;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffffff00L)|
								(btemp&0x000000FFL);
						break;
					case 1:
						itemp=(short)cpu.dregs.d[of.general.regsrc];
						if(itemp&0x8000)imsb=0x8000;
						else imsb=0;
						for(i=0;i<shiftcount;i++)
						{
							if(itemp&0x0001)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							itemp=(itemp>>1)|imsb;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffff0000L)|
								(itemp&0x0000FFFFL);
						break;
					case 2:
						ltemp=cpu.dregs.d[of.general.regsrc];
						if(ltemp<0)lmsb=0x80000000L;
						else lmsb=0;
						for(i=0;i<shiftcount;i++)
						{
							if(cpu.dregs.d[of.general.regsrc]&0x00000001L)
								cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							cpu.dregs.d[of.general.regsrc]=
								(cpu.dregs.d[of.general.regsrc]>>1)|lmsb;
						}
						break;
				}
				break;
			case 1:
				switch(of.special.size)
				{
					case 0:
						btemp=(char)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(btemp&0x80)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							btemp=((btemp<<1)&0xFE);
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffffff00L)|
								(btemp&0x000000FFL);
						break;
					case 1:
						itemp=(short)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(itemp&0x8000)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							itemp=(itemp<<1)&0xFFFE;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffff0000L)|
								(itemp&0x0000FFFFL);
						break;
					case 2:
						for(i=0;i<shiftcount;i++)
						{
							if(cpu.dregs.d[of.general.regsrc]&0x80000000L)
								cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							cpu.dregs.d[of.general.regsrc]=
								(cpu.dregs.d[of.general.regsrc]<<1)&0xFFFFFFFE;
						}
						break;
				}
				break;
			}
	}

}
//////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: logical shift
//////////////////////////////////////////////////////////////////////////////////
void COM_lsx(short opcode)
{
static short i;
static short shiftcount;
static char btemp;
static short itemp;

	CACHEFUNCTION(COM_lsx);
	if((opcode&0x0ec0)==0x02c0)	// mem //
	{
		cpu.pc+=2;
		spc=cpu.pc;
		work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
		cpu.pc=spc;
		if((opcode&0x0100)) // left //
		{
			if(work.source&0x8000)
			{
				itemp=0x0001;
			}
			else itemp=0;
			work.source=(work.source<<1)&0xFFFE;
			if(itemp)
			{
				CARRY1;
				XTEND1;
			}
			else
			{
				CARRY0;
				XTEND0;
			}
		}
		else // right //
		{
			if(work.source&0x0001)itemp=0x8000;
			else itemp=0;
			work.source=(work.source>>1)&0x7FFF;
			if(itemp)
			{
				CARRY1;
				XTEND1;
			}
			else
			{
				CARRY0;
				XTEND0;
			}
		}
		if((short)work.source<0)NEG1;
		else NEG0;
		CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,1);

	}
	else // immediate or reg. //
	{
		cpu.pc+=2;
		if(opcode&0x0020)
		{
			shiftcount=cpu.dregs.d[of.general.regdest]%64;
		}
		else
		{
			if((of.general.regdest)==0)shiftcount=8;
			else shiftcount=(of.general.regdest);
		}
		switch(((opcode&0x0100)>>8)) // direction //
		{
			case 0:
				switch((opcode&0x00c0)>>6)
				{
					case 0:
						btemp=(char)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(btemp&0x01)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							btemp=(btemp>>1)&0x7f;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffffff00L)|btemp;
						break;
					case 1:
						itemp=(short)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(itemp&0x0001)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							itemp=(itemp>>1)&0x7fff;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffff0000L)|itemp;
						break;
					case 2:
						for(i=0;i<shiftcount;i++)
						{
							if(cpu.dregs.d[of.general.regsrc]&0x00000001L)
								cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							cpu.dregs.d[of.general.regsrc]=
								(cpu.dregs.d[of.general.regsrc]>>1)&0x7fffffffL;
						}
						break;
				}
				break;
			case 1:
				switch((opcode&0x00c0)>>6)
				{
					case 0:
						btemp=(char)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(btemp&0x80)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							btemp=(btemp<<1)&0xFE;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffffff00L)|btemp;
						break;
					case 1:
						itemp=(char)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(itemp&0x8000)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							itemp=(itemp<<1)&0xFFFE;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffff0000L)|itemp;
						break;
					case 2:
						for(i=0;i<shiftcount;i++)
						{
							if(cpu.dregs.d[of.general.regsrc]&0x80000000L)
								cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							cpu.dregs.d[of.general.regsrc]=
								(cpu.dregs.d[of.general.regsrc]<<1)&0xFFFFFFFEL;
						}
						break;
				}
				break;
			}
	}

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: rotate
////////////////////////////////////////////////////////////////////////////////
void COM_rx(short opcode)
{
	Unknown(opcode);
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: rotate
////////////////////////////////////////////////////////////////////////////////
void COM_r(short opcode)
{
static short i;
static short shiftcount;
static char btemp;
static short itemp;

	CACHEFUNCTION(COM_r);
	if( ((opcode>>6)&0x0003) ==0x0003 )	// mem //
	{
		cpu.pc+=2;
		spc=cpu.pc;
		work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
		cpu.pc=spc;
		if((opcode&0x0100)>>8) // left //
		{
			if(work.source&0x8000)
			{
				itemp=0x0001;
			}
			else itemp=0;
			work.source=(work.source<<1)|itemp;
			if(work.source&0x8000)CARRY1;
			else CARRY0;
		}
		else // right //
		{
			if(work.source&0x0001)itemp=0x8000;
			else itemp=0;
			work.source=(work.source>>1)|itemp;
			if(work.source&0x0001)CARRY1;
			else CARRY0;
		}
		if((short)work.source<0)NEG1;
		else NEG0;
		CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,1);
	}
	else // immediate or reg. //
	{
		cpu.pc+=2;
		if(opcode&0x0020)
		{
			shiftcount=cpu.dregs.d[of.general.regdest]%64;
		}
		else
		{
			if((of.general.regdest)==0)shiftcount=8;
			else shiftcount=(of.general.regdest);
		}
		switch(((opcode&0x0100)>>8)) // direction //
		{
			case 0:
				switch((opcode&0x00c0)>>6)
				{
					case 0:
						btemp=(char)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(btemp&0x01)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							btemp=(btemp>>1)&0x7f;
						}
						cpu.dregs.d[of.general.regsrc]=(cpu.dregs.d[of.general.regsrc]&
										0xffffff00L)|btemp;
						break;
					case 1:
						itemp=(short)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(itemp&0x0001)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							itemp=(itemp>>1)&0x7fff;
						}
						cpu.dregs.d[of.general.regsrc]=(cpu.dregs.d[of.general.regsrc]&
									0xffff0000L)|itemp;
						break;
					case 2:
						for(i=0;i<shiftcount;i++)
						{
							if(cpu.dregs.d[of.general.regsrc]&0x00000001L)
								cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							cpu.dregs.d[of.general.regsrc]=
								(cpu.dregs.d[of.general.regsrc]>>1)&0x7fffffffL;
						}
						break;
				}
				break;
			case 1:
				switch((opcode&0x00c0)>>6)
				{
					case 0:
						btemp=(char)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(btemp&0x80)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							btemp=(btemp<<1)&0xFE;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffffff00L)|btemp;
						break;
					case 1:
						itemp=(char)cpu.dregs.d[of.general.regsrc];
						for(i=0;i<shiftcount;i++)
						{
							if(itemp&0x8000)cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							itemp=(itemp<<1)&0xFFFE;
						}
						cpu.dregs.d[of.general.regsrc]=
							(cpu.dregs.d[of.general.regsrc]&0xffff0000L)|itemp;
						break;
					case 2:
						for(i=0;i<shiftcount;i++)
						{
							if(cpu.dregs.d[of.general.regsrc]&0x80000000L)
								cpu.sregs.ccr=cpu.sregs.ccr|0x11;
							else cpu.sregs.ccr=cpu.sregs.ccr&(~0x11);
							cpu.dregs.d[of.general.regsrc]=
								(cpu.dregs.d[of.general.regsrc]<<1)&0xFFFFFFFEL;
						}
						break;
				}
				break;
			}
	}
	OVER0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: multiply signed
////////////////////////////////////////////////////////////////////////////////
void COM_muls(short opcode)
{
static short regs;

	CACHEFUNCTION(COM_muls);
	cpu.pc+=2;
	if( (of.general.regdest) == 6)
	{
		regs=GETword(cpu.pc);
		cpu.pc+=2;
		work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
		if(opcode&0x0400)
		{
			Unknown(opcode);
		}
		else
		{
			cpu.dregs.d[(regs>>12)&0x0007]=
				(long)((long)cpu.dregs.d[(regs>>12)&0x0007]*(long)work.source);
		}
	}
	else // MULS.w //
	{
		work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
		cpu.dregs.d[of.general.regdest]=
			(long)((short)cpu.dregs.d[of.general.regdest]*(short)work.source);
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: multiply unsigned
////////////////////////////////////////////////////////////////////////////////
void COM_mulu(short opcode)
{
static short regs;

	CACHEFUNCTION(COM_mulu);
	cpu.pc+=2;
	if( (of.general.regdest) == 6) // MULU.l //
	{
		regs=GETword(cpu.pc);
		cpu.pc+=2;
		work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
		if(opcode&0x0400)
		{
			Unknown(opcode);
		}
		else
		{
			cpu.dregs.d[(regs>>12)&0x0007]=
				(unsigned long)((unsigned long)cpu.dregs.d[(regs>>12)&0x0007]*
					(unsigned long)work.source);
		}
	}
	else // MULU.w //
	{
		work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
		cpu.dregs.d[of.general.regdest]=
			(unsigned long)((unsigned short)cpu.dregs.d[of.general.regdest]*
				(unsigned short)work.source);
	}
}

//////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: add binary coded decimal
//////////////////////////////////////////////////////////////////////////////////
void COM_abcd(short opcode)
{
BYTE DecCarry=0;
BYTE Number1,Number2,Number;
BYTE ccr = cpu.sregs.ccr;
	cpu.pc+=2;
	switch(opcode&0x0008) // register or memory indirect predecrement
	{
		case 0x0000: // register
			Number1=cpu.dregs.byted[of.general.regdest].dll;
			Number2=cpu.dregs.byted[of.general.regsrc].dll;
			__asm{
				xor ax,ax		// ah will contain carry after
				mov al,byte ptr Number1 // get first number
				add al,byte ptr Number2 // add second to it
				daa            // decimal adjuse
				jnc nocarryd1  // carry?
				add ah,1
			nocarryd1:
				mov bl,byte ptr ccr // get flags
				shr bl,4       // isolate extend from there
				and bl,1
				add al,bl      // add to
				daa            // decimal adjust
				jnc nocarryd2  // another carry?
				add ah,1
			nocarryd2:
				mov byte ptr Number,al  // save back result
				mov byte ptr DecCarry,ah  // save carry
			}
			cpu.dregs.byted[of.general.regdest].dll=Number;
			break;
		case 0x0008: // memory indirect predecrement
			cpu.aregs.a[of.general.regsrc]-=1;
			cpu.aregs.a[of.general.regdest]-=1;
			Number1=GETbyte(cpu.aregs.a[of.general.regsrc]);
			Number2=GETbyte(cpu.aregs.a[of.general.regdest]);
			__asm{
				xor ax,ax		// ah will contain carry after
				mov al,byte ptr Number1 // get first number
				add al,byte ptr Number2 // add second to it
				daa            // decimal adjuse
				jnc nocarrya1  // carry?
				add ah,1
			nocarrya1:
				mov bl,byte ptr ccr // get flags
				shr bl,4       // isolate extend from there
				and bl,1
				add al,bl      // add to
				daa            // decimal adjust
				jnc nocarrya2  // another carry?
				add ah,1
			nocarrya2:
				mov byte ptr Number,al  // save back result
				mov byte ptr DecCarry,ah  // save carry
			}
			PUTbyte(cpu.aregs.a[of.general.regdest],Number);
			break;
	}
	if(Number)ZERO0;
	if(DecCarry)
   {
   	CARRY1;
      XTEND1;
   }
   else
   {
   	CARRY0;
      XTEND0;
   }
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: exchange
////////////////////////////////////////////////////////////////////////////////
void COM_exg(short opcode)
{
static long temp;

	CACHEFUNCTION(COM_exg);
	cpu.pc+=2;
	switch((opcode>>3)&0x001f)
	{
		case 0x08: // data regs //
			temp=cpu.dregs.d[of.general.regdest];
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regsrc];
			cpu.dregs.d[of.general.regsrc]=temp;
			break;
		case 0x09:
			temp=cpu.aregs.a[of.general.regdest];
			cpu.aregs.a[of.general.regdest]=cpu.aregs.a[of.general.regsrc];
			cpu.aregs.a[of.general.regsrc]=temp;
			break;
		case 0x11:
			temp=cpu.dregs.d[of.general.regdest];
			cpu.dregs.d[of.general.regdest]=cpu.aregs.a[of.general.regsrc];
			cpu.aregs.a[of.general.regsrc]=temp;
			break;
	}

}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: and
////////////////////////////////////////////////////////////////////////////////

void COM_and(short opcode)
{
	CACHEFUNCTION(COM_and);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 0:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.dregs.d[of.general.regdest]=
				(cpu.dregs.d[of.general.regdest])&
					(work.source|0xffffff00L);
			if((char)cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if((short)cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 1:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.dregs.d[of.general.regdest]=
				(cpu.dregs.d[of.general.regdest])&
					(work.source|0xffff0000L);
			if((short)cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if((short)cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 2:
			work.source=
				CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]&work.source;
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 4:
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			work.destination=(cpu.dregs.d[of.general.regdest])&(work.destination|0xffffff00L);
			cpu.pc=spc;
			work.destination=
				CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,
					work.destination,0);
			if((char)work.destination==0)ZERO1;
			else ZERO0;
			if((char)work.destination<0)NEG1;
			else NEG0;
			break;
		case 5:
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			work.destination=(cpu.dregs.d[of.general.regdest])&(work.destination|0xffff0000L);
			cpu.pc=spc;
			work.destination=
				CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,
					work.destination,1);
			if((short)work.destination==0)ZERO1;
			else ZERO0;
			if((short)work.destination<0)NEG1;
			else NEG0;
			break;
		case 6:
			spc=cpu.pc;
			work.destination=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			work.destination=(cpu.dregs.d[of.general.regdest])&(work.destination);
			cpu.pc=spc;
			work.destination=CommandMode[of.general.modesrc]((char)(of.general.regsrc),
																		1,work.destination,2);
			if(work.destination==0)ZERO1;
			else ZERO0;
			if(work.destination<0)NEG1;
			else NEG0;
			break;
	}
	CARRY0;
	OVER0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: divide
////////////////////////////////////////////////////////////////////////////////

void COM_divx(short opcode)
{
static ldiv_t dv;

	CACHEFUNCTION(COM_divx);
	cpu.pc+=2;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
	if(work.source)
	{
		switch(of.general.modedest)
		{
			case 3:   // DIVU //
				dv=ldiv((unsigned long)cpu.dregs.d[of.general.regdest],
					(unsigned long)work.source);
				if((unsigned long)cpu.dregs.d[of.general.regdest]<(unsigned short)work.source)OVER1;
				else
				{
					OVER0;
					cpu.dregs.d[of.general.regdest]=((unsigned long)dv.rem<<16)|
						(unsigned long)dv.quot;
				}
				break;
			case 7:   // DIVS //
				dv=ldiv(cpu.dregs.d[of.general.regdest],work.source);
				if((short)cpu.dregs.d[of.general.regdest]<(short)work.source)OVER1;
				else
				{
					OVER0;
					cpu.dregs.d[of.general.regdest]=((unsigned long)dv.rem<<16)|
						(unsigned long)dv.quot;
				}
				break;
		}
		if(dv.quot==0)ZERO1;
		else ZERO0;
		if(dv.quot<0)NEG1;
		else NEG0;
	}
	else div_by_zero();
	CARRY0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: subtract binary coded decimal
////////////////////////////////////////////////////////////////////////////////

void COM_sbcd(short opcode)
{
BYTE DecBorrow=0,ccr=cpu.sregs.ccr;
BYTE Number1,Number2,Number;

	cpu.pc+=2;
	switch(opcode&0x0008) // register or memory indirect predecrement
	{
		case 0x0000: // register (DRD)
			Number1=cpu.dregs.byted[of.general.regdest].dll;
			Number2=cpu.dregs.byted[of.general.regsrc].dll;
			__asm{
				xor ax,ax
				mov al,Number1
				mov bl,Number2
				sub al,bl
				das
				jnc noadjustd1
				add ah,1
			noadjustd1:
				mov bl,ccr
				shr bl,4
				and bl,1
				sub al,bl
				das
				jnc noadjustd2
				add ah,1
			noadjustd2:
				mov Number,al
				mov DecBorrow,ah
			}
			cpu.dregs.byted[of.general.regdest].dll=Number;
			break;
		case 0x0008: // memory indirect predecrement (ARIPD)
			cpu.aregs.a[of.general.regsrc]--;
			cpu.aregs.a[of.general.regdest]--;
			Number1=GETbyte(cpu.aregs.a[of.general.regdest]);
			Number2=GETbyte(cpu.aregs.a[of.general.regsrc]);
			__asm{
				xor ax,ax
				mov al,Number1
				mov bl,Number2
				sub al,bl
				das
				jnc noadjusta1
				add ah,1
			noadjusta1:
				mov bl,ccr
				shr bl,4
				and bl,1
				sub al,bl
				das
				jnc noadjusta2
				add ah,1
			noadjusta2:
				mov Number,al
				mov DecBorrow,ah
			}
			PUTbyte(cpu.aregs.a[of.general.regdest],Number);
			break;
	}
	if(Number)ZERO0;
	if(DecBorrow)
	{
		CARRY1;
		XTEND1;
	}
	else
	{
		CARRY0;
		XTEND0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: or
////////////////////////////////////////////////////////////////////////////////

void COM_or(short opcode)
{
	CACHEFUNCTION(COM_or);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 0:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]|
															(work.source&0x000000FF);
			if((char)cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if((char)cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 1:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]|
														(work.source&0x0000FFFF);
			if((short)cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if((short)cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 2:
			work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,2);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]|work.source;
			if(cpu.dregs.d[of.general.regdest]==0)ZERO1;
			else ZERO0;
			if(cpu.dregs.d[of.general.regdest]<0)NEG1;
			else NEG0;
			break;
		case 4:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.pc=spc;
			work.source=work.source|(cpu.dregs.d[of.general.regdest]&0x000000FF);
			work.destination=CommandMode[of.general.modesrc]((char)(of.general.regsrc),
										1,(char)work.source,0);
			if((char)work.destination==0)ZERO1;
			else ZERO0;
			if((char)work.destination<0)NEG1;
			else NEG0;
			break;
		case 5:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.pc=spc;
			work.source=work.source|(cpu.dregs.d[of.general.regdest]&0x0000FFFF);
			work.destination=CommandMode[of.general.modesrc]((char)(of.general.regsrc),
																		1,(short)work.source,1);
			if((short)work.destination==0)ZERO1;
			else ZERO0;
			if((short)work.destination<0)NEG1;
			else NEG0;
			break;
		case 6:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.pc=spc;
			work.source=work.source|cpu.dregs.d[of.general.regdest];
			work.destination=CommandMode[of.general.modesrc]((char)(of.general.regsrc),
																		1,work.source,2);
			if(work.destination==0)ZERO1;
			else ZERO0;
			if(work.destination<0)NEG1;
			else NEG0;
			break;
	}
	CARRY0;
	OVER0;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: OR immediate to SR (priviledged instruction)
////////////////////////////////////////////////////////////////////////////////

void COM_oritoSR(short opcode)
{
	CACHEFUNCTION(COM_oritoSR);
	// test if in supervisor modes
	if(cpu.sregs.sr&0x3000)
	{
		cpu.pc+=2;
		cpu.sregs.sr=cpu.sregs.sr|GETword(cpu.pc);
		cpu.pc+=2;
	}
	else priv_viol();
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: OR immediate to CCR
////////////////////////////////////////////////////////////////////////////////

void COM_oritoCCR(short opcode)
{
	CACHEFUNCTION(COM_oritoCCR);
	cpu.pc+=2;
	cpu.sregs.ccr=cpu.sregs.ccr|(unsigned char)GETword(cpu.pc);
	cpu.pc+=2;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: exclusive OR
////////////////////////////////////////////////////////////////////////////////

void COM_eor(short opcode)
{
	CACHEFUNCTION(COM_eor);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 0:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.dregs.d[of.general.regdest]=
				cpu.dregs.d[of.general.regdest]^(work.source&0x000000FF);
			break;
		case 1:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.dregs.d[of.general.regdest]=
				cpu.dregs.d[of.general.regdest]^(work.source&0x0000FFFF);
			break;
		case 2:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.dregs.d[of.general.regdest]=cpu.dregs.d[of.general.regdest]^work.source;
			break;
		case 4:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
			cpu.pc=spc;
			work.source=work.source^(cpu.dregs.d[of.general.regdest]&0x000000FF);
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,0);
			break;
		case 5:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			cpu.pc=spc;
			work.source=work.source^(cpu.dregs.d[of.general.regdest]&0x0000FFFF);
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,1);
			break;
		case 6:
			spc=cpu.pc;
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			cpu.pc=spc;
			work.source=work.source^cpu.dregs.d[of.general.regdest];
			CommandMode[of.general.modesrc]((char)(of.general.regsrc),1,work.source,2);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: compare
////////////////////////////////////////////////////////////////////////////////

void COM_cmp(short opcode)
{
	CACHEFUNCTION(COM_cmp);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 0:
			work.source=CommandMode[of.general.modesrc]((of.general.regsrc),0,0L,0);
			work.result=(char)cpu.dregs.d[of.general.regdest]-(char)work.source;
			setcarry( gen_carry((char)work.source,
							(char)cpu.dregs.d[of.general.regdest],(char)work.result) );
			setover( gen_over((char)work.source,
							(char)cpu.dregs.d[of.general.regdest],(char)work.result) );
			if((char)work.result==0)ZERO1;
			else ZERO0;
			if((char)work.result<0)NEG1;
			else NEG0;
			break;
		case 1:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			work.result=(short)cpu.dregs.d[of.general.regdest]-(short)work.source;
			setcarry( gen_carry((short)work.source,
						(short)cpu.dregs.d[of.general.regdest],(short)work.result) );
			setover( gen_over((short)work.source,
						(short)cpu.dregs.d[of.general.regdest],(short)work.result) );
			if((short)work.result==0)ZERO1;
			else ZERO0;
			if((short)work.result<0)NEG1;
			else NEG0;
			break;
		case 2:
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			work.result=cpu.dregs.d[of.general.regdest]-work.source;
			setcarry( gen_carry(work.source,cpu.dregs.d[of.general.regdest],work.result) );
			setover( gen_over(work.source,cpu.dregs.d[of.general.regdest],work.result) );
			if(work.result==0)ZERO1;
			else ZERO0;
			if(work.result<0)NEG1;
			else NEG0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: compare address
////////////////////////////////////////////////////////////////////////////////

void COM_cmpa(short opcode)
{
	CACHEFUNCTION(COM_cmpa);
	cpu.pc+=2;
	switch(of.general.modedest)
	{
		case 3:  // word //
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
			work.result=(short)cpu.aregs.a[of.general.regdest]-(short)work.source;
			setcarry( gen_carry((short)work.source,
						(short)cpu.aregs.a[of.general.regdest],(short)work.result) );
			setover( gen_over((short)work.source,
						  (short)cpu.aregs.a[of.general.regdest],(short)work.result) );
			if((short)work.result)ZERO0;
			else ZERO1;
			if((short)work.result<0)NEG1;
			else NEG0;
			break;
		case 7: // long //
			work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
			work.result=cpu.aregs.a[of.general.regdest]-work.source;
			setcarry( gen_carry(work.source,cpu.aregs.a[of.general.regdest],work.result) );
			setover( gen_over(work.source,cpu.aregs.a[of.general.regdest],work.result) );
			if(work.result)ZERO0;
			else ZERO1;
			if(work.result<0)NEG1;
			else NEG0;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: Bitfield operations
////////////////////////////////////////////////////////////////////////////////

void COM_BitField(short opcode)
{
short extension;
long index,od,ea,Offset,Width,TempReg;
short OffWid;
BOOL bDirect=FALSE;
long i;

	CACHEFUNCTION(COM_BitField);
	cpu.pc+=2;
	OffWid=GETword(cpu.pc);
	cpu.pc+=2;
	switch(of.general.modesrc)
	{
		case 0:
			bDirect=TRUE;
			break;
		case 2: // ARI
			ea=cpu.aregs.a[(of.general.regsrc)];
			break;
		case 5: // ARID
			ea=cpu.aregs.a[(of.general.regsrc)]+(long)GETword(cpu.pc);
			break;
		case 6: // ARII
			extension=GETword(cpu.pc);
			cpu.pc+=2;
			switch((extension&0x0100)>>8)
			{
				case 0: // brief format extension
					if(extension&0x8000) // (d8,Asrc,An.x*scale)
					{
						if(extension&0x0800) // long word
						{
							ea=
							cpu.aregs.a[(of.general.regsrc)]+
								(cpu.aregs.a[(extension>>12)&0x0007])*
									(1<<((extension>>9)&0x0003));
						}
						else
						{
							ea=cpu.aregs.a[(of.general.regsrc)]+
								(long)((short)cpu.aregs.a[(extension>>12)&0x0007])*
									(1<<((extension>>9)&0x0003));
						}
					}
					else // Dn
					{
						if(extension&0x0800) // long word
						{
							ea=cpu.aregs.a[(of.general.regsrc)]+
							 (cpu.dregs.d[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
						else
						{
							ea=cpu.aregs.a[(of.general.regsrc)]+
							  (long)((short)cpu.dregs.d[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
						}
					}
					ea+=(long)(extension&0x00FF);
					break;
				case 1: // long format extension

					if(!(extension&0x0080)) // BD is not suppressed
					{
						ea=cpu.aregs.a[(of.general.regsrc)];
					}
					else ea=0;

					switch(extension&0x8800)
					{
						case 0x0000:
							index=(cpu.dregs.d[(extension>>12)&0x0007])*
										(1<<((extension>>9)&0x0003));
							break;
						case 0x0800:
							index=(cpu.dregs.d[(extension>>12)&0x0007])*
									(1<<((extension>>9)&0x0003));
							break;
						case 0x8000:
							index=(cpu.aregs.a[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
							break;
						case 0x8800:
							index=(cpu.aregs.a[(extension>>12)&0x0007])*
								(1<<((extension>>9)&0x0003));
							break;
					}

					switch((extension>>4)&0x0003) // BD size
					{
						case 0: // reserved
						case 1: // null displacement
							break;
						case 2: // word displacement
							ea+=(long)(short)GETword(cpu.pc);
							cpu.pc+=2;
							break;
						case 3: // long displacement
							ea+=(long)GETdword(cpu.pc);
							cpu.pc+=4;
							break;
					}
					if(!(extension&0x0040)) // Index is not suppressed
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								break;
							case 1: // indirect pre-indexed with null displacement
								break;
							case 2: // indirect pre-indexed with word displacement
								break;
							case 3: // indirect pre-indexed with long displacement
								break;
							case 4: // reserved
								break;
							case 5: // indirect post-indexed with null displacement
								break;
							case 6: // indirect post-indexed with word displacement
								break;
							case 7: // indirect post-indexed with long displacement
								break;
						}
					}
					else
					{
						switch(extension&0x0007)
						{
							case 0: // no memory indirection
								ea+=index;
								break;
							case 1: // memory indirect with null displacement
								ea=GETdword(cpu.aregs.a[of.general.regdest]);
								break;
							case 2: // memory indirect with word displacement
								od=(long)GETword(cpu.pc);
								ea=GETdword(cpu.aregs.a[of.general.regdest]+od);
								break;
							case 3: // memory indirect with long displacement
								od=GETdword(cpu.pc);
								ea=GETdword(cpu.aregs.a[of.general.regdest]+od);
								break;
							case 4: // reserved
							case 5:
							case 6:
							case 7:
								break;
						}
					}
					break;
			}
			break;
		case 7: // MISC (PC,Absolute)
			switch(of.general.regsrc)
			{
				case 0: // (XXX).w
					ea=(long)GETword(cpu.pc);
					cpu.pc+=2;
					break;
				case 1: // (XXX).l
					ea=GETdword(cpu.pc);
					cpu.pc+=4;
					break;
				case 2: // (d16,PC)
					ea=cpu.pc+(long)GETword(cpu.pc);
					cpu.pc+=2;
					break;
			}
			break;
		default:
			Unknown(opcode);
			break;
	}
	// extract OFFSET field
	switch((OffWid>>11)&0x0001)
	{
		case 0:
			Offset=(OffWid>>6)&0x001f;
			break;
		case 1:
			Offset=cpu.dregs.d[(OffWid>>6)&0x0007];
			break;
	}
	// extract WIDTH field
	switch((OffWid>>5)&0x0001)
	{
		case 0:
			Width=OffWid&0x001f;
			Width=Width?Width:32;
			break;
		case 1:
			Width=cpu.dregs.d[OffWid&0x0007];
			Width%=32;
			Width=Width?Width:32;
			break;
	}
	switch((opcode>>8)&0x0007)
	{
		case 0: //BFTST
			Unknown(opcode);
			switch(bDirect) // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					break;
			}
			break;
		case 1: //BFEXTU
			switch(bDirect) // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					for(TempReg=0,i=0;i<Width;i++)
					{
						if(cpu.dregs.d[of.general.regsrc]&(0x80000000L>>(i+Offset)))
						{
							TempReg|=(0x000000001L<<(Width-1-i));
						}
					}
					cpu.dregs.d[(OffWid>>12)&0x0007]=TempReg;
					break;
			}
			break;
		case 2: //BFCHG
			Unknown(opcode);
			switch(bDirect) // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					break;
			}
			break;
		case 3: //BFEXTS
			Unknown(opcode);
			switch(bDirect)  // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					break;
			}
			break;
		case 4: //BFCLR
			Unknown(opcode);
			switch(bDirect) // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					break;
			}
			break;
		case 5: //BFFFF0
			Unknown(opcode);
			switch(bDirect) // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					break;
			}
			break;
		case 6: //BFSET
			Unknown(opcode);
			switch(bDirect) // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					break;
			}
			break;
		case 7: //BFINS
			switch(bDirect) // DRD or ARIPD
			{
				case FALSE:
					break;
				case TRUE:  // destination is data register
					for(TempReg=cpu.dregs.d[of.general.regsrc],i=0;i<Width;i++)
					{
						if(cpu.dregs.d[(OffWid>>12)&0x0007]&(0x000000001L<<(Width-1-i)))
						{
							TempReg|=(0x80000000L>>(i+Offset));
						}
						else
						{
							TempReg&=~(0x80000000L>>(i+Offset));
						}
					}
					cpu.dregs.d[of.general.regsrc]=TempReg;
					break;
			}
			break;
	}
	CARRY0;
	OVER0;
}


////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move byte
////////////////////////////////////////////////////////////////////////////////

void COM_MoveByte(short opcode)
{
	CACHEFUNCTION(COM_MoveByte);
	cpu.pc+=2;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,0);
	if((of.general.modedest)==0x01)
	{
		work.destination=
			CommandMode[of.general.modedest]((char)(of.general.regdest),1,
				(long)(work.source&0x000000FF),0);
		if(work.source==0)ZERO1;
		else ZERO0;
		if(work.source<0)NEG1;
		else NEG0;
		CARRY0;
		OVER0;
	}
	else
	{
		work.destination=
			CommandMode[of.general.modedest]((char)(of.general.regdest),1,
				work.source,0);
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move long
////////////////////////////////////////////////////////////////////////////////

void COM_MoveLong(short opcode)
{
	CACHEFUNCTION(COM_MoveLong);
	cpu.pc=cpu.pc+2;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,2);
	work.destination=
		CommandMode[of.general.modedest]((char)(of.general.regdest),
			1,work.source,2);
	if((of.general.modedest)!=0x01) // NOT movea.l
	{
		if(work.source==0)ZERO1;
		else ZERO0;
		if(work.source<0)NEG1;
		else NEG0;
		CARRY0;
		OVER0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move word
////////////////////////////////////////////////////////////////////////////////

void COM_MoveWord(short opcode)
{
	CACHEFUNCTION(COM_MoveWord);
	cpu.pc+=2;
	work.source=CommandMode[of.general.modesrc]((char)(of.general.regsrc),0,0L,1);
	if((of.general.modedest)==0x01)
	{
		work.source=(long)(work.source&0x0000FFFF);
		work.destination=
			CommandMode[of.general.modedest]((char)(of.general.regdest),1,
				(long)(work.source&0x0000FFFF),2);
	}
	else
	{
		work.destination=CommandMode[of.general.modedest]((char)(of.general.regdest),
																		1,work.source,1);
		if(work.source==0)ZERO1;
		else ZERO0;
		if(work.source<0)NEG1;
		else NEG0;
		CARRY0;
		OVER0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: move quick
////////////////////////////////////////////////////////////////////////////////

void COM_movequick(short opcode)
{
	cpu.pc+=2;
	work.source=(long)((char)opcode);
	cpu.dregs.d[of.general.regdest]=work.source;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: subtract with extend
////////////////////////////////////////////////////////////////////////////////
void COM_subx(short opcode)
{
char xtend=(cpu.sregs.ccr&0x10)?1:0; // get extend flag from CCR

	cpu.pc+=2;
	if(opcode&0x0008) // DRD or ARIPD
	{
		switch(of.special.size) // which size
		{
			case 0: // BYTE
				cpu.aregs.a[of.general.regsrc]--;
				cpu.aregs.a[of.general.regdest]--;
				work.source=GETbyte(cpu.aregs.a[of.general.regsrc]);
				work.destination=GETbyte(cpu.aregs.a[of.general.regdest]);
				work.result=(char)((char)work.destination-(char)work.source-(char)xtend);
				PUTbyte(cpu.aregs.a[of.general.regdest],(char)work.result);
				if((char)work.result<0)NEG1;
				else NEG0;
				if((char)work.result)ZERO0;
				setcarry(gen_over((char)work.source,(char)work.destination,(char)work.result));
				setxtend(gen_over((char)work.source,(char)work.destination,(char)work.result));
				break;
			case 1: // WORD
				cpu.aregs.a[of.general.regsrc]-=2;
				cpu.aregs.a[of.general.regdest]-=2;
				work.source=GETword(cpu.aregs.a[of.general.regsrc]);
				work.destination=GETword(cpu.aregs.a[of.general.regdest]);
				work.result=(short)((short)work.destination-(short)work.source-(short)xtend);
				PUTword(cpu.aregs.a[of.general.regdest],(short)work.result);
				if((short)work.result<0)NEG1;
				else NEG0;
				if((short)work.result)ZERO0;
				setcarry(gen_over((short)work.source,(short)work.destination,(short)work.result));
				setxtend(gen_over((short)work.source,(short)work.destination,(short)work.result));
				break;
			case 2: // DWORD
				cpu.aregs.a[of.general.regsrc]-=4;
				cpu.aregs.a[of.general.regdest]-=4;
				work.source=GETdword(cpu.aregs.a[of.general.regsrc]);
				work.destination=GETdword(cpu.aregs.a[of.general.regdest]);
				work.result=(work.destination-work.source-(long)xtend);
				PUTdword(cpu.aregs.a[of.general.regdest],work.result);
				if(work.result<0)NEG1;
				else NEG0;
				if(work.result)ZERO0;
				setcarry(gen_over(work.source,work.destination,work.result));
				setxtend(gen_over(work.source,work.destination,work.result));
				break;
		}
	}
	else
	{
		switch(of.special.size) // which size
		{
			case 0: // BYTE
				work.source=(char)cpu.dregs.byted[of.general.regsrc].dll;
				work.destination=(char)cpu.dregs.byted[of.general.regdest].dll;
				work.result=(char)((char)work.destination-(char)work.source-(char)xtend);
				cpu.dregs.byted[of.general.regdest].dll=(char)work.result;
				if((char)work.result<0)NEG1;
				else NEG0;
				if((char)work.result)ZERO0;
				setcarry(gen_over((char)work.source,(char)work.destination,(char)work.result));
				setxtend(gen_over((char)work.source,(char)work.destination,(char)work.result));
				break;
			case 1: // WORD
				work.source=(short)cpu.dregs.wordd[of.general.regsrc].dl;
				work.destination=(short)cpu.dregs.wordd[of.general.regdest].dl;
				work.result=(short)((short)work.destination-(short)work.source-(short)xtend);
				cpu.dregs.wordd[of.general.regdest].dl=(short)work.result;
				if((short)work.result<0)NEG1;
				else NEG0;
				if((short)work.result)ZERO0;
				setcarry(gen_over((short)work.source,(short)work.destination,(short)work.result));
				setxtend(gen_over((short)work.source,(short)work.destination,(short)work.result));
				break;
			case 2: // DWORD
				work.source=cpu.dregs.d[of.general.regsrc];
				work.destination=cpu.dregs.d[of.general.regdest];
				work.result=work.destination-work.source-(long)xtend;
				cpu.dregs.d[of.general.regdest]=work.result;
				if(work.result<0)NEG1;
				else NEG0;
				if(work.result)ZERO0;
				setcarry(gen_over(work.source,work.destination,work.result));
				setxtend(gen_over(work.source,work.destination,work.result));
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: opcode $Axxx (LINEA)
////////////////////////////////////////////////////////////////////////////////

void COM_linea(short opcode)
{
	cpu.pc+=2; // increment PC
	emulatelinea(); // do line A exception
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: opcode $Fxxx (LINEF)
////////////////////////////////////////////////////////////////////////////////

void COM_linef(short opcode)
{
	cpu.pc+=2; // increment PC
	emulatelinef(); // do line F exception
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: pack
////////////////////////////////////////////////////////////////////////////////

void COM_pack(short opcode)
{
	CACHEFUNCTION(COM_pack);
	cpu.pc+=2;
	work.result=0;
	switch((opcode>>3)&0x0001) // DRD or ARIPD
	{
		case 0:	// DRD
			// get unpacked source
			work.source=cpu.dregs.wordd[of.general.regsrc].dl&0x0F0F;
			work.source+=GETword(cpu.pc); // add adjustment
			cpu.dregs.wordd[of.general.regdest].dl=
				(WORD)((WORD)(work.source&0x0000000FL)|(WORD)((work.source>>4) &
					0x000000F0L));
			break;
		case 1: // ARIPD
			cpu.aregs.a[of.general.regsrc]-=2;
			cpu.aregs.a[of.general.regdest]--;
			work.source=(WORD)GETword(cpu.aregs.a[of.general.regsrc])&0x0F0F;
			work.source+=GETword(cpu.pc); // add adjustment
			PUTbyte(cpu.aregs.a[of.general.regdest],
				(BYTE)(work.source&0x0000000FL)|(BYTE)((work.source>>4)
					&0x00000F0L));
			break;
	}
	cpu.pc+=2;
}

////////////////////////////////////////////////////////////////////////////////
// DESCRIPTION: unpack
////////////////////////////////////////////////////////////////////////////////

void COM_unpack(short opcode)
{
	BYTE source = work.source,destination;
	CACHEFUNCTION(COM_unpack);
	cpu.pc+=2;
	work.result=work.destination=0;
	switch((opcode>>3)&0x0001) // DRD or ARIPD
	{
		case 0:	// DRD
			work.source=cpu.dregs.byted[of.general.regsrc].dll;
			__asm{
				mov al,byte ptr source;
				__asm __emit 0xd4
				__asm __emit 0x10 
				and ax,0x0F0F
				mov word ptr destination,ax
			}
			work.destination+=GETword(cpu.pc); // add adjustment
			cpu.dregs.wordd[of.general.regdest].dl=work.destination;
			break;
		case 1:	// ARIPD
			cpu.aregs.a[of.general.regsrc]--;
			cpu.aregs.a[of.general.regdest]-=2;
			work.source=GETbyte(cpu.aregs.a[of.general.regsrc]);
			__asm{
				mov al,byte ptr source
				__asm __emit 0xd4
				__asm __emit 0x10 // variation of AAM (0xd4,0x0a)
				and ax,0x0F0F
				mov word ptr destination,ax
			}
			work.destination = destination;
			work.destination+=GETword(cpu.pc); // add adjustment
			PUTword(cpu.aregs.a[of.general.regdest],(WORD)work.destination);
			break;
	}
	cpu.pc+=2;
}

////////////////////////////////////////////////////////////////////////////////
//              Simulation inits and cleanups
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// NAME:				BOOL SetupSim(void)
//
// DESCRIPTION:   setup the simulation i.e. load the plugins
//
// PARAMETERS:    none
//
// RETURNS:       BOOL: indicates successful setup
//
////////////////////////////////////////////////////////////////////////////////
BOOL SetupSim(void)
{
WIN32_FIND_DATA sFindData; // structure for FINDFIRSTFILE(), FINDNEXTFILE()
HANDLE hData; // handle for FINDNEXTFILE()
HMODULE hModule; // loaded modules handle
int index=0; // index into list of DLLHDRs
char temp[64]; // guess what!
int j,k; // indices
char szPluginDir[256]; // path to plugin directory

	// get directory of plugins from EVMSIM.INI
	GetPrivateProfileString("EVMSIM","PLUGINDIR","",
		szPluginDir,256,"evmsim.ini");

	if(!strlen(szPluginDir))
	{
		GetCurrentDirectory(256,szPluginDir);
		strcat(szPluginDir,"\\");
	}

	if(strlen(szPluginDir)) // config entry found
	{
		strcat(szPluginDir,"*.dll");     // append search extension
		if((hData=FindFirstFile(szPluginDir,&sFindData))!=INVALID_HANDLE_VALUE) // find first file '*.dll'
		{
			do
			{
				hModule=LoadLibrary(sFindData.cFileName); // load DLL
				//hModule=GetModuleHandle(sFindData.cFileName); // get module handle
				if((pDllHdr[index]=(struct tag_DLLHDR*)
					GetProcAddress(hModule,"DLLHDR"))!=0) // get DLL structure
				{
					if(pDllHdr[index]->SetupProc) // does setup procedure exist?
					{
						// display data of plugin
						if(pDllHdr[index]->nAddress!=0xFFFFFFFFL)
						{
							wsprintf(temp,"%-32s 0x%08lX-0x%08lX",sFindData.cFileName,
																pDllHdr[index]->nAddress,
									pDllHdr[index]->nAddress+pDllHdr[index]->nSize-1);
						}
						else
						{
							wsprintf(temp,"%-32s a service plugin",sFindData.cFileName);
						}
						// setup structure for access by all plugins
						pDllHdr[index]->pConnect=&sConn_to_cpu;
						pDllHdr[index]->hWnd=ghWnd;
						pDllHdr[index]->pAllHdrs=(void*)pDllHdr;
						pDllHdr[index]->pCpu=&cpu;
						pDllHdr[index]->GetModule=CheckPeriAddr;
						if(pDllHdr[index]->SetupProc()==TRUE) // call DLL setup proc
						{
							// and display plugin data
							SetStartupListboxText(temp);
							index++; // next entry in proc jump table
						}
						else
						{
							if(MessageBox(ghWnd,sFindData.cFileName,
							  "EVMSIM plugin returned failure on setup.\nShould I quit?",
								MB_YESNO|MB_ICONSTOP|MB_TOPMOST)==IDYES)
							{
								return FALSE; // fatal exit
							}
							else
							{
								FreeLibrary(hModule); // not found _DLLHDR, just free library
							}
						}
					}
					else // SetupProc does not exist, so do fatal app exit
					{
						MessageBox(ghWnd,sFindData.cFileName,
							"Found plugin that has no setup procedure.\nQuitting!!!",
								MB_OK|MB_ICONSTOP|MB_TOPMOST);
						return FALSE; // fatal app exit
					}
				}
				else FreeLibrary(hModule); // not found _DLLHDR
			// do as long there are '*.dll'
			}while(FindNextFile(hData,&sFindData) && index<63);
			pDllHdr[index]=0; // mark last proc jump table entry
		}
	}

	// didn't find any plugin
	// since this can't be (at least one module (RAM/ROM) is required), we have to quit!!!
	if(!index)
	{
		MessageBox(ghWnd,"Didn't find any plugins at all.\nQuitting!!!","EVMSIM",

			MB_OK|MB_ICONSTOP|MB_TOPMOST);
		return FALSE;
	}

	// sort the modules on defined priority (pDllHdr[index]->nPriority) (bubble sort)
	for(j=0;pDllHdr[j]!=0;j++); // find end of list
	for(k=0;k<j;k++) // do sort j times
	{
		for(index=1;pDllHdr[index]!=0;index++) // for each module
		{
		struct tag_DLLHDR *temp;

			// compare on magnitude of priority
			if(pDllHdr[index]->nPriority>pDllHdr[index-1]->nPriority)
			{
				// swap the two entries on condition TRUE
				temp=pDllHdr[index-1];
				pDllHdr[index-1]=pDllHdr[index];
				pDllHdr[index]=temp;
			}
		}
	}

	// call init procedures to give the modules a chance to interconnect
	// this can be done by accessing DLLHDR.pAllHdrs
	for(index=0;pDllHdr[index]!=0;index++)pDllHdr[index]->InitProc();

	SetupMapperArrays(); // build an address table for easy and fast access to peripherals

	return TRUE; // return success
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void ExitSim(void)
//
// DESCRIPTION:   shutdown the simulation
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void ExitSim(void)
{
int index;

	// call all exit procedures in attached modules
	// so that cleanups can be done
	for(index=0;pDllHdr[index]!=0;index++)
	{
		if(pDllHdr[index]->ExitProc)pDllHdr[index]->ExitProc();
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void ResetSim(void)
//
// DESCRIPTION:   reset the simulation
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
BOOL ResetSim(void)
{
short i;

	// reset hardware IRQ counter
	nIRQs=0;
	// put CPU into run state, clear stop flag
	bStopped=FALSE;
	// get CPU PC start vector from mem loc 4
	// if there is no memory in range, stop CPU
	if(CheckPeriAddr(0x00000004L)!=-1)cpu.pc=GETdword(0x00000004L);
	else return FALSE;
	// get initial stack pointer
	// if there is no memory in range, stop CPU
	if(CheckPeriAddr(0x00000000L)!=-1)cpu.aregs.a[7]=cpu.ssp=GETdword(0x00000000L);
	else return FALSE;
	// set registers back to initial state
	for(i=0;i<8;i++){cpu.aregs.a[i]=-1; cpu.dregs.d[i]=-1;}
	// reset the special registers
	cpu.vbr=cpu.sfc=cpu.dfc=cpu.cacr=cpu.caar=0;
	// startup SR (SUPERVISOR IPL=7 No flags)
	cpu.sregs.sr=0x2700;
	// call the reset procedure of every attached plugin module
	for(i=0;pDllHdr[i]!=0;i++)
	{
		if(pDllHdr[i]->ResetProc)pDllHdr[i]->ResetProc();
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//              Simulation begin
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// NAME:				void Simulate68k(unsigned long ops)
//
// DESCRIPTION:   execution loop of simulation
//						fetch, decode, execute opcodes
//
// PARAMETERS:    unsigned long ops: number of operations in one call
//
// RETURNS:       none
//
// REMARKS:			This function could be interpreted as the pipeline
//						manager of the virtual CPU. It's general operation
//						looks like this:
//						(1)	fetch opcode if not uneven address
//						(2)	setup stack pointer that application sees
//						(3)	decode and execute opcode
//						(4)	setup stack pointer after operation
//						(5)	has STOP #imm occurred?
//						(6)	single step?
//						(7)	hardware IRQ?
//						(8)	next opcode, start at (1)
//
////////////////////////////////////////////////////////////////////////////////

void Simulate68k(unsigned long ops)
{
	short index;

	// do the process loop [ops] times
	while(ops--)
	{
		// uneven access to fetch opcode -> ADDRESS ERROR
		if(cpu.pc&0x00000001L)
		{
			addr_err();
		}

		of.o = GETword(cpu.pc);

		// setup the stack to use
		switch(cpu.sregs.sr&0x3000) // set A7 according to mode
		{
			// we are in user mode
			case 0:
			case 0x1000:
				cpu.aregs.a[7]=cpu.usp;
				break;
			// we are in supervisor mode
			case 0x2000:
				cpu.aregs.a[7]=cpu.ssp;
				break;
			// we are in master mode
			case 0x3000:
				cpu.aregs.a[7]=cpu.msp;
		}

		// save away the PC before the operation so we can check if anything happened at all
		// and we can restart a new opcode cycle (exception handling etc.)
		pcbefore=cpu.pc;


		if(!bStopped)
			Operation[of.o](of.o); 	// decode and execute command

		// PC didn't change and opcode is STOP #imm
		// testing the PC distance before and after
		// is not enough, since a branch command could
		// start out on the same PC as before, i.e.
		// label: DBRA D0,label
		if(of.o==0x4E72 && pcbefore==cpu.pc)bStopped=TRUE;

		// setup internal shadow stack pointer after the operation has taken place
		switch(cpu.sregs.sr&0x3000) // set A7 according to mode
		{
			// we switched to user mode
			case 0:
			case 0x1000:
				cpu.usp=cpu.aregs.a[7];
				break;
			// we switched to supervisor mode
			case 0x2000:
				cpu.ssp=cpu.aregs.a[7];
				break;
			// we switched to master mode
			case 0x3000:
				cpu.msp=cpu.aregs.a[7];
				break;
		}

		// call simulation procedure for each attached module
		// so that every plugin receives the focus once after
		// every executed opcode
		for(index=0;pDllHdr[index]!=0;index++)
		{
			if(pDllHdr[index]->SimProc)pDllHdr[index]->SimProc();
		}

		// suppress interrupt generation right after RTE
		// so that at least one opcode will be executed
		// between to IRQs
		if(of.o!=0x4e73) // OPCODE is not RTE
		{
			// if TRACE bit is set
			// do single step exception
			if(cpu.sregs.sr&0x8000)
			{
				single_step(); // simulate tracing control
			}
			// check if a hardware IRQ has occurred
			// and process it
			CheckForInt();
		}
	}

}

//// EOF /////////////////////////////////////////////////////////////////////////////
