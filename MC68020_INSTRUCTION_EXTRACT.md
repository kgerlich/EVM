# MC68020 Instruction Handler Extraction - EVM Simulator
## Source: /home/kgerlich/dev/EVM/EVMSim/Stcom.c

---

## INSTRUCTION 1: COM_ori - OR Immediate to EA
**Location:** Lines 106-170
**Function:** Bitwise OR of immediate value with effective address

```c
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
```

---

## INSTRUCTION 2: COM_andi - AND Immediate to EA
**Location:** Lines 176-229
**Function:** Bitwise AND of immediate value with effective address

```c
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
```

---

## INSTRUCTION 3: COM_subi - SUB Immediate to EA
**Location:** Lines 236-297
**Function:** Subtract immediate value from effective address

```c
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
```

---

## INSTRUCTION 4: COM_addi - ADD Immediate to EA
**Location:** Lines 303-364
**Function:** Add immediate value to effective address

```c
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
```

---

## INSTRUCTION 5: COM_eori - EOR Immediate to EA
**Location:** Lines 370-423
**Function:** Exclusive OR of immediate value with effective address

```c
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
```

---

## INSTRUCTION 6: COM_cmpi - COMPARE Immediate with EA
**Location:** Lines 429-480
**Function:** Compare immediate value with effective address (subtract and set flags)

```c
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
```

---

## INSTRUCTION 7: COM_addq - ADD Quick (3-bit immediate)
**Location:** Lines 3096-3142
**Function:** Add quick value (1-8, encoded in opcode) to effective address

```c
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
```

---

## INSTRUCTION 8: COM_subq - SUBTRACT Quick (3-bit immediate)
**Location:** Lines 3148-3194
**Function:** Subtract quick value (1-8, encoded in opcode) from effective address

```c
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
```

---

## INSTRUCTION 9: COM_cmpa - COMPARE Address Register
**Location:** Lines 4394-4423
**Function:** Compare address register with effective address (subtract and set flags, doesn't alter register)

```c
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
```

---

## REQUIRED HELPER FUNCTIONS

### From STFLAGS.C (Lines 107-129)

```c
// DESCRIPTION: generate carry from source, destination and result
char _fastcall gen_carry(long s,long d,long r)
{
	if( (s<0 && !(d<0)) || (r<0 && !(d<0)) || (s<0 && r<0) )return 1;
	else return 0;
}

// DESCRIPTION: generate overflow from source, destination and result
char _fastcall gen_over(long s,long d,long r)
{
	if( (!(s<0) && (d<0) && !(r<0)) || ((s<0) && !(d<0) && (r<0)) )return 1;
	else return 0;
}
```

---

## REQUIRED FLAG MANIPULATION MACROS

### From macros.h

```c
#define CARRY1    cpu.sregs.ccr=cpu.sregs.ccr|0x01
#define CARRY0    cpu.sregs.ccr=cpu.sregs.ccr&(~0x01)

#define ZERO1     cpu.sregs.ccr=cpu.sregs.ccr|0x04
#define ZERO0     cpu.sregs.ccr=cpu.sregs.ccr&(~0x04)

#define OVER1     cpu.sregs.ccr=cpu.sregs.ccr|0x02
#define OVER0     cpu.sregs.ccr=cpu.sregs.ccr&(~0x02)

#define XTEND1    cpu.sregs.ccr=cpu.sregs.ccr|0x10
#define XTEND0    cpu.sregs.ccr=cpu.sregs.ccr&(~0x10)

#define NEG1      cpu.sregs.ccr=cpu.sregs.ccr|0x08
#define NEG0      cpu.sregs.ccr=cpu.sregs.ccr&(~0x08)
```

---

## REQUIRED FLAG FUNCTION DECLARATIONS

### From STFLAGS.H

```c
extern void _fastcall setcarry(char flag); // set carry to boolean flag
extern void _fastcall setover(char flag);  // set overflow to boolean flag
extern void _fastcall setxtend(char flag); // set extend to boolean flag
extern char _fastcall gen_carry(long s,long d,long r);
extern char _fastcall gen_over(long s,long d,long r);
```

---

## KEY DATA STRUCTURES & GLOBALS

### From Stcom.c

```c
// Union for opcode decoding (lines 48-71)
union {
	unsigned short o;
	struct {
		unsigned regsrc   : 3;   // 0x0007
		unsigned modesrc  : 3;   // 0x0038
		unsigned modedest : 3;   // 0x01C0
		unsigned regdest  : 3;   // 0x0E00
		unsigned group    : 4;   // 0xF000
	}general;
	struct {
		unsigned regsrc   : 3;   // 0x0007
		unsigned modesrc  : 3;   // 0x0038
		unsigned size     : 2;   // 0x00C0
		unsigned reserved : 4;   // 0x0F00
		unsigned group    : 4;   // 0xF000
	}special;
}of; // Opcode

// Working register structure (lines 78-81)
struct tag_work {
	long source,destination,result;
}work;

// Other globals (lines 83-86)
long savepc, spc;  // Temporary PC
char AddressError=0;

// EA calculation function pointer array (lines 89-90)
long (*CommandMode[8])(char reg, char command, long destination, char size)=
	{DRD, ARD, ARI, ARIPI, ARIPD, ARID, ARII, MISC};
```

---

## MEMORY ACCESS FUNCTIONS (Declared in STMEM.H, implemented in STMEM.C)

```c
extern short _fastcall GETword(unsigned long address);
extern long _fastcall GETdword(unsigned long address);
extern void _fastcall PUTbyte(unsigned long address, char data);
extern void _fastcall PUTword(unsigned long address, short data);
extern void _fastcall PUTdword(unsigned long address, long data);
```

---

## CACHEFUNCTION MACRO

```c
#define CACHEFUNCTION(p)  // Currently does nothing, may be used for optimization
```

---

## NOTES FOR WASM CONVERSION

1. **Flag Manipulation**: All flag setting uses bitwise operations on `cpu.sregs.ccr` (Condition Code Register, bits 0-4)
   - Bit 0: Carry (C)
   - Bit 1: Overflow (V)
   - Bit 2: Zero (Z)
   - Bit 3: Negative (N)
   - Bit 4: Extend (X)

2. **Carry/Overflow Generation**: `gen_carry()` and `gen_over()` perform sign-based analysis of source, destination, and result

3. **CommandMode Array**: Dispatches to 8 addressing mode handlers (DRD, ARD, ARI, etc.) with parameters:
   - `reg`: register number
   - `command`: 0=read, 1=write
   - `destination`: value to write (when command==1)
   - `size`: 0=byte, 1=word, 2=long

4. **Memory Access**: GETword/GETdword handle endian conversion internally (68020 is big-endian, x86 is little-endian)

5. **PC Management**: `cpu.pc` is advanced by instruction handlers; `spc` saves PC across CommandMode() calls that might alter it

6. **Size Encoding in Opcode**:
   - `of.special.size` field: 0=byte, 1=word, 2=long
   - Used in most immediate instruction variants

