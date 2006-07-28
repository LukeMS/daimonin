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

#ifdef WIN32
 #define STRICT
 #include <winsock2.h>
#else
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <errno.h>
 #include <fcntl.h>
const int SOCKET_ERROR =-1;
#endif
#include <string>
#include "network.h"
#include "logger.h"
#include "option.h"
#include "define.h"
#include "network_serverfile.h"

#include "tile_manager.h"
#include "gui_manager.h"

#define DEBUG_ON

int SoundStatus=1;

enum
{
    CMD_COMPLETE=1,
    CMD_VERSION,
    CMD_DRAW_INFO,
    CMD_ADD_ME_FAIL,
    CMD_MAP2,
    CMD_DRAW_INFO2,
    CMD_ITEM_X,
    CMD_SOUND,
    CMD_TARGET_OBJECT,
    CMD_UPDATE_ITEM,
    CMD_DELETE_ITEM,
    CMD_STATS,
    CMD_IMAGE,
    CMD_FACE1,
    CMD_ANIM,
    CMD_SKILL_RDY,
    CMD_SKILL_RDY_PLAYER,
    CMD_SPELL_LIST,
    CMD_SKILL_LIST,
    CMD_GOLEM,
    CMD_ADD_ME_SUCCSESS,
    CMD_GOOD_BYE,
    CMD_SETUP,
    CMD_HANDLE_QUERY,
    CMD_DATA,

    CMD_NEW_CHAR,
    CMD_ITEM_Y,
    CMD_GROUP,
    CMD_GROUP_UPDATE,
    CMD_INTERFACE,
    CMD_BOOK,
    CMD_MARK,
    CMD_SUM
};
/*
 { "comc", CompleteCmd},
 { "version", (CmdProc) VersionCmd },
 { "drawinfo", (CmdProc) DrawInfoCmd },
 { "addme_failed", (CmdProc) AddMeFail },
 { "map2", Map2Cmd },
    { "drawinfo2", (CmdProc) DrawInfoCmd2 },
    { "itemx", ItemXCmd },
    { "sound", SoundCmd},
    { "to", TargetObject },
    { "upditem", UpdateItemCmd },
    { "delitem", DeleteItem },
    { "stats", StatsCmd },
    { "image", ImageCmd },
    { "face1", Face1Cmd},
    { "anim", AnimCmd},
    { "skill_rdy", (CmdProc) SkillRdyCmd },
    { "player", PlayerCmd },
    { "splist", SpelllistCmd },
    { "sklist", SkilllistCmd },
    { "gc", GolemCmd },
    { "addme_success", (CmdProc) AddMeSuccess },
    { "goodbye", (CmdProc) GoodbyeCmd },
    { "setup", (CmdProc) SetupCmd},
    { "query", (CmdProc) handle_query},
    { "data", (CmdProc) DataCmd},
    { "new_char", (CmdProc) NewCharCmd},
    { "itemy", ItemYCmd },
    { "group", GroupCmd },
    { "group_invite", GroupInviteCmd },
    { "group_update", GroupUpdateCmd },
    { "interface", InterfaceCmd },
    { "book", BookCmd },
    { "mark", MarkCmd },
*/

//================================================================================================
// Check if the username contains any invalid character.
//================================================================================================
inline bool is_username_valid(const char *name)
{
    for(int i=0; i< (int)strlen(name); ++i)
    {
        if (name[i]!= '_' && !(((name[i] <= 90) && (name[i]>=65))||((name[i] >= 97) && (name[i]<=122))))
            return false;
    }
    return true;
}

//================================================================================================
// Clear the MeatServer list.
//================================================================================================
void Network::clear_metaserver_data(void)
{
    while (mServerList.size())
    {
        mStructServer *node = mServerList.front();
        if (node) delete node;
        mServerList.pop_front();
    }
}

//================================================================================================
// Add a MetaServer to the list.
//================================================================================================
void Network::add_metaserver_data(const char *server, int port, int player, const char *ver,
                                  const char *desc1, const char *desc2, const char *desc3, const char *desc4)
{
    mStructServer *node = new mStructServer;
    mServerList.push_back (node);
    node->player = player;
    node->port  = port;
    node->nameip = server;
    node->version = ver;
    node->desc1  = desc1;
    node->desc2  = desc2;
    node->desc3  = desc3;
    node->desc4  = desc4;
}

//================================================================================================
// Get ServerName and ServerPort.
//================================================================================================
void Network::get_meta_server_data(int num, char *server, int *port)
{
    list<mStructServer*>::const_iterator iter;
    for (iter=mServerList.begin(); num > 0; iter++)
    {
        if (iter != mServerList.end()) return;
        --num;
    }
    strcpy(server, (*iter)->nameip.c_str());
    *port = (*iter)->port;
}

//================================================================================================
// Constructor.
//================================================================================================
Network::Network()
{
    dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
}

//================================================================================================
// Destructor.
//================================================================================================
Network::~Network()
{
    delete[] mInbuf.buf;
}

//================================================================================================
// Sends a reply to the server.  text contains the null terminated string
// of text to send.  This function basically just packs the stuff up.
//================================================================================================
void Network::send_reply(const char *text)
{
    char buf[MAXSOCKBUF];
    sprintf(buf, "reply %s", text);
    cs_write_string(buf, (int) strlen(buf));
}

