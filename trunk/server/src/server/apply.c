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
#include <tod.h>

/* need math lib for double-precision and pow() in dragon_eat_flesh() */
#include <math.h>

#if defined(vax) || defined(ibm032)
size_t  strftime(char *, size_t, const char *, const struct tm *);
time_t  mktime(struct tm *);
#endif

void draw_find(object *op, object *find)
{
    new_draw_info_format(NDI_UNIQUE, 0, op, "You find %s in the chest.", query_name(find));
}

/*
 * Return value: 1 if money was destroyed, 0 if not.
 */
static int apply_id_altar(object *money, object *altar, object *pl)
{
    object *id, *marked;
    int     success = 0;

    if (pl == NULL || pl->type != PLAYER)
        return 0;

    /* Check for MONEY type is a special hack - it prevents 'nothing needs
     * identifying' from being printed out more than it needs to be.
     */
    if (!check_altar_sacrifice(altar, money) || money->type != MONEY)
        return 0;

    /* Event trigger and quick exit */
    if(trigger_object_plugin_event(EVENT_TRIGGER,
                altar, pl, money,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
        return FALSE;

    marked = find_marked_object(pl);
    /* if the player has a marked item, identify that if it needs to be
     * identified.  IF it doesn't, then go through the player inventory.
     */
    if (marked && !QUERY_FLAG(marked, FLAG_IDENTIFIED) && need_identify(marked))
    {
        if (operate_altar(altar, &money))
        {
            identify(marked);
            new_draw_info_format(NDI_UNIQUE, 0, pl, "You have %s.", long_desc(marked, pl));
            if (marked->msg)
            {
                new_draw_info(NDI_UNIQUE, 0, pl, "The item has a story:");
                new_draw_info(NDI_UNIQUE, 0, pl, marked->msg);
            }
            return money == NULL;
        }
    }

    for (id = pl->inv; id; id = id->below)
    {
        if (!QUERY_FLAG(id, FLAG_IDENTIFIED) && !IS_INVISIBLE(id, pl) && need_identify(id))
        {
            if (operate_altar(altar, &money))
            {
                identify(id);
                new_draw_info_format(NDI_UNIQUE, 0, pl, "You have %s.", long_desc(id, pl));
                if (id->msg)
                {
                    new_draw_info(NDI_UNIQUE, 0, pl, "The item has a story:");
                    new_draw_info(NDI_UNIQUE, 0, pl, id->msg);
                }
                success = 1;
                /* If no more money, might as well quit now */
                if (money == NULL || !check_altar_sacrifice(altar, money))
                    break;
            }
            else
            {
                LOG(llevBug, "check_id_altar:  Couldn't do sacrifice when we should have been able to\n");
                break;
            }
        }
    }
    if (!success)
        new_draw_info(NDI_UNIQUE, 0, pl, "You have nothing that needs identifying");
    return money == NULL;
}

int apply_potion(object *op, object *tmp)
{
    int i, bonus = 1;

    /* some sanity checks */
    if (!op || !tmp)
    {
        LOG(llevBug, "apply_potion() called with invalid objects! obj: %s -- tmp: %s\n", query_name(op), query_name(tmp));
        return 0;
    }

    if(trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return 0;

    if (op->type == PLAYER)
    {
        /* set chosen_skill to "magic device" - thats used when we "use" a potion */
        if (!change_skill(op, SK_USE_MAGIC_ITEM))
            return 0; /* no skill, no potion use (dust & balm too!) */

        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            identify(tmp);

        /* special potions. Only players get this */
        if (tmp->last_eat == -1) /* create a force and copy the effects in */
        {
            object *force   = get_archetype("force");

            if (!force)
            {
                LOG(llevBug, "apply_potion: can't create force object!?\n");
                return 0;
            }

            force->type = POTION_EFFECT;
            SET_FLAG(force, FLAG_IS_USED_UP); /* or it will auto destroyed with first tick */
            force->stats.food += tmp->stats.food; /* how long this force will stay */
            if (force->stats.food <= 0)
                force->stats.food = 1;

            /* negative effects because cursed or damned */
            if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
            {
                /* now we have a bit work because we change (multiply,...) the
                         * base values of the potion - that can invoke out of bounce
                         * values we must catch here.
                         */

                force->stats.food *= 3; /* effects stays 3 times longer */
                for (i = 0; i < NROFATTACKS; i++)
                {
                    int tmp_r, tmp_a;

                    tmp_r = tmp->resist[i] > 0 ? -tmp->resist[i] : tmp->resist[i];
                    tmp_a = tmp->attack[i] > 0 ? -tmp->attack[i] : tmp->attack[i];

                    /* double bad effect when damned */
                    if (QUERY_FLAG(tmp, FLAG_DAMNED))
                    {
                        tmp_r *= 2;
                        tmp_a *= 2;
                    }

                    /* we don't want out of bound values ... */
                    if ((int) force->resist[i] + tmp_r > 100)
                        force->resist[i] = 100;
                    else if ((int) force->resist[i] + tmp_r < -100)
                        force->resist[i] = -100;
                    else
                        force->resist[i] += (sint8) tmp_r;

                    if ((int) force->attack[i] + tmp_a > 100)
                        force->attack[i] = 100;
                    else if ((int) force->attack[i] + tmp_a < 0)
                        force->attack[i] = 0;
                    else
                        force->attack[i] += tmp_a;
                }

                insert_spell_effect("meffect_purple", op->map, op->x, op->y);
                play_sound_map(op->map, op->x, op->y, SOUND_DRINK_POISON, SOUND_NORMAL);
            }
            else /* all positive (when not on default negative) */
            {
                /* we don't must do the hard way like cursed/damned (no multiplication or
                         * sign change).
                         */
                memcpy(force->resist, tmp->resist, sizeof(tmp->resist));
                memcpy(force->attack, tmp->attack, sizeof(tmp->attack));
                insert_spell_effect("meffect_green", op->map, op->x, op->y);
                play_sound_map(op->map, op->x, op->y, SOUND_MAGIC_DEFAULT, SOUND_SPELL);
            }

            /* now copy stats values */
            if(QUERY_FLAG(tmp, FLAG_DAMNED))
                bonus = -2;
            else if(QUERY_FLAG(tmp, FLAG_CURSED))
                bonus = -1;
            force->stats.Str = MIN(tmp->stats.Str, tmp->stats.Str * bonus);
            force->stats.Con = MIN(tmp->stats.Con, tmp->stats.Con * bonus);
            force->stats.Dex = MIN(tmp->stats.Dex, tmp->stats.Dex * bonus);
            force->stats.Int = MIN(tmp->stats.Int, tmp->stats.Int * bonus);
            force->stats.Wis = MIN(tmp->stats.Wis, tmp->stats.Wis * bonus);
            force->stats.Pow = MIN(tmp->stats.Pow, tmp->stats.Pow * bonus);
            force->stats.Cha = MIN(tmp->stats.Cha, tmp->stats.Cha * bonus);

            /* kick the force in, and apply it to player */
            force->speed_left = -1;
            force = insert_ob_in_ob(force, op);
            CLEAR_FLAG(tmp, FLAG_APPLIED);
            SET_FLAG(force, FLAG_APPLIED);
            if (!change_abil(op, force)) /* implicit fix_player() here */
                new_draw_info(NDI_UNIQUE, 0, op, "Nothing happened.");
            decrease_ob(tmp);
            return 1;
        }

        if (tmp->last_eat == 1) /* Potion of minor restoration */
        {
            object     *depl;
            archetype  *at;
            if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
            {
                if (QUERY_FLAG(tmp, FLAG_DAMNED))
                {
                    drain_stat(op);
                    drain_stat(op);
                    drain_stat(op);
                    drain_stat(op);
                }
                else
                {
                    drain_stat(op);
                    drain_stat(op);
                }
                FIX_PLAYER(op ,"apply_potion - minor restoration - damned");
                decrease_ob(tmp);
                insert_spell_effect("meffect_purple", op->map, op->x, op->y);
                play_sound_map(op->map, op->x, op->y, SOUND_DRINK_POISON, SOUND_NORMAL);
                return 1;
            }
            if ((at = find_archetype("depletion")) == NULL)
            {
                LOG(llevBug, "BUG: Could not find archetype depletion");
                return 0;
            }
            depl = present_arch_in_ob(at, op);
            if (depl != NULL)
            {
                for (i = 0; i < 7; i++)
                {
                    if (get_attr_value(&depl->stats, i))
                        new_draw_info(NDI_UNIQUE, 0, op, restore_msg[i]);
                }
                remove_ob(depl); /* in inventory of ... */
                FIX_PLAYER(op ,"apply_potion - minor restoration");
            }
            else
                new_draw_info(NDI_UNIQUE, 0, op, "You feel a great loss...");
            decrease_ob(tmp);
            insert_spell_effect("meffect_green", op->map, op->x, op->y);
            play_sound_map(op->map, op->x, op->y, SOUND_MAGIC_DEFAULT, SOUND_SPELL);
            return 1;
        }
        else if (tmp->last_eat == 2)    /* improvement potion */
        {
            int success_flag = 0, hp_flag = 0, sp_flag = 0, grace_flag = 0;

            if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
            {
                /* jump in by random - goto power */
                if (RANDOM() % 2)
                    goto hp_jump;
                else if (RANDOM() % 2)
                    goto sp_jump;
                else
                    goto grace_jump;

                while (!hp_flag || !sp_flag || !grace_flag)
                {
                    hp_jump:
                    hp_flag = 1; /* mark we have checked hp chain */
                    for (i = 2; i <= op->level; i++)
                    {
                        /* move one value to max */
                        if (CONTR(op)->levhp[i] != 1)
                        {
                            CONTR(op)->levhp[i] = 1;
                            success_flag = 2;
                            goto improve_done;
                        }
                    }
                    sp_jump:
                    sp_flag = 1; /* mark we have checked sp chain */
                    for (i = 2; i <= CONTR(op)->exp_obj_ptr[SKILLGROUP_MAGIC]->level; i++)
                    {
                        /* move one value to max */
                        if (CONTR(op)->levsp[i] != 1)
                        {
                            CONTR(op)->levsp[i] = 1;
                            success_flag = 2;
                            goto improve_done;
                        }
                    }
                    grace_jump:
                    grace_flag = 1; /* mark we have checked grace chain */
                    for (i = 2; i <= CONTR(op)->exp_obj_ptr[SKILLGROUP_WISDOM]->level; i++)
                    {
                        /* move one value to max */
                        if (CONTR(op)->levgrace[i] != 1)
                        {
                            CONTR(op)->levgrace[i] = 1;
                            success_flag = 2;
                            goto improve_done;
                        }
                    }
                };
                success_flag = 3;
            }
            else
            {
                /* jump in by random - goto power */
                if (RANDOM() % 2)
                    goto hp_jump2;
                else if (RANDOM() % 2)
                    goto sp_jump2;
                else
                    goto grace_jump2;

                while (!hp_flag || !sp_flag || !grace_flag)
                {
                    hp_jump2:
                    hp_flag = 1; /* mark we have checked hp chain */
                    for (i = 2; i <= op->level; i++)
                    {
                        /* move one value to max */
                        if (CONTR(op)->levhp[i] != (char) op->arch->clone.stats.maxhp)
                        {
                            CONTR(op)->levhp[i] = (char) op->arch->clone.stats.maxhp;
                            success_flag = 1;
                            goto improve_done;
                        }
                    }
                    sp_jump2:
                    sp_flag = 1; /* mark we have checked sp chain */
                    for (i = 2; i <= CONTR(op)->exp_obj_ptr[SKILLGROUP_MAGIC]->level; i++)
                    {
                        /* move one value to max */
                        if (CONTR(op)->levsp[i] != (char) op->arch->clone.stats.maxsp)
                        {
                            CONTR(op)->levsp[i] = (char) op->arch->clone.stats.maxsp;
                            success_flag = 1;
                            goto improve_done;
                        }
                    }
                    grace_jump2:
                    grace_flag = 1; /* mark we have checked grace chain */
                    for (i = 2; i <= CONTR(op)->exp_obj_ptr[SKILLGROUP_WISDOM]->level; i++)
                    {
                        /* move one value to max */
                        if (CONTR(op)->levgrace[i] != (char) op->arch->clone.stats.maxgrace)
                        {
                            CONTR(op)->levgrace[i] = (char) op->arch->clone.stats.maxgrace;
                            success_flag = 1;
                            goto improve_done;
                        }
                    }
                };
            }

            improve_done:
            CLEAR_FLAG(tmp, FLAG_APPLIED);
            if (!success_flag)
            {
                new_draw_info(NDI_UNIQUE, 0, op, "The potion had no effect - you are already perfect.");
                play_sound_map(op->map, op->x, op->y, SOUND_MAGIC_DEFAULT, SOUND_SPELL);
            }
            else if (success_flag == 1)
            {
                FIX_PLAYER(op ,"apply_potion - improvement");
                insert_spell_effect("meffect_yellow", op->map, op->x, op->y);
                play_sound_map(op->map, op->x, op->y, SOUND_MAGIC_DEFAULT, SOUND_SPELL);
                new_draw_info(NDI_UNIQUE, 0, op, "You feel a little more perfect!");
            }
            else if (success_flag == 2)
            {
                FIX_PLAYER(op ,"apply_potion - improvement - cursed");
                insert_spell_effect("meffect_purple", op->map, op->x, op->y);
                play_sound_map(op->map, op->x, op->y, SOUND_DRINK_POISON, SOUND_NORMAL);
                new_draw_info(NDI_UNIQUE, 0, op, "The foul potion burns like fire in you!");
            }
            else /* bad potion but all values of this player are 1! poor poor guy.... */
            {
                insert_spell_effect("meffect_purple", op->map, op->x, op->y);
                play_sound_map(op->map, op->x, op->y, SOUND_DRINK_POISON, SOUND_NORMAL);
                new_draw_info(NDI_UNIQUE, 0, op, "The potion was foul but had no effect on your tortured body.");
            }
            decrease_ob(tmp);
            return 1;
        }
    }


    if (tmp->stats.sp == SP_NO_SPELL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Nothing happens as you apply it.");
        decrease_ob(tmp);
        return 0;
    }


    /* A potion that casts a spell.  Healing, restore spellpoint (power potion)
     * and heroism all fit into this category.
     */
    if (tmp->stats.sp != SP_NO_SPELL)
    {
        cast_spell(op, tmp, 0, tmp->stats.sp, 1, spellPotion, NULL); /* apply potion ALWAYS fire on the spot the applier stands - good for healing - bad for firestorm */
        decrease_ob(tmp);
        /* if youre dead, no point in doing this... */
        if (!QUERY_FLAG(op, FLAG_REMOVED))
            FIX_PLAYER(op ,"apply_potion - cast something");
        return 1;
    }

    /* CLEAR_FLAG is so that if the character has other potions
     * that were grouped with the one consumed, his
     * stat will not be raised by them.  fix_player just clears
     * up all the stats.
     */
    CLEAR_FLAG(tmp, FLAG_APPLIED);
    FIX_PLAYER(op ,"apply_potion - end");
    decrease_ob(tmp);
    return 1;
}

/****************************************************************************
 * Weapon improvement code follows
 ****************************************************************************/

int check_item(object *op, const char *item)
{
    int count   = 0;


    if (item == NULL)
        return 0;
    op = op->below;
    while (op != NULL)
    {
        if (op->arch->name == item)
        {
            if (!QUERY_FLAG(op, FLAG_CURSED) && !QUERY_FLAG(op, FLAG_DAMNED)
                /* Loophole bug? -FD- */ && !QUERY_FLAG(op, FLAG_UNPAID))
            {
                if (op->nrof == 0)/* this is necessary for artifact sacrifices --FD-- */
                    count++;
                else
                    count += op->nrof;
            }
        }
        op = op->below;
    }
    return count;
}

void eat_item(object *op, const char *item)
{
    object *prev;

    prev = op;
    op = op->below;

    while (op != NULL)
    {
        if (op->arch->name == item)
        {
            decrease_ob_nr(op, op->nrof);
            op = prev;
        }
        prev = op;
        op = op->below;
    }
}

/* This checks to see of the player (who) is sufficient level to use a weapon
 * with improvs improvements (typically last_eat).  We take an int here
 * instead of the object so that the improvement code can pass along the
 * increased value to see if the object is usable.
 * we return 1 (true) if the player can use the weapon.
 */
int check_weapon_power(object *who, int improvs)
{
    int level   = who->level;

    /* The skill system hands out wc and dam bonuses to fighters
     * more generously than the old system (see fix_player). Thus
     * we need to curtail the power of player enchanted weapons.
     * I changed this to 1 improvement per "fighter" level/5 -b.t.
     * Note:  Nothing should break by allowing this ratio to be different or
     * using normal level - it is just a matter of play balance.
     */
    if (who->type == PLAYER)
    {
        object *wc_obj  = NULL;

        for (wc_obj = who->inv; wc_obj; wc_obj = wc_obj->below)
            if (wc_obj->type == EXPERIENCE && wc_obj->stats.Str)
                break;
        if (!wc_obj)
            LOG(llevBug, "BUG: Player: %s lacks wc experience object\n", who->name);
        else
            level = wc_obj->level;
    }
    return (improvs <= ((level / 5) + 5));
}

/* Returns the object count that of the number of objects found that
 * improver wants.
 */
static int check_sacrifice(object *op, object *improver)
{
    int count   = 0;

    if (improver->slaying != NULL)
    {
        count = check_item(op, improver->slaying);
        if (count < 1)
        {
            char    buf[200];
            sprintf(buf, "The gods want more %ss", improver->slaying);
            new_draw_info(NDI_UNIQUE, 0, op, buf);
            return 0;
        }
    }
    else
        count = 1;

    return count;
}

int improve_weapon_stat(object *op, object *improver, object *weapon, signed char *stat, int sacrifice_count,
                        char *statname)
{
    new_draw_info(NDI_UNIQUE, 0, op, "Your sacrifice was accepted.");
    *stat += sacrifice_count;
    weapon->last_eat++;
    new_draw_info_format(NDI_UNIQUE, 0, op, "Weapon's bonus to %s improved by %d", statname, sacrifice_count);
    decrease_ob(improver);

    /* So it updates the players stats and the window */
    FIX_PLAYER(op ,"improve weapon stat");
    return 1;
}

/* Types of improvements, hidden in the sp field. */
#define IMPROVE_PREPARE 1
#define IMPROVE_DAMAGE 2
#define IMPROVE_WEIGHT 3
#define IMPROVE_ENCHANT 4
#define IMPROVE_STR 5
#define IMPROVE_DEX 6
#define IMPROVE_CON 7
#define IMPROVE_WIS 8
#define IMPROVE_CHA 9
#define IMPROVE_INT 10
#define IMPROVE_POW 11


/* This does the prepare weapon scroll */

int prepare_weapon(object *op, object *improver, object *weapon)
{
    int     sacrifice_count, i;
    char    buf[MAX_BUF];

    if (weapon->level != 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Weapon already prepared.");
        return 0;
    }
    for (i = 0; i < NROFATTACKS; i++)
        if (weapon->resist[i])
            break;

    /* If we break out, i will be less than nrofattacks, preventing
     * improvement of items that already have protections.
     */
    if (i < NROFATTACKS || weapon->stats.hp ||  /* regeneration */
        weapon->stats.sp ||     /* sp regeneration */
        weapon->stats.exp ||    /* speed */
        weapon->stats.ac)   /* AC - only taifu's I think */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Cannot prepare magic weapons.");
        return 0;
    }
    sacrifice_count = check_sacrifice(op, improver);
    if (sacrifice_count <= 0)
        return 0;
    sacrifice_count = isqrt(sacrifice_count);
    weapon->level = sacrifice_count;
    new_draw_info(NDI_UNIQUE, 0, op, "Your sacrifice was accepted.");
    eat_item(op, improver->slaying);

    new_draw_info_format(NDI_UNIQUE, 0, op, "Your *%s may be improved %d times.", weapon->name, sacrifice_count);

    sprintf(buf, "%s's %s", op->name, weapon->name);
    FREE_AND_COPY_HASH(weapon->name, buf);

    weapon->nrof = 0;  /*  prevents preparing n weapons in the same
                       slot at once! */
    decrease_ob(improver);
    weapon->last_eat = 0;
    return 1;
}


/* This is the new improve weapon code */
/* build_weapon returns 0 if it was not able to work. */
/* #### We are hiding extra information about the weapon in the level and
   last_eat numbers for an object.  Hopefully this won't break anything ??
   level == max improve last_eat == current improve*/
int improve_weapon(object *op, object *improver, object *weapon)
{
    int sacrifice_count, sacrifice_needed = 0;

    if (improver->stats.sp == IMPROVE_PREPARE)
    {
        return prepare_weapon(op, improver, weapon);
    }
    if (weapon->level == 0)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "This weapon has not been prepared.");
        return 0;
    }
    if (weapon->level == weapon->last_eat)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "This weapon cannot be improved any more.");
        return 0;
    }
    if (QUERY_FLAG(weapon, FLAG_APPLIED) && !check_weapon_power(op, weapon->last_eat + 1))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Improving the weapon will make it too");
        new_draw_info(NDI_UNIQUE, 0, op, "powerful for you to use.  Unready it if you");
        new_draw_info(NDI_UNIQUE, 0, op, "really want to improve it.");
        return 0;
    }
    /* This just increases damage by 5 points, no matter what.  No sacrifice
     * is needed.  Since stats.dam is now a 16 bit value and not 8 bit,
     * don't put any maximum value on damage - the limit is how much the
     * weapon  can be improved.
     */
    if (improver->stats.sp == IMPROVE_DAMAGE)
    {
        weapon->stats.dam += 5;
        weapon->weight += 5000;     /* 5 KG's */
        new_draw_info_format(NDI_UNIQUE, 0, op, "Damage has been increased by 5 to %d", weapon->stats.dam);
        weapon->last_eat++;
        decrease_ob(improver);
        return 1;
    }
    if (improver->stats.sp == IMPROVE_WEIGHT)
    {
        /* Reduce weight by 20% */
        weapon->weight = (weapon->weight * 8) / 10;
        if (weapon->weight < 1)
            weapon->weight = 1;
        new_draw_info_format(NDI_UNIQUE, 0, op, "Weapon weight reduced to %6.1f kg", (float) weapon->weight / 1000.0);
        weapon->last_eat++;
        decrease_ob(improver);
        return 1;
    }
    if (improver->stats.sp == IMPROVE_ENCHANT)
    {
        weapon->magic++;
        weapon->last_eat++;
        new_draw_info_format(NDI_UNIQUE, 0, op, "Weapon magic increased to %d", weapon->magic);
        decrease_ob(improver);
        return 1;
    }

    sacrifice_needed = weapon->stats.Str
                     + weapon->stats.Int
                     + weapon->stats.Dex
                     + weapon->stats.Pow
                     + weapon->stats.Con
                     + weapon->stats.Cha
                     + weapon->stats.Wis;

    if (sacrifice_needed < 1)
        sacrifice_needed = 1;
    sacrifice_needed *= 2;

    sacrifice_count = check_sacrifice(op, improver);
    if (sacrifice_count < sacrifice_needed)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "You need at least %d %s", sacrifice_needed, improver->slaying);
        return 0;
    }
    eat_item(op, improver->slaying);

    switch (improver->stats.sp)
    {
        case IMPROVE_STR:
          return improve_weapon_stat(op, improver, weapon, (signed char *) &(weapon->stats.Str), 1, (char *) "strength");
        case IMPROVE_DEX:
          return improve_weapon_stat(op, improver, weapon, (signed char *) &(weapon->stats.Dex), 1, (char *) "dexterity");
        case IMPROVE_CON:
          return improve_weapon_stat(op, improver, weapon, (signed char *) &(weapon->stats.Con), 1,
                                     (char *) "constitution");
        case IMPROVE_WIS:
          return improve_weapon_stat(op, improver, weapon, (signed char *) &(weapon->stats.Wis), 1, (char *) "wisdom");
        case IMPROVE_CHA:
          return improve_weapon_stat(op, improver, weapon, (signed char *) &(weapon->stats.Cha), 1, (char *) "charisma");
        case IMPROVE_INT:
          return improve_weapon_stat(op, improver, weapon, (signed char *) &(weapon->stats.Int), 1,
                                     (char *) "intelligence");
        case IMPROVE_POW:
          return improve_weapon_stat(op, improver, weapon, (signed char *) &(weapon->stats.Pow), 1, (char *) "power");
        default:
          new_draw_info(NDI_UNIQUE, 0, op, "Unknown improvement type.");
    }
    LOG(llevBug, "BUG: improve_weapon: Got to end of function\n");
    return 0;
}

