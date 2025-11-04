////////////////////////////////////////////////////////////////////////////////
// NAME: 			popup.c
// COMPILER:		Borland C++ 5.0 (OR Borland C++ 4.5)
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						display a context popup menu
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "68681.rh"

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HMENU hPopupMenu; // handle to popup menu
char popupopen=0;

////////////////////////////////////////////////////////////////////////////////
// NAME:				void DisplayPopup(HWND hWnd)
//
// DESCRIPTION:   display popup menu on right mouse button down
//
// PARAMETERS:    HWND hWnd: parent window handle
//
// RETURNS:			none
//
////////////////////////////////////////////////////////////////////////////////
void DisplayPopup(HWND hWnd)
{
POINT pt;  // point structure for locating cursor

	hPopupMenu=CreatePopupMenu();     //  create a popup menu
	// append the necessary items
	AppendMenu(hPopupMenu,MF_STRING,IDM_CHOOSEFONT,"Choose Terminal Font...");
	// get cursor position for locating popup menu
	GetCursorPos(&pt);
	// track and process popup menu
	TrackPopupMenu(hPopupMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,
								pt.x,pt.y,0,hWnd,NULL);
	// destroy popup menu if open
	if(popupopen==1)DestroyMenu(hPopupMenu);
}
