/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <vector>
#include <fstream>
#include "fmod.h"
#include "fmod_errors.h"  //optional.
#include "define.h"
#include "sound.h"
#include "logger.h"

using namespace std;

static FMUSIC_MODULE *mpSong   =0;
static FSOUND_STREAM *mpStream =0;
static vector<FSOUND_SAMPLE*> vecHandle;

// ========================================================================
// Init the sound-system.
// ========================================================================
bool Sound::Init()
{
  Logger::log().headline("Init Sound-System");

  /////////////////////////////////////////////////////////////////////////
  // Check Version.
  /////////////////////////////////////////////////////////////////////////
  if (FSOUND_GetVersion() < FMOD_VERSION)
  {
    Logger::log().error() << "You are using the wrong DLL version! You should be using FMOD" << FMOD_VERSION;
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  // Init Fmod.
  /////////////////////////////////////////////////////////////////////////
  Logger::log().info() << "Starting fmod...";
  if (!FSOUND_Init(44100, 32, 0))
  {
    Logger::log().success(false);
    Logger::log().error() << "FSound init: " << FMOD_ErrorString(FSOUND_GetError());
    return false;
  }
  Logger::log().success(true);

  /////////////////////////////////////////////////////////////////////////
  // Load all samples.
  /////////////////////////////////////////////////////////////////////////
  createSampleDummy();
  // if you change something here - you must change it in enum SampleName, too.
  Logger::log().info() << "Loading samples...";
  loadSample(FILE_SAMPLE_MOUSE_CLICK);
  loadSample(FILE_SAMPLE_PLAYER_IDLE);
  Logger::log().success(true);
  mWeight = 1.0;
  mMusicVolume  = 50;
  mSampleVolume =255;
  return true;
}

// ========================================================================
// Sets the 3D-pos of the sample.
// ========================================================================
void Sound::createSampleDummy()
{
  const unsigned char dummy[] =
    {
      0x52,0x49,0x46,0x46,0xC0,0x00,0x00,0x00,0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
      0x12,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x11,0x2B,0x00,0x00,0x11,0x2B,0x00,0x00,
      0x01,0x00,0x08,0x00,0x00,0x00,0x66,0x61,0x63,0x74,0x04,0x00,0x00,0x00,0x8E,0x00,
      0x00,0x00,0x64,0x61,0x74,0x61,0x8E,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80
    };
  ofstream out(FILE_SAMPLE_DUMMY, ios::binary);
  if (!out)
  {
    Logger::log().error() << "Critical: Cound not create the dummy wavefile.";
    return;
  }
  out.write((char*)dummy, sizeof(dummy));
}

// ========================================================================
// Sets the 3D-pos of the sample.
// ========================================================================
int Sound::loadSample(const char *filename)
{
  FSOUND_SAMPLE *handle = FSOUND_Sample_Load((int) vecHandle.size() , filename, 0,0,0);
  if (!handle)
  {
    Logger::log().error()  << "* Error on Sample '" << filename
    << "': " << FMOD_ErrorString(FSOUND_GetError())
    << Logger::endl << "-> using dummy.wav instead.";
    mSuccess = false;
    handle = FSOUND_Sample_Load((int) vecHandle.size() , FILE_SAMPLE_DUMMY, 0,0,0);
    if (!handle)
    {
      Logger::log().error() << "Critical: Cound not load the dummy wavefile.";
      return -1;
    }
  }
  vecHandle.push_back(handle);
  return (int) vecHandle.size();
}

// ========================================================================
// Sets the 3D-pos of the sample.
// ========================================================================
void Sound::setSamplePos3D(unsigned int channel, float &posX, float &posY, float &posZ)
{
  //if (channel > ) { return; }
  float pos[3];
  pos[0] =  posX * mWeight;
  pos[1] =  posY * mWeight;
  pos[2] = -posZ * mWeight;
  FSOUND_3D_SetAttributes(channel, &pos[0], 0);
}

// ========================================================================
// Plays a sample.
// ========================================================================
int Sound::playSample(unsigned int id, float posX, float posY, float posZ)
{
  if (id > vecHandle.size())
  {
    return -1;
  }
  mChannel = FSOUND_PlaySound(FSOUND_FREE, vecHandle[id]);
  setSamplePos3D(mChannel, posX, posY, posZ);
  setVolume(mChannel, mSampleVolume);
  return mChannel;
}

// ========================================================================
// Stops a sample.
// ========================================================================
void Sound::stopSample(unsigned int channel)
{
  if (channel <= vecHandle.size())
  {
    FSOUND_StopSound(channel);
  }
}


// ========================================================================
// Set the volume.
// ========================================================================
void Sound::setVolume(unsigned int channel, int volume)
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
    Logger::log().error() << "Song load: " << FMOD_ErrorString(FSOUND_GetError());
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
  if (!mpSong)
  {
    return;
  }
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
    Logger::log().error() << "Music load: " << FMOD_ErrorString(FSOUND_GetError());
    return;
  }
  mChannel = FSOUND_Stream_Play(FSOUND_FREE, mpStream);
  if (mChannel < 0)
  {
    Logger::log().error() << "FSOUND_Stream_Play returned " << mChannel;
    return;
  }
  setVolume(mChannel, mMusicVolume);
}

// ========================================================================
// Stops the stream.
// ========================================================================
void Sound::stopStream()
{
  if (!mpStream)
  {
    return;
  }
  FSOUND_Stream_Stop(mpStream);
  mpStream = 0;
}

// ========================================================================
// Free all stuff.
// ========================================================================
void Sound::freeRecources()
{
  stopStream();
  FMUSIC_FreeSong(mpSong);
  for (unsigned int i = 0; i< vecHandle.size(); ++i)
  {
    FSOUND_Sample_Free(vecHandle[i]);
  }
  FSOUND_Close();
}
