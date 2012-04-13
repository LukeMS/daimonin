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

    The author can be reached via e-mail to info@daimonin.org
*/


/* Handles commands received by the server.
 *
 */

#include "include.h"

/* nast but tricky scroll helper */
static int  scrolldx = 0, scrolldy = 0;

/* Command function pointer list and main incoming command dispatcher
 * NOTE: the enum server_client_cmd list with
 * the SERVER_CMD_* are defined in protocol.h
 * which is a shared header between server and client.
 */

typedef void (*CmdProc) (char*, int);

struct CmdMapping
{
    void (*cmdproc)(char *, int);
};

/* the one and only sc command function pointer list */
struct CmdMapping commands[]  =
{
    /* Don't change this sorting! Its hardcoded in the server. */
    { PingCmd },
    { DrawInfoCmd },
    { AddMeFail },
    { Map2Cmd },
    { DrawInfoCmd2 },
    { ItemXCmd },
    { SoundCmd},
    { TargetObject },
    { UpdateItemCmd },
    { DeleteItem },
    { StatsCmd },
    { ImageCmd },
    { Face1Cmd},
    { SkillRdyCmd },
    { PlayerCmd },
    { SpelllistCmd },
    { SkilllistCmd },
    { GolemCmd },
    { AccNameSuccess },
    { SetupCmd},
    { DataCmd},
    { ItemYCmd },
    { GroupCmd },
    { GroupInviteCmd },
    { GroupUpdateCmd },
    { InterfaceCmd },
    { BookCmd },
    { MarkCmd },
    { AccountCmd },
#ifdef USE_CHANNELS
    { ChannelMsgCmd },
#endif
    //        { ModAnimCmd },
};

#define NCOMMANDS (sizeof(commands)/sizeof(struct CmdMapping))

/* process all sc commands */
void DoClient(void)
{
    command_buffer *cmd;

    /* Handle all enqueued commands */
    while ( (cmd = get_next_input_command()) ) /* function has mutex included */
    {
        /* LOG(LOG_MSG,"Command #%d (LT:%d)(len:%d)\n",cmd->data[0], LastTick, cmd->len); */
        if ((cmd->data[0]&~0x80) > (uint8) NCOMMANDS)
        {
            LOG(LOG_ERROR, "Bad command from server (%d)\n", cmd->data[0]);
        }
        else
        {
            int header_len = 3;
            _server_client_cmd cmd_tag = cmd->data[0];

            if( cmd_tag & 0x80)
            {
                cmd_tag &= ~0x80;
                header_len = 5;
            }
            commands[cmd_tag].cmdproc((char *)cmd->data+header_len, cmd->len-header_len);
        }
        command_buffer_free(cmd);
    }
}

/* Helper macros for incoming binary data.*/
#define GetSINT8_String(_d_)    (*((sint8 *)(_d_)) )
#define GetUINT8_String(_d_)    (*((uint8 *)(_d_)) )
#define GetSINT16_String(_d_)   ((sint16) adjust_endian_int16(*((uint16 *)(_d_))))
#define GetUINT16_String(_d_)   adjust_endian_int16(*((uint16 *)(_d_)))
#define GetSINT32_String(_d_)   (sint32) adjust_endian_int32(*((uint32 *)(_d_)))
#define GetUINT32_String(_d_)   adjust_endian_int32(*((uint32 *)(_d_)))

/**********************************************************************
 * SC COMMAND FUNCTIONS                                               *
 **********************************************************************/

void SoundCmd(char *data, int len)
{
    int x, y, num, type;

    if (len != 5)
    {
        LOG(LOG_ERROR, "Got invalid length on sound command: %d\n", len);
        return;
    }
    x = GetSINT8_String(data);
    y = GetSINT8_String(data+1);
    num = GetUINT16_String(data + 2);
    type = GetUINT8_String(data+4);

    calculate_map_sound(type, num, x, y, 0);
}

void PingCmd(char *data, int len)
{
    gameserver_t *node;

    for (node = gameserver_1st; node; node = node->next)
    {
        if (strcmp(node->address, csocket.host) ||
            node->port != csocket.port)
        {
            continue;
        }

        node->ping = (sint16)(SDL_GetTicks() - csocket.ping);

        /* If the server sent an empty string this means our current ping
         * string is up to date. Otherwise, update it from data. */
        if (len > 1)
        {
            node->ping_server = (uint32)strtoul(data, &data, 16);
            node->players = (sint16)strtol(data + 1, &data, 16);
            FREE(node->pingstring);
            MALLOC_STRING(node->pingstring, data + 1);
            locator_clear_players(node);
            gameserver_parse_pingstring(node);
        }

        return;
    }
}

