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

#include <Ogre.h>
#include "network.h"
#include "logger.h"
#include "option.h"
#include "define.h"
#include "network_serverfile.h"
#include "tile_manager.h"
#include "gui_manager.h"
#include "option.h"

using namespace Ogre;

#define DEFAULT_SERVER_PORT 13327
#define DEFAULT_METASERVER_PORT 13326
#define DEFAULT_METASERVER "damn.informatik.uni-bremen.de"

struct CmdMapping
{
    void (*serverCmd)(unsigned char *, int len);
};
struct CmdMapping commands[] =
{
    // Don't change this sorting! Its hardcoded in the server.
    { Network::DrawInfoCmd },    //  0
    { Network::AddMeFail },      //  1
    { Network::Map2Cmd },        //  2
    { Network::DrawInfoCmd2 },   //  3
    { Network::ItemXCmd },       //  4
    { Network::SoundCmd},        //  5
    { Network::TargetObject },   //  6
    { Network::ItemUpdateCmd },  //  7
    { Network::ItemDeleteCmd },  //  8
    { Network::StatsCmd },       //  9
    { Network::ImageCmd },       // 10
    { Network::Face1Cmd},        // 11
    { Network::SkillRdyCmd },    // 12
    { Network::PlayerCmd },      // 13
    { Network::SpelllistCmd },   // 14
    { Network::SkilllistCmd },   // 15
    { Network::GolemCmd },       // 16
    { Network::AccNameSuccess }, // 17
    { Network::SetupCmd },       // 18
    { Network::DataCmd },        // 19
    { Network::ItemYCmd },       // 20
    { Network::GroupCmd },       // 21
    { Network::GroupInviteCmd }, // 22
    { Network::GroupUpdateCmd }, // 23
    { Network::InterfaceCmd },   // 24
    { Network::BookCmd },        // 25
    { Network::MarkCmd },        // 26
    { Network::AccountCmd },     // 27
    //{ Network::ChannelMsgCmd },  // 28
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

// start is the first waiting item in queue, end is the most recent enqueued:
Network::command_buffer *Network::input_queue_start = 0, *Network::input_queue_end = 0;
Network::command_buffer *Network::output_queue_start= 0, *Network::output_queue_end= 0;

const int TIMEOUT_MS = 4000;
const int NO_SOCKET = -1;
// Maximum size of any packet we expect. Using this makes it so we don't need to
// allocated and deallocated the same buffer over and over again and the price
// of using a bit of extra memory. IT also makes the code simpler.
const int  MAXSOCKBUF            = 128*1024;
const int  MAX_METASTRING_BUFFER = 128*2013;
const int  MAX_BUF =  256;
const int  BIG_BUF = 1024;

//================================================================================================
//
//================================================================================================
void Network::AddIntToString(String &sl, int data, bool shortInt)
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
// Get the socket error number.
//================================================================================================
String &Network::getError()
{
    static String strError = "";
#ifdef WIN32
    strError = StringConverter::toString(WSAGetLastError());
#else
    strError = StringConverter::toString(errno);
#if defined(HAVE_STRERROR)
    strError+= ": " + strerror(errno);
#endif
#endif
    return strError;
}

//================================================================================================
// .
//================================================================================================
Network::Network()
{
    mInitDone = false;
}

//================================================================================================
// .
//================================================================================================
Network::~Network()
{
    if (!mInitDone) return;
    CloseClientSocket();
#ifdef WIN32
    WSACleanup();
#endif
    handle_socket_shutdown();
    clearMetaServerData();
}

//================================================================================================
//
//================================================================================================
bool Network::Init()
{
    if (mInitDone) return true;
    csocket.fd = NO_SOCKET;
    Logger::log().headline() << "Starting Network";
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
// Handle all enqueued commands.
//================================================================================================
void Network::update()
{
    while (1)
    {
        // Get a read command and remove it from queue.
        SDL_LockMutex(input_buffer_mutex);
        command_buffer *cmd = command_buffer_dequeue(&input_queue_start, &input_queue_end);
        SDL_UnlockMutex(input_buffer_mutex);
        if (!cmd) return;
        if (!cmd->data[0] || (cmd->data[0]&~0x80)-1 >= SUM_SERVER_COMMANDS)
            Logger::log().error() << "Bad command from server " << (int)(cmd->data[0]&~0x80)-1;
        else
        {
            int lenHeader = cmd->data[0]&0x80?5:3;
            Logger::log().error() << "Got server cmd " << (int)(cmd->data[0]&~0x80)-1;
            commands[(cmd->data[0]&~0x80) - 1].serverCmd(cmd->data+lenHeader, cmd->len-lenHeader);
        }
        command_buffer_free(cmd);
    }
}

//================================================================================================
// The main thread should poll this function which detects connection shutdowns and
// removes the threads if it happens.
//================================================================================================
void Network::handle_socket_shutdown()
{
    Logger::log().info() << "Stopping socket thread.";
    SDL_WaitThread(input_thread, NULL);
    SDL_WaitThread(output_thread, NULL);
    // Empty all queues.
    while (input_queue_start)
        command_buffer_free(command_buffer_dequeue(&input_queue_start, &input_queue_end));
    while (output_queue_start)
        command_buffer_free(command_buffer_dequeue(&output_queue_start, &output_queue_end));
    /*
        SDL_DestroyCond(input_buffer_cond);
        SDL_DestroyMutex(input_buffer_mutex);
        SDL_DestroyCond(output_buffer_cond);
        SDL_DestroyMutex(output_buffer_mutex);
        SDL_DestroyMutex(socket_mutex);
    */
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

    String sl = "ncom ";
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
int Network::send_command_binary(unsigned char cmd, unsigned char *body, int len, int flags)
{
    command_buffer *buf;
    if (!body)  // single binary command without tail
        buf = command_buffer_new(len, body);
    else
    {
        const int MAX_DATA_TAIL_LENGTH = 255;
        // we have a string with a '/0'
        if (flags & SEND_CMD_FLAG_STRING) ++len;
        if (len >= MAX_DATA_TAIL_LENGTH)
        {
            Logger::log().error() << "Network::send_command_binary: socket buffer MAX_DATA_TAIL_LENGTH >= " << MAX_DATA_TAIL_LENGTH << " (" << len << ")";
            CloseClientSocket();
            return -1;
        }
        buf = command_buffer_new(len+3, 0); // we need max len + 1 byte cmd + 2 bytes cmd_len at last
        // setup the header and copy the data tail
        if (buf)
        {
            int len_copy = len;
            int data_offset = 1; // our command
            buf->data[0] = cmd;
            if (flags & SEND_CMD_FLAG_FIXED)
            {
                // this makes no sense for our current protocol
                if (flags & SEND_CMD_FLAG_STRING)
                    Logger::log().error() << "Network::send_command_binary: SEND_CMD_FLAG_FIXED and SEND_CMD_FLAG_STRING used together makes no sense!";
                // for a fixed len we must readjust the buffer len value
                buf->len = len+1; // pure data block length + cmd tag
            }
            else
            {
                // we have a dynamic data tail length - let the server know how long it is
                buf->data[data_offset++] = (uint8) ((len >> 8) & 0xFF);
                buf->data[data_offset++] = ((uint32) (len)) & 0xFF;
            }
            //Logger::log().error() << "send binary cmd: " << (int)cmd << " len: " << len << " (blen:" << buf->len << ") (" << (int)((flags & SEND_CMD_FLAG_FIXED)?-1:buf->data[1]) << "   " << (int)((flags & SEND_CMD_FLAG_FIXED)?-1:buf->data[2]) << ")";
            memcpy(buf->data+data_offset, body, len_copy);
            // If the command requests a c-style string ending - add the "\0".
            // The server will kick you if its missing.
            // why? as a marker for block but also to ensure a valid string
            // in the raw read buffer of the server.
            if (flags & SEND_CMD_FLAG_STRING)
                buf->data[len_copy+data_offset] = 0;
        }
    }
    if (!buf)
    {
        CloseClientSocket();
        return -1;
    }
    SDL_LockMutex(output_buffer_mutex);
    command_buffer_enqueue(buf, &output_queue_start, &output_queue_end);
    SDL_CondSignal(output_buffer_cond);
    SDL_UnlockMutex(output_buffer_mutex);
    return 0;
}

//================================================================================================
//
//================================================================================================
int Network::send_command_binary(unsigned char cmd, std::stringstream &stream)
{
    /// @todo add the missing single cmd. (see function above)
    std::stringstream full_cmd;
    full_cmd << cmd << '\0' << (unsigned char) (stream.str().size()+1) << stream.str() << '\0';
    command_buffer *buf = command_buffer_new((int)full_cmd.str().size(), (unsigned char*)full_cmd.str().c_str());
    /*
        Logger::log().error() << "command_buffer: " << buf->len << "   " << stream.str().size();
        for (int i= 0; i < buf->len; ++i)
            Logger::log().error() << buf->data[i] << "  " << (int) buf->data[i];
    */
    SDL_LockMutex(output_buffer_mutex);
    command_buffer_enqueue(buf, &output_queue_start, &output_queue_end);
    SDL_CondSignal(output_buffer_cond);
    SDL_UnlockMutex(output_buffer_mutex);
    return 0;
}

//================================================================================================
// move a command/buffer to the out buffer so it can be written to the socket.
//================================================================================================
int Network::send_socklist(String msg)
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

/*
 * Lowlevel socket IO
 */

//================================================================================================
//
//================================================================================================
int Network::reader_thread_loop(void *)
{
    static unsigned char readbuf[MAXSOCKBUF+1];
    int readbuf_len = 0;
    int header_len = 0;
    int cmd_len = -1;
    int ret;
    int toread;
    Logger::log().info() << "Reader thread started  ";
    while (!abort_thread)
    {
        // First, try to read a command length sequence.
        if (!readbuf_len)
            toread = 1; // Try to read a command from the socket.
        else if (!(readbuf[0] & 0x80) && readbuf_len < 3)
            toread = 3 - readbuf_len; // read in 2 or 1 more bytes.
        else if ((readbuf[0] & 0x80) && readbuf_len < 5)
            toread = 5 - readbuf_len; // read in 4 to 1 more bytes.
        else
        {
            if (readbuf_len == 3 && !(readbuf[0] & 0x80))
            {
                header_len = 3;
                cmd_len = GetShort_String(readbuf+1);
            }
            else if (readbuf_len == 5 && (readbuf[0] & 0x80))
            {
                header_len = 5;
                cmd_len = GetInt_String(readbuf+1);

            }
            toread = cmd_len + header_len - readbuf_len;
            if (cmd_len+16 >= MAXSOCKBUF)
            {
                Logger::log().error() << "Network::reader_thread_loop: To much data from server.";
            }
        }
        ret = recv(csocket.fd, (char*)readbuf + readbuf_len, toread, 0);
        if (ret == 0)
        {
            // End of file.
            Logger::log().error() << "Reader thread got EOF trying to read "<< toread << " bytes";
            goto out;
        }
        else if (ret == -1)
        {
            // IO error.
            Logger::log().error() << "Reader thread got error " << getError();
            goto out;
        }
        else
        {
            readbuf_len += ret;
            //Logger::log().error() << "Reader got some data ("<< readbuf_len<< " bytes total)";
        }
        // Finished with a command ?
        if (readbuf_len == cmd_len + header_len && !abort_thread)
        {
            // LOG(LOG_DEBUG, "Reader got a full command\n", readbuf_len);
            command_buffer *buf = command_buffer_new(readbuf_len, readbuf);
            buf->data[readbuf_len] = 0; // we terminate our buffer for security and incoming raw strings
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
int Network::writer_thread_loop(void *)
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
                Logger::log().error() << "Writer thread got error " << getError();
                goto out;
            }
            else
                written += ret;
        }
        command_buffer_free(buf);
        // Logger::log().error() <<"Writer wrote a command (%d bytes)\n", written); */
    }
out:
    Logger::log().info() << "Writer thread stopped";
    CloseClientSocket();
    return 0;
}

//================================================================================================
//
//================================================================================================
void Network::CloseSocket()
{
    if (csocket.fd == NO_SOCKET)
        return;
    csocket.fd = NO_SOCKET;
#ifdef WIN32
    closesocket(csocket.fd);
#else
    close(csocket.fd);
#endif
}

//================================================================================================
//
//================================================================================================
void Network::CloseClientSocket()
{
    if (csocket.fd == NO_SOCKET) return;
    Logger::log().info() << "CloseClientSocket()";
    SDL_LockMutex(socket_mutex);
    CloseSocket();
    csocket.inbuf = "";
    csocket.outbuf = "";
    abort_thread = true;
    SDL_CondSignal(input_buffer_cond);
    SDL_CondSignal(output_buffer_cond);
    SDL_UnlockMutex(socket_mutex);
}

//================================================================================================
// Opens the socket
//================================================================================================
bool Network::OpenActiveServerSocket()
{
    if (!OpenSocket(mvServer[mActServerNr]->ip.c_str(), mvServer[mActServerNr]->port))
        return false;
    int tmp = 1;
    if (setsockopt(csocket.fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
    {
        Logger::log().error() << "setsockopt(TCP_NODELAY) failed";
        return false;
    }
    csocket.command_sent = 0;
    csocket.command_time = 0;
    csocket.command_received = 0;
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
            return false;
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }
    temp = 1;   // non-block
    if (ioctlsocket(csocket.fd, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << "ioctlsocket(csocket.fd, FIONBIO , &temp)";
        csocket.fd = NO_SOCKET;
        return false;
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
            return false;
        }
        int errorNr = WSAGetLastError();
        if (errorNr == WSAEISCONN)  // we have a connect!
            break;
        if (errorNr == WSAEWOULDBLOCK || errorNr == WSAEALREADY || (errorNr == WSAEINVAL && error)) // loop until we finished
        {
            error = 1;
            continue;
        }
        Logger::log().warning() <<  "Connect Error: " << errorNr;
        csocket.fd = NO_SOCKET;
        return false;
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
    return true;
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
    // Use new (getaddrinfo()) or old (gethostbyname()) socket API
#if 0
//#ifndef HAVE_GETADDRINFO
    // This method is preferable unless IPv6 is required, due to buggy distros. See mantis 0000425
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
        Logger::log().info() << "  Trying " << hostaddr;
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
            // timeout.... without connect will REALLY hang a long time
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
// Connect to meta and get server data.
//================================================================================================
void Network::contactMetaserver()
{
    clearMetaServerData();
    csocket.fd = NO_SOCKET;
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "");
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Query metaserver...");
    std::stringstream strBuf;
    strBuf << "Trying " << DEFAULT_METASERVER << " " << DEFAULT_METASERVER_PORT;
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, strBuf.str().c_str());
    if (OpenSocket(DEFAULT_METASERVER, DEFAULT_METASERVER_PORT))
    {
        read_metaserver_data();
        CloseSocket();
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "done.");
    }
    else
        GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Metaserver failed! Using default list.", 0x00ff0000);
    add_metaserver_data("daimonin.game-server.cc", "daimonin.game-server.cc"   , DEFAULT_SERVER_PORT, -1, "internet", "~#ff00ff00STABLE",                        "Main Server", "", "");
    add_metaserver_data("Test-Server"            , "test-server.game-server.cc", DEFAULT_SERVER_PORT, -1, "internet", "~#ffffff00UNSTABLE",                      "Test Server", "", "");
    add_metaserver_data("127.0.0.1"              , "127.0.0.1"                 , DEFAULT_SERVER_PORT, -1, "local"   , "Start server before you try to connect.", "Localhost."           , "", "");
    GuiManager::getSingleton().addTextline(GuiManager::GUI_WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, "Select a server.");
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
                // connection closed by meta
                break;
            }
    }
    buf[temp] = 0;
    parse_metaserver_data(buf);
    delete[] ptr;
    delete[] buf;
}