//================================================================================================
///
//================================================================================================
void Network::Update()
{
    static char buf[1024];
    // ////////////////////////////////////////////////////////////////////
    // Connected:
    // ////////////////////////////////////////////////////////////////////
    if (Option::getSingleton().getGameStatus() > GAME_STATUS_CONNECT)
    {
        if (mSocket == SOCKET_NO)
        {
            // connection closed, so we go back to GAME_STATUS_INIT_NET here.
            if (Option::getSingleton().getGameStatus() == GAME_STATUS_PLAY)
            {
                Option::getSingleton().setGameStatus(GAME_STATUS_INIT_NET);
                Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, false);
                mPasswordAlreadyAsked = 0;
            }
            else
            {
                Option::getSingleton().setGameStatus(GAME_STATUS_START);
            }
        }
        else
        {
            DoClient();
            //request_face(0, 1); // flush face request buffer
        }
    }

    if (Option::getSingleton().getGameStatus() == GAME_STATUS_PLAY)
        return;

    // ////////////////////////////////////////////////////////////////////
    // Not connected: walk through connection chain and/or wait for action
    // ////////////////////////////////////////////////////////////////////
    switch (Option::getSingleton().getGameStatus())
    {
        // ////////////////////////////////////////////////////////////////////
        // Autoinit or reset prg data.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_INIT_NET:
        clear_metaserver_data();
        Logger::log().info() << "GAME_STATUS_INIT_NET";
        Option::getSingleton().setGameStatus(GAME_STATUS_META);
        break;

        // ////////////////////////////////////////////////////////////////////
        // connect to meta and get server data
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_META:
        Logger::log().info() << "GAME_STATUS_META";
        if (!Option::getSingleton().getStrValue(Option::CMDLINE_SERVER_NAME))
        {
            add_metaserver_data(
                Option::getSingleton().getStrValue(Option::CMDLINE_SERVER_NAME),
                Option::getSingleton().getIntValue(Option::CMDLINE_SERVER_PORT),
                -1, "user server", "Server from -server '...' command line.", "", "", "");
        }
        /*
              // skip of -nometa in command line or no metaserver set in options
              if (options.no_meta || !options.metaserver[0])
              {
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Option '-nometa'.metaserver ignored.");
              }
              else
        */
        {
            Logger::log().info()
            << "Query Metaserver " << Option::getSingleton().getIntValue(Option::META_SERVER_NAME)
            << " on port "         << Option::getSingleton().getIntValue(Option::META_SERVER_PORT);
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"query metaserver...");
            sprintf(buf, "trying %s:%d", Option::getSingleton().getStrValue(Option::META_SERVER_NAME), Option::getSingleton().getIntValue(Option::META_SERVER_PORT));
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)buf);
            if (OpenSocket(Option::getSingleton().getStrValue(Option::META_SERVER_NAME),Option::getSingleton().getIntValue(Option::META_SERVER_PORT)))
            {
                read_metaserver_data();
                CloseSocket();
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"done.");
            }
            else
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"metaserver failed! using default list.");
        }
        add_metaserver_data("127.0.0.1", 13327, -1, "local", "localhost. Start server before you try to connect.", "", "", "");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"select a server.");
        Option::getSingleton().setGameStatus(GAME_STATUS_START);
        break;

        // ////////////////////////////////////////////////////////////////////
        // Go into standby.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_START:
        if (mSocket != SOCKET_NO) CloseSocket();
        Option::getSingleton().setGameStatus(GAME_STATUS_WAITLOOP);
        break;

        // ////////////////////////////////////////////////////////////////////
        // Wait for user to select a server.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_WAITLOOP:
        /*
               Dialog::getSingleton().setVisible(true);
              if (TextInput::getSingleton().startCursorSelection(mServerList.size()))
              {
                list<mStructServer*>::const_iterator iter = mServerList.begin();
                for (unsigned int i=0 ; iter != mServerList.end(); ++iter)
                {
                  Dialog::getSingleton().setSelText(i++, (*iter)->nameip.c_str());
                  // Fill the dialog-Info field.
                  if (i == TextInput::getSingleton().getSelCursorPos())
                  {
                    Dialog::getSingleton().setInfoText(0, (*iter)->desc1.c_str());
                    Dialog::getSingleton().setInfoText(1, (*iter)->desc2.c_str());
                  }
                }
                Dialog::getSingleton().UpdateLogin(DIALOG_STAGE_GET_META_SERVER);
              }
              if (TextInput::getSingleton().wasCanceled())
              {
                TextInput::getSingleton().stop();
                Dialog::getSingleton().setVisible(false);
                Option::getSingleton().mStartNetwork = false;
                Option::getSingleton().setGameStatus(GAME_STATUS_INIT_NET);
              }
              else if (TextInput::getSingleton().wasFinished())
              {
                TextInput::getSingleton().stop();
                Option::getSingleton().mSelectedMetaServer = TextInput::getSingleton().getSelCursorPos();
                Option::getSingleton().setGameStatus(GAME_STATUS_STARTCONNECT);
              }
              if (TextInput::getSingleton().hasChanged())
              {
                list<mStructServer*>::const_iterator iter = mServerList.begin();
                for (unsigned int i=0 ; i < TextInput::getSingleton().getSelCursorPos(); ++i)
                {
                  ++iter;
                }
                Dialog::getSingleton().setInfoText(0, (*iter)->version.c_str(), TXT_WHITE);
                Dialog::getSingleton().setInfoText(1, (*iter)->desc1.c_str());
                Dialog::getSingleton().setInfoText(2, "");
                Dialog::getSingleton().setInfoText(3, "");
                Dialog::getSingleton().UpdateLogin(DIALOG_STAGE_GET_META_SERVER);
              }
        */
        // Testing: delete me.
        Option::getSingleton().setIntValue(Option::SEL_META_SEVER, 0);
        Option::getSingleton().setGameStatus(GAME_STATUS_STARTCONNECT);
        break;

        // ////////////////////////////////////////////////////////////////////
        // Try the selected server.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_STARTCONNECT:
        Option::getSingleton().setGameStatus(GAME_STATUS_CONNECT);
        break;

        // ////////////////////////////////////////////////////////////////////
        // This Server was selected in the dialog-window.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_CONNECT:
        {
            mGameStatusVersionFlag = false;
            //      Dialog::getSingleton().clearInfoText();
            list<mStructServer*>::const_iterator iter = mServerList.begin();
            for (int i=0 ; i < Option::getSingleton().getIntValue(Option::SEL_META_SEVER); ++i)
            {
                ++iter;
            }
            if (!OpenSocket((char*)(*iter)->nameip.c_str(), (*iter)->port))
            {
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"connection failed!");
                Option::getSingleton().setGameStatus(GAME_STATUS_START);
            }
            else
            {
                Option::getSingleton().setGameStatus(GAME_STATUS_VERSION);
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Connected. exchange version.");
            }
        }
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_VERSION:
        Logger::log().info() << "Send Version";
        sprintf(buf, "version %d %d %s", VERSION_CS, VERSION_SC, VERSION_NAME);
        cs_write_string(buf, (int)strlen(buf));
        Option::getSingleton().setGameStatus(GAME_STATUS_WAITVERSION);
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_WAITVERSION:
        Logger::log().info() << "GAME_STATUS_WAITVERSION";
        // perhaps here should be a timer ???
        // remember, the version exchange server<->client is asynchron so perhaps
        // the server send his version faster as the client send it to server.
        if (mGameStatusVersionFlag) // wait for version answer when needed
        {
            // false version!
            if (!mGameStatusVersionOKFlag)
            {
                Option::getSingleton().setGameStatus(GAME_STATUS_START);
                Logger::log().info() << "GAME_STATUS_START";
            }
            else
            {
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"version confirmed.");
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"starting login procedure...");
                Option::getSingleton().setGameStatus(GAME_STATUS_SETUP);
                Logger::log().info() << "GAME_STATUS_SETUP";
            }
        }
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_SETUP:
        ServerFile::getSingleton().checkFiles();
        sprintf(buf, "setup sound %d map2cmd 1 mapsize %dx%d darkness 1 facecache 1"
                " skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x",
                //   SoundStatus, TileMap::getSingleton().MapStatusX, TileMap::getSingleton().MapStatusY,
                11, 16,16,
                ServerFile::getSingleton().getLength(SERVER_FILE_SKILLS),
                ServerFile::getSingleton().getCRC   (SERVER_FILE_SKILLS),
                ServerFile::getSingleton().getLength(SERVER_FILE_SPELLS),
                ServerFile::getSingleton().getCRC   (SERVER_FILE_SPELLS),
                ServerFile::getSingleton().getLength(SERVER_FILE_BMAPS),
                ServerFile::getSingleton().getCRC   (SERVER_FILE_BMAPS),
                ServerFile::getSingleton().getLength(SERVER_FILE_SETTINGS),
                ServerFile::getSingleton().getCRC   (SERVER_FILE_SETTINGS),
                ServerFile::getSingleton().getLength(SERVER_FILE_ANIMS),
                ServerFile::getSingleton().getCRC   (SERVER_FILE_ANIMS));
        cs_write_string(buf, (int)strlen(buf));
        buf[strlen(buf)] =0;
        Logger::log().info() << "Send: setup " << buf;
        mRequest_file_chain = 0;
        mRequest_file_flags = 0;
        Option::getSingleton().setGameStatus(GAME_STATUS_WAITSETUP);
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_REQUEST_FILES:
        Logger::log().info()  << "GAME_STATUS_REQUEST FILES (" << mRequest_file_chain << ")";
        if (mRequest_file_chain == 0) // check setting list
        {
            if (ServerFile::getSingleton().getStatus(SERVER_FILE_SETTINGS)
                    == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 1;
                RequestFile(SERVER_FILE_SETTINGS);
            }
            else mRequest_file_chain = 2;
        }
        else if (mRequest_file_chain == 2) // check spell list
        {
            if (ServerFile::getSingleton().getStatus(SERVER_FILE_SPELLS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 3;
                RequestFile(SERVER_FILE_SPELLS);
            }
            else
                mRequest_file_chain = 4;
        }
        else if (mRequest_file_chain == 4) // check skill list
        {
            if (ServerFile::getSingleton().getStatus(SERVER_FILE_SPELLS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 5;
                RequestFile(SERVER_FILE_SKILLS);
            }
            else
                mRequest_file_chain = 6;
        }
        else if (mRequest_file_chain == 6)
        {
            if (ServerFile::getSingleton().getStatus(SERVER_FILE_BMAPS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 7;
                RequestFile(SERVER_FILE_BMAPS);
            }
            else
                mRequest_file_chain = 8;
        }
        else if (mRequest_file_chain == 8)
        {
            if (ServerFile::getSingleton().getStatus(SERVER_FILE_ANIMS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 9;
                RequestFile(SERVER_FILE_ANIMS);
            }
            else
                mRequest_file_chain = 10;
        }
        else if (mRequest_file_chain == 10) // we have all files - start check
        {
            mRequest_file_chain++; // this ensure one loop tick and updating the messages
        }
        else if (mRequest_file_chain == 11)
        {
            // ok... now we check for bmap & anims processing...
            // TileGfx::getSingleton().read_bmap_tmp();
            // TileGfx::getSingleton().read_anim_tmp();
            // load_settings();
            mRequest_file_chain++;
        }
        else if (mRequest_file_chain == 12)
        {
            mRequest_file_chain++; // this ensure one loop tick and updating the messages
        }
        else if (mRequest_file_chain == 13)
            Option::getSingleton().setGameStatus(GAME_STATUS_ADDME);
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_ADDME:
        cs_write_string("addme", 5);  // SendAddMe
        /*
           map_transfer_flag = 0;
           cpl.name[0] = 0;
           cpl.password[0] = 0;
        */
        Option::getSingleton().setGameStatus(GAME_STATUS_LOGIN);
        // now wait for login request of the server
        break;

        // ////////////////////////////////////////////////////////////////////
        // Server wants Player name.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_NAME_INIT:
        // map_transfer_flag = 0;
        GuiManager::getSingleton().showWindow(GUI_WIN_LOGIN, true);
        GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_NAME, 20, true, true);
        Option::getSingleton().setGameStatus(GAME_STATUS_NAME_LOOP);
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_NAME_LOOP:
        if (GuiManager::getSingleton().brokenTextInput())
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Break Login.");
            GuiManager::getSingleton().showWindow(GUI_WIN_PLAYERINFO, false);
            Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, false);
            Option::getSingleton().setGameStatus(GAME_STATUS_START);
        }
        else  if (GuiManager::getSingleton().finishedTextInput())
        {
            if (is_username_valid(GuiManager::getSingleton().getTextInput()))
            {
                char name_tmp[256];

                //strcpy(cpl.name, InputString);
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
                sprintf(name_tmp,"%c%s", GameStatusLogin?'L':'C', GuiManager::getSingleton().getTextInput());
                Logger::log().info() << "Login: send name" << name_tmp;
                send_reply(name_tmp);
                Option::getSingleton().setGameStatus(GAME_STATUS_NAME_WAIT);
                // now wait again for next server question
            }
            else
            {
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_WRONG;
                GuiManager::getSingleton().cancelTextInput();
                Option::getSingleton().setGameStatus(GAME_STATUS_NAME_INIT);
            }
        }
        break;

        // ////////////////////////////////////////////////////////////////////
        // Server wants Player password.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_PSWD_INIT:
        GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_PASSWD, 20, true, true);
        Option::getSingleton().setGameStatus(GAME_STATUS_PSWD_LOOP);
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_PSWD_LOOP:
        //map_transfer_flag = 0;
        if (GuiManager::getSingleton().brokenTextInput())
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"Break Login.");
            GuiManager::getSingleton().showWindow(GUI_WIN_PLAYERINFO, false);
            Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, false);
            Option::getSingleton().setGameStatus(GAME_STATUS_START);
        }
        else  if (GuiManager::getSingleton().finishedTextInput())
        {
            const char *InputString = GuiManager::getSingleton().getTextInput();
            int strLen = (int) strlen(InputString);
            if (!GameStatusLogin && (strLen < 6 || strLen > 17))
            {
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_PWD_SHORT;
            }
            /*
                            else if (!GameStatusLogin && !strcmp(cpl.name, InputString))
                            {
                                dialog_login_warning_level = DIALOG_LOGIN_WARNING_PWD_NAME;
                            }
            */
            else
            {
                Logger::log().info() << "Login: send password <*****>";
                send_reply(InputString);
                Option::getSingleton().setGameStatus(GAME_STATUS_PSWD_WAIT);
            }
            // now wait again for next server question
        }
        break;

        // ////////////////////////////////////////////////////////////////////
        // Server wants Player password AGAIN.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_VRFY_INIT:
        GuiManager::getSingleton().startTextInput(GUI_WIN_LOGIN, GUI_TEXTINPUT_LOGIN_VERIFY, 20, true, true);
        Option::getSingleton().setGameStatus(GAME_STATUS_VRFY_LOOP);
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_VRFY_LOOP:
        {
            //map_transfer_flag = 0;
            char *InputString = (char*)GuiManager::getSingleton().getTextInput();
            if (GuiManager::getSingleton().brokenTextInput())
            {
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"Break Login.");
                GuiManager::getSingleton().showWindow(GUI_WIN_PLAYERINFO, false);
                Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, false);
                Option::getSingleton().setGameStatus(GAME_STATUS_START);
            }
            else  if (GuiManager::getSingleton().finishedTextInput())
            {
                Logger::log().info() << "Login: send verify password <*****>";

                Logger::log().info() << InputString;
                send_reply(InputString);
                Option::getSingleton().setGameStatus(GAME_STATUS_VRFY_WAIT);
                // now wait again for next server question
            }
        }
        break;

        // ////////////////////////////////////////////////////////////////////
        // Create a new character.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_NEW_CHAR:
        Option::getSingleton().setGameStatus(GAME_STATUS_WAITFORPLAY);
        /*
               if (Dialog::getSingleton().UpdateNewChar())
              {
                CreatePlayerAccount();
                Option::getSingleton().setGameStatus(GAME_STATUS_WAITFORPLAY);
              }
        */
        break;

        // ////////////////////////////////////////////////////////////////////
        // .
        // ////////////////////////////////////////////////////////////////////
        case  GAME_STATUS_WAITFORPLAY:
        /*
              clear_map();
              map_draw_map_clear();
              map_udate_flag = 2;
              map_transfer_flag = 1;
        */
        break;

        // ////////////////////////////////////////////////////////////////////
        // User quits the game.
        // ////////////////////////////////////////////////////////////////////
        case GAME_STATUS_QUIT:
        // map_transfer_flag = 0;
        break;
    }
}



