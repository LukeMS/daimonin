# Microsoft Developer Studio Project File - Name="daimonin_server" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=daimonin_server - Win32 ReleaseLog
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "daimonin_server.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "daimonin_server.mak" CFG="daimonin_server - Win32 ReleaseLog"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "daimonin_server - Win32 ReleaseLog" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "daimonin_server - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "daimonin_server - Win32 ReleaseDebug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "daimonin_server___Win32_ReleaseLog"
# PROP BASE Intermediate_Dir "daimonin_server___Win32_ReleaseLog"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseLog"
# PROP Intermediate_Dir "ReleaseLog"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\src\include" /I "..\..\..\src\random_maps" /I "..\..\..\src\plugin_python\include" /I "c:\Python23\include" /D "_CONSOLE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG" /YX"preheader.pch" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 libcross.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /libpath:"libcross\ReleaseLog" /libpath:"c:\python23\libs"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ReleaseLog\*.exe ..\..\..\*.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "daimonin_server___Win32_Debug"
# PROP BASE Intermediate_Dir "daimonin_server___Win32_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /O2 /Ob2 /I "..\..\..\source\include" /I "..\..\..\source\random_maps" /I "..\..\..\source\plugin_python\include" /I "d:\Python22\include" /D "_CONSOLE" /D "DEBUG" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG_MOVEATTACK" /YX"preheader.pch" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /Op /Ob2 /I "..\..\..\src\include" /I "..\..\..\src\random_maps" /I "..\..\..\src\plugin_python\include" /I "c:\Python23\include" /D "_CONSOLE" /D "DEBUG" /D "DEBUG_MOVEATTACK" /D "WIN32" /D "NDEBUG" /D "_MBCS" /FR /YX"preheader.pch" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /libpath:"d:\python22\libs"
# ADD LINK32 libcross.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /map /debug /machine:I386 /libpath:"libcross\Debug" /libpath:"c:\python23\libs" /fixed:no
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\*.exe ..\..\..\*.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "daimonin_server___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "daimonin_server___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /Ob2 /I "..\..\..\src\include" /I "..\..\..\src\random_maps" /I "..\..\..\src\plugin_python\include" /I "d:\Python22\include" /D "_CONSOLE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG" /YX"preheader.pch" /FD /c
# ADD CPP /nologo /MD /W3 /Gm /Gi /GX /Zi /Ob2 /I "..\..\..\src\include" /I "..\..\..\src\random_maps" /I "..\..\..\src\plugin_python\include" /I "c:\Python23\include" /D "_CONSOLE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "DEBUG" /YX"preheader.pch" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libcross.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /machine:I386 /libpath:"d:\python22\libs" /libpath:"libcross\ReleaseLog"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 libcross.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:none /debug /machine:I386 /libpath:"libcross\ReleaseDebug" /libpath:"c:\python23\libs"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ReleaseDebug\*.exe ..\..\..\*.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "daimonin_server - Win32 ReleaseLog"
# Name "daimonin_server - Win32 Debug"
# Name "daimonin_server - Win32 ReleaseDebug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\server\aiconfig.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\alchemy.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\apply.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\attack.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\ban.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_chat.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_misc.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_move.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_new.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_object.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_party.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_range.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\c_wiz.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\commands.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\disease.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\egoitem.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\gods.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\hiscore.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\init.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\login.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\main.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\monster.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\move.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\pathfinder.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\pets.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\player.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\plugins.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\resurrection.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\rune.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\shop.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\skill_util.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\skills.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\spell_effect.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\spell_util.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\swamp.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\swap.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\time.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\timers.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\server\weather.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\server"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\server"
# PROP Intermediate_Dir "ReleaseLog\server"

!ENDIF 

# End Source File
# End Group
# Begin Group "socket"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\socket\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\image.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\info.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\init.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\item.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\loop.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\lowlevel.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\metaserver.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\request.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\sounds.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\socket"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\socket"
# PROP Intermediate_Dir "ReleaseLog\socket"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\zutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket\zutil.h
# End Source File
# End Group
# Begin Group "random_maps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\random_maps\decor.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\door.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\exit.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\expand2x.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\expand2x.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\floor.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\maze_gen.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\maze_gen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\monster.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\random_map.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\random_map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\reader.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\reader.l

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=\Projects\daimonin\server\src\random_maps
InputPath=..\..\..\src\random_maps\reader.l
InputName=reader

"$(InputDir)\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\dev\win32\flex.exe -i -Prmap -o$(InputDir)\$(InputName).c $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=\Projects\daimonin\server\src\random_maps
InputPath=..\..\..\src\random_maps\reader.l
InputName=reader

"$(InputDir)\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\dev\win32\flex.exe -i -Prmap -o$(InputDir)\$(InputName).c $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=\Projects\daimonin\server\src\random_maps
InputPath=..\..\..\src\random_maps\reader.l
InputName=reader

"$(InputDir)\$(InputName).c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\dev\win32\flex.exe -i -Prmap -o$(InputDir)\$(InputName).c $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\rogue_layout.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\room_gen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\room_gen_onion.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\room_gen_spiral.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\rproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\snake.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\special.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\square_spiral.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\style.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\treasure.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\random_maps\wall.c

!IF  "$(CFG)" == "daimonin_server - Win32 ReleaseLog"

# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 Debug"

# PROP Intermediate_Dir "Debug\random_maps"

!ELSEIF  "$(CFG)" == "daimonin_server - Win32 ReleaseDebug"

# PROP BASE Intermediate_Dir "ReleaseLog\random_maps"
# PROP Intermediate_Dir "ReleaseLog\random_maps"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\include\aiconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\arch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\artifact.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\attack.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\behaviourdecl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\book.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\commands.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\define.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\face.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\funcpoint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\global.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\god.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\includes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\libproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\links.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\living.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\loader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\logger.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\material.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\mempool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\monster.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\newclient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\newserver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\object.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\pathfinder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\player.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\plugin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\plugproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\race.h
# End Source File
# Begin Source File

SOURCE="..\..\..\src\include\re-cmp.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\recipe.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\shstr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\skillist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\skills.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\sockproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\sounds.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\spellist.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\spells.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\sproto.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\timers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\tod.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\treasure.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\version.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\xdir.h
# End Source File
# End Group
# End Target
# End Project
