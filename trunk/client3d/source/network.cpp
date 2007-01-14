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
#define CLIENT_PACKAGE_NAME "Daimonin SDL Client"

int SoundStatus=1;

bool Network::GameStatusVersionOKFlag = false;
bool Network::GameStatusVersionFlag = false;
bool Network::mInitDone = false;

struct CmdMapping
{
    void (*serverCmd)(unsigned char *, int len);
};
struct CmdMapping  commands[]  =
    {    // Don't change this sorting! Its hardcoded in the server.
        { Network::CompleteCmd},     //  0
        { Network::VersionCmd },     //  1
        { Network::DrawInfoCmd },    //  2
        { Network::AddMeFail },      //  3
        { Network::Map2Cmd },        //  4
        { Network::DrawInfoCmd2 },   //  5
        { Network::ItemXCmd },       //  6
        { Network::SoundCmd},        //  7
        { Network::TargetObject },   //  8
        { Network::ItemUpdateCmd },  //  9
        { Network::ItemDeleteCmd },  // 10
        { Network::StatsCmd },       // 11
        { Network::ImageCmd },       // 12
        { Network::Face1Cmd},        // 13
        { Network::AnimCmd},         // 14
        { Network::SkillRdyCmd },    // 15
        { Network::PlayerCmd },      // 16
        { Network::SpelllistCmd },   // 17
        { Network::SkilllistCmd },   // 18
        { Network::GolemCmd },       // 19
        { Network::AddMeSuccess },   // 20
        { Network::GoodbyeCmd },     // 21
        { Network::SetupCmd},        // 22
        { Network::handle_query},    // 23
        { Network::DataCmd},         // 24
        { Network::NewCharCmd},      // 25
        { Network::ItemYCmd },       // 26
        { Network::GroupCmd },       // 27
        { Network::GroupInviteCmd }, // 28
        { Network::GroupUpdateCmd }, // 29
        { Network::InterfaceCmd },   // 30
        { Network::BookCmd },        // 31
        { Network::MarkCmd },        // 32
    };
const int SUM_SERVER_COMMANDS = sizeof(commands) / sizeof(CmdMapping);

Network::ClientSocket Network::csocket;
bool Network::abort_thread = false;
SDL_Thread *Network::input_thread=0;
SDL_mutex  *Network::input_buffer_mutex =0;
SDL_cond   *Network::input_buffer_cond =0;
SDL_Thread *Network::output_thread=0;
SDL_mutex  *Network::output_buffer_mutex =0;
SDL_cond   *Network::output_buffer_cond =0;
SDL_mutex  *Network::socket_mutex =0;

// start is the first waiting item in queue, end is the most recent enqueued
Network::command_buffer *Network::input_queue_start = 0, *Network::input_queue_end = 0;
Network::command_buffer *Network::output_queue_start= 0, *Network::output_queue_end= 0;

const int TIMEOUT_MS = 4000;
const int NO_SOCKET = -1;

// Maximum size of any packet we expect.  Using this makes it so we don't need to
// allocated and deallocated the same buffer over and over again and the price
// of using a bit of extra memory. IT also makes the code simpler.
const int  MAXSOCKBUF            = 128*1024;
const int  MAX_METASTRING_BUFFER = 128*2013;
const int  MAX_BUF =  256;
const int  BIG_BUF = 1024;
const int  STRINGCOMMAND = 0;
const int  SRV_CLIENT_FLAG_BMAP    = 1 << 0;
const int  SRV_CLIENT_FLAG_ANIM    = 1 << 1;
const int  SRV_CLIENT_FLAG_SETTING = 1 << 2;
const int  SRV_CLIENT_FLAG_SKILL   = 1 << 3;
const int  SRV_CLIENT_FLAG_SPELL   = 1 << 4;
const int  MAXMETAWINDOW  = 14; // max. shown server in meta window.
const char VERSION_NAME[] = "Daimonin SDL Client";

// Maximum size of any packet we expect.  Using this makes it so we don't need to
// allocated and deallocated teh same buffer over and over again and the price
// of using a bit of extra memory. It also makes the code simpler.