int check_improve_weapon(object *op, object *tmp)
{
    object *otmp;

    if (op->type != PLAYER)
        return 0;
    if (blocks_magic(op->map, op->x, op->y))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something blocks the magic of the scroll.");
        return 0;
    }
    otmp = find_marked_object(op);
    if (!otmp)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You need to mark a weapon object.");
        return 0;
    }
    if (otmp->type != WEAPON)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Marked item is not a weapon");
        return 0;
    }

    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return 1;

    new_draw_info(NDI_UNIQUE, 0, op, "Applied weapon builder.");
    improve_weapon(op, tmp, otmp);
    esrv_send_item(op, otmp);
    return 1;
}

/* this code is by b.t. (thomas@nomad.astro.psu.edu) -
 * only 'enchantment' of armour is possible - improving
 * the stats of a player w/ armour as well as a weapon
 * will probably horribly unbalance the game. Magic enchanting
 * depends on the level of the character - ie the plus
 * value (magic) of the armour can never be increased beyond
 * the level of the character / 10 -- rounding upish, nor may
 * the armour value of the piece of equipment exceed either
 * the users level or 90)
 * Modified by MSW for partial resistance.  Only support
 * changing of physical area right now.
 */

int improve_armour(object *op, object *improver, object *armour)
{
    int new_armour;

    new_armour = armour->resist[ATNR_PHYSICAL] + armour->resist[ATNR_PHYSICAL] / 25 + op->level / 20 + 1;
    if (new_armour > 90)
        new_armour = 90;

    if (armour->magic >= (op->level / 10 + 1) || new_armour > op->level)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You are not yet powerfull enough");
        new_draw_info(NDI_UNIQUE, 0, op, "to improve this armour.");
        return 0;
    }

    if (new_armour > armour->resist[ATNR_PHYSICAL])
    {
        armour->resist[ATNR_PHYSICAL] = new_armour;
        armour->weight += (unsigned long) ((double) armour->weight * (double) 0.05);
    }
    else
    {
        new_draw_info(NDI_UNIQUE, 0, op, "The armour value of this equipment");
        new_draw_info(NDI_UNIQUE, 0, op, "cannot be further improved.");
    }
    armour->magic++;
    if (op->type == PLAYER)
    {
        esrv_send_item(op, armour);
        if (QUERY_FLAG(armour, FLAG_APPLIED))
            FIX_PLAYER(op ,"improve armour");
    }
    decrease_ob(improver);
    return 1;
}


/*
 * convert_item() returns 1 if anything was converted, otherwise 0
 */
#define CONV_FROM(xyz)  (xyz->slaying)
#define CONV_TO(xyz)    (xyz->other_arch)
#define CONV_NR(xyz)    ((unsigned long) xyz->stats.sp)      /* receive number */
#define CONV_NEED(xyz)  ((unsigned long) xyz->stats.food)    /* cost number */

