rem ====================================
rem (c) 2005 by the Daimonin team.
rem     www.daimonin.net
rem ====================================

rem ====================================
rem Unpack all files.
rem ====================================
..\..\_Tools_\gunzip -c client3d_build_cb.tgz >build.tar
..\..\_Tools_\gunzip -c client3d_build_cb.gz >build.tar
..\..\_Tools_\tar xvf build.tar
..\..\_Tools_\gunzip -c fmod_lib.tgz >fmod_lib.tar
..\..\_Tools_\tar xvf fmod_lib.tar
..\..\_Tools_\gunzip -c fmod_dll.tgz >fmod_dll.tar
..\..\_Tools_\tar xvf fmod_dll.tar
..\..\_Tools_\gunzip -c ogre_dll.tgz >ogre_dll.tar
..\..\_Tools_\tar xvf ogre_dll.tar
..\..\_Tools_\gunzip -c ogre_inc.tgz >ogre_inc.tar
..\..\_Tools_\tar xvf ogre_inc.tar
..\..\_Tools_\gunzip -c sdl_dll.tgz >sdl_dll.tar
..\..\_Tools_\tar xvf sdl_dll.tar

rem ====================================
rem Copy the dll's to main folder.
rem ====================================
mkdir ..\..\..\debug
copy sdl_dll\*.* ..\..\..\debug
move sdl_dll\*.* ..\..\..\
move ogre_dll\debug\*.* ..\..\..\debug
copy fmod_dll\*.* ..\..\..\debug
move ogre_dll\*.* ..\..\..\
move fmod_dll\*.* ..\..\..\
rmdir ogre_dll\debug
rmdir ogre_dll
rmdir fmod_dll
rmdir sdl_dll

rem ====================================
rem Clean up.
rem ====================================
del *.tar
del fmod_dll.tgz
del fmod_lib.tgz
del ogre_dll.tgz
del ogre_inc.tgz
del sdl_dll.tgz
pause