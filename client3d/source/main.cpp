/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

#include "ortho.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__)
#include <conio.h>
#else
#include "wincompat.h"
#endif

#include "fmod.h"
#include "fmod_errors.h"    /* optional */


#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
    OrthoTestApplication app;

	// Init & start fmod
    FMUSIC_MODULE *mod = NULL;
	char buf[256];
	
    if (FSOUND_GetVersion() < FMOD_VERSION)
    {
        sprintf(buf,"Error : You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
        MessageBox( NULL, buf, "Fmod: Wrong DLL", MB_OK | MB_ICONERROR | MB_TASKMODAL);
        exit(1);
    }
    
    /*
	INITIALIZE FMOD
    */
    if (!FSOUND_Init(32000, 64, 0))
    {
        sprintf(buf,"%s\n", FMOD_ErrorString(FSOUND_GetError()));
        MessageBox( NULL, buf, "FMOD INIT ERROR", MB_OK | MB_ICONERROR | MB_TASKMODAL);
        exit(1);
    }
	/*
	LOAD SONG
	A s3m for fun... fmod of course plays nearly all, including .ogg and .wav
	*/
	mod = FMUSIC_LoadSong("media/sound/invtro94.s3m");
    if (!mod)
    {
        sprintf(buf,"%s\n", FMOD_ErrorString(FSOUND_GetError()));
        MessageBox( NULL, buf, "FMOD: Can't find sound file", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		exit(1);
    }
    FMUSIC_PlaySong(mod);   
	
	
    try {
        app.go();
    } catch( Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " << e.getFullDescription();
#endif
    }

    /*
	FREE SONG AND SHUT DOWN
    */
    FMUSIC_FreeSong(mod);
    FSOUND_Close();
	
	
    return 0;
}

#ifdef __cplusplus
}
#endif
