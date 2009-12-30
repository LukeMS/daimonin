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

    The author can be reached via e-mail to info@daimonin.net
*/

/* This file deals with higher level functions for sending informations 
 * and commands to the server. Most low level functions are in socket.c
 */

#include <include.h>
#include <stdio.h>

ClientSocket    csocket;

/* helper array to cast a key num input to a server dir value */
static int move_dir[] = {0,6,5,4,7,0,3,8,1,2};

/* helper functions for working with binary parms for the socklist */
static inline void SockList_AddShort(SockList *const sl, const uint16 data)
{
    if(sl->buf)
    	*((uint16 *)(sl->buf+sl->len)) = adjust_endian_int16(data);
    else
        *((uint16 *)(sl->defbuf+sl->len)) = adjust_endian_int16(data);
	sl->len+=2;
}
static inline void SockList_AddInt(SockList *const sl, const uint32 data)
{
    if(sl->buf)
    	*((uint32 *)(sl->buf+sl->len)) = adjust_endian_int32(data);
    else
        *((uint32 *)(sl->defbuf+sl->len)) = adjust_endian_int32(data);
	sl->len+=4;
}
static inline void SockList_AddBuffer(SockList *const sl, const char *const buf, const int len)
{
    if(sl->buf)
        memcpy(sl->buf+sl->len,buf,len);
    else
        memcpy(sl->defbuf+sl->len,buf,len);
    sl->len+=len;
}

static inline void SockList_AddString(SockList *const sl, const char *const buf)
{
    int len = strlen(buf);

    if(sl->buf)
    {
        memcpy(sl->buf+sl->len,buf,len);
        *(sl->buf+sl->len+len++) = 0; /* ensure the string is send with 0 end marker */
    }
    else
    {
        memcpy(sl->defbuf+sl->len,buf,len);
        *(sl->defbuf+sl->len+len++) = 0;
    }
    sl->len+=len;


}

