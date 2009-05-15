/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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
#include <include.h>

_media_file             media_file[MEDIA_MAX];

/* keybind menu */
int                     keybind_status;

struct _spell_list      spell_list[SPELL_LIST_MAX]; /* skill list entries */
struct _skill_list      skill_list[SKILL_LIST_MAX]; /* skill list entries */

struct _dialog_list_set spell_list_set;
struct _dialog_list_set skill_list_set;
struct _dialog_list_set option_list_set;
struct _dialog_list_set bindkey_list_set;
struct _dialog_list_set create_list_set;

int                     media_count;        /* buffered media files*/
int                     media_show; /* show this media file*/
int                     media_show_update;
int                     keybind_startoff    = 0;
static char            *get_range_item_name(int id);

_quickslot              quick_slots[MAX_QUICK_SLOTS];
int                     quickslots_pos[MAX_QUICK_SLOTS][2]  =
    {
        {17,1}, {50,1}, {83,1}, {116,1}, {149,1}, {182,1}, {215,1}, {248,1}
    };

void do_console(int x, int y)
{
    show_help_screen = 0;
    if (InputStringEscFlag == TRUE)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CONSOLE, 0, 0, 100);
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        cur_widget[IN_CONSOLE_ID].show = FALSE;
        map_udate_flag = 2;
    }
    /* if set, we got a finished input!*/
    if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CONSOLE, 0, 0, 100);
        if (InputString[0])
        {
            /*
                    sprintf(buf,":%s",InputString);
                    draw_info(buf,COLOR_DGOLD);*/
            if (client_command_check(InputString))
                goto no_send_cmd;
            send_game_command(InputString);
        }
no_send_cmd:
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        cur_widget[IN_CONSOLE_ID].show = FALSE;
    }
    else
        cur_widget[IN_CONSOLE_ID].show = TRUE;
}

/* client_command_check()
 * Analyze /<cmd> type commands the player has typed in the console
 * or bound to a key. Sort out the "client intern" commands and
 * expand or pre process them for the server.
 * Return: TRUE=don't send command to server
 * FALSE: send command to server
 */
int client_command_check(char *cmd)
{
    char    tmp[256];
    char    cpar1[256];
    int     par1, par2;

    if (!strnicmp(cmd, "/ready_spell", strlen("/ready_spell")))
    {
        cmd = strchr(cmd, ' ');
        if (!cmd || *++cmd == 0)
        {
            draw_info("usage: /ready_spell <spell name>", COLOR_GREEN);
        }
        else
        {
            int i, ii;

            for (i = 0; i < SPELL_LIST_MAX; i++)
            {
                for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
                {
                    if (spell_list[i].entry[0][ii].flag >= LIST_ENTRY_USED)
                    {
                        if (!strcmp(spell_list[i].entry[0][ii].name, cmd))
                        {
                            if (spell_list[i].entry[0][ii].flag == LIST_ENTRY_KNOWN)
                            {
                                fire_mode_tab[FIRE_MODE_SPELL].spell = &spell_list[i].entry[0][ii];
                                RangeFireMode = FIRE_MODE_SPELL;
                                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                                draw_info("spell ready.", COLOR_GREEN);
                                return TRUE;
                            }
                        }
                    }
                    if (spell_list[i].entry[1][ii].flag >= LIST_ENTRY_USED)
                    {
                        if (!strcmp(spell_list[i].entry[1][ii].name, cmd))
                        {
                            if (spell_list[i].entry[1][ii].flag == LIST_ENTRY_KNOWN)
                            {
                                fire_mode_tab[FIRE_MODE_SPELL].spell = &spell_list[i].entry[1][ii];
                                RangeFireMode = FIRE_MODE_SPELL;
                                sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, MENU_SOUND_VOL);
                                draw_info("spell ready.", COLOR_GREEN);
                                return TRUE;
                            }
                        }
                    }
                }
            }
        }
        draw_info("unknown spell.", COLOR_GREEN);
        return TRUE;
    }
#ifdef USE_CHANNELS
    /* i know hardcoding is most of the time bad, but the channel system will be uses
     * really often. Also we can type directly the '-' in the textwin.
     */
    else if (!strnicmp(cmd, "-", strlen("-")))
    {
        char channelbuf[1024];
        sprintf(channelbuf,"/channel %s",cmd+1);
        send_game_command(channelbuf);
        return TRUE;
    }
