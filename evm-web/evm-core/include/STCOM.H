////////////////////////////////////////////////////////////////////////////////
// NAME: 			stcom.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						Header file
//						declarations for module stcom.c
////////////////////////////////////////////////////////////////////////////////

#include "STSTDDEF.H"

// some stuff regarding the dynamic linking of peripheral modules
// that can additionally be written by any user

// pointer to DLL required procedures
typedef BOOL              (*SETUPPROC) (void); // Setup procedure
typedef void              (*INITPROC) (void); // Init procedure
typedef void              (*EXITPROC)  (void); // Exit procedure
typedef void              (*RESETPROC) (void); // Reset procedure
typedef void				  (*SIMPROC)	(void); // Simulate procedure
typedef unsigned long     (*READPROC)  (long address,short size); // Read procedure
typedef void              (*WRITEPROC) (long address,long data,short size); // Write procedure

#pragma pack(1) // packing size is now one byte

//
typedef struct tag_CONN_TO_CPU
{
	char ipl;
	char VecNum;
   unsigned long bNonAutoVector;
   unsigned long nCycles;
};

typedef struct tag_DLLHDR
{
	const SETUPPROC SetupProc;
	const INITPROC  InitProc;
   const EXITPROC  ExitProc;
   const RESETPROC ResetProc;
	const SIMPROC   SimProc;
   const READPROC  ReadProc;
   const WRITEPROC WriteProc;
   const unsigned long nAddress;
   const unsigned long nSize;
   const unsigned short nPriority;
   const BOOL bCacheable;
	volatile struct tag_CONN_TO_CPU *pConnect;
   volatile void* hWnd;
   volatile struct tag_DLLHDR **pAllHdrs;
   volatile struct tag_CPU *pCpu;
};


extern struct tag_DLLHDR* pDllHdr[]; // pointer to DLL header with procedure addresses
extern int unknown;  // Opcode, Status Register
extern BOOL bStopped; // indicate processor stop via STOP instruction
extern unsigned long nIRQs;
extern char szPluginDir[];
extern struct tag_CONN_TO_CPU sConn_to_cpu;

extern struct tag_CPU
{
	union
	{
		short sr;    // Status Register
		char ccr,scr;  // condition code register
	}sregs;
	union      // access long,word or byte
	{
		long a[8];
		struct
		{
			short al;
			short ah;
		}worda[8];
		struct
		{
			char all,alh; // all=A lower low alh=A lower high
			char aul,auh; // aul=A upper low auh=A upper high
		}bytea[8];
	}aregs;
	union      // access long,word or byte
	{
		long d[8];
		struct
		{
			short dl;
			short dh;
		}wordd[8];
		struct
		{
			char dll,dlh; // all=A lower low alh=A lower high
			char dul,duh; // aul=A upper low auh=A upper high
		}byted[8];
	}dregs;
	long ssp,usp,msp; // Address registers, System SP, User SP , Master SP
	long sfc,dfc,cacr,vbr,caar;
	long pc;
}cpu; 
extern long pc,savepc;  // Program counter, temporary PC
extern long source,destination,result;
extern char vector;
extern long pcbefore;
extern char AddressError;
extern BOOL valid_tag[];
// function pointer array: EA calculation
extern long (*CommandMode[8])(char reg,char command,long destination,char size); 
										// reg: index to register
										// command: READ or WRITR
										// destination: index to destination
										// size: BYTE, WORD or DWORD

extern BOOL SetupSim(void);
extern void ExitSim(void);
extern BOOL ResetSim(void);
extern void Simulate68k(unsigned long lp); // do one opcode from current PC