#ifdef WIN32
const int MSG_DONTWAIT = 0;
#else
typedef int SOCKET;
#endif



//================================================================================================
//
//================================================================================================
void Network::AddIntToString(std::string &sl, int data, bool shortInt)
{
    if (!shortInt)
    {
        sl += (data >> 24) & 0xff;
        sl += (data >> 16) & 0xff;
    }
    sl += (data >>  8) & 0xff;
    sl += (data      ) & 0xff;
}

//================================================================================================
//
//================================================================================================
inline static char *strerror_local(int errnum)
{
#if defined(HAVE_STRERROR)
    return(strerror(errnum));
#else
    return("strerror not implemented");
#endif
}

//================================================================================================
//
//================================================================================================
int Network::GetError()
{
#ifdef WIN32
    return(WSAGetLastError());
#else
    return errno;
#endif
}

//================================================================================================
// .
//================================================================================================
Network::Network()
{}

//================================================================================================
// .
//================================================================================================
Network::~Network()
{
    CloseClientSocket();
#ifdef WIN32
    WSACleanup();
#endif
    handle_socket_shutdown();
    clearMetaServerData();
}

//================================================================================================
// .
//================================================================================================
void Network::clearMetaServerData()
{
    GuiManager::getSingleton().clearTable(GuiManager::GUI_WIN_SERVERSELECT, GuiImageset::GUI_TABLE);
    for (vector<Server*>::iterator i = mvServer.begin(); i != mvServer.end(); ++i)
        delete (*i);
    mvServer.clear();
}

//================================================================================================
//
//================================================================================================
bool Network::Init()
{
    if (mInitDone) return true;
    csocket.fd = NO_SOCKET;
    csocket.cs_version = 0;
    SocketStatusErrorNr= 0;

    Logger::log().headline("Starting Network");
#ifdef WIN32
    WSADATA w;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &w))
    {
        wVersionRequested = MAKEWORD(2, 0);
        if (WSAStartup(wVersionRequested, &w))
        {
            wVersionRequested = MAKEWORD(1, 1);
            int error = WSAStartup(wVersionRequested, &w);
            if (error)
            {
                Logger::log().error() << "Error init starting Winsock: "<< error;
                return false;
            }
        }
    }
    Logger::log().info() <<  "Using socket version " << w.wVersion;
#endif
    mInitDone = true;
    return true;
}

//================================================================================================
//
//================================================================================================
void Network::socket_thread_start()
{
    if (!input_buffer_cond)
    {
        input_buffer_cond  = SDL_CreateCond();
        input_buffer_mutex = SDL_CreateMutex();
        output_buffer_cond = SDL_CreateCond();
        output_buffer_mutex= SDL_CreateMutex();
        socket_mutex = SDL_CreateMutex();
    }
    abort_thread = false;
    input_thread = SDL_CreateThread(reader_thread_loop, NULL);
    if (!input_thread)
        Logger::log().error() <<  "Unable to start socket thread: " << SDL_GetError();

    output_thread = SDL_CreateThread(writer_thread_loop, NULL);
    if (!output_thread)
        Logger::log().error() <<  "Unable to start socket thread: " << SDL_GetError();
}

//================================================================================================
// .
//================================================================================================
void Network::update()
{
    if (!mInitDone) return;
    command_buffer *cmd;
    // Handle all enqueued commands.
    while ((cmd = get_next_input_command()))
    {
        Logger::log().error() << "network cmd: " << cmd->data[0] - 1 << "  " << cmd->data[0];

        if (!cmd->data[0] || cmd->data[0] >= SUM_SERVER_COMMANDS)
            Logger::log().error() << "Bad command from server " << cmd->data[0];
        else
            commands[cmd->data[0] - 1].serverCmd(cmd->data+1, cmd->len-1);
        command_buffer_free(cmd);
    }
}

