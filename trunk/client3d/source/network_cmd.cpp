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

#include <fstream>
#include "network.h"
#include "logger.h"
#include "stdio.h"
#include "option.h"
#include "zlib.h"
#include "define.h"
#include "option.h"
#include "gui_manager.h"
#include "network_serverfile.h"
#include "tile_manager.h"

using namespace std;

const int  REQUEST_FACE_MAX = 250;
const char MAX_LEN_LOGIN_NAME = 15;

std::string strTemp;

char playerName[80];
char playerPassword[80];
int scrolldx, scrolldy;

enum {MAP_UPDATE_CMD_SAME, MAP_UPDATE_CMD_NEW, MAP_UPDATE_CMD_CONNECTED};


//================================================================================================
// Ascii to int (32bit).
//================================================================================================
int GetInt_String(unsigned char *data)
{
    return ((data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3]);
}

//================================================================================================
// Ascii to short (16bit).
//================================================================================================
short GetShort_String(unsigned char *data)
{
    return ((data[0] << 8) + data[1]);
}


//================================================================================================
// .
//================================================================================================
void Network::CompleteCmd(unsigned char *data, int len)
{
    if (len != 6)
    {
        Logger::log().error() << "CompleteCmd - invalid length (" << len << ") - will be ignored";
        // is here a return; missing?
    }
    csocket.command_received = GetShort_String(data);
    csocket.command_time = GetInt_String(data + 2);
}

//================================================================================================
// .
//================================================================================================
void Network::VersionCmd(unsigned char *data, int len)
{
    char    buf[1024];
    GameStatusVersionOKFlag = false;
    GameStatusVersionFlag = true;
    csocket.cs_version = atoi((char*)data);

    // The first version is the client to server version the server wants
    // ATM, we just do for "must match".
    // Later it will be smart to define range where the differences are ok
    if (VERSION_CS != csocket.cs_version)
    {
        Logger::log().error() << "Invalid CS version (" <<  VERSION_CS << " " << csocket.cs_version << ")";
        if (VERSION_CS > csocket.cs_version)
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"The server is outdated!");
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Select a different one!");
            Logger::log().error() << "The selected server is outdated.";
        }
        else
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Your client is outdated!");
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)"Update your client!");
            Logger::log().error() << "The client is outdated.";
        }
        SOCKET_CloseSocket();
        Option::getSingleton().setGameStatus(GAME_STATUS_START);
        SDL_Delay(3250);
        return;
    }
    char *cp = (char *) (strchr((char *)data, ' '));
    if (!cp)
    {
        sprintf(buf, "Invalid version string: %s", data);
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)data);
        Logger::log().error() << data;
        SOCKET_CloseSocket();
        Option::getSingleton().setGameStatus(GAME_STATUS_START);
        SDL_Delay(3250);
        return;
    }
    csocket.sc_version = atoi(cp);
    if (csocket.sc_version != VERSION_SC)
    {
        sprintf(buf, "Invalid SC version (%d,%d)", VERSION_SC, csocket.sc_version);
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)buf);
        if (VERSION_SC > csocket.sc_version)
            sprintf(buf, "The server is outdated!\nSelect a different one!");
        else
            sprintf(buf, "Your client is outdated!\nUpdate your client!");
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)buf);
        Logger::log().error() << buf;
        SOCKET_CloseSocket();
        Option::getSingleton().setGameStatus(GAME_STATUS_START);
        SDL_Delay(3250);
        return;
    }
    cp = (char *) (strchr(cp + 1, ' '));
    if (!cp || strncmp(cp + 1, "Daimonin Server", 15))
    {
        sprintf(buf, "Invalid server name: %s", cp);
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)buf);
        Logger::log().error() << buf;
        SOCKET_CloseSocket();
        Option::getSingleton().setGameStatus(GAME_STATUS_START);
        SDL_Delay(3250);
        return;
    }
    GameStatusVersionOKFlag = true;
}

//================================================================================================
// .
//================================================================================================
void Network::DrawInfoCmd(unsigned char *data, int len)
{
//    int color   = atoi(data);
    // Todo: Convert indexed color into rgb and add it to the text.

    char *buf = strchr((char *)data, ' ');
    if (!buf)
    {
        Logger::log().error() << "DrawInfoCmd - got no data";
        buf = "";
    }
    else
        ++buf;
    GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)buf);
}

//================================================================================================
// Handles when the server says we can't be added.  In reality, we need to close the connection
// and quit out, because the client is going to close us down anyways.
//================================================================================================
void Network::AddMeFail(unsigned char *data, int len)
{
    Logger::log().error() << "addme_failed received.\n";
    SOCKET_CloseSocket();
    SDL_Delay(1250);
    Option::getSingleton().setGameStatus(GAME_STATUS_INIT_NET);
}

