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

#if defined(vax) || defined(ibm032)
size_t  strftime(char *, size_t, const char *, const struct tm *);
time_t  mktime(struct tm *);
#endif

static void ApplySavebed(player_t *pl, object_t *bed);
static void ApplyIdAltar(object_t *money, object_t *altar, object_t *pl);
static void ApplyPotion(object_t *op, object_t *tmp);
static void ApplyContainer(object_t *op, object_t *sack);
static void ApplyAltar(object_t *altar, object_t *sacrifice, object_t *originator);
static void ApplyShopMat(object_t *shop_mat, object_t *op);
static void ApplySign(object_t *op, object_t *sign);
static void ApplyBook(object_t *op, object_t *tmp);
static void ApplySpellbook(object_t *op, object_t *tmp);
static void ApplyScroll(object_t *op, object_t *tmp);
static void ApplyTreasure(object_t *op, object_t *tmp);
static void ApplyFood(object_t *op, object_t *tmp);
static void ApplyPoison(object_t *op, object_t *tmp);
static void ApplyArmourImprover(object_t *op, object_t *tmp);
static void ApplyLightRefill(object_t *who, object_t *op);
static void ApplyLighter(object_t *who, object_t *lighter);
static void ApplyPowerCrystal(object_t *op, object_t *crystal);

