# Microsoft Developer Studio Project File - Name="Daimonin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Daimonin - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Daimonin.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Daimonin.mak" CFG="Daimonin - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Daimonin - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "Daimonin - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Daimonin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /Ox /Ob2 /I "../../../" /I "../../../src/include" /I "../../../sdl" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "__WIN_32" /YX"include.h" /FD /c
# SUBTRACT CPP /Oa /Ow
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /subsystem:windows /pdb:none /machine:I386
# SUBTRACT LINK32 /profile /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\Daimonin.exe ..\..\..\Daimonin.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Daimonin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "../../../" /I "../../../src/include" /I "../../../sdl" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "__WIN_32" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\Daimonin.exe ..\..\..\Daimonin.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Daimonin - Win32 Release"
# Name "Daimonin - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\book.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\client.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\commands.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dialog.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\event.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\filewrap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\group.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\interface.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\inventory.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\item.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\map.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\menu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\player.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\socket.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\sound.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\sprite.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\textwin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\wrapper.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zutil.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\include\book.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\cflinux.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\client.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\commands.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\dialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\event.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\filewrap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\group.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\include.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\interface.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\inventory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\item.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\main.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\map.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\menu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\misc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\player.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\sdlsocket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\sprite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\textwin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\win32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\wrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\include\zutil.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=..\..\..\sdl\SDL.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\sdl\SDL_image.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\sdl\SDL_mixer.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\sdl\SDLmain.lib
# End Source File
# End Target
# End Project
