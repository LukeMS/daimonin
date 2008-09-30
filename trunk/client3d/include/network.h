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
    // flags for send_command_binary()
    enum
    {
        SEND_CMD_FLAG_DYNAMIC = 1 << 0, // Data tail length can vary, add 2 length bytes
        SEND_CMD_FLAG_STRING  = 1 << 1, // add a '\0' to the outbuffer string as sanity set
        SEND_CMD_FLAG_NOSTRING= 1 << 2, // no add of  '\0'
        SEND_CMD_FLAG_FIXED   = 1 << 3, // the the command as fixed, without length tag (server knows length)
    };

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
    typedef struct command_buffer
    {
        struct command_buffer *next; // Next in queue.
        struct command_buffer *prev; // Previous in queue.
        int len;
        unsigned char *data;
    }
    command_buffer;
    static command_buffer *input_queue_start,  *input_queue_end;
    static command_buffer *output_queue_start, *output_queue_end;

    typedef struct
    {
        Ogre::String nameip;
        Ogre::String version;
        Ogre::String desc1;
        Ogre::String desc2;
        Ogre::String desc3;
        Ogre::String desc4;
        int player;
        int port;
    }
    mStructServer;

    // ClientSocket could probably hold more of the global values - it could
    // probably hold most all socket/communication related values instead
    // of globals.
    typedef struct
    {
        int fd;
        Ogre::String inbuf;
        Ogre::String outbuf;
        // These are used for the newer 'windowing' method of commands -
        // number of last command sent, number of received confirmation
        int command_sent, command_received;
        // Time (in ms) players commands currently take to execute
        int command_time;
    }
    ClientSocket;

    static bool GameStatusVersionOKFlag;
    static bool GameStatusVersionFlag;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static Network &getSingleton()
    {
        static Network Singleton; return Singleton;
    }
    bool Init();
    void clearMetaServerData();

    static int   GetInt_String(unsigned char *data);
    static short GetShort_String(unsigned char *data);
    static command_buffer *get_next_input_command();
    static command_buffer *command_buffer_new(unsigned int len, unsigned char *data);
    static command_buffer *command_buffer_dequeue(command_buffer **queue_start, command_buffer **queue_end);
    static void command_buffer_free(command_buffer *buf);
    static void command_buffer_enqueue(command_buffer *buf, command_buffer **queue_start, command_buffer **queue_end);
    static void checkFileStatus(const char *cmd, char *param, int fileNr);
    static void AddIntToString(Ogre::String &sl, int data, bool shortInt);

    static int reader_thread_loop(void *);
    static int writer_thread_loop(void *);

    static int send_command(const char *command, int repeat, int force);
    static int send_command_binary(unsigned char cmd, unsigned char *body, int len, int flags);
    static int send_command_binary(unsigned char cmd, std::stringstream &stream);
    static int send_socklist(Ogre::String msg);
    void socket_thread_start();
    void socket_thread_stop();
    bool isInit()
    {
        return mInitDone;
    }
    void setActiveServer(int nr)
    {
        mActServerNr = nr;
    }
    bool InitSocket();
    bool OpenSocket(const char *host, int port);
    bool OpenClientSocket(const char *host, int port);
    bool OpenActiveServerSocket()
    {
        return OpenClientSocket(mvServer[mActServerNr]->ip.c_str(), mvServer[mActServerNr]->port);
    }
    static void CloseSocket();
    static void CloseClientSocket();
    static void send_reply(const char *text);
    static void cs_write_string(const char *buf);
    static Ogre::String &Network::getError();  // returns socket error
    void read_metaserver_data();
    void handle_socket_shutdown();
    void update();
    void contactMetaserver();
    void add_metaserver_data(const char *ip, const char *server, int port, int player, const char *ver,
                             const char *desc1, const char *desc2, const char *desc3, const char *desc4);
    // Commands
    static void DrawInfoCmd    (unsigned char *data, int len);
    static void DrawInfoCmd2   (unsigned char *data, int len);
    static void AddMeFail      (unsigned char *data, int len);
    static void Map2Cmd        (unsigned char *data, int len);
    static void SoundCmd       (unsigned char *data, int len);
    static void TargetObject   (unsigned char *data, int len);
    static void StatsCmd       (unsigned char *data, int len);
    static void ImageCmd       (unsigned char *data, int len);
    static void Face1Cmd       (unsigned char *data, int len);
    static void AnimCmd        (unsigned char *data, int len);
    static void SkillRdyCmd    (unsigned char *data, int len);
    static void PlayerCmd      (unsigned char *data, int len);
    static void SpelllistCmd   (unsigned char *data, int len);
    static void SkilllistCmd   (unsigned char *data, int len);
    static void GolemCmd       (unsigned char *data, int len);
    static void AccountCmd     (unsigned char *data, int len);
    static void AccNameSuccess (unsigned char *data, int len);
    static void AddMeSuccess   (unsigned char *data, int len);
    static void GoodbyeCmd     (unsigned char *data, int len);
    static void SetupCmd       (unsigned char *data, int len);
    static void handle_query   (unsigned char *data, int len);
    static void DataCmd        (unsigned char *data, int len);
    static void NewCharCmd     (unsigned char *data, int len);
    static void InterfaceCmd   (unsigned char *data, int len);
    static void BookCmd        (unsigned char *data, int len);
    static void MarkCmd        (unsigned char *data, int len);

    static void GroupCmd       (unsigned char *data, int len);
    static void GroupInviteCmd (unsigned char *data, int len);
    static void GroupUpdateCmd (unsigned char *data, int len);

    static void ItemXCmd       (unsigned char *data, int len);
    static void ItemYCmd       (unsigned char *data, int len);
    static void ItemUpdateCmd  (unsigned char *data, int len);
    static void ItemDeleteCmd  (unsigned char *data, int len);

    // Commands helper.
    static int  request_face(int, int);
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

    static bool abort_thread;
    static SDL_Thread *input_thread;
    static SDL_mutex  *input_buffer_mutex;
    static SDL_cond   *input_buffer_cond;
    static SDL_Thread *output_thread;
    static SDL_mutex  *output_buffer_mutex;
    static SDL_cond   *output_buffer_cond;
    static SDL_mutex  *socket_mutex;
    static ClientSocket csocket;
    int SocketStatusErrorNr;
    int mActServerNr;
    struct sockaddr_in  insock;       // Server's attributes
    static bool mInitDone;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    void parse_metaserver_data(Ogre::String strMetaData);
    Network();
    ~Network();
    Network(const Network&); // disable copy-constructor.
};

#endif
