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

/* attack_melee_weapon() - this handles melee weapon attacks -b.t.
 * For now we are just checking to see if we have a ready weapon here.
 * But there is a real neato possible feature of this scheme which
 * bears mentioning:
 * Since we are only calling this from do_skill() in the future
 * we may make this routine handle 'special' melee weapons attacks
 * (like disarming manuever with sai) based on player SK_level and
 * weapon type.
 */

int attack_melee_weapon(object *op, int dir, char *string)
{
    if (!QUERY_FLAG(op, FLAG_READY_WEAPON))
    {
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "You have no ready weapon to attack with!");
        return 0;
    }

    return skill_attack(NULL, op, dir, string);
}

/* attack_hth() - this handles all hand-to-hand attacks -b.t. */

/* July 5, 1995 - I broke up attack_hth() into 2 parts. In the first
 * (attack_hth) we check for weapon use, etc in the second (the new
 * function skill_attack() we actually attack.
 */

int attack_hth(object *pl, int dir, char *string)
{
    object*enemy =  NULL, *weapon;

    if (QUERY_FLAG(pl, FLAG_READY_WEAPON))
        for (weapon = pl->inv; weapon; weapon = weapon->below)
        {
            if (weapon->type != WEAPON || !QUERY_FLAG(weapon, FLAG_APPLIED))
                continue;
            CLEAR_FLAG(weapon, FLAG_APPLIED);
            CLEAR_FLAG(pl, FLAG_READY_WEAPON);
            FIX_PLAYER(pl ,"attack hth");
            new_draw_info(NDI_UNIQUE, 0, pl, "You unwield your weapon in order to attack.");
            esrv_update_item(UPD_FLAGS, weapon);

            break;
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
 * Initial implementation by -bt thomas@astro.psu.edu
 */

int skill_attack(object *tmp, object *pl, int dir, char *string)
{
    int         xt, yt;
    mapstruct  *m;

    if (!dir)
        dir = pl->facing;

    /* If we don't yet have an opponent, find if one exists, and attack.
     * Legal opponents are the same as outlined in move_player()
     */

    if (tmp == NULL)
    {
        xt = pl->x + freearr_x[dir];
        yt = pl->y + freearr_y[dir];
        if (!(m = out_of_map(pl->map, &xt, &yt)))
            return 0;

        /* rewrite this for new "head only" multi arches and battlegrounds. MT. */
        for (tmp = GET_MAP_OB(m, xt, yt); tmp; tmp = tmp->above)
        {
            if ((IS_LIVE(tmp) && (tmp->head == NULL ? tmp->stats.hp > 0 : tmp->head->stats.hp > 0))
             || QUERY_FLAG(tmp, FLAG_CAN_ROLL) || tmp->type == LOCKED_DOOR)
            {
                /* lets skip pvp outside battleground (pvp area) */
                if (pl->type == PLAYER && tmp->type == PLAYER && !op_on_battleground(tmp, NULL, NULL))
                    continue;
                break;
            }
        }
    }
    if (tmp != NULL)
        return do_skill_attack(tmp, pl, string);

    if (pl->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, pl, "There is nothing to attack!");

    return 0;
}

/* do_skill_attack() - We have got an appropriate opponent from either
 * move_player() or skill_attack(). In this part we get on with
 * attacking, take care of messages from the attack and changes in invisible.
 * Returns true if the attack damaged the opponent.
 * -b.t. thomas@astro.psu.edu
 */

int do_skill_attack(object *tmp, object *op, char *string)
{
    int     success;
    char    buf[MEDIUM_BUF];
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

    if (success && string != NULL)
    {
        sprintf(buf, "%s", string);
        if (op->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, op, "You %s %s!", buf, query_name_full(tmp, NULL));
        else if (tmp->type == PLAYER)
            new_draw_info(NDI_UNIQUE, 0, tmp, "%s %s you!", query_name_full(op, NULL), buf);
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
int SK_level(object *op)
{
    object *head    = op->head ? op->head : op;
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
int find_traps(object *op, int level)
{
    uint8 i,
          found = 0,
          aware = 0;

    /* Search the squares in the 8 directions. */
    for (i = 0; i < 9; i++)
    {
        int         xt = op->x + freearr_x[i],
                    yt = op->y + freearr_y[i];
        mapstruct  *m;
        object     *next,
                   *this;

        /* Ensure the square isn't out of bounds. */
        if (!(m = out_of_map(op->map, &xt, &yt)))
            continue;
 
        next = GET_MAP_OB(m, xt, yt);
        while ((this = next))
        {
            /* this is the object on the map, that is the current object under
             * consideration. */
            object *that = this;

            next = this->above;

            /* op, players, and monsters are opaque to find traps. */
            if (that == op || that->type == PLAYER || that->type == MONSTER)
                continue;

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
                    else
                        if (that->level <= (level * 1.8f))
                            aware = 1;
                }
                that = find_next_object(that, RUNE, FNO_MODE_CONTAINERS, that);
            }
        }
    }

   /* Only players get messages. */
    if (op->type == PLAYER && CONTR(op))
    {
        if (!found)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You find no new traps this time...");
            if (aware)
                new_draw_info(NDI_UNIQUE, 0, op, "But you find signs of traps hidden beyond your skill...");
        }
        else
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You find %d new traps!",
                                                      found);
            if (aware)
                new_draw_info(NDI_UNIQUE, 0, op, "You also find signs of more traps hidden beyond your skill...");
        }
    }

    return 0;
}

/* remove_trap() - This skill will disarm any previously discovered trap
 * the algorithm is based (almost totally) on the old command_disarm() - b.t.
 */

int remove_trap(object *op, int dir, int level)
{
    object     *tmp, *tmp2;
    mapstruct  *m;
    int         i, x, y;

    for (i = 0; i < 9; i++)
    {
        x = op->x + freearr_x[i];
        y = op->y + freearr_y[i];
        if (!(m = out_of_map(op->map, &x, &y)))
            continue;

        /*  Check everything in the square for trapness */
        for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
        {
            /* And now we'd better do an inventory traversal of each
                       * of these objects' inventory */

            for (tmp2 = tmp->inv; tmp2 != NULL; tmp2 = tmp2->below)
            {
                if (tmp2->type == RUNE && tmp2->stats.Cha <= 1)
                {
                    if (QUERY_FLAG(tmp2, FLAG_SYS_OBJECT) || QUERY_FLAG(tmp2, FLAG_IS_INVISIBLE))
                        trap_show(tmp2, tmp);
                    trap_disarm(op, tmp2, 1);
                    return 0;
                }
            }
            if (tmp->type == RUNE && tmp->stats.Cha <= 1)
            {
                if (QUERY_FLAG(tmp, FLAG_SYS_OBJECT) || QUERY_FLAG(tmp, FLAG_IS_INVISIBLE))
                    trap_show(tmp, tmp);
                trap_disarm(op, tmp, 1);
                return 0;
            }
        }
    }
    new_draw_info(NDI_UNIQUE, 0, op, "You have found no nearby traps to remove yet!");
    return 0;
}