void SetupCmd(char *buf, int len)
{
    int     s;
    char   *cmd, *param;

    scrolldy = scrolldx = 0;
    /* setup the endian syncronization */
    if(!setup_endian_sync(buf))
    {
        textwin_show_string(0, NDI_COLR_RED, "Corrupt endian template!");
        LOG(LOG_ERROR, "Corrupt endian template!\n");
        SOCKET_CloseSocket(csocket.fd);
        GameStatus = GAME_STATUS_START;
        return;
    }

    LOG(LOG_MSG, "Got SetupCmd:: %s\n", buf);
    for (s = 6; ;)
    {
        while (s < len && buf[s] == ' ')
            s++;
        if (s >= len)
            break;
        cmd = &buf[s];
        while (s < len && buf[s] != ' ')
            s++;
        if (s >= len)
             break;
        buf[s++] = 0;
        if (s >= len)
             break;
        while (s < len && buf[s] == ' ')
            s++;
        if (s >= len)
            break;
        param = &buf[s];
        while (s < len && buf[s] != ' ')
            s++;
        buf[s++] = 0;
        while (s < len && buf[s] == ' ')
            s++;

        if (!strcmp(cmd, "dv"))
        {
            char   *cp1 = NULL,
                   *cp2 = NULL,
                    tmpbuf[TINY_BUF];
            sint16  rel = 0,
                    maj = 0,
                    min = 0;

            if ((cp1 = strchr(param, '.')))
            {
                *cp1++ = '\0';
                rel = (sint16)strtol(param, NULL, 10);

                if ((cp2 = strchr(cp1, '.')))
                {
                    *cp2++ = '\0';
                    maj = (sint16)strtol(cp1, NULL, 10);
                    min = (sint16)strtol(cp2, NULL, 10);
                }
            }

            if (!cp1 ||
                !cp2)
            {
                sprintf(buf, "The server is broken!\nPlease report it and select a different one!");
                textwin_show_string(0, NDI_COLR_RED, buf);
                LOG(LOG_ERROR, "%s\n", buf);
                SOCKET_CloseSocket(csocket.fd);
                GameStatus = GAME_STATUS_START;

                return;
            }

            options.server_version_release = (uint32)rel;
            options.server_version_major = (uint32)maj;
            options.server_version_minor = (uint32)min;

            if (rel != DAI_VERSION_RELEASE ||
                (rel == DAI_VERSION_RELEASE &&
                 maj != DAI_VERSION_MAJOR))
            {
                textwin_show_string(0, NDI_COLR_RED, "Mismatched x.y versions (server: %u.%u, client: %u.%u)!",
                                   rel, maj, DAI_VERSION_RELEASE,
                                   DAI_VERSION_MAJOR);

                if (rel < DAI_VERSION_RELEASE ||
                    (rel == DAI_VERSION_RELEASE &&
                     maj < DAI_VERSION_MAJOR))
                {
                    sprintf(tmpbuf, "The server is outdated!\nSelect a different one!");
                }
                else
                {
                    sprintf(tmpbuf, "Your client is outdated!\nUpdate your client!");
                }

                LOG(LOG_ERROR, "%s\n", tmpbuf);
                textwin_show_string(0, NDI_COLR_RED, tmpbuf);
                SOCKET_CloseSocket(csocket.fd);
                GameStatus = GAME_STATUS_START;
                SDL_Delay(3250);

                return;
            }

            if (min != DAI_VERSION_MINOR)
            {
                textwin_show_string(0, NDI_COLR_ORANGE, "Mismatched z version (server: %u, client: %u)!",
                                   min, DAI_VERSION_MINOR);
                textwin_show_string(0, NDI_COLR_ORANGE, "You can still connect and play but you might encounter minor problems or new features may not work properly.");
            }
        }
        else if (!strcmp(cmd, "pv"))
        {
            uint32 pv = (uint32)strtoul(param, NULL, 10);

            options.server_protocol = pv;

            if (pv != PROTOCOL_VERSION)
            {
                char tmpbuf[TINY_BUF];

                textwin_show_string(0, NDI_COLR_RED, "Mismatched protocol versions (server: %u, client: %u)",
                                   pv, PROTOCOL_VERSION);

                if (pv < PROTOCOL_VERSION)
                    sprintf(tmpbuf, "The server is outdated!\nSelect a different one!");
                else
                    sprintf(tmpbuf, "Your client is outdated!\nUpdate your client!");

                LOG(LOG_ERROR, "%s\n", tmpbuf);
                textwin_show_string(0, NDI_COLR_RED, tmpbuf);
                SOCKET_CloseSocket(csocket.fd);
                GameStatus = GAME_STATUS_START;
                SDL_Delay(3250);

                return;
            }
        }
        else if (!strcmp(cmd, "ac"))
        {
        }
        else if (!strcmp(cmd, "fc"))
        {

        }
        else if (!strcmp(cmd, "skf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LOG(LOG_MSG, "Get skf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;
                        srvfile_set_status(SRV_CLIENT_SKILLS,
                                           SRVFILE_STATUS_UPDATE, atoi(param),
                                           strtoul(cp + 1, NULL, 16));

                        break;
                    }
                }
            }
            else
            {
                srvfile_set_status(SRV_CLIENT_SKILLS, SRVFILE_STATUS_OK, 0, 0);
            }
        }
        else if (!strcmp(cmd, "spf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LOG(LOG_MSG, "Get spf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;
                        srvfile_set_status(SRV_CLIENT_SPELLS,
                                           SRVFILE_STATUS_UPDATE, atoi(param),
                                           strtoul(cp + 1, NULL, 16));

                        break;
                    }
                }
            }
            else
            {
                srvfile_set_status(SRV_CLIENT_SPELLS, SRVFILE_STATUS_OK, 0, 0);
            }
        }
        else if (!strcmp(cmd, "stf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LOG(LOG_MSG, "Get stf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;
                        srvfile_set_status(SRV_CLIENT_SETTINGS,
                                           SRVFILE_STATUS_UPDATE, atoi(param),
                                           strtoul(cp + 1, NULL, 16));

                        break;
                    }
                }
            }
            else
            {
                srvfile_set_status(SRV_CLIENT_SETTINGS, SRVFILE_STATUS_OK, 0, 0);
            }
        }
        else if (!strcmp(cmd, "bpf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LOG(LOG_MSG, "Get bpf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;
                        srvfile_set_status(SRV_CLIENT_BMAPS,
                                           SRVFILE_STATUS_UPDATE, atoi(param),
                                           strtoul(cp + 1, NULL, 16));

                        break;
                    }
                }
            }
            else
            {
                srvfile_set_status(SRV_CLIENT_BMAPS, SRVFILE_STATUS_OK, 0, 0);
            }
        }
        else if (!strcmp(cmd, "amf"))
        {
            if (!strcmp(param, "FALSE"))
            {
                LOG(LOG_MSG, "Get amf:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;
                        srvfile_set_status(SRV_CLIENT_ANIMS,
                                           SRVFILE_STATUS_UPDATE, atoi(param),
                                           strtoul(cp + 1, NULL, 16));

                        break;
                    }
                }
            }
            else
            {
                srvfile_set_status(SRV_CLIENT_ANIMS, SRVFILE_STATUS_OK, 0, 0);
            }
        }
        else if (!strcmp(cmd, "sn")) /* sound */
        {
            if (!strcmp(param, "FALSE"))
            {
                LOG(LOG_MSG, "Get sn:: %s\n", param);
            }
            else if (strcmp(param, "OK"))
            {
                char   *cp;

                for (cp = param; *cp != 0; cp++)
                {
                    if (*cp == '|')
                    {
                        *cp = 0;
                        srvfile_set_status(SRV_CLIENT_SOUNDS,
                                           SRVFILE_STATUS_UPDATE, atoi(param),
                                           strtoul(cp + 1, NULL, 16));

                        break;
                    }
                }
            }
            else
            {
                srvfile_set_status(SRV_CLIENT_SOUNDS, SRVFILE_STATUS_OK, 0, 0);
            }
        }
        else if (!strcmp(cmd, "mz")) /* mapsize */
        {
        }
        else if (!strcmp(cmd,  "geo")) /* geolocation */
        {
        }
        else
        {
            LOG(LOG_ERROR, "Got setup for a command we don't understand: %s %s\n", cmd, param);
            sprintf(buf, "The server is outdated!\nSelect a different one!");
            textwin_show_string(0, NDI_COLR_RED, buf);
            LOG(LOG_ERROR, "%s\n", buf);
            SOCKET_CloseSocket(csocket.fd);
            GameStatus = GAME_STATUS_START;
            return;
        }
    }
    GameStatus = GAME_STATUS_REQUEST_FILES;
}

/* We only get here if the server believes we are caching images. */
/* We rely on the fact that the server will only send a face command for
 * a particular number once - at current time, we have no way of knowing
 * if we have already received a face for a particular number.
 */
/* I don't understand the use of the word cacheing here. Why it ever be useful
 * for the server to send metainfo about an image (which we already have in
 * FILE_SRV_FACEINFO) but no actual image data? Fortunately, the server never
 * actually sends this command anyway.
 * -- Smacky 20110515 */
void Face1Cmd(char *data, int len)
{
    data[len] = '\0';
    face_saveinfo(GetUINT16_String(data), GetUINT32_String(data + 2),
                  (char *)data + 6);
}

/* Handles when the server says we can't be added.
 */
void AddMeFail(char *data, int len)
{
    char buf[MAX_BUF];
    int msg = ADDME_MSG_DISCONNECT;

    LOG(LOG_MSG, "addme_failed received.\n");

    if(len)
        msg = GetUINT8_String(data);

    /* if the server really don't like us, close the connection.
     * Server had done it already.
     */
    if(msg == ADDME_MSG_DISCONNECT)
    {
        LOG(LOG_MSG, "Server rejected your action - closing socket.\n");
        SOCKET_CloseSocket(csocket.fd);
        SDL_Delay(1250);
        GameStatus = GAME_STATUS_INIT;
    }
    else
    {
        /* something is wrong... char is banned, not ready, we can't join
         * check the game status and decide where we can go on.
         * Do it right - the server remembers our action and will dicon/ban
         * us without any announce when we try something stupid.
         */
         if(msg == ADDME_MSG_OK) /* huch? should not happen! */
         {
             LOG(LOG_MSG, "Bug addme_failed received ADDME_MSG_OK?!\n");
         }

         /* something was going wrong when loading the file...
          * player should ask a GM/DM for help
          */
         textwin_show_string(0, NDI_COLR_ORANGE, "***PLAYER LOGIN FAILED***");
         if(msg == ADDME_MSG_OK || msg == ADDME_MSG_INTERNAL || msg == ADDME_MSG_CORRUPT)
         {
             sprintf(buf,"Can't load player file %s\n Error Code: %d\nCall a game master for help!\n", cpl.name, msg);
             textwin_show_string(0, NDI_COLR_ORANGE, buf);
         }
         else if(msg == ADDME_MSG_UNKNOWN)
         {
             textwin_show_string(0, NDI_COLR_ORANGE, "Unknown player name!");
         }
         else if(msg == ADDME_MSG_TAKEN)
         {
             textwin_show_string(0, NDI_COLR_ORANGE, "Name is already taken!\nChoose a different one.");
         }
         else if(msg == ADDME_MSG_ACCOUNT)
         {
             textwin_show_string(0, NDI_COLR_ORANGE, "Player owned by different account!");
         }
         else if(msg == ADDME_MSG_BANNED)
         {
             textwin_show_string(0, NDI_COLR_ORANGE, "Player is BANNED!");
         }
         else if (msg == ADDME_MSG_WRONGPWD)
         {
             textwin_show_string(0, NDI_COLR_ORANGE, "Player exists but is saved in B4 format!\nNow enter the password to reclaim this name.");

             if(GameStatus == GAME_STATUS_ACCOUNT_CHAR_CREATE_WAIT)
             {
                 reset_input_mode();
                 InputStringFlag = 1;
                 InputStringEndFlag = 0;
                 open_input_mode(17);
                 GameStatus = GAME_STATUS_ACCOUNT_CHAR_RECLAIM;
                 cpl.menustatus = MENU_NO;

                 return;
             }
         }
         else
         {
             textwin_show_string(0, NDI_COLR_ORANGE, "Player loading failed!");
         }
         SDL_Delay(1250);
         if(GameStatus == GAME_STATUS_ACCOUNT_CHAR_CREATE_WAIT)
         {
             reset_input_mode();
             cpl.name[0] = 0;
             InputStringFlag=1;
             InputStringEndFlag=0;
             open_input_mode(MAX_PLAYER_NAME);
             GameStatus = GAME_STATUS_ACCOUNT_CHAR_NAME;
             cpl.menustatus = MENU_NO;
         }
         else
             GameStatus = GAME_STATUS_ACCOUNT;
    }
    return;
}

