# Microsoft Developer Studio Generated NMAKE File, Based on EVMSIM.dsp
!IF "$(CFG)" == ""
CFG=EVMSIM - Win32 Debug
!MESSAGE No configuration specified. Defaulting to EVMSIM - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "EVMSIM - Win32 Release" && "$(CFG)" != "EVMSIM - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "EVMSIM.mak" CFG="EVMSIM - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EVMSIM - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "EVMSIM - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "EVMSIM - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : "..\EVMSIM.exe"

!ELSE 

ALL : "evmrom - Win32 Release" "evmram - Win32 Release" "debug - Win32 Release" "build - Win32 Release" "68681 - Win32 Release" "68230 - Win32 Release" "..\EVMSIM.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"68230 - Win32 ReleaseCLEAN" "68681 - Win32 ReleaseCLEAN" "build - Win32 ReleaseCLEAN" "debug - Win32 ReleaseCLEAN" "evmram - Win32 ReleaseCLEAN" "evmrom - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\Stcom.obj"
	-@erase "$(INTDIR)\steacalc.obj"
	-@erase "$(INTDIR)\Stexep.obj"
	-@erase "$(INTDIR)\Stflags.obj"
	-@erase "$(INTDIR)\Stmain.obj"
	-@erase "$(INTDIR)\stmem.obj"
	-@erase "$(INTDIR)\Stpopup.obj"
	-@erase "$(INTDIR)\ststartw.obj"
	-@erase "$(INTDIR)\ststate.obj"
	-@erase "$(INTDIR)\sttable.obj"
	-@erase "$(INTDIR)\stwin.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "..\EVMSIM.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /Ot /Oa /Og /Oi /Oy /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\EVMSIM.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\stwin.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\EVMSIM.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"..\EVMSIM.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Stcom.obj" \
	"$(INTDIR)\steacalc.obj" \
	"$(INTDIR)\Stexep.obj" \
	"$(INTDIR)\Stflags.obj" \
	"$(INTDIR)\Stmain.obj" \
	"$(INTDIR)\stmem.obj" \
	"$(INTDIR)\Stpopup.obj" \
	"$(INTDIR)\ststartw.obj" \
	"$(INTDIR)\ststate.obj" \
	"$(INTDIR)\sttable.obj" \
	"$(INTDIR)\stwin.res" \
	"..\68230\Release\68230.lib" \
	"..\68681\Release\68681.lib" \
	"..\debug\Release\debug.lib" \
	"..\evmram\Release\evmram.lib" \
	"..\evmrom\Release\evmrom.lib"

"..\EVMSIM.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : "..\EVMSIM.exe"

!ELSE 

ALL : "evmrom - Win32 Debug" "evmram - Win32 Debug" "debug - Win32 Debug" "build - Win32 Debug" "68681 - Win32 Debug" "68230 - Win32 Debug" "..\EVMSIM.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"68230 - Win32 DebugCLEAN" "68681 - Win32 DebugCLEAN" "build - Win32 DebugCLEAN" "debug - Win32 DebugCLEAN" "evmram - Win32 DebugCLEAN" "evmrom - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\Stcom.obj"
	-@erase "$(INTDIR)\steacalc.obj"
	-@erase "$(INTDIR)\Stexep.obj"
	-@erase "$(INTDIR)\Stflags.obj"
	-@erase "$(INTDIR)\Stmain.obj"
	-@erase "$(INTDIR)\stmem.obj"
	-@erase "$(INTDIR)\Stpopup.obj"
	-@erase "$(INTDIR)\ststartw.obj"
	-@erase "$(INTDIR)\ststate.obj"
	-@erase "$(INTDIR)\sttable.obj"
	-@erase "$(INTDIR)\stwin.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\EVMSIM.pdb"
	-@erase "..\EVMSIM.exe"
	-@erase "..\EVMSIM.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\EVMSIM.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\stwin.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\EVMSIM.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\EVMSIM.pdb" /debug /machine:I386 /out:"..\EVMSIM.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Stcom.obj" \
	"$(INTDIR)\steacalc.obj" \
	"$(INTDIR)\Stexep.obj" \
	"$(INTDIR)\Stflags.obj" \
	"$(INTDIR)\Stmain.obj" \
	"$(INTDIR)\stmem.obj" \
	"$(INTDIR)\Stpopup.obj" \
	"$(INTDIR)\ststartw.obj" \
	"$(INTDIR)\ststate.obj" \
	"$(INTDIR)\sttable.obj" \
	"$(INTDIR)\stwin.res" \
	"..\68230\Debug\68230.lib" \
	"..\68681\Debug\68681.lib" \
	"..\debug\debug___Win32_Debug\debug.lib" \
	"..\evmram\Debug\evmram.lib" \
	"..\evmrom\Debug\evmrom.lib"

