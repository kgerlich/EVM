////////////////////////////////////////////////////////////////////////////////
// NAME: 			vt100.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				created: 4/1997
//						changed: 03/07/1997 did some cleanups
// DESCRIPTION:
//						simulate a QUME VT101 terminal
//						see various chapters from
//						QVT101 terminal BENUTZER HANDBUCH
//						QUME GmbH, Düsseldorf, copyright Januar 1985
//						used to get the protocol requirements right
////////////////////////////////////////////////////////////////////////////////
#define STRICT          // do STRICT type checking
#define WIN32_LEAN_AND_MEAN // only necessary windows APIs
#include <windows.h> 	// the WINDOWS API
#include <commdlg.h>    // common dialogs (CHOOSEFONT)
#include <stdio.h>
#include "68681.h"
#include "popup.h"
#include "68681.rh"

#define MAXLINES 24		// terminal has 24 lines
#define MAXCOLUMNS 80   // terminal has 80 columns

#define NORMAL 0        // state machine modes
								// for processing each call to terminal emulation
#define SPECIAL 1       // STATE: special character received
#define ESCAPE 2        // STATE: escape character received
#define SETABSPOS 3     // STATE: set absolute position
#define VT101 4
#define BLINK 5

////////////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////////////
long FAR PASCAL TerminalWndProc(HWND hWnd,UINT iMessage,WORD wParam,long lParam);
long FAR PASCAL NewWndProc(HWND hWnd,UINT iMessage,WORD wParam,long lParam);
DWORD CursorUpdateThreadFunc(LPDWORD lpdwParam);

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
short TerminalMode=NORMAL;						// state of state machine
unsigned short TerminalX=0,TerminalY=0;   // current terminal positions
unsigned short OldX=0,OldY=0;             // update terminal positions
long xChar, yChar;              				// character sizes
HLOCAL hScreen,hAttribut;						// handles to local display
														// memory/attribute memory
short nEscapeParameter=0;          			// count of ESCAPE params
BOOL ShiftOn=TRUE,bCursorBlink=FALSE;     // flags: SHIFT/BLINK
HFONT hOldFont=0,hDefaultFont=0;				// for restauration:
														// oldfont/ handle the
														// fixed terminal font
DWORD dwThreadId;									// thread variables
HANDLE hThread;
BOOL bThreadRunning;                      // FLAG: second thread running
BOOL bUpdateCritical;							// SEMAPHORE: don't update when set
char *gpScreen,*gpAttribut;					// pointer to screen/attr memory
HWND ghWnd=0;										// global window handle
UINT nPopupTimerID;								// popup timer ID
BOOL bRButtonUp=TRUE;                     // FLAG: right mouse button up
WNDPROC OldWndProc;								//

////////////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////////////
// timer proc for display update
void CALLBACK TimerProc(HWND hWnd,UINT msg,UINT idEvent,DWORD dwTime);
void UpdateCursor(void); // forward prototype of UpdateCursor()
DWORD TimerThreadFunc(DWORD lpParam);

