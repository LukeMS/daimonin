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


/* Handles commands received by the server.
 *
 */

#include <include.h>

/* nast but tricky scroll helper */
static int  scrolldx = 0, scrolldy = 0;


/* Command function pointer list and main incoming command dispatcher
 * NOTE: the enum server_client_cmd list with
 * the BINARY COMMAND TAGS for commands[] are in protocol.h
 * which is a shared header between server and client.
 * The protocol level cs commands are in client.c
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
    { AddMeSuccess },
    { SetupCmd},
    { handle_query},
    { DataCmd},
    { NewCharCmd},
    { ItemYCmd },
    { GroupCmd },
    { GroupInviteCmd },
    { GroupUpdateCmd },
    { InterfaceCmd },
    { BookCmd },
    { MarkCmd },
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
        if (!cmd->data[0] || (cmd->data[0]&~0x80) > NCOMMANDS)
            LOG(LOG_ERROR, "Bad command from server (%d)\n", cmd->data[0]);
        else
        {
            int header_len = 3;
            _server_client_cmd cmd_tag = cmd->data[0];

            if( cmd_tag & 0x80)
            {
                cmd_tag &= ~0x80;
                header_len = 5;
            }
            commands[cmd_tag - 1].cmdproc((char *)cmd->data+header_len, cmd->len-header_len);
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

    if (type == SOUND_SPELL)
    {
        if (num < 0 || num >= SPELL_SOUND_MAX)
        {
            LOG(LOG_ERROR, "Got invalid spell sound id: %d\n", num);
            return;
        }
        num += SOUND_MAX; /* this maps us to the spell sound table part */
    }
    else
    {
        if (num < 0 || num >= SOUND_MAX)
        {
            LOG(LOG_ERROR, "Got invalid sound id: %d\n", num);
            return;
        }
    }
    calculate_map_sound(num, x, y, 0);
}


