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

/* TODO: Filename: sparkly.c. */
/* Sparklies are temporary visual/sonic special effects that happen when
 * something (usually a player or monster) has achieved some milestone (such as
 * levelling up) or is in a process with a delayed outcome (you'll see later).
 *
 * So a sparkly is an indicator that something has or is about to happen.
 *
 * Sparklies have a visual element and optionally a sonic one too.
 *
 * A sparkly is associated with the object which has done or is doing. So if
 * the object moves during the sparklies existence, the sparkly does too; and
 * if the object is removed, the sparkly is too. Sparklies only exist if/while
 * their associated object is on a map. */
/* TODO: Sparklies currently only work properly on singleparts. */
/* TODO: Sparklies are visual, informational effects for players only -- they
 * have no relevance to or effect on actual gameplay otherwise. As such they
 * are FLAG_NO_SEND (means no data is sent to thte cclient so they never appear
 * in the below window). */

#include "global.h"

/* sparkly_create() creates a sparkly from at. The sparkly is associated with
 * who and is inserted at the same map location. It lasts for t ticks (if t<=0,
 * the arch default is used. If nr>=0 this sound effect is played (stype is the
 * sound tyoe which currently the server requires but will be removed
 * later). */
object_t *sparkly_create(archetype_t *at, object_t *who, sint16 t, sint16 nr, uint8 stype)
{
    object_t *sparkly;

    if (!at ||
        !who ||
        !who->map ||
        !(sparkly = arch_to_object(at)))
    {
        return NULL;
    }

    sparkly->x = who->x;
    sparkly->y = who->y;
    sparkly = insert_ob_in_map(sparkly, who->map, NULL, INS_NO_MERGE | INS_NO_WALK_ON);

    if (sparkly)
    {
        set_owner(sparkly, who);

        if (t > 0)
        {
            sparkly->stats.food = t;
        }

        if (nr >= 0)
        {
            play_sound_map(MSP_KNOWN(who), nr, stype);
        }
    }

    return sparkly;
}

/* sparkly_move() checks that sparkly is still on its owner's msp, (re)moving
 * it as necessary. */
void sparkly_move(object_t *sparkly)
{
    object_t *owner = get_owner(sparkly);

    if (!owner ||
        !owner->map)
    {
        remove_ob(sparkly);
    }
    else if (sparkly->map != owner->map ||
             sparkly->x != owner->x ||
             sparkly->y != owner->y)
    {
        remove_ob(sparkly);
        sparkly->x = owner->x;
        sparkly->y = owner->y;
        (void)insert_ob_in_map(sparkly, owner->map, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
    }

    return;
}
