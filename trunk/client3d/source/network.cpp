/*GAME_STATUS_CONNECT
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
#include <list>
#include <string>
#include "network.h"
#include "logfile.h"
#include "option.h"
#include "define.h"
#include "dialog.h"
#include "serverfile.h"
#include "textinput.h"
#include "textwindow.h"
#include "map.h"
#include "tile_gfx.h"

#define DEBUG_ON

using namespace std;

// testing:
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
}mStructServer;

list<mStructServer*> mServerList;

int SoundStatus=1;

// ========================================================================
// Clear the MeatServer list.
// ========================================================================
void clear_metaserver_data(void)
{
	while (mServerList.size() > 0)
	{
		mStructServer *node = mServerList.front();
		if (node) { delete node; }
		mServerList.pop_front();
	}
}

// ========================================================================
// Add a MetaServer to the list.
// ========================================================================
void add_metaserver_data(const char *server, int port, int player, const char *ver, 
	const char *desc1, const char *desc2, const char *desc3, const char *desc4)
{
	mStructServer *node = new mStructServer;
	mServerList.push_back (node);
	node->player	= player;
	node->port		= port;
	node->nameip	= server;
	node->version	= ver;
	node->desc1		= desc1;
	node->desc2		= desc2;
	node->desc3		= desc3;
	node->desc4		= desc4;
}

// ========================================================================
// Get ServerName and ServerPort.
// ========================================================================
void get_meta_server_data(int num, char *server, int *port)
{
	list<mStructServer*>::const_iterator iter;
	for (iter=mServerList.begin(); num > 0; iter++)
	{
		if (iter != mServerList.end())  { return; }
		--num;
	}
	strcpy(server, (*iter)->nameip.c_str());
	*port = (*iter)->port;
}

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


// ========================================================================
// Sends a reply to the server.  text contains the null terminated string 
// of text to send.  This function basically just packs the stuff up.
// ========================================================================
void Network::send_reply(char *text)
{
	char buf[MAXSOCKBUF];
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
			// connection closed, so we go back to GAME_STATUS_INIT here.
			if (Option::getSingelton().GameStatus == GAME_STATUS_PLAY)
			{
				Option::getSingelton().GameStatus = GAME_STATUS_INIT;
				Option::getSingelton().mStartNetwork = false;
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
		///////////////////////////////////////////////////////////////////////// 
		// autoinit or reset prg data
		///////////////////////////////////////////////////////////////////////// 
		if (Option::getSingelton().GameStatus == GAME_STATUS_INIT)
		{
			clear_metaserver_data();
			LogFile::getSingelton().Info("GAME_STATUS_INIT\n");
			Option::getSingelton().GameStatus = GAME_STATUS_META;
		}

/*
    if (Option::getSingelton().GameStatus < GAME_STATUS_REQUEST_FILES)
         show_meta_server(start_server, metaserver_start, metaserver_sel);
    else if (Option::getSingelton().GameStatus >= GAME_STATUS_REQUEST_FILES && Option::getSingelton().GameStatus < GAME_STATUS_NEW_CHAR)
        show_login_server();
    else if (Option::getSingelton().GameStatus == GAME_STATUS_NEW_CHAR)
         cpl.menustatus = MENU_CREATE;
*/


		///////////////////////////////////////////////////////////////////////// 
		// connect to meta and get server data
		///////////////////////////////////////////////////////////////////////// 
		else if (Option::getSingelton().GameStatus == GAME_STATUS_META)
		{
			LogFile::getSingelton().Info("GAME_STATUS_META\n");
/*
			if (argServerName[0] != 0)
				add_metaserver_data(argServerName, argServerPort, -1, "user server", "Server from -server '...' command line.", "", "", "");
			// skip of -nometa in command line or no metaserver set in options 
			if (options.no_meta || !options.metaserver[0])
			{
				TextWin->Print("Option '-nometa'.metaserver ignored.");
			}
        else
*/
		{
			LogFile::getSingelton().Info("Query MetaServer %s on port %d\n", 
				Option::getSingelton().mMetaServer.c_str(), Option::getSingelton().mMetaServerPort);
			TextWin->Print("query metaserver...");
			sprintf(buf, "trying %s:%d", Option::getSingelton().mMetaServer.c_str(), Option::getSingelton().mMetaServerPort);
			TextWin->Print(buf);
			if (OpenSocket(Option::getSingelton().mMetaServer.c_str(),Option::getSingelton().mMetaServerPort))
			{
				read_metaserver_data();
				CloseSocket();
				TextWin->Print("done.");
			}
			else
			TextWin->Print("metaserver failed! using default list.", TXT_RED);
		}
		add_metaserver_data("127.0.0.1", 13327, -1, "local", "localhost. Start server before you try to connect.", "", "", "");
		TextWin->Print("select a server.");
		Option::getSingelton().GameStatus = GAME_STATUS_START;
	}

	///////////////////////////////////////////////////////////////////////// 
	// Go into standby.
	///////////////////////////////////////////////////////////////////////// 
	else if (Option::getSingelton().GameStatus == GAME_STATUS_START)
	{
		if (mSocket != SOCKET_NO) { CloseSocket(); }
		Option::getSingelton().GameStatus = GAME_STATUS_WAITLOOP;
	}

	///////////////////////////////////////////////////////////////////////// 
	// Wait for user to select a server.
	///////////////////////////////////////////////////////////////////////// 
	else if (Option::getSingelton().GameStatus == GAME_STATUS_WAITLOOP)
	{
		Dialog::getSingelton().visible(true);
		if ((TextInput::getSingleton().startCursorSelection(0, mServerList.size())))
		{
			list<mStructServer*>::const_iterator iter = mServerList.begin();
			for (unsigned int i=0 ; iter != mServerList.end(); ++iter)
			{
				Dialog::getSingelton().setSelText(i++, (*iter)->nameip.c_str());
				// Fill the dialog-Info field.
				if (i == TextInput::getSingleton().getSelCursorPos())
				{
					Dialog::getSingelton().setInfoText(0, (*iter)->desc1.c_str());
					Dialog::getSingelton().setInfoText(1, (*iter)->desc2.c_str());
				}
			}
			Dialog::getSingelton().UpdateLogin(DIALOG_STAGE_GET_META_SERVER);
		}
		if (TextInput::getSingleton().isCanceled())
		{
			TextInput::getSingleton().stop();
			Dialog::getSingelton().visible(false);
			Option::getSingelton().mStartNetwork = false;
			Option::getSingelton().GameStatus = GAME_STATUS_INIT;
		}
		else if (TextInput::getSingleton().isFinished())
		{
			TextInput::getSingleton().stop();
			Option::getSingelton().mSelectedMetaServer = TextInput::getSingleton().getSelCursorPos();
			Option::getSingelton().GameStatus = GAME_STATUS_STARTCONNECT;
		}
		if (TextInput::getSingleton().getChange())
		{
			list<mStructServer*>::const_iterator iter = mServerList.begin();
			for (unsigned int i=0 ; i < TextInput::getSingleton().getSelCursorPos(); ++i) { ++iter; }
			Dialog::getSingelton().setInfoText(0, (*iter)->version.c_str(), TXT_WHITE);
			Dialog::getSingelton().setInfoText(1, (*iter)->desc1.c_str());
			Dialog::getSingelton().setInfoText(2, "");
			Dialog::getSingelton().setInfoText(3, "");
			Dialog::getSingelton().UpdateLogin(DIALOG_STAGE_GET_META_SERVER);
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Try the selected server.
	///////////////////////////////////////////////////////////////////////
	else if (Option::getSingelton().GameStatus == GAME_STATUS_STARTCONNECT)
	{
		Option::getSingelton().GameStatus = GAME_STATUS_CONNECT;
	}
	else if (Option::getSingelton().GameStatus == GAME_STATUS_CONNECT)
	{
		///////////////////////////////////////////////////////////////////////// 
		// This Server was selected in the dialog-window.
		///////////////////////////////////////////////////////////////////////// 
		mGameStatusVersionFlag = false;
		Dialog::getSingelton().clearInfoText();
		list<mStructServer*>::const_iterator iter = mServerList.begin();
		for (unsigned int i=0 ; i < Option::getSingelton().mSelectedMetaServer; ++i) { ++iter; }
		if (!OpenSocket((char*)(*iter)->nameip.c_str(), (*iter)->port))
		{
			TextWin->Print("connection failed!", TXT_RED);
			Option::getSingelton().GameStatus = GAME_STATUS_START;
		}
		else
		{
			Option::getSingelton().GameStatus = GAME_STATUS_VERSION;
			TextWin->Print("Connected. exchange version.");
		}
	}
	else if (Option::getSingelton().GameStatus == GAME_STATUS_VERSION)
	{   // Send client version.
		LogFile::getSingelton().Info("Send Version\n");
		sprintf(buf, "version %d %d %s", VERSION_CS, VERSION_SC, VERSION_NAME);
		cs_write_string(buf, strlen(buf)); 
		Option::getSingelton().GameStatus = GAME_STATUS_WAITVERSION;
	}
	else if (Option::getSingelton().GameStatus == GAME_STATUS_WAITVERSION)
	{
		LogFile::getSingelton().Info("GAME_STATUS_WAITVERSION\n");
		// perhaps here should be a timer ???
		// remember, the version exchange server<->client is asynchron so perhaps
		// the server send his version faster as the client send it to server.
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
				TextWin->Print("version confirmed.");
				TextWin->Print("starting login procedure...");
				Option::getSingelton().GameStatus = GAME_STATUS_SETUP;
				LogFile::getSingelton().Info("GAME_STATUS_SETUP\n");
			}
		}
	}
	else if (Option::getSingelton().GameStatus == GAME_STATUS_SETUP)
	{
		ServerFile::getSingelton().checkFiles();  
		sprintf(buf, "setup sound %d map2cmd 1 mapsize %dx%d darkness 1 facecache 1"
			" skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x", 
			SoundStatus, Map::getSingelton().MapStatusX, Map::getSingelton().MapStatusY, 
			ServerFile::getSingelton().getLength(SERVER_FILE_SKILLS),
			ServerFile::getSingelton().getCRC   (SERVER_FILE_SKILLS),
			ServerFile::getSingelton().getLength(SERVER_FILE_SPELLS),
			ServerFile::getSingelton().getCRC   (SERVER_FILE_SPELLS),
			ServerFile::getSingelton().getLength(SERVER_FILE_BMAPS),
			ServerFile::getSingelton().getCRC   (SERVER_FILE_BMAPS),
			ServerFile::getSingelton().getLength(SERVER_FILE_SETTINGS),
			ServerFile::getSingelton().getCRC   (SERVER_FILE_SETTINGS),
			ServerFile::getSingelton().getLength(SERVER_FILE_ANIMS),
			ServerFile::getSingelton().getCRC   (SERVER_FILE_ANIMS));
		cs_write_string(buf, strlen(buf));
		buf[strlen(buf)] =0;
		LogFile::getSingelton().Info("Send: setup %s\n", buf);
		mRequest_file_chain = 0;
		mRequest_file_flags = 0;
		Option::getSingelton().GameStatus = GAME_STATUS_WAITSETUP;
	}
	else if (Option::getSingelton().GameStatus == GAME_STATUS_REQUEST_FILES)
	{
		LogFile::getSingelton().Info("GAME_STATUS_REQUEST FILES (%d)\n", mRequest_file_chain);
        if (mRequest_file_chain == 0) // check setting list
        {
            if (ServerFile::getSingelton().getStatus(SERVER_FILE_SETTINGS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 1;
                RequestFile(SERVER_FILE_SETTINGS);
            }
            else
                mRequest_file_chain = 2;
        }
        else if (mRequest_file_chain == 2) // check spell list
        {
            if (ServerFile::getSingelton().getStatus(SERVER_FILE_SPELLS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 3;
                RequestFile(SERVER_FILE_SPELLS);
            }
            else
                mRequest_file_chain = 4;
        }
        else if (mRequest_file_chain == 4) // check skill list
        {
            if (ServerFile::getSingelton().getStatus(SERVER_FILE_SPELLS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 5;
                RequestFile(SERVER_FILE_SKILLS);
            }
            else
                mRequest_file_chain = 6;
        }
        else if (mRequest_file_chain == 6)
        {
            if (ServerFile::getSingelton().getStatus(SERVER_FILE_BMAPS) == SERVER_FILE_STATUS_UPDATE)
            {
                mRequest_file_chain = 7;
                RequestFile(SERVER_FILE_BMAPS);
            }
            else
                mRequest_file_chain = 8;
        }
        else if (mRequest_file_chain == 8)
        {
            if (ServerFile::getSingelton().getStatus(SERVER_FILE_ANIMS) == SERVER_FILE_STATUS_UPDATE)
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

//            TileGfx::getSingelton().read_bmap_tmp();
//            TileGfx::getSingelton().read_anim_tmp();
//            load_settings();

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
		// map_transfer_flag = 0;
		TextInput::getSingleton().startTextInput(1); // every start() needs a stop()!
		if (TextInput::getSingleton().isCanceled())
		{
			TextWin->Print("Break Login.", TXT_RED);
			TextInput::getSingleton().stop();
			Dialog::getSingelton().visible(false);
			Option::getSingelton().GameStatus = GAME_STATUS_START;
		}
	}
	else if (Option::getSingelton().GameStatus == GAME_STATUS_NAME)
	{
		// map_transfer_flag = 0;
		Dialog::getSingelton().UpdateLogin(DIALOG_STAGE_LOGIN_GET_NAME);
		if (TextInput::getSingleton().isCanceled())
		{
			Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
		}
		else if (TextInput::getSingleton().isFinished())
		{
			//strcpy(cpl.name, InputString);
			send_reply((char*)TextInput::getSingleton().getString());
			Dialog::getSingelton().setWarning(DIALOG_WARNING_NONE);
			Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
		}
	}
	else if (Option::getSingelton().GameStatus == GAME_STATUS_PSWD)
	{
		// map_transfer_flag = 0;
        // textwin_clearhistory();
		Dialog::getSingelton().UpdateLogin(DIALOG_STAGE_LOGIN_GET_PASSWD);
		if (TextInput::getSingleton().isCanceled())
		{
			Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
		}
        else if (TextInput::getSingleton().isFinished())
        {
            // strncpy(cpl.password, InputString, 39);
            send_reply((char*)TextInput::getSingleton().getString());
            Dialog::getSingelton().setWarning(DIALOG_WARNING_NONE);
            Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
        }
	}
    else if (Option::getSingelton().GameStatus == GAME_STATUS_VERIFYPSWD)
    {
		// map_transfer_flag = 0;
		Dialog::getSingelton().UpdateLogin(DIALOG_STAGE_LOGIN_GET_PASSWD_AGAIN);
		if (TextInput::getSingleton().isCanceled())
		{
			Option::getSingelton().GameStatus = GAME_STATUS_LOGIN;
		}
        else if (TextInput::getSingleton().isFinished())
        {
            send_reply((char*)TextInput::getSingleton().getString());
            Dialog::getSingelton().setWarning(DIALOG_WARNING_NONE);
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
	return true;
}

// =====================================================================
// Check for Winsock.
// =====================================================================
inline bool Network::InitSocket()
{
	#ifdef WIN32
    int error;
    mSocket = SOCKET_NO;
    mCs_version = 0;
    mSocketStatusErrorNr = 0;
    WSADATA w;
    error = WSAStartup(0x0101, &w);
    if (error)
    {
        LogFile::getSingelton().Error("Init Winsock failed: %d\n", error);
        return false;
    }
    if (w.wVersion != 0x0101)
    {
        LogFile::getSingelton().Error("Wrong WinSock version!\n");
		return false;
	}
	#endif
	return true;
}

// ========================================================================
// Open a scoket to "host" on "port"
// ========================================================================
inline bool Network::OpenSocket(const char *host, int port)
{
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
        if (mHostbn == (struct hostent *) NULL)
        {
            LogFile::getSingelton().Error("Unknown host: %s\n", host);
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

	#else
	struct protoent *protox;
	protox = getprotobyname("tcp");
	if (!protox)
	{
		LogFile::getSingelton().Error("Error an getting ProtoByName (tcp)\n");
		return false;
	}
	mSocket = socket(PF_INET, SOCK_STREAM, protox->p_proto);
	if (mSocket == -1)
	{
		LogFile::getSingelton().Error("Init connection: Error on socket command.\n");
		mSocket = SOCKET_NO;
		return false;
	}
	if (connect(mSocket,(struct sockaddr *)&mInsock,sizeof(mInsock)) ==  SOCKET_ERROR)
	{
		LogFile::getSingelton().Error("Can't connect to server");
		return false;
	}
	if (fcntl(mSocket, F_SETFL, O_NDELAY) == SOCKET_ERROR)
	{
		LogFile::getSingelton().Error("InitConnection:  Error on fcntl.\n");
	}
	#endif

	///////////////////////////////////////////////////////////////////////
	// we got a connect here!
	///////////////////////////////////////////////////////////////////////
	int oldbufsize;
	int newbufsize = 65535, buflen = sizeof(int); 
	#ifdef WIN32
	if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &buflen) == -1)
	#else
	socklen_t socklen= buflen;
	if (getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &oldbufsize, &socklen) == -1)
	#endif
	{
		oldbufsize = 0;
	}
	if (oldbufsize < newbufsize)
	{
		if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *) &newbufsize, sizeof(&newbufsize)))
		{
			LogFile::getSingelton().Error("InitConnection: setsockopt unable to set output buf size to %d\n", newbufsize);
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

// ========================================================================
// Read data from the metaserver.
// ========================================================================
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
			LogFile::getSingelton().Error("Error reading metaserver data!: %d\n", WSAGetLastError());
			break;
		}
		#else
		do {stat = recv (mSocket, buffer, MAX_METASTRING_BUFFER, 0); }
		while (stat == -1);
		#endif
		if(stat == 0)	{ break; } // connect closed by meta
		if(stat > 0)		{ strMetaData += buffer; }
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// Parse the metadata.
	/////////////////////////////////////////////////////////////////////////////////////////
	LogFile::getSingelton().Info("GET: %s\n", strMetaData.c_str());
	unsigned int startPos=0, endPos;
	string strIP, str1, strName, strPlayer, strVersion, strDesc1, strDesc2, strDesc3, strDesc4;
	// Server IP.
	endPos = strMetaData.find( '|',  startPos);
	strIP = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// unknown 1.
	endPos = strMetaData.find( '|',  startPos);
	str1 = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// Server name.
	endPos = strMetaData.find( '|',  startPos);
	strName = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// Number of players online.
	endPos = strMetaData.find( '|',  startPos);
	strPlayer = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// Server version.
	endPos = strMetaData.find( '|',  startPos);
	strVersion = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// Description1
	endPos = strMetaData.find( '|',  startPos);
	strDesc1 = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// Description2.
	endPos = strMetaData.find( '|',  startPos);
	strDesc2 = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// Description3.
	endPos = strMetaData.find( '|',  startPos);
	strDesc3 = strMetaData.substr(startPos, endPos-startPos);
	startPos = endPos+1;
	// Description4.
	strDesc4 = strMetaData.substr(startPos, strMetaData.size()-startPos);

	const int port = 13327;
	add_metaserver_data(strName.c_str(), port, atoi(strPlayer.c_str()), strVersion.c_str(),
		strDesc1.c_str(),  strDesc2.c_str(),  strDesc3.c_str(),  strDesc4.c_str());
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

// ========================================================================
//
// ========================================================================
int Network::send_socklist(SockList &msg)
{
	unsigned char sbuf[2];
	sbuf[0] = ((unsigned int) (msg.len) >> 8) & 0xFF;
	sbuf[1] = ((unsigned int) (msg.len)     ) & 0xFF;
	write_socket(sbuf, 2);
	return write_socket(msg.buf, msg.len);
}

// ========================================================================
// Write socket.
// ========================================================================
int Network::write_socket(unsigned char *buf, int len)
{
	int amt = 0;
	unsigned char *pos = buf;

	//LogFile::getSingelton().Error("write socket befehl: %s %d\n",  buf, len);
	// If we manage to write more than we wanted, take it as a bonus
	while (len > 0)
	{
		#ifdef WIN32
		amt = send(mSocket, (char*)pos, len, 0);
		if (amt == -1 && WSAGetLastError() != WSAEWOULDBLOCK)
		{
			LogFile::getSingelton().Error("New socket write failed (wsb) (%d).\n", WSAGetLastError());
			TextWin->Print("SOCKET ERROR: Server write failed.", TXT_RED);
			return -1;
		}
		if (amt == 0)
		{
			LogFile::getSingelton().Error("Write_To_Socket: No data written out (%d).\n", WSAGetLastError());
			TextWin->Print("SOCKET ERROR: No data written out", TXT_RED);
			return -1;
		}
		#else
		amt = write(mSocket, pos, len);
		if (amt < 0)
		{
			if (errno==EINTR) { continue; }
			 LogFile::getSingelton().Error("New socket (fd=%d) write failed.\n", mSocket);
			TextWin->Print("SOCKET ERROR: Server write failed.", TXT_RED);
			return -1;
		}
		#endif
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
			// CompleteCmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
			break;
		case  2: // BINARY_CMD_MAP2
			#ifdef DEBUG_ON
			LogFile::getSingelton().Info("command: BINARY_CMD_MAP2 (%d)\n", mInbuf.buf[2]); 
			#endif
			// Map2Cmd(mInbuf.buf + OFFSET, mInbuf.len - OFFSET);
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
			Dialog::getSingelton().visible(false);
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
			LogFile::getSingelton().Info("command: <UNKNOWN> (%d)\n", mInbuf.buf[2]); 
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
		#ifdef WIN32
		stat=recv(mSocket, (char*)mInbuf.buf + mInbuf.len, 2 - mInbuf.len, 0);
		if (stat<0) 
		{
			if ((stat==-1) && WSAGetLastError() !=WSAEWOULDBLOCK)
			{
				LogFile::getSingelton().Error("ReadPacket got error %d, returning -1\n",WSAGetLastError());
				TextWin->Print("WARNING: Lost or bad server connection.", TXT_RED);
				return -1;
			}
			return 0;
		}
		#else
		do { stat=recv(mSocket, (char*)mInbuf.buf + mInbuf.len, 2 - mInbuf.len, 0); }
		while ((stat==-1) && (errno==EINTR));
		if (stat<0)
		{
			// In non blocking mode, EAGAIN is set when there is no data available.
			if (errno!=EAGAIN && errno!=EWOULDBLOCK)
			{
				LogFile::getSingelton().Error("ReadPacket got error %d, returning 0",errno);
				TextWin->Print("WARNING: Lost or bad server connection.", TXT_RED);
				return -1;
			}
			return 0;
		}
		#endif
		if (stat==0) 
		{
			TextWin->Print("WARNING: Server read package error.", TXT_RED);
			return -1;
		}
		mInbuf.len += stat;
		if (stat < 2) {return 0; } // Still don't have a full packet
		readsome = 1;
	}
	// Figure out how much more data we need to read.  Add 2 from the
	// end of this - size header information is not included.
	toread = 2 + (mInbuf.buf[0] << 8) + mInbuf.buf[1] - mInbuf.len;
	if ((toread + mInbuf.len) > MAXSOCKBUF)
	{
		TextWin->Print("WARNING: Server read package error.", TXT_RED);
		LogFile::getSingelton().Error("SockList_ReadPacket: Want to read more bytes than will fit in buffer.\n");
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
		do { stat = recv(mSocket, (char*)mInbuf.buf + mInbuf.len, toread, 0); }
		while ((stat<0) && (errno==EINTR));
		if (stat<0)
		{
			if (errno!=EAGAIN && errno!=EWOULDBLOCK)
			{
		#endif
				LogFile::getSingelton().Error("ReadPacket got error %d, returning 0",errno);
				TextWin->Print("WARNING: Lost or bad server connection.", TXT_RED);
				return -1;
			}
			return 0;
		}
		if (stat==0)
		{
			TextWin->Print("WARNING: Server read package error.", TXT_RED);
			return -1;
		}
		mInbuf.len += stat;
		toread -= stat;
		if (toread == 0) { return 1; }
		if (toread < 0)
		{
			LogFile::getSingelton().Error("SockList_ReadPacket: Read more bytes than desired.\n");
			TextWin->Print("WARNING: Server read package error.", TXT_RED);
			return -1;
		}
	}
	while (toread > 0);
	return 0;
}

// ========================================================================
// Request a file from server.
// ========================================================================
void Network::RequestFile(int index)
{
	char buf[MAX_BUF];
	sprintf(buf, "rf %d", index);
	cs_write_string(buf, strlen(buf));
}