void SetupCmd(char *buf, int len)
{
    int     s;
    char   *cmd, *param;

    scrolldy = scrolldx = 0;
    /* setup the endian syncronization */
    if(!setup_endian_sync(buf))
    {
        draw_info("Corrupt endian template!", COLOR_RED);
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

        if (!strcmp(cmd, "sc") || !strcmp(cmd, "cs"))
        {
            if( (!strcmp(cmd, "sc") && strcmp(param, VERSION_SC)) || 
                                (!strcmp(cmd, "cs") && strcmp(param, VERSION_CS)) )
            {
                char tmpbuf[MAX_BUF];

                sprintf(tmpbuf, "Invalid version %s: %s   (CS:%s,SC:%s)", cmd, param, VERSION_CS, VERSION_SC);
                draw_info(tmpbuf, COLOR_RED);
                if (!strcmp(cmd, "cs"))
                    sprintf(tmpbuf, "The server is outdated!\nSelect a different one!");
                else
                    sprintf(tmpbuf, "Your client is outdated!\nUpdate your client!");
                draw_info(tmpbuf, COLOR_RED);
                SOCKET_CloseSocket(csocket.fd);
                GameStatus = GAME_STATUS_START;
                LOG(LOG_ERROR, "%s\n", tmpbuf);
                SDL_Delay(3250);
                return;
            }
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
                LOG(LOG_MSG, "Get spf:: %s\n", param);
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
                LOG(LOG_MSG, "Get stf:: %s\n", param);
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
                LOG(LOG_MSG, "Get bpf:: %s\n", param);
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
                LOG(LOG_MSG, "Get amf:: %s\n", param);
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
        else if (!strcmp(cmd, "sn")) /* sound */
        {}
        else if (!strcmp(cmd, "mz")) /* mapsize */
        {}
        else
        {
            LOG(LOG_ERROR, "Got setup for a command we don't understand: %s %s\n", cmd, param);
            sprintf(buf, "The server is outdated!\nSelect a different one!");
            draw_info(buf, COLOR_RED);
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

void Face1Cmd(char *data, int len)
{
    int     pnum;
    uint32  checksum;
    char   *face;

    pnum = GetUINT16_String(data);
    checksum = GetUINT32_String(data + 2);
    face = (char *) data + 6;
    data[len] = '\0';

    finish_face_cmd(pnum, checksum, face);
}

/* Handles when the server says we can't be added.  In reality, we need to
 * close the connection and quit out, because the client is going to close
 * us down anyways.
 */
void AddMeFail(char *data, int len)
{
    LOG(LOG_MSG, "addme_failed received.\n");
    SOCKET_CloseSocket(csocket.fd);
    SDL_Delay(1250);
    GameStatus = GAME_STATUS_INIT;
    /*GameStatus = GAME_STATUS_START;*/
    /* add here error handling */
    return;
}

void AddMeSuccess(char *data, int len)
{
    LOG(LOG_MSG, "addme_success received.\n");
    return;
}

void ImageCmd(char *data, int len)
{
    int     pnum, plen;
    char    buf[2048];
    /*int fd,l; */
    FILE   *stream;

    pnum = GetSINT32_String(data);
    plen = GetSINT32_String(data + 4);
 
    if (len < 8 || (len - 8) < plen)
    {
        LOG(LOG_ERROR, "PixMapCmd: Lengths don't compare (%d,%d)\n", (len - 8), plen);
        return;
    }

    /* save picture to cache*/
    /* and load it to FaceList*/

    sprintf(buf, "%s%s", GetCacheDirectory(), FaceList[pnum].name);
    LOG(LOG_DEBUG, "ImageFromServer: %s\n", FaceList[pnum].name);
    if ((stream = fopen_wrapper(buf, "wb+")) != NULL)
    {
        fwrite((char *) data + 8, 1, plen, stream);
        fclose(stream);
    }
    FaceList[pnum].sprite = sprite_tryload_file(buf, 0, NULL);
    map_udate_flag = 2;
    map_redraw_flag = TRUE;
//    draw_info_format(COLOR_GREEN,"map_draw_update: ImageCmd");

}


void SkillRdyCmd(char *data, int len)
{
    int i, ii;

    strcpy(cpl.skill_name, (const char *)data);
    WIDGET_REDRAW(SKILL_EXP_ID);

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
    int     color   = atoi(data);
    char   *buf;

    buf = strchr(data, ' ');
    if (!buf)
    {
        LOG(LOG_ERROR, "DrawInfoCmd - got no data\n");
        buf = "";
    }
    else
        buf++;
    draw_info(buf, color);
}

/* new draw command */
void DrawInfoCmd2(char *data, int len)
{
    int     flags;
    char    *tmp=NULL, buf[2048];
    Boolean buddy=FALSE;

    flags = GetUINT16_String(data);
    data += 2;

    len -= 2;
    if (len >= 0)
    {
        if (len > 2000)
            len = 2000;
        strncpy(buf, data, len);
        buf[len] = 0;
    }
    else
        buf[0] = 0;

    if (buf[0])
    {

        tmp = strchr(data, ' ');
        if (tmp)
            *tmp = 0;
    }
    if (flags & NDI_VIM)
    {
        strncpy(vim.msg,buf,128);
        vim.msg[127]='\0';
        vim.starttick = LastTick;
        vim.active = TRUE;
    }
    /* we log even before we ignore, or cfilter */
    if (options.msglog>0)
    {
        if ((options.msglog==2) || (flags & (NDI_PLAYER|NDI_SAY|NDI_SHOUT|NDI_TELL|NDI_GSAY|NDI_EMOTE)))
            MSGLOG(buf);
    }
    /* we have communication input */
    if (tmp && flags & (NDI_PLAYER|NDI_SAY|NDI_SHOUT|NDI_TELL|NDI_GSAY|NDI_EMOTE))
    {
        if (!(flags & NDI_GM))
        {
            if ((flags & NDI_SAY) && (ignore_check(data,"say")))
                return;
            if ((flags & NDI_SHOUT) && (ignore_check(data,"shout")))
                return;
            if ((flags & NDI_TELL) && (ignore_check(data,"tell")))
                return;
            if ((flags & NDI_EMOTE) && (ignore_check(data,"emote")))
                return;
        }

        if (options.chatfilter)
            chatfilter_filter(buf); /* Filter incoming msg for f*words */

        if (buddy_check(data)) /* Color messages from buddys */
        {
            buddy=TRUE;
            flags = (flags & 0xff00) | 113;
        }
        else
            buddy=FALSE;

        if ((flags & NDI_SHOUT) && (options.shoutoff) && !buddy)
            return;

        /* save last incomming tell player for client sided /reply */
        if (flags & NDI_TELL)
            strcpy(cpl.player_reply, data);

        /*LOG(-1,"IGNORE?: player >%s<\n", data);*/
        if (flags & NDI_EMOTE)
            flags &= ~NDI_PLAYER;
    }
    if (options.smileys)
        smiley_convert(buf);
    draw_info(buf, flags);
}

void TargetObject(char *data, int len)
{
    cpl.target_mode = *data++;
    if (cpl.target_mode)
        sound_play_effect(SOUND_WEAPON_ATTACK, 0, 0, 100);
    else
        sound_play_effect(SOUND_WEAPON_HOLD, 0, 0, 100);
    cpl.target_color = *data++;
    cpl.target_code = *data++;
    strcpy(cpl.target_name, (const char *)data);
    map_udate_flag = 2;
    map_redraw_flag = TRUE;
//    draw_info_format(COLOR_GREEN,"map_draw_update: TargetObject\n");


    /*    sprintf(buf,"TO: %d %d >%s< (len: %d)\n",cpl.target_mode,cpl.target_code,cpl.target_name,len);
        draw_info(buf,COLOR_GREEN);*/

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
            WIDGET_REDRAW(RESIST_ID);
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
                    WIDGET_REDRAW(REGEN_ID);
                    break;
                case CS_STAT_REG_MANA:
                    cpl.gen_sp = ((float) GetUINT16_String(data + i)) / 10.0f;
                    i += 2;
                    WIDGET_REDRAW(REGEN_ID);
                    break;
                case CS_STAT_REG_GRACE:
                    cpl.gen_grace = ((float) GetUINT16_String(data + i)) / 10.0f;
                    i += 2;
                    WIDGET_REDRAW(REGEN_ID);
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
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_MAXHP:

                    cpl.stats.maxhp = GetSINT32_String(data + i);
                    i += 4;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_SP:
                    temp = GetUINT16_String(data + i);
                    cpl.stats.tempsp = temp-cpl.stats.sp;
                    cpl.stats.sptick = LastTick;
                    cpl.stats.sp = temp;
                    i += 2;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_MAXSP:
                    cpl.stats.maxsp = GetUINT16_String(data + i);
                    i += 2;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_GRACE:
                    temp = GetUINT16_String(data + i);
                    cpl.stats.tempgrace = temp-cpl.stats.grace;
                    cpl.stats.gracetick = LastTick;
                    cpl.stats.grace = temp;
                    i += 2;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_MAXGRACE:
                    cpl.stats.maxgrace = GetUINT16_String(data + i);
                    i += 2;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_STR:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Str)
                        cpl.warn_statup = TRUE;
                    else
                        cpl.warn_statdown = TRUE;

                    cpl.stats.Str = temp;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_INT:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Int)
                        cpl.warn_statup = TRUE;
                    else
                        cpl.warn_statdown = TRUE;

                    cpl.stats.Int = temp;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_POW:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Pow)
                        cpl.warn_statup = TRUE;
                    else
                        cpl.warn_statdown = TRUE;

                    cpl.stats.Pow = temp;
                    WIDGET_REDRAW(STATS_ID);

                    break;
                case CS_STAT_WIS:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Wis)
                        cpl.warn_statup = TRUE;
                    else
                        cpl.warn_statdown = TRUE;

                    cpl.stats.Wis = temp;

                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_DEX:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Dex)
                        cpl.warn_statup = TRUE;
                    else
                        cpl.warn_statdown = TRUE;

                    cpl.stats.Dex = temp;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_CON:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Con)
                        cpl.warn_statup = TRUE;
                    else
                        cpl.warn_statdown = TRUE;

                    cpl.stats.Con = temp;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_CHA:
                    temp = (int) * (data + i++);
                    if (temp >= cpl.stats.Cha)
                        cpl.warn_statup = TRUE;
                    else
                        cpl.warn_statdown = TRUE;

                    cpl.stats.Cha = temp;
                    WIDGET_REDRAW(STATS_ID);
                    break;
                case CS_STAT_EXP:
                    temp = GetSINT32_String(data + i);
                    if (temp < cpl.stats.exp)
                        cpl.warn_drain = TRUE;
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
                    WIDGET_REDRAW(MAIN_LVL_ID);
                    break;
                case CS_STAT_LEVEL:
                    cpl.stats.level = (sint8) * (data + i++);
                    if (cpl.stats.level != cpl.stats.exp_level)
                    {
                        cpl.warn_drain = TRUE;
                    }
                    WIDGET_REDRAW(MAIN_LVL_ID);
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
                    cpl.stats.dist_dam = GetUINT16_String(data + i);
                    cpl.stats.dist_dps = (float)cpl.stats.dist_dam/10.0f;
                    i += 2;
                    break;
                case CS_STAT_DIST_WC:
                    cpl.stats.dist_wc = GetUINT16_String(data + i);
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
                    cpl.stats.spell_fumble = (float)GetUINT32_String(data + i)/10.0f;
                    i += 4;
                    break;
                case CS_STAT_FOOD:
                    cpl.stats.food = GetUINT16_String(data + i);
                    i += 2;
                    WIDGET_REDRAW(STATS_ID);
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
                case CS_STAT_SKILLEXP_AGILITY:
                case CS_STAT_SKILLEXP_PERSONAL:
                case CS_STAT_SKILLEXP_MENTAL:
                case CS_STAT_SKILLEXP_PHYSIQUE:
                case CS_STAT_SKILLEXP_MAGIC:
                case CS_STAT_SKILLEXP_WISDOM:
                    cpl.stats.skill_exp[(c - CS_STAT_SKILLEXP_START) / 2] = GetUINT32_String(data + i);
                    i += 4;
                    WIDGET_REDRAW(SKILL_LVL_ID);
                    break;
                case CS_STAT_SKILLEXP_AGLEVEL:
                case CS_STAT_SKILLEXP_PELEVEL:
                case CS_STAT_SKILLEXP_MELEVEL:
                case CS_STAT_SKILLEXP_PHLEVEL:
                case CS_STAT_SKILLEXP_MALEVEL:
                case CS_STAT_SKILLEXP_WILEVEL:
                    cpl.stats.skill_level[(c - CS_STAT_SKILLEXP_START - 1) / 2] = (sint16) * (data + i++);
                    WIDGET_REDRAW(SKILL_LVL_ID);
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

