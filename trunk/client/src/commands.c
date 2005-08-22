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


/* Handles commands received by the server.  This does not necessarily
 * handle all commands - some might be in other files (like init.c)
 *
 * This file handles commans from the server->client.  See player.c
 * for client->server commands.
 *
 * this file contains most of the commands for the dispatch loop most of
 * the functions are self-explanatory, the pixmap/bitmap commands recieve
 * the picture, and display it.  The drawinfo command draws a string
 * in the info window, the stats command updates the local copy of the stats
 * and displays it. handle_query prompts the user for input.
 * send_reply sends off the reply for the input.
 * player command gets the player information.
 * MapScroll scrolls the map on the client by some amount
 * MapCmd displays the map either with layer packing or stack packing.
 *   packing/unpacking is best understood by looking at the server code
 *   (server/ericserver.c)
 *   stack packing is easy, for every map entry that changed, we pack
 *   1 byte for the x/y location, 1 byte for the count, and 2 bytes per
 *   face in the stack.
 *   layer packing is harder, but I seem to remember more efficient:
 *   first we pack in a list of all map cells that changed and are now
 *   empty.  The end of this list is a 255, which is bigger that 121, the
 *   maximum packed map location.
 *   For each changed location we also pack in a list of all the faces and
 *   X/Y coordinates by layer, where the layer is the depth in the map.
 *   This essentially takes slices through the map rather than stacks.
 *   Then for each layer, (max is MAXMAPCELLFACES, a bad name) we start
 *   packing the layer into the message.  First we pack in a face, then
 *   for each place on the layer with the same face, we pack in the x/y
 *   location.  We mark the last x/y location with the high bit on
 *   (11*11 = 121 < 128).  We then continue on with the next face, which
 *   is why the code marks the faces as -1 if they are finished.  Finally
 *   we mark the last face in the layer again with the high bit, clearly
 *   limiting the total number of faces to 32767, the code comments it's
 *   16384, I'm not clear why, but the second bit may be used somewhere
 *   else as well.
 *   The unpacking routines basically perform the opposite operations.
 */

#include <include.h>
static int  scrolldx = 0, scrolldy = 0;
int PasswordAlreadyAsked = 0;

void SoundCmd(unsigned char *data, int len)
{
    int x, y, num, type;

    if (len != 5)
    {
        LOG(LOG_ERROR, "Got invalid length on sound command: %d\n", len);
        return;
    }
    x = (signed char) data[0];
    y = (signed char) data[1];
    num = GetShort_String(data + 2);
    type = data[4];
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
    LOG(LOG_MSG, "Get SetupCmd:: %s\n", buf);
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
            LOG(LOG_ERROR, "Got setup for a command we don't understand: %s %s\n", cmd, param);
        }
    }
    GameStatus = GAME_STATUS_REQUEST_FILES;
}

/* We only get here if the server believes we are caching images. */
/* We rely on the fact that the server will only send a face command for
 * a particular number once - at current time, we have no way of knowing
 * if we have already received a face for a particular number.
 */

void Face1Cmd(unsigned char *data, int len)
{
    int     pnum;
    uint32  checksum;
    char   *face;

    pnum = GetShort_String(data);
    checksum = GetInt_String(data + 2);
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
    GameStatus = GAME_STATUS_START;
    /* add here error handling */
    return;
}

/* This is really a throwaway command - there really isn't any reason to
 * send addme_success commands.
 */
void AddMeSuccess(char *data, int len)
{
    LOG(LOG_MSG, "addme_success received.\n");
    return;
}

void GoodbyeCmd(char *data, int len)
{
    /* This could probably be greatly improved - I am not sure if anything
     * needs to be saved here, but certainly it should be possible to
     * reconnect to the server or a different server without having to
     * rerun the client.
     */

    /* Damn, this should not be here - if the version not matches, the server
         * drops the connnect - so we get a client shutdown here?
         * NEVER do this again.
         */
    /* fprintf(stderr,"Received goodbye command from server - exiting\n");
          exit(0);*/
}

