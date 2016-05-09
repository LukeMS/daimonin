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

Client_Player   cpl;
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

_player_doll_pos    widget_player_doll[PDOLL_INIT] =
    {
        {93,44}, {93,5}, {93,122}, {93,161}, {50,92},
        {135,92}, {50,131}, {135,131}, {54,48}, {44,7},
        {5,7}, {141,7}, {180,7}, {93,83}, {180,113},
        {132,48}, {5,113}
    };
void clear_player(void)
{
    quickslot_init();
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
    cpl.stats.hptick = cpl.stats.sptick = cpl.stats.gracetick = LastTick;

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
    cpl.action_timer = 0.0f;
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
/* temp handler for prayerbutton in player data */
void widget_player_data_event(int x, int y)
{
    int mx=0, my=0;
    mx = x - cur_widget[PLAYER_INFO_ID].x1;
    my = y - cur_widget[PLAYER_INFO_ID].y1;

    if (mx>=184 && mx <= 210 && my >=5 && my<=35)
    {
        send_game_command("/rest");
    }
}


/* player name, exp, level, titel*/
void widget_show_player_data(int x, int y)
{
    char    buf[256];

    sprite_blt(Bitmaps[BITMAP_PLAYER_INFO], x, y, NULL, NULL);
    if (cpl.rank[0] != 0)
        sprintf(buf, "%s %s\n", cpl.rank, cpl.pname);
    else
        strcpy(buf, cpl.pname);
    string_blt(ScreenSurface, &font_small, buf, x+6, y+2, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%s %s %s", cpl.gender, cpl.race, cpl.title);
    string_blt(ScreenSurface, &font_small, buf, x+6, y+14, COLOR_HGOLD, NULL, NULL);
    if (strcmp(cpl.godname, "none"))
        sprintf(buf, "%s follower of %s", cpl.alignment, cpl.godname);
    else
        strcpy(buf, cpl.alignment);
    string_blt(ScreenSurface, &font_small, buf, x+6, y+26, COLOR_HGOLD, NULL, NULL);

    /* temp prayer button */
    sprite_blt(Bitmaps[BITMAP_PRAY], x+184, y+5, NULL, NULL);
}

/* hp, grace.... */
void widget_player_stats(int x, int y)
{
    char        buf[256];
    double      temp;
    SDL_Rect    box;
    int         mx, my;
    _BLTFX      bltfx;

    SDL_GetMouseState(&mx, &my);

    /* lets look if we have a backbuffer SF, if not create one from the background */
    if (!widgetSF[STATS_ID])
        widgetSF[STATS_ID]=SDL_ConvertSurface(Bitmaps[BITMAP_STATS_BG]->bitmap,Bitmaps[BITMAP_STATS_BG]->bitmap->format,Bitmaps[BITMAP_STATS_BG]->bitmap->flags);

    /* we have a backbuffer SF, test for the redrawing flag and do the redrawing */
    if (cur_widget[STATS_ID].redraw)
    {
        cur_widget[STATS_ID].redraw=0;

        /* we redraw here only all halfway static stuff */
        /* we simply don't need to redraw that stuff every frame, how often the stats change? */

        bltfx.surface=widgetSF[STATS_ID];
        bltfx.flags = 0;
        bltfx.dark_level = 0;
        bltfx.alpha=0;

        sprite_blt(Bitmaps[BITMAP_STATS_BG], 0, 0, NULL, &bltfx);

        string_blt(widgetSF[STATS_ID], &font_tiny_out, "Stats", 8, 1, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%02d", cpl.stats.Str);
        string_blt(widgetSF[STATS_ID], &font_small, "Str", 8, 17, COLOR_WHITE, NULL, NULL);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 30, 17, COLOR_GREEN, NULL, NULL);
        sprintf(buf, "%02d", cpl.stats.Dex);
        string_blt(widgetSF[STATS_ID], &font_small, "Dex", 8, 28, COLOR_WHITE, NULL, NULL);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 30, 28, COLOR_GREEN, NULL, NULL);
        sprintf(buf, "%02d", cpl.stats.Con);
        string_blt(widgetSF[STATS_ID], &font_small, "Con", 8, 39, COLOR_WHITE, NULL, NULL);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 30, 39, COLOR_GREEN, NULL, NULL);
        sprintf(buf, "%02d", cpl.stats.Int);
        string_blt(widgetSF[STATS_ID], &font_small, "Int", 8, 50, COLOR_WHITE, NULL, NULL);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 30, 50, COLOR_GREEN, NULL, NULL);
        sprintf(buf, "%02d", cpl.stats.Wis);
        string_blt(widgetSF[STATS_ID], &font_small, "Wis", 8, 61, COLOR_WHITE, NULL, NULL);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 30, 61, COLOR_GREEN, NULL, NULL);
        sprintf(buf, "%02d", cpl.stats.Pow);
        string_blt(widgetSF[STATS_ID], &font_small, "Pow", 8, 72, COLOR_WHITE, NULL, NULL);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 30, 72, COLOR_GREEN, NULL, NULL);
        sprintf(buf, "%02d", cpl.stats.Cha);
        string_blt(widgetSF[STATS_ID], &font_small, "Cha", 8, 83, COLOR_WHITE, NULL, NULL);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 30, 83, COLOR_GREEN, NULL, NULL);


        string_blt(widgetSF[STATS_ID], &font_small, "HP", 58, 10, COLOR_WHITE, NULL, NULL);
        sprintf(buf, "%d (%d)", cpl.stats.hp, cpl.stats.maxhp);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 160 - string_width(&font_small, buf), 10,
                  COLOR_GREEN, NULL, NULL);
        sprite_blt(Bitmaps[BITMAP_HP_BACK], 57, 23, NULL, &bltfx);


        string_blt(widgetSF[STATS_ID], &font_small, "Mana", 58, 34, COLOR_WHITE, NULL, NULL);
        sprintf(buf, "%d (%d)", cpl.stats.sp, cpl.stats.maxsp);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 160 - string_width(&font_small, buf), 34,
                  COLOR_GREEN, NULL, NULL);
        sprite_blt(Bitmaps[BITMAP_SP_BACK], 57, 47, NULL, &bltfx);

        string_blt(widgetSF[STATS_ID], &font_small, "Grace", 58, 58, COLOR_WHITE, NULL, NULL);
        sprintf(buf, "%d (%d)", cpl.stats.grace, cpl.stats.maxgrace);
        string_blt(widgetSF[STATS_ID], &font_small, buf, 160 - string_width(&font_small, buf), 58,
                  COLOR_GREEN, NULL, NULL);
        sprite_blt(Bitmaps[BITMAP_GRACE_BACK], 57, 71, NULL, &bltfx);

        sprite_blt(Bitmaps[BITMAP_FOOD_BACK], 87, 88, NULL, &bltfx);

        /* the foodstuff is also more or less static, the bar is only updated every second */
        if (cpl.stats.food)
        {
            int bar = BITMAP_FOOD2;
            int tmp = cpl.stats.food;

            if (tmp < 1)
            {
                string_blt(widgetSF[STATS_ID], &font_small, "Food", 58, 84, COLOR_WHITE, NULL, NULL);
                tmp *= -1;
            }
            else if (tmp == 999)
            {
                string_blt(widgetSF[STATS_ID], &font_small, "Rest", 58, 84, COLOR_WHITE, NULL, NULL);
            }
            else
            {
                bar = BITMAP_FOOD;
                string_blt(widgetSF[STATS_ID], &font_small, "Wait", 58, 84, COLOR_WHITE, NULL, NULL);
            }

            tmp++; /* adjust in order to draw the bar correctly */
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
            sprite_blt(Bitmaps[bar], 87, 88, &box, &bltfx);
        }



    }


    /* now we blit our backbuffer SF */
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widgetSF[STATS_ID], NULL, ScreenSurface, &box);



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
        sprite_blt(Bitmaps[BITMAP_HP], x + 57, y + 23, &box, NULL);
    }

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

        temp = tmp / (double) cpl.stats.maxsp;
        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[BITMAP_SP]->bitmap->h;
        box.w = (int) (Bitmaps[BITMAP_SP]->bitmap->w * temp);
        if (tmp && !box.w)
            box.w = 1;
        if (box.w > Bitmaps[BITMAP_SP]->bitmap->w)
            box.w = Bitmaps[BITMAP_SP]->bitmap->w;
        sprite_blt(Bitmaps[BITMAP_SP], x + 57, y + 47, &box, NULL);
    }

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

        temp = (double) tmp / (double) cpl.stats.maxgrace;

        box.x = 0;
        box.y = 0;
        box.h = Bitmaps[BITMAP_GRACE]->bitmap->h;
        box.w = (int) (Bitmaps[BITMAP_GRACE]->bitmap->w * temp);
        if (tmp && !box.w)
            box.w = 1;
        if (box.w > Bitmaps[BITMAP_GRACE]->bitmap->w)
            box.w = Bitmaps[BITMAP_GRACE]->bitmap->w;

        sprite_blt(Bitmaps[BITMAP_GRACE], x + 57, y + 71, &box, NULL);
    }
}

