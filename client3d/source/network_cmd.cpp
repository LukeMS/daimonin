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

#include <fstream>
#include "network.h"
#include "logfile.h"
#include "stdio.h"
#include "option.h"
#include "zconf.h"
#include "zlib.h"
#include "define.h"
#include "option.h"
#include "textinput.h"
#include "serverfile.h"

using namespace std;

const char MAX_LEN_LOGIN_NAME = 15;

// ========================================================================
// Compare server and client version number.
// ========================================================================
void Network::VersionCmd(char *data, int len)
{
    char   *cp;
    char    buf[1024];

    mGameStatusVersionOKFlag = false;
    mGameStatusVersionFlag   =  true;
    mCs_version = atoi(data);

    // The first version is the client to server version the server wants
    // ATM, we just do for "must match".
    // Later it will be smart to define range where the differences are ok
    if (VERSION_CS != mCs_version)
    {
        sprintf(buf, "Invalid CS version (%d,%d)", VERSION_CS, mCs_version);
        //draw_info(buf, COLOR_RED);
        if (VERSION_CS > mCs_version)
            sprintf(buf, "The server is outdated!\nSelect a different one!");
        else
            sprintf(buf, "Your client is outdated!\nUpdate your client!");
        //draw_info(buf, COLOR_RED);
        LogFile::getSingelton().Error("%s\n", buf);            
        return;
    }
    cp = (char *) (strchr(data, ' '));
    if (!cp)
    {
        sprintf(buf, "Invalid version string: %s", data);
        //draw_info(buf, COLOR_RED);
        LogFile::getSingelton().Error("%s\n", buf);            
        return;
    }
    mCs_version = atoi(cp);
    if (mCs_version != VERSION_SC)
    {
        sprintf(buf, "Invalid SC version (%d,%d)", VERSION_SC, mCs_version);
        //draw_info(buf, COLOR_RED);
        LogFile::getSingelton().Error("%s\n", buf);            
        return;
    }
    cp = (char *) (strchr(cp + 1, ' '));
    if (!cp || strncmp(cp + 1, "Daimonin Server", 15))
    {
        sprintf(buf, "Invalid server name: %s", cp);
        //draw_info(buf, COLOR_RED);
        LogFile::getSingelton().Error("%s\n", buf);            
        return;
    }

    LogFile::getSingelton().Info("Playing on server type %s\n", cp);
    mGameStatusVersionOKFlag = true;
}

// ========================================================================
// Server has send the setup command..
// ========================================================================
void Network::SetupCmd(char *buf, int len)
{
    const int OFFSET = 3;  // 2 byte package len + 1 byte binary cmd.
    int     s, f;
    char   *cmd, *param;
    LogFile::getSingelton().Info("Get SetupCmd: %s\n", mInbuf.buf + OFFSET); 
    for (s = 0; ;)
    {
		// command.
        while (buf[s] == ' ') { ++s; }
        if (s >= len)         { break; }
        cmd = &buf[s];
        for (; buf[s] && buf[s] != ' '; ++s) { ; }
        buf[s++] = 0;

		// parameter.
        while (buf[s] == ' ') { ++s; }
        if (s >= len)         { break; }
        param = &buf[s];
        for (; buf[s] && buf[s] != ' '; ++s) { ; }
        buf[s++] = 0;

        // skip whitspaces.
        while (buf[s] == ' ') { ++s; }

        // parse the command.
        if      (!strcmp(cmd, "sound"))
		{
        }
        else if (!strcmp(cmd, "mapsize"))
        {
        }
        else if (!strcmp(cmd, "map2cmd"))
        {
        }
        else if (!strcmp(cmd, "darkness"))
        {
        }
        else if (!strcmp(cmd, "facecache"))
        {
        }
        for (f=0; f< SERVER_FILE_SUM; f++)
		{
            if (!ServerFile::getSingelton().checkID(f, cmd)) { continue; }
            if (!strcmp(param, "FALSE"))
            {
                LogFile::getSingelton().Info("Get %s: %s\n", cmd, param);
            }
            else if (strcmp(param, "OK"))
            {
                ServerFile::getSingelton().setStatus(f, SERVER_FILE_STATUS_UPDATE);
                for (char *cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;    
                        ServerFile::getSingelton().setLength(f, atoi(param));
                        ServerFile::getSingelton().setCRC   (f, strtoul(cp + 1, NULL, 16));
                        break;
                    }
                }
            }
        }
		if (f == SERVER_FILE_SUM-1)
        {
            LogFile::getSingelton().Error("Got setup for a command we don't understand: %s %s\n", cmd, param);
        }
    }
    Option::getSingelton().GameStatus = GAME_STATUS_REQUEST_FILES;
}

