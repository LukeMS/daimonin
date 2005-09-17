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
 
You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <fstream>
#include "network.h"
#include "logger.h"
#include "stdio.h"
#include "option.h"
#include "zlib.h"
#include "define.h"
#include "option.h"
#include "textinput.h"
#include "serverfile.h"
#include "TileManager.h"

using namespace std;

const int  REQUEST_FACE_MAX = 250;
const char MAX_LEN_LOGIN_NAME = 15;

inline short GetShort_String(char *data)
{
  return ((((unsigned char)data[0]) << 8) + (unsigned char)data[1]);
}

// ========================================================================
// Compare server and client version number.
// ========================================================================
void Network::NewCharCmd(char *, int )
{
  Option::getSingleton().GameStatus = GAME_STATUS_NEW_CHAR;
  //    CloseSocket();
}

// ========================================================================
// Compare server and client version number.
// ========================================================================
void Network::VersionCmd(char *data, int )
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
    Logger::log().error() << buf;
    return;
  }
  cp = (char *) (strchr(data, ' '));
  if (!cp)
  {
    sprintf(buf, "Invalid version string: %s", data);
    //draw_info(buf, COLOR_RED);
    Logger::log().error() << buf;
    return;
  }
  mCs_version = atoi(cp);
  if (mCs_version != VERSION_SC)
  {
    sprintf(buf, "Invalid SC version (%d,%d)", VERSION_SC, mCs_version);
    //draw_info(buf, COLOR_RED);
    Logger::log().error() << buf;
    return;
  }
  cp = (char *) (strchr(cp + 1, ' '));
  if (!cp || strncmp(cp + 1, "Daimonin Server", 15))
  {
    sprintf(buf, "Invalid server name: %s", cp);
    //draw_info(buf, COLOR_RED);
    Logger::log().error() << buf;
    return;
  }
  Logger::log().info() << "Playing on server type " << cp;
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
  Logger::log().info() << "Get SetupCmd: " << (mInbuf.buf + OFFSET);
  for (s = 0; ;)
  {
    // command.
    while (buf[s] == ' ')
    {
      ++s;
    }
    if (s >= len)
    {
      break;
    }
    cmd = &buf[s];
    for (; buf[s] && buf[s] != ' '; ++s)
    {
      ;
    }
    buf[s++] = 0;

    // parameter.
    while (buf[s] == ' ')
    {
      ++s;
    }
    if (s >= len)
    {
      break;
    }
    param = &buf[s];
    for (; buf[s] && buf[s] != ' '; ++s)
    {
      ;
    }
    buf[s++] = 0;

    // skip whitspaces.
    while (buf[s] == ' ')
    {
      ++s;
    }

    // parse the command.
    if      (!strcmp(cmd, "sound"))
    {}
    else if (!strcmp(cmd, "mapsize"))
    {}
    else if (!strcmp(cmd, "map2cmd"))
    {}
    else if (!strcmp(cmd, "darkness"))
    {}
    else if (!strcmp(cmd, "facecache"))
    {}
    for (f=0; f< SERVER_FILE_SUM; f++)
    {
      if (!ServerFile::getSingleton().checkID(f, cmd))
      {
        continue;
      }
      if (!strcmp(param, "FALSE"))
        Logger::log().info() << "Get " << cmd << ": " << param;
      else if (strcmp(param, "OK"))
      {
        ServerFile::getSingleton().setStatus(f, SERVER_FILE_STATUS_UPDATE);
        for (char *cp = param; *cp != 0; cp++)
        {
          if (*cp == '|')
          {
            *cp = 0;
            ServerFile::getSingleton().setLength(f, atoi(param));
            ServerFile::getSingleton().setCRC   (f, strtoul(cp + 1, NULL, 16));
            break;
          }
        }
      }
    }
    if (f == SERVER_FILE_SUM-1)
      Logger::log().error()  << "Got setup for a command we don't understand: "
      << cmd << " " << param;
  }
  Option::getSingleton().GameStatus = GAME_STATUS_REQUEST_FILES;
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
    Logger::log().error()  << "data cmd: unknown type "
    << data_type << " (len:" << len << ")";
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
  ofstream out(ServerFile::getSingleton().getFilename(data_cmd), ios::out|ios::binary);
  if (!out)
    Logger::log().error()  << "save data cmd file : write() of "
    << ServerFile::getSingleton().getFilename(data_cmd)
    << "failed.";
  else
  {
    out.write(data, len);
  }
  if (dest)
  {
    delete[] dest;
  }

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
            LogFile::getSingleton().Info("Login: Enter name\n");
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
    Option::getSingleton().GameStatus = GAME_STATUS_NAME;
    TextInput::getSingleton().startTextInput(MAX_LEN_LOGIN_NAME, false, false); // every start() needs a stop()!
  }
  if (strstr(cmd, "What is your password?"))
  {
    TextInput::getSingleton().stop();
    Option::getSingleton().GameStatus = GAME_STATUS_PSWD;
    TextInput::getSingleton().startTextInput(MAX_LEN_LOGIN_NAME); // every start() needs a stop()!
    mPasswordAlreadyAsked = 1;
  }
  if (strstr(cmd, "Please type your password again."))
  {
    TextInput::getSingleton().stop();
    Option::getSingleton().GameStatus = GAME_STATUS_VERIFYPSWD;
    TextInput::getSingleton().startTextInput(MAX_LEN_LOGIN_NAME); // every start() needs a stop()!
    mPasswordAlreadyAsked = 2;
  }
}