//================================================================================================
// .
//================================================================================================
void Network::Map2Cmd(unsigned char *data, int len)
{
    static int map_w=0, map_h=0,mx=0,my=0;
    int     mask, x, y, pos = 0, ext_flag, xdata;
    int     mapstat, ext1, ext2, ext3, probe;
    bool    map_new_flag = false;
    int     ff0, ff1, ff2, ff3, ff_flag, xpos, ypos;
    char    pname1[64], pname2[64], pname3[64], pname4[64];
    char mapname[256];
    uint16  face;

    mapstat = (data[pos++]);
    // map_transfer_flag = 0;
    if (mapstat != MAP_UPDATE_CMD_SAME)
    {
        strcpy(mapname, (char*)data + pos);
        pos += strlen(mapname)+1;
        if (mapstat == MAP_UPDATE_CMD_NEW)
        {
            //map_new_flag = TRUE;
            map_w = (uint8) (data[pos++]);
            map_h = (uint8) (data[pos++]);
            xpos =  (uint8) (data[pos++]);
            ypos =  (uint8) (data[pos++]);
            mx = xpos;
            my = ypos;
//            remove_item_inventory(locate_item(0)); // implicit clear below
//            InitMapData(mapname, map_w, map_h, xpos, ypos);
        }
        else
        {
            int xoff, yoff;
            mapstat = (char) (data[pos++]);
            xoff = (char) (data[pos++]);
            yoff = (char) (data[pos++]);
            xpos = (uint8)(data[pos++]);
            ypos = (uint8)(data[pos++]);
            mx = xpos;
            my = ypos;
//            remove_item_inventory(locate_item(0)); // implicit clear below
//            display_mapscroll(xoff, yoff);
        }
    }
    else
    {
        xpos = (uint8) (data[pos++]);
        ypos = (uint8) (data[pos++]);

        // we have moved
        if ((xpos - mx || ypos - my))
        {
//            remove_item_inventory(locate_item(0)); // implicit clear below
//            if (cpl.menustatus != MENU_NO) reset_menu_status();
        }
//        display_mapscroll(xpos - mx, ypos - my);

        mx = xpos;
        my = ypos;
    }

    if (map_new_flag)
    {
//       adjust_map_cache(xpos, ypos);
    }

//    MapData.posx = xpos; // map windows is from range to +MAPWINSIZE_X
//    MapData.posy = ypos;
    Logger::log().info() << "MapPos x: " << xpos << " y: " << ypos << " (nflag: " << map_new_flag << ")";
    while (pos < len)
    {
        ext_flag = 0;
        ext1 = ext2 = ext3 = 0;
        // first, we get the mask flag - it decribes what we now get
        mask = GetShort_String(data + pos); pos += 2;
        x = (mask >> 11) & 0x1f;
        y = (mask >> 6) & 0x1f;

        // These are the "damage tags" - shows damage an object got from somewhere.
        // ff_flag hold the layer info and how much we got here.
        // 0x08 means a damage comes from unknown or vanished source.
        // this means the object is destroyed.
        // the other flags are assigned to map layer.
        if ((mask & 0x3f) == 0)
        {
//            display_map_clearcell(x, y);
        }

        ext3 = ext2 = ext1 = -1;
        pname1[0] = 0;pname2[0] = 0;pname3[0] = 0;pname4[0] = 0;
        // the ext flag defines special layer object assigned infos.
        // Like the Zzz for sleep, paralyze msg, etc.
        if (mask & 0x20) // catch the ext. flag...
        {
            ext_flag = (uint8) (data[pos++]);

            if (ext_flag & 0x80) // we have player names....
            {
                char    c;
                int     i, pname_flag = (uint8) (data[pos++]);


                if (pname_flag & 0x08) // floor ....
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname1[i++] = c;
                    };
                    pname1[i] = 0;
                }
                if (pname_flag & 0x04) // fm....
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname2[i++] = c;
                    };
                    pname2[i] = 0;
                }
                if (pname_flag & 0x02) // l1 ....
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname3[i++] = c;
                    };
                    pname3[i] = 0;
                }
                if (pname_flag & 0x01) // l2 ....
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname4[i++] = c;
                    };
                    pname4[i] = 0;
                }
            }
            if (ext_flag & 0x40) // damage add on the map
            {
                ff0 = ff1 = ff2 = ff3 = -1;
                ff_flag = (uint8) (data[pos++]);
                if (ff_flag & 0x8)
                {
                    ff0 = GetShort_String(data + pos); pos += 2;
//                    add_anim(ANIM_KILL, 0, 0, xpos + x, ypos + y, ff0);
                }
                if (ff_flag & 0x4)
                {
                    ff1 = GetShort_String(data + pos); pos += 2;
//                   add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff1);
                }
                if (ff_flag & 0x2)
                {
                    ff2 = GetShort_String(data + pos); pos += 2;
//                   add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff2);
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
//                if (ext3 & FFLAG_PROBE)
                {
                    probe = (int) (data[pos++]);
                }

//                set_map_ext(x, y, 3, ext3, probe);
            }
            if (ext_flag & 0x10)
            {
                probe = 0;
                ext2 = (int) (data[pos++]);
//                if (ext2 & FFLAG_PROBE)
                {
                    probe = (int) (data[pos++]);
                }
//                set_map_ext(x, y, 2, ext2, probe);
            }
            if (ext_flag & 0x20)
            {
                probe = 0;
                ext1 = (int) (data[pos++]);
//                if (ext1 & FFLAG_PROBE)
                {
                    probe = (int) (data[pos++]);
                }
//                set_map_ext(x, y, 1, ext1, probe);
            }
        }

        if (mask & 0x10)
        {
//            set_map_darkness(x, y, (uint8) (data[pos]));
            ++pos;
        }

        // at last, we get the layer faces.
        // a set ext_flag here marks this entry as face from a multi tile arch.
        // we got another byte then which all information we need to display
        // this face in the right way (position and shift offsets)
        if (mask & 0x8)
        {
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
            xdata = 0;
//            set_map_face(x, y, 0, face, xdata, -1, pname1);
        }
        if (mask & 0x4)
        {
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
            xdata = 0;
            if (ext_flag & 0x04) // we have here a multi arch, fetch head offset
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
//            set_map_face(x, y, 1, face, xdata, ext1, pname2);
        }
        if (mask & 0x2)
        {
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
//            Logger::log().info() << "we got face: " << face << " (" << face&~0x8000 << ") -> " << FaceList[face&~0x8000].name?FaceList[face&~0x8000].name:"(null)";
            xdata = 0;
            if (ext_flag & 0x02) // we have here a multi arch, fetch head offset
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
//            set_map_face(x, y, 2, face, xdata, ext2, pname3);
        }
        if (mask & 0x1)
        {
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
//            Logger::log().info() << "we got face: " << face << " (" << face&~0x8000 << ") -> " << FaceList[face&~0x8000].name?FaceList[face&~0x8000].name:"(null)";
            xdata = 0;
            if (ext_flag & 0x01) // we have here a multi arch, fetch head offset
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
//            set_map_face(x, y, 3, face, xdata, ext3, pname4);
        }
    } // more tiles
//    map_udate_flag = 2;
}

//================================================================================================
// .
//================================================================================================
void Network::DrawInfoCmd2(unsigned char *data, int len)
{
    char *tmp= 0, buf[2048];
//    int flags = GetShort_String(data);
    data += 2;
    len -= 2;
    if (len >= 0)
    {
        if (len > 2000) len = 2000;
        strncpy(buf, (char*)data, len);
        buf[len] = 0;
    }
    else
        buf[0] = 0;
    if (buf[0])
    {
        tmp = strchr((char*)data, ' ');
        if (tmp) *tmp = 0;
    }
    // we have communication input
    GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)buf); // TESTING!!!
    /*
        if (tmp && flags & (NDI_PLAYER|NDI_SAY|NDI_SHOUT|NDI_TELL|NDI_GSAY|NDI_EMOTE))
        {
            if ( !(flags & NDI_GM) && ignore_check(data))
                return;

            // save last incomming tell player for client sided /reply
            if (flags & NDI_TELL)
                strcpy(cpl.player_reply, data);

            //Logger::log().info() << "IGNORE?: player >" << data << "<";
            if (flags & NDI_EMOTE)
                flags &= ~NDI_PLAYER;
        }
        draw_info(buf, flags);
    */
}

//================================================================================================
// ItemXCmd is ItemCmd with sort order normal (add to end.
//================================================================================================
void Network::ItemXCmd(unsigned char *data, int len)
{
    // ItemXYCmd(data, len, false);
}

//================================================================================================
// .
//================================================================================================

#define SOUND_NORMAL    0
#define SOUND_SPELL     1

