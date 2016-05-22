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

/* TODO: Filename: door.c. */
/* Doors are map objects which have two states, closed or open. When closed,
 * they block passage and optionally view. When open, these blocks are removed.
 * A door is generally opened by a player or monster moving onto the same
 * square. It'll then remain open for a short time until it automatically
 * closes.
 *
 * Doors may be locked, in which case the potential opener will need a matching
 * key or marker force in his inv. Attempting to open a locked door with the
 * key will generate a message.
 *
 * Doors may be trapped, in which case the trap(s) will spring on the opener.
 *
 * While a door itself should be singlepart only, it is common practice to have
 * two side-by-side to create a double door (three or more looks a little
 * strange because of the hinges but for gameplay purposes these are ignored
 * anyway). Since 0.10.7 multiparts can move through doors if their is enough
 * room. */
/* ->slaying = non-NULL if door is lockef, NULL if it is not
 * FLAG_CURSED = if also locked, cannot be picked UNIMPLEMENTED
 * ->msg = themessage when a locked door is tried without success
 * ->inv = traps go here
 * ->state = 0 if door is closed, 1 if door is open, more or less standard use
 * ->stats.sp = max seconds until open door autocloses
 * ->last_sp = current seconds until open door autocloses
 * ->speed, ->speed_left = standard activelist stuff (needed for autoclosing)
 * FLAG_BLOCKSVIEW, FLAG_ALLOWSVIEW, FLAG_OBSCURESVIEW = standard use
 * ->stats.grace = 0 if closed door does not block, allow, or obscure view, 1-3
 * if it does
 * FLAG_DOOR_CLOSED = standard use
 * ->last_grace = 0 if door is not closed, 1 if it is
 * FLAG_IS_TURNABLE, FLAG_ANIMATE) = standard use, should definitely be at
 * least one of these
 * ->direction = standard use
 * ->sub_type1 = determines if/which sound is played when changing state */
/* TODO: I don't think pass_thru/pass_ethereal is/was ever handled properly/at
 * all. */
/* TODO: Door traps are a one-shot deal which doesn't always make sense in a
 * multiplayer game. */
/* TODO: Should doors be attackable? Ie, no key to this locked door -- smash it
 * down! */
/* TODO: Trapdoors have similarities to doors and use the same base attributes
 * and functions. However, they are in active (re)development, will be
 * committed soon. The current implementation is poor/broken so DO NOT USE. */

#include "global.h"

/* door_find_key() browse the inv of who (assumed to be PLAYER or MONSTER) and
 * any CONTAINER in there for a KEY or FORCE with a matching ->slaying to that
 * of what. The first match is returned if found, or NULL.
 *
 * Although this function is primarily used for TYPE_DOOR, it is also used
 * for CONTAINER and TRAPDOOR.
 *
 * Both who and what must be non-NULL. The precise types are flexible as noted
 * above. */
object_t *door_find_key(object_t *who, object_t *what)
{
    object_t *this,
             *next,
             *key;

    FOREACH_OBJECT_IN_OBJECT(this, who, next)
    {
        if (this->type == TYPE_KEY ||
            this->type == FORCE)
        {
            if (this->slaying == what->slaying)
            {
                return this;
            }
        }
        /* FIXME: This search loop, in common with others that descend into
         * containers, does not respect locked containers. Ironic.
         *
         * -- Smacky 20151210 */
        else if (this->type == CONTAINER &&
            this->inv &&
            (key = door_find_key(this, what)))
        {
            return key;
        }
    }

    return NULL;
}

/* door_open() checks if who can open the door (what) and/or actually does it.
 *
 * who may be NULL. Othwerwise it is assumed who will be either PLAYER or
 * MONSTER but this is probably not terribly important.
 *
 * what must be TYPE_DOOR.
 *
 * mode should be either DOOR_MODE_TEST or DOOR_MODE_OPEN depending on what is
 * being attempted.
 *
 * The return is 0 if it is not possible for who to open what (either mode) or
 * 1 if who opening what is possible (in test mode) or was actually done (in
 * open mode).
 *
 * It is the caller's responsibility to QUERY_FLAG(who, FLAG_CAN_OPEN_DOOR), if
 * appropriate (it usually is) before calling this function. */