void		widget_menubuttons(int x, int y)
{
    sprite_blt(Bitmaps[BITMAP_MENU_BUTTONS], x, y, NULL, NULL);
}

void        widget_menubuttons_event(int x, int y, int MEvent)
{
    int dx, dy;
    dx=x-cur_widget[MENU_B_ID].x1;
    dy=y-cur_widget[MENU_B_ID].y1;
    if (dx >= 3 && dx <= 44)
    {
        if (dy >= 1 && dy <= 24) /* spell list */
            check_menu_macros("?M_SPELL_LIST");
        else if (dy >= 26 && dy <= 49) /* skill list */
            check_menu_macros("?M_SKILL_LIST");
        else if (dy >= 51 && dy <= 74) /* quest list */
            send_game_command("/qlist");
        else if (dy >= 76 && dy <= 99) /* online help */
            process_macro_keys(KEYFUNC_HELP, 0);
    }
}

void widget_skillgroups(int x, int y)
{
    char        buf[256];
    _BLTFX bltfx;
    SDL_Rect box;

    if (!widgetSF[SKILL_LVL_ID])
        widgetSF[SKILL_LVL_ID]=SDL_ConvertSurface(Bitmaps[BITMAP_SKILL_LVL_BG]->bitmap,
                Bitmaps[BITMAP_SKILL_LVL_BG]->bitmap->format,Bitmaps[BITMAP_SKILL_LVL_BG]->bitmap->flags);

    if (cur_widget[SKILL_LVL_ID].redraw)
    {
        cur_widget[SKILL_LVL_ID].redraw=0;

        bltfx.surface=widgetSF[SKILL_LVL_ID];
        bltfx.flags = 0;
        bltfx.alpha=0;
        sprite_blt(Bitmaps[BITMAP_SKILL_LVL_BG], 0, 0, NULL, &bltfx);

        string_blt(widgetSF[SKILL_LVL_ID], &font_tiny_out, "Skill Groups", 3, 1, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_LVL_ID], &font_tiny_out, "name / level", 3, 13, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[0]);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, "Ag:", 6, 26, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, buf, 44 - string_width(&font_small, buf), 26, COLOR_WHITE,
                  NULL, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[2]);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, "Me:", 6, 38, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, buf, 44 - string_width(&font_small, buf), 38, COLOR_WHITE,
                  NULL, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[4]);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, "Ma:", 6, 49, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, buf, 44 - string_width(&font_small, buf), 49, COLOR_WHITE,
                  NULL, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[1]);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, "Pe:", 6, 62, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, buf, 44 - string_width(&font_small, buf), 62, COLOR_WHITE,
                  NULL, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[3]);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, "Ph:", 6, 74, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, buf, 44 - string_width(&font_small, buf), 74, COLOR_WHITE,
                  NULL, NULL);
        sprintf(buf, " %d", cpl.stats.skill_level[5]);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, "Wi:", 6, 86, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_LVL_ID], &font_small, buf, 44 - string_width(&font_small, buf), 86, COLOR_WHITE,
                  NULL, NULL);
    }
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widgetSF[SKILL_LVL_ID], NULL, ScreenSurface, &box);
}

