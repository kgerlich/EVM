////////////////////////////////////////////////////////////////////////////////
// NAME: 			file.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						header for file.c
////////////////////////////////////////////////////////////////////////////////
extern DWORD OpenAFile(HWND hWnd,LPSTR Title,LPSTR Filename,LPSTR Filter,LPSTR Extension);
extern DWORD SaveAFile(HWND hWnd,LPSTR Title,LPSTR Filename,LPSTR Filter,LPSTR Extension);