void AccNameSuccess(char *data, int len)
{
    int num = ACCOUNT_STATUS_DISCONNECT;

    if(len)
        num = GetUINT8_String(data);

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
            /* we continue with the account creation */
            GameStatus = GAME_STATUS_LOGIN_NEW;
            /* but now we go to password input */
            LoginInputStep = LOGIN_STEP_PASS1;
            dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
            cpl.password[0] = 0;
            open_input_mode(MAX_ACCOUNT_PASSWORD);
        }
        else /* something is wrong, try again */
        {
            GameStatus = GAME_STATUS_LOGIN_NEW;
            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
            open_input_mode(MAX_ACCOUNT_PASSWORD);
        }
    }

    return;
}

void ImageCmd(char *data, int len)
{
    int          pnum = GetSINT32_String(data), // only uint16 is needed, save 2 byted
                 plen = GetSINT32_String(data + 4); // unsigned?
    widget_id_t wid;

    if (len < 8 ||
        (len - 8) < plen)
    {
        LOG(LOG_ERROR, "PixMapCmd: Lengths don't compare (%d,%d)\n",
            len - 8, plen);

        return;
    }

    face_save((uint16)pnum, (uint8 *)data + 8, (uint32)plen);
    map_udate_flag = 2;
    map_redraw_flag |= MAP_REDRAW_FLAG_NORMAL;

    for (wid = 0; wid < WIDGET_NROF; wid++)
    {
        WIDGET_REDRAW(wid) = 1;
    }
}


void SkillRdyCmd(char *data, int len)
{
    int i, ii;

    strcpy(cpl.skill_name, (const char *)data);
    WIDGET_REDRAW(WIDGET_SKEXP_ID) = 1;

    /* lets find the skill... and setup the shortcuts to the exp values*/
    for (ii = 0; ii < SKILL_LIST_MAX; ii++)
    {
        for (i = 0; i < DIALOG_LIST_ENTRY; i++)
        {
            /* we have a list entry */
            if (skill_list[ii].entry[i].flag == LIST_ENTRY_KNOWN)
            {
                /* and is it the one we searched for? */
                if (!strcmp(skill_list[ii].entry[i].name, cpl.skill_name))
                {
                    cpl.skill_g = ii;
                    cpl.skill_e = i;
                    return;
                }
            }
        }
    }
}

void DrawInfoCmd(char *data, int len)
{
    char *buf;

    MALLOC(buf, len);
    *(uint16 *)buf = (uint8)atoi(data);
    sprintf(buf + 2, "%s", data + 2);
    DrawInfoCmd2(buf, len);
    FREE(buf);
}

/* new draw command */
void DrawInfoCmd2(char *data, int len)
{
    uint16  mode = GetUINT16_String(data);
    uint32  flags = mode & NDI_MASK_FLAGS,
            colr = mode & NDI_MASK_COLRS;
    char   *tmp = NULL,
            buf[HUGE_BUF];
 
    /* TODO: We translate the 8-bit server colour to a 32/24-bit client colour.
     * This will be unnecessary in 0.11.0. */
    switch (colr)
    {
        case 1: 
            colr = NDI_COLR_ORANGE;

            break;

        case 2: 
            colr = NDI_COLR_TEAL;

            break;

        case 3: 
            colr = NDI_COLR_RED;

            break;

        case 4: 
            colr = NDI_COLR_LIME;

            break;

        case 5: 
            colr = NDI_COLR_AQUA;

            break;

        case 6: 
            colr = NDI_COLR_GREY;

            break;

        case 7: 
            colr = NDI_COLR_MAROON;

            break;

        case 8: 
            colr = NDI_COLR_PURPLE;

            break;

        case 9:
            colr = NDI_COLR_FUSCHIA;

            break;

        case 10:
            colr = NDI_COLR_YELLOW;

            break;

        case 255: 
            colr = NDI_COLR_BLACK;

            break;

        default:
            colr = NDI_COLR_WHITE;
    }

    data += 2;

    if ((len -= 2) >= 0)
    {
        if (len >= HUGE_BUF)
        {
            len = HUGE_BUF - 1;
        }

        strncpy(buf, data, len);
        buf[len] = '\0';

        if ((tmp = strchr(data, ' ')))
        {
            *tmp = '\0';
        }
    }
    else
    {
        buf[0] = '\0';
    }

    /* Do we have a VIM? */
    if ((flags & NDI_FLAG_VIM))
    {
        char  *vimbuf;
        uint8  i = 0;

        MALLOC_STRING(vimbuf, buf);

        /* Split the string at newlines, sending each substring as a separate
         * arbitrary VIM with suitable delay. */
        for (tmp = strtok(vimbuf, "\n"); tmp; tmp = strtok(NULL, "\n"))
        {
            strout_vim_add(VIM_MODE_ARBITRARY, 8, 8, tmp, colr, 3000, 450 * i++);
        }
    }

    /* TODO: This is a horrid compatibility hack. In 0.11.0 we will do this
     * properly. */
    if (options.combat_smackvatts)
    {
        static uint8 doit = 0;

        if (options.combat_smackvatts == 2)
        {
            if (strstr(buf, "Direct Hit!"))
            {
                doit = 1;
            }
        }
        else
        {
            doit = 1;
        }

        if (doit &&
            strstr(buf, " hits you ") &&
            (tmp = strstr(buf, "damage with ")))
        {
            uint8 i;

            for (i = 0; i < NROFATTACKS; i++)
            {
                if (!strncmp(tmp + 12, player_attackredraw[i].name,
                             strlen(player_attackredraw[i].name)))
                {
                    if (player_attackredraw[i].flag > MAP_REDRAW_FLAG_NO)
                    {
                        doit = 0;
                        map_udate_flag = 2;
                        map_redraw_flag |= player_attackredraw[i].flag;
                    }

                    break;
                }
            }
        }
    }

    textwin_show_string(flags, colr, "%s", buf);
}

void TargetObject(char *data, int len)
{
    if ((cpl.target_mode = *data++))
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_WEAPON_ATTACK, 0, 0, 100);
    }
    else
    {
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_WEAPON_HOLD, 0, 0, 100);
    }

    /* Translate target's colour to the skin's preference. */
    if (*data == 6)
    {
        cpl.target_colr = skin_prefs.target_grey;
    }
    else if (*data == 4)
    {
        cpl.target_colr = skin_prefs.target_green;
    }
    else if (*data == 5)
    {
        cpl.target_colr = skin_prefs.target_blue;
    }
    else if (*data == 8)
    {
        cpl.target_colr = skin_prefs.target_purple;
    }
    else if (*data == 3)
    {
        cpl.target_colr = skin_prefs.target_red;
    }
    else if (*data == 1)
    {
        cpl.target_colr = skin_prefs.target_orange;
    }
    else
    {
        cpl.target_colr = skin_prefs.target_yellow;
    }

    cpl.target_code = *++data;
    strcpy(cpl.target_name, (const char *)++data);
    WIDGET_REDRAW(WIDGET_TARGET_ID) = 1;
    map_udate_flag = 2;
    map_redraw_flag |= MAP_REDRAW_FLAG_NORMAL;
}