"..\EVMSIM.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("EVMSIM.dep")
!INCLUDE "EVMSIM.dep"
!ELSE 
!MESSAGE Warning: cannot find "EVMSIM.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "EVMSIM - Win32 Release" || "$(CFG)" == "EVMSIM - Win32 Debug"
SOURCE=.\Stcom.c

!IF  "$(CFG)" == "EVMSIM - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /Ot /Oa /Og /Oi /Oy /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\EVMSIM.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Stcom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\EVMSIM.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Stcom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\steacalc.c

"$(INTDIR)\steacalc.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Stexep.c

"$(INTDIR)\Stexep.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Stflags.c

"$(INTDIR)\Stflags.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Stmain.c

"$(INTDIR)\Stmain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stmem.c

"$(INTDIR)\stmem.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Stpopup.c

"$(INTDIR)\Stpopup.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ststartw.c

"$(INTDIR)\ststartw.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ststate.c

"$(INTDIR)\ststate.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sttable.c

"$(INTDIR)\sttable.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stwin.rc

"$(INTDIR)\stwin.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!IF  "$(CFG)" == "EVMSIM - Win32 Release"

"68230 - Win32 Release" : 
   cd "\KLAUS\evm\68230"
   $(MAKE) /$(MAKEFLAGS) /F .\68230.mak CFG="68230 - Win32 Release" 
   cd "..\EVMSIM"

"68230 - Win32 ReleaseCLEAN" : 
   cd "\KLAUS\evm\68230"
   $(MAKE) /$(MAKEFLAGS) /F .\68230.mak CFG="68230 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

"68230 - Win32 Debug" : 
   cd "\KLAUS\evm\68230"
   $(MAKE) /$(MAKEFLAGS) /F .\68230.mak CFG="68230 - Win32 Debug" 
   cd "..\EVMSIM"

"68230 - Win32 DebugCLEAN" : 
   cd "\KLAUS\evm\68230"
   $(MAKE) /$(MAKEFLAGS) /F .\68230.mak CFG="68230 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ENDIF 

!IF  "$(CFG)" == "EVMSIM - Win32 Release"

"68681 - Win32 Release" : 
   cd "\KLAUS\evm\68681"
   $(MAKE) /$(MAKEFLAGS) /F .\68681.mak CFG="68681 - Win32 Release" 
   cd "..\EVMSIM"

"68681 - Win32 ReleaseCLEAN" : 
   cd "\KLAUS\evm\68681"
   $(MAKE) /$(MAKEFLAGS) /F .\68681.mak CFG="68681 - Win32 Release" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

"68681 - Win32 Debug" : 
   cd "\KLAUS\evm\68681"
   $(MAKE) /$(MAKEFLAGS) /F .\68681.mak CFG="68681 - Win32 Debug" 
   cd "..\EVMSIM"

"68681 - Win32 DebugCLEAN" : 
   cd "\KLAUS\evm\68681"
   $(MAKE) /$(MAKEFLAGS) /F .\68681.mak CFG="68681 - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ENDIF 

!IF  "$(CFG)" == "EVMSIM - Win32 Release"

