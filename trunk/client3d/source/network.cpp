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
//  { Network::ChannelMsgCmd },  // 28
};

const int SUM_SERVER_COMMANDS = sizeof(commands) / sizeof(CmdMapping);

SDL_Thread *Network::mInputThread =0;
SDL_Thread *Network::mOutputThread=0;
SDL_mutex  *Network::mMutex =0;
SDL_cond   *Network::mInputCond  =0;
SDL_cond   *Network::mOutputCond =0;
// start is the first waiting item in queue, end is the most recent enqueued:
Network::command_buffer *Network::mInputQueueStart = 0, *Network::mInputQueueEnd = 0;
Network::command_buffer *Network::mOutputQueueStart= 0, *Network::mOutputQueueEnd= 0;

const int TIMEOUT_MS = 4000;
const int NO_SOCKET  = -1;
const int MAXSOCKBUF = 128*1024; // Maximum size of any packet we expect.
const int MAX_BUF    = 256;

int Network::mSocket = NO_SOCKET;
bool Network::mAbortThread  = true;
bool Network::mEndianConvert= false;
unsigned char readbuf[MAXSOCKBUF+1];

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
    sl += (data) & 0xff;
}

//================================================================================================
// Get the socket error number.
//================================================================================================
String &Network::getError()
{
    static String strError;
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

void Network::freeRecources()
{
    mAbortThread = true;
    CloseClientSocket();
    if (!mMutex) return;
    SDL_WaitThread(mInputThread, NULL);  // Stopping reader thread.
    SDL_WaitThread(mOutputThread, NULL); // Stopping writer thread.
    // Empty all queues.
    while (mInputQueueStart)
        command_buffer_free(command_buffer_dequeue(&mInputQueueStart, &mInputQueueEnd));
    while (mOutputQueueStart)
        command_buffer_free(command_buffer_dequeue(&mOutputQueueStart, &mOutputQueueEnd));
    SDL_DestroyMutex(mMutex);
    SDL_DestroyCond(mInputCond);
    SDL_DestroyCond(mOutputCond);
    clearMetaServerData();
    mMutex = 0;
}

//================================================================================================
//
//================================================================================================
bool Network::Init()
{
    if (mMutex) return false;
    Logger::log().headline() << "Starting Network";
#ifdef WIN32
    WSADATA w;
    if (WSAStartup(MAKEWORD(2, 2), &w))
    {
        if (WSAStartup(MAKEWORD(2, 0), &w))
        {
            int error = WSAStartup(MAKEWORD(1, 1), &w);
            if (error)
            {
                Logger::log().error() << "Error init starting Winsock: "<< error;
                return false;
            }
        }
    }
    Logger::log().info() <<  "Using socket version " << w.wVersion;
#endif
    return true;
}

//================================================================================================
//
//================================================================================================
void Network::socket_thread_start()
{
    if (mMutex) return;
    mInputCond  = SDL_CreateCond();
    mOutputCond = SDL_CreateCond();
    mMutex = SDL_CreateMutex();
    mAbortThread = false;
    mInputThread = SDL_CreateThread(reader_thread_loop, NULL);
    if (!mInputThread)
        Logger::log().error() <<  "Unable to start input thread: " << SDL_GetError();
    mOutputThread = SDL_CreateThread(writer_thread_loop, NULL);
    if (!mOutputThread)
        Logger::log().error() <<  "Unable to start output thread: " << SDL_GetError();
}

//================================================================================================
// Handle all enqueued commands.
//================================================================================================
void Network::update()
{
    while (!mAbortThread)
    {
        // Get a read command and remove it from queue.
        SDL_LockMutex(mMutex);
        command_buffer *cmd = command_buffer_dequeue(&mInputQueueStart, &mInputQueueEnd);
        SDL_UnlockMutex(mMutex);
        if (!cmd) return;
        if (!cmd->data[0] || (cmd->data[0]&~0x80)-1 >= SUM_SERVER_COMMANDS)
            Logger::log().error() << "Bad command from server " << (int)(cmd->data[0]&~0x80)-1;
        else
        {
            int lenHeader = cmd->data[0]&0x80?5:3;
            Logger::log().error() << "Got server cmd " << (int)(cmd->data[0]&~0x80)-1 << " len (incl. Header) =" << cmd->len;
            commands[(cmd->data[0]&~0x80) - 1].serverCmd(cmd->data+lenHeader, cmd->len-lenHeader);
        }
        command_buffer_free(cmd);
    }
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

/*
// Strips excess whitespace from string, writing the normalized string to ScratchSpace.
static char *normalize_string(const char *string)
{
    static char ScratchSpace[MAX_BUF];
    static char  buf[MAX_BUF], // this will be a wc of string
    *token = NULL;
    strcpy(buf, string);
    // Wipe ScratchSpace clean every time:
    *ScratchSpace = '\0';
    // Get the next non-whitespace token from buf and concatenate it and one
    // trailing whitespace to ScratchSpace:
    for (token = strtok(buf, " \t"); token != NULL; token = strtok(NULL, " \t"))
    {
        strcat(ScratchSpace, token);
        strcat(ScratchSpace, " ");
    }
    // There will be one trailing whitespace left. Get rid of it:
    ScratchSpace[strlen(ScratchSpace) - 1] = '\0';
    return ScratchSpace;
}

// Splits command at the next #
// returning a pointer to the occurrence (which is overwritten with \0 first) or
// NULL if no next multicommand is found or command is chat, etc.
static char *BreakMulticommand(const char *command)
{
    char *c = NULL;
    // Only look for a multicommand if the command is not one of these:
    if (!(!strnicmp(command, "/tell", 5)
            || !strnicmp(command, "/say", 4)
            || !strnicmp(command, "/reply", 6)
            || !strnicmp(command, "/gsay", 5)
            || !strnicmp(command, "/shout", 6)
            || !strnicmp(command, "/talk", 5)
            || !strnicmp(command, "/channel", 8)
            || (*command == '-')
            || !strnicmp(command, "/create", 7)))
    {
        if ((c = (char*)strchr(command, '#'))) // multicommand separator '#'
            *c = '\0';
    }
    return c;
}
*/

//================================================================================================
// Send a higher level game command like /tell, /say or other "slash" text commants.
// Usually, this kind of commands are typed in console or are bound to macros.
// The underlaying protocol command is CLIENT_CMD_GENERIC, which means its a command holding
// another command.
// For realtime or system commands, commands with binary params and such,
// not a slash command should be used but a new protocol command.
// Only that commands hold real binary params and can be pre-processed
// by the server protocol functions.
//================================================================================================
void Network::send_game_command(const char *command)
{
    /*
    char *token, cmd[1024];
    // Copy a normalized (leading, trailing, and excess inline whitespace stripped) command to cmd:
    strcpy(cmd, normalize_string(command));
    // Now go through cmd, possibly separating multicommands.
    // Each command (before separation) is pointed to by token:
    token = cmd;
    while (token != NULL && *token)
    {
        char *end;
        if (*token != '/' && *token != '-') // if not a command ... its chat  (- is for channel system)
        {
            char buf[MAX_BUF];
            sprintf(buf, "/say %s", token);
            strcpy(token, buf);
        }
        end = BreakMulticommand(token);
        if (!console_command_check(token))
        {
            // Nasty hack. Treat /talk as a special case: lowercase it and
            // print it to the message window as Topic: foo. -- Smacky 20071210
            if (!strnicmp(token, "/talk", 5))
            {
                int c;
                for (c = 0; *(token + c) != '\0'; c++)
                    *(token + c) = tolower(*(token + c));
                //draw_info_format(COLOR_DGOLD, "Topic: %s", token + 6);
            }
        */
    std::stringstream strCmd;
    strCmd << command;
    //Logger::log().error() << "send: " << strCmd.str();
    send_command_binary(CLIENT_CMD_GENERIC, strCmd);
    /*
            }
            if (end != NULL)
                token = end + 1;
            else
                token = NULL;
        }
    */
}

//================================================================================================
//
//================================================================================================
int Network::send_command_binary(unsigned char cmd, std::stringstream &stream)
{
    command_buffer *buf;
    if (stream.str().size() == 1) // Single byte command.
    {
        unsigned char full_cmd[2] = { cmd, (unsigned char) *stream.str().c_str() };
        buf = command_buffer_new(2, full_cmd);
    }
    else
    {
        std::stringstream full_cmd;
        full_cmd << cmd << '\0' << (unsigned char)(stream.str().size()+1) << stream.str() << '\0';
        buf = command_buffer_new((int)full_cmd.str().size(), (unsigned char*)full_cmd.str().c_str());
    }
    /*
        Logger::log().error() << "send " << buf->len << " bytes:";
        String str1="", str2="";
        char strBuf[12];
        for (int i= 0; i < buf->len; ++i)
        {
            sprintf(strBuf,"%02x,", buf->data[i]);
            str1+= strBuf;
            str2+= " ";
            if (!buf->data[i])
                str2+= "|";
            else
                str2+= (char)buf->data[i];
        }
        Logger::log().error() << str1.substr(0, str1.size()-1);
        Logger::log().error() << str2;
    */
    SDL_LockMutex(mMutex);
    command_buffer_enqueue(buf, &mOutputQueueStart, &mOutputQueueEnd);
    SDL_CondSignal(mOutputCond); // Restart thread.
    SDL_UnlockMutex(mMutex);
    return 0;
}

/*
 * Lowlevel socket IO
 */

//================================================================================================
//
//================================================================================================
int Network::strToInt(unsigned char *data, int bytes)
{
    if (bytes == 2)
    {
        if (mEndianConvert)
            return (data[0] << 8) + data[1];
        return (data[1] << 8) + data[0];
    }
    if (mEndianConvert)
        return (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
    return (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
}

//================================================================================================
//
//================================================================================================
int Network::reader_thread_loop(void *)
{
    Logger::log().info() << "Reader thread started ";
    int cmd_len, pos;
    while (!mAbortThread)
    {
        // Read the command byte.
        if (!(pos = recv(mSocket, (char*)readbuf, 1, 0))) break;
        int sizeOfLen = readbuf[0] & (1<<7)?4:2;
        // If bit 7 is set in the cmd, sizeof(cmd_len) = 4 else sizeof(cmd_len) = 2.
        if (!(pos+= recv(mSocket, (char*)readbuf+1, sizeOfLen, 0))) break;
        cmd_len = strToInt(readbuf+1, sizeOfLen) + pos;
        if (cmd_len >= MAXSOCKBUF)
        {
            Logger::log().error() << "Network::reader_thread_loop: To much data from server.";
            break;
        }
        while (!mAbortThread && cmd_len-pos >0)
            pos+= recv(mSocket, (char*)readbuf+pos, cmd_len-pos, 0);
        command_buffer *buf = command_buffer_new(cmd_len, readbuf);
        buf->data[cmd_len] = 0; // We terminate the buffer for security and incoming raw strings.
        SDL_LockMutex(mMutex);
        command_buffer_enqueue(buf, &mInputQueueStart, &mInputQueueEnd);
        SDL_UnlockMutex(mMutex);
    }
    Logger::log().info() << "Reader thread stopped";
    return 0;
}

//================================================================================================
//
//================================================================================================
int Network::writer_thread_loop(void *)
{
    Logger::log().info() << "Writer thread started";
    int written, ret;
    command_buffer *buf;
    while (!mAbortThread)
    {
        written = 0;
        SDL_LockMutex(mMutex);
        SDL_CondWait(mOutputCond, mMutex);
        buf = command_buffer_dequeue(&mOutputQueueStart, &mOutputQueueEnd);
        SDL_UnlockMutex(mMutex);
        if (!buf) break; // Happens when this thread is stopped by the main thread.
        while (written < buf->len && !mAbortThread)
        {
            ret = send(mSocket, (char*)buf->data + written, buf->len - written, 0);
            if (ret == 0)
            {
                Logger::log().error() << "Writer got EOF";
                break;
            }
            if (ret == -1)
            {
                Logger::log().error() << "Writer thread got error " << getError();
                break;
            }
            written += ret;
        }
        command_buffer_free(buf);
    }
    Logger::log().info() << "Writer thread stopped";
    return 0;
}

//================================================================================================
//
//================================================================================================
void Network::CloseSocket()
{
    if (mSocket == NO_SOCKET) return;
#ifdef WIN32
    shutdown(mSocket, SD_BOTH); // Do we have to wait here for WSAAsyncSelect() to send a FD_CLOSE?
    closesocket(mSocket);
    WSACleanup();
#else
    shutdown(mSocket, SHUT_RDWR);
    close(mSocket);
#endif
    mSocket = NO_SOCKET;
}

//================================================================================================
//
//================================================================================================
void Network::CloseClientSocket()
{
    if (!mMutex) return;
    SDL_LockMutex(mMutex);
    CloseSocket();
    SDL_UnlockMutex(mMutex);
    SDL_LockMutex(mMutex);
    SDL_CondSignal(mInputCond);  // Restart thread.
    SDL_CondSignal(mOutputCond); // Restart thread.
    SDL_UnlockMutex(mMutex);
}

//================================================================================================
// Opens the socket
//================================================================================================
bool Network::OpenActiveServerSocket()
{
    if (!OpenSocket(mvServer[mActServerNr]->ip.c_str(), mvServer[mActServerNr]->port))
        return false;
    int tmp = 1;
    if (setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
    {
        Logger::log().error() << "setsockopt(TCP_NODELAY) failed";
        return false;
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
    uint32          timeout;
    struct linger   linger_opt;
    Logger::log().info() <<  "OpenSocket: "<< host;
    // The way to make the sockets work on XP Home - The 'unix' style socket seems to fail under xp home.
    mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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
            mSocket = NO_SOCKET;
            return false;
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }
    temp = 1;   // non-block
    if (ioctlsocket(mSocket, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << "ioctlsocket(mSocket, FIONBIO , &temp)";
        mSocket = NO_SOCKET;
        return false;
    }
    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 5;
    if (setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
        Logger::log().error() << "BUG: Error on setsockopt LINGER";
    error = 0;
    timeout = SDL_GetTicks() + TIMEOUT_MS;
    while (connect(mSocket, (struct sockaddr *) &insock, sizeof(insock)) == SOCKET_ERROR)
    {
        SDL_Delay(30);
        if (SDL_GetTicks() > timeout)
        {
            mSocket = NO_SOCKET;
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
        mSocket = NO_SOCKET;
        return false;
    }
    // we got a connect here!
    // Clear nonblock flag
    temp = 0;
    if (ioctlsocket(mSocket, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << "ioctlsocket(mSocket, FIONBIO , &temp == 0)";
        mSocket = NO_SOCKET;
        return false;
    }

    if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;
    if (oldbufsize < newbufsize)
    {
        if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
            setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
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
    uint32 timeout;
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
    mSocket = socket(PF_INET, SOCK_STREAM, protox->p_proto);
    if (mSocket == -1)
    {
        Logger::log().error() << "init_connection:  Error on socket command.";
        mSocket = NO_SOCKET;
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
    flags = fcntl(mSocket, F_GETFL);
    if (fcntl(mSocket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        Logger::log().error() << "socket: Error on switching to non-blocking.\n";
        mSocket = NO_SOCKET;
        return false;
    }
    // Try to connect.
    timeout = SDL_GetTicks() + TIMEOUT_MS;
    while (connect(mSocket, (struct sockaddr *) &insock, sizeof(insock)) == -1)
    {
        SDL_Delay(3);
        if (SDL_GetTicks() > timeout)
        {
            Logger::log().error() << "Can't connect to server";
            mSocket = NO_SOCKET;
            return false;
        }
    }
    // Set back to blocking.
    if (fcntl(mSocket, F_SETFL, flags) == -1)
    {
        Logger::log().error() << "socket: Error on switching to blocking.";
        mSocket = NO_SOCKET;
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
        mSocket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (mSocket == -1)
        {
            mSocket = NO_SOCKET;
            continue;
        }
        // Set non-blocking.
        flags = fcntl(mSocket, F_GETFL);
        if (fcntl(mSocket, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            Logger::log().error() << "socket: Error on switching to non-blocking.";
            mSocket = NO_SOCKET;
            return false;
        }
        // Try to connect.
        timeout = SDL_GetTicks() + TIMEOUT_MS;
        while (connect(mSocket, ai->ai_addr, ai->ai_addrlen) != 0)
        {
            SDL_Delay(3);
            if (SDL_GetTicks() > timeout)
            {
                close(mSocket);
                mSocket = NO_SOCKET;
                break;
            }
        }
        if (mSocket == NO_SOCKET) continue;
        // Got a connection. Set back to blocking.
        if (fcntl(mSocket, F_SETFL, flags) == -1)
        {
            Logger::log().error() << "socket: Error on switching to blocking.";
            mSocket = NO_SOCKET;
            return false;
        }
        break;
    }
    freeaddrinfo(res);
    if (mSocket == NO_SOCKET)
    {
        Logger::log().error() << "Can't connect to server";
        return false;
    }
#endif
    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 5;
    if (setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
        Logger::log().error() <<  "BUG: Error on setsockopt LINGER";
    if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;
    if (oldbufsize < newbufsize)
    {
        if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            Logger::log().error() << "socket: setsockopt unable to set output buf size to " << newbufsize;
            setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
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
    mSocket = NO_SOCKET;
    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "");
    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Query metaserver...");
    std::stringstream strBuf;
    strBuf << "Trying " << DEFAULT_METASERVER << " " << DEFAULT_METASERVER_PORT;
    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, strBuf.str().c_str());
    if (OpenSocket(DEFAULT_METASERVER, DEFAULT_METASERVER_PORT))
    {
        read_metaserver_data();
        CloseSocket();
        GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "done.");
    }
    else
        GuiManager::getSingleton().sendMsg( GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Metaserver failed! Using default list.", 0x00ff0000);
    add_metaserver_data("daimonin.game-server.cc", "daimonin.game-server.cc"   , DEFAULT_SERVER_PORT, -1, "internet", "~#ff00ff00STABLE",                        "Main Server", "", "");
    add_metaserver_data("Test-Server"            , "test-server.game-server.cc", DEFAULT_SERVER_PORT, -1, "internet", "~#ffffff00UNSTABLE",                      "Test Server", "", "");
    add_metaserver_data("127.0.0.1"              , "127.0.0.1"                 , DEFAULT_SERVER_PORT, -1, "local"   , "Start server before you try to connect.", "Localhost."           , "", "");
    GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, "Select a server.");
}

//================================================================================================
// We used our core connect routine to connect to metaserver, this is the special read one.
//================================================================================================
void Network::read_metaserver_data()
{
    static String buf = "";
    int len;
    char *ptr = new char[MAXSOCKBUF];
    while (1)
    {
        len = recv(mSocket, ptr, MAXSOCKBUF, 0);
        if (len == -1)
        {
#ifdef WIN32
            int errorNr = WSAGetLastError();
            if (errorNr != WSAEWOULDBLOCK)
            {
                Logger::log().error() << "Error reading metaserver data!: " << errorNr;
                break;
            }
#endif
            continue;
        }
        if (!len || (int)buf.size() + len >= MAXSOCKBUF) break;
        buf[len] = '\0';
        buf += ptr;
    }
    delete[] ptr;
    parse_metaserver_data(buf);
}

//================================================================================================
// Parse the metadata.
//================================================================================================
void Network::parse_metaserver_data(String &strMetaData)
{
    String::size_type startPos;
    String::size_type endPos =0;
    String strIP, strPort, strName, strPlayer, strVersion, strDesc1, strDesc2, strDesc3, strDesc4;
    while (1)
    {
        // Server IP.
        startPos = endPos+0;
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strIP = strMetaData.substr(startPos, endPos-startPos);
        // unknown 1.
        startPos = endPos+1;
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strPort = strMetaData.substr(startPos, endPos-startPos);
        // Server name.
        startPos = endPos+1;
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strName = strMetaData.substr(startPos, endPos-startPos);
        // Number of players online.
        startPos = endPos+1;
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strPlayer = strMetaData.substr(startPos, endPos-startPos);
        // Server version.
        startPos = endPos+1;
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strVersion = strMetaData.substr(startPos, endPos-startPos);
        // Description1
        startPos = endPos+1;
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strDesc1 = strMetaData.substr(startPos, endPos-startPos);
        // Description2.
        startPos = endPos+1;
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strDesc2 = strMetaData.substr(startPos, endPos-startPos);
        startPos = endPos+1;
        // Description3.
        endPos = strMetaData.find('|',  startPos);
        if (endPos == String::npos) break;
        strDesc3 = strMetaData.substr(startPos, endPos-startPos);
        // Description4.
        startPos = endPos+1;
        endPos = strMetaData.find('\n',  startPos);
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
    node->desc[0] = desc1;
    node->desc[1] = desc2;
    node->desc[2] = desc3;
    node->desc[3] = desc4;
    mvServer.push_back(node);
    String strRow;
    strRow+= desc2; strRow+= ";";
    strRow+= desc1; strRow+= ";";
    strRow+= server;
    strRow+=(player <0)?",-":","+StringConverter::toString(player);
    GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_ADD_ROW, strRow.c_str());
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
    GuiManager::getSingleton().sendMsg(GuiManager::GUI_TABLE, GuiManager::MSG_CLEAR);
    if (!mvServer.size()) return;
    for (std::vector<Server*>::iterator i = mvServer.begin(); i != mvServer.end(); ++i)
        delete(*i);
    mvServer.clear();
}
