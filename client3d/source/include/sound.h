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

#ifndef SOUND_H
#define SOUND_H

class Sound
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    enum SampleName
    {
        BG_MUSIC,
        DUMMY,
        BUTTON_CLICK,
        PLAYER_IDLE,
        SAMPLE_SUM
    };

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    static Sound &getSingleton()
    {
        static Sound Singleton; return Singleton;
    }
    bool Init();
    void freeRecources();
    void createDummy();
    void createStream(int id);
    void playStream(int id);
    void stopStream(int id);
    void set3DPos( unsigned int channel,  float &posX, float &posY, float &posZ );
    void setVolume( unsigned int channel, float volume =-1);

private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    float mMusicVolume, mSoundVolume;
    bool  mSound3D;
    bool  mInit;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    Sound()
    {}
    ~Sound()
    {}
    Sound( const Sound& ); // disable copy-constructor.
};

#endif
