////////////////////////////////////////////////////////////////////////////////
// NAME: 			stmain.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						startup and user interface code for 68K simulator for Windows
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winnt.h>
#include <string.h>
#include <stdio.h>
#include <commdlg.h>
#include <commctrl.h>
//#include <values.h>
#include "stcom.h"
#include "stmem.h"
#include "stpopup.h"
#include "ststate.h"
#include "ststartw.h"
#include "stwin.rh"  // resources header

#define PARTS (8)  // number of tiled bitmaps in main window client area

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
HANDLE ghInstance;						// instance handle
LPCSTR gszAppName="STWIN";    		// app name
HWND ghWnd;          					// global handle to main window
HWND hStartup;  							// handle of startup windows
BOOL bRunning=TRUE; 						// FLAG: simulator is bRunning/ will stop
												// and exit if this flag becomes FALSE
const unsigned long nOpsAtOnce=100000; 		// operations to be performed in one call
long nStartTime=0,nStopTime=0; 		// measuring performance...
long nTimeForOps=1,nOpsPerSec=1;    // preset to 1, so no DIV_BY_ZERO occurs
long nMaxOpsPerSec=1;					// maximum of operations per second
UINT nInfoTimerID; 						// ID of Info timer
DWORD dwThreadId=0,dwExitCode=0;		// thread params
DWORD dwThrdParam=0;
HANDLE hSimThread=0;                // handle to simulation thread
BOOL bAppHasFocus=TRUE;             // TRUE if application is foreground task

////////////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////////////

// main window procedure
LRESULT CALLBACK WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);
DWORD SimulationThread(LPDWORD lpdwParam);   // simulation thread
void StartSim(void);	// start simulation
void StopSim(void);	// stop simulation

////////////////////////////////////////////////////////////////////////////////
// NAME: 			WinMain()
//
// DESCRIPTION:   entry point to program
//                create main window
//						setup simulation
// PARAMETERS:    (standard WIN32 API)
//
// RETURNS:       (standard WIN32 API)
//
////////////////////////////////////////////////////////////////////////////////

int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
						LPSTR lpszCmdLine,int nCmdShow)
{
HWND hWnd;
MSG msg;
WNDCLASS wndclass;

	//InitCommonControls(); // load COMCTL32.DLL and initialize the subsystem

	ghInstance = hInstance; // save away the instance handle globally

	// if there is no previous instance, register new window class
	if(!hPrevInstance)
	{
		wndclass.style=CS_HREDRAW|CS_VREDRAW; // redraw window
		wndclass.lpfnWndProc=(WNDPROC)WndProc;
		wndclass.cbClsExtra=0;
		wndclass.cbWndExtra=0;
		wndclass.hInstance=hInstance;
		wndclass.hIcon=LoadIcon(hInstance,gszAppName);
		wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
		wndclass.hbrBackground=GetStockObject(BLACK_BRUSH);
		wndclass.lpszMenuName=gszAppName;
		wndclass.lpszClassName=gszAppName;

		if(!RegisterClass(&wndclass))return FALSE;
	}

	// create the main window
	hWnd=CreateWindowEx(
					WS_EX_CLIENTEDGE|WS_EX_STATICEDGE,
					gszAppName,
					"68K Simulator for Windows",
					WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					NULL,
					LoadMenu(hInstance,gszAppName),
					hInstance,
					NULL);

	ghWnd=hWnd; // globalize window handle

	// create startup info window
	ShowStartupDlg();

	SetStartupText("Setting up simulation...");
	// setup the CPU simulation
	// if any module returns FALSE, exit
	if(!SetupSim())
	{
		SetStartupText("Tried to setup simulation - FAILED");
		ExitSim();
		Sleep(2000);
		return FALSE;
	}

	Sleep(1000);
	SetStartupText("Resetting simulation...");
	// reset simulation
	// if start vectors not found, exit!
	if(!ResetSim())
	{
		SetStartupText("Tried to reset simulation - FAILED");
		ExitSim();
		Sleep(2000);
		return FALSE;
	}

	ShowWindow(hWnd,SW_SHOWMAXIMIZED);
	UpdateWindow(hWnd);

	DisplayStatus(hWnd,hInstance); 	// display the status bar window


	Sleep(1000);
	SetStartupText("Starting simulation...");
	// create the main simulation thread
	StartSim();

	Sleep(1000);
	CloseStartupDlg(); // close start window

	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);	// do some key translations
		DispatchMessage(&msg);  // send message to WndProc
	}
	return msg.wParam;  // exit app with code
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void StartSim(void)
//
// DESCRIPTION:   start simulation thread
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void StartSim(void)
{
	bRunning=TRUE;
	hSimThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SimulationThread,
									  &dwThrdParam,0,&dwThreadId);
	if(hSimThread==INVALID_HANDLE_VALUE)
		MessageBox(ghWnd,"Couldn't start simulation!","EVMSIM",MB_OK|MB_TOPMOST);

	// create info timer
	nInfoTimerID=SetTimer(ghWnd,1,1000,NULL);
	SetStatusText(0,"Running...");
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				void StopSim(void)
//
// DESCRIPTION:   stop simulation thread
//
// PARAMETERS:    none
//
// RETURNS:       none
//
////////////////////////////////////////////////////////////////////////////////
void StopSim(void)
{
	bRunning=FALSE; // set global status to not running
	// wait for thread to finish, if it doesn't, kill it!
	if(WaitForSingleObject(hSimThread,3000)!=WAIT_OBJECT_0)
	{
		// kill thread, if it can't be killed
		// notify user
		if(!TerminateThread(hSimThread,0L))
		{
			MessageBox(ghWnd,"Simulation couldn't be terminated (TERMINATETHREAD)",
						  "EVMSIM",MB_OK|MB_TOPMOST);
		}
	}
	// close thread handle, if not possible
	// notify user
	if(!CloseHandle(hSimThread))
	{
		// notify user
		MessageBox(ghWnd,"Simulation didn't stop correctly (CLOSEHANDLE)",
					  "EVMSIM",MB_OK|MB_TOPMOST);
	}
	else hSimThread=0;
	// kill info timer
	KillTimer(ghWnd,nInfoTimerID);
	// reset max performance counter
	nMaxOpsPerSec=1;
}