//================================================================================================
// The main thread should poll this function which detects connection shutdowns and
// removes the threads if it happens.
//================================================================================================
bool Network::handle_socket_shutdown()
{
    socket_thread_stop();
    // Empty all queues.
    while (input_queue_start)
        command_buffer_free(command_buffer_dequeue(&input_queue_start, &input_queue_end));
    while (output_queue_start)
        command_buffer_free(command_buffer_dequeue(&output_queue_start, &output_queue_end));
    Logger::log().info() << "Connection lost";
    return true;
}

//================================================================================================
// .
//================================================================================================
Network::command_buffer *Network::command_buffer_new(unsigned int len, unsigned char *data)
{
    command_buffer *buf = new command_buffer;
    buf->next = buf->prev = NULL;
    buf->len = len;
    buf->data = new unsigned char[len+1];
    if (data) memcpy(buf->data, data, len);
    buf->data[len] = 0; // Buffer overflow sentinel.
    return buf;
}

//================================================================================================
// .
//================================================================================================
void Network::command_buffer_enqueue(command_buffer *buf, command_buffer **queue_start, command_buffer **queue_end)
{
    buf->next = 0;
    buf->prev = *queue_end;
    if (*queue_start == NULL)
        *queue_start = buf;
    if (buf->prev)
        buf->prev->next = buf;
    *queue_end = buf;
}

//================================================================================================
// .
//================================================================================================
Network::command_buffer *Network::command_buffer_dequeue(command_buffer **queue_start, command_buffer **queue_end)
{
    command_buffer *buf = *queue_start;
    if (buf)
    {
        *queue_start = buf->next;
        if (buf->next)
            buf->next->prev = NULL;
        else
            *queue_end = NULL;
    }
    return buf;
}

//================================================================================================
// .
//================================================================================================
void Network::command_buffer_free(command_buffer *buf)
{
    delete[] buf->data;
    delete buf;
}


/* High-level external interface */

//================================================================================================
// This should be used for all 'command' processing.  Other functions should call this so that
// proper windowing will be done.
// command is the text command, repeat is a count value, or -1 if none is desired and we don't
// want to reset the current count.
// force means we must send this command no matter what (ie, it is an administrative type
// of command like fire_stop, and failure to send it will cause definate problems.
// return 1 if command was sent, 0 if not sent.
//================================================================================================
int Network::send_command(const char *command, int repeat, int force)
{
    int commdiff = csocket.command_sent - csocket.command_received;
    if (commdiff < 0)
        commdiff += 256;
    ++csocket.command_sent &= 0xff; // max out at 255.

    std::string sl = "ncom ";
    AddIntToString(sl, csocket.command_sent, true);
    AddIntToString(sl, repeat, false);
    sl += command;
    send_socklist(sl);
    return 1;
}

//================================================================================================
// Add a binary command to the output buffer.
// If body is NULL, a single-byte command is created from cmd.
// Otherwise body should include the length and cmd header
//================================================================================================
int Network::send_command_binary(unsigned char cmd, unsigned char *body, unsigned int len)
{
    command_buffer *buf;

    if (body)
        buf = command_buffer_new(len, (unsigned char*)body);
    else
    {
        unsigned char tmp[3];
        len = 0x8001;
        // Packet order is obviously big-endian for length data.
        tmp[0] = (len >> 8) & 0xFF;
        tmp[1] = len & 0xFF;
        tmp[2] = cmd;
        buf = command_buffer_new(3, tmp);
    }
    SDL_LockMutex(output_buffer_mutex);
    command_buffer_enqueue(buf, &output_queue_start, &output_queue_end);
    SDL_CondSignal(output_buffer_cond);
    SDL_UnlockMutex(output_buffer_mutex);
    return 0;
}

//================================================================================================
// move a command/buffer to the out buffer so it can be written to the socket.
//================================================================================================
int Network::send_socklist(std::string msg)
{
    command_buffer *buf = command_buffer_new((int)msg.size() + 2, 0);
    memcpy(buf->data + 2, msg.c_str(), msg.size());
    buf->data[0] = (unsigned char) ((msg.size() >> 8) & 0xFF);
    buf->data[1] = ((uint32) (msg.size())) & 0xFF;
    SDL_LockMutex(output_buffer_mutex);
    command_buffer_enqueue(buf, &output_queue_start, &output_queue_end);
    SDL_CondSignal(output_buffer_cond);
    SDL_UnlockMutex(output_buffer_mutex);
    return 0;
}

