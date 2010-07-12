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

/**
 ** This singleton class handles all sound related stuff.
 *****************************************************************************/
class Sound
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////GUI_
    typedef enum
    {
        // Dummy sound (for error handling). MUST be the first sound.
        DUMMY,
        // Internal gui sounds. Must be copied from gui_manager.
        GUI_WRONG_INPUT,
        GUI_MOUSE_CLICK,
        GUI_KEY_PRESSED,
        GUI_RESERVED_1,
        GUI_RESERVED_2,
        GUI_RESERVED_3,
        GUI_RESERVED_4,
        GUI_RESERVED_5,
        // Short sounds (wav)
        BUTTON_CLICK,
        MALE_HIT_01,
        FEMALE_HIT_01,
        FEMALE_HIT_02,
        TENTACLE_HIT,
        GOLEM_HIT,
        ATTACK_01,
        // Long sound e.g. spoken text (ogg).
        PLAYER_IDLE,
        GREETS_VISITOR,
        MALE_BOUNTY_01,
        // Background music.
        BG_MUSIC,
        SAMPLE_SUM
    }SampleID;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Sound &getSingleton()
    {
        static Sound Singleton; return Singleton;
    }
    bool Init(const char *filePath);
    void freeRecources();
    void createDummy();
    void openStream(SampleID id);
    void playStream(SampleID id);
    void playMusic(std::string filename, bool loop = true);
    void playGuiSounds(unsigned char activeSounds);
    void stopStream(SampleID id);
    void set3DPos(SampleID id, float &posX, float &posY, float &posZ);
    void setVolume(SampleID id, float volume = 1.0f);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    std::string mFilePath;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Sound() {}
    ~Sound() {}
    Sound(const Sound&);            /**< disable copy-constructor. **/
    Sound &operator=(const Sound&); /**< disable assignment operator. **/
};

#endif
