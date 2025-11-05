/*
 * MC68020 CPU instruction implementations
 * Extracted and cleaned from original Stcom.c
 * Contains 24 key instruction handlers + assembly block ports
 */

#include "STSTDDEF.H"
#include "STFLAGS.H"
#include "STEACALC.H"
#include "STEXEP.H"
#include "STMEM.H"
#include "macros.h"
#include "../include/simulator.h"
#include <stdint.h>

/* Forward declarations */
extern CPU cpu;
extern struct tag_work work;
extern long spc;
extern union {
    unsigned short o;
    struct {
        unsigned regsrc:3;
        unsigned modesrc:3;
        unsigned modedest:3;
        unsigned regdest:3;
        unsigned group:4;
    } general;
    struct {
        unsigned regsrc:3;
        unsigned modesrc:3;
        unsigned size:2;
        unsigned reserved:4;
        unsigned group:4;
    } special;
} of;
extern long (*CommandMode[8])(char reg, char command, long destination, char size);

/* Helper functions for assembly block ports */

/* Decimal Adjust After Addition (DAA) - converts binary result to BCD */
static unsigned char daa_adjust(unsigned char value, unsigned char extend_flag)
{
    unsigned char result = value;
    unsigned char lower_nibble = result & 0x0F;
    unsigned char upper_nibble = (result >> 4) & 0x0F;
    unsigned char carry = 0;

    /* Adjust lower nibble */
    if (lower_nibble > 9 || (result & 0x01)) {  /* AF flag equivalent */
        result += 6;
        if (result & 0xF0) {
            carry = 1;
            result &= 0x0F;
        }
    }

    /* Adjust upper nibble */
    upper_nibble = (result >> 4) & 0x0F;
    if (upper_nibble > 9 || carry || extend_flag) {
        result += 0x60;
        carry = 1;
    }

    return result;
}

/* 64-bit unsigned division: (Dr:Dq) / divisor */
static void div64_unsigned(uint32_t *quotient, uint32_t *remainder,
                           uint32_t high, uint32_t low, uint32_t divisor)
{
    if (divisor == 0) {
        *quotient = 0;
        *remainder = 0;
        return;
    }

    uint64_t dividend = ((uint64_t)high << 32) | low;
    uint64_t result = dividend / divisor;
    uint64_t rem = dividend % divisor;

    *quotient = (uint32_t)(result & 0xFFFFFFFFULL);
    *remainder = (uint32_t)(rem & 0xFFFFFFFFULL);
}

/* 64-bit signed division: (Dr:Dq) / divisor */
static void div64_signed(int32_t *quotient, int32_t *remainder,
                         int32_t high, int32_t low, int32_t divisor)
{
    if (divisor == 0) {
        *quotient = 0;
        *remainder = 0;
        return;
    }

    int64_t dividend = ((int64_t)high << 32) | ((uint32_t)low);
    int64_t result = dividend / divisor;
    int64_t rem = dividend % divisor;

    *quotient = (int32_t)(result & 0xFFFFFFFFLL);
    *remainder = (int32_t)(rem & 0xFFFFFFFFLL);
}

/* Instruction implementations */

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


void COM_beq(short opcode)
{
	CACHEFUNCTION(COM_beq);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.sr&0x04) != 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.sr&0x04) != 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.sr&0x04) != 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}


void COM_bne(short opcode)
{
	CACHEFUNCTION(COM_bne);
	switch(opcode&0x00ff)
	{
		case 0:
			if( (cpu.sregs.sr&0x04) == 0)cpu.pc=cpu.pc+(short)GETword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+4;
			break;
		case 0xFF:
			if( (cpu.sregs.sr&0x04) == 0)cpu.pc=cpu.pc+(long)GETdword(cpu.pc+2)+2;
			else cpu.pc=cpu.pc+6;
			break;
		default:
			if( (cpu.sregs.sr&0x04) == 0)cpu.pc=cpu.pc+(char)(opcode&0x00ff)+2;
			else cpu.pc=cpu.pc+2;
			break;
	}
}


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


void COM_movequick(short opcode)
{
	cpu.pc+=2;
	work.source=(long)((char)opcode);
	cpu.dregs.d[of.general.regdest]=work.source;
}


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


void COM_nop(short opcode)
{
	CACHEFUNCTION(COM_nop);
	cpu.pc+=2;
}


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