void AnimCmd(unsigned char *data, int len)
{
    short   anum;
    int     i, j;

    anum = GetShort_String(data);
    if (anum<0 || anum> MAXANIM)
    {
        fprintf(stderr, "AnimCmd: animation number invalid: %d\n", anum);
        return;
    }

    animations[anum].flags = *(data + 2);
    animations[anum].facings = *(data + 3);
    animations[anum].num_animations = (len - 4) / 2;
    if (animations[anum].num_animations < 1)
    {
        LOG(LOG_DEBUG, "AnimCmd: num animations invalid: %d\n", animations[anum].num_animations);
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
        LOG(LOG_DEBUG, "Calculated animations does not equal stored animations?(%d!=%d)\n", j,
            animations[anum].num_animations);
    /*LOG(LOG_MSG,"Received animation %d, %d facings and %d faces\n", anum,animations[anum].facings,animations[anum].num_animations);*/
}

void ImageCmd(unsigned char *data, int len)
{
    int     pnum, plen;
    char    buf[2048];
    /*int fd,l; */
    FILE   *stream;

    pnum = GetInt_String(data);
    plen = GetInt_String(data + 4);
    if (len < 8 || (len - 8) != plen)
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
}


void SkillRdyCmd(char *data, int len)
{
    int i, ii;

    strcpy(cpl.skill_name, data);

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
    char    buf[2048];

    flags = (int) GetShort_String(data);
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
    draw_info(buf, flags);
}

void TargetObject(unsigned char *data, int len)
{
    cpl.target_mode = *data++;
    if (cpl.target_mode)
        sound_play_effect(SOUND_WEAPON_ATTACK, 0, 0, 100);
    else
        sound_play_effect(SOUND_WEAPON_HOLD, 0, 0, 100);
    cpl.target_color = *data++;
    cpl.target_code = *data++;
    strcpy(cpl.target_name, data);
    map_udate_flag = 2;

    /*    sprintf(buf,"TO: %d %d >%s< (len: %d)\n",cpl.target_mode,cpl.target_code,cpl.target_name,len);
        draw_info(buf,COLOR_GREEN);*/

}

void StatsCmd(unsigned char *data, int len)
{
    int     i   = 0;
    int     c, temp;
    char   *tmp, *tmp2;

    while (i < len)
    {
        c = data[i++];

        if (c >= CS_STAT_PROT_START && c <= CS_STAT_PROT_END)
        {
            cpl.stats.protection[c - CS_STAT_PROT_START] = (sint16) * (data + i++);
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
                  if (temp > cpl.stats.Str)
                      cpl.warn_statup = TRUE;
                  else
                      cpl.warn_statdown = TRUE;

                  cpl.stats.Str = temp;
                  break;
                case CS_STAT_INT:
                  temp = (int) * (data + i++);
                  if (temp > cpl.stats.Int)
                      cpl.warn_statup = TRUE;
                  else
                      cpl.warn_statdown = TRUE;

                  cpl.stats.Int = temp;
                  break;
                case CS_STAT_POW:
                  temp = (int) * (data + i++);
                  if (temp > cpl.stats.Pow)
                      cpl.warn_statup = TRUE;
                  else
                      cpl.warn_statdown = TRUE;

                  cpl.stats.Pow = temp;

                  break;
                case CS_STAT_WIS:
                  temp = (int) * (data + i++);
                  if (temp > cpl.stats.Wis)
                      cpl.warn_statup = TRUE;
                  else
                      cpl.warn_statdown = TRUE;

                  cpl.stats.Wis = temp;

                  break;
                case CS_STAT_DEX:
                  temp = (int) * (data + i++);
                  if (temp > cpl.stats.Dex)
                      cpl.warn_statup = TRUE;
                  else
                      cpl.warn_statdown = TRUE;

                  cpl.stats.Dex = temp;
                  break;
                case CS_STAT_CON:
                  temp = (int) * (data + i++);
                  if (temp > cpl.stats.Con)
                      cpl.warn_statup = TRUE;
                  else
                      cpl.warn_statdown = TRUE;

                  cpl.stats.Con = temp;
                  break;
                case CS_STAT_CHA:
                  temp = (int) * (data + i++);
                  if (temp > cpl.stats.Cha)
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
                  i += 4;
                  break;
                case CS_STAT_LEVEL:
                  cpl.stats.level = (char) * (data + i++);
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
                  cpl.stats.weapon_sp = (int) * (data + i++);
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
                      /*
                                                    int rlen=data[i++];
                                                    strncpy(cpl.title2,
                                                    (const char*)data+i,rlen);
                                                    cpl.title2[rlen]='\0';
                                                    i += rlen;
                                                */
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
    if (strstr(cmd, "What is your name?"))
    {
        LOG(LOG_MSG, "Login: Enter name\n");
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
        GameStatus = GAME_STATUS_NAME;
    }
    if (strstr(cmd, "What is your password?"))
    {
        LOG(LOG_MSG, "Login: Enter password\n");
        GameStatus = GAME_STATUS_PSWD;
        PasswordAlreadyAsked = 1;
    }
    if (strstr(cmd, "Please type your password again."))
    {
        LOG(LOG_MSG, "Login: Enter verify password\n");
        GameStatus = GAME_STATUS_VERIFYPSWD;
        PasswordAlreadyAsked = 2;
    }
    if (GameStatus >= GAME_STATUS_NAME && GameStatus <= GAME_STATUS_VERIFYPSWD)
        open_input_mode(12);
}


void handle_query(char *data, int len)
{
    char   *buf, *cp;
    /*uint8 flags = atoi(data);*/

    buf = strchr(data, ' ');
    if (buf)
        buf++;

    if (buf)
    {
        cp = buf;
        while ((buf = strchr(buf, '\n')) != NULL)
        {
            *buf++ = '\0';
            LOG(LOG_MSG, "Received query string: %s\n", cp);
            PreParseInfoStat(cp);
            cp = buf;
        }
    }
}

/* Sends a reply to the server.  text contains the null terminated
 * string of text to send.  This function basically just packs
 * the stuff up.
 */
void send_reply(char *text)
{
    char    buf[MAXSOCKBUF];
    sprintf(buf, "reply %s", text);
    cs_write_string(csocket.fd, buf, strlen(buf));
}



/* This function copies relevant data from the archetype to the
 * object.  Only copies data that was not set in the object
 * structure.
 *
 */

void PlayerCmd(unsigned char *data, int len)
{
    char    name[MAX_BUF];
    int     tag, weight, face, i = 0, nlen;

    GameStatus = GAME_STATUS_PLAY;
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
}



/* no item command, including the delinv... */
/* this is a bit hacked now - perhaps we should consider
 * in the future a new designed item command.
 */
void ItemXCmd(unsigned char *data, int len)
{
    int     weight, loc, tag, face, flags, pos = 0, nlen, anim, nrof, dmode;
    uint8   itype, stype, item_qua, item_con, item_skill, item_level;
    uint8   animspeed, direction = 0;
    char    name[MAX_BUF];

    map_udate_flag = 2;
    itype = stype = item_qua = item_con = item_skill = item_level = 0;

    dmode = GetInt_String(data);
    pos += 4;

    /*LOG(-1,"ITEMX:(%d) %s\n", dmode, locate_item(dmode)?(locate_item(dmode)->d_name?locate_item(dmode)->s_name:"no name"):"no LOC");*/

    loc = GetInt_String(data + pos);

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
        LOG(LOG_ERROR, "ItemCmd: Got location with no other data\n");
    }
    else
    {
        while (pos < len)
        {
            tag = GetInt_String(data + pos); pos += 4;
            flags = GetInt_String(data + pos); pos += 4;
            weight = GetInt_String(data + pos); pos += 4;
            face = GetInt_String(data + pos); pos += 4;
            request_face(face, 0);
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
            anim = GetShort_String(data + pos); pos += 2;
            animspeed = data[pos++];
            nrof = GetInt_String(data + pos); pos += 4;
            update_item(tag, loc, name, weight, face, flags, anim, animspeed, nrof, itype, stype, item_qua, item_con,
                        item_skill, item_level, direction, FALSE);
        }
        if (pos > len)
            LOG(LOG_ERROR, "ItemCmd: ERROR: Overread buffer: %d > %d\n", pos, len);
    }
    map_udate_flag = 2;
}

/* no item command, including the delinv... */
/* this is a bit hacked now - perhaps we should consider
 * in the future a new designed item command.
 */
void ItemYCmd(unsigned char *data, int len)
{
    int     weight, loc, tag, face, flags, pos = 0, nlen, anim, nrof, dmode;
    uint8   itype, stype, item_qua, item_con, item_skill, item_level;
    uint8   animspeed, direction = 0;
    char    name[MAX_BUF];

    map_udate_flag = 2;
    itype = stype = item_qua = item_con = item_skill = item_level = 0;

    dmode = GetInt_String(data);
    pos += 4;

    /*LOG(-1,"ITEMX:(%d) %s\n", dmode, locate_item(dmode)?(locate_item(dmode)->d_name?locate_item(dmode)->s_name:"no name"):"no LOC");*/

    loc = GetInt_String(data + pos);

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
        LOG(LOG_ERROR, "ItemCmd: Got location with no other data\n");
    }
    else
    {
        while (pos < len)
        {
            tag = GetInt_String(data + pos); pos += 4;
            flags = GetInt_String(data + pos); pos += 4;
            weight = GetInt_String(data + pos); pos += 4;
            face = GetInt_String(data + pos); pos += 4;
            request_face(face, 0);
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
            anim = GetShort_String(data + pos); pos += 2;
            animspeed = data[pos++];
            nrof = GetInt_String(data + pos); pos += 4;
            update_item(tag, loc, name, weight, face, flags, anim, animspeed, nrof, itype, stype, item_qua, item_con,
                        item_skill, item_level, direction, TRUE);
        }
        if (pos > len)
            LOG(LOG_ERROR, "ItemCmd: ERROR: Overread buffer: %d > %d\n", pos, len);
    }
    map_udate_flag = 2;
}

void GroupCmd(unsigned char *data, int len)
{
    char    name[64], *tmp;
    int     hp, mhp, sp, msp, gr, mgr, level, slot = 0;

    /* len == 0, its a GroupCmd which means "no group" */
    clear_group();
    if (len)
    {
        /*sprintf(buf, "GROUP CMD: %s (%d)", data, len);
        draw_info(buf, COLOR_GREEN);*/

        global_group_status = GROUP_MEMBER;
        tmp = strchr(data, '|');
        while (tmp)
        {
            tmp++;
            sscanf(tmp, "%s %d %d %d %d %d %d %d", name, &hp, &mhp, &sp, &msp, &gr, &mgr, &level);
            set_group(slot, name, level, hp, mhp, sp, msp, gr, mgr);

            /*LOG(-1, "GROUP: %s [(%x)]\n", tmp, tmp);*/
            slot++;
            tmp = strchr(tmp, '|');
        }
    }
}

/* Someone want invite us to a group.
 */
void GroupInviteCmd(unsigned char *data, int len)
{
    if(global_group_status != GROUP_NO) /* bug */
        LOG(LOG_ERROR, "ERROR: Got group invite when g_status != GROUP_NO (%s).\n", data);
    else
    {
        global_group_status = GROUP_INVITE;
        strncpy(group_invite, data, 30);
    }
}


void GroupUpdateCmd(unsigned char *data, int len)
{
    char        *tmp;
    int         hp, mhp, sp, msp, gr, mgr, level, slot = 0;

    if (!len)
        return;

    tmp = strchr(data, '|');
    while (tmp)
    {
        tmp++;
        sscanf(tmp, "%d %d %d %d %d %d %d %d", &slot, &hp, &mhp, &sp, &msp, &gr, &mgr, &level);
        set_group(slot, NULL, level, hp, mhp, sp, msp, gr, mgr);

        /*LOG(-1, "UPDATE: %s :: %d %d %d %d %d %d %d %d\n", tmp, slot, level, hp, mhp, sp, msp, gr, mgr);*/
        tmp = strchr(tmp, '|');
    }
}

void BookCmd(unsigned char *data, int len)
{
    int mode;

    sound_play_effect(SOUND_BOOK, 0, 0, 100);
    cpl.menustatus = MENU_BOOK;

    mode = *((int*)data);
    data+=4;

    /*LOG(-1,"BOOK (%d): %s\n", mode, data);
    draw_info(data,COLOR_YELLOW);*/

    //gui_book_interface =
    gui_interface_book = load_book_interface(mode, data, len-4);

}

void InterfaceCmd(unsigned char *data, int len)
{

    map_udate_flag = 2;
    if((gui_interface_npc && gui_interface_npc->status != GUI_INTERFACE_STATUS_WAIT) &&
         (cpl.menustatus == MENU_NPC && !len) || (len && cpl.menustatus != MENU_NPC))
    {
        sound_play_effect(SOUND_SCROLL, 0, 0, 100);
    }
    reset_gui_interface();
    if(len)
    {
        int mode, pos = 0;

        /*interface_mode = INTERFACE_MODE_NPC;*/
        mode = *data;
        pos ++;

    gui_interface_npc = load_gui_interface(mode, data, len, pos);
    if(!gui_interface_npc)
        draw_info("INVALID GUI CMD", COLOR_RED);
    else
    {
        gui_interface_npc->win_length = precalc_interface_npc();
        interface_mode = mode;
        cpl.menustatus = MENU_NPC;
        gui_interface_npc->startx = 400-(Bitmaps[BITMAP_NPC_INTERFACE]->bitmap->w / 2);
        gui_interface_npc->starty = 50;
        active_button = -1;
        mb_clicked=0;
    }
    }
}

/* UpdateItemCmd updates some attributes of an item */
void UpdateItemCmd(unsigned char *data, int len)
{
    int     weight, loc, tag, face, sendflags, flags, pos = 0, nlen, anim, nrof;
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
    /*LOG(-1,"UPDATE: loc:%d tag:%d\n",loc, tag); */
    weight = (int) (ip->weight * 1000);
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
            fprintf(stderr, "UpdateItemCmd: unknown object tag (%d) for new location\n", loc);
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
        fprintf(stderr, "UpdateItemCmd: Overread buffer: %d > %d\n", pos, len);
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
    update_item(tag, loc, name, weight, face, flags, anim, animspeed, nrof, 254, 254, 254, 254, 254, 254, direction,
                FALSE);
    map_udate_flag = 2;
}

void DeleteItem(unsigned char *data, int len)
{
    int pos = 0, tag;

    while (pos < len)
    {
        tag = GetInt_String(data); pos += 4;
        delete_item(tag);
    }
    if (pos > len)
        fprintf(stderr, "ItemCmd: Overread buffer: %d > %d\n", pos, len);
    map_udate_flag = 2;
}

void DeleteInventory(unsigned char *data, int len)
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


void Map2Cmd(unsigned char *data, int len)
{
    int     mask, x, y, pos = 0, ext_flag, xdata;
    int     ext1, ext2, ext3, probe;
    int     map_new_flag    = FALSE;
    int     ff0, ff1, ff2, ff3, ff_flag, xpos, ypos;
    char    pname1[64], pname2[64], pname3[64], pname4[64];
    uint16  face;

    if (scrolldx || scrolldy)
    {
        if(cpl.menustatus != MENU_NO)
            reset_menu_status();
        display_mapscroll(scrolldx, scrolldy);
    }

    scrolldy = scrolldx = 0;
    map_transfer_flag = 0;
    xpos = (uint8) (data[pos++]);
    if (xpos == 255) /* its not xpos, its the changed map marker */
    {
        map_new_flag = TRUE;

        xpos = (uint8) (data[pos++]);
    }

    ypos = (uint8) (data[pos++]);
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
        /* first, we get the mask flag - it decribes what we now get */
        mask = GetShort_String(data + pos); pos += 2;
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
            }
            if (ext_flag & 0x40) /* damage add on the map */
            {
                ff0 = ff1 = ff2 = ff3 = -1;
                ff_flag = (uint8) (data[pos++]);
                if (ff_flag & 0x8)
                {
                    ff0 = GetShort_String(data + pos); pos += 2;
                    add_anim(ANIM_KILL, 0, 0, xpos + x, ypos + y, ff0);
                }
                if (ff_flag & 0x4)
                {
                    ff1 = GetShort_String(data + pos); pos += 2;
                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff1);
                }
                if (ff_flag & 0x2)
                {
                    ff2 = GetShort_String(data + pos); pos += 2;
                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff2);
                }
                if (ff_flag & 0x1)
                {
                    ff3 = GetShort_String(data + pos); pos += 2;
                    add_anim(ANIM_DAMAGE, 0, 0, xpos + x, ypos + y, ff3);
                }
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
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
            xdata = 0;
            set_map_face(x, y, 0, face, xdata, -1, pname1);
        }
        if (mask & 0x4)
        {
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
            xdata = 0;
            if (ext_flag & 0x04) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 1, face, xdata, ext1, pname2);
        }
        if (mask & 0x2)
        {
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
            xdata = 0;
            if (ext_flag & 0x02) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 2, face, xdata, ext2, pname3);
        }
        if (mask & 0x1)
        {
            face = GetShort_String(data + pos); pos += 2;
            request_face(face, 0);
            /*LOG(0,"we got face: %x (%x) ->%s\n", face, face&~0x8000, FaceList[face&~0x8000].name?FaceList[face&~0x8000].name:"(null)" );*/
            xdata = 0;
            if (ext_flag & 0x01) /* we have here a multi arch, fetch head offset */
            {
                xdata = (uint8) (data[pos]);
                pos++;
            }
            set_map_face(x, y, 3, face, xdata, ext3, pname4);
        }
    } /* more tiles */
    map_udate_flag = 2;
}