//================================================================================================
// get a read command from the queue. remove it from queue and return a pointer to it.
// return NULL if there is no command
//================================================================================================
Network::command_buffer *Network::get_next_input_command()
{
    SDL_LockMutex(input_buffer_mutex);
    command_buffer *buf = command_buffer_dequeue(&input_queue_start, &input_queue_end);
    SDL_UnlockMutex(input_buffer_mutex);
    return buf;
}


/*
 * Lowlevel socket IO
 */

//================================================================================================
//
//================================================================================================
int Network::reader_thread_loop(void *)
{
    unsigned char readbuf[MAXSOCKBUF+1];
    int readbuf_len = 0;
    int header_len = 0;
    int cmd_len = -1;
    int ret;
    int toread;
    Logger::log().info() << "Reader thread started  ";

    while (!abort_thread)
    {
        // First, try to read a command length sequence.
        if (readbuf_len < 2)
        {
            if (readbuf_len > 0 && (readbuf[0] & 0x80)) // three-byte length?
                toread = 3 - readbuf_len;
            else
                toread = 2 - readbuf_len;
        }
        else if (readbuf_len == 2 && (readbuf[0] & 0x80))
            toread = 1;
        else
        {
            // If we have a finished header, get the packet size from it.
            if (readbuf_len <= 3)
            {
                unsigned char *p = readbuf;
                header_len = (*p & 0x80) ? 3 : 2;
                cmd_len = 0;
                if (header_len == 3)
                    cmd_len += ((int)(*p++) & 0x7f) << 16;
                cmd_len += ((int)(*p++)) << 8;
                cmd_len += ((int)(*p++));
            }
            toread = cmd_len + header_len - readbuf_len;
        }
        ret = recv(csocket.fd, (char*)readbuf + readbuf_len, toread, 0);
        if (ret == 0)
        {
            // End of file.
            Logger::log().error() << "Reader thread got EOF trying to read "<< toread << "bytes";
            goto out;
        }
        else if (ret == -1)
        {
            // IO error.
#ifdef WIN32
            Logger::log().error() << "Reader thread got error " << WSAGetLastError();
#else
            Logger::log().error() << "Reader thread got error " << errno << " : " << strerror_local(errno);
#endif
            goto out;
        }
        else
        {
            readbuf_len += ret;
            //Logger::log().error() << "Reader got some data ("<< readbuf_len<< " bytes total)";
        }

        // Finished with a command ?
        if (readbuf_len == cmd_len + header_len)
        {
            // LOG(LOG_DEBUG, "Reader got a full command\n", readbuf_len);
            command_buffer *buf = command_buffer_new(readbuf_len - header_len, readbuf + header_len);
            SDL_LockMutex(input_buffer_mutex);
            command_buffer_enqueue(buf, &input_queue_start, &input_queue_end);
            SDL_CondSignal(input_buffer_cond);
            SDL_UnlockMutex(input_buffer_mutex);
            cmd_len = -1;
            header_len = 0;
            readbuf_len = 0;
        }
    }
out:
    Logger::log().error() << "Reader thread stopped";
    CloseClientSocket();
    return -1;
}

//================================================================================================
//
//================================================================================================
int Network::writer_thread_loop(void *nix)
{
    Logger::log().info() << "Writer thread started";
    int written, ret;
    command_buffer *buf;
    while (!abort_thread)
    {
        written = 0;

        SDL_LockMutex(output_buffer_mutex);
        while (!output_queue_start && !abort_thread)
            SDL_CondWait(output_buffer_cond, output_buffer_mutex);
        buf = command_buffer_dequeue(&output_queue_start, &output_queue_end);
        SDL_UnlockMutex(output_buffer_mutex);

        if (abort_thread) break;

        while (written < buf->len && !abort_thread)
        {
            ret = send(csocket.fd, (char*)buf->data + written, buf->len - written, 0);
            if (ret == 0)
            {
                Logger::log().error() << "Writer got EOF";
                goto out;
            }
            else if (ret == -1)
            {
                // IO error.
#ifdef WIN32
                Logger::log().error() << "Writer thread got error " << WSAGetLastError();
#else
                Logger::log().error() << "Writer thread got error " << errno << " : " << strerror_local(errno);
#endif
                goto out;
            }
            else
                written += ret;
        }
        command_buffer_free(buf);
        //      Logger::log().error() <<"Writer wrote a command (%d bytes)\n", written); */
    }
out:
    Logger::log().info() << "Writer thread stopped";
    CloseClientSocket();
    return 0;
}

