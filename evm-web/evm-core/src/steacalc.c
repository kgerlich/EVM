////////////////////////////////////////////////////////////////////////////////
// NAME: 			steacalc.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						effective address calculation for executing a 68K opcode
////////////////////////////////////////////////////////////////////////////////
#include "STMEM.H"   // high level memory handling
#include "STSTDDEF.H" // standard defines
#include "STCOM.H"   // global simulation stuff (variables etc...)

///////////////////////////////////////////////////////////////////////////
//              EA calculation
///////////////////////////////////////////////////////////////////////////
// long EAFunctionName(char reg,char command,long destination,char size)
// PARAMETER				DESCRIPTION
// =========				===========
// reg						register number
// command					read or write access
// destination				use only with write access = data to be written
// size						byte/word/long size of operand
///////////////////////////////////////////////////////////////////////////
// every function in this module is called from a subfunction in STCOM.C
// Its purpose is the decoding of the effective address of the operation
// via the related mode fields in the opcode and return the associated
// data or write it to the right address in memory.
///////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// NAME:				long DRD(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Data Register Direct (mode field = 000)
// 					example: CLR.W D0
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long DRD(char reg,char command,long destination,char size)
{
	if(command==READ) // do we read or write anything
	{
		switch(size)
		{
			case 0:
				return cpu.dregs.byted[reg].dll;
			case 1:
				return cpu.dregs.wordd[reg].dl;
			case 2:
				return cpu.dregs.d[reg];
		}
	}
	else
	{
		switch(size)
		{
			case 0:
				cpu.dregs.byted[reg].dll=(char)(destination);
				break;
			case 1:
				cpu.dregs.wordd[reg].dl=(short)(destination);
				break;
			case 2:
				cpu.dregs.d[reg]=destination;
				break;
		}
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long ARD(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Address Register Direct (mode field = 001)
// 					example: CLR.W A5
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long ARD(char reg,char command,long destination,char size)
{
	switch(command)
	{
		case READ:
			switch(size)
			{
				case 0:
					return cpu.aregs.bytea[reg].all;
				case 1:
					return cpu.aregs.worda[reg].al;
				case 2:
					return cpu.aregs.a[reg];
			}
			break;
		case WRITE:
			switch(size)
			{
				case 0:
					cpu.aregs.bytea[reg].all=(char)(destination);
					break;
				case 1:
					cpu.aregs.worda[reg].al=(short)(destination);
					break;
				case 2:
					cpu.aregs.a[reg]=destination;
					break;
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long ARI(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Address Register Indirect (mode field = 010)
// 					example: CLR.B (A1)
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long ARI(char reg,char command,long destination,char size)
{
	switch(size)
	{
		case 0:
			switch(command)
			{
				case READ:
					return (GETbyte(cpu.aregs.a[reg]));
				case WRITE:
					PUTbyte(cpu.aregs.a[reg],(char)destination);
					break;
			}
			break;
		case 1:
			switch(command)
			{
				case READ:
					return (GETword(cpu.aregs.a[reg]));
				case WRITE:
					PUTword(cpu.aregs.a[reg],(short)destination);
					break;
			}
			break;
		case 2:
			switch(command)
			{
				case READ:
					return (GETdword(cpu.aregs.a[reg]));
				case WRITE:
					PUTdword(cpu.aregs.a[reg],destination);
					break;
			}
			break;
  }
  return 0L;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long ARIPI(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Address Register Indirect
//						with Post Increment (mode field = 011)
// 					example: MOVE.W (A7)+,D0 (pop D0 from stack)
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long ARIPI(char reg,char command,long destination,char size)
{

	switch(command)
	{
		case READ:
			cpu.aregs.a[reg]=cpu.aregs.a[reg]+(1<<size);
			break;
		case WRITE:
			if(size==SIZE_BYTE)PUTbyte(cpu.aregs.a[reg],(char)destination);
			else if(size==SIZE_WORD)PUTword(cpu.aregs.a[reg],(short)destination);
			else PUTdword(cpu.aregs.a[reg],destination);
			cpu.aregs.a[reg]=cpu.aregs.a[reg]+(1<<size);
			break;
	}
	if(size==SIZE_BYTE)return (GETbyte(cpu.aregs.a[reg]-(1<<size)));
	else if(size==SIZE_WORD)return (GETword(cpu.aregs.a[reg]-(1<<size)));
	else if(size==SIZE_DWORD)return (GETdword(cpu.aregs.a[reg]-(1<<size)));
	else return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long ARIPD(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Address Register Indirect with
//						Pre Decrement (mode field = 100)
// 					example: MOVE.W D3,-(A7) (push D3 on stack)
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long ARIPD(char reg,char command,long destination,char size)
{
static long spc=0;

	switch(command)
	{
		case READ:
			cpu.aregs.a[reg]=cpu.aregs.a[reg]-(1<<size);
			spc=cpu.pc;
			break;
		case WRITE:
			// PATCH
			// if source and destination register are the same
			// this would cause wrong increment
			// so, check first if this is true
			// by comparing the PC before and after
			// maybe better yet, compare reg,size etc. too
			if(cpu.pc!=spc)
			{
				cpu.aregs.a[reg]=cpu.aregs.a[reg]-(1<<size);
			}
			if(size==SIZE_BYTE)PUTbyte(cpu.aregs.a[reg],(char)destination);
			else if(size==SIZE_WORD)PUTword(cpu.aregs.a[reg],(short)destination);
			else if(size==SIZE_DWORD)PUTdword(cpu.aregs.a[reg],destination);
			break;
	}
	switch(size)
	{
		case 0:
			return (GETbyte(cpu.aregs.a[reg]));
		case 1:
			return (GETword(cpu.aregs.a[reg]));
		case 2:
			return (GETdword(cpu.aregs.a[reg]));
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long ARID(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Address Register Indirect with
//						Displacement (mode field = 101)
// 					example: MOVE.W D0,$8(A3) (put D0 to EA A3+$8)
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long ARID(char reg,char command,long destination,char size)
{
	switch(size)
	{
		case 0:
			switch(command)
			{
				case READ:
					cpu.pc=cpu.pc+2;
					return (GETbyte(cpu.aregs.a[reg]+(long)GETword(cpu.pc-2)));
				case WRITE:
					cpu.pc=cpu.pc+2;
					PUTbyte(cpu.aregs.a[reg]+(long)GETword(cpu.pc-2),(short)destination);
					break;
			}
		break;
		case 1:
			switch(command)
			{
				case READ:
					cpu.pc=cpu.pc+2;
					return (GETword(cpu.aregs.a[reg]+(long)(GETword(cpu.pc-2))));
				case WRITE:
					cpu.pc=cpu.pc+2;
					PUTword(cpu.aregs.a[reg]+(long)(GETword(cpu.pc-2)),(short)destination);
					break;
			}
		break;
		case 2:
			switch(command)
			{
				case READ:
					cpu.pc=cpu.pc+2;
					return (GETdword(cpu.aregs.a[reg]+(long)(GETword(cpu.pc-2))));
				case WRITE:
					cpu.pc=cpu.pc+2;
					PUTdword(cpu.aregs.a[reg]+(long)(GETword(cpu.pc-2)),destination);
					break;
			}
		break;

	}
	return 0L;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long ARII(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Address Register Indirect Indexed (mode field = 110)
// 					example: MOVE.W D1,$4(A0,D0) (put D1 into EA A0+D0+$4)
//									68020 extended addressing modes
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long ARII(char reg,char command,long destination,char size)
{
static long indexreg,bd,od,ea;
static unsigned short extension,scale;

	extension=GETword(cpu.pc);  // 16 bit extension
	switch((extension>>9)&0x0003)  // retreive scaling of index reg
	{
		case 0:
			scale=1;
			break;
		case 1:
			scale=2;
			break;
		case 2:
			scale=4;
			break;
		case 4:
			scale=8;
			break;
	}
	if((extension&0x0100)==0) // simple 68000 extension
	{
		switch(extension&0x8800)
		{
			case 0x0000: // index=Dn index size=word
				indexreg=(long)(cpu.dregs.d[(extension&0x7000)>>12]&0x0000FFFF);
				break;
			case 0x8000: // index=An index size=word
				indexreg=(long)(cpu.aregs.a[(extension&0x7000)>>12]&0x0000FFFF);
				break;
			case 0x0800: // index=Dn index size=long
				indexreg=cpu.dregs.d[(extension&0x7000)>>12];
				break;
			case 0x8800: // index=An index size=long
				indexreg=cpu.aregs.a[(extension&0x7000)>>12];
				break;
		}

		indexreg=indexreg*scale; // scale factor
		switch(size)
		{
			case 0:
				switch(command)
				{
					case READ:
						cpu.pc=cpu.pc+2;
						return ( GETbyte( cpu.aregs.a[reg]+ (short)(extension&0x00ff) +indexreg) );
					case WRITE:
						cpu.pc=cpu.pc+2;
						PUTbyte(cpu.aregs.a[reg]+(short)(extension&0x00ff)+indexreg,(char)destination);
						break;
				}
			break;
			case 1:
				switch(command)
				{
					case READ:
						cpu.pc=cpu.pc+2;
						return (GETword(cpu.aregs.a[reg]+(short)(extension&0x00ff)+indexreg));
					case WRITE:
						cpu.pc=cpu.pc+2;
						PUTword(cpu.aregs.a[reg]+(short)(extension&0x00ff)+indexreg,(short)destination);
						break;
				}
			break;
			case 2:
				switch(command)
				{
					case READ:
						cpu.pc=cpu.pc+2;
						return (GETdword(cpu.aregs.a[reg]+(short)(extension&0x00ff)+indexreg));
					case WRITE:
						cpu.pc=cpu.pc+2;
						PUTdword(cpu.aregs.a[reg]+(short)(extension&0x00ff)+indexreg,destination);
						break;
				}
			break;
		}
	}
	else  // extended 68020 operation
	{
		if(!(extension&0x0040)) // do not suppress index register
		{
			switch(extension&0x8800)
			{
				case 0x0000: // index=Dn index size=word
					indexreg=(long)(cpu.dregs.d[(extension&0x7000)>>12]&0x0000FFFF);
					break;
				case 0x8000: // index=An index size=word
					indexreg=(long)(cpu.aregs.a[(extension&0x7000)>>12]&0x0000FFFF);
					break;
				case 0x0800: // index=Dn index size=long
					indexreg=cpu.dregs.d[(extension&0x7000)>>12];
					break;
				case 0x8800: // index=An index size=long
					indexreg=cpu.aregs.a[(extension&0x7000)>>12];
					break;
			}
			indexreg=indexreg*scale; // scale factor
		}
		else indexreg=0;

      switch((extension>>7)&0x0001) // BS field
      {
      	case 0: // base register added
				ea=cpu.aregs.a[reg];
	        	break;
         case 1: // base register suppressed
				ea=0;
         	break;
      }

      switch((extension>>4)&0x0003) // base displacement size
      {
         case 0: // reserved
         case 1: // null base displacement
            bd=0;
            break;
         case 2: // base displacement is one word
            cpu.pc+=4;
            bd=(long)GETword(cpu.pc-2);
            break;
         case 3: // base displacement is two words
            cpu.pc+=6;
            bd=GETdword(cpu.pc-4);
            break;
      }

      switch((extension>>6)&0x0001) // IS field
      {
      	case 0: // evaluate and add index
            switch(extension&0x0007) // I/IS field
            {
					case 0: // no memory indirection
						ea=ea+indexreg+bd;
						break;
					case 1: // indirect pre-indexed with null displacement
						ea=GETdword(ea+bd+indexreg);
						break;
					case 2: // indirect pre-indexed with word displacement
						od=(long)GETword(cpu.pc);
						cpu.pc+=2;
						ea=GETdword(ea+bd+indexreg)+od;
						break;
					case 3: // indirect pre-indexed with long displacement
						od=GETdword(cpu.pc);
						cpu.pc+=4;
						ea=GETdword(ea+bd+indexreg)+od;
						break;
					case 4: // reserved
						break;
					case 5: // indirect post-indexed with null displacement
						ea=GETdword(ea+bd)+indexreg;
						break;
					case 6: // indirect post-indexed with word displacement
						od=(long)GETword(cpu.pc);
						cpu.pc+=2;
						ea=GETdword(ea+bd)+indexreg+od;
						break;
					case 7: // indirect post-indexed with long displacement
						od=GETdword(cpu.pc);
						cpu.pc+=4;
						ea=GETdword(ea+bd)+indexreg+od;
						break;
				}
				break;
			case 1: // suppress index
				switch(extension&0x0007) // I/IS field
				{
					case 0: // no memory indirection
						ea=ea+bd;
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
				break;
		}
		switch(size)
		{
			case 0:
				switch(command)
				{
					case READ:
						return GETbyte(ea);
					case WRITE:
						PUTbyte(ea,(char)destination);
						break;
				}
			break;
			case 1:
				switch(command)
				{
					case READ:
						return GETword(ea);
					case WRITE:
						PUTword(ea,(short)destination);
						break;
				}
			break;
			case 2:
				switch(command)
				{
					case READ:
						return GETdword(ea);
					case WRITE:
						PUTdword(ea,destination);
						break;
				}
			break;
		}
	}
	return 0L;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long MISC(char reg,char command,long destination,char size)
//
// DESCRIPTION:   Miscellaneous addressing modes (mode field = 111)
//						PC relative, absolute
// 					example: MOVE.L ($8,PC),$800000
//
// PARAMETERS:    (see at top of module)
//
// RETURNS:       (see at top of module)
//
////////////////////////////////////////////////////////////////////////////////
long MISC(char reg,char command,long destination,char size)
{
long t1,t2;

	switch(reg)
	{
		case 0: // absolute short
			// example: MOVE.W D0,$7300 (put D0 to EA $7300)
			switch(command)
			{
				case READ:
					switch(size)
               {
               	case 0:
							cpu.pc=cpu.pc+2;
							return GETbyte((long)GETword(cpu.pc-2));
               	case 1:
							cpu.pc=cpu.pc+2;
							return GETword((long)GETword(cpu.pc-2));
               	case 2:
							cpu.pc=cpu.pc+2;
							return GETdword((long)GETword(cpu.pc-2));
               }
				case WRITE:
            	switch(size)
               {
               	case 0:
							cpu.pc=cpu.pc+2;
							PUTbyte((long)GETword(cpu.pc-2),(char)destination);
                     break;
               	case 1:
							cpu.pc=cpu.pc+2;
							PUTword((long)GETword(cpu.pc-2),(short)destination);
                     break;
               	case 2:
							cpu.pc=cpu.pc+2;
							PUTdword((long)GETword(cpu.pc-2),destination);
                     break;
               }
					break;
			}
			break;
		case 1: // absolute long 
			// example: MOVE.W D0,$00447300 (put D0 to EA $00447300) 
			switch(size)
			{
				case 0:
					switch(command)
					{
						case READ:
							cpu.pc=cpu.pc+4;
							return GETbyte(GETdword(cpu.pc-4));
						case WRITE:
							cpu.pc=cpu.pc+4;
							PUTbyte(GETdword(cpu.pc-4),(char)destination);
							return destination;
					}
					break;
				case 1:
					switch(command)
					{
						case READ:
							cpu.pc=cpu.pc+4;
							return (GETword(GETdword(cpu.pc-4)));
						case WRITE:
							cpu.pc=cpu.pc+4;
							PUTword(GETdword(cpu.pc-4),(short)destination);
							return destination;
					}
					break;
				case 2:
					switch(command)
					{
						case READ:
							cpu.pc=cpu.pc+4;
							return (GETdword(GETdword(cpu.pc-4)));
						case WRITE:
							cpu.pc=cpu.pc+4;
							PUTdword(GETdword(cpu.pc-4),destination);
							return destination;
					}
					break;
			}
		case 2: // PC relative with displacement 
			// example: MOVE.W D4,$20(PC) (put D4 to EA PC+$20) 
			switch(command)
			{
				case READ:
					cpu.pc=cpu.pc+2;
               switch(size)
               {
               	case 0:
							return GETbyte((long)(GETword(cpu.pc-2))+(long)(cpu.pc)-2);
                  case 1:
							return GETword((long)(GETword(cpu.pc-2))+(long)(cpu.pc)-2);
                  case 2:
							return GETdword((long)(GETword(cpu.pc-2))+(long)(cpu.pc)-2);
               }
               break;
				case WRITE:
					switch(size)
               {
               	case 0:
							PUTbyte((long)(GETword(cpu.pc-2))+(long)(cpu.pc)-2,(char)destination);
                     break;
                  case 1:
							PUTword((long)(GETword(cpu.pc-2))+(long)(cpu.pc)-2,(short)destination);
                     break;
                  case 2:
							PUTdword((long)(GETword(cpu.pc-2))+(long)(cpu.pc)-2,destination);
                     break;
               }
					break;
			}
			break;
		case 3: // PC relative with displacement and index 
			// example: MOVE.W D4,$20(PC,D1.W) (put D4 to EA PC+D1.W+$20) 
			switch(command)
			{
				case READ:
					cpu.pc=cpu.pc+2;
					t1=(GETword(cpu.pc-2)&0x00FF);
					t2=cpu.dregs.d[(GETword(cpu.pc-2)>>12)&0x0007];
					if((GETword(cpu.pc-2)&0x8000)==0)t2=t2&0x0000FFFF;
					if(size==SIZE_BYTE)return GETbyte((long)(t1+(long)(cpu.pc)-2+t2));
					else if(size==SIZE_WORD)return GETword((long)(t1+(long)(cpu.pc)-2+t2));
					else if(size==SIZE_DWORD)return GETdword((long)(t1+(long)(cpu.pc)-2+t2));
				case WRITE:
					break;
			}
			break;
		case 4: // immediate 
			// example: MOVE.W #$08,D0 (put #$08 in D0) 
			switch(size)
			{
				case 0:
					switch(command)
					{
						case READ:
							cpu.pc=cpu.pc+2;
							return (GETword(cpu.pc-2)&0x00FF);
						case WRITE:
							break;
					}
					break;
				case 1:
					switch(command)
					{
						case READ:
							cpu.pc=cpu.pc+2;
							return (GETword(cpu.pc-2));
						case WRITE:
							break;
					}
				break;
				case 2:
					switch(command)
					{
						case READ:
							cpu.pc=cpu.pc+4;
							return (GETdword(cpu.pc-4));
						case WRITE:
							break;
					}
				break;
			}
	}
	return 0L; // don't care what returns 
}
