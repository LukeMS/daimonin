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

#include <boost/thread/mutex.hpp>
#include <Ogre.h>
#include <OgreTimer.h>
#include "logger.h"
#include "profiler.h"
#include "option.h"
#include "network.h"
#include "tile/tile_manager.h"
#include "gui/gui_manager.h"

using namespace Ogre;

static const char *DEFAULT_METASERVER = "www.daimonin.com";
static const int DEFAULT_METASERVER_PORT = 13326;
static const int THREAD_SLEEPING_TIME = 50;
static const int TIMEOUT_MS = 4000;
static const int NO_SOCKET  = -1;
static const int MAXSOCKBUF = 128*1024; // Maximum size of any packet we expect.
static const int MAX_BUF    = 256;
static boost::mutex mutex;

struct CmdMapping
{
    void (*serverCmd)(uchar *, int len);
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

static const int SUM_SERVER_COMMANDS = sizeof(commands) / sizeof(CmdMapping);

// start is the first waiting item in queue, end is the most recent enqueued:
Network::command_buffer *Network::mInputQueueStart = 0, *Network::mInputQueueEnd = 0;
Network::command_buffer *Network::mOutputQueueStart= 0, *Network::mOutputQueueEnd= 0;
int Network::mSocket = NO_SOCKET;
bool Network::mThreadsActive = false;
bool Network::mAbortThread   = true;
bool Network::mEndianConvert = false;

//================================================================================================
// .
//================================================================================================
Network::command_buffer *Network::command_buffer_new(unsigned int len, uchar *data)
{
//    PROFILE()
    command_buffer *buf = new command_buffer;
    buf->next = buf->prev = NULL;
    buf->len = len;
    buf->data = new uchar[len+1];
    if (data) memcpy(buf->data, data, len);
    buf->data[len] = 0; // Buffer overflow sentinel.
    return buf;
}

//================================================================================================
// .
//================================================================================================
void Network::command_buffer_enqueue(command_buffer *buf, command_buffer **queue_start, command_buffer **queue_end)
{
//    PROFILE()
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
//    PROFILE()
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
//    PROFILE()
    delete[] buf->data;
    delete buf;
}

//================================================================================================
// Get the socket error number.
//================================================================================================
String &Network::getError()
{
//    PROFILE()
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

//================================================================================================
//
//================================================================================================
void Network::freeRecources()
{
    PROFILE()
    CloseSocket();
#ifdef WIN32
    WSACleanup();
#endif
    clearMetaServerData();
    if (mThreadsActive)
    {
        mAbortThread = true;
        mInputThread.join();  // Wait for the thread to end.
        Logger::log().info() << Logger::ICON_CLIENT << "Reader thread stopped";
        mOutputThread.join(); // Wait for the thread to end.
        Logger::log().info() << Logger::ICON_CLIENT << "Writer thread stopped";
        mThreadsActive = false;
    }
    // Empty all queues.
    while (mInputQueueStart)
        command_buffer_free(command_buffer_dequeue(&mInputQueueStart, &mInputQueueEnd));
    while (mOutputQueueStart)
        command_buffer_free(command_buffer_dequeue(&mOutputQueueStart, &mOutputQueueEnd));
    mInputQueueStart = mInputQueueEnd = 0;
    mOutputQueueStart= mOutputQueueEnd= 0;
}

//================================================================================================
//
//================================================================================================
bool Network::Init()
{
    PROFILE()
    Logger::log().headline() << "Starting Network";
#ifdef WIN32
    WSADATA w;
    if (WSAStartup(MAKEWORD(2, 2), &w)) return true; // Version 2.2
    if (WSAStartup(MAKEWORD(2, 0), &w)) return true; // Version 2.0
    int error = WSAStartup(MAKEWORD(1, 1), &w);      // Version 1.1
    if (error)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Error init starting Winsock: "<< error;
        return false;
    }
#endif
    return true;
}

//================================================================================================
//
//================================================================================================
void Network::socket_thread_start()
{
    //  PROFILE()
    if (mThreadsActive) return;
    mAbortThread = false;
    mInputThread = boost::thread(&inputThread);
    mOutputThread= boost::thread(&outputThread);
    mThreadsActive = true;
}

//================================================================================================
// Handle one of the enqueued commands.
//================================================================================================
void Network::update(Real timeSinceLastFrame)
{
    PROFILE()
    static Real dTime = 0;
    dTime += timeSinceLastFrame * 1000;
    if ((dTime < THREAD_SLEEPING_TIME) ||  mAbortThread) return;
    dTime = 0;
    // Get a read command and remove it from queue.
    boost::mutex::scoped_lock // Mutex is locked for this block of code.
    lock(mutex);
    command_buffer *cmd = command_buffer_dequeue(&mInputQueueStart, &mInputQueueEnd);
    if (!cmd) return;// The queue is empty.
    // Bit 7 holds the header length.
    int lenHeader = cmd->data[0]&0x80?5:3;
    //Logger::log().error() << Logger::ICON_CLIENT << "Got server cmd " << (int)(cmd->data[0]&~0x80)-1 << " len (incl. Header) =" << cmd->len;
    commands[(cmd->data[0]&~0x80) - 1].serverCmd(cmd->data+lenHeader, cmd->len-lenHeader);
    command_buffer_free(cmd);
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
//   PROFILE()
    if (!command) return;
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
    if (*command == '/') ++command;
    strCmd << command;
    //String str = "send: " + strCmd.str();
    //GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, str.c_str());
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
void Network::send_command_binary(uchar cmd, std::stringstream &stream)
{
    //  PROFILE()
    command_buffer *buf;
    boost::mutex::scoped_lock // Mutex is locked for this block of code.
    lock(mutex);
    if (stream.str().size() == 1) // Single byte command.
    {
        uchar full_cmd[2] = { cmd, (uchar) *stream.str().c_str() };
        buf = command_buffer_new(2, full_cmd);
    }
    else
    {
        std::stringstream full_cmd;
        full_cmd << cmd << '\0' << (uchar)(stream.str().size()+1) << stream.str() << '\0';
        buf = command_buffer_new((int)full_cmd.str().size(), (uchar*)full_cmd.str().c_str());
    }
    /*
        Logger::log().error() << Logger::ICON_CLIENT << "send " << buf->len << " bytes:";
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
        Logger::log().error() << Logger::ICON_CLIENT << str1.substr(0, str1.size()-1);
        Logger::log().error() << Logger::ICON_CLIENT << str2;
    */
    command_buffer_enqueue(buf, &mOutputQueueStart, &mOutputQueueEnd);
}

/*
 * Lowlevel socket IO
 */
//================================================================================================
//
//================================================================================================
int Network::getCmdLen(uchar *data, int bytes)
{
//    PROFILE()
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
// The input thread is waiting for server commands.
//================================================================================================
void Network::inputThread()
{
    static uchar readbuf[MAXSOCKBUF+1];
    Logger::log().info() << Logger::ICON_CLIENT << "Reader thread started ";
    while (!mAbortThread)
    {
        // Read the command byte.
        if (!recv(mSocket, (char*)readbuf, 1, 0)) break; // Read error!
        // Is this a valid command?
        if (!readbuf[0] || (readbuf[0]&~0x80)-1 >= SUM_SERVER_COMMANDS)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Bad command from server: Command number is " << (int)(readbuf[0]&~0x80)-1 << ").";
            continue;
        }
        // Bit 7 of the command byte indicates the datatype (2 or 4 byte sized integer)
        int sizeOfCmd = readbuf[0] & (1<<7)?4:2;
        if (!sizeOfCmd)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Bad command from server: Command size is zero!";
            continue;
        }
        int pos = 1;
        if (!(pos+= recv(mSocket, (char*)readbuf+1, sizeOfCmd, 0))) break;
        // The cmd_len variable holds the length of the command send by the server.
        int cmd_len = getCmdLen(readbuf+1, sizeOfCmd) + pos;
        if (cmd_len >= MAXSOCKBUF)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Network::reader_thread_loop: To much data from server.";
            break;
        }
        while (cmd_len-pos >0 && !mAbortThread)
            pos+= recv(mSocket, (char*)readbuf+pos, cmd_len-pos, 0);
        boost::mutex::scoped_lock // Mutex is locked for this block of code.
        lock(mutex);
        command_buffer *buf = command_buffer_new(cmd_len, readbuf);
        buf->data[cmd_len] = 0; // We terminate the buffer for security and incoming raw strings.
        command_buffer_enqueue(buf, &mInputQueueStart, &mInputQueueEnd);
    }
}