void map_scrollCmd(char *data, int len)
{
    static int  step    = 0;
    char       *buf;

    if(cpl.menustatus != MENU_NO)
        reset_menu_status();

    scrolldx += atoi(data);
    buf = strchr(data, ' ');
    if (!buf)
    {
        LOG(LOG_ERROR, "ERROR: map_scrollCmd: Got short packet.\n");
        return;
    }
    buf++;
    scrolldy += atoi(buf);
    if (++step % 2)
        sound_play_effect(SOUND_STEP1, 0, 0, 100);
    else
        sound_play_effect(SOUND_STEP2, 0, 0, 100);

    if (media_show != MEDIA_SHOW_NO)
    {
        if (!--media_show_update)
            media_show = MEDIA_SHOW_NO; /* kick media file depending on map position */
    }
}

void MagicMapCmd(unsigned char *data, int len)
{
}

void VersionCmd(char *data, int len)
{
    char   *cp;
    char    buf[1024];

    GameStatusVersionOKFlag = FALSE;
    GameStatusVersionFlag = TRUE;
    csocket.cs_version = atoi(data);

    /* The first version is the client to server version the server wants
     * ATM, we just do for "must match".
     * Later it will be smart to define range where the differences are ok
     */
    if (VERSION_CS != csocket.cs_version)
    {
        sprintf(buf, "Invalid CS version (%d,%d)", VERSION_CS, csocket.cs_version);
        draw_info(buf, COLOR_RED);
        if (VERSION_CS > csocket.cs_version)
            sprintf(buf, "The server is outdated!\nSelect a different one!");
        else
            sprintf(buf, "Your client is outdated!\nUpdate your client!");
        draw_info(buf, COLOR_RED);
        LOG(LOG_ERROR, "%s\n", buf);
        return;
    }
    cp = (char *) (strchr(data, ' '));
    if (!cp)
    {
        sprintf(buf, "Invalid version string: %s", data);
        draw_info(buf, COLOR_RED);
        LOG(LOG_ERROR, "%s\n", buf);
        return;
    }
    csocket.sc_version = atoi(cp);
    if (csocket.sc_version != VERSION_SC)
    {
        sprintf(buf, "Invalid SC version (%d,%d)", VERSION_SC, csocket.sc_version);
        draw_info(buf, COLOR_RED);
        LOG(LOG_ERROR, "%s\n", buf);
        return;
    }
    cp = (char *) (strchr(cp + 1, ' '));
    if (!cp || strncmp(cp + 1, "Daimonin Server", 15))
    {
        sprintf(buf, "Invalid server name: %s", cp);
        draw_info(buf, COLOR_RED);
        LOG(LOG_ERROR, "%s\n", buf);
        return;
    }

    LOG(LOG_MSG, "Playing on server type %s\n", cp);
    GameStatusVersionOKFlag = TRUE;
}

