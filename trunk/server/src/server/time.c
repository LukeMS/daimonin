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

/*
 * Routines that is executed from objects based on their speed have been
 * collected in this file.
 */

#include <global.h>

/* search op for the needed key to open door.
 * This function does really not more give back a useable key ptr
 * or NULL - it don't open, delete or doing any other action.
 */
object * find_key(object *op, object *door)
{
    object *tmp, *key;

    /* First, lets try to find a key in the top level inventory */
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
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
object * find_force(object *op, object *door)
{
	object *tmp;

    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
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
 * 0:door is NOT opened and not possible to open from op. 1: door was opened.
 * op: object which will open a door on map m, position x,y
 * mode: 0 - check but don't open the door. 1: check and open the door when possible
 */
int open_door(object *op, mapstruct *m, int x, int y, int mode)
{
    object *tmp, *key = NULL;
    object *force = NULL;

    /* Make sure a monster/npc actually can open doors */
    if (op->type != PLAYER && !QUERY_FLAG(op, FLAG_CAN_OPEN_DOOR))
        return 0;

    /* Search for door across all layers. */
    for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
    {
        if (tmp->type == LOCKED_DOOR)
        {
            if (tmp->slaying) /* door needs a key? */
            {
                if (!(key = find_key(op, tmp)) && !(force = find_force(op, tmp)))
                {
                    if (op->type == PLAYER && mode)
                        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, tmp->msg);
                    return 0; /* we can't open it! */
                }
            }

            /* are we here, the door can be opened 100% */
            if (mode) /* really opening the door? */
            {
                remove_door2(tmp, op);
                if (op->type == PLAYER)
                {
                    if (key)
                        new_draw_info_format(NDI_UNIQUE, NDI_BROWN, op, "You open the door with the %s.",
                                             query_short_name(key, op));
                    else if (force)
                        new_draw_info(NDI_UNIQUE, NDI_BROWN, op, "The door is unlocked for you.");
                }
            }

            return 1;
        }
    }

