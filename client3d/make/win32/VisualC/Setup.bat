rem ====================================
rem (c) 2005 by the Daimonin team.
rem     www.daimonin.net
rem ====================================

del ..\..\..\ogre.cfg
rem ====================================
rem Copy the dll's to main folder.
rem ====================================
copy .\OgreSDK\bin\release\cg.dll                      ..\..\..\cg.dll

copy .\OgreSDK\bin\release\OgreMain.dll                ..\..\..\OgreMain.dll
copy .\OgreSDK\bin\release\OIS.dll                     ..\..\..\OIS.dll
copy .\OgreSDK\bin\release\Plugin_ParticleFX.dll       ..\..\..\Plugin_ParticleFX.dll
copy .\OgreSDK\bin\release\Plugin_CgProgramManager.dll ..\..\..\Plugin_CgProgramManager.dll
copy .\OgreSDK\bin\release\RenderSystem_GL.dll         ..\..\..\RenderSystem_GL.dll
copy .\OgreSDK\bin\release\RenderSystem_Direct3D9.dll  ..\..\..\RenderSystem_Direct3D9.dll

copy .\OgreSDK\bin\debug\OgreMain_d.dll                ..\..\..\OgreMain_d.dll
copy .\OgreSDK\bin\debug\OIS_d.dll                     ..\..\..\OIS_d.dll
copy .\OgreSDK\bin\debug\Plugin_ParticleFX_d.dll       ..\..\..\Plugin_ParticleFX_d.dll
copy .\OgreSDK\bin\debug\Plugin_CgProgramManager_d.dll ..\..\..\Plugin_CgProgramManager_d.dll
copy .\OgreSDK\bin\debug\RenderSystem_GL_d.dll         ..\..\..\RenderSystem_GL_d.dll
copy .\OgreSDK\bin\debug\RenderSystem_Direct3D9_D.dll  ..\..\..\RenderSystem_Direct3D9_d.dll