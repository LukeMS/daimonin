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

#include "global.h"

static object_t *IsValidTarget(object_t *what, object_t*who);

/* attack_melee_weapon() - this handles melee weapon attacks -b.t.
 * For now we are just checking to see if we have a ready weapon here.
 * But there is a real neato possible feature of this scheme which
 * bears mentioning:
 * Since we are only calling this from do_skill() in the future
 * we may make this routine handle 'special' melee weapons attacks
 * (like disarming manuever with sai) based on player SK_level and
 * weapon type.
 */

int attack_melee_weapon(object_t *op, int dir, char *string)
{
    if (!QUERY_FLAG(op, FLAG_READY_WEAPON))
    {
        if (op->type == PLAYER)
            ndi(NDI_UNIQUE, 0, op, "You have no ready weapon to attack with!");
        return 0;
    }

    return skill_attack(NULL, op, dir, string);
}

/* attack_hth() - this handles all hand-to-hand attacks -b.t. */

/* July 5, 1995 - I broke up attack_hth() into 2 parts. In the first
 * (attack_hth) we check for weapon use, etc in the second (the new
 * function skill_attack() we actually attack.
 */

int attack_hth(object_t *pl, int dir, char *string)
{
    object_t *enemy = NULL;

    if (QUERY_FLAG(pl, FLAG_READY_WEAPON))
    {
        object_t *weapon,
               *next;

        FOREACH_OBJECT_IN_OBJECT(weapon, pl, next)
        {
            if (weapon->type != WEAPON ||
                !QUERY_FLAG(weapon, FLAG_APPLIED))
            {
                continue;
            }

            CLEAR_FLAG(weapon, FLAG_APPLIED);
            CLEAR_FLAG(pl, FLAG_READY_WEAPON);
            FIX_PLAYER(pl ,"attack hth");
            ndi(NDI_UNIQUE, 0, pl, "You unwield your weapon in order to attack.");
            esrv_update_item(UPD_FLAGS, weapon);
            break;
        }
    }

    return skill_attack(enemy, pl, dir, string);
}

/* skill_attack() - Core routine for use when we attack using a skills
 * system. There are'nt too many changes from before, basically this is
 * a 'wrapper' for the old attack system. In essence, this code handles
 * all skill-based attacks, ie hth, missile and melee weapons should be
 * treated here. If an opponent is already supplied by move_player(),
 * we move right onto do_skill_attack(), otherwise we find if an
 * appropriate opponent exists.
 *
 * This is called by move_player() and attack_hth()
 *
 * Initial implementation by -bt thomas@astro.psu.edu */
int skill_attack(object_t *tmp, object_t *pl, int dir, char *string)
{
    if (!dir)
    {
        dir = pl->facing;
    }

    /* If we don't yet have an opponent, find if one exists, and attack.
     * Legal opponents are the same as outlined in move_player(). */
    if (!tmp)
    {
        map_t *m = pl->map;
        sint16     x = pl->x + OVERLAY_X(dir),
                   y = pl->y + OVERLAY_Y(dir);
        msp_t  *msp = MSP_GET(m, x, y);
        object_t    *this,
                  *next;

        if (!msp)
        {
            return 0;
        }

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            object_t *head = (this->head) ? this->head : this;

            if ((tmp = IsValidTarget(head, pl)))
            {
                break;
            }
        }
    }
    else
    {
        tmp = IsValidTarget(tmp, pl);
    }

    if (tmp)
    {
        return do_skill_attack(tmp, pl, string);
    }
    else if (pl->type == PLAYER)
    {
        ndi(NDI_UNIQUE, 0, pl, "There is nothing to attack!");
    }

    return 0;
}

/* IsValidTarget() returns what if what is attackable by who or NULL. */
static object_t *IsValidTarget(object_t *what, object_t*who)
{
    /* PvP is not available unless both players are in PvP msps. */
    if (who->type == PLAYER &&
        what->type == PLAYER &&
        (!(MSP_KNOWN(who)->flags & MSP_FLAG_PVP) ||
         !(MSP_KNOWN(what)->flags & MSP_FLAG_PVP)))
    {
        return NULL;
    }

    /* We make no judgment on whether who should be attacking what based on
     * relative alignments, powers, etc (that should have been caught long
     * before this stage), just on whether it is technically possible for what
     * to be attacked. */
    /* TODO: Not sure the FLAG_CAN_ROLL/LOCKED_DOOR stuff is implemented. */
    if ((IS_LIVE(what) &&
         what->stats.hp > 0) ||
        QUERY_FLAG(what, FLAG_CAN_ROLL) ||
        what->type == LOCKED_DOOR)
    {
        return what;
    }

    return NULL;
}

/* do_skill_attack() - We have got an appropriate opponent from either
 * move_player() or skill_attack(). In this part we get on with
 * attacking, take care of messages from the attack and changes in invisible.
 * Returns true if the attack damaged the opponent.
 * -b.t. thomas@astro.psu.edu
 */