#endif
	/* lets try out some things to load some work from the server to the client */
	else if (!strnicmp(cmd, "/apply", strlen("/apply")))
	{
		return apply_command(cmd+6);
	}
	else if (!strnicmp(cmd, "/markdmbuster", strlen("makrdmbuster")))
	{
        markdmbuster();
	    return TRUE;
	}
	else if (!strnicmp(cmd, "/shout_off", strlen("/shout_off")))
	{
	    options.shoutoff=TRUE;
        draw_info_format(COLOR_RED,"Shout disabled");
        return TRUE;
	}
	else if (!strnicmp(cmd, "/shout_on", strlen("/shout_on")))
	{
	    options.shoutoff=FALSE;
        draw_info_format(COLOR_GREEN,"Shout enabled");
        return TRUE;
	}
	else if (!strnicmp(cmd, "/buddy", strlen("/buddy")))
	{
		buddy_command(cmd+6);
		return TRUE;
	}
    else if (!strnicmp(cmd, "/ignore", strlen("/ignore")))
    {
        ignore_command(cmd+7);
        return TRUE;
    }
    else if (!strnicmp(cmd, "/cfilter", strlen("/cfilter")))
    {
        chatfilter_command(cmd+8);
        return TRUE;
    }
    else if (!strnicmp(cmd, "/kills", strlen("/kills")))
    {
         kill_command(cmd+6);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f1", strlen("/f1")))
    {
         quickslot_key(NULL,0);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f2", strlen("/f2")))
    {
         quickslot_key(NULL,1);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f3", strlen("/f3")))
    {
         quickslot_key(NULL,2);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f4", strlen("/f4")))
    {
         quickslot_key(NULL,3);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f5", strlen("/f5")))
    {
         quickslot_key(NULL,4);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f6", strlen("/f6")))
    {
         quickslot_key(NULL,5);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f7", strlen("/f7")))
    {
         quickslot_key(NULL,6);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/f8", strlen("/f8")))
    {
         quickslot_key(NULL,7);
         return TRUE;
    }
    else if (!strnicmp(cmd, "/imagestats", strlen("/imagestats")))
    {
        draw_info_format(COLOR_WHITE,"IMAGE-LOADING-STATISTICS\n==========================================");
        draw_info_format(COLOR_WHITE,"Sprites in Memory: %d",ImageStats.loadedsprites);
        draw_info_format(COLOR_WHITE,"TrueColors: %d",ImageStats.truecolors);
        draw_info_format(COLOR_WHITE,"Greyscales in Memory: %d",ImageStats.greyscales);
        draw_info_format(COLOR_WHITE,"Redscales in Memory: %d",ImageStats.redscales);
        draw_info_format(COLOR_WHITE,"Fowscales in Memory: %d",ImageStats.fowscales);
        return TRUE;
    }
#ifdef DEVELOPMENT
    else if (!strnicmp(cmd, "/searchpath", strlen("/searchpath")))
    {
        char **i, **j;

        for (j = i = PHYSFS_getSearchPath(); *i != NULL; i++)
        {
            draw_info_format(COLOR_WHITE,"[%s] is in the search path.", *i);
        }

        PHYSFS_freeList(j);
        return TRUE;
    }
    else if (!strnicmp(cmd, "/teststretch", strlen("/teststretch")))
    {
        int h[5][5] =
        {   {1,1,1,1,1},
            {1,3,5,3,1},
            {1,5,10,5,1},
            {1,3,5,3,1},
            {1,1,1,1,1}  };
        int i, j;
        for (i=6;i<11;i++)
        {
            for (j=6;j<11;j++)
            {
                set_map_height(i, j, h[i-6][j-6]);
            }
        }
        map_udate_flag = TRUE;
        map_redraw_flag = TRUE;
        return TRUE;
    }
#endif
    else if (!strnicmp(cmd, "/changeskin", strlen("/changeskin")))
    {
        cmd = strchr(cmd, ' ');
        if (!cmd || *++cmd == 0)
            draw_info("usage: /changeskin <skin>", COLOR_GREEN);
        else
        {
            if (stricmp(options.skin,cmd)==0)
            {
                draw_info_format(COLOR_WHITE, "You are already using skin %s.",options.skin);
            }
            else
            {
                char buf[512];
                Boolean newskin = FALSE;

                sprintf(buf,"skins/%s.zip",cmd);
                if (PHYSFS_exists(buf))
                {
                    if (PHYSFS_addToSearchPath(buf , 0)==0)
                        LOG(LOG_MSG,"PHYSFS_addPath (%s) failed: %s\n",buf,PHYSFS_getLastError());
                    else
                        newskin=TRUE;
                }
                sprintf(buf,"skins/%s",cmd);
                if (PHYSFS_isDirectory(buf))
                {
                    if (PHYSFS_addToSearchPath(buf , 0)==0)
                        LOG(LOG_MSG,"PHYSFS_addPath (%s) failed: %s\n",buf,PHYSFS_getLastError());
                    else
                        newskin=TRUE;
                }
                if (newskin)
                {
                    if (strnicmp(options.skin, "subred", strlen("subred")))
                    {
                        sprintf(buf,"skins/%s.zip",options.skin);
                        if (PHYSFS_removeFromSearchPath(buf)==0)
                            LOG(LOG_MSG,"PHYSFS_removePath (%s) failed: %s\n",buf,PHYSFS_getLastError());
                        sprintf(buf,"skins/%s",options.skin);
                        if (PHYSFS_removeFromSearchPath(buf)==0)
                            LOG(LOG_MSG,"PHYSFS_removePath (%s) failed: %s\n",buf,PHYSFS_getLastError());
                    }
                    strncpy(options.skin, cmd, 63);
                    save_options_dat();
                    reload_skin();
                    reload_icons();
                }
                else
                {
                    draw_info_format(COLOR_RED,"Skin '%s' not found, using old one.",cmd);
                }
            }
        }
        return TRUE;
    }
    else if (!strnicmp(cmd, "/reloadskinnow", strlen("/reloadskinnow")))
    {
        draw_info_format(COLOR_GREEN,"Reloading skin. This function is only for skin creating, and may be removed anytime!");
        reload_skin();
        reload_icons();
        return TRUE;
    }
    else if (!strnicmp(cmd, "/reply", strlen("/reply")))
    {
        cmd = strchr(cmd, ' ');
        if (!cmd || *++cmd == 0)
            draw_info("usage: /reply <message>", COLOR_GREEN);
        else
        {

            if (!cpl.player_reply[0])
            {
                draw_info("There is no one you can /reply.", COLOR_GREEN);
            }
            else
            {
                char buf[2048];

                sprintf(buf,"/tell %s %s", cpl.player_reply, cmd);
                LOG(-1,"REPLY: %s\n", buf);
                send_game_command(buf);
            }
        }
        return TRUE;
    }
    else if (!strnicmp(cmd, "/statreset",strlen("/statreset")))
    {
        statometer.exp=0;
        statometer.kills=0;
        statometer.starttime=LastTick-1;
        statometer.lastupdate=LastTick;
        statometer.exphour=0.0f;
        statometer.killhour=0.0f;

        return TRUE;
    }
    else if (!strnicmp(cmd, "/sleeptimer", strlen("/sleeptimer")))
    {
        char    tmp[30];
        int   stpar1 = -1, stpar2 = -1;
        sscanf(cmd, "%s %d:%d",tmp,&stpar1,&stpar2);

        if ((stpar1==-1) || (stpar2==-1))
        {
                draw_info_format(COLOR_WHITE,"Sleeptimer OFF\nUsage: /sleeptimer HH:MM");
                options.sleepcounter = FALSE;
        }
        else
        {
            struct tm *temp;
            char buft[512];

            options.sleepcounter = TRUE;
            time(&sleeptime);
            temp = localtime(&sleeptime);
            if (stpar1<temp->tm_hour)
                temp->tm_mday+=1;
            temp->tm_hour=stpar1;
            temp->tm_min=stpar2;
            sleeptime=mktime(temp);
            strftime(buft, sizeof buft, "%d-%m-%y %H:%M:%S", localtime(&sleeptime));
            draw_info_format(COLOR_WHITE,"Sleeptime set to %s", buft);
        }
        return TRUE;
    }
    else if (!strnicmp(cmd, "/setwinalpha", strlen("/setwinalpha")))
    {
        int wrong   = 0;
        par2 = -1;
        sscanf(cmd, "%s %s %d", tmp, cpar1, &par2);

        if (!strnicmp(cpar1, "ON", strlen("ON")))
        {
            if (par2 != -1)
            {
                if (par2 <0 || par2>255)
                    par2 = 128;
                options.textwin_alpha = par2;
            }
            options.use_TextwinAlpha = 1;
            sprintf(tmp, ">>set textwin alpha ON (alpha=%d).", options.textwin_alpha);
            draw_info(tmp, COLOR_GREEN);
        }
        else if (!strnicmp(cpar1, "OFF", strlen("OFF")))
        {
            draw_info(">>set textwin alpha mode OFF.", COLOR_GREEN);
            options.use_TextwinAlpha = 0;
        }
        else
            wrong = 1;


        if (wrong)
            draw_info("Usage: '/setwinalpha on|off |<alpha>|'\nExample:\n/setwinalpha ON 172\n/setwinalpha OFF",
                      COLOR_WHITE);
        return TRUE;
    }
    else if (!strnicmp(cmd, "/setwin", strlen("/setwin")))
    {
        int wrong   = 0;
        par1 = 9, par2 = -1;
        sscanf(cmd, "%s %d %d", tmp, &par1, &par2);

        if (par1<2 || par1>38)
        {
            wrong = 1;
            draw_info("/setwin: parameters out of bounds.", COLOR_RED);
        }
        else
        {
            sprintf(tmp, ">>set textwin to %d rows.", par1);
            draw_info(tmp, COLOR_GREEN);
            options.use_TextwinSplit = 0;
            txtwin[TW_MIX].size = par1 - 1;
        }

        if (wrong)
            draw_info("Usage: '/setwin <Msg> <Chat>'\nExample:\n/setwin 9 5", COLOR_WHITE);
        return TRUE;
    }
    else if (!strnicmp(cmd, "/keybind", strlen("/keybind")))
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
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
        reset_keys();
        return TRUE;
    }
    else if (!strncmp(cmd, "/target", strlen("/target")))
    {
        /* logic is: if first parameter char is a digit, is enemy,friend or self.
             * if first char a character - then its a name of a living object.
             */
        if (!strncmp(cmd, "/target friend", strlen("/target friend")))
            strcpy(cmd, "/target 1");
        else if (!strncmp(cmd, "/target enemy", strlen("/target enemy")))
            strcpy(cmd, "/target 0");
        else if (!strncmp(cmd, "/target self", strlen("/target self")))
            strcpy(cmd, "/target 2");
    }
#ifdef DEVELOPMENT
    else if (!strnicmp(cmd, "/reset", strlen("/reset")))
    {
        if (!strnicmp(cmd, "/reset buddy", strlen("/reset buddy")))
        {
            draw_info("Resetting buddy list!", COLOR_HGOLD);
            buddy_list_clear();
            buddy_list_save();
            return TRUE;
        }
        else if (!strnicmp(cmd, "/reset ignore", strlen("/reset ignore")))
        {
            draw_info("Resetting ignore list!", COLOR_HGOLD);
            ignore_list_clear();
            ignore_list_save();
            return TRUE;
        }
        else if (!strnicmp(cmd, "/reset chatfilter", strlen("/reset chatfilter")) || !strnicmp(cmd, "/reset cfilter", strlen("/reset cfilter")))
        {
            draw_info("Resetting chatfilter list!", COLOR_HGOLD);
            chatfilter_list_clear();
            chatfilter_list_save();
            return TRUE;
        }
        else if (!strnicmp(cmd, "/reset kills", strlen("/reset kills")))
        {
            draw_info("Resetting kill list!", COLOR_HGOLD);
            kill_list_clear();
            kill_list_save();
            return TRUE;
        }
        else if (!strnicmp(cmd, "/reset stats", strlen("/reset stats")))
        {
            draw_info("Resetting stat-o-meter!", COLOR_HGOLD);
            statometer.exp        = 0;
            statometer.kills      = 0;
            statometer.starttime  = LastTick - 1;
            statometer.lastupdate = LastTick;
            statometer.exphour    = 0.0f;
            statometer.killhour   = 0.0f;
            return TRUE;
        }
        else if (!strnicmp(cmd, "/reset widgetstatus", strlen("/reset widgetstatus")))
        {
            int nID;
            draw_info("Resetting widgetstatus!", COLOR_HGOLD);
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
            return TRUE;
        }
        else if (!strnicmp(cmd, "/reset widgets", strlen("/reset widgets")))
        {
            draw_info("Resetting widgets!", COLOR_HGOLD);
            init_widgets_fromDefault();
            return TRUE;
        }
        else if (!strnicmp(cmd, "/resetmap", strlen("/resetmap")))
            return FALSE; /* DM command, handled by server */
        else
        {
            draw_info("Usage: ~/reset buddy~ to reset the buddylist,\n~/reset ignore~ to reset the ignorelist,\n~/reset chatfilter~ to reset the chatfilter list,\n~/reset kills~ to reset the kill list,\n~/reset stats~ to reset the stat-o-meter,\n~/reset widgets~ to reset the widgets, or\n~/reset widgetstatus~ to show any previously hidden widgets.", COLOR_WHITE);
            return TRUE;
        }
    }
#endif

    return FALSE;
}

void widget_show_console(int x, int y)
{
    sprite_blt(Bitmaps[BITMAP_TEXTINPUT],x, y, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, show_input_string(InputString, &SystemFont, 239), x+9, y+7, COLOR_WHITE, NULL,
              NULL);
}

void do_number(int x, int y)
{
    show_help_screen = 0;
    if (InputStringEscFlag == TRUE)
    {
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        cur_widget[IN_NUMBER_ID].show = FALSE;
        map_udate_flag = 2;
    }
    /* if set, we got a finished input!*/
    if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
    {
        if (InputString[0])
        {
            int     tmp;
            char    buf[300];
            tmp = atoi(InputString);
            if (tmp > 0 && tmp <= cpl.nrof)
            {
                send_inv_move(cpl.loc, cpl.tag, tmp);
                sprintf(buf, "%s %d from %d %s", cpl.nummode == NUM_MODE_GET ? "get" : "drop", tmp, cpl.nrof,
                        cpl.num_text);
                if (cpl.nummode == NUM_MODE_GET)
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
                else
                    sound_play_effect(SOUNDTYPE_NORMAL, SOUND_DROP, 0, 0, 100);

                draw_info(buf, COLOR_DGOLD);
            }
        }
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        cur_widget[IN_NUMBER_ID].show = FALSE;
    }
    else
        cur_widget[IN_NUMBER_ID].show = TRUE;

}

void do_keybind_input(void)
{
    show_help_screen = 0;
    if (InputStringEscFlag == TRUE)
    {
        reset_keys();
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
        cpl.input_mode = INPUT_MODE_NO;
        keybind_status = KEYBIND_STATUS_NO;
        map_udate_flag = 2;
    }
    /* if set, we got a finished input!*/
    if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
    {
        if (InputString[0])
        {
            strcpy(bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text, InputString);
            keybind_status = KEYBIND_STATUS_EDITKEY; /* now get the key code */
        }
        else
        {
            /* cleared string - delete entry */
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].text[0] = 0;
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].keyname[0] = 0;
            bindkey_list[bindkey_list_set.group_nr].entry[bindkey_list_set.entry_nr].key = 0;
            keybind_status = KEYBIND_STATUS_NO;
        }
        reset_keys();
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
    }
}

void do_npcdialog_input(void)
{
    show_help_screen = 0;
    if (InputStringEscFlag == TRUE)
    {
        reset_keys();
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
        gui_interface_npc->input_flag = FALSE;
    }

    /* if set, we got a finished input!*/
    if (InputStringFlag == FALSE && InputStringEndFlag == TRUE)
    {
        if (InputString[0])
        {
            gui_interface_send_command(2,InputString);
        }
        reset_keys();
        gui_interface_npc->input_flag = FALSE;
        cpl.input_mode = INPUT_MODE_NO;
        map_udate_flag = 2;
    }
}

void widget_number_event(int x, int y, SDL_Event event)
{
    int mx=0, my=0;
    mx = x - cur_widget[IN_NUMBER_ID].x1;
    my = y - cur_widget[IN_NUMBER_ID].y1;

    /* close number input */
    if (InputStringFlag && cpl.input_mode == INPUT_MODE_NUMBER)
    {
        if (mx > 239 && mx < 249 && my > 5 && my < 17)
        {
            SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
            InputStringFlag = FALSE;
            InputStringEndFlag = TRUE;
        }
    }
}

void widget_show_number(int x, int y)
{
    SDL_Rect    tmp;
    char        buf[512];

    tmp.w = 238;

    sprite_blt(Bitmaps[BITMAP_NUMBER], x, y, NULL, NULL);
    sprintf(buf, "%s how many from %d %s", cpl.nummode == NUM_MODE_GET ? "get" : "drop", cpl.nrof, cpl.num_text);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 8, y + 6, COLOR_HGOLD, &tmp, NULL);
    StringBlt(ScreenSurface, &SystemFont,
              show_input_string(InputString, &SystemFont, Bitmaps[BITMAP_NUMBER]->bitmap->w - 22), x + 8, y + 25,
              COLOR_WHITE, &tmp, NULL);
}

static inline void print_resist(char *name, int x, int y, int num)
{
    char    buf[16];

    StringBlt(widgetSF[RESIST_ID], &SystemFont, name, x+5, y, (num>ATNR_GODPOWER)?COLOR_DGOLD:COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.protection[num]);
    StringBlt(widgetSF[RESIST_ID], &SystemFont, buf, x + 17, y, cpl.stats.protection[num] ? (cpl.stats.protection[num]<0?COLOR_RED:(cpl.stats.protection[num]>=100?COLOR_ORANGE:COLOR_WHITE)) : COLOR_GREY,NULL, NULL);
}

void widget_show_resist(int x, int y)
{
    _BLTFX bltfx;
    SDL_Rect box;

    if (!widgetSF[RESIST_ID])
        widgetSF[RESIST_ID]=SDL_ConvertSurface(Bitmaps[BITMAP_RESIST_BG]->bitmap,Bitmaps[BITMAP_RESIST_BG]->bitmap->format,Bitmaps[BITMAP_RESIST_BG]->bitmap->flags);

    if (cur_widget[RESIST_ID].redraw)
    {
        cur_widget[RESIST_ID].redraw=FALSE;

        bltfx.surface=widgetSF[RESIST_ID];
        bltfx.flags = 0;
        bltfx.alpha=0;

        sprite_blt(Bitmaps[BITMAP_RESIST_BG], 0, 0, NULL, &bltfx);
        StringBlt(widgetSF[RESIST_ID], &Font6x3Out, "Resistance Table", 5,  1, COLOR_HGOLD, NULL, NULL);

        print_resist("IM", 68, 3, ATNR_PHYSICAL);
        print_resist("SL", 98, 3, ATNR_SLASH);
        print_resist("CL", 128, 3, ATNR_CLEAVE);
        print_resist("PI", 158, 3, ATNR_PIERCE);

        print_resist("FI", 8, 15, ATNR_FIRE);
        print_resist("CO", 38, 15, ATNR_COLD);
        print_resist("EL", 68, 15, ATNR_ELECTRICITY);
        print_resist("PO", 98, 15, ATNR_POISON);
        print_resist("AC", 128, 15, ATNR_ACID);
        print_resist("SO", 158, 15, ATNR_SONIC);

        print_resist("MA", 8, 27, ATNR_FORCE);
        print_resist("PS", 38, 27, ATNR_PSIONIC);
        print_resist("LI", 68, 27, ATNR_LIGHT);
        print_resist("SH", 98, 27, ATNR_SHADOW);
        print_resist("LS", 128, 27, ATNR_LIFESTEAL);
        print_resist("AE", 158, 27, ATNR_AETHER);

        print_resist("NE", 8, 39, ATNR_NETHER);
        print_resist("CH", 38, 39, ATNR_CHAOS);
        print_resist("DE", 68, 39, ATNR_DEATH);

        print_resist("WE", 98, 39, ATNR_WEAPONMAGIC);
        print_resist("GO", 128, 39, ATNR_GODPOWER);

        print_resist("DR", 8, 51, ATNR_DRAIN);
        print_resist("DE", 38, 51, ATNR_DEPLETION);
        print_resist("CR", 68, 51, ATNR_CORRUPTION);
        print_resist("CM", 98, 51, ATNR_COUNTERMAGIC);
        print_resist("CA", 128, 51, ATNR_CANCELLATION);
        print_resist("CF", 158, 51, ATNR_CONFUSION);

        print_resist("FE", 8, 63, ATNR_FEAR);
        print_resist("SL", 38, 63, ATNR_SLOW);
        print_resist("PA", 68, 63, ATNR_PARALYZE);
        print_resist("SN", 98, 63, ATNR_SNARE);
    }
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widgetSF[RESIST_ID], NULL, ScreenSurface, &box);
}

#define ICONDEFLEN 32
Boolean blt_face_centered(int face, int x, int y)
{
    register int temp;
    SDL_Rect    box;

    if (!FaceList[face].sprite)
        return FALSE;

    if (FaceList[face].sprite->status != SPRITE_STATUS_LOADED)
        return FALSE;

    box.x = FaceList[face].sprite->border_left;
    box.w = FaceList[face].sprite->bitmap->w;
    temp = box.w - FaceList[face].sprite->border_left - FaceList[face].sprite->border_right;
    if (temp > 32)
    {
        box.w = 32;
        temp -= 32;
        temp >>= 1;
        box.x += temp;
    }
    else if (temp < 32)
    {
        temp = 32 - temp;
        box.x -= (temp >> 1);
    }

    box.y = -FaceList[face].sprite->border_up;
    box.h = FaceList[face].sprite->bitmap->h;
    temp = box.h - FaceList[face].sprite->border_up - FaceList[face].sprite->border_down;
    if (temp > 32)
    {
        box.h = 32;
        temp -= 32;
        temp >>= 1;
        box.y += temp;
    }
    else if (temp < 32)
    {
        temp = 32 - temp;
        box.y = -(temp >> 1) + FaceList[face].sprite->border_up;
    }
    sprite_blt(FaceList[face].sprite, x, y, &box, NULL);

    return TRUE;
}

void widget_range_event(int x, int y, SDL_Event event, int MEvent)
{
    if (x > cur_widget[RANGE_ID].x1 + 5 &&
        x < cur_widget[RANGE_ID].x1 + 38 &&
        y >= cur_widget[RANGE_ID].y1 + 3 &&
        y <= cur_widget[RANGE_ID].y1 + 33)
    {
        if (MEvent==MOUSE_DOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
                process_macro_keys(KEYFUNC_RANGE, 0);
            else if (event.button.button == 4) /* mousewheel up */
                process_macro_keys(KEYFUNC_RANGE, 0);
            else if (event.button.button == 5) /* mousewheel down */
                process_macro_keys(KEYFUNC_RANGE_BACK, 0);
            else
                process_macro_keys(KEYFUNC_RANGE_BACK, 0);
        }
        else if (MEvent==MOUSE_UP)
        {
            if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
            {
                /* KEYFUNC_APPLY and KEYFUNC_DROP works only if cpl.inventory_win = IWIN_INV. The tag must
                            be placed in cpl.win_inv_tag. So we do this and after DnD we restore the old values. */
                int   old_inv_win = cpl.inventory_win;
                int   old_inv_tag = cpl.win_inv_tag;
                cpl.inventory_win = IWIN_INV;

                /* range field */
                if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_INV &&
                    x >= cur_widget[RANGE_ID].x1 &&
                    x <= cur_widget[RANGE_ID].x1 + 78 &&
                    y >= cur_widget[RANGE_ID].y1 &&
                    y <= cur_widget[RANGE_ID].y1 + 35)
                {
                    RangeFireMode = 4;
                    process_macro_keys(KEYFUNC_APPLY, 0); /* drop to player-doll */
                }
                cpl.inventory_win = old_inv_win;
                cpl.win_inv_tag = old_inv_tag;
            }
        }
    }
}

void widget_show_range(int x, int y)
{
    char        buf[256];
    SDL_Rect    rec_range;
    SDL_Rect    rec_item;
    item       *tmp, *tmp2;

    rec_range.w = 160;
    rec_item.w = 185;
    examine_range_inv();

    sprite_blt(Bitmaps[BITMAP_RANGE], x, y, NULL, NULL);

    switch (RangeFireMode)
    {
        case FIRE_MODE_BOW:
            if (fire_mode_tab[FIRE_MODE_BOW].item != FIRE_ITEM_NO)
            {
                tmp2=locate_item(fire_mode_tab[FIRE_MODE_BOW].item);
                if (!tmp2)
                {
                    LOG(LOG_DEBUG,"BUG: applied range weapon don't exist\n");
                    StringBlt(ScreenSurface, &SystemFont, "using Nothing", x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);
                }
                else
                {
                    sprintf(buf, "using %s", tmp2->s_name);
                    blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_BOW].item, x + 5, y + 2);

                    StringBlt(ScreenSurface, &SystemFont, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);
                }

                if (fire_mode_tab[FIRE_MODE_BOW].amun != FIRE_ITEM_NO)
                {
                    tmp = locate_item_from_item(cpl.ob, fire_mode_tab[FIRE_MODE_BOW].amun);
                    if (tmp)
                    {
                        if (tmp->itype == TYPE_ARROW)
                            sprintf(buf, "ammo %s (%d)", get_range_item_name(fire_mode_tab[FIRE_MODE_BOW].amun), tmp->nrof);
                        else
                            sprintf(buf, "ammo %s", get_range_item_name(fire_mode_tab[FIRE_MODE_BOW].amun));
                    }
                    else
                        strcpy(buf, "ammo not selected");
                    blt_inventory_face_from_tag(fire_mode_tab[FIRE_MODE_BOW].amun, x + 45, y + 2);
                }
                else if (tmp2->itype==TYPE_BOW)
                {
                    sprintf(buf, "ammo not selected");
                }
                else if (tmp2->itype==TYPE_ARROW)
                {
                    sprintf(buf, "amount: %d",tmp2->nrof);
                }
                else
//                    sprintf(buf, "Type: %d",tmp2->itype);
                    buf[0]=0;

                StringBlt(ScreenSurface, &SystemFont, buf, x + 5, y + 47, COLOR_WHITE, &rec_item, NULL);
            }
            else
            {
                sprintf(buf, "no range weapon applied");
                StringBlt(ScreenSurface, &SystemFont, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);
            }

            sprite_blt(Bitmaps[BITMAP_RANGE_MARKER], x + 5, y + 2, NULL, NULL);
            break;

            /* these are client only, no server signal needed */
        case FIRE_MODE_SKILL:
            if (fire_mode_tab[FIRE_MODE_SKILL].skill)
            {
                sprite_blt(Bitmaps[BITMAP_RANGE_SKILL], x + 5, y + 2, NULL, NULL);
                if (fire_mode_tab[FIRE_MODE_SKILL].skill->flag != -1)
                {
                    sprite_blt(fire_mode_tab[FIRE_MODE_SKILL].skill->icon, x + 45, y + 2, NULL, NULL);
                    StringBlt(ScreenSurface, &SystemFont, fire_mode_tab[FIRE_MODE_SKILL].skill->name, x + 5, y + 47,
                              COLOR_WHITE, &rec_item, NULL);
                }
                else
                    fire_mode_tab[FIRE_MODE_SKILL].skill = NULL;
            }
            else
            {
                sprite_blt(Bitmaps[BITMAP_RANGE_SKILL_NO], x + 5, y + 2, NULL, NULL);
                sprintf(buf, "no skill selected");
                StringBlt(ScreenSurface, &SystemFont, buf, x + 5, y + 47, COLOR_WHITE, &rec_item, NULL);
            }
            sprintf(buf, "use skill");
            StringBlt(ScreenSurface, &SystemFont, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);

            break;
        case FIRE_MODE_SPELL:
            if (fire_mode_tab[FIRE_MODE_SPELL].spell)
            {
                /* we use wiz spells as default */
                sprite_blt(Bitmaps[BITMAP_RANGE_WIZARD], x + 5, y + 2, NULL, NULL);
                if (fire_mode_tab[FIRE_MODE_SPELL].spell->flag != -1)
                {
                    sprite_blt(fire_mode_tab[FIRE_MODE_SPELL].spell->icon, x + 45, y + 2, NULL, NULL);
                    StringBlt(ScreenSurface, &SystemFont, fire_mode_tab[FIRE_MODE_SPELL].spell->name, x + 5, y + 47,
                              COLOR_WHITE, &rec_item, NULL);
                }
                else
                    fire_mode_tab[FIRE_MODE_SPELL].spell = NULL;
            }
            else
            {
                sprite_blt(Bitmaps[BITMAP_RANGE_WIZARD_NO], x + 5, y + 2, NULL, NULL);
                sprintf(buf, "no spell selected");
                StringBlt(ScreenSurface, &SystemFont, buf, x + 5, y + 47, COLOR_WHITE, &rec_item, NULL);
            }
            sprintf(buf, "cast spell");
            StringBlt(ScreenSurface, &SystemFont, buf, x + 5, y + 36, COLOR_WHITE, &rec_range, NULL);

            break;
    };
}

