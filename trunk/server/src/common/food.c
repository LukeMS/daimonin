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

    The author can be reached via e-mail to info@daimonin.net
*/

#include <global.h>

/* for pow() */
#include <math.h>

/* NOTE: For B4 we removed the old food system. Foods are now a regeneration force over time,
 * like you quaff a potion or cast a spell and the healing/reg effects comes in ticks for x seconds.
 * There is the old "dragon player code" still here. I like the idea very much and hopefully we can
 * add it later again to daimonin.
 * For that i removed FLESH from the eatable things for player. Only FOOD and DRINK will work like
 * a force here (honestly... we need drink as own type? ATM is just exactly the same as food)
 * MT-20.01.2007
 */

void apply_food(object *op, object *tmp)
{
    object *force;

    if (op->type != PLAYER)
    {
        /* food applied by non players... lets handle it later when we have a good idea */
        /*
        if(trigger_object_plugin_event(
                    EVENT_APPLY, tmp, op, NULL,
                    NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                    return;
        */
    }
    else
    {
        /* check if this is a dragon (player), eating some flesh */
        if (tmp->type == FLESH && is_dragon_pl(op) && dragon_eat_flesh(op, tmp))
        {
        }
        else
        {
            if (tmp->type == FLESH)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "You can't consume that!");
                return;
            }

            if(trigger_object_plugin_event(
                        EVENT_APPLY, tmp, op, NULL,
                        NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                return;
        }

        /* new food code for beta 4... food will now always work as force.
         * We want 2 things:
         * first, a regeneration every x ticks of hp and/or sp/grace.
         * second, AFTER the food is eaten we will perhaps install the food
         * as a buf force.
         * For that we copy the arch and do a type transmission to create
         * a force from sub type food. We search the player inventory,
         * removing every old food force and kick this new one in.
         * If the food is also a buf force, then we insert the food in the force
         * inventory and when the reg force is done, the buf is invoked.
         * This is a kind "after we have eaten the food complete, we get a STR+5
         * for 5 min".
         */

        force   = get_archetype("force");
        if (!force)
        {
            LOG(llevBug, "apply_food: can't create force object!?\n");
            return;
        }

        force->type = TYPE_FOOD_FORCE;
        SET_FLAG(force, FLAG_IS_USED_UP); /* or it will auto destroyed with first tick */
        SET_FLAG(force, FLAG_NO_SAVE);
        force->stats.food += tmp->last_eat; /* how long this force will stay */
        force->last_eat = tmp->last_eat;    /* we need that to know the base time */
        if (force->stats.food <= 0)
            force->stats.food = 1;
        force->stats.hp = tmp->stats.hp;
        force->stats.sp = tmp->stats.sp;
        force->stats.grace = tmp->stats.grace;
        force->speed = 0.125f;

        /* applying the food will put as in "rest mode" - but instead of rest regeneration we
         * will just eat... But eat will get interrupted when hit and such like normal rest too!
         * So, no eating in combat... perhaps a single tick will come in but on the price of a wasted food
         */

        CONTR(op)->food_status = -1000;
        CONTR(op)->rest_mode = 1;

        SET_FLAG(force, FLAG_APPLIED);
        SET_FLAG(op, FLAG_EATING);
        force = insert_ob_in_ob(force, op);

        new_draw_info_format(NDI_UNIQUE| NDI_NAVY, 0, op, "You start consuming the %s", STRING_SAFE(tmp->name));

    }
    decrease_ob(tmp);
}

/* remove all food forces (should be only one but safe is safe) from a player */
void remove_food_force(object *op)
{
    object *tmp, *tmp2;

    CLEAR_FLAG(op, FLAG_EATING);
    for(tmp=op->inv;tmp;tmp=tmp2) /* here are the damage infos */
    {
        tmp2=tmp->below;
        if(tmp->type == TYPE_FOOD_FORCE)
                remove_ob(tmp);
    }
}

/* Every time the food force is active, it increase the player stats,
 * like a reverse poison force.
 */