void SendVersion(ClientSocket csock)
{
    char    buf[MAX_BUF];

    sprintf(buf, "version %d %d %s", VERSION_CS, VERSION_SC, PACKAGE_NAME);
    cs_write_string(csock.fd, buf, strlen(buf));
}

void RequestFile(ClientSocket csock, int index)
{
    char    buf[MAX_BUF];

    sprintf(buf, "rf %d", index);
    cs_write_string(csock.fd, buf, strlen(buf));
}

void SendAddMe(ClientSocket csock)
{
    cs_write_string(csock.fd, "addme", 5);
}

void SendSetFaceMode(ClientSocket csock, int mode)
{
    char    buf[MAX_BUF];

    sprintf(buf, "setfacemode %d", mode);
    cs_write_string(csock.fd, buf, strlen(buf));
}


void MapstatsCmd(unsigned char *data, int len)
{
    char    name[256];
    char   *tmp;
    int     w, h, x, y;

    sscanf(data, "%d %d %d %d", &w, &h, &x, &y);
    tmp = strchr(data, ' ');
    tmp = strchr(tmp + 1, ' ');
    tmp = strchr(tmp + 1, ' ');
    tmp = strchr(tmp + 1, ' ');
    strcpy(name, tmp + 1);
    InitMapData(name, w, h, x, y);
    map_udate_flag = 2;
}

