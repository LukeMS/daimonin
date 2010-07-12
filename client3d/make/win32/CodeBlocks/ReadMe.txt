///===================================================
/// Client3d as Code::Blocks project HOWTO
///===================================================

-------------------------------------------------------------
Ogre SDK Download Page (http://www.ogre3d.org/download/sdk)
-------------------------------------------------------------
* Install the latest Ogre SDK for MinGW into "client3d/make/win32/CodeBlocks/".
* Rename the folder    "OgreSDK*" to "OgreSDK".
* Rename the subfolder "boost*"   to "boost".
* Rename the boost lib "libboost_thread*.lib"    to "boost_thread.lib"
* Rename the boost lib "libboost_thread*-d*.lib" to "boost_thread_d.lib"
* Install the MinGW package.

-------------------------------------------------------------
CodeBlocks (http://www.codeblocks.org/)
-------------------------------------------------------------
Install the latest nightly build of Code::Blocks.

-------------------------------------------------------------
cAudio (http://sourceforge.net/projects/caudio/)
-------------------------------------------------------------
download cAudio and copy these files to "client3d/make/win32/CodeBlocks/Sound/"
* libcAudio.a
* cAp_EAXLegacyPreset.dll
* cAp_MP3Decoder.dll
* cAudio.dll
* OpenAL32.dll
* wrap_oal.dll

-------------------------------------------------------------
In client3d/make/win32/CodeBlocks/
-------------------------------------------------------------
* run the setup.bat
* Dblclk on the Code::Blocks project file

-------------------------------------------------------------
Inside code::blocks
-------------------------------------------------------------
* Select a build target (Release or Debug)
* Build and run