/* Splits command at the next #,
* returning a pointer to the occurrence (which is overwritten with \0 first) or
* NULL if no next multicommand is found or command is chat, etc.
*/
static char *BreakMulticommand(const char *command)
{
    char *c = NULL;

    /* Only look for a multicommand if the command is not one of these: */
    if (strnicmp(command, "/create ", 8) &&
#ifdef USE_CHANNELS
        *command != '-' &&
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

/* Analyze /<cmd> type commands the player has typed in the console
 * or bound to a key. Sort out the "client intern" commands and
 * expand or pre process them for the server.
 * Return 0 and update cmd and params as necessary to send the command to the
 * server, or  1 not to (ie, command has been fully handled client-side). */
static uint8 CommandCheck(char *cmd, char *params)
{
#ifdef USE_CHANNELS
    /* i know hardcoding is most of the time bad, but the channel system will
     * be used really often. Also we can type directly the '-' in the
     * textwin. */
    if (*cmd == '-')
    {
        /* Only send if there is something to send! */
        if (*params)
        {
            char tmpbuf[MEDIUM_BUF];

            sprintf(tmpbuf, "/channel %s", cmd + 1);
            sprintf(cmd, "%s", tmpbuf);

            return 0;
        }

        return 1;
    }
#endif

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
//                draw_info_format(COLOR_DGOLD, "%s %s", cmd, params);
//            }

            draw_info_format(COLOR_DEFAULT, "No %sitem could be found!",
                             (*params) ? "such " : "");
        }
        else
        {
            draw_info_format(COLOR_DGOLD, "%s %s", cmd, obj->s_name);
            client_send_apply(tag);
        }

        return 1;
    }
    else if (!strcmp(cmd, "/qlist"))
    {
        send_talk_command(GUI_NPC_MODE_QUEST, params);

        return 1;
    }
    else if (!strcmp(cmd, "/reply"))
    {
//        if (!*params)
//        {
//            draw_info("usage: /reply <message>", COLOR_WHITE);
//        }

        if (!cpl.player_reply[0])
        {
            draw_info("There is no one to whom you can /reply!", COLOR_WHITE);

            return 1;
        }

        sprintf(cmd, "/tell %s", cpl.player_reply);

        return 0;
    }
    else if (!strcmp(cmd, "/talk"))
    {
        if (*params)
        {
            send_talk_command(GUI_NPC_MODE_NPC, params);
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
    else if (!strcmp(cmd, "/changeskin"))
    {
        char    tmpbuf[LARGE_BUF];
        Boolean newskin = FALSE;

//        if (!*params)
//        {
//            draw_info("usage: /changeskin <skin>", COLOR_GREEN);
//        }

        if (!stricmp(options.skin, params))
        {
            draw_info("You are already using that skin", COLOR_WHITE);
        }

        sprintf(tmpbuf, "skins/%s.zip", params);

        if (PHYSFS_exists(tmpbuf))
        {
            if (!PHYSFS_addToSearchPath(tmpbuf, 0))
            {
                LOG(LOG_MSG, "PHYSFS_addPath (%s) failed: %s\n",
                    tmpbuf, PHYSFS_getLastError());
            }
            else
            {
                newskin = TRUE;
            }
        }

        sprintf(tmpbuf, "skins/%s", params);

        if (PHYSFS_isDirectory(tmpbuf))
        {
            if (!PHYSFS_addToSearchPath(tmpbuf, 0))
            {
                LOG(LOG_MSG, "PHYSFS_addPath (%s) failed: %s\n",
                    tmpbuf, PHYSFS_getLastError());
            }
            else
            {
                newskin = TRUE;
            }
        }

        if (!newskin)
        {
            draw_info_format(COLOR_RED, "Skin '%s' not found, using old one.",
                             params);
        }
        else
        {
            if (strnicmp(options.skin, "subred", 6))
            {
                sprintf(tmpbuf, "skins/%s.zip", options.skin);

                if (!PHYSFS_removeFromSearchPath(tmpbuf))
                {
                    LOG(LOG_MSG, "PHYSFS_removePath (%s) failed: %s\n",
                        tmpbuf, PHYSFS_getLastError());
                }

                sprintf(tmpbuf, "skins/%s", options.skin);

                if (!PHYSFS_removeFromSearchPath(tmpbuf))
                {
                    LOG(LOG_MSG, "PHYSFS_removePath (%s) failed: %s\n",
                        tmpbuf, PHYSFS_getLastError());
                }
            }

            strncpy(options.skin, params, 63);
            save_options_dat();
            reload_skin();
            reload_icons();
        }

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
#ifdef DEVELOPMENT
    else if (!strcmp(cmd, "/grid"))
    {
        options.grid = !options.grid;

        return 1;
    }
#endif
    else if (!strcmp(cmd, "/ignore"))
    {
        ignore_command(params);

        return 1;
    }
    else if (!strcmp(cmd, "/imagestats"))
    {
        draw_info_format(COLOR_WHITE, "IMAGE-LOADING-STATISTICS");
        draw_info_format(COLOR_WHITE, "==========================================");
        draw_info_format(COLOR_WHITE, "Sprites in Memory: %d",
                         ImageStats.loadedsprites);
        draw_info_format(COLOR_WHITE, "TrueColors: %d",
                         ImageStats.truecolors);
        draw_info_format(COLOR_WHITE, "Greyscales in Memory: %d",
                         ImageStats.greyscales);
        draw_info_format(COLOR_WHITE, "Redscales in Memory: %d",
                         ImageStats.redscales);
        draw_info_format(COLOR_WHITE, "Fowscales in Memory: %d",
                         ImageStats.fowscales);

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
            save_keybind_file(KEYBIND_FILE);
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
//            draw_info("usage: /ready_spell <spell name>", COLOR_WHITE);
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
                            fire_mode_tab[FIRE_MODE_SPELL].spell = &spell_list[i].entry[0][j];
                            RangeFireMode = FIRE_MODE_SPELL;
                            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0,
                                              0, MENU_SOUND_VOL);
                            draw_info("Spell readied", COLOR_LBLUE);

                            return 1;
                        }
                    }
                }

                if (spell_list[i].entry[1][j].flag >= LIST_ENTRY_USED)
                {
                    if (!strcmp(spell_list[i].entry[1][j].name, cmd))
                    {
                        if (spell_list[i].entry[1][j].flag == LIST_ENTRY_KNOWN)
                        {
                            fire_mode_tab[FIRE_MODE_SPELL].spell = &spell_list[i].entry[1][j];
                            RangeFireMode = FIRE_MODE_SPELL;
                            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0,
                                              0, MENU_SOUND_VOL);
                            draw_info("Prayer readied", COLOR_LBLUE);

                            return 1;
                        }
                    }
                }
            }
        }

        draw_info("Unknown spell.", COLOR_LBLUE);

        return 1;
    }
    else if (!strcmp(cmd, "/reloadskinnow"))
    {
        draw_info_format(COLOR_GREEN, "Reloading skin. This function is only for skin creating, and may be removed anytime!");
        reload_skin();
        reload_icons();
    }
    else if (!strcmp(cmd, "/reset"))
    {
        if (!stricmp(params, "buddy"))
        {
            draw_info("Resetting buddy list!", COLOR_WHITE);
            buddy_list_clear();
            buddy_list_save();
        }
        else if (!stricmp(params, "ignore"))
        {
            draw_info("Resetting ignore list!", COLOR_WHITE);
            ignore_list_clear();
            ignore_list_save();
        }
        else if (!stricmp(params, "chatfilter") ||
                 !stricmp(params, "cfilter"))
        {
            draw_info("Resetting chatfilter list!", COLOR_WHITE);
            chatfilter_list_clear();
            chatfilter_list_save();
        }
        else if (!stricmp(params, "kills"))
        {
            draw_info("Resetting kill list!", COLOR_WHITE);
            kill_list_clear();
            kill_list_save();
        }
        else if (!stricmp(params, "stats"))
        {
            draw_info("Resetting stat-o-meter!", COLOR_WHITE);
            statometer.exp = 0;
            statometer.kills = 0;
            statometer.starttime = LastTick - 1;
            statometer.lastupdate = LastTick;
            statometer.exphour = 0.0f;
            statometer.killhour = 0.0f;
        }
        else if (!stricmp(params, "widgetstatus"))
        {
            int nID;
            draw_info("Resetting widgetstatus!", COLOR_WHITE);
            for (nID = 0; nID < TOTAL_WIDGETS; nID++)
            {
                switch (nID)
                {
                    case 10: // MIXWIN
                        break;
                    case 12: // PLAYERDOLL, actually this shouldn't be necessary as we can't override the option with mouse-hiding, but JIC
                        if (options.playerdoll)
                            cur_widget[nID].show = TRUE;
                        break;
                    case 17: // MAININV
                    case 19: // CONSOLE
                    case 20: // NUMBER
                        break;
                    case 21: // STATOMETER, actually this shouldn't be necessary as we can't override the option with mouse-hiding, but JIC
                        if (options.statsupdate)
                            cur_widget[nID].show = TRUE;
                        break;
                    default:
                        cur_widget[nID].show = TRUE;
                }
            }
        }
        else if (!stricmp(params, "widgets"))
        {
            draw_info("Resetting widgets!", COLOR_WHITE);
            init_widgets_fromDefault();
        }
//        else
//        {
//            draw_info("Usage: ~/reset buddy~ to reset the buddylist,\n~/reset ignore~ to reset the ignorelist,\n~/reset chatfilter~ to reset the chatfilter list,\n~/reset kills~ to reset the kill list,\n~/reset stats~ to reset the stat-o-meter,\n~/reset widgets~ to reset the widgets, or\n~/reset widgetstatus~ to show any previously hidden widgets.", COLOR_WHITE);
//        }

        return 1;
    }
