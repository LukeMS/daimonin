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
#include <cAudio.h>
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
    // Dummy sound (for error handling).
    { 0, "dummy.wav",           false, false},
    // The 8 internal gui sounds
    { 0, "console.wav",         false, false},
    { 0, "console.wav",         false, false},
    { 0, "console.wav",         false, false},
    { 0, "console.wav",         false, false},
    { 0, "console.wav",         false, false},
    { 0, "console.wav",         false, false},
    { 0, "console.wav",         false, false},
    { 0, "console.wav",         false, false},
    // Short sound.
    { 0, "console.wav",         false, false},
    { 0, "male_hit_01.wav",     false, false},
    { 0, "female_hit_01.wav",   false, false},
    { 0, "female_hit_02.wav",   false, false},
    { 0, "tentacle_hit_01.wav", false, false},
    { 0, "golem_hit_01.wav",    false, false},
    { 0, "attack_01.wav",       false, false},
    // Long sound (e.g. spoken text).
    { 0, "Player_Idle.ogg",     false, true },
    { 0, "Wizard_Visitor.ogg",  false, true },
    { 0, "male_bounty_01.ogg",  false, true },
    // Background music.
    { 0, "intro94.s3m",         true , true },
};

class myLogReceiver : public ILogReceiver
{
public:
    myLogReceiver() {}
    ~myLogReceiver() {}
    bool OnLogMessage(const char* sender, const char* message, LogLevel level, float time)
    {
        if      (level == ELL_ERROR || level == ELL_CRITICAL)
            Logger::log().error()   << Logger::ICON_CAUDIO << message;
        else if (level == ELL_WARNING)
            Logger::log().warning() << Logger::ICON_CAUDIO << message;
        else
            Logger::log().info()    << Logger::ICON_CAUDIO << message;
        return true;
    }
};

static IAudioManager *mSoundManager = 0;
static myLogReceiver *mLogger = 0;

//================================================================================================
// Init the sound-system.
//================================================================================================
bool Sound::Init(const char *filePath, int preferredDevice)
{
    PROFILE()
    if (Option::getSingleton().getIntValue(Option::CMDLINE_OFF_SOUND)) return false;
    Logger::log().headline() << "Init Sound-System";
    mFilePath = filePath;
    // Create the main system object.
    mSoundManager = cAudio::createAudioManager(false);
    if(!mSoundManager)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Failed to create audio playback manager.";
        return false;
    }
    // Add our LogListener to cAudio.
    mLogger = new myLogReceiver;
    cAudio::getLogger()->registerLogReceiver(mLogger, "Loggin");
    // Unregester the internal cAudio LogListener.
    cAudio::getLogger()->unRegisterLogReceiver("File");
    remove("cAudioEngineLog.html");
    // Show all playback devices
    std::string defaultDeviceName = mSoundManager->getDefaultDeviceName();
    int defaultDevice = -1;
    Logger::log().info() << Logger::ICON_CLIENT << "\nAvailable Playback Devices: \n";
    for (unsigned int i=0; i< mSoundManager->getAvailableDeviceCount(); ++i)
    {
        std::string deviceName = mSoundManager->getAvailableDeviceName(i);
        if (!deviceName.compare(defaultDeviceName))
        {
            Logger::log().info() << Logger::ICON_CLIENT << i << "): " << deviceName << " [DEFAULT]";
            defaultDevice = i;
        }
        else
            Logger::log().info() << Logger::ICON_CLIENT << i << "): " << deviceName;
    }
    // If no preferedDevice was defined, use the system default.
    if (preferredDevice < 0) preferredDevice = defaultDevice;
    // Initialize the manager.
    if (!mSoundManager->initialize(mSoundManager->getAvailableDeviceName(preferredDevice)))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Failed to initialize the sound manager with device " << mSoundManager->getAvailableDeviceName(preferredDevice);
        destroyAudioManager(mSoundManager);
        mSoundManager = 0;
        return false;
    }
    Logger::log().info() << Logger::ICON_CLIENT << "Playback Device '" << mSoundManager->getAvailableDeviceName(preferredDevice) << "' is active.";
    // Load all samples.
    createDummy();
    Logger::log().info() << Logger::ICON_CLIENT << "Loading all Sounds.";
    for (SampleID i = GUI_WRONG_INPUT; i< SAMPLE_SUM; i=SampleID(i+1))
        openStream(i);
    //playStream(PLAYER_IDLE); // Just for testing. delete me!
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
    delete mLogger;
}