//================================================================================================
// Init the network.
//================================================================================================
bool Network::Init()
{
    Logger::log().headline("Init Network");
    mSocket = SOCKET_NO;
    mInbuf.buf = new unsigned char[MAXSOCKBUF+1];
    mInbuf.buf[0] =0;
    mInbuf.buf[1] =0;
    mGameStatusVersionFlag  = false;
    mGameStatusVersionOKFlag= true;
    Logger::log().info() << "init socket...";
    bool status = InitSocket();
    Logger::log().success(status);
    return status;
}

//================================================================================================
// Check for Winsock.
//================================================================================================
inline bool Network::InitSocket()
{
#ifdef WIN32
    mCs_version = 0;
    mSocketStatusErrorNr = 0;
    WSADATA w;
    int error = WSAStartup(0x0101, &w);
    if (error)
    {
        Logger::log().error() << "Init Winsock failed: " << error;
        return false;
    }
    if (w.wVersion != 0x0101)
    {
        Logger::log().error() << "Wrong WinSock version!";
        return false;
    }
#endif
    return true;
}

//================================================================================================
// Open a scoket to "host" on "port"
//================================================================================================
bool Network::OpenSocket(const char *host, int port)
{
    mOpenPort = port;
    mInbuf.len = 0;
    sockaddr_in mInsock;
    mInsock.sin_family = AF_INET;
    mInsock.sin_port = htons((unsigned short) port);
    if (isdigit(*host))
    {
        mInsock.sin_addr.s_addr = inet_addr(host);
    }
    else
    {
        struct hostent *mHostbn = gethostbyname(host);
        if (mHostbn == (struct hostent *) 0)
        {
            Logger::log().warning() << "Unknown host: " << host;
            mSocket = SOCKET_NO;
            return false;
        }
        memcpy(&mInsock.sin_addr, mHostbn->h_addr, mHostbn->h_length);
    }
    mCommand_time = 0;
    mCommand_sent = 0;
    mCommand_received = 0;

#ifdef WIN32
    // The way to make the sockets work on XP Home - The 'unix' style socket seems to fail inder xp home.
    unsigned long temp = 1; // non-block.
    mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ioctlsocket(mSocket, FIONBIO, &temp) == -1)
    {
        Logger::log().error() << "Error in ioctlsocket(*socket_temp, FIONBIO , &temp)";
        mSocket = SOCKET_NO;
        Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, false);
        return false;
    }
    int error = 0;
    int retries = 15;
    while (connect(mSocket, (struct sockaddr *) &mInsock, sizeof(mInsock)) == SOCKET_ERROR)
    {
        Sleep(3);
        if (--retries == 0)
        {
            Logger::log().error() << "Connect Error: " << mSocketStatusErrorNr;
            mSocket = SOCKET_NO;
            Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, false);
            return false;
        }
        mSocketStatusErrorNr = WSAGetLastError();
        if (mSocketStatusErrorNr == WSAEISCONN)  // we have a connect!
            break;
        if (mSocketStatusErrorNr == WSAEWOULDBLOCK
                || mSocketStatusErrorNr == WSAEALREADY
                ||(mSocketStatusErrorNr == WSAEINVAL && error)) // loop until we finished
        {
            error = 1;
        }
    }