/* music mode - controls how the music is played and started */
#define MUSIC_MODE_NORMAL 1
#define MUSIC_MODE_DIRECT 2
#define MUSIC_MODE_FORCED 4 /* thats needed for some map event sounds */

// sound ids. //
typedef enum _sound_id
{
    SOUND_EVENT01,
    SOUND_BOW01,
    SOUND_LEARNSPELL,
    SOUND_FAILSPELL,
    SOUND_FAILROD,
    SOUND_DOOR,
    SOUND_PUSHPLAYER,
    SOUND_HIT_IMPACT,
    // 8
    SOUND_HIT_CLEAVE,
    SOUND_HIT_SLASH,
    SOUND_HIT_PIERCE,
    SOUND_HIT_BLOCK,
    SOUND_HIT_HAND,
    SOUND_MISS_MOB1,
    SOUND_MISS_MOB2,
    SOUND_PETDEAD,
    // 16
    SOUND_PLAYERDEAD,
    SOUND_EXPLOSION00,
    SOUND_EXPLOSION01,
    SOUND_KILL,
    SOUND_PULLLEVER,
    SOUND_FALLHOLE,
    SOUND_POISON,
    SOUND_DROP,
    // 24
    SOUND_LOSE_SOME,
    SOUND_THROW,
    SOUND_GATE_OPEN,
    SOUND_GATE_CLOSE,
    SOUND_OPEN_CONTAINER,
    SOUND_GROWL,
    SOUND_ARROW_HIT,
    SOUND_DOOR_CLOSE,
    SOUND_TELEPORT,
    SOUND_SCROLL,
    // here we have client side sounds - add server sounds BEFORE this.
    SOUND_STEP1,
    SOUND_STEP2,
    SOUND_PRAY,
    SOUND_CONSOLE,
    SOUND_CLICKFAIL,
    SOUND_CHANGE1,
    SOUND_WARN_FOOD,
    SOUND_WARN_DRAIN,
    SOUND_WARN_STATUP,
    SOUND_WARN_STATDOWN,
    SOUND_WARN_HP,
    SOUND_WARN_HP2,
    SOUND_WEAPON_ATTACK,
    SOUND_WEAPON_HOLD,
    SOUND_GET,
    SOUND_BOOK,
    SOUND_PAGE,
    SOUND_MAX
};

// to call a spell sound here, do
// SOUND_MAX + SOUND_MAGIC_xxx

// this enum should be same as in server //
typedef enum _spell_sound_id
{
    SOUND_MAGIC_DEFAULT,
    SOUND_MAGIC_ACID,
    SOUND_MAGIC_ANIMATE,
    SOUND_MAGIC_AVATAR,
    SOUND_MAGIC_BOMB,
    SOUND_MAGIC_BULLET1,
    SOUND_MAGIC_BULLET2,
    SOUND_MAGIC_CANCEL,
    SOUND_MAGIC_COMET,
    SOUND_MAGIC_CONFUSION,
    SOUND_MAGIC_CREATE,
    SOUND_MAGIC_DARK,
    SOUND_MAGIC_DEATH,
    SOUND_MAGIC_DESTRUCTION,
    SOUND_MAGIC_ELEC,
    SOUND_MAGIC_FEAR,
    SOUND_MAGIC_FIRE,
    SOUND_MAGIC_FIREBALL1,
    SOUND_MAGIC_FIREBALL2,
    SOUND_MAGIC_HWORD,
    SOUND_MAGIC_ICE,
    SOUND_MAGIC_INVISIBLE,
    SOUND_MAGIC_INVOKE,
    SOUND_MAGIC_INVOKE2,
    SOUND_MAGIC_MAGIC,
    SOUND_MAGIC_MANABALL,
    SOUND_MAGIC_MISSILE,
    SOUND_MAGIC_MMAP,
    SOUND_MAGIC_ORB,
    SOUND_MAGIC_PARALYZE,
    SOUND_MAGIC_POISON,
    SOUND_MAGIC_PROTECTION,
    SOUND_MAGIC_RSTRIKE,
    SOUND_MAGIC_RUNE,
    SOUND_MAGIC_SBALL,
    SOUND_MAGIC_SLOW,
    SOUND_MAGIC_SNOWSTORM,
    SOUND_MAGIC_STAT,
    SOUND_MAGIC_STEAMBOLT,
    SOUND_MAGIC_SUMMON1,
    SOUND_MAGIC_SUMMON2,
    SOUND_MAGIC_SUMMON3,
    SOUND_MAGIC_TELEPORT,
    SOUND_MAGIC_TURN,
    SOUND_MAGIC_WALL,
    SOUND_MAGIC_WALL2,
    SOUND_MAGIC_WOUND,
    SPELL_SOUND_MAX
};

void Network::SoundCmd(unsigned char *data, int len)
{
    if (len != 5)
    {
        Logger::log().error() << "Got invalid length on sound command: " << len;
        return;
    }
    int x = (char) data[0];
    int y = (char) data[1];
    int num = GetShort_String(data + 2);
    int type = data[4];
    if (type == SOUND_SPELL)
    {
        if (num < 0 || num >= SPELL_SOUND_MAX)
        {
            Logger::log().error() << "Got invalid spell sound id: " <<  num;
            return;
        }
        num += SOUND_MAX; // this maps us to the spell sound table part
    }
    else
    {
        if (num < 0 || num >= SOUND_MAX)
        {
            Logger::log().error() << "Got invalid sound id: " << num;
            return;
        }
    }
//    calculate_map_sound(num, x, y, 0);
    Logger::log().warning() << "Play sound: " << num << " posX: " << x << " posY: " << y;
}

//================================================================================================
//================================================================================================
// .
//================================================================================================
void Network::TargetObject(unsigned char *data, int len)
{
    /*
        cpl.target_mode = *data++;
        if (cpl.target_mode)
            sound_play_effect(SOUND_WEAPON_ATTACK, 0, 0, 100);
        else
            sound_play_effect(SOUND_WEAPON_HOLD, 0, 0, 100);
        cpl.target_color = *data++;
        cpl.target_code = *data++;
        strcpy(cpl.target_name, data);
        map_udate_flag = 2;

        sprintf(buf,"TO: %d %d >%s< (len: %d)\n",cpl.target_mode,cpl.target_code,cpl.target_name,len);
        GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN, (void*)buf);
    */
}