static char * get_range_item_name(int tag)
{
    item   *tmp;

    if (tag != FIRE_ITEM_NO)
    {
        tmp = locate_item(tag);
        if (tmp)
            return tmp->s_name;
    }
    return("Nothing");
}

void blt_inventory_face_from_tag(int tag, int x, int y)
{
    item   *tmp;

    /* check item is in inventory and faces are loaded, etc */
    tmp = locate_item(tag);
    if (!tmp)
        return;
    blt_inv_item_centered(tmp, x, y);
}

void show_menu(void)
{
    SDL_Rect    box;

    if (!cpl.menustatus)
        return;
    if (cpl.menustatus == MENU_KEYBIND)
        show_keybind();
    else if (cpl.menustatus == MENU_BOOK)
        show_book(400-Bitmaps[BITMAP_JOURNAL]->bitmap->w/2,300-Bitmaps[BITMAP_JOURNAL]->bitmap->h/2);
    else if (cpl.menustatus == MENU_NPC)
        show_interface_npc(esc_menu_index);
    else if (cpl.menustatus == MENU_STATUS)
        show_status();
    else if (cpl.menustatus == MENU_SPELL)
    {
        show_spelllist();
        box.x = Screensize.x / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->w / 2;
        box.y = Screensize.y / 2 - Bitmaps[BITMAP_DIALOG_BG]->bitmap->h / 2 - 42;
        box.h = 42;
        box.w = Bitmaps[BITMAP_DIALOG_BG]->bitmap->w;
        SDL_FillRect(ScreenSurface, &box, 0);
        show_quickslots(box.x + 120, box.y + 3);
    }
    else if (cpl.menustatus == MENU_SKILL)
        show_skilllist();
    else if (cpl.menustatus == MENU_OPTION)
        show_optwin();
    else if (cpl.menustatus == MENU_CREATE)
        show_newplayer_server();
}

