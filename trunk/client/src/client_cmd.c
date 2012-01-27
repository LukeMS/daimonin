/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.

    Copyright (C) 2008 Michael Toennies

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    The author can be reached via e-mail to info@daimonin.org
*/

/* This file deals with higher level functions for sending informations 
 * and commands to the server. Most low level functions are in socket.c
 */

#include "include.h"

/* The *_REAL macros are not directly used in the client_cmd_* functions.
 * Rather, they provide the underlying functionality for the non-_REAL macros,
 * which are the ones used directly. The purpose of this is so we can log on a
 * development client when DEBUG_CLIENT_CMD is defined. */
#define START_REAL(_SL_, _BUF_, _CMD_, _FLAGS_) \
    memset((_SL_), 0, sizeof(SockList)); \
    (_SL_)->buf = (_BUF_); \
    (_SL_)->cmd = (_CMD_); \
    (_SL_)->flags = (_FLAGS_)
#define ADDUINT8_REAL(_SL_, _V_) \
    if ((_SL_)->buf) \
    { \
        *((uint8 *)((_SL_)->buf + (_SL_)->len)) = (uint8)(_V_); \
    } \
    else \
    { \
        *((uint8 *)((_SL_)->defbuf + (_SL_)->len)) = (uint8)(_V_); \
    } \
    (_SL_)->len += 1
#define ADDUINT16_REAL(_SL_, _V_) \
    if ((_SL_)->buf) \
    { \
        *((uint16 *)((_SL_)->buf + (_SL_)->len)) = adjust_endian_int16(_V_); \
    } \
    else \
    { \
        *((uint16 *)((_SL_)->defbuf + (_SL_)->len)) = adjust_endian_int16(_V_); \
    } \
    (_SL_)->len += 2
#define ADDUINT32_REAL(_SL_, _V_) \
    if ((_SL_)->buf) \
    { \
        *((uint32 *)((_SL_)->buf + (_SL_)->len)) = adjust_endian_int32(_V_); \
    } \
    else \
    { \
        *((uint32 *)((_SL_)->defbuf + (_SL_)->len)) = adjust_endian_int32(_V_); \
    } \
    (_SL_)->len += 4
#define ADDBUFFER_REAL(_SL_, _BUF_, _LEN_) \
    if ((_SL_)->buf) \
    { \
        memcpy((_SL_)->buf + (_SL_)->len, (_BUF_), (_LEN_)); \
    } \
    else \
    { \
        memcpy((_SL_)->defbuf + (_SL_)->len, (_BUF_), (_LEN_)); \
    } \
    (_SL_)->len += (_LEN_)
#define ADDSTRING_REAL(_SL_, _BUF_, _LEN_) \
    if ((_SL_)->buf) \
    { \
        memcpy((_SL_)->buf + (_SL_)->len, (_BUF_), (_LEN_)); \
        *((_SL_)->buf + (_SL_)->len + (_LEN_)) = 0; \
    } \
    else \
    { \
        memcpy((_SL_)->defbuf + (_SL_)->len, (_BUF_), (_LEN_)); \
        *((_SL_)->defbuf + (_SL_)->len + (_LEN_)) = 0; \
    } \
    (_SL_)->len += (_LEN_) + 1
#define FINISH_REAL(_SL_) \
    send_command_binary((_SL_)->cmd, \
                        ((_SL_)->buf) ? (_SL_)->buf : (_SL_)->defbuf, \
                        (_SL_)->len, (_SL_)->flags)

#ifdef DEBUG_CLIENT_CMD
# define START(_SL_, _BUF_, _CMD_, _FLAGS_) \
    LOG(LOG_MSG, "Sent CMD (%u):%u", (_FLAGS_), (_CMD_)); \
    START_REAL((_SL_), (_BUF_), (_CMD_), (_FLAGS_))
# define ADDUINT8(_SL_, _V_) \
    LOG(LOG_MSG, " U8:%u", (_V_)); \
    ADDUINT8_REAL((_SL_), (_V_))
# define ADDUINT16(_SL_, _V_) \
    LOG(LOG_MSG, " U16:%u", (_V_)); \
    ADDUINT16_REAL((_SL_), (_V_))
# define ADDUINT32(_SL_, _V_) \
    LOG(LOG_MSG, " U32:%u", (_V_)); \
    ADDUINT32_REAL((_SL_), (_V_))