void SkilllistCmd(unsigned char *data, int len)
{
    unsigned char *tmp, *tmp2, *tmp3, *tmp4;
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
                        }
                    }
                }
            }
        }
    }
}

void SpelllistCmd(unsigned char *data, int len)
{
    int     i, ii, mode;
    unsigned char   *tmp, *tmp2;
    char    name[256];

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
            strncpy(name, data, tmp2 - data);
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

void GolemCmd(unsigned char *data, int len)
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
        request_face(face, 0);
        tmp = strchr(tmp + 1, ' '); /* find start of a name */
        sprintf(buf, "You lose control of %s.", tmp + 1);
        draw_info(buf, COLOR_WHITE);

        fire_mode_tab[FIRE_MODE_SUMMON].item = FIRE_ITEM_NO;
        fire_mode_tab[FIRE_MODE_SUMMON].name[0] = 0;
    }
    else
    {
        tmp = strchr(data, ' '); /* find start of a name */
        face = atoi(tmp + 1);
        request_face(face, 0);
        tmp = strchr(tmp + 1, ' '); /* find start of a name */
        sprintf(buf, "You get control of %s.", tmp + 1);
        draw_info(buf, COLOR_WHITE);
        fire_mode_tab[FIRE_MODE_SUMMON].item = face;
        strncpy(fire_mode_tab[FIRE_MODE_SUMMON].name, tmp + 1, 100);
        RangeFireMode = FIRE_MODE_SUMMON;
    }
}