void StatsCmd(char *data, int len)
{
    int     i   = 0, x;
    int     c, temp;
    char   *tmp, *tmp2;

    while (i < len)
    {
        c = GetUINT8_String(data + i++);

        if (c >= CS_STAT_RES_START && c <= CS_STAT_RES_END)
        {
            cpl.stats.protection[c - CS_STAT_RES_START] = GetSINT8_String(data + i++);
            cpl.stats.protection_change = 1;
            WIDGET_REDRAW(WIDGET_RESIST_ID) = 1;
        }
        else
        {
            switch (c)
            {
                case CS_STAT_TARGET_HP:
                    cpl.target_hp = (int) * (data + i++);
                    break;
                case CS_STAT_REG_HP:
                    cpl.gen_hp = ((float) GetUINT16_String(data + i)) / 10.0f;
                    i += 2;
                    WIDGET_REDRAW(WIDGET_REGEN_ID) = 1;
                    break;
                case CS_STAT_REG_MANA:
                    cpl.gen_sp = ((float) GetUINT16_String(data + i)) / 10.0f;
                    i += 2;
                    WIDGET_REDRAW(WIDGET_REGEN_ID) = 1;
                    break;
                case CS_STAT_REG_GRACE:
                    cpl.gen_grace = ((float) GetUINT16_String(data + i)) / 10.0f;
                    i += 2;
                    WIDGET_REDRAW(WIDGET_REGEN_ID) = 1;
                    break;

                case CS_STAT_HP:
                    temp = GetSINT32_String(data + i);
                    if (temp < cpl.stats.hp)
                    {
                        cpl.warn_hp = 1;
                        if (cpl.stats.maxhp / 12 <= cpl.stats.hp - temp)
                            cpl.warn_hp = 2;
                    }
                    cpl.stats.temphp = temp-cpl.stats.hp;
                    cpl.stats.hptick=LastTick;
                    cpl.stats.hp = temp;
                    i += 4;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_MAXHP:

                    cpl.stats.maxhp = GetSINT32_String(data + i);
                    i += 4;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_SP:
                    temp = GetUINT16_String(data + i);
                    cpl.stats.tempsp = temp-cpl.stats.sp;
                    cpl.stats.sptick = LastTick;
                    cpl.stats.sp = temp;
                    i += 2;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_MAXSP:
                    cpl.stats.maxsp = GetUINT16_String(data + i);
                    i += 2;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_GRACE:
                    temp = GetUINT16_String(data + i);
                    cpl.stats.tempgrace = temp-cpl.stats.grace;
                    cpl.stats.gracetick = LastTick;
                    cpl.stats.grace = temp;
                    i += 2;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_MAXGRACE:
                    cpl.stats.maxgrace = GetUINT16_String(data + i);
                    i += 2;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_STR:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Str)
                        cpl.warn_statup = 1;
                    else
                        cpl.warn_statdown = 1;

                    cpl.stats.Str = temp;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_INT:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Int)
                        cpl.warn_statup = 1;
                    else
                        cpl.warn_statdown = 1;

                    cpl.stats.Int = temp;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_POW:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Pow)
                        cpl.warn_statup = 1;
                    else
                        cpl.warn_statdown = 1;

                    cpl.stats.Pow = temp;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;

                    break;
                case CS_STAT_WIS:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Wis)
                        cpl.warn_statup = 1;
                    else
                        cpl.warn_statdown = 1;

                    cpl.stats.Wis = temp;

                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_DEX:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Dex)
                        cpl.warn_statup = 1;
                    else
                        cpl.warn_statdown = 1;

                    cpl.stats.Dex = temp;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_CON:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Con)
                        cpl.warn_statup = 1;
                    else
                        cpl.warn_statdown = 1;

                    cpl.stats.Con = temp;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_CHA:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Cha)
                        cpl.warn_statup = 1;
                    else
                        cpl.warn_statdown = 1;

                    cpl.stats.Cha = temp;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_EXP:
                    temp = GetSINT32_String(data + i);

                    if (temp == 0 &&
                        cpl.stats.exp == 0)
                    {
                        textwin_show_string(0, NDI_COLR_WHITE,
                                           "\n|WHAT NOW?|\n"\
                                           "As this is your first time playing, you may be asking this question.\n"\
                                           "The character nearby is called ~Fanrir~. His job is to help new players get started in the game. You should talk to him. Do this by pressing the ~T~ key.\n"\
                                           "He will tell you how to do a lot of things and give you a lot of things to do, so pay attention to him and good luck! :)\n");
                    }

                    cpl.warn_exp_down = (temp < cpl.stats.exp) ? 1 : 0;
                    cpl.stats.exp = temp;
                    cpl.stats.exp_level = server_level.level; //we need to set it to max_level as default!!!
                    /* get the real level depending on the exp */
                    for (x=0;x<=server_level.level;x++)
                    {
                       if (server_level.exp[x]>(uint32)temp)
                        {
                            cpl.stats.exp_level = x-1;
                            break;
                        }
                    }
                    i += 4;
                    WIDGET_REDRAW(WIDGET_MALVL_ID) = 1;
                    break;
                case CS_STAT_LEVEL:
                    cpl.stats.level = (sint8) * (data + i++);
                    cpl.warn_drained = (cpl.stats.level < cpl.stats.exp_level) ? 2 : 0;
                    WIDGET_REDRAW(WIDGET_MALVL_ID) = 1;
                    break;
                case CS_STAT_WC:
                    cpl.stats.wc = GetUINT16_String(data + i);
                    i += 2;
                    break;
                case CS_STAT_AC:
                    cpl.stats.ac = GetUINT16_String(data + i);
                    i += 2;
                    break;
                case CS_STAT_DAM:
                    cpl.stats.dam = GetUINT16_String(data + i);
                    cpl.stats.dps = (float)cpl.stats.dam/10.0f;
                    i += 2;
                    break;
                case CS_STAT_DIST_DPS:
                    cpl.stats.dist_dam = GetSINT16_String(data + i);
                    cpl.stats.dist_dps = (float)cpl.stats.dist_dam/10.0f;
                    i += 2;
                    break;
                case CS_STAT_DIST_WC:
                    cpl.stats.dist_wc = GetSINT16_String(data + i);
                    i += 2;
                    break;
                case CS_STAT_DIST_TIME:
                    cpl.stats.dist_time = ((float)GetUINT32_String(data + i))/1000.0f;
                    i += 4;
                    break;
                case CS_STAT_SPEED:
                    cpl.stats.speed = (float)GetUINT32_String(data + i)/10.0f;
                    i += 4;
                    break;
                case CS_STAT_SPELL_FUMBLE:
                    cpl.stats.spell_fumble = (float)GetSINT32_String(data + i)/10.0f;
                    i += 4;
                    break;
                case CS_STAT_FOOD:
                    cpl.stats.food = GetSINT16_String(data + i);
                    i += 2;
                    WIDGET_REDRAW(WIDGET_STATS_ID) = 1;
                    break;
                case CS_STAT_WEAP_SP:
                    cpl.stats.weapon_sp = ((float)GetUINT32_String(data + i))/1000.0f;
                    i += 4;
                    break;
                case CS_STAT_FLAGS:
                    cpl.stats.flags = GetUINT16_String(data + i);
                    i += 2;
                    break;
                case CS_STAT_WEIGHT_LIM:
                    set_weight_limit(GetUINT32_String(data + i));
                    i += 4;
                    break;
                case CS_STAT_ACTION_TIME:
                    cpl.action_time_max = ((float)ABS(GetSINT32_String(data + i))) / 1000.0f;
                    cpl.action_timer = cpl.action_time_max;
                    /* If the actiontimer has expired, make a noise to indicate it.
                     * But only if the player chooses that option. */
                    if (cpl.action_timer == 0.00f)
                        if (options.use_timer_sound == 1)
                            sound_play_effect(SOUNDTYPE_CLIENT, SOUND_CLICK, 0, 0, 100);
                    i += 4;
                    WIDGET_REDRAW(WIDGET_SKEXP_ID) = 1;
                    break;
                case CS_STAT_SKILLEXP_AGILITY:
                case CS_STAT_SKILLEXP_PERSONAL:
                case CS_STAT_SKILLEXP_MENTAL:
                case CS_STAT_SKILLEXP_PHYSIQUE:
                case CS_STAT_SKILLEXP_MAGIC:
                case CS_STAT_SKILLEXP_WISDOM:
                    cpl.stats.skill_exp[(c - CS_STAT_SKILLEXP_START) / 2] = GetUINT32_String(data + i);
                    i += 4;
                    WIDGET_REDRAW(WIDGET_SKLVL_ID) = 1;
                    break;
                case CS_STAT_SKILLEXP_AGLEVEL:
                case CS_STAT_SKILLEXP_PELEVEL:
                case CS_STAT_SKILLEXP_MELEVEL:
                case CS_STAT_SKILLEXP_PHLEVEL:
                case CS_STAT_SKILLEXP_MALEVEL:
                case CS_STAT_SKILLEXP_WILEVEL:
                    cpl.stats.skill_level[(c - CS_STAT_SKILLEXP_START - 1) / 2] = (sint16) * (data + i++);
                    WIDGET_REDRAW(WIDGET_SKLVL_ID) = 1;
                    break;
                case CS_STAT_RANGE:
                {
                    int   rlen    = (uint8) data[i++];
                    strncpy(cpl.range, (const char *) data + i, rlen);
                    cpl.range[rlen] = '\0';
                    i += rlen;
                    break;
                }

                case CS_STAT_EXT_TITLE:
                {
                    int   rlen    = (uint8) data[i++];

                    tmp = strchr((char *)data + i, '\n');
                    *tmp = 0;
                    strcpy(cpl.rank, (const char *)data + i);
                    tmp2 = strchr(tmp + 1, '\n');
                    *tmp2 = 0;
                    strcpy(cpl.pname, tmp + 1);
                    tmp = strchr(tmp2 + 1, '\n');
                    *tmp = 0;
                    strcpy(cpl.race, tmp2 + 1);
                    tmp2 = strchr(tmp + 1, '\n');
                    *tmp2 = 0;
                    strcpy(cpl.title, tmp + 1); /* profession title */
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

                    /* prepare rank + name for fast access
                                               * the pname is <name> <title>.
                                               * is there no title, there is still
                                               * always a ' ' at the end - we skip this
                                               * here!
                                               */
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
                    LOG(LOG_MSG, "Command get stats: CS_STAT_TITLE is outdated\n");
                }
                break;
                default:
                    fprintf(stderr, "Unknown stat number %d\n", c);
            }
        }
    }
    if (i > len)
    {
        fprintf(stderr, "got stats overflow, processed %d bytes out of %d\n", i, len);
    }
}


