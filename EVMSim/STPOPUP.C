////////////////////////////////////////////////////////////////////////////////
// NAME: 			stpopup.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						display a context popup menu
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "stwin.rh"  // resources header

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HMENU hPopupMenu; // handle to popup menu

////////////////////////////////////////////////////////////////////////////////
// NAME:          void DisplayPopup(HWND hWnd)
//
// DESCRIPTION:	display popup (right mouse button) context menu
//
// PARAMETERS:    HWND hWnd: handle to parent window
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void DisplayPopup(HWND hWnd)
{
POINT pt;  // point structure for locating cursor

	hPopupMenu=CreatePopupMenu();     //  create a popup menu
	// append the necessary items
	AppendMenu(hPopupMenu,MF_STRING,IDM_RESET,"Reset simulation");
	AppendMenu(hPopupMenu,MF_STRING,IDM_QUIT,"Quit simulation");
	EnableMenuItem(hPopupMenu,IDM_QUIT,MF_ENABLED);
	EnableMenuItem(hPopupMenu,IDM_RESET,MF_ENABLED);
	// get cursor position for locating popup menu
	GetCursorPos(&pt);
	// track and process popup menu
	TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,
						pt.x,pt.y,0,hWnd,NULL);
	// destroy popup menu if open
	DestroyMenu(hPopupMenu);
}