#ifdef DEVELOPMENT
    else if (!strcmp(cmd, "/searchpath"))
    {
        char **i,
             **j;

        for (i = j = PHYSFS_getSearchPath(); *i; i++)
        {
            draw_info_format(COLOR_WHITE, "[%s] is in the search path.", *i);
        }

        PHYSFS_freeList(j);

        return 1;
    }
#endif
    else if (!strcmp(cmd, "/setwin"))
    {
        int msg,
            chat;

        if (!*params ||
            sscanf(params, "%d %d", &msg, &chat) != 2 ||
            msg < 2 ||
            msg + chat > 38)
        {
            draw_info("Parameters out of bounds.", COLOR_WHITE);
//            draw_info("Usage: '/setwin <Msg> <Chat>'\nExample:\n/setwin 9 5", COLOR_WHITE);
        }

        draw_info_format(COLOR_WHITE, "Set textwin to %d rows.", msg);
        options.use_TextwinSplit = 0;
        txtwin[TW_MIX].size = msg - 1;

        return 1;
    }
    else if (!strcmp(cmd, "/setwinalpha"))
    {
        if (!strnicmp(params, "on", 2))
        {
            int alpha;

            if (sscanf(params, "%*s %d", &alpha) == 1)
            {
                options.textwin_alpha = alpha;
            }

            options.use_TextwinAlpha = 1;
            draw_info_format(COLOR_WHITE, "Set textwin alpha ~on~ (alpha=%d).",
                             options.textwin_alpha);
        }
        else if (!strnicmp(params, "off", 3))
        {
            options.use_TextwinAlpha = 0;
            draw_info("Set textwin alpha mode ~off~.", COLOR_WHITE);
        }
//        else
//        {
//            draw_info("Usage: '/setwinalpha on|off [<alpha>]'\nExample:\n/setwinalpha ON 172\n/setwinalpha OFF",
//                      COLOR_WHITE);
//        }

        return 1;
    }
    else if (!strcmp(cmd, "/shout_off"))
    {
        options.shoutoff = 1;
        draw_info_format(COLOR_WHITE, "Shout disabled");

        return 1;
    }
    else if (!strcmp(cmd, "/shout_on"))
    {
        options.shoutoff = 0;
        draw_info_format(COLOR_WHITE,"Shout enabled");

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
//                draw_info_format(COLOR_WHITE, "Sleeptimer OFF\nUsage: /sleeptimer HH:MM");
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
           draw_info_format(COLOR_WHITE, "Sleeptime set to %s", tmpbuf);
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
#ifdef DEVELOPMENT
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
        map_redraw_flag = 1;

        return 1;
    }