/* The one and only character/player init function.
 * If we get this, the server added us to the player list
 * and kicked us to the map.
 * Set the client in playing mode and wait for the incoming
 * regular server data.
 */
/* FIXME: Save some bytes here in 0.11.0. */
void PlayerCmd(char *data, int len)
{
    char    name[MAX_BUF];
    int     tag, weight, face, i = 0, nlen;
    textwin_id_t twid;

    options.firststart = 0;
    GameStatus = GAME_STATUS_PLAY;
    InputStringEndFlag = 0;
    tag = GetSINT32_String(data);
    i += 4;
    weight = GetSINT32_String(data + i);
    i += 4;
    face = GetSINT32_String(data + i);
    face_get(face);
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
    map_overlay(skin_sprites[SKIN_SPRITE_BLACKTILE]);
    map_transfer_flag = 1;
    map_udate_flag = 2;
    map_redraw_flag |= MAP_REDRAW_FLAG_NORMAL;
    widget_load();

    for (twid = 0; twid < TEXTWIN_NROF; twid++)
    {
        WIDGET_REDRAW(textwin[twid].wid) = 1;
        textwin_calculate_dimensions(&textwin[twid]);
    }

    ignore_list_load();
    chatfilter_list_load();
    kill_list_load();
    buddy_list_load();
    LOG(LOG_MSG, "Loading quickslot settings for server\n");
    load_quickslots_entrys();
    save_options_dat();
}

/* no item command, including the delinv... */
/* this is a bit hacked now - perhaps we should consider
 * in the future a new designed item command.
 */
/* FIXME: Save some bytes here in 0.11.0. */
void ItemXYCmd(char *data, int len, int bflag)
{
    int     weight, loc, tag, face, flags, pos = 0, nlen, anim, nrof, dmode;
    uint8   itype, stype, item_qua, item_con, item_skill, item_level;
    uint8   animspeed, direction = 0;
    char    name[MAX_BUF];

    map_udate_flag = 2;
    itype = stype = item_qua = item_con = item_skill = item_level = 0;

    dmode = GetSINT32_String(data);
    pos += 4;

    /*LOG(LOG_DEBUG,"ITEMXY:(%d) %s\n", dmode, locate_item(dmode)?(locate_item(dmode)->d_name?locate_item(dmode)->s_name:"no name"):"no LOC");*/

    loc = GetSINT32_String(data + pos);

    if (dmode >= 0)
        remove_item_inventory(locate_item(loc));

    if (dmode == -4) /* send item flag */
    {
        if (loc == cpl.container_tag)
            loc = -1; /* and redirect it to our invisible sack */
    }
    else if (dmode == -1)   /* container flag! */
    {
        cpl.container_tag = loc; /* we catch the REAL container tag */
        remove_item_inventory(locate_item(-1));

        if (loc == -1) /* if this happens, we want close the container */
        {
            cpl.container_tag = -998;
            return;
        }

        loc = -1; /* and redirect it to our invisible sack */
    }


    pos += 4;

    if (pos == len && loc != -1)
    {
        /* server sends no clean command to clear below window */
        /*LOG(LOG_ERROR, "ItemCmd: Got location with no other data\n");*/
    }
    else
    {
        while (pos < len)
        {
            tag = GetSINT32_String(data + pos); pos += 4;
            flags = GetSINT32_String(data + pos); pos += 4;
            weight = GetSINT32_String(data + pos); pos += 4;
            face = GetSINT32_String(data + pos); pos += 4;
            face_get(face);
            direction = data[pos++];

            if (loc)
            {
                itype = data[pos++];
                stype = data[pos++];

                item_qua = data[pos++];
                item_con = data[pos++];
                item_level = data[pos++];
                item_skill = data[pos++];
            }
            nlen = ((unsigned char *)data)[pos++];
            memcpy(name, (char *) data + pos, nlen);
            pos += nlen;
            name[nlen] = '\0';
            anim = GetUINT16_String(data + pos); pos += 2;
            animspeed = data[pos++];
            nrof = GetSINT32_String(data + pos); pos += 4;
            update_item(tag, loc, name, weight, face, flags, anim, animspeed, nrof, itype, stype, item_qua, item_con,
                        item_skill, item_level, direction, bflag);
        }
        if (pos > len)
            LOG(LOG_ERROR, "ItemCmd: ERROR: Overread buffer: %d > %d\n", pos, len);
    }
    map_udate_flag = 2;
}

/* ItemXCmd is ItemCmd with sort order normal (add to end) */
void ItemXCmd(char *data, int len)
{
    ItemXYCmd(data, len, 0);
}

/* ItemYCmd is ItemCmd with sort order reversed (add to front) */
void ItemYCmd(char *data, int len)
{
    ItemXYCmd(data, len, 1);
}

void GroupCmd(char *data, int len)
{
    char    name[64], *tmp;
    int     hp, mhp, sp, msp, gr, mgr, level;

    /* len == 0, its a GroupCmd which means "no group" */
    clear_group();
    if (len)
    {
        global_group_status = GROUP_MEMBER;
        tmp = strchr((char *)data, '|');
        while (tmp)
        {
            tmp++;
            sscanf(tmp, "%s %d %d %d %d %d %d %d", name, &hp, &mhp, &sp, &msp, &gr, &mgr, &level);
            set_group(group_count, name, level, hp, mhp, sp, msp, gr, mgr);

            /*LOG(LOG_DEBUG, "GROUP: %s [(%x)]\n", tmp, tmp);*/
            group_count++;
            tmp = strchr(tmp, '|');
        }
    }
}

/* Someone want invite us to a group.
 */
void GroupInviteCmd(char *data, int len)
{
    if (global_group_status != GROUP_NO) /* bug */
        LOG(LOG_ERROR, "ERROR: Got group invite when g_status != GROUP_NO (%s).\n", data);
    else
    {
        global_group_status = GROUP_INVITE;
        strncpy(group_invite, (const char *)data, 30);
    }
}

/* Server confirms our mark request
 */
void MarkCmd(char *data, int len)
{

    cpl.mark_count = GetSINT32_String(data);
}

