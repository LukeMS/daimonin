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

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef SOUND_H
#define SOUND_H

#include "fmod.h"

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
