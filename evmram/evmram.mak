# Microsoft Developer Studio Generated NMAKE File, Based on evmram.dsp
!IF "$(CFG)" == ""
CFG=evmram - Win32 Debug
!MESSAGE No configuration specified. Defaulting to evmram - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "evmram - Win32 Release" && "$(CFG)" != "evmram - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "evmram.mak" CFG="evmram - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "evmram - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "evmram - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "evmram - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\evmram.dll"


CLEAN :
	-@erase "$(INTDIR)\evmram.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\evmram.exp"
	-@erase "$(OUTDIR)\evmram.lib"
	-@erase "..\evmram.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MT /W3 /GX /Ot /Oa /Og /Oi /Oy /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EVMRAM_EXPORTS" /Fp"$(INTDIR)\evmram.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\evmram.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /profile /machine:I386 /out:"..\evmram.dll" /implib:"$(OUTDIR)\evmram.lib" 
LINK32_OBJS= \
	"$(INTDIR)\evmram.obj"

"..\evmram.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "evmram - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\evmram.dll"


CLEAN :
	-@erase "$(INTDIR)\evmram.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\evmram.exp"
	-@erase "$(OUTDIR)\evmram.lib"
	-@erase "$(OUTDIR)\evmram.pdb"
	-@erase "..\evmram.dll"
	-@erase "..\evmram.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EVMRAM_EXPORTS" /Fp"$(INTDIR)\evmram.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\evmram.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\evmram.pdb" /debug /machine:I386 /out:"..\evmram.dll" /implib:"$(OUTDIR)\evmram.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\evmram.obj"

"..\evmram.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("evmram.dep")
!INCLUDE "evmram.dep"
!ELSE 
!MESSAGE Warning: cannot find "evmram.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "evmram - Win32 Release" || "$(CFG)" == "evmram - Win32 Debug"
SOURCE=.\evmram.c

"$(INTDIR)\evmram.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