# define ADDBUFFER(_SL_, _BUF_, _LEN_, _SSH_) \
    LOG(LOG_MSG, " BUF (%u):%s", (_LEN_), ((_SSH_)) ? "*****" : (_BUF_)); \
    ADDBUFFER_REAL((_SL_), (_BUF_), (_LEN_))
# define ADDSTRING(_SL_, _BUF_, _LEN_, _SSH_) \
    LOG(LOG_MSG, " STR (%u):%s", (_LEN_), ((_SSH_)) ? "*****" : (_BUF_)); \
    ADDSTRING_REAL((_SL_), (_BUF_), (_LEN_))
# define FINISH(_SL_) \
    LOG(LOG_MSG, "\n"); \
    FINISH_REAL((_SL_))
#else
# define START(_SL_, _BUF_, _CMD_, _FLAGS_) \
    START_REAL((_SL_), (_BUF_), (_CMD_), (_FLAGS_))
# define ADDUINT8(_SL_, _V_) \
    ADDUINT8_REAL((_SL_), (_V_))
# define ADDUINT16(_SL_, _V_) \
    ADDUINT16_REAL((_SL_), (_V_))
# define ADDUINT32(_SL_, _V_) \
    ADDUINT32_REAL((_SL_), (_V_))
# define ADDBUFFER(_SL_, _BUF_, _LEN_, _SSH_) \
    ADDBUFFER_REAL((_SL_), (_BUF_), (_LEN_))
# define ADDSTRING(_SL_, _BUF_, _LEN_, _SSH_) \
    ADDSTRING_REAL((_SL_), (_BUF_), (_LEN_))
# define FINISH(_SL_) \
    FINISH_REAL((_SL_))
#endif

/* helper array to cast a key num input to a server dir value */
static int move_dir[] = {0,6,5,4,7,0,3,8,1,2};

static char *SplitCommand(const char *command);
static uint8 CheckCommand(char *cmd, char *params);

/* Sends a ping. A ping is a small package of data. By measuring the time taken
 * for a reply we can provide an estimation of the connection speed to the
 * server.
 *
 * The data pinged to the server is the server tick of the current update of
 * the so-called 'ping string'. This determines what the server will send in
 * reply (see server_cmd.c). */
void client_cmd_ping(uint32 ping)
{
    char     buf[TINY_BUF];
    int      len;
    SockList sl;

    len = sprintf(buf, "%x", ping);
    START(&sl, NULL, CLIENT_CMD_PING, SEND_CMD_FLAG_STRING);
    ADDSTRING(&sl, buf, len, 0);
    FINISH(&sl);
}

/* Sends the setup string. This is the handshake command after the client
 * connects and the first data which is exchanged between client and server
 * (excluding pings). With the response from the server we get endian info
 * which enables us to send binary data (without fixed shifting). */
void client_cmd_setup(void)
{
    SockList sl;
    char     buf_sound[TINY_BUF],
             buf[MEDIUM_BUF];
    int      len;

    if (SoundStatus)
    {
        sprintf(buf_sound, "%d|%x",
                srvfile[SRV_CLIENT_SOUNDS].len,
                srvfile[SRV_CLIENT_SOUNDS].crc);
    }
    else
    {
        strcpy(buf_sound, "0");
    }

    sprintf(buf, "dv %u.%u.%u pv %u mz %dx%d geo %d|%d amf %d|%x bpf %d|%x stf %d|%x skf %d|%x sn %s spf %d|%x",
            DAI_VERSION_RELEASE, DAI_VERSION_MAJOR, DAI_VERSION_MINOR,
            PROTOCOL_VERSION, MapStatusX, MapStatusY,
            locator.client.lx, locator.client.ly,
            srvfile[SRV_CLIENT_ANIMS].len, srvfile[SRV_CLIENT_ANIMS].crc,
            srvfile[SRV_CLIENT_BMAPS].len, srvfile[SRV_CLIENT_BMAPS].crc,
            srvfile[SRV_CLIENT_SETTINGS].len, srvfile[SRV_CLIENT_SETTINGS].crc,
            srvfile[SRV_CLIENT_SKILLS].len, srvfile[SRV_CLIENT_SKILLS].crc,
            buf_sound,
            srvfile[SRV_CLIENT_SPELLS].len, srvfile[SRV_CLIENT_SPELLS].crc);
    len = strlen(buf);
    START(&sl, NULL, CLIENT_CMD_SETUP, SEND_CMD_FLAG_STRING);
    ADDSTRING(&sl, buf, len, 0);
    FINISH(&sl);
}

