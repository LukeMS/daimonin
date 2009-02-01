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

#include <fstream>
#include <stdio.h>
#include "zlib.h"
#include "logger.h"
#include "define.h"
#include "network.h"
#include "option.h"
#include "tile_map.h"
#include "tile_manager.h"
#include "gui_manager.h"
#include "gui_window_dialog.h"
#include "network_serverfile.h"
#include "object_manager.h"
#include "object_hero.h"
#include "item.h"

using namespace Ogre;

// list of requested files shared between server and client.
enum
{
    DATA_CMD_NO,
    DATA_CMD_SKILL_LIST,
    DATA_CMD_SPELL_LIST,
    DATA_CMD_SETTINGS_LIST,
    DATA_CMD_ANIM_LIST,
    DATA_CMD_BMAP_LIST
};

Network::ConsoleCmdString Network::mConsoleCmd[CONSOLE_CMD_SUM]=
{
    { "/apply",         CONSOLE_CMD_APPLY        },
    { "/buddy",         CONSOLE_CMD_BUDDY        },
    { "/cfilter",       CONSOLE_CMD_CFILTER      },
    { "/changeskin",    CONSOLE_CMD_CHANGESKIN   },
    { "/channel",       CONSOLE_CMD_CHANNEL      },
    { "/f",             CONSOLE_CMD_FKEY         }, /**< Function keys (F1...F9) */
    { "/ignore",        CONSOLE_CMD_IGNORE       },
    { "/imagestats",    CONSOLE_CMD_IMGSTATS     },
    { "/keybind",       CONSOLE_CMD_KEYBIND      },
    { "/kills",         CONSOLE_CMD_KILLS        },
    { "/markdmbuster",  CONSOLE_CMD_MARKDMBUSTER },
    { "/ready_spell",   CONSOLE_CMD_RDYSPELL     },
    { "/reloadskinnow", CONSOLE_CMD_RELOADSKIN   },
    { "/reply",         CONSOLE_CMD_REPLY        },
    { "/reset",         CONSOLE_CMD_RESET        },
    { "/searchpath",    CONSOLE_CMD_SEARCHPATH   },
    { "/setwin",        CONSOLE_CMD_SETWIN       },
    { "/setwinalpha",   CONSOLE_CMD_SETWINALPHA  },
    { "/shout_off",     CONSOLE_CMD_SHOUTOFF     },
    { "/shout_on",      CONSOLE_CMD_SHOUTON      },
    { "/sleeptimer",    CONSOLE_CMD_SLEEPTIMER   },
    { "/statreset",     CONSOLE_CMD_STATSRESET   },
    { "/target",        CONSOLE_CMD_TARGET       },
    { "/teststretch",   CONSOLE_CMD_TESTSTRETCH  },
};

enum
{
    MAP_UPDATE_CMD_SAME,
    MAP_UPDATE_CMD_NEW,
    MAP_UPDATE_CMD_CONNECTED
};

//================================================================================================
// Ascii to int (32bit).
//================================================================================================
int Network::GetInt_String(unsigned char *data)
{
    if (mEndianConvert)
        return (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
    return (data[3] << 24) + (data[2] << 16) + (data[1] << 8) + data[0];
}

//================================================================================================
// Ascii to short (16bit).
//================================================================================================
short Network::GetShort_String(unsigned char *data)
{
    if (mEndianConvert)
        return (data[0] << 8) + data[1];
    return (data[1] << 8) + data[0];
}

//================================================================================================
// .
//================================================================================================
void Network::AccNameSuccess(unsigned char *data, int len)
{
    GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)"Account Success");
    Logger::log().error() << "AccNameSuccess";
    /*
    int num = (len)?GetUINT8_String(data):ACCOUNT_STATUS_DISCONNECT;
    if(num == ACCOUNT_STATUS_DISCONNECT)
    {
        LOG(LOG_MSG, "Server rejected your account action - closing socket.\n");
        SOCKET_CloseSocket(csocket.fd);
        SDL_Delay(1250);
        GameStatus = GAME_STATUS_INIT;
    }
    else
    {
        if(num == ACCOUNT_STATUS_OK)
        {
            // we continue with the account creation
            GameStatus = GAME_STATUS_LOGIN_NEW;
            // but now we go to password input
            LoginInputStep = LOGIN_STEP_PASS1;
            dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
            cpl.password[0] = 0;
            open_input_mode(MAX_ACCOUNT_PASSWORD);
        }
        else // something is wrong, try again
        {
            GameStatus = GAME_STATUS_LOGIN_NEW;
            sound_play_effect(SOUND_SCROLL, 0, 0, 100);
            open_input_mode(MAX_ACCOUNT_PASSWORD);
        }
    }
    */
}