void widget_show_player_doll_event(int x, int y, int MEvent)
{
    int   old_inv_win = cpl.inventory_win;
    int   old_inv_tag = cpl.win_inv_tag;
    cpl.inventory_win = IWIN_INV;

    if (draggingInvItem(DRAG_GET_STATUS) == DRAG_QUICKSLOT)
    {
        cpl.win_inv_tag = cpl.win_quick_tag;
        if (!(locate_item(cpl.win_inv_tag))->applied)
            process_macro_keys(KEYFUNC_APPLY, 0); /* drop to player-doll */
    }
    else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_INV)
    {
        if ((locate_item(cpl.win_inv_tag))->applied)
            textwin_showstring(COLOR_WHITE, "This is applied already!");
        else
            process_macro_keys(KEYFUNC_APPLY, 0); /* drop to player-doll */
    }
    else if (draggingInvItem(DRAG_GET_STATUS) == DRAG_IWIN_BELOW)
    {
        item *op;

        cpl.inventory_win = IWIN_BELOW;
        sound_play_effect(SOUNDTYPE_CLIENT, SOUND_GET, 0, 0, 100);
        process_macro_keys(KEYFUNC_GET, 0); /* get to inv */

        /* In case object disappears or auto-applies when picked up. */
        if ((op = locate_item(cpl.win_inv_tag)) &&
            !op->applied)
        {
            cpl.inventory_win = IWIN_INV;
	    cpl.win_inv_tag = cpl.win_below_tag;
            process_macro_keys(KEYFUNC_APPLY, 0); /* drop to player-doll */
        }
    }

    cpl.inventory_win = old_inv_win;
    cpl.win_inv_tag = old_inv_tag;
    draggingInvItem(DRAG_NONE);
    itemExamined = 0;

    return;
}