    /* we should not be here... We have a misplaced door_closed flag
     * or a door on a wrong layer... both is not good, so drop a bug msg.
     */
    LOG(llevSystem,
        "BUG: open_door() - door on wrong layer or misplaced P_DOOR_CLOSED flag - map:%s (%d,%d) (op: %s)\n", m->path,
        x, y, query_name(op));
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
void remove_door(object *op)
{
    int     i;
    object *tmp;
    for (i = 1; i < 9; i += 2)
        if ((tmp = present(DOOR, op->map, op->x + freearr_x[i], op->y + freearr_y[i])) != NULL)
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
        play_sound_map(op->map, op->x, op->y, SOUND_OPEN_DOOR, SOUND_NORMAL);
    remove_ob(op);
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
}

void remove_door2(object *op, object *opener)
{
    object *tmp, *tmp2;

    /* - NO cascading for locked doors!
    int i;
    for(i=1;i<9;i+=2) {
      tmp=present(LOCKED_DOOR,op->map,op->x+freearr_x[i],op->y+freearr_y[i]);
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
        tmp = arch_to_object(op->other_arch);
        tmp->state = 0; /* 0= closed, 1= opened */
        tmp->x = op->x;tmp->y = op->y;tmp->map = op->map;tmp->level = op->level;
        tmp->direction = op->direction;
        if (QUERY_FLAG(tmp, FLAG_IS_TURNABLE) || QUERY_FLAG(tmp, FLAG_ANIMATE))
            SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction + tmp->state);
        insert_ob_in_map(tmp, op->map, op, 0);
        if (op->sub_type1 == ST1_DOOR_NORMAL)
            play_sound_map(op->map, op->x, op->y, SOUND_OPEN_DOOR, SOUND_NORMAL);
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
    }
    else if (!op->last_eat) /* if set, we are have opened a closed door - now handle autoclose */
    {
        remove_ob(op); /* to trigger all the updates/changes on map and for player, we
                         * remove and reinsert it. a bit overhead but its secure and simple
                         */
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        for (tmp2 = op->inv; tmp2; tmp2 = tmp2->below)
        {
            if (tmp2 && tmp2->type == RUNE && tmp2->level)
                spring_trap(tmp2, opener);
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
            play_sound_map(op->map, op->x, op->y, SOUND_OPEN_DOOR, SOUND_NORMAL);
        insert_ob_in_map(op, op->map, op, 0);
    }
}

/* thats called from time.c - here we handle autoclosing doors */
void remove_door3(object *op)
{
    if (!op->last_eat) /* thats a bug - active speed but not marked as active */
    {
        LOG(llevBug, "BUG: door has speed but is not marked as active. (%s - map:%s (%d,%d))\n", query_name(op),
            op->map ? op->map->name : "(no map name!)", op->x, op->y);
        op->last_eat = 0; /* thats not a real fix ... */
        return;
    }
    if (!op->map) /* ouch */
    {
        LOG(llevBug, "BUG: door with speed but no map?! killing object...done. (%s - (%d,%d))\n", query_name(op), op->x,
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

    /* her we can use or new & shiny blocked() - we simply check the given flags */
    if (blocked(NULL, op->map, op->x, op->y, TERRAIN_ALL) & (P_NO_PASS | P_IS_ALIVE | P_IS_PLAYER))
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
            play_sound_map(op->map, op->x, op->y, SOUND_DOOR_CLOSE, SOUND_NORMAL);
        insert_ob_in_map(op, op->map, op, 0);
    }
}

void generate_monster(object *gen)
{
    int         i;
    object     *op, *head = NULL, *prev = NULL;
    archetype  *at  = gen->other_arch;

    if (GENERATE_SPEED(gen) && random_roll(0, GENERATE_SPEED(gen) - 1))
        return;
    if (gen->other_arch == NULL)
    {
        LOG(llevBug, "BUG: Generator without other_arch: %s\n", query_name(gen));
        return;
    }
    i = find_free_spot(at, gen->map, gen->x, gen->y, 1, 9);
    if (i == -1)
        return;
    while (at != NULL)
    {
        op = arch_to_object(at);
        op->x = gen->x + freearr_x[i] + at->clone.x;
        op->y = gen->y + freearr_y[i] + at->clone.y;
        if (head != NULL)
            op->head = head,prev->more = op;
        if (random_roll(0, 9))
            generate_artifact(op, gen->map->difficulty, 0, 99);
        if (!insert_ob_in_map(op, gen->map, gen, 0))
            return;
        if (op->randomitems != NULL)
            create_treasure_list(op->randomitems, op, GT_APPLY, (op->level ? op->level : gen->map->difficulty),
                                 ART_CHANCE_UNSET, 0);
        if (head == NULL)
            head = op;
        prev = op;
        at = at->more;
    }
}

void regenerate_rod(object *rod)
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

void remove_force(object *op)
{
    object *env;

    if ((env=op->env) == NULL)
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }
    CLEAR_FLAG(op, FLAG_APPLIED);
    remove_ob(op);
    if(env->type == PLAYER)
        change_abil(env, op);
    else
        FIX_PLAYER(env ,"remove force - bug? fix monster?");
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
}

/* TODO: implement and fix word of recall for instance map patch */
void execute_wor(object *op)
{
/*
    object *wor = op;
    while (op != NULL && op->type != PLAYER)
        op = op->env;
    if (op != NULL)
    {

        if (blocks_magic(op->map, op->x, op->y))
            new_draw_info(NDI_UNIQUE, 0, op, "You feel something fizzle inside you.");
        else
            enter_map_by_exit(op, wor);
    }
    remove_ob(wor);
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
*/
}

void poison_more(object *op)
{
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
            new_draw_info(NDI_UNIQUE, 0, op->env, "You feel much better now.");
        }
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }
    if (op->env->type == PLAYER)
    {
        op->env->stats.food--;
        new_draw_info(NDI_UNIQUE, 0, op->env, "You feel very sick...");
    }
    damage_ob(op->env, op->stats.dam, op, ENV_ATTACK_CHECK);
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
void move_gate(object *op)
{
    int     update  = UP_OBJ_FACE; /* default update is only face */
    int     reached_end = 0;

    if (op->stats.wc < 0 || (int) op->stats.wc >= (NUM_ANIMATIONS(op) / NUM_FACINGS(op)))
    {
        dump_object(op);
        LOG(llevBug, "BUG: Gate error: animation was %d, max=%d\n:%s\n", op->stats.wc,
            (NUM_ANIMATIONS(op) / NUM_FACINGS(op)), errmsg);
        op->stats.wc = 0;
    }

    /* Check for crushing when closing the gate */
    if(op->weight_limit == 0 && (int) op->stats.wc >= (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) / 2)
    {
        object *tmp = op;
        for(tmp = GET_BOTTOM_MAP_OB(op); tmp != NULL; tmp = tmp->above)
        {
            if (IS_LIVE(tmp))
            {
                damage_ob(tmp, 4, op, ENV_ATTACK_CHECK);
                if (tmp->type == PLAYER)
                    new_draw_info_format(NDI_UNIQUE, 0, tmp, "You are crushed by the %s!", op->name);
            }

            /* If the object is alive, or the object either can
             * be picked up or the object rolls, move the object
             * off the gate. */
            if (IS_LIVE(tmp) || !QUERY_FLAG(tmp, FLAG_NO_PICK) || QUERY_FLAG(tmp, FLAG_CAN_ROLL))
            {
                /* If it has speed, it should move itself, otherwise: */
                int i   = find_free_spot(tmp->arch, op->map, op->x, op->y, 1, 9);

                /* If there is a free spot, move the object someplace */
                if (i != -1)
                {
                    remove_ob(tmp);
                    check_walk_off(tmp, NULL, MOVE_APPLY_VANISHED);
                    tmp->x += freearr_x[i],tmp->y += freearr_y[i];
                    insert_ob_in_map(tmp, op->map, op, 0);
                }

                break; /* Only remove one object for now... */
            }
        }

        /* Still anything blocking? */
        for(tmp = GET_BOTTOM_MAP_OB(op); tmp != NULL; tmp = tmp->above)
        {
            if (IS_LIVE(tmp) || !QUERY_FLAG(tmp, FLAG_NO_PICK) || QUERY_FLAG(tmp, FLAG_CAN_ROLL))
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
        if ((int)++op->stats.wc >= ((NUM_ANIMATIONS(op) / NUM_FACINGS(op))))
        {
            op->stats.wc = (signed char) (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) - 1;
            reached_end = 1;
        }
    }

    /* we do the QUERY_FLAG() here to check we must rebuild the tile flags or not,
     * if we don't change the object settings here, just change the face but
     * don't rebuild the flag tiles.
     */
    if ((int) op->stats.wc < ((NUM_ANIMATIONS(op) / NUM_FACINGS(op)) / 2 + 1))
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
            op->weight_limit = !op->weight_limit;
        else
        {
            op->speed = 0;
            update_ob_speed(op); /* Reached top, let's stop */
        }
    }
    op->state = (uint8) op->stats.wc;
    SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction + op->state);
    update_object(op, update);
}

