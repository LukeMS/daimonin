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

#include <string>
#include "network.h"
#include "logger.h"
#include "option.h"
#include "define.h"
#include "network_serverfile.h"

#include "tile_manager.h"
#include "gui_manager.h"
#include "option.h"
#define DEBUG_ON

#define DEFAULT_SERVER_PORT 13327
#define DEFAULT_METASERVER_PORT 13326
#define DEFAULT_METASERVER "damn.informatik.uni-bremen.de"
#define PACKAGE_NAME "Daimonin SDL Client"
int SoundStatus=1;

enum
{
    BINARY_CMD_COMC = 1,
    BINARY_CMD_VERSION,
    BINARY_CMD_DRAWINFO,
    BINARY_CMD_ADDME_FAIL,
    BINARY_CMD_MAP2,
    BINARY_CMD_DRAWINFO2,
    BINARY_CMD_ITEMX,
    BINARY_CMD_SOUND,
    BINARY_CMD_TARGET,
    BINARY_CMD_UPITEM,
    BINARY_CMD_DELITEM,
    BINARY_CMD_STATS,
    BINARY_CMD_IMAGE,
    BINARY_CMD_FACE1,
    BINARY_CMD_ANIM,
    BINARY_CMD_SKILLRDY,
    BINARY_CMD_PLAYER,
    BINARY_CMD_SPELL_LIST,
    BINARY_CMD_SKILL_LIST,
    BINARY_CMD_GOLEMCMD,
    BINARY_CMD_ADDME_SUC,
    BINARY_CMD_BYE,
    BINARY_CMD_SETUP,
    BINARY_CMD_QUERY,
    BINARY_CMD_DATA,
    BINARY_CMD_NEW_CHAR,
    BINARY_CMD_ITEMY,
    BINARY_CMD_GROUP,
    BINARY_CMD_INVITE,
    BINARY_CMD_GROUP_UPDATE,
    BINARY_CMD_INTERFACE,
    BINARY_CMD_BOOK,
    BINARY_CMD_MARK,
    BINARY_CMD_SUM
};


bool Network::GameStatusVersionOKFlag = false;
bool Network::GameStatusVersionFlag = false;
bool Network::mInitDone = false;
int Network::mRequest_file_chain =0;

typedef void (*CmdProc)(unsigned char *, int len);

struct CmdMapping
{
    char  *cmdname;
    void (*cmdproc)(unsigned char *, int);
};

struct CmdMapping commands[]  =
    {
        // Order of this table doesn't make a difference.
        // I tried to sort of cluster the related stuff together.
        { "comc",                       Network::CompleteCmd},
        { "version",          (CmdProc) Network::VersionCmd },
        { "drawinfo",         (CmdProc) Network::DrawInfoCmd },
        { "addme_failed",     (CmdProc) Network::AddMeFail },
        { "map2",                       Network::Map2Cmd },
        { "drawinfo2",        (CmdProc) Network::DrawInfoCmd2 },
        { "itemx",                      Network::ItemXCmd },
        { "sound",                      Network::SoundCmd},
        { "to",                         Network::TargetObject },
        { "upditem",                    Network::UpdateItemCmd },
        { "delitem",                    Network::DeleteItem },
        { "stats",                      Network::StatsCmd },
        { "image",                      Network::ImageCmd },
        { "face1",                      Network::Face1Cmd},
        { "anim",                       Network::AnimCmd},
        { "skill_rdy",        (CmdProc) Network::SkillRdyCmd },
        { "player",                     Network::PlayerCmd },
        { "splist",                     Network::SpelllistCmd },
        { "sklist",                     Network::SkilllistCmd },
        { "gc",                         Network::GolemCmd },
        { "addme_success",    (CmdProc) Network::AddMeSuccess },
        { "goodbye",          (CmdProc) Network::GoodbyeCmd },
        { "setup",            (CmdProc) Network::SetupCmd},
        { "query",            (CmdProc) Network::handle_query},
        { "data",             (CmdProc) Network::DataCmd},
        { "new_char",         (CmdProc) Network::NewCharCmd},
        { "itemy",                      Network::ItemYCmd },
        { "group",                      Network::GroupCmd },
        { "group_invite",               Network::GroupInviteCmd },
        { "group_update",               Network::GroupUpdateCmd },
        { "interface",                  Network::InterfaceCmd },
        { "book",                       Network::BookCmd },
        { "mark",                       Network::MarkCmd },
        // unused!
        { "magicmap",                   Network::MagicMapCmd},
        { "delinv",                     Network::DeleteInventory }
    };


