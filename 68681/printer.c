////////////////////////////////////////////////////////////////////////////////
// NAME: 			printer.c
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:
//						print text from EVM
//	REMARKS:			this is very rudimentary quick and dirty stuff
//						maybe someone knows better than i
////////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "68681.h"
#include "68681.rh"

////////////////////////////////////////////////////////////////////////////////
// prototypes
////////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY PrintDlgProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam);

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// NAME:				void DoPrint(char* pPrintBuf,int nBufferLength)
//
// DESCRIPTION:   print an ASCII text buffer with current printer
//
// PARAMETERS:   char *pPrintBuf:  pointer to print buffer
//					  int nBufferLength: length of buffer
//
// RETURNS:			none
//
////////////////////////////////////////////////////////////////////////////////
void DoPrint(char* pPrintBuf,int nBufferLength)
{
char temp[256],lpszDriver[64],lpszDevice[64],lpszOutput[64],*p;
HDC hPrinterDC;
DOCINFO di;
int i,j,nLineCount,nLineMax,nPageCount;
RECT rc;
char tempbuf[256];
PLOGFONT plf = (PLOGFONT) LocalAlloc(LPTR, sizeof(LOGFONT));
HFONT hOldFont,hFont;
TEXTMETRIC tm;
HWND hPrintDlg;


	// get current installed active printer
	GetProfileString((LPSTR)"windows",(LPSTR)"device",(LPSTR)"",
									(LPSTR)temp,sizeof(temp));
	if(strlen(temp))
	{
			hPrintDlg=CreateDialog(GetModuleHandle("68681.dll"),
										  "PRINTDLG",hParentWnd,PrintDlgProc);

			p=temp;
			i=0;
			while(*p!=',' && *p!=0)
			{
				lpszDriver[i++]=*p;
				p++;
			}
			lpszDriver[i]=0;
			p++;
			i=0;
			while(*p!=',' && *p!=0)
			{
				lpszDevice[i++]=*p;
				p++;
			}
			lpszDevice[i]=0;
			i=0;
			p++;
			while(*p!=',' && *p!=0)
			{
				lpszOutput[i++]=*p;
				p++;
			}
			lpszOutput[i]=0;

			// create printer DC
			if((hPrinterDC=CreateDC((LPSTR)NULL,(LPSTR)lpszDriver,
												(LPSTR)NULL,(DEVMODE*)NULL))!=0)
			{
				// set sizes
				GetClipBox(hPrinterDC,&rc);
				SetMapMode(hPrinterDC,MM_ISOTROPIC);
				SetWindowOrgEx(hPrinterDC,0,0,NULL);
				SetWindowExtEx(hPrinterDC,rc.right,rc.bottom,NULL);
				SetViewportExtEx(hPrinterDC,rc.right/2,rc.bottom/2,NULL);

				// specify a font typeface name and weight.
				lstrcpy(plf->lfFaceName, "courier new");
				plf->lfEscapement = 0;
				plf->lfWeight = FW_NORMAL; // normal font weight
				plf->lfHeight = -(rc.bottom-rc.top)/66; // 66 lines per page
				plf->lfWidth = -(rc.right-rc.left)/80; // 80 chars per line
				plf->lfPitchAndFamily=FIXED_PITCH; // fixed size per char
				// create the font
				hFont = CreateFontIndirect(plf);
				// select it into printers DC
				hOldFont = SelectObject(hPrinterDC, hFont);

				// count number of pages
				GetTextMetrics(hPrinterDC,&tm);
				nLineMax=(rc.bottom-rc.top)/(tm.tmHeight+tm.tmExternalLeading);
				for(nLineCount=0,i=0;i<nBufferLength;)
				{
					for(j=0;pPrintBuf[i+j]!=0x0d &&
							  pPrintBuf[i+j]!=0x0a &&
							  j<sizeof(tempbuf);j++);
					i+=(j+2);
					nLineCount++;
				}
				nPageCount=(nLineCount/nLineMax)+1;

				// fill in DOCINFO
				sprintf(temp,"EVMSim32 (%u pages)",nPageCount);
				di.cbSize=sizeof(DOCINFO);
				di.lpszDocName=temp;
				di.lpszOutput=NULL;
				di.lpszDatatype=NULL;
				di.fwType=0;
				// start document
				if(StartDoc(hPrinterDC,&di)!=SP_ERROR)
				{
					// start first page
					StartPage(hPrinterDC);

					// for all data in buffer
					for(nLineCount=0,i=0;i<nBufferLength;)
					{
						// copy a line into temporary buffer
						// LF or CR marks end of line
						for(j=0;pPrintBuf[i+j]!=0x0d &&
								  pPrintBuf[i+j]!=0x0a &&
								  j<sizeof(tempbuf);j++)tempbuf[j]=pPrintBuf[i+j];
						tempbuf[j]=0; // finish off line
						i+=(j+2); // increment data counter
						// output line
						TextOut(hPrinterDC,0,
								nLineCount*
								(tm.tmHeight+tm.tmExternalLeading),
								tempbuf,strlen(tempbuf));
						nLineCount++; // increment line counter
						// if there's no space left on page
						// start new page
						if(nLineCount>=nLineMax)
						{
							EndPage(hPrinterDC);
							StartPage(hPrinterDC);
							nLineCount=0;
						}
					}
					// finish last page and document
					EndPage(hPrinterDC);
					EndDoc(hPrinterDC);
				}
				// cleanups
				SelectObject(hPrinterDC, hOldFont);
				DeleteObject(hFont);
				DeleteDC(hPrinterDC);
				DestroyWindow(hPrintDlg);
			}
		}
	else MessageBox(hParentWnd,"No printer installed!",__FILE__,MB_OK);
}

BOOL APIENTRY PrintDlgProc(HWND hWnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	return FALSE;
}

