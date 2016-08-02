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

/* TODO: Reference only. Will be removed. */
/* Note that somewhat stripped down versions of the essentials are at the end.
 * What has been stripped out has often been removed just to keep things a bit
 * simpler for now. But also it potentially added some flavour to different
 * gods so should be looked at again when we actually have different gods.
 * Also, there may be some overlap with the guilds system.
 *
 * -- Smacky 20140519 */
#if 0
///* Oct 3, 1995 - Code laid down for initial gods, priest alignment, and
// * monster race initialization. b.t.
// */
//
///* Sept 1996 - moved code over to object -oriented gods -b.t. */
//
//#include <global.h>
//
///* define this if you want to allow gods to assign more gifts
// * and limitations to priests */
//#define MORE_PRIEST_GIFTS
//
//static int  god_gives_present(object *op, object *god, treasure *tr);
//static void follower_remove_similar_item(object *op, object *item);
//
//// TODO: look over usage of those two functions and see if we can get
//// away without the strncmp/strcmp calls
//int lookup_god_by_name(const char *name)
//{
//    int godnr = -1, nmlen = strlen(name);
//
//    if (name && name != shstr_cons.none)
//    {
//        godlink    *gl;
//        for (gl = first_god; gl; gl = gl->next)
//            if (!strncmp(name, gl->name, MIN((int) strlen(gl->name), nmlen)))
//                break;
//        if (gl)
//            godnr = gl->id;
//    }
//    return godnr;
//}
//
//object * find_god(const char *name)
//{
//    object *god = NULL;
//
//    if (name && name != shstr_cons.none)
//    {
//        godlink    *gl;
//        for (gl = first_god; gl; gl = gl->next)
//            if (!strcmp(name, gl->name))
//                break;
//        if (gl)
//            god = pntr_to_god_obj(gl);
//    }
//    return god;
//}
//
//void pray_at_altar(object *pl, object *altar)
//{
//    object *pl_god  = find_god(determine_god(pl));
//
//    /* If non consecrate altar, don't do anything */
//    if (!altar->other_arch)
//        return;
//
//    /* hmm. what happend depends on pl's current god, level, etc */
//    if (!pl_god)
//    {
//        /*new convert */
//        become_follower(pl, &altar->other_arch->clone);
//        return;
//    }
//    else if (pl_god->name == altar->other_arch->clone.name)
//    {
//        /* pray at your gods altar */
//        int bonus   = ((pl->stats.Wis / 10) + (SK_level(pl) / 10));
//
//        new_draw_info(NDI_UNIQUE, 0, pl, "You feel the powers of your deity %s.", pl_god->name);
//
//        /* we can get neg grace up faster */
//        if (pl->stats.grace < 0)
//            pl->stats.grace += (bonus > -1 * (pl->stats.grace / 10) ? bonus : -1 * (pl->stats.grace / 10));
//        /* we can super-charge grace to 2x max */
//        if (pl->stats.grace < (2 * pl->stats.maxgrace))
//        {
//            pl->stats.grace += bonus / 2;
//        }
//        /* I think there was a bug here in that this was nested
//         * in the if immediately above
//         */
//        if (pl->stats.grace > pl->stats.maxgrace)
//        {
//            pl->stats.grace = pl->stats.maxgrace;
//        }
//
//        /* Every once in a while, the god decides to checkup on their
//         * follower, and may intervene to help them out.
//         */
//        bonus = MAX(1, bonus); /* -- DAMN -- */
//
//        if ((random_roll(0, 399) - bonus) < 0)
//            god_intervention(pl, pl_god);
//    }
//    else
//    {
//        /* praying to another god! */
//        int loss = 0, angry = 1;
//
//        /* I believe the logic for detecting opposing gods was completely
//         * broken - I think it should work now.  altar->other_arch
//         * points to the god of this altar (which we have
//         * already verified is non null).  pl_god->other_arch
//         * is the opposing god - we need to verify that exists before
//         * using its values.
//         */
//        if (pl_god->other_arch && (altar->other_arch->name == pl_god->other_arch->name))
//        {
//            angry = 2;
//            if (random_roll(0, SK_level(pl) + 2) - 5 > 0)
//            {
//                /* you really screwed up */
//                angry = 3;
//                new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, pl, "Foul Priest! %s punishes you!", pl_god->name);
//                cast_mana_storm(pl, pl_god->level + 20);
//            }
//            new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, pl, "Foolish heretic! %s is livid!", pl_god->name);
//        }
//        else
//            new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, pl, "Heretic! %s is angered!", pl_god->name);
//
//        /* whether we will be successfull in defecting or not -
//         * we lose experience from the clerical experience obj */
//
//        loss = (int) ((float) 0.1 * (float) pl->chosen_skill->skillgroup->stats.exp);
//        if (loss)
//            lose_priest_exp(pl, random_roll(0, loss * angry - 1));
//
//        /* May switch Gods, but its random chance based on our current level
//         * note it gets harder to swap gods the higher we get */
//        if ((angry == 1) && !(random_roll(0, pl->chosen_skill->skillgroup->level)))
//        {
//            become_follower(pl, &altar->other_arch->clone);
//        } /* If angry... switching gods */
//        else
//        {
//            /* toss this player off the altar.  He can try again. */
//            new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, pl, "A divine force pushes you off the altar.");
//            move_player(pl, absdir(pl->facing + 4), 1); /* back him off the way he came. */
//        } /* didn't successfully change, so forced off altar. */
//    } /* If prayed at altar to other god */
//}
//
//static int get_spell_number(object *op)
//{
//    int spell;
//
//    if (op->slaying && (spell = look_up_spell_name(op->slaying)) >= 0)
//        return spell;
//    else
//        return op->stats.sp;
//}
//
///*
// * become_follower - This function is called whenever a player has
// * switched to a new god. It handles basically all the stat changes
// * that happen to the player, including the removal of godgiven
// * items (from the former cult).
// */
//void become_follower(object *op, object *new_god)
//{
//    objectlink *ol;
//    object     *skillgroup = op->chosen_skill->skillgroup; /* obj. containing god data */
//    object     *old_god = NULL;                      /* old god */
//    treasure   *tr;
//    /*    object *item ;*/
//    int         i;
//
//    CONTR(op)->socket.ext_title_flag = 1;
//    /* get old god */
//    if (skillgroup->title)
//        old_god = find_god(skillgroup->title);
//
//    /* bad & buggy code! first, we must include here a "is_godgiven" flag..
//    * second, when you manipulate the player inv with non sys items, be sure
//    * you tell it the player and you call the send_inv_update function.
//    */
//    /* take away any special god-characteristic items. */
//    /* invisible start equip?? MT-11-2002 */
//    /* remove all invisible startequ. items which are
//         not skill, exp or force */
//    /*
//       for(item=op->inv;item!=NULL;item=item->below) {
//           if(QUERY_FLAG(item,FLAG_NO_DROP)) {
//        if(item->type==TYPE_SKILL || item->type==TYPE_SKILLGROUP ||
//           item->type==FORCE) continue;
//        remove_ob(item);
//        item=op->inv;
//    }
//       }*/
//
//    /* give the player any special god-characteristic-items. */
//    for (ol = new_god->randomitems; ol; ol = ol->next)
//    {
//        for (tr = ol->objlink.tl->items; tr != NULL; tr = tr->next)
//        {
//            /* TODO: The old system seemed to involve gods having their magic
//             * stored in BOOKs and SPELLBOOKs in their invs. These were sys
//             * objects to prevent the god from giving the source of his power
//             * away.
//             *
//             * This system involves several unnecessary checks so is
//             * technically no good.
//             *
//             * I've semi-fixed this so that these types in a god's inv are
//             * always assumed to be his powers and non-giftable. This should do
//             * for now (especially as Dai does not have such god interaction
//             * yet anyway). In future, gods should use randomitems/ABILITYs.
//             *
//             * --Smacky 20130216 */
//            if (tr->item &&
//                !QUERY_FLAG(&tr->item->clone, FLAG_SYS_OBJECT) &&
//                tr->item->clone.type != SPELLBOOK &&
//                tr->item->clone.type != BOOK)
//            {
//                god_gives_present(op, new_god, tr);
//            }
//        }
//    }
//
//    if (!op || !new_god)
//        return;
//
//    if (op->race && new_god->slaying && strstr(op->race, new_god->slaying))
//    {
//        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "Fool! %s detests your kind!", new_god->name);
//        if (random_roll(0, op->level - 1) - 5 > 0)
//            cast_mana_storm(op, new_god->level + 10);
//        return;
//    }
//
//    new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "You become a follower of %s!", new_god->name);
//
//    if (skillgroup->title)
//    {
//        /* get rid of old god */
//        new_draw_info(NDI_UNIQUE, 0, op, "%s's blessing is withdrawn from you.", skillgroup->title);
//        CLEAR_FLAG(skillgroup, FLAG_APPLIED);
//        change_abil(op, skillgroup);
//        FREE_AND_CLEAR_HASH2(skillgroup->title);
//    }
//
//    /* now change to the new gods attributes to skillgroup */
//    FREE_AND_COPY_HASH(skillgroup->title, new_god->name);
//    skillgroup->path_attuned = new_god->path_attuned;
//    skillgroup->path_repelled = new_god->path_repelled;
//    skillgroup->path_denied = new_god->path_denied;
//    /* copy god's resistances */
//    memcpy(skillgroup->resist, new_god->resist, sizeof(new_god->resist));
//
//    /* make sure that certain immunities do NOT get passed
//     * to the follower! */
//    for (i = 0; i < NROFATTACKS; i++)
//        if (skillgroup->resist[i] > 30 && (i == ATNR_FIRE || i == ATNR_COLD || i == ATNR_ELECTRICITY || i == ATNR_POISON))
//            skillgroup->resist[i] = 30;
//
//#ifdef MORE_PRIEST_GIFTS
//    skillgroup->stats.hp = (sint16) new_god->last_heal;
//    skillgroup->stats.sp = (sint16) new_god->last_sp;
//    skillgroup->stats.grace = (sint16) new_god->last_grace;
//    skillgroup->stats.food = (sint16) new_god->last_eat;
//    /* gods may pass on certain flag properties */
//    update_priest_flag(new_god, skillgroup, FLAG_SEE_IN_DARK);
//    update_priest_flag(new_god, skillgroup, FLAG_CAN_REFL_SPELL);
//    update_priest_flag(new_god, skillgroup, FLAG_CAN_REFL_MISSILE);
//    update_priest_flag(new_god, skillgroup, FLAG_STEALTH);
//    update_priest_flag(new_god, skillgroup, FLAG_SEE_INVISIBLE);
//    update_priest_flag(new_god, skillgroup, FLAG_UNDEAD);
//    update_priest_flag(new_god, skillgroup, FLAG_BLIND);
//    update_priest_flag(new_god, skillgroup, FLAG_XRAYS); /* better have this if blind! */
//#endif
//
//    new_draw_info(NDI_UNIQUE, 0, op, "You are bathed in %s's aura.", new_god->name);
//
//#ifdef MORE_PRIEST_GIFTS
//    /* Weapon/armour use are special...handle flag toggles here as this can
//     * only happen when gods are worshipped and if the new priest could
//     * have used armour/weapons in the first place */
//    update_priest_flag(new_god, skillgroup, FLAG_USE_WEAPON);
//    update_priest_flag(new_god, skillgroup, FLAG_USE_ARMOUR);
//
//    if (worship_forbids_use(op, skillgroup, FLAG_USE_WEAPON, "weapons"))
//        stop_using_item(op, WEAPON, 2);
//
//    if (worship_forbids_use(op, skillgroup, FLAG_USE_ARMOUR, "armour"))
//    {
//        stop_using_item(op, ARMOUR, 1);
//        stop_using_item(op, HELMET, 1);
//        stop_using_item(op, LEGS, 1);
//        stop_using_item(op, SHOULDER, 1);
//        stop_using_item(op, BOOTS, 1);
//        stop_using_item(op, GLOVES, 1);
//        stop_using_item(op, SHIELD, 1);
//    }
//#endif
//
//    SET_FLAG(skillgroup, FLAG_APPLIED);
//    change_abil(op, skillgroup);
//}
//
///* op is the player.
// * skillgroup is the widsom experience.
// * flag is the flag to check against.
// * string is the string to print out.
// */
//
//int worship_forbids_use(object *op, object *skillgroup, uint32 flag, char *string)
//{
//    if (QUERY_FLAG(&op->arch->clone, flag))
//        if (QUERY_FLAG(op, flag) != QUERY_FLAG(skillgroup, flag))
//        {
//            update_priest_flag(skillgroup, op, flag);
//            if (QUERY_FLAG(op, flag))
//                new_draw_info(NDI_UNIQUE, 0, op, "You may use %s again.", string);
//            else
//            {
//                new_draw_info(NDI_UNIQUE, 0, op, "You are forbidden to use %s.", string);
//                return 1;
//            }
//        }
//    return 0;
//}
//
///* stop_using_item() - unapplies up to number worth of items of type */
//void stop_using_item(object *op, int type, int number)
//{
//    object *tmp;
//
//    for (tmp = op->inv; tmp && number; tmp = tmp->below)
//        if (tmp->type == type && QUERY_FLAG(tmp, FLAG_APPLIED))
//        {
//            apply_special(op, tmp, AP_UNAPPLY | AP_IGNORE_CURSE);
//            number--;
//        }
//}
//
///* update_priest_flag() - if the god does/doesnt have this flag, we
// * give/remove it from the experience object if it doesnt/does
// * already exist. For players only!
// */
//
//void update_priest_flag(object *god, object *skillgroup, uint32 flag)
//{
//    if (QUERY_FLAG(god, flag) && !QUERY_FLAG(skillgroup, flag))
//        SET_FLAG(skillgroup, flag);
//    else if (QUERY_FLAG(skillgroup, flag) && !QUERY_FLAG(god, flag))
//    {
//        /*  When this is called with the skillgroup set to the player,
//         * this check is broken, because most all players arch
//         * allow use of weapons.  I'm not actually sure why this
//         * check is here - I guess if you had a case where the
//         * value in the archetype (wisdom) should over ride the restrictions
//         * the god places on it, this may make sense.  But I don't think
//         * there is any case like that.
//         */
//
//        /*        if (!(QUERY_FLAG(&(skillgroup->arch->clone),flag)))*/
//        CLEAR_FLAG(skillgroup, flag);
//    };
//}
//
///* Determines if op worships a god. Returns the godname if they do. In the case
// * of an NPC, if they have no god, we give them a random one. */
//const char *determine_god(object *op)
//{
//    /* Spells. */
//    if ((op->type == FBULLET ||
//         op->type == CONE ||
//         op->type == FBALL ||
//         op->type == SWARM_SPELL) &&
//        op->title)
//    {
//        if (lookup_god_by_name(op->title) >= 0)
//        {
//            return op->title;
//        }
//    }
//
//    /* If we are player, lets search a bit harder for the god.  This
//     * is a fix for perceive self (before, we just looked at the active
//     * skill). */
//    if (op->type == PLAYER)
//    {
//        object *this;
//
//        for (this = op->inv; this; this = this->below)
//        {
//            /* Gecko: we should actually only need to check either
//             * this->stats.Wis or this->sub_type1, but to avoid future
//             * mistakes we check both here. */
//            if (this->type == TYPE_SKILLGROUP &&
//                this->stats.Wis &&
//                this->sub_type1 == 5) // TODO: meaningful constant?
//            {
//                if (this->title)
//                {
//                    return this->title;
//                }
//                else
//                {
//                    return shstr_cons.none;
//                }
//            }
//        }
//    }
//    else if (QUERY_FLAG(op, FLAG_ALIVE))
//    {
//        if (!op->title)
//        {
//            godlink *gl;
//            int      godnr;
//
//            for (gl = first_god; gl; gl = gl->next)
//            {
//                if (gl == first_god)
//                {
//                    godnr = random_roll(1, gl->id);
//                }
//                else if (gl->id == godnr)
//                {
//                    FREE_AND_COPY_HASH(op->title, gl->name);
//
//                    break;
//                }
//            }
//        }
//
//        return op->title;
//    }
//
//    return shstr_cons.none;
//}
//
//archetype * determine_holy_arch(object *god, const char *type)
//{
//    object     *item;
//    objectlink *ol;
//    treasure   *tr;
//
//    if (!god || !god->randomitems)
//    {
//        LOG(llevBug, "BUG: determine_holy_arch(): no god or god without randomitems\n");
//        return NULL;
//    }
//
//    for (ol = god->randomitems; ol; ol = ol->next)
//    {
//        for (tr = ol->objlink.tl->items; tr != NULL; tr = tr->next)
//        {
//            if (!tr->item)
//                continue;
//            item = &tr->item->clone;
//
//            if (item->type == BOOK &&
//                !strcmp(item->name, type))
//            {
//                return item->other_arch;
//            }
//        }
//    }
//    return NULL;
//}
//
//
//static int god_removes_curse(object *op, int remove_damnation)
//{
//    object *tmp;
//    int     success = 0;
//
//    for (tmp = op->inv; tmp; tmp = tmp->below)
//    {
//        if (QUERY_FLAG(tmp, FLAG_DAMNED) && !remove_damnation)
//            continue;
//        if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
//        {
//            success = 1;
//            CLEAR_FLAG(tmp, FLAG_DAMNED);
//            CLEAR_FLAG(tmp, FLAG_CURSED);
//            CLEAR_FLAG(tmp, FLAG_KNOWN_CURSED);
//            esrv_update_item(UPD_FLAGS, tmp);
//        }
//    }
//
//    if (success)
//        new_draw_info(NDI_UNIQUE, 0, op, "You feel like someone is helping you.");
//    return success;
//}
//
//static int follower_level_to_enchantments(int level, int difficulty)
//{
//    if (difficulty < 1)
//    {
//        LOG(llevBug, "BUG: follower_level_to_enchantments(): difficulty %d is invalid\n", difficulty);
//        return 0;
//    }
//
//    if (level <= 20)
//        return level / difficulty;
//    if (level <= 40)
//        return (20 + (level - 20) / 2) / difficulty;
//    return (30 + (level - 40) / 4) / difficulty;
//}
//
//static int god_enchants_weapon(object *op, object *god, object *tr)
//{
//    char    buf[MEDIUM_BUF];
//    object *weapon;
//    /*uint32  attacktype;*/
//    int     tmp;
//
//    for (weapon = op->inv; weapon; weapon = weapon->below)
//        if (weapon->type == WEAPON && QUERY_FLAG(weapon, FLAG_APPLIED))
//            break;
//    if (weapon == NULL || god_examines_item(god, weapon) <= 0)
//        return 0;
//
//    /* First give it a title, so other gods won't touch it */
//    if (!weapon->title)
//    {
//        sprintf(buf, "of %s", god->name);
//        FREE_AND_COPY_HASH(weapon->title, buf);
//        esrv_update_item(UPD_NAME, weapon);
//        new_draw_info(NDI_UNIQUE, 0, op, "Your weapon quivers as if struck!");
//    }
//
//    /* Allow the weapon to slay enemies */
//    if (!weapon->slaying && god->slaying)
//    {
//        FREE_AND_COPY_HASH(weapon->slaying, god->slaying);
//        new_draw_info(NDI_UNIQUE, 0, op, "Your %s now hungers to slay enemies of your god!", weapon->name);
//        return 1;
//    }
//
//    /* Add the gods attacktype */
//    /*attacktype = (weapon->attacktype == 0) ? AT_PHYSICAL : weapon->attacktype;*/
//    /*
//    if ((attacktype & god->attacktype) != god->attacktype)
//    {
//        new_draw_info(NDI_UNIQUE, 0, op, "Your weapon suddenly glows!");
//        weapon->attacktype = attacktype | god->attacktype;
//        return 1;
//    }*/
//
//    /* Higher magic value */
//    tmp = follower_level_to_enchantments(SK_level(op), tr->level);
//    if (weapon->magic < tmp)
//    {
//        new_draw_info(NDI_UNIQUE, 0, op, "A phosphorescent glow envelops your weapon!");
//        weapon->magic++;
//        esrv_update_item(UPD_NAME, weapon);
//
//        return 1;
//    }
//
//    return 0;
//}
//
///*
// * follower_has_similar_item - Checks for any occurrence of
// * the given 'item' in the inventory of 'op' (recursively).
// * Returns 1 if found, else 0.
// */
//static int follower_has_similar_item(object *op, object *item)
//{
//    object *tmp;
//
//    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
//    {
//        if (tmp->type == item->type
//         && tmp->name == item->name
//         && tmp->title == item->title
//         && tmp->msg == item->msg
//         && tmp->slaying == item->slaying)
//            return 1;
//        if (tmp->inv && follower_has_similar_item(tmp, item))
//            return 1;
//    }
//    return 0;
//}
//
///*
// * follower_remove_similar_item - Checks for any occurrence of
// * the given 'item' in the inventory of 'op' (recursively).
// * Any matching items in the inventory are deleted, and a
// * message is displayed to the player.
// */
//static void follower_remove_similar_item(object *op, object *item)
//{
//    object *tmp, *next;
//
//    if (op && op->type == PLAYER && CONTR(op))
//    {
//        /* search the inventory */
//        for (tmp = op->inv; tmp != NULL; tmp = next)
//        {
//            next = tmp->below;   /* backup in case we remove tmp */
//
//            if (tmp->type == item->type
//             && tmp->name == item->name
//             && tmp->title == item->title
//             && tmp->msg == item->msg
//             && tmp->slaying == item->slaying)
//            {
//                new_draw_info(NDI_UNIQUE, 0, op, "The %s %s to dust!",
//                    QUERY_SHORT_NAME(tmp, op), (tmp->nrof > 1) ? "crumble" : "crumbles");
//                remove_ob(tmp);    /* remove obj from players inv. */
//            }
//            if (tmp->inv)
//                follower_remove_similar_item(tmp, item);
//        }
//    }
//}
//
//static int god_gives_present(object *op, object *god, treasure *tr)
//{
//    object *tmp;
//
//    if (follower_has_similar_item(op, &tr->item->clone))
//        return 0;
//
//    tmp = arch_to_object(tr->item);
//    new_draw_info(NDI_UNIQUE, 0, op, "%s lets %s appear in your hands.",
//        god->name, QUERY_SHORT_NAME(tmp, op));
//    tmp = insert_ob_in_ob(tmp, op);
//
//    return 1;
//}
//
//
///* god_intervention() - called from praying() currently. Every
// * once in a while the god will intervene to help the worshiper.
// * Later, this fctn can be used to supply quests, etc for the
// * priest. -b.t.
// */
//
//void god_intervention(object *op, object *god)
//{
//    int         level   = SK_level(op);
//    objectlink *ol;
//    treasure   *tr;
//
//    if (!god || !god->randomitems)
//    {
//        LOG(llevBug, "BUG: god_intervention(): (p:%s) no god %s or god without randomitems\n", STRING_OBJ_NAME(op),
//            STRING_OBJ_NAME(god));
//        return;
//    }
//
//    /* lets do some checks of whether we are kosher with our god */
//    if (god_examines_priest(op, god) < 0)
//        return;
//
//    new_draw_info(NDI_UNIQUE, 0, op, "You feel a holy presence!");
//
//    for (ol = god->randomitems; ol; ol = ol->next)
//    {
//        for (tr = ol->objlink.tl->items; tr != NULL; tr = tr->next)
//        {
//            object *item;
//
//            if (tr->chance <= random_roll(0, 99))
//                continue;
//
//            // Treasurelist - generate some treasure for the follower
//            if (tr->name)
//            {
//                treasurelist   *tl  = find_treasurelist(tr->name);
//                if (tl == NULL)
//                    continue;
//                new_draw_info(NDI_UNIQUE, 0, op,
//                              "Something appears before your "
//                              "eyes.  You catch it before it falls to the ground.");
//                create_treasure(tl, op, GT_NO_DROP | GT_ONLY_GOOD | GT_UPDATE_INV, level, T_STYLE_UNSET,
//                                ART_CHANCE_UNSET, T_MAGIC_UNSET, T_MAGIC_CHANCE_UNSET, 0, NULL);
//                return;
//            }
//
//            if (!tr->item)
//            {
//                LOG(llevBug, "BUG: empty entry in %s's treasure list\n", STRING_OBJ_NAME(god));
//                continue;
//            }
//
//            item = &tr->item->clone;
//
//            if (QUERY_FLAG(item, FLAG_SYS_OBJECT))
//            {
//                continue;
//            }
//            else if (item->type == BOOK)
//            {
//                if (item->name == shstr_cons.grace_limit)
//                {
//                    if (op->stats.grace < item->stats.grace || op->stats.grace < op->stats.maxgrace)
//                    {
//                        // Follower lacks the required grace for the following
//                        // treasure list items.
//                        (void) cast_change_attr(op, op, op, 0, SP_HOLY_POSSESSION);
//                        return;
//                    }
//                    continue;
//                }
//                else if (item->name == shstr_cons.restore_grace)
//                {
//                    if (op->stats.grace >= 0)
//                        continue;
//                    op->stats.grace = random_roll(0, 9);
//                    new_draw_info(NDI_UNIQUE, 0, op, "You are returned to a state of grace.");
//                    return;
//                }
//                else if (item->name == shstr_cons.restore_hitpoints)
//                {
//                    if (op->stats.hp >= op->stats.maxhp)
//                        continue;
//                    new_draw_info(NDI_UNIQUE, 0, op, "A white light surrounds and heals you!");
//                    op->stats.hp = op->stats.maxhp;
//                    return;
//                }
//                else if (item->name == shstr_cons.restore_spellpoints)
//                {
//                    int max     = (int) ((float) op->stats.maxsp * ((float) item->stats.maxsp / (float) 100.0));
//                    // Restore to 50 .. 100%, if sp < 50%
//                    int new_sp  = (int) ((float) random_roll(1000, 1999) / (float) 2000.0 * (float) max);
//                    if (op->stats.sp >= max / 2)
//                        continue;
//                    new_draw_info(NDI_UNIQUE, 0, op, "A blue lightning strikes " "your head but doesn't hurt you!");
//                    op->stats.sp = new_sp;
//                }
//                else if (item->name == shstr_cons.heal_spell)
//                {
//                    if (cast_heal(op, 1, op, get_spell_number(item)))
//                        return;
//                    else
//                        continue;
//                }
//                else if (item->name == shstr_cons.remove_curse)
//                {
//                    if (god_removes_curse(op, 0))
//                        return;
//                    else
//                        continue;
//                }
//                else if (item->name == shstr_cons.remove_damnation)
//                {
//                    if (god_removes_curse(op, 1))
//                        return;
//                    else
//                        continue;
//                }
//                else if (item->name == shstr_cons.heal_depletion)
//                {
//                    object     *depl;
//                    archetype  *at;
//                    int         i;
//
//                    if ((at = find_archetype("depletion")) == NULL)
//                    {
//                        LOG(llevBug, "BUG: Could not find archetype depletion.\n");
//                        continue;
//                    }
//                    depl = present_arch_in_ob(at, op);
//                    if (depl == NULL)
//                        continue;
//                    new_draw_info(NDI_UNIQUE, 0, op, "Shimmering light surrounds and restores you!");
//                    for (i = 0; i < NUM_STATS; i++)
//                        if (get_stat_value(&depl->stats, i))
//                            new_draw_info(NDI_UNIQUE, 0, op, "%s", restore_msg[i]);
//                    remove_ob(depl);
//                    FIX_PLAYER(op ,"god intervention");
//                    return;
//                }
//                else if (item->name == shstr_cons.message)
//                {
//                    new_draw_info(NDI_UNIQUE, 0, op, "%s", item->msg);
//                    return;
//                }
//                else if (item->name == shstr_cons.enchant_weapon)
//                {
//                    if (god_enchants_weapon(op, god, item))
//                        return;
//                    else
//                        continue;
//                }
//            }
//            else if (item->type == SPELLBOOK)
//            {
//                int spell   = get_spell_number(item);
//                if (check_spell_known(op, spell))
//                    continue;
//                if (spells[spell].level > level)
//                    continue;
//                new_draw_info(NDI_UNIQUE, 0, op, "%s grants you use of a special prayer!", god->name);
//                do_learn_spell(op, spell, 1);
//                return;
//            }
//            else
//            {
//                if (god_gives_present(op, god, tr))
//                    return;
//                else
//                    continue;
//            }
//            // else ignore it
//        }
//    }
//
//    new_draw_info(NDI_UNIQUE, 0, op, "You feel rapture.");
//}
//
//
//int god_examines_priest(object *op, object *god)
//{
//    int     reaction    = 1;
//    object *item        = NULL;
//
//    for (item = op->inv; item; item = item->below)
//    {
//        if (QUERY_FLAG(item, FLAG_APPLIED))
//        {
//            reaction += god_examines_item(god, item) * (item->magic ? abs(item->magic) : 1);
//        }
//    }
//
//    /* well, well. Looks like we screwed up. Time for god's revenge */
//    if (reaction < 0)
//    {
//        int     loss    = 10000000;
//        int     angry   = abs(reaction);
//        if (op->chosen_skill->skillgroup)
//            loss = (int) ((float) 0.05 * (float) op->chosen_skill->skillgroup->stats.exp);
//        lose_priest_exp(op, random_roll(0, loss * angry - 1));
//        if (random_roll(0, angry))
//            cast_mana_storm(op, SK_level(op) + (angry * 3));
//        new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, op, "%s becomes angry and punishes you!", god->name);
//    }
//
//    return reaction;
//}
//
///* god_likes_item() - your god looks at the item you
// * are using and we return either -1 (bad), 0 (neutral) or
// * 1 (item is ok). If you are using the item of an enemy
// * god, it can be bad...-b.t. */
//
//int god_examines_item(object *god, object *item)
//{
//    char    buf[MEDIUM_BUF];
//
//    if (!god || !item)
//        return 0;
//
//    if (!item->title)
//        return 1; /* unclaimed item are ok */
//
//    sprintf(buf, "of %s", god->name);
//    if (!strcmp(item->title, buf))
//        return 1; /* belongs to that God */
//
//    if (god->title)
//    {
//        /* check if we have any enemy blessed item*/
//        sprintf(buf, "of %s", god->title);
//        if (!strcmp(item->title, buf))
//        {
//            if (item->env &&
//                item->env->type == PLAYER)
//            {
//                new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, item->env, "Heretic! You are using %s!",
//                    QUERY_SHORT_NAME(item, item->env));
//            }
//            return -1;
//        }
//    }
//
//    return 0; /* item is sacred to a non-enemy god/or is otherwise magical */
//}
//
///* get_god() - returns the gods index in linked list
// * if exists, if not, it returns -1. -b.t.  */
//
//int get_god(object *priest)
//{
//    int godnr   = lookup_god_by_name(determine_god(priest));
//
//    return godnr;
//}
//
//
///* tailor_god_spell() - changes the attributes of cone, smite,
// * and ball spells as needed by the code. Returns false if there
// * was no race to assign to the slaying field of the spell, but
// * the spell attacktype contains AT_HOLYWORD.  -b.t.
// */
//
//int tailor_god_spell(object *spellop, object *caster)
//{
///*    object *god             = find_god(determine_god(caster)); */
//    int     caster_is_spell = 0;
//
//    if (caster->type == FBULLET || caster->type == CONE || caster->type == FBALL || caster->type == SWARM_SPELL)
//        caster_is_spell = 1;
//
///*    if (!god || (spellop->attacktype & AT_HOLYWORD && !god->race))
//    {
//        if (!caster_is_spell)
//            new_draw_info(NDI_UNIQUE, 0, caster, "This prayer is useless unless you worship an appropriate god");
//        else
//            LOG(llevBug, "BUG: tailor_god_spell(): no god\n");
//        return 0;
//    }
//*/
//    /* either holy word or godpower attacks will set the slaying field */
//    /*
//    if (spellop->attacktype & AT_GODPOWER)
//    {
//        FREE_AND_CLEAR_HASH2(spellop->slaying);
//        if (!caster_is_spell)
//        {
//            FREE_AND_COPY_HASH(spellop->slaying, god->slaying);
//        }
//        else if (caster->slaying)
//            FREE_AND_COPY_HASH(spellop->slaying, caster->slaying);
//    }
//    */
//
//    /* only the godpower attacktype adds the god's attack onto the spell */
//    /*
//    if (spellop->attacktype & AT_GODPOWER)
//        spellop->attacktype = spellop->attacktype | god->attacktype;
//    */
//    /* tack on the god's name to the spell */
//    /*
//    if (spellop->attacktype & AT_GODPOWER)
//    {
//        FREE_AND_COPY_HASH(spellop->title, god->name);
//        if (spellop->title)
//        {
//            char    buf[MEDIUM_BUF];
//            sprintf(buf, "%s of %s", spellop->name, spellop->title);
//            FREE_AND_COPY_HASH(spellop->name, buf);
//        }
//    }
//    */
//
//    return 1;
//}
//
///* we need a skill for this, not the skill group! */
//void lose_priest_exp(object *pl, int loss)
//{
//    /*
//      if(!pl||pl->type!=PLAYER||!pl->chosen_skill
//         ||!pl->chosen_skill->skillgroup)
//      {
//        LOG(llevBug,"BUG: Bad call to lose_priest_exp() \n");
//        return;
//      }
//      if((loss = check_dm_add_exp_to_obj(pl->chosen_skill->skillgroup,loss)))
//        add_exp(pl,-loss,pl->chosen_skill->stats.sp, 1);
//    */
//}
#else
#include "global.h"