#define SOCKET_NO -1

bool Network::thread_flag = false;
//int  Network::mSocket = SOCKET_NO;
ClientSocket Network::csocket;
SDL_Thread *Network::socket_thread =0;
SDL_mutex *Network::read_lock =0;
SDL_mutex *Network::write_lock =0;
SDL_mutex *Network::socket_lock =0;
SDL_cond *Network::socket_cond =0;
Network::_command_buffer_read *Network::read_cmd_end=NULL;
Network::_command_buffer_read *Network::read_cmd_start=NULL;

//SockList Network::mInbuf;
//SockList Network::mOutbuf;

//sockaddr_in mInsock;

// This is really, really a bad implementation.
//This is still weird test code and need a real code solution.
void parse_metaserver_data(string strMetaData)
{
    /// ////////////////////////////////////////////////////////////////////
    /// Parse the metadata.
    /// ////////////////////////////////////////////////////////////////////
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

//    add_metaserver_data(strName.c_str(), mOpenPort, atoi(strPlayer.c_str()), strVersion.c_str(), strDesc1.c_str(),  strDesc2.c_str(),  strDesc3.c_str(),  strDesc4.c_str());

}




#ifndef WIN32
inline static char *strerror_local(int errnum)
{
#if defined(HAVE_STRERROR)
    return(strerror(errnum));
#else
    return("strerror not implemented");
#endif
}
#endif

Network::Network()
{}

Network::~Network()
{}


void Network::update()
{
   if (!mInitDone) return;
    _command_buffer_read *cmd;
    while (1)
    {
        if(!read_cmd_start) // we have a filled command?
            break;
        cmd = get_read_cmd(); // function has mutex included
        if(!cmd) break;

        if (!cmd->data[0] || cmd->data[0] >= BINARY_CMD_SUM)
            Logger::log().error() << "Bad command from server " << cmd->data[0];
        else
        {
            Logger::log().error() << "command #" << commands[cmd->data[0]-1].cmdname << " >" << cmd->data+1 << "<";
            commands[cmd->data[0] - 1].cmdproc(cmd->data+1, cmd->len-1);
        }
        free_read_cmd(cmd);
    }
}


void Network::send_command_binary(int cmd, const char *body, int len)
{
    SDL_LockMutex(write_lock);

    if(csocket.fd == SOCKET_NO || csocket.outbuf.len + 3 > MAXSOCKBUF)
    {
        if(csocket.fd != SOCKET_NO)
            SOCKET_CloseClientSocket();
        SDL_UnlockMutex(write_lock);
        return;
    }
    // adjust the buffer
    if(csocket.outbuf.pos)
        memcpy(csocket.outbuf.buf, csocket.outbuf.buf+csocket.outbuf.pos, csocket.outbuf.len);

    csocket.outbuf.pos=0;
    if(!body)
    {
        len = 0x8001;

        csocket.outbuf.buf[csocket.outbuf.len++] = ((uint32) (len) >> 8) & 0xFF;
        csocket.outbuf.buf[csocket.outbuf.len++] = ((uint32) (len)) & 0xFF;
        csocket.outbuf.buf[csocket.outbuf.len++] = (unsigned char) cmd;
    }
    SDL_UnlockMutex(write_lock);
}

// move a command/buffer to the out buffer so it can be written to the socket
int Network::send_socklist(SockList msg)
{
    SDL_LockMutex(write_lock);
    if(csocket.fd == SOCKET_NO || csocket.outbuf.len + msg.len > MAXSOCKBUF)
    {
        if(csocket.fd != SOCKET_NO)
            SOCKET_CloseClientSocket();
        SDL_UnlockMutex(write_lock);
        return -1;
    }
    // adjust the buffer
    if(csocket.outbuf.pos && csocket.outbuf.len)
        memcpy(csocket.outbuf.buf, csocket.outbuf.buf+csocket.outbuf.pos, csocket.outbuf.len);

    csocket.outbuf.pos=0;
    csocket.outbuf.buf[csocket.outbuf.len++] = (uint8) ((msg.len >> 8) & 0xFF);
    csocket.outbuf.buf[csocket.outbuf.len++] = ((uint32) (msg.len)) & 0xFF;
    memcpy(csocket.outbuf.buf+csocket.outbuf.len, msg.buf, msg.len);
    csocket.outbuf.len += msg.len;
    SDL_UnlockMutex(write_lock);
    return 0;
}