void show_media(int x, int y)
{
    _Sprite    *bmap;
    int         xtemp;

    if (media_show != MEDIA_SHOW_NO)
    {
        /* we show a png*/
        if (media_file[media_show].type == MEDIA_TYPE_PNG)
        {
            bmap = (_Sprite *) media_file[media_show].data;
            if (bmap)
            {
                xtemp = x - bmap->bitmap->w;
                sprite_blt(bmap, xtemp, y, NULL, NULL);
            }
        }
    }
}

void widget_show_mapname(int x, int y)
{
    StringBlt(ScreenSurface, &BigFont, MapData.name, x, y, COLOR_HGOLD, NULL, NULL);
}


void show_status(void)
{
    /*
            int y, x;
            x= Screensize.x/2-Bitmaps[BITMAP_STATUS]->bitmap->w/2;
            y= Screensize.y/2-Bitmaps[BITMAP_STATUS]->bitmap->h/2;
            sprite_blt(Bitmaps[BITMAP_STATUS],x, y, NULL, NULL);
    */
}

#if 0
int init_media_tag(char *tag)
{
    char   *p1, *p2, buf[256];
    int     temp;
    int     ret = 0;

    if (tag == NULL)
    {
        LOG(LOG_MSG, "MediaTagError: Tag == NULL\n");
        return ret;
    }
    p1 = strchr(tag, '|');
    p2 = strrchr(tag, '|');
    if (p1 == NULL || p2 == NULL)
    {
        LOG(LOG_MSG, "MediaTagError: Parameter == NULL (%x %x)\n", p1, p2);
        return ret;
    }
    *p1++ = 0;
    *p2++ = 0;

    if (strstr(tag + 1, ".ogg"))
    {
        sound_play_music(tag + 1, options.music_volume, 2000, atoi(p2), atoi(p1), MUSIC_MODE_NORMAL);
        ret = 1; /* because we have called sound_play_music, we don't must fade out extern */
    }
    else if (strstr(tag + 1, ".png"))
    {
        media_show_update = 2;
        /* because we chain this to map_scroll, but map_scroll can
        * come behind the draw_info cmd... sigh*/

        /* first, we look in our media buffers.. perhaps this is still buffered
        * is so, just update the paramter and fire it up */
        if (!strcmp(media_file[media_count].name, tag + 1))
        {
            media_show = media_count;
            media_file[media_count].p1 = atoi(p1);
            media_file[media_count].p2 = atoi(p2);
            return ret;
        }
        /* if not loaded, we overwrite our oldest buffered media file */

        media_show = MEDIA_SHOW_NO;
        temp = (media_count + 1) % MEDIA_MAX;
        if (media_file[temp].data) /* if some here, kick it*/
        {
            media_file[temp].type = MEDIA_TYPE_NO;
            FreeMemory(&media_file[temp].data);
        }
        sprintf(buf, "%s%s", GetMediaDirectory(), tag + 1);
        if (!(media_file[temp].data = sprite_load_file(buf, 0)))
            return ret;

        media_file[temp].type = MEDIA_TYPE_PNG;
        strcpy(media_file[temp].name, tag + 1);
        media_count = temp;
        media_show = media_count;
    }
    return ret;
}
#else
/* The name and file of this func makes little sense anymore, as the so-called
 * 'media tag' is only used for a map's background music. But maybe someone
 * will come up with other usefull media types in the future.
 *
 * We no longer allow PNG media. This is because PNG media makes no sense now
 * that we have floating widgets, etc. AFAIK it was only ever used in the old
 * <=B3 tutorial to show the keypad image (if indeed it was even used there).
 * In any case, Gridarta assumes it is for music only so I doubt very much that
 * anyone has used it otherwise.
 *
 * When we drop support for § completely, we can remove this func and move
 * the code into UpdateMapMusic() in map.c.
 *
 * ATM we still use pipes to separate the parameters, mainly because it is just
 * easier that way. These may be changed or removed in the future. */
int init_media_tag(char *tag)
{
    char *p1,
         *p2;

    if (tag == NULL)
    {
        LOG(LOG_MSG, "MediaTagError: Tag == NULL\n");

        return 0;
    }

    p1 = strchr(tag, '|');
    p2 = strrchr(tag, '|');

    if (p1 == NULL || p2 == NULL)
    {
        LOG(LOG_MSG, "MediaTagError: Parameter == NULL (%x %x)\n",
            p1, p2);

        return 0;
    }

    *p1++ = 0;
    *p2++ = 0;

    if (strstr(tag + 1, ".ogg"))
    {
        sound_play_music(tag + 1, options.music_volume, 2000, atoi(p2),
                         atoi(p1), MUSIC_MODE_NORMAL);

        return 1;
    }

    LOG(LOG_MSG, "MediaTagError: Unrecognised media (not .OGG)\n");

    return 0;
}
#endif

int blt_window_slider(_Sprite *slider, int maxlen, int winlen, int startoff, int len, int x, int y)
{
    SDL_Rect    box;
    double      temp;
    int         startpos, len_h;

    if (len != -1)
        len_h = len;
    else
        len_h = slider->bitmap->h;

    if (maxlen < winlen)
        maxlen = winlen;
    if (startoff + winlen > maxlen)
        maxlen = startoff + winlen;

    box.x = 0;
    box.y = 0;
    box.w = slider->bitmap->w;

    /* now we have 100% = 1.0 to 0% = 0.0 of the length */
    temp = (double) winlen / (double) maxlen; /* between 0.0 <-> 1.0 */
    startpos = (int) ((double) startoff * ((double) len_h / (double) maxlen)); /* startpixel */
    temp = (double) len_h * temp;
    box.h = (Uint16) temp;
    if (!box.h)
        box.h = 1;

    if (startoff + winlen >= maxlen && startpos + box.h < len_h)
        startpos ++;

    sprite_blt(slider, x, y + startpos, &box, NULL);
    return 0;
}

int read_anim_tmp(void)
{
    FILE       *stream;
    struct stat stat_bmap, stat_anim, stat_tmp;

    /* if this fails, we have a urgent problem somewhere before */
    if ((stream = fopen_wrapper(FILE_BMAPS_TMP, "rb")) == NULL)
    {
        LOG(LOG_ERROR, "read_anim_tmp:Error reading bmap.tmp for anim.tmp!\n");
        SYSTEM_End(); /* fatal */
        exit(0);
    }
    fstat(fileno(stream), &stat_bmap);
    fclose(stream);

    if ((stream = fopen_wrapper(FILE_CLIENT_ANIMS, "rb")) == NULL)
    {
        LOG(LOG_ERROR, "read_anim_tmp:Error reading bmap.tmp for anim.tmp!\n");
        SYSTEM_End(); /* fatal */
        exit(0);
    }
    fstat(fileno(stream), &stat_anim);
    fclose(stream);

    if ((stream = fopen_wrapper(FILE_ANIMS_TMP, "rb")) != NULL)
    {
        fstat(fileno(stream), &stat_tmp);
        fclose(stream);

        /* our anim file must be newer as our default anim file */
        if (difftime(stat_tmp.st_mtime, stat_anim.st_mtime) > 0.0f)
        {
            /* our anim file must be newer as our bmaps.tmp */
            if (difftime(stat_tmp.st_mtime, stat_bmap.st_mtime) > 0.0f)
                return load_anim_tmp(); /* all fine - load file */
        }
    }

    create_anim_tmp();

    return load_anim_tmp(); /* all fine - load file */
}

/* The editor creates the facespack alphabetically, so we can do a quick binary search... */
int get_facenum_from_name(char * name)
{
    int l=0;
    int r=(bmaptype_table_size-1);
    int x;
    while(r >= l)
    {
        x=(l+r)/2;
        if(strcmp(name, bmaptype_table[x].name)<0)/* smaller? */
            r=x-1;
        else      /* bigger! */
            l=x+1;
        if(!strcmp(name, bmaptype_table[x].name))
            return x;     /* Gefunden; x = Position*/
    }
    return -1;
}


void read_anims(void)
{
    FILE       *stream;
    unsigned char *temp_buf;
    struct stat statbuf;
    int         i;

    LOG(LOG_DEBUG, "Loading %s....", FILE_CLIENT_ANIMS);
    srv_client_files[SRV_CLIENT_ANIMS].len = 0;
    srv_client_files[SRV_CLIENT_ANIMS].crc = 0;
    if ((stream = fopen_wrapper(FILE_CLIENT_ANIMS, "rb")) != NULL)
    {
        /* temp load the file and get the data we need for compare with server */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_ANIMS].len = i;
        temp_buf = malloc(i);
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_ANIMS].crc = crc32(1L, temp_buf, i);
        free(temp_buf);
        fclose(stream);
        LOG(LOG_DEBUG, " found file!(%d/%x)", srv_client_files[SRV_CLIENT_ANIMS].len,
            srv_client_files[SRV_CLIENT_ANIMS].crc);
    }
    LOG(LOG_DEBUG, "done.\n");
}

/* after we tested and/or created bmaps.p0 - load the data from it */
static void load_bmaps_p0(void)
{
    char    buf[HUGE_BUF];
    char    name[HUGE_BUF];
    int     len, pos, num;
    unsigned int crc;
    _bmaptype  *at;
    FILE       *fbmap;

    /* clear bmap hash table */
    memset((void *) bmap_table, 0, BMAPTABLE * sizeof(_bmaptype *));

    /* try to open bmaps_p0 file */
    if ((fbmap = fopen_wrapper(FILE_BMAPS_P0, "rb")) == NULL)
    {
        LOG(LOG_ERROR, "FATAL: Error loading bmaps.p0!\n");
        SYSTEM_End(); /* fatal */
        unlink(FILE_BMAPS_P0);
        exit(0);
    }
    while (fgets(buf, HUGE_BUF - 1, fbmap) != NULL)
    {
        sscanf(buf, "%d %d %x %d %s", &num, &pos, &crc, &len, name);

        at = (_bmaptype *) malloc(sizeof(_bmaptype));
        at->name = (char *) malloc(strlen(name) + 1);
        strcpy(at->name, name);
        at->crc = crc;
        at->num = num;
        at->len = len;
        at->pos = pos;
        add_bmap(at);
        /*LOG(LOG_DEBUG,"%d %d %d %x >%s<\n", num, pos, len, crc, name);*/
    }
    fclose(fbmap);
}


/* read and/or create the bmaps.p0 file out of the
 * daimonin.p0 file
 */
