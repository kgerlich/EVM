////////////////////////////////////////////////////////////////////////////////
// NAME: 			ststartw.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						startup window
////////////////////////////////////////////////////////////////////////////////
#define STRICT // strict type checking
#define WIN32_LEAN_AND_MEAN // use only rudimentary windows API
#include <windows.h> // the window API
#include "stmain.h" // main module header
#include "stwin.rh"  // resources header

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HWND hStartup; // handle to startup window

////////////////////////////////////////////////////////////////////////////////
// NAME:				void ShowStartupDlg(void)
//
// DESCRIPTION:   display startup dialog window
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void ShowStartupDlg(void)
{
RECT wr;

	// load startup window resource and create window
	hStartup=CreateDialog(ghInstance,"STARTUP",NULL,NULL);
	// get size of startup window
	GetWindowRect(hStartup,&wr);
	// position window to center of screen (don't size dialog)
	SetWindowPos(hStartup,
					 HWND_TOPMOST,
					 (GetSystemMetrics(SM_CXSCREEN)/2)-((wr.right-wr.left)/4),
					 (GetSystemMetrics(SM_CYSCREEN)/2)-((wr.bottom-wr.top)/2),
					 0,
					 0,
					 SWP_NOSIZE);
	UpdateWindow(hStartup); // refresh
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void SetStartupText(LPSTR s)
//
// DESCRIPTION:   set string in startup dialog
//
// PARAMETERS:    LPSTR s: pointer to string to set
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void SetStartupText(LPSTR s)
{
	// set text
	SetDlgItemText(hStartup,IDC_START,s);
	UpdateWindow(hStartup); // refresh
}

////////////////////////////////////////////////////////////////////////////////
// NAME:          void SetStartupListboxText(LPSTR s)
//
// DESCRIPTION:	add a string to startup window listbox
//
// PARAMETERS:    LPSTR s: string to add
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void SetStartupListboxText(LPSTR s)
{
	// add string
	SendDlgItemMessage(hStartup,IDC_MODULES,LB_ADDSTRING,0,(LPARAM)s);
	UpdateWindow(hStartup); // refresh
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void CloseStartupDlg(void)
//
// DESCRIPTION:   close startup dialog window
//
// PARAMETERS:		none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void CloseStartupDlg(void)
{
	// destroy window
	DestroyWindow(hStartup);
}