static void ApplyIdAltar(object_t *money, object_t *altar, object_t *pl)
{
    object_t *id,
           *next,
           *marked;
    int     success = 0;

    if (pl == NULL || pl->type != PLAYER)
        return;

    /* Check for MONEY type is a special hack - it prevents 'nothing needs
     * identifying' from being printed out more than it needs to be.
     */
    if (!check_altar_sacrifice(altar, money) || money->type != MONEY)
        return;

    /* Event trigger and quick exit */
    if(trigger_object_plugin_event(EVENT_TRIGGER,
                altar, pl, money,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
        return;

    marked = find_marked_object(pl);
    /* if the player has a marked item, identify that if it needs to be
     * identified.  IF it doesn't, then go through the player inventory.
     */
    if (marked && !QUERY_FLAG(marked, FLAG_IDENTIFIED) && need_identify(marked))
    {
        if (operate_altar(altar, &money))
        {
            identify(marked);
            ndi(NDI_UNIQUE, 0, pl, "You have %s.",
                QUERY_SHORT_NAME(marked, pl));
            if (marked->msg)
            {
                ndi(NDI_UNIQUE, 0, pl, "The item has a story:\n%s", marked->msg);
            }
            return;
        }
    }

    FOREACH_OBJECT_IN_OBJECT(id, pl, next)
    {
        if (!QUERY_FLAG(id, FLAG_IDENTIFIED) &&
            !QUERY_FLAG(id, FLAG_SYS_OBJECT) &&
            !IS_NORMAL_INVIS_TO(id, pl) &&
            need_identify(id))
        {
            if (operate_altar(altar, &money))
            {
                identify(id);
                ndi(NDI_UNIQUE, 0, pl, "You have %s.",
                    QUERY_SHORT_NAME(id, pl));
                if (id->msg)
                {
                    ndi(NDI_UNIQUE, 0, pl, "The item has a story:\n%s", id->msg);
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
        ndi(NDI_UNIQUE, 0, pl, "You have nothing that needs identifying");
}

static void ApplyPotion(object_t *op, object_t *tmp)
{
    int i, bonus = 1;

    /* some sanity checks */
    if (!op || !tmp)
    {
        LOG(llevBug, "ApplyPotion() called with invalid objects! obj: %s -- tmp: %s\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp));
        return;
    }

    if(trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    if (op->type == PLAYER)
    {
        object_t *sparkly;

        /* set chosen_skill to "magic device" - thats used when we "use" a potion */
        if (!change_skill(op, SK_MAGIC_DEVICES))
            return; /* no skill, no potion use (dust & balm too!) */

        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            identify(tmp);

        /* special potions. Only players get this */
        if (tmp->last_eat == -1) /* create a force and copy the effects in */
        {
            object_t *force = arch_to_object(archetype_global._potion_effect);

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

                sparkly = sparkly_create(archetype_global._meffect, op, -1, SOUND_DRINK_POISON, SOUND_NORMAL);
                i = find_animation("meffect_purple");
                sparkly->animation_id = i;                
            }
            else /* all positive (when not on default negative) */
            {
                /* we don't must do the hard way like cursed/damned (no multiplication or
                         * sign change).
                         */
                memcpy(force->resist, tmp->resist, sizeof(tmp->resist));
                memcpy(force->attack, tmp->attack, sizeof(tmp->attack));
                sparkly = sparkly_create(archetype_global._meffect, op, -1, SOUND_MAGIC_DEFAULT, SOUND_SPELL);
                i = find_animation("meffect_purple");
                sparkly->animation_id = i;                
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
                ndi(NDI_UNIQUE, 0, op, "Nothing happened.");
            decrease_ob_nr(tmp, 1);
            return;
        }

        if (tmp->last_eat == 1) /* Potion of minor restoration */
        {
            object_t     *depl;
            archetype_t  *at;
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
                FIX_PLAYER(op ,"ApplyPotion - minor restoration - damned");
                decrease_ob_nr(tmp, 1);
                sparkly = sparkly_create(archetype_global._meffect, op, -1, SOUND_DRINK_POISON, SOUND_NORMAL);
                i = find_animation("meffect_purple");
                sparkly->animation_id = i;                
                return;
            }
            if ((at = find_archetype("drain")) == NULL)
            {
                LOG(llevBug, "BUG: Could not find archetype depletion");
                return;
            }
            depl = present_arch_in_ob(at, op);
            if (depl != NULL)
            {
                stat_nr_t nr;

                for (nr = 0; nr < STAT_NROF; nr++)
                {
                    if (get_stat_value(&depl->stats, nr))
                        ndi(NDI_UNIQUE, 0, op, "%s", restore_msg[nr]);
                }
                remove_ob(depl); /* in inventory of ... */
                FIX_PLAYER(op ,"ApplyPotion - minor restoration");
            }
            else
            {
                ndi(NDI_UNIQUE, 0, op, "You feel a great loss...");
            }

            decrease_ob_nr(tmp, 1);
            sparkly = sparkly_create(archetype_global._meffect, op, -1, SOUND_MAGIC_DEFAULT, SOUND_SPELL);
            i = find_animation("meffect_yellow");
            sparkly->animation_id = i;                
            return;
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
                    for (i = 2; i <= CONTR(op)->skillgroup_ptr[SKILLGROUP_MAGIC]->level; i++)
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
                    for (i = 2; i <= CONTR(op)->skillgroup_ptr[SKILLGROUP_WISDOM]->level; i++)
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
                    for (i = 2; i <= CONTR(op)->skillgroup_ptr[SKILLGROUP_MAGIC]->level; i++)
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
                    for (i = 2; i <= CONTR(op)->skillgroup_ptr[SKILLGROUP_WISDOM]->level; i++)
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
                ndi(NDI_UNIQUE, 0, op, "The potion had no effect - you are already perfect.");
                play_sound_map(MSP_KNOWN(op), SOUND_MAGIC_DEFAULT, SOUND_SPELL);
            }
            else if (success_flag == 1)
            {
                FIX_PLAYER(op ,"ApplyPotion - improvement");
                sparkly = sparkly_create(archetype_global._meffect, op, -1, SOUND_MAGIC_DEFAULT, SOUND_SPELL);
                i = find_animation("meffect_green");
                sparkly->animation_id = i;                
                ndi(NDI_UNIQUE, 0, op, "You feel a little more perfect!");
            }
            else if (success_flag == 2)
            {
                FIX_PLAYER(op ,"ApplyPotion - improvement - cursed");
                sparkly = sparkly_create(archetype_global._meffect, op, -1, SOUND_DRINK_POISON, SOUND_NORMAL);
                i = find_animation("meffect_purple");
                sparkly->animation_id = i;                
                ndi(NDI_UNIQUE, 0, op, "The foul potion burns like fire in you!");
            }
            else /* bad potion but all values of this player are 1! poor poor guy.... */
            {
                sparkly = sparkly_create(archetype_global._meffect, op, -1, SOUND_DRINK_POISON, SOUND_NORMAL);
                i = find_animation("meffect_pink");
                sparkly->animation_id = i;                
                ndi(NDI_UNIQUE, 0, op, "The potion was foul but had no effect on your tortured body.");
            }
            decrease_ob_nr(tmp, 1);
            return;
        }
    }


    if (tmp->stats.sp == SP_NO_SPELL)
    {
        ndi(NDI_UNIQUE, 0, op, "Nothing happens as you apply it.");
        decrease_ob_nr(tmp, 1);
        return;
    }


    /* A potion that casts a spell.  Healing, restore spellpoint (power potion)
     * and heroism all fit into this category.
     */
    if (tmp->stats.sp != SP_NO_SPELL)
    {
        cast_spell(op, tmp, 0, tmp->stats.sp, 1, spellPotion, NULL); /* apply potion ALWAYS fire on the spot the applier stands - good for healing - bad for firestorm */
        decrease_ob_nr(tmp, 1);
        /* if youre dead, no point in doing this... */
        if (!QUERY_FLAG(op, FLAG_REMOVED))
            FIX_PLAYER(op ,"ApplyPotion - cast something");
        return;
    }

    /* CLEAR_FLAG is so that if the character has other potions
     * that were grouped with the one consumed, his
     * stat will not be raised by them.  fix_player just clears
     * up all the stats.
     */
    CLEAR_FLAG(tmp, FLAG_APPLIED);
    FIX_PLAYER(op ,"ApplyPotion - end");
    decrease_ob_nr(tmp, 1);
}

/****************************************************************************
 * Weapon improvement code follows
 ****************************************************************************/

int check_item(object_t *op, const char *item)
{
    int count   = 0;


    if (item == NULL)
        return 0;
    op = op->below;
    while (op != NULL)
    {
        if (op->arch->name == item)
        {
            if (!QUERY_FLAG(op, FLAG_CURSED) && !QUERY_FLAG(op, FLAG_DAMNED) /* Loophole bug? -FD- */ && !QUERY_FLAG(op, FLAG_UNPAID))
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

void eat_item(object_t *op, const char *item)
{
    object_t *prev;

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
int check_weapon_power(object_t *who, int improvs)
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
        object_t *wc_obj,
               *next;

        FOREACH_OBJECT_IN_OBJECT(wc_obj, who, next)
        {
            if (wc_obj->type == TYPE_SKILLGROUP &&
                wc_obj->stats.Str)
            {
                break;
            }
        }

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
static int check_sacrifice(object_t *op, object_t *improver)
{
    int count   = 0;

    if (improver->slaying != NULL)
    {
        count = check_item(op, improver->slaying);
        if (count < 1)
        {
            ndi(NDI_UNIQUE, 0, op, "The gods want more %ss", improver->slaying);
            return 0;
        }
    }
    else
        count = 1;

    return count;
}

int improve_weapon_stat(object_t *op, object_t *improver, object_t *weapon, signed char *stat, int sacrifice_count,
                        char *statname)
{
    ndi(NDI_UNIQUE, 0, op, "Your sacrifice was accepted.");
    *stat += sacrifice_count;
    weapon->last_eat++;
    ndi(NDI_UNIQUE, 0, op, "Weapon's bonus to %s improved by %d", statname, sacrifice_count);
    decrease_ob_nr(improver, 1);

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

int prepare_weapon(object_t *op, object_t *improver, object_t *weapon)
{
    int     sacrifice_count, i;
    char    buf[MEDIUM_BUF];

    if (weapon->level != 0)
    {
        ndi(NDI_UNIQUE, 0, op, "Weapon already prepared.");
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
        ndi(NDI_UNIQUE, 0, op, "Cannot prepare magic weapons.");
        return 0;
    }
    sacrifice_count = check_sacrifice(op, improver);
    if (sacrifice_count <= 0)
        return 0;
    sacrifice_count = isqrt(sacrifice_count);
    weapon->level = sacrifice_count;
    ndi(NDI_UNIQUE, 0, op, "Your sacrifice was accepted.");
    eat_item(op, improver->slaying);

    ndi(NDI_UNIQUE, 0, op, "Your *%s may be improved %d times.", weapon->name, sacrifice_count);

    sprintf(buf, "%s's %s", op->name, weapon->name);
    FREE_AND_COPY_HASH(weapon->name, buf);

    weapon->nrof = 0;  /*  prevents preparing n weapons in the same
                       slot at once! */
    decrease_ob_nr(improver, 1);
    weapon->last_eat = 0;
    return 1;
}


/* This is the new improve weapon code */
/* build_weapon returns 0 if it was not able to work. */
/* #### We are hiding extra information about the weapon in the level and
   last_eat numbers for an object.  Hopefully this won't break anything ??
   level == max improve last_eat == current improve*/
int improve_weapon(object_t *op, object_t *improver, object_t *weapon)
{
    int sacrifice_count, sacrifice_needed = 0;

    if (improver->stats.sp == IMPROVE_PREPARE)
    {
        return prepare_weapon(op, improver, weapon);
    }
    if (weapon->level == 0)
    {
        ndi(NDI_UNIQUE, 0, op, "This weapon has not been prepared.");
        return 0;
    }
    if (weapon->level == weapon->last_eat)
    {
        ndi(NDI_UNIQUE, 0, op, "This weapon cannot be improved any more.");
        return 0;
    }
    if (QUERY_FLAG(weapon, FLAG_APPLIED) && !check_weapon_power(op, weapon->last_eat + 1))
    {
        ndi(NDI_UNIQUE, 0, op, "Improving the weapon will make it too powerful for you to use. Unready it if you really want to improve it.");
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
        ndi(NDI_UNIQUE, 0, op, "Damage has been increased by 5 to %d", weapon->stats.dam);
        weapon->last_eat++;
        decrease_ob_nr(improver, 1);
        return 1;
    }
    if (improver->stats.sp == IMPROVE_WEIGHT)
    {
        /* Reduce weight by 20% */
        weapon->weight = (weapon->weight * 8) / 10;
        if (weapon->weight < 1)
            weapon->weight = 1;
        ndi(NDI_UNIQUE, 0, op, "Weapon weight reduced to %6.1f kg", (float) weapon->weight / 1000.0);
        weapon->last_eat++;
        decrease_ob_nr(improver, 1);
        return 1;
    }
    if (improver->stats.sp == IMPROVE_ENCHANT)
    {
        weapon->magic++;
        weapon->last_eat++;
        ndi(NDI_UNIQUE, 0, op, "Weapon magic increased to %d", weapon->magic);
        decrease_ob_nr(improver, 1);
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
        ndi(NDI_UNIQUE, 0, op, "You need at least %d %s", sacrifice_needed, improver->slaying);
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
          ndi(NDI_UNIQUE, 0, op, "Unknown improvement type.");
    }
    LOG(llevBug, "BUG: improve_weapon: Got to end of function\n");
    return 0;
}

int check_improve_weapon(object_t *op, object_t *tmp)
{
    object_t *otmp;

    if (op->type != PLAYER)
        return 0;
    otmp = find_marked_object(op);
    if (!otmp)
    {
        ndi(NDI_UNIQUE, 0, op, "You need to mark a weapon object.");
        return 0;
    }
    if (otmp->type != WEAPON)
    {
        ndi(NDI_UNIQUE, 0, op, "Marked item is not a weapon");
        return 0;
    }

    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return 1;

    ndi(NDI_UNIQUE, 0, op, "Applied weapon builder.");
    improve_weapon(op, tmp, otmp);
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

int improve_armour(object_t *op, object_t *improver, object_t *armour)
{
    int new_armour;

    new_armour = armour->resist[ATNR_IMPACT] + armour->resist[ATNR_IMPACT] / 25 + op->level / 20 + 1;
    if (new_armour > 90)
        new_armour = 90;

    if (armour->magic >= (op->level / 10 + 1) || new_armour > op->level)
    {
        ndi(NDI_UNIQUE, 0, op, "You are not yet powerfull enough to improve this armour.");
        return 0;
    }

    if (new_armour > armour->resist[ATNR_IMPACT])
    {
        armour->resist[ATNR_IMPACT] = new_armour;
        armour->weight += (unsigned long) ((double) armour->weight * (double) 0.05);
    }
    else
    {
        ndi(NDI_UNIQUE, 0, op, "The armour value of this equipment cannot be further improved.");
    }
    armour->magic++;
    if (op->type == PLAYER)
    {
        if (QUERY_FLAG(armour, FLAG_APPLIED))
            FIX_PLAYER(op ,"improve armour");
    }
    decrease_ob_nr(improver, 1);
    return 1;
}


/*
 * convert_item() returns 1 if anything was converted, otherwise 0
 */
#define CONV_FROM(xyz)  (xyz->slaying)
#define CONV_TO(xyz)    (xyz->other_arch)
#define CONV_NR(xyz)    ((unsigned long) xyz->stats.sp)      /* receive number */
#define CONV_NEED(xyz)  ((unsigned long) xyz->stats.food)    /* cost number */

int convert_item(object_t *item, object_t *converter, object_t *originator)
{
    int       nr = 0;
    msp_t *msp;
    object_t   *tmp;

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

    msp = MSP_KNOWN(converter);
    MSP_GET_SYS_OBJ(msp, SHOP_FLOOR, tmp);

    if (tmp)
    {
        SET_FLAG(item, FLAG_UNPAID);
    }

    item->x = converter->x;
    item->y = converter->y;
    insert_ob_in_map(item, converter->map, converter, 0);
    return 1;
}

/* Eneq(@csd.uu.se): Handle apply on containers.
 * op is the player, sack is the container the player is opening or closing.
 * return 1 if an object is apllied somehow or another, 0 if error/no apply
 *
 * Reminder - there are three states for any container - closed (non applied),
 * applied (not open, but objects that match get tossed into it), and open
 * (applied flag set, and op->container points to the open container)
 * I added mutiple apply of one container with a player list. MT 07.02.2004 */
static void ApplyContainer(object_t *op, object_t *sack)
{
    object_t *cont;

    /* TODO: add support for cursed containers that can't be unreadied? */

    if (op->type != PLAYER)
    {
        LOG(llevBug, "BUG: ApplyContainer: called from non player: <%s>!\n", STRING_OBJ_NAME(op));
        return;
    }

    cont = CONTR(op)->container; /* cont is NULL or the container player already has opened */

    if (sack == NULL || sack->type != CONTAINER || (cont && cont->type != CONTAINER))
    {
        LOG(llevBug, "BUG: ApplyContainer: object_t *sack = %s is not container (cont:<%s>)!\n", STRING_OBJ_NAME(sack),
            STRING_OBJ_NAME(cont));
        return;
    }


    /* close container? */
    if (cont) /* if cont != sack || cont == sack - in both cases we close cont */
    {
        if(trigger_object_plugin_event(EVENT_CLOSE, cont, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
            return;

        ndi(NDI_UNIQUE, 0, op, "You %s %s.",
            (container_unlink(CONTR(op), cont)) ? "close" : "leave",
            query_name(cont, op, ARTICLE_DEFINITE, 0));

        if (cont == sack) /* we closing the one we applied */
            return;
    }

    /* at this point we ready a container OR we open it! */

    /* If the player is trying to open it (which he must be doing if we got here),
     * and it is locked, check to see if player has the equipment to open it.
     */

    if (sack->slaying) /* it's locked or personalized*/
    {
        if (sack->sub_type1 == ST1_CONTAINER_NORMAL)
        {
            object_t *key = find_key(op, sack);

            if (!key)
            {
                ndi(NDI_UNIQUE, 0, op, "You don't have a key to unlock %s.",
                    query_name(sack, op, ARTICLE_DEFINITE, 0));
                return;
            }

            ndi(NDI_UNIQUE, 0, op, "You unlock %s with %s.",
                query_name(sack, op, ARTICLE_DEFINITE, 0),
                QUERY_SHORT_NAME(key, op));
        }
        else
        {
            /* only give player with right name access */
            if (sack->sub_type1 == ST1_CONTAINER_CORPSE_group &&
                    (!(CONTR(op)->group_status & GROUP_STATUS_GROUP) ||
                        CONTR(CONTR(op)->group_leader)->group_id != sack->stats.maxhp))
            {
                ndi(NDI_UNIQUE, 0, op, "Its not your groups bounty.");
                return;
            }
            else if (sack->sub_type1 == ST1_CONTAINER_CORPSE_player && sack->slaying != op->name)
            {
                ndi(NDI_UNIQUE, 0, op, "Its not your bounty.");
                return;
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
            ndi(NDI_UNIQUE, 0, op, "You can't open %s.",
                query_name(sack, op, ARTICLE_DEFINITE, 0));
            return;
        }

        ndi(NDI_UNIQUE, 0, op, "You open %s.",
            query_name(sack, op, ARTICLE_DEFINITE, 0));
        SET_FLAG(sack, FLAG_BEEN_APPLIED);
        container_link(CONTR(op), sack);
    }
    else/* sack is in players inventory */
    {
        if (QUERY_FLAG(sack, FLAG_APPLIED)) /* readied sack becoming open */
        {
            ndi(NDI_UNIQUE, 0, op, "You open %s.",
                query_name(sack, op, ARTICLE_DEFINITE, 0));
            SET_FLAG(sack, FLAG_BEEN_APPLIED);
            container_link(CONTR(op), sack);
        }
        else
        {
            object_t *tmp,
                   *next;

            /* We don't allow multiple applied containers of the same type (race) */
            /* No need for recursive search, since only top-level containers may be applied */
            FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
            {
                if(QUERY_FLAG(tmp, FLAG_APPLIED) && tmp->type == CONTAINER &&
                        tmp->race == sack->race && tmp != sack)
                {
                    CLEAR_FLAG(tmp, FLAG_APPLIED);
                    ndi(NDI_UNIQUE, 0, op, "You unreadied %s.",
                        query_name(tmp, op, ARTICLE_DEFINITE, 0));
#ifndef USE_OLD_UPDATE
                    OBJECT_UPDATE_UPD(tmp, UPD_FLAGS);
#else
                    update_object(tmp, UP_OBJ_FACE);
                    esrv_update_item(UPD_FLAGS, tmp);
#endif
                }
            }

            ndi(NDI_UNIQUE, 0, op, "You readied %s.",
                query_name(sack, op, ARTICLE_DEFINITE, 0));
            SET_FLAG(sack, FLAG_APPLIED);
#ifndef USE_OLD_UPDATE
            OBJECT_UPDATE_UPD(sack, UPD_FLAGS);
#else
            update_object(sack, UP_OBJ_FACE);
            esrv_update_item(UPD_FLAGS, sack);
#endif
            container_trap(op, sack);   /* search & explode a rune in the container */
        }
    }
}

static void ApplyAltar(object_t *altar, object_t *sacrifice, object_t *originator)
{
    /* Only players can make sacrifices on spell casting altars. */
    if (altar->stats.sp != -1 && (!originator || originator->type != PLAYER))
        return;
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
            ndi(NDI_WHITE, 0, originator, "The altar casts %s.", spells[altar->stats.sp].name);
            cast_spell(originator, altar, altar->last_sp, altar->stats.sp, 0, spellPotion, NULL);
            /* If it is connected, push the button.  Fixes some problems with
             * old maps.
             */
            signal_connection(altar, sacrifice, originator, altar->map);
        }
        else
        {
            altar->weight_limit = 1;  /* works only once */
            signal_connection(altar, sacrifice, originator, altar->map);
        }
        return;
    }
    else
    {
        return;
    }
}

static void ApplyShopMat(object_t *shop_mat, object_t *op)
{
    msp_t *msp;

    /* Event trigger and quick exit */
    if (trigger_object_plugin_event(EVENT_TRIGGER, shop_mat, op, NULL, NULL,
        NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
    {
        return;
    }

    SET_FLAG(op, FLAG_NO_APPLY); // prevent loops

    /* Players who can use a shop mat (either entering a shop or leaving with
     * affordable goods (will be bought by shop_checkout())/nothing). */
    if (op->type == PLAYER &&
        shop_checkout(op, op->inv))
    {
        object_t   *shop;

        msp = MSP_KNOWN(shop_mat);
        MSP_GET_SYS_OBJ(msp, SHOP_FLOOR, shop);

        /* When leaving a shop, print the mat msg if any. */
        if (shop &&
            shop_mat->msg)
        {
            ndi(NDI_UNIQUE, 0, op, "%s", shop_mat->msg);
        }

        /* Non-destructively teleported. */
        if (!teleport(shop_mat, op))
        {
            msp = MSP_KNOWN(op);
            MSP_GET_SYS_OBJ(msp, SHOP_FLOOR, shop);

            /* When entering a shop, print the floor msg and name if any. */
            if (shop &&
                shop->msg)
            {
                char buf[LARGE_BUF];

                sprintf(buf, "%s", shop->msg);

                if (shop->name)
                {
                    sprintf(strrchr(buf, '\n'), " %s!\n", shop->name);
                }

                ndi(NDI_UNIQUE, 0, op, "%s", buf);
            }
        }
    }
    /* Players who cannot pass though the shop mat and non-players. */
    else
    {
        msp = MSP_KNOWN(op);
        (void)enter_map(op, msp, shop_mat, OVERLAY_FIXED, 0);
    }

    CLEAR_FLAG(op, FLAG_NO_APPLY);
}

static void ApplySign(object_t *op, object_t *sign)
{
    int raceval = 0;

    if (sign->stats.food)
    {
        if (sign->last_eat >= sign->stats.food)
        {
            if (!QUERY_FLAG(sign, FLAG_WALK_ON) && !QUERY_FLAG(sign, FLAG_FLY_ON))
                ndi(NDI_UNIQUE, 0, op, "You cannot read it anymore.");
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
    if (!QUERY_FLAG(sign, FLAG_WALK_ON) &&
        !QUERY_FLAG(sign, FLAG_FLY_ON))
    {
        if (QUERY_FLAG(op, FLAG_BLIND))
        {
            ndi(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
            return;
        }

        /* you has the right skill & language knowledge to read it? */
        else if (sign->msg)
        {
            if (!change_skill(op, SK_LITERACY))
            {
                ndi(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
                return;
            }
            else if((op->chosen_skill->weight_limit & sign->weight_limit) != sign->weight_limit)
            {
                ndi(NDI_UNIQUE, 0, op, "You are unable to decipher %s.\nIt is written in %s.",
                    query_name(sign, op, ARTICLE_DEFINITE, 0), get_language(sign->weight_limit));
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
    if (sign->race)
        sscanf(sign->race, "%d", &raceval);
    if (!QUERY_FLAG(sign, FLAG_SYS_OBJECT))
    {
        if (!sign->msg)
            ndi(NDI_UNIQUE, 0, op, "Nothing is written on %s.",
                query_name(sign, op, ARTICLE_DEFINITE, 0));
        else
        {
            ndi(NDI_UNIQUE, 0, op, "%s is written in %s.\nYou start reading it.",
                query_name(sign, op, ARTICLE_DEFINITE, 0), get_language(sign->weight_limit));
            ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "%s", sign->msg);
        }
    }
    /* magic mouth */
    else if (sign->msg ||
             (raceval &&
              sign->slaying))
    {
        if (sign->direction == 0 ||
            sign->direction == op->direction ||
            (QUERY_FLAG(sign, FLAG_SPLITTING) &&
             (absdir(sign->direction + 1) == op->direction ||
              absdir(sign->direction - 1) == op->direction)))
        {
            if (sign->msg)
                ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "%s", sign->msg);

            if (raceval && sign->slaying)
            {
                int sid;

                if ((sid = lookup_sound(raceval - 1, sign->slaying)) >= 0)
                    play_sound_player_only(CONTR(op), sid, raceval - 1, 0, 0);
            }
        }
    }
}


/* 'victim' moves onto 'trap' (trap has FLAG_WALK_ON or FLAG_FLY_ON set) or
 * 'victim' leaves 'trap' (trap has FLAG_WALK_OFF or FLAG_FLY_OFF) set.
 *
 * if victim is a player with gmaster_wizpass, don't trigger the 'trap'.
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
void move_apply(object_t *const trap_obj, object_t *const victim, object_t *const originator, const int flags)
{
    object_t *const trap = (trap_obj->head) ? trap_obj->head : trap_obj;
    player_t       *pl = NULL;
    static int    recursion_depth = 0;

    if (victim->type == PLAYER &&
        !(pl = CONTR(victim)))
    {
        return;
    }

    if (pl)
    {
        if (pl->gmaster_wizpass)
        {
            return;
        }
    }
    else
    {
        if (QUERY_FLAG(trap, FLAG_PLAYER_ONLY))
        {
            return;
        }
    }

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
          ApplyAltar(trap, victim, originator);
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

//        case FBULLET:
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
              object_t   *ab;

              if ((flags & MOVE_APPLY_VANISHED))
                  goto leave;

              if (!trap->weight_limit)
              {
                  sint32    tot;
                  for (ab = trap->above,tot = 0; ab != NULL; ab = ab->above)
                      if (!IS_AIRBORNE(ab))
                          tot += WEIGHT_OVERALL(ab);
                  if (!(trap->weight_limit = (tot > trap->weight) ? 1 : 0))
                      goto leave;
                  SET_ANIMATION(trap, (NUM_ANIMATIONS(trap) / NUM_FACINGS(trap)) * trap->direction + trap->weight_limit);
#ifndef USE_OLD_UPDATE
                  OBJECT_UPDATE_UPD(trap, UPD_ANIM);
#else
                  update_object(trap, UP_OBJ_FACE);
#endif
              }
              for (ab = trap->above, max = 100, sound_was_played = 0;
                   --max && ab && !IS_AIRBORNE(ab);
                   ab = ab->above)
              {
                  if (!sound_was_played)
                  {
                      play_sound_map(MSP_KNOWN(trap), SOUND_FALL_HOLE, SOUND_NORMAL);
                      sound_was_played = 1;
                  }
                  if (ab->type == PLAYER)
                      ndi(NDI_UNIQUE, 0, ab, "You fall into a trapdoor!");
                  (void)enter_map_by_exit(ab, trap);
              }
              goto leave;
          }


        case PIT:
          /* Pit not open? */
          if ((flags & MOVE_APPLY_VANISHED) || trap->stats.wc > 0)
              goto leave;
          play_sound_map(MSP_KNOWN(victim), SOUND_FALL_HOLE, SOUND_NORMAL);

          if(enter_map_by_exit(victim, trap))
              if (pl)
                  ndi(NDI_UNIQUE, 0, victim, "You fall through the hole!\n");

          goto leave;

        case EXIT:
          if (!(flags & MOVE_APPLY_VANISHED) &&
              pl)
          {
              (void)enter_map_by_exit(victim, trap);
          }

          goto leave;

        case SHOP_MAT:
          if (!(flags & MOVE_APPLY_VANISHED))
              ApplyShopMat(trap, victim);
          goto leave;

          /* Drop a certain amount of gold, and have one item identified */
        case IDENTIFY_ALTAR:
          if (!(flags & MOVE_APPLY_VANISHED))
              ApplyIdAltar(victim, trap, originator);
          goto leave;

        case SIGN:
          if (pl) /* only player should be able read signs */
              ApplySign(victim, trap);
          goto leave;

          /* FIXME: Huh? can containers be WALK_ON??? Gecko 2006-11-14 */
        case CONTAINER:
          if (pl)
          {
              if(!trigger_object_plugin_event(EVENT_TRIGGER, trap, victim, NULL,
                          NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING))
                  ApplyContainer(victim, trap);
          }
          goto leave;

        case RUNE:
          if (!(flags & MOVE_APPLY_VANISHED) && trap->level && IS_LIVE(victim))
              spring_trap(trap, victim);
          goto leave;

        case GRAVESTONE:
          if (!(flags & MOVE_APPLY_VANISHED) &&
              pl &&
              trap->level >= 1 &&
              trap->stats.dam >= 1)
          {
              int wis = victim->stats.Wis;

              if ((wis >= 30 &&
                   random_roll(0, 99)) ||
                  (wis <= 29 &&
                   wis >= 16 &&
                   random_roll(0, 99) < (wis + (wis - 16) * 5)))
              {
                  damage_ob(victim, trap->stats.dam, trap, ENV_ATTACK_CHECK);
                  ndi(NDI_UNIQUE, 0, victim, "Your passage disturbs holy ground and you are punished by the gods!");
              }
          }

          goto leave;

#if 0
//        /* The following missile types do not cause TRIGGER events */
//        case MMISSILE:
//          /* no need to hit anything */
//          if (IS_LIVE(victim) && !(flags & MOVE_APPLY_VANISHED))
//          {
//              tag_t     trap_tag    = trap->count;
//              damage_ob(victim, trap->stats.dam, trap, ENV_ATTACK_CHECK);
//              if (!was_destroyed(trap, trap_tag))
//                  remove_ob(trap);
//              check_walk_off(trap, NULL, MOVE_APPLY_VANISHED);
//          }
//          goto leave;
//        case CANCELLATION:
//        case BALL_LIGHTNING:
//          if (IS_LIVE(victim) && !(flags & MOVE_APPLY_VANISHED))
//              damage_ob(victim, trap->stats.dam, trap, ENV_ATTACK_CHECK);
//          goto leave;
//
//          /* we don't have this atm.
//          case DEEP_SWAMP:
//            if(!(flags&MOVE_APPLY_VANISHED))
//              walk_on_deep_swamp (trap, victim);
//            goto leave;
//          */
#endif

        default:
          LOG(llevMapbug, "MAPBUG:: %s[%s %d %d]: Type %d with fly/walk on/off not handled!\n",
              STRING_OBJ_NAME(trap), STRING_MAP_PATH(trap->map), trap->x,
              trap->y, trap->type);

          goto leave;
    }

    leave : recursion_depth--;
}


static void ApplyBook(object_t *op, object_t *tmp)
{
    sockbuf_struct *sptr;
    char    buf[HUGE_BUF];
    size_t  len;
    size_t  catlen;

    if (QUERY_FLAG(op, FLAG_BLIND))
    {
        ndi(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
        return;
    }

    /* you has the right skill & language knowledge to read it? */
    if (!change_skill(op, SK_LITERACY))
    {
        ndi(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
        return;
    }
    else if((op->chosen_skill->weight_limit & tmp->weight_limit)!=tmp->weight_limit)
    {
        ndi(NDI_UNIQUE, 0, op, "You are unable to decipher %s.\nIts written in %s.",
            query_name(tmp, op, ARTICLE_DEFINITE, 0), get_language(tmp->weight_limit));
        return;
    }

    ndi(NDI_UNIQUE, 0, op, "You open %s and start reading.",
        query_name(tmp, op, ARTICLE_DEFINITE, 0));

    /* Non-zero return value from script means stop here */
    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
        return;

    if (tmp->msg == NULL)
    {
        ndi(NDI_UNIQUE, 0, op, "You open %s and find it empty.",
            query_name(tmp, op, ARTICLE_DEFINITE, 0));
        return;
    }

    /* invoke the new client sided book interface */
    sprintf(buf,"<b t=\"%s%s%s\">", tmp->name?tmp->name:"Book",tmp->title?" ":"",tmp->title?tmp->title:"");
    len = strlen(buf);
    catlen = strlen(tmp->msg);
    if ((len + catlen) >= HUGE_BUF)
        catlen = HUGE_BUF - len - 1;
    strncat(buf, tmp->msg, catlen);
    buf[len + catlen] = '\0';
    len = strlen(buf);

    SOCKBUF_REQUEST_BUFFER(&CONTR(op)->socket, (len > SOCKET_SIZE_MEDIUM) ? SOCKET_SIZE_HUGE : SOCKET_SIZE_MEDIUM);
    sptr = ACTIVE_SOCKBUF(&CONTR(op)->socket);

    SockBuf_AddInt(sptr, tmp->weight_limit);
    SockBuf_AddString(sptr, buf, len);

    SOCKBUF_REQUEST_FINISH(&CONTR(op)->socket, SERVER_CMD_BOOK, SOCKBUF_DYNAMIC);
    /*ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "%s", tmp->msg);*/

    /* identify the book - successful reading will do it always */
    if (!QUERY_FLAG(tmp, FLAG_NO_SKILL_IDENT))
    {
        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            identify(tmp);
        }

        /*add_exp(op,exp_gain,op->chosen_skill->stats.sp, 1);*/
        SET_FLAG(tmp, FLAG_NO_SKILL_IDENT); /* so no more xp gained from this book */
    }
}

/* Special prayers are granted by gods and lost when the follower decides
 * to pray to a different gods.  'Force' objects keep track of which
 * prayers are special. */
static object_t *find_special_prayer_mark(object_t *op, int spell)
{
    object_t *tmp,
           *next;

    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        if (tmp->type == FORCE &&
            tmp->slaying &&
            tmp->slaying == shstr_cons.special_prayer &&
            tmp->stats.sp == spell)
        {
            return tmp;
        }
    }

    return NULL;
}

static void insert_special_prayer_mark(object_t *op, int spell)
{
    object_t *force   = get_archetype("force");
    force->speed = 0;
    update_ob_speed(force);
    FREE_AND_COPY_HASH(force->slaying, "special prayer");
    force->stats.sp = spell;
    insert_ob_in_ob(force, op);
}

extern void do_learn_spell(object_t *op, int spell, int special_prayer)
{
    object_t *tmp = find_special_prayer_mark(op, spell);

    if (op->type != PLAYER)
    {
        LOG(llevBug, "BUG: do_learn_spell(): not a player ->%s\n", op->name);
        return;
    }

    /* Upgrade special prayers to normal prayers */
    if (check_spell_known(op, spell))
    {
        ndi(NDI_UNIQUE, 0, op, "You already know the spell '%s'!", spells[spell].name);

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
    ndi(NDI_UNIQUE, 0, op, "You have learned the %s %s!",
                  (spells[spell].type == SPELL_TYPE_PRIEST) ? "prayer" :
                  "spell", spells[spell].name);
}

extern void do_forget_spell(object_t *op, int spell)
{
    object_t *tmp;
    int     i;

    if (op->type != PLAYER)
    {
        LOG(llevBug, "BUG: do_forget_spell(): not a player: %s (%d)\n", STRING_OBJ_NAME(op), spell);
        return;
    }
    if (!check_spell_known(op, spell))
    {
        LOG(llevBug, "BUG: do_forget_spell(): spell %d not known\n", spell);
        return;
    }

    play_sound_player_only(CONTR(op), SOUND_LOSE_SOME, SOUND_NORMAL, 0, 0);
    ndi(NDI_UNIQUE, 0, op, "You lose knowledge of %s.", spells[spell].name);

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

static void ApplySpellbook(object_t *op, object_t *tmp)
{
    if (QUERY_FLAG(op, FLAG_BLIND))
    {
        ndi(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
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
            ndi(NDI_UNIQUE, 0, op, "The book's formula for %s is incomplete", tmp->slaying);
            return;
        }
        /* now clear tmp->slaying since we no longer need it */
        FREE_AND_CLEAR_HASH2(tmp->slaying);
    }

    /* need a literacy skill to learn spells. Also, having a literacy level
     * lower than the spell will make learning the spell more difficult */
    if (!change_skill(op, SK_LITERACY))
    {
        ndi(NDI_UNIQUE, 0, op, "You can't read! Your attempt fails.");
        return;
    }
    if (tmp->stats.sp <0 || tmp->stats.sp> NROFREALSPELLS || spells[tmp->stats.sp].level > (SK_level(op) + 10))
    {
        ndi(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
        return;
    }

    if(trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    ndi(NDI_UNIQUE, 0, op, "The spellbook contains the spell %s (lvl %d).",
        spells[tmp->stats.sp].name,
        spells[tmp->stats.sp].level);

    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
    {
        identify(tmp);
    }

    if (check_spell_known(op, tmp->stats.sp) && (tmp->stats.Wis || find_special_prayer_mark(op, tmp->stats.sp) == NULL))
    {
        ndi(NDI_UNIQUE, 0, op, "You already know that spell.\n");
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
        ndi(NDI_UNIQUE, 0, op, "In your confused state you flub the wording of the text!");
        /* this needs to be a - number [garbled] */
        scroll_failure(op, 0 - random_roll(0, spells[tmp->stats.sp].level), spells[tmp->stats.sp].sp);
    }
    else if (QUERY_FLAG(tmp, FLAG_NO_DROP))
    {
        ndi(NDI_UNIQUE, 0, op, "You succeed in learning the spell!");
        do_learn_spell(op, tmp->stats.sp, 0);

        /* xp gain to literacy for spell learning */
        if (!QUERY_FLAG(tmp, FLAG_NO_DROP))
            add_exp(op, calc_skill_exp(op, tmp, 1.0f,-1, NULL), op->chosen_skill->stats.sp, 1);
    }
    else
    {
        play_sound_player_only(CONTR(op), SOUND_FUMBLE_SPELL, SOUND_NORMAL, 0, 0);
        ndi(NDI_UNIQUE, 0, op, "You fail to learn the spell.\n");
    }
    decrease_ob_nr(tmp, 1);
}

static void ApplyScroll(object_t *op, object_t *tmp)
{
    /*object_t *old_skill;*/
    int scroll_spell = tmp->stats.sp;

    if (QUERY_FLAG(op, FLAG_BLIND))
    {
        ndi(NDI_UNIQUE, 0, op, "You are unable to read while blind.");
        return;
    }

    if (scroll_spell < 0 || scroll_spell >= NROFREALSPELLS)
    {
        ndi(NDI_UNIQUE, 0, op, "The scroll just doesn't make sense!");
        return;
    }

    if (op->type == PLAYER)
    {
        /*old_skill = op->chosen_skill;*/
        /* players need a literacy skill to read stuff! */
        if (!change_skill(op, SK_LITERACY))
        {
            ndi(NDI_UNIQUE, 0, op, "You are unable to decipher the strange symbols.");
            /* op->chosen_skill=old_skill;*/
            return;
        }

        /* thats new: literacy for reading but a player need also the
             * right spellcasting spell. Reason: the exp goes then in that
             * skill. This makes scroll different from wands or potions.
             */
        if (!change_skill(op, (spells[scroll_spell].type == SPELL_TYPE_PRIEST ? SK_DIVINE_PRAYERS : SK_WIZARDRY_SPELLS)))
        {
            ndi(NDI_UNIQUE, 0, op, "You can read the scroll but you don't understand it.");
            /* op->chosen_skill=old_skill;*/
            return;
        }

    }

    if(trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED))
    {
        identify(tmp);
    }

    ndi(NDI_WHITE, 0, op, "The scroll of %s turns to dust.", spells[tmp->stats.sp].name);
    cast_spell(op, tmp, op->facing ? op->facing : 4, scroll_spell, 0, spellScroll, NULL);
    decrease_ob_nr(tmp, 1);
}

/* op opens treasure chest tmp */
static void ApplyTreasure(object_t *op, object_t *tmp)
{
    object_t                 *treas;
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
        play_sound_map(MSP_KNOWN(tmp), SOUND_OPEN_CONTAINER, SOUND_NORMAL);

    if (tmp->msg) /* msg like "the chest crumbles to dust" */
        ndi(NDI_UNIQUE, 0, op, "%s", tmp->msg);
    if (treas == NULL)
    {
        ndi(NDI_UNIQUE, 0, op, "The chest was empty.");
        decrease_ob_nr(tmp, 1);
        return;
    }
    do
    {
        remove_ob(treas);
        check_walk_off(treas, NULL, MOVE_APPLY_VANISHED);
        ndi(NDI_UNIQUE, 0, op, "You find %s in the chest.",
            QUERY_SHORT_NAME(treas, op));
        treas->x = op->x,treas->y = op->y;
//        /* Monsters can be trapped in treasure chests */
//        /* No they can't -- Smacky 20140518 */
//        if (treas->type == MONSTER)
//        {
//            int i   = find_free_spot(treas->arch, treas, op->map, treas->x, treas->y, 0, 0, SIZEOFFREE1 + 1);
//            if (i != -1)
//            {
//                treas->x += OVERLAY_X(i);
//                treas->y += OVERLAY_Y(i);
//            }
//            fix_monster(treas);
//        }
        treas = insert_ob_in_map(treas, op->map, op, 0);
        if (treas && treas->type == RUNE && treas->level && IS_LIVE(op))
            spring_trap(treas, op);

        if (!OBJECT_VALID(op, op_tag) || !OBJECT_VALID(tmp, tmp_tag))
            break;
    }
    while ((treas = tmp->inv) != NULL);

    if (OBJECT_VALID(tmp, tmp_tag) && tmp->inv == NULL)
        decrease_ob_nr(tmp, 1);

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

/* NOTE: For B4 we removed the old food system. Foods are now a regeneration
 * force over time, like you quaff a potion or cast a spell and the healing/reg
 * effects comes in ticks for x seconds. */
static void ApplyFood(object_t *op, object_t *tmp)
{
    object_t *force;

    if (trigger_object_plugin_event(EVENT_APPLY, tmp, op, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
    {
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
        LOG(llevBug, "ApplyFood: can't create force object!?\n");
        return;
    }

    force->type = TYPE_FOOD_FORCE;
    SET_FLAG(force, FLAG_IS_USED_UP); /* or it will auto destroyed with first tick */
    SET_FLAG(force, FLAG_NO_SAVE);
    force->stats.food += tmp->last_eat + 1; /* how long this force will stay */
    force->last_eat = tmp->last_eat + 1;    /* we need that to know the base time */
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

    ndi(NDI_UNIQUE| NDI_NAVY, 0, op, "You start consuming %s",
        QUERY_SHORT_NAME(tmp, op));

    decrease_ob_nr(tmp, 1);
}

static void ApplyPoison(object_t *op, object_t *tmp)
{
    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    if (op->type == PLAYER)
    {
        play_sound_player_only(CONTR(op), SOUND_DRINK_POISON, SOUND_NORMAL, 0, 0);
        ndi(NDI_UNIQUE, 0, op, "Yech!  That tasted poisonous!");
    }
    if (tmp->stats.dam)
    {
        LOG(llevDebug, "Trying to poison player/monster for %d hp\n", tmp->stats.hp);
        /* internal damage part will take care about our poison */
        damage_ob(op, tmp->stats.dam, tmp, ENV_ATTACK_CHECK);
    }
    op->stats.food -= op->stats.food / 4;
    decrease_ob_nr(tmp, 1);
}

static void ApplySavebed(player_t *pl, object_t *bed)
{
    if (trigger_object_plugin_event(EVENT_APPLY, bed, pl->ob, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
    {
        return;
    }

    MAP_SET_PLAYER_BED_INFO_CURRENT(pl);
    ndi(NDI_UNIQUE, 0, pl->ob, "In future you will respawn here.");
}

static void ApplyArmourImprover(object_t *op, object_t *tmp)
{
    object_t *armor;

    armor = find_marked_object(op);
    if (!armor)
    {
        ndi(NDI_UNIQUE, 0, op, "You need to mark an armor object.");
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
        ndi(NDI_UNIQUE, 0, op, "Your marked item is not armour!\n");
        return;
    }

    if(trigger_object_plugin_event(
                EVENT_APPLY, tmp, op, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return;

    ndi(NDI_UNIQUE, 0, op, "Applying armour enchantment.");
    improve_armour(op, tmp, armor);
}

/* Return value (or bits):
 *   0: player or monster can't apply objects of that type
 *   1: has been applied, or there was an error applying the object
 *   2: objects of that type can't be applied if not in inventory
 *   4: no fix_player() needed.
 *   8: apply action will NOT break sitting/resting (like eating food)
 *
 * who is the object that is causing object to be applied, what is the object
 * being applied.
 *
 * aflag is special (always apply/unapply) flags.  Nothing is done with
 * them in this function - they are passed to apply_equipment(). */
int manual_apply(object_t *who, object_t *what, int aflag)
{
    player_t *pl = NULL;
    int     r;

    what = (what->head) ? what->head : what;

    /* who is flying and applying what not in inventory. */
    if (!what->env &&
        (IS_AIRBORNE(who) &&
         (!IS_AIRBORNE(what) ||
          !QUERY_FLAG(what, FLAG_FLY_ON))))
    {
        ndi(NDI_UNIQUE, 0, who, "But you are floating high above the ground!");
        return 0;
    }

    if (who->type == PLAYER &&
        !(pl = CONTR(who)))
    {
        return 0;
    }

    if (pl)
    {
        int ego_mode;

        if (QUERY_FLAG(what, FLAG_UNPAID) &&
            !QUERY_FLAG(what, FLAG_APPLIED))
        {
            ndi(NDI_UNIQUE, 0, who, "You should pay for it first.");
            return 4;
        }
        else if ((ego_mode = check_ego_item(who, what)))
        {
            if (ego_mode == EGO_ITEM_BOUND_UNBOUND)
            {
                ndi(NDI_UNIQUE, 0, who, "This is an ego item!\nType \"/egobind\" for more info about applying it!");
            }
            else if (ego_mode == EGO_ITEM_BOUND_PLAYER)
            {
                ndi (NDI_UNIQUE, 0, who, "This is not your ego item!");
            }

            return 1;
        }
        else if (what->item_level)
        {
            int tmp_lev = (what->item_skill) ? pl->skillgroup_ptr[what->item_skill-1]->level : who->level;

            if (what->item_level > tmp_lev)
            {
                ndi(NDI_UNIQUE, 0, who, "The item level is too high for you to apply!");
                return 4;
            }
        }
    }
    else
    {
        if (what->type == EXIT ||
            what->type == GRAVESTONE ||
            what->type == SIGN ||
            what->type == BOOK ||
            what->type == SPELLBOOK ||
            what->type == TYPE_SKILL ||
            what->type == CLOSE_CON ||
            what->type == CONTAINER ||
            what->type == TREASURE ||
            what->type == SAVEBED ||
            what->type == CLOCK ||
            what->type == MENU ||
            what->type == LIGHTER ||
            what->type == ARMOUR_IMPROVER ||
            what->type == WEAPON_IMPROVER ||
            QUERY_FLAG(what, FLAG_PLAYER_ONLY) ||
            QUERY_FLAG(what, FLAG_UNPAID) ||
            check_ego_item(who, what) == EGO_ITEM_BOUND_PLAYER ||
            what->item_level > who->level)
        {
            return 0;
        }
    }

    /* Since we want to defer the event triggers until all tests have been
     * passed, but not until after any side effects, we must handle each
     * object type differently (yuck!) when it comes to apply events. */
    switch (what->type)
    {
        case HOLY_ALTAR:
        ndi(NDI_UNIQUE, 0, who, "You touch the %s.",
            QUERY_SHORT_NAME(what, who));

        if (change_skill(who, SK_DIVINE_PRAYERS))
        {
            if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
            {
                 r = 1;
                 break;
            }

            pray_at_altar(who, what);
        }
        else
        {
            ndi(NDI_UNIQUE, 0, who, "Nothing happens. It seems you miss the right skill.");
        }

        r = 4;
        break;

        case CF_HANDLE:
        if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
             r = 1;
             break;
        }

        ndi(NDI_UNIQUE, 0, who, "You turn the handle.");
        play_sound_map(MSP_KNOWN(who), SOUND_TURN_HANDLE, SOUND_NORMAL);
        what->weight_limit = what->weight_limit ? 0 : 1;
        SET_ANIMATION(what, ((NUM_ANIMATIONS(what) / NUM_FACINGS(what)) * what->direction) + what->weight_limit);
#ifndef USE_OLD_UPDATE
        OBJECT_UPDATE_UPD(what, UPD_ANIM);
#else
        update_object(what, UP_OBJ_FACE);
#endif
        signal_connection(what, who, who, what->map);
        r = 4;
        break;

        case TRIGGER:
        if(trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
             r = 1;
             break;
        }

        if (check_trigger(what, who, who))
        {
            ndi(NDI_UNIQUE, 0, who, "You turn the handle.");
            play_sound_map(MSP_KNOWN(what), SOUND_TURN_HANDLE, SOUND_NORMAL);
        }
        else
        {
            ndi(NDI_UNIQUE, 0, who, "The handle doesn't move.");
        }

        r = 4;
        break;

        case EXIT:
        if (QUERY_FLAG(who, FLAG_PARALYZED))
        {
            ndi(NDI_UNIQUE, 0, who, "You try to use the %s, but are unable to move your legs.", what->name);
            r = 1;
            break;
        }
        else if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
            r = 1;
            break;
        }

        (void)enter_map_by_exit(who, what);
        r = 4;
        break;

        case GRAVESTONE:
        ApplySign(who, what); /* probably should have apply_gravestone() */
        r = 4;
        break;

        case SIGN:
        ApplySign(who, what);
        r = 4;
        break;

        case BOOK:
        ApplyBook(who, what);
        r = 4;
        break;

        case SPELLBOOK:
        ApplySpellbook(who, what);
        r = 1;
        break;

        case SCROLL:
        ApplyScroll(who, what);
        r = 1;
        break;

        case POTION:
        if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
             r = 1; /* 1 = do not write an error message to the player */
             break;
        }

        ApplyPotion(who, what);
        r = (1+8);
        break;

        case TYPE_LIGHT_APPLY:
        /* Lights cannot be applied unless directly in a player/monster inv
         * or on a map. */
        if (what->env &&
            what->env->type != PLAYER &&
            what->env->type != MONSTER)
        {
             r = 2;
             break;
        }

        apply_light(who, what);
        r = 1;
        break;

        case TYPE_LIGHT_REFILL:
        ApplyLightRefill(who, what);
        r = 1;
        break;

          /* Eneq(@csd.uu.se): Handle apply on containers. */
        case CLOSE_CON:
        if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
            r = 1;
            break;
        }

        ApplyContainer(who, what->env);
        r = 4;
        break;

        case CONTAINER:
        if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
            r = 1;
            break;
        }

        ApplyContainer(who, what);
        r = 4;
        break;

        case TREASURE:
        ApplyTreasure(who, what);
        r = 1;
        break;

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
        case BOW:
        case ARROW:
        case TYPE_SKILL:
        if (what->env != who)
        {
            r = 2;
            break;
        }

        apply_equipment(who, what, aflag);
        r = 1;
        break;

        case DRINK:
        case FOOD:
        ApplyFood(who, what);
        r = (1+8);
        break;

        case POISON:
        ApplyPoison(who, what);
        r = (1+8);
        break;

        case SAVEBED:
        ApplySavebed(pl, what);
        r = 1;
        break;

        case ARMOUR_IMPROVER:
        ApplyArmourImprover(who, what);
        r = 1;
        break;

        case WEAPON_IMPROVER:
        (void)check_improve_weapon(who, what);
        r = 1;
        break;

        case CLOCK:
        if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
            r = 1;
            break;
        }

        (void)command_time(who, "showtime");
        r = 4;
        break;

        case MENU:
        if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, &aflag, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
            r = 1;
            break;
        }

        r = 4;
        break;

        case POWER_CRYSTAL:
        /* TODO: plugin events */
        ApplyPowerCrystal(who, what);  /*  see egoitem.c */
        r = 1;
        break;

        case LIGHTER:
        /* TODO: plugin events */
        ApplyLighter(who, what);
        r = 1;
        break;

        default: /* Now we can put scripts even on NON-applyable items */
        if (trigger_object_plugin_event(EVENT_APPLY, what, who, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
           r = 1;
           break;
        }

        r = 0;
        break;
    }

    if (pl)
    {
        /* we have applied something which makes us standing up */
        if(pl->rest_sitting &&
           !(r & 8))
        {
            /* But anyone can read when sitting... */
            if (what->type != BOOK)
            {
                pl->rest_sitting = pl->rest_mode = 0;
            }
        }

        if (r & 1)
        {
            FIX_PLAYER(who ,"player apply ");
        }

        if (!(aflag & AP_QUIET))
        {
            if (r == 0)
            {
                ndi(NDI_UNIQUE, 0, who, "You don't know how to apply %s.",
                    QUERY_SHORT_NAME(what, who));
            }
            else if (r == 2)
            {
                ndi(NDI_UNIQUE, 0, who, "You must get it first!");
            }
        }
    }

    return r;
}

/* who is the object using the object.
 * what is the object they are using.
 * aflags is special flags (0 - normal/toggle, AP_APPLY=always apply,
 * AP_UNAPPLY=always unapply).
 *
 * Optional flags:
 *   AP_IGNORE_CURSE: unapply cursed items
 *
 * Usage example:  apply_equipment (who, what, AP_UNAPPLY | AP_IGNORE_CURSE)
 *
 * apply_equipment() doesn't check for unpaid items.
 */
int apply_equipment(object_t *who, object_t *what, int aflags)
{
    player_t *pl = NULL;
    int     ego_mode,
            basic_flag  = (aflags & AP_BASIC_FLAGS);
    int     tmp_flag    = 0;
    object_t *tmp,
           *next;
    char    buf[HUGE_BUF];
    int     i;

    if (!who)
    {
        LOG(llevBug, "BUG:: %s:apply_equipment(): who is NULL!\n", __FILE__);
        return 1;
    }

    if (who->type == PLAYER &&
        !(pl = CONTR(who)))
    {
        return 1;
    }

    if (what->env != who)
    {
        return 1;
    }

    /* lets check we have an ego item */
    if((ego_mode = check_ego_item(what, who)))
    {
        if (pl)
        {
            if (ego_mode == EGO_ITEM_BOUND_UNBOUND)
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, what, "This is an ego item!\nType \"/egobind\" for more info about applying it!");
                }
            }
            else if (ego_mode == EGO_ITEM_BOUND_PLAYER)
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, what, "This is not your ego item!");
                }
            }
        }
        return 1;
    }

    buf[0] = '\0';      /* Needs to be initialized */
    if (QUERY_FLAG(what, FLAG_APPLIED))
    {
        uint16 flags = UPD_FLAGS;

        /* always apply, so no reason to unapply */
        if (basic_flag == AP_APPLY)
            return 0;
        if (what->item_condition && !(aflags & AP_IGNORE_CURSE) && (QUERY_FLAG(what, FLAG_CURSED) || QUERY_FLAG(what, FLAG_DAMNED)))
        {
            if (!(aflags & AP_QUIET))
            {
                ndi(NDI_UNIQUE, 0, who, "No matter how hard you try, you just can't remove it!");
            }
            return 1;
        }

        if (QUERY_FLAG(what, FLAG_PERM_CURSED))
            SET_FLAG(what, FLAG_CURSED);
        if (QUERY_FLAG(what, FLAG_PERM_DAMNED))
            SET_FLAG(what, FLAG_DAMNED);

        /* This is actually an (UN)APPLY event. Scripters should check
         * the applied flag */
        if(trigger_object_plugin_event(
                    EVENT_APPLY, what, who, NULL,
                    NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
            return 1;

        CLEAR_FLAG(what, FLAG_APPLIED);
        switch (what->type)
        {
            case WEAPON:
                change_abil(who, what);
                CLEAR_FLAG(who, FLAG_READY_WEAPON);
                if(!what->item_condition)
                {
                    if (!(aflags & AP_QUIET))
                    {
                        sprintf(buf, "%s is broken!", query_name(what, who, ARTICLE_DEFINITE, 0));
                    }
                }
                else
                {
                    if (!(aflags & AP_QUIET))
                    {
                        sprintf(buf, "You no longer wield %s.", query_name(what, who, ARTICLE_DEFINITE, 0));
                    }
                }
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
            case CLOAK:
                change_abil(who, what);

                if(!what->item_condition)
                {
                    if (!(aflags & AP_QUIET))
                    {
                        sprintf(buf, "%s is broken!", query_name(what, who, ARTICLE_DEFINITE, 0));
                    }
                }
                else
                {
                    if (!(aflags & AP_QUIET))
                    {
                        sprintf(buf, "You take off %s.", query_name(what, who, ARTICLE_DEFINITE, 0));
                    }
                }
                break;

            case ARROW:
            case BOW:
            case WAND:
            case ROD:
            case HORN:
                if(!what->item_condition)
                {
                    if (!(aflags & AP_QUIET))
                    {
                        sprintf(buf, "%s is broken!", query_name(what, who, ARTICLE_DEFINITE, 0));
                    }
                }
                else
                {
                    if (!(aflags & AP_QUIET))
                    {
                        sprintf(buf, "You unready %s.", query_name(what, who, ARTICLE_DEFINITE, 0));
                    }
                }
                if(what->type != ARROW || what->sub_type1 > 127)
                {
                    if (!pl)
                    {
                        CLEAR_FLAG(who, FLAG_READY_BOW);
                        break;
                    }
                }
                break;
            case TYPE_SKILL:
                who->chosen_skill = NULL;
                buf[0] = '\0';
                break;

            default:
                if (!(aflags & AP_QUIET))
                {
                    sprintf(buf, "You unapply %s.", query_name(what, who, ARTICLE_DEFINITE, 0));
                }
                break;
        }
        if (buf[0] != '\0') /* urgh... what use of buf */
        {
            if (pl)
            {
                ndi(NDI_UNIQUE, 0, who, "%s", buf);
                FIX_PLAYER(who ,"apply special ");
            }
            else
                fix_monster(who);
        }

        if (merge_ob(what, NULL))
        {
            flags |= UPD_NROF | UPD_WEIGHT;
        }

#ifndef USE_OLD_UPDATE
        OBJECT_UPDATE_UPD(what, flags);
#else
        esrv_update_item(flags, what);
#endif
        return 0;
    }
    if (basic_flag == AP_UNAPPLY)
        return 0;
    i = 0;

    /* This goes through and checks to see if the player already has something
     * of that type applied - if so, unapply it.
     * This is a VERY important part -it ensures
     */
    if (what->type == WAND || what->type == ROD || what->type == HORN || what->type == BOW || (what->type == ARROW && what->sub_type1 >127))
        tmp_flag = 1;

    FOREACH_OBJECT_IN_OBJECT(tmp, who, next)
    {
        if ((tmp->type == what->type || (tmp_flag && (tmp->type == WAND || tmp->type == ROD || tmp->type == HORN || tmp->type == BOW || (tmp->type == ARROW && tmp->sub_type1 >127))))
                && QUERY_FLAG(tmp, FLAG_APPLIED) && tmp != what)
        {
            if (tmp->type == RING && !i)
                i = 1;
            else if (apply_equipment(who, tmp, 0))
                return 1;
        }
    }

    /* For clarity and ease of event handling I split this into
     * two parts, first a check and then the modifications */
    switch (what->type)
    {
        case RING:
        case AMULET:
        case BOW:
        case ARROW:
            if(!what->item_condition)
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "%s is broken and can't be applied.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }
                return 1;
            }
            break;
        case WEAPON:
            if(!what->item_condition)
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "%s is broken and can't be applied.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }
                return 1;
            }
            if (!QUERY_FLAG(who, FLAG_USE_WEAPON))
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "You can't use %s.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }
                return 1;
            }
            if (!check_weapon_power(who, what->last_eat))
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "That weapon is too powerful for you to use. It would consume your soul!.");
                }
                return 1;
            }
            if (what->level && (strncmp(what->name, who->name, strlen(who->name))))
            {
                /* if the weapon does not have the name as the character, can't use it. */
                /*        (Ragnarok's sword attempted to be used by Foo: won't work) */
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "The weapon does not recognize you as its owner.");
                }
                return 1;
            }

            /* if we have applied a shield, don't allow apply of polearm or 2hand weapon */
            if (pl &&
                pl->equipment[PLAYER_EQUIP_SHIELD] &&
                (what->sub_type1 >= WEAP_POLE_IMPACT ||
                 what->sub_type1 >= WEAP_2H_IMPACT))
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "You can't wield this weapon and a shield.");
                }
                return 1;
            }

            if (!check_skill_to_apply(who, what))
            {
                return 1;
            }
            break;
        case SHIELD:
            /* don't allow of polearm or 2hand weapon with a shield */
            if(!what->item_condition)
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "%s is broken and can't be applied.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }
                return 1;
            }
            if (pl &&
                pl->equipment[PLAYER_EQUIP_WEAPON1] &&
                (pl->equipment[PLAYER_EQUIP_WEAPON1]->sub_type1 >= WEAP_POLE_IMPACT ||
                 pl->equipment[PLAYER_EQUIP_WEAPON1]->sub_type1 >= WEAP_2H_IMPACT))
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "You can't wield this shield and a weapon.");
                }
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
            if(!what->item_condition)
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "%s is broken and can't be applied.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }
                return 1;
            }
            if (!QUERY_FLAG(who, FLAG_USE_ARMOUR))
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "You can't use %s.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }
                return 1;
            }
            break;

            /* this part is needed for skill-tools */
        case TYPE_SKILL:
            if (who->chosen_skill)
            {
                LOG(llevBug, "BUG: apply_equipment(): can't apply two skills\n");
                return 1;
            }
            break;
    }

    /* Now we should be done with 99% of all tests. Generate the event
     * and then go on with side effects */
    if(trigger_object_plugin_event(
                EVENT_APPLY, what, who, NULL,
                NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        return 1; /* 1 = do not write an error message to the player */

    if (what->nrof > 1 && what->type != ARROW)
        tmp = get_split_ob(what, what->nrof - 1);
    else
        tmp = NULL;

    switch (what->type)
    {
        case WEAPON:
            SET_FLAG(what, FLAG_APPLIED);
            SET_FLAG(who, FLAG_READY_WEAPON);
            change_abil(who, what);
            if (!(aflags & AP_QUIET))
            {
                ndi(NDI_UNIQUE, 0, who, "You wield %s.",
                    query_name(what, who, ARTICLE_DEFINITE, 0));
            }
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
            SET_FLAG(what, FLAG_APPLIED);
            change_abil(who, what);
            if (!(aflags & AP_QUIET))
            {
                ndi(NDI_UNIQUE, 0, who, "You put on %s.",
                    query_name(what, who, ARTICLE_DEFINITE, 0));
            }
            break;

        case ARROW:
            /* an arrow can be a.) a throw item (= distance weapon) or b.) ammunition for a distance weapon
            * as throw item we handle it like a bow/wand/etc...
            * as normal arrow special - we test first we have applied the right distance weapon for it
            */
            if(what->sub_type1 < 127) /* its amunition */
            {
                /* we want apply amun. Lets only allow to apply amun fitting the applied bow! */
                if (pl &&
                    (!pl->equipment[PLAYER_EQUIP_BOW] ||
                     pl->equipment[PLAYER_EQUIP_BOW]->type != BOW ||
                     pl->equipment[PLAYER_EQUIP_BOW]->sub_type1 != what->sub_type1))
                {
                    if (!(aflags & AP_QUIET))
                    {
                        ndi(NDI_UNIQUE, 0, who, "You can't use %s with applied range weapon!",
                            QUERY_SHORT_NAME(what, NULL));
                    }
                    return 1;
                }
            }
        case WAND:
        case ROD:
        case HORN:
        case BOW:
            if (!check_skill_to_apply(who, what))
                return 1;

            if((what->type == ROD || what->type == HORN) && who->chosen_skill->level < what->level)
            {
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "Your ~%s~ skill level is to low!",
                        STRING_OBJ_NAME(who->chosen_skill));
                }
                return 1;
            }

            SET_FLAG(what, FLAG_APPLIED);

            if (!(aflags & AP_QUIET))
            {
                ndi(NDI_UNIQUE, 0, who, "You ready %s.",
                    query_name(what, who, ARTICLE_DEFINITE, 0));
            }

            if (pl)
            {
                if (what->type != BOW)
                {
                    pl->known_spell = (QUERY_FLAG(what, FLAG_BEEN_APPLIED) || QUERY_FLAG(what, FLAG_IDENTIFIED));
                }
            }
            else
            {
                SET_FLAG(who, FLAG_READY_BOW); break;
            }
            break;

        case TYPE_SKILL:
            if (pl)
            {
                /* At least one of these lines is unnecessary: confirmation or
                 * just spam? I vote to get rid of the ndi() -- srs() uses less
                 * resources and allows the client more control. */
                if (!(aflags & AP_QUIET))
                {
                    ndi(NDI_UNIQUE, 0, who, "You ready the skill ~%s~.",
                        STRING_OBJ_NAME(what));
                }

                send_ready_skill(pl, what->name);
            }

            SET_FLAG(what, FLAG_APPLIED);
            /* change_abil(who, what); */
            /*LOG(llevDebug, "APPLY SKILL: %s change %s to %s\n", STRING_OBJ_NAME(who), STRING_OBJ_NAME(who->chosen_skill), STRING_OBJ_NAME(what) );*/
            who->chosen_skill = what;
            break;

        default:
            if (!(aflags & AP_QUIET))
            {
                ndi(NDI_UNIQUE, 0, who, "You apply %s.",
                    query_name(what, who, ARTICLE_DEFINITE, 0));
            }
    }
    if (!QUERY_FLAG(what, FLAG_APPLIED))
        SET_FLAG(what, FLAG_APPLIED);
    if (tmp != NULL)
        tmp = insert_ob_in_ob(tmp, who);
    FIX_PLAYER(who ,"apply special - you apply ");
    if (what->type != WAND && pl)
        SET_FLAG(what, FLAG_BEEN_APPLIED);

    if (QUERY_FLAG(what, FLAG_PERM_CURSED))
        SET_FLAG(what, FLAG_CURSED);
    if (QUERY_FLAG(what, FLAG_PERM_DAMNED))
        SET_FLAG(what, FLAG_DAMNED);

    if (QUERY_FLAG(what, FLAG_CURSED) || QUERY_FLAG(what, FLAG_DAMNED))
    {
        if (pl)
        {
            if (!(aflags & AP_QUIET))
            {
                ndi(NDI_UNIQUE, 0, who, "Oops, it feels deadly cold!");
            }
            SET_FLAG(what, FLAG_KNOWN_CURSED);
        }
    }

#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(what, UPD_FLAGS);
#else
    esrv_update_item(UPD_FLAGS, what);
#endif

    return 0;
}

/* Refill lamps and all refill type light sources
 * The light source must be in the inventory of the player, then he must mark the
 * light source and apply the refill item (lamp oil for example). */
static void ApplyLightRefill(object_t *who, object_t *op)
{
    object_t *item;
    int     tmp;

    item = find_marked_object(who);
    if (!item)
    {
        ndi(NDI_UNIQUE, 0, who, "Mark a light source first you want refill.");
        return;
    }


    if (item->type != TYPE_LIGHT_APPLY ||
        !item->race ||
        item->race != op->race)
    {
        ndi(NDI_UNIQUE, 0, who, "You can't refill %s with %s.",
            query_name(item, who, ARTICLE_DEFINITE, 0), QUERY_SHORT_NAME(op, who));
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
        ndi(NDI_UNIQUE, 0, who, "%s is full and can't be refilled.",
            query_name(item, who, ARTICLE_DEFINITE, 0));
        return;
    }

    if (op->stats.food <= tmp)
    {
        item->stats.food += op->stats.food;
        ndi(NDI_UNIQUE, 0, who, "You refill %s with %d units from %s.",
            query_name(item, who, ARTICLE_DEFINITE, 0), op->stats.food, QUERY_SHORT_NAME(op, who));
        decrease_ob_nr(op, 1);
    }
    else
    {
        object_t *filler;
        if (op->nrof > 1)
        {
            filler = get_split_ob(op, 1);
            filler->stats.food -= tmp;
            insert_ob_in_ob(filler, who);
        }
        else
        {
            filler = op;
            filler->stats.food -= tmp;
        }

        item->stats.food += tmp;
        ndi(NDI_UNIQUE, 0, who, "You refill %s with %d units from %s.",
            query_name(item, who, ARTICLE_DEFINITE, 0), tmp, QUERY_SHORT_NAME(filler, who));
    }

    FIX_PLAYER(who ,"apply light refill");
}

void turn_on_light(object_t *op)
{
    msp_t *msp = (op->map) ? MSP_KNOWN(op) : NULL;
    object_t *op_old;
    int     tricky_flag = FALSE; /* to delay insertion of object - or it simple remerge! */

    /* simple case for map light sources */
    if(op->type == LIGHT_SOURCE)
    {
        op->glow_radius = (sint8)op->last_sp;

        if (msp &&
            op->glow_radius)
        {
            adjust_light_source(msp, op->glow_radius);
        }

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
        object_t *one = get_object();
        copy_object(op, one);
        op->nrof -= 1;
        one->nrof = 1;
#ifndef USE_OLD_UPDATE
        OBJECT_UPDATE_UPD(op, UPD_NROF);
#else
        if (op->env)
            esrv_update_item(UPD_NROF, op);
        else
            update_object(op, UP_OBJ_FACE);
#endif

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
            }
        }

        op->glow_radius = (sint8)op->last_sp;

        if (msp &&
            op->glow_radius)
        {
            adjust_light_source(msp, op->glow_radius);
        }
#ifndef USE_OLD_UPDATE
#else

        update_object(op, UP_OBJ_FACE);
#endif
    }

#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(op, UPD_FACE | UPD_ANIM);
#else
    esrv_update_item(UPD_FACE | UPD_ANIM, op);
#endif
}