"build - Win32 Release" : 
   cd "\KLAUS\evm\build"
   $(MAKE) /$(MAKEFLAGS) /F .\build.mak CFG="build - Win32 Release" 
   cd "..\EVMSIM"

"build - Win32 ReleaseCLEAN" : 
   cd "\KLAUS\evm\build"
   $(MAKE) /$(MAKEFLAGS) /F .\build.mak CFG="build - Win32 Release" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

"build - Win32 Debug" : 
   cd "\KLAUS\evm\build"
   $(MAKE) /$(MAKEFLAGS) /F .\build.mak CFG="build - Win32 Debug" 
   cd "..\EVMSIM"

"build - Win32 DebugCLEAN" : 
   cd "\KLAUS\evm\build"
   $(MAKE) /$(MAKEFLAGS) /F .\build.mak CFG="build - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ENDIF 

!IF  "$(CFG)" == "EVMSIM - Win32 Release"

"debug - Win32 Release" : 
   cd "\KLAUS\evm\debug"
   $(MAKE) /$(MAKEFLAGS) /F .\debug.mak CFG="debug - Win32 Release" 
   cd "..\EVMSIM"

"debug - Win32 ReleaseCLEAN" : 
   cd "\KLAUS\evm\debug"
   $(MAKE) /$(MAKEFLAGS) /F .\debug.mak CFG="debug - Win32 Release" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

"debug - Win32 Debug" : 
   cd "\KLAUS\evm\debug"
   $(MAKE) /$(MAKEFLAGS) /F .\debug.mak CFG="debug - Win32 Debug" 
   cd "..\EVMSIM"

"debug - Win32 DebugCLEAN" : 
   cd "\KLAUS\evm\debug"
   $(MAKE) /$(MAKEFLAGS) /F .\debug.mak CFG="debug - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ENDIF 

!IF  "$(CFG)" == "EVMSIM - Win32 Release"

"evmram - Win32 Release" : 
   cd "\KLAUS\evm\evmram"
   $(MAKE) /$(MAKEFLAGS) /F .\evmram.mak CFG="evmram - Win32 Release" 
   cd "..\EVMSIM"

"evmram - Win32 ReleaseCLEAN" : 
   cd "\KLAUS\evm\evmram"
   $(MAKE) /$(MAKEFLAGS) /F .\evmram.mak CFG="evmram - Win32 Release" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

"evmram - Win32 Debug" : 
   cd "\KLAUS\evm\evmram"
   $(MAKE) /$(MAKEFLAGS) /F .\evmram.mak CFG="evmram - Win32 Debug" 
   cd "..\EVMSIM"

"evmram - Win32 DebugCLEAN" : 
   cd "\KLAUS\evm\evmram"
   $(MAKE) /$(MAKEFLAGS) /F .\evmram.mak CFG="evmram - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ENDIF 

!IF  "$(CFG)" == "EVMSIM - Win32 Release"

"evmrom - Win32 Release" : 
   cd "\KLAUS\evm\evmrom"
   $(MAKE) /$(MAKEFLAGS) /F .\evmrom.mak CFG="evmrom - Win32 Release" 
   cd "..\EVMSIM"

"evmrom - Win32 ReleaseCLEAN" : 
   cd "\KLAUS\evm\evmrom"
   $(MAKE) /$(MAKEFLAGS) /F .\evmrom.mak CFG="evmrom - Win32 Release" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ELSEIF  "$(CFG)" == "EVMSIM - Win32 Debug"

"evmrom - Win32 Debug" : 
   cd "\KLAUS\evm\evmrom"
   $(MAKE) /$(MAKEFLAGS) /F .\evmrom.mak CFG="evmrom - Win32 Debug" 
   cd "..\EVMSIM"

"evmrom - Win32 DebugCLEAN" : 
   cd "\KLAUS\evm\evmrom"
   $(MAKE) /$(MAKEFLAGS) /F .\evmrom.mak CFG="evmrom - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\EVMSIM"

!ENDIF 


!ENDIF 

