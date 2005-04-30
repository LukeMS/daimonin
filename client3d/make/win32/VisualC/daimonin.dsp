# Microsoft Developer Studio Project File - Name="Daimonin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Daimonin - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "daimonin.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "daimonin.mak" CFG="Daimonin - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Daimonin - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE "Daimonin - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Daimonin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\bin\Debug"
# PROP BASE Intermediate_Dir "..\..\..\obj\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\..\source\include" /I "..\ogre_inc" /I "..\fmod_inc" /D "_MBCS" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /YX"daimonin.pch" /FD /GZ /Zm500 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /pdbtype:sept
# ADD LINK32 ws2_32.lib fmodvc.lib OgreMain_d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"LIBCMT" /out:"Debug/daimonin3d.exe" /pdbtype:sept /libpath:"..\ogre_lib" /libpath:"..\fmod_lib"
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\*.exe ..\..\..\*.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Daimonin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Daimonin___Win32_Release"
# PROP BASE Intermediate_Dir "Daimonin___Win32_Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\..\source\include" /I "..\ogre_inc" /I "..\fmod_inc" /D "_MBCS" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /FD /GZ /Zm500 /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "..\..\..\source\include" /I "..\ogre_inc" /I "..\fmod_inc" /D "_MBCS" /D "_WINDOWS" /D "WIN32" /YX"daimonin.pch" /FD /Zm500 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib fmodvc.lib OgreMain_d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:IX86 /nodefaultlib:"LIBCMT" /out:"Debug/daimonin3d.exe" /pdbtype:sept /libpath:"..\ogre_lib" /libpath:"..\fmod_lib"
# SUBTRACT BASE LINK32 /verbose
# ADD LINK32 ws2_32.lib fmodvc.lib OgreMain.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:IX86 /nodefaultlib:"LIBCMT" /out:"Release/daimonin3d.exe" /libpath:"..\ogre_lib" /libpath:"..\fmod_lib"
# SUBTRACT LINK32 /verbose /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\*.exe ..\..\..\*.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Daimonin - Win32 Debug"
# Name "Daimonin - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\source\animate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\client.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\dialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\event.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\logfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\network.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\network_cmd.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\object_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\object_npc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\object_static.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\option.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\option_init.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\player.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\serverfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\sound.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\textwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\tile_gfx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\tile_map.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\source\include\animate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\client.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\define.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\dialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\event.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\logfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\network.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\object_manager.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\object_npc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\object_static.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\option.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\player.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\serverfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\sound.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\sprite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\textinput.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\textwindow.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\tile_gfx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\tile_map.h
# End Source File
# End Group
# Begin Group "3rd Party"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\source\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\source\zutil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\source\include\zutil.h
# End Source File
# End Group
# Begin Group "Readme"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=..\README.txt
# End Source File
# End Group
# End Target
# End Project