sint8 door_open(object_t *who, object_t *what, uint8 mode)
{
    object_t *key = NULL;

    /* Sanity checks. */
    if (!what ||
        what->type != TYPE_DOOR)
    {
        return 0;
    }

    /* If the door is locked, does who have the right key? Note that if who is
     * NULL this check is skipped. */
    if (who &&
        what->slaying)
    {
        object_t *key = door_find_key(who, what);

        if (!key)
        {
            if (mode == DOOR_MODE_OPEN &&
                who->type == PLAYER)
            {
                ndi(NDI_UNIQUE | NDI_NAVY, 0, who, "%s",
                    what->msg);
            }

            return 0; /* we can't open it! */
        }
        else if (key->type == TYPE_KEY &&
            mode == DOOR_MODE_OPEN &&
            who->type == PLAYER)
        {
            ndi(NDI_UNIQUE | NDI_BROWN, 0, who, "You unlock %s with %s.",
                QUERY_SHORT_NAME(what, who), QUERY_SHORT_NAME(key, who));
        }
    }

    /* If this is just testing mode, that's it; who CAN open this door. */
    if (mode == DOOR_MODE_TEST)
    {
        return 1;
    }

    /* Is the door trapped? Note that if who is NULL this check is skipped. */
    if (who &&
        what->inv)
    {
        object_t *this,
                 *next;

        FOREACH_OBJECT_IN_OBJECT(this, what, next)
        {
            if (this->type == RUNE &&
                this->level)
            {
                spring_trap(this, who);
            }
        }
    }

    /* To trigger all the updates/changes on map and for player, we
     * remove and reinsert it. a bit overhead but its secure and simple. */
    remove_ob(what);
    move_check_off(what, NULL, MOVE_FLAG_VANISHED);

    /* Now we update and reinsert it. */
    //what->state = 1;
    what->direction = absdir(what->direction + 1);
    what->last_sp = what->stats.sp;
    what->speed = 0.125f;
    what->speed_left = -0.250f;
    update_ob_speed(what);

    if (QUERY_FLAG(what, FLAG_OBSCURESVIEW))
    {
        what->stats.grace = 3;
    }
    else if (QUERY_FLAG(what, FLAG_ALLOWSVIEW))
    {
        what->stats.grace = 2;
    }
    else if (QUERY_FLAG(what, FLAG_BLOCKSVIEW))
    {
        what->stats.grace = 1;
    }
    else
    {
        what->stats.grace = 0;
    }

    CLEAR_FLAG(what, FLAG_OBSCURESVIEW);
    CLEAR_FLAG(what, FLAG_ALLOWSVIEW);
    CLEAR_FLAG(what, FLAG_BLOCKSVIEW);

    if (QUERY_FLAG(what, FLAG_DOOR_CLOSED))
    {
        CLEAR_FLAG(what, FLAG_DOOR_CLOSED);
        what->last_grace = 1;
    }
    else
    {
        what->last_grace = 0;
    }

    if (QUERY_FLAG(what, FLAG_IS_TURNABLE) ||
        QUERY_FLAG(what, FLAG_ANIMATE))
    {
        SET_ANIMATION(what, (NUM_ANIMATIONS(what) / NUM_FACINGS(what)) * what->direction + what->state);
    }

    insert_ob_in_map(what, what->map, what, 0);

    /* Give sound confirmation as appropriate. */
    if (what->sub_type1 == ST1_DOOR_NORMAL)
    {
        play_sound_map(MSP_KNOWN(what), SOUND_OPEN_DOOR, SOUND_NORMAL);
    }

    /* All done; the door is open. */
    return 1;
}

/* door_close() autocloses TYPE_DOOR.
 *
 * what must be TYPE_DOOR. */
void door_close(object_t *what)
{
    /* Sanity checks. */
    if (!what ||
        what->type != TYPE_DOOR)
    {
        return;
    }

    /* Decrement the ticker. Only when this is <= 0 do we consider closing. */
    if (what->last_sp-- > 0)
    {
        return;
    }

    /* Something in the way? Remain open a bit longer. */
    if ((msp_blocked(NULL, what->map, what->x, what->y) & (MSP_FLAG_NO_PASS | MSP_FLAG_ALIVE | MSP_FLAG_PLAYER)))
    {
        what->last_sp = what->stats.sp;
        return;
    }

    /* To trigger all the updates/changes on map and for player, we
     * remove and reinsert it. a bit overhead but its secure and simple. */
    remove_ob(what);
    move_check_off(what, NULL, MOVE_FLAG_VANISHED);

    /* Now we update and reinsert it. */
    //what->state = 0;
    what->direction = absdir(what->direction - 1);
    what->speed = what->speed_left = 0.0f;
    update_ob_speed(what);

    if (what->stats.grace == 3)
    {
        CLEAR_FLAG(what, FLAG_BLOCKSVIEW);
        CLEAR_FLAG(what, FLAG_ALLOWSVIEW);
        SET_FLAG(what, FLAG_OBSCURESVIEW);
        what->stats.grace = 0;
    }
    else if (what->stats.grace == 2)
    {
        CLEAR_FLAG(what, FLAG_BLOCKSVIEW);
        SET_FLAG(what, FLAG_ALLOWSVIEW);
        CLEAR_FLAG(what, FLAG_OBSCURESVIEW);
        what->stats.grace = 0;
    }
    else if (what->stats.grace == 1)
    {
        SET_FLAG(what, FLAG_BLOCKSVIEW);
        CLEAR_FLAG(what, FLAG_ALLOWSVIEW);
        CLEAR_FLAG(what, FLAG_OBSCURESVIEW);
        what->stats.grace = 0;
    }

    if (what->last_grace == 0)
    {
        CLEAR_FLAG(what, FLAG_DOOR_CLOSED);
    }
    else
    {
        SET_FLAG(what, FLAG_DOOR_CLOSED);
        what->last_grace = 0;
    }

    if (QUERY_FLAG(what, FLAG_IS_TURNABLE) ||
        QUERY_FLAG(what, FLAG_ANIMATE))
    {
        SET_ANIMATION(what, (NUM_ANIMATIONS(what) / NUM_FACINGS(what)) * what->direction + what->state);
    }

    insert_ob_in_map(what, what->map, what, 0);

    /* Give sound confirmation as appropriate. */
    if (what->sub_type1 == ST1_DOOR_NORMAL)
    {
        play_sound_map(MSP_KNOWN(what), SOUND_DOOR_CLOSE, SOUND_NORMAL);
    }
}
