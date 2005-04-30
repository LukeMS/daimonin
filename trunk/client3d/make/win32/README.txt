To install and compile the Daimonin3D client you need the ogre libs & dlls.

You will find all what you need in the same folder as this README.txt.
To run/compile the Daimonin3D client under windows you need to do this:

Code::Blocks Project
--------------------

1.) Install Code::Blocks like you was told in the Ogre3d Wiki.
2.) Copy the plugins.cfg from client3d/make/win32/CodeBlocks to client3d/
3.) Unpack all  *.dll from client3d/make/win32/dll_gcc.tgz       to  client3d/
4.) Unpack all  *.a   from client3d/make/win32/fmod_lib.zip      to  client3d/make/win32/fmod_lib/
5.) Unpack all  *.a   from client3d/make/win32/ogre_lib_gcc.tgz  to  client3d/make/win32/ogre_lib_gcc/
6.) Unpack all  *.h   from client3d/make/win32/fmod_inc.zip      to  client3d/make/win32/fmod_inc/
7.) Unpack all  *.h   from client3d/make/win32/ogre_inc.zip      to  client3d/make/win32/ogre_inc/
8.) start Code::Blocks by doubleclick the project-file in client3d/make/win32/CodeBlocks/


VC6 Project
-----------

1.) Read the client3d/README.install.
2.) Unpack client3d/make/win32/fmod_lib.zip     to  client3d/make/win32/fmod_lib/
3.) Unpack client3d/make/win32/ogre_lib.zip     to  client3d/make/win32/ogre_lib/
4.) Unpack client3d/make/win32/fmod_inc.zip     to  client3d/make/win32/fmod_inc/
5.) Unpack client3d/make/win32/ogre_inc.zip     to  client3d/make/win32/ogre_inc/
choose one of the following:
6.) Unpack client3d/make/win32/dll_release.zip  to  client3d/
6.) Unpack client3d/make/win32/dll_debug.zip    to  client3d/
compile and start client3d/daimonin3d.exe

last updated: 30.april-05 <polyveg>