@echo off
rem ========================================
rem (c) 2009 by the Daimonin team.
rem     www.daimonin.net
rem ========================================
echo -- ############################################################
echo -- #Setting up the environment
echo -- ############################################################

rem ========================================
rem Cut versions numbering from Ogre folder
rem ========================================
for /f "tokens=*" %%i in ('dir Ogre* /b /a:d') do set var=%%i
rename %var% OgreSDK
if not '%var%' == 'OgreSDK' echo * renamed %var% to OgreSDK

rem ========================================
rem Cut versions numbering from Boost folder
rem ========================================
for /f "tokens=*" %%i in ('dir OgreSDK\boost* /b /a:d') do set var=%%i
rename OgreSDK\%var% boost
if not '%var%' == 'boost' echo * renamed %var% to boost

rem ========================================
rem Cut versions numbering from Boost libs
rem ========================================
cd OgreSDK\boost\lib
for /f "tokens=*" %%i in ('dir libboost_thread*d*.lib /b') do set var=%%i
rename %var% debug.lib
if not '%var%' == 'libboost_thread_d.lib' echo * renamed %var% to libboost_thread_d.lib
for /f "tokens=*" %%i in ('dir libboost_thread*.lib /b') do set var=%%i
rename %var% libboost_thread.lib
if not '%var%' == 'libboost_thread.lib' echo * renamed %var% to libboost_thread.lib
rename debug.lib libboost_thread_d.lib
cd ..\..\..

rem ========================================
rem Delete old settings
rem ========================================
IF exist ..\..\..\ogre.cfg del ..\..\..\ogre.cfg

rem ========================================
rem Copy the dll's to main folder.
rem ========================================
copy .\OgreSDK\bin\release\cg.dll                      ..\..\..\cg.dll >nul

copy .\OgreSDK\bin\release\OgreMain.dll                ..\..\..\OgreMain.dll >nul
copy .\OgreSDK\bin\release\OIS.dll                     ..\..\..\OIS.dll >nul
copy .\OgreSDK\bin\release\Plugin_ParticleFX.dll       ..\..\..\Plugin_ParticleFX.dll >nul
copy .\OgreSDK\bin\release\Plugin_CgProgramManager.dll ..\..\..\Plugin_CgProgramManager.dll >nul
copy .\OgreSDK\bin\release\RenderSystem_GL.dll         ..\..\..\RenderSystem_GL.dll >nul
copy .\OgreSDK\bin\release\RenderSystem_Direct3D9.dll  ..\..\..\RenderSystem_Direct3D9.dll >nul

copy .\OgreSDK\bin\debug\OgreMain_d.dll                ..\..\..\OgreMain_d.dll >nul
copy .\OgreSDK\bin\debug\OIS_d.dll                     ..\..\..\OIS_d.dll >nul
copy .\OgreSDK\bin\debug\Plugin_ParticleFX_d.dll       ..\..\..\Plugin_ParticleFX_d.dll >nul
copy .\OgreSDK\bin\debug\Plugin_CgProgramManager_d.dll ..\..\..\Plugin_CgProgramManager_d.dll >nul
copy .\OgreSDK\bin\debug\RenderSystem_GL_d.dll         ..\..\..\RenderSystem_GL_d.dll >nul
copy  .\OgreSDK\bin\debug\RenderSystem_Direct3D9_D.dll  ..\..\..\RenderSystem_Direct3D9_d.dll >nul

rename .\Sound\libcAudio.a cAudio.a
copy .\Sound\*.dll ..\..\..\ >nul

rem ========================================
rem Use cmake to build the project files.
rem ========================================
echo --
CHOICE /C dr /N /M "-- Build Type [D]ebug or [R]elease?"
iF %ERRORLEVEL% == 2 set BUILD_TYPE=Release
iF %ERRORLEVEL% == 1 set BUILD_TYPE=Debug
echo --
cmake -G"CodeBlocks - MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ../../../

