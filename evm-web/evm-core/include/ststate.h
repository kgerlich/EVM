////////////////////////////////////////////////////////////////////////////////
// NAME: 			ststate.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						status window header ststate.c
////////////////////////////////////////////////////////////////////////////////
extern HWND hStatusWnd;
extern void DisplayStatus(HWND hWnd,HINSTANCE hInstance);
extern void SetStatusText(int nPart,LPSTR s);