void turn_off_light(object_t *op)
{
    msp_t *msp = (op->map) ? MSP_KNOWN(op) : NULL;

    if (msp &&
        op->glow_radius)
    {
        adjust_light_source(msp, -(op->glow_radius));
    }

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
    op->glow_radius = 0;
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(op, UPD_FLAGS | UPD_FACE | UPD_ANIM);
#else
    update_object(op, UP_OBJ_FACE);
    esrv_update_item(UPD_FLAGS | UPD_FACE | UPD_ANIM, op);
#endif
}

/* Note that op->msg is used for both non-applyable applyable lights (ie, to
 * respond to apply attempts) and for successfully applied applyable lights,
 * and is used both for light and extinguish attempts. Therefore, care should
 * be taken to change the msg if ever no_fix_player is changed (which can only
 * be done in the map file or via a script anyway) and the on/off status cannot
 * be mentioned in the (static) msg -- Smacky 20080905 */
void apply_light(object_t *who, object_t *op)
{
    object_t *tmp;

    /* Lights with no_fix_player 1 cannot be lit/extinguished by applying them
     * -- to prevent players buggering about with map design and puzzles. */
    if (QUERY_FLAG(op, FLAG_NO_FIX_PLAYER)) // FLAG_NO_APPLY would be better but there is no arch attribute
    {
        if (!(QUERY_FLAG(op, FLAG_NO_PICK)))
            LOG(llevBug, "BUG:: %s:apply_light(): Pickable applyable light source flagged as no_apply!\n",
                         __FILE__);

        if (op->msg)
            ndi(NDI_UNIQUE, 0, who, "%s", op->msg);
        else
            ndi(NDI_UNIQUE, 0, who, "You cannot %s %s.",
                (!op->glow_radius) ? "light" : "extinguish",
                query_name(op, who, ARTICLE_DEFINITE, 0));

        return;
    }

    if (QUERY_FLAG(op, FLAG_APPLIED))
    {
        if (QUERY_FLAG(op, FLAG_CURSED) || QUERY_FLAG(op, FLAG_DAMNED))
        {
            ndi(NDI_UNIQUE, 0, who, "No matter how hard you try, you just can't remove it!");
            return;
        }
        if (QUERY_FLAG(op, FLAG_PERM_CURSED))
            SET_FLAG(op, FLAG_CURSED);
        if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
            SET_FLAG(op, FLAG_DAMNED);

        if (trigger_object_plugin_event(EVENT_APPLY, op, who, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
        {
#ifndef USE_OLD_UPDATE
            OBJECT_UPDATE_UPD(who, UPD_FLAGS);
#endif
            return;
        }

        if (op->msg)
            ndi(NDI_UNIQUE, 0, who, "%s", op->msg);
        else
            ndi(NDI_UNIQUE, 0, who, "You extinguish %s.",
                query_name(op, who, ARTICLE_DEFINITE, 0));

        turn_off_light(op);
#ifndef USE_OLD_UPDATE
#else
        update_object(who, UP_OBJ_FACE);
#endif
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
            ndi(NDI_UNIQUE, 0, who, "%s can't be lit.",
                query_name(op, who, ARTICLE_DEFINITE, 0));
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
                    ndi(NDI_UNIQUE, 0, who, "You must first refill or recharge %s.",
                        query_name(op, who, ARTICLE_DEFINITE, 0));
                    return;
                }
            }

            if(trigger_object_plugin_event(EVENT_APPLY, op, who, NULL,
                        NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                return;

            if (op->env && op->env->type == PLAYER)
            {
                ndi(NDI_UNIQUE, 0, who, "You prepare %s to be your light source.",
                    query_name(op, who, ARTICLE_DEFINITE, 0));
                turn_on_light(op);
                FIX_PLAYER(who ,"apply light - turn on light");
            }
            else
            {
                if (op->msg)
                    ndi(NDI_UNIQUE, 0, who, "%s", op->msg);
                else
                    ndi(NDI_UNIQUE, 0, who, "You light %s.",
                        query_name(op, who, ARTICLE_DEFINITE, 0));

                turn_on_light(op);
            }
        }
        else
        {
            if (op->env && op->env->type == PLAYER)
            {
                object_t *next;

                /* remove any other applied light source first */
                FOREACH_OBJECT_IN_OBJECT(tmp, who, next)
                {
                    if (tmp->type == op->type && QUERY_FLAG(tmp, FLAG_APPLIED) && tmp != op)
                    {
                        if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
                        {
                            ndi(NDI_UNIQUE, 0, who,
                                    "No matter how hard you try, you just can't remove it!");
                            return;
                        }
                        if (QUERY_FLAG(tmp, FLAG_PERM_CURSED))
                            SET_FLAG(tmp, FLAG_CURSED);
                        if (QUERY_FLAG(tmp, FLAG_PERM_DAMNED))
                            SET_FLAG(tmp, FLAG_DAMNED);

                        if (trigger_object_plugin_event(EVENT_APPLY, op, who, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                        {
#ifndef USE_OLD_UPDATE
                            OBJECT_UPDATE_UPD(who, UPD_FLAGS);
#endif
                            return;
                        }

                        if (tmp->msg)
                            ndi(NDI_UNIQUE, 0, who, "%s", tmp->msg);
                        else
                            ndi(NDI_UNIQUE, 0, who, "You extinguish %s.",
                                query_name(tmp, who, ARTICLE_DEFINITE, 0));

                        CLEAR_FLAG(tmp, FLAG_APPLIED);
                        turn_off_light(tmp);
#ifndef USE_OLD_UPDATE
#else
                        esrv_update_item(UPD_FLAGS | UPD_FACE, tmp);
#endif
                    }
                }

                if (op->msg)
                    ndi(NDI_UNIQUE, 0, who, "%s", op->msg);
                else
                    ndi(NDI_UNIQUE, 0, who, "You apply %s as your light source.",
                        query_name(op, who, ARTICLE_DEFINITE, 0));

                SET_FLAG(op, FLAG_APPLIED);
                FIX_PLAYER(who ," apply light - apply light");
#ifndef USE_OLD_UPDATE
                OBJECT_UPDATE_UPD(op, UPD_FLAGS);
                OBJECT_UPDATE_UPD(who, UPD_FACE);
#else
                esrv_update_item(UPD_FLAGS, op);
                update_object(who, UP_OBJ_FACE);
#endif
            }
            else /* not part of player inv - turn light off ! */
            {
                if (QUERY_FLAG(op, FLAG_PERM_CURSED))
                    SET_FLAG(op, FLAG_CURSED);
                if (QUERY_FLAG(op, FLAG_PERM_DAMNED))
                    SET_FLAG(op, FLAG_DAMNED);

                if (trigger_object_plugin_event(EVENT_APPLY, op, who, NULL, NULL, NULL, NULL, NULL, SCRIPT_FIX_ACTIVATOR))
                {
#ifndef USE_OLD_UPDATE
                    OBJECT_UPDATE_UPD(who, UPD_FLAGS);
#endif
                    return;
                }

                if (op->msg)
                    ndi(NDI_UNIQUE, 0, who, "%s", op->msg);
                else
                    ndi(NDI_UNIQUE, 0, who, "You extinguish %s.",
                        query_name(op, who, ARTICLE_DEFINITE, 0));

                turn_off_light(op);
            }
        }
    }
}

/* ApplyLighter() - designed primarily to light torches/lanterns/etc.
 * Also burns up burnable material too. First object in the inventory is
 * the selected object to "burn". -b.t.
 */
/* i have this item type not include in daimonin atm - MT-2004 */
/* Currently doesn not generate APPLY events - Gecko 2005-05-15 */
static void ApplyLighter(object_t *who, object_t *lighter)
{
    object_t *item;
    tag_t   count;
    uint32  nrof;
    int     is_player_env   = 0;
    char    item_name[MEDIUM_BUF];

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
                object_t *oneLighter  = get_object();
                copy_object(lighter, oneLighter);
                lighter->nrof -= 1;
                oneLighter->nrof = 1;
                oneLighter->stats.food--;
                oneLighter = insert_ob_in_ob(oneLighter, who);
            }
            else
            {
                lighter->stats.food--;
            }
        }
        else if (lighter->last_eat)
        {
            /* no charges left in lighter */
            ndi(NDI_UNIQUE, 0, who, "You attempt to light the %s with a used up %s.", item->name,
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
            ndi(NDI_UNIQUE, 0, who, "You light the %s with the %s.", item_name, lighter->name);
            if (is_player_env)
                FIX_PLAYER(who ,"apply lighter ");
        }
        else
        {
            ndi(NDI_UNIQUE, 0, who, "You attempt to light the %s with the %s and fail.", item->name,
                                 lighter->name);
        }
    }
    else /* nothing to light */
        ndi(NDI_UNIQUE, 0, who, "You need to mark a lightable object.");
}

/* scroll_failure()- hacked directly from spell_failure */

void scroll_failure(object_t *op, int failure, int power)
{
    if (abs(failure / 4) > power)
        power = abs(failure / 4); /* set minimum effect */

    if (failure <= -1 && failure > -15) /* wonder */
    {
        ndi(NDI_UNIQUE, 0, op, "Your spell warps!.");
        cast_cone(op, op, 0, 10, SP_WOW, spellarch[SP_WOW], SK_level(op), 0);
    }
    else if (failure <= -15 && failure > -35) /* drain mana */
    {
        ndi(NDI_UNIQUE, 0, op, "Your mana is drained!.");
        op->stats.sp -= random_roll(0, power - 1);
        if (op->stats.sp < 0)
            op->stats.sp = 0;
    }

    /* even nastier effects continue...*/
#ifdef SPELL_FAILURE_EFFECTS /* removed this - but perhaps we want add some of this nasty effects */
    else if (failure <= -35 && failure > -60) /* confusion */
    {
        ndi(NDI_UNIQUE, 0, op, "The magic recoils on you!");
        confuse_player(op, op, power);
    }
    else if (failure <= -60 && failure > -70) /* paralysis */
    {
        ndi(NDI_UNIQUE, 0, op, "The magic recoils and paralyzes you!");
        paralyze_player(op, op, power);
    }
    else if (failure <= -70 && failure > -80) /* blind */
    {
        ndi(NDI_UNIQUE, 0, op, "The magic recoils on you!");
        blind_player(op, op, power);
    }
    else if (failure <= -80) /* blast the immediate area */
    {
        ndi(NDI_UNIQUE, 0, op, "You unlease uncontrolled mana!");
        cast_mana_storm(op, power);
    }
#endif
}

/*  peterm:  do_power_crystal

object_t *op, object_t *crystal

This function handles the application of power crystals.
Power crystals, when applied, either suck power from the applier,
if he's at full spellpoints, or gives him power, if it's got
spellpoins stored. */
static void ApplyPowerCrystal(object_t *op, object_t *crystal)
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
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(crystal, UPD_ANIMSPEED);
#else
    esrv_update_item(UPD_ANIMSPEED, crystal);
#endif
}
