VC6 Install
-----------

To install and compile the Daimonin3D client you need the ogre libs & dlls.

You will find all what you need in the same folder as this README.txt.
To run/compile the Daimonin3D client under windows you need to do 

0.) Read the client3d/README.install. 
1.) entpack ogre_inc.zip to this folder (So you see /DevCpp, /VisualC and /ogre_inc as folder)
2.) entpack ogre_lib.zip to this folder (So you see /ogre_lib next to /ogre_inc, /VisualC and /DevCpp)
3.) do THE SAME for fmod_inc.zip and fmod_lib.zip
    You should see now next to /VisualC:  /fmod_inc, /fmod_lib, /ogre_inc and /ogre_lib
4a.) FOR DEBUG COMPILE: entpack dll_debug.zip to the main folder /client3d
4b.) FOR RELEASE COMPILE: entpack dll_release.zip to the main folder /client3d
5.) compile
6.) The daimonin3d.exe will appear in /client3d folder next to the dlls of ogre & fmod
7.) start it



DevCpp Install
--------------

1) Install the DevCpp like you was told in the Ogre3d Wiki.
2) Copy the whole srv_files directory from the 2d client to client3d/
3) Copy the daimonin.p0 from the 2d client to client3d/ 
4) Copy the plugins.cfg from client3d/make/win32/DevCpp to client3d/
5) Copy all *.dll from dev-Cpp/bin/OGRE to client3d/
6) start devCpp by doubleclick the project-file in client3d/make/win32/DevCpp

