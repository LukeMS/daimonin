/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include "client.h"
#include "option.h"
#include "logfile.h"
#include "network.h"
#include "xyz.h"



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
	DaimoninClient client;

	// Init & start fmod
    FMUSIC_MODULE *mod = NULL;
	char buf[256];
	
    if (FSOUND_GetVersion() < FMOD_VERSION)
    {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        sprintf(buf,"Error : You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
        MessageBox( NULL, buf, "Fmod: Wrong DLL", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "FMOD: Wrong version. You need " << FMOD_VERSION << std::endl;
#endif
        return 1;
    }
    
    /*
	INITIALIZE FMOD
    */
    if (!FSOUND_Init(32000, 64, 0))
    {
        sprintf(buf,"%s\n", FMOD_ErrorString(FSOUND_GetError()));
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, buf, "FMOD INIT ERROR", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "FMOD: INIT ERROR " << buf;
#endif
        return 1;
    }
	/*
	LOAD SONG
	A s3m for fun... fmod of course plays nearly all, including .ogg and .wav
	*/
	mod = FMUSIC_LoadSong("media/sound/invtro94.s3m");
    if (!mod)
    {
        sprintf(buf,"%s\n", FMOD_ErrorString(FSOUND_GetError()));
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, buf, "FMOD: Can't find sound file", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "FMOD: Can't find sound file: " << buf;
#endif
        return 1;
    }
    FMUSIC_PlaySong(mod);   
	
	
    try {
        client.go();
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
