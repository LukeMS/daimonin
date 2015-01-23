/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001 Michael Toennies

    A split from Crossfire, a Multiplayer game for X-windows.

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

#ifndef __EXP_H
#define __EXP_H

/* MAXLEVEL is defined in protocol.h */
#define MAXMOBLEVEL 127 // must be >= MAXLEVEL

typedef struct _level_color
{
    int green;
    int blue;
    int yellow;
    int orange;
    int red;
    int purple;
} _level_color;

extern sint32       new_levels[MAXLEVEL + 2];
extern _level_color level_color[MAXMOBLEVEL + 1];
extern int exp_from_base_skill(player_t *pl, int base_exp, int sk);

#endif /* ifndef __EXP_H */