int convert_item(object *item, object *converter, object *originator)
{
    int     nr  = 0;
    object *tmp;

    /* We make some assumptions - we assume if it takes money as it type,
     * it wants some amount.  We don't make change (ie, if something costs
     * 3 gp and player drops a platinum, tough luck)
     */
    if (CONV_FROM(converter) == shstr_cons.money)
    {
        sint64 cost;
        nr = (int) (((sint64)item->nrof * item->value) / (sint64)CONV_NEED(converter));
        if (!nr)
            return 0;

        if(trigger_object_plugin_event(EVENT_TRIGGER,
                converter, item, originator,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
            return 0;

        cost = nr * CONV_NEED(converter) / item->value;
        /* take into account rounding errors */
        if (nr * CONV_NEED(converter) % item->value)
            cost++;

        /* this is outdated code too... we have here a overflow problem */
        if(cost >(1<<31))
            cost = (1<<31);
        decrease_ob_nr(item, (int)cost);
    }
    else
    {
        if (item->type == PLAYER
         || CONV_FROM(converter) != item->arch->name
         || (CONV_NEED(converter) && CONV_NEED(converter) > item->nrof))
            return 0;

        if(trigger_object_plugin_event(EVENT_TRIGGER,
                converter, item, originator,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
            return 0;

        if (CONV_NEED(converter))
        {
            nr = item->nrof / CONV_NEED(converter);
            decrease_ob_nr(item, nr * CONV_NEED(converter));
        }
        else
        {
            remove_ob(item);
            check_walk_off(item, NULL, MOVE_APPLY_VANISHED);
        }
    }
    item = arch_to_object(converter->other_arch);
    if (CONV_NR(converter))
        item->nrof = CONV_NR(converter);
    if (nr)
        item->nrof *= nr;
    for (tmp = get_map_ob(converter->map, converter->x, converter->y); tmp != NULL; tmp = tmp->above)
    {
        if (tmp->type == SHOP_FLOOR)
            break;
    }
    if (tmp != NULL)
        SET_FLAG(item, FLAG_UNPAID);
    item->x = converter->x;
    item->y = converter->y;
    insert_ob_in_map(item, converter->map, converter, 0);
    return 1;
}

/*
 * Eneq(@csd.uu.se): Handle apply on containers.
 * op is the player, sack is the container the player is opening or closing.
 * return 1 if an object is apllied somehow or another, 0 if error/no apply
 *
 * Reminder - there are three states for any container - closed (non applied),
 * applied (not open, but objects that match get tossed into it), and open
 * (applied flag set, and op->container points to the open container)
 * I added mutiple apply of one container with a player list. MT 07.02.2004
 */

int esrv_apply_container(object *op, object *sack)
{
    object *cont, *tmp;

    /*
     * TODO: add support for cursed containers that can't be unreadied?
     */

    if (op->type != PLAYER)
    {
        LOG(llevBug, "BUG: esrv_apply_container: called from non player: <%s>!\n", query_name(op));
        return 0;
    }

    cont = CONTR(op)->container; /* cont is NULL or the container player already has opened */

    if (sack == NULL || sack->type != CONTAINER || (cont && cont->type != CONTAINER))
    {
        LOG(llevBug, "BUG: esrv_apply_container: object *sack = %s is not container (cont:<%s>)!\n", query_name(sack),
            query_name(cont));
        return 0;
    }


    /* close container? */
    if (cont) /* if cont != sack || cont == sack - in both cases we close cont */
    {
        if(trigger_object_plugin_event(EVENT_CLOSE, cont, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
            return 1;

        if (container_unlink(CONTR(op), cont))
            new_draw_info_format(NDI_UNIQUE, 0, op, "You close the %s.", query_name(cont));
        else
            new_draw_info_format(NDI_UNIQUE, 0, op, "You leave the %s.", query_name(cont));


        if (cont == sack) /* we closing the one we applied */
            return 1;
    }

    /* at this point we ready a container OR we open it! */

    /* If the player is trying to open it (which he must be doing if we got here),
     * and it is locked, check to see if player has the equipment to open it.
     */

    if (sack->slaying) /* it's locked or personalized*/
    {
        if (sack->sub_type1 == ST1_CONTAINER_NORMAL)
        {
            tmp = find_key(op, sack);
            if (tmp)
                new_draw_info_format(NDI_UNIQUE, 0, op, "You unlock %s with %s.", query_name(sack), query_name(tmp));
            else
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "You don't have the key to unlock %s.", query_name(sack));
                return 0;
            }
        }
        else
        {
            /* only give player with right name access */
            if (sack->sub_type1 == ST1_CONTAINER_CORPSE_group &&
                    (!(CONTR(op)->group_status & GROUP_STATUS_GROUP) ||
                        CONTR(CONTR(op)->group_leader)->group_id != sack->stats.maxhp))
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "Its not your groups bounty.");
                return 0;
            }
            else if (sack->sub_type1 == ST1_CONTAINER_CORPSE_player && sack->slaying != op->name)
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "Its not your bounty.");
                return 0;
            }
        }
    }

    /* By the time we get here, we have made sure any other container has been closed and
     * if this is a locked container, the player they key to open it.
     */

    /* There are really two cases - the sack is either on the ground, or the sack is
     * part of the players inventory.  If on the ground, we assume that the player is
     * opening it, since if it was being closed, that would have been taken care of above.
     * If it in the players inventory, we can READY the container.
     */
    if (sack->env != op) /* container is NOT in players inventory */
    {
        /* this is not possible - opening a container inside another container or a another player */
        if (sack->env)
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "You can't open %s", query_name(sack));
            return 0;
        }

        new_draw_info_format(NDI_UNIQUE, 0, op, "You open %s.", query_name(sack));
        container_link(CONTR(op), sack);
    }
    else/* sack is in players inventory */
    {
        if (QUERY_FLAG(sack, FLAG_APPLIED)) /* readied sack becoming open */
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "You open %s.", query_name(sack));
            container_link(CONTR(op), sack);
        }
        else
        {
            /* We don't allow multiple applied containers of the same type (race) */
            /* No need for recursive search, since only top-level containers may be applied */
            for(tmp = op->inv; tmp; tmp = tmp->below)
            {
                if(QUERY_FLAG(tmp, FLAG_APPLIED) && tmp->type == CONTAINER &&
                        tmp->race == sack->race && tmp != sack)
                {
                    CLEAR_FLAG(tmp, FLAG_APPLIED);
                    new_draw_info_format(NDI_UNIQUE, 0, op, "You unreadied %s.", query_name(tmp));
                    update_object(tmp, UP_OBJ_FACE);
                    esrv_update_item(UPD_FLAGS, op, tmp);
                }
            }

            new_draw_info_format(NDI_UNIQUE, 0, op, "You readied %s.", query_name(sack));
            SET_FLAG(sack, FLAG_APPLIED);
            update_object(sack, UP_OBJ_FACE);
            esrv_update_item(UPD_FLAGS, op, sack);
            container_trap(op, sack);   /* search & explode a rune in the container */
        }
    }
    return 1;
}

/*
 * Returns true if sacrifice was accepted.
 */
static int apply_altar(object *altar, object *sacrifice, object *originator)
{
    /* Only players can make sacrifices on spell casting altars. */
    if (altar->stats.sp != -1 && (!originator || originator->type != PLAYER))
        return 0;
    if (operate_altar(altar, &sacrifice))
    {
        /* Simple check.
         * with an altar.  We call it a Potion - altars are stationary - it
         * is up to map designers to use them properly.
        * Change: I changed .sp from 0 = no spell to -1. So we can cast first
        * spell too... No idea why this was not done in crossfire. ;T-2003
         */
        if (altar->stats.sp != -1)
        {
            new_draw_info_format(NDI_WHITE, 0, originator, "The altar casts %s.", spells[altar->stats.sp].name);
            cast_spell(originator, altar, altar->last_sp, altar->stats.sp, 0, spellPotion, NULL);
            /* If it is connected, push the button.  Fixes some problems with
             * old maps.
             */
            push_button(altar, sacrifice, originator);
        }
        else
        {
            altar->weight_limit = 1;  /* works only once */
            push_button(altar, sacrifice, originator);
        }
        return sacrifice == NULL;
    }
    else
    {
        return 0;
    }
}



/*
 * Returns 1 if 'op' was destroyed, 0 if not.
 * Largely re-written to not use nearly as many gotos, plus
 * some of this code just looked plain out of date.
 * MSW 2001-08-29
 */
static int apply_shop_mat(object *shop_mat, object *op)
{
    int     rv  = 0;
    object *tmp;

    /* Event trigger and quick exit */
    if(trigger_object_plugin_event(EVENT_TRIGGER,
                shop_mat, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
        return FALSE;

    SET_FLAG(op, FLAG_NO_APPLY);   /* prevent loops */

    if (op->type != PLAYER)
    {
        if (QUERY_FLAG(op, FLAG_UNPAID))
        {
            /* Somebody dropped an unpaid item, just move to an adjacent place. */
            int i   = find_free_spot(op->arch, op->map, op->x, op->y, 1, 9);
            if (i != -1)
            {
                rv = enter_map(op, shop_mat, op->map,op->x + freearr_x[i], op->y + freearr_y[i], MAP_STATUS_FIXED_POS);
            }
        }
        /* Removed code that checked for multipart objects - it appears that
         * the teleport function should be able to handle this just fine.
         */

        rv = teleport(shop_mat, SHOP_MAT, op);
    }
    /* immediate block below is only used for players */
    else if (get_payment(op))
    {
        rv = teleport(shop_mat, SHOP_MAT, op);
        if (shop_mat->msg)
        {
            new_draw_info(NDI_UNIQUE, 0, op, shop_mat->msg);
        }
        /* This check below is a bit simplistic - generally it should be correct,
            * but there is never a guarantee that the bottom space on the map is
            * actually the shop floor.
            */
        else if (!rv && (tmp = get_map_ob(op->map, op->x, op->y)) != NULL && tmp->type != SHOP_FLOOR)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "Thank you for visiting our shop.");
        }
    }
    else
    {
        /* if we get here, a player tried to leave a shop but was not able
            * to afford the items he has.  We try to move the player so that
            * they are not on the mat anymore
            */

        int i   = find_free_spot(op->arch, op->map, op->x, op->y, 1, 9);
        if (i == -1)
        {
            LOG(llevBug, "BUG: Internal shop-mat problem (map:%s object:%s pos: %d,%d).\n", op->map->name, op->name,
                op->x, op->y);
        }
        else
        {
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_DEFAULT);
            op->x += freearr_x[i];
            op->y += freearr_y[i];
            rv = (insert_ob_in_map(op, op->map, shop_mat, 0) == NULL);
        }
    }

    CLEAR_FLAG(op, FLAG_NO_APPLY);
    return rv;
}

static void apply_sign(object *op, object *sign)
{
    if (sign->stats.food)
    {
        if (sign->last_eat >= sign->stats.food)
        {
            if (!QUERY_FLAG(sign, FLAG_WALK_ON) && !QUERY_FLAG(sign, FLAG_FLY_ON))
                new_draw_info(NDI_UNIQUE, 0, op, "You cannot read it anymore.");
            return;
        }
        sign->last_eat++;
    }

    /* Sign or magic mouth?  Do we need to see it, or does it talk to us?
     * No way to know for sure.
     *
     * This check fails for signs with FLAG_WALK_ON/FLAG_FLY_ON.  Checking
     * for FLAG_INVISIBLE instead of FLAG_WALK_ON/FLAG_FLY_ON would fail
     * for magic mouths that have been made visible.
     */
    if(!QUERY_FLAG(op, FLAG_WIZ) && !QUERY_FLAG(sign, FLAG_WALK_ON) && !QUERY_FLAG(sign,FLAG_FLY_ON))
    {
        if (QUERY_FLAG(op, FLAG_BLIND))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
            return;
        }

        /* you has the right skill & language knowledge to read it? */
        else if (sign->msg)
        {
            if (!change_skill(op, SK_LITERACY))
            {
                new_draw_info(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
                return;
            }
            else if((op->chosen_skill->weight_limit & sign->weight_limit) != sign->weight_limit)
            {
                new_draw_info_format(NDI_UNIQUE, 0, op, "You are unable to decipher the %s.\nIt is written in %s.",
                                     query_name(sign), get_language(sign->weight_limit));
                return;
            }
        }
    }

    /* Signs should trigger APPLY events, and magic mouths should trigger TRIGGER events,
     * so we trigger both to be sure... */
    if(trigger_object_plugin_event(
                EVENT_APPLY, sign, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;
    if(trigger_object_plugin_event(
                EVENT_TRIGGER, sign, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    /* sign */
    if (!QUERY_FLAG(sign, FLAG_SYS_OBJECT))
    {
        if (!sign->msg)
            new_draw_info_format(NDI_UNIQUE, 0, op, "Nothing is written on the %s.",
                                 query_name(sign));
        else
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "The %s is written in %s.\nYou start reading it.",
                                 query_name(sign), get_language(sign->weight_limit));
            new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, sign->msg);
        }
    }
    /* magic mouth */
    else if (sign->msg && (sign->direction == 0 || sign->direction == op->direction))
        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, sign->msg);
}


/* 'victim' moves onto 'trap' (trap has FLAG_WALK_ON or FLAG_FLY_ON set) or
 * 'victim' leaves 'trap' (trap has FLAG_WALK_OFF or FLAG_FLY_OFF) set.
 *
 * originator: Player, monster or other object that caused 'victim' to move
 * onto 'trap'.  Will receive messages caused by this action.  May be NULL.
 * However, some types of traps require an originator to function.
 *
 * I added the flags parameter to give the single events more information
 * about whats going on:
 * Most important is the "MOVE_APPLY_VANISHED" flag.
 * If set, a object has left a tile but "vanished" and not moved (perhaps
 * it exploded or whatever). This means that some events are not triggered
 * like trapdoors or teleporter traps for example which have a "FLY/MOVE_OFF"
 * set. This will avoid that they touch invalid objects.
 */
