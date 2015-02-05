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

/*
 * Routines that is executed from objects based on their speed have been
 * collected in this file.
 */

#include <global.h>

static object_t *CheckForDuplicate(object_t *what, msp_t *msp);

/* search op for the needed key to open door.
 * This function does really not more give back a useable key ptr
 * or NULL - it don't open, delete or doing any other action.
 */
object_t * find_key(object_t *op, object_t *door)
{
    object_t *tmp,
           *next,
           *key;

    /* First, lets try to find a key in the top level inventory */
    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        if (tmp->type == SPECIAL_KEY && tmp->slaying == door->slaying)
            return tmp;

        /* we brute force us through every CONTAINER inventory.*/
        if (tmp->type == CONTAINER && tmp->inv)
        {
            if ((key = find_key(tmp, door)) != NULL)
                return key;
        }
    }
    return NULL;
}

/* Grommit 31-Jul-2007
 * Function to look for marker force with slaying field the same as
 * the locked door. A mark should allow the player to open the door.
 * I don't want to mess with find_key.
 * We only need to look in the top level inventory.
 */
object_t * find_force(object_t *op, object_t *door)
{
    object_t *tmp,
           *next;

    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        if ((tmp->type == FORCE) && (tmp->slaying == door->slaying))
            return tmp;
    }
    return NULL;
}

/* this is our main open_door() function. It is used for doors
 * which will auto open/close and/or need a special key. It is
 * used from npc, mobs and players and use the remove_doorX()
 * functions below.
 * Calling function must QUERY_FLAG(op, FLAG_CAN_OPEN_DOOR).
 * 0:door is NOT opened and not possible to open from op. 1: door was opened.
 * op: object which will open a door on map m, position x,y
 * mode: 0 - check but don't open the door. 1: check and open the door when possible
 */
int open_door(object_t *op, msp_t *msp, int mode)
{
    object_t *this,
           *next,
           *key = NULL,
           *force = NULL;

    /* Search for door across all layers. */
    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (this->type == LOCKED_DOOR)
        {
            if (this->slaying) /* door needs a key? */
            {
                if (!(key = find_key(op, this)) &&
                    !(force = find_force(op, this)))
                {
                    if (op->type == PLAYER &&
                        mode)
                    {
                        ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "%s",
                            this->msg);
                    }

                    return 0; /* we can't open it! */
                }
            }

            /* are we here, the door can be opened 100% */
            if (mode) /* really opening the door? */
            {
                remove_door2(this, op);

                if (op->type == PLAYER)
                {
                    if (key)
                    {
                        ndi(NDI_UNIQUE, NDI_BROWN, op, "You open the door with %s.",
                            QUERY_SHORT_NAME(key, op));
                    }

                    /* Remove force message - it's usually inappropriate to say anything
                     * when opening with a hidden marker (one-way doors for example) */
                    /* else if (force)
                     *    ndi(NDI_UNIQUE, NDI_BROWN, op, "You force the door open.");
                     */
                }
            }

            return 1;
        }
    }

#if 0
    /* we should not be here... We have a misplaced door_closed flag
     * or a door on a wrong layer... both is not good, so drop a bug msg.
     */
    LOG(llevSystem,
        "BUG: open_door() - door on wrong layer or misplaced MSP_FLAG_DOOR_CLOSED flag - map:%s (%d,%d) (op: %s)\n", m->path,
        x, y, STRING_OBJ_NAME(op));
#endif
    return 0;
}

/* The following removes doors.  The functions check to see if similar
 * doors are next to the one that is being removed, and if so, set it
 * so those will be removed shortly (in a cascade like fashion.)
 */

/* this is OLD door - atm i have NOT any of them in the arches of daimonin.
 * but this doors can be tricky used like cascading openings and so on.
 * So i let them in.
 */
/* the "cascading trick" works very simple - if we are here, we search for similiar
 * attached doors. if we find them, we give them speed. That means they will in some
 * ticks processed by active object process function. This funcion will come back here
 * and we go on and on - this will give the effect thats around a whole map a doorwall
 * will open one after one.
 */
/* i think i will add a "LOCKED_DOOR_CHAINED" type or something. for normal doors,
 * i don't want this cascading - but its a nice tricky map manipulation trick which
 * opens some fine fun or map makers.
 */
void remove_door(object_t *op)
{
    int     i;
    object_t *tmp;
    for (i = 1; i < 9; i += 2)
        if ((tmp = present(DOOR, op->map, op->x + OVERLAY_X(i), op->y + OVERLAY_Y(i))) != NULL)
        {
            tmp->speed = 0.1f;
            update_ob_speed(tmp);
            tmp->speed_left = -0.2f;
        }

    if (op->other_arch)
    {
        tmp = arch_to_object(op->other_arch);
        tmp->x = op->x;tmp->y = op->y;tmp->map = op->map;tmp->level = op->level;
        tmp->direction = op->direction;
        if (QUERY_FLAG(tmp, FLAG_IS_TURNABLE) || QUERY_FLAG(tmp, FLAG_ANIMATE))
            SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction + tmp->state);
        insert_ob_in_map(tmp, op->map, op, 0);
    }
    if (op->sub_type1 == ST1_DOOR_NORMAL)
        play_sound_map(MSP_KNOWN(op), SOUND_OPEN_DOOR, SOUND_NORMAL);
    remove_ob(op);
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
}