#else
    struct protoent *protox;
    protox = getprotobyname("tcp");
    if (!protox)
    {
        Logger::log().error() << "Error an getting ProtoByName (tcp)";
        return false;
    }
    mSocket = socket(PF_INET, SOCK_STREAM, protox->p_proto);
    if (mSocket == -1)
    {
        Logger::log().error() << "Init connection: Error on socket command.";
        mSocket = SOCKET_NO;
        Option::getSingleton().setIntValue(Option::UPDATE_NETWORK, false);
        return false;
    }
    if (connect(mSocket,(struct sockaddr *)&mInsock,sizeof(mInsock)) ==  SOCKET_ERROR)
    {
        Logger::log().error() << "Can't connect to server";
        return false;
    }
    if (fcntl(mSocket, F_SETFL, O_NDELAY) == SOCKET_ERROR)
    {
        Logger::log().error() << "InitConnection:  Error on fcntl.\n";
    }
#endif

    // ////////////////////////////////////////////////////////////////////
    // we got a connect here!
    // ////////////////////////////////////////////////////////////////////
    int oldbufsize, newbufsize = 65535;
#ifdef WIN32
    int buflen = sizeof(int);
#else
    socklen_t buflen = sizeof(int);
#endif
    if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
    {
        oldbufsize = 0;
    }
    if (oldbufsize < newbufsize)
    {
        if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            Logger::log().error() << "InitConnection: setsockopt unable to set output buf size to " << newbufsize;
            setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
        }
    }
    Logger::log().info() << "Connected to " << host << ":" << port;
    return true;
}

