VC6 Project
-----------

To install and compile the Daimonin3D client you need the ogre libs & dlls.

You will find all what you need in the same folder as this README.txt.
To run/compile the Daimonin3D client under windows you need to do 

1.) Read the client3d/README.install.
2.) entpack ogre_inc.zip to this folder (So you see /DevCpp, /VisualC and /ogre_inc as folder)
3.) entpack ogre_lib.zip to this folder (So you see /ogre_lib next to /ogre_inc, /VisualC and /DevCpp)
4.) do THE SAME for fmod_inc.zip and fmod_lib.zip
    You should see now next to /VisualC:  /fmod_inc, /fmod_lib, /ogre_inc and /ogre_lib
5a.) FOR DEBUG COMPILE: entpack dll_debug.zip to the main folder /client3d
5b.) FOR RELEASE COMPILE: entpack dll_release.zip to the main folder /client3d
6.) compile
7.) The daimonin3d.exe will appear in /client3d folder next to the dlls of ogre & fmod
8.) start it



DevCpp Project
--------------

1.) Read the client3d/README.install.
2.) Install the DevCpp like you was told in the Ogre3d Wiki.
3.) Copy the plugins.cfg from client3d/make/win32/DevCpp to client3d/
4.) Copy all *.dll from client3d/make/win32/dll_devcpp to client3d/
5.) start DevCpp by doubleclick the project-file in client3d/make/win32/DevCpp

