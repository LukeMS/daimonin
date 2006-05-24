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

#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

typedef enum _game_status
{
    GAME_STATUS_CHECK_HARDWARE,
    GAME_STATUS_INIT_VIEWPORT,     // Init all basic ogre stuff.
    GAME_STATUS_INIT_SOUND,        // Init the sound-system.
    GAME_STATUS_INIT_LIGHT,        // Init the light-system.
    GAME_STATUS_INIT_SPELL,        // Init the spells.
    GAME_STATUS_INIT_PARTICLE,     // Init the particles.
    GAME_STATUS_INIT_OBJECT,       // Init the objects.
    GAME_STATUS_INIT_GUI_IMAGESET, // Parse the IMageSet.
    GAME_STATUS_INIT_GUI_WINDOWS,  // Parse the Windows.
    GAME_STATUS_INIT_TILE,         // Init the tile-engine.
    GAME_STATUS_INIT_NET,          // init the network.
    //  GAME_STATUS_INIT_DONE,         // DUMMY - delete me!
    GAME_STATUS_META,              // connect to meta server.
    GAME_STATUS_START,             // start all up (without full reset or meta calling).
    GAME_STATUS_WAITLOOP,          // we are NOT connected to anything.
    GAME_STATUS_STARTCONNECT,      // we have a server+port, init and start.
    GAME_STATUS_CONNECT,           // if this is set, we start connecting.
    GAME_STATUS_VERSION,           // now the steps: Connect, we send version.
    GAME_STATUS_WAITVERSION,       // wait for response... add up in version cmd.
    GAME_STATUS_SETUP,             // we ready to send setup commands.
    GAME_STATUS_WAITSETUP,         // we wait for server response.
    GAME_STATUS_REQUEST_FILES,     // after we get response from setup, we request files if needed.
    GAME_STATUS_ADDME,             // all setup is done, now try to enter game!
    GAME_STATUS_LOGIN,             // now we wait for LOGIN request of the server.
    GAME_STATUS_NAME,              // all this here is tricky
    GAME_STATUS_PSWD,              // server will trigger this when asking for
    GAME_STATUS_VERIFYPSWD,        // client will then show input panel or so
    GAME_STATUS_NEW_CHAR,          // show new char creation screen and send /nc command when finished
    GAME_STATUS_WAITFORPLAY,       // we simply wait for game start means, this is not a serial stepping here
    GAME_STATUS_QUIT,              // we are in quit menu
    GAME_STATUS_PLAY,              // we play now!!
    GAME_STATUS_SUM
};


class Option
{
public:
    enum enumOption
    {
        /// Dialog window options.
        VOL_SOUND, VOL_MUSIC, VOL_VOICE,
        META_SERVER_NAME, META_SERVER_PORT,
        ///
        SEPARATOR,
        /// Non-Dialog options.
        SEL_META_SEVER,
        HIGH_TEXTURE_DETAILS,
        HIGH_TILES_DETAILS,
        LOG_GUI_ELEMENTS,
        CREATE_RAW_FONTS,
        CREATE_TILE_TEXTURES,
        UPDATE_NETWORK,
        CMDLINE_SERVER_NAME,
        CMDLINE_SERVER_PORT,
        CMDLINE_FALLBACK,
        CMDLINE_OFF_SOUND,
        SUM_OPTIONS
    };
    enum selType
    {
        SEL_BUTTON,
        SEL_CHECKBOX,
        SEL_INT_RANGE,
        /// values < 1000 are integer values.
        SEL_TXT_RANGE = SEPARATOR,
        SEL_TEXT
    };
    typedef struct optionStruct
    {
        selType type;
        char *name;     /**< Name of the Option**/
        char *info1;    /**< Info text row 1 **/
        char *info2;    /**< Info text row 2 **/
        char *val_text; /**< Text-replacement (values are separated by '#') **/
        std::string txtValue;
        int  intValue;
        int  minRange, maxRange, deltaRange;
        bool pageFeed;
    };
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    static optionStruct optStruct[SEPARATOR];
    static std::string  optValue[SUM_OPTIONS - SEPARATOR-1];

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    bool openDescFile(const char *filename);
    void closeDescFile();
    bool getDescStr(const char *descrEntry, string &strBuffer, unsigned int nr=0);

    static Option &getSingleton()
    {
        static Option Singleton; return Singleton;
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
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    unsigned int mGameStatus;
    ifstream *mDescFile;
    string mDescBuffer;
    string mFilename;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    Option();
    ~Option();
    Option(const Option&); // disable copy-constructor.
};

#endif