//================================================================================================
// Send a command to the server.
//================================================================================================
void Network::outputThread()
{
    Logger::log().info() << Logger::ICON_CLIENT << "Writer thread started";
    while (!mAbortThread)
    {
#ifdef WIN32
        Sleep(THREAD_SLEEPING_TIME);
#else
        usleep(THREAD_SLEEPING_TIME);
#endif
        boost::mutex::scoped_lock // Mutex is locked for this block of code.
        lock(mutex);
        command_buffer *buf = command_buffer_dequeue(&mOutputQueueStart, &mOutputQueueEnd);
        if (!buf) continue;
        int written = 0;
        while (written < buf->len && !mAbortThread)
        {
            int ret = send(mSocket, (char*)buf->data + written, buf->len - written, 0);
            if (ret == 0)
            {
                Logger::log().error() << Logger::ICON_CLIENT << "Writer got EOF";
                break;
            }
            if (ret == -1)
            {
                Logger::log().error() << Logger::ICON_CLIENT << "Writer thread got error " << getError();
                break;
            }
            written += ret;
        }
        command_buffer_free(buf);
    }
}

//================================================================================================
//
//================================================================================================
void Network::CloseSocket(int socket)
{
    PROFILE()
    if (mSocket == NO_SOCKET) return;
#ifdef WIN32
    shutdown(socket, SD_BOTH); // Do we have to wait here for WSAAsyncSelect() to send a FD_CLOSE?
    closesocket(socket);
#else
    shutdown(socket, SHUT_RDWR);
    close(socket);
#endif
    socket = NO_SOCKET;
}

