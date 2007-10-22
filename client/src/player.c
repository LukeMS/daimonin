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

/* This file handles various player related functions.  This includes
 * both things that operate on the player item, cpl structure, or
 * various commands that the player issues.
 *
 *  This file does most of the handling of commands from the client to
 *  server (see commands.c for server->client)
 *
 *  does most of the work for sending messages to the server
 *   Again, most of these appear self explanatory.  Most send a bunch of
 *   commands like apply, examine, fire, run, etc.  This looks like it
 *   was done by Mark to remove the old keypress stupidity I used.
 */

/* This translates the numeric direction id's into the actual direction
 * commands.  This lets us send the actual command (ie, 'north'), which
 * makes handling on the server side easier.
 */

#include <include.h>
#include <math.h>
/*
 *  Initialiazes player item, information is received from server
 */

_server_level   server_level;

typedef enum _player_doll_enum
{
    PDOLL_ARMOUR,
    PDOLL_HELM,
    PDOLL_LEGS,
    PDOLL_BOOT,
    PDOLL_RHAND,
    PDOLL_LHAND,
    PDOLL_RRING,
    PDOLL_LRING,
    PDOLL_BRACER,
    PDOLL_ROBE,
    PDOLL_AMULET,
    PDOLL_DISTANCE,
    PDOLL_AMUN,
    PDOLL_GIRDLE,
    PDOLL_GAUNTLET,
    PDOLL_SHOULDER,
    PDOLL_LIGHT,
    PDOLL_INIT /* must be last element */
}   _player_doll_enum;

typedef struct _player_doll_pos
{
    int                 xpos;
    int                 ypos;
}
_player_doll_pos;

_player_doll_pos    player_doll[PDOLL_INIT] =
    {
        {93,91}, {93,44}, {93,136}, {93,194}, {135,131}, {50,131}, {50,170}, {135,170}, {54,87}, {141,46}, {180,46}, {5,200}, {5,238},
        {5,144}, {180,144}, {43,46}, {4,46}
    };

void clear_player(void)
{
    memset(quick_slots, -1, sizeof(quick_slots));
    free_all_items(cpl.sack);
    free_all_items(cpl.below);
    free_all_items(cpl.ob);
    cpl.ob = player_item();
    init_player_data();
}

void new_player(uint32 tag, char *name, uint32 weight, short face)
{
    cpl.ob->tag = tag;
    cpl.ob->weight = weight;
    cpl.ob->face = face;
    copy_name(cpl.ob->d_name, name);
}


void new_char(_server_char *nc)
{
    char    buf[MAX_BUF];

    sprintf(buf, "nc %s %d %d %d %d %d %d %d %d", nc->char_arch[nc->gender_selected], nc->stats[0], nc->stats[1],
            nc->stats[2], nc->stats[3], nc->stats[4], nc->stats[5], nc->stats[6], nc->skill_selected);
    cs_write_string(csocket.fd, buf, strlen(buf));
}

void look_at(int x, int y)
{
    char    buf[MAX_BUF];

    sprintf(buf, "lt %d %d", x, y);
    cs_write_string(csocket.fd, buf, strlen(buf));
}

void client_send_apply(int tag)
{
    char    buf[MAX_BUF];

    sprintf(buf, "ap %d", tag);
    cs_write_string(csocket.fd, buf, strlen(buf));
}

void client_send_examine(int tag)
{
    char    buf[MAX_BUF];

    sprintf(buf, "ex %d", tag);
    cs_write_string(csocket.fd, buf, strlen(buf));
}

/* Requests nrof objects of tag get moved to loc. */
void client_send_move(int loc, int tag, int nrof)
{
    char    buf[MAX_BUF];

    sprintf(buf, "mv %d %d %d", loc, tag, nrof);
    cs_write_string(csocket.fd, buf, strlen(buf));
}

void client_send_tell_extended(char *body, char *tail)
{
    char    buf[MAX_BUF];

    sprintf(buf, "tx %s %s", body, tail);
    cs_write_string(csocket.fd, buf, strlen(buf));
}

/* This should be used for all 'command' processing.  Other functions should
 * call this so that proper windowing will be done.
 * command is the text command, repeat is a count value, or -1 if none
 * is desired and we don't want to reset the current count.
 * must_send means we must send this command no matter what (ie, it is
 * an administrative type of command like fire_stop, and failure to send
 * it will cause definate problems
 * return 1 if command was sent, 0 if not sent.
 */