void widget_show_player_doll(int x, int y)
{
    item   *tmp;
    char    buf[512];
    int     index, tooltip_index = -1, ring_flag = 0;
    int     mx, my;

    sprite_blt(Bitmaps[BITMAP_DOLL_BG], x, y, NULL, NULL);

    if (!cpl.ob)
        return;

	string_blt(ScreenSurface, &font_tiny_out, "Melee", x + 5, y + 40, COLOR_HGOLD, NULL, NULL);
	string_blt(ScreenSurface, &font_small, "WC", x + 5, y + 53, COLOR_HGOLD, NULL, NULL);
    string_blt(ScreenSurface, &font_small, "DPS", x + 5, y + 63, COLOR_HGOLD, NULL, NULL);
    string_blt(ScreenSurface, &font_small, "WS", x + 5, y + 73, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.wc);
    string_blt(ScreenSurface, &font_small, buf, x + 25, y + 53, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%.1f", cpl.stats.dps);
    string_blt(ScreenSurface, &font_small, buf, x + 25, y + 63, COLOR_WHITE, NULL, NULL);
    sprintf(buf, "%1.2f", cpl.stats.weapon_sp);
    string_blt(ScreenSurface, &font_small, buf, x + 25, y + 73, COLOR_WHITE, NULL, NULL);

    string_blt(ScreenSurface, &font_small, "AC", x + 180, y + 95, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%02d", cpl.stats.ac);
    string_blt(ScreenSurface, &font_small, buf, x + 195, y + 95, COLOR_WHITE, NULL, NULL);
    string_blt(ScreenSurface, &font_small, "SF", x + 5, y + 95, COLOR_HGOLD, NULL, NULL);
    sprintf(buf, "%.1f", cpl.stats.spell_fumble);
    string_blt(ScreenSurface, &font_small, buf, x + 20, y + 95, COLOR_WHITE, NULL, NULL);

	string_blt(ScreenSurface, &font_small, "Speed", x + 60, y + 167, COLOR_HGOLD, NULL, NULL);
	sprintf(buf, "%.1f%%", cpl.stats.speed);
	string_blt(ScreenSurface, &font_small, buf, x + 130, y + 167, COLOR_WHITE, NULL, NULL);

	string_blt(ScreenSurface, &font_tiny_out, "Distance", x + 170, y + 40, COLOR_HGOLD, NULL, NULL);
	string_blt(ScreenSurface, &font_small, "WC", x + 170, y + 53, COLOR_HGOLD, NULL, NULL);
    string_blt(ScreenSurface, &font_small, "DPS", x + 170, y + 63, COLOR_HGOLD, NULL, NULL);
    string_blt(ScreenSurface, &font_small, "WS", x + 170, y + 73, COLOR_HGOLD, NULL, NULL);

	if(cpl.stats.dist_dps == -0.1f)
	{
		string_blt(ScreenSurface, &font_small, "--", x + 190, y + 53, COLOR_WHITE, NULL, NULL);
		string_blt(ScreenSurface, &font_small, "--", x + 190, y + 63, COLOR_WHITE, NULL, NULL);
	}
	else if(cpl.stats.dist_dps == -0.2f) /* marks rods/wands/horns */
	{
		string_blt(ScreenSurface, &font_small, "**", x + 190, y + 53, COLOR_WHITE, NULL, NULL);
		string_blt(ScreenSurface, &font_small, "**", x + 190, y + 63, COLOR_WHITE, NULL, NULL);
		sprintf(buf, "%1.2f", cpl.stats.dist_time);
		string_blt(ScreenSurface, &font_small, buf, x + 190, y + 73, COLOR_WHITE, NULL, NULL);
	}
	else
	{
		sprintf(buf, "%02d", cpl.stats.dist_wc);
		string_blt(ScreenSurface, &font_small, buf, x + 190, y + 53, COLOR_WHITE, NULL, NULL);
		sprintf(buf, "%.1f", cpl.stats.dist_dps);
		string_blt(ScreenSurface, &font_small, buf, x + 190, y + 63, COLOR_WHITE, NULL, NULL);
		sprintf(buf, "%1.2f", cpl.stats.dist_time);
		string_blt(ScreenSurface, &font_small, buf, x + 190, y + 73, COLOR_WHITE, NULL, NULL);
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
                blt_inv_item_centered(tmp, widget_player_doll[index].xpos + x, widget_player_doll[index].ypos + y);
                mb = SDL_GetMouseState(&mx, &my);
                /* prepare item_name tooltip */
                if (mx >= x+widget_player_doll[index].xpos
                        && mx < x+widget_player_doll[index].xpos + 33
                        && my >= y+widget_player_doll[index].ypos
                        && my < y+widget_player_doll[index].ypos + 33)
                {
                    tooltip_index = index;
                    sprintf(buf,"%s (QC: %d/%d)",tmp->s_name, tmp->item_qua, tmp->item_con);
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
        show_tooltip(mx, my, buf);
}

void widget_show_main_lvl(int x, int y)
{
    char        buf[256];
    double      multi, line;
    SDL_Rect    box;
    int         s, level_exp;
    _BLTFX      bltfx;

    if (!widgetSF[MAIN_LVL_ID])
        widgetSF[MAIN_LVL_ID]=SDL_ConvertSurface(Bitmaps[BITMAP_MAIN_LVL_BG]->bitmap,
                Bitmaps[BITMAP_MAIN_LVL_BG]->bitmap->format,Bitmaps[BITMAP_MAIN_LVL_BG]->bitmap->flags);

    if (cur_widget[MAIN_LVL_ID].redraw)
    {
        cur_widget[MAIN_LVL_ID].redraw=0;

        bltfx.surface=widgetSF[MAIN_LVL_ID];
        bltfx.flags = 0;
        bltfx.alpha=0;

        sprite_blt(Bitmaps[BITMAP_MAIN_LVL_BG], 0, 0, NULL, &bltfx);

        string_blt(widgetSF[MAIN_LVL_ID], &font_tiny_out, "Level / Exp", 4, 1, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%d", cpl.stats.level);
        if (cpl.stats.exp_level != cpl.stats.level)
            string_blt(widgetSF[MAIN_LVL_ID], &font_large_out, buf, 91 - string_width(&font_large_out, buf), 4, COLOR_RED, NULL, NULL);
        else if (cpl.stats.level == MAXLEVEL)
            string_blt(widgetSF[MAIN_LVL_ID], &font_large_out, buf, 91 - string_width(&font_large_out, buf), 4, COLOR_HGOLD, NULL, NULL);
        else
            string_blt(widgetSF[MAIN_LVL_ID], &font_large_out, buf, 91 - string_width(&font_large_out, buf), 4, COLOR_WHITE, NULL, NULL);
        sprintf(buf, "%d", cpl.stats.exp);
        string_blt(widgetSF[MAIN_LVL_ID], &font_small, buf, 5, 20, COLOR_WHITE, NULL, NULL);

        /* calc the exp bubbles */
        level_exp = cpl.stats.exp - server_level.exp[cpl.stats.exp_level];
        multi = modf(((double) level_exp
                      / (double) (server_level.exp[cpl.stats.exp_level + 1] - server_level.exp[cpl.stats.exp_level]) * 10.0),
                     &line);

        sprite_blt(Bitmaps[BITMAP_EXP_BORDER], 9, 49, NULL, &bltfx);
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
            sprite_blt(Bitmaps[BITMAP_EXP_SLIDER], 9, 49, &box, &bltfx);
        }

        for (s = 0; s < 10; s++)
            sprite_blt(Bitmaps[BITMAP_EXP_BUBBLE2], 10 + s * 8, 40, NULL, &bltfx);
        for (s = 0; s < (int) line; s++)
            sprite_blt(Bitmaps[BITMAP_EXP_BUBBLE1], 10 + s * 8, 40, NULL, &bltfx);

    }
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widgetSF[MAIN_LVL_ID], NULL, ScreenSurface, &box);
}

void widget_show_skill_exp(int x, int y)
{
    _BLTFX      bltfx;
	char        buf[256];
    double      multi, line;
    SDL_Rect    box;
    int         s, level_exp;
    long int liLExp = 0;
    long int liLExpTNL = 0;
    long int liTExp = 0;
    long int liTExpTNL = 0;
    float fLExpPercent = 0;
	static int action_tick = 0;

	multi = line = 0;

    /* pre-emptively tick down the skill delay timer */
    if (cpl.action_timer > 0)
    {
        if (LastTick - action_tick > 125)
        {
            cpl.action_timer -= (float) (LastTick - action_tick) / 1000.0f;
            if (cpl.action_timer < 0)
                cpl.action_timer = 0;
            action_tick = LastTick;
            WIDGET_REDRAW(SKILL_EXP_ID);
        }
    }
    else
        action_tick = LastTick;

    if (!widgetSF[SKILL_EXP_ID])
        widgetSF[SKILL_EXP_ID]=SDL_ConvertSurface(Bitmaps[BITMAP_SKILL_EXP_BG]->bitmap,
                Bitmaps[BITMAP_SKILL_EXP_BG]->bitmap->format,Bitmaps[BITMAP_SKILL_EXP_BG]->bitmap->flags);

    if (cur_widget[SKILL_EXP_ID].redraw)
    {
        cur_widget[SKILL_EXP_ID].redraw=0;

        bltfx.surface=widgetSF[SKILL_EXP_ID];
        bltfx.flags = 0;
        bltfx.alpha=0;

        sprite_blt(Bitmaps[BITMAP_SKILL_EXP_BG], 0, 0, NULL, &bltfx);

        string_blt(widgetSF[SKILL_EXP_ID], &font_tiny_out, "Used", 4, -1, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[SKILL_EXP_ID], &font_tiny_out, "Skill", 4, 7, COLOR_HGOLD, NULL, NULL);

        if (cpl.skill_name[0] != 0)
        {
            /* BEGIN robed's exp-Display Patch */
            switch (options.iExpDisplay)
            {
                /* Default */
                default:
                case 0:
                    sprintf(buf, "%s", cpl.skill_name);
                break;

                /* LExp% || LExp/LExp tnl || TExp/TExp tnl || (LExp%) LExp/LExp tnl */
                case 1: case 2: case 3: case 4:
                    if ((skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0) || (skill_list[cpl.skill_g].entry[cpl.skill_e].exp == -2))
                        sprintf(buf, "%s - level: %d", cpl.skill_name, skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level);
                    else
                        sprintf(buf, "%s - level: **", cpl.skill_name);
                break;
            }
            string_blt(widgetSF[SKILL_EXP_ID], &font_small, buf, 28, -1, COLOR_WHITE, NULL, NULL);

            if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
            {
                level_exp = skill_list[cpl.skill_g].entry[cpl.skill_e].exp - server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level];
                multi = modf(((double)level_exp/(double)
                           (server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level+1]-server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level])*10.0), &line);

                liTExp    = skill_list[cpl.skill_g].entry[cpl.skill_e].exp;
                liTExpTNL = server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level + 1];

                liLExp    = liTExp    - server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level];
                liLExpTNL = liTExpTNL - server_level.exp[skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level];

                fLExpPercent = ((float) liLExp / (float) (liLExpTNL)) * 100.0f;
            }

            switch (options.iExpDisplay)
            {
                /* Default */
                default:
                case 0:
                    if(skill_list[cpl.skill_g].entry[cpl.skill_e].exp >=0)
                        sprintf(buf, "%d / %-9d", skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level,skill_list[cpl.skill_g].entry[cpl.skill_e].exp );
                    else if(skill_list[cpl.skill_g].entry[cpl.skill_e].exp == -2)
                        sprintf(buf, "%d / **", skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level );
                    else
                        sprintf(buf, "** / **");
                break;

                /* LExp% */
                case 1:
                    if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        sprintf(buf, "%#05.2f%%", fLExpPercent);
                    else
                        sprintf(buf, "**.**%%");
                break;

                /* LExp/LExp tnl */
                case 2:
                    if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        sprintf(buf, "%ld / %ld", liLExp, liLExpTNL);
                    else
                        sprintf(buf, "** / **");
                break;

                /* TExp/TExp tnl */
                case 3:
                    if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        sprintf(buf, "%ld / %ld", liTExp, liTExpTNL);
                    else
                        sprintf(buf, "** / **");
                break;

                /* (LExp%) LExp/LExp tnl */
                case 4:
                    if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp >= 0)
                        sprintf(buf, "%#05.2f%% - %ld", fLExpPercent, liLExpTNL - liLExp);
                    else
                        sprintf(buf, "(**.**%%) **");
                break;
            }
            if (skill_list[cpl.skill_g].entry[cpl.skill_e].exp_level==MAXLEVEL)
                sprintf(buf, "more levels in 2 weeks (tm)");
            string_blt(widgetSF[SKILL_EXP_ID], &font_small, buf, 28, 9, COLOR_WHITE, NULL, NULL);
            sprintf(buf, "%1.2f sec", cpl.action_timer);
            string_blt(widgetSF[SKILL_EXP_ID], &font_small, buf, 160, -1, COLOR_WHITE, NULL, NULL);
            /* END robed's exp-display-Patch */
        }
        sprite_blt(Bitmaps[BITMAP_EXP_SKILL_BORDER], 143, 11, NULL, &bltfx);

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
            sprite_blt(Bitmaps[BITMAP_EXP_SKILL_LINE], 146, 18, &box, &bltfx);
        }

        if (line > 0)
        {
            for (s = 0; s < (int) line; s++)
                sprite_blt(Bitmaps[BITMAP_EXP_SKILL_BUBBLE], 146 + s * 5, 13, NULL, &bltfx);
        }
    }
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widgetSF[SKILL_EXP_ID], NULL, ScreenSurface, &box);
}