// get a read command from the queue.
// remove it from queue and return a pointer to it.
// return NULL if there is no command
Network::_command_buffer_read *Network::get_read_cmd(void)
{
    _command_buffer_read *tmp;

    SDL_LockMutex(read_lock);

    if(!read_cmd_start)
    {
        SDL_UnlockMutex(read_lock);
        return NULL;
    }

    tmp = read_cmd_start;
    read_cmd_start = tmp->next;
    if(read_cmd_end == tmp)
        read_cmd_end = NULL;

    SDL_UnlockMutex(read_lock);

    return tmp;
}

// free a read cmd buffer struct and its data tail
void Network::free_read_cmd(_command_buffer_read *cmd)
{
    delete[] cmd->data;
    delete cmd;
}

// clear & free the whole read cmd queue
void Network::clear_read_cmd_queue(void)
{
    SDL_LockMutex(read_lock);

    while(read_cmd_start)
        free_read_cmd(get_read_cmd());

    SDL_UnlockMutex(read_lock);
}

// write stuff to the socket
inline void Network::write_socket_buffer(int fd, SockList *sl)
{
    if (sl->len == 0)
        return;
    int amt;
    SDL_LockMutex(write_lock);
    amt = send(fd, (const char*)sl->buf + sl->pos, sl->len, MSG_DONTWAIT);

    // following this link: http://www-128.ibm.com/developerworks/linux/library/l-sockpit/#N1019D
    // send() with MSG_DONTWAIT under linux can return 0 which means the data
    // is "queued for transmission". I was not able to find that in the send() man pages...
    // In my testings it never happend, so i put it here in to have it perhaps triggered in
    // some server runs (but we should trust perhaps ibm developer infos...).

    if(!amt)
        amt = sl->len; // as i understand, the data is now internal buffered? So remove it from our write buffer

    if (amt > 0)
    {
        sl->pos += amt;
        sl->len -= amt;
    }
    SDL_UnlockMutex(write_lock);

    if (amt < 0) // error
    {
#ifdef WIN32 // ***win32 write_socket_buffer: change error handling
        if (WSAGetLastError() == WSAEWOULDBLOCK)
            return;
        Logger::log().error() << "New socket write failed (wsb) " << WSAGetLastError();
        SOCKET_CloseClientSocket();
#else
        if (errno == EWOULDBLOCK || errno == EINTR)
            return;
        Logger::log().error() << "New socket write failed (wsb " << EAGAIN << ") (" << errno << ": " << strerror_local(errno) << ")";
        SOCKET_CloseClientSocket();
#endif
        return;
    }

}

// read stuff from socket
inline int Network::read_socket_buffer(int fd, SockList *sl)
{
    int         stat_ret, read_bytes, tmp;

    // calculate how many bytes can be read in one row in our round robin buffer
    tmp = sl->pos+sl->len;

    // we have still some bytes until we hit our buffer border ?
    if(tmp >= MAXSOCKBUF)
    {
        tmp = tmp-MAXSOCKBUF; // thats our start offset
        read_bytes = sl->pos - tmp; // thats our free buffer until ->pos
    }
    else
        read_bytes = MAXSOCKBUF-tmp; // tmp is our offset and there is still a bit to read in

    stat_ret = recv(fd, (char*)sl->buf + tmp, read_bytes, MSG_DONTWAIT);

    if (stat_ret > 0)
        sl->len += stat_ret;
    else if (stat_ret < 0) // lets check its a real problem
    {
#ifdef WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK)
            return 1;

        if (WSAGetLastError() == WSAECONNRESET)
            Logger::log().error() << "Connection closed by server\n";
        else
            Logger::log().error() << "ReadPacket got error " << WSAGetLastError() << " returning 0";
#else
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return 1;

        Logger::log().error() << "ReadPacket got error " << errno << ": " << strerror_local(errno) << "returning 0";
#endif
        SOCKET_CloseClientSocket();
    }
    else
        SOCKET_CloseClientSocket();

    return stat_ret;
}

