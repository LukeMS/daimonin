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

#include <Ogre.h>
#include "network.h"
#include "logfile.h"
#include "option.h"

using namespace Ogre;

#define DEBUG_ON

char *ServerName = "127.0.0.1";
int ServerPort = 13327;

int SoundStatus=1;

#define MAP_MAX_SIZE    17 
int MapStatusX =MAP_MAX_SIZE;
int MapStatusY =MAP_MAX_SIZE;



#define MAXMETAWINDOW 14        /* count max. shown server in meta window*/ 

// ========================================================================
// Return the instance.
// ========================================================================
Network &Network::getSingelton()
{
   static Network singelton;
   return singelton;
}

// ========================================================================
// Constructor.
// ========================================================================
Network::Network()
{
	mSocket = SOCKET_NO;
    mInbuf.buf = new unsigned char[MAXSOCKBUF+1];
	mInbuf.buf[0] =0;
	mInbuf.buf[1] =0;
    mGameStatusVersionFlag  = false; 
    mGameStatusVersionOKFlag= true;
}

// ========================================================================
// Destructor.
// ========================================================================
Network::~Network()
{
	delete[] mInbuf.buf;
}



// Ensures that the username doesn't contain any invalid character
static inline int is_username_valid(const char *name)
{
    for(int i=0; i< (int)strlen(name); i++)
    {
        if (!(((name[i] <= 90) && (name[i]>=65))||((name[i] >= 97) && (name[i]<=122))))
            return 0;
    }
    return 1;
} 



// Sends a reply to the server.  text contains the null terminated
// string of text to send.  This function basically just packs
// the stuff up.
void Network::send_reply(char *text)
{
    char    buf[MAXSOCKBUF];
    sprintf(buf, "reply %s", text);
    cs_write_string(buf, strlen(buf));
}

 


