////////////////////////////////////////////////////////////////////////////////
// NAME: 			stmain.h
// COMPILER:		Borland C++ 5.0 
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						includes for stmain.c
////////////////////////////////////////////////////////////////////////////////
extern char popupopen; // flag: is popup menu displayed
extern HWND ghWnd; // global window handle to main window
extern HINSTANCE ghInstance; // global instance handle
extern long nTimeForOps,nOpsPerSec,nMaxOpsPerSec; // performance mearsurement
extern HWND hStartup; // startup window handle
extern BOOL bRunning; // FLAG: simulation is running