//================================================================================================
// Opens the socket of the active server.
//================================================================================================
bool Network::OpenActiveServerSocket()
{
    PROFILE()
    if (!OpenSocket(mvServer[mActServerNr]->ip.c_str(), mvServer[mActServerNr]->port, mSocket))
        return false;
    int tmp = 1;
    if (setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof(tmp)))
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Network::OpenActiveServerSocket(): setsockopt(TCP_NODELAY) failed";
        return false;
    }
    return true;
}

#ifdef WIN32
//================================================================================================
// Opens a socket.
//================================================================================================
bool Network::OpenSocket(const char *host, int port, int &sock)
{
    PROFILE()
    Logger::log().info() << Logger::ICON_CLIENT <<  "OpenSocket: " << host << " " << port;
    // The way to make the sockets work on XP Home - The 'unix' style socket seems to fail under xp home.
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in insock;
    insock.sin_family = AF_INET;
    insock.sin_port = htons((unsigned short) port);
    if (isdigit(*host))
        insock.sin_addr.s_addr = inet_addr(host);
    else
    {
        struct hostent *hostbn = gethostbyname(host);
        if (hostbn == (struct hostent *) NULL)
        {
            Logger::log().warning() << Logger::ICON_CLIENT <<  "Unknown host: "<< host;
            sock = NO_SOCKET;
            return false;
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }
    long temp = 1;   // non-block
    if (ioctlsocket(sock, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "ioctlsocket(socket, FIONBIO , &temp)";
        sock = NO_SOCKET;
        return false;
    }
    struct linger linger_opt;
    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 5;
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
        Logger::log().error() << Logger::ICON_CLIENT << "BUG: Error on setsockopt LINGER";
    int error = 0;
    uint32 timeout =  Root::getSingleton().getTimer()->getMilliseconds() + TIMEOUT_MS;
    while (connect(sock, (struct sockaddr *) &insock, sizeof(insock)) == SOCKET_ERROR)
    {
        if (Root::getSingleton().getTimer()->getMilliseconds() > timeout)
        {
            sock = NO_SOCKET;
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
        Logger::log().warning() << Logger::ICON_CLIENT <<  "Connect Error: " << errorNr;
        sock = NO_SOCKET;
        return false;
    }
    // we got a connect here!
    // Clear nonblock flag
    temp = 0;
    if (ioctlsocket(sock, FIONBIO, (u_long*)&temp) == -1)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "ioctlsocket(Socket, FIONBIO , &temp == 0)";
        sock = NO_SOCKET;
        return false;
    }
    int oldbufsize, newbufsize = 65535, buflen = sizeof(int);
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;
    if (oldbufsize < newbufsize)
    {
        if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
            setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
    }
    Logger::log().info() << Logger::ICON_CLIENT <<  "Connected to "<< host << "  " <<  port;
    return true;
}
#endif

#ifndef WIN32
//================================================================================================
//
//================================================================================================
bool Network::OpenSocket(const char *host, int port, int &sock)
{
    PROFILE()
    // Use new (getaddrinfo()) or old (gethostbyname()) socket API
#if 0
    //#ifndef HAVE_GETADDRINFO
    // This method is preferable unless IPv6 is required, due to buggy distros. See mantis 0000425
    struct protoent *protox;
    struct sockaddr_in insock;
    Logger::log().info() << Logger::ICON_CLIENT << "Opening to " << host << " " << port;
    protox = getprotobyname("tcp");
    if (protox == (struct protoent *) NULL)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Error on getting prorobyname (tcp)";
        return false;
    }
    sock = socket(PF_INET, SOCK_STREAM, protox->p_proto);
    if (sock == -1)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "init_connection:  Error on socket command.";
        sock = NO_SOCKET;
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
            Logger::log().error() << Logger::ICON_CLIENT << "Unknown host: " << host;
            return false;
        }
        memcpy(&insock.sin_addr, hostbn->h_addr, hostbn->h_length);
    }
    // Set non-blocking.
    int flags = fcntl(sock, F_GETFL);
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "socket: Error on switching to non-blocking.\n";
        sock = NO_SOCKET;
        return false;
    }
    // Try to connect.
    uint32 timeout =  Root::getSingleton().getTimer()->getMilliseconds() + TIMEOUT_MS;
    while (connect(sock, (struct sockaddr *) &insock, sizeof(insock)) == -1)
    {
        if (Root::getSingleton().getTimer()->getMilliseconds() > timeout)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "Can't connect to server";
            sock = NO_SOCKET;
            return false;
        }
    }
    // Set back to blocking.
    if (fcntl(sock, F_SETFL, flags) == -1)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "socket: Error on switching to blocking.";
        sock = NO_SOCKET;
        return false;
    }