//================================================================================================
// Create a dummy Sample.
//================================================================================================
void Sound::createDummy()
{
    PROFILE()
    const unsigned char dummy[] =
    {
        'R' ,'I' , 'F','F' , 0x2c,0x00,0x00,0x00, 'W' ,'A' ,'V' ,'E' ,
        'f' ,'m' ,'t' ,' ' , 0x10,0x00,0x00,0x00, 0x01,0x00,0x01,0x00,
        0x44,0xac,0x00,0x00, 0x88,0x58,0x01,0x00, 0x02,0x00,0x10,0x00,
        'd' ,'a' ,'t' ,'a' , 0x08,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };
    std::ofstream out((mFilePath + mSoundFiles[DUMMY].filename).c_str(), std::ios::binary);
    if (out)
        out.write((char*)dummy, sizeof(dummy));
    else
        Logger::log().error() << Logger::ICON_CLIENT << "Critical: Cound not create the dummy wavefile.";
}

//================================================================================================
//Create an audio source and load a sound from a file.
//================================================================================================
void Sound::openStream(SampleID id)
{
    PROFILE()
    if (!mSoundManager) return;
    mSoundFiles[id].sound = mSoundManager->create(0, (mFilePath + mSoundFiles[id].filename).c_str(), mSoundFiles[id].stream);
    if (!mSoundFiles[id].sound)
    {
        Logger::log().warning() << Logger::ICON_CLIENT << "Soundfile " << mSoundFiles[id].filename << " could not be found or the codec is unknown.";
        mSoundFiles[id].sound = mSoundFiles[DUMMY].sound; // use the dummy wavefile.
    }
}

//================================================================================================
// Play an already created stream.
//================================================================================================
void Sound::playStream(SampleID id)
{
    PROFILE()
    if (!mSoundManager) return;
    if (mSoundFiles[id].sound->isPlaying()) stopStream(id);
    if (!mSoundFiles[id].sound->play2d(mSoundFiles[id].loop))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Error on creating Soundstream " << mSoundFiles[id].filename;
    }
}

//================================================================================================
// Play an already created stream.
//================================================================================================
void Sound::stopStream(SampleID id)
{
    PROFILE()
    if (!mSoundManager) return;
    mSoundFiles[id].sound->stop();
}

//================================================================================================
// Create and play a stream for the background music.
//================================================================================================
void Sound::playMusic(std::string filename, bool loop)
{
    PROFILE()
    if (!mSoundManager) return;
    static IAudioSource *music = 0;
    if (music) mSoundManager->release(music);
    music = mSoundManager->create(0, (mFilePath + filename).c_str(), true);
    if (!music)
        Logger::log().warning() << Logger::ICON_CLIENT << "Music " << music << " could not be found or the codec is unknown.";
    else
        music->play2d(loop);
}

//================================================================================================
// .
//================================================================================================
void Sound::playGuiSounds(unsigned char activeSounds)
{
    PROFILE()
    if (!mSoundManager || !activeSounds) return;
    if (activeSounds &  1) playStream(SampleID(GUI_WRONG_INPUT+0));
    if (activeSounds &  2) playStream(SampleID(GUI_WRONG_INPUT+1));
    if (activeSounds &  4) playStream(SampleID(GUI_WRONG_INPUT+2));
    if (activeSounds &  8) playStream(SampleID(GUI_WRONG_INPUT+3));
    if (activeSounds & 16) playStream(SampleID(GUI_WRONG_INPUT+4));
    if (activeSounds & 32) playStream(SampleID(GUI_WRONG_INPUT+5));
    if (activeSounds & 64) playStream(SampleID(GUI_WRONG_INPUT+6));
    if (activeSounds &128) playStream(SampleID(GUI_WRONG_INPUT+7));
}

//================================================================================================
// Set the volume.
//================================================================================================
void Sound::setVolume(SampleID id, float volume)
{
    PROFILE()
    if (!mSoundManager) return;
    mSoundFiles[id].sound->setVolume(volume);
}

//================================================================================================
// Sets the 3D-pos of the stream.
//================================================================================================
void Sound::set3DPos(SampleID id, float &x, float &y, float &z)
{
    PROFILE()
    if (!mSoundManager) return;
    mSoundFiles[id].sound->setPosition(cVector3(x,y,z));
}
