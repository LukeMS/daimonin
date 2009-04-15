/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include <vector>
#include <fstream>
#include "fmod.h"
#include "fmod_errors.h"
#include "define.h"
#include "sound.h"
#include "option.h"
#include "logger.h"

// We can't use c++ api on non m$ compilers - shame on fmod!

using namespace std;

FMOD_SYSTEM *System = 0;
FMOD_RESULT result;

typedef struct
{
    const char   *filename;
    FMOD_SOUND   *sound;
    FMOD_CHANNEL *channel;
    bool isMusic;
    bool is2D;
    //FMOD_VECTOR  pos;
    //FMOD_VECTOR vel;
}
SoundFiles;

SoundFiles mSoundFiles[Sound::SAMPLE_SUM] =
{
    // Background musics.
    { "invtro94.s3m",        0, 0, true , true },
    // Long sound e.g. spoken text (ogg).
    { "Player_Idle.ogg",     0, 0, false, true },
    { "Wizard_Visitor.ogg",  0, 0, false, true },
    { "male_bounty_01.ogg",  0, 0, false, true },
    // Short sounds (wav)
    { "console.wav",         0, 0, false, true },
    { "male_hit_01.wav",     0, 0, false, true },
    { "female_hit_01.wav",   0, 0, false, true },
    { "female_hit_02.wav",   0, 0, false, true },
    { "tentacle_hit_01.wav", 0, 0, false, true },
    { "golem_hit_01.wav",    0, 0, false, true },
    { "attack_01.wav",       0, 0, false, true },
    // Dummy sound (for error handling).
    { "dummy.wav",           0, 0, false, true },
};

const float DISTANCEFACTOR = 1.0f; // Units per meter. (feet = 3.28.  cm = 100).

//================================================================================================
// Init the sound-system.
//================================================================================================
bool Sound::Init()
{
    if (Option::getSingleton().getIntValue(Option::CMDLINE_OFF_SOUND)) return false;
    Logger::log().headline() << "Init Sound-System";
    // ////////////////////////////////////////////////////////////////////
    // Create the main system object.
    // ////////////////////////////////////////////////////////////////////
    result = FMOD_System_Create(&System);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "FMOD error! " << result << " " << FMOD_ErrorString(result);
        return false;
    }
    // ////////////////////////////////////////////////////////////////////
    // Init Fmod.
    // ////////////////////////////////////////////////////////////////////
    result = FMOD_System_Init(System, 32, FMOD_INIT_NORMAL, 0);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "FMOD error! " << result << " " << FMOD_ErrorString(result);
        return false;
    }
    result = FMOD_System_Set3DSettings(System, 1.0, DISTANCEFACTOR, 1.0f);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "FMOD error! " << result << " " << FMOD_ErrorString(result);
        return false;
    }
    mMusicVolume = 0.5;
    mSoundVolume = 1.0;
    mInit = true;
    // ////////////////////////////////////////////////////////////////////
    // Load all samples.
    // ////////////////////////////////////////////////////////////////////
    createDummy();
    Logger::log().info() << "Loading all Sounds.";
    for (unsigned int i = 0; i< SAMPLE_SUM; ++i)
    {
        createStream(i);
    }
    return true;
}

//================================================================================================
// Free all stuff.
//================================================================================================
void Sound::freeRecources()
{
    for (unsigned int i = 0; i< SAMPLE_SUM; ++i)
    {
        FMOD_Sound_Release(mSoundFiles[i].sound);
    }
    result = FMOD_System_Close(System);
    result = FMOD_System_Release(System);
}

