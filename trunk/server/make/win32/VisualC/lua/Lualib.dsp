# Microsoft Developer Studio Project File - Name="Lualib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Lualib - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Lualib.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Lualib.mak" CFG="Lualib - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Lualib - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "Lualib - Win32 ReleaseLog" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Lualib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Lualib___Win32_Debug"
# PROP BASE Intermediate_Dir "Lualib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gi /GX /Od /Ob2 /I "..\..\..\..\src\lua\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "LUA_OPNAMES" /YX"lualib.pch" /FD /GZ /c
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\luad.lib"

!ELSEIF  "$(CFG)" == "Lualib - Win32 ReleaseLog"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Lualib___Win32_ReleaseLog"
# PROP BASE Intermediate_Dir "Lualib___Win32_ReleaseLog"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseLog"
# PROP Intermediate_Dir "ReleaseLog"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\source\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "LUA_OPNAMES" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\..\src\lua\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "LUA_OPNAMES" /YX"lualib.pch" /FD /c
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\lua.lib"
# ADD LIB32 /nologo /out:"ReleaseLog\lua.lib"

!ENDIF 

# Begin Target

# Name "Lualib - Win32 Debug"
# Name "Lualib - Win32 ReleaseLog"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\lua\lapi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lcode.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ldebug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ldo.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ldump.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lfunc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lgc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\llex.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lmem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lobject.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lopcodes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lparser.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lstate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lstring.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ltable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ltests.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ltm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lundump.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lvm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lzio.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=..\..\..\..\src\lua\lapi.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lcode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ldebug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ldo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lfunc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lgc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\llex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\llimits.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lmem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lobject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lopcodes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lparser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lstate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lstring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ltable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\ltm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\include\lua.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lundump.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lvm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\lua\lzio.h
# End Source File
# End Group
# End Target
# End Project
