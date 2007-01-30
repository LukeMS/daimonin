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
#ifndef __AGGRO_H
#define __AGGRO_H

/**
 * DEFAULT_DMG_INVALID_TIME determins the maximum number of ticks for a damage to stay valid.
 * If the age of a damage exceeds DEFAULT_DMG_INVALID_TIME, the damage is invalid.
 * Currently, it's regarded that 1000 ticks is the maximum age for a damage.
 */
#define DEFAULT_DMG_INVALID_TIME 1000

extern struct obj *aggro_get_damage(struct obj *target, struct obj *hitter);
extern struct obj *aggro_insert_damage(struct obj *target, struct obj *hitter);
extern struct obj *aggro_update_info(struct obj *target, struct obj *target_owner,
                                     struct obj *hitter, struct obj *hitter_object, int dmg, int flags);
extern struct obj *aggro_calculate_exp(struct obj *victim, struct obj *slayer, char *kill_msg);

#endif