//================================================================================================
// UpdateItemCmd updates some attributes of an item.
//================================================================================================
void Network::UpdateItemCmd(unsigned char *data, int len)
{
    /*
        int     weight, loc, tag, face, sendflags, flags, pos = 0, nlen, anim, nrof, quality=254, condition=254;
        uint8   direction;
        char    name[MAX_BUF];
        item   *ip, *env = NULL;
        uint8   animspeed;

        map_udate_flag = 2;
        sendflags = GetShort_String(data);
        pos += 2;
        tag = GetInt_String(data + pos);
        pos += 4;
        ip = locate_item(tag);
        if (!ip)
        {
            return;
        }
        *name = '\0';
        loc = ip->env ? ip->env->tag : 0;
        // Logger::log().error() <<  "UPDATE: loc: "<< loc << " tag: "<<  tag;
        weight = ip->weight;
        face = ip->face;
        request_face(face, 0);
        flags = ip->flagsval;
        anim = ip->animation_id;
        animspeed = (uint8) ip->anim_speed;
        nrof = ip->nrof;
        direction = ip->direction;

        if (sendflags & UPD_LOCATION)
        {
            loc = GetInt_String(data + pos);
            env = locate_item(loc);
            if (!env)
                Logger::log().error() << "UpdateItemCmd: unknown object tag "<<loc << " for new location";
            pos += 4;
        }
        if (sendflags & UPD_FLAGS)
        {
            flags = GetInt_String(data + pos);
            pos += 4;
        }
        if (sendflags & UPD_WEIGHT)
        {
            weight = GetInt_String(data + pos);
            pos += 4;
        }
        if (sendflags & UPD_FACE)
        {
            face = GetInt_String(data + pos);
            request_face(face, 0);
            pos += 4;
        }
        if (sendflags & UPD_DIRECTION)
            direction = data[pos++];
        if (sendflags & UPD_NAME)
        {
            nlen = data[pos++];
            memcpy(name, (char *) data + pos, nlen);
            pos += nlen;
            name[nlen] = '\0';
        }
        if (pos > len)
        {
            Logger::log().error() << "UpdateItemCmd: Overread buffer: " << pos << " > " << len;
            return;
        }
        if (sendflags & UPD_ANIM)
        {
            anim = GetShort_String(data + pos);
            pos += 2;
        }
        if (sendflags & UPD_ANIMSPEED)
        {
            animspeed = data[pos++];
        }
        if (sendflags & UPD_NROF)
        {
            nrof = GetInt_String(data + pos);
            pos += 4;
        }
        if (sendflags & UPD_QUALITY)
     {
            quality = (int)(data[pos++]);
            condition = (int)(data[pos++]);
     }
        update_item(tag, loc, name, weight, face, flags, anim, animspeed, nrof, 254, 254, quality, condition, 254, 254, direction,
                    false);
        map_udate_flag = 2;
    */
}

//================================================================================================
// .
//================================================================================================
void Network::DeleteItem(unsigned char *data, int len)
{
    /*
        int pos = 0, tag;
        while (pos < len)
        {
            tag = GetInt_String(data); pos += 4;
            delete_item(tag);
        }
        if (pos > len)
            Logger::log().error() <<  "DeleteCmd: Overread buffer: " << pos << " > " << len;
        map_udate_flag = 2;
    */
}

