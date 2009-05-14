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

#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <Ogre.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_mutex.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#endif

/**
 ** This singleton class handles all network related stuff except serverfiles.
 ** Files send by the server are handled in network_serverfile.
 *****************************************************************************/
class Network
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {SC_NORMAL, SC_FIRERUN, SC_ALWAYS};
    enum {VERSION_CS = 991027};
    enum {VERSION_SC = 991027};

    // List of client to server (cs) binary command tags.
    enum client_cmd
    {
        // start of pre-processed cmds
        CLIENT_CMD_PING = 0, // unused.
        // Ns_Login mode only commands.
        CLIENT_CMD_SETUP,
        CLIENT_CMD_REQUESTFILE,
        CLIENT_CMD_CHECKNAME,
        CLIENT_CMD_LOGIN,
        // Ns_Account mode only commands.
        CLIENT_CMD_NEWCHAR,
        CLIENT_CMD_DELCHAR,
        CLIENT_CMD_ADDME,
        // Ns_Playing mode only commands.
        CLIENT_CMD_FACE, // special case: Allowed since Ns_Login for face sending servers.
        CLIENT_CMD_MOVE,
        // end of pre-processed cmds.
        CLIENT_CMD_APPLY,
        CLIENT_CMD_EXAMINE,
        CLIENT_CMD_INVMOVE,
        CLIENT_CMD_GUITALK,
        CLIENT_CMD_LOCK,
        CLIENT_CMD_MARK,
        CLIENT_CMD_FIRE,
        CLIENT_CMD_GENERIC,
        CLIENT_CMD_SUM
    };
    enum console_cmd
    {
        CONSOLE_CMD_APPLY,
        CONSOLE_CMD_BUDDY,
        CONSOLE_CMD_CFILTER,
        CONSOLE_CMD_CHANGESKIN,
        CONSOLE_CMD_CHANNEL,
        CONSOLE_CMD_FKEY,           //< Function keys (F1...F9)
        CONSOLE_CMD_IGNORE,
        CONSOLE_CMD_IMGSTATS,
        CONSOLE_CMD_KEYBIND,
        CONSOLE_CMD_KILLS,
        CONSOLE_CMD_MARKDMBUSTER,
        CONSOLE_CMD_RDYSPELL,
        CONSOLE_CMD_RELOADSKIN,
        CONSOLE_CMD_REPLY,
        CONSOLE_CMD_RESET,
        CONSOLE_CMD_SEARCHPATH,
        CONSOLE_CMD_SETWIN,
        CONSOLE_CMD_SETWINALPHA,
        CONSOLE_CMD_SHOUTOFF,
        CONSOLE_CMD_SHOUTON,
        CONSOLE_CMD_SLEEPTIMER,
        CONSOLE_CMD_STATSRESET,
        CONSOLE_CMD_TARGET,
        CONSOLE_CMD_TESTSTRETCH,
        CONSOLE_CMD_SUM,
    };

    typedef struct
    {
        const char *cmd;
        unsigned int index;
    }
    ConsoleCmdString;
    static ConsoleCmdString mConsoleCmd[CONSOLE_CMD_SUM];

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Network &getSingleton()
    {
        static Network Singleton; return Singleton;
    }
    bool Init();
    bool OpenActiveServerSocket();
    static void CloseSocket(int &socket);
    static void CloseClientSocket();
    static void send_game_command(const char *command);
    static int  send_command_binary(Ogre::uchar cmd, std::stringstream &stream);
    static int  GetInt_String(Ogre::uchar *data);
    static short GetShort_String(Ogre::uchar *data);
    void clearMetaServerData();
    void socket_thread_start();
    void setActiveServer(int nr)
    {
        mActServerNr = nr;
    }
    bool InitSocket();
    void read_metaserver_data(int &socket);
    void freeRecources();
    void update();
    void contactMetaserver();
    void add_metaserver_data(Ogre::String metaData);
    // Commands
    static void DrawInfoCmd    (Ogre::uchar *data, int len);
    static void AddMeFail      (Ogre::uchar *data, int len);
    static void Map2Cmd        (Ogre::uchar *data, int len);
    static void DrawInfoCmd2   (Ogre::uchar *data, int len);
    static void ItemXCmd       (Ogre::uchar *data, int len);
    static void SoundCmd       (Ogre::uchar *data, int len);
    static void TargetObject   (Ogre::uchar *data, int len);
    static void ItemUpdateCmd  (Ogre::uchar *data, int len);
    static void ItemDeleteCmd  (Ogre::uchar *data, int len);
    static void StatsCmd       (Ogre::uchar *data, int len);
    static void ImageCmd       (Ogre::uchar *data, int len);
    static void Face1Cmd       (Ogre::uchar *data, int len);
    static void SkillRdyCmd    (Ogre::uchar *data, int len);
    static void PlayerCmd      (Ogre::uchar *data, int len);
    static void SpelllistCmd   (Ogre::uchar *data, int len);
    static void SkilllistCmd   (Ogre::uchar *data, int len);
    static void GolemCmd       (Ogre::uchar *data, int len);
    static void AccNameSuccess (Ogre::uchar *data, int len);
    static void SetupCmd       (Ogre::uchar *data, int len);
    static void DataCmd        (Ogre::uchar *data, int len);
    static void ItemYCmd       (Ogre::uchar *data, int len);
    static void GroupCmd       (Ogre::uchar *data, int len);
    static void GroupInviteCmd (Ogre::uchar *data, int len);
    static void GroupUpdateCmd (Ogre::uchar *data, int len);
    static void InterfaceCmd   (Ogre::uchar *data, int len);
    static void BookCmd        (Ogre::uchar *data, int len);
    static void MarkCmd        (Ogre::uchar *data, int len);
    static void AccountCmd     (Ogre::uchar *data, int len);
    // Commands helper.
    static void CreatePlayerAccount();
    static void PreParseInfoStat(char *cmd);
    const char *get_metaserver_info(int line, int infoLineNr);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        Ogre::String name;
        Ogre::String ip;
        Ogre::String version;
        Ogre::String desc[4];
        int player;
        int port;
    }
    Server;
    std::vector<Server*>mvServer;

    typedef struct command_buffer
    {
        struct command_buffer *next; // Next in queue.
        struct command_buffer *prev; // Previous in queue.
        int len;
        Ogre::uchar *data;
    }
    command_buffer;

    static int mSocket;
    static bool mAbortThread;
    static bool mEndianConvert;
    static SDL_Thread *mInputThread;
    static SDL_Thread *mOutputThread;
    static SDL_mutex  *mMutex;
    static SDL_cond   *mOutputCond;
    static SDL_cond   *mInputCond;
    static command_buffer *mInputQueueStart,  *mInputQueueEnd;
    static command_buffer *mOutputQueueStart, *mOutputQueueEnd;

    int mActServerNr;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    Network()  {}
    ~Network() {}
    Network(const Network&); // disable copy-constructor.
    bool OpenSocket(const char *host, int port, int &socket);
    static Ogre::String &getError();
    static command_buffer *command_buffer_new(unsigned int len, Ogre::uchar *data);
    static command_buffer *command_buffer_dequeue(command_buffer **queue_start, command_buffer **queue_end);
    static int strToInt(Ogre::uchar *buf, int bytes); /**< Must be private to make it thread safe **/
    static bool console_command_check(Ogre::String cmd);
    static void do_console_cmd(Ogre::String &stCmd, int cmd);
    static void command_buffer_free(command_buffer *buf);
    static void command_buffer_enqueue(command_buffer *buf, command_buffer **queue_start, command_buffer **queue_end);
    static void checkFileStatus(const char *cmd, char *param, int fileNr);
    static void AddIntToString(Ogre::String &sl, int data, bool shortInt);
    static int reader_thread_loop(void *);
    static int writer_thread_loop(void *);
};

#endif