/* Requests a srvfile (see srvfile.c). */
void client_cmd_requestfile(uint8 index)
{
    SockList sl;

    /* for binary stuff we better use the socklist system */
    START(&sl, NULL, CLIENT_CMD_REQUESTFILE, SEND_CMD_FLAG_FIXED);
    ADDUINT8(&sl, index);
    FINISH(&sl);

    /* The following 1 second delay (in fact 900ms seems to be enough but lets
     * add another 100ms for safety -- hardly human-noticeable) seems to be
     * necessary to allow the client to 'catch up with itself'. I am not clear
     * on why it is necessary at this stage (for example, a delay when
     * receiving the data from the server or writing to disk would seem to make
     * more sense, but does not work) but whatever, it is needed after r6285 so
     * I assume the speedups and efficiency improvements are sufficient to
     * cause the client to get ahead of itself. By addiing a delay here we only
     * cause that pause for thought when the client is actually getting a file
     * from the server, which in fact is no bad thing (gives the player a
     * tangible indication that work is being done).
     * -- Smacky 20110514 */
    /* As of r6299 we need about 1100ms to properly load the largest srv_file
     * (FACEINFO). This implies it is related to file size which obviously we
     * do not know here, so this is not the best place. For safety we'll give
     * it a flat 1200ms then.
     * -- Smacky 20110516 */
    SDL_Delay(1200);
}

/* Sends an account name to the server for it to check validity (see
 * protocol.c) and whether this name exists. This is only used when the client
 * tries to create a new account. */
void client_cmd_checkname(char *name)
{
    SockList sl;
    int      len = strlen(name);

    START(&sl, NULL, CLIENT_CMD_CHECKNAME, 0);
    ADDSTRING(&sl, name, len, 0);
    FINISH(&sl);
}

/* Attempts to login to the named account with the given password. The server
 * will do the validity/existence checks as for client_cmd_checkname(). */
void client_cmd_login(int mode, char *name, char *pass)
{
    SockList sl;
    int      len_name = strlen(name),
             len_pass = strlen(pass);

    START(&sl, NULL, CLIENT_CMD_LOGIN, 0);
    ADDUINT8(&sl, mode);
    ADDSTRING(&sl, name, len_name, 0);
    ADDSTRING(&sl, pass, len_pass, 1);
    FINISH(&sl);
}

/* Sends basic info about a new character that we have created. As well as
 * checking the stats, etc are legal, the server will also check that the
 * player name is not already taken. If the name is already taken and the
 * character is from the pre-account era, the server will reply to that effect
 * and this cmd will need to be resent, this time with a so-called 'reclaim
 * password' ini order to move the name into this account (the character will
 * be wholly new apart from the name). */
void client_cmd_newchar(_server_char *nc)
{
    _server_char *sc = first_server_char;
    uint8         i = 0;
    SockList      sl;
    int           len_name = strlen(cpl.name),
                  len_pass = strlen(cpl.reclaim_password);

    /* lets find the entry number */
    while (sc)
    {
        /* get our current template */
        if (!strcmp(sc->name, new_character.name))
        {
            break;
        }

        i++;
    }

    START(&sl, NULL, CLIENT_CMD_NEWCHAR, SEND_CMD_FLAG_DYNAMIC);
    ADDUINT8(&sl, i);
    ADDUINT8(&sl, nc->gender_selected);
    ADDUINT8(&sl, nc->skill_selected);
    ADDSTRING(&sl, cpl.name, len_name, 0);
    ADDSTRING(&sl, cpl.reclaim_password, len_pass, 1);
    FINISH(&sl);
}

/* Tells the server to delete the named character from our account. */
void client_cmd_delchar(char *name)
{
    SockList sl;
    int      len = strlen(name);

    START(&sl, NULL, CLIENT_CMD_DELCHAR, SEND_CMD_FLAG_DYNAMIC);
    ADDSTRING(&sl, name, len, 0);
    FINISH(&sl);
}

/* Tells the server to add the named player to the gameworld. */
void client_cmd_addme(char *name)
{
    SockList sl;
    int      len = strlen(name);

    START(&sl, NULL, CLIENT_CMD_ADDME, SEND_CMD_FLAG_DYNAMIC);
    ADDSTRING(&sl, name, len, 0);
    FINISH(&sl);
}