//================================================================================================
// Create a dummy Sample.
//================================================================================================
void Sound::createDummy()
{
    const unsigned char dummy[] =
    {
        0x52,0x49,0x46,0x46,0xC0,0x00,0x00,0x00,0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
        0x12,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x11,0x2B,0x00,0x00,0x11,0x2B,0x00,0x00,
        0x01,0x00,0x08,0x00,0x00,0x00,0x66,0x61,0x63,0x74,0x04,0x00,0x00,0x00,0x8E,0x00,
        0x00,0x00,0x64,0x61,0x74,0x61,0x8E,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80
    };
    std::string filename = PATH_SND;
    filename += mSoundFiles[DUMMY].filename;
    ofstream out(filename.c_str(), ios::binary);
    if (!out)
    {
        Logger::log().error() << "Critical: Cound not create the dummy wavefile.";
        return;
    }
    out.write((char*)dummy, sizeof(dummy));
}

//================================================================================================
// Create a stream.
//================================================================================================
void Sound::createStream(int id)
{
    std::string filename = PATH_SND;
    filename += mSoundFiles[id].filename;
    int options = FMOD_HARDWARE;
    options |= mSoundFiles[id].isMusic?FMOD_LOOP_NORMAL:FMOD_LOOP_OFF;
    options |= mSoundFiles[id].is2D?FMOD_2D:FMOD_3D;
    result = FMOD_System_CreateStream(System, filename.c_str(), options, 0, &mSoundFiles[id].sound);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "Error on creating Soundstream " << mSoundFiles[id].filename << " : " << FMOD_ErrorString(result);
    }
}

//================================================================================================
// Plays a stream.
//================================================================================================
void Sound::playStream(int id)
{
    if (!mInit) return;
    stopStream(id);
    result = FMOD_System_PlaySound(System, FMOD_CHANNEL_FREE, mSoundFiles[id].sound, 0, &mSoundFiles[id].channel);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "Error on play Soundstream " << mSoundFiles[id].filename << " : " << FMOD_ErrorString(result);
        return;
    }
    setVolume(id);
    //set3DPos(id, );
}

void Sound::playStream(const char *file, bool loop)
{
    if (!mInit || !file) return;
    std::string filename = PATH_SND;
    filename+= file;
    int options = FMOD_HARDWARE | FMOD_2D;
    options |= (loop)?FMOD_LOOP_NORMAL:FMOD_LOOP_OFF;
    result = FMOD_System_CreateStream(System, filename.c_str(), options, 0, &mSoundFiles[DUMMY].sound);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "Error on creating Soundstream " << filename << " : "  << FMOD_ErrorString(result);
        return;
    }
    stopStream(DUMMY);
    result = FMOD_System_PlaySound(System, FMOD_CHANNEL_FREE, mSoundFiles[DUMMY].sound, 0, &mSoundFiles[DUMMY].channel);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "Error on play Soundstream " << filename << " : " << FMOD_ErrorString(result);
        return;
    }
    setVolume(DUMMY);
    //set3DPos(id, );
}

//================================================================================================
// Stops the stream.
//================================================================================================
void Sound::stopStream(int id)
{
    if (mInit) FMOD_Channel_SetPaused(mSoundFiles[id].channel, true);
}

//================================================================================================
// Set the volume.
//================================================================================================
void Sound::setVolume(unsigned int id, float volume)
{
    if (!mInit) return;
    if (volume <0)
    {
        if (mSoundFiles[id].isMusic)
            FMOD_Channel_SetVolume(mSoundFiles[id].channel, mMusicVolume);
        else
            FMOD_Channel_SetVolume(mSoundFiles[id].channel, mSoundVolume);
    }
    else
    {
        FMOD_Channel_SetVolume(mSoundFiles[id].channel, volume);
    }
}

//================================================================================================
// Sets the 3D-pos of the stream.
//================================================================================================
void Sound::set3DPos(unsigned int id, float &posX, float &posY, float &posZ)
{
    if (!mInit) return;
    FMOD_VECTOR pos =
    {
        posX * DISTANCEFACTOR, posY, posZ
    };
    FMOD_VECTOR vel =
    {
        0.0f, 0.0f, 0.0f
    };
    result = FMOD_Channel_Set3DAttributes(mSoundFiles[id].channel, &pos, &vel);
    if (result != FMOD_OK)
    {
        Logger::log().error() << "Coundn't set 3D pos of sound. " << result << " " << FMOD_ErrorString(result);
    }
}