// TODO: look over usage of those two functions and see if we can get
// away without the strncmp/strcmp calls
int lookup_god_by_name(const char *name)
{
    int godnr = -1, nmlen = strlen(name);

    if (name && name != shstr_cons.none)
    {
        godlink    *gl;
        for (gl = first_god; gl; gl = gl->next)
            if (!strncmp(name, gl->name, MIN((int) strlen(gl->name), nmlen)))
                break;
        if (gl)
            godnr = gl->id;
    }
    return godnr;
}

/* Determines if op worships a god. Returns the godname if they do. In the case
 * of an NPC, if they have no god, we give them a random one. */
const char *determine_god(object_t *op)
{
    /* Spells. */
    if (IS_GOD_SPELL(op) &&
        op->title)
    {
        if (lookup_god_by_name(op->title) >= 0)
        {
            return op->title;
        }
    }

    /* If we are player, lets search a bit harder for the god.  This
     * is a fix for perceive self (before, we just looked at the active
     * skill). */
    if (op->type == PLAYER)
    {
        object_t *this,
               *next;

        FOREACH_OBJECT_IN_OBJECT(this, op, next)
        {
            /* Gecko: we should actually only need to check either
             * this->stats.Wis or this->sub_type1, but to avoid future
             * mistakes we check both here. */
            if (this->type == TYPE_SKILLGROUP &&
                this->stats.Wis &&
                this->sub_type1 == 5) // TODO: meaningful constant?
            {
                if (this->title)
                {
                    return this->title;
                }
                else
                {
                    return shstr_cons.none;
                }
            }
        }
    }
    else if (QUERY_FLAG(op, FLAG_ALIVE))
    {
        if (!op->title)
        {
            godlink *gl;
            int      godnr;

            for (gl = first_god; gl; gl = gl->next)
            {
                if (gl == first_god)
                {
                    godnr = random_roll(1, gl->id);
                }
                else if (gl->id == godnr)
                {
                    FREE_AND_COPY_HASH(op->title, gl->name);

                    break;
                }
            }
        }

        return op->title;
    }

    return shstr_cons.none;
}