#endif

    return 0;
}

/* send_game_command() will send a higher level game command like /tell, /say or
 * other "slash" text commants. Usually, this kind of commands are typed in 
 * console or are bound to macros. 
 * The underlaying protocol command is CLIENT_CMD_GENERIC, which means
 * its a command holding another command.
 * For realtime or system commands, commands with binary params and such,
 * not a slash command should be used but a new protocol command.
 * Only that commands hold real binary params and can be pre-processed
 * by the server protocol functions.
 */
void send_game_command(const char *command)
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

#ifdef USE_CHANNELS
        if (*token != '/' &&
            *token != '-') /* if not a command ... its chat  (- is for channel system)*/
#else
        if (*token != '/')
#endif
        {
            char tmpbuf[LARGE_BUF];

            sprintf(tmpbuf, "/say %s", token);
            sprintf(token, "%s", tmpbuf);
        }

        end = BreakMulticommand(token);

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

        if (!CommandCheck(cmd, params))
        {
            char     tmpbuf[LARGE_BUF];
            SockList sl;

            sprintf(tmpbuf, "%s%s%s", cmd + 1, (*params) ? " " : "", params);
            SockList_INIT(&sl, NULL);
            SockList_COMMAND(&sl, CLIENT_CMD_GENERIC, SEND_CMD_FLAG_STRING);
            SockList_AddBuffer(&sl, tmpbuf, strlen(tmpbuf));
            send_socklist_binary(&sl);
        }
    }
}

