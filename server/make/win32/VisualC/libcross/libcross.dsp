# Microsoft Developer Studio Project File - Name="libcross" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=LIBCROSS - WIN32 RELEASELOG
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "libcross.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "libcross.mak" CFG="LIBCROSS - WIN32 RELEASELOG"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "libcross - Win32 ReleaseLog" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "libcross - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "libcross - Win32 ReleaseDebug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libcross - Win32 ReleaseLog"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libcross___Win32_ReleaseLog"
# PROP BASE Intermediate_Dir "libcross___Win32_ReleaseLog"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseLog"
# PROP Intermediate_Dir "ReleaseLog"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\include" /I "d:\Python21\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "PYTHON_PLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\..\src\include" /I "c:\Python23\include" /D "_LIB" /D "PYTHON_PLUGIN_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG" /YX"libcross.pch" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libcross - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libcross___Win32_Debug"
# PROP BASE Intermediate_Dir "libcross___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\..\source\include" /I "d:\Python22\include" /D "_LIB" /D "PYTHON_PLUGIN_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG_MOVEATTACK" /YX"libcross.pch" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /Ob2 /I "..\..\..\..\src\include" /I "c:\Python23\include" /D "_CONSOLE" /D "DEBUG" /D "DEBUG_MOVEATTACK" /D "PYTHON_PLUGIN_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX"libcross.pch" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libcross - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libcross___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "libcross___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /Ob2 /I "..\..\..\..\src\include" /I "d:\Python22\include" /D "_LIB" /D "PYTHON_PLUGIN_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG" /YX"libcross.pch" /FD /c
# ADD CPP /nologo /MD /W3 /Gm /Gi /GX /Zi /Ob2 /I "..\..\..\..\src\include" /I "c:\Python23\include" /D "_LIB" /D "PYTHON_PLUGIN_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG" /YX"libcross.pch" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libcross - Win32 ReleaseLog"
# Name "libcross - Win32 Debug"
# Name "libcross - Win32 ReleaseDebug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\common\anim.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\arch.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\button.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\exp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\friend.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\glue.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\holy.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\image.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\info.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\init.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\item.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\links.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\living.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\loader.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\loader.l

!IF  "$(CFG)" == "libcross - Win32 ReleaseLog"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=\Projects\daimonin\server\src\common
InputPath=..\..\..\..\src\common\loader.l
InputName=loader

"$(InputDir)\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\..\dev\win32\flex.exe -i  -o$(InputDir)\$(InputName).c $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libcross - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=\Projects\daimonin\server\src\common
InputPath=..\..\..\..\src\common\loader.l
InputName=loader

"$(InputDir)\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\..\dev\win32\flex.exe -i  -o$(InputDir)\$(InputName).c $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "libcross - Win32 ReleaseDebug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=\Projects\daimonin\server\src\common
InputPath=..\..\..\..\src\common\loader.l
InputName=loader

"$(InputDir)\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\..\dev\win32\flex.exe -i  -o$(InputDir)\$(InputName).c $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\logger.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\los.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\map.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\object.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\player.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\porting.c
# End Source File
# Begin Source File

SOURCE="..\..\..\..\src\common\re-cmp.c"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\readable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\recipe.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\shstr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\time.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\treasure.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\common\win32.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\include\arch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\artifact.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\attack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\book.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\commands.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\define.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\face.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\funcpoint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\global.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\god.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\includes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\libproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\living.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\loader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\logger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\material.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\newclient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\newserver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\object.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\player.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\plugin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\plugproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\race.h
# End Source File
# Begin Source File

SOURCE="..\..\..\..\src\include\re-cmp.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\recipe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\shstr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\skillist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\skills.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\sockproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\sounds.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\spellist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\spells.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\sproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\timers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\tod.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\treasure.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\version.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\src\include\xdir.h
# End Source File
# End Group
# End Target
# End Project