//================================================================================================
// Server is sending us our account data or the reason why not
//================================================================================================
void Network::AccountCmd(unsigned char *data, int len)
{
    GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)"Account cmd from server");
    ObjectHero::getSingleton().clearAccount();
    // First, get the account status - it tells us too when login failed
    if (*data) // something is wrong when not ACCOUNT_STATUS_OK (0)
    {
        GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)"Account fail");
        GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)"Account does not exist");
        GuiManager::getSingleton().setElementText(GuiManager::WIN_LOGIN, GuiImageset::GUI_TEXTBOX_LOGIN_WARN, "Account does not exist");
        Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
    }
    else // we have account data... set it up and move player to account view mode
    {
        int count = 1;
        GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)"Account ok");
        for (int nr = 0; nr < ObjectHero::ACCOUNT_MAX_PLAYER; ++nr)
        {
            if (count >= len) break;
            count += ObjectHero::getSingleton().fillAccount(nr, data+count);
        }
        Option::getSingleton().setGameStatus(Option::GAME_STATUS_LOGIN_DONE);
    }
}


//================================================================================================
// .
//================================================================================================
void Network::DrawInfoCmd(unsigned char *data, int len)
{
    // int color   = atoi(data);
    // Todo: Convert indexed color into rgb and add it to the text.
    char *buf = strchr((char *)data, ' ');
    if (!buf)
    {
        Logger::log().error() << "DrawInfoCmd - got no data";
        buf = (char*)"";
    }
    else
        ++buf;
    GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)buf);
}

//================================================================================================
// Handles when the server says we can't be added.  In reality, we need to close the connection
// and quit out, because the client is going to close us down anyways.
//================================================================================================
void Network::AddMeFail(unsigned char *data, int len)
{
    Logger::log().error() << "addme_failed received.\n";
    CloseSocket();
    SDL_Delay(1250);
    Option::getSingleton().setGameStatus(Option::GAME_STATUS_INIT_NET);
}