/* help function for receiving faces (pictures)
* NOTE: This feature must be enabled from the server
*/
static void face_flag_extension(int pnum, char *buf)
{
    char   *stemp;

    FaceList[pnum].flags = FACE_FLAG_NO;

    /* Check for the "alt a"/"alt b"/"double"/"up" tag in the picture name. */
    if ((stemp = strstr(buf, ".a")))
    {
        char fname[LARGE_BUF];
        int  i;

        FaceList[pnum].flags |= FACE_FLAG_ALTERNATIVE;

        /* Strip off any path information and the file extension, and replace
         * .a with .b. */
        if ((stemp = strrchr(buf, '/')))
            stemp++;
        else
            stemp = buf;
        strcpy(fname, stemp);
        *(fname + (strlen(fname) - 4)) = '\0';
        *(strstr(fname, ".a") + 1) = 'b';

        /* Look for fname in the bmap table, requesting it if necessary. */
        if ((i = get_facenum_from_name(fname)) == -1)
            LOG(LOG_MSG, "%s/face_flag_extension(): %s could not be found in bmaptype_table!\n",
                __FILE__, fname);
        else
        {
            request_face(i);

            /* Set our_ref. */
            FaceList[pnum].alt_a = -1;
            FaceList[pnum].alt_b = i;

            /* Set your_ref. */
            FaceList[i].alt_a = pnum;
            FaceList[i].alt_b = -1;
        }
    }
    else if ((stemp = strstr(buf, ".b")))
    {
        char fname[LARGE_BUF];
        int  i;

        FaceList[pnum].flags |= FACE_FLAG_ALTERNATIVE;

        /* Strip off any path information and the file extension, and replace
         * .b with .a. */
        if ((stemp = strrchr(buf, '/')))
            stemp++;
        else
            stemp = buf;
        strcpy(fname, stemp);
        *(fname + (strlen(fname) - 4)) = '\0';
        *(strstr(fname, ".b") + 1) = 'a';

        /* Look for fname in the bmap table, requesting it if necessary. */
        if ((i = get_facenum_from_name(fname)) == -1)
            LOG(LOG_MSG, "%s/face_flag_extension(): %s could not be found in bmaptype_table!\n",
                __FILE__, fname);
        else
        {
            request_face(i);

            /* Set our_ref. */
            FaceList[pnum].alt_a = i;
            FaceList[pnum].alt_b = -1;

            /* Set your_ref. */
            FaceList[i].alt_a = -1;
            FaceList[i].alt_b = pnum;
        }
    }
    else if ((stemp = strstr(buf, ".d")))
        FaceList[pnum].flags |= FACE_FLAG_DOUBLE;
    else if ((stemp = strstr(buf, ".u")))
    {
        int tc;

        FaceList[pnum].flags |= FACE_FLAG_UP;

        for (tc = 0; tc < 4; tc++)
        {
            if (!*(stemp + tc)) /* has the string a '0' before our anim tags */
                goto finish_face_cmd_j1;
        }

        switch (*(stemp + tc))
        {
            case '0':
            case '2':
            case '4':
            case '6':
            case '8':
                FaceList[pnum].flags |= (FACE_FLAG_D3 | FACE_FLAG_D1);
                break;

            case '1':
            case '5':
                FaceList[pnum].flags |= FACE_FLAG_D1;
                break;

            case '3':
            case '7':
                FaceList[pnum].flags |= FACE_FLAG_D3;
                break;
        }
    }

finish_face_cmd_j1: /* error jump from for() */
    return;
}