//================================================================================================
// .
//================================================================================================
void Network::StatsCmd(unsigned char *data, int len)
{
    /*
        int     i   = 0, x;
        int     c, temp;
        char   *tmp, *tmp2;

        while (i < len)
        {
            c = data[i++];

            if (c >= CS_STAT_PROT_START && c <= CS_STAT_PROT_END)
            {
                cpl.stats.protection[c - CS_STAT_PROT_START] = (sint16) *(((signed char*)data) + i++);
                cpl.stats.protection_change = 1;
            }
            else
            {
                switch (c)
                {
                    case CS_STAT_TARGET_HP:
                      cpl.target_hp = (int) * (data + i++);
                      break;
                    case CS_STAT_REG_HP:
                      cpl.gen_hp = ((float) GetShort_String(data + i)) / 10.0f;
                      i += 2;
                      break;
                    case CS_STAT_REG_MANA:
                      cpl.gen_sp = ((float) GetShort_String(data + i)) / 10.0f;
                      i += 2;
                      break;
                    case CS_STAT_REG_GRACE:
                      cpl.gen_grace = ((float) GetShort_String(data + i)) / 10.0f;
                      i += 2;
                      break;

                    case CS_STAT_HP:
                      temp = GetInt_String(data + i);
                      if (temp < cpl.stats.hp && cpl.stats.food)
                      {
                          cpl.warn_hp = 1;
                          if (cpl.stats.maxhp / 12 <= cpl.stats.hp - temp)
                              cpl.warn_hp = 2;
                      }
                      cpl.stats.hp = temp;
                      i += 4;
                      break;
                    case CS_STAT_MAXHP:
                      cpl.stats.maxhp = GetInt_String(data + i);
                      i += 4;
                      break;
                    case CS_STAT_SP:
                      cpl.stats.sp = GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_MAXSP:
                      cpl.stats.maxsp = GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_GRACE:
                      cpl.stats.grace = GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_MAXGRACE:
                      cpl.stats.maxgrace = GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_STR:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Str)
                          cpl.warn_statup = TRUE;
                      else
                          cpl.warn_statdown = TRUE;

                      cpl.stats.Str = temp;
                      break;
                    case CS_STAT_INT:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Int)
                          cpl.warn_statup = TRUE;
                      else
                          cpl.warn_statdown = TRUE;

                      cpl.stats.Int = temp;
                      break;
                    case CS_STAT_POW:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Pow)
                          cpl.warn_statup = TRUE;
                      else
                          cpl.warn_statdown = TRUE;

                      cpl.stats.Pow = temp;

                      break;
                    case CS_STAT_WIS:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Wis)
                          cpl.warn_statup = TRUE;
                      else
                          cpl.warn_statdown = TRUE;

                      cpl.stats.Wis = temp;

                      break;
                    case CS_STAT_DEX:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Dex)
                          cpl.warn_statup = TRUE;
                      else
                          cpl.warn_statdown = TRUE;

                      cpl.stats.Dex = temp;
                      break;
                    case CS_STAT_CON:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Con)
                          cpl.warn_statup = TRUE;
                      else
                          cpl.warn_statdown = TRUE;

                      cpl.stats.Con = temp;
                      break;
                    case CS_STAT_CHA:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Cha)
                          cpl.warn_statup = TRUE;
                      else
                          cpl.warn_statdown = TRUE;

                      cpl.stats.Cha = temp;
                      break;
                    case CS_STAT_EXP:
                      temp = GetInt_String(data + i);
                      if (temp < cpl.stats.exp)
                          cpl.warn_drain = TRUE;
                      cpl.stats.exp = temp;
                      // get the real level depending on the exp
                      for(x=0;x<=110;x++)
                      {
                          if(server_level.exp[x]>(uint32)temp)
                          {
                              cpl.stats.exp_level = x-1;
                              break;
                          }
                      }
                      i += 4;
                      break;
                    case CS_STAT_LEVEL:
                      cpl.stats.level = (char) * (data + i++);
                      if (cpl.stats.level != cpl.stats.exp_level)
                      {
                          cpl.warn_drain = TRUE;
                      }
                      break;
                    case CS_STAT_WC:
                      cpl.stats.wc = (char) GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_AC:
                      cpl.stats.ac = (char) GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_DAM:
                      cpl.stats.dam = GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_SPEED:
                      cpl.stats.speed = GetInt_String(data + i);
                      i += 4;
                      break;
                    case CS_STAT_FOOD:
                      cpl.stats.food = GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_WEAP_SP:
                        cpl.stats.weapon_sp = ((float)GetInt_String(data + i))/1000.0f;
                        i += 4;
                      break;
                    case CS_STAT_FLAGS:
                      cpl.stats.flags = GetShort_String(data + i);
                      i += 2;
                      break;
                    case CS_STAT_WEIGHT_LIM:
                      set_weight_limit(GetInt_String(data + i));
                      i += 4;
                      break;
                    case CS_STAT_SKILLEXP_AGILITY:
                    case CS_STAT_SKILLEXP_PERSONAL:
                    case CS_STAT_SKILLEXP_MENTAL:
                    case CS_STAT_SKILLEXP_PHYSIQUE:
                    case CS_STAT_SKILLEXP_MAGIC:
                    case CS_STAT_SKILLEXP_WISDOM:
                      cpl.stats.skill_exp[(c - CS_STAT_SKILLEXP_START) / 2] = GetInt_String(data + i);
                      i += 4;
                      break;
                    case CS_STAT_SKILLEXP_AGLEVEL:
                    case CS_STAT_SKILLEXP_PELEVEL:
                    case CS_STAT_SKILLEXP_MELEVEL:
                    case CS_STAT_SKILLEXP_PHLEVEL:
                    case CS_STAT_SKILLEXP_MALEVEL:
                    case CS_STAT_SKILLEXP_WILEVEL:
                      cpl.stats.skill_level[(c - CS_STAT_SKILLEXP_START - 1) / 2] = (sint16) * (data + i++);
                      break;
                    case CS_STAT_RANGE:
                      {
                          int   rlen    = data[i++];
                          strncpy(cpl.range, (const char *) data + i, rlen);
                          cpl.range[rlen] = '\0';
                          i += rlen;
                          break;
                      }

                    case CS_STAT_EXT_TITLE:
                      {
                          int   rlen    = data[i++];

                          tmp = strchr(data + i, '\n');
                          *tmp = 0;
                          strcpy(cpl.rank, data + i);
                          tmp2 = strchr(tmp + 1, '\n');
                          *tmp2 = 0;
                          strcpy(cpl.pname, tmp + 1);
                          tmp = strchr(tmp2 + 1, '\n');
                          *tmp = 0;
                          strcpy(cpl.race, tmp2 + 1);
                          tmp2 = strchr(tmp + 1, '\n');
                          *tmp2 = 0;
                          strcpy(cpl.title, tmp + 1); // profession title
                          tmp = strchr(tmp2 + 1, '\n');
                          *tmp = 0;
                          strcpy(cpl.alignment, tmp2 + 1);
                          tmp2 = strchr(tmp + 1, '\n');
                          *tmp2 = 0;
                          strcpy(cpl.godname, tmp + 1);

                          strcpy(cpl.gender, tmp2 + 1);
                          if (cpl.gender[0] == 'm')
                              strcpy(cpl.gender, "male");
                          else if (cpl.gender[0] == 'f')
                              strcpy(cpl.gender, "female");
                          else if (cpl.gender[0] == 'h')
                              strcpy(cpl.gender, "hermaphrodite");
                          else
                              strcpy(cpl.gender, "neuter");
                          i += rlen;

                          // prepare rank + name for fast acces the pname is <name> <title>.
                          // is there no title, there is still always a ' ' at the end - we skip this here!
                          strcpy(cpl.rankandname, cpl.rank);
                          strcat(cpl.rankandname, cpl.pname);
                          if (strlen(cpl.rankandname) > 0)
                              cpl.rankandname[strlen(cpl.rankandname) - 1] = 0;
                          adjust_string(cpl.rank);
                          adjust_string(cpl.rankandname);
                          adjust_string(cpl.pname);
                          adjust_string(cpl.race);
                          adjust_string(cpl.title);
                          adjust_string(cpl.alignment);
                          adjust_string(cpl.gender);
                          adjust_string(cpl.godname);
                      }
                      break;
                    case CS_STAT_TITLE:
                      {
                          Logger::log().warning() << "Command get stats: CS_STAT_TITLE is outdated");
                          //    int rlen=data[i++];
                          //    strncpy(cpl.title2,
                          //    (const char*)data+i,rlen);
                          //    cpl.title2[rlen]='\0';
                          //    i += rlen;
                      }
                      break;
                    default:
                      Logger::log().error() << "Unknown stat number " << c;
                }
            }
        }
        if (i > len)
        {
            Logger::log().error() << "got stats overflow, processed " << i << " bytes out of " << len;
        }
    */
}

//================================================================================================
// .
//================================================================================================
void Network::ImageCmd(unsigned char *data, int len)
{
    /*
        int     pnum, plen;
        char    buf[2048];
        FILE   *stream;

        pnum = GetInt_String(data);
        plen = GetInt_String(data + 4);
        if (len < 8 || (len - 8) != plen)
        {
            Logger::log().error() << "PixMapCmd: Lengths don't compare (" << (len - 8) " " << " " << plen << ")";
            return;
        }

        // save picture to cache
        // and load it to FaceList
        sprintf(buf, "%s%s", GetCacheDirectory(), FaceList[pnum].name);
        if ((stream = fopen_wrapper(buf, "wb+")) != NULL)
        {
            fwrite((char *) data + 8, 1, plen, stream);
            fclose(stream);
        }
        FaceList[pnum].sprite = sprite_tryload_file(buf, 0, NULL);
        map_udate_flag = 2;
    */
}

//================================================================================================
// .
//================================================================================================
void Network::Face1Cmd(unsigned char *data, int len)
{
    /*
        int pnum = GetShort_String(data);
        uint32 checksum = GetInt_String(data + 2);
        char *face = (char *) data + 6;
        data[len] = '\0';
        finish_face_cmd(pnum, checksum, face);
    */
}