////////////////////////////////////////////////////////////////////////////////
// NAME:				DWORD SimulationThread(LPDWORD lpdwParam)
//
// DESCRIPTION:   simulation thread
//
// PARAMETERS:    LPDWORD lpdwParam (not used)
//
// RETURNS:       DWORD (not used)
//
////////////////////////////////////////////////////////////////////////////////

DWORD SimulationThread(LPDWORD lpdwParam)
{
	// as long as global status is RUNNING
	while(bRunning==TRUE)
	{
		if(bAppHasFocus)
		{
			nStartTime=GetTickCount();  // start measuring time
			// is foreground process?
			Simulate68k(nOpsAtOnce); // do some OPSATONCE operations
			nStopTime=GetTickCount();    // stop measuring time
			if((nStopTime-nStartTime)>0) // enough measured time to display OPS/sec.?
			{
				nTimeForOps=nStopTime-nStartTime;
			}
		}
	}
	return 0L; // return when thread exits
}

////////////////////////////////////////////////////////////////////////////////
// NAME:  			WndProc()
//
// DESCRIPTION:   window procedure for application (main) window
//
// PARAMETERS:    (standard WIN32 API)
//
// RETURNS:       (standard WIN32 API)
//
////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
char temp[64]; // guess what?
HDC hDCMemory; // memory DC
PAINTSTRUCT ps; // paint structure for WM_PAINT
HBITMAP hBitmap, hOldBitmap; // BITMAP handles for tiled background bitmaps
BITMAP bitmap; // bitmap container
RECT rc;	// window rect
int x,y; // some counter vars

	switch(iMessage)
	{
		// task switch
		case WM_ACTIVATEAPP:
			// if task gets switched to/away, this will be set accordingly
			bAppHasFocus=(BOOL)wParam;
			// if being switched to task reset max performance counter
			if((BOOL)wParam)nMaxOpsPerSec=1;
			break;
		// main window is being destroyed
		case WM_DESTROY:
			PostQuitMessage(0);    // send quit to app -> exit message loop
			break;
		// main window is being sized
		case WM_SIZE:
			// broadcast messages to status bar
			PostMessage(hStatusWnd,iMessage,wParam,lParam);
			break;
		case WM_PAINT:
			// get PAINTSTRUCT
			BeginPaint(hWnd,&ps);
			// load bitmap resource and retreive its handle
			hBitmap = LoadBitmap(ghInstance, "FHMFB04");
			// load bitmap data
			GetObject(hBitmap, sizeof(BITMAP), &bitmap);
			// create a memory DC
			hDCMemory = CreateCompatibleDC(ps.hdc);
			// select bitmap into DC
			hOldBitmap = SelectObject(hDCMemory,hBitmap);
			// StretchBlt it onto real screen
			GetClientRect(hWnd,&rc);
			for(y=0;y<PARTS;y++)
			{
				for(x=0;x<PARTS;x++)
				{
					StretchBlt(ps.hdc,
								  x*(rc.right-rc.left+1)/PARTS,
								  y*(rc.bottom-rc.top+1)/PARTS,
								  (rc.right-rc.left+1)/PARTS,
								  (rc.bottom-rc.top+1)/PARTS,
								  hDCMemory,
								  0,
								  0,
								  bitmap.bmWidth,
								  bitmap.bmHeight,
								  SRCCOPY);
				}
			}
			// clean up and end painting
			SelectObject(hDCMemory, hOldBitmap);
			DeleteDC(hDCMemory);
			EndPaint(hWnd,&ps);
			// broadcast messages to status bar
			PostMessage(hStatusWnd,iMessage,wParam,lParam);
			break;
		// item from system command was selected
		case WM_SYSCOMMAND:
			switch(wParam) // which item
			{
				case SC_CLOSE: // close window (ALT-F4)
					if(MessageBox(hWnd,"Are you sure you want to quit?","EVMSIM",
						MB_YESNO|MB_ICONEXCLAMATION|MB_TOPMOST)==IDYES)
					{
						SetStatusText(0,"Exiting...");
						StopSim(); // nomen est omen
						ExitSim(); // exit CPU simulation
						// close main window, forces close of childs too.
						DestroyWindow(hWnd);
					}
					else return FALSE;
					break;
			}
			break;
		// menu command handling
		case WM_COMMAND:
			switch(HIWORD(wParam)) // notification code
			{
				default: // menu selected
					switch(wParam)
					{
						// quit app
						case IDM_QUIT:
							if(MessageBox(hWnd,"Are you sure you want to quit?",
								"EVMSIM",MB_YESNO|MB_ICONEXCLAMATION|MB_TOPMOST)==IDYES)
							{
								SetStatusText(0,"Exiting...");
								StopSim(); // nomen est omen
								ExitSim(); // exit CPU simulation
								DestroyWindow(hWnd);
							}
							break;
						// reset simulation
						case IDM_RESET:
							SetStatusText(0,"Reset...");
							StopSim();  // nomen est omen
							ResetSim(); // nomen est omen
							StartSim(); // nomen est omen
							break;
						case IDM_ABOUT:
							MessageBox(hWnd,"Diplomarbeit\n\"MC68020 on WIN32\"",
										  "MC68020 Simulator",MB_OK|MB_ICONINFORMATION|MB_TOPMOST);
							break;
					}
					break;
			}
			break;
		// timer messages
		case WM_TIMER:
			// this timer message displays the performance counter
			// in the status bar...
			if(wParam==nInfoTimerID)
			{
				extern ULONG hits,misses;
				nOpsPerSec=(long)((long)(nOpsAtOnce)*((long)(1000))/
																			(long)(nTimeForOps));
				// calc max performance
				if(nOpsPerSec>nMaxOpsPerSec)nMaxOpsPerSec=nOpsPerSec;
				wsprintf(temp,"%lu Operations/sec (%u%%) %u %u",nOpsPerSec,
						  (nOpsPerSec*100)/nMaxOpsPerSec,hits,misses);
				SetStatusText(2,temp);
				wsprintf(temp,"%lu HW-IRQs occurred",nIRQs);
				SetStatusText(1,temp);
			}
			// ...,while this one is responsible for
			// auto-closing of popup menu on right mouse button
			else
			{
				// check if right mouse button is up
				if((GetAsyncKeyState(VK_RBUTTON))==0)
				{
					DestroyMenu(hPopupMenu); // close popup
					KillTimer(ghWnd,0); // kill the timer
				}
			}
			break;
		// display popup on right mouse button down
		case WM_RBUTTONDOWN:
			// set timer to close popup automatically if no selection is made
			SetTimer(hWnd,0,1,NULL);
			DisplayPopup(hWnd);		 // display the popup window
			break;
	}
	// default window handling
	return DefWindowProc(hWnd,iMessage,wParam,lParam);
}