//================================================================================================
//
//================================================================================================
void Network::socket_thread_stop()
{
    Logger::log().info() << "Stopping thread.";
    SDL_WaitThread(input_thread, NULL);
    SDL_WaitThread(output_thread, NULL);
    /*
        SDL_DestroyCond(input_buffer_cond);
        SDL_DestroyMutex(input_buffer_mutex);
        SDL_DestroyCond(output_buffer_cond);
        SDL_DestroyMutex(output_buffer_mutex);
        SDL_DestroyMutex(socket_mutex);
    */
}

//================================================================================================
//
//================================================================================================
bool Network::CloseSocket()
{
    if (csocket.fd == NO_SOCKET)
        return true;
#ifdef WIN32
    closesocket(csocket.fd);
#else
    close(csocket.fd);
#endif
    return true;
}

//================================================================================================
//
//================================================================================================
bool Network::CloseClientSocket()
{
    SDL_LockMutex(socket_mutex);
    if (csocket.fd != NO_SOCKET)
    {
        Logger::log().info() << "CloseClientSocket()";
        CloseSocket();
        csocket.inbuf = "";
        csocket.outbuf = "";
        csocket.fd = NO_SOCKET;
        abort_thread = true;
        SDL_CondSignal(input_buffer_cond);
        SDL_CondSignal(output_buffer_cond);
    }
    SDL_UnlockMutex(socket_mutex);
    return true;
}