////////////////////////////////////////////////////////////////////////////////
// NAME:				void UpdateTerminal(void)
//
// DESCRIPTION:   update the terminal window at current cursor position
//
// PARAMETERS:		none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void UpdateTerminal(void)
{
char temp; 		// temporary character container
HDC hDC=GetDC(ghWnd);
	SelectObject(hDC,hDefaultFont);
	// set critical section flag for second (cursor) thread
	bUpdateCritical=TRUE;
	// set fore- and background colors
	SetTextColor(hDC,RGB(0,255,0));
	SetBkColor(hDC,RGB(0,0,0));
	// get character at current position
	temp=*(gpScreen+MAXCOLUMNS*TerminalY+TerminalX);
	hOldFont=SelectObject(hDC,hDefaultFont);
	// and refresh that position again
	TextOut(hDC,xChar*TerminalX,yChar*TerminalY,&temp,1);
	// reset critical section flag for second (cursor) thread
	bUpdateCritical=FALSE;
	SelectObject(hDC,hOldFont);
	ReleaseDC(ghWnd,hDC);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void UpdateWholeTerminal(HDC hDC)
//
// DESCRIPTION:   update whole terminal line by line
//
// PARAMETERS:    HDC hDC: device context of window
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void UpdateWholeTerminal(HDC hDC) // DC is provided from caller,
											 // since function is used from WM_PAINT logic
{
short i,j; // line and column of refresh
char temp; // temporary character container

	SelectObject(hDC,hDefaultFont);
	bUpdateCritical=TRUE;   // set critical section flag for second (cursor) thread
	for(i=0;i<MAXLINES;i++)						// for each line in display
	{
		for(j=0;j<MAXCOLUMNS;j++)           // process each column
		{
			// attribute at current position? 1==reverse video 0==normal video
			if(*(gpAttribut+MAXCOLUMNS*i+j)==1 &&
				bCursorBlink &&
				*(gpScreen+MAXCOLUMNS*i+j)!=' ')
			{
				// set the colors according to attribute
				SetTextColor(hDC,RGB(0,0,0));
				SetBkColor(hDC,RGB(0,255,0));
			}
			else
			{
				SetTextColor(hDC,RGB(0,255,0));
				SetBkColor(hDC,RGB(0,0,0));
			}
			// get current character at current position
			temp=*(gpScreen+MAXCOLUMNS*i+j);
			// refresh character with corresponding attributes
			TextOut(hDC,j*xChar,yChar*i,&temp,1);

		}
	}
	SelectObject(hDC,hOldFont);
	// reset critical section flag for second (cursor) thread
	bUpdateCritical=FALSE;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				void ClearTerminal(void)
//
// DESCRIPTION:   clear whole terminal window
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void ClearTerminal(void)
{
int i;  		// index
HDC hDC=GetDC(ghWnd);


	// set critical section flag for second (cursor) thread
	bUpdateCritical=TRUE;
	for(i=0;i<MAXLINES*MAXCOLUMNS;i++)
	{
		*(gpScreen+i)=' '; // space out the display mem
		*(gpAttribut+i)=0; // space out the attribute mem
	}
	UpdateWholeTerminal(hDC);

	ReleaseDC(ghWnd,hDC);
	// set critical section flag for second (cursor) thread
	bUpdateCritical=FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void DeleteLine(void)
//
// DESCRIPTION:   delete line at current cursor position
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void DeleteLine(void)
{
short i,j;
HDC hDC;

	// move up one all lines from current line to pre-last line
	for(i=TerminalY;i<MAXLINES-1;i++)
	{
		for(j=0;j<MAXCOLUMNS;j++)
		{
			*(gpScreen+MAXCOLUMNS*i+j)=*(gpScreen+MAXCOLUMNS*(i+1)+j);
		}
	}
	// delete last line
	for(j=0;j<MAXCOLUMNS;j++)
	{
		*(gpScreen+(MAXCOLUMNS*(MAXLINES-1))+j)=' ';
	}
	hDC=GetDC(ghWnd);
	UpdateWholeTerminal(hDC);
	ReleaseDC(ghWnd,hDC);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void InsertLine(void)
//
// DESCRIPTION:   insert a line at current cursor position
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void InsertLine(void)
{
short i,j;
HDC hDC;

	for(i=MAXLINES-2;i>=TerminalY;i--)
	{
		for(j=0;j<MAXCOLUMNS;j++)
		{
			*(gpScreen+MAXCOLUMNS*(i+1)+j)=*(gpScreen+MAXCOLUMNS*i+j);
		}
	}
	for(j=0;j<MAXCOLUMNS;j++)
	{
		*(gpScreen+MAXCOLUMNS*TerminalY+j)=' ';
	}
	for(i=MAXLINES-2;i>=TerminalY;i--)
	{
		for(j=0;j<MAXCOLUMNS;j++)
		{
			*(gpAttribut+MAXCOLUMNS*(i+1)+j)=*(gpAttribut+MAXCOLUMNS*i+j);
		}
	}
	for(j=0;j<MAXCOLUMNS;j++)
	{
		*(gpAttribut+MAXCOLUMNS*TerminalY+j)=0;
	}
	hDC=GetDC(ghWnd);
	UpdateWholeTerminal(hDC);
	ReleaseDC(ghWnd,hDC);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void SetTerminalAttribut(short x,short y,char data)
//
// DESCRIPTION:   set terminal attribute at position (x,y)
//
// PARAMETERS:    short x,y: position in window
//
// RETURNS:			none
//
////////////////////////////////////////////////////////////////////////////////
void SetTerminalAttribut(short x,short y,char data)
{
	*(gpAttribut+MAXCOLUMNS*y+x)=data;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				void ScrollTerminal(short direction)
//
// DESCRIPTION:   scroll terminal window up/down
//						only up scrolling is implemented and need
//
// PARAMETERS:		short direction: 0=UP scrolling
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void ScrollTerminal(short direction)
{
short i,j;
HDC hDC;

	switch(direction)
	{
		case 0: // UP
			// copy first MAXLINES lines
			for(i=0;i<MAXLINES-1;i++)
			{
				for(j=0;j<MAXCOLUMNS;j++)
				{
					*(gpScreen+MAXCOLUMNS*i+j)=*(gpScreen+MAXCOLUMNS*(i+1)+j);
				}
			}
			// delete last line
			for(j=0;j<MAXCOLUMNS;j++)
			{
				*(gpScreen+(MAXCOLUMNS*(MAXLINES-1))+j)=' ';
			}
			TerminalY=MAXLINES-1;
			break;
	}
	hDC=GetDC(ghWnd);
	UpdateWholeTerminal(hDC);
	ReleaseDC(ghWnd,hDC);
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				void SimulateTerminal(unsigned char data)
//
// DESCRIPTION:   simulate QUME type terminal
//
// PARAMETERS:    unsigned char data: data received from serial input
//
// RETURNS:			none
//
////////////////////////////////////////////////////////////////////////////////
void SimulateTerminal(unsigned char data)
{
	// set critical section flag for second (cursor) thread
	bUpdateCritical=TRUE;

	// is data a terminal control character?
	if(data<0x20 && TerminalMode==NORMAL)TerminalMode=SPECIAL;

	// which terminal mode is selected
	switch(TerminalMode)
	{
		// normal character data
		case NORMAL:
			// if terminal is in SHIFT mode, convert data to upper
			if(ShiftOn)data=(unsigned char)toupper(data);
			// write the data to display memory
			*(gpScreen+MAXCOLUMNS*TerminalY+TerminalX)=data;
			UpdateTerminal();  // update only current position
			// not end of display? -> increment cursor position
			if(TerminalX<MAXCOLUMNS)TerminalX++;
			break;
		// control sequences
		case SPECIAL:
			switch(data)
			{
				case 0x04: // FORWARD
					TerminalMode=NORMAL;
					if(TerminalX<MAXCOLUMNS)TerminalX++;
					break;
				case 0x07: // BELL
					TerminalMode=NORMAL;
					MessageBeep(-1);
					break;
				case 0x08: // BS
					if(TerminalX>0)
					{
						TerminalX--;
					}
					TerminalMode=NORMAL;
					break;
				case 0x0e: // SO
					ShiftOn=FALSE;
					TerminalMode=NORMAL;
					break;
				case 0x0f: // sI
					ShiftOn=TRUE;
					TerminalMode=NORMAL;
					break;
				case 0x0a: // LF
					TerminalMode=NORMAL;
					if((TerminalY)<MAXLINES-1)
					{
						TerminalY++;
					}
					else ScrollTerminal(0);
					break;
				case 0x0d:  // CR
					TerminalMode=NORMAL;
					TerminalX=0;
					if((TerminalY)<MAXLINES-1)
					{
						TerminalY++;
					}
					else ScrollTerminal(0);
					break;
				case 0x1a: // HOME
					ClearTerminal();
					TerminalX=TerminalY=0;
					TerminalMode=NORMAL;
					break;
				case 0x1b: // ESCAPE
					TerminalMode=ESCAPE;
					break;
			}
			break;
		case ESCAPE:
			switch(data)
			{
				case '|':
					TerminalMode=VT101;
					nEscapeParameter=0;
					break;
				case '=':
					TerminalMode=SETABSPOS;
					nEscapeParameter=2;
					break;
				case '+':
					ClearTerminal();
					TerminalMode=NORMAL;
					break;
				case 'G':
					TerminalMode=BLINK;
					break;
				case 'E': //INSLINE
					InsertLine();
					TerminalMode=NORMAL;
					break;
				case 'R': //DELLINE
					DeleteLine();
					TerminalMode=NORMAL;
					break;
				default:
					TerminalMode=NORMAL;
					break;
			}
			break;
		case SETABSPOS:
			if(nEscapeParameter)
			{
				if(nEscapeParameter==2)TerminalY=(short)(data-0x20);
				else if(nEscapeParameter==1)TerminalX=(short)(data-0x20);
				nEscapeParameter--;
			}
			if(nEscapeParameter==0)TerminalMode=NORMAL;
			break;
		case VT101:
			if(data==0x19)TerminalMode=NORMAL;
			break;
		case BLINK:
			if(data=='2')
			{
			short i;

				for(i=TerminalX;i<MAXCOLUMNS;i++)SetTerminalAttribut(i,TerminalY,1);
			}
			TerminalMode=NORMAL;
			break;
	}
	// reset critical section flag for second (cursor) thread
	bUpdateCritical=FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void ChooseTerminalFont(void)
//
// DESCRIPTION:   display CHOOSEFONT dialog and lets user choose font
//						for terminal (only fixed pitch)
//
// PARAMETERS:		none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void ChooseTerminalFont(void)
{
void AdjustTerminal(void);
HLOCAL hLogFont,hCFont;
LOGFONT *pLogFont;
CHOOSEFONT *pCFont;
HDC hDC=GetDC(ghWnd);
TEXTMETRIC tm;

	// create LOGFONT struct in memory
	hLogFont=LocalAlloc(LHND,sizeof(LOGFONT)); // LHND = moveable and ZEROINIT
	pLogFont=(LOGFONT*)LocalLock(hLogFont);
	hCFont=LocalAlloc(LHND,sizeof(CHOOSEFONT)); // LHND = moveable and ZEROINIT
	pCFont=(CHOOSEFONT*)LocalLock(hCFont);

	pCFont->lStructSize=sizeof(CHOOSEFONT);
	pCFont->hwndOwner=ghWnd;
	pCFont->lpLogFont=pLogFont;
	pCFont->Flags=CF_SCREENFONTS|CF_FIXEDPITCHONLY;
	pCFont->nSizeMax=20;
	if(ChooseFont(pCFont))
	{
	char temp[32];

		// Create the fixed system terminal font -> get handle to font
		if(hOldFont)SelectObject(hDC,hOldFont);
		hDefaultFont = CreateFontIndirect((LPLOGFONT) pLogFont);
		hOldFont=SelectObject(hDC,hDefaultFont);

		// get text metrics for paint/WM_PAINT
		GetTextMetrics(hDC, &tm);   		// get text metrics
		xChar = tm.tmMaxCharWidth;       // get character sizes width and height
		yChar = tm.tmHeight + tm.tmExternalLeading;

		AdjustTerminal();
		UpdateWholeTerminal(hDC);
		WritePrivateProfileString("FONTS","FACENAME",pLogFont->lfFaceName,
											".\\68681.ini");
		sprintf(temp,"%ld",pLogFont->lfHeight);
		WritePrivateProfileString("FONTS","HEIGHT",temp,".\\68681.ini");
		sprintf(temp,"%ld",pLogFont->lfWidth);
		WritePrivateProfileString("FONTS","WIDTH",temp,".\\68681.ini");
		sprintf(temp,"%ld",pLogFont->lfWeight);
		WritePrivateProfileString("FONTS","WEIGHT",temp,".\\68681.ini");
	}

	LocalUnlock(hLogFont);
	LocalFree(hLogFont);
	LocalUnlock(hCFont);
	LocalFree(hCFont);
	ReleaseDC(ghWnd,hDC);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void CreateTerminalFont(HDC hDC,LPSTR szFaceName,
//          					long nFontHeight,long nFontWidth,long nWeight)
//
// DESCRIPTION:	create the font used in the terminal emulation
//
// PARAMETERS:    HDC hDC: device context of window for font
//						LPSTR szFaceName: name of font
//						long FontHeight: height of font
//						long FontWidth: width of font
//						long Weight: strike weight of font
//
// RETURNS:
//
////////////////////////////////////////////////////////////////////////////////
void CreateTerminalFont(HDC hDC,LPSTR szFaceName,long nFontHeight,
								long nFontWidth,long nWeight)
{
TEXTMETRIC tm;			// TEXTMETRICS of display font
HLOCAL hLogFont;
LOGFONT* pLogFont;

	// create LOGFONT struct in memory
	hLogFont=LocalAlloc(LHND,sizeof(LOGFONT)); // LHND = moveable and ZEROINIT
	pLogFont=(LOGFONT*)LocalLock(hLogFont);

	// create fixed font
	pLogFont->lfHeight         =  nFontHeight;
	pLogFont->lfWidth          =  nFontWidth;
	pLogFont->lfCharSet        =  DEFAULT_CHARSET;
	pLogFont->lfPitchAndFamily =  FIXED_PITCH ;
	pLogFont->lfOutPrecision   = 	OUT_RASTER_PRECIS	;
	pLogFont->lfWeight         =  nWeight;
	pLogFont->lfQuality        =  DRAFT_QUALITY;
	strcpy(pLogFont->lfFaceName,szFaceName);

	// Create the fixed system terminal font -> get handle to font
	if(hOldFont)SelectObject(hDC,hOldFont);
	hDefaultFont = CreateFontIndirect((LPLOGFONT) pLogFont);
	hOldFont=SelectObject(hDC,hDefaultFont);

	LocalUnlock(hLogFont);
	LocalFree(hLogFont);

	// select font into DC for restore purpose

	// get text metrics for paint/WM_PAINT
	GetTextMetrics(hDC, &tm);   		// get text metrics
	xChar = tm.tmMaxCharWidth;       // get character sizes width and height
	yChar = tm.tmHeight + tm.tmExternalLeading;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void SetupTerminal(void)
//
// DESCRIPTION:   setup the terminal window and display it
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void SetupTerminal(void)
{
void AdjustTerminal(void);
HWND hWnd;
WNDCLASS wndclass;
HDC hDC;
char temp[64];

	// allocate local storage for display memory
	hScreen=LocalAlloc(LHND,MAXCOLUMNS*MAXLINES);
	// and attributes
	hAttribut=LocalAlloc(LHND,MAXCOLUMNS*MAXLINES);
	gpScreen=LocalLock(hScreen);	// get pointers to display/attributes
	gpAttribut=LocalLock(hAttribut);

	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc=(WNDPROC)TerminalWndProc;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=(HINSTANCE)GetWindowLong(hParentWnd,GWL_HINSTANCE);
	wndclass.hIcon=NULL;
	wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground=GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName=gszAppName;
	wndclass.lpszClassName=gszAppName;

	if(RegisterClass(&wndclass))
	{
	RECT rc;
		GetClientRect(hParentWnd,&rc);
		hWnd=CreateWindowEx(
					WS_EX_CLIENTEDGE,
					gszAppName,
					gszAppName,
					WS_CHILDWINDOW|WS_VISIBLE|WS_CAPTION|WS_CLIPSIBLINGS,
					rc.left,
					rc.top,
					rc.right,
					rc.bottom,
					hParentWnd,
					NULL,
					(HINSTANCE)GetWindowLong(hParentWnd,GWL_HINSTANCE),
					NULL);
		ghWnd=hWnd;
		ShowWindow(hWnd,SW_SHOW);
		UpdateWindow(hWnd);
		hDC=GetDC(hWnd);
		GetPrivateProfileString("FONTS","FACENAME","",temp,sizeof(temp),
										".\\68681.ini");
		CreateTerminalFont(hDC,
							temp,
							GetPrivateProfileInt("FONTS","HEIGHT",-14,".\\68681.ini"),
							GetPrivateProfileInt("FONTS","WIDTH",0,".\\68681.ini"),
							GetPrivateProfileInt("FONTS","WEIGHT",400,".\\68681.ini"));
		ReleaseDC(hWnd,hDC);
		AdjustTerminal();
		ClearTerminal();

		// subclass parent window to get hold on WM_CHAR messages
		OldWndProc=(WNDPROC)GetWindowLong(hParentWnd,GWL_WNDPROC);
		SetWindowLong(hParentWnd,GWL_WNDPROC,(DWORD)NewWndProc);

		// create cursor update timer event
		bThreadRunning=TRUE;
		hThread=CreateThread(NULL,0,
									(LPTHREAD_START_ROUTINE)CursorUpdateThreadFunc,
									NULL,0,&dwThreadId);
	}

}

////////////////////////////////////////////////////////////////////////////////
// NAME:				long FAR PASCAL NewWndProc(HWND hWnd,UINT iMessage,
//														WORD wParam,long lParam)
//
// DESCRIPTION:   terminal subclass window proc to get hold on WM_CHAR
//						/WM_KEYDOWN messages for main window
//
// PARAMETERS:    (standard WIN32 API)
//
// RETURNS:			(standard WIN32 API)
//
////////////////////////////////////////////////////////////////////////////////
long FAR PASCAL NewWndProc(HWND hWnd,UINT iMessage,WORD wParam,long lParam)
{
	switch(iMessage)
	{
		case WM_CHAR:
		case WM_KEYDOWN:
			// send to terminal window
			SendMessage(ghWnd,iMessage,wParam,lParam);
			break;
	}
	return OldWndProc(hWnd,iMessage,wParam,lParam);
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				void AdjustTerminal(void)
//
// DESCRIPTION:	set size of terminal window according to font size
//
// PARAMETERS:		none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void AdjustTerminal(void)
{
RECT rc; // rectangle structures to store client and status window coords

	// the terminal window must be sized at startup to equal MAXLINES/MAXCOLUMNS
	rc.left=0;
	rc.top=0;
	// horizontal size of client area of terminal window
	rc.right=MAXCOLUMNS*xChar;
	// vertical size of client area of terminal window
	rc.bottom=(MAXLINES)*yChar;

	AdjustWindowRectEx(&rc,GetWindowLong(ghWnd,GWL_STYLE),FALSE,WS_EX_CLIENTEDGE);
	MoveWindow(ghWnd,
					GetPrivateProfileInt("WINDOW","X",0,".\\68681.ini"),
					GetPrivateProfileInt("WINDOW","Y",0,".\\68681.ini"),
					rc.right-rc.left,rc.bottom-rc.top,TRUE);
	ShowWindow(ghWnd,SW_SHOW);
	UpdateWindow(ghWnd);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void ExitTerminal(void)
//
// DESCRIPTION:   exit terminal simulation aand cleanup
//
// PARAMETERS:
//
// RETURNS:
//
////////////////////////////////////////////////////////////////////////////////
void ExitTerminal(void)
{
HDC hDC=GetDC(ghWnd);
RECT rc;
char temp[64];
POINT pt;

	SetWindowLong(hParentWnd,GWL_WNDPROC,(DWORD)OldWndProc); // undo subclassing
	bThreadRunning=FALSE; // tell cursor thread to stop
	GetWindowRect(ghWnd,&rc);
	pt.x=rc.left; pt.y=rc.top;
	ScreenToClient(hParentWnd,&pt);
	sprintf(temp,"%ld",pt.x);
	WritePrivateProfileString("WINDOW","X",temp,".\\68681.ini");
	sprintf(temp,"%ld",pt.y);
	WritePrivateProfileString("WINDOW","Y",temp,".\\68681.ini");
	SelectObject(hDC,hOldFont); // restore the old font into display
	ReleaseDC(ghWnd,hDC);		 // and release the device context
	DeleteObject(hDefaultFont); // delete the terminal font
	LocalUnlock(hScreen);
	LocalUnlock(hAttribut);
	LocalFree(hScreen);         // free the display memory
	LocalFree(hAttribut);		 // and the attribute memory
	DestroyWindow(ghWnd);
}

////////////////////////////////////////////////////////////////////////////////
// NAME:          void UpdateCursor(void)
//
// DESCRIPTION:	updates the character at the current cursor position
//						and the blinking cursor
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void UpdateCursor(void)
{
char temp;
HDC hDC=GetDC(ghWnd);

	SetTextColor(hDC,RGB(0,255,0));			// set text color to terminal green
	SetBkColor(hDC,RGB(0,0,0));    			// set background color to black
	SelectObject(hDC,hDefaultFont);

	if(OldX!=TerminalX || OldY!=TerminalY)       // has cursor position changed?
	{
		temp=*(gpScreen+MAXCOLUMNS*OldY+OldX);     // calc position in display memory
		// put text to old position without cursor block
		TextOut(hDC,xChar*OldX,yChar*OldY,&temp,1);
		// update the X/Y position for next UpdateCursor
		OldX=TerminalX;
		OldY=TerminalY;
	}

	if(bCursorBlink)
	{
		SetTextColor(hDC,RGB(0,0,0)); // if cursor blinking here -> reverse colors
		SetBkColor(hDC,RGB(0,255,0));
	}

	// get position in display memory
	temp=*(gpScreen+MAXCOLUMNS*TerminalY+TerminalX);
	// and update the blinking cursor there
	TextOut(hDC,xChar*TerminalX,yChar*TerminalY,&temp,1);
	SelectObject(hDC,hOldFont);
	ReleaseDC(ghWnd,hDC);

}

////////////////////////////////////////////////////////////////////////////////
// NAME:				DWORD CursorUpdateThreadFunc(LPDWORD lpdwParam)
//
// DESCRIPTION:   a seperate thread that keeps on blinking the cursor
//
// PARAMETERS:    LPDWORD lpdwParam: not used
//
// RETURNS:       DWORD:  not used
//
////////////////////////////////////////////////////////////////////////////////

DWORD CursorUpdateThreadFunc(LPDWORD lpdwParam)
{
short i;
static BOOL ScreenDirty=FALSE;
	while(bThreadRunning)
	{
		// for each line and column, check if display is dirty
		for(i=0;i<MAXLINES*MAXCOLUMNS && ScreenDirty==FALSE;i++)
		{
			if(*(gpScreen+i)==' ')*(gpAttribut+i)=0;
			else if(*(gpAttribut+i)==1)
			{
				ScreenDirty=TRUE;
			}
		}
		// screen is dirty, so update the whole terminal
		if(ScreenDirty)
		{
		HDC hDC=GetDC(ghWnd);
			UpdateWholeTerminal(hDC);
			ReleaseDC(ghWnd,hDC);
			ScreenDirty=FALSE;
		}
		if(bUpdateCritical==FALSE)
		{
			bCursorBlink=!bCursorBlink;  // toggle cursor blink flag
			UpdateCursor();           // and update the cursor at current position
		}
		else
		{
			bCursorBlink=FALSE;
			UpdateCursor();           // and update the cursor at current position
		}
		Sleep(250); // go to sleep for 200ms
	}
	return 0L;
}


////////////////////////////////////////////////////////////////////////////////
// NAME:				long FAR PASCAL TerminalWndProc(HWND hWnd,UINT iMessage,
//														WORD wParam,long lParam)
//
// DESCRIPTION:   terminal window procedure
//
// PARAMETERS:    (standard WIN32 API)
//
// RETURNS:			(standard WIN32 API)
//
////////////////////////////////////////////////////////////////////////////////

long FAR PASCAL TerminalWndProc(HWND hWnd,UINT iMessage,WORD wParam,long lParam)
{
PAINTSTRUCT ps;

	switch(iMessage)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDM_CHOOSEFONT:
					ChooseTerminalFont();
					break;
			}
			break;
		case WM_KEYDOWN:
			// translate keyboard virtual keys to QUME keys
			switch((short)wParam) // virtual keycode
			{
				case VK_ESCAPE:
					RBA=0x03; // BREAK
					SRA|=0x01;
					break;
				case VK_UP:
					RBA=0x0b; // LINEUP
					SRA|=0x01;
					break;
				case VK_DOWN:
					RBA=0x0a; // LINEDOWN
					SRA|=0x01;
					break;
				case VK_NEXT:
					RBA=0x10; // PAGEDOWN
					SRA|=0x01;
					break;
				case VK_PRIOR:
					RBA=0x11; // PAGEUP
					SRA|=0x01;
					break;
				case VK_HOME:
					RBA=0x1e; // HOME
					SRA|=0x01;
					break;
				case VK_INSERT:
					RBA=0x16; // INSERT
					SRA|=0x01;
					break;
				case VK_BACK:
					RBA=0x7f; // DELETE
					SRA|=0x01;
					break;
				case VK_DELETE:
					RBA=0x07; // RUB
					SRA|=0x01;
					break;
				case VK_RIGHT:
					RBA=0x0c; // ARROW RIGHT
					SRA|=0x01;
					break;
				case VK_LEFT:
					RBA=0x08; // ARROW LEFT
					SRA|=0x01;
					break;
			}
			break;
		case WM_CHAR:
			RBA=(char)wParam;
			SRA|=0x01;
			break;
		case WM_PAINT:
			if(hWnd==ghWnd)
			{
				BeginPaint(hWnd,&ps);
				UpdateWholeTerminal(ps.hdc);
				EndPaint(hWnd,&ps);
			}
			break;
		// display popup on right mouse button down
		case WM_TIMER:
			if(popupopen==1)
			{
			POINT cp;
				GetCursorPos(&cp);
				if((GetAsyncKeyState(VK_RBUTTON))==0)bRButtonUp=TRUE;
				else{
					bRButtonUp=FALSE;
				}
				if(bRButtonUp==TRUE)
				{
					DestroyMenu(hPopupMenu);
					popupopen=0;
					bRButtonUp=FALSE;
					KillTimer(hWnd,nPopupTimerID);
				}
			}
			break;
		case WM_LBUTTONDOWN:
			SetFocus(hWnd);
			break;
		case WM_RBUTTONDOWN:
			bRButtonUp=FALSE;
			popupopen=1;
			nPopupTimerID=SetTimer(hWnd,1,100,NULL);
			DisplayPopup(hWnd);
			break;
	}
	return DefWindowProc(hWnd,iMessage,wParam,lParam);
}

//// EOF //////////////////////////////////////////////////////////////////////////