void GroupUpdateCmd(char *data, int len)
{
    char        *tmp;
    int         hp, mhp, sp, msp, gr, mgr, level, slot = 0;

    if (!len)
        return;

    tmp = strchr((char *)data, '|');
    while (tmp)
    {
        tmp++;
        sscanf(tmp, "%d %d %d %d %d %d %d %d", &slot, &hp, &mhp, &sp, &msp, &gr, &mgr, &level);
        set_group(slot, NULL, level, hp, mhp, sp, msp, gr, mgr);

        /*LOG(LOG_DEBUG, "UPDATE: %s :: %d %d %d %d %d %d %d %d\n", tmp, slot, level, hp, mhp, sp, msp, gr, mgr);*/
        tmp = strchr(tmp, '|');
    }
}

void BookCmd(char *data, int len)
{
    int mode;

    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_BOOK, 0, 0, 100);
    cpl.menustatus = MENU_BOOK;

    mode = *((int*)data);
    data+=4;

    gui_interface_book = load_book_interface(mode, (char *)data, len-4);
}

void InterfaceCmd(char *data, int len)
{
    uint8 mode;

    map_udate_flag = 2;
    mode = *((uint8 *)data++);
    len--;
    reset_keys();
    cpl.input_mode = INPUT_MODE_NO;
    gui_npc_reset();

    if (mode > GUI_NPC_MODE_NO &&
        mode < GUI_NPC_MODE_END &&
        len)
    {
        if (!gui_npc_create(mode, data, len, 0))
        {
            textwin_show_string(0, NDI_COLR_RED, "INVALID GUI CMD");
        }
    }
}

/* UpdateItemCmd updates some attributes of an item */
/* FIXME: Save some bytes here in 0.11.0. */
void UpdateItemCmd(char *data, int len)
{
    int     weight, loc, tag, face, sendflags, flags, pos = 0, nlen, anim, nrof, quality=254, condition=254;
    uint8   direction;
    char    name[MAX_BUF];
    item   *ip, *env = NULL;
    uint8   animspeed;

    map_udate_flag = 2;
    sendflags = GetUINT16_String(data);
    pos += 2;
    tag = GetSINT32_String(data + pos);
    pos += 4;
    ip = locate_item(tag);
    if (!ip)
    {
        return;
    }
    *name = '\0';
    loc = ip->env ? ip->env->tag : 0;
    /*LOG(LOG_DEBUG,"UPDATE: loc:%d tag:%d\n",loc, tag); */
    weight = ip->weight;
    face = ip->face;
    face_get(face);
    flags = ip->flagsval;
    anim = 0;
    animspeed = 0;
if (ip->anim)
{
    anim = ip->anim->animnum;
    animspeed = (uint8) ip->anim->speed;
}
    nrof = ip->nrof;
    direction = ip->direction;

    if (sendflags & UPD_LOCATION)
    {
        loc = GetSINT32_String(data + pos);
        env = locate_item(loc);
        if (!env)
            fprintf(stderr, "UpdateItemCmd: unknown object tag (%d) for new location\n", loc);
        pos += 4;
    }
    if (sendflags & UPD_FLAGS)
    {
        flags = GetSINT32_String(data + pos);
        pos += 4;
    }
    if (sendflags & UPD_WEIGHT)
    {
        weight = GetSINT32_String(data + pos);
        pos += 4;
    }
    if (sendflags & UPD_FACE)
    {
        face = GetSINT32_String(data + pos);
        face_get(face);
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
        fprintf(stderr, "UpdateItemCmd: Overread buffer: %d > %d\n", pos, len);
        return;
    }
    if (sendflags & UPD_ANIM)
    {
        anim = GetUINT16_String(data + pos);
        pos += 2;
    }
    if (sendflags & UPD_ANIMSPEED)
    {
        animspeed = data[pos++];
    }
    if (sendflags & UPD_NROF)
    {
        nrof = GetSINT32_String(data + pos);
        pos += 4;
    }
    if (sendflags & UPD_QUALITY)
    {
        quality = (int)(data[pos++]);
        condition = (int)(data[pos++]);
    }
    update_item(tag, loc, name, weight, face, flags, anim, animspeed, nrof, 254, 254, quality, condition, 254, 254, direction,
                0);
    map_udate_flag = 2;
}

void DeleteItem(char *data, int len)
{
    int pos = 0, tag;

    while (pos < len)
    {
        tag = GetSINT32_String(data); pos += 4;
        delete_item(tag);
    }
    if (pos > len)
        fprintf(stderr, "DeleteCmd: Overread buffer: %d > %d\n", pos, len);
    map_udate_flag = 2;
}

void DeleteInventory(char *data, int len)
{
    int tag;

    tag = atoi((const char *) data);
    if (tag < 0)
    {
        fprintf(stderr, "DeleteInventory: Invalid tag: %d\n", tag);
        return;
    }
    remove_item_inventory(locate_item(tag));
    map_udate_flag = 2;
}