//================================================================================================
//
//================================================================================================
bool Network::OpenClientSocket(const char *host, int port)
{
    if (!OpenSocket(host, port)) return false;
    csocket.command_sent = 0;
    csocket.command_received = 0;
    csocket.command_time = 0;
    int tmp = 1;
    if (setsockopt(csocket.fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
    {
        Logger::log().error() << "setsockopt(TCP_NODELAY) failed";
    }
    return true;
}

#ifdef WIN32
//================================================================================================
//
//================================================================================================
bool Network::OpenSocket(const char *host, int port)
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
            csocket.fd = NO_SOCKET;
            return(false);
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }
    temp = 1;   // non-block
    if (ioctlsocket(csocket.fd, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << "ioctlsocket(csocket.fd, FIONBIO , &temp)";
        csocket.fd = NO_SOCKET;
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
        SDL_Delay(30);
        // timeout.... without connect will REALLY hang a long time
        if (start_timer + TIMEOUT_MS < SDL_GetTicks())
        {
            csocket.fd = NO_SOCKET;
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
        csocket.fd = NO_SOCKET;
        return(false);
    }
    // we got a connect here!

    // Clear nonblock flag
    temp = 0;
    if (ioctlsocket(csocket.fd, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << "ioctlsocket(csocket.fd, FIONBIO , &temp == 0)";
        csocket.fd = NO_SOCKET;
        return(FALSE);
    }

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
#endif

#ifndef WIN32
//================================================================================================
//
//================================================================================================
bool Network::OpenSocket(const char *host, int port)
{
    unsigned int  oldbufsize, newbufsize = 65535, buflen = sizeof(int);
    struct linger linger_opt;
    int flags;
    uint32 start_timer;
    /* Use new (getaddrinfo()) or old (gethostbyname()) socket API */
#if 0
//#ifndef HAVE_GETADDRINFO
    /* This method is preferable unless IPv6 is required, due to buggy distros. See mantis 0000425 */
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
        csocket.fd = NO_SOCKET;
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
    // Set non-blocking.
    flags = fcntl(csocket.fd, F_GETFL);
    if (fcntl(csocket.fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        Logger::log().error() << "socket: Error on switching to non-blocking.\n";
        csocket.fd = NO_SOCKET;
        return false;
    }
    // Try to connect.
    start_timer = SDL_GetTicks();
    while (connect(csocket.fd, (struct sockaddr *) &insock, sizeof(insock)) == -1)
    {
        SDL_Delay(3);
        /* timeout.... without connect will REALLY hang a long time */
        if (start_timer + TIMEOUT_MS < SDL_GetTicks())
        {
            perror("Can't connect to server");
            csocket.fd = NO_SOCKET;
            return false;
        }
    }
    // Set back to blocking.
    if (fcntl(csocket.fd, F_SETFL, flags) == -1)
    {
        Logger::log().error() << "socket: Error on switching to blocking.";
        csocket.fd = NO_SOCKET;
        return false;
    }
#else
    struct addrinfo hints;
    struct addrinfo *res=0;
    struct addrinfo *ai;
    char hostaddr[40];
    Logger::log().info() << "Opening to "<< host << " " << port;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    // Workaround for issue #425 on OSs with broken NIS+ like FC5.
    // This should disable any service lookup
    hints.ai_flags = AI_NUMERICSERV;
    if (getaddrinfo(host, StringConverter::toString(port).c_str(), &hints, &res) != 0)
        return false;
    for (ai = res; ai != NULL; ai = ai->ai_next)
    {
        getnameinfo(ai->ai_addr, ai->ai_addrlen, hostaddr, sizeof(hostaddr), NULL, 0, NI_NUMERICHOST);
        Logger::log().info() << "  trying " << hostaddr;
        csocket.fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (csocket.fd == -1)
        {
            csocket.fd = NO_SOCKET;
            continue;
        }
        // Set non-blocking.
        flags = fcntl(csocket.fd, F_GETFL);
        if (fcntl(csocket.fd, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            Logger::log().error() << "socket: Error on switching to non-blocking.";
            csocket.fd = NO_SOCKET;
            return false;
        }
        // Try to connect.
        start_timer = SDL_GetTicks();
        while (connect(csocket.fd, ai->ai_addr, ai->ai_addrlen) != 0)
        {
            SDL_Delay(3);
            /* timeout.... without connect will REALLY hang a long time */
            if (start_timer + TIMEOUT_MS < SDL_GetTicks())
            {
                close(csocket.fd);
                csocket.fd = NO_SOCKET;
                goto next_try;
            }
        }
        // Set back to blocking.
        if (fcntl(csocket.fd, F_SETFL, flags) == -1)
        {
            Logger::log().error() << "socket: Error on switching to blocking.";
            csocket.fd = NO_SOCKET;
            return false;
        }
        break;
next_try:
        ;
    }
    freeaddrinfo(res);
    if (csocket.fd == NO_SOCKET)
    {
        Logger::log().error() << "Can't connect to server";
        return false;
    }
#endif
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

//================================================================================================
//
//================================================================================================
void Network::SendVersion()
{
    std::stringstream strCmd;
    strCmd << "version " << VERSION_CS << " " << VERSION_SC << " " << CLIENT_PACKAGE_NAME;
    Logger::log().info() << "Send version command: " << strCmd.str();
    cs_write_string(strCmd.str().c_str());
}

//================================================================================================
//
//================================================================================================
void Network::send_reply(const char *text)
{
    std::string strTxt = "reply ";
    strTxt+= text;
    cs_write_string(strTxt.c_str());
}

//================================================================================================
//
//================================================================================================
void Network::cs_write_string(const char *buf)
{
    std::string sl = buf;
    send_socklist(sl);
}

//================================================================================================
// We used our core connect routine to connect to metaserver, this is the special read one.
//================================================================================================
void Network::read_metaserver_data()
{
    int  stat, temp =0;
    char *ptr = new char[MAX_METASTRING_BUFFER];
    char *buf = new char[MAX_METASTRING_BUFFER];
    while (1)
    {
#ifdef WIN32
        stat = recv(csocket.fd, ptr, MAX_METASTRING_BUFFER, 0);
        if ((stat == -1) && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            Logger::log().error() << "Error reading metaserver data!: "<< WSAGetLastError();
            break;
        }
        else
#else
        do
        {
            stat = recv(csocket.fd, ptr, MAX_METASTRING_BUFFER, 0);
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

//================================================================================================
// Connect to meta and get server data.
//================================================================================================
void Network::contactMetaserver()
{
    clearMetaServerData();
    csocket.fd = NO_SOCKET;
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "");
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "query metaserver...");
    std::stringstream strBuf;
    strBuf << "trying " << DEFAULT_METASERVER << " " << DEFAULT_METASERVER_PORT;
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strBuf.str().c_str());
    if (OpenSocket(DEFAULT_METASERVER, DEFAULT_METASERVER_PORT))
    {
        read_metaserver_data();
        CloseSocket();
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "done.");
    }
    else
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "metaserver failed! using default list.");
    add_metaserver_data("127.0.0.1", "127.0.0.1", DEFAULT_SERVER_PORT, -1, "local", "localhost.", "Start server before you try to connect.", "", "");
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "select a server.");
}

//================================================================================================
// Parse the metadata.
//================================================================================================
void Network::parse_metaserver_data(string strMetaData)
{
    string::size_type startPos;
    string::size_type endPos =0;
    string strIP, strPort, strName, strPlayer, strVersion, strDesc1, strDesc2, strDesc3, strDesc4;
    while (1)
    {
        // Server IP.
        startPos = endPos+0;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strIP = strMetaData.substr(startPos, endPos-startPos);
        // unknown 1.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strPort = strMetaData.substr(startPos, endPos-startPos);
        // Server name.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strName = strMetaData.substr(startPos, endPos-startPos);
        // Number of players online.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strPlayer = strMetaData.substr(startPos, endPos-startPos);
        // Server version.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strVersion = strMetaData.substr(startPos, endPos-startPos);
        // Description1
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strDesc1 = strMetaData.substr(startPos, endPos-startPos);
        // Description2.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strDesc2 = strMetaData.substr(startPos, endPos-startPos);
        startPos = endPos+1;
        // Description3.
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == std::string::npos) break;
        strDesc3 = strMetaData.substr(startPos, endPos-startPos);
        // Description4.
        startPos = endPos+1;
        endPos = strMetaData.find( '\n',  startPos);
        if (endPos == std::string::npos) endPos = strMetaData.size();
        strDesc4 = strMetaData.substr(startPos, endPos -startPos);
        if (endPos < strMetaData.size()) ++endPos;
        // Add the server to the linked list.
        add_metaserver_data(strIP.c_str(), strName.c_str(), atoi(strPort.c_str()), atoi(strPlayer.c_str()), strVersion.c_str(),
                            strDesc1.c_str(),  strDesc2.c_str(),  strDesc3.c_str(),  strDesc4.c_str());
    }
}

//================================================================================================
// Add server data to a linked list.
//================================================================================================
void Network::add_metaserver_data(const char *ip, const char *server, int port, int player, const char *ver,
                                  const char *desc1, const char *desc2, const char *desc3, const char *desc4)
{
    Server *node = new Server;
    node->player = player;
    node->port   = DEFAULT_SERVER_PORT;//port;
    node->name   = server;
    node->ip     = ip;
    node->version= ver;
    node->desc[0]  = desc1;
    node->desc[1]  = desc2;
    node->desc[2]  = desc3;
    node->desc[3]  = desc4;
    mvServer.push_back(node);
    string strRow = server;
    if (player <0) strRow+=",-";
    else           strRow+=","+StringConverter::toString(player);
    GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_SERVERSELECT, GuiManager::GUI_MSG_ADD_TABLEROW, GuiImageset::GUI_TABLE, (void*) strRow.c_str());
}

//================================================================================================
// .
//================================================================================================
const char *Network::get_metaserver_info(int node, int infoLineNr)
{
    return mvServer[node]->desc[infoLineNr &3].c_str();
}