//================================================================================================
// Close a socket.
//================================================================================================
bool Network::CloseSocket()
{
    if (mSocket == SOCKET_NO)
    {
        return true;
    }
#ifdef WIN32
    // seems differents sockets have different way to shutdown connects??
    // win32 needs this hard way, normally you should wait for a read() == 0...
    shutdown(mSocket, SD_BOTH);
    closesocket(mSocket);
#else
    close(mSocket);
#endif
    mSocket = SOCKET_NO;
    return true;
}

//================================================================================================
// Read data from the metaserver.
//================================================================================================
void Network::read_metaserver_data()
{
    int stat;
    char buffer[MAX_METASTRING_BUFFER];
    string strMetaData ="";
    while(1)
    {
#ifdef WIN32
        stat = recv (mSocket, buffer, MAX_METASTRING_BUFFER, 0);
        if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)
        {
            Logger::log().error()  << "Error reading metaserver data!: " << WSAGetLastError();
            break;
        }
#else
        do
        {
            stat = recv (mSocket, buffer, MAX_METASTRING_BUFFER, 0);
        }
        while (stat == -1);
#endif
        if(stat == 0)
        {
            break;
        } // connect closed by meta
        if(stat >  0)
        {
            strMetaData += buffer;
        }
    }

    // ////////////////////////////////////////////////////////////////////
    // Parse the metadata.
    // ////////////////////////////////////////////////////////////////////
    Logger::log().info() << "GET: " << strMetaData;
    unsigned int startPos=0, endPos;
    string strIP, str1, strName, strPlayer, strVersion, strDesc1, strDesc2, strDesc3, strDesc4;
    // Server IP.
    endPos = (int)strMetaData.find( '|',  startPos);
    strIP = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // unknown 1.
    endPos = (int)strMetaData.find( '|',  startPos);
    str1 = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // Server name.
    endPos = (int)strMetaData.find( '|',  startPos);
    strName = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // Number of players online.
    endPos = (int)strMetaData.find( '|',  startPos);
    strPlayer = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // Server version.
    endPos = (int)strMetaData.find( '|',  startPos);
    strVersion = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // Description1
    endPos = (int)strMetaData.find( '|',  startPos);
    strDesc1 = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // Description2.
    endPos = (int)strMetaData.find( '|',  startPos);
    strDesc2 = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // Description3.
    endPos = (int)strMetaData.find( '|',  startPos);
    strDesc3 = strMetaData.substr(startPos, endPos-startPos);
    startPos = endPos+1;
    // Description4.
    strDesc4 = strMetaData.substr(startPos, strMetaData.size()-startPos);

    add_metaserver_data(strName.c_str(), mOpenPort, atoi(strPlayer.c_str()), strVersion.c_str(),
                        strDesc1.c_str(),  strDesc2.c_str(),  strDesc3.c_str(),  strDesc4.c_str());
}

