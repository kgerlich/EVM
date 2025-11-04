////////////////////////////////////////////////////////////////////////////////
// NAME: 			port.h
// COMPILER:		Borland C++ 5.0 (OR Borland C++ 4.5)
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:   Header file
//						defines for module port.c
////////////////////////////////////////////////////////////////////////////////
extern void DisplayPorts(void); // update CPU status in window State of Simulation
extern void CreatePortsWindow(HANDLE hInstance,HANDLE hParentWindow);  // create asm dump window
extern HWND hPortDlg;
extern void DestroyPortDialog(void);
