rem ====================================
rem (c) 2005 by the Daimonin team.
rem     www.daimonin.net
rem ====================================

rem ====================================
rem Unpack all files.
rem ====================================
..\..\_Tools_\gunzip -c sdl_lib.tgz >sdl_lib.tar
..\..\_Tools_\tar xvf sdl_lib.tar
..\..\_Tools_\gunzip -c sdl_inc.tgz >sdl_inc.tar
..\..\_Tools_\tar xvf sdl_inc.tar 
..\..\_Tools_\gunzip -c sdl_dll.tgz >sdl_dll.tar
..\..\_Tools_\tar xvf sdl_dll.tar
..\..\_Tools_\gunzip -c curl.tgz >curl.tar
..\..\_Tools_\tar xvf curl.tar
rem ====================================
rem Copy the dll's to main folder.
rem ====================================
move sdl_dll\*.* ..\..\..\
rmdir sdl_dll
copy physfs.dll ..\..\..\

rem ====================================
rem Clean up.
rem ====================================
del *.tar
