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

int                     global_group_status;
struct _group           group[GROUP_MAX_MEMBER];
char                    group_invite[32]; /* name of player who has send the invite */
int                     group_count = 0;

static int              group_pos[GROUP_MAX_MEMBER][2]      =
    {
        {5,29}, {5,53}, {5,77}, {5,101}, {5,125}, {5,149}
    };

/*  widget_show_group()
 *  widgetized version of show_group with dynamic resizing
 *  generate the group window.
 *  Show data & names of group members and options.
 */
void widget_show_group(int x, int y)
{
    int         s, len, mb, mx, my;
    SDL_Rect    box;
    char        buf[256];

    /*first we need to calculate the size for the window */
    box.x = 0;
    box.y = 0;
    box.w = 120;
    if (global_group_status < GROUP_INVITE)
        box.h = 27;
    else if (global_group_status == GROUP_INVITE || global_group_status == GROUP_WAIT)
        box.h = 135;
    else
        box.h = (group_count*24)+31;

    cur_widget[GROUP_ID].ht = box.h+4;
    sprite_blt(Bitmaps[BITMAP_GROUP_BG], x, y, &box, NULL);
    sprite_blt(Bitmaps[BITMAP_GROUP_BG_BOTTOM],x,y+box.h, NULL, NULL);

    StringBlt(ScreenSurface, &Font6x3Out, "Group:", x +50, y+2 , COLOR_HGOLD, NULL, NULL);


    if (global_group_status < GROUP_INVITE)
    {
        StringBlt(ScreenSurface, &Font6x3Out, "type '/help group' for info", x+13, y+13, COLOR_WHITE, NULL, NULL);
        return;
    }


    mb = SDL_GetMouseState(&mx, &my);
    if (global_group_status == GROUP_INVITE || global_group_status == GROUP_WAIT)
    {
        sprite_blt(Bitmaps[BITMAP_GROUP_INVITE], x + 10, y +32, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "GROUP INVITE", x+30, y+13,COLOR_GREEN, NULL, NULL);
        len =  get_string_pixel_length(group_invite, &SystemFont);
        StringBlt(ScreenSurface, &SystemFont, group_invite, x + 60-len/2, y + 45, COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "has invited you", x + 28, y +65, COLOR_DEFAULT, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "to join a group.", x + 28, y +78, COLOR_DEFAULT, NULL, NULL);

        if (global_group_status == GROUP_INVITE)
        {
            if (add_button(x + 4 , y + 110, 101, BITMAP_BUTTON_BLACK_UP, "join", "join"))
            {
                global_group_status = GROUP_WAIT;
                send_command("/join", -1, SC_NORMAL);
            }
            if (add_button(x + 61, y + 110, 102, BITMAP_BUTTON_BLACK_UP, "deny", "deny"))
            {
                global_group_status = GROUP_NO;
                send_command("/deny", -1, SC_NORMAL);
            }
        }
    }
    else /* status: GROUP_MEMBER */
    {
        if (add_button(x+4, y + 7, 103, BITMAP_SMALL_UP, "leave", "leave"))
        {
            if (global_group_status != GROUP_LEAVE)
            {
                global_group_status = GROUP_LEAVE;
                send_command("/leave", -1, SC_NORMAL);
            }
        }

        for (s = 0; s < GROUP_MAX_MEMBER; s++)
        {
            /* sprite_blt(Bitmaps[BITMAP_GROUP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 1, NULL, NULL); */
            if (group[s].name[0] != '\0')
            {
                sprite_blt(Bitmaps[BITMAP_GROUP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 1, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, group[s].name, x + group_pos[s][0] + 33, y + group_pos[s][1] + 1,COLOR_DEFAULT, NULL, NULL);
                sprintf(buf, "%3d", group[s].level);
                StringBlt(ScreenSurface, &Font6x3Out, buf, x + group_pos[s][0] + 8, y + group_pos[s][1], COLOR_DEFAULT, NULL, NULL);

                if (group[s].maxhp)
                {
                    int tmp = group[s].hp;
                    double temp;

                    if (tmp < 0)
                        tmp = 0;
                    temp = (double) tmp / (double) group[s].maxhp;
                    box.x = 0;
                    box.y = 0;
                    box.h = Bitmaps[BITMAP_GROUP_HP]->bitmap->h;
                    box.w = (int) (Bitmaps[BITMAP_GROUP_HP]->bitmap->w * temp);
                    if (tmp && !box.w)
                        box.w = 1;
                    if (box.w > Bitmaps[BITMAP_GROUP_HP]->bitmap->w)
                        box.w = Bitmaps[BITMAP_GROUP_HP]->bitmap->w;
                    sprite_blt(Bitmaps[BITMAP_GROUP_HP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 17, &box, NULL);
                }
                if (group[s].maxsp)
                {
                    int tmp = group[s].sp;
                    double temp;

                    if (tmp < 0)
                        tmp = 0;
                    temp = (double) tmp / (double) group[s].maxsp;
                    box.x = 0;
                    box.y = 0;
                    box.h = Bitmaps[BITMAP_GROUP_MANA]->bitmap->h;
                    box.w = (int) (Bitmaps[BITMAP_GROUP_MANA]->bitmap->w * temp);
                    if (tmp && !box.w)
                        box.w = 1;
                    if (box.w > Bitmaps[BITMAP_GROUP_MANA]->bitmap->w)
                        box.w = Bitmaps[BITMAP_GROUP_MANA]->bitmap->w;
                    sprite_blt(Bitmaps[BITMAP_GROUP_MANA], x + group_pos[s][0] + 2, y + group_pos[s][1] + 19, &box, NULL);
                }
                if (group[s].maxgrace)
                {
                    int tmp = group[s].grace;
                    double temp;

                    if (tmp < 0)
                        tmp = 0;
                    temp = (double) tmp / (double) group[s].maxgrace;
                    box.x = 0;
                    box.y = 0;
                    box.h = Bitmaps[BITMAP_GROUP_GRACE]->bitmap->h;
                    box.w = (int) (Bitmaps[BITMAP_GROUP_GRACE]->bitmap->w * temp);
                    if (tmp && !box.w)
                        box.w = 1;
                    if (box.w > Bitmaps[BITMAP_GROUP_GRACE]->bitmap->w)
                        box.w = Bitmaps[BITMAP_GROUP_GRACE]->bitmap->w;
                    sprite_blt(Bitmaps[BITMAP_GROUP_GRACE], x + group_pos[s][0] + 2, y + group_pos[s][1] + 21, &box, NULL);
                }
            }
        }
    }
}




/*  show_group()
 *  generate the group window.
 *  Show data & names of group members and options.
 */
void show_group(int x, int y)
{
    int         s, len, mb, mx, my;
    SDL_Rect    box;
    char        buf[256];


    /*
    for (s = 0; s < GROUP_MAX_MEMBER; s++)
        sprite_blt(Bitmaps[BITMAP_GROUP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 1, NULL, NULL);
    s=0;
    sprite_blt(Bitmaps[BITMAP_GROUP_HP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 17, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_GROUP_MANA], x + group_pos[s][0] + 2, y + group_pos[s][1] + 19, NULL, NULL);
    sprite_blt(Bitmaps[BITMAP_GROUP_GRACE], x + group_pos[s][0] + 2, y + group_pos[s][1] + 21, NULL, NULL);
    */

    if (global_group_status < GROUP_INVITE)
    {
        StringBlt(ScreenSurface, &Font6x3Out, "type '/help group' for info", 40, Screensize.yoff+585, COLOR_WHITE, NULL, NULL);
        return;
    }


    mb = SDL_GetMouseState(&mx, &my);
    if (global_group_status == GROUP_INVITE || global_group_status == GROUP_WAIT)
    {
        sprite_blt(Bitmaps[BITMAP_GROUP_INVITE], x + group_pos[0][0] + 2, y + group_pos[0][1] + 1, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, "GROUP INVITE", x + group_pos[0][0] + 76, y + group_pos[0][1] + 5,COLOR_GREEN, NULL, NULL);
        len =  get_string_pixel_length(group_invite, &SystemFont);
        StringBlt(ScreenSurface, &SystemFont, group_invite, x + group_pos[0][0]+107-len/2, y + group_pos[0][1] + 19,COLOR_HGOLD, NULL, NULL);
        StringBlt(ScreenSurface, &SystemFont, " has invited you to join a group.", x + group_pos[0][0] + 40, y + group_pos[0][1] + 31,COLOR_DEFAULT, NULL, NULL);

        if (global_group_status == GROUP_INVITE)
        {
            if (add_button(x + group_pos[0][0] + 40, y + group_pos[0][1] + 48, 101, BITMAP_BUTTON_BLACK_UP, "join", "join"))
            {
                global_group_status = GROUP_WAIT;
                send_command("/join", -1, SC_NORMAL);
            }
            if (add_button(x + group_pos[0][0] + 120, y + group_pos[0][1] + 48, 102, BITMAP_BUTTON_BLACK_UP, "deny", "deny"))
            {
                global_group_status = GROUP_NO;
                send_command("/deny", -1, SC_NORMAL);
            }
        }
    }
    else /* status: GROUP_MEMBER */
    {
        if (add_button(x, y + 56, 103, BITMAP_SMALL_UP, "leave", "leave"))
        {
            if (global_group_status != GROUP_LEAVE)
            {
                global_group_status = GROUP_LEAVE;
                send_command("/leave", -1, SC_NORMAL);
            }
        }

        for (s = 0; s < GROUP_MAX_MEMBER; s++)
        {
            /* sprite_blt(Bitmaps[BITMAP_GROUP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 1, NULL, NULL); */
            if (group[s].name[0] != '\0')
            {
//                sprite_blt(Bitmaps[BITMAP_GROUP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 1, NULL, NULL);
                StringBlt(ScreenSurface, &SystemFont, group[s].name, x + group_pos[s][0] + 33, y + group_pos[s][1] + 1,COLOR_DEFAULT, NULL, NULL);
                sprintf(buf, "%3d", group[s].level);
                StringBlt(ScreenSurface, &Font6x3Out, buf, x + group_pos[s][0] + 8, y + group_pos[s][1], COLOR_DEFAULT, NULL, NULL);

                if (group[s].maxhp)
                {
                    int tmp = group[s].hp;
                    double temp;

                    if (tmp < 0)
                        tmp = 0;
                    temp = (double) tmp / (double) group[s].maxhp;
                    box.x = 0;
                    box.y = 0;
                    box.h = Bitmaps[BITMAP_GROUP_HP]->bitmap->h;
                    box.w = (int) (Bitmaps[BITMAP_GROUP_HP]->bitmap->w * temp);
                    if (tmp && !box.w)
                        box.w = 1;
                    if (box.w > Bitmaps[BITMAP_GROUP_HP]->bitmap->w)
                        box.w = Bitmaps[BITMAP_GROUP_HP]->bitmap->w;
                    sprite_blt(Bitmaps[BITMAP_GROUP_HP], x + group_pos[s][0] + 2, y + group_pos[s][1] + 17, &box, NULL);
                }
                if (group[s].maxsp)
                {
                    int tmp = group[s].sp;
                    double temp;

                    if (tmp < 0)
                        tmp = 0;
                    temp = (double) tmp / (double) group[s].maxsp;
                    box.x = 0;
                    box.y = 0;
                    box.h = Bitmaps[BITMAP_GROUP_MANA]->bitmap->h;
                    box.w = (int) (Bitmaps[BITMAP_GROUP_MANA]->bitmap->w * temp);
                    if (tmp && !box.w)
                        box.w = 1;
                    if (box.w > Bitmaps[BITMAP_GROUP_MANA]->bitmap->w)
                        box.w = Bitmaps[BITMAP_GROUP_MANA]->bitmap->w;
                    sprite_blt(Bitmaps[BITMAP_GROUP_MANA], x + group_pos[s][0] + 2, y + group_pos[s][1] + 19, &box, NULL);
                }
                if (group[s].maxgrace)
                {
                    int tmp = group[s].grace;
                    double temp;

                    if (tmp < 0)
                        tmp = 0;
                    temp = (double) tmp / (double) group[s].maxgrace;
                    box.x = 0;
                    box.y = 0;
                    box.h = Bitmaps[BITMAP_GROUP_GRACE]->bitmap->h;
                    box.w = (int) (Bitmaps[BITMAP_GROUP_GRACE]->bitmap->w * temp);
                    if (tmp && !box.w)
                        box.w = 1;
                    if (box.w > Bitmaps[BITMAP_GROUP_GRACE]->bitmap->w)
                        box.w = Bitmaps[BITMAP_GROUP_GRACE]->bitmap->w;
                    sprite_blt(Bitmaps[BITMAP_GROUP_GRACE], x + group_pos[s][0] + 2, y + group_pos[s][1] + 21, &box, NULL);
                }
            }
        }
    }
}

void clear_group(void)
{
    global_group_status = GROUP_NO;
    memset(group, 0, sizeof(group));
    group_count = 0;
}

void set_group(int slot, char *name, int level, int hp, int maxhp, int sp, int maxsp, int grace, int maxgrace)
{
    if (name)
        strcpy(group[slot].name, name);
    group[slot].level = level;
    group[slot].hp = hp;
    group[slot].maxhp = maxhp;
    group[slot].sp = sp;
    group[slot].maxsp = maxsp;
    group[slot].grace = grace;
    group[slot].maxgrace = maxgrace;
}