void remove_door2(object_t *op, object_t *opener)
{
    /* - NO cascading for locked doors!
    int i;
    for(i=1;i<9;i+=2) {
      tmp=present(LOCKED_DOOR,op->map,op->x+OVERLAY_X(i),op->y+OVERLAY_Y(i));
      if(tmp && tmp->slaying == op->slaying)
    {
        tmp->speed = 0.1f;
        update_ob_speed(tmp);
        tmp->speed_left= -0.2f;
      }
    }*/

    /* mow 2 ways to handle open doors.
     * a.) if other_arch is set, we insert that object and remove the old door.
     * b.) if not set, we toggle from close to open when needed.
     */

    if (op->other_arch) /* if set, just exchange and delete old door */
    {
        object_t *tmp = arch_to_object(op->other_arch);

        tmp->state = 0; /* 0= closed, 1= opened */
        tmp->x = op->x;tmp->y = op->y;tmp->map = op->map;tmp->level = op->level;
        tmp->direction = op->direction;
        if (QUERY_FLAG(tmp, FLAG_IS_TURNABLE) || QUERY_FLAG(tmp, FLAG_ANIMATE))
            SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction + tmp->state);
        insert_ob_in_map(tmp, op->map, op, 0);
        if (op->sub_type1 == ST1_DOOR_NORMAL)
            play_sound_map(MSP_KNOWN(op), SOUND_OPEN_DOOR, SOUND_NORMAL);
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
    }
    else if (!op->last_eat) /* if set, we are have opened a closed door - now handle autoclose */
    {
        object_t *this,
               *next;

        remove_ob(op); /* to trigger all the updates/changes on map and for player, we
                         * remove and reinsert it. a bit overhead but its secure and simple
                         */
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        FOREACH_OBJECT_IN_OBJECT(this, op, next)
        {
            if (this->type == RUNE &&
                this->level)
            {
                spring_trap(this, opener);
            }
        }

        op->last_eat = 1; /* mark this door as "its open" */
        op->speed = 0.1f;       /* put it on active list, so it will close automatically */
        update_ob_speed(op);
        op->speed_left = -0.2f;
        op->state = 1; /* change to "open door" faces */
        op->last_sp = op->stats.sp; /* init "open" counter */
        /* save and clear blocksview and no_pass */
        QUERY_FLAG(op, FLAG_BLOCKSVIEW) ? (op->stats.grace = 1) : (op->stats.grace = 0);
        QUERY_FLAG(op, FLAG_DOOR_CLOSED) ? (op->last_grace = 1) : (op->last_grace = 0);
        CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
        CLEAR_FLAG(op, FLAG_DOOR_CLOSED);
        if (QUERY_FLAG(op, FLAG_IS_TURNABLE) || QUERY_FLAG(op, FLAG_ANIMATE))
            SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
        if (op->sub_type1 == ST1_DOOR_NORMAL)
            play_sound_map(MSP_KNOWN(op), SOUND_OPEN_DOOR, SOUND_NORMAL);
        insert_ob_in_map(op, op->map, op, 0);
    }
}

/* thats called from time.c - here we handle autoclosing doors */
void remove_door3(object_t *op)
{
    if (!op->last_eat) /* thats a bug - active speed but not marked as active */
    {
        LOG(llevBug, "BUG: door has speed but is not marked as active. (%s - map:%s (%d,%d))\n", STRING_OBJ_NAME(op),
            op->map ? op->map->name : "(no map name!)", op->x, op->y);
        op->last_eat = 0; /* thats not a real fix ... */
        return;
    }
    if (!op->map) /* ouch */
    {
        LOG(llevBug, "BUG: door with speed but no map?! killing object...done. (%s - (%d,%d))\n", STRING_OBJ_NAME(op), op->x,
            op->y);
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }

    /* now check or counter - if not <=0 we still be open */
    if (op->last_sp-- > 0)
        return;
    /* now we try to close the door. The rule is:
     * is the tile of the door not blocked by a no_pass object OR a player OR a mob -
     * then close the door.
     * IF it is blocked - then restart a new "is open" phase.
     */

    if (msp_blocked(NULL, op->map, op->x, op->y) & (MSP_FLAG_NO_PASS | MSP_FLAG_ALIVE | MSP_FLAG_PLAYER))
    {
        /* let it open one more round */
        op->last_sp = op->stats.sp; /* reinit "open" counter */
    }
    else /* ok - NOW we close it */
    {
        remove_ob(op); /* to trigger all the updates/changes on map and for player, we
                         * remove and reinsert it. a bit overhead but its secure and simple
                         */
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        op->last_eat = 0; /* mark this door as "its closed" */
        op->speed = 0.0f;   /* remove from active list */
        op->speed_left = 0.0f;
        update_ob_speed(op);
        op->state = 0; /* change to "close door" faces */
        op->stats.grace == 1 ? SET_FLAG(op, FLAG_BLOCKSVIEW) : CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
        op->last_grace == 1 ? SET_FLAG(op, FLAG_DOOR_CLOSED) : CLEAR_FLAG(op, FLAG_DOOR_CLOSED);
        op->stats.grace = 0;op->last_grace = 0;
        if (QUERY_FLAG(op, FLAG_IS_TURNABLE) || QUERY_FLAG(op, FLAG_ANIMATE))
            SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
        if (op->sub_type1 == ST1_DOOR_NORMAL)
            play_sound_map(MSP_KNOWN(op), SOUND_DOOR_CLOSE, SOUND_NORMAL);
        insert_ob_in_map(op, op->map, op, 0);
    }
}

void regenerate_rod(object_t *rod)
{
    if (++rod->stats.food > rod->stats.hp / 10 || rod->type == HORN)
    {
        rod->stats.food = 0;
        if (rod->stats.hp < rod->stats.maxhp)
        {
            rod->stats.hp += 1 + rod->stats.maxhp / 10;
            if (rod->stats.hp > rod->stats.maxhp)
                rod->stats.hp = rod->stats.maxhp;
            fix_rod_speed(rod);
        }
    }
}

/* TODO: This needs work to make it more efficient. Rather than calculating and
 * sending messages for each removed force when it is removed, we should do all
 * the removals then calc/send any messages in one.
 *
 * -- Smacy 20140408 */
void remove_force(object_t *op)
{
    object_t *env = op->env;

    if (!env)
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);

        return;
    }

    /* give out some useful message - very nice when a mob lose a effect */
    if (op->type == FORCE &&
        IS_LIVE(env) &&
        env->map)
    {
        switch (op->sub_type1)
        {
            case ST1_FORCE_DEPLETE:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s recovers depleted stats.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You recover depleted stats!");
                }

                break;

            case ST1_FORCE_DRAIN:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s recovers drained levels.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You recover drained levels!");
                }

                break;

            case ST1_FORCE_SLOWED:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s suddenly moves faster.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "The world suddenly moves slower!");
                }

                break;

            case ST1_FORCE_FEAR:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s suddenly looks braver.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You suddenly feel braver!");
                }

                break;

            case ST1_FORCE_SNARE:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s suddenly walks faster.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You suddenly walk faster!");
                }

                break;

            case ST1_FORCE_PARALYZE:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s suddenly moves again.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You suddenly can move again!");
                }

                break;

            case ST1_FORCE_CONFUSED:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s suddenly regain his senses.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You suddenly regain your senses!");
                }

                break;

            case ST1_FORCE_BLIND:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s suddenly can see again.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You suddenly can see again!");
                }

                break;

            case ST1_FORCE_POISON:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s's body seems cleansed.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "Your body seems cleansed!");
                }

                break;

            case ST1_FORCE_DEATHSICK:
                ndi_map(NDI_UNIQUE | NDI_GREY, MSP_KNOWN(env), MAP_INFO_NORMAL, env, NULL, "%s seems to no longer suffer from death sickness.",
                    QUERY_SHORT_NAME(env, NULL));

                if (env->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, env, "You no longer suffer from death sickness!");
                }

                break;
        }
    }

    CLEAR_FLAG(op, FLAG_APPLIED);
    remove_ob(op);

    if (env->type == PLAYER)
    {
        change_abil(env, op);
    }
    else
    {
        FIX_PLAYER(env ,"remove force - bug? fix monster?");
    }

    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
}