void Map2Cmd(char *data, int len)
{
    static int     map_w=0, map_h=0,mx=0,my=0;
    static int      step = 0;
    int     mapstat, probe,pos = 0;
    int     xpos, ypos;
    char mapname[SMALL_BUF],
         music[SMALL_BUF];
    sint32  face;

    mapstat = (uint8) (data[pos++]);
    map_transfer_flag = 0;
    if (mapstat != MAP_UPDATE_CMD_SAME)
    {
        strcpy(mapname, (const char *)(data + pos));
        pos += strlen(mapname)+1;
        strcpy(music, (const char *)(data + pos));
        pos += strlen(music)+1;
        if (mapstat == MAP_UPDATE_CMD_NEW)
        {
            map_w = (uint8) (data[pos++]);
            map_h = (uint8) (data[pos++]);
            xpos = (uint8) (data[pos++]);
            ypos = (uint8) (data[pos++]);
            mx = xpos;
            my = ypos;
            remove_item_inventory(locate_item(0)); /* implicit clear below */
            InitMapData(map_w, map_h, xpos, ypos);
        }
        else
        {
            int xoff, yoff;
            mapstat = (sint8) (data[pos++]);
            xoff = (sint8) (data[pos++]);
            yoff = (sint8) (data[pos++]);
            xpos = (uint8) (data[pos++]);
            ypos = (uint8) (data[pos++]);
            mx = xpos;
            my = ypos;
            remove_item_inventory(locate_item(0)); /* implicit clear below */
            display_mapscroll(xoff, yoff);
        }
        UpdateMapName(mapname);
        UpdateMapMusic(music);
    }
    else
    {
        xpos = (uint8) (data[pos++]);
        ypos = (uint8) (data[pos++]);

        /* we have moved */
        if ((xpos - mx || ypos - my))
        {
            remove_item_inventory(locate_item(0)); /* implicit clear below */
            cpl.win_below_slot = 0;
            if (cpl.menustatus != MENU_NO)
                reset_menu_status();

            display_mapscroll(xpos - mx, ypos - my);

            if(++step%2)
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_STEP1,0,0,100);
            else
                    sound_play_effect(SOUNDTYPE_CLIENT, SOUND_STEP2,0,0,100);

        }

        mx = xpos;
        my = ypos;
    }

    MapData.posx = xpos; /* map windows is from range to +MAPWINSIZE_X */
    MapData.posy = ypos;
    /*LOG(LOG_DEBUG,"MAPPOS: x:%d y:%d\n",xpos,ypos);*/
    while (pos < len)
    {
        /* first, we get the mask flag - it decribes what we now get */
        uint16 mask = GetUINT16_String(data + pos),
               x = (mask >> 11) & 0x1f,
               y = (mask >> 6) & 0x1f,
               ext_flag = 0,
               xdata = 0;
        char   pname1[TINY_BUF] = "",
               pname2[TINY_BUF] = "",
               pname3[TINY_BUF] = "",
               pname4[TINY_BUF] = "";
        sint16 ext1 = -1,
               ext2 = -1,
               ext3 = -1,
               height_2 = 0,
               height_3 = 0,
               height_4 = 0;

        pos += 2;

        if (!(mask & 0x3f))
        {
            uint8 i;

            the_map.cells[x][y].darkness = 0;
            the_map.cells[x][y].fogofwar = 1;

            for (i = 0; i < MAXFACES; i++)
            {
                the_map.cells[x][y].pname[i][0] = '\0';

                /* A mob/player. */
                if  ((the_map.cells[x][y].faces[i] & 0x8000))
                {
                    the_map.cells[x][y].faces[i] = 0;
                    the_map.cells[x][y].ext[i] = 0;
                    the_map.cells[x][y].pos[i] = 0;
                    the_map.cells[x][y].probe[i] = 0;
                }
            }
        }
        else
        {
            the_map.cells[x][y].fogofwar = 0;
        }

        /* the ext flag defines special layer object assigned infos.
         * Like the Zzz for sleep, paralyze msg, etc.
         */
        if (mask & 0x20) /* catch the ext. flag... */
        {
            ext_flag = (uint8) (data[pos++]);

            if (ext_flag & 0x80) /* we have player names.... */
            {
                char    c;
                int     i, pname_flag = (uint8) (data[pos++]);


                if (pname_flag & 0x08) /* floor .... */
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname1[i++] = c;
                    };
                    pname1[i] = 0;
                }
                if (pname_flag & 0x04) /* fm.... */
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname2[i++] = c;
                    };
                    pname2[i] = 0;
                }
                if (pname_flag & 0x02) /* l1 .... */
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname3[i++] = c;
                    };
                    pname3[i] = 0;
                }
                if (pname_flag & 0x01) /* l2 .... */
                {
                    i = 0;
                    while ((c = (char) (data[pos++])))
                    {
                        pname4[i++] = c;
                    };
                    pname4[i] = 0;
                }

                /* the following is for future height use, but we will probably change things before we need this */
                if (pname_flag & 0x40)
                {
                    height_2 = GetSINT16_String(data + pos);
                    pos += 2;
                    c = (data[pos++]);
                }
                if (pname_flag & 0x20)
                {
                    height_3 = GetUINT16_String(data + pos);
                    pos += 2;
                    c = (data[pos++]);
                }
                if (pname_flag & 0x10)
                {
                    height_4 = GetUINT16_String(data + pos);
                    pos += 2;
                    c = (data[pos++]);
                }
            }

            /* These are the "damage tags". */
            if (ext_flag & 0x40)
            {
                uint8  dmg_flag = data[pos++];
                sint16 dmg0 = 0,
                       dmg1 = 0,
                       dmg2 = 0,
                       dmg3 = 0;
                char   dmg_buf[TINY_BUF];

                if ((dmg_flag & 0x8))
                {
                    dmg0 = GetUINT16_String(data + pos);
                    sprintf(dmg_buf, "%d", dmg0);
                    pos += 2;
                    strout_vim_add(VIM_MODE_KILL, x, y, dmg_buf, NDI_COLR_ORANGE,
                            1500, 0);
                }

                if ((dmg_flag & 0x4))
                {
                    dmg1 = GetSINT16_String(data + pos);
                    sprintf(dmg_buf, "%d", ABS(dmg1));
                    pos += 2;
                    strout_vim_add(VIM_MODE_DAMAGE_SELF, x, y, dmg_buf,
                            (dmg1 >= 0) ? NDI_COLR_RED : NDI_COLR_LIME, 1250, 0);
                }

                if ((dmg_flag & 0x2))
                {
                    dmg2 = GetSINT16_String(data + pos);
                    sprintf(dmg_buf, "%d", dmg2);
                    pos += 2;
                    strout_vim_add(VIM_MODE_DAMAGE_OTHER, x, y, dmg_buf,
                            NDI_COLR_YELLOW, 1000, 0);
                }

                if ((dmg_flag & 0x1))
                {
                    dmg3 = GetSINT16_String(data + pos);
                    sprintf(dmg_buf, "%d", dmg3);
                    pos += 2;
                    strout_vim_add(VIM_MODE_DAMAGE_OTHER, x, y, dmg_buf,
                            NDI_COLR_ORANGE, 1000, 0);
                }
//                LOG(LOG_DEBUG,"Damage: dmg_flag %x, (%d, %d, %d, %d)",dmg_flag, dmg0, dmg1, dmg2, dmg3);
            }

            if (ext_flag & 0x08)
            {
                probe = 0;
                ext3 = (int) (data[pos++]);
                if (ext3 & FFLAG_PROBE)
                {
                    probe = (int) (data[pos++]);
                }

                set_map_ext(x, y, 3, ext3, probe);
            }
            if (ext_flag & 0x10)
            {
                probe = 0;
                ext2 = (int) (data[pos++]);
                if (ext2 & FFLAG_PROBE)
                {
                    probe = (int) (data[pos++]);
                }
                set_map_ext(x, y, 2, ext2, probe);
            }
            if (ext_flag & 0x20)
            {
                probe = 0;
                ext1 = (int) (data[pos++]);
                if (ext1 & FFLAG_PROBE)
                {
                    probe = (int) (data[pos++]);
                }
                set_map_ext(x, y, 1, ext1, probe);
            }
        }

        if (mask & 0x10)
        {
            set_map_darkness(x, y, (uint8) (data[pos]));
            pos++;
        }

        /* at last, we get the layer faces.
         * a set ext_flag here marks this entry as face from a multi tile arch.
         * we got another byte then which all information we need to display
         * this face in the right way (position and shift offsets)
         */
        /* Layer 1 */
        if (mask & 0x8)
        {
            sint16  z_height = 0;
            face = GetUINT16_String(data + pos); pos += 2;
            face_get((face & ~0x8000));
            xdata = 0;
            /* incoming height for floor (this is a sint16 */
#ifdef USE_TILESTRETCHER
            z_height = GetUINT16_String(data + pos); pos += 2;
#endif
            set_map_face(x, y, 0, face, xdata, -1, pname1,z_height);

        }

        /* Layer 2 */
        if (mask & 0x4)
        {
            face = GetUINT16_String(data + pos); pos += 2;
            face_get((face & ~0x8000));
            xdata = 0;
            if (ext_flag & 0x04) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 1, face, xdata, ext1, pname2,height_2);
        }

        /* Layers 3, 4, 5, and 7 */
        if (mask & 0x2)
        {
            face = GetUINT16_String(data + pos); pos += 2;
            face_get((face & ~0x8000));
            /* LOG(LOG_DEBUG,"we got face: %x (%x) ->%s\n", face, face&~0x8000, face_list[face&~0x8000].name?face_list[face&~0x8000].name:"(null)" );*/
            xdata = 0;
            if (ext_flag & 0x02) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 2, face, xdata, ext2, pname3,height_3);
        }

        /* Layer 6 */
        if (mask & 0x1)
        {
            face = GetUINT16_String(data + pos); pos += 2;
            face_get((face & ~0x8000));
            /*LOG(LOG_DEBUG,"we got face2: %x (%x) ->%s\n", face, face&~0x8000, face_list[face&~0x8000].name?face_list[face&~0x8000].name:"(null)" );*/
            xdata = 0;
            if (ext_flag & 0x01) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 3, face, xdata, ext3, pname4,height_4);
        }
    } /* more tiles */
    map_udate_flag = 2;
    map_redraw_flag |= MAP_REDRAW_FLAG_NORMAL;
}

void SkilllistCmd(char *data, int len)
{
    char *tmp, *tmp2, *tmp3, *tmp4;
    int     l, e, i, ii, mode;
    char    name[256];

    /*LOG(LOG_DEBUG,"sklist: %s\n", data);*/

    /* we grap our mode */
    mode = atoi(data);

    /* now look for the members fo the list we have */
    for (; ;)
    {
        tmp = strchr(data, '/'); /* find start of a name */
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

        /*LOG(LOG_DEBUG,"sname (%d): >%s<\n", mode, name);*/
        tmp3 = strchr(name, '|');
        *tmp3 = 0;
        tmp4 = strchr(tmp3 + 1, '|');

        l = atoi(tmp3 + 1);
        e = atoi(tmp4 + 1);

        /* we have a name, the level and exp - now setup the list */
        for (ii = 0; ii < SKILL_LIST_MAX; ii++)
        {
            for (i = 0; i < DIALOG_LIST_ENTRY; i++)
            {
                /* we have a list entry */
                if (skill_list[ii].entry[i].flag != LIST_ENTRY_UNUSED)
                {
                    /* and it is the one we searched for? */
                    if (!strcmp(skill_list[ii].entry[i].name, name))
                    {
                        /*LOG(LOG_DEBUG,"skill found (%d): >%s< %d | %d\n", mode, name, l, e);*/
                        if (mode == SPLIST_MODE_REMOVE) /* remove? */
                            skill_list[ii].entry[i].flag = LIST_ENTRY_USED;
                        else
                        {
                            skill_list[ii].entry[i].flag = LIST_ENTRY_KNOWN;
                            skill_list[ii].entry[i].exp = e;
                            skill_list[ii].entry[i].exp_level = l;
                            WIDGET_REDRAW(WIDGET_SKEXP_ID) = 1;
                        }
                    }
                }
            }
        }
    }
}