//================================================================================================
// .
//================================================================================================
void Network::AnimCmd(unsigned char *data, int len)
{
    /*
        short   anum;
        int     i, j;

        anum = GetShort_String(data);
        if (anum<0 || anum> MAXANIM)
        {
            Logger::log().error() << "AnimCmd: animation number invalid: " << anum;
            return;
        }

        animations[anum].flags = *(data + 2);
        animations[anum].facings = *(data + 3);
        animations[anum].num_animations = (len - 4) / 2;
        if (animations[anum].num_animations < 1)
        {
            Logger::log().error() << "AnimCmd: num animations invalid: " << animations[anum].num_animations;
            return;
        }
        if (animations[anum].facings > 1)
            animations[anum].frame = animations[anum].num_animations / animations[anum].facings;
        else
            animations[anum].frame = animations[anum].num_animations;
        animations[anum].faces = _malloc(sizeof(uint16) * animations[anum].num_animations, "AnimCmd(): facenum buf");
        for (i = 4,j = 0; i < len; i += 2,j++)
        {
            animations[anum].faces[j] = GetShort_String(data + i);
            request_face(animations[anum].faces[j], 0);
        }
        if (j != animations[anum].num_animations)
            Logger::log().error() <<  "Calculated animations does not equal stored animations?("
              << j <<  " != " << animations[anum].num_animations) <<")";
    */
}

//================================================================================================
// .
//================================================================================================
void Network::SkillRdyCmd(unsigned char *data, int len)
{
    /*
        strcpy(cpl.skill_name, data);
        // lets find the skill... and setup the shortcuts to the exp values.
        for (int ii = 0; ii < SKILL_LIST_MAX; ii++)
        {
            for (int i = 0; i < DIALOG_LIST_ENTRY; i++)
            {
                // we have a list entry
                if (skill_list[ii].entry[i].flag == LIST_ENTRY_KNOWN)
                {
                    // and is it the one we searched for?
                    if (!strcmp(skill_list[ii].entry[i].name, cpl.skill_name))
                    {
                        cpl.skill_g = ii;
                        cpl.skill_e = i;
                        return;
                    }
                }
            }
        }
    */
}

//================================================================================================
// Copies relevant data from the archetype to the object.
// Only copies data that was not set in the object structure.
//================================================================================================
void Network::PlayerCmd(unsigned char *data, int len)
{
    Option::getSingleton().setGameStatus(GAME_STATUS_PLAY);
    /*
    char    name[MAX_BUF];
    int     tag, weight, face, i = 0, nlen;

    InputStringEndFlag = false;
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
        Logger::log().error() << "PlayerCmd: lengths do not match (" << len << " != " << i << ")";
    }
    new_player(tag, name, weight, (short) face);
    map_draw_map_clear();
    map_transfer_flag = 1;
    map_udate_flag = 2;
    */
    Logger::log().info() << "Loading quickslot settings for server";
    //load_quickslots_entrys();
}

//================================================================================================
// .
//================================================================================================
void Network::SpelllistCmd(unsigned char *data, int len)
{
    /*
        int     i, ii, mode;
        unsigned char   *tmp, *tmp2;
        char    name[256];

        // we grap our mode
        mode = atoi(data);

        for (; ;)
        {
            tmp = strchr(data, '/'); // find start of a name
            if (!tmp)
                return;
            data = tmp + 1;

            tmp2 = strchr(data, '/');
            if (tmp2)
            {
                strncpy(name, data, tmp2 - data);
                name[tmp2 - data] = 0;
                data = tmp2;
            }
            else
                strcpy(name, data);

            // we have a name - now check the spelllist file and set the entry to _KNOWN

            for (i = 0; i < SPELL_LIST_MAX; i++)
            {
                for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
                {
                    if (spell_list[i].entry[0][ii].flag >= LIST_ENTRY_USED)
                    {
                        if (!strcmp(spell_list[i].entry[0][ii].name, name))
                        {
                            if (mode == SPLIST_MODE_REMOVE)
                                spell_list[i].entry[0][ii].flag = LIST_ENTRY_USED;
                            else
                                spell_list[i].entry[0][ii].flag = LIST_ENTRY_KNOWN;
                            goto next_name;
                        }
                    }
                    if (spell_list[i].entry[1][ii].flag >= LIST_ENTRY_USED)
                    {
                        if (!strcmp(spell_list[i].entry[1][ii].name, name))
                        {
                            if (mode == SPLIST_MODE_REMOVE)
                                spell_list[i].entry[1][ii].flag = LIST_ENTRY_USED;
                            else
                                spell_list[i].entry[1][ii].flag = LIST_ENTRY_KNOWN;
                            goto next_name;
                        }
                    }
                }
            }
            next_name:;
        }
    */
}

//================================================================================================
// .
//================================================================================================
void Network::SkilllistCmd(unsigned char *data, int len)
{
    /*
        unsigned char *tmp, *tmp2, *tmp3, *tmp4;
        int     l, e, i, ii, mode;
        char    name[256];


        // we grap our mode
        mode = atoi(data);

        // now look for the members fo the list we have
        for (; ;)
        {
            tmp = strchr(data, '/'); // find start of a name
            if (!tmp)
                return;
            data = tmp + 1;

            tmp2 = strchr(data, '/');
            if (tmp2)
            {
                strncpy(name, data, tmp2 - data);
                name[tmp2 - data] = 0;
                data = tmp2;
            }
            else
                strcpy(name, data);

            tmp3 = strchr(name, '|');
            *tmp3 = 0;
            tmp4 = strchr(tmp3 + 1, '|');

            l = atoi(tmp3 + 1);
            e = atoi(tmp4 + 1);

            // we have a name, the level and exp - now setup the list
            for (ii = 0; ii < SKILL_LIST_MAX; ii++)
            {
                for (i = 0; i < DIALOG_LIST_ENTRY; i++)
                {
                    // we have a list entry
                    if (skill_list[ii].entry[i].flag != LIST_ENTRY_UNUSED)
                    {
                        // and it is the one we searched for?
                        if (!strcmp(skill_list[ii].entry[i].name, name))
                        {
                            if (mode == SPLIST_MODE_REMOVE) // remove?
                                skill_list[ii].entry[i].flag = LIST_ENTRY_USED;
                            else
                            {
                                skill_list[ii].entry[i].flag = LIST_ENTRY_KNOWN;
                                skill_list[ii].entry[i].exp = e;
                                skill_list[ii].entry[i].exp_level = l;
                            }
                        }
                    }
                }
            }
        }
    */
}