void poison_more(object_t *op)
{
    object_t     *tmp;
    if (op->env == NULL || !IS_LIVE(op->env) || op->env->stats.hp < 0)
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }
    if (!op->stats.food)
    {
        /* need to remove the object before fix_player is called, else fix_player
         * will not do anything.
         */
        if (op->env->type == PLAYER)
        {
            CLEAR_FLAG(op, FLAG_APPLIED);
            FIX_PLAYER(op->env ,"poison more");
            ndi(NDI_UNIQUE, 0, op->env, "You feel much better now.");
        }
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }
    if (op->env->type == PLAYER)
    {
        /* This is here so that nothing happens instantaneously.
         * Everything happens progressively, like real poison. Poison originally
         * dropped stats and hit with a DOT along with the hitter's other attack type(s)
         * instantly after the hit, which made it way too strong against players.
         */
        if (!QUERY_FLAG(op, FLAG_APPLIED))
        {
            SET_FLAG(op, FLAG_APPLIED);
            tmp = check_obj_stat_buffs(op, op->env);
            FIX_PLAYER(op->env ,"poison more");
            return;
        }
        op->env->stats.food--;
        ndi(NDI_UNIQUE, 0, op->env, "You feel very sick...");
    }

    damage_ob(op->env, op->stats.dam, op, ENV_ATTACK_CHECK);

    // Someone may have died after damage_ob, so make sure they're still alive.
    if (op && op->env)
    {
        op->stats.dam = (sint16)((float)op->stats.dam * (1 + op->env->level / (MAXLEVEL * 2)));
    }
}

/* TODO: i have not included damage to mobs/player on reverse up going gates!
 * Look in the code!
 * also, i included sounds for open & close gates! we need to add a tracker the
 * get is going up or down.
 */
/*
 * wc:           animation frame (first=fully open, last=fully closed - independent on last_heal)
 * weight_limit: 1=opening, 0=closing (standard trigger attribute)
 * ac:           1=never blocks view, 0=blocks view when closed
 * food:         1=gate is temporary going down after crushing something
 * maxsp:        1=gate is reversed, 0=gate is normal
 */
void move_gate(object_t *op)
{
    int n = NUM_ANIMATIONS(op) / NUM_FACINGS(op);
    int update  = UP_OBJ_FACE; /* default update is only face */
    int reached_end = 0;

    if (op->stats.wc < 0 ||
        (int)op->stats.wc >= n)
    {
        dump_object(op);
        LOG(llevBug, "BUG:i: %s/move_gate(): animation was %d, max=%d\n:%s\n",
            __FILE__, op->stats.wc, n, errmsg);
        op->stats.wc = 0;
    }

    /* Check for crushing when closing the gate */
    if(op->weight_limit == 0 &&
       (int)op->stats.wc >= n / 2)
    {
        map_t *m = op->map;
        msp_t  *msp = MSP_KNOWN(op);
        object_t    *this,
                  *next;

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            if (IS_LIVE(this))
            {
                int dam = (op->level > 0) ? op->level : MAP_DIFFICULTY(m);

                dam = dam * 3 + (this->level - dam) + 1;
                damage_ob(this, dam, op, ENV_ATTACK_CHECK);

                if (this->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, this, "You are crushed by %s!",
                        query_name(op, this, ARTICLE_DEFINITE, 0));
                }
            }

            /* If the object is alive, or the object either can
             * be picked up or the object rolls, move the object
             * off the gate. */
            if (m == this->map &&
                (IS_LIVE(this) ||
                 !QUERY_FLAG(this, FLAG_NO_PICK) ||
                 QUERY_FLAG(this, FLAG_CAN_ROLL)))
            {
                /* If it has speed, it should move itself, otherwise: */
                sint8 i = overlay_find_free(msp, this, 1, OVERLAY_3X3, 0);

                /* If there is a free spot, move the object someplace */
                if (i != -1)
                {
                    remove_ob(this);
                    check_walk_off(this, NULL, MOVE_APPLY_VANISHED);
                    this->x += OVERLAY_X(i);
                    this->y += OVERLAY_Y(i);
                    insert_ob_in_map(this, m, op, 0);
                }

                break; /* Only remove one object for now... */
            }
        }

        /* Still anything blocking? */
        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            if (IS_LIVE(this) ||
                !QUERY_FLAG(this, FLAG_NO_PICK) ||
                QUERY_FLAG(this, FLAG_CAN_ROLL))
            {
                op->stats.food = 1;
                break;
            }
        }
    }

    /* Do the actual moving */
    if (op->stats.food)
    {
        /* Lower gate and retry if it was blocked */
        if (--op->stats.wc <= 0)
        {
            op->stats.wc = 0;
            op->stats.food = 0;
        }
    }
    else if (op->weight_limit)
    {
        if (--op->stats.wc <= 0)
        {
            op->stats.wc = 0;
            reached_end = 1;
        }
    }
    else
    {
        if ((int)++op->stats.wc >= n)
        {
            op->stats.wc = (signed char)n - 1;
            reached_end = 1;
        }
    }

    /* we do the QUERY_FLAG() here to check we must rebuild the tile flags or not,
     * if we don't change the object settings here, just change the face but
     * don't rebuild the flag tiles.
     */
    if ((int)op->stats.wc < (n / 2 + 1))
    {
        /* Less than half open. Always make passable + non-block view */
        if (QUERY_FLAG(op, FLAG_NO_PASS))
        {
            update = UP_OBJ_FLAGFACE;
            CLEAR_FLAG(op, FLAG_NO_PASS);
        }
        if (QUERY_FLAG(op, FLAG_BLOCKSVIEW))
        {
            update = UP_OBJ_FLAGFACE;
            CLEAR_FLAG(op, FLAG_BLOCKSVIEW);
        }
    }
    else
    {
        /* half closed or more. Make sure no pass and possibly block view */
        if (!QUERY_FLAG(op, FLAG_NO_PASS))
        {
            update = UP_OBJ_FLAGFACE;
            SET_FLAG(op, FLAG_NO_PASS);    /* The coast is clear, block the way */
        }
        if (!op->stats.ac)
        {
            if (!QUERY_FLAG(op, FLAG_BLOCKSVIEW))
            {
                update = UP_OBJ_FLAGFACE;
                SET_FLAG(op, FLAG_BLOCKSVIEW);
            }
        }
    }

    if(reached_end)
    {
        if(op->type == TIMED_GATE)
        {
            op->weight_limit = !op->weight_limit;
        }
        else
        {
            op->speed = 0;
            update_ob_speed(op); /* Reached top, let's stop */
        }
    }

    op->state = (uint8)op->stats.wc;
    SET_ANIMATION(op, n * op->direction + op->state);
    update_object(op, update);
}

