# Microsoft Developer Studio Project File - Name="plugin_python" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PLUGIN_PYTHON - WIN32 RELEASELOG
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "plugin_python.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "plugin_python.mak" CFG="PLUGIN_PYTHON - WIN32 RELEASELOG"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "plugin_python - Win32 ReleaseLog" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "plugin_python - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "plugin_python - Win32 ReleaseDebug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "plugin_python - Win32 ReleaseLog"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseLog"
# PROP BASE Intermediate_Dir "ReleaseLog"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseLog"
# PROP Intermediate_Dir "ReleaseLog"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PLUGIN_PYTHON_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /Ob2 /I "..\..\..\..\src\include" /I "..\..\..\..\src\plugin_python\include" /I "c:\python22\include" /D "_WINDOWS" /D "_USRDLL" /D "PYTHON_PLUGIN_EXPORTS" /D "DEBUG_MOVEATTACK" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX"plugin.pch" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 libcross.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /libpath:"..\libcross\ReleaseLog" /libpath:"c:\python22\libs"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ReleaseLog\*.dll ..\..\..\..\plugins\*.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "plugin_python - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "plugin_python___Win32_Debug"
# PROP BASE Intermediate_Dir "plugin_python___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\..\source\include" /I "..\..\..\..\source\plugin_python\include" /I "d:\python22\include" /D "_WINDOWS" /D "_USRDLL" /D "PYTHON_PLUGIN_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG_MOVEATTACK" /YX"plugin.pch" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /Ob2 /I "..\..\..\..\src\include" /I "..\..\..\..\src\plugin_python\include" /I "c:\python22\include" /D "_WINDOWS" /D "_USRDLL" /D "PYTHON_PLUGIN_EXPORTS" /D "DEBUG_MOVEATTACK" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG" /YX"plugin.pch" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libcross.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /libpath:"d:\python22\libs" /libpath:"..\libcross\ReleaseLog"
# ADD LINK32 libcross.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /libpath:"..\libcross\Debug" /libpath:"c:\python22\libs" /fixed:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\*.dll ..\..\..\..\plugins\*.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "plugin_python - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "plugin_python___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "plugin_python___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /Ob2 /I "..\..\..\..\src\include" /I "..\..\..\..\src\plugin_python\include" /I "d:\python22\include" /D "_WINDOWS" /D "_USRDLL" /D "PYTHON_PLUGIN_EXPORTS" /D "DEBUG_MOVEATTACK" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX"plugin.pch" /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /Ob2 /I "..\..\..\..\src\include" /I "..\..\..\..\src\plugin_python\include" /I "c:\python22\include" /D "_WINDOWS" /D "_USRDLL" /D "PYTHON_PLUGIN_EXPORTS" /D "DEBUG_MOVEATTACK" /D "WIN32" /D "NDEBUG" /D "_MBCS" /YX"plugin.pch" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libcross.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /libpath:"d:\python22\libs" /libpath:"..\libcross\ReleaseLog"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 libcross.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /pdb:none /machine:I386 /libpath:"..\libcross\ReleaseDebug" /libpath:"c:\python22\libs"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ReleaseDebug\*.dll ..\..\..\..\plugins\*.dll
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "plugin_python - Win32 ReleaseLog"
# Name "plugin_python - Win32 Debug"
# Name "plugin_python - Win32 ReleaseDebug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\src\plugin_python\plugin_python.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\src\plugin_python\include\plugin_python.h
# End Source File
# End Group
# End Target
# End Project