#else
    struct addrinfo hints;
    struct addrinfo *res=0;
    struct addrinfo *ai;
    char hostaddr[40];
    Logger::log().info() << Logger::ICON_CLIENT << "Opening to "<< host << " " << port;
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
        Logger::log().info() << Logger::ICON_CLIENT << "  Trying " << hostaddr;
        sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock == -1)
        {
            sock = NO_SOCKET;
            continue;
        }
        // Set non-blocking.
        int flags = fcntl(sock, F_GETFL);
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "socket: Error on switching to non-blocking.";
            sock = NO_SOCKET;
            return false;
        }
        // Try to connect.
        uint32 timeout =  Root::getSingleton().getTimer()->getMilliseconds() + TIMEOUT_MS;
        while (connect(sock, ai->ai_addr, ai->ai_addrlen) != 0)
        {
            if (Root::getSingleton().getTimer()->getMilliseconds() > timeout)
            {
                close(sock);
                sock = NO_SOCKET;
                break;
            }
        }
        if (sock == NO_SOCKET) continue;
        // Got a connection. Set back to blocking.
        if (fcntl(sock, F_SETFL, flags) == -1)
        {
            Logger::log().error() << Logger::ICON_CLIENT << "socket: Error on switching to blocking.";
            sock = NO_SOCKET;
            return false;
        }
        break;
    }
    freeaddrinfo(res);
    if (sock == NO_SOCKET)
    {
        Logger::log().error() << Logger::ICON_CLIENT << "Can't connect to server";
        return false;
    }
#endif
    unsigned int oldbufsize, newbufsize = 65535, buflen = sizeof(int);
    struct linger linger_opt;
    linger_opt.l_onoff = 1;
    linger_opt.l_linger = 5;
    if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *) &linger_opt, sizeof(struct linger)))
        Logger::log().error() << Logger::ICON_CLIENT <<  "BUG: Error on setsockopt LINGER";
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
        oldbufsize = 0;
    if (oldbufsize < newbufsize)
    {
        if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            Logger::log().error() << Logger::ICON_CLIENT << "socket: setsockopt unable to set output buf size to " << newbufsize;
            setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
        }
    }
    Logger::log().info() << Logger::ICON_CLIENT <<  "Connected to "<< host << "  " <<  port;
    return true;
}
#endif

//================================================================================================
// Connect to meta and get server data.
//================================================================================================
void Network::contactMetaserver()
{
    PROFILE()
    clearMetaServerData();
    GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "");
    GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Query metaserver...");
    String str = "Trying "; str+= DEFAULT_METASERVER; str+= " " + StringConverter::toString(DEFAULT_METASERVER_PORT);
    GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, str.c_str());
    int socket = NO_SOCKET;
    if (OpenSocket(DEFAULT_METASERVER, DEFAULT_METASERVER_PORT, socket))
    {
        read_metaserver_data(socket);
        CloseSocket(socket);
        GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "done.");
    }
    else
    {
        GuiManager::getSingleton().print( GuiManager::LIST_MSGWIN, "Metaserver failed! Using default list.", 0xffffffa8);
        str = " 28|Test_Server|62.75.168.180|13327|Unknown|-|See_and_play_here_the_newest_maps_&_features!\n"
              "223|Daimonin   |62.75.224.80 |13327|Unknown|-|Public_Daimonin_game_server_from_www.daimonin.com\n";
        add_metaserver_data(str);
    }
    str = "223|Localhost  |127.0.0.1|13327|Unknown|-|Start server before you try to connect.\n";
    add_metaserver_data(str);
    GuiManager::getSingleton().print(GuiManager::LIST_MSGWIN, "Select a server.");
}