void move_apply(object *const trap_obj, object *const victim, object *const originator, const int flags)
{
    object *const trap = trap_obj->head ? trap_obj->head: trap_obj;
    static int  recursion_depth = 0;

    /* move_apply() is the most likely candidate for causing unwanted and
     * possibly unlimited recursion. */
    /* The following was changed because it was causing perfeclty correct
       maps to fail.  1)  it's not an error to recurse:
       rune detonates, summoning monster.  monster lands on nearby rune.
       nearby rune detonates.  This sort of recursion is expected and
       proper.  This code was causing needless crashes. */
    if (recursion_depth >= 500)
    {
        LOG(llevDebug,
            "WARNING: move_apply(): aborting recursion "
            "[trap arch %s, name %s; victim arch %s, name %s]\n",
            trap->arch->name, trap->name, victim->arch->name, victim->name);
        return;
    }
    recursion_depth++;

    switch (trap->type)
    {
          /* these objects can trigger other objects connected to them.
           * We need to check them at map loading time and other special
           * events to be sure to have a 100% working map state.
           */
        case BUTTON:
        case PEDESTAL:
          update_button(trap, victim, originator);
          goto leave;

        case TRIGGER_BUTTON:
        case TRIGGER_PEDESTAL:
        case TRIGGER_ALTAR:
          check_trigger(trap, victim, originator);
          goto leave;

        case CHECK_INV:
          check_inv(victim, trap);
          goto leave;

          /* these objects trigger to but they are "instant".
           * We don't need to check them when loading.
           */
        case ALTAR:
          /* sacrifice victim on trap */
          apply_altar(trap, victim, originator);
          goto leave;

        case CONVERTER:
          if (!(flags & MOVE_APPLY_VANISHED))
              convert_item(victim, trap, originator);
          goto leave;

        case PLAYERMOVER:
          /*
           if (trap->attacktype && (trap->level || victim->type!=PLAYER)) {
          if (!trap->stats.maxsp) trap->stats.maxsp=2;
          victim->speed_left = -FABS(trap->stats.maxsp*victim->speed/trap->speed);
          if (victim->speed_left<-50.0) victim->speed_left=-50.0;
           }
          */
          goto leave;

        case SPINNER:
          /* should be walk_on/fly_on only */
          if (victim->direction)
          {
              if(trigger_object_plugin_event(EVENT_TRIGGER,
                          trap, victim, originator,
                          NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                  goto leave;

              if ((victim->direction = victim->direction + trap->direction) > 8)
                  victim->direction = (victim->direction % 8) + 1;
              update_turn_face(victim);
          }
          goto leave;

        case DIRECTOR:
          if (victim->direction)
          {
              if(trigger_object_plugin_event(EVENT_TRIGGER,
                          trap, victim, originator,
                          NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                  goto leave;

              if (QUERY_FLAG(victim, FLAG_IS_MISSILE))
              {
                  SET_FLAG(victim, FLAG_WAS_REFLECTED);
                  if (!missile_reflection_adjust(victim, 0))
                      goto leave;
              }

              victim->direction = trap->direction;
              update_turn_face(victim);
          }
          goto leave;


        /* The following missile types do not cause TRIGGER events */
        case MMISSILE:
          /* no need to hit anything */
          if (IS_LIVE(victim) && !(flags & MOVE_APPLY_VANISHED))
          {
              tag_t     trap_tag    = trap->count;
              damage_ob(victim, trap->stats.dam, trap, ENV_ATTACK_CHECK);
              if (!was_destroyed(trap, trap_tag))
                  remove_ob(trap);
              check_walk_off(trap, NULL, MOVE_APPLY_VANISHED);
          }
          goto leave;

        case THROWN_OBJ:
          if (trap->inv == NULL || (flags & MOVE_APPLY_VANISHED))
              goto leave;
          /* fallthrough */
        case ARROW:
          /* bad bug: monster throw a object, make a step forwards, step on object ,
           * trigger this here and get hit by own missile - and will be own enemy.
           * Victim then is his own enemy and will start to kill herself (this is
           * removed) but we have not synced victim and his missile. To avoid senseless
           * action, we avoid hits here */
          if ((IS_LIVE(victim) && trap->speed) && trap->owner != victim)
              hit_with_arrow(trap, victim);
          goto leave;

        case CANCELLATION:
        case BALL_LIGHTNING:
          if (IS_LIVE(victim) && !(flags & MOVE_APPLY_VANISHED))
              damage_ob(victim, trap->stats.dam, trap, ENV_ATTACK_CHECK);
          goto leave;

        case CONE:
        case LIGHTNING: /* bolt */
            /*
            if ((IS_LIVE(victim) && trap->speed) && trap->owner != victim)
                damage_ob(victim, trap->stats.dam, trap, ENV_ATTACK_CHECK);
            goto leave;
            */
          /*
           if(IS_LIVE(victim)&&trap->speed) {
             uint32 attacktype = trap->attacktype & ~AT_COUNTERSPELL;
             if (attacktype)
               damage_ob(victim,trap->stats.dam, ENV_ATTACK_CHECK);
           }
          */
          //goto leave;

        case FBULLET:
        case BULLET:
          if ((QUERY_FLAG(victim, FLAG_NO_PASS) || IS_LIVE(victim)) && !(flags & MOVE_APPLY_VANISHED))
              check_fired_arch(trap);
          goto leave;

          /* FIXME: this doesn't look correct.
           * This function will be called once for every object on the square (I think),
           * but the code below also goes through the first 100 objects on the square)
           * Gecko 2006-11-14. */
        case TRAPDOOR:
          {
              int       max, sound_was_played;
              object   *ab;

              if ((flags & MOVE_APPLY_VANISHED))
                  goto leave;

              if (!trap->weight_limit)
              {
                  sint32    tot;
                  for (ab = trap->above,tot = 0; ab != NULL; ab = ab->above)
                      if (!IS_AIRBORNE(ab))
                          tot += WEIGHT(ab);
                  if (!(trap->weight_limit = (tot > trap->weight) ? 1 : 0))
                      goto leave;
                  SET_ANIMATION(trap, (NUM_ANIMATIONS(trap) / NUM_FACINGS(trap)) * trap->direction + trap->weight_limit);
                  update_object(trap, UP_OBJ_FACE);
              }
              for (ab = trap->above, max = 100, sound_was_played = 0;
                   --max && ab && !IS_AIRBORNE(ab);
                   ab = ab->above)
              {
                  if (!sound_was_played)
                  {
                      play_sound_map(trap->map, trap->x, trap->y, SOUND_FALL_HOLE, SOUND_NORMAL);
                      sound_was_played = 1;
                  }
                  if (ab->type == PLAYER)
                      new_draw_info(NDI_UNIQUE, 0, ab, "You fall into a trapdoor!");
                  enter_map_by_exit(ab, trap);
              }
              goto leave;
          }


        case PIT:
          /* Pit not open? */
          if ((flags & MOVE_APPLY_VANISHED) || trap->stats.wc > 0)
              goto leave;
          play_sound_map(victim->map, victim->x, victim->y, SOUND_FALL_HOLE, SOUND_NORMAL);

          if(enter_map_by_exit(victim, trap))
              if (victim->type == PLAYER)
                  new_draw_info(NDI_UNIQUE, 0, victim, "You fall through the hole!\n");

          goto leave;

        case EXIT:
          if (!(flags & MOVE_APPLY_VANISHED) && victim->type == PLAYER)
              enter_map_by_exit(victim, trap);
          goto leave;

        case SHOP_MAT:
          if (!(flags & MOVE_APPLY_VANISHED))
              apply_shop_mat(trap, victim);
          goto leave;

          /* Drop a certain amount of gold, and have one item identified */
        case IDENTIFY_ALTAR:
          if (!(flags & MOVE_APPLY_VANISHED))
              apply_id_altar(victim, trap, originator);
          goto leave;

        case SIGN:
          if (victim->type == PLAYER) /* only player should be able read signs */
              apply_sign(victim, trap);
          goto leave;

          /* FIXME: Huh? can containers be WALK_ON??? Gecko 2006-11-14 */
        case CONTAINER:
          if (victim->type == PLAYER)
          {
              if(!trigger_object_plugin_event(EVENT_TRIGGER, trap, victim, NULL,
                          NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                  esrv_apply_container(victim, trap);
          }
          goto leave;

        case RUNE:
          if (!(flags & MOVE_APPLY_VANISHED) && trap->level && IS_LIVE(victim))
              spring_trap(trap, victim);
          goto leave;

          /* we don't have this atm.
          case DEEP_SWAMP:
            if(!(flags&MOVE_APPLY_VANISHED))
              walk_on_deep_swamp (trap, victim);
            goto leave;
          */
        default:
          LOG(llevDebug, "name %s, arch %s, type %d with fly/walk on/off not "
                         "handled in move_apply()\n", trap->name,
              trap->arch->name, trap->type);
          goto leave;
    }

    leave : recursion_depth--;
}


static void apply_book(object *op, object *tmp)
{
    sockbuf_struct *sptr;
    char    buf[HUGE_BUF];
    size_t  len;

    if (QUERY_FLAG(op, FLAG_BLIND) && !QUERY_FLAG(op, FLAG_WIZ))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
        return;
    }

    /* you has the right skill & language knowledge to read it? */
    if (!QUERY_FLAG(op, FLAG_WIZ))
    {
        if (!change_skill(op, SK_LITERACY))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
            return;
        }
        else if((op->chosen_skill->weight_limit & tmp->weight_limit)!=tmp->weight_limit)
        {
            new_draw_info_format(NDI_UNIQUE, 0, op, "You are unable to decipher the %s.\nIts written in %s.",
                                 query_name(tmp), get_language(tmp->weight_limit));
            return;
        }
    }

    new_draw_info_format(NDI_UNIQUE, 0, op, "You open the %s and start reading.", query_name(tmp));

    /* Non-zero return value from script means stop here */
    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        return;

    if (tmp->msg == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "You open the %s and find it empty.", query_name(tmp));
        return;
    }

    /* invoke the new client sided book interface */
    sprintf(buf,"<b t=\"%s%s%s\">", tmp->name?tmp->name:"Book",tmp->title?" ":"",tmp->title?tmp->title:"");
    strcat(buf, tmp->msg);
    len = strlen(buf);

    SOCKBUF_REQUEST_BUFFER(&CONTR(op)->socket, (len > SOCKET_SIZE_MEDIUM) ? SOCKET_SIZE_HUGE : SOCKET_SIZE_MEDIUM);
    sptr = ACTIVE_SOCKBUF(&CONTR(op)->socket);

    SockBuf_AddInt(sptr, tmp->weight_limit);
    SockBuf_AddString(sptr, buf, strlen(buf));

    SOCKBUF_REQUEST_FINISH(&CONTR(op)->socket, BINARY_CMD_BOOK, SOCKBUF_DYNAMIC);
    /*new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, tmp->msg);*/

    /* identify the book - successful reading will do it always */
    if (!QUERY_FLAG(tmp, FLAG_NO_SKILL_IDENT))
    {
        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            SET_FLAG(tmp, FLAG_IDENTIFIED);
            /* If in a container, update how it looks */
            if (tmp->env)
                esrv_update_item(UPD_FLAGS | UPD_NAME, op, tmp);
            else
                CONTR(op)->socket.update_tile = 0;
        }
        /*add_exp(op,exp_gain,op->chosen_skill->stats.sp);*/
        SET_FLAG(tmp, FLAG_NO_SKILL_IDENT); /* so no more xp gained from this book */
    }
}


static void apply_skillscroll(object *op, object *tmp)
{
    if(trigger_object_plugin_event( EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;
    switch ((int) learn_skill(op, tmp, NULL, -1, 1))
    {
        case 0:
          new_draw_info(NDI_UNIQUE, 0, op, "You already possess the knowledge ");
          new_draw_info_format(NDI_UNIQUE, 0, op, "held within the %s.\n", query_name(tmp));
          return;

        case 1:
          new_draw_info_format(NDI_UNIQUE, 0, op, "You succeed in learning %s", skills[tmp->stats.sp].name);
          new_draw_info_format(NDI_UNIQUE, 0, op, "Type 'bind ready_skill %s", skills[tmp->stats.sp].name);
          new_draw_info(NDI_UNIQUE, 0, op, "to store the skill in a key.");
          FIX_PLAYER(op ,"apply skill scroll");
          decrease_ob(tmp);
          return;

        default:
          new_draw_info_format(NDI_UNIQUE, 0, op, "You fail to learn the knowledge of the %s.\n", query_name(tmp));
          decrease_ob(tmp);
          return;
    }
}


/* Special prayers are granted by gods and lost when the follower decides
 * to pray to a different gods.  'Force' objects keep track of which
 * prayers are special.
 */

static object * find_special_prayer_mark(object *op, int spell)
{
    object *tmp;

    for (tmp = op->inv; tmp; tmp = tmp->below)
        if (tmp->type == FORCE && tmp->slaying && tmp->slaying == shstr_cons.special_prayer && tmp->stats.sp == spell)
            return tmp;
    return 0;
}

static void insert_special_prayer_mark(object *op, int spell)
{
    object *force   = get_archetype("force");
    force->speed = 0;
    update_ob_speed(force);
    FREE_AND_COPY_HASH(force->slaying, "special prayer");
    force->stats.sp = spell;
    insert_ob_in_ob(force, op);
}

extern void do_learn_spell(object *op, int spell, int special_prayer)
{
    object *tmp = find_special_prayer_mark(op, spell);

    if (op->type != PLAYER)
    {
        LOG(llevBug, "BUG: do_learn_spell(): not a player ->%s\n", op->name);
        return;
    }

    /* Upgrade special prayers to normal prayers */
    if (check_spell_known(op, spell))
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "You already know the spell '%s'!", spells[spell].name);

        if (special_prayer || !tmp)
        {
            LOG(llevBug, "BUG: do_learn_spell(): spell already known, but can't upgrade it\n");
            return;
        }
        remove_ob(tmp);
        return;
    }

    /* Learn new spell/prayer */
    if (tmp)
    {
        LOG(llevBug, "BUG: do_learn_spell(): spell unknown, but special prayer mark present\n");
        remove_ob(tmp);
    }
    play_sound_player_only(CONTR(op), SOUND_LEARN_SPELL, SOUND_NORMAL, 0, 0);
    CONTR(op)->known_spells[CONTR(op)->nrofknownspells++] = spell;

    /* For godgiven spells the player gets a reminder-mark inserted,
       that this spell must be removed on changing cults! */
    if (special_prayer)
        insert_special_prayer_mark(op, spell);

    send_spelllist_cmd(op, spells[spell].name, SPLIST_MODE_ADD);
    new_draw_info_format(NDI_UNIQUE, 0, op, "You have learned the spell %s!", spells[spell].name);
}

extern void do_forget_spell(object *op, int spell)
{
    object *tmp;
    int     i;

    if (op->type != PLAYER)
    {
        LOG(llevBug, "BUG: do_forget_spell(): not a player: %s (%d)\n", query_name(op), spell);
        return;
    }
    if (!check_spell_known(op, spell))
    {
        LOG(llevBug, "BUG: do_forget_spell(): spell %d not known\n", spell);
        return;
    }

    play_sound_player_only(CONTR(op), SOUND_LOSE_SOME, SOUND_NORMAL, 0, 0);
    new_draw_info_format(NDI_UNIQUE, 0, op, "You lose knowledge of %s.", spells[spell].name);

    send_spelllist_cmd(op, spells[spell].name, SPLIST_MODE_REMOVE);
    tmp = find_special_prayer_mark(op, spell);
    if (tmp)
        remove_ob(tmp);

    for (i = 0; i < CONTR(op)->nrofknownspells; i++)
    {
        if (CONTR(op)->known_spells[i] == spell)
        {
            CONTR(op)->known_spells[i] = CONTR(op)->known_spells[--CONTR(op)->nrofknownspells];
            return;
        }
    }
    LOG(llevBug, "BUG: do_forget_spell(): couldn't find spell %d\n", spell);
}

static void apply_spellbook(object *op, object *tmp)
{
    if (QUERY_FLAG(op, FLAG_BLIND) && !QUERY_FLAG(op, FLAG_WIZ))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
        return;
    }

    /* artifact_spellbooks have 'slaying' field point to a spell name,
    ** instead of having their spell stored in stats.sp.  We should update
    ** stats->sp to point to that spell */

    if (tmp->slaying != NULL)
    {
        if ((tmp->stats.sp = look_up_spell_name(tmp->slaying)) < 0)
        {
            tmp->stats.sp = -1;
            new_draw_info_format(NDI_UNIQUE, 0, op, "The book's formula for %s is incomplete", tmp->slaying);
            return;
        }
        /* now clear tmp->slaying since we no longer need it */
        FREE_AND_CLEAR_HASH2(tmp->slaying);
    }

    /* need a literacy skill to learn spells. Also, having a literacy level
     * lower than the spell will make learning the spell more difficult */
    if (!change_skill(op, SK_LITERACY))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You can't read! Your attempt fails.");
        return;
    }
    if (tmp->stats.sp <0 || tmp->stats.sp> NROFREALSPELLS || spells[tmp->stats.sp].level > (SK_level(op) + 10))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
        return;
    }

    if(trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    new_draw_info_format(NDI_UNIQUE, 0, op, "The spellbook contains the %s level spell %s.",
                         get_levelnumber(spells[tmp->stats.sp].level), spells[tmp->stats.sp].name);

    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
    {
        identify(tmp);
        if (tmp->env)
            esrv_update_item(UPD_FLAGS | UPD_NAME, op, tmp);
        else
            CONTR(op)->socket.update_tile = 0;
    }

    if (check_spell_known(op, tmp->stats.sp) && (tmp->stats.Wis || find_special_prayer_mark(op, tmp->stats.sp) == NULL))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You already know that spell.\n");
        return;
    }

    /* I changed spell learning in 3 ways:
     *
     *  1- MU spells use Int to learn, Cleric spells use Wisdom
     *
     *  2- The learner's level (in skills sytem level==literacy level; if no
     *     skills level == overall level) impacts the chances of spell learning.
     *
     *  3 -Automatically fail to learn if you read while confused
     *
     * Overall, chances are the same but a player will find having a high
     * literacy rate very useful!  -b.t.
     */
    if (QUERY_FLAG(op, FLAG_CONFUSED))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "In your confused state you flub the wording of the text!");
        /* this needs to be a - number [garbled] */
        scroll_failure(op, 0 - random_roll(0, spells[tmp->stats.sp].level), spells[tmp->stats.sp].sp);
    }
    else if (QUERY_FLAG(tmp, FLAG_STARTEQUIP))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You succeed in learning the spell!");
        do_learn_spell(op, tmp->stats.sp, 0);

        /* xp gain to literacy for spell learning */
        if (!QUERY_FLAG(tmp, FLAG_STARTEQUIP))
            add_exp(op, calc_skill_exp(op, tmp, 1.0f,-1, NULL), op->chosen_skill->stats.sp);
    }
    else
    {
        play_sound_player_only(CONTR(op), SOUND_FUMBLE_SPELL, SOUND_NORMAL, 0, 0);
        new_draw_info(NDI_UNIQUE, 0, op, "You fail to learn the spell.\n");
    }
    decrease_ob(tmp);
}