//================================================================================================
// .
//================================================================================================
void Network::GolemCmd(unsigned char *data, int len)
{
    /*
        int     mode, face;
        char   *tmp, buf[256];

        // we grap our mode
        mode = atoi(data);
        if (mode == GOLEM_CTR_RELEASE)
        {
            tmp = strchr(data, ' '); // find start of a name
            face = atoi(tmp + 1);
            request_face(face, 0);
            tmp = strchr(tmp + 1, ' '); // find start of a name
            sprintf(buf, "You lose control of %s.", tmp + 1);
            draw_info(buf, COLOR_WHITE);

            fire_mode_tab[FIRE_MODE_SUMMON].item = FIRE_ITEM_NO;
            fire_mode_tab[FIRE_MODE_SUMMON].name[0] = 0;
        }
        else
        {
            tmp = strchr(data, ' '); // find start of a name
            face = atoi(tmp + 1);
            request_face(face, 0);
            tmp = strchr(tmp + 1, ' '); // find start of a name
            sprintf(buf, "You get control of %s.", tmp + 1);
            draw_info(buf, COLOR_WHITE);
            fire_mode_tab[FIRE_MODE_SUMMON].item = face;
            strncpy(fire_mode_tab[FIRE_MODE_SUMMON].name, tmp + 1, 100);
            RangeFireMode = FIRE_MODE_SUMMON;
        }
    */
}

//================================================================================================
// .
//================================================================================================
void Network::AddMeSuccess(unsigned char *data, int len)
{
    //Logger::log().info() << "addme_success received.";
}

//================================================================================================
// This could probably be greatly improved - I am not sure if anything needs to be saved here,
// but certainly it should be possible to reconnect to the server or a different server without
// having to rerurn the client.
//================================================================================================
void Network::GoodbyeCmd(unsigned char *data, int len)
{
    // Damn, this should not be here - if the version not matches, the server
    // drops the connnect - so we get a client shutdown here?
    // NEVER do this again.
}

//================================================================================================
// .
//================================================================================================
void Network::SetupCmd(unsigned char *data, int len)
{
    /*
        int     s;
        char   *cmd, *param;

        scrolldy = scrolldx = 0;
        Logger::log().info() << "Get SetupCmd: " << buf;
        for (s = 0; ;)
        {
            while (buf[s] == ' ')
                s++;
            if (s >= len)
                break;
            cmd = &buf[s];
            for (; buf[s] && buf[s] != ' '; s++)
                ;
            buf[s++] = 0;
            while (buf[s] == ' ')
                s++;
            if (s >= len)
                break;
            param = &buf[s];
            for (; buf[s] && buf[s] != ' '; s++)
                ;
            buf[s++] = 0;
            while (buf[s] == ' ')
                s++;

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
                    Logger::log().info() << "Get skf:: " << param;
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
                    Logger::log().info() << "Get spf:: " << param;
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
                    Logger::log().info() << "Get stf:: " << param;
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
                    Logger::log().info() << "Get bpf:: " << param;
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
                    Logger::log().info() << "Get amf:: " << param;
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
                Logger::log().error() << "Got setup for a command we don't understand: " << cmd << " " << param;
                sprintf(buf, "The server is outdated!\nSelect a different one!");
             draw_info(buf, COLOR_RED);
       SOCKET_CloseSocket();
       Option::getSingleton().setGameStatus(GAME_STATUS_START);
       return;
            }
        }
        Option::getSingleton().setGameStatus(GAME_STATUS_REQUEST_FILES);
    */
}

//================================================================================================
// .
//================================================================================================
void Network::handle_query(unsigned char *data, int len)
{
    //uint8 flags = atoi(data);  // ATM unused parameter
    char *buf = strchr((char *)data, ' ');
    if (buf) ++buf;
    // one query string
    Logger::log().info() << "Received query string: " <<  buf;
    PreParseInfoStat(buf);
}

//================================================================================================
// Server has send us a file: uncompress and save it.
//================================================================================================
void Network::DataCmd(unsigned char *data, int len)
{
    // ////////////////////////////////////////////////////////////////////
    // check for valid command:
    // 0 = NC, 1 = SERVER_FILE_SKILLS, 2 = SERVER_FILE_SPELLS, (...)
    // ////////////////////////////////////////////////////////////////////
    unsigned char data_type = data[0];
    unsigned char data_cmd  = (data_type &~DATA_PACKED_CMD) -1;
    if (data_cmd > SERVER_FILE_SUM)
    {
        Logger::log().error()  << "data cmd: unknown type " << data_type << " (len:" << len << ")";
        return;
    }
    --len;
    ++data;

    // ////////////////////////////////////////////////////////////////////
    // Uncompress if needed.
    // ////////////////////////////////////////////////////////////////////
    char *dest =0;
    if (data_type & DATA_PACKED_CMD)
    {
        // warning! if the uncompressed size of a incoming compressed(!) file
        // is larger as this dest_len default setting, the file is cutted and
        // the rest skiped. Look at the zlib docu for more info.
        unsigned long dest_len = 512 * 1024;
        dest = new char[dest_len];
        uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
        data = (unsigned char*)dest;
        len  = dest_len;
    }
    ++mRequest_file_chain;

    // ////////////////////////////////////////////////////////////////////
    // Save the file.
    // ////////////////////////////////////////////////////////////////////
    ofstream out(ServerFile::getSingleton().getFilename(data_cmd), ios::out|ios::binary);
    if (!out)
        Logger::log().error()  << "save data cmd file : write() of "
        << ServerFile::getSingleton().getFilename(data_cmd) << " failed.";
    else
        out.write((char*)data, len);
    delete[] dest;

    // ////////////////////////////////////////////////////////////////////
    // Reload the new file.
    // ////////////////////////////////////////////////////////////////////
    //         if (data_cmd-1 == SERVER_FILE_SKILLS) { read_skills(); }
    //    else if (data_cmd-1 == SERVER_FILE_SPELLS) { read_spells(); }
}

//================================================================================================
// server tells us to go to the new char creation.
//================================================================================================
void Network::NewCharCmd(unsigned char *data, int len)
{
    //dialog_new_char_warn = 0;
    Option::getSingleton().setGameStatus(GAME_STATUS_NEW_CHAR);
}

//================================================================================================
// ItemYCmd is ItemCmd with sort order reversed (add to front).
//================================================================================================
void Network::ItemYCmd(unsigned char *data, int len)
{
    // ItemXYCmd(data, len, true);
}

//================================================================================================
// .
//================================================================================================
void Network::GroupCmd(unsigned char *data, int len)
{
    /*
        char    name[64], *tmp;
        int     hp, mhp, sp, msp, gr, mgr, level, slot = 0;

        // len == 0, its a GroupCmd which means "no group"
        clear_group();
        if (len)
        {
            //sprintf(buf, "GROUP CMD: %s (%d)", data, len);
            //draw_info(buf, COLOR_GREEN);

            global_group_status = GROUP_MEMBER;
            tmp = strchr(data, '|');
            while (++tmp)
            {
                sscanf(tmp, "%s %d %d %d %d %d %d %d", name, &hp, &mhp, &sp, &msp, &gr, &mgr, &level);
                set_group(slot, name, level, hp, mhp, sp, msp, gr, mgr);
                ++slot;
                tmp = strchr(tmp, '|');
            }
        }
    */
}

