# Microsoft Developer Studio Generated NMAKE File, Based on evmrom.dsp
!IF "$(CFG)" == ""
CFG=evmrom - Win32 Debug
!MESSAGE No configuration specified. Defaulting to evmrom - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "evmrom - Win32 Release" && "$(CFG)" != "evmrom - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "evmrom.mak" CFG="evmrom - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "evmrom - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "evmrom - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "evmrom - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\evmrom.dll"


CLEAN :
	-@erase "$(INTDIR)\evmrom.obj"
	-@erase "$(INTDIR)\s19.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\evmrom.exp"
	-@erase "$(OUTDIR)\evmrom.lib"
	-@erase "..\evmrom.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EVMROM_EXPORTS" /Fp"$(INTDIR)\evmrom.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\evmrom.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /profile /machine:I386 /out:"..\evmrom.dll" /implib:"$(OUTDIR)\evmrom.lib" 
LINK32_OBJS= \
	"$(INTDIR)\evmrom.obj" \
	"$(INTDIR)\s19.obj"

"..\evmrom.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "evmrom - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\evmrom.dll"


CLEAN :
	-@erase "$(INTDIR)\evmrom.obj"
	-@erase "$(INTDIR)\s19.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\evmrom.exp"
	-@erase "$(OUTDIR)\evmrom.lib"
	-@erase "$(OUTDIR)\evmrom.pdb"
	-@erase "..\evmrom.dll"
	-@erase "..\evmrom.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EVMROM_EXPORTS" /Fp"$(INTDIR)\evmrom.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\evmrom.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\evmrom.pdb" /debug /machine:I386 /out:"..\evmrom.dll" /implib:"$(OUTDIR)\evmrom.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\evmrom.obj" \
	"$(INTDIR)\s19.obj"

"..\evmrom.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("evmrom.dep")
!INCLUDE "evmrom.dep"
!ELSE 
!MESSAGE Warning: cannot find "evmrom.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "evmrom - Win32 Release" || "$(CFG)" == "evmrom - Win32 Debug"
SOURCE=.\evmrom.c

"$(INTDIR)\evmrom.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\s19.c

"$(INTDIR)\s19.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

