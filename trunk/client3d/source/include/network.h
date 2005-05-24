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

#ifndef NETWORK_H
#define NETWORK_H

#include <list>

using namespace std;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

// Maximum size of any packet we expect.  Using this makes it so we don't need to
// allocated and deallocated the same buffer over and over again and the price
// of using a bit of extra memory. IT also makes the code simpler.
const int  MAXSOCKBUF            =  64*1024;
const int  MAX_METASTRING_BUFFER = 128*2013;
const int  SOCKET_NO = -1;
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
const int  VERSION_CS = 991017;
const int  VERSION_SC = 991017;
const char VERSION_NAME[] = "Daimonin SDL Client";

struct SockList
{
    int            len;
    unsigned char *buf;
};

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

////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class Network
{
  public:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
	int  mRequest_file_chain;
	int  mRequest_file_flags;
    int  mPasswordAlreadyAsked;
    bool mGameStatusVersionOKFlag;
    bool mGameStatusVersionFlag;

    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
     Network();
    ~Network();
    static Network &getSingleton() { static Network Singleton; return Singleton; } 
    bool Init();
    void RequestShutdown();
    void Shutdown();
    bool GetServerData();
	bool OpenSocket(const char *host, int port);
    bool CloseSocket();
	void Update();
    int  request_face(int pnum, int mode);
	void send_reply(char *text);
	void read_metaserver_data();
    int  cs_write_string(char *buf, int len);
    int  send_socklist(SockList &msg);
    int  read_socket();
	int  write_socket(unsigned char *buf, int len);
	void DoClient();

    // Server commands..
    void VersionCmd      (char *data, int len);
	void SetupCmd        (char *data, int len);
    void DataCmd         (char *data, int len);
    void PlayerCmd       (char *data, int len);
    void Map2Cmd         (char *data, int len);
    void NewCharCmd      (char *data, int len);
    void HandleQuery     (char *data, int len);
    void PreParseInfoStat(char *cmd);
    void RequestFile(int index);
    
    void CreatePlayerAccount();
  private:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
    // Contains the base information we use to make up a packet we want to send.
    int  mCs_version, mSc_version; // Server versions of these
    // These are used for the newer 'windowing' method of commands -
    // number of last command sent, number of received confirmation
    int mCommand_sent, mCommand_received;
    int mCommand_time; // Time (in ms) players commands currently take to execute
    int mSocketStatusErrorNr;
    SockList  mInbuf;
	int mSocket;
    list<mStructServer*> mServerList;
    
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
    Network(const Network&);  // disable copy-constructor.
    bool InitSocket();
    void clear_metaserver_data(void);
    void get_meta_server_data(int num, char *server, int *port);   
    void add_metaserver_data(const char *server, int port, int player, const char *ver, 
        const char *desc1, const char *desc2, const char *desc3, const char *desc4);
};

#endif