//================================================================================================
// Someone want invite us to a group.
//================================================================================================
void Network::GroupInviteCmd(unsigned char *data, int len)
{
    /*
        if(global_group_status != GROUP_NO) // bug
            Logger::log().error() << "Got group invite when g_status != GROUP_NO (" << data << ")";
        else
        {
            global_group_status = GROUP_INVITE;
            strncpy(group_invite, data, 30);
        }
    */
}

//================================================================================================
// .
//================================================================================================
void Network::GroupUpdateCmd(unsigned char *data, int len)
{
    /*
        if (!len) return;
        int hp, mhp, sp, msp, gr, mgr, level, slot = 0;

        char *tmp = strchr(data, '|');
        while (++tmp)
        {
            sscanf(tmp, "%d %d %d %d %d %d %d %d", &slot, &hp, &mhp, &sp, &msp, &gr, &mgr, &level);
            set_group(slot, NULL, level, hp, mhp, sp, msp, gr, mgr);
            tmp = strchr(tmp, '|');
        }
    */
}

//================================================================================================
// .
//================================================================================================
void Network::InterfaceCmd(unsigned char *data, int len)
{
    /*
        map_udate_flag = 2;
        if((gui_interface_npc && gui_interface_npc->status != GUI_INTERFACE_STATUS_WAIT) &&
             ((!len && cpl.menustatus == MENU_NPC) || (len && cpl.menustatus != MENU_NPC)))
        {
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        }
     reset_keys();
     cpl.input_mode = INPUT_MODE_NO;
        reset_gui_interface();
        if(len)
        {
            int mode, pos = 0;

            mode = *data;
            pos ++;

      Logger::log().error() <<  "Interface command: " << (char*)(data+pos));
            gui_interface_npc = load_gui_interface(mode, (char*)data, len, pos);
            if(!gui_interface_npc)
                draw_info("INVALID GUI CMD", COLOR_RED);
            else
            {
                gui_interface_npc->win_length = precalc_interface_npc();
                interface_mode = mode;
                cpl.menustatus = MENU_NPC;
                gui_interface_npc->startx = 400-(Bitmaps[BITMAP_NPC_INTERFACE]->bitmap->w / 2);
                gui_interface_npc->starty = 50;
                mb_clicked=0;
                 // Prefilled (and focused) textfield
                   if(gui_interface_npc->used_flag&GUI_INTERFACE_TEXTFIELD)
                   {
                       gui_interface_npc->input_flag = TRUE;

                       reset_keys();
                       open_input_mode(240);
                       textwin_putstring(gui_interface_npc->textfield.text);
                       cpl.input_mode = INPUT_MODE_NPCDIALOG;
                       HistoryPos = 0;
                   }
            }
        }
    */
}

//================================================================================================
// .
//================================================================================================
void Network::BookCmd(unsigned char *data, int len)
{
    /*
        sound_play_effect(SOUND_BOOK, 0, 0, 100);
        cpl.menustatus = MENU_BOOK;

        int mode = *((int*)data);
        data+=4;
        gui_interface_book = load_book_interface(mode, data, len-4);
    */
}

//================================================================================================
// .
//================================================================================================
void Network::MarkCmd(unsigned char *data, int len)
{
    //cpl.mark_count = GetInt_String(data);
}

//================================================================================================
// .
//================================================================================================
void Network::MagicMapCmd(unsigned char *data, int len)
{}

//================================================================================================
// .
//================================================================================================
void Network::DeleteInventory(unsigned char *data, int len)
{
    /*
        int tag = atoi((const char *) data);
        if (tag < 0)
        {
            Logger::log().error() << "DeleteInventory: Invalid tag: " << tag;
            return;
        }
        remove_item_inventory(locate_item(tag));
        map_udate_flag = 2;
    */
}

//================================================================================================
//
//================================================================================================
void Network::PreParseInfoStat(char *cmd)
{
    int status = cmd[2] -'0';
    if (!strncmp(cmd, "QN",2))
    {
        switch (status)
        {
            case 0:
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED, GUI_TEXTBOX_LOGIN_WARN, (void*)"");
                break;
            case 1:
                if (Option::getSingleton().getLoginType() == Option::LOGIN_EXISTING_PLAYER)
                    GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                           GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000There is no character with that name!~");
                break;
            case 2:
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                       GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Name or character is in creating process or blocked!~");
                break;
            case 3:
                if (Option::getSingleton().getLoginType() == Option::LOGIN_EXISTING_PLAYER)
                    GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                           GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Name is taken - choose a different one!~");
                break;
            case 4:
                if (Option::getSingleton().getLoginType() == Option::LOGIN_NEW_PLAYER)
                    GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                           GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Name is taken - choose a different one!~");
                break;
            case 5:
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                       GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Name is banned - choose a different one!~");
                break;
            case 6:
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                       GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Name is illegal - ITS TO SHORT OR ILLEGAL SIGNS!~");
                break;
            case 7:
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                       GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Name is illegal - ITS TO SHORT OR ILLEGAL SIGNS!~");
                break;
            default:
                GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                       GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Password is illegal or does not match!~");
                break;
        }
        Option::getSingleton().setGameStatus(GAME_STATUS_NAME_INIT);
    }
    else if (!strncmp(cmd, "QP",2))
    {
        if (status)
            GuiManager::getSingleton().sendMessage(GUI_WIN_LOGIN, GUI_MSG_TXT_CHANGED,
                                                   GUI_TEXTBOX_LOGIN_WARN, (void*)"~#ffff0000Password is illegal or does not match!~");
        Option::getSingleton().setGameStatus(GAME_STATUS_PSWD_INIT);
    }
    else if (!strncmp(cmd, "QV",2))
    {
        Option::getSingleton().setGameStatus(GAME_STATUS_VRFY_INIT);
    }
}

//================================================================================================
// we got a face - test we have it loaded. If not, say server "send us face cmd "
// Return: 0 - face not there, requested.  1: face requested or loaded
// This command collect all new faces and then flush it at once.
// I insert the flush command after the socket call.
//================================================================================================
int Network::request_face(int, int)
{
    return 1;
}

//================================================================================================
//
//================================================================================================
void Network::CreatePlayerAccount()
{
    char    buf[MAX_BUF];
    //   sprintf(buf, "nc %s %d %d %d %d %d %d %d", nc->char_arch[nc->gender_selected], nc->stats[0], nc->stats[1], nc->stats[2], nc->stats[3], nc->stats[4], nc->stats[5], nc->stats[6]);
    sprintf(buf, "%s", "nc human_male 14 14 13 12 12 12 12 0");
    cs_write_string(buf, (int)strlen(buf));
}

