/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2006 Michael Toennies

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

#include <global.h>

/* for pow() */
#include <math.h>

/* remove all food forces (should be only one but safe is safe) from a player */
void remove_food_force(object_t *op)
{
    object_t *tmp,
           *next;

    CLEAR_FLAG(op, FLAG_EATING);
    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        if(tmp->type == TYPE_FOOD_FORCE)
                remove_ob(tmp);
    }
}

/* Every time the food force is active, it increase the player stats,
 * like a reverse poison force.
 */
void food_force_reg(object_t *op)
{
    if(op->env && CONTR(op->env))
    {
        if(op->env->type == PLAYER)
        {
            /* implicit interruption of the food force - stop eat by killing the force */
            if (!CONTR(op->env)->rest_mode)
            {
                ndi(NDI_UNIQUE| NDI_NAVY, 0, op->env, "Your meal is interrupted!");
                CONTR(op->env)->food_status = 0;
                CLEAR_FLAG(op->env, FLAG_EATING);
                remove_ob(op);
                return;
            }

            /*ndi(NDI_UNIQUE, 0, op, "food-force ticks %d\n", op->stats.food);*/

            /* negative food_status count signals active food force (and not resting) as source for regeneration */
            CONTR(op->env)->food_status = (1000/op->last_eat* op->stats.food)*-1;

            /* regenerate hp, sp and grace if the force can do it */
            if(op->stats.hp && op->env->stats.hp <= op->env->stats.maxhp)
            {
                op->env->stats.hp += op->stats.hp;
                if(op->env->stats.hp > op->env->stats.maxhp)
                    op->env->stats.hp = op->env->stats.maxhp;
            }

            if(op->stats.sp && op->env->stats.sp <= op->env->stats.maxsp)
            {
                op->env->stats.sp += op->stats.sp;
                if(op->env->stats.sp > op->env->stats.maxsp)
                    op->env->stats.sp = op->env->stats.maxsp;
            }

            if(op->stats.grace && op->env->stats.grace <= op->env->stats.maxgrace)
            {
                op->env->stats.grace += op->stats.grace;
                if(op->env->stats.grace > op->env->stats.maxgrace)
                    op->env->stats.grace = op->env->stats.maxgrace;
            }
        }
    }
}

/* this is used from DRINK, FOOD & POISON forces now - to include buff/debuff
 * effects of stats & resists to the player. Cursed & damned effects are in too
 */
void create_food_buf_force(object_t *who, object_t *food, object_t *force)
{
    int i;

    force->stats.Str = food->stats.Str;
    force->stats.Pow = food->stats.Pow;
    force->stats.Dex = food->stats.Dex;
    force->stats.Con = food->stats.Con;
    force->stats.Int = food->stats.Int;
    force->stats.Wis = food->stats.Wis;
    force->stats.Cha = food->stats.Cha;

    for (i = 0; i < NROFATTACKS; i++)
        force->resist[i] = food->resist[i];

    /* if damned, set all negative if not and double or triple them */
    if (QUERY_FLAG(food, FLAG_CURSED))
    {
        if (force->stats.Str > 0)
            force->stats.Str = -force->stats.Str;
        force->stats.Str *= 2;
        if (force->stats.Dex > 0)
            force->stats.Dex = -force->stats.Dex;
        force->stats.Dex *= 2;
        if (force->stats.Con > 0)
            force->stats.Con = -force->stats.Con;
        force->stats.Con *= 2;
        if (force->stats.Int > 0)
            force->stats.Int = -force->stats.Int;
        force->stats.Int *= 2;
        if (force->stats.Wis > 0)
            force->stats.Wis = -force->stats.Wis;
        force->stats.Wis *= 2;
        if (force->stats.Pow > 0)
            force->stats.Pow = -force->stats.Pow;
        force->stats.Pow *= 2;
        if (force->stats.Cha > 0)
            force->stats.Cha = -force->stats.Cha;
        force->stats.Cha *= 2;
        for (i = 0; i < NROFATTACKS; i++)
        {
            if (force->resist[i] > 0)
                force->resist[i] = -force->resist[i];
            force->resist[i] *= 2;
        }
    }
    if (QUERY_FLAG(food, FLAG_DAMNED))
    {
        if (force->stats.Pow > 0)
            force->stats.Pow = -force->stats.Pow;
        force->stats.Pow *= 3;
        if (force->stats.Str > 0)
            force->stats.Str = -force->stats.Str;
        force->stats.Str *= 3;
        if (force->stats.Dex > 0)
            force->stats.Dex = -force->stats.Dex;
        force->stats.Dex *= 3;
        if (force->stats.Con > 0)
            force->stats.Con = -force->stats.Con;
        force->stats.Con *= 3;
        if (force->stats.Int > 0)
            force->stats.Int = -force->stats.Int;
        force->stats.Int *= 3;
        if (force->stats.Wis > 0)
            force->stats.Wis = -force->stats.Wis;
        force->stats.Wis *= 3;
        if (force->stats.Cha > 0)
            force->stats.Cha = -force->stats.Cha;
        force->stats.Cha *= 3;
        for (i = 0; i < NROFATTACKS; i++)
        {
            if (force->resist[i] > 0)
                force->resist[i] = -force->resist[i];
            force->resist[i] *= 3;
        }
    }
    if (food->speed_left)
        force->speed = food->speed_left;
        force = check_obj_stat_buffs(force, who);
        SET_FLAG(force, FLAG_APPLIED);
        force = insert_ob_in_ob(force, who);

    if (who->type == PLAYER)
        change_abil(who, force); /* Mostly to display any messages */
    else
        FIX_PLAYER(who ,"create food force (bug? can't be fix_player. fix_monster?"); /* huch? should be fix_monster, right? */
}