int Network::socket_thread_loop(void *)
{
    while(thread_flag)
    {
        // Want a valid socket for the IO loop
        SDL_LockMutex(socket_lock);
        if(csocket.fd == SOCKET_NO)
        {
            SDL_CondWait(socket_cond, socket_lock);
            if(!thread_flag)
            {
                SDL_UnlockMutex(socket_lock);
                break;
            }
        }

        if(csocket.fd == SOCKET_NO && Option::getSingleton().getGameStatus() >= GAME_STATUS_STARTCONNECT)
        {
            SDL_Delay(150);
            SDL_UnlockMutex(socket_lock);
            continue;
        }

        if(csocket.fd != SOCKET_NO && Option::getSingleton().getGameStatus() >= GAME_STATUS_STARTCONNECT)
            read_socket_buffer(csocket.fd, &csocket.inbuf);

        // lets check we have a valid command
        while(csocket.inbuf.len >= 2)
        {
            _command_buffer_read *tmp;
            int head_off=2, toread = -1, pos = csocket.inbuf.pos;

            if(csocket.inbuf.buf[pos] & 0x80) // 3 byte length heasder?
            {
                if(csocket.inbuf.len > 2)
                {
                    head_off = 3;

                    toread = ((csocket.inbuf.buf[pos]&0x7f) << 16);
                    if(++pos >= MAXSOCKBUF)
                        pos -= MAXSOCKBUF;
                    toread += (csocket.inbuf.buf[pos] << 8);
                    if(++pos >= MAXSOCKBUF)
                        pos -= MAXSOCKBUF;
                    toread += csocket.inbuf.buf[pos];
                }

            }
            else // 2 size length header
            {
                toread = (csocket.inbuf.buf[pos] << 8);
                if(++pos >= MAXSOCKBUF)
                    pos -= MAXSOCKBUF;
                toread += csocket.inbuf.buf[pos];
            }

            // adjust pos to data start
            if(++pos >= MAXSOCKBUF)
                pos -= MAXSOCKBUF;

            // leave collecting commands when we hit an incomplete one
            if(toread == -1 || csocket.inbuf.len < toread+head_off)
            {
                SDL_UnlockMutex(socket_lock);
                break;
            }

            tmp = new _command_buffer_read;
            tmp->data = new uint8[toread + 1];
            tmp->len = toread;
            tmp->next = NULL;

            if(pos + toread > MAXSOCKBUF) // splitted data tail?
            {
                int tmp_read, read_part;

                read_part = (pos + toread) - MAXSOCKBUF;
                tmp_read = toread - read_part;
                memcpy(tmp->data, csocket.inbuf.buf+pos, tmp_read);
                memcpy(tmp->data+tmp_read, csocket.inbuf.buf, read_part);
                csocket.inbuf.pos = read_part;
            }
            else
            {
                memcpy(tmp->data, csocket.inbuf.buf+pos, toread);
                csocket.inbuf.pos = pos + toread;
            }
            tmp->data[tmp->len] = 0; // ensure we have a zero at the end - simple buffer overflow proection
            csocket.inbuf.len -= toread + head_off;

            SDL_LockMutex(read_lock);
            // put tmp to the end of our read cmd queue
            if(!read_cmd_start)
                read_cmd_start = tmp;
            else
                read_cmd_end->next = tmp;
            read_cmd_end = tmp;
            SDL_UnlockMutex(read_lock);
        }

        if(csocket.fd != SOCKET_NO && Option::getSingleton().getGameStatus() >= GAME_STATUS_STARTCONNECT)
            write_socket_buffer(csocket.fd, &csocket.outbuf);

        SDL_UnlockMutex(socket_lock);

    }

    return 0;
}

void Network::socket_thread_start()
{
    Logger::log().info() << "START THREAD";
    thread_flag = true;

    socket_cond = SDL_CreateCond();
    socket_lock = SDL_CreateMutex();
    read_lock = SDL_CreateMutex();
    write_lock = SDL_CreateMutex();

    socket_thread = SDL_CreateThread(socket_thread_loop, NULL);
    if ( socket_thread == NULL )
        Logger::log().error() <<  "Unable to start socket thread: " << SDL_GetError();
}