/* Attributes are the same as for normal gates plus the following:
 *  hp      : how long door is open/closed
 *  maxhp   : initial value for hp
 *  sp      : 1 = triggered, 0 = sleeping or resetting
 */
void move_timed_gate(object_t *op)
{
    sint32 v = op->weight_limit;

    if (op->stats.sp)
    {
        move_gate(op);
        if (op->weight_limit != v)   /* change direction ? */
            op->stats.sp = 0;
        return;
    }

    /* Countdown activation timer */
    if (--op->stats.hp <= 0)
    {
        /* keep gate down */
        move_gate(op);
        if (op->weight_limit != v)
        {
            /* ready ? */
            op->speed = 0;
            update_ob_speed(op);
        }
    }
}

/*  slaying:    name of the thing the detector is to look for
 *   speed:      frequency of 'glances'
 *   connected:  connected value of detector
 *  sp:         1 if detection sets buttons
 *              -1 if detection unsets buttons */
void move_detector(object_t *op)
{
    msp_t  *msp = MSP_KNOWN(op);
    object_t    *this,
              *next;
    int        last = op->weight_limit;
    int        detected = 0;

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (op->stats.hp)
        {
            object_t *that,
                   *next2;

            FOREACH_OBJECT_IN_OBJECT(that, this, next2)
            {
                if (op->slaying &&
                    op->slaying == that->name)
                {
                    detected = 1;
                }
                else if (that->type == FORCE &&
                         that->slaying &&
                         that->slaying == op->slaying)
                {
                    detected = 1;
                }
            }
        }

        if (op->slaying &&
            op->slaying == this->name)
        {
            detected = 1;
        }
        else if (this->type == SPECIAL_KEY &&
                 this->slaying == op->slaying)
        {
            detected = 1;
        }

        if (detected)
        {
            break;
        }
    }

    if (detected &&
        last == !op->stats.sp)
    {
        op->weight_limit = op->stats.sp;
        signal_connection(op, this, NULL, op->map);
    }
    else if (!detected &&
             last == op->stats.sp)
    {
        op->weight_limit = !op->stats.sp;
        signal_connection(op, NULL, NULL, op->map);
    }
}

void animate_trigger(object_t *op)
{
    if ((unsigned char)++op->stats.wc >= NUM_ANIMATIONS(op) / NUM_FACINGS(op))
    {
        op->stats.wc = 0;
        check_trigger(op, NULL, NULL);
    }
    else
    {
        op->state = (uint8) op->stats.wc;
        SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
        update_object(op, UP_OBJ_FACE);
    }
}

/* close or open pit. op->value is set when something connected to the pit
 * is triggered.
 */
void move_pit(object_t *op)
{
    object_t *next, *tmp;

    if (op->weight_limit)
    {
        /* We're opening */
        if (--op->stats.wc <= 0)
        {
            /* Opened, let's stop */
            op->stats.wc = 0;
            op->speed = 0;
            update_ob_speed(op);
            SET_FLAG(op, FLAG_WALK_ON);
            for (tmp = op->above; tmp != NULL; tmp = next)
            {
                next = tmp->above;
                move_apply(op, tmp, tmp, 0);
            }
        }
        op->state = (uint8) op->stats.wc;
        SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
        update_object(op, UP_OBJ_FACE);
        return;
    }
    /* We're closing */
    CLEAR_FLAG(op, FLAG_WALK_ON);
    op->stats.wc++;
    if ((int) op->stats.wc >= NUM_ANIMATIONS(op) / NUM_FACINGS(op))
        op->stats.wc = NUM_ANIMATIONS(op) / NUM_FACINGS(op) - 1;
    op->state = (uint8) op->stats.wc;
    SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
    update_object(op, UP_OBJ_FACE);
    if ((unsigned char) op->stats.wc == (NUM_ANIMATIONS(op) / NUM_FACINGS(op) - 1))
    {
        op->speed = 0;
        update_ob_speed(op); /* closed, let's stop */
        return;
    }
}


/* This routine doesnt seem to work for "inanimate" objects that
 * are being carried, ie a held torch leaps from your hands!.
 * Modified this routine to allow held objects. b.t.
 */
void change_object(object_t *op)
{
    /* Doesn`t handle linked objs yet */

    object_t *tmp, *env ;
    int     i, j;

    /* In non-living items only change when food value is 0 */
    if (!IS_LIVE(op))
    {
        if (op->stats.food-- > 0)
            return;
        else
        {
            /* we had hooked applyable light object here - handle them special */
            if (op->type == TYPE_LIGHT_APPLY)
            {
                CLEAR_FLAG(op, FLAG_CHANGING);

                /* thats special lights like lamp which can be refilled */
                if (op->other_arch == NULL || (op->other_arch && !(op->other_arch->clone.sub_type1 & 2)))
                {
                    sint32 flags;

                    op->stats.food = 0;
                    if (op->other_arch && op->other_arch->clone.sub_type1 & 1)
                    {
                        op->animation_id = op->other_arch->clone.animation_id;
                        SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
                        flags = UPD_ANIM;
                    }
                    else
                    {
                        CLEAR_FLAG(op, FLAG_ANIMATE);
                        op->face = op->arch->clone.face;
                        flags = UPD_FACE;
                    }

                    if (op->env) /* not on map? */
                    {
                        if (op->env->type == PLAYER) /* inside player char? */
                        {
                            ndi(NDI_UNIQUE, 0, op->env, "%s burnt out.",
                                query_name(op, op->env, ARTICLE_DEFINITE, 0));
                            op->glow_radius = 0;
                            FIX_PLAYER(op->env ,"change object");
                        }
                        else /* atm, lights inside other inv as players don't set light masks */
                        {
                            /* but we need to update container which are possible watched by players */
                            op->glow_radius = 0;
                        }

                        esrv_update_item(flags, op);
                    }
                    else /* object is on map */
                    {
                        msp_t *msp = MSP_KNOWN(op);

                        /* remove light mask from map */
                        adjust_light_source(msp, -(op->glow_radius));
                        update_object(op, UP_OBJ_FACE); /* tell map update we have something changed */
                        op->glow_radius = 0;
                    }
                    return;
                }
                else /* this object will be deleted and exchanged with other_arch */
                {
                    /* but give the player a note about it too */
                    if (op->env && op->env->type == PLAYER)
                        ndi(NDI_UNIQUE, 0, op->env, "%s burnt out.",
                            query_name(op, op->env, ARTICLE_DEFINITE, 0));
                }
            }
        }
    } /* end non living objects */

    if (op->other_arch == NULL)
    {
        LOG(llevBug, "BUG: Change object (%s) without other_arch error.\n", op->name);
        return;
    }

    env = op->env;
    remove_ob(op);
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);

    for (i = 0; i < 1; i++) /* atm we only generate per change tick *ONE* object_t */
    {
        tmp = arch_to_object(op->other_arch);
        tmp->stats.hp = op->stats.hp; /* The only variable it keeps. */
        if (tmp->type == TYPE_LIGHT_APPLY && tmp->other_arch)
            tmp->stats.food = op->stats.maxhp; /* means we pass max ticks of light down the chain */
        if (env)
        {
            tmp->x = env->x,tmp->y = env->y;
            (void)insert_ob_in_ob(tmp, env);
        }
        else
        {
            msp_t *msp = MSP_KNOWN(op);

            /* The problem with searching for a free spot in this kind of
             * object change (where the change is only technical, ie, in the
             * gameworld the object is the same physical object, just with a
             * changed status) is that the object jumps about for no
             * player-obvious reason if, eg, it is on the same square as a
             * player during the change -- Smacky 20080704 */
            j = overlay_find_free(msp, tmp, 1, OVERLAY_7X7, OVERLAY_FIRST_AVAILABLE);
            if (j != -1)  /* Found a free spot */
            {
                if (op->type == TYPE_LIGHT_APPLY && tmp->other_arch)
                {
                    /* remove light mask from map */
                    adjust_light_source(msp, -(tmp->glow_radius));
                    update_object(op, UP_OBJ_FACE); /* tell map update we have something changed */
                    op->glow_radius = 0;
                }
                tmp->x = op->x + OVERLAY_X(j),tmp->y = op->y + OVERLAY_Y(j);
                insert_ob_in_map(tmp, op->map, op, 0);
                if (tmp->type == TYPE_LIGHT_APPLY)
                    turn_on_light(tmp);
            }
        }
    }
}


