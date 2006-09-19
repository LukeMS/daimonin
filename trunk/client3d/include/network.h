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

#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_mutex.h>
#ifdef WIN32
#include <winsock.h>
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

const int SOCKET_NO = -1;

using namespace std;

/// Maximum size of any packet we expect.  Using this makes it so we don't need to
/// allocated and deallocated the same buffer over and over again and the price
/// of using a bit of extra memory. IT also makes the code simpler.
const int  MAXSOCKBUF            =  64*1024;
const int  MAX_METASTRING_BUFFER = 128*2013;

const int  MAX_BUF =  256;
const int  BIG_BUF = 1024;
const int  STRINGCOMMAND = 0;
const int  DATA_PACKED_CMD = 0x80;
const int  SRV_CLIENT_FLAG_BMAP    = 1;
const int  SRV_CLIENT_FLAG_ANIM    = 2;
const int  SRV_CLIENT_FLAG_SETTING = 4;
const int  SRV_CLIENT_FLAG_SKILL   = 8;
const int  SRV_CLIENT_FLAG_SPELL   =16;
const int  MAXMETAWINDOW           =14; // max. shown server in meta window.
const int  VERSION_CS = 991022;
const int  VERSION_SC = 991022;
const char VERSION_NAME[] = "Daimonin SDL Client";

// Contains the base information we use to make up a packet we want to send.
typedef struct SockList
{
    int             len; /**< How much data in buf */
    int             pos; /**< Start of data in buf */
    unsigned char  *buf;
}
SockList;

typedef struct mStructServer
{
    string nameip;
    string version;
    string desc1;
    string desc2;
    string desc3;
    string desc4;
    int player;
    int port;
}
mStructServer;

// ClientSocket could probably hold more of the global values - it could
// probably hold most all socket/communication related values instead
// of globals.
typedef struct ClientSocket
{
    int fd;        // typedef your socket type to SOCKET
    SockList  inbuf;
    SockList  outbuf;
    int       cs_version, sc_version; // Server versions of these
    // These are used for the newer 'windowing' method of commands -
    // number of last command sent, number of received confirmation
    int       command_sent, command_received;
    // Time (in ms) players commands currently take to execute
    int       command_time;
}
ClientSocket;

// Maximum size of any packet we expect.  Using this makes it so we don't need to
// allocated and deallocated teh same buffer over and over again and the price
// of using a bit of extra memory. It also makes the code simpler.

#ifdef WIN32
const int MSG_DONTWAIT = 0;
#else
typedef int SOCKET;
#endif
const int SOCKET_TIMEOUT_SEC = 8;
class Network
{
public:
    static Network &getSingleton()
    {
        static Network Singleton; return Singleton;
    }

    typedef struct command_buffer_read
    {
        struct command_buffer_read *next;
        int len;
        Uint8 *data;
    }
    _command_buffer_read;

    bool Init();
    static _command_buffer_read *read_cmd_start, *read_cmd_end;

    _command_buffer_read *get_read_cmd(void);
    void send_command_binary(int cmd, const char *body, int len);
    static int  send_socklist(SockList msg);
    void free_read_cmd(_command_buffer_read *cmd);
    void clear_read_cmd_queue(void);
    void socket_thread_start(void);
    void socket_thread_stop(void);
    void setActiveServer(int nr)
    {
        mActServerNr = nr;
    }
    bool SOCKET_InitSocket(void);
    bool SOCKET_DeinitSocket(void);
    bool SOCKET_OpenSocket(const char *host, int port);
    bool SOCKET_OpenClientSocket(const char *host, int port);
    bool OpenActiveServerSocket()
    {
        return SOCKET_OpenClientSocket(mvServer[mActServerNr]->ip.c_str(), mvServer[mActServerNr]->port);
    }
    static bool SOCKET_CloseSocket();
    int  SOCKET_GetError(void);  // returns socket error
    void read_metaserver_data(SOCKET fd);
    void update();
    void contactMetaserver();
    static bool SOCKET_CloseClientSocket();
    static int socket_thread_loop(void *);
    static void write_socket_buffer(int fd, SockList *sl);
    static int read_socket_buffer(int fd, SockList *sl);
    static int cs_write_string(char *buf, int len);
    void SendVersion();
    void add_metaserver_data(const char *ip, const char *server, int port, int player, const char *ver,
                             const char *desc1, const char *desc2, const char *desc3, const char *desc4);
    static bool GameStatusVersionOKFlag;
    static bool GameStatusVersionFlag;

    // Commands
    static void CompleteCmd(unsigned char *data, int len);
    static void VersionCmd(char *data, int len);
    static void DrawInfoCmd(char *data, int len);
    static void AddMeFail(char *data, int len);
    static void Map2Cmd(unsigned char *data, int len);
    static void DrawInfoCmd2(char *data, int len);
    static void ItemXCmd(unsigned char *data, int len);
    static void SoundCmd(unsigned char *data, int len);
    static void TargetObject(unsigned char *data, int len);
    static void UpdateItemCmd(unsigned char *data, int len);
    static void DeleteItem(unsigned char *data, int len);
    static void StatsCmd(unsigned char *data, int len);
    static void ImageCmd(unsigned char *data, int len);
    static void Face1Cmd(unsigned char *data, int len);
    static void AnimCmd(unsigned char *data, int len);
    static void SkillRdyCmd(char *data, int len);
    static void PlayerCmd(unsigned char *data, int len);
    static void SpelllistCmd(unsigned char *data, int len);
    static void SkilllistCmd(unsigned char *data, int len);
    static void GolemCmd(unsigned char *data, int len);
    static void AddMeSuccess(char *data, int len);
    static void GoodbyeCmd(char *data, int len);
    static void SetupCmd(char *data, int len);
    static void handle_query(char *data, int len);
    static void DataCmd(char *data, int len);
    static void NewCharCmd(char *data, int len);
    static void ItemYCmd(unsigned char *data, int len);
    static void GroupCmd(unsigned char *data, int len);
    static void GroupInviteCmd(unsigned char *data, int len);
    static void GroupUpdateCmd(unsigned char *data, int len);
    static void InterfaceCmd(unsigned char *data, int len);
    static void BookCmd(unsigned char *data, int len);
    static void MarkCmd(unsigned char *data, int len);
    static void MagicMapCmd(unsigned char *data, int len);
    static void DeleteInventory(unsigned char *data, int len);
    // Commands helper.
    static int  request_face(int, int);
    static void CreatePlayerAccount();
    static void PreParseInfoStat(char *cmd);

private:
    typedef struct
    {
        std::string name;
        std::string ip;
        std::string version;
        std::string desc1;
        std::string desc2;
        std::string desc3;
        std::string desc4;
        int player;
        int port;
    }Server;
    std::vector<Server*>mvServer;

    static bool thread_flag;
    static SDL_Thread *socket_thread;
    static SDL_mutex  *read_lock;
    static SDL_mutex  *write_lock;
    static SDL_mutex  *socket_lock;
    static SDL_cond   *socket_cond;
    static ClientSocket csocket;
    static int mRequest_file_chain;
    int SocketStatusErrorNr;
    int mActServerNr;
    struct sockaddr_in  insock;       // Server's attributes

    static bool mInitDone;


    void parse_metaserver_data(string strMetaData);

    Network();
    ~Network();
    Network(const Network&); // disable copy-constructor.
};

#endif