/* Requests the numbered face (see face.c). */
void client_cmd_face(uint16 num)
{
    SockList sl;

    START(&sl, NULL, CLIENT_CMD_FACE, 0);
    ADDUINT16(&sl, num);
    FINISH(&sl);
}

/* Sends a player move command. */
void client_cmd_move(uint8 dir, uint8 mode)
{
    SockList sl;

    START(&sl, NULL, CLIENT_CMD_MOVE, SEND_CMD_FLAG_FIXED);
    ADDUINT8(&sl, move_dir[dir]);
    ADDUINT8(&sl, mode);
    FINISH(&sl);
}

/* Sends an apply command. */
void client_cmd_apply(int tag)
{
    SockList sl;

    START(&sl, NULL, CLIENT_CMD_APPLY, SEND_CMD_FLAG_FIXED);
    ADDUINT32(&sl, tag);
    FINISH(&sl);
}

/* Sends an examine command. */
void client_cmd_examine(int tag)
{
    SockList sl;

    START(&sl, NULL, CLIENT_CMD_EXAMINE, SEND_CMD_FLAG_FIXED);
    ADDUINT32(&sl, tag);
    FINISH(&sl);
}

/* Sends an object move command. */
void client_cmd_invmove(int loc, int tag, int nrof)
{
    SockList sl;

    START(&sl, NULL, CLIENT_CMD_INVMOVE, SEND_CMD_FLAG_FIXED);
    ADDUINT32(&sl, loc);
    ADDUINT32(&sl, tag);
    ADDUINT32(&sl, nrof);
    FINISH(&sl);
}

/* Sends a talk command. The topic iis always lowercased before bing sent. This
 * makes it much easier to deal with server-side (by scripts especially) and
 * the player needb't worry about capitalisation. */
void client_cmd_guitalk(sint8 mode, char *topic)
{
    uint16   c;
    SockList sl;
    int      len = strlen(topic);

    for (c = 0; *(topic + c); c++)
    {
        *(topic + c) = tolower(*(topic + c));
    }

    textwin_show_string(0, NDI_COLR_OLIVE, "Topic: %s", topic);
    START(&sl, NULL, CLIENT_CMD_GUITALK, SEND_CMD_FLAG_DYNAMIC);
    ADDUINT8(&sl, mode);
    ADDBUFFER(&sl, topic, len, 0);
    FINISH(&sl);
}

/* Sends an inventory lock command. */
void client_cmd_lock(int mode, int tag)
{
    SockList sl;

    START(&sl, NULL, CLIENT_CMD_LOCK, SEND_CMD_FLAG_FIXED);
    ADDUINT8(&sl, mode);
    ADDUINT32(&sl, tag);
    FINISH(&sl);
}

/* Sends an inventory mark command. */
void client_cmd_mark(int tag)
{
    SockList sl;

    START(&sl, NULL, CLIENT_CMD_MARK, SEND_CMD_FLAG_FIXED);
    ADDUINT32(&sl, tag);
    FINISH(&sl);
}

/* Sends a fire command. */
void client_cmd_fire(int num, int mode, char *name)
{
    SockList sl;
    int      len = strlen(name);

    START(&sl, NULL, CLIENT_CMD_FIRE, 0);
    ADDUINT32(&sl, move_dir[num]);
    ADDUINT32(&sl, mode);

    if (name)
    {
        ADDBUFFER(&sl, name, len, 0);
    }

    ADDUINT8(&sl, 0); /* be sure we finish with zero - server will check it */
    FINISH(&sl);
}

/* Sends a higher level /command (eg, /tell, /say, etc) as a string. IOW this
 * really a generic wrapper not a binary cmd. */
