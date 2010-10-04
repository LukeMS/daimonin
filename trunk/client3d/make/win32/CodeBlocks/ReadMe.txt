//===================================================
// Client3d as Code::Blocks project HOWTO
//===================================================

-------------------------------------------------------------
CMake Download Page (http://www.cmake.org/cmake/resources/software.html)
-------------------------------------------------------------
* Install cmake for Windows(Win32 Installer)

-------------------------------------------------------------
Ogre SDK Download Page (http://www.ogre3d.org/download/sdk)
-------------------------------------------------------------
* Install the latest Ogre SDK for MinGW into "client3d/make/win32/CodeBlocks/".
* Install the recommended MinGW package.

-------------------------------------------------------------
CodeBlocks (http://www.codeblocks.org/)
-------------------------------------------------------------
* Install the latest nightly build of Code::Blocks.

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
* run the configure.bat
* Dblclk on the Code::Blocks project file

-------------------------------------------------------------
Inside code::blocks
-------------------------------------------------------------
* Select a build target (Release or Debug)
* Build and run