static void save_data_cmd_file(char *path, char *data, int len)
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

/* server tells us to go to the new char creation */
void NewCharCmd(char *data, int len)
{
    dialog_new_char_warn = FALSE;
    GameStatus = GAME_STATUS_NEW_CHAR;
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
    uint32  dest_len    = 512 * 1024;
    char   *dest;

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
              uncompress(dest, (void *) &dest_len, data, len);
              data = dest;
              len = dest_len;
          }
          request_file_chain++;
          save_data_cmd_file(FILE_CLIENT_SKILLS, data, len);
          read_skills();
          break;
        case DATA_CMD_SPELL_LIST:
          if (data_comp)
          {
              LOG(LOG_DEBUG, "data cmd: compressed spell list(len:%d)\n", len);
              uncompress(dest, (void *) &dest_len, data, len);
              data = dest;
              len = dest_len;
          }
          request_file_chain++;
          save_data_cmd_file(FILE_CLIENT_SPELLS, data, len);
          read_spells();
          break;
        case DATA_CMD_SETTINGS_LIST:
          if (data_comp)
          {
              LOG(LOG_DEBUG, "data cmd: compressed settings file(len:%d)\n", len);
              uncompress(dest, (void *) &dest_len, data, len);
              data = dest;
              len = dest_len;
          }
          request_file_chain++;
          save_data_cmd_file(FILE_CLIENT_SETTINGS, data, len);
          /*read_settings();*/
          break;

        case DATA_CMD_BMAP_LIST:
          if (data_comp)
          {
              LOG(LOG_DEBUG, "data cmd: compressed bmaps file(len:%d)\n", len);
              uncompress(dest, (void *) &dest_len, data, len);
              data = dest;
              len = dest_len;
          }
          request_file_chain++;
          save_data_cmd_file(FILE_CLIENT_BMAPS, data, len);
          request_file_flags |= SRV_CLIENT_FLAG_BMAP;
          break;

        case DATA_CMD_ANIM_LIST:
          if (data_comp)
          {
              uncompress(dest, (void *) &dest_len, data, len);
              LOG(LOG_DEBUG, "data cmd: compressed anims file(len:%d) -> %d\n", len, dest_len);
              data = dest;
              len = dest_len;
          }
          request_file_chain++;
          save_data_cmd_file(FILE_CLIENT_ANIMS, data, len);
          request_file_flags |= SRV_CLIENT_FLAG_ANIM;
          break;

        default:
          LOG(LOG_ERROR, "data cmd: unknown type %d (len:%d)\n", data_type, len);
          break;
    }
    free(dest);
}





















