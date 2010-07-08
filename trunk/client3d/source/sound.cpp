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
#include "include/cAudio.h"
#include "define.h"
#include "sound.h"
#include "option.h"
#include "logger.h"
#include "profiler.h"

using namespace cAudio;

typedef struct
{
    IAudioSource *sound;
    const char   *filename;
    bool loop;
    bool stream;
}
SoundFiles;

SoundFiles mSoundFiles[Sound::SAMPLE_SUM] =
{
    // Background music.
    { 0, "intro94.s3m",         true , true },
    // Long sound (e.g. spoken text).
    { 0, "Player_Idle.ogg",     false, true },
    { 0, "Wizard_Visitor.ogg",  false, true },
    { 0, "male_bounty_01.ogg",  false, true },
    // Short sound.
    { 0, "console.wav",         false, false},
    { 0, "male_hit_01.wav",     false, false},
    { 0, "female_hit_01.wav",   false, false},
    { 0, "female_hit_02.wav",   false, false},
    { 0, "tentacle_hit_01.wav", false, false},
    { 0, "golem_hit_01.wav",    false, false},
    { 0, "attack_01.wav",       false, false},
    // Dummy sound (for error handling).
    { 0, "dummy.wav",           false, false},
};

static IAudioManager *mSoundManager = 0;

//================================================================================================
// Init the sound-system.
//================================================================================================
bool Sound::Init()
{
    PROFILE()
    if (Option::getSingleton().getIntValue(Option::CMDLINE_OFF_SOUND)) return false;
    Logger::log().headline() << "Init Sound-System";
    // ////////////////////////////////////////////////////////////////////
    // Create the main system object.
    // ////////////////////////////////////////////////////////////////////
    mSoundManager = cAudio::createAudioManager(false);
    if(!mSoundManager)
    {
        Logger::log().error() << "Failed to create audio playback manager.";
        return false;
    }
    //Allow the user to choose a playback device
    Logger::log().info() << "\nAvailable Playback Devices: \n";
    std::string defaultDeviceName = mSoundManager->getDefaultDeviceName();
    for(unsigned int i=0; i< mSoundManager->getAvailableDeviceCount(); ++i)
    {
        std::string deviceName = mSoundManager->getAvailableDeviceName(i);
        if (!deviceName.compare(defaultDeviceName))
            Logger::log().info() << i << "): " << deviceName << " [DEFAULT]";
        else
            Logger::log().info() << i << "): " << deviceName;
    }
    //Initialize the manager with first device.
    if (!mSoundManager->initialize(mSoundManager->getAvailableDeviceName(0)))
    {
        Logger::log().error() << "Failed to initialize the sound manager.";
        mSoundManager = 0;
        return false;
    }
    mMusicVolume = 0.5;
    mSoundVolume = 1.0;
    // ////////////////////////////////////////////////////////////////////
    // Load all samples.
    // ////////////////////////////////////////////////////////////////////
    createDummy();
    Logger::log().info() << "Loading all Sounds.";
    for (SampleID i = BG_MUSIC; i< SAMPLE_SUM; i=SampleID(i+1))
        openStream(i);
    //playStream(ATTACK_01);
    return true;
}

//================================================================================================
// Free all stuff.
//================================================================================================
void Sound::freeRecources()
{
    PROFILE()
    if (!mSoundManager) return;
    mSoundManager->releaseAllSources();
    mSoundManager->shutDown();
    destroyAudioManager(mSoundManager);
}

//================================================================================================
// Create a dummy Sample.
//================================================================================================
void Sound::createDummy()
{
    PROFILE()
    const char dummy[] =
    {
        'R' ,'I' , 'F','F' , 0x2c,0x00,0x00,0x00, 'W' ,'A' ,'V' ,'E' ,
        'f' ,'m' ,'t' ,' ' , 0x10,0x00,0x00,0x00, 0x01,0x00,0x01,0x00,
        0x44,0xac,0x00,0x00, 0x88,0x58,0x01,0x00, 0x02,0x00,0x10,0x00,
        'd' ,'a' ,'t' ,'a' , 0x08,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };
    std::ofstream out((PATH_SND + mSoundFiles[DUMMY].filename).c_str(), std::ios::binary);
    if (out)
        out.write(dummy, sizeof(dummy));
    else
        Logger::log().error() << "Critical: Cound not create the dummy wavefile.";
}

//================================================================================================
//Create an audio source and load a sound from a file.
//================================================================================================
void Sound::openStream(SampleID id)
{
    PROFILE()
    mSoundFiles[id].sound = mSoundManager->create(0, (PATH_SND + mSoundFiles[id].filename).c_str(), mSoundFiles[id].stream);
}

//================================================================================================
// Create a stream.
//================================================================================================
void Sound::playStream(SampleID id)
{
    if (!mSoundFiles[id].sound->play2d(mSoundFiles[id].loop))
    {
        Logger::log().error() << "Error on creating Soundstream " << mSoundFiles[id].filename;
    }
}


//================================================================================================
// Create a stream.
//================================================================================================
void Sound::playStream(const char *filename, bool loop)
{
}

//================================================================================================
// Set the volume.
//================================================================================================
void Sound::setVolume(unsigned int id, float volume)
{

}

//================================================================================================
// Sets the 3D-pos of the stream.
//================================================================================================
void Sound::set3DPos(unsigned int id, float &posX, float &posY, float &posZ)
{

}
