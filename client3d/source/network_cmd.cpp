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

#include "network.h"
#include "logfile.h"
#include "stdio.h"
#include "option.h"
#include "zconf.h"
#include "zlib.h"
#include "xyz.h"
_srv_client_files   srv_client_files[SRV_CLIENT_FILES]; 


// ========================================================================
// Compare server and client version number.
// ========================================================================
void Network::VersionCmd(char *data, int len)
{
    char   *cp;
    char    buf[1024];

    mGameStatusVersionOKFlag = FALSE;
    mGameStatusVersionFlag = TRUE;
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
    mGameStatusVersionOKFlag = TRUE;
}

const int OFFSET = 3;  // 2 byte package len + 1 byte binary cmd.

// ========================================================================
// .
// ========================================================================
void Network::SetupCmd(char *buf, int len)
{
    int     s;
    char   *cmd, *param;
    LogFile::getSingelton().Info("Get SetupCmd: %s\n", mInbuf.buf + OFFSET); 
//    scrolldy = scrolldx = 0;
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

        if (!strcmp(cmd, "sound"))
        {
            if (!strcmp(param, "FALSE"))
            {
            }
        }
        else if (!strcmp(cmd, "skf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LogFile::getSingelton().Info("Get skf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                srv_client_files[SRV_CLIENT_SKILLS].status = SRV_CLIENT_STATUS_UPDATE;
                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;    
                        srv_client_files[SRV_CLIENT_SKILLS].server_len = atoi(param);
                        srv_client_files[SRV_CLIENT_SKILLS].server_crc = strtoul(cp + 1, NULL, 16);
                        break;
                    }
                }
            }
        }
        else if (!strcmp(cmd, "spf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LogFile::getSingelton().Info("Get spf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                srv_client_files[SRV_CLIENT_SPELLS].status = SRV_CLIENT_STATUS_UPDATE;
                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;    
                        srv_client_files[SRV_CLIENT_SPELLS].server_len = atoi(param);
                        srv_client_files[SRV_CLIENT_SPELLS].server_crc = strtoul(cp + 1, NULL, 16);
                        break;
                    }
                }
            }
        }
        else if (!strcmp(cmd, "stf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LogFile::getSingelton().Info("Get stf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                srv_client_files[SRV_CLIENT_SETTINGS].status = SRV_CLIENT_STATUS_UPDATE;
                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;    
                        srv_client_files[SRV_CLIENT_SETTINGS].server_len = atoi(param);
                        srv_client_files[SRV_CLIENT_SETTINGS].server_crc = strtoul(cp + 1, NULL, 16);
                        break;
                    }
                }
            }
        }
        else if (!strcmp(cmd, "bpf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LogFile::getSingelton().Info("Get bpf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                srv_client_files[SRV_CLIENT_BMAPS].status = SRV_CLIENT_STATUS_UPDATE;
                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;    
                        srv_client_files[SRV_CLIENT_BMAPS].server_len = atoi(param);
                        srv_client_files[SRV_CLIENT_BMAPS].server_crc = strtoul(cp + 1, NULL, 16);
                        break;
                    }
                }
            }
        }
        else if (!strcmp(cmd, "amf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LogFile::getSingelton().Info("Get amf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                srv_client_files[SRV_CLIENT_ANIMS].status = SRV_CLIENT_STATUS_UPDATE;
                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;    
                        srv_client_files[SRV_CLIENT_ANIMS].server_len = atoi(param);
                        srv_client_files[SRV_CLIENT_ANIMS].server_crc = strtoul(cp + 1, NULL, 16);
                        break;
                    }
                }
            }
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
        else
        {
            LogFile::getSingelton().Error("Got setup for a command we don't understand: %s %s\n", cmd, param);
        }
    }
    Option::getSingelton().GameStatus = GAME_STATUS_REQUEST_FILES;
}

// ========================================================================
// 
// ========================================================================
void Network::save_data_cmd_file(const char *path, char *data, int len)
{
    FILE   *stream;

    if ((stream = fopen(path, "wb")) != NULL)
    {
        if (fwrite(data, sizeof(char), len, stream) != (size_t) len)
            LogFile::getSingelton().Error("save data cmd file : write() of %s failed. (len:%d)\n", path);
        fclose(stream);
    }
    else
        LogFile::getSingelton().Error("save data cmd file : Can't open %s for write. (len:%d)\n", path, len);
}

// ========================================================================
// 
// ========================================================================
void Network::DataCmd(char *data, int len)
{
    unsigned char data_type = data[0];
    unsigned char data_comp ;
    // warning! if the uncompressed size of a incoming compressed(!) file is larger
    // as this dest_len default setting, the file is cutted and
    // the rest skiped. Look at the zlib docu for more info.
    unsigned long dest_len = 512 * 1024; 
    char *dest = new char[dest_len];
    data_comp = (data_type & DATA_PACKED_CMD);
    --len;
    ++data;

 LogFile::getSingelton().Error("ghrouegheougah\n");

    switch (data_type & ~DATA_PACKED_CMD)
    {
        case DATA_CMD_SKILL_LIST:
          // this is a server send skill list 
          // uncompress when needed and save it
          if (data_comp)
          {
              uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
              data = dest;
              len = dest_len;
          }
          ++mRequest_file_chain;
          save_data_cmd_file(FILE_CLIENT_SKILLS, data, len);
          read_skills();
          break;
        case DATA_CMD_SPELL_LIST:
          if (data_comp)
          {
              uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
              data = dest;
              len = dest_len;
          }
          ++mRequest_file_chain;
          save_data_cmd_file(FILE_CLIENT_SPELLS, data, len);
          read_spells();
          break;
        case DATA_CMD_SETTINGS_LIST:
          if (data_comp)
          {
              uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
              data = dest;
              len = dest_len;
          }
          ++mRequest_file_chain;
          save_data_cmd_file(FILE_CLIENT_SETTINGS, data, len);
          break;

        case DATA_CMD_BMAP_LIST:
          if (data_comp)
          {
              uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
              data = dest;
              len = dest_len;
          }
          ++mRequest_file_chain;
          save_data_cmd_file(FILE_CLIENT_BMAPS, data, len);
          mRequest_file_flags |= SRV_CLIENT_FLAG_BMAP;
          break;

        case DATA_CMD_ANIM_LIST:
          if (data_comp)
          {
              uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
              data = dest;
              len = dest_len;
          }
          ++mRequest_file_chain;
          save_data_cmd_file(FILE_CLIENT_ANIMS, data, len);
          mRequest_file_flags |= SRV_CLIENT_FLAG_ANIM;
          break;

        default:
          LogFile::getSingelton().Error("data cmd: unknown type %d (len:%d)\n", data_type, len);
          break;
    }
    delete[] dest;
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
        Option::getSingelton().GameStatus = GAME_STATUS_NAME;
    }
    if (strstr(cmd, "What is your password?"))
    {
        Option::getSingelton().GameStatus = GAME_STATUS_PSWD;
        mPasswordAlreadyAsked = 1;
    }
    if (strstr(cmd, "Please type your password again."))
    {
        Option::getSingelton().GameStatus = GAME_STATUS_VERIFYPSWD;
        mPasswordAlreadyAsked = 2;
    }
/*
    if (Option::getSingelton().GameStatus >= GAME_STATUS_NAME 
	 && Option::getSingelton().GameStatus <= GAME_STATUS_VERIFYPSWD)
        open_input_mode(12);
*/
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
