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

    The author can be reached via e-mail to daimonin@nord-com.net
*/

#include <global.h>

static int push_internal(object *op, char *params, int dir)
{
    push_roll_object(op, dir, TRUE);
    return 0;
}

int command_push_object (object *op, char *params)
{
    sint8 dir = op->facing;

    return push_internal(op, params, dir);
}

int command_turn_right (object *op, char *params)
{
    sint8 dir = absdir(op->facing + 1);

    op->anim_last_facing = op->anim_last_facing_last = op->facing = dir;

    return 1;
}

int command_turn_left (object *op, char *params)
{
    sint8 dir = absdir(op->facing - 1);

    op->anim_last_facing = op->anim_last_facing_last = op->facing = dir;

    return 1;
}
