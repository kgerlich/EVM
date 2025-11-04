////////////////////////////////////////////////////////////////////////////////
// NAME: 			vt100.h
// COMPILER:		Borland C++ 5.0
// AUTHOR:			Klaus P. Gerlicher
// DATE:				4/1997
// DESCRIPTION:   Header file
//						includes for vt100.c
////////////////////////////////////////////////////////////////////////////////
void SimulateTerminal(unsigned char data);
void SetupTerminal(void);
void ExitTerminal(void);
void UpdateTerminal(void);
void UpdateWholeTerminal(HDC hDC);
void AdjustTerminal(void);
void CreateTerminalFont(int nFontSize);
void RefreshTerminal(void);
