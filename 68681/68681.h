////////////////////////////////////////////////////////////////////////////////
// NAME: 			68681.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:   Header file
//						for module 68681.c
////////////////////////////////////////////////////////////////////////////////
extern char *gszAppName; // app name
extern HWND hParentWnd; // handle to parent

// 68681 registers
extern BYTE MR1A,MR2A,SRA,CSRA,CRA,RBA,TBA,IPCR,ACR,ISR,IMR,CTUR,CTLR,
		 MR1B,MR2B,SRB,CSRB,CRB,RBB,TBB,IVR,IPU,OPCR,OPR;



// extern funcs
extern BYTE Read68681(long address,short size);
extern void Write68681(long address,long data,int size);
extern void Simulate68681(void);