void PreParseInfoStat(char *cmd)
{
    /* Find input name*/
    if(!cmd)
    {
        GameStatus = GAME_STATUS_START;
        LOG(LOG_MSG, "Bug: Invalid response from server\n");
        return;
    }
    if (!strncmp(cmd, "QN",2))
    {
        int status = cmd[2]-'0';

        LOG(LOG_MSG, "Login: Enter name - status %d\n", status);
        cpl.name[0] = 0;
        cpl.password[0] = 0;

        switch (status)
        {
            case 1:
                if (!GameStatusLogin)
                    dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
                else
                    dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_NO;
                break;
            case 2:
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_BLOCKED;
                break;
            case 3:
                if (GameStatusLogin)
                    dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
                else
                    dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_PLAYING;
                break;
            case 4:
                if (GameStatusLogin)
                    dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
                else
                    dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_TAKEN;
                break;
            case 5:
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_BANNED;
                break;
            case 6:
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NAME_WRONG;
                break;
            case 7:
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_PWD_WRONG;
                break;

            default: /* is also status 0 */
                dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
                break;
        }

        GameStatus = GAME_STATUS_NAME;
    }
    else if (!strncmp(cmd, "QP",2))
    {
        int status = cmd[2]-'0';

        dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
        LOG(LOG_MSG, "Login: Enter password\n");
        if (status != 0)
            dialog_login_warning_level = DIALOG_LOGIN_WARNING_PWD_WRONG;
        GameStatus = GAME_STATUS_PSWD;
    }
    else if (!strncmp(cmd, "QV",2))
    {
        LOG(LOG_MSG, "Login: Enter verify password\n");
        dialog_login_warning_level = DIALOG_LOGIN_WARNING_NONE;
        GameStatus = GAME_STATUS_VERIFYPSWD;
    }
    if (GameStatus == GAME_STATUS_NAME)
        open_input_mode(12);
    else if (GameStatus <= GAME_STATUS_VERIFYPSWD)
        open_input_mode(17);
}