//================================================================================================
// .
//================================================================================================
void Network::Map2Cmd(unsigned char *data, int len)
{
    static int map_w=0, map_h=0,mx=0,my=0;
    int     mask, x, y, pos = 0, ext_flag, xdata;
    int height_2, height_3, height_4;
    int     mapstat, ext1, ext2, ext3, probe;
    bool    map_new_flag = false;
    int     ff0, ff1, ff2, ff3, ff_flag, xpos, ypos;
    char    pname1[64], pname2[64], pname3[64], pname4[64];
    String  mapname;
    uint16  face;

    mapstat = (data[pos++]);
    TileManager::getSingleton().map_transfer_flag = 0;
    if (mapstat != MAP_UPDATE_CMD_SAME)
    {
        mapname = (char*)data + pos;
        pos += (int)mapname.size()+1;
        if (mapstat == MAP_UPDATE_CMD_NEW)
        {
            map_w = (uint8)(data[pos++]);
            map_h = (uint8)(data[pos++]);
            xpos = (uint8)(data[pos++]);
            ypos = (uint8)(data[pos++]);
            mx = xpos;
            my = ypos;
//            remove_item_inventory(locate_item(0)); // implicit clear below
            TileMap::getSingleton().InitMapData(mapname.c_str(), map_w, map_h, xpos, ypos);
        }
        else
        {
            int xoff, yoff;
            mapstat = (char)(data[pos++]);
            xoff = (char)(data[pos++]);
            yoff = (char)(data[pos++]);
            xpos = (uint8)(data[pos++]);
            ypos = (uint8)(data[pos++]);
            mx = xpos;
            my = ypos;
//            remove_item_inventory(locate_item(0)); // implicit clear below
//            TileMap::getSingleton().scroll(xoff, yoff);
        }
    }
    else
    {
        xpos = (uint8)(data[pos++]);
        ypos = (uint8)(data[pos++]);
        // we have moved
        if ((xpos - mx || ypos - my))
        {
//            remove_item_inventory(locate_item(0)); // implicit clear below
//            if (cpl.menustatus != MENU_NO) reset_menu_status();
        }
        TileMap::getSingleton().scroll(xpos - mx, ypos - my);
        mx = xpos;
        my = ypos;
    }
    if (map_new_flag)
    {
        TileMap::getSingleton().adjust_map_cache(xpos, ypos);
    }
    TileMap::getSingleton().mMapData.posx = xpos; // map windows is from range to +MAPWINSIZE_X
    TileMap::getSingleton().mMapData.posy = ypos;
    //Logger::log().info() << "MapPos x: " << xpos << " y: " << ypos << " (nflag: " << map_new_flag << ")";
    while (pos < len)
    {
        ext_flag = 0;
        ext1 = ext2 = ext3 = 0;
        // first, we get the mask flag - it decribes what we now get
        mask = GetShort_String(data + pos); pos += 2;
        x = (mask >> 11) & 0x1f;
        y = (mask >>  6) & 0x1f;
        // These are the "damage tags" - shows damage an object got from somewhere.
        // ff_flag hold the layer info and how much we got here.
        // 0x08 means a damage comes from unknown or vanished source.
        // this means the object is destroyed.
        // the other flags are assigned to map layer.
        if ((mask & 0x3f) == 0)
        {
            TileMap::getSingleton().display_map_clearcell(x, y);
        }
        ext3 = ext2 = ext1 = -1;
        pname1[0] = 0;
        pname2[0] = 0;
        pname3[0] = 0;
        pname4[0] = 0;
        // the ext flag defines special layer object assigned infos.
        // Like the Zzz for sleep, paralyze msg, etc.
        if (mask & 0x20) // catch the ext. flag...
        {
            ext_flag = (uint8)(data[pos++]);
            if (ext_flag & 0x80) // we have player names....
            {
                char c;
                int  i, pname_flag = (uint8) data[pos++];
                if (pname_flag & 0x08) // floor ....
                {
                    i = 0;
                    while ((c = (char) data[pos++]))
                        pname1[i++] = c;
                    pname1[i] = 0;
                }
                if (pname_flag & 0x04) // fm....
                {
                    i = 0;
                    while ((c = (char) data[pos++]))
                        pname2[i++] = c;
                    pname2[i] = 0;
                }
                if (pname_flag & 0x02) // l1 ....
                {
                    i = 0;
                    while ((c = (char)(data[pos++])))
                        pname3[i++] = c;
                    pname3[i] = 0;
                }
                if (pname_flag & 0x01) // l2 ....
                {
                    i = 0;
                    while ((c = (char)(data[pos++])))
                        pname4[i++] = c;
                    pname4[i] = 0;
                }
                if (pname_flag & 0x40)
                {
                    height_2 = GetShort_String(data + pos); pos+=2;
                    c = data[pos++];
                }
                if (pname_flag & 0x20)
                {
                    height_3 = GetShort_String(data + pos); pos+=2;
                    c = data[pos++];
                }
                if (pname_flag & 0x10)
                {
                    height_4 = GetShort_String(data + pos); pos+=2;
                    c = data[pos++];
                }
            }
            if (ext_flag & 0x40) // damage add on the map
            {
                ff0 = ff1 = ff2 = ff3 = -1;
                ff_flag = (uint8) data[pos++];
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
                ext3 = (int) data[pos++];
                if (ext3 & TileMap::FFLAG_PROBE)
                    probe = (int) data[pos++];
                else
                    probe = 0;
                TileMap::getSingleton().set_map_ext(x, y, 3, ext3, probe);
            }
            if (ext_flag & 0x10)
            {
                ext2 = (int)(data[pos++]);
                if (ext2 & TileMap::FFLAG_PROBE)
                    probe = (int)(data[pos++]);
                else
                    probe = 0;
                TileMap::getSingleton().set_map_ext(x, y, 2, ext2, probe);
            }
            if (ext_flag & 0x20)
            {
                ext1 = (int)(data[pos++]);
                if (ext1 & TileMap::FFLAG_PROBE)
                    probe = (int)(data[pos++]);
                else
                    probe = 0;
                TileMap::getSingleton().set_map_ext(x, y, 1, ext1, probe);
            }
        }
        if (mask & 0x10)
        {
            //TileMap::getSingleton().set_map_darkness(x, y, (uint8) (data[pos]));
            ++pos;
        }
        // at last, we get the layer faces.
        // a set ext_flag here marks this entry as face from a multi tile arch.
        // we got another byte then which all information we need to display
        // this face in the right way (position and shift offsets)
        if (mask & 0x8) // Layer 0 (Ground tiles).
        {
            face = GetShort_String(data + pos); pos += 2;
            xdata = 0;
            int height = GetShort_String(data + pos); pos += 2;
            TileMap::getSingleton().set_map_face(x, y, 0, face, xdata, -1, pname1, height);
            //Logger::log().error() << "Layer 0: " << x << " "<< y << " "<< height;
        }
        if (mask & 0x4) // Layer 1 (gras, bridge, ...).
        {
            face = GetShort_String(data + pos); pos += 2;
            xdata = 0;
            if (ext_flag & 0x04) // we have here a multi arch, fetch head offset
                xdata = (uint8) data[pos++];
            TileMap::getSingleton().set_map_face(x, y, 1, face, xdata, ext1, pname2);
        }
        if (mask & 0x2) // Layer 2 (wall, ...).
        {
            face = GetShort_String(data + pos); pos += 2;
            xdata = 0;
            if (ext_flag & 0x02) // we have here a multi arch, fetch head offset
                xdata = (uint8) data[pos++];
            TileMap::getSingleton().set_map_face(x, y, 2, face, xdata, ext2, pname3);
        }
        if (mask & 0x1) // Layer 3 (plant, npc, chair, ...).
        {
            face = GetShort_String(data + pos); pos += 2;
            xdata = 0;
            if (ext_flag & 0x01) // we have here a multi arch, fetch head offset
                xdata = (uint8) data[pos++];
            TileMap::getSingleton().set_map_face(x, y, 3, face, xdata, ext3, pname4);
        }
    } // more tiles
    TileMap::getSingleton().draw();
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
    GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)buf); // TESTING!!!
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
// .
//================================================================================================