void SpelllistCmd(char *data, int len)
{
    int     i, ii, mode;
    char   *tmp, *tmp2, name[256];

    /*LOG(LOG_DEBUG,"slist: <%s>\n", data);*/
    /* we grap our mode */
    mode = atoi(data);

    for (; ;)
    {
        tmp = strchr(data, '/'); /* find start of a name */
        if (!tmp)
            return;
        data = tmp + 1;

        tmp2 = strchr(data, '/');
        if (tmp2)
        {
            strncpy(name,data, tmp2 - data);
            name[tmp2 - data] = 0;
            data = tmp2;
        }
        else
            strcpy(name, data);

        /* we have a name - now check the spelllist file and set the entry
           to _KNOWN */

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
}

void GolemCmd(char *data, int len)
{
    int   mode,
          face;
    char *tmp;

    /*LOG(LOG_DEBUG,"golem: <%s>\n", data);*/
    /* we grap our mode */
    tmp = strchr(data, ' '); /* find start of a name */
    face = atoi(tmp + 1);
    face_get(face);
    tmp = strchr(tmp + 1, ' '); /* find start of a name */
    textwin_show_string(0, NDI_COLR_WHITE, "You %s control of %s.",
                       ((mode = atoi(data)) == GOLEM_CTR_RELEASE) ? "lose" :
                       "gain", tmp + 1);
}

/* server has send us a block of data...
 * lets check what we got
 */
void DataCmd(char *data, int len)
{
    uint8         data_type = (uint8)(*data++),
                  data_comp = (data_type & DATA_PACKED_CMD);
    /* warning! if the uncompressed size of a incoming compressed(!) file is larger
     * as this dest_len default setting, the file is cutted and
     * the rest skiped. Look at the zlib docu for more info.
     */
    unsigned long dest_len;
    unsigned char *dest = NULL;

    len--;

    if (data_comp)
    {
        dest_len = GetUINT32_String(data);
        MALLOC(dest, dest_len);
        len-=4;
        data+=4;
        LOG(LOG_DEBUG, "data cmd: compressed file(len:%d->destlen:%d)\n", len, dest_len);
    }

    switch (data_type & ~DATA_PACKED_CMD)
    {
        case DATA_CMD_SOUND_LIST:
            if (data_comp)
            {
                LOG(LOG_DEBUG, "data cmd: compressed sound list(len:%d)\n", len);
                uncompress(dest, &dest_len, (unsigned char *)data, len);
                data = (char *)dest;
                len = dest_len;
            }
            request_file_chain++;
            srvfile_save(FILE_SRV_SOUNDS, SRV_CLIENT_SOUNDS, (unsigned char *)data, len);
            break;
        case DATA_CMD_SKILL_LIST:
            /* this is a server send skill list */
            /* uncompress when needed and save it */
            if (data_comp)
            {
                LOG(LOG_DEBUG, "data cmd: compressed skill list(len:%d)\n", len);
                uncompress(dest, &dest_len, (unsigned char *)data, len);
                data = (char *)dest;
                len = dest_len;
            }
            request_file_chain++;
            srvfile_save(FILE_SRV_SKILLS, SRV_CLIENT_SKILLS, (unsigned char *)data, len);
            break;
        case DATA_CMD_SPELL_LIST:
            if (data_comp)
            {
                LOG(LOG_DEBUG, "data cmd: compressed spell list(len:%d)\n", len);
                uncompress(dest, &dest_len, (unsigned char *)data, len);
                data = (char *)dest;
                len = dest_len;
            }
            request_file_chain++;
            srvfile_save(FILE_SRV_SPELLS, SRV_CLIENT_SPELLS, (unsigned char *)data, len);
            break;
        case DATA_CMD_SETTINGS_LIST:
            if (data_comp)
            {
                LOG(LOG_DEBUG, "data cmd: compressed settings file(len:%d)\n", len);
                uncompress(dest, &dest_len, (unsigned char *)data, len);
                data = (char *)dest;
                len = dest_len;
            }
            request_file_chain++;
            srvfile_save(FILE_SRV_SETTINGS, SRV_CLIENT_SETTINGS, (unsigned char *)data, len);
            break;

        case DATA_CMD_BMAP_LIST:
            if (data_comp)
            {
                LOG(LOG_DEBUG, "data cmd: compressed faces file(len:%d)\n", len);
                uncompress(dest, &dest_len, (unsigned char *)data, len);
                data = (char *)dest;
                len = dest_len;
            }
            request_file_chain++;
            srvfile_save(FILE_SRV_FACEINFO, SRV_CLIENT_BMAPS, (unsigned char *)data, len);
            break;

        case DATA_CMD_ANIM_LIST:
            if (data_comp)
            {
                uncompress(dest, &dest_len, (unsigned char *)data, len);
                LOG(LOG_DEBUG, "data cmd: compressed anims file(len:%d) -> %d\n", len, dest_len);
                data = (char *)dest;
                len = dest_len;
            }
            request_file_chain++;
            srvfile_save(FILE_SRV_ANIMS, SRV_CLIENT_ANIMS, (unsigned char *)data, len);
            break;

        default:
            LOG(LOG_ERROR, "data cmd: unknown type %d (len:%d)\n", data_type, len);
            break;
    }
    if (dest!=NULL)
        FREE(dest);
}

#ifdef USE_CHANNELS

/* TODO: This cmd will be removed entirely in 0.11.0. Server will send
 * DRAWINFO2 with appropriate flags and colr. */
void ChannelMsgCmd(char *data, int len)
{
    uint8  mode = GetUINT8_String(data++);
    uint32 flags = NDI_FLAG_PLAYER | NDI_FLAG_CHANNEL,
           colr = GetUINT8_String(data++);

    /* TODO: We translate the 8-bit server colour to a 32/24-bit client colour.
     * This will be unnecessary in 0.11.0. */
    switch (colr)
    {
        case 1: 
            colr = NDI_COLR_ORANGE;

            break;

        case 2: 
            colr = NDI_COLR_TEAL;

            break;

        case 3: 
            colr = NDI_COLR_RED;

            break;

        case 4: 
            colr = NDI_COLR_LIME;

            break;

        case 5: 
            colr = NDI_COLR_AQUA;

            break;

        case 6: 
            colr = NDI_COLR_GREY;

            break;

        case 7: 
            colr = NDI_COLR_MAROON;

            break;

        case 8: 
            colr = NDI_COLR_PURPLE;

            break;

        case 9:
            colr = NDI_COLR_FUSCHIA;

            break;

        case 10:
            colr = NDI_COLR_YELLOW;

            break;

        case 255: 
            colr = NDI_COLR_BLACK;

            break;

        default:
            colr = NDI_COLR_WHITE;
    }

    if (mode == 1)
    {
        flags |= NDI_FLAG_EMOTE;
    }

    textwin_show_string(flags, colr, "%s", data);
}

#endif

/* Server is sending us our account data or the reason why not */
void AccountCmd(char *data, int len)
{
    int count = 0;
    int ac_status;

    /* First, get the account status - it tells us too when login failed */
    ac_status = GetSINT8_String(data+count++);
    if(ac_status) /* something is wrong when not ACCOUNT_STATUS_OK (0) */
    {
        textwin_show_string(0, NDI_COLR_RED, "Unknown Account: %s", cpl.acc_name);
        GameStatus = GAME_STATUS_LOGIN_ACCOUNT;
        LoginInputStep = LOGIN_STEP_NAME;
        dialog_login_warning_level = DIALOG_LOGIN_WARNING_ACCOUNT_UNKNOWN;
        cpl.acc_name[0] = 0;
        open_input_mode(MAX_ACCOUNT_NAME);

    }
    else /* we have account data... set it up and move player to account view mode */
    {
        memset(&account, 0 , sizeof(Account));
        while(count < len)
        {
            strcpy(account.name[account.count], data+count);
            count += strlen(account.name[account.count]) + 1;
            account.level[account.count] = GetSINT8_String(data+count++);
            account.race[account.count] = GetSINT8_String(data+count++);
            account.gender[account.count] = GetSINT8_String(data+count++);
            account.count++;
        }

        GameStatus = GAME_STATUS_ACCOUNT;
        LoginInputStep = LOGIN_STEP_NOTHING;
    }
}