void widget_skill_exp_event(int x, int y, int MEvent)
{
   /* make a button for robed's exp-patch */
    int i, ii, j, jj, bFound = 0;

    /* lets find the skill... and setup the shortcuts to the exp values */
    for (ii = 0; ii <= SKILL_LIST_MAX && (!bFound); ii++)
    {
        jj = cpl.skill_g + ii;
        if (jj >= SKILL_LIST_MAX)
            jj -= SKILL_LIST_MAX;

        for (i = 0; i < DIALOG_LIST_ENTRY && (!bFound); i++)
        {
            // First page, we have to be offset (and break before looping)
            if (ii == 0)
            {
                j = cpl.skill_e + i + 1;
                if (j >= DIALOG_LIST_ENTRY)
                    break;
            }
            // Other pages we look through MUST NOT BE OFFSET
            else
                j = i;

            if (j >= DIALOG_LIST_ENTRY)
                j -= DIALOG_LIST_ENTRY;

            /* we have a list entry */
            if (skill_list[jj].entry[j].flag == LIST_ENTRY_KNOWN)
            {
                /* First one we find is the one we want */
                sprintf(cpl.skill_name, "%s", skill_list[jj].entry[j].name);
                cpl.skill_g = jj;
                cpl.skill_e = j;
                bFound = 1;
                break;
            }
        }
    }
    WIDGET_REDRAW(SKILL_EXP_ID);
}