void client_cmd_generic(const char *command)
{
    char  buf[LARGE_BUF],
         *token,
         *end;

    /* Copy a normalized (leading, trailing, and excess inline whitespace-
     * stripped) command to buf: */
    sprintf(buf, "%s", normalize_string(command));

    /* Now go through buf, possibly separating multicommands.
     * Each command (before separation) is pointed to by token: */
    for (token = buf; token && *token; token = (end) ? end + 1 : NULL)
    {
        char  *p,
               cmd[MEDIUM_BUF],
               params[MEDIUM_BUF];
        uint8  c;

        if (*token != '/')
        {
            char tmpbuf[LARGE_BUF];

            sprintf(tmpbuf, "/say %s", token);
            sprintf(token, "%s", tmpbuf);
        }

        end = SplitCommand(token);

        /* Now we copy token to cmd... */
        sprintf(cmd, "%s", token);

        /* And separate the params too. */
        *params = '\0';

        if ((p = strchr(cmd, ' ')))
        {
            *(p++) = '\0';

            if (p)
            {
                sprintf(params, "%s", p);
            }
        }

        /* Lowercase cmd. */
        for (c = 0; *(cmd + c); c++)
        {
            *(cmd + c) = tolower(*(cmd + c));
        }

#if 0
        LOG(LOG_DEBUG,">>>%s<<<\n>>>%s<<<\n", cmd, params);
#endif

        if (!CheckCommand(cmd, params))
        {
            char     tmpbuf[LARGE_BUF];
            SockList sl;
            int      len;

            sprintf(tmpbuf, "%s%s%s", cmd + 1, (*params) ? " " : "", params);
            len = strlen(tmpbuf);
            START(&sl, NULL, CLIENT_CMD_GENERIC, SEND_CMD_FLAG_STRING);
            ADDBUFFER(&sl, tmpbuf, len, 0);
            FINISH(&sl);
        }
    }
}

/* Splits command at the next #, returning a pointer to the occurrence (which
 * is overwritten with \0 first) or NULL if no next multicommand is found or
 * command is chat, etc. */
static char *SplitCommand(const char *command)
{
    char *c = NULL;

    /* Only look for a multicommand if the command is not one of these: */
    if (strnicmp(command, "/create ", 8) &&
#ifdef USE_CHANNELS
        strnicmp(command, "/channel ", 9) &&
#endif
        strnicmp(command, "/gsay ", 6) &&
        strnicmp(command, "/qlist ", 7) &&
        strnicmp(command, "/reply ", 7) &&
        strnicmp(command, "/say ", 5) &&
        strnicmp(command, "/shout ", 7) &&
        strnicmp(command, "/talk ", 6) &&
        strnicmp(command, "/tell ", 6))
    {
        if ((c = strchr(command, '#'))) /* multicommand separator '#' */
        {
            *c = '\0';
        }
    }

    return c;
}

/* Analyzes /commands to separate commands which are dealt with purely by the
 * client (return 1) and those which are expanded or preprocessed before being
 * sent to the server (return 0). */