void Network::socket_thread_stop(void)
{
    Logger::log().info() << "STOP THREAD";

    if(thread_flag)
    {
        thread_flag = false;
        SDL_CondSignal(socket_cond);
        SDL_WaitThread(socket_thread, NULL);

        SDL_DestroyCond(socket_cond);
        SDL_DestroyMutex(socket_lock);
        SDL_DestroyMutex(read_lock);
        SDL_DestroyMutex(write_lock);
    }
}


int Network::SOCKET_GetError()
{
#ifdef WIN32
    return(WSAGetLastError());
#else
    return errno;
#endif
}

bool Network::SOCKET_CloseSocket()
{
    if (csocket.fd == SOCKET_NO)
        return(true);
#ifdef WIN32
    closesocket(csocket.fd);
#else
    close(csocket.fd);
#endif
    return(true);
}

bool Network::SOCKET_CloseClientSocket()
{
    if (csocket.fd == SOCKET_NO)
        return true;

    Logger::log().info() << "CloseClientSocket()";

    // No more socket for the IO thread
    SDL_LockMutex(socket_lock);

    delete[] csocket.inbuf.buf;
    delete[] csocket.outbuf.buf;
    csocket.inbuf.buf = csocket.outbuf.buf = NULL;
    csocket.inbuf.len = 0;
    csocket.outbuf.len = 0;
    csocket.inbuf.pos = 0;
    csocket.outbuf.pos = 0;
    csocket.fd = SOCKET_NO;

    SDL_CondSignal(socket_cond);
    SDL_UnlockMutex(socket_lock);
    return(true);
}


bool Network::Init()
{
    if (mInitDone) return true;
#ifdef WIN32
    WSADATA w;
    WORD wVersionRequested = MAKEWORD( 2, 2 );
    int     error;

    csocket.fd = SOCKET_NO;
    csocket.cs_version = 0;

    SocketStatusErrorNr = 0;
    error = WSAStartup(wVersionRequested, &w);
    if (error)
    {
        wVersionRequested = MAKEWORD( 2, 0 );
        error = WSAStartup(wVersionRequested, &w);
        if (error)
        {
            wVersionRequested = MAKEWORD( 1, 1 );
            error = WSAStartup(wVersionRequested, &w);
            if (error)
            {
                Logger::log().error() << "Error init starting Winsock: "<< error;
                return(false);
            }
        }
    }
    Logger::log().info() <<  "Using socket version " << w.wVersion;
#endif
    mInitDone = true;
    socket_thread_start();
    return true;
}


bool Network::SOCKET_DeinitSocket()
{
    if(csocket.fd != SOCKET_NO)
        SOCKET_CloseClientSocket();

#ifdef WIN32
    WSACleanup();
#endif

    return(true);
}