static void apply_scroll(object *op, object *tmp)
{
    /*object *old_skill;*/
    int scroll_spell = tmp->stats.  sp;

    if (QUERY_FLAG(op, FLAG_BLIND) && !QUERY_FLAG(op, FLAG_WIZ))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
        return;
    }

    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        identify(tmp);

    if (scroll_spell < 0 || scroll_spell >= NROFREALSPELLS)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "The scroll just doesn't make sense!");
        return;
    }

    if (op->type == PLAYER)
    {
        /*old_skill = op->chosen_skill;*/
        /* players need a literacy skill to read stuff! */
        if (!change_skill(op, SK_LITERACY))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
            /* op->chosen_skill=old_skill;*/
            return;
        }

        /* thats new: literacy for reading but a player need also the
             * right spellcasting spell. Reason: the exp goes then in that
             * skill. This makes scroll different from wands or potions.
             */
        if (!change_skill(op, (spells[scroll_spell].type == SPELL_TYPE_PRIEST ? SK_PRAYING : SK_SPELL_CASTING)))
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You can read the scroll but you don't understand it.");
            /* op->chosen_skill=old_skill;*/
            return;
        }

    }

    if(trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    new_draw_info_format(NDI_WHITE, 0, op, "The scroll of %s turns to dust.", spells[tmp->stats.sp].name);
    /*    {
          char buf[MAX_BUF];

          sprintf(buf, "%s reads a scroll of %s.",op->name,spells[tmp->stats.sp].name);
          new_info_map(NDI_ORANGE, op->map, buf);
        }
        */

    cast_spell(op, tmp, op->facing ? op->facing : 4, scroll_spell, 0, spellScroll, NULL);
    decrease_ob(tmp);
}

/* op opens treasure chest tmp */
static void apply_treasure(object *op, object *tmp)
{
    object                 *treas;
    tag_t tmp_tag = tmp->   count, op_tag = op->count;

    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;
    /*  Nice side effect of new treasure creation method is that the treasure
        for the chest is done when the chest is created, and put into the chest
        inventory.  So that when the chest burns up, the items still exist.  Also
        prevents people fromt moving chests to more difficult maps to get better
        treasure
    */
    treas = tmp->inv;

    if (tmp->map)
        play_sound_map(tmp->map, tmp->x, tmp->y, SOUND_OPEN_CONTAINER, SOUND_NORMAL);

    if (tmp->msg) /* msg like "the chest crumbles to dust" */
        new_draw_info(NDI_UNIQUE, 0, op, tmp->msg);
    if (treas == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "The chest was empty.");
        decrease_ob(tmp);
        return;
    }
    do
    {
        remove_ob(treas);
        check_walk_off(treas, NULL, MOVE_APPLY_VANISHED);
        draw_find(op, treas);
        treas->x = op->x,treas->y = op->y;
        if (treas->type == MONSTER)
        {
            /* Monsters can be trapped in treasure chests */
            int i   = find_free_spot(treas->arch, op->map, treas->x, treas->y, 0, 9);
            if (i != -1)
            {
                treas->x += freearr_x[i];
                treas->y += freearr_y[i];
            }
            fix_monster(treas);
        }
        treas = insert_ob_in_map(treas, op->map, op, 0);
        if (treas && treas->type == RUNE && treas->level && IS_LIVE(op))
            spring_trap(treas, op);

        if (was_destroyed(op, op_tag) || was_destroyed(tmp, tmp_tag))
            break;
    }
    while ((treas = tmp->inv) != NULL);

    if (!was_destroyed(tmp, tmp_tag) && tmp->inv == NULL)
        decrease_ob(tmp);

#if 0
    /* Can't rely on insert_ob_in_map to do any restacking,
     * so lets disable this.
     */
    if ( ! QUERY_FLAG(op, FLAG_REMOVED)) {
      /* Done to re-stack map with player on top? */
      SET_FLAG (op, FLAG_NO_APPLY);
      remove_ob (op);
      check_walk_off (op, NULL, MOVE_APPLY_DEFAULT);
      insert_ob_in_map (op, op->map, NULL,0);
      CLEAR_FLAG (op, FLAG_NO_APPLY);
    }
#endif
}


void apply_poison(object *op, object *tmp)
{
    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    if (op->type == PLAYER)
    {
        play_sound_player_only(CONTR(op), SOUND_DRINK_POISON, SOUND_NORMAL, 0, 0);
        new_draw_info(NDI_UNIQUE, 0, op, "Yech!  That tasted poisonous!");
        FREE_AND_ADD_REF_HASH(CONTR(op)->killer, shstr_cons.poisonous_food);
    }
    if (tmp->stats.dam)
    {
        LOG(llevDebug, "Trying to poison player/monster for %d hp\n", tmp->stats.hp);
        /* internal damage part will take care about our poison */
        damage_ob(op, tmp->stats.dam, tmp, ENV_ATTACK_CHECK);
    }
    op->stats.food -= op->stats.food / 4;
    decrease_ob(tmp);
}

static void apply_savebed(object *pl, object *bed)
{
    player *p_ptr =CONTR(pl);

    if (!p_ptr || !p_ptr->name_changed || !pl->stats.exp)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "You don't deserve to save your character yet.");
        return;
    }

    if(trigger_object_plugin_event(
                EVENT_APPLY, bed, pl, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    /* update respawn position */
    set_bindpath_by_name(p_ptr, pl->map->path, pl->map->orig_path, pl->map->map_status, pl->x, pl->y);

    new_draw_info(NDI_UNIQUE, 0, pl, "You save and quit the game. Bye!\nleaving...");
    p_ptr->socket.status = Ns_Dead;
}


static void apply_armour_improver(object *op, object *tmp)
{
    object *armor;

    if (blocks_magic(op->map, op->x, op->y))
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Something blocks the magic of the scroll.");
        return;
    }
    armor = find_marked_object(op);
    if (!armor)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You need to mark an armor object.");
        return;
    }
    if (armor->type != ARMOUR
     && armor->type != CLOAK
     && armor->type != BOOTS
     && armor->type != GLOVES
     && armor->type != BRACERS
     && armor->type != SHIELD
     && armor->type != SHOULDER
     && armor->type != LEGS
     && armor->type != HELMET)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your marked item is not armour!\n");
        return;
    }

    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    new_draw_info(NDI_UNIQUE, 0, op, "Applying armour enchantment.");
    improve_armour(op, tmp, armor);
}



/* Return value (or bits):
 *   0: player or monster can't apply objects of that type
 *   1: has been applied, or there was an error applying the object
 *   2: objects of that type can't be applied if not in inventory
 *   4: no fix_player() needed.
 *   8: apply action will NOT break sitting/resting (like eating food)
 *
 * op is the object that is causing object to be applied, tmp is the object
 * being applied.
 *
 * aflag is special (always apply/unapply) flags.  Nothing is done with
 * them in this function - they are passed to apply_special
 */

