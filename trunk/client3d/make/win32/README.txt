To install and compile the Daimonin3D client you need the ogre libs & dlls.

** ATM only the Visual Studio 6++ makefile works **

You will find all what you need in the same folder as this README.txt.
To run/compile the Daimonin3D client under windows you need to do 

1.) entpack ogre_inc.zip to this folder (So you see /DevCpp, /VisualC and /ogre_inc as folder)
2.) entpack ogre_lib.zip to this folder (So you see /ogre_lib next to /ogre_inc, /VisualC and /DevCpp)
3.) do THE SAME for fmod_inc.zip and fmod_lib.zip
    You should see now next to /VisualC:  /fmod_inc, /fmod_lib, /ogre_inc and /ogre_lib

4a.) FOR DEBUG COMPILE: entpack dll_debug.zip to the main folder /client3d
4b.) FOR RELEASE COMPILE: entpack dll_release.zip to the main folder /client3d

5.) compile
6.) The daimonin3d.exe will appear in /client3d folder next to the dlls of ogre & fmod
7.) start it