bool Network::SOCKET_OpenClientSocket(char *host, int port)
{
    int tmp = 1;

    // No more socket for the IO thread
    SDL_LockMutex(socket_lock);

    if(! SOCKET_OpenSocket(host, port))
        return false;

    csocket.inbuf.buf = new unsigned char[MAXSOCKBUF];
    csocket.inbuf.len = 0;
    csocket.inbuf.pos = 0;
    csocket.outbuf.buf = new unsigned char[MAXSOCKBUF];
    csocket.outbuf.len = 0;
    csocket.outbuf.pos = 0;

    csocket.command_sent = 0;
    csocket.command_received = 0;
    csocket.command_time = 0;

    if (setsockopt(csocket.fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
    {
        Logger::log().error() << "setsockopt(TCP_NODELAY) failed";
    }

    // socket available for socket thread
    SDL_CondSignal(socket_cond);
    SDL_UnlockMutex(socket_lock);

    return true;
}

// we used our core connect routine to connect to metaserver, this is the special read one.
void Network::read_metaserver_data(SOCKET fd)
{
    int     stat, temp;
    char *ptr = new char[MAX_METASTRING_BUFFER];
    char *buf = new char[MAX_METASTRING_BUFFER];
    temp = 0;
    for (; ;)
    {
#ifdef WIN32
        stat = recv(fd, ptr, MAX_METASTRING_BUFFER, 0);
        if ((stat == -1) && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            Logger::log().error() << "Error reading metaserver data!: "<< WSAGetLastError();
            break;
        }
        else
#else
        do
        {
            stat = recv(fd, ptr, MAX_METASTRING_BUFFER, 0);
        }
        while (stat == -1);
#endif
        if (stat > 0)
        {
            if (temp + stat >= MAX_METASTRING_BUFFER)
            {
                memcpy(buf + temp, ptr, temp + stat - MAX_METASTRING_BUFFER - 1);
                temp += stat;
                break;
            }
            memcpy(buf + temp, ptr, stat);
            temp += stat;
        }
        else if (stat == 0)
        {
            // connect closed by meta
            break;
        }
    }
    buf[temp] = 0;
    parse_metaserver_data(buf);
    delete[] ptr;
    delete[] buf;
}

#ifdef WIN32
bool Network::SOCKET_OpenSocket(char *host, int port)
{
    int             error;
    long            temp;
    struct hostent *hostbn;
    int             oldbufsize;
    int             newbufsize = 65535, buflen = sizeof(int);
    uint32          start_timer;
    struct linger   linger_opt;

    Logger::log().info() <<  "OpenSocket: "<< host;
    // The way to make the sockets work on XP Home - The 'unix' style socket seems to fail under xp home.

    csocket.fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    insock.sin_family = AF_INET;
    insock.sin_port = htons((unsigned short) port);

    if (isdigit(*host))
        insock.sin_addr.s_addr = inet_addr(host);
    else
    {
        hostbn = gethostbyname(host);
        if (hostbn == (struct hostent *) NULL)
        {
            Logger::log().warning() <<  "Unknown host: "<< host;
            csocket.fd = SOCKET_NO;
            return(false);
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }

    temp = 1;   // non-block

    if (ioctlsocket(csocket.fd, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << "ioctlsocket(*socket_temp, FIONBIO , &temp)";
        csocket.fd = SOCKET_NO;
        return(false);
    }

    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 5;
    if (setsockopt(csocket.fd, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
        Logger::log().error() << "BUG: Error on setsockopt LINGER";

    error = 0;

    start_timer = SDL_GetTicks();
    while (connect(csocket.fd, (struct sockaddr *) &insock, sizeof(insock)) == SOCKET_ERROR)
    {
        SDL_Delay(3);

        // timeout.... without connect will REALLY hang a long time
        if (start_timer + SOCKET_TIMEOUT_SEC * 1000 < SDL_GetTicks())
        {
            csocket.fd = SOCKET_NO;
            return(false);
        }

        SocketStatusErrorNr = WSAGetLastError();
        if (SocketStatusErrorNr == WSAEISCONN)  // we have a connect!
            break;

        if (SocketStatusErrorNr == WSAEWOULDBLOCK
                || SocketStatusErrorNr == WSAEALREADY
                || (SocketStatusErrorNr == WSAEINVAL && error)) // loop until we finished
        {
            error = 1;
            continue;
        }

        Logger::log().warning() <<  "Connect Error: " << SocketStatusErrorNr;
        csocket.fd = SOCKET_NO;
        return(false);
    }
    // we got a connect here!

    if (getsockopt(csocket.fd, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;

    if (oldbufsize < newbufsize)
    {
        if (setsockopt(csocket.fd, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            setsockopt(csocket.fd, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
        }
    }

    Logger::log().info() <<  "Connected to "<< host << "  " <<  port;
    return(true);
}

#else
bool Network::SOCKET_OpenSocket(char *host, int port)
{
    unsigned int  oldbufsize, newbufsize = 65535, buflen = sizeof(int);
    struct linger       linger_opt;

    // Use new (getaddrinfo()) or old (gethostbyname()) socket API
#if 1 // small hack until we make it configurable to fix mantis 0000425
    //#ifndef HAVE_GETADDRINFO
    struct protoent *protox;
    struct sockaddr_in  insock;

    Logger::log().info() << "Opening to " << host << " " << port;
    protox = getprotobyname("tcp");

    if (protox == (struct protoent *) NULL)
    {
        Logger::log().error() << "Error on getting prorobyname (tcp)";
        return false;
    }
    csocket.fd = socket(PF_INET, SOCK_STREAM, protox->p_proto);

    if (csocket.fd == -1)
    {
        Logger::log().error() << "init_connection:  Error on socket command.";
        csocket.fd = SOCKET_NO;
        return false;
    }
    insock.sin_family = AF_INET;
    insock.sin_port = htons((unsigned short) port);
    if (isdigit(*host))
        insock.sin_addr.s_addr = inet_addr(host);
    else
    {
        struct hostent *hostbn  = gethostbyname(host);
        if (hostbn == (struct hostent *) NULL)
        {
            Logger::log().error() << "Unknown host: " << host;
            return false;
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }

    if (connect(csocket.fd, (struct sockaddr *) &insock, sizeof(insock)) == (-1))
    {
        perror("Can't connect to server");
        return false;
    }
#else
struct addrinfo hints;
struct addrinfo *res = NULL, *ai;
char port_str[6], hostaddr[40];

Logger::log().info() << "Opening to "<< host << " " << port;
snprintf(port_str, sizeof(port_str), "%d", port);
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
// Workaround for issue #425 on OSs with broken NIS+ like FC5.
// This should disable any service lookup
hints.ai_flags = AI_NUMERICSERV;

if (getaddrinfo(host, port_str, &hints, &res) != 0)
    return false;

for (ai = res; ai != NULL; ai = ai->ai_next)
{
    getnameinfo(ai->ai_addr, ai->ai_addrlen, hostaddr, sizeof(hostaddr), NULL, 0, NI_NUMERICHOST);
    Logger::log().info() << "  trying " << hostaddr;

    csocket.fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (csocket.fd == -1)
    {
        csocket.fd = SOCKET_NO;
        continue;
    }

    if (connect(csocket.fd, ai->ai_addr, ai->ai_addrlen) != 0)
    {
        close(*socket_temp);
        csocket.fd = SOCKET_NO;
        continue;
    }

    break;
}

freeaddrinfo(res);
if (csocket.fd == SOCKET_NO)
{
    Logger::log().error() << "Can't connect to server";
    return false;
}
#endif

    if (fcntl(csocket.fd, F_SETFL, fcntl(csocket.fd, F_GETFL) | O_NONBLOCK ) == -1)
    {
        Logger::log().error() << "socket:  Error on fcntl " << fcntl(csocket.fd, F_GETFL);
        csocket.fd = SOCKET_NO;
        return(false);
    }

    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 5;
    if (setsockopt(csocket.fd, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
        Logger::log().error() <<  "BUG: Error on setsockopt LINGER";

    if (getsockopt(csocket.fd, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;

    if (oldbufsize < newbufsize)
    {
        if (setsockopt(csocket.fd, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            Logger::log().error() << "socket: setsockopt unable to set output buf size to " << newbufsize;
            setsockopt(csocket.fd, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
        }
    }
    return true;
}

#endif


void Network::contactMetaserver()
{
    csocket.fd = SOCKET_NO;
    char buf[256];
    GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"query metaserver...");
    sprintf(buf, "trying %s:%d", DEFAULT_METASERVER, DEFAULT_METASERVER_PORT);
    GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)buf);
    if (SOCKET_OpenSocket(DEFAULT_METASERVER, DEFAULT_METASERVER_PORT))
    {
        read_metaserver_data(csocket.fd);
        SOCKET_CloseSocket();
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"done.");
    }
    else
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"metaserver failed! using default list.");

//    add_metaserver_data("127.0.0.1", DEFAULT_SERVER_PORT, -1, "local", "localhost. Start server before you try to connect.", "", "", "");
//    count_meta_server();
    GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)"select a server.");

}

void Network::SendVersion()
{
    char buf[MAX_BUF];
    sprintf(buf, "version %d %d %s", VERSION_CS, VERSION_SC, PACKAGE_NAME);
    Logger::log().error() << "Send version command: " << buf;
    cs_write_string(buf, (int)strlen(buf));
}

int Network::cs_write_string(char *buf, int len)
{
    SockList sl;
    sl.len = len;
    sl.buf = (unsigned char *) buf;
    return send_socklist(sl);
}