//================================================================================================
// Takes a string of data, and writes it out to the socket. A very handy
// shortcut function.
//================================================================================================
int Network::cs_write_string(const char *buf, int len)
{
    static SockList sl;
    sl.len = len;
    sl.buf = (unsigned char *) buf;
    return send_socklist(sl);
}

//================================================================================================
///
//================================================================================================
int Network::send_socklist(SockList &msg)
{
    unsigned char sbuf[2];
    sbuf[0] = ((unsigned int) (msg.len) >> 8) & 0xFF;
    sbuf[1] = ((unsigned int) (msg.len)     ) & 0xFF;
    write_socket(sbuf, 2);
    return write_socket(msg.buf, msg.len);
}

//================================================================================================
// Write socket.
//================================================================================================
int Network::write_socket(unsigned char *buf, int len)
{
    int amt = 0;
    unsigned char *pos = buf;

    //LogFile::getSingleton().Error("write socket befehl: %s %d\n",  buf, len);
    // If we manage to write more than we wanted, take it as a bonus
    while (len > 0)
    {
#ifdef WIN32
        amt = send(mSocket, (char*)pos, len, 0);
        if (amt == -1 && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            Logger::log().error()  << "New socket write failed (wsb) ("  << WSAGetLastError() << ").";
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"SOCKET ERROR: Server write failed.");
            return -1;
        }
        if (amt == 0)
        {
            Logger::log().error()  << "Write_To_Socket: No data written out (" << WSAGetLastError() << ").";
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"SOCKET ERROR: No data written out");
            return -1;
        }
