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

#if !defined(__GROUP_H)
#define __GROUP_H

/* define the group status.
 * never show a invite when we are a member.
 * (should be done by server too)
 * status WAIT & LEAVE is needed to handle
 * the asynchron server/client handling of leaving/joining
 */
#define GROUP_NO     0
#define GROUP_INVITE 1
#define GROUP_LEAVE  2
#define GROUP_WAIT   3
#define GROUP_MEMBER 4

#define GROUP_MAX_MEMBER 6

typedef struct _group
{
    char name[32];
    int  level;
    int  hp;
    int  maxhp;
    int  sp;
    int  maxsp;
    int  grace;
    int  maxgrace;
}
_groups;

extern int           global_group_status;
extern struct _group group[GROUP_MAX_MEMBER];
extern char          group_invite[32]; /* name of player who has send the invite */
extern int           group_count;      /* we need the player count for dynamic window resizing */

extern void show_group(int x, int y);
extern void widget_show_group(int x, int y);
extern void clear_group(void);
extern void set_group(int slot, char *name, int level, int hp, int maxhp, int sp, int maxsp, int grace, int maxgrace);

#endif
