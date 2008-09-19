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

#ifndef SOUND_H
#define SOUND_H

#include "fmod.h"

/**
 ** This singleton class handles all sound related stuff.
 *****************************************************************************/
class Sound
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum SampleName
    {
        // Background musics.
        BG_MUSIC,
        // Long sound e.g. spoken text (ogg).
        PLAYER_IDLE,
        GREETS_VISITOR,
        MALE_BOUNTY_01,
        // Short sounds (wav)
        BUTTON_CLICK,
        MALE_HIT_01,
        FEMALE_HIT_01,
        FEMALE_HIT_02,
        TENTACLE_HIT,
        GOLEM_HIT,
        ATTACK_01,
        // Dummy sound (for error handling).
        DUMMY,
        SAMPLE_SUM
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Sound &getSingleton()
    {
        static Sound Singleton; return Singleton;
    }
    bool Init();
    void freeRecources();
    void createDummy();
    void createStream(int id);
    void playStream(int id);
    void playStream(char *filename, bool loop =false);
    void stopStream(int id);
    void set3DPos( unsigned int channel,  float &posX, float &posY, float &posZ );
    void setVolume( unsigned int channel, float volume =-1);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    float mMusicVolume, mSoundVolume;
    bool  mSound3D;
    bool  mInit;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Sound()
    {}
    ~Sound()
    {}
    Sound( const Sound& ); // disable copy-constructor.
};

#endif