void widget_show_regeneration(int x, int y)
{
    char        buf[256];
    SDL_Rect    box;
    _BLTFX      bltfx;

    if (!widgetSF[REGEN_ID])
        widgetSF[REGEN_ID]=SDL_ConvertSurface(Bitmaps[BITMAP_REGEN_BG]->bitmap,
                Bitmaps[BITMAP_REGEN_BG]->bitmap->format,Bitmaps[BITMAP_REGEN_BG]->bitmap->flags);

    if (cur_widget[REGEN_ID].redraw)
    {
        cur_widget[REGEN_ID].redraw=0;

        bltfx.surface=widgetSF[REGEN_ID];
        bltfx.flags = 0;
        bltfx.alpha=0;

        sprite_blt(Bitmaps[BITMAP_REGEN_BG], 0, 0, NULL, &bltfx);

        string_blt(widgetSF[REGEN_ID], &font_tiny_out, "Regeneration", 4, 1, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[REGEN_ID], &font_small, "HP", 61, 13, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%2.1f", cpl.gen_hp);
        string_blt(widgetSF[REGEN_ID], &font_small, buf, 75, 13, COLOR_WHITE, NULL, NULL);

        string_blt(widgetSF[REGEN_ID], &font_small, "Mana", 5, 13, COLOR_HGOLD, NULL, NULL);
        string_blt(widgetSF[REGEN_ID], &font_small, "Grace", 5, 24, COLOR_HGOLD, NULL, NULL);
        sprintf(buf, "%2.1f", cpl.gen_sp);
        string_blt(widgetSF[REGEN_ID], &font_small, buf, 35, 13, COLOR_WHITE, NULL, NULL);
        sprintf(buf, "%2.1f", cpl.gen_grace);
        string_blt(widgetSF[REGEN_ID], &font_small, buf, 35, 24, COLOR_WHITE, NULL, NULL);
    }
    box.x=x;
    box.y=y;
    SDL_BlitSurface(widgetSF[REGEN_ID], NULL, ScreenSurface, &box);
}

void widget_show_statometer(int x, int y)
{
    char statbuf[128];

    if (!options.statsupdate)
    {
        cur_widget[STATOMETER_ID].show=0;
        return;
    }

    string_blt(ScreenSurface, &font_large_out, "Stat-O-Meter:", x+2, y+2, COLOR_BLUE,NULL,NULL);
    sprintf(statbuf,"EXP: %d",statometer.exp);
    string_blt(ScreenSurface, &font_small,statbuf,x+2,y+15,COLOR_WHITE,NULL,NULL);
    sprintf(statbuf,"(%.2f/hour)",statometer.exphour);
    string_blt(ScreenSurface, &font_small,statbuf,x+82,y+15,COLOR_WHITE,NULL,NULL);
    sprintf(statbuf,"Kills: %d",statometer.kills);
    string_blt(ScreenSurface, &font_small,statbuf,x+2,y+25,COLOR_WHITE,NULL,NULL);
    sprintf(statbuf,"(%.2f/hour)",statometer.killhour);
    string_blt(ScreenSurface, &font_small,statbuf,x+82,y+25,COLOR_WHITE,NULL,NULL);
}