void finish_face_cmd(int pnum, uint32 checksum, char *face)
{
    SockList        sl;
    char            buf[2048];
    FILE           *stream;
    struct stat     statbuf;
    int             len;
    static uint32   newsum  = 0;
    unsigned char * data;
    void   *tmp_free;

    /* first, check our memory... perhaps we have it loaded */
    /*LOG(LOG_MSG,"FACE: %s (->%s)\n", face,FaceList[pnum].name);*/
    if (FaceList[pnum].name) /* loaded OR requested...hm, no double request check yet*/
    {
        /* lets check the name and checksum and sprite. ONLY if all is
         * ok, we stay with it
         */
        if (!strcmp(face, FaceList[pnum].name) && checksum == FaceList[pnum].checksum && FaceList[pnum].sprite)
        {
            face_flag_extension(pnum, FaceList[pnum].name);
            return;
        }
        /* ok, some is different.
         * no big work, clear face data and lets go on
         *  all this check for invalid ptr, so fire it up
        */
        tmp_free = &FaceList[pnum].name;
        FreeMemory(tmp_free);
        sprite_free_sprite(FaceList[pnum].sprite);
    }

    /* first, safe face data: name & checksum */
    sprintf(buf, "%s.png", face);
    FaceList[pnum].name = (char *) _malloc(strlen(buf) + 1, "finish_face_cmd(): FaceList name");
    strcpy(FaceList[pnum].name, buf);

    FaceList[pnum].checksum = checksum;

    /* Check private cache first */
    sprintf(buf, "%s%s", GetCacheDirectory(), FaceList[pnum].name);
    if ((stream = fopen_wrapper(buf, "rb")) != NULL)
    {
        fstat(fileno(stream), &statbuf);
        len = (int) statbuf.st_size;
        data = malloc(len);
        len = fread(data, 1, len, stream);
        fclose(stream);
        newsum = 0;

        /* something is wrong... now unlink the file and let it reload then possible and needed */
        if (len <= 0) 
            checksum = 1; /* mark as wrong */
        else /* lets go for the checksum check*/
            newsum = crc32(1L, data, len);

        free(data);

        if (newsum == checksum)
        {
            FaceList[pnum].sprite = sprite_tryload_file(buf, 0, NULL);
            if (FaceList[pnum].sprite)
            {
                face_flag_extension(pnum, buf);
                return; /* found and loaded!*/
            }
        }
        unlink(buf); /* forget this face - unlink it and request a new one! */
    }

    LOG(LOG_MSG,"FACE: call server for %s (%d)\n", face, pnum);
    face_flag_extension(pnum, buf);

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_FACE, 0);
    SockList_AddShort(&sl, pnum);
    send_socklist_binary(&sl);

}


/* we have stored this picture in daimonin.p0 - load it from it! */
static int load_picture_from_pack(int num)
{
    FILE       *stream;
    char       *pbuf;
    size_t      dummy; // purely to avoid GCC's warn_unused_result warning
    SDL_RWops  *rwop;

    if ((stream = fopen_wrapper(FILE_DAIMONIN_P0, "rb")) == NULL)
        return 1;

    lseek(fileno(stream), bmaptype_table[num].pos, SEEK_SET);

    pbuf = malloc(bmaptype_table[num].len);
    dummy = fread(pbuf, bmaptype_table[num].len, 1, stream);
    rwop = SDL_RWFromMem(pbuf, bmaptype_table[num].len);
    FaceList[num].sprite = sprite_tryload_file(NULL, 0, rwop);

    if (FaceList[num].sprite)
        face_flag_extension(num, FaceList[num].name);

    fclose(stream);
    free(pbuf);

    return 0;
}
/* we got a face - test we have it loaded.
 * if not, say server "send us face cmd "
 * Return: 0 - face not there, requested.
 * 1: face requested or loaded
 */
