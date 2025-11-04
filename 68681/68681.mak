# Microsoft Developer Studio Generated NMAKE File, Based on 68681.dsp
!IF "$(CFG)" == ""
CFG=68681 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to 68681 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "68681 - Win32 Release" && "$(CFG)" != "68681 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "68681.mak" CFG="68681 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "68681 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "68681 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "68681 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\68681.dll"


CLEAN :
	-@erase "$(INTDIR)\68681.obj"
	-@erase "$(INTDIR)\68681.res"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\popup.obj"
	-@erase "$(INTDIR)\printer.obj"
	-@erase "$(INTDIR)\state.obj"
	-@erase "$(INTDIR)\updown.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vt100.obj"
	-@erase "$(OUTDIR)\68681.exp"
	-@erase "$(OUTDIR)\68681.lib"
	-@erase "..\68681.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MY68681_EXPORTS" /Fp"$(INTDIR)\68681.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\68681.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\68681.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\68681.pdb" /machine:I386 /out:"..\68681.dll" /implib:"$(OUTDIR)\68681.lib" 
LINK32_OBJS= \
	"$(INTDIR)\vt100.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\popup.obj" \
	"$(INTDIR)\printer.obj" \
	"$(INTDIR)\state.obj" \
	"$(INTDIR)\updown.obj" \
	"$(INTDIR)\68681.obj" \
	"$(INTDIR)\68681.res"

"..\68681.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "68681 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\68681.dll"


CLEAN :
	-@erase "$(INTDIR)\68681.obj"
	-@erase "$(INTDIR)\68681.res"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\popup.obj"
	-@erase "$(INTDIR)\printer.obj"
	-@erase "$(INTDIR)\state.obj"
	-@erase "$(INTDIR)\updown.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vt100.obj"
	-@erase "$(OUTDIR)\68681.exp"
	-@erase "$(OUTDIR)\68681.lib"
	-@erase "$(OUTDIR)\68681.pdb"
	-@erase "..\68681.dll"
	-@erase "..\68681.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MY68681_EXPORTS" /Fp"$(INTDIR)\68681.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x407 /fo"$(INTDIR)\68681.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\68681.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\68681.pdb" /debug /machine:I386 /out:"..\68681.dll" /implib:"$(OUTDIR)\68681.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\vt100.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\popup.obj" \
	"$(INTDIR)\printer.obj" \
	"$(INTDIR)\state.obj" \
	"$(INTDIR)\updown.obj" \
	"$(INTDIR)\68681.obj" \
	"$(INTDIR)\68681.res"

"..\68681.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("68681.dep")
!INCLUDE "68681.dep"
!ELSE 
!MESSAGE Warning: cannot find "68681.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "68681 - Win32 Release" || "$(CFG)" == "68681 - Win32 Debug"
SOURCE=.\68681.c

"$(INTDIR)\68681.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\file.c

"$(INTDIR)\file.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\popup.c

"$(INTDIR)\popup.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\printer.c

"$(INTDIR)\printer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\state.c

"$(INTDIR)\state.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\updown.c

"$(INTDIR)\updown.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vt100.c

"$(INTDIR)\vt100.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\68681.rc

"$(INTDIR)\68681.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