//================================================================================================
// We used our core connect routine to connect to metaserver, this is the special read one.
//================================================================================================
void Network::read_metaserver_data(int &socket)
{
    PROFILE()
    static String buf;
    buf.clear();
    char *ptr = new char[MAXSOCKBUF];
    while (1)
    {
        int len = recv(socket, ptr, MAXSOCKBUF, 0);
        if (len == -1)
        {
#ifdef WIN32
            int errorNr = WSAGetLastError();
            if (errorNr != WSAEWOULDBLOCK)
            {
                Logger::log().error() << Logger::ICON_CLIENT << "Error reading metaserver data!: " << errorNr;
                break;
            }
#endif
            continue;
        }
        if (!len || (int)buf.size() + len >= MAXSOCKBUF) break;
        ptr[len] = '\0';
        buf += ptr;
    }
    delete[] ptr;
    add_metaserver_data(buf);
}

//================================================================================================
// Add server data to a linked list.
//================================================================================================
void Network::add_metaserver_data(String strMetaData)
{
    PROFILE()
    enum
    {
        DATA_DESC1,
        DATA_NAME,
        DATA_IP,
        DATA_PORT,
        DATA_VERSION,
        DATA_PLAYER,
        DATA_INFO,
        DATA_SUM
    };
    String strData[DATA_SUM];
    size_t ServerEnd, ServerStart = 0;
    while (1)
    {
        ServerEnd = strMetaData.find('\n',  ServerStart);
        if (ServerEnd == String::npos) break;
        size_t endPos, startPos = ServerStart;
        for (int i = 0; i < DATA_SUM; ++i)
        {
            endPos = strMetaData.find('|',  startPos);
            if (endPos >= ServerEnd) endPos = ServerEnd;
            strData[i] = strMetaData.substr(startPos, endPos-startPos);
            std::replace(strData[i].begin(), strData[i].end(), '_', ' ');
            // ',' must be replaced, because its a keysign of GUI_ELEMENT_TABLE
            std::replace(strData[i].begin(), strData[i].end(), ',', '-');
            if (endPos == ServerEnd) break;
            startPos = ++endPos;
        }
        // Add a server.
        Server *node = new Server;
        node->player = atoi(strData[DATA_PLAYER].c_str());
        node->port   = atoi(strData[DATA_PORT].c_str());
        node->name   = strData[DATA_NAME];
        node->ip     = strData[DATA_IP];
        node->version= strData[DATA_VERSION];
        node->desc[0]= strData[DATA_DESC1];
        node->desc[1]= strData[DATA_INFO];
        node->desc[2]= strData[DATA_DESC1];
        node->desc[3]= strData[DATA_DESC1];
        mvServer.push_back(node);
        String strRow = "~#ff00ff00";
             if (strData[DATA_NAME].find("Dev")       != String::npos) strRow = "~#ffff0000";
        else if (strData[DATA_NAME].find("Test")      != String::npos) strRow = "~#ffffff00";
        else if (strData[DATA_NAME].find("Localhost") != String::npos) strRow = "~#ffffffff";
        strRow+= node->name+ "~;";
        strRow+= "~#ffffffa8" + node->ip +"~ (Version: " + node->version + ");";
        strRow+= "~#ffffffff" + strData[DATA_INFO];
        strRow+= (node->player <0)?",~#ffffffa8  -":",~#ffffffa8  "+strData[DATA_PLAYER];
        GuiManager::getSingleton().addLine(GuiManager::TABLE, strRow.c_str());
        // Next server
        ServerStart = ++ServerEnd;
    }
}

//================================================================================================
// .
//================================================================================================
const char *Network::get_metaserver_info(int node, int infoLineNr)
{
    PROFILE()
    return mvServer[node]->desc[infoLineNr &3].c_str();
}

//================================================================================================
// .
//================================================================================================
void Network::clearMetaServerData()
{
    PROFILE()
    GuiManager::getSingleton().clear(GuiManager::TABLE);
    if (mvServer.empty()) return;
    for (std::vector<Server*>::iterator i = mvServer.begin(); i != mvServer.end(); ++i)
        delete(*i);
    mvServer.clear();
}