int request_face(int pnum)
{
    char        buf[256 * 2];
    FILE       *stream;
    struct stat statbuf;
    int         len;
    unsigned char * data;
    uint16      num     = (uint16) (pnum&~0x8000);

    if (FaceList[num].name || FaceList[num].flags & FACE_REQUESTED) /* loaded OR requested.. */
        return 1;

    if (num >= bmaptype_table_size)
    {
        LOG(LOG_ERROR, "REQUEST_FILE(): server sent picture id to big (%d %d)\n", num, bmaptype_table_size);
        return 0;
    }

    /* now lets check BEFORE we do any other test for this name in /gfx_user.
     * Perhaps we have a customized picture here.
     */
    sprintf(buf, "%s%s.png", GetGfxUserDirectory(), bmaptype_table[num].name);
    if ((stream = fopen_wrapper(buf, "rb")) != NULL)
    {
        /* yes we have a picture with this name in /gfx_user!
             * lets try to load.
             */
        fstat(fileno(stream), &statbuf);
        len = (int) statbuf.st_size;
        data = malloc(len);
        len = fread(data, 1, len, stream);
        fclose(stream);
        if (len > 0)
        {
            /* lets try to load first... */
            FaceList[num].sprite = sprite_tryload_file(buf, 0, NULL);
            if (FaceList[num].sprite) /* NOW we have a valid png with right name ...*/
            {
                face_flag_extension(num, buf);
                sprintf(buf, "%s%s.png", GetGfxUserDirectory(), bmaptype_table[num].name);
                FaceList[num].name = (char *) malloc(strlen(buf) + 1);
                strcpy(FaceList[num].name, buf);
                FaceList[num].checksum = crc32(1L, data, len);
                free(data);
                return 1;
            }
        }
        /* if we are here something was wrong with the gfx_user file.*/
        free(data);
    }

    /* ok - at this point we hook in our client stored png lib.
    */

    if (bmaptype_table[num].pos != -1) /* best case - we have it in daimonin.p0! */
    {
        sprintf(buf, "%s.png", bmaptype_table[num].name);
        FaceList[num].name = (char *) _malloc(strlen(buf) + 1, "request_face(): FaceList name");
        strcpy(FaceList[num].name, buf);
        FaceList[num].checksum = bmaptype_table[num].crc;
        load_picture_from_pack(num);
    }
    else /* check client /cache folder. If its not there, finish_face_cmd() will stop or ask the server */
    {
        FaceList[num].flags |= FACE_REQUESTED;
        finish_face_cmd(num, bmaptype_table[num].crc, bmaptype_table[num].name);
    }

    return 1;
}

/* send the setup command to the server
 * This is the handshake command after the client connects
 * and the first data which are send between server & client
 * NOTE: Because this is the first command, the data part is 
 * String only. With the response from the server we get 
 * endian info which enables us to send binary data (without
 * fixed shifting)
 */
void SendSetupCmd(void)
{
    char tmpbuf[TINY_BUF],
         buf[MEDIUM_BUF];

    if (SoundStatus)
        sprintf(tmpbuf, "%d|%x",
                srv_client_files[SRV_CLIENT_SOUNDS].len,
                srv_client_files[SRV_CLIENT_SOUNDS].crc);
    else
        strcpy(tmpbuf, "0");

    sprintf(buf, "pv %u sn %s mz %dx%d skf %d|%x spf %d|%x bpf %d|%x stf %d|%x amf %d|%x",
    PROTOCOL_VERSION, tmpbuf, MapStatusX, MapStatusY, srv_client_files[SRV_CLIENT_SKILLS].len,
    srv_client_files[SRV_CLIENT_SKILLS].crc, srv_client_files[SRV_CLIENT_SPELLS].len,
    srv_client_files[SRV_CLIENT_SPELLS].crc, srv_client_files[SRV_CLIENT_BMAPS].len,
    srv_client_files[SRV_CLIENT_BMAPS].crc, srv_client_files[SRV_CLIENT_SETTINGS].len,
    srv_client_files[SRV_CLIENT_SETTINGS].crc, srv_client_files[SRV_CLIENT_ANIMS].len,
    srv_client_files[SRV_CLIENT_ANIMS].crc);

    send_command_binary(CLIENT_CMD_SETUP, buf, strlen(buf), SEND_CMD_FLAG_STRING);
}

/* Request a so called "server file" from the server.
 * Which includes a list of skills the server knows,
 * spells and such, and how they are described and 
 * visualized
 */
void RequestFile(ClientSocket csock, int index)
{
    SockList    sl;

    /* for binary stuff we better use the socklist system */
    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_REQUESTFILE, SEND_CMD_FLAG_FIXED);
    SockList_AddChar(&sl, index);
    send_socklist_binary(&sl);
}

/* ONLY send this when we are valid connected to our account.
 * Server will assume a hacking attempt when something is wrong
 * we are not logged to an account or name don't exists in that
 * account. Will invoke a hack warning and a temp ban!
 * This command will invoke the login for char name and put player
 * in playing mode or invoke an account cmd error msg
 */
