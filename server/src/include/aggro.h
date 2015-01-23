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

#ifndef __AGGRO_H
#define __AGGRO_H

/** lets say that 1000 ticks is a maximum range for damage.
 * if a damage info is older, is automatically invalid.
 */
#define DEFAULT_DMG_INVALID_TIME 1000

/** Returns the damage from a target for a damage source hitter.
 * This is used for AoE spells and other target synchronized damage dealers.
 * @param target Target.
 * @param hitter Hitter.
 * @return damage info object for hitter or <code>NULL</code>
 */
extern object_t *aggro_get_damage(object_t *target, object_t *hitter);

extern object_t *aggro_insert_damage(object_t *target, object_t *hitter);

/** Updates the damage and aggro marker for a target after the aggro and damage is done from the hitter to the target.
 * @param target Target to update.
 * @param hitter Hitter to get information from.
 * @param hitter_object Object the hitter used to cause the damage (?).
 * @param dmg Damage that was done.
 * @return aggro or <code>NULL</code> if not available.
 */
extern object_t *aggro_update_info(object_t *target, object_t *hitter, object_t *hitter_object, int dmg);

/** Calculates the experience for an aggro.
 * Analyzes all aggro info in the supplied object and gives the player experience based on this information.
 * @param victim
 * @param slayer
 * @param kill_msg
 * @return
 */
extern object_t *aggro_calculate_exp(object_t *victim, object_t *slayer, char *kill_msg);

#endif /* ifndef __AGGRO_H */