void handle_query(char *data, int len)
{
    char   *buf;
    /*uint8 flags = atoi(data); ATM unused parameter*/

    buf = strchr(data, ' ');
    if (buf)
        buf++;

    /* one query string */
    LOG(LOG_MSG, "Received query string: %s\n", buf);
    PreParseInfoStat(buf);
}


/* This function copies relevant data from the archetype to the
 * object.  Only copies data that was not set in the object
 * structure.
 *
 */

void PlayerCmd(char *data, int len)
{
    char    name[MAX_BUF];
    int     tag, weight, face, i = 0, nlen;
    char filename[255];

    options.firststart = FALSE;
    GameStatus = GAME_STATUS_PLAY;
    txtwin[TW_MIX].size = txtwin_start_size;
    InputStringEndFlag = FALSE;
    tag = GetSINT32_String(data);
    i += 4;
    weight = GetSINT32_String(data + i);
    i += 4;
    face = GetSINT32_String(data + i);
    request_face(face);
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
    map_redraw_flag=TRUE;
//    draw_info_format(COLOR_GREEN,"map_draw_update: PlayerCmd");


    ignore_list_load();
    chatfilter_list_load();
    kill_list_load();
    buddy_list_load();
#if defined( __WIN_32)  || defined(__LINUX)
            sprintf(filename,"logs/%s.chat.log",cpl.name);
            LOG(LOG_DEBUG,"trying to open chatlogfile: %s\n",filename);
            if (!msglog) msglog = fopen_wrapper(filename, "a");
#endif
    LOG(LOG_MSG, "Loading quickslot settings for server\n");
    load_quickslots_entrys();
    save_options_dat();
}



