//===================================================
// Client3d as Visual c++ project HOWTO
//===================================================

-------------------------------------------------------------
Ogre SDK Download Page (http://www.ogre3d.org/download/sdk)
-------------------------------------------------------------
* Install the latest Ogre SDK for VisualC++ into "\client3d\make\win32\VisualC\".
* Rename the folder    "OgreSDK*" to "OgreSDK".
* Rename the subfolder "boost*"   to "boost".
* Rename the boost lib "libboost_thread*.lib"   to "boost_thread.lib"
* Rename the boost lib "libboost_thread*-d.lib" to "boost_thread_d.lib"

-------------------------------------------------------------
Visual Studio (Readme must be updated to the latest version!)
-------------------------------------------------------------
* Install Visual c++ 2005 Express Edition
* Install Windows Platform SDK
* Copy "bin", "include" and "lib" from PlatformSDK into \Microsoft Visual Studio\VC\

-------------------------------------------------------------
cAudio (http://sourceforge.net/projects/caudio/)
-------------------------------------------------------------
download cAudio and copy these files to "client3d/make/win32/CodeBlocks/Sound/"
* cAudio.lib
* cAp_EAXLegacyPreset.dll
* cAp_MP3Decoder.dll
* cAudio.dll
* OpenAL32.dll
* wrap_oal.dll

-------------------------------------------------------------
In client3d/make/win32/VisaulC/
-------------------------------------------------------------
* run the setup.bat
* Dblclk on the project file

-------------------------------------------------------------
Inside of VisualC
-------------------------------------------------------------
* Compile (If you are asked to save the project-file -> press save)