int do_skill_attack(object_t *tmp, object_t *op, char *string)
{
    int     success;
    float   ticks = 0.0f;

    if (op->type == PLAYER)
    {
        if (!CONTR(op)->selected_weapon) /* ok... lets change to our hth skill */
        {
            if (CONTR(op)->skill_weapon)
            {
                if (change_skill_to_skill(op, CONTR(op)->skill_weapon))
                {
                    LOG(llevBug, "BUG: do_skill_attack() could'nt give new hth skill to %s\n", STRING_OBJ_NAME(op));
                    return 0;
                }
            }
            else
            {
                LOG(llevBug, "BUG: do_skill_attack(): no hth skill in player %s\n", STRING_OBJ_NAME(op));
                return 0;
            }
        }
        /* if we have 'ready weapon' but no 'melee weapons' skill readied
         * this will flip to that skill. This is only window dressing for
         * the players--no need to do this for monsters.
         */
        if (QUERY_FLAG(op, FLAG_READY_WEAPON)
             && (!op->chosen_skill || op->chosen_skill->stats.sp != CONTR(op)->set_skill_weapon))
        {
            change_skill(op, CONTR(op)->set_skill_weapon);
        }
    }

    success = attack_ob(tmp, op, NULL);

    /* print appropriate  messages to the player */
    if (success &&
        string)
    {
        if (op->type == PLAYER)
         {
            ndi(NDI_UNIQUE, 0, op, "You %s %s!",
                string, QUERY_SHORT_NAME(tmp, op));
        }
        else if (tmp->type == PLAYER)
        {          
            ndi(NDI_UNIQUE, 0, tmp, "%s %s you!",
                QUERY_SHORT_NAME(op, tmp), string);
        }
    }

    /* set the skill delay from the attack so we can't use other skills during the cooldown time */
    if (op->type == PLAYER)
    {
        ticks = FABS(CONTR(op)->ob->weapon_speed);
        LOG(llevDebug, "AC-melee: %2.2f\n", ticks);
        set_action_time(op, ticks);
    }

    return success;
}


/* This is in the same spirit as the similar routine for spells
 * it should be used anytime a function needs to check the user's
 * level.
 */
int SK_level(object_t *op)
{
    object_t *head    = op->head ? op->head : op;
    int     level;

    if (head->type == PLAYER && head->chosen_skill && head->chosen_skill->level != 0)
    {
        level = head->chosen_skill->level;
    }
    else
    {
        level = head->level;
    }

    if (level <= 0)
    {
        LOG(llevBug, "BUG: SK_level(arch %s, name %s): level <= 0\n", op->arch->name, STRING_OBJ_NAME(op));
        level = 1;   /* safety */
    }

    return level;
}

/* The FIND_TRAPS skill. This routine is taken mostly from the
 * command_search loop. It seemed easier to have a separate command,
 * rather than overhaul the existing code - this makes sure things
 * still work for those people who don't want to have skill code
 * implemented. */
int find_traps(object_t *op, int level)
{
    uint8 i,
          found = 0,
          aware = 0;

    /* Search the squares in the 8 directions. */
    for (i = 0; i < 9; i++)
    {
        map_t  *m = op->map;
        sint16      x = op->x + OVERLAY_X(i),
                    y = op->y + OVERLAY_Y(i);
        msp_t   *msp = MSP_GET(m, x, y);
        object_t     *next,
                   *this;

        /* Ensure the square isn't out of bounds. */
        if (!msp)
        {
            continue;
        }
 
        next = msp->last;

        while ((this = next))
        {
            /* this is the object on the map, that is the current object under
             * consideration. */
            object_t *that = this;

            next = this->below;

            /* op, players, and monsters are opaque to find traps. */
            if (that == op ||
                IS_LIVE(that))
            {
                continue;
            }

            /* Otherwise, check that and (if necessary) inventory of that. */
            while (that)
            {
                if (that->type == RUNE && that->stats.Cha > 1)
                {
                    if (trap_see(op, that, level))
                    {
                        trap_show(that, this);
                        found++;
                    }
                    else if (that->level <= (level * 1.8f))
                    {
                        aware = 1;
                    }
                }

                that = find_next_object(that, RUNE, FNO_MODE_CONTAINERS, that);
            }
        }
    }

   /* Only players get messages. */
    if (op->type == PLAYER &&
        CONTR(op))
    {
        if (!found)
        {
            ndi(NDI_UNIQUE, 0, op, "You find no new traps this time%s...",
                (aware) ? ", but you find signs of traps hidden beyond your skill" : "");
        }
        else
        {
            ndi(NDI_UNIQUE, 0, op, "You find %d new traps%s!",
                found,
                (aware) ? " and signs of more traps hidden beyond your skill" : "");
        }
    }

    return 0;
}

/* remove_trap() - This skill will disarm any previously discovered trap
 * the algorithm is based (almost totally) on the old command_disarm() - b.t.
 */

int remove_trap(object_t *op, int dir, int level)
{
    uint8 i;

    for (i = 0; i < 9; i++)
    {
        map_t *m = op->map;
        sint16     x = op->x + OVERLAY_X(i),
                   y = op->y + OVERLAY_Y(i);
        msp_t  *msp = MSP_GET(m, x, y);
        object_t    *this,
                  *next;

        if (!msp)
        {
            continue;
        }

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            object_t *that,
                   *next2;

            FOREACH_OBJECT_IN_OBJECT(that, this, next2)
            {
                if (that->type == RUNE &&
                    that->stats.Cha <= 1)
                {
                    this = that;
                    break;
                }
            }

            if (this->type == RUNE &&
                this->stats.Cha <= 1)
            {
                if (QUERY_FLAG(this, FLAG_SYS_OBJECT) ||
                    QUERY_FLAG(this, FLAG_IS_INVISIBLE))
                {
                    trap_show(this, this);
                }

                trap_disarm(op, this, 1);
                return 0;
            }
        }
    }

    ndi(NDI_UNIQUE, 0, op, "You have found no nearby traps to remove yet!");
    return 0;
}