void read_bmaps_p0(void)
{
    FILE   *fbmap, *fpic;
    unsigned char *temp_buf;
    char *cp;
    int     bufsize, len, num, pos;
    unsigned int crc;
    char   buf[HUGE_BUF], line_buf[256];

    if ((fpic = fopen_wrapper(FILE_DAIMONIN_P0, "rb")) == NULL)
    {
        LOG(LOG_ERROR, "FATAL: Can't find daimonin.p0 file!\n");
        SYSTEM_End(); /* fatal */
        unlink(FILE_BMAPS_P0);
        exit(0);
    }

    if ((fbmap = fopen_wrapper(FILE_BMAPS_P0, "w")) == NULL)
    {
        LOG(LOG_ERROR, "FATAL: Can't create bmaps.p0 file!\n");
        SYSTEM_End(); /* fatal */
        fclose(fbmap);
        unlink(FILE_BMAPS_P0);
        exit(0);
    }
    temp_buf = malloc((bufsize = 24 * 1024));

    while (fgets(buf, HUGE_BUF - 1, fpic) != NULL)
    {
        if (strncmp(buf, "IMAGE ", 6) != 0)
        {
            LOG(LOG_ERROR, "read_client_images:Bad image line - not IMAGE, instead\n%s", buf);
            SYSTEM_End(); /* fatal */
            fclose(fbmap);
            fclose(fpic);
            unlink(FILE_BMAPS_P0);
            exit(0);
        }

        num = atoi(buf + 6);
        /* Skip accross the number data */
        for (cp = buf + 6; *cp != '\0' && *cp != ' '; cp++)
            ;
        len = atoi(cp);

        pos = (int) ftell(fpic);

        if (len > bufsize) /* dynamic buffer adjustment */
        {
            free(temp_buf);
            /* we assume thats this is nonsense */
            if (len > 128 * 1024)
            {
                LOG(LOG_ERROR, "read_client_images:Size of picture out of bounds!(len:%d)(pos:%d)\n", len, pos);
                SYSTEM_End(); /* fatal */
                fclose(fbmap);
                fclose(fpic);
                unlink(FILE_BMAPS_P0);
                exit(0);
            }
            bufsize = len;
            temp_buf = malloc(bufsize);
        }

        fread(temp_buf, 1, len, fpic);
        crc = crc32(1L, temp_buf, len);

        /* now we got all we needed! */
        sprintf(line_buf, "%d %d %x %s", num, pos, crc, cp);
        fputs(line_buf, fbmap);
        /*      LOG(LOG_DEBUG,"FOUND: %s\n", temp_buf);       */
    }


    free(temp_buf);
    fclose(fbmap);
    fclose(fpic);
    load_bmaps_p0();
    return;
}

void delete_bmap_tmp(void)
{
    int i;

    bmaptype_table_size = 0;
    for (i = 0; i < BMAPTABLE; i++)
    {
        if (bmaptype_table[i].name)
            free(bmaptype_table[i].name);
        bmaptype_table[i].name = NULL;
    }
}

static int load_bmap_tmp(void)
{
    FILE   *stream;
    char    buf[HUGE_BUF], name[HUGE_BUF];
    int     i = 0, len, pos;
    unsigned int crc;

    delete_bmap_tmp();
    if ((stream = fopen_wrapper(FILE_BMAPS_TMP, "rt")) == NULL)
    {
        LOG(LOG_ERROR, "bmaptype_table(): error open file <bmap.tmp>\n");
        SYSTEM_End(); /* fatal */
        exit(0);
    }
    while (fgets(buf, HUGE_BUF - 1, stream) != NULL)
    {
        sscanf(buf, "%d %d %x %s\n", &pos, &len, &crc, name);
        bmaptype_table[i].crc = crc;
        bmaptype_table[i].len = len;
        bmaptype_table[i].pos = pos;
        bmaptype_table[i].name = (char *) malloc(strlen(name) + 1);
        strcpy(bmaptype_table[i].name, name);
        i++;
    }
    bmaptype_table_size = i;
    fclose(stream);
    return 0;
}


int read_bmap_tmp(void)
{
    FILE       *stream, *fbmap0;
    char        buf[HUGE_BUF], name[HUGE_BUF];
    struct stat stat_bmap, stat_tmp, stat_bp0;
    int len;
    unsigned int crc;
    _bmaptype  *at;

    if ((stream = fopen_wrapper(FILE_CLIENT_BMAPS, "rb")) == NULL)
    {
        /* we can't make bmaps.tmp without this file */
        unlink(FILE_BMAPS_TMP);
        return 1;
    }
    fstat(fileno(stream), &stat_bmap);
    fclose(stream);

    if ((stream = fopen_wrapper(FILE_BMAPS_P0, "rb")) == NULL)
    {
        /* we can't make bmaps.tmp without this file */
        unlink(FILE_BMAPS_TMP);
        return 1;
    }
    fstat(fileno(stream), &stat_bp0);
    fclose(stream);

    if ((stream = fopen_wrapper(FILE_BMAPS_TMP, "rb")) == NULL)
        goto create_bmap_tmp;
    fstat(fileno(stream), &stat_tmp);
    fclose(stream);

    /* ok - client_bmap & bmaps.p0 are there - now check
     * our bmap_tmp is newer - is not newer as both, we
     * create it new - then it is newer.
     */

    if (difftime(stat_tmp.st_mtime, stat_bmap.st_mtime) > 0.0f)
    {
        if (difftime(stat_tmp.st_mtime, stat_bp0.st_mtime) > 0.0f)
            return load_bmap_tmp(); /* all fine */
    }

create_bmap_tmp:
    unlink(FILE_BMAPS_TMP);

    /* NOW we are sure... we must create us a new bmaps.tmp */
    if ((stream = fopen_wrapper(FILE_CLIENT_BMAPS, "rb")) != NULL)
    {
        /* we can use text mode here, its local */
        if ((fbmap0 = fopen_wrapper(FILE_BMAPS_TMP, "wt")) != NULL)
        {
            /* read in the bmaps from the server, check with the
                     * loaded bmap table (from bmaps.p0) and create with
                     * this information the bmaps.tmp file.
                     */
            while (fgets(buf, HUGE_BUF - 1, stream) != NULL)
            {
                sscanf(buf, "%x %x %s", &len, &crc, name);
                at = find_bmap(name);

                /* now we can check, our local file package has
                         * the right png - if not, we mark this pictures
                         * as "in cache". We don't check it here now -
                         * that will happens at runtime.
                         * That can change when we include later a forbidden
                         * flag in the server (no face send) - then we need
                         * to break and upddate the picture and/or check the cache.
                         */
                /* position -1 mark "not i the daimonin.p0 file */
                if (!at || at->len != len || at->crc != crc) /* is different or not there! */
                    sprintf(buf, "-1 %d %x %s\n", len, crc, name);
                else /* we have it */
                    sprintf(buf, "%d %d %x %s\n", at->pos, len, crc, name);
                fputs(buf, fbmap0);
            }
            fclose(fbmap0);
        }
        fclose(stream);
    }
    return load_bmap_tmp(); /* all fine */
}


void read_bmaps(void)
{
    FILE          *stream;
    unsigned char *temp_buf;
    struct stat statbuf;
    int         i;

    srv_client_files[SRV_CLIENT_BMAPS].len = 0;
    srv_client_files[SRV_CLIENT_BMAPS].crc = 0;
    LOG(LOG_DEBUG, "Reading %s....", FILE_CLIENT_BMAPS);
    if ((stream = fopen_wrapper(FILE_CLIENT_BMAPS, "rb")) != NULL)
    {
        /* temp load the file and get the data we need for compare with server */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_BMAPS].len = i;
        temp_buf = malloc(i);
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_BMAPS].crc = crc32(1L, temp_buf, i);
        free(temp_buf);
        fclose(stream);
        LOG(LOG_DEBUG, " found file!(%d/%x)", srv_client_files[SRV_CLIENT_BMAPS].len,
            srv_client_files[SRV_CLIENT_BMAPS].crc);
    }
    else
    {
        unlink(FILE_BMAPS_TMP);
        LOG(LOG_DEBUG, "done.\n");
        return;
    }

    LOG(LOG_DEBUG, "done.\n");
}

/* in the setting files we have a list of chars templates
 * for char building. Delete this list here.
 */
void delete_server_chars(void)
{
    _server_char   *tmp, *tmp1;

    for (tmp1 = tmp = first_server_char; tmp1; tmp = tmp1)
    {
        tmp1 = tmp->next;
        free(tmp->name);
        free(tmp->desc[0]);
        free(tmp->desc[1]);
        free(tmp->desc[2]);
        free(tmp->desc[3]);
        free(tmp->char_arch[0]);
        free(tmp->char_arch[1]);
        free(tmp->char_arch[2]);
        free(tmp->char_arch[3]);
        free(tmp);
    }
    first_server_char = NULL;
}


/* find a face ID by name,
 * request the face (find it, load it or request it)
 * and return the ID
 */
int get_bmap_id(char *name)
{
    int i;

    for (i = 0; i < bmaptype_table_size; i++)
    {
        if (bmaptype_table[i].name[0] && !strcmp(bmaptype_table[i].name, name))
        {
            request_face(i);
            return i;
        }
    }

    return -1;
}

void load_settings(void)
{
    FILE   *stream;
    char    buf[HUGE_BUF], buf1[HUGE_BUF], buf2[HUGE_BUF];
    char    cmd[HUGE_BUF];
    char    para[HUGE_BUF];
    int     para_count = 0, last_cmd = 0;
    int     tmp_level   = 0;

    delete_server_chars();
    LOG(LOG_DEBUG, "Loading %s....\n", FILE_CLIENT_SETTINGS);
    if ((stream = fopen_wrapper(FILE_CLIENT_SETTINGS, "rb")) != NULL)
    {
        while (fgets(buf, HUGE_BUF - 1, stream) != NULL)
        {
            if (buf[0] == '#' || buf[0] == '\0')
                continue;

            if (last_cmd == 0)
            {
                sscanf(adjust_string(buf), "%s %s", cmd, para);
                if (!strcmp(cmd, "char"))
                {
                    _server_char   *serv_char   = malloc(sizeof(_server_char));

                    memset(serv_char, 0, sizeof(_server_char));
                    /* copy name */
                    serv_char->name = malloc(strlen(para) + 1);
                    strcpy(serv_char->name, para);

                    /* get next legal line */
                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    sscanf(adjust_string(buf), "%s %d %d %d %d %d %d", buf1, &serv_char->bar[0], &serv_char->bar[1],
                           &serv_char->bar[2], &serv_char->bar_add[0], &serv_char->bar_add[1], &serv_char->bar_add[2]);

                    serv_char->pic_id = get_bmap_id(buf1);

                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    sscanf(adjust_string(buf), "%d %s %s", &serv_char->gender[0], buf1, buf2);
                    serv_char->char_arch[0] = malloc(strlen(buf1) + 1);
                    strcpy(serv_char->char_arch[0], buf1);
                    serv_char->face_id[0] = get_bmap_id(buf2);

                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    sscanf(adjust_string(buf), "%d %s %s", &serv_char->gender[1], buf1, buf2);
                    serv_char->char_arch[1] = malloc(strlen(buf1) + 1);
                    strcpy(serv_char->char_arch[1], buf1);
                    serv_char->face_id[1] = get_bmap_id(buf2);

                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    sscanf(adjust_string(buf), "%d %s %s", &serv_char->gender[2], buf1, buf2);
                    serv_char->char_arch[2] = malloc(strlen(buf1) + 1);
                    strcpy(serv_char->char_arch[2], buf1);
                    serv_char->face_id[2] = get_bmap_id(buf2);

                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    sscanf(adjust_string(buf), "%d %s %s", &serv_char->gender[3], buf1, buf2);
                    serv_char->char_arch[3] = malloc(strlen(buf1) + 1);
                    strcpy(serv_char->char_arch[3], buf1);
                    serv_char->face_id[3] = get_bmap_id(buf2);

                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    sscanf(adjust_string(buf), "%d %d %d %d %d %d %d\n",
                           &serv_char->stats[0],
                           &serv_char->stats[1], &serv_char->stats[2], &serv_char->stats[3],
                           &serv_char->stats[4], &serv_char->stats[5], &serv_char->stats[6]);

                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    serv_char->desc[0] = malloc(strlen(adjust_string(buf)) + 1);
                    strcpy(serv_char->desc[0], buf);
                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    serv_char->desc[1] = malloc(strlen(adjust_string(buf)) + 1);
                    strcpy(serv_char->desc[1], buf);
                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    serv_char->desc[2] = malloc(strlen(adjust_string(buf)) + 1);
                    strcpy(serv_char->desc[2], buf);
                    while (fgets(buf, HUGE_BUF - 1, stream) != NULL && (buf[0] == '#' || buf[0] == '\0'))
                        ;
                    serv_char->desc[3] = malloc(strlen(adjust_string(buf)) + 1);
                    strcpy(serv_char->desc[3], buf);
                    serv_char->skill_selected = 0;

                    /* add this char template to list */
                    if (!first_server_char)
                        first_server_char = serv_char;
                    else
                    {
                        _server_char   *tmpc;

                        for (tmpc = first_server_char; tmpc->next; tmpc = tmpc->next)
                            ;
                        tmpc->next = serv_char;
                        serv_char->prev = tmpc;
                    }
                }
                else if (!strcmp(cmd, "level"))
                {
                    tmp_level = atoi(para);
                    if (tmp_level<0 || tmp_level> 450)
                    {
                        fclose(stream);
                        LOG(LOG_ERROR, "client_settings_: level cmd out of bounds! >%s<\n", buf);
                        return;
                    }
                    server_level.level = tmp_level;
                    last_cmd = 1; /* cmd 'level' */
                    para_count = 0;
                }
                else /* we close here... better we include later a fallback to login */
                {
                    fclose(stream);
                    LOG(LOG_ERROR, "Unknown command in client_settings! >%s<\n", buf);
                    return;
                }
            }
            else if (last_cmd == 1)
            {
                server_level.exp[para_count++] = strtoul(buf, NULL, 16);
                if (para_count > tmp_level)
                    last_cmd = 0;
            }
        }
        fclose(stream);
    }

    if (first_server_char)
    {
        int g;

        memcpy(&new_character, first_server_char, sizeof(_server_char));
        new_character.skill_selected = 0;
        /* adjust gender */
        for (g = 0; g < 4; g++)
        {
            if (new_character.gender[g])
            {
                new_character.gender_selected = g;
                break;
            }
        }
    }
}

