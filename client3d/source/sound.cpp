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

#include "fmod.h"
#include "fmod_errors.h"  //optional.
#include "sound.h"
#include "logfile.h"

struct _sample
{
	FSOUND_SAMPLE *handle;
	const char *filename;
};

_sample Sample[SAMPLE_SUM]=
{
	{ 0, "media/sound/console.wav"  }, // SAMPLE_BUTTON_CLICK
	{ 0, "media/sound/Idle.ogg"     }  // SAMPLE_PLAYER_IDLE
};

static FMUSIC_MODULE *mod    = 0;


// ========================================================================
// Init the sound-system.
// ========================================================================
bool Sound::Init()
{
    LogFile::getSingelton().Headline("Init Soundystem");
    LogFile::getSingelton().Info("Starting fmod...");

    ///////////////////////////////////////////////////////////////////////// 
    // Check Version.
	/////////////////////////////////////////////////////////////////////////
    if (FSOUND_GetVersion() < FMOD_VERSION)
    {
		LogFile::getSingelton().Success(false);
        LogFile::getSingelton().Error("You are using the wrong DLL version! "
			"You should be using FMOD %.02f\n", FMOD_VERSION);
        return false;
    }

    ///////////////////////////////////////////////////////////////////////// 
    // Init Fmod.
	/////////////////////////////////////////////////////////////////////////
    if (!FSOUND_Init(32000, 64, 0))
    {
		LogFile::getSingelton().Success(false);
        LogFile::getSingelton().Error("FSound init: %s\n", FMOD_ErrorString(FSOUND_GetError()));
        return false;
    }
	LogFile::getSingelton().Success(true);

    ///////////////////////////////////////////////////////////////////////// 
    // Load background music.
	/////////////////////////////////////////////////////////////////////////
    LogFile::getSingelton().Info("Loading music...");
	mod = FMUSIC_LoadSong("media/sound/invtro94.s3m");
    if (!mod)
    {
		LogFile::getSingelton().Success(false);
        LogFile::getSingelton().Error("Song load: %s\n", FMOD_ErrorString(FSOUND_GetError()));
        return false;
    }
	LogFile::getSingelton().Success(true);

	
	
	


	FMUSIC_SetMasterVolume(mod, 50);
	FMUSIC_PlaySong(mod);





    ///////////////////////////////////////////////////////////////////////// 
    // Load samples.
	/////////////////////////////////////////////////////////////////////////
    LogFile::getSingelton().Info("Loading samples...");
    for (unsigned int i = 0; i< SAMPLE_SUM; ++i)
	{ 
		if (!(Sample[i].handle = FSOUND_Sample_Load(i, Sample[i].filename, 0,0,0)))
		{
			LogFile::getSingelton().Success(false);
			LogFile::getSingelton().Error("Sample load: %s\n", FMOD_ErrorString(FSOUND_GetError()));
		}
	}
	LogFile::getSingelton().Success(true);
	return true;
}

// ========================================================================
// Plays a sound.
// ========================================================================
void Sound::PlaySample(unsigned int index)
{
	if (index < SAMPLE_SUM && Sample[index].handle)
	{ 
		FSOUND_PlaySound(index, Sample[index].handle);
	}
}

// ========================================================================
// Destructor.
// ========================================================================
Sound::~Sound()
{
    FMUSIC_FreeSong(mod);
    for (unsigned int i = 0; i< SAMPLE_SUM; ++i)
	{ 
		if (Sample[i].handle) { FSOUND_Sample_Free(Sample[i].handle); }
	}

    FSOUND_Close();
}
