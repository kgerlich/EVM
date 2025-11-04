////////////////////////////////////////////////////////////////////////////////
// NAME: 			file.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						get open or save filename (common dialogs)
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <commdlg.h>

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
OPENFILENAME ofn;

////////////////////////////////////////////////////////////////////////////////
// NAME:				DWORD OpenAFile(HWND hWnd,LPSTR Title,LPSTR Filename,
//											LPSTR Filter,LPSTR Extension)
//
// DESCRIPTION:   open common dialog box GetOpenFilename
//
// PARAMETERS:    HWND hWnd:		  parent window
//                LPSTR Title:     caption text
//						LPSTR Filename:  on return contains filename
//						LPSTR Filter:    filter mask type of files
//						LPSTR Extension: default extension
//
// RETURNS:       DWORD: index into Filter, type of file OR -1 on failure
//
////////////////////////////////////////////////////////////////////////////////
DWORD OpenAFile(HWND hWnd,LPSTR Title,LPSTR Filename,LPSTR Filter,LPSTR Extension)
{
	// init OPENFILENAME struct
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=hWnd;
	ofn.hInstance=0;
	ofn.lpstrFilter=Filter;
	ofn.lpstrCustomFilter=NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=Filename;
	ofn.nMaxFile=260;
	ofn.lpstrFileTitle=NULL;
	ofn.nMaxFileTitle=0;
	ofn.lpstrInitialDir=0;
	ofn.lpstrTitle=Title;
	ofn.Flags=0;
	ofn.lpfnHook=NULL;
	ofn.lpstrDefExt=Extension;
	Filename[0]=0;
	if(GetOpenFileName(&ofn))
	{
		return ofn.nFilterIndex;
	}
	else
	{
		return -1;
	}
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				DWORD SaveAFile(HWND hWnd,LPSTR Title,LPSTR Filename,
//											LPSTR Filter,LPSTR Extension)
//
// DESCRIPTION:   open common dialog box GetSaveFilename
//
// PARAMETERS:    HWND hWnd:		  parent window
//                LPSTR Title:     caption text
//						LPSTR Filename:  on return contains filename
//						LPSTR Filter:    filter mask type of files
//						LPSTR Extension: default extension
//
// RETURNS:       DWORD: index into Filter, type of file OR -1 on failure
//
////////////////////////////////////////////////////////////////////////////////
DWORD SaveAFile(HWND hWnd,LPSTR Title,LPSTR Filename,LPSTR Filter,LPSTR Extension)
{

	// init OPENFILENAME struct
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=hWnd;
	ofn.hInstance=0;
	ofn.lpstrFilter=Filter;
	ofn.lpstrCustomFilter=NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=Filename;
	ofn.nMaxFile=260;
	ofn.lpstrFileTitle=NULL;
	ofn.nMaxFileTitle=0;
	ofn.lpstrInitialDir=0;
	ofn.lpstrTitle=Title;
	ofn.Flags=0;
	ofn.lpfnHook=NULL;
	ofn.lpstrDefExt=Extension;
	Filename[0]=0;
	if(GetSaveFileName(&ofn))
	{
		return ofn.nFilterIndex;
	}
	else
	{
		return -1;
	}
}