// ========================================================================
// Return the instance.
// ========================================================================
void Network::Update()
{
	static char buf[1024];
    ///////////////////////////////////////////////////////////////////////// 
    // Connected:
	/////////////////////////////////////////////////////////////////////////
    if (Option::getSingelton().GameStatus > GAME_STATUS_CONNECT)
    {
        if (mSocket == SOCKET_NO)
		{
            // connection closed, so we go back to INIT here
            if (Option::getSingelton().GameStatus == GAME_STATUS_PLAY)
            {
                Option::getSingelton().GameStatus = GAME_STATUS_INIT;
                mPasswordAlreadyAsked = 0;
            }
            else
			{
                 Option::getSingelton().GameStatus = GAME_STATUS_START;
			}
        }
        else
        {
			DoClient();
            //request_face(0, 1); // flush face request buffer
        }
    }

    ///////////////////////////////////////////////////////////////////////// 
    // Not connected: walk through connection chain and/or wait for action
	/////////////////////////////////////////////////////////////////////////
    if (Option::getSingelton().GameStatus != GAME_STATUS_PLAY)
    {
         // autoinit or reset prg data
        if (Option::getSingelton().GameStatus == GAME_STATUS_INIT)
		{
//           clear_metaserver_data();
             Option::getSingelton().GameStatus = GAME_STATUS_META;
		}
        // connect to meta and get server data
        else if (Option::getSingelton().GameStatus == GAME_STATUS_META)
		{
		/*
             LogFile::getSingelton().Info("GAME_STATUS_META\n");
             if (argServerName[0] != 0)
            add_metaserver_data(argServerName, argServerPort, -1, "user server",
                                "Server from -server '...' command line.", "", "", "");

        // skip of -nometa in command line or no metaserver set in options 
        if (options.no_meta || !options.metaserver[0])
        {
            draw_info("Option '-nometa'.metaserver ignored.", COLOR_GREEN);
        }
        else
        {
            draw_info("query metaserver...", COLOR_GREEN);
            sprintf(buf, "trying %s:%d", options.metaserver, options.metaserver_port);
            draw_info(buf, COLOR_GREEN);
            if (SOCKET_OpenSocket(&csocket.fd, &csocket, options.metaserver, options.metaserver_port))
            {
                read_metaserver_data();
                SOCKET_CloseSocket(csocket.fd);
                draw_info("done.", COLOR_GREEN);
            }
            else
                draw_info("metaserver failed! using default list.", COLOR_GREEN);
        }

        add_metaserver_data("127.0.0.1", 13327, -1, "local", "localhost. Start server before you try to connect.", "",
                            "", "");
        count_meta_server();
        draw_info("select a server.", COLOR_GREEN);
		*/
        Option::getSingelton().GameStatus = GAME_STATUS_START;
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_START)
    {
        if (mSocket != SOCKET_NO) CloseSocket();
        Option::getSingelton().GameStatus = GAME_STATUS_WAITLOOP;


        Option::getSingelton().GameStatus = GAME_STATUS_STARTCONNECT; // only for Testing. emulates Pressed Enter for login !!!!!!!!!!!!!

    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_STARTCONNECT)
    {
        Option::getSingelton().GameStatus = GAME_STATUS_CONNECT;
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_CONNECT)
    {
		mGameStatusVersionFlag = FALSE; 
        if (!OpenSocket(ServerName, ServerPort))
        {
            //sprintf(buf, "connection failed!");
            //draw_info(buf, COLOR_RED);
            Option::getSingelton().GameStatus = GAME_STATUS_START;
        }
        Option::getSingelton().GameStatus = GAME_STATUS_VERSION;


//        sprintf(buf, "connected. exchange version.");
//        draw_info(buf, COLOR_GREEN); 
	}
    else if (Option::getSingelton().GameStatus == GAME_STATUS_VERSION)
    {   // Send client version.
        LogFile::getSingelton().Info("Send Version\n");
        sprintf(buf, "version %d %d %s", VERSION_CS, VERSION_SC, PACKAGE_NAME);
        cs_write_string(buf, strlen(buf)); 
        Option::getSingelton().GameStatus = GAME_STATUS_WAITVERSION;
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_WAITVERSION)
    {
		LogFile::getSingelton().Info("GAME_STATUS_WAITVERSION\n");
        // perhaps here should be a timer ???
        // remember, the version exchange server<->client is asynchron
        // so perhaps the server send his version faster
        // as the client send it to server
        if (mGameStatusVersionFlag) // wait for version answer when needed
        {
            // false version!
            if (!mGameStatusVersionOKFlag)
            {
                Option::getSingelton().GameStatus = GAME_STATUS_START;
				LogFile::getSingelton().Info("GAME_STATUS_START\n");
            }
            else
            {
               // sprintf(buf, "version confirmed.\nstarting login procedure...");
               // draw_info(buf, COLOR_GREEN);
                Option::getSingelton().GameStatus = GAME_STATUS_SETUP;
				LogFile::getSingelton().Info("GAME_STATUS_SETUP\n");
            }
        }
	}
    else if (Option::getSingelton().GameStatus == GAME_STATUS_SETUP)
    {
		LogFile::getSingelton().Info("GAME_STATUS_SETUP\n");
        srv_client_files[SRV_CLIENT_SETTINGS].status = SRV_CLIENT_STATUS_OK;
        srv_client_files[SRV_CLIENT_BMAPS].status    = SRV_CLIENT_STATUS_OK;
        srv_client_files[SRV_CLIENT_ANIMS].status    = SRV_CLIENT_STATUS_OK;
        srv_client_files[SRV_CLIENT_SKILLS].status   = SRV_CLIENT_STATUS_OK;
        srv_client_files[SRV_CLIENT_SPELLS].status   = SRV_CLIENT_STATUS_OK;
        sprintf(buf,
                "setup sound %d map2cmd 1 mapsize %dx%d darkness 1 facecache 1 skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x", 
                SoundStatus, MapStatusX, MapStatusY,       srv_client_files[SRV_CLIENT_SKILLS].len,
                srv_client_files[SRV_CLIENT_SKILLS].crc,   srv_client_files[SRV_CLIENT_SPELLS].len,
                srv_client_files[SRV_CLIENT_SPELLS].crc,   srv_client_files[SRV_CLIENT_BMAPS].len,
                srv_client_files[SRV_CLIENT_BMAPS].crc,    srv_client_files[SRV_CLIENT_SETTINGS].len,
                srv_client_files[SRV_CLIENT_SETTINGS].crc, srv_client_files[SRV_CLIENT_ANIMS].len,
                srv_client_files[SRV_CLIENT_ANIMS].crc);
//        cs_write_string(buf, strlen(buf));
//		LogFile::getSingelton().Info(buf); LogFile::getSingelton().Info("\n");

char test[]="setup sound 1 map2cmd 1 mapsize 17x17 darkness 1 facecache 1 skf 3386|dd2527ea spf 2678|f6d05927 bpf 102062|a0cf8e35 stf 1853|81e29fc6 amf 132149|6b4136db";
cs_write_string(test, strlen(test));
LogFile::getSingelton().Info("%s\n",test);



        mRequest_file_chain = 0;
        mRequest_file_flags = 0;
        Option::getSingelton().GameStatus = GAME_STATUS_WAITSETUP;
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_REQUEST_FILES)
    {
		LogFile::getSingelton().Info("GAME_STATUS_REQUEST FILES (%d)\n", mRequest_file_chain);
        if (mRequest_file_chain == 0) // check setting list
        {
            if (srv_client_files[SRV_CLIENT_SETTINGS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                mRequest_file_chain = 1;
                RequestFile(SRV_CLIENT_SETTINGS);
            }
            else
                mRequest_file_chain = 2;
        }
        else if (mRequest_file_chain == 2) // check spell list
        {
            if (srv_client_files[SRV_CLIENT_SPELLS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                mRequest_file_chain = 3;
                RequestFile(SRV_CLIENT_SPELLS);
            }
            else
                mRequest_file_chain = 4;
        }
        else if (mRequest_file_chain == 4) // check skill list
        {
            if (srv_client_files[SRV_CLIENT_SKILLS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                mRequest_file_chain = 5;
                RequestFile(SRV_CLIENT_SKILLS);
            }
            else
                mRequest_file_chain = 6;
        }
        else if (mRequest_file_chain == 6)
        {
            if (srv_client_files[SRV_CLIENT_BMAPS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                mRequest_file_chain = 7;
                RequestFile(SRV_CLIENT_BMAPS);
            }
            else
                mRequest_file_chain = 8;
        }
        else if (mRequest_file_chain == 8)
        {
            if (srv_client_files[SRV_CLIENT_ANIMS].status == SRV_CLIENT_STATUS_UPDATE)
            {
                mRequest_file_chain = 9;
                RequestFile(SRV_CLIENT_ANIMS);
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
/*
            read_bmap_tmp();
            read_anim_tmp();
            load_settings();
*/
            mRequest_file_chain++;
        }
        else if (mRequest_file_chain == 12)
        {
            mRequest_file_chain++; // this ensure one loop tick and updating the messages 
        }
        else if (mRequest_file_chain == 13)
            Option::getSingelton().GameStatus = GAME_STATUS_ADDME;
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_ADDME)
    {
        cs_write_string("addme", 5);  // SendAddMe
	    /*
        map_transfer_flag = 0;
        cpl.name[0] = 0;
        cpl.password[0] = 0;
		*/
        Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
        // now wait for login request of the server
	}
    else if (Option::getSingelton().GameStatus == GAME_STATUS_LOGIN)
    {
		/*
        map_transfer_flag = 0;
        if (InputStringEscFlag)
        {
            sprintf(buf, "Break Login.");
            draw_info(buf, COLOR_RED);
            Option::getSingelton().GameStatus = GAME_STATUS_START;
        }
        reset_input_mode();
		*/
//        Option::getSingelton().GameStatus = GAME_STATUS_NAME;  ///!!!!! only testing !!!!
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_NAME)
    {

{
send_reply("myNameLang");
Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;  
}	
	
/*
        map_transfer_flag = 0;
        // we have a fininshed console input
        if (InputStringEscFlag)
            Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
        else if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
        {
            int check;
            check = is_username_valid(InputString);
            if (check)
            {
                strcpy(cpl.name, InputString);
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
                LOG(LOG_MSG,"Login: send name %s\n", InputString);
                send_reply(InputString);
                Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
                // now wait again for next server question
            }
            else
            {
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_WRONGNAME;
                InputStringFlag=TRUE;
                InputStringEndFlag=FALSE;
            }
        }
*/
	}
    else if (Option::getSingelton().GameStatus == GAME_STATUS_PSWD)
    {

/*	  
		map_transfer_flag = 0;
        // we have a fininshed console input
        textwin_clearhistory();
        if (InputStringEscFlag)
            Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
        else if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
        {
            strncpy(cpl.password, InputString, 39);
            cpl.password[39] = 0;   // insanity 0
            LOG(LOG_MSG, "Login: send password <*****>\n");
            send_reply(cpl.password);
            Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
            // now wait again for next server question
        }
*/
{
send_reply("myNameLang");  
Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
}	
  

	}
    else if (Option::getSingelton().GameStatus == GAME_STATUS_VERIFYPSWD)
    {

/*
        map_transfer_flag = 0;
        // we have a fininshed console input
        if (InputStringEscFlag)
            Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
        else if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
        {
            LOG(LOG_MSG, "Login: send verify password %s\n", InputString);
            send_reply(InputString);
            Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
            // now wait again for next server question
        }
*/    
{
send_reply("myNameLang");
Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
}	

	}
    else if (Option::getSingelton().GameStatus == GAME_STATUS_WAITFORPLAY)
    {
		/*
        clear_map();
        map_draw_map_clear();
        map_udate_flag = 2;
        map_transfer_flag = 1;
		*/
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_NEW_CHAR)
    {
        // map_transfer_flag = 0;
    }
    else if (Option::getSingelton().GameStatus == GAME_STATUS_QUIT)
    {
        // map_transfer_flag = 0;
    }
    return;

    }
/*
    if (Option::getSingelton().GameStatus < GAME_STATUS_REQUEST_FILES)
         show_meta_server(start_server, metaserver_start, metaserver_sel);
    else if (Option::getSingelton().GameStatus >= GAME_STATUS_REQUEST_FILES && Option::getSingelton().GameStatus < GAME_STATUS_NEW_CHAR)
        show_login_server();
    else if (Option::getSingelton().GameStatus == GAME_STATUS_NEW_CHAR)
         cpl.menustatus = MENU_CREATE;
*/
}



// ========================================================================
// Init the network.
// ========================================================================
bool Network::Init()
{
    LogFile::getSingelton().Headline("Init Network");

    LogFile::getSingelton().Info("init socket...");
    if (!InitSocket())
	{ 
		LogFile::getSingelton().Success(false);
		return false;
	}
    LogFile::getSingelton().Success(true);
/*
    LogFile::getSingelton().Info("Try to read data from server...\n");
    if (!GetServerData())
	{ 
		LogFile::getSingelton().Success(false);
		return false;
	}
*/
    return true;
}


// =====================================================================
// 
// =====================================================================
inline bool Network::InitSocket()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    int error;
    mSocket     = SOCKET_NO;
    mCs_version = 0;
    mSocketStatusErrorNr = 0;    
    WSADATA w;
    error = WSAStartup(0x0101, &w);
    if (error)
    {
        LogFile::getSingelton().Error("Init Winsockfaild: %d\n", error);
        return(FALSE);
    }
    if (w.wVersion != 0x0101)
    {
        LogFile::getSingelton().Error("Wrong WinSock version!\n");
        return(FALSE);
    }
#endif
    return true;
}


// ========================================================================
// connect to meta and get server data.
// ========================================================================
inline bool Network::GetServerData()
{
/*
    if (argServerName[0] != 0)
	{
        add_metaserver_data(argServerName, argServerPort, -1, "user server",
                                "Server from -server '...' command line.", "", "", "");
    }
    // skip of -nometa in command line or no metaserver set in options.
    if (options.no_meta || !options.metaserver[0])
	{
        draw_info("Option '-nometa'.metaserver ignored.", COLOR_GREEN);
	}
    else
	{
        draw_info("query metaserver...", COLOR_GREEN);
        sprintf(buf, "trying %s:%d", options.metaserver, options.metaserver_port);
        draw_info(buf, COLOR_GREEN);
        if (SOCKET_OpenSocket(&csocket.fd, &csocket, options.metaserver, options.metaserver_port))
		{
            read_metaserver_data();
            SOCKET_CloseSocket(csocket.fd);
            draw_info("done.", COLOR_GREEN);
		}
        else { draw_info("metaserver failed! using default list.", COLOR_GREEN); }
	}
    add_metaserver_data("127.0.0.1", 13327, -1, "local", 
		"localhost. Start server before you try to connect.", "", "", "");
    count_meta_server();
    draw_info("select a server.", COLOR_GREEN);
*/
   
  if (OpenSocket(ServerName, ServerPort))  // only testing..
//if (OpenSocket(Option::getSingelton().mMetaServer, Option::getSingelton().mMetaServerPort))
   {
       read_metaserver_data();
       CloseSocket();
   }

    if (OpenSocket(ServerName, ServerPort))
    {
      //  sprintf(buf, "connection failed!");
      ///  draw_info(buf, COLOR_RED);
      //  Option::getSingelton().GameStatus = GAME_STATUS_START;
     }
     //   Option::getSingelton().GameStatus = GAME_STATUS_VERSION;
     //   sprintf(buf, "connected. exchange version.");
     //   draw_info(buf, COLOR_GREEN);
    
    


   return true;
}

// ========================================================================
// .
// ========================================================================
inline bool Network::OpenSocket(const char *host, int port)
{
    // The way to make the sockets work on XP Home - The 'unix' style socket seems to fail inder xp home.
    mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    mInbuf.len = 0;
    mInsock.sin_family = AF_INET;
    mInsock.sin_port = htons((unsigned short) port);

    if (isdigit(*host))
	{ 
		mInsock.sin_addr.s_addr = inet_addr(host);
	}
    else
    {
        mHostbn = gethostbyname(host);
        if (mHostbn == (struct hostent *) NULL)
        {
			LogFile::getSingelton().Error("Unknown host: %s\n", host);
            mSocket = SOCKET_NO;
        return false;
        }
        memcpy(&mInsock.sin_addr, mHostbn->h_addr, mHostbn->h_length);
    }

    mCommand_sent = 0;
    mCommand_received = 0;
    mCommand_time = 0;

    unsigned long temp = 1; // non-block.
    if (ioctlsocket(mSocket, FIONBIO, &temp) == -1)
    {
        LogFile::getSingelton().Error("Error in ioctlsocket(*socket_temp, FIONBIO , &temp)\n");
        mSocket = SOCKET_NO;
        return false;
    }

    int error = 0;
    int retries = 15;

    while (connect(mSocket, (struct sockaddr *) &mInsock, sizeof(mInsock)) == SOCKET_ERROR)
    {
        Sleep(3);
        if (--retries == 0)
        {
            LogFile::getSingelton().Error("Connect Error:  %d\n", mSocketStatusErrorNr);
            mSocket = SOCKET_NO;
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

    // we got a connect here!
    int oldbufsize;
    int newbufsize = 65535, buflen = sizeof(int); 
    if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
	{
        oldbufsize = 0;
    }
    if (oldbufsize < newbufsize)
    {
        if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
        {
            setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, sizeof(&oldbufsize));
        }
    }
    LogFile::getSingelton().Info("Connected to %s:%d\n", host, port);
    return true;
}

// ========================================================================
// Close a socket.
// ========================================================================
bool Network::CloseSocket()
{
    if (mSocket == SOCKET_NO) { return true; }
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    // seems differents sockets have different way to shutdown connects??
    // win32 needs this hard way, normally you should wait for a read() == 0...
    shutdown(mSocket, SD_BOTH);
#endif
    closesocket(mSocket);
    mSocket = SOCKET_NO; 
    return true;
}

// ========================================================================
// Read data from the metaserver.
// ========================================================================
void Network::read_metaserver_data()
{
    int stat, temp =0;
    char *ptr = new char[MAX_METASTRING_BUFFER];
    char *buf = new char[MAX_METASTRING_BUFFER];

    while (1)
    {
		stat = recv(mSocket, ptr, MAX_METASTRING_BUFFER, 0);
	    if (stat <= 0) 
        {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
			   LogFile::getSingelton().Error("Error reading metaserver data!: %d\n", WSAGetLastError());
            break;
        }
        else if (stat > 0)
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
    }
    buf[temp] = 0;
	LogFile::getSingelton().Info("Get: %d bytes:\n", temp);
    //parse_metaserver_data(buf);

    delete[] ptr;
	delete[] buf;
}


// ========================================================================
// Takes a string of data, and writes it out to the socket. A very handy
// shortcut function.
// ========================================================================
int Network::cs_write_string(char *buf, int len)
{
    static SockList sl;

    sl.len = len;
    sl.buf = (unsigned char *) buf;
    return send_socklist(sl);
} 

int Network::send_socklist(SockList &msg)
{
    unsigned char sbuf[2];

    sbuf[0] = ((unsigned int) (msg.len) >> 8) & 0xFF;
    sbuf[1] = ((unsigned int) (msg.len)     ) & 0xFF;
    write_socket(sbuf, 2);
    return write_socket(msg.buf, msg.len);
}


// ========================================================================
// 
// ========================================================================
int Network::write_socket(unsigned char *buf, int len)
{
    int amt = 0;
    unsigned char *pos = buf;

    // If we manage to write more than we wanted, take it as a bonus
    while (len > 0)
    {
        amt = send(mSocket, (char*)pos, len, 0);

        if (amt == -1 && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            LogFile::getSingelton().Error("New socket write failed (wsb) (%d).\n", WSAGetLastError());
            //draw_info("SOCKET ERROR: Server write failed.", COLOR_RED);                
            return -1;
        }
        if (amt == 0)
        {
            LogFile::getSingelton().Error("Write_To_Socket: No data written out (%d).\n", WSAGetLastError());
            //draw_info("SOCKET ERROR: No data written out", COLOR_RED);                
            return -1;
        }
        len -= amt;
        pos += amt;
    }
    return 0;
} 

// ========================================================================
// 
// ========================================================================
void Network::DoClient()
{
    const int OFFSET = 3;  // 2 byte package len + 1 byte binary cmd.
    int             pollret;
    struct timeval  timeout;
    fd_set          tmp_read, tmp_write, tmp_exceptions;


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
        LogFile::getSingelton().Error("Got on selectcall.\n");
        return;
    }
    if (!FD_ISSET(mSocket, &tmp_read))
	{
		return;
	}
	int i = read_socket();
    if (i <= 0)
	{   // Need to add some better logic here
        if (i < 0)
		{
            LogFile::getSingelton().Error("Got error on read socket.");
            CloseSocket();
		}
		return; // Still don't have a full packet
	}
    switch (mInbuf.buf[2])
	{
    	case  1: // BINARY_CMD_COMC
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_COMC (%d)\n", mInbuf.buf[2]); 
            #endif
         //   CompleteCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  2: // BINARY_CMD_MAP2
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_MAP2 (%d)\n", mInbuf.buf[2]); 
            #endif
         //             Map2Cmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  3: // BINARY_CMD_DRAWINFO
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_DRAWINFO (%d)\n", mInbuf.buf[2]); 
            #endif
         //             DrawInfoCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  4: // BINARY_CMD_DRAWINFO2
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_DRAWINFO2 (%d)\n", mInbuf.buf[2]); 
            #endif
         //             DrawInfoCmd2(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
        case  5: // BINARY_CMD_MAP_SCROLL
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_MAP_SCROLL (%d)\n", mInbuf.buf[2]); 
            #endif
         //             map_scrollCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  6: // BINARY_CMD_ITEMX
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_ITEMX (%d)\n", mInbuf.buf[2]); 
            #endif
         //             ItemXCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  7: // BINARY_CMD_SOUND
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_SOUND (%d)\n", mInbuf.buf[2]); 
            #endif
         //             SoundCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  8: // BINARY_CMD_TARGET
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_TARGET (%d)\n", mInbuf.buf[2]); 
            #endif
         //             TargetObject(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  9: // BINARY_CMD_UPITEM
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_UPITEM (%d)\n", mInbuf.buf[2]); 
            #endif
         //             UpdateItemCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 10: // BINARY_CMD_DELITEM
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_DELITEM (%d)\n", mInbuf.buf[2]); 
            #endif
         //             DeleteItem(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 11: // BINARY_CMD_STATS
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_STATS (%d)\n", mInbuf.buf[2]); 
            #endif
         //             StatsCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 12: // BINARY_CMD_IMAGE
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_IMAGE (%d)\n", mInbuf.buf[2]); 
            #endif
         //             ImageCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 13: // BINARY_CMD_FACE1
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_FACE1 (%d)\n", mInbuf.buf[2]); 
            #endif
         //             Face1Cmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 14: // BINARY_CMD_ANIM
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_ANIM (%d)\n", mInbuf.buf[2]); 
            #endif
         //             AnimCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 15: // BINARY_CMD_SKILLRDY
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_SKILLRDY (%d)\n", mInbuf.buf[2]); 
            #endif
         //            SkillRdyCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 16: // BINARY_CMD_PLAYER
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_PLAYER (%d)\n", mInbuf.buf[2]); 
            #endif
            PlayerCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 17: // BINARY_CMD_MAPSTATS
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_MAPSTATS (%d)\n", mInbuf.buf[2]); 
            #endif
         //             MapstatsCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 18: // BINARY_CMD_SPELL_LIST
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_SPELL_LIST (%d)\n", mInbuf.buf[2]); 
            #endif
         //             SpelllistCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		    case 19: // BINARY_CMD_SKILL_LIST
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_SKILL_LIST (%d)\n", mInbuf.buf[2]); 
            #endif
         //             SkilllistCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 20: // BINARY_CMD_GOLEMCMD
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_GOLEMCMD (%d)\n", mInbuf.buf[2]); 
            #endif
         //             GolemCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 21: // BINARY_CMD_ADDME_SUC
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_ADDME_SUC (%d)\n", mInbuf.buf[2]); 
            #endif
         //             AddMeSuccess(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 22: // BINARY_CMD_ADDME_FAIL
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_ADDME_FAIL (%d)\n", mInbuf.buf[2]); 
            #endif
         //             AddMeFail(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 23: // BINARY_CMD_VERSION
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_VERSION (%d)\n", mInbuf.buf[2]); 
            #endif
            VersionCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 24: // BINARY_CMD_BYE
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_BYE (%d)\n", mInbuf.buf[2]); 
            #endif
         //             GoodbyeCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 25: // BINARY_CMD_SETUP
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_SETUP (%d)\n", mInbuf.buf[2]); 
            #endif
            SetupCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 26: // BINARY_CMD_QUERY
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_QUERY (%d)\n", mInbuf.buf[2]); 
            #endif
            handle_query((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 27: // BINARY_CMD_DATA
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_DATA (%d)\n", mInbuf.buf[2]); 
            #endif
            DataCmd((char*)mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 28: // BINARY_CMD_NEW_CHAR
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_NEW_CHAR (%d)\n", mInbuf.buf[2]); 
            #endif
         //             NewCharCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 29: // BINARY_CMD_ITEMY
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_ITEMY (%d)\n", mInbuf.buf[2]); 
            #endif
         //             ItemYCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 30: // BINARY_CMD_GROUP
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_GROUP (%d)\n", mInbuf.buf[2]); 
            #endif
         //             GroupCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 31: // BINARY_CMD_INVITE
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_INVITE (%d)\n", mInbuf.buf[2]); 
            #endif
         //             GroupInviteCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case 32: // BINARY_CMD_GROUP_UPDATE
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_GROUP_UPDATE (%d)\n", mInbuf.buf[2]); 
            #endif
         //             GroupUpdateCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
        default: // ERROR
            #ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: <UNKNOWN> (%d)\n", mInbuf.buf[2]); 
            #endif
         //             LogFile::getSingelton().Error("Bad command from server (%d)\n", mInbuf.buf[2]);
			break;
		}
	mInbuf.len =0;
}

// ========================================================================
// read socket.
// We make the assumption the buffer is at least 2 bytes long.
// ========================================================================
int Network::read_socket()
{
    int stat, toread, readsome = 0;
    // We already have a partial packet
    if (mInbuf.len < 2)
    {
        stat = recv(mSocket, (char*)mInbuf.buf + mInbuf.len, 2 - mInbuf.len, 0);
        if (stat < 0)
        {
            if ((stat == -1) && WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LogFile::getSingelton().Error("ReadPacket got error %d, returning -1\n", WSAGetLastError());
                //draw_info("WARNING: Lost or bad server connection.", COLOR_RED);                
                return -1;
            }
            return 0;
        }
        if (stat == 0)
        {
            //draw_info("WARNING: Server read package error.", COLOR_RED);                
            return -1;
        }
        mInbuf.len += stat;
        if (stat < 2)
            return 0;   // Still don't have a full packet
        readsome = 1;
    }

    // Figure out how much more data we need to read.  Add 2 from the
    // end of this - size header information is not included.
    toread = 2 + (mInbuf.buf[0] << 8) + mInbuf.buf[1] - mInbuf.len;
    if ((toread + mInbuf.len) > MAXSOCKBUF)
    {
        //draw_info("WARNING: Server read package error.", COLOR_RED);                
        LogFile::getSingelton().Error("SockList_ReadPacket: Want to read more bytes than will fit in buffer.\n");
        // return error so the socket is closed
        return -1;
    }
    do
    {
        stat = recv(mSocket, (char*)mInbuf.buf + mInbuf.len, toread, 0);
        if (stat < 0)
        {
            if ((stat == -1) && WSAGetLastError() != WSAEWOULDBLOCK)
            {
                LogFile::getSingelton().Error("ReadPacket got error %d, returning 0", WSAGetLastError());
                //draw_info("WARNING: Lost or bad server connection.", COLOR_RED);                
                return -1;
            }
            return 0;
        }
        if (stat == 0)
        {
            //draw_info("WARNING: Server read package error.", COLOR_RED);                
            return -1;
        }
        mInbuf.len += stat;
        toread -= stat;
        if (toread == 0) { return 1; }
        if (toread < 0)
        {
            LogFile::getSingelton().Error("SockList_ReadPacket: Read more bytes than desired.");
            //draw_info("WARNING: Server read package error.", COLOR_RED);                
            return -1;
        }
    }
    while (toread > 0);
    return 0;
} 

// ========================================================================
// .
// ========================================================================
void Network::RequestFile(int index)
{        
	char    buf[MAX_BUF];

    sprintf(buf, "rf %d", index);
    cs_write_string(buf, strlen(buf));
}