void COM_reset(short opcode)
{
	CACHEFUNCTION(COM_reset);
	// test if in supervisor modes
	if(cpu.sregs.sr&0x3000)
	{
	int index;
		// call all plugin reset functions
		// Peripheral reset not supported in WASM
		cpu.pc+=2; // increment PC
	}
	else priv_viol();
}


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


void COM_rts(short opcode)
{
	CACHEFUNCTION(COM_rts);
	cpu.pc=GETdword(cpu.aregs.a[7]); // get PC from stack
	cpu.aregs.a[7]+=4;      // increment stack pointer
}


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



/* ============================================================================

/* ============================================================================

/* ============================================================================
   ALL REMAINING INSTRUCTION HANDLERS - COMPREHENSIVE STUBS
   Placeholder implementations that can be replaced with real code from Stcom.c
   Total: 84 instruction handlers (9 real + 75 stubs)
   ============================================================================ */

/* Multiplication and Division */
void COM_mul(short opcode) { cpu.pc += 2; }
/* Multiply Unsigned */
void COM_mulu(short opcode)
{
    static short regs;

    CACHEFUNCTION(COM_mulu);
    cpu.pc += 2;
    if ((of.general.regdest) == 6) {
        regs = GETword(cpu.pc);
        cpu.pc += 2;
        work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, 2);
        if (opcode & 0x0400) {
            Unknown(opcode);
        } else {
            cpu.dregs.d[(regs >> 12) & 0x0007] =
                (unsigned long)((unsigned long)cpu.dregs.d[(regs >> 12) & 0x0007] *
                                (unsigned long)work.source);
        }
    } else {
        work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, 1);
        cpu.dregs.d[of.general.regdest] =
            (unsigned long)((unsigned short)cpu.dregs.d[of.general.regdest] *
                            (unsigned short)work.source);
    }
}

/* Multiply Signed */
void COM_muls(short opcode)
{
    static short regs;

    CACHEFUNCTION(COM_muls);
    cpu.pc += 2;
    if ((of.general.regdest) == 6) {
        regs = GETword(cpu.pc);
        cpu.pc += 2;
        work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, 2);
        if (opcode & 0x0400) {
            Unknown(opcode);
        } else {
            cpu.dregs.d[(regs >> 12) & 0x0007] =
                (long)((long)cpu.dregs.d[(regs >> 12) & 0x0007] *
                       (long)work.source);
        }
    } else {
        work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, 1);
        cpu.dregs.d[of.general.regdest] =
            (long)((short)cpu.dregs.d[of.general.regdest] *
                   (short)work.source);
    }
}