/* First, with tiled maps it is a big problem, that teleporters
 * can only move player over maps. Second, i added a "no_teleport"
 * flag to the engine.
 * The teleporter will now teleport ANY object on the tile node - also
 * multi arch objects which are with one part on the teleporter.
 * WARNING: Also system objects will be teleported when they don't
 * have a "no_teleport" flag.
 * Because we can teleport multi arch monster now with a single tile
 * teleporter, i removed multi arch teleporters. */
void move_teleporter(object_t *op)
{
    msp_t  *msp = MSP_KNOWN(op);
    object_t    *this,
              *next;

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (QUERY_FLAG(this, FLAG_NO_TELEPORT))
        {
            continue;
        }

        /* teleport to different map */
        if (EXIT_PATH(op))
        {
            if (trigger_object_plugin_event(EVENT_TRIGGER, op, this, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
            {
                continue;
            }

            (void)enter_map_by_exit(this, op);
        }
        else if (EXIT_X(op) != -1 &&
                 EXIT_Y(op) != -1) /* teleport inside this map */
        {
            map_t *m2 = op->map;
            sint16     x2 = EXIT_X(op),
                       y2 = EXIT_Y(op);

            /* use OUT_OF_REAL_MAP() - we want be truly on THIS map */
            if (OUT_OF_REAL_MAP(m2, x2, y2))
            {
                LOG(llevMapbug, "MAPBUG:: Removed illegal teleporter [%s %d %d] (destination out of map = %d %d)!\n",
                    STRING_MAP_PATH(m2), op->x, op->y, x2, y2);
                remove_ob(op);
                check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
                return;
            }

            if (trigger_object_plugin_event(EVENT_TRIGGER, op, this, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
            {
                continue;
            }

            (void)enter_map(this, MSP_RAW(m2, x2, y2), op, OVERLAY_FIRST_AVAILABLE | OVERLAY_SPECIAL, 0);
        }
        else
        {
            /* Random teleporter */
            if (trigger_object_plugin_event(EVENT_TRIGGER, op, this, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
            {
                continue;
            }

            teleport(op, this);
        }
    }
}

/*  peterm:  firewalls generalized to be able to shoot any type
    of spell at all.  the stats.dam field of a firewall object
    contains it's spelltype.      The direction of the wall is stored
    in op->direction. walls can have hp, so they can be torn down. */
/* added some new features to FIREWALL - on/off features by connected,
 * advanced spell selection and full turnable by connected and
 * autoturn. MT-2003
 */
void move_firewall(object_t *op)
{
    if (!op->map || !op->last_eat || op->stats.dam == -1) /* last_eat 0 = off */
        return;   /* dm has created a firewall in his inventory or no legal spell selected */
    cast_spell(op, op, op->direction, op->stats.dam, 1, spellNPC, NULL);
}

#if 0
//void move_firechest(object_t *op)
//{
//    if (!op->map)
//        return;   /* dm has created a firechest in his inventory */
//    fire_a_ball(op, random_roll(1, 8), 7);
//}
#endif

/*  move_player_mover:  this function takes a "player mover" as an
 * argument, and performs the function of a player mover, which is:

 * a player mover finds any players that are sitting on it.  It
 * moves them in the op->stats.sp direction.  speed is how often it'll move.
 * If attacktype is nonzero it will paralyze the player.  If lifesave is set,
 * it'll dissapear after hp+1 moves.  If hp is set and attacktype is set,
 * it'll paralyze the victim for hp*his speed/op->speed

*/
void move_player_mover(object_t *op)
{
    map_t *m;
    sint16     x,
               y;
    msp_t  *msp;
    object_t    *this,
              *next;
    int        dir;

    /* e.g. mover is inside a creator */
    if (!op->map)
    {
        return;
    }

    m = op->map;
    x = op->x;
    y = op->y;

    if (!(msp_blocked(NULL, m, x, y) & (MSP_FLAG_ALIVE | MSP_FLAG_PLAYER)))
    {
        return;
    }

    /* Determine direction now for random movers so we do the right thing */
    if (!(dir = op->direction))
    {
        dir = random_roll(1, 8);
    }

    msp = MSP_KNOWN(op);

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        map_t *mt;
        sint16     xt,
                   yt;
        msp_t  *mspt;
        object_t    *that,
                  *next2;

        /* Not convinced this is entirely correct. I think may it should be:
         *   if (op->stats.maxhp && IS_LIVE(this) && !IS_AIRBORNE(this))
         * -- Smacky 20090220 */
        if ((IS_LIVE(this) &&
             !IS_AIRBORNE(this)) ||
            op->stats.maxhp)
        {
            if (QUERY_FLAG(op, FLAG_LIFESAVE) &&
                op->stats.hp-- < 0)
            {
                remove_ob(op);
                return;
            }

            mt = m;
            xt = x + OVERLAY_X(dir);
            yt = y + OVERLAY_Y(dir);
            mspt = MSP_GET(mt, xt, yt);

            if (!mspt)
            {
                return;
            }

            FOREACH_OBJECT_IN_MSP(that, mspt, next2)
            {
                if (that->type == PLAYERMOVER)
                {
                    that->speed_left = -0.99;
                }
                else if (IS_LIVE(that))
                {
                    op->speed_left = -1.1;  /* wait until the next thing gets out of the way */
                }
            }

            if (this->type == PLAYER)
            {
                /*  only level >=1 movers move people */
                if (!op->level)
                {
                    return;
                }

                /* Following is a bit of hack.  We need to make sure it
                 * is cleared, otherwise the player will get stuck in
                 * place.  This can happen if the player used a spell to
                 * get to this space. */
                this->speed_left = -FABS(this->speed);
                move_player(this, dir, 1);
            }
            else
            {
                (void)move_ob(this, dir, op);
            }

            /* flag to paralyze the player */
            /* ATM disabled when i removed attacktype - when needed move to better attribute
               MT 09-2005
            if (!op->stats.maxsp && op->attacktype)
                op->stats.maxsp = 2;
            if (op->attacktype)
                this->speed_left = -FABS(op->stats.maxsp * this->speed / op->speed);
             */
        }
    }
}

/*  move_creator (by peterm)
  Let the creator object create it's other_arch right on top of itself.
  connected:  what will trigger it
  stats.hp:  how many times it may create before stopping
  FLAG_LIFESAVE:  if set, it'll never disappear but will go on creating
    everytime it's triggered
  FLAG_ONE_DROP:  if set, it'll check before creating to avoid duplicates
  other_arch: (optional) the archetype to create
  inv: objects to clone (if other_arch == NULL)
*/
/* not multi arch fixed, i think MT */
void move_creator(object_t *op)
{
    msp_t *msp;
    object_t   *creation;

    if (op->stats.hp <= 0 &&
        !QUERY_FLAG(op, FLAG_LIFESAVE))
    {
        return;
    }

    msp = MSP_KNOWN(op);

    /* Create from other_arch */
    if(op->other_arch)
    {
        creation = arch_to_object(op->other_arch);

        if (op->slaying)
        {
            FREE_AND_ADD_REF_HASH(creation->name, op->slaying);
            FREE_AND_ADD_REF_HASH(creation->title, op->slaying);
        }

        if(QUERY_FLAG(op, FLAG_ONE_DROP) &&
           CheckForDuplicate(creation, msp))
        {
            return;
        }

        creation->x = op->x;
        creation->y = op->y;
        insert_ob_in_map(creation, op->map, op, 0);
        op->stats.hp--;
        creation->level = op->level;
    }
    /* Clone from inventory. */
    else
    {
        object_t *this,
               *next;
        int     n = 0;
        uint8   didit = 0;

        /* Create single random item from inventory? */
        if (QUERY_FLAG(op, FLAG_SPLITTING))
        {
            /* Count applicable items */
            FOREACH_OBJECT_IN_OBJECT(this, op, next)
            {
                if (QUERY_FLAG(this, FLAG_SYS_OBJECT) &&
                    this->type != PLAYERMOVER)
                {
                    continue;
                }

                n++;
            }

            if (n == 0)
            {
                return; /* Avoid div by zero */
            }

            n = RANDOM() % n;
        }

        FOREACH_OBJECT_IN_OBJECT(this, op, next)
        {
            if (QUERY_FLAG(this, FLAG_SYS_OBJECT) &&
                this->type != PLAYERMOVER)
            {
                continue;
            }

            /* FIXME: This is plainly broken. This will create every object at
             * or below the 0 index.
             *
             * -- Smacky 20140426 */
#if 0
            /* Count down to target if creating a single random item */
            if (QUERY_FLAG(op, FLAG_SPLITTING) &&
                 --n >= 0)
            {
                continue;
            }

            creation = clone_object(this, MODE_INVENTORY);

            if (QUERY_FLAG(op, FLAG_ONE_DROP) &&
                CheckForDuplicate(creation, msp))
            {
                continue;
            }

            creation->x = op->x;
            creation->y = op->y;
            insert_ob_in_map(creation, op->map, op, 0);
            didit = 1;
#else
            if (n == 0)
            {
                creation = clone_object(this, MODE_INVENTORY);

                if (QUERY_FLAG(op, FLAG_ONE_DROP) &&
                    CheckForDuplicate(creation, msp))
                {
                    continue;
                }

                creation->x = op->x;
                creation->y = op->y;
                insert_ob_in_map(creation, op->map, op, 0);
                didit = 1;
            }

            /* Count down to target if creating a single random item */
            if (QUERY_FLAG(op, FLAG_SPLITTING))
            {
                if (--n < 0)
                {
                    break;
                }
            }
#endif
        }

        if (didit)
        {
            op->stats.hp--;
        }
    }
}

/** Search a single map square for duplicates of op.
 * A duplicate is anything with the same type, name and arch
 */
static object_t *CheckForDuplicate(object_t *what, msp_t *msp)
{
    object_t *this,
           *next;

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (this->name == what->name &&
            this->type == what->type &&
            this->arch == what->arch)
        {
            return this;
        }
    }

    return NULL;
}

/* hp = time left
 * maxhp = time to reset to
 * FLAG_CURSED - reset when triggered
 * value = connection state
 */
void move_timer(object_t *op)
{
    if(op->stats.hp >= 0)
    {
        op->stats.hp--;
        if(op->stats.hp < 0)
        {
            op->weight_limit = !op->weight_limit;
            signal_connection(op, NULL, NULL, op->map);

            if(QUERY_FLAG(op, FLAG_CURSED))
                op->stats.hp = op->stats.maxhp;
        }
    } else {
        /* Disable this timer */
        op->speed = 0;
        update_ob_speed(op);
    }
}

void move_environment_sensor(object_t *op)
{
    map_t *m;
    int        trig_tad = 0,
               trig_dow = 0,
               trig_bright = 0;

    m = parent_map(op);
    get_tad(m->tadnow, m->tadoffset);

    /* Time of day triggered */
    if (!op->slaying)
    {
        trig_tad = 1;
    }
    else
    {
        int hh1,
            mm1,
            hh2,
            mm2;

        if (sscanf(op->slaying, "%2d:%2d-%2d:%2d", &hh1, &mm1, &hh2, &mm2) == 4)
        {
            int t1 = CLAMP(hh1, 0, ARKHE_HRS_PER_DY - 1) * ARKHE_MES_PER_HR +
                CLAMP(mm1, 0, ARKHE_MES_PER_HR - 1),
                t2 = CLAMP(hh2, 0, ARKHE_HRS_PER_DY - 1) * ARKHE_MES_PER_HR +
                CLAMP(mm2, 0, ARKHE_MES_PER_HR - 1);
            int tnow = m->tadnow->hour * ARKHE_MES_PER_HR + m->tadnow->minute;

            /* Two cases: interval either spans midnight or not */
            if ((t1 > t2 &&
                 (tnow >= t1 ||
                  tnow <= t2)) ||
                (t1 <= t2 &&
                 (tnow >= t1 &&
                  tnow <= t2)))
            {
                trig_tad = 1;
            }
        }
        else
        {
            /* Interval is obviously invalid, drop it */
            FREE_AND_CLEAR_HASH(op->slaying);
        }
    }

    /* Day of Week triggered? */
    if (!op->last_heal ||
        (op->last_heal & (1 << m->tadnow->day))) // FIXME
    {
        trig_dow = 1;
    }

    /* Brightness triggered? */
    if(op->last_grace == 0)
        trig_bright = 1;
    else
    {
        object_t *tmp;

        op->last_grace = CLAMP(op->last_grace, -MAX_DARKNESS, MAX_DARKNESS);

        /* if sensor is inside container, see whats it like outside */
        for(tmp = op; tmp && tmp->env && tmp->map == NULL; tmp = tmp->env)
            ;

        /* If sensor can't see through closed containers */
        if (!QUERY_FLAG(op, FLAG_SEE_INVISIBLE) &&
            op->env)
        {
            if ((op->type != TYPE_QUEST_CONTAINER &&
                 op->type != CONTAINER) ||
                !QUERY_FLAG(op, FLAG_APPLIED))
            {
                tmp = NULL;
            }
       }

        if(tmp)
        {
            int light_level = MSP_GET_REAL_BRIGHTNESS(MSP_RAW(m, op->x, op->y));

            if ((op->last_grace < 0 &&
                 light_level < ABS(op->last_grace)) ||
                (op->last_grace > 0 &&
                 light_level > op->last_grace))
            {
                trig_bright = 1;
            }
        }
    }

    /* Trigger if sensor status changes (or when not initialized) */
    if ((trig_tad &&
         trig_dow &&
         trig_bright &&
         op->weight_limit == 0) ||
         (!trig_tad &&
          !trig_dow &&
          !trig_bright &&
          op->weight_limit == 1) ||
         !QUERY_FLAG(op, FLAG_INITIALIZED))
    {
        SET_FLAG(op, FLAG_INITIALIZED);
        op->weight_limit = (trig_tad && trig_dow && trig_bright) ? 1 : 0;
        signal_connection(op, NULL, NULL, m);
    }
}

/*
 * last_grace = output connection
 * subtype = logical function
 * anim_enemy_dir = has been updated since load (not saved)
 */
/* TODO: I want to add forwarding of connections to other maps to this
 * type too. */
void move_conn_sensor(object_t *op)
{
    sint32 newvalue = 0;
    int numinputs = 0, numactive = 0;

    objectlink_t   *obp;
    objectlink_t *ol;

    if(op->map == NULL)
        return;

    /* Count number of active inputs. Lets define an input
     * as active if the majority of the connection objects
     * on that connection has value > 0 */
    for (obp = op->map->buttons; obp; obp = obp->next)
    {
        int myinput = 0;
        int numzeroes = 0, numones = 0;

        /* Don't count our own output */
        if(op->last_grace == obp->value)
            continue;

        /* Combined searhing for connections this sensor is part of
         * and checking value of connection */
        for (ol = obp->objlink.link; ol; ol = ol->next)
        {
            if (ol->objlink.ob == op && ol->id == op->count)
                myinput = 1;
            else
            {
                /* Don't count receivers towards number of inputs */
                switch(ol->objlink.ob->type)
                {
                    case TYPE_CONN_SENSOR:
                        /* Only count the output connection on conn_sensors */
                        if(ol->objlink.ob->last_grace == obp->value)
                            break;
                    case LIGHT_SOURCE:
                    case GATE:
                    case TIMED_GATE:
                    case PIT:
                    case SIGN:
                    case MOOD_FLOOR:
                    case TYPE_LIGHT_APPLY:
                    case FIREWALL:
                    case DIRECTOR:
                    case TELEPORTER:
                    case CREATOR:
                    case SPAWN_POINT:
                        continue;
                }

                if(ol->objlink.ob->weight_limit > 0)
                    numones++;
                else
                    numzeroes++;
            }
        }

        /* Count it as an active input? */
        if(myinput)
        {
            numinputs++;
            if(numones >= numzeroes)
                numactive++;
        }
    }


    /* Perform the logic filtering */
    switch(op->sub_type1)
    {
        case ST1_CONN_SENSOR_NAND: /* Require _no_ active inputs */
            newvalue = (numactive == 0);
            break;
        case ST1_CONN_SENSOR_AND:  /* Require _all_ active inputs */
            newvalue = (numactive == numinputs);
            break;
        case ST1_CONN_SENSOR_OR:   /* Require _any_ active inputs */
            newvalue = (numactive > 0);
            break;
        case ST1_CONN_SENSOR_XOR:  /* Require _exactly one_ active input */
            newvalue = (numactive == 1);
            break;
    }

    /* LOG(llevDebug, "move_conn_sensor: count %d, type=%d, numactive=%d, numinputs=%d, value=%d -> %d, inited=%d\n", op->count, op->sub_type1, numactive, numinputs, op->weight_limit, newvalue, QUERY_FLAG(op, FLAG_INITIALIZED)); */

    /* Trigger only on state change (or at first init) */
    if(op->weight_limit != newvalue || !QUERY_FLAG(op, FLAG_INITIALIZED))
    {
        SET_FLAG(op, FLAG_INITIALIZED);
        op->weight_limit = newvalue;
        signal_connection(op, NULL, NULL, op->map);
    }
}

/* move_marker --peterm@soda.csua.berkeley.edu
   when moved, a marker will search for a player sitting above
   it, and insert an invisible, weightless force into him
   with a specific code as the slaying field.
   At that time, it writes the contents of its own message
   field to the player.  The marker will decrement hp to
   0 and then delete itself every time it grants a mark.
   unless hp was zero to start with, in which case it is infinite.*/

void move_marker(object_t *op)
{
    msp_t  *msp = MSP_KNOWN(op);
    object_t    *this,
              *next;

    if (!msp)
    {
        return;
    }

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        /* we've got someone to MARK */
        if (this->type == PLAYER)
        {
            object_t *that,
                   *next2,
                   *old = NULL,
                   *current = NULL;

            /* remove an old force with a slaying field == op->name */
            FOREACH_OBJECT_IN_OBJECT(that, this, next2)
            {
                if (that->type == FORCE)
                {
                    if (that->slaying == op->name)
                    {
                        old = that;
                    }
                    else if (that->slaying == op->slaying)
                    {
                        current = that;
                    }
                }

                if (old &&
                    current)
                {
                    break;
                }
            }

            if (old)
            {
                remove_ob(old);
            }

            /* if we didn't find our own MARK */
            if (!current)
            {
                current = get_archetype("force");

                if (op->stats.food)
                {
                    current->speed = 0.01;
                    current->speed_left = (float)-op->stats.food;
                }
                else
                {
                    current->speed = current->speed_left = 0.0;
                }

                update_ob_speed(current);

                /* put in the lock code */
                FREE_AND_COPY_HASH(current->slaying, op->slaying);
                insert_ob_in_ob(current, this);

                if (op->msg)
                {
                    ndi(NDI_UNIQUE | NDI_NAVY, 0, this, "%s",
                        op->msg);
                }

                /* marker expires--granted mark number limit */
                if (--op->stats.hp <= 0)
                {
                    remove_ob(op);
                    return;
                }
            }
        }
    }
}

int process_object(object_t *op)
{
    if (OBJECT_FREE(op))
        return 1;

    if (QUERY_FLAG(op, FLAG_MONSTER))
        if (move_monster(op, TRUE))
            return 1;

    if (QUERY_FLAG(op, FLAG_CHANGING) && !op->state)
    {
        change_object(op);
        return 1;
    }

    if (QUERY_FLAG(op, FLAG_IS_USED_UP) && --op->stats.food <= 0)
    {
        if (op->type == TYPE_FOOD_FORCE && op->env && op->env->type == PLAYER && CONTR(op->env))
        {
            CLEAR_FLAG(op->env, FLAG_EATING);
            CONTR(op->env)->food_status = 0;
            ndi(NDI_UNIQUE| NDI_NAVY, 0, op->env, "You finish digesting your meal.");
        }

        if (QUERY_FLAG(op, FLAG_APPLIED) && op->type != CONTAINER)
        {
            remove_force(op);
        }
        else
        {
            /* we have a decying container on the floor (asuming its only possible here) ! */
            if (op->type == CONTAINER && (op->sub_type1 & 1) == ST1_CONTAINER_CORPSE)
            {
                if (op->attacked_by) /* this means someone access the corpse */
                {
                    /* then stop decaying! - we don't want delete this under the hand of the guy! */
                    op->stats.food += 3; /* give him a bit time back */
                    goto process_object_dirty_jump; /* go on */
                }

                /* now we do something funny: WHEN the corpse is a (personal) bounty,
                 * we delete the bounty marker (->slaying) and reseting the counter.
                 * Now other people can access the corpse for stuff which are leaved
                 * here perhaps.
                 */
                if (op->slaying)
                {
                    FREE_AND_CLEAR_HASH2(op->slaying);
                    op->stats.food = op->arch->clone.stats.food;
                    remove_ob(op);                       /* another lame way to update view of players... */
                    insert_ob_in_map(op, op->map, NULL, INS_NO_WALK_ON);
                    return 1;
                }

                remove_ob(op);
                check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
                return 1;
            }

            (void)kill_object(op, NULL);
        }
        return 1;
    }

    process_object_dirty_jump:

    /* I don't like this script object here ..  this is *the* core loop.  */
    /* The redundant flag test avoids a function call in the common case */
    if (op->event_flags & EVENT_FLAG_TIME)
        if(trigger_object_plugin_event(EVENT_TIME, op, NULL, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
            return 0;

    switch (op->type)
    {
        case ROD:
        case HORN:
          regenerate_rod(op);
          return 1;
        case FORCE:
          if (!QUERY_FLAG(op, FLAG_IS_USED_UP))
              remove_force(op);
          else if (op->sub_type1 == ST1_FORCE_POISON)
              poison_more(op);
          return 1;
        case POTION_EFFECT:
          if (!QUERY_FLAG(op, FLAG_IS_USED_UP))
              remove_force(op);
          return 1;
        case SPAWN_POINT:
          spawn_point(op);
          return 0;
/* Temp fix because diseases cause crashes. */
#if 0
        case DISEASE:
          move_disease(op);
          return 0;
        case SYMPTOM:
          move_symptom(op);
          return 0;
#else
        case DISEASE:
        case SYMPTOM:
          if (!QUERY_FLAG(op, FLAG_REMOVED))
          {
                remove_ob(op);
          }
          return 0;
#endif
        case TYPE_FOOD_FORCE:
          food_force_reg(op);
        return 0;
        case BULLET:
          move_fired_arch(op);
          return 0;
        case THROWN_OBJ:
        case ARROW:
          move_missile(op);
          return 0;
        case LIGHTNING:
          /* It now moves twice as fast */
          move_bolt(op);
          return 0;
        case CONE:
          move_cone(op);
          return 0;
        case DOOR:
          remove_door(op);
          return 0;
        case LOCKED_DOOR:
          remove_door3(op); /* handle autoclosing */
          return 0;
        case TELEPORTER:
          move_teleporter(op);
          return 0;
        case GOLEM:
          move_golem(op);
          return 0;
        case FIREWALL:
          move_firewall(op);
          return 0;
        case GATE:
          move_gate(op);
          return 0;
        case TIMED_GATE:
          move_timed_gate(op);
          return 0;
        case TRIGGER:
        case TRIGGER_BUTTON:
        case TRIGGER_PEDESTAL:
        case TRIGGER_ALTAR:
          animate_trigger(op);
          return 0;
        case DETECTOR:
          move_detector(op);
          return 0;
        case PIT:
          move_pit(op);
          return 0;
        case PLAYERMOVER:
          move_player_mover(op);
          return 0;
        case CREATOR:
          move_creator(op);
          return 0;
        case MARKER:
          move_marker(op);
          return 0;
        case TYPE_TIMER:
          move_timer(op);
          return 0;
        case TYPE_ENV_SENSOR:
          move_environment_sensor(op);
          return 0;
#if 0
//        case FBULLET:
//          move_fired_arch(op);
//          return 0;
//        case MMISSILE:
//          move_magic_missile(op);
//          return 0;
//        case FIRECHEST:
//          move_firechest(op);
//          return 0;
//        case FBALL:
//        case POISONCLOUD:
//          explosion(op);
//          return 0;
//        case WORD_OF_RECALL:
//          execute_wor(op);
//          return 0;
//        case BOMB:
//          animate_bomb(op);
//          return 0;
//        case EARTHWALL:
//          damage_ob(op, 2, op, ENV_ATTACK_CHECK);
//          return 0;
//        case MOOD_FLOOR:
//          do_mood_floor(op, op);
//          return 0;
//        case DEEP_SWAMP:
//          move_deep_swamp(op);
//          return 0;
//        case CANCELLATION:
//          move_cancellation(op);
//          return 0;
//        case BALL_LIGHTNING:
//          move_ball_lightning(op);
//          return 0;
//        case SWARM_SPELL:
//          move_swarm_spell(op);
//          return 0;
//        case AURA:
//          move_aura(op);
//          return 0;
//        case PEACEMAKER:
//          move_peacemaker(op);
//          return 0;
#endif
    }

    return 0;
}