void food_force_reg(object *op)
{
    if(op->env && CONTR(op->env))
    {
        if(op->env->type == PLAYER)
        {
            /* implicit interruption of the food force - stop eat by killing the force */
            if (!CONTR(op->env)->rest_mode)
            {
                new_draw_info(NDI_UNIQUE| NDI_NAVY, 0, op->env, "Your meal is interrupted!");
                CONTR(op->env)->food_status = 0;
                CLEAR_FLAG(op->env, FLAG_EATING);
                remove_ob(op);
                return;
            }

            /*new_draw_info_format(NDI_UNIQUE, 0, op, "food-force ticks %d\n", op->stats.food);*/

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
void create_food_buf_force(object *who, object *food, object *force)
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

    SET_FLAG(force, FLAG_APPLIED);
    force = insert_ob_in_ob(force, who);
    if(who->type == PLAYER)
        change_abil(who, force); /* Mostly to display any messages */
    else
        FIX_PLAYER(who ,"create food force (bug? can't be fix_player. fix_monster?"); /* huch? should be fix_monster, right? */
}


/*
 * A dragon is eating some flesh. If the flesh contains resistances,
 * there is a chance for the dragon's skin to get improved.
 *
 * attributes:
 *     object *op        the object (dragon player) eating the flesh
 *     object *meal      the flesh item, getting chewed in dragon's mouth
 * return:
 *     int               1 if eating successful, 0 if it doesn't work
 */
int dragon_eat_flesh(object *op, object *meal)
{
    object *skin        = NULL;    /* pointer to dragon skin force*/
    object *abil        = NULL;    /* pointer to dragon ability force*/
    object *tmp         = NULL;     /* tmp. object */

    char    buf[MAX_BUF];            /* tmp. string buffer */
    double  chance;                /* improvement-chance of one resistance type */
    double  maxchance   = 0;           /* highest chance of any type */
    double  bonus       = 0;               /* level bonus (improvement is easier at lowlevel) */
    double  mbonus      = 0;              /* monster bonus */
    int     atnr_winner[NROFATTACKS]; /* winning candidates for resistance improvement */
    int     winners     = 0;                /* number of winners */
    int     i;                        /* index */

    /* let's make sure and doublecheck the parameters */
    if (meal->type != FLESH || !is_dragon_pl(op))
        return 0;

    /* now grab the 'dragon_skin'- and 'dragon_ability'-forces
       from the player's inventory */
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
    {
        if (tmp->type == FORCE)
        {
            if (tmp->arch->name == shstr_cons.dragon_skin_force)
                skin = tmp;
            else if (tmp->arch->name == shstr_cons.dragon_ability_force)
                abil = tmp;
        }
    }

    /* if either skin or ability are missing, this is an old player
       which is not to be considered a dragon -> bail out */
    if (skin == NULL || abil == NULL)
        return 0;

    /* now start by filling stomache and health, according to food-value */
    if ((999 - op->stats.food) < meal->stats.food)
        op->stats.hp += (999 - op->stats.food) / 50;
    else
        op->stats.hp += meal->stats.food / 50;
    if (op->stats.hp > op->stats.maxhp)
        op->stats.hp = op->stats.maxhp;

    op->stats.food = MIN(999, op->stats.food + meal->stats.food);

    /*printf("-> player: %d, flesh: %d\n", op->level, meal->level);*/

    /* on to the interesting part: chances for adding resistance */
    for (i = 0; i < NROFATTACKS; i++)
    {
        if (meal->resist[i] > 0 && atnr_is_dragon_enabled(i))
        {
            /* got positive resistance, now calculate improvement chance (0-100) */

            /* this bonus makes resistance increase easier at lower levels */
            bonus = (MAXLEVEL - op->level) * 30. / ((double) MAXLEVEL);
            if (i == abil->stats.exp)
                bonus += 5;  /* additional bonus for resistance of ability-focus */

            /* monster bonus increases with level, because high-level
               flesh is too rare */
            mbonus = op->level * 20. / ((double) MAXLEVEL);

            chance = (((double) MIN(op->level + bonus, meal->level + bonus + mbonus)) * 100. / ((double) MAXLEVEL))
                   - skin->resist[i];

            if (chance >= 0.)
                chance += 1.;
            else
                chance = (chance < -12) ? 0. : 1. / pow(2., -chance);

            /* chance is proportional to amount of resistance (max. 50) */
            chance *= ((double) (MIN(meal->resist[i], 50))) / 50.;

            /* doubled chance for resistance of ability-focus */
            if (i == abil->stats.exp)
                chance = MIN(100., chance * 2.);

            /* now make the throw and save all winners (Don't insert luck bonus here!) */
            if (RANDOM() % 10000 < (int) (chance * 100))
            {
                atnr_winner[winners] = i;
                winners++;
            }

            if (chance > maxchance)
                maxchance = chance;

            /*printf("   %s: bonus %.1f, chance %.1f\n", attacks[i], bonus, chance);*/
        }
    }

    /* print message according to maxchance */
    if (maxchance > 50.)
        sprintf(buf, "Hmm! The %s tasted delicious!", meal->name);
    else if (maxchance > 10.)
        sprintf(buf, "The %s tasted very good.", meal->name);
    else if (maxchance > 1.)
        sprintf(buf, "The %s tasted good.", meal->name);
    else if (maxchance > 0.0001)
        sprintf(buf, "The %s had a boring taste.", meal->name);
    else if (meal->last_eat > 0 && atnr_is_dragon_enabled(meal->last_eat))
        sprintf(buf, "The %s tasted strange.", meal->name);
    else
        sprintf(buf, "The %s had no taste.", meal->name);
    new_draw_info(NDI_UNIQUE, 0, op, buf);

    /* now choose a winner if we have any */
    i = -1;
    if (winners > 0)
        i = atnr_winner[RANDOM() % winners];

    if (i >= 0 && i < NROFATTACKS && skin->resist[i] < 95)
    {
        /* resistance increased! */
        skin->resist[i]++;
        FIX_PLAYER(op ,"dragon eat flesh - resist");

        sprintf(buf, "Your skin is now more resistant to %s!", attack_name[i]);
        new_draw_info(NDI_UNIQUE | NDI_RED, 0, op, buf);
    }

    /* if this flesh contains a new ability focus, we mark it
       into the ability_force and it will take effect on next level */
    if (meal->last_eat > 0 && atnr_is_dragon_enabled(meal->last_eat) && meal->last_eat != abil->last_eat)
    {
        abil->last_eat = meal->last_eat; /* write: last_eat <new attnr focus> */

        if (meal->last_eat != abil->stats.exp)
        {
            sprintf(buf, "Your metabolism prepares to focus on %s!", attack_name[meal->last_eat]);
            new_draw_info(NDI_UNIQUE, 0, op, buf);
            sprintf(buf, "The change will happen at level %d", abil->level + 1);
            new_draw_info(NDI_UNIQUE, 0, op, buf);
        }
        else
        {
            sprintf(buf, "Your metabolism will continue to focus on %s.", attack_name[meal->last_eat]);
            new_draw_info(NDI_UNIQUE, 0, op, buf);
            abil->last_eat = 0;
        }
    }
    return 1;
}