static uint8 CheckCommand(char *cmd, char *params)
{
    /* Commands which require preprocessing before (and to determine if) they
     * are sent to the server, */
    /* This is for /apply commands where the item is specified as a string (eg,
     * /apply lamp) as opposed to ?M_APPLY commands where the item is
     * always what is under the cursor.
     *
     * So with /apply you can bind the apply command to a key for example to always
     * apply a lamp if it's in your inv or at your feet.
     *
     * For some unknown reason '/apply' on its own is bound to SPACE by default
     * (?M_APPLY is A). Obviously this will never specify an item.
     *
     * In these circumstances (no item is specified) we act as if ?M_APPLY was
     * issued and use the item under the cursor.
     * -- Smacky 20090730 */
    if (!strcmp(cmd, "/apply"))
    {
        int   tag;
        item *obj;

        if (*params)
        {
            tag = locate_item_tag_from_name(params);
        }
        else
        {
            if (cpl.inventory_win == IWIN_BELOW)
            {
                tag = cpl.win_below_tag;
            }
            else
            {
                tag = cpl.win_inv_tag;
            }
        }

        if (tag == -1 ||
            !(obj = locate_item(tag)))
        {
//            if (*params)
//            {
//                textwin_show_string(0, NDI_COLR_OLIVE, "%s %s", cmd, params);
//            }

            textwin_show_string(0, NDI_COLR_WHITE, "No %sitem could be found!",
                               (*params) ? "such " : "");
        }
        else
        {
            textwin_show_string(0, NDI_COLR_OLIVE, "%s %s", cmd, obj->s_name);
            client_cmd_apply(tag);
        }

        return 1;
    }
    else if (!strcmp(cmd, "/qlist"))
    {
        client_cmd_guitalk(GUI_NPC_MODE_QUEST, params);

        return 1;
    }
    else if (!strcmp(cmd, "/reply"))
    {
//        if (!*params)
//        {
//            textwin_show_string(0, NDI_COLR_WHITE, "usage: /reply <message>");
//        }

        if (cpl.reply[0] == '\0')
        {
            textwin_show_string(0, NDI_COLR_WHITE, "There is no one to whom you can /reply!");

            return 1;
        }

        sprintf(cmd, "/tell %s", cpl.reply);

        return 0;
    }
    else if (!strcmp(cmd, "/talk"))
    {
        if (*params)
        {
            client_cmd_guitalk(GUI_NPC_MODE_NPC, params);
        }

        return 1;
    }
    else if (!strcmp(cmd, "/target"))
    {
        if (!stricmp(params, "enemy"))
        {
            sprintf(params, "0");
        }
        else if (!stricmp(params, "friend"))
        {
            sprintf(params, "1");
        }
        else if (!stricmp(params, "self"))
        {
            sprintf(params, "2");
        }

        return 0;
    }

    /* Client-side only commands, */
    if (!strcmp(cmd, "/buddy"))
    {
        buddy_command(params);

        return 1;
    }
    else if (!strcmp(cmd, "/cfilter"))
    {
        chatfilter_command(params);

        return 1;
    }
    else if (!strcmp(cmd, "/f1"))
    {
        quickslot_key(NULL, 0);

        return 1;
    }
    else if (!strcmp(cmd, "/f2"))
    {
        quickslot_key(NULL, 1);

        return 1;
    }
    else if (!strcmp(cmd, "/f3"))
    {
        quickslot_key(NULL, 2);

        return 1;
    }
    else if (!strcmp(cmd, "/f4"))
    {
        quickslot_key(NULL, 3);

        return 1;
    }
    else if (!strcmp(cmd, "/f5"))
    {
        quickslot_key(NULL, 4);

        return 1;
    }
    else if (!strcmp(cmd, "/f6"))
    {
        quickslot_key(NULL, 5);

        return 1;
    }
    else if (!strcmp(cmd, "/f7"))
    {
        quickslot_key(NULL, 6);

        return 1;
    }
    else if (!strcmp(cmd, "/f8"))
    {
        quickslot_key(NULL, 7);

        return 1;
    }
    else if (!strcmp(cmd, "/grid"))
    {
        options.grid = !options.grid;

        return 1;
    }
    else if (!strcmp(cmd, "/ignore"))
    {
        ignore_command(params);

        return 1;
    }
    else if (!strcmp(cmd, "/imagestats"))
    {
        textwin_show_string(0, NDI_COLR_WHITE,
                           "IMAGE-LOADING-STATISTICS\n"\
                           "==========================================\n"\
                           "Sprites in Memory: %d\n"\
                           "TrueColors: %d\n"\
                           "Fires: %d\n"\
                           "Colds: %d\n"\
                           "Electricities: %d\n"\
                           "Fogofwars in Memory: %d\n"\
                           "Infravisions in Memory: %d\n"\
                           "Xrayvisions in Memory: %d",
                           ImageStats.loadedsprites,
                           ImageStats.truecolors,
                           ImageStats.fires,
                           ImageStats.colds,
                           ImageStats.electricities,
                           ImageStats.fogofwars,
                           ImageStats.infravisions,
                           ImageStats.xrayvisions);
        return 1;
    }
    else if (!strcmp(cmd, "/keybind"))
    {
        map_udate_flag = 2;

        if (cpl.menustatus != MENU_KEYBIND)
        {
            keybind_status = KEYBIND_STATUS_NO;
            cpl.menustatus = MENU_KEYBIND;
        }
        else
        {
            save_keybind_file();
            cpl.menustatus = MENU_NO;
        }

        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
        reset_keys();

        return 1;
    }
    else if (!strcmp(cmd, "/kills"))
    {
        kill_command(params);

        return 1;
    }
    else if (!strcmp(cmd, "/markdmbuster"))
    {
        markdmbuster();

        return 1;
    }
    else if (!strcmp(cmd, "/ready_spell"))
    {
        uint8 i;

//        if (!*params)
//        {
//            textwin_show_string(0, NDI_COLR_WHITE, "usage: /ready_spell <spell name>");
//        }

        for (i = 0; i < SPELL_LIST_MAX; i++)
        {
            uint8 j;

            for (j = 0; j < DIALOG_LIST_ENTRY; j++)
            {
                if (spell_list[i].entry[0][j].flag >= LIST_ENTRY_USED)
                {
                    if (!strcmp(spell_list[i].entry[0][j].name, params))
                    {
                        if (spell_list[i].entry[0][j].flag == LIST_ENTRY_KNOWN)
                        {
                            fire_mode.spell = &spell_list[i].entry[0][j];
                            fire_mode.mode = FIRE_MODE_SPELL_ID;
                            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0,
                                              0, MENU_SOUND_VOL);
                            textwin_show_string(0, NDI_COLR_WHITE, "Spell readied");

                            return 1;
                        }
                    }
                }

                if (spell_list[i].entry[1][j].flag >= LIST_ENTRY_USED)
                {
                    if (!strcmp(spell_list[i].entry[1][j].name, params))
                    {
                        if (spell_list[i].entry[1][j].flag == LIST_ENTRY_KNOWN)
                        {
                            fire_mode.spell = &spell_list[i].entry[1][j];
                            fire_mode.mode = FIRE_MODE_SPELL_ID;
                            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0,
                                              0, MENU_SOUND_VOL);
                            textwin_show_string(0, NDI_COLR_WHITE, "Prayer readied");

                            return 1;
                        }
                    }
                }
            }
        }

        textwin_show_string(0, NDI_COLR_WHITE, "Unknown spell.");

        return 1;
    }
    /* TODO; check and remove/fix. */
    else if (!strcmp(cmd, "/reloadskinnow"))
    {
        textwin_show_string(0, NDI_COLR_LIME, "Reloading skin. This function is only for skin creating, and may be removed anytime!");
        skin_reload();
    }
    else if (!strcmp(cmd, "/reset"))
    {
        if (!stricmp(params, "buddy"))
        {
            textwin_show_string(0, NDI_COLR_WHITE, "Resetting buddy list!");
            buddy_list_clear();
            buddy_list_save();
        }
        else if (!stricmp(params, "ignore"))
        {
            textwin_show_string(0, NDI_COLR_WHITE, "Resetting ignore list!");
            ignore_list_clear();
            ignore_list_save();
        }
        else if (!stricmp(params, "chatfilter") ||
                 !stricmp(params, "cfilter"))
        {
            textwin_show_string(0, NDI_COLR_WHITE, "Resetting chatfilter list!");
            chatfilter_list_clear();
            chatfilter_list_save();
        }
        else if (!stricmp(params, "kills"))
        {
            textwin_show_string(0, NDI_COLR_WHITE, "Resetting kill list!");
            kill_list_clear();
            kill_list_save();
        }
        else if (!stricmp(params, "stats"))
        {
            textwin_show_string(0, NDI_COLR_WHITE, "Resetting stat-o-meter!");
            statometer.exp = 0;
            statometer.kills = 0;
            statometer.starttime = LastTick - 1;
            statometer.lastupdate = LastTick;
            statometer.exphour = 0.0f;
            statometer.killhour = 0.0f;
        }
        else if (!stricmp(params, "widgetstatus"))
        {
            widget_id_t id;

            textwin_show_string(0, NDI_COLR_WHITE, "Resetting widgetstatus!");

            for (id = 0; id < WIDGET_NROF; id++)
            {
                switch (id)
                {
                    case WIDGET_MIXWIN_ID:
                        break;

                    case WIDGET_PDOLL_ID:
                        if (options.playerdoll)
                        {
                            WIDGET_SHOW(id) = 1;
                        }

                        break;

                    case WIDGET_MAIN_INV_ID:
                    case WIDGET_IN_CONSOLE_ID:
                    case WIDGET_IN_NUMBER_ID:

                        break;

                    case WIDGET_STATOMETER_ID:
                        if (options.statsupdate)
                        {
                            WIDGET_SHOW(id) = 1;
                        }

                        break;

                    default:
                        WIDGET_SHOW(id) = 1;
                }
            }
        }
        else if (!stricmp(params, "widgets"))
        {
            textwin_show_string(0, NDI_COLR_WHITE, "Resetting widgets!");
            widget_deinit();
            widget_init();
        }
#if 0
        else
        {
            textwin_show_string(0, NDI_COLR_WHITE, "Usage: "\
                "~/reset buddy~ to reset the buddylist,\n"\
                "~/reset ignore~ to reset the ignorelist,\n"\
                "~/reset chatfilter~ to reset the chatfilter list,\n"\
                "~/reset kills~ to reset the kill list,\n"\
                "~/reset stats~ to reset the stat-o-meter,\n"\
                "~/reset widgets~ to reset the widgets, or\n"\
                "~/reset widgetstatus~ to show any previously hidden widgets.");
        }
#endif

        return 1;
    }