object_t * find_god(const char *name)
{
    object_t *god = NULL;

    if (name && name != shstr_cons.none)
    {
        godlink    *gl;
        for (gl = first_god; gl; gl = gl->next)
            if (!strcmp(name, gl->name))
                break;
        if (gl)
            god = pntr_to_god_obj(gl);
    }
    return god;
}

void pray_at_altar(object_t *pl, object_t *altar)
{
    object_t *pl_god  = find_god(determine_god(pl));

    /* If non consecrate altar, don't do anything */
    if (!altar->other_arch)
        return;

    /* hmm. what happend depends on pl's current god, level, etc */
    if (!pl_god)
    {
        /*new convert */
        become_follower(pl, &altar->other_arch->clone);
        return;
    }
    else if (pl_god->name == altar->other_arch->clone.name)
    {
        /* pray at your gods altar */
        int bonus   = ((pl->stats.Wis / 10) + (SK_level(pl) / 10));

        ndi(NDI_UNIQUE, 0, pl, "You feel the powers of your deity %s.", pl_god->name);

        /* we can get neg grace up faster */
        if (pl->stats.grace < 0)
            pl->stats.grace += (bonus > -1 * (pl->stats.grace / 10) ? bonus : -1 * (pl->stats.grace / 10));
        /* we can super-charge grace to 2x max */
        if (pl->stats.grace < (2 * pl->stats.maxgrace))
        {
            pl->stats.grace += bonus / 2;
        }
        /* I think there was a bug here in that this was nested
         * in the if immediately above
         */
        if (pl->stats.grace > pl->stats.maxgrace)
        {
            pl->stats.grace = pl->stats.maxgrace;
        }
//
//        /* Every once in a while, the god decides to checkup on their
//         * follower, and may intervene to help them out.
//         */
//        bonus = MAX(1, bonus); /* -- DAMN -- */
//
//        if ((random_roll(0, 399) - bonus) < 0)
//            god_intervention(pl, pl_god);
    }
    else
    {
        /* praying to another god! */
        int loss = 0, angry = 1;

        /* I believe the logic for detecting opposing gods was completely
         * broken - I think it should work now.  altar->other_arch
         * points to the god of this altar (which we have
         * already verified is non null).  pl_god->other_arch
         * is the opposing god - we need to verify that exists before
         * using its values.
         */
        if (pl_god->other_arch && (altar->other_arch->name == pl_god->other_arch->name))
        {
            angry = 2;
            if (random_roll(0, SK_level(pl) + 2) - 5 > 0)
            {
                /* you really screwed up */
                angry = 3;
                ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "Foul Priest! %s punishes you!", pl_god->name);
                cast_mana_storm(pl, pl_god->level + 20);
            }
            ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "Foolish heretic! %s is livid!", pl_god->name);
        }
        else
            ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "Heretic! %s is angered!", pl_god->name);

        /* May switch Gods, but its random chance based on our current level
         * note it gets harder to swap gods the higher we get */
        if ((angry == 1) && !(random_roll(0, pl->chosen_skill->skillgroup->level)))
        {
            become_follower(pl, &altar->other_arch->clone);
        } /* If angry... switching gods */
        else
        {
            /* toss this player off the altar.  He can try again. */
            ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "A divine force pushes you off the altar.");
            move_player(pl, absdir(pl->facing + 4), 1); /* back him off the way he came. */
        } /* didn't successfully change, so forced off altar. */
    } /* If prayed at altar to other god */
}

