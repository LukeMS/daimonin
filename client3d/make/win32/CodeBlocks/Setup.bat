rem ====================================
rem (c) 2005 by the Daimonin team.
rem     www.daimonin.net
rem ====================================

rem ====================================
rem Unpack all files.
rem ====================================
..\..\_Tools_\gunzip ogre_lib.tgz 
..\..\_Tools_\tar xvf ogre_lib.tar
..\..\_Tools_\gunzip ogre_inc.tgz
..\..\_Tools_\tar xvf ogre_inc.tar
..\..\_Tools_\gunzip ogre_dll.tgz
..\..\_Tools_\tar xvf ogre_dll.tar

..\..\_Tools_\gunzip fmod_lib.tgz 
..\..\_Tools_\tar xvf fmod_lib.tar
..\..\_Tools_\gunzip fmod_dll.tgz
..\..\_Tools_\tar xvf fmod_dll.tar

rem ====================================
rem Copy the dll's to main folder.
rem ====================================
move ogre_dll\*.* ..\..\..\
move fmod_dll\*.* ..\..\..\
rmdir ogre_dll
rmdir fmod_dll

rem ====================================
rem We need the Code::Blocks plugins.
rem ====================================
copy plugins.cfg ..\..\..\

rem ====================================
rem Clean up.
rem ====================================
del *.tar