#ifdef DAI_DEVELOPMENT
    else if (!strcmp(cmd, "/searchpath"))
    {
        char **i,
             **j;

        for (i = j = PHYSFS_getSearchPath(); *i; i++)
        {
            textwin_show_string(0, NDI_COLR_WHITE, "[%s] is in the search path.", *i);
        }

        PHYSFS_freeList(j);

        return 1;
    }
#endif
    else if (!strcmp(cmd, "/shout_off"))
    {
        options.shoutoff = 1;
        textwin_show_string(0, NDI_COLR_WHITE, "Shout disabled");

        return 1;
    }
    else if (!strcmp(cmd, "/shout_on"))
    {
        options.shoutoff = 0;
        textwin_show_string(0, NDI_COLR_WHITE,"Shout enabled");

        return 1;
    }
    else if (!strnicmp(cmd, "/sleeptimer", strlen("/sleeptimer")))
    {
        sint32     hour,
                   min;
        struct tm *tmptime;

        if (!*params ||
            sscanf(params, "%d:%d", &hour, &min) != 2 ||
            hour < 0 ||
            hour > 23 ||
            min < 0 ||
            min > 59)
        {
//                textwin_show_string(0, NDI_COLR_WHITE, "Sleeptimer OFF\nUsage: /sleeptimer HH:MM");
                options.sleepcounter = 0;
        }

        options.sleepcounter = 1;

        if (time(&sleeptime) == -1)
        {
             LOG(LOG_ERROR, "ERROR:: Calendar time not available!\n");
        }

        tmptime = localtime(&sleeptime);

        if (hour < tmptime->tm_hour)
        {
            tmptime->tm_mday += 1;
        }

        tmptime->tm_hour = hour;
        tmptime->tm_min = min;

        if ((sleeptime = mktime(tmptime)) != -1)
        {
           char tmpbuf[MEDIUM_BUF];

           strftime(tmpbuf, sizeof tmpbuf, "%d-%m-%y %H:%M:%S",
                    localtime(&sleeptime));
           textwin_show_string(0, NDI_COLR_WHITE, "Sleeptime set to %s", tmpbuf);
        }

        return 1;
    }
    else if (!strcmp(cmd, "/statreset"))
    {
        statometer.exp = 0;
        statometer.kills = 0;
        statometer.starttime = LastTick - 1;
        statometer.lastupdate = LastTick;
        statometer.exphour = 0.0f;
        statometer.killhour = 0.0f;

        return 1;
    }