#define SOUND_NORMAL    0
#define SOUND_SPELL     1

/* music mode - controls how the music is played and started */
#define MUSIC_MODE_NORMAL 1
#define MUSIC_MODE_DIRECT 2
#define MUSIC_MODE_FORCED 4 /* thats needed for some map event sounds */

// sound ids. //
enum _sound_id
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
enum _spell_sound_id
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

//================================================================================================
// .
//================================================================================================
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
// .
//================================================================================================
void Network::TargetObject(unsigned char *data, int len)
{
    String strTmp = "[";
    strTmp += (char*)data+3;
    strTmp += "] selected";
    GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)strTmp.c_str());
    /*
        cpl.target_mode = *data++;
        if (cpl.target_mode)
            sound_play_effect(SOUND_WEAPON_ATTACK, 0, 0, 100);
        else
            sound_play_effect(SOUND_WEAPON_HOLD, 0, 0, 100);
        cpl.target_color = *data++;
        cpl.target_code = *data++;
        strcpy(cpl.target_name, data);
        TileManager::getSingleton().map_udate_flag = 2;

        (buf,"TO: %d %d >%s< (len: %d)\n",cpl.target_mode,cpl.target_code,cpl.target_name,len);
        GuiManager::getSingleton().addTextline(WIN_TEXTWINDOW, GUI_LIST_MSGWIN, buf);
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
                          cpl.warn_statup = true;
                      else
                          cpl.warn_statdown = true;

                      cpl.stats.Str = temp;
                      break;
                    case CS_STAT_INT:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Int)
                          cpl.warn_statup = true;
                      else
                          cpl.warn_statdown = true;

                      cpl.stats.Int = temp;
                      break;
                    case CS_STAT_POW:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Pow)
                          cpl.warn_statup = true;
                      else
                          cpl.warn_statdown = true;

                      cpl.stats.Pow = temp;

                      break;
                    case CS_STAT_WIS:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Wis)
                          cpl.warn_statup = true;
                      else
                          cpl.warn_statdown = true;

                      cpl.stats.Wis = temp;

                      break;
                    case CS_STAT_DEX:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Dex)
                          cpl.warn_statup = true;
                      else
                          cpl.warn_statdown = true;

                      cpl.stats.Dex = temp;
                      break;
                    case CS_STAT_CON:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Con)
                          cpl.warn_statup = true;
                      else
                          cpl.warn_statdown = true;

                      cpl.stats.Con = temp;
                      break;
                    case CS_STAT_CHA:
                      temp = (int) * (data + i++);
                      if (temp >= cpl.stats.Cha)
                          cpl.warn_statup = true;
                      else
                          cpl.warn_statdown = true;

                      cpl.stats.Cha = temp;
                      break;
                    case CS_STAT_EXP:
                      temp = GetInt_String(data + i);
                      if (temp < cpl.stats.exp)
                          cpl.warn_drain = true;
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
                          cpl.warn_drain = true;
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
        (buf, "%s%s", GetCacheDirectory(), FaceList[pnum].name);
        if ((stream = fopen_wrapper(buf, "wb+")) != NULL)
        {
            fwrite((char *) data + 8, 1, plen, stream);
            fclose(stream);
        }
        FaceList[pnum].sprite = sprite_tryload_file(buf, 0, NULL);
        TileManager::getSingleton().map_udate_flag = 2;
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
    Option::getSingleton().setGameStatus(Option::GAME_STATUS_PLAY);
    Item::getSingleton().setBackpackID(GetInt_String(data));
    int i = 4;
//    ObjectHero::getSingleton().weight = GetInt_String(data + i);
    i += 4;
    //ObjectHero.getSingleton().face = GetInt_String(data + i);
    //request_face(face, 0);
    i += 4;
    int nlen = data[i++];
    char *name = new char[nlen+1];
    memcpy(name, (const char *) data + i, nlen);
    name[nlen] = '\0';
    i += nlen;
    if (i != len)
    {
        Logger::log().error() << "PlayerCmd: lengths does not match (" << len << " != " << i << ")";
    }
//    new_player(tag, name, weight, (short) face);
//    map_draw_map_clear();
//    map_transfer_flag = 1;
//    TileManager::getSingleton().map_update_flag = 2;
    delete[] name;

    static bool once = true;
    if (once)
    {
        once = false;
        ObjectStatic::sObject obj;
        obj.nickName  = "Polyveg";
        obj.meshName  = "Human_M_Fighter.mesh";
        obj.type      = ObjectManager::OBJECT_PLAYER;
        obj.boundingRadius = 2;
        obj.friendly  = 100;
        obj.attack    = 100;
        obj.defend    = 100;
        obj.maxHP     = 150;
        obj.maxMana   = 150;
        obj.maxGrace  = 150;
        obj.pos.x     = TileManager::TILE_SIZE * TileManager::CHUNK_SIZE_X/2;
        obj.pos.z     = TileManager::TILE_SIZE *(TileManager::CHUNK_SIZE_Z-6) - TileManager::TILE_SIZE/2;
        obj.level     = 0;
        obj.facing    = -60;
        obj.particleNr=-1;
        ObjectManager::getSingleton().addMobileObject(obj);
    }
    Logger::log().info() << "Loading quickslot settings";
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
            tmp = strchr(tmp + 1, ' '); // find start of a name
            (buf, "You lose control of %s.", tmp + 1);
            draw_info(buf, COLOR_WHITE);

            fire_mode_tab[FIRE_MODE_SUMMON].item = FIRE_ITEM_NO;
            fire_mode_tab[FIRE_MODE_SUMMON].name[0] = 0;
        }
        else
        {
            tmp = strchr(data, ' '); // find start of a name
            face = atoi(tmp + 1);
            tmp = strchr(tmp + 1, ' '); // find start of a name
            (buf, "You get control of %s.", tmp + 1);
            draw_info(buf, COLOR_WHITE);
            fire_mode_tab[FIRE_MODE_SUMMON].item = face;
            strncpy(fire_mode_tab[FIRE_MODE_SUMMON].name, tmp + 1, 100);
            RangeFireMode = FIRE_MODE_SUMMON;
        }
    */
}

