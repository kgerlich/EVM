///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NAME: 			ststate.c
// COMPILER:		Borland C++ 5.0 (OR Borland C++ 4.5)
// AUTHOR:			Klaus P. Gerlicher
// DATE:				1996
// DESCRIPTION:
//						status window
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <commctrl.h>
       
HWND hStatusWnd=0;

void DisplayStatus(HWND hWnd,HINSTANCE hInstance)
{
RECT rcClient;
HLOCAL hloc;
LPINT lpParts;
int i, nWidth,nParts=2;

//   InitCommonControls();

	hStatusWnd=CreateWindowEx(
		 WS_EX_TOPMOST,                       // no extended styles
		 STATUSCLASSNAME,         // name of status window class
		 "Starting up...",          // no text when first created
		 WS_CHILD,                // creates a child window
		 0, 0, 0, 0,              // ignores size and position
		 hWnd,              // handle to parent window
		 NULL,       // child window identifier
		 hInstance,                   // handle to application instance
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

	// Free the array, and return.
	LocalUnlock(hloc);
	LocalFree(hloc);

	SendMessage(hStatusWnd, SB_SETTEXT, (WPARAM) 0,(LPARAM) "Waiting...");
	ShowWindow(hStatusWnd,SW_SHOW);
   UpdateWindow(hStatusWnd);
}

void SetStatusText(int nPart,LPSTR s)
{
	if(hStatusWnd)SendMessage(hStatusWnd,SB_SETTEXT,(WPARAM)nPart,(LPARAM)s);
}