void read_settings(void)
{
    FILE       *stream;
    unsigned char *temp_buf;
    struct stat statbuf;
    int         i;

    srv_client_files[SRV_CLIENT_SETTINGS].len = 0;
    srv_client_files[SRV_CLIENT_SETTINGS].crc = 0;
    LOG(LOG_DEBUG, "Reading %s....", FILE_CLIENT_SETTINGS);
    if ((stream = fopen_wrapper(FILE_CLIENT_SETTINGS, "rb")) != NULL)
    {
        /* temp load the file and get the data we need for compare with server */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_SETTINGS].len = i;
        temp_buf = malloc(i);
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_SETTINGS].crc = crc32(1L, temp_buf, i);
        free(temp_buf);
        fclose(stream);
        LOG(LOG_DEBUG, " found file!(%d/%x)", srv_client_files[SRV_CLIENT_SETTINGS].len,
            srv_client_files[SRV_CLIENT_SETTINGS].crc);
    }
    LOG(LOG_DEBUG, "done.\n");
}

void read_spells(void)
{
    int         i, ii, panel;
    char        type, nchar, *tmp, *tmp2;
    struct stat statbuf;
    FILE       *stream;
    unsigned char *temp_buf;
    char        spath[255],line[255], name[255], d1[255], d2[255], d3[255], d4[255], icon[128];

    for (i = 0; i < SPELL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            spell_list[i].entry[0][ii].flag = LIST_ENTRY_UNUSED;
            spell_list[i].entry[1][ii].flag = LIST_ENTRY_UNUSED;
            spell_list[i].entry[0][ii].name[0] = 0;
            spell_list[i].entry[1][ii].name[0] = 0;
        }
    }
    spell_list_set.class_nr = 0;
    spell_list_set.entry_nr = 0;
    spell_list_set.group_nr = 0;

    srv_client_files[SRV_CLIENT_SPELLS].len = 0;
    srv_client_files[SRV_CLIENT_SPELLS].crc = 0;
    LOG(LOG_DEBUG, "Reading %s.... ", FILE_CLIENT_SPELLS);
    if ((stream = fopen_wrapper(FILE_CLIENT_SPELLS, "rb")) != NULL)
    {
        /* temp load the file and get the data we need for compare with server */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_SPELLS].len = i;
        temp_buf = malloc(i);
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_SPELLS].crc = crc32(1L, temp_buf, i);
        free(temp_buf);
        rewind(stream);

        for (i = 0; ; i++)
        {
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(name, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            sscanf(line, "%c %c %s %s", &type, &nchar, spath, icon);
            /*LOG(-1,"STRING:(%s) >%s< >%s<\n",line,  spath, icon);*/
            if (isdigit(spath[0]))
            {
                panel = atoi(spath)-1;
                if (panel >=SPELL_LIST_MAX)
                {
                    LOG(LOG_DEBUG,"BUG: spell path out of range (%d) for line %s\n", panel, line);
                    panel = 0;
                }
            }
            else
            {
                int a;

                panel = -1;
                for (a=0;a<SPELL_LIST_MAX;a++)
                {
                    if (!strcmp(spell_tab[a], spath))
                    {
                        panel = a;
                    }
                }
                if (panel == -1)
                {
                    LOG(LOG_DEBUG,"BUG: spell path out of range/wrong name (%s) for line %s\n", spath, line);
                    panel = 0;
                }
            }
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d1, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d2, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d3, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d4, tmp + 1);
            spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].flag = LIST_ENTRY_USED;
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].icon_name, icon);
            sprintf(line, "%s%s", GetIconDirectory(), icon);
            spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].icon = sprite_load_file(line, SURFACE_FLAG_DISPLAYFORMAT);

            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].name, name);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[0], d1);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[1], d2);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[2], d3);
            strcpy(spell_list[panel].entry[type == 'w' ? 0 : 1][nchar - 'a'].desc[3], d4);
        }
        fclose(stream);
        LOG(LOG_DEBUG, " found file!(%d/%x)", srv_client_files[SRV_CLIENT_SPELLS].len,
            srv_client_files[SRV_CLIENT_SPELLS].crc);
    }
    LOG(LOG_DEBUG, "done.\n");
}

void read_skills(void)
{
    int         i, ii, panel;
    unsigned char *temp_buf;
    char        nchar, *tmp, *tmp2;
    struct stat statbuf;
    FILE       *stream;
    char        line[255], name[255], d1[255], d2[255], d3[255], d4[255], icon[128];

    for (i = 0; i < SKILL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            skill_list[i].entry[ii].flag = LIST_ENTRY_UNUSED;
            skill_list[i].entry[ii].name[0] = 0;
        }
    }

    skill_list_set.group_nr = 0;
    skill_list_set.entry_nr = 0;

    srv_client_files[SRV_CLIENT_SKILLS].len = 0;
    srv_client_files[SRV_CLIENT_SKILLS].crc = 0;

    LOG(LOG_DEBUG, "Reading %s....", FILE_CLIENT_SKILLS);
    if ((stream = fopen_wrapper(FILE_CLIENT_SKILLS, "rb")) != NULL)
    {
        /* temp load the file and get the data we need for compare with server */
        fstat(fileno(stream), &statbuf);
        i = (int) statbuf.st_size;
        srv_client_files[SRV_CLIENT_SKILLS].len = i;
        temp_buf = malloc(i);
        fread(temp_buf, sizeof(char), i, stream);
        srv_client_files[SRV_CLIENT_SKILLS].crc = crc32(1L, temp_buf, i);
        free(temp_buf);
        rewind(stream);

        for (i = 0; ; i++)
        {
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(name, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            sscanf(line, "%d %c %s", &panel, &nchar, icon);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d1, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d2, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d3, tmp + 1);
            if (fgets(line, 255, stream) == NULL)
                break;
            line[250] = 0;
            tmp = strchr(line, '"');
            tmp2 = strchr(tmp + 1, '"');
            *tmp2 = 0;
            strcpy(d4, tmp + 1);

            skill_list[panel].entry[nchar - 'a'].flag = LIST_ENTRY_USED;
            skill_list[panel].entry[nchar - 'a'].exp = 0;
            skill_list[panel].entry[nchar - 'a'].exp_level = 0;

            strcpy(skill_list[panel].entry[nchar - 'a'].icon_name, icon);
            sprintf(line, "%s%s", GetIconDirectory(), icon);
            skill_list[panel].entry[nchar - 'a'].icon = sprite_load_file(line, SURFACE_FLAG_DISPLAYFORMAT);

            strcpy(skill_list[panel].entry[nchar - 'a'].name, name);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[0], d1);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[1], d2);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[2], d3);
            strcpy(skill_list[panel].entry[nchar - 'a'].desc[3], d4);
        }
        fclose(stream);
        LOG(LOG_DEBUG, " found file!(%d/%x)", srv_client_files[SRV_CLIENT_SKILLS].len,
            srv_client_files[SRV_CLIENT_SKILLS].crc);
    }
    LOG(LOG_DEBUG, "done.\n");
}


