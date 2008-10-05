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

#ifndef OPTION_H
#define OPTION_H

#include <Ogre.h>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 ** This singleton class handles all options the user and the engine can set.
 *****************************************************************************/
class Option
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum _game_status
    {
        GAME_STATUS_CHECK_HARDWARE,
        GAME_STATUS_INIT_VIEWPORT,     /**< Init all basic ogre stuff. **/
        GAME_STATUS_INIT_SOUND,        /**< Init the sound-system. **/
        GAME_STATUS_INIT_LIGHT,        /**< Init the light-system. **/
        GAME_STATUS_INIT_SPELL,        /**< Init the spells. **/
        GAME_STATUS_INIT_PARTICLE,     /**< Init the particles. **/
        GAME_STATUS_INIT_OBJECT,       /**< Init the objects. **/
        GAME_STATUS_INIT_GUI_IMAGESET, /**< Parse the IMageSet. **/
        GAME_STATUS_INIT_GUI_WINDOWS,  /**< Parse the Windows. **/
        GAME_STATUS_INIT_TILE,         /**< Init the tile-engine. **/
        GAME_STATUS_INIT_GUI,          /**< Init the GUI. **/
        GAME_STATUS_INIT_NET,          /**< init the network. **/

        GAME_STATUS_META,              /**< connect to meta server. **/
        GAME_STATUS_START,             /**< start all up (without full reset or meta calling). **/
        GAME_STATUS_WAITLOOP,          /**< we are NOT connected to anything. **/
        GAME_STATUS_STARTCONNECT,      /**< we have a server+port, init and start. **/
        GAME_STATUS_CONNECT,           /**< if this is set, we start connecting. **/
        GAME_STATUS_SETUP,             /**< we're ready to send the setup commands. **/
        GAME_STATUS_WAITSETUP,         /**< we wait for server to response to the steup. **/
        GAME_STATUS_REQUEST_FILES,     /**< after we get response from setup, we request files if needed. **/

        GAME_STATUS_LOGIN,
        GAME_STATUS_LOGIN_NAME,
        GAME_STATUS_LOGIN_NAME_WAIT,
        GAME_STATUS_LOGIN_PASWD,
        GAME_STATUS_LOGIN_PASWD_WAIT,
        GAME_STATUS_LOGIN_NEW_ACCOUNT,
        GAME_STATUS_LOGIN_DONE,
        GAME_STATUS_LOGIN_CHARACTER,
        GAME_STATUS_LOGIN_CHARACTER_WAIT,

        GAME_STATUS_WAITFORPLAY,       /**< we simply wait for game start means, this is not a serial stepping here **/
        GAME_STATUS_QUIT,              /**< we are in quit menu **/
        GAME_STATUS_PLAY,              /**< we play now!! **/
        GAME_STATUS_SUM
    };

    enum enumOption
    {
        // Dialog window options.
        VOL_SOUND, VOL_MUSIC, VOL_VOICE,
        META_SERVER_NAME, META_SERVER_PORT,
        //
        SEPARATOR,
        // Non-Dialog options.
        SEL_META_SEVER,
        HIGH_TEXTURE_DETAILS,
        HIGH_TILES_DETAILS,
        UPDATE_NETWORK,

        CMDLINE_LOG_GUI_ELEMENTS,
        CMDLINE_CREATE_RAW_FONTS,
        CMDLINE_CREATE_TILE_TEXTURES,
        CMDLINE_SERVER_NAME,
        CMDLINE_SERVER_PORT,
        CMDLINE_TILEENGINE_LOD,
        CMDLINE_OFF_SOUND,
        CMDLINE_SHOW_BOUNDING_BOX,
        CMDLINE_CREATE_IMPOSTERS,
        CMDLINE_CREATE_ITEMS,
        SUM_OPTIONS
    };
    enum enumLoginType
    {
        LOGIN_NEW_PLAYER,
        LOGIN_EXISTING_PLAYER
    };
    enum selType
    {
        SEL_BUTTON,
        SEL_CHECKBOX,
        SEL_INT_RANGE,
        // values < 1000 are integer values.
        SEL_TXT_RANGE = SEPARATOR,
        SEL_TEXT
    };
    typedef struct
    {
        selType type;
        const char *name;     /**< Name of the Option **/
        const char *info1;    /**< Info text row 1    **/
        const char *info2;    /**< Info text row 2    **/
        const char *val_text; /**< Text-replacement (values are separated by '#') **/
        Ogre::String txtValue;
        int  intValue;
        int  minRange, maxRange, deltaRange;
        bool pageFeed;
    }
    optionStruct;

    static optionStruct optStruct[SEPARATOR];
    static Ogre::String  optValue[SUM_OPTIONS - SEPARATOR-1];

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    bool openDescFile(const char *filename);
    void closeDescFile();
    bool getDescStr(const char *descrEntry, Ogre::String &strBuffer, unsigned int nr=0);

    static Option &getSingleton()
    {
        static Option Singleton; return Singleton;
    }

    void setLoginType(enum enumLoginType type)
    {
        mLoginType = type;
    }
    int getLoginType()
    {
        return mLoginType;
    }

    int getIntValue(enumOption option)
    {
        if (option < SEPARATOR)
        {
            return optStruct[option].intValue;
        }
        else
        {
            return atoi(optValue[option - SEPARATOR-1].c_str());
        }
    }

    void setIntValue(enumOption option, int value)
    {
        if (option < SEPARATOR)
        {
            optStruct[option].intValue = value;
        }
        else
        {
            std::ostringstream os;
            os << value;
            optValue[option - SEPARATOR-1] = os.str();
            os.rdbuf()->str("");
        }
    }

    void setStrValue(enumOption option, const char *value)
    {
        if (option < SEPARATOR)
        {
            optStruct[option].txtValue = value;
        }
        else
        {
            optValue[option - SEPARATOR-1] = value;
        }
    }

    const char *getStrValue(enumOption option)
    {
        if (option < SEPARATOR)
        {
            return optStruct[option].txtValue.c_str();
        }
        else
        {
            return optValue[option - SEPARATOR-1].c_str();
        }
    }

    bool setGameStatus(int status)
    {
        if (status > GAME_STATUS_SUM) return false;
        mGameStatus = status;
        return true;
    }
    int getGameStatus()
    {
        return mGameStatus;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    unsigned int mGameStatus;
    enum enumLoginType mLoginType;
    std::ifstream *mDescFile;
    Ogre::String mDescBuffer;
    Ogre::String mFilename;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Option();
    ~Option();
    Option(const Option&); // disable copy-constructor.
};

#endif