// ========================================================================
//
// ========================================================================
void Network::HandleQuery(char *data, int)
{
  char   *buf, *cp;
  buf = strchr(data, ' ');
  if (buf)
  {
    ++buf;
  }
  if (buf)
  {
    cp = buf;
    while ((buf = strchr(buf, '\n')) != NULL)
    {
      *buf++ = '\0';
      Logger::log().info() << "Received query string:" << cp;
      PreParseInfoStat(cp);
      cp = buf;
    }
  }
}

// ========================================================================
//
// ========================================================================
void Network::PlayerCmd(char *, int)
{
  Option::getSingleton().GameStatus = GAME_STATUS_PLAY;

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

static int scrolldx, scrolldy;

// ========================================================================
//
// ========================================================================
void Network::Map2Cmd(char *data, int len)
{
  int mask, x, y, pos = 0, ext_flag, xdata;
  int ext1, ext2, ext3, probe;
  int map_new_flag = false;
  int ff0, ff1, ff2, ff3, ff_flag, xpos, ypos;
  char pname1[64], pname2[64], pname3[64], pname4[64];
  int face;

  //    if (scrolldx || scrolldy) { TileMap::getSingleton().display_mapscroll(scrolldx, scrolldy); }
  scrolldy = scrolldx = 0;
  //    TileMap::getSingleton().map_transfer_flag = 0;
  xpos = (unsigned char) data[pos++];
  if (xpos == 255) // its not xpos, its the changed map marker
  {
    map_new_flag = true;
    xpos = (unsigned char) (data[pos++]);
  }

  ypos = (unsigned char) (data[pos++]);
  /*
      if (map_new_flag) { TileMap::getSingleton().adjust_map_cache(xpos, ypos); }
      TileMap::getSingleton().MapData.posx = xpos; // map windows is from range to +MAPWINSIZE_X
      TileMap::getSingleton().MapData.posy = ypos;
  */
  while (pos < len)
  {
    ext_flag = 0;
    ext1 = ext2 = ext3 = 0;
    // first, we get the mask flag - it decribes what we now get
    mask = GetShort_String(data + pos);
    pos += 2;
    x = (mask >> 11) & 0x1f;
    y = (mask >>  6) & 0x1f;
    //LogFile::getSingleton().Info("MAPPOS: x:%.2d y:%.2d (nflag:%x)\n", x, y, map_new_flag);

    // these are the "damage tags" - shows damage an object got from somewhere.
    // ff_flag hold the layer info and how much we got here.
    // 0x08 means a damage comes from unknown or vanished source.
    // this means the object is destroyed.
    // the other flags are assigned to map layer.
    //        if ((mask & 0x3f) == 0) { TileMap::getSingleton().display_map_clearcell(x, y); }
    ext3 = ext2 = ext1 = -1;
    pname1[0] = 0;
    pname2[0] = 0;
    pname3[0] = 0;
    pname4[0] = 0;
    // the ext flag defines special layer object assigned infos.
    // Like the Zzz for sleep, paralyze msg, etc.
    if (mask & 0x20) // catch the ext. flag...
    {
      ext_flag = (unsigned int) (data[pos++]);
      if (ext_flag & 0x80) // we have player names....
      {
        char c;
        int  i, pname_flag = (unsigned int) (data[pos++]);

        if (pname_flag & 0x08) // floor ....
        {
          i = 0;
          while ((c = (char) (data[pos++])))
          {
            pname1[i++] = c;
          }
          pname1[i] = 0;
        }
        if (pname_flag & 0x04) // fm....
        {
          i = 0;
          while ((c = (char) (data[pos++])))
          {
            pname2[i++] = c;
          }
          pname2[i] = 0;
        }
        if (pname_flag & 0x02) // l1 ....
        {
          i = 0;
          while ((c = (char) (data[pos++])))
          {
            pname3[i++] = c;
          }
          pname3[i] = 0;
        }
        if (pname_flag & 0x01) // l2 ....
        {
          i = 0;
          while ((c = (char) (data[pos++])))
          {
            pname4[i++] = c;
          }
          pname4[i] = 0;
        }
      }
      if (ext_flag & 0x40) // damage add on the map
      {
        ff0 = ff1 = ff2 = ff3 = -1;
        ff_flag = (unsigned int) (data[pos++]);
        if (ff_flag & 0x8)
        {
          ff0 = GetShort_String(data + pos); pos += 2;
          //                    add_anim(ANIM_KILL, 0, 0, xpos + x, ypos + y, ff0);
        }
        if (ff_flag & 0x4)
        {
          ff1 = GetShort_String(data + pos); pos += 2;
          //                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff1);
        }
        if (ff_flag & 0x2)
        {
          ff2 = GetShort_String(data + pos); pos += 2;
          //                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff2);
        }
        if (ff_flag & 0x1)
        {
          ff3 = GetShort_String(data + pos); pos += 2;
          //                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff3);
        }
      }
      if (ext_flag & 0x08)
      {
        probe = 0;
        ext3 = (int) (data[pos++]);
        //                if (ext3 & FFLAG_PROBE) { probe = (int) (data[pos++]); }
        //                TileMap::getSingleton().set_map_ext(x, y, 3, ext3, probe);
      }
      if (ext_flag & 0x10)
      {
        probe = 0;
        ext2 = (int) (data[pos++]);
        //if (ext2 & FFLAG_PROBE) { probe = (int) (data[pos++]); }
        //                TileMap::getSingleton().set_map_ext(x, y, 2, ext2, probe);
      }
      if (ext_flag & 0x20)
      {
        probe = 0;
        ext1 = (int) (data[pos++]);
        //                if (ext1 & FFLAG_PROBE) { probe = (int) (data[pos++]); }
        //                TileMap::getSingleton().set_map_ext(x, y, 1, ext1, probe);
      }
    }
    if (mask & 0x10)
    {
      //            TileMap::getSingleton().set_map_darkness(x, y, (unsigned int) (data[pos]));
      pos++;
    }
    // at last, we get the layer faces. A set ext_flag here marks this entry as face from
    // a multi tile arch. We got another byte then which all information we need to display
    // this face in the right way (position and shift offsets)
    if (mask & 0x8)
    {
      face = GetShort_String(data + pos); pos += 2;
      //            request_face(face, 0);
      xdata = 0;
      //            TileMap::getSingleton().set_map_face(x, y, 0, face, xdata, -1, pname1);
      Logger::log().info() << "MAPPOS: x:" << x << " y:" << y << " face: " << face;
    }
    if (mask & 0x4)
    {
      face = GetShort_String(data + pos); pos += 2;
      //            request_face(face, 0);
      xdata = 0;
      if (ext_flag & 0x04) // we have here a multi arch, fetch head offset
      {
        xdata = (unsigned int) (data[pos]);
        pos++;
      }
      //            TileMap::getSingleton().set_map_face(x, y, 1, face, xdata, ext1, pname2);
    }
    if (mask & 0x2)
    {
      face = GetShort_String(data + pos); pos += 2;
      //            request_face(face, 0);
      xdata = 0;
      if (ext_flag & 0x02) // we have here a multi arch, fetch head offset
      {
        xdata = (unsigned int) (data[pos]);
        pos++;
      }
      //TileMap::getSingleton().set_map_face(x, y, 2, face, xdata, ext2, pname3);
    }
    if (mask & 0x1)
    {
      face = GetShort_String(data + pos); pos += 2;
      //            request_face(face, 0);
      //  CRASH!     LogFile::getSingleton().Info("we got face: %x (%x) ->%s\n", face, face&~0x8000,TileGfx::getSingleton().FaceList[face&~0x8000].name?TileGfx::getSingleton().FaceList[face&~0x8000].name:"(null)" );
      xdata = 0;
      if (ext_flag & 0x01) // we have here a multi arch, fetch head offset
      {
        xdata = (unsigned int) (data[pos]);
        pos++;
      }
      //            TileMap::getSingleton().set_map_face(x, y, 3, face, xdata, ext3, pname4);
    }
  } // more tiles
  //    TileMap::getSingleton().map_udate_flag = 2;
}

// ========================================================================
// we got a face - test we have it loaded. If not, say server "send us face cmd "
// Return: 0 - face not there, requested.  1: face requested or loaded
// This command collect all new faces and then flush it at once.
// I insert the flush command after the socket call.
// ========================================================================
int Network::request_face(int , int)
{
  return 1;
}

// ========================================================================
//
// ========================================================================
void Network::CreatePlayerAccount()
{
  char    buf[MAX_BUF];
  //   sprintf(buf, "nc %s %d %d %d %d %d %d %d", nc->char_arch[nc->gender_selected], nc->stats[0], nc->stats[1], nc->stats[2], nc->stats[3], nc->stats[4], nc->stats[5], nc->stats[6]);
  sprintf(buf, "%s", "nc human_male 14 14 13 12 12 12 12");
  cs_write_string(buf, strlen(buf));
}