#ifdef DAI_DEVELOPMENT
    else if (!strcmp(cmd, "/teststretch"))
    {
        uint8 i,
              h[5][5] =
        {
            {1, 1, 1, 1, 1},
            {1, 3, 5, 3, 1},
            {1, 5, 10, 5, 1},
            {1, 3, 5, 3, 1},
            {1, 1, 1, 1, 1}
        };

        for (i = 6; i < 11; i++)
        {
            uint8 j;

            for (j = 6; j < 11; j++)
            {
                set_map_height(i, j, h[i - 6][j - 6]);
            }
        }

        map_udate_flag = 1;
        map_redraw_flag |= MAP_REDRAW_FLAG_NORMAL;

        return 1;
    }
#endif
    else if (!stricmp(cmd, "/version"))
    {
        PHYSFS_File *handle;
        char         buf[TINY_BUF];
 
        LOG(LOG_SYSTEM, "Loading '%s'... ", FILE_VERSION);

        if (!(handle = PHYSFS_openRead(FILE_VERSION)))
        {
            LOG(LOG_ERROR, "FAILED (%s)!\n", PHYSFS_getLastError());
            strcpy(buf, "UNKNOWN");
        }
        else if (PHYSFS_readString(handle, buf, sizeof(buf)) < 0)
        {
            LOG(LOG_ERROR, "FAILED (Empty file?)!\n");
            strcpy(buf, "UNKNOWN");
        }

        PHYSFS_close(handle);
        LOG(LOG_SYSTEM, "OK!\n");
        textwin_show_string(0, NDI_COLR_WHITE, "~Client version~: %u.%u.%u / %u (%s)",
                           DAI_VERSION_RELEASE, DAI_VERSION_MAJOR,
                           DAI_VERSION_MINOR, PROTOCOL_VERSION, buf);
        textwin_show_string(0, NDI_COLR_WHITE, "~Server version~: %u.%u.%u / %u",
                           options.server_version_release,
                           options.server_version_major,
                           options.server_version_minor,
                           options.server_protocol);

        return 1;
    }

    return 0;
}