//================================================================================================
// Server send us the setup cmd..
//================================================================================================
void Network::SetupCmd(unsigned char *buf, int len)
{
    buf+=6; // Skip the endian test - because its crap! Please use htonl()/htons() instead!
    unsigned char *cmd, *param;
    //scrolldy = scrolldx = 0;
    int pos =0;
    while (1)
    {
        // Get command.
        while (pos < len && buf[pos] == ' ') ++pos;
        if (pos >= len) break;
        cmd = &buf[pos];
        while (pos < len && buf[pos] != ' ') ++pos;
        if (pos >= len) break;
        buf[pos++] = '\0';
        // Get parameter.
        while (pos < len && buf[pos] == ' ') ++pos;
        if (pos >= len) break;
        param = &buf[pos];
        while (pos < len && buf[pos] != ' ') ++pos;
        if (pos >= len) break;
        buf[pos++] = '\0';

        if (!strcmp((const char*)cmd, "cs"))
        {
            if (VERSION_CS != atoi((const char*)param))
            {
                GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)"~Your client is outdated!~");
                Logger::log().error() << "Client is outdated";
                CloseClientSocket();
                SDL_Delay(3250);
                return;
            }
            Logger::log().info() << "Client version confirmed";
            continue;
        }
        if (!strcmp((const char*)cmd, "sc"))
        {
            if (VERSION_SC != atoi((const char*)param))
            {
                GuiManager::getSingleton().sendMsg(GuiManager::WIN_TEXTWINDOW, GuiImageset::GUI_LIST_MSGWIN, GuiManager::MSG_ADD_ROW, (void*)"~The server is outdated!\nSelect a different one!~");
                CloseClientSocket();
                SDL_Delay(3250);
                return;
            }
            Logger::log().info() << "Server version confirmed";
            continue;
        }
        if (!strcmp((const char*)cmd, "ac"))
        {
            continue;
        }
        if (!strcmp((const char*)cmd, "fc"))
        {
            continue;
        }
        if (!strcmp((const char*)cmd, "mz")) // MapSize.
        {
            continue;
        }
        if (!strcmp((const char*)cmd, "sn")) // Sound.
        {
            if (!strcmp((const char*)param, "false"))
            {
                ;
            }
            continue;
        }
        if (!strcmp((const char*)cmd, "skf"))
        {
            checkFileStatus((const char*)cmd, (char*)param, ServerFile::FILE_SKILLS);
            continue;
        }
        if (!strcmp((const char*)cmd, "spf"))
        {
            checkFileStatus((const char*)cmd, (char*)param, ServerFile::FILE_SPELLS);
            continue;
        }
        if (!strcmp((const char*)cmd, "stf"))
        {
            checkFileStatus((const char*)cmd, (char*)param, ServerFile::FILE_SETTINGS);
            continue;
        }
        if (!strcmp((const char*)cmd, "bpf"))
        {
            checkFileStatus((const char*)cmd, (char*)param, ServerFile::FILE_BMAPS);
            continue;
        }
        if (!strcmp((const char*)cmd, "amf"))
        {
            checkFileStatus((const char*)cmd, (char*)param, ServerFile::FILE_ANIMS);
            continue;
        }
        Logger::log().error() << "Got setup for a command we don't understand: " << cmd << " " << param;
        Option::getSingleton().setGameStatus(Option::GAME_STATUS_START);
        CloseSocket();
        SDL_Delay(3250);
        return;
    }
    Option::getSingleton().setGameStatus(Option::GAME_STATUS_REQUEST_FILES);
}