/* no item command, including the delinv... */
/* this is a bit hacked now - perhaps we should consider
 * in the future a new designed item command.
 */
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

    /*LOG(-1,"ITEMXY:(%d) %s\n", dmode, locate_item(dmode)?(locate_item(dmode)->d_name?locate_item(dmode)->s_name:"no name"):"no LOC");*/

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
            request_face(face);
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
            nlen = data[pos++];
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
    ItemXYCmd(data, len, FALSE);
}

/* ItemYCmd is ItemCmd with sort order reversed (add to front) */
void ItemYCmd(char *data, int len)
{
    ItemXYCmd(data, len, TRUE);
}

void GroupCmd(char *data, int len)
{
    char    name[64], *tmp;
    int     hp, mhp, sp, msp, gr, mgr, level;

    /* len == 0, its a GroupCmd which means "no group" */
    clear_group();
    if (len)
    {
        /*sprintf(buf, "GROUP CMD: %s (%d)", data, len);
        draw_info(buf, COLOR_GREEN);*/

        global_group_status = GROUP_MEMBER;
        tmp = strchr((char *)data, '|');
        while (tmp)
        {
            tmp++;
            sscanf(tmp, "%s %d %d %d %d %d %d %d", name, &hp, &mhp, &sp, &msp, &gr, &mgr, &level);
            set_group(group_count, name, level, hp, mhp, sp, msp, gr, mgr);

            /*LOG(-1, "GROUP: %s [(%x)]\n", tmp, tmp);*/
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
    /* draw_info_format(COLOR_WHITE, "MARK: %d",cpl.mark_count); */
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

        /*LOG(-1, "UPDATE: %s :: %d %d %d %d %d %d %d %d\n", tmp, slot, level, hp, mhp, sp, msp, gr, mgr);*/
        tmp = strchr(tmp, '|');
    }
}

void BookCmd(char *data, int len)
{
    int mode;

    sound_play_effect(SOUND_BOOK, 0, 0, 100);
    cpl.menustatus = MENU_BOOK;

    mode = *((int*)data);
    data+=4;

    /*LOG(-1,"BOOK (%d): %s\n", mode, data);
    draw_info(data,COLOR_YELLOW);*/

    //gui_book_interface =
    gui_interface_book = load_book_interface(mode, (char *)data, len-4);

}

void InterfaceCmd(char *data, int len)
{

    map_udate_flag = 2;
    if ((gui_interface_npc && gui_interface_npc->status != GUI_INTERFACE_STATUS_WAIT) &&
            ((!len && cpl.menustatus == MENU_NPC) || (len && cpl.menustatus != MENU_NPC)))
    {
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
    }
    reset_keys();
    cpl.input_mode = INPUT_MODE_NO;
    reset_gui_interface();
    if (len)
    {
        int mode, pos = 0;

        mode = *data;
        pos ++;

#ifdef DEVELOPMENT
        LOG(LOG_DEBUG, "Interface command: %s\n", (char*)(data+pos));
#endif
        gui_interface_npc = load_gui_interface(mode, (char*)data, len, pos);
        if (!gui_interface_npc)
            draw_info("INVALID GUI CMD", COLOR_RED);
        else
        {
            gui_interface_npc->win_length = precalc_interface_npc();
            interface_mode = mode;
            cpl.menustatus = MENU_NPC;
            gui_interface_npc->startx = (Screensize.x/2)-(Bitmaps[BITMAP_NPC_INTERFACE]->bitmap->w / 2);
            gui_interface_npc->starty = (Screensize.y-600)/2+50;
            if (gui_interface_npc->icon_count > 0)
            {
                int i;
                for (i = 0; i <= gui_interface_npc->icon_count; i++)
                {
                    if (gui_interface_npc->icon[i].mode == 'S')
                    {
                        gui_interface_npc->selected = i+1;
                        break;
                    }
                }
            }
            if (gui_interface_npc->icon_count > 0)
            {
                int i;
                for (i = 0; i <= gui_interface_npc->icon_count; i++)
                {
                    if (gui_interface_npc->icon[i].mode == 'S')
                    {
                        gui_interface_npc->selected = i+1;
                        break;
                    }
                }
            }
            mb_clicked=0;
            /* Prefilled (and focused) textfield */
            if (gui_interface_npc->used_flag&GUI_INTERFACE_TEXTFIELD)
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
}

/* UpdateItemCmd updates some attributes of an item */
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
    /*LOG(-1,"UPDATE: loc:%d tag:%d\n",loc, tag); */
    weight = ip->weight;
    face = ip->face;
    request_face(face);
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
        request_face(face);
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
                FALSE);
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
    int     mask, x, y, pos = 0, ext_flag, xdata;
    int     mapstat, ext1, ext2, ext3, probe;
    int     map_new_flag    = FALSE;
    int     ff0, ff1, ff2, ff3, ff_flag, xpos, ypos;

    sint16 height_2, height_3, height_4;

    char    pname1[64], pname2[64], pname3[64], pname4[64];
    char mapname[256];
    uint16  face;

    mapstat = (uint8) (data[pos++]);
    map_transfer_flag = 0;
    if (mapstat != MAP_UPDATE_CMD_SAME)
    {
        strcpy(mapname, (const char *)(data + pos));
        pos += strlen(mapname)+1;
        if (mapstat == MAP_UPDATE_CMD_NEW)
        {
            /*
                    map_new_flag = TRUE;
            */
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
                    sound_play_effect(SOUND_STEP1,0,0,100);
            else
                    sound_play_effect(SOUND_STEP2,0,0,100);

        }

        mx = xpos;
        my = ypos;
    }

    if (map_new_flag)
    {
        adjust_map_cache(xpos, ypos);
    }

    MapData.posx = xpos; /* map windows is from range to +MAPWINSIZE_X */
    MapData.posy = ypos;
    /*LOG(-1,"MAPPOS: x:%d y:%d (nflag:%x)\n",xpos,ypos,map_new_flag);*/
    while (pos < len)
    {
        ext_flag = 0;
        ext1 = ext2 = ext3 = 0;
        height_2 = height_3 = height_4 = 0;
        /* first, we get the mask flag - it decribes what we now get */
        mask = GetUINT16_String(data + pos); pos += 2;
        x = (mask >> 11) & 0x1f;
        y = (mask >> 6) & 0x1f;

        /* these are the "damage tags" - shows damage an object got from somewhere.
         * ff_flag hold the layer info and how much we got here.
         * 0x08 means a damage comes from unknown or vanished source.
         * this means the object is destroyed.
         * the other flags are assigned to map layer.
         */
        if ((mask & 0x3f) == 0)
        {
            display_map_clearcell(x, y);
        }

        ext3 = ext2 = ext1 = -1;
        pname1[0] = 0;pname2[0] = 0;pname3[0] = 0;pname4[0] = 0;
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
            if (ext_flag & 0x40) /* damage add on the map */
            {
                ff0 = ff1 = ff2 = ff3 = -1;
                ff_flag = (uint8) (data[pos++]);
                if (ff_flag & 0x8)
                {
                    ff0 = GetUINT16_String(data + pos); pos += 2;
                    add_anim(ANIM_KILL, 0, 0, xpos + x, ypos + y, ff0);
                }
                if (ff_flag & 0x4)
                {
                    ff1 = GetUINT16_String(data + pos); pos += 2;
                    add_anim(ANIM_SELF_DAMAGE, 0, 0, options.mapstart_x+(int)((MAP_START_XOFF+20)*(options.zoom/100.0)), options.mapstart_y+(int)((146+50)*(options.zoom/100.0)), ff1);
                }
                if (ff_flag & 0x2)
                {
                    ff2 = GetUINT16_String(data + pos); pos += 2;
                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff2);
                }
                if (ff_flag & 0x1)
                {
                    ff3 = GetUINT16_String(data + pos); pos += 2;
                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff3);
                }
//                LOG(LOG_DEBUG,"Damage: ff_flag %x, (%d, %d, %d, %d)",ff_flag, ff0, ff1, ff2, ff3);
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
        if (mask & 0x8)
        {
            sint16  z_height = 0;
            face = GetUINT16_String(data + pos); pos += 2;
            request_face(face);
            xdata = 0;
            /* incoming height for floor (this is a sint16 */
#ifdef USE_TILESTRETCHER
            z_height = GetUINT16_String(data + pos); pos += 2;
#endif
            set_map_face(x, y, 0, face, xdata, -1, pname1,z_height);

        }
        if (mask & 0x4)
        {
            face = GetUINT16_String(data + pos); pos += 2;
            request_face(face);
            xdata = 0;
            if (ext_flag & 0x04) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 1, face, xdata, ext1, pname2,height_2);
        }
        if (mask & 0x2)
        {
            face = GetUINT16_String(data + pos); pos += 2;
            request_face(face);
            /* LOG(0,"we got face: %x (%x) ->%s\n", face, face&~0x8000, FaceList[face&~0x8000].name?FaceList[face&~0x8000].name:"(null)" );*/
            xdata = 0;
            if (ext_flag & 0x02) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 2, face, xdata, ext2, pname3,height_3);
        }
        if (mask & 0x1)
        {
            face = GetUINT16_String(data + pos); pos += 2;
            request_face(face);
            /*LOG(0,"we got face2: %x (%x) ->%s\n", face, face&~0x8000, FaceList[face&~0x8000].name?FaceList[face&~0x8000].name:"(null)" );*/
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
    map_redraw_flag = TRUE;
//    draw_info_format(COLOR_GREEN,"map_draw_update: Map2Cmd");

}