/* Attributes are the same as for normal gates plus the following:
 *  hp      : how long door is open/closed
 *  maxhp   : initial value for hp
 *  sp      : 1 = triggered, 0 = sleeping or resetting
 */
void move_timed_gate(object *op)
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
 *              -1 if detection unsets buttons
 */

void move_detector(object *op)
{
    object *tmp;
    int     last    = op->weight_limit;
    int     detected;
    detected = 0;

    for (tmp = GET_BOTTOM_MAP_OB(op); tmp != NULL && !detected; tmp = tmp->above)
    {
        object *tmp2;
        if (op->stats.hp)
        {
            for (tmp2 = tmp->inv; tmp2; tmp2 = tmp2->below)
            {
                if (op->slaying && op->slaying==tmp->name)
                    detected = 1;
                if (tmp2->type == FORCE && tmp2->slaying && tmp2->slaying == op->slaying)
                    detected = 1;
            }
        }
        if (op->slaying && op->slaying == tmp->name)
        {
            detected = 1;
        }
        else if (tmp->type == SPECIAL_KEY && tmp->slaying == op->slaying)
            detected = 1;
    }

    /* the detector sets the button if detection is found */
    if (op->stats.sp == 1)
    {
        if (detected && last == 0)
        {
            op->weight_limit = 1;
            push_button(op, tmp, NULL);
        }
        if (!detected && last == 1)
        {
            op->weight_limit = 0;
            push_button(op, NULL, NULL);
        }
    }
    else
    {
        /* in this case, we unset buttons */
        if (detected && last == 1)
        {
            op->weight_limit = 0;
            push_button(op, tmp, NULL);
        }
        if (!detected && last == 0)
        {
            op->weight_limit = 1;
            push_button(op, NULL, NULL);
        }
    }
}