int get_quickslot(int x, int y)
{
    int i;
    int qsx, qsy, xoff;
    if (cur_widget[QUICKSLOT_ID].ht > 34)
    {
        qsx = 1;
        qsy = 0;
        xoff = 0;
    }
    else
    {
        qsx = 0;
        qsy = 1;
        xoff= -17;
    }

    for (i = 0; i < MAX_QUICK_SLOTS; i++)
    {
        if (x >= cur_widget[QUICKSLOT_ID].x1 + quickslots_pos[i][qsx]+xoff
                && x <= cur_widget[QUICKSLOT_ID].x1 + quickslots_pos[i][qsx]+xoff + 32
                && y >= cur_widget[QUICKSLOT_ID].y1 + quickslots_pos[i][qsy]
                && y <= cur_widget[QUICKSLOT_ID].y1 + quickslots_pos[i][qsy] + 32)
            return i;
    }
    return -1;
}
void show_quickslots(int x, int y)
{
    int     i, mx, my;
    char    buf[512];
    int     qsx, qsy, xoff;

        qsx = 0;
        qsy = 1;
        xoff = -17;
        sprite_blt(Bitmaps[BITMAP_QUICKSLOTS], x, y, NULL, NULL);

    SDL_GetMouseState(&mx, &my);
    update_quickslots(-1);

    for (i = MAX_QUICK_SLOTS - 1; i >= 0; i--)
    {
        if (quick_slots[i].shared.tag != -1)
        {
            /* spell in quickslot */
            if (quick_slots[i].shared.is_spell == TRUE)
            {
                sprite_blt(spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].icon,
                           x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy], NULL, NULL);
                if (mx >= x + quickslots_pos[i][qsx]+xoff
                        && mx < x + quickslots_pos[i][qsx]+xoff + 33
                        && my >= y + quickslots_pos[i][qsy]
                        && my < y + quickslots_pos[i][qsy] + 33
                        && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    show_tooltip(mx, my,
                                 spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].name);
            }
            /* item in quickslot */
            else
            {
                item   *tmp;
                tmp = locate_item_from_item(cpl.ob, quick_slots[i].shared.tag);
                if (tmp)
                {
                    blt_inv_item(tmp, x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy]);
                    /* show tooltip */
                    if (mx >= x + quickslots_pos[i][qsx]+xoff
                            && mx < x + quickslots_pos[i][qsx]+xoff + 33
                            && my >= y + quickslots_pos[i][qsy]
                            && my < y + quickslots_pos[i][qsy] + 33
                            && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    {
                        sprintf(buf,"%s (q/c: %d/%d)",tmp->s_name, tmp->item_qua, tmp->item_con);
                        show_tooltip(mx, my, buf);
                    }
                }
            }
        }
        sprintf(buf, "F%d", i + 1);
        StringBlt(ScreenSurface, &Font6x3Out, buf, x + quickslots_pos[i][qsx]+xoff + 12, y + quickslots_pos[i][qsy] - 6,
                  COLOR_DEFAULT, NULL, NULL);
    }
}
void widget_quickslots(int x, int y)
{
    int     i, mx, my;
    char    buf[512];
    int     qsx, qsy, xoff;

    if (cur_widget[QUICKSLOT_ID].ht > 34)
    {
        qsx = 1;
        qsy = 0;
        xoff = 0;
        sprite_blt(Bitmaps[BITMAP_QUICKSLOTSV], x, y, NULL, NULL);
    }
    else
    {
        qsx = 0;
        qsy = 1;
        xoff = -17;
        sprite_blt(Bitmaps[BITMAP_QUICKSLOTS], x, y, NULL, NULL);
    }

    SDL_GetMouseState(&mx, &my);
    update_quickslots(-1);

    for (i = MAX_QUICK_SLOTS - 1; i >= 0; i--)
    {
        if (quick_slots[i].shared.tag != -1)
        {
            /* spell in quickslot */
            if (quick_slots[i].shared.is_spell == TRUE)
            {
                sprite_blt(spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].icon,
                           x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy], NULL, NULL);
                if (mx >= x + quickslots_pos[i][qsx]+xoff
                        && mx < x + quickslots_pos[i][qsx]+xoff + 33
                        && my >= y + quickslots_pos[i][qsy]
                        && my < y + quickslots_pos[i][qsy] + 33
                        && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    show_tooltip(mx, my,
                                 spell_list[quick_slots[i].spell.groupNr].entry[quick_slots[i].spell.classNr][quick_slots[i].shared.tag].name);
            }
            /* item in quickslot */
            else
            {
                item   *tmp;
                tmp = locate_item_from_item(cpl.ob, quick_slots[i].shared.tag);
                if (tmp)
                {
                    blt_inv_item(tmp, x + quickslots_pos[i][qsx]+xoff, y + quickslots_pos[i][qsy]);
                    /* show tooltip */
                    if (mx >= x + quickslots_pos[i][qsx]+xoff
                            && mx < x + quickslots_pos[i][qsx]+xoff + 33
                            && my >= y + quickslots_pos[i][qsy]
                            && my < y + quickslots_pos[i][qsy] + 33
                            && GetMouseState(&mx,&my,QUICKSLOT_ID))
                    {
                        sprintf(buf,"%s (QC: %d/%d)",tmp->s_name, tmp->item_qua, tmp->item_con);
                        show_tooltip(mx, my, buf);
                    }
                }
            }
        }
        sprintf(buf, "F%d", i + 1);
        StringBlt(ScreenSurface, &Font6x3Out, buf, x + quickslots_pos[i][qsx]+xoff + 12, y + quickslots_pos[i][qsy] - 6,
                  COLOR_DEFAULT, NULL, NULL);
    }
}
void widget_quickslots_mouse_event(int x, int y, int MEvent)
{
    if (MEvent==1) /* Mouseup Event */
    {
        if (draggingInvItem(DRAG_GET_STATUS) > DRAG_IWIN_BELOW)
        {
            int ind = get_quickslot(x, y);
            if (ind != -1) /* valid slot */
            {
                if (draggingInvItem(DRAG_GET_STATUS) == DRAG_QUICKSLOT_SPELL)
                {
                    quick_slots[ind].shared.is_spell = TRUE;
                    quick_slots[ind].spell.groupNr = quick_slots[cpl.win_quick_tag].spell.groupNr;
                    quick_slots[ind].spell.classNr = quick_slots[cpl.win_quick_tag].spell.classNr;
                    quick_slots[ind].shared.tag = quick_slots[cpl.win_quick_tag].spell.spellNr;
                    cpl.win_quick_tag = -1;
                }
                else
                {
                    if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_INV)
                        cpl.win_quick_tag = cpl.win_inv_tag;
                    else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_PDOLL)
                        cpl.win_quick_tag = cpl.win_pdoll_tag;
                    quick_slots[ind].shared.tag = cpl.win_quick_tag;
                    quick_slots[ind].item.invSlot = ind;
                    quick_slots[ind].shared.is_spell = FALSE;
                    /* now we do some tests... first, ensure this item can fit */
                    update_quickslots(-1);
                    /* now: if this is null, item is *not* in the main inventory
                                       * of the player - then we can't put it in quickbar!
                                       * Server will not allow apply of items in containers!
                                       */
                    if (!locate_item_from_inv(cpl.ob->inv, cpl.win_quick_tag))
                    {
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICKFAIL, 0, 0, 100);
                        draw_info("Only items from main inventory allowed in quickbar!", COLOR_WHITE);
                    }
                    else
                    {
                        char      buf[256];
                        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100); /* no bug - we 'get' it in quickslots */
                        sprintf(buf, "set F%d to %s", ind + 1, locate_item(cpl.win_quick_tag)->s_name);
                        draw_info(buf, COLOR_DGOLD);
                    }
                }
            }
            draggingInvItem(DRAG_NONE);
            itemExamined = 0; /* ready for next item */
        }
    }
    else /*Mousedown Event */
    {
        /* drag from quickslots */
        int   ind = get_quickslot(x, y);
        if (ind != -1 && quick_slots[ind].shared.tag != -1) /* valid slot */
        {
            cpl.win_quick_tag = quick_slots[ind].shared.tag;
            if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)))
            {
                if (quick_slots[ind].shared.is_spell == TRUE)
                {
                    draggingInvItem(DRAG_QUICKSLOT_SPELL);
                    quick_slots[ind].spell.spellNr = quick_slots[ind].shared.tag;
                    cpl.win_quick_tag = ind;
                }
                else
                {
                    draggingInvItem(DRAG_QUICKSLOT);
                }
                quick_slots[ind].shared.tag = -1;
            }
            else
            {
                int stemp = cpl.      inventory_win, itemp = cpl.win_inv_tag;
                cpl.inventory_win = IWIN_INV;
                cpl.win_inv_tag = quick_slots[ind].shared.tag;
                process_macro_keys(KEYFUNC_APPLY, 0);
                cpl.inventory_win = stemp;
                cpl.win_inv_tag = itemp;
            }
        }
        else if (x >= cur_widget[QUICKSLOT_ID].x1+266
                 && x <= cur_widget[QUICKSLOT_ID].x1 + 282
                 && y >= cur_widget[QUICKSLOT_ID].y1
                 && y <= cur_widget[QUICKSLOT_ID].y1 + 34
                 && (cur_widget[QUICKSLOT_ID].ht <= 34))
        {
            cur_widget[QUICKSLOT_ID].wd = 34;
            cur_widget[QUICKSLOT_ID].ht = 282;
            cur_widget[QUICKSLOT_ID].x1 +=266;
        }
        else if (x >= cur_widget[QUICKSLOT_ID].x1
                 && x <= cur_widget[QUICKSLOT_ID].x1 + 34
                 && y >= cur_widget[QUICKSLOT_ID].y1
                 && y <= cur_widget[QUICKSLOT_ID].y1 + 15
                 && (cur_widget[QUICKSLOT_ID].ht > 34))
        {
            cur_widget[QUICKSLOT_ID].wd = 282;
            cur_widget[QUICKSLOT_ID].ht = 34;
            cur_widget[QUICKSLOT_ID].x1 -=266;
        }
    }


    return;
}


void update_quickslots(int del_item)
{
    int i;

    for (i = 0; i < MAX_QUICK_SLOTS; i++)
    {
        if (quick_slots[i].shared.tag == del_item)
            quick_slots[i].shared.tag = -1;
        if (quick_slots[i].shared.tag == -1)
            continue;
        /* only items in the *main* inventory can used with quickslot! */
        if (quick_slots[i].shared.is_spell == FALSE)
        {
            if (!locate_item_from_inv(cpl.ob->inv, quick_slots[i].shared.tag))
                quick_slots[i].shared.tag = -1;
            if (quick_slots[i].shared.tag != -1)
                quick_slots[i].item.nr = locate_item_nr_from_tag(cpl.ob->inv, quick_slots[i].shared.tag);
        }
    }
}

static void freeQuickSlots(_quickslot *quickslots, int size)
{
    int i;

    for (i = 0; i != size; ++i)
    {
        if (quickslots[i].shared.is_spell == FALSE)
            free(quickslots[i].name.name);
    }
}

