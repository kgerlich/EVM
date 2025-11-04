////////////////////////////////////////////////////////////////////////////////
// NAME: 			stdll.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//                dynamic linking header for extension modules
// 					some stuff regarding the dynamic linking of peripheral modules
// 					that can additionally be written by any user
////////////////////////////////////////////////////////////////////////////////
#pragma pack(1) // packing size is now one byte

// pointer types to DLL required procedures
typedef BOOL              (*SETUPPROC)  (void); // Setup procedure
typedef void              (*INITPROC)   (void); // Init procedure
typedef void              (*EXITPROC)   (void); // Exit procedure
typedef void              (*RESETPROC)  (void); // Reset procedure
typedef void				  (*SIMPROC)	 (void); // Simulate procedure
typedef unsigned long     (*READPROC)   (long address,short size); // Read procedure
typedef void              (*WRITEPROC)  (long address,long data,short size); // Write procedure
// get index of module from address
typedef int               (*GETMODULE)  (unsigned long address);

// CPU structure
// the virtual CPUs register set
typedef struct tag_CPU
{
	union
	{
		short sr;    // Status Register
		char ccr,scr;  // condition code register
	}sregs;
	union      // address register access long,word or byte
	{
		long a[8];
		struct
		{
			short al;
			short ah;
		}worda[8];
		struct
		{
			char  all,alh; // all=A lower low alh=A lower high
			char  aul,auh; // aul=A upper low auh=A upper high
		}bytea[8];
	}aregs;
	union      // data register access long,word or byte
	{
		long d[8];
		struct
		{
			short dl;
			short dh;
		}wordd[8];
		struct
		{
			char  dll,dlh; // all=A lower low alh=A lower high
			char  dul,duh; // aul=A upper low auh=A upper high
		}byted[8];
	}dregs;
	long ssp,usp,msp; // Address registers, System SP, User SP, Master SP //
	long sfc,dfc,cacr,vbr,caar;
	long pc;
}CPU;


// the connection to CPU structure
typedef struct tag_CONN_TO_CPU
{
	char ipl;
	char VecNum;
	unsigned long bNonAutoVector;
};

// the DLLHDR structure
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
	volatile HWND hWnd;
	volatile struct tag_DLLHDR **pAllHdrs;
	volatile struct tag_CPU *pCpu;
	volatile GETMODULE GetModule;
};

// priorities
#define SIM_HIGHEST_PRIORITY 100
#define SIM_NORMAL_PRIORITY 50
#define SIM_LOWEST_PRIORITY 0

// MAKE_HDR macro
#define MAKE_HDR(a,b,c,d) BOOL Setup(void); \
								void Exit(void);\
								void Simulate(void);\
								void Init(void);\
								void Reset(void);\
								void Simulate(void);\
								unsigned long Read(long address,short size);\
								void Write(long address,long data,short size);\
								__declspec(dllexport) struct tag_DLLHDR  \
								DLLHDR={Setup,Init,Exit,Reset,Simulate,Read,Write,a,b,c,d,0L};

