////////////////////////////////////////////////////////////////////////////////
// NAME: 			ststate.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						state window
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <commctrl.h>

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HWND hStatusWnd=0; // state window handle

////////////////////////////////////////////////////////////////////////////////
// NAME:				void DisplayStatus(HWND hWnd,HINSTANCE hInstance)
//
// DESCRIPTION:   display status bar at bottom of main window
//
// PARAMETERS:		HWND hWnd: 				handle of parent
//                HINSTANCE hInstance: instance of parent
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void DisplayStatus(HWND hWnd,HINSTANCE hInstance)
{
RECT rcClient;
HLOCAL hloc;
LPINT lpParts;
int i, nWidth,nParts=3;


	hStatusWnd = CreateWindowEx(
		  WS_EX_TOPMOST,           // topmost window
		  STATUSCLASSNAME,         // name of status window class
		  "",          				// no text when first created
		  WS_CHILD|WS_CLIPSIBLINGS,// creates a child window
		  0, 0, 0, 0,              // ignores size and position
		  hWnd,              		// handle to parent window
		  NULL,       					// child window identifier
		  hInstance,               // handle to application instance
		  NULL);                   // no window creation data


	// Get the coordinates of the parent window's client area.
	GetClientRect(hWnd, &rcClient);

	// Allocate an array for holding the right edge coordinates.
	hloc = LocalAlloc(LHND, sizeof(int) * nParts);
	lpParts = LocalLock(hloc);

	// Calculate the right edge coordinate for each part, and
	// copy the coordinates to the array.
	nWidth = rcClient.right / nParts;
	for (i = 0; i < nParts; i++) {
		 lpParts[i] = nWidth;
		 nWidth += nWidth;
	}

	// Tell the status window to create the window parts.
	SendMessage(hStatusWnd, SB_SETPARTS, (WPARAM) nParts,(LPARAM) lpParts);

	// Free the array
	LocalUnlock(hloc);
	LocalFree(hloc);

	ShowWindow(hStatusWnd,SW_SHOW);  // show the window
}

////////////////////////////////////////////////////////////////////////////////
// NAME:          void SetStatusText(int nPart,LPSTR s)
//
// DESCRIPTION:	set text in state window
//
// PARAMETERS:    LPSTR s: pointer to string to set
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void SetStatusText(int nPart,LPSTR s)
{
	if(hStatusWnd)
	{
		SendMessage(hStatusWnd,SB_SETTEXT,(WPARAM)nPart,(LPARAM)s);
		UpdateWindow(hStatusWnd);
	}
}
