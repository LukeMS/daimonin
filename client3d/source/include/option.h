/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <iostream>
#include <fstream>

using namespace std;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////
typedef struct _options
{
    // Sound
    int  sound_volume;
    int  music_volume;
    // Server
    char  metaserver[256];
    int   metaserver_port;
}_options;
extern _options options;

typedef struct _option
{
    char *name;
    char *info1;  // info text row 1
    char *info2;  // info text row 2
    char *val_text; // text-replacement for number values
    int  sel_type;
    int  minRange, maxRange, deltaRange;
    int  default_val;
    void *value;
    int  value_type;
}_option;
//extern _option opt[];


extern enum
{
    VAL_BOOL,
    VAL_TEXT,
    VAL_CHAR,
    VAL_INT,
    VAL_U32
} value_type;

typedef enum _game_status
{
    GAME_STATUS_INIT,          // start to autoinit.
    GAME_STATUS_META,          // connect to meta server.
    GAME_STATUS_START,         // start all up (without full reset or meta calling).
    GAME_STATUS_WAITLOOP,      // we are NOT connected to anything.
    GAME_STATUS_STARTCONNECT,  // we have a server+port, init and start.
    GAME_STATUS_CONNECT,       // if this is set, we start connecting.
    GAME_STATUS_VERSION,       // now the steps: Connect, we send version.
    GAME_STATUS_WAITVERSION,   // wait for response... add up in version cmd.
    GAME_STATUS_SETUP,         // we ready to send setup commands.
    GAME_STATUS_WAITSETUP,     // we wait for server response.
    GAME_STATUS_REQUEST_FILES, // after we get response from setup, we request files if needed.
    GAME_STATUS_ADDME,         // all setup is done, now try to enter game!
    GAME_STATUS_LOGIN,         // now we wait for LOGIN request of the server.
    GAME_STATUS_NAME,          // all this here is tricky
    GAME_STATUS_PSWD,          // server will trigger this when asking for
    GAME_STATUS_VERIFYPSWD,    // client will then show input panel or so
    GAME_STATUS_NEW_CHAR,      // show new char creation screen and send /nc command when finished
    GAME_STATUS_WAITFORPLAY,   // we simply wait for game start means, this is not a serial stepping here
    GAME_STATUS_QUIT,          // we are in quit menu
    GAME_STATUS_PLAY,          // we play now!!
} _game_status;



////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class Option
{
  public:
    ////////////////////////////////////////////////////////////
    // Variables.
    ////////////////////////////////////////////////////////////
    _options  *my_options;
    int GameStatus;
    std::string mMetaServer;
    unsigned int  mMetaServerPort;
    unsigned int  mSelectedMetaServer;
    bool mStartNetwork;

    ////////////////////////////////////////////////////////////
    // Functions.
    ////////////////////////////////////////////////////////////
     Option();
    ~Option();
    static Option &getSingleton() { static Option Singleton; return Singleton; }

    bool openDescFile(const char *filename);
    void closeDescFile();
    bool getDescStr(const char *descrEntry, string &strBuffer, unsigned int nr=0);
    bool Init();

  private:
    ////////////////////////////////////////////////////////////
    // Variables.
    ////////////////////////////////////////////////////////////
    bool mLogin;
    ifstream *mDescFile;
    string mDescBuffer;
    string mFilename;

    ////////////////////////////////////////////////////////////
    // Functions.
    ////////////////////////////////////////////////////////////
    Option(const Option&); // disable copy-constructor.
};

#endif