int send_command(const char *command, int repeat, int must_send)
{
    char        buf[MAX_BUF];
    SockList    sl;

    /* Does the server understand 'ncom'? If so, special code */
    if (csocket.cs_version >= 1021)
    {
        int commdiff    = csocket.command_sent - csocket.command_received;
        if (commdiff < 0)
            commdiff += 256;

        csocket.command_sent++;
        csocket.command_sent &= 0xff;   /* max out at 255 */

        sl.buf = (unsigned char *) buf;
        strcpy((char *) sl.buf, "ncom ");
        sl.len = 5;
        SockList_AddShort(&sl, (uint16) csocket.command_sent);
        SockList_AddInt(&sl, repeat);
        strncpy((char *) sl.buf + sl.len, command, MAX_BUF - sl.len);
        sl.buf[MAX_BUF - 1] = 0;
        sl.len += strlen(command);
        send_socklist(csocket.fd, sl);
    }
    else
    {
        sprintf(buf, "cm %d %s", repeat, command);
        cs_write_string(csocket.fd, buf, strlen(buf));
    }
    if (repeat != -1)
        cpl.count = 0;
    return 1;
}

void CompleteCmd(unsigned char *data, int len)
{
    if (len != 6)
    {
        fprintf(stderr, "comc - invalid length %d - ignoring\n", len);
    }
    csocket.command_received = GetShort_String(data);
    csocket.command_time = GetInt_String(data + 2);
}

/* Show a basic help message */
void show_help()
{}

/* This is an extended command (ie, 'who, 'whatever, etc).  In general,
 * we just send the command to the server, but there are a few that
 * we care about (bind, unbind)
 *
 * The command past to us can not be modified - if it is a keybinding,
 * we get passed the string that is that binding - modifying it effectively
 * changes the binding.
 */

void extended_command(const char *ocommand)
{}

void set_weight_limit(uint32 wlim)
{
    cpl.weight_limit = wlim;
}

void init_player_data(void)
{
    int i;

    new_player(0, "", 0, 0);

    cpl.fire_on = cpl.firekey_on = 0;
    cpl.resize_twin = 0;
    cpl.resize_twin_marker = 0;
    cpl.run_on = cpl.runkey_on = 0;
    cpl.inventory_win = IWIN_BELOW;

    cpl.count_left = 0;
    cpl.container_tag = -996;
    cpl.container = NULL;
    memset(&cpl.stats, 0, sizeof(Stats));
    cpl.stats.maxsp = 1;
    cpl.stats.maxhp = 1;
    cpl.gen_hp = 0.0f;
    cpl.gen_sp = 0.0f;
    cpl.gen_grace = 0.0f;
    cpl.target_hp = 0;

    cpl.stats.maxgrace = 1;
    cpl.stats.speed = 100.0f;
    cpl.stats.spell_fumble = 0.0f;
    cpl.input_text[0] = '\0';

    cpl.stats.hptick = cpl.stats.sptick = cpl.stats.gracetick = LastTick;

    cpl.title[0] = '\0';
    cpl.alignment[0] = '\0';
    cpl.gender[0] = '\0';
    cpl.range[0] = '\0';

    for (i = 0; i < range_size; i++)
        cpl.ranges[i] = NULL;

    cpl.map_x = 0;
    cpl.map_y = 0;

    cpl.ob->nrof = 1;

    /* this is set from title in stat cmd */
    strcpy(cpl.pname, "");
    strcpy(cpl.title, "");

    cpl.menustatus = MENU_NO;
    cpl.menustatus = MENU_NO;
    cpl.count_left = 0;
    cpl.stats.maxsp = 1;    /* avoid div by 0 errors */
    cpl.stats.maxhp = 1;    /* ditto */
    cpl.stats.maxgrace = 1; /* ditto */
    /* ditto - displayed weapon speed is weapon speed/speed */
    cpl.stats.speed = 100.0f;
    cpl.stats.spell_fumble = 0.0f;
    cpl.stats.weapon_sp = 0;
    cpl.input_text[0] = '\0';
    cpl.range[0] = '\0';

    for (i = 0; i < range_size; i++)
        cpl.ranges[i] = NULL;
    cpl.map_x = 0;
    cpl.map_y = 0;
    cpl.container_tag = -997;
    cpl.container = NULL;
    cpl.magicmap = NULL;

    RangeFireMode = 0;
}