/*
 * become_follower - This function is called whenever a player has
 * switched to a new god. It handles basically all the stat changes
 * that happen to the player, including the removal of godgiven
 * items (from the former cult).
 */
void become_follower(object_t *op, object_t *new_god)
{
    object_t     *skillgroup = op->chosen_skill->skillgroup; /* obj. containing god data */
    object_t     *old_god = NULL;                      /* old god */
    int         i;

    if (!op || !new_god)
        return;

    CONTR(op)->socket.ext_title_flag = 1;
    /* get old god */
    if (skillgroup->title)
        old_god = find_god(skillgroup->title);

    if (op->race && new_god->slaying && strstr(op->race, new_god->slaying))
    {
        ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "Fool! %s detests your kind!", new_god->name);
        if (random_roll(0, op->level - 1) - 5 > 0)
            cast_mana_storm(op, new_god->level + 10);
        return;
    }

    ndi(NDI_UNIQUE | NDI_NAVY, 0, op, "You become a follower of %s!", new_god->name);

    if (skillgroup->title)
    {
        /* get rid of old god */
        ndi(NDI_UNIQUE, 0, op, "%s's blessing is withdrawn from you.", skillgroup->title);
        CLEAR_FLAG(skillgroup, FLAG_APPLIED);
        change_abil(op, skillgroup);
        FREE_AND_CLEAR_HASH2(skillgroup->title);
    }

    /* now change to the new gods attributes to skillgroup */
    FREE_AND_COPY_HASH(skillgroup->title, new_god->name);
    skillgroup->path_attuned = new_god->path_attuned;
    skillgroup->path_repelled = new_god->path_repelled;
    skillgroup->path_denied = new_god->path_denied;
    /* copy god's resistances */
    memcpy(skillgroup->resist, new_god->resist, sizeof(new_god->resist));

    /* make sure that certain immunities do NOT get passed
     * to the follower! */
    for (i = 0; i < NROFATTACKS; i++)
        if (skillgroup->resist[i] > 30 && (i == ATNR_FIRE || i == ATNR_COLD || i == ATNR_ELECTRICITY || i == ATNR_POISON))
            skillgroup->resist[i] = 30;

    ndi(NDI_UNIQUE, 0, op, "You are bathed in %s's aura.", new_god->name);
    SET_FLAG(skillgroup, FLAG_APPLIED);
    change_abil(op, skillgroup);
}
#endif