//================================================================================================
// Parse the metadata.
//================================================================================================
void Network::parse_metaserver_data(String strMetaData)
{
    String::size_type startPos;
    String::size_type endPos =0;
    String strIP, strPort, strName, strPlayer, strVersion, strDesc1, strDesc2, strDesc3, strDesc4;
    while (1)
    {
        // Server IP.
        startPos = endPos+0;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strIP = strMetaData.substr(startPos, endPos-startPos);
        // unknown 1.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strPort = strMetaData.substr(startPos, endPos-startPos);
        // Server name.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strName = strMetaData.substr(startPos, endPos-startPos);
        // Number of players online.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strPlayer = strMetaData.substr(startPos, endPos-startPos);
        // Server version.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strVersion = strMetaData.substr(startPos, endPos-startPos);
        // Description1
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strDesc1 = strMetaData.substr(startPos, endPos-startPos);
        // Description2.
        startPos = endPos+1;
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strDesc2 = strMetaData.substr(startPos, endPos-startPos);
        startPos = endPos+1;
        // Description3.
        endPos = strMetaData.find( '|',  startPos);
        if (endPos == String::npos) break;
        strDesc3 = strMetaData.substr(startPos, endPos-startPos);
        // Description4.
        startPos = endPos+1;
        endPos = strMetaData.find( '\n',  startPos);
        if (endPos == String::npos) endPos = strMetaData.size();
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
    node->port   = DEFAULT_SERVER_PORT;
    node->name   = server;
    node->ip     = ip;
    node->version= ver;
    node->desc[0]  = desc1;
    node->desc[1]  = desc2;
    node->desc[2]  = desc3;
    node->desc[3]  = desc4;
    mvServer.push_back(node);
    String strRow;
    strRow+= desc2; strRow+= ";";
    strRow+= desc1; strRow+= ";";
    strRow+=server;
    strRow+=(player <0)?",-":","+StringConverter::toString(player);
    GuiManager::getSingleton().sendMessage(GuiManager::GUI_WIN_SERVERSELECT, GuiManager::GUI_MSG_ADD_TABLEROW, GuiImageset::GUI_TABLE, (void*) strRow.c_str());
}

//================================================================================================
// .
//================================================================================================
const char *Network::get_metaserver_info(int node, int infoLineNr)
{
    return mvServer[node]->desc[infoLineNr &3].c_str();
}

//================================================================================================
// .
//================================================================================================
void Network::clearMetaServerData()
{
    GuiManager::getSingleton().clearTable(GuiManager::GUI_WIN_SERVERSELECT, GuiImageset::GUI_TABLE);
    if (!mvServer.size()) return;
    for (std::vector<Server*>::iterator i = mvServer.begin(); i != mvServer.end(); ++i)
        delete (*i);
    mvServer.clear();
}
