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
#include "define.h"
#include "sound.h"
#include "logfile.h"

struct _sample
{
	FSOUND_SAMPLE *handle;
	const char *filename;
};

_sample Sample[SAMPLE_SUM]=
{
	{ 0, FILE_SAMPLE_MOUSE_CLICK }, // SAMPLE_BUTTON_CLICK
	{ 0, FILE_SAMPLE_PLAYER_IDLE }  // SAMPLE_PLAYER_IDLE
};

static FMUSIC_MODULE *mpSong   =0;
static FSOUND_STREAM *mpStream =0;

// ========================================================================
// Init the sound-system.
// ========================================================================
bool Sound::Init()
{
    LogFile::getSingleton().Headline("Init Soundystem");
    LogFile::getSingleton().Info("Starting fmod...");

    ///////////////////////////////////////////////////////////////////////// 
    // Check Version.
	/////////////////////////////////////////////////////////////////////////
    if (FSOUND_GetVersion() < FMOD_VERSION)
    {
		LogFile::getSingleton().Success(false);
        LogFile::getSingleton().Error("You are using the wrong DLL version! "
			"You should be using FMOD %.02f\n", FMOD_VERSION);
        return false;
    }

    ///////////////////////////////////////////////////////////////////////// 
    // Init Fmod.
	/////////////////////////////////////////////////////////////////////////
    if (!FSOUND_Init(44100, 32, 0))
    {
		LogFile::getSingleton().Success(false);
        LogFile::getSingleton().Error("FSound init: %s\n", FMOD_ErrorString(FSOUND_GetError()));
        return false;
    }
	LogFile::getSingleton().Success(true);

    /////////////////////////////////////////////////////////////////////////
    // Load all samples.
	/////////////////////////////////////////////////////////////////////////
    LogFile::getSingleton().Info("Loading samples...");
    for (unsigned int i = 0; i< SAMPLE_SUM; ++i)
	{
		if (!(Sample[i].handle = FSOUND_Sample_Load(i, Sample[i].filename, 0,0,0)))
		{
			LogFile::getSingleton().Success(false);
			LogFile::getSingleton().Error("Sample load: %s\n", FMOD_ErrorString(FSOUND_GetError()));
		}
	}
	LogFile::getSingleton().Success(true);

	mWeight = 1.0;
	mMusicVolume  = 50;
	mSampleVolume =255;
	return true;
}

// ========================================================================
// Set the volume.
// ========================================================================
void Sound::setVolume(int channel, int volume)
{
	FSOUND_SetVolume(channel, volume);
}

// ========================================================================
// Plays a song.
// ========================================================================
void Sound::playSong(const char *filename)
{
	stopSong();
	mpSong = FMUSIC_LoadSong(filename);
    if (!mpSong)
    {
        LogFile::getSingleton().Error("Song load: %s\n", FMOD_ErrorString(FSOUND_GetError()));
        return;
    }
	FMUSIC_SetMasterVolume(mpSong, mMusicVolume);
	FMUSIC_PlaySong(mpSong);
}

// ========================================================================
// Plays a song.
// ========================================================================
void Sound::stopSong()
{
	if (!mpSong) { return; }
	FMUSIC_StopSong(mpSong);
}

// ========================================================================
// Plays a stream.
// ========================================================================
void Sound::playStream(const char *filename)
{
	stopStream();
	mpStream = FSOUND_Stream_Open(filename, FSOUND_LOOP_NORMAL, 0, 0);
    if (!mpStream)
    {
		LogFile::getSingleton().Success(false);
        LogFile::getSingleton().Error("Music load: %s\n", FMOD_ErrorString(FSOUND_GetError()));
        return;
    }
	mChannel = FSOUND_Stream_Play(FSOUND_FREE, mpStream);
    if (mChannel < 0)
    {
        LogFile::getSingleton().Error("FSOUND_Stream_Play returned %d\n", mChannel);
        return;
	}
	setVolume(mChannel, mMusicVolume);
}

// ========================================================================
// Stops the stream.
// ========================================================================
void Sound::stopStream()
{
	if (!mpStream) { return; }
	FSOUND_Stream_Stop(mpStream);
	mpStream = 0;
}

// ========================================================================
// Plays a sample.
// ========================================================================
void Sound::setSamplePos3D(int channel, float &posX, float &posY, float &posZ)
{
	if (channel < 0) { return; }
	float pos[3];
	pos[0] =  posX * mWeight;
	pos[1] =  posY * mWeight;
	pos[2] = -posZ * mWeight;
	FSOUND_3D_SetAttributes(channel, &pos[0], 0);
}

// ========================================================================
// Plays a sample.
// ========================================================================
int Sound::playSample(int id, float posX, float posY, float posZ)
{
	if (id >= SAMPLE_SUM) { return -1; }
	mChannel = FSOUND_PlaySound(FSOUND_FREE, Sample[id].handle);
	setSamplePos3D(mChannel, posX, posY, posZ);
	setVolume(mChannel, mSampleVolume);
	return mChannel;
}

// ========================================================================
// Stops a sample.
// ========================================================================
void Sound::stopSample(int channel)
{
	if (channel >= 0)  { FSOUND_StopSound(channel); }
}

// ========================================================================
// Destructor.
// ========================================================================
Sound::~Sound()
{
	stopStream();
    FMUSIC_FreeSong(mpSong);
    for (unsigned int i = 0; i< SAMPLE_SUM; ++i)
	{ 
		if (Sample[i].handle) { FSOUND_Sample_Free(Sample[i].handle); }
	}
    FSOUND_Close();
}