int manual_apply(object *op, object *tmp, int aflag)
{
    int ego_mode;

    if (tmp->head)
        tmp = tmp->head;

    if (QUERY_FLAG(tmp, FLAG_UNPAID) && !QUERY_FLAG(tmp, FLAG_APPLIED))
    {
        if (op->type == PLAYER)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You should pay for it first.");
            return 4;
        }
        else
        {
            return 0;   /* monsters just skip unpaid items */
        }
    }

    /* monsters mustn't apply random chests, nor magic_mouths with a counter */
    if (op->type != PLAYER && tmp->type == TREASURE)
        return 0;

    /* lets check we have an ego item */
    if((ego_mode = check_ego_item(op, tmp)))
    {
        if(op->type == PLAYER)
        {
            if(ego_mode == EGO_ITEM_BOUND_UNBOUND)
                new_draw_info (NDI_UNIQUE, 0, op, "This is an ego item!\nType \"/egobind\" for more info about applying it!");
            else if(ego_mode == EGO_ITEM_BOUND_PLAYER)

                new_draw_info (NDI_UNIQUE, 0, op, "This is not your ego item!");
        }
        return 1;
    }

    /* control apply by controling a set exp object level or player exp level*/
    if(tmp->item_level && op->type == PLAYER)
    {
        int tmp_lev;

        if (tmp->item_skill)
            tmp_lev = CONTR(op)->exp_obj_ptr[tmp->item_skill-1]->level; /* use player struct shortcut ptrs */
        else
            tmp_lev = op->level;

        if (tmp->item_level > tmp_lev)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "The item level is to high to apply.");
            return 4;
        }
    }

    /* Since we want to defer the event triggers until all tests have been
     * passed, but not until after any side effects, we must handle each
     * object type differently (yuck!) when it comes to apply events. */

    switch (tmp->type)
    {
        case HOLY_ALTAR:
            new_draw_info_format(NDI_UNIQUE, 0, op, "You touch the %s.", tmp->name);
            if (change_skill(op, SK_PRAYING))
            {
                if(trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL,
                            NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                    return 1; /* 1 = do not write an error message to the player */
                pray_at_altar(op, tmp);
            }
            else
                new_draw_info(NDI_UNIQUE, 0, op, "Nothing happens. It seems you miss the right skill.");
            return 4;

        case CF_HANDLE:
            if(trigger_object_plugin_event(
                        EVENT_APPLY, tmp, op, NULL,
                        NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                return 4; /* 1 = do not write an error message to the player */
            new_draw_info(NDI_UNIQUE, 0, op, "You turn the handle.");
            play_sound_map(op->map, op->x, op->y, SOUND_TURN_HANDLE, SOUND_NORMAL);
            tmp->weight_limit = tmp->weight_limit ? 0 : 1;
            SET_ANIMATION(tmp, ((NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction) + tmp->weight_limit);
            update_object(tmp, UP_OBJ_FACE);
            push_button(tmp, op, op);
            return 4;

        case TRIGGER:
          if(trigger_object_plugin_event(
                      EVENT_APPLY, tmp, op, NULL,
                      NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
              return 1; /* 1 = do not write an error message to the player */
          if (check_trigger(tmp, op, op))
          {
              new_draw_info(NDI_UNIQUE, 0, op, "You turn the handle.");
              play_sound_map(tmp->map, tmp->x, tmp->y, SOUND_TURN_HANDLE, SOUND_NORMAL);
          }
          else
          {
              new_draw_info(NDI_UNIQUE, 0, op, "The handle doesn't move.");
          }
          return 4;

        case EXIT:
          if (op->type != PLAYER)
              return 0;
          if(trigger_object_plugin_event(
                      EVENT_APPLY, tmp, op, NULL,
                      NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
              return 4; /* 1 = do not write an error message to the player */
              enter_map_by_exit(op, tmp);
          return 4;

        case GRAVESTONE:
            if (op->type == PLAYER)
            {
                apply_sign(op, tmp); /* probably should have apply_gravestone() */
                return 4;
            }
            return 0;

        case SIGN:
            if (op->type == PLAYER)
            {
                apply_sign(op, tmp);
                return 4;
            }
            return 0;

        case BOOK:
          if (op->type == PLAYER)
          {
              apply_book(op, tmp);
              return 4;
          }
          return 0;

        case SKILLSCROLL:
          if (op->type == PLAYER)
          {
              apply_skillscroll(op, tmp);
              return 1;
          }
          return 0;

        case SPELLBOOK:
          if (op->type == PLAYER)
          {
              apply_spellbook(op, tmp);
              return 1;
          }
          return 0;

        case SCROLL:
          apply_scroll(op, tmp);
          return 1;

        case POTION:
          if(trigger_object_plugin_event(
                      EVENT_APPLY, tmp, op, NULL,
                      NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
              return 1; /* 1 = do not write an error message to the player */
          (void) apply_potion(op, tmp);
          return (1+8);

        case TYPE_LIGHT_APPLY:
          apply_player_light(op, tmp);
          return 1;

        case TYPE_LIGHT_REFILL:
          apply_player_light_refill(op, tmp);
          return 1;

          /* Eneq(@csd.uu.se): Handle apply on containers. */
        case CLOSE_CON:
          if (op->type == PLAYER)
          {
              if(trigger_object_plugin_event(
                          EVENT_APPLY, tmp, op, NULL,
                          NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                  return 1; /* 1 = do not write an error message to the player */
              (void) esrv_apply_container(op, tmp->env);
          }
          return 4;

        case CONTAINER:
          if (op->type == PLAYER)
          {
              if(trigger_object_plugin_event(
                          EVENT_APPLY, tmp, op, NULL,
                          NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                  return 1; /* 1 = do not write an error message to the player */
              (void) esrv_apply_container(op, tmp);
          }
          return 4;

        case TREASURE:
          apply_treasure(op, tmp);
          return 1;

        case WEAPON:
        case ARMOUR:
        case BOOTS:
        case GLOVES:
        case AMULET:
        case GIRDLE:
        case BRACERS:
        case SHIELD:
        case HELMET:
        case SHOULDER:
        case LEGS:
        case RING:
        case CLOAK:
        case WAND:
        case ROD:
        case HORN:
        case SKILL:
        case BOW:
        case ARROW:
          if (tmp->env != op)
              return 2;   /* not in inventory */
          apply_special(op, tmp, aflag);
          return 1;

        case DRINK:
        case FOOD:
        case FLESH:
          apply_food(op, tmp);
          return (1+8);

        case POISON:
          apply_poison(op, tmp);
          return (1+8);

        case SAVEBED:
          if (op->type == PLAYER)
          {
              apply_savebed(op, tmp);
              return 1;
          }
          return 0;

        case ARMOUR_IMPROVER:
          if (op->type == PLAYER)
          {
              apply_armour_improver(op, tmp);
              return 1;
          }
          return 0;

        case WEAPON_IMPROVER:
          (void) check_improve_weapon(op, tmp);
          return 1;

        case CLOCK:
          if (op->type == PLAYER)
          {
              char          buf[MAX_BUF];
              timeofday_t   tod;

              if(trigger_object_plugin_event(
                          EVENT_APPLY, tmp, op, NULL,
                          NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                  return 4; /* 1 = do not write an error message to the player */

              get_tod(&tod);
              sprintf(buf, "It is %d minute%s past %d o'clock %s", tod.minute + 1, ((tod.minute + 1 < 2) ? "" : "s"),
      ((tod.hour % (HOURS_PER_DAY / 2) == 0) ? (HOURS_PER_DAY / 2) : ((tod.hour) % (HOURS_PER_DAY / 2))),
      ((tod.hour >= (HOURS_PER_DAY / 2)) ? "pm" : "am"));
              new_draw_info(NDI_UNIQUE, 0, op, buf);
              return 4;
          }
          return 0;

        case MENU:
          if (op->type == PLAYER)
          {
              if(trigger_object_plugin_event(
                          EVENT_APPLY, tmp, op, NULL,
                          NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                  return 1; /* 1 = do not write an error message to the player */
              return 4;
          }
          return 0;

        case POWER_CRYSTAL:
          /* TODO: plugin events */
          apply_power_crystal(op, tmp);  /*  see egoitem.c */
          return 1;

        case LIGHTER:
          /* for lighting torches/lanterns/etc */
          /* TODO: plugin events */
          if (op->type == PLAYER)
          {
              apply_lighter(op, tmp);
              return 1;
          }
          return 0;

        default: /* Now we can put scripts even on NON-applyable items */
          if(trigger_object_plugin_event(
                      EVENT_APPLY, tmp, op, NULL,
                      NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
              return 1; /* 1 = do not write an error message to the player */
          return 0;
    }
}

/* quiet suppresses the "don't know how to apply" and "you must get it first"
 * messages as needed by player_apply_below().  But there can still be
 * "but you are floating high above the ground" messages.
 *
 * Same return value as apply() function.
 */
int player_apply(object *pl, object *op, int aflag, int quiet)
{
    int tmp;

        /* player is flying and applying object not in inventory */
    if (op->env == NULL &&
        CONTR(pl)->gmaster_mode != GMASTER_MODE_DM &&
        (IS_AIRBORNE(pl) && (!IS_AIRBORNE(op) || !QUERY_FLAG(op, FLAG_FLY_ON))))
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "But you are floating high above the ground!");
        return 0;
    }

    /* skip not needed fix_player() calls for trivial action */
    SET_FLAG(pl, FLAG_NO_FIX_PLAYER);
    tmp = manual_apply(pl, op, aflag);
    CLEAR_FLAG(pl, FLAG_NO_FIX_PLAYER);

    /* we have applied something which makes us standing up */
    if(CONTR(pl) && CONTR(pl)->rest_sitting && !(tmp & 8))
        CONTR(pl)->rest_sitting = CONTR(pl)->rest_mode = 0;
    if(tmp & 1)
        FIX_PLAYER(pl ,"player apply ");

    if (!quiet)
    {
        if (tmp == 0)
            new_draw_info_format(NDI_UNIQUE, 0, pl, "I don't know how to apply the %s.", query_name(op));
        else if (tmp == 2)
            new_draw_info_format(NDI_UNIQUE, 0, pl, "You must get it first!\n");
    }
    return tmp;
}

/* player_apply_below attempts to apply the object 'below' the player.
 * If the player has an open container, we use that for below, otherwise
 * we use the ground.
 */

void player_apply_below(object *pl)
{
    object *tmp, *next;
    int     floors;

    if (pl->type != PLAYER)
    {
        LOG(llevBug, "BUG: player_apply_below() called for non player object >%s<\n", query_name(pl));
        return;
    }
    /* If using a container, set the starting item to be the top
     * item in the container.  Otherwise, use the map.
     */
    /* i removed this... it can lead in very evil situations like
     * someone hammers on the /apply macro for fast leaving a map but
     * invokes dozens of potions from a open bag.....
     */
    /*tmp = (pl->contr->container != NULL) ? pl->contr->container->inv : pl->below;*/
    tmp = pl->below;

    /* This is perhaps more complicated.  However, I want to make sure that
     * we don't use a corrupt pointer for the next object, so we get the
     * next object in the stack before applying.  This is can only be a
     * problem if player_apply() has a bug in that it uses the object but does
     *  not return a proper value.
     */
    for (floors = 0; tmp != NULL; tmp = next)
    {
        next = tmp->below;
        /* this was is_floor test - but floor has moved
         * in map node. Now, the first for sure not applyable
         * object is the first sys_object
         */
        if (QUERY_FLAG(tmp, FLAG_SYS_OBJECT))
            floors++;
        else if (floors > 0)
            return;   /* process only floor (or sys_objects) objects after first floor object */
        if (!IS_INVISIBLE(tmp, pl) || QUERY_FLAG(tmp, FLAG_WALK_ON) || QUERY_FLAG(tmp, FLAG_FLY_ON))
        {
            if (player_apply(pl, tmp, 0, 1) == 1)
                return;
        }
        if (floors >= 2)
            return;   /* process at most two floor objects */
    }
}


/* who is the object using the object.
 * op is the object they are using.
 * aflags is special flags (0 - normal/toggle, AP_APPLY=always apply,
 * AP_UNAPPLY=always unapply).
 *
 * Optional flags:
 *   AP_NO_MERGE: don't merge an unapplied object with other objects
 *   AP_IGNORE_CURSE: unapply cursed items
 *
 * Usage example:  apply_special (who, op, AP_UNAPPLY | AP_IGNORE_CURSE)
 *
 * apply_special() doesn't check for unpaid items.
 */
int apply_special(object *who, object *op, int aflags)
{
    /* wear/wield */
    player *pl = CONTR(who);
    int     ego_mode, basic_flag  = aflags &AP_BASIC_FLAGS;
    int     tmp_flag    = 0;
    object *tmp;
    char    buf[HUGE_BUF];
    int     i;

    if (who == NULL)
    {
        LOG(llevBug, "BUG: apply_special() from object without environment.\n");
        return 1;
    }

    if (op->env != who)
        return 1;   /* op is not in inventory */

    /* lets check we have an ego item */
    if((ego_mode = check_ego_item(op, who)))
    {
        if(op->type == PLAYER)
        {
            if(ego_mode == EGO_ITEM_BOUND_UNBOUND)
                new_draw_info (NDI_UNIQUE, 0, op, "This is an ego item!\nType \"/egobind\" for more info about applying it!");
            else if(ego_mode == EGO_ITEM_BOUND_PLAYER)

                new_draw_info (NDI_UNIQUE, 0, op, "This is not your ego item!");
        }
        return 1;
    }

    buf[0] = '\0';      /* Needs to be initialized */
    if (QUERY_FLAG(op, FLAG_APPLIED))
    {
        /* always apply, so no reason to unapply */
        if (basic_flag == AP_APPLY)
            return 0;
        if (op->item_condition && !(aflags & AP_IGNORE_CURSE) && (QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED)))
        {
            new_draw_info_format(NDI_UNIQUE, 0, who, "No matter how hard you try, you just can't remove it!");
            return 1;
        }

        if (QUERY_FLAG(op, FLAG_PERM_CURSED))
            SET_FLAG(op, FLAG_CURSED);
        if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
            SET_FLAG(op, FLAG_DAMNED);

        /* This is actually an (UN)APPLY event. Scripters should check
         * the applied flag */
        if(trigger_object_plugin_event(
                    EVENT_APPLY, op, who, NULL,
                    NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
            return 1;

        CLEAR_FLAG(op, FLAG_APPLIED);
        switch (op->type)
        {
            case WEAPON:
                change_abil(who, op);
                CLEAR_FLAG(who, FLAG_READY_WEAPON);
                if(!op->item_condition)
                    sprintf(buf, "Your %s is broken!", query_name(op));
                else
                    sprintf(buf, "You no longer wield the %s.", query_name(op));
                break;

            case SKILL:
                if (who->type == PLAYER)
                {
                    if (!IS_INVISIBLE(op, who))
                    {
                        /* its a tool, need to unlink it */
                        unlink_skill(op);
                        new_draw_info_format(NDI_UNIQUE, 0, who, "You stop using the %s.", query_name(op));
                        new_draw_info_format(NDI_UNIQUE, 0, who, "You can no longer use the skill: %s.",
                                skills[op->stats.sp].name);
                    }
                }
                /* i disabled here change_abil - because skill changing is somewhat often called
                 * AND automatically done. We simply don't give out change_abil() messages here
                 * and safe alot cpu (include a fix_player() inside change_abil())
                 */
                /*change_abil(who, op);*/
                /*LOG(llevDebug, "UNAPPLY SKILL: %s change %s (%s) to NULL\n", query_name(who), query_name(op), query_name(who->chosen_skill) );*/
                who->chosen_skill = NULL;
                buf[0] = '\0';
                break;

            case ARMOUR:
            case HELMET:
            case SHOULDER:
            case LEGS:
            case SHIELD:
            case RING:
            case BOOTS:
            case GLOVES:
            case AMULET:
            case GIRDLE:
            case BRACERS:
            case CLOAK:
                change_abil(who, op);
                if(!op->item_condition)
                    sprintf(buf, "Your %s is broken!", query_name(op));
                else
                    sprintf(buf, "You take off the %s.", query_name(op));
                break;

            case ARROW:
            case BOW:
            case WAND:
            case ROD:
            case HORN:
                if(!op->item_condition)
                    sprintf(buf, "Your %s is broken!", query_name(op));
                else
                    sprintf(buf, "You unready the %s.", query_name(op));
                if(op->type != ARROW || op->sub_type1 > 127)
                {
                    if (who->type != PLAYER)
                    {
                        CLEAR_FLAG(who, FLAG_READY_BOW); break;
                    }
                }
                break;
            default:
                sprintf(buf, "You unapply the %s.", query_name(op));
                break;
        }
        if (buf[0] != '\0') /* urgh... what use of buf */
        {
            if (who->type == PLAYER)
            {
                new_draw_info(NDI_UNIQUE, 0, who, buf);
                FIX_PLAYER(who ,"apply special ");
            }
            else
                fix_monster(who);
        }

        if (!(aflags & AP_NO_MERGE))
        {
            tag_t   del_tag = op->count;
            object *cont    = op->env;
            tmp = merge_ob(op, NULL);
            if (who->type == PLAYER)
            {
                if (tmp)
                {
                    /* it was merged */
                    esrv_del_item(pl, del_tag, cont);
                    op = tmp;
                }
            }
        }

        if (who->type == PLAYER)
            esrv_send_item(who, op);

        return 0;
    }
    if (basic_flag == AP_UNAPPLY)
        return 0;
    i = 0;

    /* This goes through and checks to see if the player already has something
     * of that type applied - if so, unapply it.
     * This is a VERY important part -it ensures
     */
    if (op->type == WAND || op->type == ROD || op->type == HORN || op->type == BOW || (op->type == ARROW && op->sub_type1 >127))
        tmp_flag = 1;

    for (tmp = who->inv; tmp != NULL; tmp = tmp->below)
    {
        if ((tmp->type == op->type || (tmp_flag && (tmp->type == WAND || tmp->type == ROD || tmp->type == HORN || tmp->type == BOW || (tmp->type == ARROW && tmp->sub_type1 >127))))
                && QUERY_FLAG(tmp, FLAG_APPLIED) && tmp != op)
        {
            if (tmp->type == RING && !i)
                i = 1;
            else if (apply_special(who, tmp, 0))
                return 1;
        }
    }

    /* For clarity and ease of event handling I split this into
     * two parts, first a check and then the modifications */
    switch (op->type)
    {
        case RING:
        case AMULET:
        case BOW:
        case ARROW:
            if(!op->item_condition)
            {
                sprintf(buf, "The %s is broken and can't be applied.", query_name(op));
                new_draw_info(NDI_UNIQUE, 0, who, buf);
                return 1;
            }
            break;
        case WEAPON:
            if(!op->item_condition)
            {
                sprintf(buf, "The %s is broken and can't be applied.", query_name(op));
                new_draw_info(NDI_UNIQUE, 0, who, buf);
                return 1;
            }
            if (!QUERY_FLAG(who, FLAG_USE_WEAPON))
            {
                sprintf(buf, "You can't use %s.", query_name(op));
                new_draw_info(NDI_UNIQUE, 0, who, buf);
                return 1;
            }
            if (!check_weapon_power(who, op->last_eat))
            {
                new_draw_info(NDI_UNIQUE, 0, who, "That weapon is too powerful for you to use.");
                new_draw_info(NDI_UNIQUE, 0, who, "It would consume your soul!.");
                return 1;
            }
            if (op->level && (strncmp(op->name, who->name, strlen(who->name))))
            {
                /* if the weapon does not have the name as the character, can't use it. */
                /*        (Ragnarok's sword attempted to be used by Foo: won't work) */
                new_draw_info(NDI_UNIQUE, 0, who, "The weapon does not recognize you as its owner.");
                return 1;
            }

            /* if we have applied a shield, don't allow apply of polearm or 2hand weapon */
            if ((op->sub_type1 >= WEAP_POLE_IMPACT || op->sub_type1 >= WEAP_2H_IMPACT)
                    && who->type == PLAYER
                    && pl
                    && pl->equipment[PLAYER_EQUIP_SHIELD])
            {
                new_draw_info(NDI_UNIQUE, 0, who, "You can't wield this weapon and a shield.");
                return 1;
            }

            if (!check_skill_to_apply(who, op))
                return 1;
            break;
        case SHIELD:
            /* don't allow of polearm or 2hand weapon with a shield */
            if(!op->item_condition)
            {
                sprintf(buf, "The %s is broken and can't be applied.", query_name(op));
                new_draw_info(NDI_UNIQUE, 0, who, buf);
                return 1;
            }
            if ((who->type == PLAYER && pl && pl->equipment[PLAYER_EQUIP_WEAPON1])
                    && (pl->equipment[PLAYER_EQUIP_WEAPON1]->sub_type1 >= WEAP_POLE_IMPACT
                        || pl->equipment[PLAYER_EQUIP_WEAPON1]->sub_type1 >= WEAP_2H_IMPACT))
            {
                new_draw_info(NDI_UNIQUE, 0, who, "You can't wield this shield and a weapon.");
                return 1;
            }
            /* Fall through to next test... */

        case ARMOUR:
        case HELMET:
        case SHOULDER:
        case LEGS:
        case BOOTS:
        case GLOVES:
        case GIRDLE:
        case BRACERS:
        case CLOAK:
            if(!op->item_condition)
            {
                sprintf(buf, "The %s is broken and can't be applied.", query_name(op));
                new_draw_info(NDI_UNIQUE, 0, who, buf);
                return 1;
            }
            if (!QUERY_FLAG(who, FLAG_USE_ARMOUR))
            {
                sprintf(buf, "You can't use %s.", query_name(op));
                new_draw_info(NDI_UNIQUE, 0, who, buf);
                return 1;
            }
            break;

            /* this part is needed for skill-tools */
        case SKILL:
            if (who->chosen_skill)
            {
                LOG(llevBug, "BUG: apply_special(): can't apply two skills\n");
                return 1;
            }
            break;
    }

    /* Now we should be done with 99% of all tests. Generate the event
     * and then go on with side effects */
    if(trigger_object_plugin_event(
                EVENT_APPLY, op, who, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return 1; /* 1 = do not write an error message to the player */

    if (op->nrof > 1 && op->type != ARROW)
        tmp = get_split_ob(op, op->nrof - 1);
    else
        tmp = NULL;

    switch (op->type)
    {
        case WEAPON:
            SET_FLAG(op, FLAG_APPLIED);
            SET_FLAG(who, FLAG_READY_WEAPON);
            change_abil(who, op);
            sprintf(buf, "You wield the %s.", query_name(op));
            break;

        case SHIELD:
        case ARMOUR:
        case HELMET:
        case SHOULDER:
        case LEGS:
        case BOOTS:
        case GLOVES:
        case GIRDLE:
        case BRACERS:
        case CLOAK:
        case RING:
        case AMULET:
            SET_FLAG(op, FLAG_APPLIED);
            change_abil(who, op);
            sprintf(buf, "You put on the %s.", query_name(op));
            break;

            /* this part is needed for skill-tools */
        case SKILL:
            if (who->type == PLAYER)
            {
                if (!IS_INVISIBLE(op, who))
                {
                    /* for tools */
                    if (op->exp_obj)
                        LOG(llevBug, "BUG: apply_special(SKILL): found unapplied tool with experience object\n");
                    else
                        link_player_skill(who, op);
                    new_draw_info_format(NDI_UNIQUE, 0, who, "You ready the %s.", query_name(op));
                    new_draw_info_format(NDI_UNIQUE, 0, who, "You can now use the skill: %s.", skills[op->stats.sp].name);
                }
                else
                    send_ready_skill(who, skills[op->stats.sp].name);
            }
            SET_FLAG(op, FLAG_APPLIED);
            /* change_abil(who, op); */
            /*LOG(llevDebug, "APPLY SKILL: %s change %s to %s\n", query_name(who), query_name(who->chosen_skill), query_name(op) );*/
            who->chosen_skill = op;
            buf[0] = '\0';
            break;

        case ARROW:
            /* an arrow can be a.) a throw item (= distance weapon) or b.) ammunition for a distance weapon
            * as throw item we handle it like a bow/wand/etc...
            * as normal arrow special - we test first we have applied the right distance weapon for it
            */
            if(op->sub_type1 < 127) /* its amunition */
            {
                /* we want apply amun. Lets only allow to apply amun fitting the applied bow! */
                if(!pl->equipment[PLAYER_EQUIP_BOW] || pl->equipment[PLAYER_EQUIP_BOW]->type != BOW
                                        || pl->equipment[PLAYER_EQUIP_BOW]->sub_type1 != op->sub_type1)
                {
                    new_draw_info_format(NDI_UNIQUE, 0, who, "You can't use %s with applied range weapon.", query_short_name(op, NULL));
                    return 1;
                }
            }
        case WAND:
        case ROD:
        case HORN:
        case BOW:
            if (!check_skill_to_apply(who, op))
                return 1;

            if((op->type == ROD || op->type == HORN) && who->chosen_skill->level < op->level)
            {
                new_draw_info_format(NDI_UNIQUE, 0, who, "Your %s skill level is to low!", query_short_name(who->chosen_skill, who));
                return 1;
            }

            SET_FLAG(op, FLAG_APPLIED);
            new_draw_info_format(NDI_UNIQUE, 0, who, "You ready the %s.", query_name(op));
            if (who->type == PLAYER)
            {
                if (op->type != BOW)
                {
                    pl->known_spell = (QUERY_FLAG(op, FLAG_BEEN_APPLIED) || QUERY_FLAG(op, FLAG_IDENTIFIED));
                }
            }
            else
            {
                SET_FLAG(who, FLAG_READY_BOW); break;
            }
            break;

        default:
            sprintf(buf, "You apply the %s.", query_name(op));
    }
    if (!QUERY_FLAG(op, FLAG_APPLIED))
        SET_FLAG(op, FLAG_APPLIED);
    if (buf[0] != '\0')
        new_draw_info(NDI_UNIQUE, 0, who, buf);
    if (tmp != NULL)
        tmp = insert_ob_in_ob(tmp, who);
    FIX_PLAYER(who ,"apply special - you apply ");
    if (op->type != WAND && who->type == PLAYER)
        SET_FLAG(op, FLAG_BEEN_APPLIED);

    if (QUERY_FLAG(op, FLAG_PERM_CURSED))
        SET_FLAG(op, FLAG_CURSED);
    if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
        SET_FLAG(op, FLAG_DAMNED);

    if (QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED))
    {
        if (who->type == PLAYER)
        {
            new_draw_info(NDI_UNIQUE, 0, who, "Oops, it feels deadly cold!");
            SET_FLAG(op, FLAG_KNOWN_CURSED);
        }
    }
    if (who->type == PLAYER)
    {
        /* if multiple objects were applied, update both slots */
        if (tmp)
            esrv_send_item(who, tmp);
        esrv_send_item(who, op);
    }
    return 0;
}


int monster_apply_special(object *who, object *op, int aflags)
{
    if (QUERY_FLAG(op, FLAG_UNPAID) && !QUERY_FLAG(op, FLAG_APPLIED))
        return 1;
    return apply_special(who, op, aflags);
}




/* apply_player_light_refill() - refill lamps and all refill type light sources
 * from apply_player_light().
 * The light source must be in the inventory of the player, then he must mark the
 * light source and apply the refill item (lamp oil for example).
 */
void apply_player_light_refill(object *who, object *op)
{
    object *item;
    int     tmp;

    item = find_marked_object(who);
    if (!item)
    {
        new_draw_info_format(NDI_UNIQUE, 0, who, "Mark a light source first you want refill.");
        return;
    }


    if (item->type != TYPE_LIGHT_APPLY || !item->race || !strstr(item->race, op->race))
    {
        new_draw_info_format(NDI_UNIQUE, 0, who, "You can't refill the %s with the %s.", query_name(item),
                query_name(op));
        return;
    }

    if(trigger_object_plugin_event(EVENT_APPLY, op, who, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    /* ok, all is legal - now we refill the light source = settings item->food
     * = op-food. Then delete op or if its a stack, decrease nrof.
     * no idea about unidentified or cursed/damned effects for both items.
     */

    tmp = (int) item->stats.maxhp - item->stats.food;
    if (!tmp)
    {
        new_draw_info_format(NDI_UNIQUE, 0, who, "The %s is full and can't be refilled.", query_name(item));
        return;
    }

    if (op->stats.food <= tmp)
    {
        item->stats.food += op->stats.food;
        new_draw_info_format(NDI_UNIQUE, 0, who, "You refill the %s with %d units %s.", query_name(item),
                op->stats.food, query_name(op));
        decrease_ob(op);
    }
    else
    {
        object *filler;
        if (op->nrof > 1)
        {
            filler = get_split_ob(op, 1);
            filler->stats.food -= tmp;
            insert_ob_in_ob(filler, who);
            if (QUERY_FLAG(op, FLAG_REMOVED))
                esrv_del_item(CONTR(who), op->count, op->env);
            else
                esrv_send_item(who, op);
        }
        else
        {
            filler = op;
            filler->stats.food -= tmp;
        }

        item->stats.food += tmp;
        new_draw_info_format(NDI_UNIQUE, 0, who, "You refill the %s with %d units %s.", query_name(item), tmp,
                query_name(filler));

        esrv_send_item(who, filler);
    }
    esrv_send_item(who, item);
    FIX_PLAYER(who ,"apply light refill");
}

void turn_on_light(object *op)
{
    object *op_old;
    int     tricky_flag = FALSE; /* to delay insertion of object - or it simple remerge! */

    /* simple case for map light sources */
    if(op->type == LIGHT_SOURCE)
    {
        op->glow_radius = (sint8) op->last_sp;
        if (!op->env && op->glow_radius)
            adjust_light_source(op->map, op->x, op->y, op->glow_radius);
        return;
    }

    /* now we have a filled or permanent, extinguished light source
     * lets light it - BUT we still have light_radius not active
     * when we not drop or apply the source.
     */

    /* the old split code has some side effects -
     * i force now first a split of #1 per hand
     */
    op_old = op;
    if (op->nrof > 1)
    {
        object *one = get_object();
        copy_object(op, one);
        op->nrof -= 1;
        one->nrof = 1;
        if (op->env)
            esrv_update_item(UPD_NROF, op->env, op);
        else
            update_object(op, UP_OBJ_FACE);

        tricky_flag = TRUE;
        op = one;
    }

    /* light is applied in player inventory - so we
     * start the 3 apply chain - because it can be taken
     * in hand.
     */
    if (op_old->env && op_old->env->type == PLAYER)
    {
        if (op->last_eat) /* we have a non permanent source */
            SET_FLAG(op, FLAG_CHANGING);
        if(op->speed)
        {
            SET_FLAG(op, FLAG_ANIMATE);
            op->animation_id = op->arch->clone.animation_id; /* be sure to get the right anim */
            SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
        }
        if (tricky_flag)
        {
            op = insert_ob_in_ob(op, op_old->env);
            esrv_send_item(op->env, op);
        }
        op->glow_radius = (sint8) op->last_sp;
    }
    else /* we are not in a player inventory - so simple turn it on */
    {
        if (op->last_eat) /* we have a non permanent source */
            SET_FLAG(op, FLAG_CHANGING);
        if(op->speed)
        {
            SET_FLAG(op, FLAG_ANIMATE);
            op->animation_id = op->arch->clone.animation_id;
            SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
        }
        if (QUERY_FLAG(op, FLAG_PERM_CURSED))
            SET_FLAG(op, FLAG_CURSED);
        if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
            SET_FLAG(op, FLAG_DAMNED);

        if (tricky_flag)
        {
            if (!op_old->env)
                /* the item WAS before this on this spot - we only turn it on but we don't moved it */
                insert_ob_in_map(op, op_old->map, op_old, INS_NO_WALK_ON);
            else
            {
                op = insert_ob_in_ob(op, op_old->env);
                esrv_send_item(op->env, op);
            }
        }

        op->glow_radius = (sint8) op->last_sp;
        if (!op->env && op->glow_radius)
            adjust_light_source(op->map, op->x, op->y, op->glow_radius);

        update_object(op, UP_OBJ_FACE);
    }
}

void turn_off_light(object *op)
{
    if (!op->env && op->glow_radius) /* on map */
        adjust_light_source(op->map, op->x, op->y, -(op->glow_radius));

    /* Simple case for map light sources (non-appliable) */
    if(op->type == LIGHT_SOURCE)
    {
        op->last_sp = op->glow_radius;
        op->glow_radius = 0;
        return;
    }

    CLEAR_FLAG(op, FLAG_APPLIED);
    CLEAR_FLAG(op, FLAG_CHANGING);
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
    /* CLEAR_FLAG(op,FLAG_ANIMATE);
       op->face = op->arch->clone.face;
     */
    update_object(op, UP_OBJ_FACE);
    op->glow_radius = 0;
}

/* apply_player_light() - the new player light. old style torches will be
 * removed from arches but still in game. */
/* Note that op->msg is used for both non-applyable applyable lights (ie, to
 * respond to apply attempts) and for successfully applied applyable lights,
 * and is used both for light and extinguish attempts. Therefore, care should
 * be taken to change the msg if ever no_fix_player is changed (which can only
 * be done in the map file or via a script anyway) and the on/off status cannot
 * be mentioned in the (static) msg -- Smacky 20080905 */
void apply_player_light(object *who, object *op)
{
    object *tmp;

    /* Lights with no_fix_player 1 cannot be lit/extinguished by applying them
     * -- to prevent players buggering about with map design and puzzles. */
    if (QUERY_FLAG(op, FLAG_NO_FIX_PLAYER)) // FLAG_NO_APPLY would be better but there is no arch attribute
    {
        if (!(QUERY_FLAG(op, FLAG_NO_PICK)))
            LOG(llevBug, "BUG:: %s apply_player_light(): Pickable applyable light source flagged as no_apply!\n",
                         __FILE__);

        if (op->msg)
            new_draw_info(NDI_UNIQUE, 0, who, op->msg);
        else if (!op->glow_radius)
            new_draw_info_format(NDI_UNIQUE, 0, who, "You cannot light the %s.", query_name(op));
        else
            new_draw_info_format(NDI_UNIQUE, 0, who, "You cannot extinguish the %s.", query_name(op));

        return;
    }

    if (QUERY_FLAG(op, FLAG_APPLIED))
    {
        if ((QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED)))
        {
            new_draw_info_format(NDI_UNIQUE, 0, who, "No matter how hard you try, you just can't remove it!");
            return;
        }
        if (QUERY_FLAG(op, FLAG_PERM_CURSED))
            SET_FLAG(op, FLAG_CURSED);
        if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
            SET_FLAG(op, FLAG_DAMNED);

        if(trigger_object_plugin_event(EVENT_APPLY, who, op, NULL,
                    NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
            return;

        if (op->msg)
            new_draw_info(NDI_UNIQUE, 0, who, op->msg);
        else
            new_draw_info_format(NDI_UNIQUE, 0, who, "You extinguish the %s.",
                                 query_name(op));

        turn_off_light(op);

        update_object(who, UP_OBJ_FACE);
        FIX_PLAYER(who ,"apply light - extinguish");
    }
    else
    {
        /* now the tricky thing: with the first apply cmd, we enlight the light source.
         * with the second, we apply it. if we unapply a light source, we always extinguish
         * them implicit.
         */

        /* TYPE_LIGHT_APPLY light sources with last_sp (aka glow_radius) 0 are useless -
         * for example burnt out torches. The burnt out lights are still from same type
         * because they are perhaps applied from the player as they burnt out
         * and we don't want a player applying a illegal item.
         */
        if (!op->last_sp)
        {
            new_draw_info_format(NDI_UNIQUE, 0, who, "The %s can't be lit.", query_name(op));
            return;
        }


        /* if glow_radius == 0, we have a extinguished light source.
         * before we can put it in the hand to use it, we have to turn
         * the light on.
         */
        if (!op->glow_radius)
        {
            if (op->last_eat) /* we have a non permanent source */
            {
                if (!op->stats.food) /* if not permanent, this is "filled" counter */
                {
                    /* no food charges, we can't light it up-
                     * Note that light sources with other_arch set
                     * are non rechargable lights - like torches.
                     * they destroy
                     */
                    new_draw_info_format(NDI_UNIQUE, 0, who, "You must first refill or recharge the %s.", query_name(op));
                    return;
                }
            }

            if(trigger_object_plugin_event(EVENT_APPLY, who, op, NULL,
                        NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                return;

            if (op->env && op->env->type == PLAYER)
            {
                new_draw_info_format(NDI_UNIQUE, 0, who, "You prepare the %s to be your light source.",
                                     query_name(op));
                turn_on_light(op);
                FIX_PLAYER(who ,"apply light - turn on light");
            }
            else
            {
                if (op->msg)
                    new_draw_info(NDI_UNIQUE, 0, who, op->msg);
                else
                    new_draw_info_format(NDI_UNIQUE, 0, who, "You light the %s.",
                                         query_name(op));

                turn_on_light(op);
            }
        }
        else
        {
            if (op->env && op->env->type == PLAYER)
            {
                /* remove any other applied light source first */
                for (tmp = who->inv; tmp != NULL; tmp = tmp->below)
                {
                    if (tmp->type == op->type && QUERY_FLAG(tmp, FLAG_APPLIED) && tmp != op)
                    {
                        if ((QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED)))
                        {
                            new_draw_info_format(NDI_UNIQUE, 0, who,
                                    "No matter how hard you try, you just can't remove it!");
                            return;
                        }
                        if (QUERY_FLAG(tmp, FLAG_PERM_CURSED))
                            SET_FLAG(tmp, FLAG_CURSED);
                        if (QUERY_FLAG(tmp, FLAG_PERM_DAMNED))
                            SET_FLAG(tmp, FLAG_DAMNED);

                        if(trigger_object_plugin_event(EVENT_APPLY, who, op, NULL,
                                    NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                            return;

                        if (tmp->msg)
                            new_draw_info(NDI_UNIQUE, 0, who, tmp->msg);
                        else
                            new_draw_info_format(NDI_UNIQUE, 0, who, "You extinguish the %s.",
                                                 query_name(tmp));

                        CLEAR_FLAG(tmp, FLAG_APPLIED);

                        turn_off_light(tmp);
                        esrv_send_item(who, tmp);
                    }
                }

                if (op->msg)
                    new_draw_info(NDI_UNIQUE, 0, who, op->msg);
                else
                    new_draw_info_format(NDI_UNIQUE, 0, who, "You apply the %s as your light source.",
                                         query_name(op));

                SET_FLAG(op, FLAG_APPLIED);
                FIX_PLAYER(who ," apply light - apply light");
                update_object(who, UP_OBJ_FACE);
            }
            else /* not part of player inv - turn light off ! */
            {
                if (QUERY_FLAG(op, FLAG_PERM_CURSED))
                    SET_FLAG(op, FLAG_CURSED);
                if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
                    SET_FLAG(op, FLAG_DAMNED);

                if(trigger_object_plugin_event(EVENT_APPLY, who, op, NULL,
                            NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                    return;

                if (op->msg)
                    new_draw_info(NDI_UNIQUE, 0, who, op->msg);
                else
                    new_draw_info_format(NDI_UNIQUE, 0, who, "You extinguish the %s.",
                                         query_name(op));

                turn_off_light(op);
            }
        }
    }
    if (op->env && (op->env->type == PLAYER || op->env->type == CONTAINER))
        esrv_send_item(op->env, op);
}


/* apply_lighter() - designed primarily to light torches/lanterns/etc.
 * Also burns up burnable material too. First object in the inventory is
 * the selected object to "burn". -b.t.
 */
/* i have this item type not include in daimonin atm - MT-2004 */
/* Currently doesn not generate APPLY events - Gecko 2005-05-15 */
void apply_lighter(object *who, object *lighter)
{
    object *item;
    tag_t   count;
    uint32  nrof;
    int     is_player_env   = 0;
    char    item_name[MAX_BUF];

    item = find_marked_object(who);
    if (item)
    {
        if (lighter->last_eat && lighter->stats.food)
        {
            /* lighter gets used up */
            /* Split multiple lighters if they're being used up.  Otherwise    *
             * one charge from each would be used up.  --DAMN        */
            if (lighter->nrof > 1)
            {
                object *oneLighter  = get_object();
                copy_object(lighter, oneLighter);
                lighter->nrof -= 1;
                oneLighter->nrof = 1;
                oneLighter->stats.food--;
                esrv_send_item(who, lighter);
                oneLighter = insert_ob_in_ob(oneLighter, who);
                esrv_send_item(who, oneLighter);
            }
            else
            {
                lighter->stats.food--;
            }
        }
        else if (lighter->last_eat)
        {
            /* no charges left in lighter */
            new_draw_info_format(NDI_UNIQUE, 0, who, "You attempt to light the %s with a used up %s.", item->name,
                                 lighter->name);
            return;
        }
        /* Perhaps we should split what we are trying to light on fire?
         * I can't see many times when you would want to light multiple
         * objects at once.
         */
        nrof = item->nrof;
        count = item->count;
        /* If the item is destroyed, we don't have a valid pointer to the
         * name object, so make a copy so the message we print out makes
         * some sense.
         */
        strcpy(item_name, item->name);
        if (who == is_player_inv(item))
            is_player_env = 1;

        /* Change to check count and not freed, since the object pointer
         * may have gotten recycled
         */
        if ((nrof != item->nrof) || (count != item->count))
        {
            new_draw_info_format(NDI_UNIQUE, 0, who, "You light the %s with the %s.", item_name, lighter->name);
            if (is_player_env)
                FIX_PLAYER(who ,"apply lighter ");
        }
        else
        {
            new_draw_info_format(NDI_UNIQUE, 0, who, "You attempt to light the %s with the %s and fail.", item->name,
                                 lighter->name);
        }
    }
    else /* nothing to light */
        new_draw_info(NDI_UNIQUE, 0, who, "You need to mark a lightable object.");
}

/* scroll_failure()- hacked directly from spell_failure */

void scroll_failure(object *op, int failure, int power)
{
    if (abs(failure / 4) > power)
        power = abs(failure / 4); /* set minimum effect */

    if (failure <= -1 && failure > -15) /* wonder */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your spell warps!.");
        cast_cone(op, op, 0, 10, SP_WOW, spellarch[SP_WOW], SK_level(op), 0);
    }
    else if (failure <= -15 && failure > -35) /* drain mana */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Your mana is drained!.");
        op->stats.sp -= random_roll(0, power - 1);
        if (op->stats.sp < 0)
            op->stats.sp = 0;
    }

    /* even nastier effects continue...*/
#ifdef SPELL_FAILURE_EFFECTS /* removed this - but perhaps we want add some of this nasty effects */
    else if (failure <= -35 && failure > -60) /* confusion */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "The magic recoils on you!");
        confuse_player(op, op, power);
    }
    else if (failure <= -60 && failure > -70) /* paralysis */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "The magic recoils and paralyzes you!");
        paralyze_player(op, op, power);
    }
    else if (failure <= -70 && failure > -80) /* blind */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "The magic recoils on you!");
        blind_player(op, op, power);
    }
    else if (failure <= -80) /* blast the immediate area */
    {
        new_draw_info(NDI_UNIQUE, 0, op, "You unlease uncontrolled mana!");
        cast_mana_storm(op, power);
    }
#endif
}

/*  peterm:  do_power_crystal

object *op, object *crystal

This function handles the application of power crystals.
Power crystals, when applied, either suck power from the applier,
if he's at full spellpoints, or gives him power, if it's got
spellpoins stored.

*/
int apply_power_crystal(object *op, object *crystal)
{
    int available_power;
    int power_space;
    int power_grab;

    available_power = op->stats.sp - op->stats.maxsp;
    power_space = crystal->stats.maxsp - crystal->stats.sp;
    power_grab = 0;
    if (available_power >= 0 && power_space > 0)
        power_grab = (int) MIN((float) power_space, ((float) 0.5 * (float) op->stats.sp));
    if (available_power <0 && crystal->stats.sp>0)
        power_grab = -MIN(-available_power, crystal->stats.sp);

    op->stats.sp -= power_grab;
    crystal->stats.sp += power_grab;
    crystal->speed = (float) crystal->stats.sp / (float) crystal->stats.maxsp;
    update_ob_speed(crystal);
    if (op->type == PLAYER)
        esrv_update_item(UPD_ANIMSPEED, op, crystal);

    return 1;
}