void SkilllistCmd(char *data, int len)
{
    char *tmp, *tmp2, *tmp3, *tmp4;
    int     l, e, i, ii, mode;
    char    name[256];

    /*LOG(-1,"sklist: %s\n", data);*/

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

        /*LOG(-1,"sname (%d): >%s<\n", mode, name);*/
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
                        /*LOG(-1,"skill found (%d): >%s< %d | %d\n", mode, name, l, e);*/
                        if (mode == SPLIST_MODE_REMOVE) /* remove? */
                            skill_list[ii].entry[i].flag = LIST_ENTRY_USED;
                        else
                        {
                            skill_list[ii].entry[i].flag = LIST_ENTRY_KNOWN;
                            skill_list[ii].entry[i].exp = e;
                            skill_list[ii].entry[i].exp_level = l;
                            WIDGET_REDRAW(SKILL_EXP_ID);
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
    int     mode, face;
    char   *tmp, buf[256];

    /*LOG(LOG_DEBUG,"golem: <%s>\n", data);*/
    /* we grap our mode */
    mode = atoi(data);
    if (mode == GOLEM_CTR_RELEASE)
    {
        tmp = strchr(data, ' '); /* find start of a name */
        face = atoi(tmp + 1);
        request_face(face);
        tmp = strchr(tmp + 1, ' '); /* find start of a name */
        sprintf(buf, "You lose control of %s.", tmp + 1);
        draw_info(buf, COLOR_WHITE);

    }
    else
    {
        tmp = strchr(data, ' '); /* find start of a name */
        face = atoi(tmp + 1);
        request_face(face);
        tmp = strchr(tmp + 1, ' '); /* find start of a name */
        sprintf(buf, "You get control of %s.", tmp + 1);
        draw_info(buf, COLOR_WHITE);
    }
}


static void save_data_cmd_file(char *path, unsigned char *data, int len)
{
    FILE   *stream;

    if ((stream = fopen_wrapper(path, "wb")) != NULL)
    {
        if (fwrite(data, sizeof(char), len, stream) != (size_t) len)
            LOG(LOG_ERROR, "save data cmd file : write() of %s failed. (len:%d)\n", path);
        fclose(stream);
    }
    else
        LOG(LOG_ERROR, "save data cmd file : Can't open %s for write. (len:%d)\n", path, len);
}

/* server has send us a block of data...
 * lets check what we got
 */
void DataCmd(char *data, int len)
{
    uint8   data_type   = (uint8) (*data);
    uint8   data_comp ;
    /* warning! if the uncompressed size of a incoming compressed(!) file is larger
     * as this dest_len default setting, the file is cutted and
     * the rest skiped. Look at the zlib docu for more info.
     */
    unsigned long  dest_len    = 512 * 1024;
    unsigned char *dest;

    dest = malloc(dest_len);
    data_comp = (data_type & DATA_PACKED_CMD);

    len--;
    data++;

    switch (data_type & ~DATA_PACKED_CMD)
    {
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
            save_data_cmd_file(FILE_CLIENT_SKILLS, (unsigned char *)data, len);
            read_skills();
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
            save_data_cmd_file(FILE_CLIENT_SPELLS, (unsigned char *)data, len);
            read_spells();
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
            save_data_cmd_file(FILE_CLIENT_SETTINGS, (unsigned char *)data, len);
            read_settings();
            break;

        case DATA_CMD_BMAP_LIST:
            if (data_comp)
            {
                LOG(LOG_DEBUG, "data cmd: compressed bmaps file(len:%d)\n", len);
                uncompress(dest, &dest_len, (unsigned char *)data, len);
                data = (char *)dest;
                len = dest_len;
            }
            request_file_chain++;
            save_data_cmd_file(FILE_CLIENT_BMAPS, (unsigned char *)data, len);
            request_file_flags |= SRV_CLIENT_FLAG_BMAP;
            read_bmaps();
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
            save_data_cmd_file(FILE_CLIENT_ANIMS, (unsigned char *)data, len);
            request_file_flags |= SRV_CLIENT_FLAG_ANIM;
            read_anims();
            break;

        default:
            LOG(LOG_ERROR, "data cmd: unknown type %d (len:%d)\n", data_type, len);
            break;
    }
    free(dest);
}

/* server tells us to go to the new char creation */
void NewCharCmd(char *data, int len)
{
    dialog_new_char_warn = 0;
    GameStatus = GAME_STATUS_NEW_CHAR;
}

#ifdef USE_CHANNELS

/**
* stringbreak for channelsystem, we have to include the prefix in normal msg
* and have to add spaces in emotes
* @param text message to break
* @param prefix prefix to add to the lines
* @param one_prefix emotes got only prefix in first line
* @param result breaked and prefixed string
*/
static inline void break_string(char *text, char *prefix, Boolean one_prefix, char *result)
{
    char buf[200];
    char pref[50];
    int  i, a, len;
    int winlen=244;
    int preflen, restlen;

    /*
    * TODO: some security checks for max string len's
    */

    /* we have to convert to smileys for correct string width calculation */
    if (options.smileys)
        smiley_convert(text);

    /* lets calculate the space used by the prefix */
    preflen=0;
    buf[0]=0;
    for (i=0;prefix[i]!=0;i++)
        preflen += SystemFont.c[(uint8) (prefix[i])].w + SystemFont.char_offset;

    restlen=winlen-preflen;
    result[0]=0;
    strcat(result,prefix);

    switch (options.channelformat)
    {
    case 0:
        if (one_prefix)
        {
            for (i=0;i<(preflen/2);i++)
                pref[i]=' ';
            pref[i+1]=0;
        }
        else strcpy(pref,prefix);
        break;

    case 1:
        one_prefix=TRUE;
        strcpy(pref,"       ");
        break;
    }

    /* lets do some codestealing from client's draw_info:) */
    len = 0;
    for (a = i = 0; ; i++)
    {
        len += SystemFont.c[(uint8) (text[i])].w + SystemFont.char_offset;
        if (len >= restlen || text[i] == 0x0a || text[i] == 0)
        {

            /* now the special part - lets look for a good point to cut */
            if (len >= restlen && a > 10)
            {
                int ii =a, it = i, ix = a, tx = i;

                while (ii >= a / 2)
                {
                    if (text[it] == ' '
                        || text[it] == ':'
                        || text[it] == '.'
                        || text[it] == ','
                        || text[it] == '('
                        || text[it] == ';'
                        || text[it] == '-'
                        || text[it] == '+'
                        || text[it] == '*'
                        || text[it] == '?'
                        || text[it] == '/'
                        || text[it] == '='
                        || text[it] == '.'
                        || text[it] == 0
                        || text[it] == 0x0a)
                    {
                        tx = it;
                        ix = ii;
                        break;
                    }
                    it--;
                    ii--;
                };
                i = tx;
                a = ix;
            }
            buf[a] = 0x0a;
            buf[a+1]= 0;
            strcat(result,buf);

            a = len = 0;

            if (text[i] == 0)
                break;

            strcat(result,pref);

            /* if we cut on space, space must be removed!!! */
            if (text[i]==' ')
                i++;

        }
        if (text[i] != 0x0a)
            buf[a++] = text[i];
    }

    /* remove last newline */
    result[strlen(result)-1]=0;

    return;
}

void ChannelMsgCmd(char *data, int len)
{
    uint8 mode;
    uint8 color;
    char    channelname[32];
    char    playername[32];
    char    *message=NULL;
    char    prefix[64];
    char    outstring[1024];

    mode=data[0];
    color=data[1];
    data+=2;

    if (strlen((char *)data)==0)
    {
        LOG(LOG_ERROR,"ChannelMsgCmd: Got no data!\n");
        return;
    }
    message=strchr((char *)data,':');
    if (!message)
    {
        LOG(LOG_ERROR,"ChannelMsgCmd: Empty Message!\n");
        return;
    }
    *(message++) = '\0';
    sscanf((char *)data,"%s %s",channelname, playername);
    if (ignore_check(playername, channelname)) return;
//    draw_info_format(COLOR_WHITE,"chmsg: c: %s, p: %s, m: %s",channelname, playername, message);
    if (mode==1)
    {
        char message2[1024];
        sprintf(prefix,"[%s:%s ",channelname, playername);
        sprintf(message2,"%s%c",message,']');
        break_string(message2, prefix, TRUE, outstring);
    }
    else
    {
        sprintf(prefix,"[%s:%s] ",channelname, playername);
        break_string(message, prefix, FALSE, outstring);
    }

    draw_info(outstring,(NDI_PLAYER|color));

}

#endif