//================================================================================================
// .
//================================================================================================
void Network::checkFileStatus(const char *cmd, char *param, int fileNr)
{
    ServerFile::getSingleton().checkFileStatus(cmd, param, fileNr);
}

//================================================================================================
// .
//================================================================================================
void Network::DataCmd(unsigned char *data, int len)
{
    const int DATA_PACKED_CMD = 1<<7;
    // ////////////////////////////////////////////////////////////////////
    // check for valid command:
    // 0 = NC, 1 = SERVER_FILE_SKILLS, 2 = SERVER_FILE_SPELLS, (...)
    // ////////////////////////////////////////////////////////////////////
    unsigned char data_type = data[0];
    unsigned char data_cmd  = (data_type &~DATA_PACKED_CMD) -1;
    if (data_cmd >= ServerFile::FILE_SUM)
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
        // Warning! if the uncompressed size of a incoming compressed(!) file is larger then
        // dest_len default setting, the file is cutted and the rest skiped.
        // Look at the zlib docu for more info.
        unsigned long dest_len = 512 * 1024;
        dest = new char[dest_len];
        uncompress((unsigned char *)dest, &dest_len, (unsigned char *)data, len);
        data = (unsigned char*)dest;
        len  = dest_len;
    }
    // ////////////////////////////////////////////////////////////////////
    // Save the file.
    // ////////////////////////////////////////////////////////////////////
    std::ofstream out(ServerFile::getSingleton().getFilename(data_cmd), std::ios::out|std::ios::binary);
    if (!out)
        Logger::log().error()  << "save data cmd file : writing of file "
        << ServerFile::getSingleton().getFilename(data_cmd) << " failed.";
    else
        out.write((char*)data, len);
    delete[] dest;
    ServerFile::getSingleton().setStatus(data_cmd, ServerFile::STATUS_OK);
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
            //(buf, "GROUP CMD: %s (%d)", data, len);
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
    //TileManager::getSingleton().map_udate_flag = 2;
    /*
        if ((gui_interface_npc && gui_interface_npc->status != GUI_INTERFACE_STATUS_WAIT) &&
                ((!len && cpl.menustatus == MENU_NPC) || (len && cpl.menustatus != MENU_NPC)))
        {
            //sound_play_effect(SOUND_SCROLL, 0, 0, 100);
        }
    */