void animate_trigger(object *op)
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
void move_pit(object *op)
{
    object *next, *tmp;

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
void change_object(object *op)
{
    /* Doesn`t handle linked objs yet */

    object *tmp, *env ;
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
                    op->stats.food = 0;
                    if (op->other_arch && op->other_arch->clone.sub_type1 & 1)
                    {
                        op->animation_id = op->other_arch->clone.animation_id;
                        SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
                    }
                    else
                    {
                        CLEAR_FLAG(op, FLAG_ANIMATE);
                        op->face = op->arch->clone.face;
                    }

                    if (op->env) /* not on map? */
                    {
                        if (op->env->type == PLAYER) /* inside player char? */
                        {
                            new_draw_info_format(NDI_UNIQUE, 0, op->env, "The %s burnt out.", query_name(op));
                            op->glow_radius = 0;
                            esrv_send_item(op->env, op);
                            FIX_PLAYER(op->env ,"change object");
                        }
                        else /* atm, lights inside other inv as players don't set light masks */
                        {
                            /* but we need to update container which are possible watched by players */
                            op->glow_radius = 0;
                            if (op->env->type == CONTAINER)
                                esrv_send_item(NULL, op);
                        }
                    }
                    else /* object is on map */
                    {
                        /* remove light mask from map */
                        adjust_light_source(op->map, op->x, op->y, -(op->glow_radius));
                        update_object(op, UP_OBJ_FACE); /* tell map update we have something changed */
                        op->glow_radius = 0;
                    }
                    return;
                }
                else /* this object will be deleted and exchanged with other_arch */
                {
                    /* but give the player a note about it too */
                    if (op->env && op->env->type == PLAYER)
                        new_draw_info_format(NDI_UNIQUE, 0, op->env, "The %s burnt out.", query_name(op));
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
    for (i = 0; i < 1; i++) /* atm we only generate per change tick *ONE* object */
    {
        tmp = arch_to_object(op->other_arch);
        tmp->stats.hp = op->stats.hp; /* The only variable it keeps. */
        if (tmp->type == TYPE_LIGHT_APPLY)
        {
            tmp->stats.food = tmp->stats.maxhp;
            tmp->glow_radius = tmp->last_sp;
        }
        if (env)
        {
            tmp->x = env->x,tmp->y = env->y;
            tmp = insert_ob_in_ob(tmp, env);

            /* this should handle in future insert_ob_in_ob() */
            if (env->type == PLAYER)
            {
                esrv_del_item(CONTR(env), op->count, NULL);
                esrv_send_item(env, tmp);
            }
            else if (env->type == CONTAINER)
            {
                esrv_del_item(NULL, op->count, env);
                esrv_send_item(env, tmp);
            }
        }
        else
        {
            /* The problem with searching for a free spot in this kind of
             * object change (where the change is only technical, ie, in the
             * gameworld the object is the same physical object, just with a
             * changed status) is that the object jumps about for no
             * player-obvious reason if, eg, it is on the same square as a
             * player during the change -- Smacky 20080704 */
            j = find_first_free_spot(tmp->arch, op->map, op->x, op->y);
            if (j != -1)  /* Found a free spot */
            {
                if (op->type == TYPE_LIGHT_APPLY && tmp->other_arch)
                {
                    /* remove light mask from map */
                    adjust_light_source(op->map, op->x, op->y, -(tmp->glow_radius));
                    update_object(op, UP_OBJ_FACE); /* tell map update we have something changed */
                    op->glow_radius = 0;
                }
                tmp->x = op->x + freearr_x[j],tmp->y = op->y + freearr_y[j];
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
 * teleporter, i removed multi arch teleporters.
 */

void move_teleporter(object *op)
{
    object *tmp, *next;

    /* get first object of this map node */
    for (tmp = GET_BOTTOM_MAP_OB(op); tmp != NULL; tmp = next)
    {
        next = tmp->above;
        if (QUERY_FLAG(tmp, FLAG_NO_TELEPORT))
            continue;

        /* teleport to different map */
        if (EXIT_PATH(op))
        {
            if(trigger_object_plugin_event(EVENT_TRIGGER,
                        op, tmp, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                    continue;

            enter_map_by_exit(tmp, op);
        }
        else if (EXIT_X(op) != -1 && EXIT_Y(op) != -1) /* teleport inside this map */
        {
            /* use OUT_OF_REAL_MAP() - we want be truly on THIS map */
            if(OUT_OF_REAL_MAP(op->map, EXIT_X(op), EXIT_Y(op)))
            {
                LOG(llevBug, "BUG: Removed illegal teleporter (map: %s (%d,%d)) -> (%d,%d)\n", op->map->name, op->x,
                    op->y, EXIT_X(op), EXIT_Y(op));
                remove_ob(op);
                check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
                return;
            }

            if(trigger_object_plugin_event(EVENT_TRIGGER,
                        op, tmp, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                    continue;
            enter_map(tmp, op, tmp->map, EXIT_X(op), EXIT_Y(op), 0);
        }
        else
        {
            /* Random teleporter */
            if(trigger_object_plugin_event(EVENT_TRIGGER,
                        op, tmp, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                    continue;
            teleport(op, TELEPORTER, tmp);
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
void move_firewall(object *op)
{
    if (!op->map || !op->last_eat || op->stats.dam == -1) /* last_eat 0 = off */
        return;   /* dm has created a firewall in his inventory or no legal spell selected */
    cast_spell(op, op, op->direction, op->stats.dam, 1, spellNPC, NULL);
}

void move_firechest(object *op)
{
    if (!op->map)
        return;   /* dm has created a firechest in his inventory */
    fire_a_ball(op, random_roll(1, 8), 7);
}


/*  move_player_mover:  this function takes a "player mover" as an
 * argument, and performs the function of a player mover, which is:

 * a player mover finds any players that are sitting on it.  It
 * moves them in the op->stats.sp direction.  speed is how often it'll move.
 * If attacktype is nonzero it will paralyze the player.  If lifesave is set,
 * it'll dissapear after hp+1 moves.  If hp is set and attacktype is set,
 * it'll paralyze the victim for hp*his speed/op->speed

*/
void move_player_mover(object *op)
{
    object     *victim, *nextmover;
    mapstruct  *mt;
    int         xt, yt, dir = op->direction;

    /* e.g. mover is inside a creator */
    if(op->map == NULL)
        return;

    if (!(blocked(NULL, op->map, op->x, op->y, TERRAIN_NOTHING) & (P_IS_ALIVE | P_IS_PLAYER)))
        return;
    /* Determine direction now for random movers so we do the right thing */
    if (!dir)
        dir = random_roll(1, 8);
    for (victim = GET_BOTTOM_MAP_OB(op); victim != NULL; victim = victim->above)
    {
        if (IS_LIVE(victim) && ((!QUERY_FLAG(victim, FLAG_FLYING)&&!QUERY_FLAG(victim, FLAG_LEVITATE)) || op->stats.maxhp))
        {
            if (QUERY_FLAG(op, FLAG_LIFESAVE) && op->stats.hp-- < 0)
            {
                destruct_ob(op);
                return;
            }

            xt = op->x + freearr_x[dir];
            yt = op->y + freearr_y[dir];
            if (!(mt = out_of_map(op->map, &xt, &yt)))
                return;

            for (nextmover = GET_MAP_OB(mt, xt, yt); nextmover != NULL; nextmover = nextmover->above)
            {
                if (nextmover->type == PLAYERMOVER)
                    nextmover->speed_left = -0.99f;
                if (IS_LIVE(nextmover))
                {
                    op->speed_left = -1.1f;  /* wait until the next thing gets out of the way */
                }
            }

            if (victim->type == PLAYER)
            {
                /*  only level >=1 movers move people */
                if (op->level)
                {
                    /* Following is a bit of hack.  We need to make sure it
                         * is cleared, otherwise the player will get stuck in
                         * place.  This can happen if the player used a spell to
                         * get to this space.
                         */
                    victim->speed_left = -FABS(victim->speed);
                    move_player(victim, dir, TRUE);
                }
                else
                    return;
            }
            else
                move_object(victim, dir);

            /* flag to paralyze the player */
            /* ATM disabled when i removed attacktype - when needed move to better attribute
               MT 09-2005
            if (!op->stats.maxsp && op->attacktype)
                op->stats.maxsp = 2;
            if (op->attacktype)
                victim->speed_left = -FABS(op->stats.maxsp * victim->speed / op->speed);
             */
        }
    }
}

/** Search a single map square for duplicates of op.
 * A duplicate is anything with the same type, name and arch
 */
static int check_for_duplicate_ob(object *op, mapstruct *map, int x, int y)
{
    object *tmp;
    for(tmp = GET_MAP_OB(map, x, y); tmp != NULL; tmp = tmp->above)
    {
        if(tmp->name == op->name &&
                tmp->type == op->type &&
                tmp->arch == op->arch &&
                tmp != op)
            return 1;
    }
    return 0;
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
void move_creator(object *op)
{
    if (op->stats.hp <= 0 && !QUERY_FLAG(op, FLAG_LIFESAVE))
        return;

    if(op->other_arch)
    {
        /* Create from other_arch */
        object *tmp = arch_to_object(op->other_arch);
        if (op->slaying)
        {
            FREE_AND_ADD_REF_HASH(tmp->name, op->slaying);
            FREE_AND_ADD_REF_HASH(tmp->title, op->slaying);
        }
        tmp->x = op->x;
        tmp->y = op->y;
        tmp->level = op->level;

        if(QUERY_FLAG(op, FLAG_ONE_DROP) &&
                check_for_duplicate_ob(tmp, op->map, op->x, op->y))
            return;

        op->stats.hp--;
        insert_ob_in_map(tmp, op->map, op, 0);
    }
    else
    {
        /* Clone from inventory
         * sys objects won't be copied, with an exception for player movers */
        object *source, *tmp;
        int didit = 0;
        int cloneindex = 0;

        /* Create single random item from inventory? */
        if(QUERY_FLAG(op, FLAG_SPLITTING))
        {
            int numobs = 0;
            /* Count applicable items */
            for(tmp = op->inv; tmp != NULL; tmp = tmp->below)
                if(! QUERY_FLAG(tmp, FLAG_SYS_OBJECT) || tmp->type == PLAYERMOVER)
                    numobs++;
            if(numobs == 0)
                return; /* Avoid div by zero */
            cloneindex = RANDOM()%numobs;
        }

        for(source = op->inv; source != NULL && cloneindex >= 0; source = source->below)
        {
            /* Don't clone sys objects */
            if(QUERY_FLAG(source, FLAG_SYS_OBJECT) && source->type != PLAYERMOVER)
                continue;

            /* Count down to target if creating a single random item */
            if(QUERY_FLAG(op, FLAG_SPLITTING) && --cloneindex >= 0)
                continue;

            tmp = ObjectCreateClone(source);
            tmp->x = op->x;
            tmp->y = op->y;

            if(QUERY_FLAG(op, FLAG_ONE_DROP) &&
                    check_for_duplicate_ob(tmp, op->map, op->x, op->y))
                continue;

            insert_ob_in_map(tmp, op->map, op, 0);
            didit = 1;
        }

        /* Reduce count if we cloned any object from inventory */
        if(didit)
            op->stats.hp--;
    }
}

/* hp = time left
 * maxhp = time to reset to
 * FLAG_CURSED - reset when triggered
 * value = connection state
 */
void move_timer(object *op)
{
    if(op->stats.hp >= 0)
    {
        op->stats.hp--;
        if(op->stats.hp < 0)
        {
            use_trigger(op, NULL);

            if(QUERY_FLAG(op, FLAG_CURSED))
                op->stats.hp = op->stats.maxhp;
        }
    } else {
        /* Disable this timer */
        op->speed = 0;
        update_ob_speed(op);
    }
}

void move_environment_sensor(object *op)
{
    int trig_tod = 0, trig_dow = 0, trig_bright = 0;
    timeofday_t tod;

    if(op->slaying || op->last_heal)
        get_tod(&tod);

    /* Time of day triggered? */
    if(op->slaying == NULL)
        trig_tod = 1;
    else
    {
        int hh1,mm1,hh2,mm2;
        if(sscanf(op->slaying, "%2d:%2d-%2d:%2d", &hh1, &mm1, &hh2, &mm2) == 4)
        {
            /* Simplify time comparisons */
            int t1 = CLAMP(hh1, 0, 23) * 60 + CLAMP(mm1, 0, 59);
            int t2 = CLAMP(hh2, 0, 23) * 60 + CLAMP(mm2, 0, 59);
            int tnow = tod.hour*60 + tod.minute;

            /* Two cases: interval either spans midnight or not */
            if( (t1 > t2 && (tnow >= t1 || tnow <= t2)) ||
                    (t1 <= t2 && (tnow >= t1 && tnow <= t2)))
                    trig_tod = 1;

//            LOG(llevDebug, "tod: %02d:%02d, trig (%s): %d\n", tod.hour, tod.minute, op->slaying, trig_tod);
        } else
        {
            /* Interval is obviously invalid, drop it */
            FREE_AND_CLEAR_HASH(op->slaying);
        }
    }

    /* Day of Week triggered? */
    if(op->last_heal == 0)
        trig_dow = 1;
    else
    {
        if(op->last_heal & (1 << tod.dayofweek))
            trig_dow = 1;

        // LOG(llevDebug, "Weekday %d, trig (%d): %d\n", tod.dayofweek, op->last_grace, trig_dow);
    }

    /* Brightness triggered? */
    if(op->last_grace == 0)
        trig_bright = 1;
    else
    {
        object *tmp;

        op->last_grace = CLAMP(op->last_grace, -MAX_DARKNESS, MAX_DARKNESS);

        /* if sensor is inside container, see whats it like outside */
        for(tmp = op; tmp && tmp->env && tmp->map == NULL; tmp = tmp->env)
            ;

        /* If sensor can't see through closed containers */
        if(!QUERY_FLAG(op, FLAG_SEE_INVISIBLE) && op->env) {
            if((op->type != TYPE_QUEST_CONTAINER && op->type != CONTAINER) ||
                    !QUERY_FLAG(op, FLAG_APPLIED))
                tmp = NULL;
        }

        if(tmp)
        {
            int light_level = map_brightness(tmp->map, tmp->x, tmp->y);
            if ((op->last_grace < 0 && light_level < global_darkness_table[ABS(op->last_grace)]) ||
                    (op->last_grace > 0 && light_level > global_darkness_table[ABS(op->last_grace)]))
                trig_bright = 1;
//            LOG(llevDebug, "env_sensor: trig_lvl = %d, real=%d, trig: %d\n", global_darkness_table[ABS(op->last_grace)], light_level, trig_bright);
        }
    }

    /* Trigger if sensor status changes (or when not initialized) */
    if(
            ( (trig_tod && trig_dow && trig_bright) && op->weight_limit == 0) ||
            (!(trig_tod && trig_dow && trig_bright) && op->weight_limit == 1) ||
            !QUERY_FLAG(op, FLAG_INITIALIZED))
    {
        SET_FLAG(op, FLAG_INITIALIZED);
//        LOG(llevDebug, "env_sensor toggled from %d to %d\n", op->value, !op->value);

        op->weight_limit = (trig_tod && trig_dow && trig_bright) ? 1 : 0;

        push_button(op, NULL, NULL);
    }
}

/*
 * last_grace = output connection
 * subtype = logical function
 * anim_enemy_dir = has been updated since load (not saved)
 */
/* TODO: I want to add forwarding of connections to other maps to this
 * type too. */
void move_conn_sensor(object *op)
{
    sint32 newvalue = 0;
    int numinputs = 0, numactive = 0;

    oblinkpt   *obp;
    objectlink *ol;

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
        push_button(op, NULL, NULL);
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

void move_marker(object *op)
{
    object *tmp, *tmp2;

    for (tmp = GET_BOTTOM_MAP_OB(op); tmp != NULL; tmp = tmp->above)
    {
        if (tmp->type == PLAYER)
        {
            /* we've got someone to MARK */

            /* remove an old force with a slaying field == op->name */
            for (tmp2 = tmp->inv; tmp2 != NULL; tmp2 = tmp2->below)
            {
                if (tmp2->type == FORCE && tmp2->slaying == op->name)
                    break;
            }
            if (tmp2)
                remove_ob(tmp2);

            /* cycle through his inventory to look for the MARK we want to place */
            for (tmp2 = tmp->inv; tmp2 != NULL; tmp2 = tmp2->below)
            {
                if (tmp2->type == FORCE && tmp2->slaying == op->slaying)
                    break;
            }

            /* if we didn't find our own MARK */
            if (tmp2 == NULL)
            {
                object *force   = get_archetype("force");
                force->speed = 0;
                if (op->stats.food)
                {
                    force->speed = 0.01f;
                    force->speed_left = (float) - op->stats.food;
                }
                update_ob_speed(force);
                /* put in the lock code */
                FREE_AND_COPY_HASH(force->slaying, op->slaying);
                insert_ob_in_ob(force, tmp);
                if (op->msg)
                    new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, tmp, op->msg);
                if (op->stats.hp > 0)
                {
                    op->stats.hp--;
                    if (op->stats.hp == 0)
                    {
                        /* marker expires--granted mark number limit */
                        destruct_ob(op);
                        return;
                    }
                }
            }
        }
    }
}

int process_object(object *op)
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
            new_draw_info(NDI_UNIQUE| NDI_NAVY, 0, op->env, "You finish digesting your meal.");
        }

        if (QUERY_FLAG(op, FLAG_APPLIED) && op->type != CONTAINER)
        {

            /* give out some useful message - very nice when a mob lose a effect */
            if(IS_LIVE(op->env)&&op->env->map)
            {
                if(op->sub_type1 == ST1_FORCE_SNARE)
                {
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                        MAP_INFO_NORMAL, op->env, op->env, "%s suddenly walks faster.", query_name(op->env));
                    if(op->env->type == PLAYER)
                        new_draw_info(NDI_UNIQUE, 0, op->env, "You suddenly walk faster!");
                }
                else if(op->sub_type1 == ST1_FORCE_BLIND)
                {
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                        MAP_INFO_NORMAL, op->env, op->env, "%s suddenly can see again.", query_name(op->env));
                    if(op->env->type == PLAYER)
                        new_draw_info(NDI_UNIQUE, 0, op->env, "You suddenly can see again!");
                }
                else if(op->sub_type1 == ST1_FORCE_CONFUSED)
                {
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                        MAP_INFO_NORMAL, op->env, op->env, "%s suddenly regain his senses.", query_name(op->env));
                    if(op->env->type == PLAYER)
                        new_draw_info(NDI_UNIQUE, 0, op->env, "You suddenly regain your senses!");
                }
                else if(op->sub_type1 == ST1_FORCE_PARALYZE)
                {
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                        MAP_INFO_NORMAL, op->env, op->env, "%s suddenly moves again.", query_name(op->env));
                    if(op->env->type == PLAYER)
                        new_draw_info(NDI_UNIQUE, 0, op->env, "You suddenly can move again!");
                }
                else if(op->sub_type1 == ST1_FORCE_FEAR)
                {
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                        MAP_INFO_NORMAL, op->env, op->env, "%s suddenly looks braver.", query_name(op->env));
                    if(op->env->type == PLAYER)
                        new_draw_info(NDI_UNIQUE, 0, op->env, "You suddenly feel braver!");
                }
                else if(op->sub_type1 == ST1_FORCE_SLOWED)
                {
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                    MAP_INFO_NORMAL, op->env, op->env, "%s suddenly moves faster.", query_name(op->env));
                    if(op->env->type == PLAYER)
                        new_draw_info(NDI_UNIQUE, 0, op->env, "The world suddenly moves slower!");
                }
                else if(op->sub_type1 == ST1_FORCE_DEPLETE) /* depletion */
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                                MAP_INFO_NORMAL, op->env, op->env, "%s recovers depleted stats.", query_name(op->env));
                else if(op->sub_type1 == ST1_FORCE_DRAIN)
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                                MAP_INFO_NORMAL, op->env, op->env, "%s recovers drained levels.", query_name(op->env));
                /*
                else
                    new_info_map_except_format(NDI_UNIQUE|NDI_GREY, op->env->map, op->env->x, op->env->y,
                                MAP_INFO_NORMAL, op->env, op->env, "%s lose some effects.", query_name(op->env));
                */

            }
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

                if (op->env && op->env->type == CONTAINER)
                    esrv_del_item(NULL, op->count, op->env);
                else
                {
                    object *pl  = is_player_inv(op);
                    if (pl)
                        esrv_del_item(CONTR(pl), op->count, op->env);
                }

                remove_ob(op);
                check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
                return 1;
            }

            /* IF necessary, delete the item from the players inventory */
            if (op->env && op->env->type == CONTAINER)
                esrv_del_item(NULL, op->count, op->env);
            else
            {
                object *pl  = is_player_inv(op);
                if (pl)
                    esrv_del_item(CONTR(pl), op->count, op->env);
            }
            destruct_ob(op);
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
        case POTION_EFFECT:
          if (!QUERY_FLAG(op, FLAG_IS_USED_UP))
              remove_force(op);
          return 1;
        case SPAWN_POINT:
          spawn_point(op);
          return 0;
        case POISONING:
          poison_more(op);
          return 0;
        case DISEASE:
          move_disease(op);
          return 0;
        case SYMPTOM:
          move_symptom(op);
          return 0;
          /*
        case WORD_OF_RECALL:
          execute_wor(op);
          return 0;
          */
        case TYPE_FOOD_FORCE:
          food_force_reg(op);
        return 0;
        case BULLET:
          move_fired_arch(op);
          return 0;
        case MMISSILE:
          move_magic_missile(op);
          return 0;
        case THROWN_OBJ:
        case ARROW:
          move_missile(op);
          return 0;
        case FBULLET:
          move_fired_arch(op);
          return 0;
        case FBALL:
        case POISONCLOUD:
          explosion(op);
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
        case BOMB:
          animate_bomb(op);
          return 0;
        case GOLEM:
          move_golem(op);
          return 0;
          /*
        case EARTHWALL:
          damage_ob(op, 2, op, ENV_ATTACK_CHECK);
          return 0;
          */
        case FIREWALL:
          move_firewall(op);
          return 0;
        case FIRECHEST:
          move_firechest(op);
          return 0;
          /*
        case MOOD_FLOOR:
          do_mood_floor(op, op);
          return 0;
          */
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
          /*
        case DEEP_SWAMP:
          move_deep_swamp(op);
          return 0;
          */
        case CANCELLATION:
          move_cancellation(op);
          return 0;
        case BALL_LIGHTNING:
          move_ball_lightning(op);
          return 0;
        case SWARM_SPELL:
          move_swarm_spell(op);
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
          /*
        case AURA:
          move_aura(op);
          return 0;
        case PEACEMAKER:
          move_peacemaker(op);
          return 0;
          */
        case TYPE_TIMER:
          move_timer(op);
          return 0;
        case TYPE_ENV_SENSOR:
          move_environment_sensor(op);
          return 0;
    }

    return 0;
}