/* player name, exp, level, titel*/
void show_player_data(int x, int y)
{
    char    buf[256];

    if (GameStatus == GAME_STATUS_PLAY)
    {
        if (cpl.rank[0] != 0)
            sprintf(buf, "%s %s\n", cpl.rank, cpl.pname);
        else
            strcpy(buf, cpl.pname);
        StringBlt(ScreenSurface, &SystemFont, buf, 6, 2, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%s %s %s", cpl.gender, cpl.race, cpl.title);
        StringBlt(ScreenSurface, &SystemFont, buf, 6, 14, COLOR_HGOLD, NULL, NULL);
        if (strcmp(cpl.godname, "none"))
            sprintf(buf, "%s follower of %s", cpl.alignment, cpl.godname);
        else
            strcpy(buf, cpl.alignment);
        StringBlt(ScreenSurface, &SystemFont, buf, 6, 26, COLOR_HGOLD, NULL, NULL);
    }
}

/* player data like stats, hp etc */
void show_player_stats(int x, int y)
{
    char        buf[256];
    double      temp, multi, line;
    SDL_Rect    box;
    int         s, level_exp;
    int         mx, my;

    SDL_GetMouseState(&mx, &my);

    sprite_blt(Bitmaps[BITMAP_STATS], x, y, NULL, NULL);

    StringBlt(ScreenSurface, &Font6x3Out, "Stats", x + 8, y + 1, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.Str);
    StringBlt(ScreenSurface, &SystemFont, "Str", x + 8, y + 17, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 30, y + 17, COLOR_GREEN, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.Dex);
    StringBlt(ScreenSurface, &SystemFont, "Dex", x + 8, y + 28, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 30, y + 28, COLOR_GREEN, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.Con);
    StringBlt(ScreenSurface, &SystemFont, "Con", x + 8, y + 39, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 30, y + 39, COLOR_GREEN, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.Int);
    StringBlt(ScreenSurface, &SystemFont, "Int", x + 8, y + 50, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 30, y + 50, COLOR_GREEN, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.Wis);
    StringBlt(ScreenSurface, &SystemFont, "Wis", x + 8, y + 61, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 30, y + 61, COLOR_GREEN, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.Pow);
    StringBlt(ScreenSurface, &SystemFont, "Pow", x + 8, y + 72, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 30, y + 72, COLOR_GREEN, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.Cha);
    StringBlt(ScreenSurface, &SystemFont, "Cha", x + 8, y + 83, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 30, y + 83, COLOR_GREEN, NULL, NULL);


    StringBlt(ScreenSurface, &SystemFont, "HP", x + 58, y + 10, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%d (%d)", cpl.stats.hp, cpl.stats.maxhp);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 160 - get_string_pixel_length(buf, &SystemFont), y + 10,
              COLOR_GREEN, NULL, NULL);

    sprite_blt(Bitmaps[BITMAP_HP_BACK], x + 57, y + 23, NULL, NULL);

    if (cpl.stats.maxhp)
    {
        double tmp = cpl.stats.hp;
        if (tmp < 0)
            tmp = 0;

        if ((LastTick-cpl.stats.hptick)<=1000)
        {
            if (cpl.stats.temphp>0)
                tmp -= (double)((cpl.stats.temphp)*(1.0f-(double)(LastTick-cpl.stats.hptick)/1000.0f));
            else
                tmp += (double)(abs(cpl.stats.temphp)*(1.0f-(double)(LastTick-cpl.stats.hptick)/1000.0f));
        }

        temp = tmp / (double) cpl.stats.maxhp;
        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[BITMAP_HP]->bitmap->h;
        box.w = (int) (Bitmaps[BITMAP_HP]->bitmap->w * temp);
        if (tmp && !box.w)
            box.w = 1;
        if (box.w > Bitmaps[BITMAP_HP]->bitmap->w)
            box.w = Bitmaps[BITMAP_HP]->bitmap->w;
        sprite_blt(Bitmaps[BITMAP_HP_BACK], x + 57, y + 23, NULL, NULL);
        sprite_blt(Bitmaps[BITMAP_HP], x + 57, y + 23, &box, NULL);
    }

    StringBlt(ScreenSurface, &SystemFont, "Mana", x + 58, y + 34, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%d (%d)", cpl.stats.sp, cpl.stats.maxsp);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 160 - get_string_pixel_length(buf, &SystemFont), y + 34,
              COLOR_GREEN, NULL, NULL);
    if (cpl.stats.maxsp)
    {
        double tmp = cpl.stats.sp;
        if (tmp < 0)
            tmp = 0;

        if ((LastTick-cpl.stats.sptick)<=1000)
        {
            if (cpl.stats.tempsp>0)
                tmp -= (double)((cpl.stats.tempsp)*(1.0f-(double)(LastTick-cpl.stats.sptick)/1000.0f));
            else
                tmp += (double)(abs(cpl.stats.tempsp)*(1.0f-(double)(LastTick-cpl.stats.sptick)/1000.0f));
        }

        temp =  tmp / (double) cpl.stats.maxsp;
        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[BITMAP_SP]->bitmap->h;
        box.w = (int) (Bitmaps[BITMAP_SP]->bitmap->w * temp);
        if (tmp && !box.w)
            box.w = 1;
        if (box.w > Bitmaps[BITMAP_SP]->bitmap->w)
            box.w = Bitmaps[BITMAP_SP]->bitmap->w;
        sprite_blt(Bitmaps[BITMAP_SP_BACK], x + 57, y + 47, NULL, NULL);
        sprite_blt(Bitmaps[BITMAP_SP], x + 57, y + 47, &box, NULL);
    }
    StringBlt(ScreenSurface, &SystemFont, "Grace", x + 58, y + 58, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%d (%d)", cpl.stats.grace, cpl.stats.maxgrace);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 160 - get_string_pixel_length(buf, &SystemFont), y + 58,
              COLOR_GREEN, NULL, NULL);
    if (cpl.stats.maxgrace)
    {
        double tmp = cpl.stats.grace;
        if (tmp < 0)
            tmp = 0;

        if ((LastTick-cpl.stats.gracetick)<=1000)
        {
            if (cpl.stats.temphp>0)
                tmp -= (double)((cpl.stats.tempgrace)*(1.0f-(double)(LastTick-cpl.stats.gracetick)/1000.0f));
            else
                tmp += (double)(abs(cpl.stats.tempgrace)*(1.0f-(double)(LastTick-cpl.stats.gracetick)/1000.0f));
        }

        temp = tmp / (double) cpl.stats.maxgrace;

        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[BITMAP_GRACE]->bitmap->h;
        box.w = (int) (Bitmaps[BITMAP_GRACE]->bitmap->w * temp);
        if (tmp && !box.w)
            box.w = 1;
        if (box.w > Bitmaps[BITMAP_GRACE]->bitmap->w)
            box.w = Bitmaps[BITMAP_GRACE]->bitmap->w;
        sprite_blt(Bitmaps[BITMAP_GRACE_BACK], x + 57, y + 71, NULL, NULL);
        sprite_blt(Bitmaps[BITMAP_GRACE], x + 57, y + 71, &box, NULL);
    }

    sprite_blt(Bitmaps[BITMAP_FOOD_BACK], x + 87, y + 88, NULL, NULL);

    if (cpl.stats.food)
    {
        int bar = BITMAP_FOOD2;
        int tmp = cpl.stats.food;

        if (tmp < 1)
        {
            StringBlt(ScreenSurface, &SystemFont, "Food", x + 58, y + 84, COLOR_WHITE, NULL, NULL);
            tmp *= -1;
        }
        else if (tmp == 999)
        {
            StringBlt(ScreenSurface, &SystemFont, "Rest", x + 58, y + 84, COLOR_WHITE, NULL, NULL);
        }
        else
        {
            bar = BITMAP_FOOD;
            StringBlt(ScreenSurface, &SystemFont, "Wait", x + 58, y + 84, COLOR_WHITE, NULL, NULL);
        }

        if (tmp < 0)
            tmp = 0;
        temp = (double) tmp / 1000;
        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[bar]->bitmap->h;
        box.w = (int) (Bitmaps[bar]->bitmap->w * temp);
        if (tmp && !box.w)
            box.w = 1;
        if (box.w > Bitmaps[bar]->bitmap->w)
            box.w = Bitmaps[bar]->bitmap->w;
        sprite_blt(Bitmaps[bar], x + 87, y + 88, &box, NULL);

    }


    StringBlt(ScreenSurface, &Font6x3Out, "Skill Groups", x + 472, y + 1, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &Font6x3Out, "name / level", x + 472, y + 13, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, " %d", cpl.stats.skill_level[0]);
    StringBlt(ScreenSurface, &SystemFont, "Ag:", x + 475, y + 26, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 513 - get_string_pixel_length(buf, &SystemFont), y + 26, COLOR_WHITE,
              NULL, NULL);
    sprintf(buf, " %d", cpl.stats.skill_level[2]);
    StringBlt(ScreenSurface, &SystemFont, "Me:", x + 475, y + 38, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 513 - get_string_pixel_length(buf, &SystemFont), y + 38, COLOR_WHITE,
              NULL, NULL);
    sprintf(buf, " %d", cpl.stats.skill_level[4]);
    StringBlt(ScreenSurface, &SystemFont, "Ma:", x + 475, y + 49, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 513 - get_string_pixel_length(buf, &SystemFont), y + 49, COLOR_WHITE,
              NULL, NULL);
    sprintf(buf, " %d", cpl.stats.skill_level[1]);
    StringBlt(ScreenSurface, &SystemFont, "Pe:", x + 475, y + 62, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 513 - get_string_pixel_length(buf, &SystemFont), y + 62, COLOR_WHITE,
              NULL, NULL);
    sprintf(buf, " %d", cpl.stats.skill_level[3]);
    StringBlt(ScreenSurface, &SystemFont, "Ph:", x + 475, y + 74, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 513 - get_string_pixel_length(buf, &SystemFont), y + 74, COLOR_WHITE,
              NULL, NULL);
    sprintf(buf, " %d", cpl.stats.skill_level[5]);
    StringBlt(ScreenSurface, &SystemFont, "Wi:", x + 475, y + 86, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 513 - get_string_pixel_length(buf, &SystemFont), y + 86, COLOR_WHITE,
              NULL, NULL);
    StringBlt(ScreenSurface, &Font6x3Out, "Used", x + 274, y + 78, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &Font6x3Out, "Skill", x + 274, y + 86, COLOR_HGOLD, NULL, NULL);


    multi = line = 0;
    if (cpl.skill_name[0] != 0)
    {
        sprintf(buf, "%s", cpl.skill_name);

        StringBlt(ScreenSurface, &SystemFont, buf, x + 298, y + 78, COLOR_WHITE, NULL, NULL);

        if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
            sprintf(buf, "%d / %-9d", skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level,
                    skill_list[cpl.skill_g].entry[cpl.skill_e].exp);
        else if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp == -2)
            sprintf(buf, "%d / **", skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level);
        else
            sprintf(buf, "** / **");

        StringBlt(ScreenSurface, &SystemFont, buf, x + 298, y + 88, COLOR_WHITE, NULL, NULL);

        if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
        {
            level_exp = skill_list[cpl.skill_g].entry[cpl.skill_e].exp
                        - server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level];
            multi = modf(((double)
                          level_exp
                          / (double)
                          (server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level + 1]
                           - server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level]) * 10.0),
                         &line);
        }
    }
    sprite_blt(Bitmaps[BITMAP_EXP_SKILL_BORDER], x + 413, y + 90, NULL, NULL);

    if (multi)
    {
        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[BITMAP_EXP_SKILL_LINE]->bitmap->h;
        box.w = (int) (Bitmaps[BITMAP_EXP_SKILL_LINE]->bitmap->w * multi);
        if (!box.w)
            box.w = 1;
        if (box.w > Bitmaps[BITMAP_EXP_SKILL_LINE]->bitmap->w)
            box.w = Bitmaps[BITMAP_EXP_SKILL_LINE]->bitmap->w;
        sprite_blt(Bitmaps[BITMAP_EXP_SKILL_LINE], x + 416, y + 97, &box, NULL);
    }

    if (line > 0)
    {
        for (s = 0; s < (int) line; s++)
            sprite_blt(Bitmaps[BITMAP_EXP_SKILL_BUBBLE], x + 416 + s * 5, y + 92, NULL, NULL);
    }

    StringBlt(ScreenSurface, &Font6x3Out, "Regeneration", x + 177, y + 1, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "HP", x + 234, y + 13, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%2.1f", cpl.gen_hp);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 248, y + 13, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &SystemFont, "Mana", x + 178, y + 13, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "Grace", x + 178, y + 24, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%2.1f", cpl.gen_sp);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 208, y + 13, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%2.1f", cpl.gen_grace);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 208, y + 24, COLOR_WHITE, NULL, NULL);

    StringBlt(ScreenSurface, &Font6x3Out, "Level / Exp", x + 177, y + 40, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%d", cpl.stats.level);
    if (cpl.stats.exp_level != cpl.stats.level)
        StringBlt(ScreenSurface, &BigFont, buf, x + 264 - get_string_pixel_length(buf, &BigFont), y + 43, COLOR_RED, NULL, NULL);
    else if (cpl.stats.level == server_level.level)
        StringBlt(ScreenSurface, &BigFont, buf, x + 264 - get_string_pixel_length(buf, &BigFont), y + 43, COLOR_HGOLD, NULL, NULL);
    else
        StringBlt(ScreenSurface, &BigFont, buf, x + 264 - get_string_pixel_length(buf, &BigFont), y + 43, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%d", cpl.stats.exp);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 178, y + 59, COLOR_WHITE, NULL, NULL);

    /* calc the exp bubbles */
    level_exp = cpl.stats.exp - server_level.exp[cpl.stats.exp_level];
    multi = modf(((double) level_exp
                  / (double) (server_level.exp[cpl.stats.exp_level + 1] - server_level.exp[cpl.stats.exp_level]) * 10.0),
                 &line);

    sprite_blt(Bitmaps[BITMAP_EXP_BORDER], x + 182, y + 88, NULL, NULL);
    if (multi)
    {
        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[BITMAP_EXP_SLIDER]->bitmap->h;
        box.w = (int) (Bitmaps[BITMAP_EXP_SLIDER]->bitmap->w * multi);
        if (!box.w)
            box.w = 1;
        if (box.w > Bitmaps[BITMAP_EXP_SLIDER]->bitmap->w)
            box.w = Bitmaps[BITMAP_EXP_SLIDER]->bitmap->w;
        sprite_blt(Bitmaps[BITMAP_EXP_SLIDER], x + 182, y + 88, &box, NULL);
    }


    for (s = 0; s < 10; s++)
        sprite_blt(Bitmaps[BITMAP_EXP_BUBBLE2], x + 183 + s * 8, y + 79, NULL, NULL);
    for (s = 0; s < (int) line; s++)
        sprite_blt(Bitmaps[BITMAP_EXP_BUBBLE1], x + 183 + s * 8, y + 79, NULL, NULL);
}

/* player doll with inventory */
void show_player_doll(int x, int y)
{
    item   *tmp;
    char   *tooltip_text    = NULL;
    char    buf[128];
    int     index, tooltip_index = -1, ring_flag = 0;
    int     mx, my;

    sprite_blt(Bitmaps[BITMAP_DOLL], 0, 0, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_PDOLL2], 0, 194, NULL, NULL);
    if (!cpl.ob)
        return;

    StringBlt(ScreenSurface, &SystemFont, "AC", x + 8, y + 90, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.ac);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 25, y + 90, COLOR_WHITE, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "SF", x + 8, y + 100, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%.1f", cpl.stats.spell_fumble);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 25, y + 100, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &SystemFont, "Speed", x + 8, y + 113, COLOR_HGOLD, NULL, NULL);
	sprintf(buf, "%.1f %%", cpl.stats.speed);
	StringBlt(ScreenSurface, &SystemFont, buf, x + 8, y + 123, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &Font6x3Out, "Melee", x + 153, y + 83, COLOR_HGOLD, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "WC", x + 153, y + 95, COLOR_HGOLD, NULL, NULL);
    StringBlt(ScreenSurface, &SystemFont, "DPS", x + 153, y + 105, COLOR_HGOLD, NULL, NULL);

    sprintf(buf, "%02d", cpl.stats.wc);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 173, y + 95, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%.1f", cpl.stats.dps);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 173, y + 105, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%1.2f sec", cpl.stats.weapon_sp);
    StringBlt(ScreenSurface, &SystemFont, buf, x + 173, y + 115, COLOR_WHITE, NULL, NULL);

	StringBlt(ScreenSurface, &Font6x3Out, "Distance", x + 45, y + 205, COLOR_HGOLD, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "WC", x + 45, y + 217, COLOR_HGOLD, NULL, NULL);
	StringBlt(ScreenSurface, &SystemFont, "DPS", x + 45, y + 227, COLOR_HGOLD, NULL, NULL);

	if(cpl.stats.dist_dps == -0.1f)
	{
		StringBlt(ScreenSurface, &SystemFont, "--", x + 65, y + 217, COLOR_WHITE, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, "--", x + 65, y + 227, COLOR_WHITE, NULL, NULL);
	}
	else if(cpl.stats.dist_dps == -0.2f) /* marks rods/wands/horns */
	{
		StringBlt(ScreenSurface, &SystemFont, "**", x + 65, y + 217, COLOR_WHITE, NULL, NULL);
		StringBlt(ScreenSurface, &SystemFont, "**", x + 65, y + 227, COLOR_WHITE, NULL, NULL);
		sprintf(buf, "%1.2f sec", cpl.stats.dist_time);
		StringBlt(ScreenSurface, &SystemFont, buf, x + 65, y + 237, COLOR_WHITE, NULL, NULL);
	}
	else
	{
		sprintf(buf, "%02d", cpl.stats.dist_wc);
		StringBlt(ScreenSurface, &SystemFont, buf, x + 65, y + 217, COLOR_WHITE, NULL, NULL);
		sprintf(buf, "%.1f", cpl.stats.dist_dps);
		StringBlt(ScreenSurface, &SystemFont, buf, x + 65, y + 227, COLOR_WHITE, NULL, NULL);
		sprintf(buf, "%1.2f sec", cpl.stats.dist_time);
		StringBlt(ScreenSurface, &SystemFont, buf, x + 65, y + 237, COLOR_WHITE, NULL, NULL);
	}


    for (tmp = cpl.ob->inv; tmp; tmp = tmp->next)
    {
        if (tmp->applied)
        {
            index = -1;
            if (tmp->itype == TYPE_ARMOUR)
                index = PDOLL_ARMOUR;
            else if (tmp->itype == TYPE_HELMET)
                index = PDOLL_HELM;
            else if (tmp->itype == TYPE_GIRDLE)
                index = PDOLL_GIRDLE;
            else if (tmp->itype == TYPE_BOOTS)
                index = PDOLL_BOOT;
            else if (tmp->itype == TYPE_WEAPON)
                index = PDOLL_RHAND;
            else if (tmp->itype == TYPE_SHIELD)
                index = PDOLL_LHAND;
            else if (tmp->itype == TYPE_RING)
                index = PDOLL_RRING;
            else if (tmp->itype == TYPE_BRACERS)
                index = PDOLL_BRACER;
            else if (tmp->itype == TYPE_AMULET)
                index = PDOLL_AMULET;
            else if (tmp->itype == TYPE_SHOULDER)
                index = PDOLL_SHOULDER;
            else if (tmp->itype == TYPE_LEGS)
                index = PDOLL_LEGS;
            else if (tmp->itype == TYPE_BOW || tmp->itype == TYPE_WAND ||
					 tmp->itype == TYPE_ROD || tmp->itype == TYPE_HORN || (tmp->itype == TYPE_ARROW && tmp->stype >= 128))
                index = PDOLL_DISTANCE;
            else if (tmp->itype == TYPE_GLOVES)
                index = PDOLL_GAUNTLET;
            else if (tmp->itype == TYPE_CLOAK)
                index = PDOLL_ROBE;
            else if (tmp->itype == TYPE_LIGHT_APPLY)
                index = PDOLL_LIGHT;
            else if (tmp->itype == TYPE_ARROW && tmp->stype < 128)
                index = PDOLL_AMUN;

            if (index == PDOLL_RRING)
                index += ++ring_flag & 1;
            if (index != -1)
            {
                int mb;
                blt_inv_item_centered(tmp, player_doll[index].xpos + x, player_doll[index].ypos + y);
                mb = SDL_GetMouseState(&mx, &my);
                /* prepare item_name tooltip */
                if (mx >= player_doll[index].xpos
                        && mx < player_doll[index].xpos + 33
                        && my >= player_doll[index].ypos
                        && my < player_doll[index].ypos + 33)
                {
                    tooltip_index = index;
                    tooltip_text = tmp->s_name;
                    if ((mb & SDL_BUTTON(SDL_BUTTON_LEFT)) && !draggingInvItem(DRAG_GET_STATUS))
                    {
                        cpl.win_pdoll_tag = tmp->tag;
                        draggingInvItem(DRAG_PDOLL);
                    }
                }
            }
        }
    }
    /* draw a item_name tooltip */
    if (tooltip_index != -1)
        show_tooltip(mx, my, tooltip_text);
}