//GuiManager::getSingleton().addTextline(WIN_TEXTWINDOW, GUI_LIST_MSGWIN, (const char*)(data+1));

    GuiDialog::getSingleton().reset();
    if (len)
    {
        int mode = *data;
        int pos =1;
        if (!GuiDialog::getSingleton().load(mode, (char*)data, len, pos))
        {
            Logger::log().error() << "INVALID GUI CMD";
            return;
        }
        GuiDialog::getSingleton().show();


        /*
                gui_interface_npc->win_length = precalc_interface_npc();
                interface_mode = mode;
                cpl.menustatus = MENU_NPC;
                gui_interface_npc->startx = 400-(Bitmaps[BITMAP_NPC_INTERFACE]->bitmap->w / 2);
                gui_interface_npc->starty = 50;
                // Prefilled (and focused) textfield
                if (gui_interface_npc->used_flag&GUI_INTERFACE_TEXTFIELD)
                {
                    gui_interface_npc->input_flag = true;
                    open_input_mode(240);
                    textwin_putstring(gui_interface_npc->textfield.text);
                    cpl.input_mode = INPUT_MODE_NPCDIALOG;
                    HistoryPos = 0;
                }
        */
    }
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
//
//================================================================================================
void Network::CreatePlayerAccount()
{
    //   (buf, "nc %s %d %d %d %d %d %d %d", nc->char_arch[nc->gender_selected], nc->stats[0], nc->stats[1], nc->stats[2], nc->stats[3], nc->stats[4], nc->stats[5], nc->stats[6]);
    //cs_write_string("nc human_male 14 14 13 12 12 12 12 0");
}