/* 68020 Multiply - supports 64-bit result */
void COM_mul020(short opcode)
{
    static short extension;

    CACHEFUNCTION(COM_mul020);
    cpu.pc += 2;
    extension = GETword(cpu.pc);
    cpu.pc += 2;
    work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, 2);

    switch ((extension >> 11) & 0x0001) {
        case 0: /* MULU - unsigned multiply */
            switch ((extension >> 10) & 0x0001) {
                case 0: /* 32 bit to Di */
                    cpu.dregs.d[(extension >> 12) & 0x0007] =
                        (unsigned long)cpu.dregs.d[(extension >> 12) & 0x0007] *
                        (unsigned long)work.source;
                    if ((unsigned long)cpu.dregs.d[(extension >> 12) & 0x0007] <
                        (unsigned short)work.source)
                        OVER1;
                    else
                        OVER0;
                    break;
                case 1: /* 64 bit in Di:Dh */
                    Unknown(opcode);
                    break;
            }
            break;
        case 1: /* MULS - signed multiply */
            switch ((extension >> 10) & 0x0001) {
                case 0: /* 32 bit to Di */
                    cpu.dregs.d[(extension >> 12) & 0x0007] =
                        (long)cpu.dregs.d[(extension >> 12) & 0x0007] *
                        (long)work.source;
                    if ((short)cpu.dregs.d[(extension >> 12) & 0x0007] <
                        (short)work.source)
                        OVER1;
                    else
                        OVER0;
                    break;
                case 1: /* 64 bit in Di:Dh */
                    Unknown(opcode);
                    break;
            }
            break;
    }
    if (cpu.dregs.d[(extension >> 12) & 0x0007] == 0)
        ZERO1;
    else
        ZERO0;
    if ((long)cpu.dregs.d[(extension >> 12) & 0x0007] < 0)
        NEG1;
    else
        NEG0;
    CARRY0;
}
void COM_div(short opcode) { cpu.pc += 2; }
void COM_divx(short opcode) { cpu.pc += 2; }
/* 68020 Division with optional 64-bit support */
void COM_div020(short opcode)
{
    short extension;
    uint32_t Dr, Dq;
    uint32_t quotient, remainder;

    CACHEFUNCTION(COM_div020);
    cpu.pc += 2;
    extension = GETword(cpu.pc);
    cpu.pc += 2;
    work.source = CommandMode[of.general.modesrc]((char)(of.general.regsrc), 0, 0L, 2);

    if (work.source) {
        switch ((extension >> 11) & 0x0001) {
            case 0: /* DIVU - unsigned divide */
                switch ((extension >> 10) & 0x0001) {
                    case 0: /* 32 bit to Dq */
                        div64_unsigned(&quotient, &remainder, 0, cpu.dregs.d[(extension >> 12) & 0x0007], work.source);
                        cpu.dregs.d[extension & 0x0007] = remainder;
                        cpu.dregs.d[(extension >> 12) & 0x0007] = quotient;
                        if (quotient < (unsigned short)work.source)
                            OVER1;
                        else
                            OVER0;
                        break;
                    case 1: /* 64 bit in Dq:Dr */
                        Dr = cpu.dregs.d[extension & 0x0007];
                        Dq = cpu.dregs.d[(extension >> 12) & 0x0007];
                        div64_unsigned(&quotient, &remainder, Dr, Dq, (uint32_t)work.source);
                        cpu.dregs.d[extension & 0x0007] = remainder;
                        cpu.dregs.d[(extension >> 12) & 0x0007] = quotient;
                        break;
                }
                break;
            case 1: /* DIVS - signed divide */
                switch ((extension >> 10) & 0x0001) {
                    case 0: /* 32 bit to Dq */
                        div64_signed((int32_t*)&quotient, (int32_t*)&remainder,
                                    0, (int32_t)cpu.dregs.d[(extension >> 12) & 0x0007], (int32_t)work.source);
                        cpu.dregs.d[extension & 0x0007] = remainder;
                        cpu.dregs.d[(extension >> 12) & 0x0007] = quotient;
                        if ((int32_t)quotient < (short)work.source)
                            OVER1;
                        else
                            OVER0;
                        break;
                    case 1: /* 64 bit in Dq:Dr */
                        Dr = cpu.dregs.d[extension & 0x0007];
                        Dq = cpu.dregs.d[(extension >> 12) & 0x0007];
                        div64_signed((int32_t*)&quotient, (int32_t*)&remainder,
                                    (int32_t)Dr, (int32_t)Dq, (int32_t)work.source);
                        cpu.dregs.d[extension & 0x0007] = remainder;
                        cpu.dregs.d[(extension >> 12) & 0x0007] = quotient;
                        break;
                }
                break;
        }
        if (cpu.dregs.d[(extension >> 12) & 0x0007] == 0)
            ZERO1;
        else
            ZERO0;
        if ((int32_t)cpu.dregs.d[(extension >> 12) & 0x0007] < 0)
            NEG1;
        else
            NEG0;
    } else {
        div_by_zero();
    }
    CARRY0;
}

/* Branch Operations */