#else
        amt = write(mSocket, pos, len);
        if (amt < 0)
        {
            if (errno==EINTR)
            {
                continue;
            }
            Logger::log().error() << "New socket (fd=" << mSocket << ") write failed.";
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"SOCKET ERROR: Server write failed.");
            return -1;
        }
#endif
        len -= amt;
        pos += amt;
    }
    return 0;
}

//================================================================================================
///
//================================================================================================
void Network::DoClient()
{
    const int OFFSET = 3;  // 2 byte package len + 1 byte binary cmd.
    int             pollret;
    struct timeval  timeout;
    fd_set tmp_read, tmp_write, tmp_exceptions;

    FD_ZERO(&tmp_read);
    FD_ZERO(&tmp_write);
    FD_ZERO(&tmp_exceptions);
    FD_SET((unsigned int) mSocket, &tmp_exceptions);
    FD_SET((unsigned int) mSocket, &tmp_read);
    FD_SET((unsigned int) mSocket, &tmp_write);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if ((pollret = select(mSocket+ 1, &tmp_read, &tmp_write, &tmp_exceptions, &timeout)) == -1)
    {
        Logger::log().error() << "Got on selectcall.";
        return;
    }
    if (!FD_ISSET(mSocket, &tmp_read)) return;
    int i = read_socket();
    if (i <= 0)
    {   // Need to add some better logic here
        if (i < 0)
        {
            Logger::log().error() << "Got error on read socket.";
            CloseSocket();
        }
        return; // Still don't have a full packet
    }


    switch (mInbuf.buf[2])
    {

        case  CMD_SKILL_RDY_PLAYER:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_RDY_PLAYER (" << (int) mInbuf.buf[2] << ")";
#endif
        PlayerCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;

        case  CMD_COMPLETE:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_COMPLETE (" << (int) mInbuf.buf[2] << ")";
#endif
        // CompleteCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_MAP2:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_MAP2 (" << (int) mInbuf.buf[2] << ")";
#endif
        Map2Cmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case  CMD_DRAW_INFO:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_DRAWINFO (" << (int) mInbuf.buf[2] << ")";
#endif
        // DrawInfoCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case  CMD_DRAW_INFO2:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_DRAWINFO2 (" << (int) mInbuf.buf[2] << ")";
#endif
        // DrawInfoCmd2(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        /*
                case  5: // BINARY_CMD_MAP_SCROLL
        #ifdef DEBUG_ON
                Logger::log().info() << "command: BINARY_CMD_MAP_SCROLL (" << (int) mInbuf.buf[2] << ")";
        #endif
                // map_scrollCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
                break;
        */
        case  CMD_ITEM_X:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_ITEMX (" << (int) mInbuf.buf[2] << ")";
#endif
        // ItemXCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case  CMD_SOUND:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_SOUND (" << (int) mInbuf.buf[2] << ")";
#endif
        // SoundCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case  CMD_TARGET_OBJECT:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_TARGET (" << (int) mInbuf.buf[2] << ")";
#endif
        // TargetObject(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case  CMD_UPDATE_ITEM:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_UPITEM (" << (int) mInbuf.buf[2] << ")";
#endif
        // UpdateItemCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_DELETE_ITEM:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_DELITEM (" << (int) mInbuf.buf[2] << ")";
#endif
        // DeleteItem(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_STATS:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_STATS (" << (int) mInbuf.buf[2] << ")";
#endif
        // StatsCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_IMAGE:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_IMAGE (" << (int) mInbuf.buf[2] << ")";
#endif
        // ImageCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_FACE1:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_FACE1 (" << (int) mInbuf.buf[2] << ")";
#endif
        // Face1Cmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_ANIM:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_ANIM (" << (int) mInbuf.buf[2] << ")";
#endif
        // AnimCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_SKILL_RDY:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_SKILLRDY (" << (int) mInbuf.buf[2] << ")";
#endif
        // SkillRdyCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        /*
                case 16: // BINARY_CMD_PLAYER
        #ifdef DEBUG_ON
                Logger::log().info() << "command: BINARY_CMD_PLAYER (" << (int) mInbuf.buf[2] << ")";
        #endif
                //Dialog::getSingleton().setVisible(false);
                PlayerCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
                break;
                case 17: // BINARY_CMD_MAPSTATS
        #ifdef DEBUG_ON
                Logger::log().info() << "command: BINARY_CMD_MAPSTATS (" << (int) mInbuf.buf[2] << ")";
        #endif
                // MapstatsCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
                break;
        */
        case CMD_SPELL_LIST:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_SPELL_LIST (" << (int) mInbuf.buf[2] << ")";
#endif
        // SpelllistCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_SKILL_LIST:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_SKILL_LIST (" << (int) mInbuf.buf[2] << ")";
#endif
        // SkilllistCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_GOLEM:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_GOLEMCMD (" << (int) mInbuf.buf[2] << ")";
#endif
        // GolemCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_ADD_ME_SUCCSESS:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_ADDME_SUC (" << (int) mInbuf.buf[2] << ")";
#endif
        // AddMeSuccess(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_ADD_ME_FAIL:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_ADDME_FAIL (" << (int) mInbuf.buf[2] << ")";
#endif
        // AddMeFail(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_VERSION:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_VERSION (" << (int) mInbuf.buf[2] << ")";
#endif
        VersionCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_GOOD_BYE:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_BYE (" << (int) mInbuf.buf[2] << ")";
#endif
        // GoodbyeCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_SETUP:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_SETUP (" << (int) mInbuf.buf[2] << ")";
#endif
        SetupCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_HANDLE_QUERY: // BINARY_CMD_QUERY
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_QUERY (" << (int) mInbuf.buf[2] << ")";
#endif
        HandleQuery((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_DATA:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_DATA (" << (int) mInbuf.buf[2] << ")";
#endif
        DataCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_NEW_CHAR:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_NEW_CHAR (" << (int) mInbuf.buf[2] << ")";
#endif
        NewCharCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_ITEM_Y:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_ITEMY (" << (int) mInbuf.buf[2] << ")";
#endif
        // ItemYCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_GROUP:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_GROUP (" << (int) mInbuf.buf[2] << ")";
#endif
        // GroupCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        case CMD_GROUP_UPDATE:
#ifdef DEBUG_ON
        Logger::log().info() << "command: BINARY_CMD_INVITE (" << (int) mInbuf.buf[2] << ")";
#endif
        // GroupInviteCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
        break;
        /*
                case 32: // BINARY_CMD_GROUP_UPDATE
        #ifdef DEBUG_ON
                Logger::log().info() << "command: BINARY_CMD_GROUP_UPDATE (" << (int) mInbuf.buf[2] << ")";
        #endif
                // GroupUpdateCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
                break;
        */
        default: // ERROR
        Logger::log().info() << "command: <UNKNOWN> (" << (int) mInbuf.buf[2] << ")";
        break;
    }
    mInbuf.len =0;
}

//================================================================================================
// read socket.
// We make the assumption the buffer is at least 2 bytes long.
//================================================================================================
int Network::read_socket()
{
    int stat, toread, readsome = 0;
    // We already have a partial packet
    if (mInbuf.len < 2)
    {
#ifdef WIN32
        stat=recv(mSocket, (char*)mInbuf.buf + mInbuf.len, 2 - mInbuf.len, 0);
        if (stat<0)
        {
            if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)
            {
                Logger::log().error()  << "ReadPacket got error " << WSAGetLastError() << ", returning -1\n";
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"WARNING: Lost or bad server connection.");
                return -1;
            }
            return 0;
        }
#else
        do
        {
            stat=recv(mSocket, (char*)mInbuf.buf + mInbuf.len, 2 - mInbuf.len, 0);
        }
        while ((stat==-1) && (errno==EINTR));
        if (stat<0)
        {
            // In non blocking mode, EAGAIN is set when there is no data available.
            if (errno!=EAGAIN && errno!=EWOULDBLOCK)
            {
                Logger::log().error()  << "ReadPacket got error " << errno
                << "%d, returning 0";
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"WARNING: Lost or bad server connection.");
                return -1;
            }
            return 0;
        }
#endif
        if (stat==0)
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"WARNING: Server read package error.");
            return -1;
        }
        mInbuf.len += stat;
        if (stat < 2)
        {
            return 0;
        } // Still don't have a full packet
        readsome = 1;
    }
    // Figure out how much more data we need to read.  Add 2 from the
    // end of this - size header information is not included.
    toread = 2 + (mInbuf.buf[0] << 8) + mInbuf.buf[1] - mInbuf.len;
    if ((toread + mInbuf.len) > MAXSOCKBUF)
    {
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"WARNING: Server read package error.");
        Logger::log().error() << "SockList_ReadPacket: Want to read more bytes than will fit in buffer.";
        // return error so the socket is closed
        return -1;
    }
    do
    {
#ifdef WIN32
        stat = recv(mSocket, (char*)mInbuf.buf + mInbuf.len, toread, 0);
        if (stat<0)
        {
            if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)
            {
#else
        do
        {
            stat = recv(mSocket, (char*)mInbuf.buf + mInbuf.len, toread, 0);
        }
        while ((stat<0) && (errno==EINTR));
        if (stat<0)
        {
            if (errno!=EAGAIN && errno!=EWOULDBLOCK)
            {
#endif
                Logger::log().error()   << "ReadPacket got error " << errno  << ", returning 0";
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"WARNING: Lost or bad server connection.");
                return -1;
            }
            return 0;
        }
        if (stat==0)
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"WARNING: Server read package error.");
            return -1;
        }
        mInbuf.len += stat;
        toread -= stat;
        if (toread == 0)
        {
            return 1;
        }
        if (toread < 0)
        {
            Logger::log().error() << "SockList_ReadPacket: Read more bytes than desired.";
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"WARNING: Server read package error.");
            return -1;
        }
    }
    while (toread > 0);
    return 0;
}

//================================================================================================
// Request a file from server.
//================================================================================================
void Network::RequestFile(int index)
{
    char buf[MAX_BUF];
    sprintf(buf, "rf %d", index);
    cs_write_string(buf, (int)strlen(buf));
}