//================================================================================================
// UpdateItemCmd updates some attributes of an item.
//================================================================================================
void Network::ItemUpdateCmd(unsigned char *data, int len)
{
    /*
        int     weight, loc, tag, face, sendflags, flags, pos = 0, nlen, anim, nrof, quality=254, condition=254;
        uint8   direction;
        char    name[MAX_BUF];
        item   *ip, *env = NULL;
        uint8   animspeed;

        TileManager::getSingleton().map_udate_flag = 2;
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
        TileManager::getSingleton().map_udate_flag = 2;
    */
}

//================================================================================================
// .
//================================================================================================
void Network::ItemDeleteCmd(unsigned char *data, int len)
{
    int pos = 0, tag;
    while (pos < len)
    {
        tag = GetInt_String(data);
        pos += 4;
        //delete_item(tag);
    }
    if (pos > len)
        Logger::log().error() <<  "DeleteCmd: Overread buffer: " << pos << " > " << len;
    //TileManager::getSingleton().map_udate_flag = 2;
}

//================================================================================================
// ItemCmd with sort order normal (add to end.
//================================================================================================
void Network::ItemXCmd(unsigned char *data, int len)
{
    Item::getSingleton().ItemXYCmd(data, len, false);
}

//================================================================================================
// ItemCmd with sort order reversed (add to front).
//================================================================================================
void Network::ItemYCmd(unsigned char *data, int len)
{
    Item::getSingleton().ItemXYCmd(data, len, true);
}

//================================================================================================
// Analyze /<cmd> type commands the player has typed in the console or bound to a key.
// Sort out the "client intern" commands and expand or pre process them for the server.
// Return: true = don't send command to server
//         false= send command to server
//================================================================================================
bool Network::console_command_check(String cmd)
{
    for (int i = 0; i < CONSOLE_CMD_SUM; ++i)
    {
        if (StringUtil::startsWith(cmd, mConsoleCmd[i].cmd, true))
        {
            int len = (int)strlen(mConsoleCmd[i].cmd);
Logger::log().error() << "before: " << cmd;
            cmd = cmd.substr(len, cmd.size()-len);
Logger::log().error() << "after: " << cmd;
            do_console_cmd(cmd, i);
            return true;;
        }
    }
    return false;
}

//================================================================================================
//
//================================================================================================
void Network::do_console_cmd(String &stCmd, int cmd)
{
    return;
}