static int readNextQuickSlots(FILE *fp, char *server, int *port, char *name, _quickslot *quickslots)
{
    int     ch, i, r;

    for (ch = 1, i = 0; ch;)
    {
        if (i == 2048)
            return 0;
        ch = fgetc(fp);
        if (ch == EOF)
            return 0;
        server[i++] = ch;
    }
    if (!fread(port, sizeof(int), 1, fp))
        return 0;
    for (ch = 1, i = 0; ch;)
    {
        if (i == 40)
            return 0;
        ch = fgetc(fp);
        if (ch == EOF)
            return 0;
        name[i++] = ch;
    }
    for (i = r = 0; i != MAX_QUICK_SLOTS; ++i)
    {
        if (!fread(&quickslots[i].shared.is_spell, sizeof(Boolean), 1, fp))
        {
            freeQuickSlots(quickslots, i);
            return 0;
        }
        r += sizeof(Boolean);
        if (quickslots[i].shared.is_spell == FALSE)
        {
            int j;

            if (!fread(&quickslots[i].item.nr, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            r += sizeof(int);
            quickslots[i].name.name = (char *)malloc(sizeof(char) * 128);
            for (ch = 1, j = 0; ch; ++r)
            {
                if (j == 128)
                {
                    freeQuickSlots(quickslots, i + 1);
                    return 0;
                }
                ch = fgetc(fp);
                if (ch == EOF)
                {
                    freeQuickSlots(quickslots, i + 1);
                    return 0;
                }
                quickslots[i].name.name[j++] = ch;
            }
        }
        else
        {
            if (!fread(&quickslots[i].shared.tag, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            if (!fread(&quickslots[i].spell.groupNr, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            if (!fread(&quickslots[i].spell.classNr, sizeof(int), 1, fp))
            {
                freeQuickSlots(quickslots, i);
                return 0;
            }
            r += sizeof(int) * 3;
        }
    }
    return r;
}

/******************************************************************
 Restore quickslots from last game.
******************************************************************/
#define QUICKSLOT_FILE "settings/quick.dat"
#define QUICKSLOT_FILE_VERSION 2
#define QUICKSLOT_FILE_HEADER ((QUICKSLOT_FILE_VERSION << 24) | 0x53 << 16 | 0x51 << 8 | 0x44)
void load_quickslots_entrys()
{
    long        header;
    int         i, port;
    char        name[40], server[2048];
    _quickslot  quickslots[MAX_QUICK_SLOTS];
    FILE       *stream;

    if (!(stream = fopen_wrapper(QUICKSLOT_FILE, "rb")))
        return;
    fread(&header, sizeof(header), 1, stream);
    if (header != QUICKSLOT_FILE_HEADER)
    {
        fclose(stream);
        remove(file_path(QUICKSLOT_FILE, ""));
        return;
    }
    while (readNextQuickSlots(stream, server, &port, name, quickslots))
    {
        if (!strcmp(ServerName, server) && ServerPort == port)
        {
            Boolean cont = FALSE;

            port = strlen(cpl.name) + 1;
            for (i = 0; i != port; ++i)
            {
                if (tolower(cpl.name[i]) != tolower(name[i]))
                {
                    cont = TRUE;
                    break;
                }
            }

            if (cont == TRUE)
                continue;

            for (i = 0; i != MAX_QUICK_SLOTS; ++i)
            {
                if (quick_slots[i].shared.is_spell == FALSE)
                {
                    int      j;
                    Boolean  match = FALSE;
                    item    *ob = cpl.ob->inv;

                    for (j = 0; ob != NULL; ++j, ob = ob->next)
                    {
                        if (j == quick_slots[i].item.nr)
                        {
                            if (!strcmp(ob->s_name, quick_slots[i].name.name))
                            {
                                quick_slots[i].item.tag = ob->tag;
                                match = TRUE;
                            }
                            break;
                        }
                    }
                    if (match == FALSE)
                    {
                        for (ob = cpl.ob->inv; ob; ob = ob->next)
                        {
                            if (!strcmp(ob->s_name, quick_slots[i].name.name))
                            {
                                quick_slots[i].item.tag = ob->tag;
                                match = TRUE;
                                break;
                            }
                        }
                        if (match == FALSE)
                        {
                            cont = TRUE;
                            quick_slots[i].item.tag = -1;
                        }
                    }
                    free(quick_slots[i].name.name);
                }
                else
                {
                    memcpy(&quick_slots[i], &quickslots[i], sizeof(_quickslot));
                    if (quick_slots[i].shared.tag == -1)
                        cont = TRUE;
                }
                if (cont == TRUE)
                    continue;
                if (quick_slots[i].shared.is_spell == FALSE)
                    cpl.win_inv_slot = quick_slots[i].item.invSlot;
            }
            break;
        }
    }
    fclose(stream);
    update_quickslots(-1);
}

/******************************************************************
 Save the current quickslots.
******************************************************************/
void save_quickslots_entrys()
{
    long        header;
    char        name[40], server[2048];
    int         n, size, w;
    _quickslot  quickslots[MAX_QUICK_SLOTS];
    FILE       *stream;

    if (!(stream = fopen_wrapper(QUICKSLOT_FILE, "rb+")))
    {
        if (!(stream = fopen_wrapper(QUICKSLOT_FILE, "wb+")))
            return;
    }
    header = QUICKSLOT_FILE_HEADER;
    fwrite(&header, sizeof(header), 1, stream);
    for (n = w = 0; n != MAX_QUICK_SLOTS; ++n)
    {
        w += sizeof(Boolean);
        if (quick_slots[n].shared.is_spell == FALSE)
        {
            item *ob = locate_item_from_inv(cpl.ob->inv, quick_slots[n].item.tag);

            w += sizeof(int);
            quick_slots[n].name.name = (char *)malloc(sizeof(char) * 128);
            if (ob != NULL)
            {
                int i = strlen(ob->s_name) + 1;

                strncpy(quick_slots[n].name.name, ob->s_name, i);
                w += i;
            }
            else
                w += sizeof(char);
        }
        else
            w += sizeof(int) * 3;
    }
    /* readNextQuickSlots has problems with wb+ and rb+ opened files */
    n = ftell(stream);
    freopen(file_path(QUICKSLOT_FILE, "rb"),"rb",stream);
    fseek(stream,n,SEEK_SET);
    while ((size = readNextQuickSlots(stream, server, &n, name, quickslots)) != 0)
    {
        if (!strcmp(ServerName, server) && n == ServerPort && !strcmp(cpl.name, name))
        {
            if ((n = w - size) != 0)
            {
                char *buf;
                long  pos = ftell(stream);
                freopen(file_path(QUICKSLOT_FILE, "rb+"),"rb+",stream);
                fseek(stream, 0, SEEK_END);
                w = ftell(stream) - pos;
                buf = (char *)malloc(w);
                fseek(stream, pos, SEEK_SET);
                fread(buf, 1, w, stream);
                fseek(stream, pos + n, SEEK_SET);
                fwrite(buf, 1, w, stream);
                if (n < 0)
                {
                    w = ftell(stream);
                    rewind(stream);
                    buf = (char *)realloc(buf, w);
                    fread(buf, 1, w, stream);
                    freopen(file_path(QUICKSLOT_FILE, "wb+"), "wb+", stream);
                    fwrite(buf, 1, w, stream);
                }
                free(buf);
                fseek(stream, pos, SEEK_SET);
            }
            fseek(stream, -size, SEEK_CUR);
            for (n = 0; n != MAX_QUICK_SLOTS; ++n)
            {
                fwrite(&quick_slots[n].shared.is_spell, sizeof(Boolean), 1, stream);
                if (quick_slots[n].shared.is_spell == FALSE)
                {
                    fwrite(&quick_slots[n].item.nr, sizeof(int), 1, stream);
                    fwrite(quick_slots[n].name.name, sizeof(char), strlen(quick_slots[n].name.name) + 1, stream);
                }
                else
                {
                    fwrite(&quick_slots[n].shared.tag, sizeof(int), 1, stream);
                    fwrite(&quick_slots[n].spell.groupNr, sizeof(int), 1, stream);
                    fwrite(&quick_slots[n].spell.classNr, sizeof(int), 1, stream);
                }
            }
            fclose(stream);
            freeQuickSlots(quick_slots, MAX_QUICK_SLOTS);
            return;
        }
    }
    freopen(file_path(QUICKSLOT_FILE, "rb+"),"rb+",stream);
    fseek(stream, 0, SEEK_END);
    fwrite(&ServerName, sizeof(char), strlen(ServerName) + 1, stream);
    fwrite(&ServerPort, sizeof(int), 1, stream);
    fwrite(&cpl.name, sizeof(char), strlen(cpl.name) + 1, stream);
    for (n = 0; n != MAX_QUICK_SLOTS; ++n)
    {
        fwrite(&quick_slots[n].shared.is_spell, sizeof(Boolean), 1, stream);
        if (quick_slots[n].shared.is_spell == FALSE)
        {
            fwrite(&quick_slots[n].item.nr, sizeof(int), 1, stream);
            fwrite(quick_slots[n].name.name, sizeof(char), strlen(quick_slots[n].name.name) + 1, stream);
        }
        else
        {
            fwrite(&quick_slots[n].shared.tag, sizeof(int), 1, stream);
            fwrite(&quick_slots[n].spell.groupNr, sizeof(int), 1, stream);
            fwrite(&quick_slots[n].spell.classNr, sizeof(int), 1, stream);
        }
    }
    fclose(stream);
    freeQuickSlots(quick_slots, MAX_QUICK_SLOTS);
}
void widget_event_target(int x, int y, SDL_Event event)
{
    /* combat modus */
    if (y > cur_widget[TARGET_ID].y1+3 &&
        y < cur_widget[TARGET_ID].y1+38 &&
        x > cur_widget[TARGET_ID].x1+3 &&
        x < cur_widget[TARGET_ID].x1+30)
    {
        check_keys(SDLK_c);
    }
    /* talk button */
    if (y > cur_widget[TARGET_ID].y1 + 7 &&
        y < cur_widget[TARGET_ID].y1 + 25 &&
        x > cur_widget[TARGET_ID].x1 + 223 &&
        x < cur_widget[TARGET_ID].x1 + 259)
    {
        if (cpl.target_code)
            send_game_command("/talk hello");
    }
}

void widget_show_target(int x, int y)
{
    char       *ptr = NULL;
    SDL_Rect    box;
    double      temp;
    int         hp_tmp;

    sprite_blt(Bitmaps[BITMAP_TARGET_BG], x, y, NULL, NULL);

    sprite_blt(Bitmaps[cpl.target_mode ? BITMAP_TARGET_ATTACK : BITMAP_TARGET_NORMAL], x+5, y+4, NULL, NULL);

    sprite_blt(Bitmaps[BITMAP_TARGET_HP_B], x+4, y+24, NULL, NULL);

    /* redirect target_hp to our hp - server don't send it
     * because we should now our hp exactly
     */
    hp_tmp = (int) cpl.target_hp ;
    if (cpl.target_code == 0)
        hp_tmp = (int) (((float) cpl.stats.hp / (float) cpl.stats.maxhp) * 100.0f);

    if (cpl.target_code == 0)
    {
        if (cpl.target_mode)
            ptr = "target self (hold attack)";
        else
            ptr = "target self";
    }
    else if (cpl.target_code == 1)
    {
        if (cpl.target_mode)
            ptr = "target and attack enemy";
        else
            ptr = "target enemy";
    }
    else if (cpl.target_code == 2)
    {
        if (cpl.target_mode)
            ptr = "target friend (hold attack)";
        else
            ptr = "target friend";
    }
    if (cpl.target_code)
        sprite_blt(Bitmaps[BITMAP_TARGET_TALK], x + 223, y + 7, NULL, NULL);

    if (options.show_target_self || cpl.target_code != 0)
    {
        if (hp_tmp)
        {
            temp = (double) hp_tmp * 0.01;
            box.x = 0;
            box.y = 0;
            box.h = Bitmaps[BITMAP_TARGET_HP]->bitmap->h;
            box.w = (int) (Bitmaps[BITMAP_TARGET_HP]->bitmap->w * temp);
            if (!box.w)
                box.w = 1;
            if (box.w > Bitmaps[BITMAP_TARGET_HP]->bitmap->w)
                box.w = Bitmaps[BITMAP_TARGET_HP]->bitmap->w;
            sprite_blt(Bitmaps[BITMAP_TARGET_HP], x+5, y + 25, &box, NULL);
        }

        if (ptr)
        {
            /* BEGIN modified robed's HP-%-patch */
            /* Draw the name of the target */
             StringBlt(ScreenSurface, &SystemFont, cpl.target_name, x + 35, y+3, cpl.target_color, NULL, NULL);
            /* Either draw HP remaining percent and description... */
            if (hp_tmp)
            {
                char hp_text[9];
                int hp_color;
                int xhpoffset=0;
                sprintf((char *)hp_text, "HP: %d%%", hp_tmp);
                     if (hp_tmp > 90) hp_color = COLOR_GREEN;
                else if (hp_tmp > 75) hp_color = COLOR_DGOLD;
                else if (hp_tmp > 50) hp_color = COLOR_HGOLD;
                else if (hp_tmp > 25) hp_color = COLOR_ORANGE;
                else if (hp_tmp > 10) hp_color = COLOR_YELLOW;
                else                  hp_color = COLOR_RED;

               StringBlt(ScreenSurface, &SystemFont, hp_text, x + 35, y + 14, hp_color, NULL, NULL);
               xhpoffset=50;

                StringBlt(ScreenSurface, &SystemFont, ptr, x + 35 + xhpoffset, y + 14, cpl.target_color, NULL, NULL);
            }
            /* ...or draw just the description */
            else
                StringBlt(ScreenSurface, &SystemFont, ptr, x + 35, y + 14, cpl.target_color, NULL, NULL);
            /* END modified robed's HP-%-patch */
        }
    }
}


void reset_menu_status(void)
{
    if (cpl.menustatus != MENU_NO)
    {
        cpl.menustatus = MENU_NO;
    }

}

void reload_icons(void)
{
    int i, ii;
    char    buf[512];

    for (i = 0; i < SPELL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            if ((spell_list[i].entry[0][ii].flag != LIST_ENTRY_UNUSED) && (spell_list[i].entry[0][ii].icon_name))
            {
                if (spell_list[i].entry[0][ii].icon)
                    sprite_free_sprite(spell_list[i].entry[0][ii].icon);
               sprintf(buf,"%s%s",GetIconDirectory(),spell_list[i].entry[0][ii].icon_name);
               spell_list[i].entry[0][ii].icon=sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);
            }
            if ((spell_list[i].entry[1][ii].flag != LIST_ENTRY_UNUSED) && (spell_list[i].entry[1][ii].icon_name))
            {
                if (spell_list[i].entry[1][ii].icon)
                    sprite_free_sprite(spell_list[i].entry[1][ii].icon);
               sprintf(buf,"%s%s",GetIconDirectory(),spell_list[i].entry[1][ii].icon_name);
               spell_list[i].entry[1][ii].icon=sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);
            }
        }
    }

    for (i = 0; i < SKILL_LIST_MAX; i++)
    {
        for (ii = 0; ii < DIALOG_LIST_ENTRY; ii++)
        {
            if ((skill_list[i].entry[ii].flag != LIST_ENTRY_UNUSED) && (skill_list[i].entry[ii].icon_name))
            {
                if (skill_list[i].entry[ii].icon)
                    sprite_free_sprite(skill_list[i].entry[ii].icon);
               sprintf(buf,"%s%s",GetIconDirectory(),skill_list[i].entry[ii].icon_name);
               skill_list[i].entry[ii].icon=sprite_load_file(buf, SURFACE_FLAG_DISPLAYFORMAT);
            }
        }
    }

}