void SendAddMe(char *name)
{
    SockList    sl;
    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_ADDME, SEND_CMD_FLAG_DYNAMIC);
    SockList_AddString(&sl, name);
    send_socklist_binary(&sl);
}

/* the server also parsed client_settings. 
 * We only tell him our name, the selected default arch (as gender_selected)
 * and the weapon skill
 * The server will grap the other values from the loaded file
 */
void send_new_char(_server_char *nc)
{
    int i =0;
    SockList    sl;
    _server_char   *tmpc;


    /* lets find the entry number */
    for (tmpc = first_server_char; tmpc; tmpc = tmpc->next)
    {
        /* get our current template */
        if (!strcmp(tmpc->name, new_character.name))
            break;
        i++;
    }

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_NEWCHAR, SEND_CMD_FLAG_DYNAMIC);
    SockList_AddChar(&sl, i);
    SockList_AddChar(&sl, nc->gender_selected);
    SockList_AddChar(&sl, nc->skill_selected);
    SockList_AddString(&sl, cpl.name);
    send_socklist_binary(&sl);
}

/* delete a character */
void send_del_char(char *name)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_DELCHAR, SEND_CMD_FLAG_DYNAMIC);
    SockList_AddString(&sl, name);
    send_socklist_binary(&sl);
}


void client_send_apply(int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_APPLY, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

/* THE main move command function */
void send_move_command(int dir, int mode)
{
    SockList    sl;
    // remapped to: "idle", "/sw", "/s", "/se", "/w", "/stay", "/e", "/nw", "/n", "/ne" 

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_MOVE, SEND_CMD_FLAG_FIXED);
    SockList_AddChar(&sl, move_dir[dir]);
    SockList_AddChar(&sl, mode);
    send_socklist_binary(&sl);
}

void client_send_examine(int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_EXAMINE, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

/* Requests nrof objects of tag get moved to loc. */
void send_inv_move(int loc, int tag, int nrof)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_INVMOVE, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, loc);
    SockList_AddInt(&sl, tag);
    SockList_AddInt(&sl, nrof);
    send_socklist_binary(&sl);
}

void send_talk_command(sint8 mode, char *topic)
{
    uint16   c;
    SockList sl;

    for (c = 0; *(topic + c); c++)
    {
        *(topic + c) = tolower(*(topic + c));
    }

    draw_info_format(COLOR_DGOLD, "Topic: %s", topic);
    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_GUITALK, SEND_CMD_FLAG_DYNAMIC);
    SockList_AddChar(&sl, mode);
    SockList_AddBuffer(&sl, topic, strlen(topic));
    send_socklist_binary(&sl);
}

void send_lock_command(int mode, int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_LOCK, SEND_CMD_FLAG_FIXED);
    SockList_AddChar(&sl, mode);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

void send_mark_command(int tag)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_MARK, SEND_CMD_FLAG_FIXED);
    SockList_AddInt(&sl, tag);
    send_socklist_binary(&sl);
}

void send_fire_command(int num, int mode, char *tmp_name)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_FIRE, 0);
    SockList_AddInt(&sl, move_dir[num]);
    SockList_AddInt(&sl, mode);
    if(tmp_name)
        SockList_AddBuffer(&sl, tmp_name, strlen(tmp_name));
    SockList_AddChar(&sl, 0); /* be sure we finish with zero - server will check it */
    send_socklist_binary(&sl);

}

void client_send_checkname(char *buf)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_CHECKNAME, 0);
    SockList_AddString(&sl, buf);
    send_socklist_binary(&sl);
}

void client_send_login(int mode, char *name, char *pass)
{
    SockList    sl;

    SockList_INIT(&sl, NULL);
    SockList_COMMAND(&sl, CLIENT_CMD_LOGIN, 0);
    SockList_AddChar(&sl, mode);
    SockList_AddString(&sl, name);
    SockList_AddString(&sl, pass);
    send_socklist_binary(&sl);
}

