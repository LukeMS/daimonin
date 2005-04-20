To install and compile the Daimonin3D client you need the ogre libs & dlls.

You will find all what you need in the same folder as this README.txt.
To run/compile the Daimonin3D client under windows you need to do this:

Code::Blocks Project
--------------------

1.) Read the client3d/README.install.
2.) Install Code::Blocks like you was told in the Ogre3d Wiki.
3.) Copy the plugins.cfg from client3d/make/win32/CodeBlocks to client3d/
4.) Unpack all *.dll from client3d/make/win32/dll_gcc to      client3d/make/win32/CodeBlocks/
5.) Unpack all *.a   from client3d/make/win32/ogre_lib_gcc to client3d/make/win32/CodeBlocks/
6.) start Code::Blocks by doubleclick the project-file in     client3d/make/win32/CodeBlocks/


VC6 Project
-----------

TODO: we must update all VC6 ogre files. They still have version 1.0.0

1.) Read the client3d/README.install.
2.) Unpack ogre_inc.zip to this folder (So you see /DevCpp, /VisualC and /ogre_inc as folder)
3.) Unpack ogre_lib.zip to this folder (So you see /ogre_lib next to /ogre_inc, /VisualC and /DevCpp)
4.) do THE SAME for fmod_inc.zip and fmod_lib.zip
    You should see now next to /VisualC:  /fmod_inc, /fmod_lib, /ogre_inc and /ogre_lib
5a.) FOR DEBUG COMPILE: Unpack dll_debug.zip to the main folder /client3d
5b.) FOR RELEASE COMPILE: Unpack dll_release.zip to the main folder /client3d
6.) compile
7.) The daimonin3d.exe will appear in /client3d folder next to the dlls of ogre & fmod
8.) start it