// ========================================================================
// Server has send us a file: uncompress and save it. 
// ========================================================================
void Network::DataCmd(char *data, int len)
{
    ///////////////////////////////////////////////////////////////////////// 
    // check for valid command:
    // 0 = NC, 1 = SERVER_FILE_SKILLS, 2 = SERVER_FILE_SPELLS, (...)
    ///////////////////////////////////////////////////////////////////////// 
    unsigned char data_type = data[0];
	unsigned char data_cmd  = (data_type &~DATA_PACKED_CMD) -1; 
    if (data_cmd > SERVER_FILE_SUM)
	{
        LogFile::getSingelton().Error("data cmd: unknown type %d (len:%d)\n", data_type, len);
		return;
	}
    --len;
    ++data;
    
    ///////////////////////////////////////////////////////////////////////// 
    // Uncompress if needed.
	/////////////////////////////////////////////////////////////////////////
    char *dest =0;
    if (data_type & DATA_PACKED_CMD)
	{
        // warning! if the uncompressed size of a incoming compressed(!) file 
		// is larger as this dest_len default setting, the file is cutted and
        // the rest skiped. Look at the zlib docu for more info.
        unsigned long dest_len = 512 * 1024; 
        dest = new char[dest_len];
        uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
        data = dest;
        len  = dest_len;
    }
    ++mRequest_file_chain;

    ///////////////////////////////////////////////////////////////////////// 
    // Save the file.
	/////////////////////////////////////////////////////////////////////////
    ofstream out(ServerFile::getSingelton().getFilename(data_cmd), ios::out|ios::binary);
    if (!out)
	{
        LogFile::getSingelton().Error("save data cmd file : write() of %s failed. (len:%d)\n", 
		    ServerFile::getSingelton().getFilename(data_cmd));
	}					 
    else
	{
	    out.write(data, len);
	}
    if (dest) { delete[] dest; }

    ///////////////////////////////////////////////////////////////////////// 
    // Reload the new file.
	/////////////////////////////////////////////////////////////////////////
//    if (data_command-1 == SERVER_FILE_SKILLS) { read_skills(); }
//    if (data_command-1 == SERVER_FILE_SPELLS) { read_spells(); }
}


// ========================================================================
// 
// ========================================================================
void Network::PreParseInfoStat(char *cmd)
{
    // Find input name
    if (strstr(cmd, "What is your name?"))
    {
/*
        LogFile::getSingelton().Info("Login: Enter name\n");
        cpl.name[0] = 0;
        cpl.password[0] = 0;
        if (PasswordAlreadyAsked == 1)
        {
            dialog_login_warning_level = DIALOG_LOGIN_WARNING_WRONGPASS;
            PasswordAlreadyAsked = 0;
        } 
        else if (PasswordAlreadyAsked == 2)
        {
            dialog_login_warning_level = DIALOG_LOGIN_WARNING_VERIFY_FAILED;
            PasswordAlreadyAsked = 0;
        }
*/
		TextInput::getSingleton().stop();
        Option::getSingelton().GameStatus = GAME_STATUS_NAME;
		TextInput::getSingleton().startTextInput(MAX_LEN_LOGIN_NAME, false, false); // every start() needs a stop()!
    }
    if (strstr(cmd, "What is your password?"))
    {
		TextInput::getSingleton().stop();
        Option::getSingelton().GameStatus = GAME_STATUS_PSWD;
		TextInput::getSingleton().startTextInput(MAX_LEN_LOGIN_NAME); // every start() needs a stop()!
		mPasswordAlreadyAsked = 1;
    }
    if (strstr(cmd, "Please type your password again."))
    {
		TextInput::getSingleton().stop();
        Option::getSingelton().GameStatus = GAME_STATUS_VERIFYPSWD;
		TextInput::getSingleton().startTextInput(MAX_LEN_LOGIN_NAME); // every start() needs a stop()!
        mPasswordAlreadyAsked = 2;
    }
}

// ========================================================================
// 
// ========================================================================
void Network::handle_query(char *data, int len)
{
    char   *buf, *cp;
    buf = strchr(data, ' ');
    if (buf) { ++buf; }
    if (buf)
    {
        cp = buf;
        while ((buf = strchr(buf, '\n')) != NULL)
        {
            *buf++ = '\0';
            LogFile::getSingelton().Info("Received query string: %s\n", cp);
            PreParseInfoStat(cp);
            cp = buf;
        }
    }
}

// ========================================================================
// 
// ========================================================================

void Network::PlayerCmd(unsigned char *data, int len)
{
    Option::getSingelton().GameStatus = GAME_STATUS_PLAY;

/*
    char    name[MAX_BUF];
    int     tag, weight, face, i = 0, nlen;

    InputStringEndFlag = FALSE;
    tag = GetInt_String(data);
    i += 4;
    weight = GetInt_String(data + i);
    i += 4;
    face = GetInt_String(data + i);
    request_face(face, 0);
    i += 4;
    nlen = data[i++];
    memcpy(name, (const char *) data + i, nlen);

    name[nlen] = '\0';
    i += nlen;

    if (i != len)
    {
        fprintf(stderr, "PlayerCmd: lengths do not match (%d!=%d)\n", len, i);
    }
    new_player(tag, name, weight, (short) face);
    map_draw_map_clear();
    map_transfer_flag = 1;
    map_udate_flag = 2;        
    load_quickslots_entrys();
*/
}