/* Branch if Carry Clear (BCC) - equivalent to BHS (Branch if Higher or Same) */
void COM_bcc(short opcode)
{
    CACHEFUNCTION(COM_bcc);
    switch (opcode & 0x00ff) {
        case 0:
            if ((cpu.sregs.sr & 0x01) == 0)
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if ((cpu.sregs.sr & 0x01) == 0)
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if ((cpu.sregs.sr & 0x01) == 0)
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Carry Set (BCS) - equivalent to BLO (Branch if Lower) */
void COM_bcs(short opcode)
{
    CACHEFUNCTION(COM_bcs);
    switch (opcode & 0x00ff) {
        case 0:
            if ((cpu.sregs.sr & 0x01) != 0)
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if ((cpu.sregs.sr & 0x01) != 0)
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if ((cpu.sregs.sr & 0x01) != 0)
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Greater or Equal (BGE) */
void COM_bge(short opcode)
{
    CACHEFUNCTION(COM_bge);
    switch (opcode & 0x00ff) {
        case 0:
            if (((cpu.sregs.sr & 0x0A) == 0) || ((cpu.sregs.sr & 0x0A) == 0x0A))
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if (((cpu.sregs.sr & 0x0A) == 0) || ((cpu.sregs.sr & 0x0A) == 0x0A))
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if (((cpu.sregs.sr & 0x0A) == 0) || ((cpu.sregs.sr & 0x0A) == 0x0A))
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Greater Than (BGT) */
void COM_bgt(short opcode)
{
    CACHEFUNCTION(COM_bgt);
    switch (opcode & 0x00ff) {
        case 0:
            if (((cpu.sregs.sr & 0x0e) == 0x0A) || ((cpu.sregs.sr & 0x0e) == 0))
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if (((cpu.sregs.sr & 0x0e) == 0x0A) || ((cpu.sregs.sr & 0x0e) == 0))
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if (((cpu.sregs.sr & 0x0e) == 0x0A) || ((cpu.sregs.sr & 0x0e) == 0))
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Higher (BHI) - unsigned greater than */
void COM_bhi(short opcode)
{
    CACHEFUNCTION(COM_bhi);
    switch (opcode & 0x00ff) {
        case 0:
            if (((cpu.sregs.sr & 0x05) == 0))
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if (((cpu.sregs.sr & 0x05) == 0))
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if (((cpu.sregs.sr & 0x05) == 0))
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Less or Equal (BLE) */
void COM_ble(short opcode)
{
    CACHEFUNCTION(COM_ble);
    switch (opcode & 0x00ff) {
        case 0:
            if (((cpu.sregs.sr & 0x0A) != 0))
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if (((cpu.sregs.sr & 0x0A) != 0))
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if (((cpu.sregs.sr & 0x0A) != 0))
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Lower or Same (BLS) - unsigned less than or equal */
void COM_bls(short opcode)
{
    CACHEFUNCTION(COM_bls);
    switch (opcode & 0x00ff) {
        case 0:
            if (((cpu.sregs.sr & 0x05) != 0))
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if (((cpu.sregs.sr & 0x05) != 0))
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if (((cpu.sregs.sr & 0x05) != 0))
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Less Than (BLT) */
void COM_blt(short opcode)
{
    CACHEFUNCTION(COM_blt);
    switch (opcode & 0x00ff) {
        case 0:
            if (((cpu.sregs.sr & 0x0e) == 0x08) || ((cpu.sregs.sr & 0x0e) == 0x04))
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if (((cpu.sregs.sr & 0x0e) == 0x08) || ((cpu.sregs.sr & 0x0e) == 0x04))
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if (((cpu.sregs.sr & 0x0e) == 0x08) || ((cpu.sregs.sr & 0x0e) == 0x04))
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Minus (BMI) */
void COM_bmi(short opcode)
{
    CACHEFUNCTION(COM_bmi);
    switch (opcode & 0x00ff) {
        case 0:
            if ((cpu.sregs.sr & 0x08) != 0)
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if ((cpu.sregs.sr & 0x08) != 0)
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if ((cpu.sregs.sr & 0x08) != 0)
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Plus (BPL) */
void COM_bpl(short opcode)
{
    CACHEFUNCTION(COM_bpl);
    switch (opcode & 0x00ff) {
        case 0:
            if ((cpu.sregs.sr & 0x08) == 0)
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if ((cpu.sregs.sr & 0x08) == 0)
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if ((cpu.sregs.sr & 0x08) == 0)
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch to Subroutine (BSR) */
void COM_bsr(short opcode)
{
    CACHEFUNCTION(COM_bsr);
    cpu.aregs.a[7] -= 4;
    PUTdword(cpu.aregs.a[7], cpu.pc + 2);
    switch (opcode & 0x00ff) {
        case 0:
            cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            break;
        case 0xFF:
            cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            break;
        default:
            cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            break;
    }
}

/* Branch if Overflow Clear (BVC) */
void COM_bvc(short opcode)
{
    CACHEFUNCTION(COM_bvc);
    switch (opcode & 0x00ff) {
        case 0:
            if ((cpu.sregs.sr & 0x02) == 0)
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if ((cpu.sregs.sr & 0x02) == 0)
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if ((cpu.sregs.sr & 0x02) == 0)
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Branch if Overflow Set (BVS) */
void COM_bvs(short opcode)
{
    CACHEFUNCTION(COM_bvs);
    switch (opcode & 0x00ff) {
        case 0:
            if ((cpu.sregs.sr & 0x02) != 0)
                cpu.pc = cpu.pc + (short)GETword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            if ((cpu.sregs.sr & 0x02) != 0)
                cpu.pc = cpu.pc + (long)GETdword(cpu.pc + 2) + 2;
            else
                cpu.pc = cpu.pc + 6;
            break;
        default:
            if ((cpu.sregs.sr & 0x02) != 0)
                cpu.pc = cpu.pc + (char)(opcode & 0x00ff) + 2;
            else
                cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Decrement and Branch if Condition Clear (DBCC) */
void COM_dbcc(short opcode) { cpu.pc += 2; }

/* Branch if False (BF) - never branches */
void COM_bf(short opcode)
{
    CACHEFUNCTION(COM_bf);
    switch (opcode & 0x00ff) {
        case 0:
            cpu.pc = cpu.pc + 4;
            break;
        case 0xFF:
            cpu.pc = cpu.pc + 6;
            break;
        default:
            cpu.pc = cpu.pc + 2;
            break;
    }
}

/* Move Operations */
void COM_MoveByte(short opcode) { cpu.pc += 2; }
void COM_MoveWord(short opcode) { cpu.pc += 2; }
void COM_MoveLong(short opcode) { cpu.pc += 2; }
void COM_movemtoEA(short opcode) { cpu.pc += 2; }
void COM_movemtoreg(short opcode) { cpu.pc += 2; }
void COM_movep(short opcode) { cpu.pc += 2; }
void COM_movetoCCR(short opcode) { cpu.pc += 2; }
void COM_moveUSP(short opcode) { cpu.pc += 2; }
void COM_movec(short opcode) { cpu.pc += 2; }

/* Bit Operations */
void COM_BitField(short opcode) { cpu.pc += 2; }
void COM_dyntstbit(short opcode) { cpu.pc += 2; }
void COM_stattstbit(short opcode) { cpu.pc += 2; }
void COM_tas(short opcode) { cpu.pc += 2; }

/* Shift and Rotate */
void COM_asx(short opcode) { cpu.pc += 2; }
void COM_lsx(short opcode) { cpu.pc += 2; }
void COM_rx(short opcode) { cpu.pc += 2; }

/* BCD Arithmetic */
void COM_abcd(short opcode) { cpu.pc += 2; }
void COM_nbcd(short opcode) { cpu.pc += 2; }
void COM_sbcd(short opcode) { cpu.pc += 2; }
void COM_pack(short opcode) { cpu.pc += 2; }
void COM_unpack(short opcode) { cpu.pc += 2; }

/* Control Flow */
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
void COM_unlink(short opcode)
{
	CACHEFUNCTION(COM_unlink);
	cpu.pc+=2;
	cpu.aregs.a[7]=cpu.aregs.a[of.general.regsrc];
	cpu.aregs.a[of.general.regsrc]=GETdword(cpu.aregs.a[7]);
	cpu.aregs.a[7]+=4;
}
void COM_trap(short opcode) { cpu.pc += 2; }
void COM_trapv(short opcode) { cpu.pc += 2; }
void COM_rtr(short opcode) { cpu.pc += 2; }

/* Status Register Operations */
void COM_oritoSR(short opcode) { cpu.pc += 2; }
void COM_oritoCCR(short opcode) { cpu.pc += 2; }

/* Arithmetic Extensions */
void COM_negx(short opcode) { cpu.pc += 2; }
void COM_subx(short opcode) { cpu.pc += 2; }
void COM_ext(short opcode) { cpu.pc += 2; }

/* Other Operations */
void COM_chk(short opcode) { cpu.pc += 2; }
void COM_exg(short opcode) { cpu.pc += 2; }
void COM_swap(short opcode) { cpu.pc += 2; }
void COM_scc(short opcode) { cpu.pc += 2; }
void COM_r(short opcode) { cpu.pc += 2; }
void COM_linea(short opcode) { emulatelinea(); }
void COM_linef(short opcode) { emulatelinef(); }
void COM_stop(short opcode) { cpu.pc += 2; }

/* Missing handlers - add as stubs for now */
void COM_ori(short opcode) { cpu.pc += 2; }  /* Stub - will replace with real implementation */
void COM_illegal(short opcode) { illegal(); }

/* All handlers now defined */

/* Additional missing handlers - quick stubs */
void COM_andi(short opcode) { cpu.pc += 2; }


/* Auto-generated stubs for missing handlers */
void COM_addi(short opcode) { cpu.pc += 2; }
void COM_addq(short opcode) { cpu.pc += 2; }
void COM_cmpa(short opcode) { cpu.pc += 2; }
void COM_cmpi(short opcode) { cpu.pc += 2; }
void COM_eori(short opcode) { cpu.pc += 2; }
void COM_subi(short opcode) { cpu.pc += 2; }
void COM_subq(short opcode) { cpu.pc += 2; }
