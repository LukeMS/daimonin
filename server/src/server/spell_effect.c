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
/* TODO: The generic name for (wizard) spells and (divine) prayers is
 * castables, so the filenames/prefixes will be changed accordingly.
 *
 * -- Smacky 20140815 */

#include <global.h>

static int Detection(int type, object_t *this);

/* Oct 95 - hacked on this to bring in cosmetic differences for MULTIPLE_GOD hack -b.t. */

void prayer_failure(object_t *op, int failure, int power)
{
    const char *godname;

    if ((godname = determine_god(op)) == shstr_cons.none)
        godname = "Your spirit";

    if (failure <= -20 && failure > -40) /* wonder */
    {
        ndi(NDI_UNIQUE, 0, op, "%s gives a sign to renew your faith.", godname);
        cast_cone(op, op, 0, 10, SP_WOW, spellarch[SP_WOW], SK_level(op), 0);
    }
    else if (failure <= -40 && failure > -60) /* confusion */
    {
        ndi(NDI_UNIQUE, 0, op, "Your diety touches your mind!");
        confuse_player(op, op, 160);
    }
    else if (failure <= -60 && failure > -150) /* paralysis */
    {
        ndi(NDI_UNIQUE, 0, op, "%s requires you to pray NOW.", godname);
        ndi(NDI_UNIQUE, 0, op, "You comply, ignoring all else.");
        paralyze_player(op, op, 99);
    }
    else if (failure <= -150) /* blast the immediate area */
    {
        ndi(NDI_UNIQUE, 0, op, "%s smites you!", godname);
        cast_magic_storm(op, get_archetype("god_power"), power);
    }
}

/* Should really just replace all calls to cast_mana_storm to call
 * cast_magic_storm directly. */
void cast_mana_storm(object_t *op, int lvl)
{
    object_t *tmp = get_archetype("loose_magic");

    cast_magic_storm(op, tmp, lvl);
}


void cast_magic_storm(object_t *op, object_t *tmp, int lvl)
{
    if (!tmp)
        return; /* error */
    tmp->level = SK_level(op);
    tmp->x = op->x;tmp->y = op->y;
    tmp->stats.hp += lvl / 5;  /* increase the area of destruction */
    tmp->stats.dam = lvl; /* nasty recoils! */
    tmp->stats.maxhp = tmp->count; /*??*/
    insert_ob_in_map(tmp, op->map, op, 0);
}

int recharge(object_t *op)
{
    object_t *wand,
           *next;

    FOREACH_OBJECT_IN_OBJECT(wand, op, next)
    {
        if (wand->type == WAND &&
            QUERY_FLAG(wand, FLAG_APPLIED))
        {
            if (!(RANDOM_ROLL(0, 3)))
            {
                ndi(NDI_UNIQUE, 0, op, "%s vibrates violently, then explodes!",
                    QUERY_SHORT_NAME(wand, op));
                play_sound_map(MSP_KNOWN(op), SOUND_OB_EXPLODE, SOUND_NORMAL);
                remove_ob(wand);
            }
            else
            {
                ndi(NDI_UNIQUE, 0, op, "%s glows with power.",
                    QUERY_SHORT_NAME(wand, op));
                wand->stats.food += RANDOM_ROLL(1, spells[wand->stats.sp].charges);

                if (wand->arch &&
                    QUERY_FLAG(&wand->arch->clone, FLAG_ANIMATE))
                {
                    SET_FLAG(wand, FLAG_ANIMATE);
                    wand->speed = wand->arch->clone.speed;
                    update_ob_speed(wand);
                }
            }

            return 1;
        }
    }

    return 0;
}

int probe(object_t *op)
{
    msp_t *msp = MSP_KNOWN(op);
    object_t   *this,
             *next;

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        object_t *head = (this->head) ? this->head : this,
               *owner;

        if (IS_LIVE(head) &&
            (owner = get_owner(op)) &&
            owner->type == PLAYER)
        {
#ifdef DEBUG_PROBE_IS_CHARM
            /* Temporarily made probe into charm to test pet code */
            if (add_pet(owner, head, 0) == 0)
            {
                ndi(NDI_UNIQUE, 0, owner, "Your probe charms %s.",
                    QUERY_SHORT_NAME(head, owner));
            }

#else
            ndi(NDI_UNIQUE, 0, owner, "Your probe analyse%s.",
                QUERY_SHORT_NAME(head, owner));
            examine(owner, head, 1);
#endif
            return 1;
        }
    }

    return 0;
}

int cast_invisible(object_t *op, object_t *caster, int spell_type)
{
    /* object_t *tmp; */

    /*
    if(op->invisible>1000) {
      ndi(NDI_UNIQUE, 0,op,"You are already as invisible as you can get.");
      return 0;
    }
    */
    switch (spell_type)
    {
        case SP_INVIS:
          CLEAR_FLAG(op, FLAG_UNDEAD);
          break;
        case SP_INVIS_UNDEAD:
          SET_FLAG(op, FLAG_UNDEAD);
          break;
        case SP_IMPROVED_INVIS:
          break;
    }
    ndi(NDI_UNIQUE, 0, op, "You can't see your hands!");
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(op, UPD_FACE);
#else
    update_object(op, UP_OBJ_FACE);
#endif

    /* Gecko: removed entirely. This is instead handled in mob AI core */
    /* Gecko: fixed to only go through active objects. Nasty loop anyway... */
    /* for (tmp = active_objects->active_next; tmp != NULL; tmp = tmp->active_next)
        if (tmp->enemy == op)
            register_npc_enemy(tmp, NULL);*/
    return 1;
}

int perceive_self(object_t *op)
{
    char*cp = describe_item(op);
    archetype_t              *at  = find_archetype("depletion");
    object_t                 *tmp;
    int                     i;

    tmp = find_god(determine_god(op));
    if (tmp)
        ndi(NDI_UNIQUE, 0, op, "You worship %s", tmp->name);
    else
        ndi(NDI_UNIQUE, 0, op, "You worship no god");

    tmp = present_arch_in_ob(at, op);

    if (*cp == '\0' && tmp == NULL)
        ndi(NDI_UNIQUE, 0, op, "You feel very mundane");
    else
    {
        ndi(NDI_UNIQUE, 0, op, "You have:\n%s", cp);
        if (tmp != NULL)
        {
            for (i = 0; i < STAT_NROF; i++)
            {
                if (get_stat_value(&tmp->stats, i) < 0)
                {
                    ndi(NDI_UNIQUE, 0, op, "Your %s is depleted by %d",
                                  stat_name[i],
                                  -(get_stat_value(&tmp->stats, i)));
                }
            }
        }
    }
    return 1;
}

int cast_heal(object_t *op, int level, object_t *target, int spell_type)
{
    int     heal = 0,
            success = 0;
    /*object_t *tmp;*/

    /*LOG(llevNoLog,"dir: %d (%s -> %s)\n", dir, op?op->name:"<no op>",tmp?tmp->name:"<no tmp>");*/

    if (!op || !target)
    {
        LOG(llevBug, "BUG: cast_heal(): target or caster NULL (op: %s target: %s)\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
        return 0;
    }

    switch (spell_type)
    {
        case SP_MINOR_HEAL:
          success = 1;

          heal = RANDOM_ROLL(level / 2, (level + 3) / 4 * 3) + 5;

          /* give bonus or malus to damage depending on if the player/mob is attuned/repelled to that spell path */
          heal = (int)(heal * (double)(PATH_DMG_MULT(op, find_spell(spell_type))));
          if (op->type == PLAYER)
          {
              if (heal > 0)
                  ndi(NDI_UNIQUE, 0, op, "The prayer heals %s for %d hp!",
                      (op == target) ? "you" : QUERY_SHORT_NAME(target, op), heal);
              else
                  ndi(NDI_UNIQUE, 0, op, "The healing prayer fails!");
          }

          if (op != target && target->type == PLAYER)
          {
              if (heal > 0)
                  ndi(NDI_UNIQUE, 0, target, "%s casts minor healing on you healing %d hp!",
                      QUERY_SHORT_NAME(target, op), heal);
              else
                  ndi(NDI_UNIQUE, 0, target, "%s casts minor healing on you but it fails!",
                      QUERY_SHORT_NAME(target, op));
          }

          break;

        case SP_CURE_DISEASE:
          if (cure_disease(target, op))
              success = 1;
          break;

        case SP_REMOVE_DEPLETION:
          success = (cure_what_ails_you(target, ST1_FORCE_DEPLETE)) ? 1 : 0;

          break;

        case SP_RESTORATION:
          success = (cure_what_ails_you(target, ST1_FORCE_DRAIN)) ? 1 : 0;

          break;

        case SP_REMOVE_SLOW:
          success = (cure_what_ails_you(target, ST1_FORCE_SLOWED)) ? 1 : 0;

          break;

        case SP_REMOVE_FEAR:
          success = (cure_what_ails_you(target, ST1_FORCE_FEAR)) ? 1 : 0;

          break;

        case SP_REMOVE_SNARE:
          success = (cure_what_ails_you(target, ST1_FORCE_SNARE)) ? 1 : 0;

          break;

        case SP_REMOVE_PARALYZE:
          success = (cure_what_ails_you(target, ST1_FORCE_PARALYZE)) ? 1 : 0;

          break;

        case SP_REMOVE_CONFUSED:
          success = (cure_what_ails_you(target, ST1_FORCE_CONFUSED)) ? 1 : 0;

          break;

        case SP_REMOVE_BLIND:
          success = (cure_what_ails_you(target, ST1_FORCE_BLIND)) ? 1 : 0;

          break;

        case SP_CURE_POISON:
          success = (cure_what_ails_you(target, ST1_FORCE_POISON)) ? 1 : 0;

          break;

        case SP_REMOVE_DEATHSICK:
          success = (cure_what_ails_you(target, ST1_FORCE_DEATHSICK)) ? 1 : 0;

          break;

          /*
            case SP_MED_HEAL:
              heal=RANDOM_ROLL_roll(3, 6)+4;
              ndi(NDI_UNIQUE, 0,tmp, "Your wounds start to fade.");
              break;
            case SP_MAJOR_HEAL:
              ndi(NDI_UNIQUE, 0,tmp, "Your skin looks as good as new!");
              heal=RANDOM_ROLL_roll(4, 8)+8;
              break;
            case SP_HEAL:
              heal=tmp->stats.maxhp;
              ndi(NDI_UNIQUE, 0,tmp, "You feel just fine!");
              break;
            */
    }

    if (heal > 0)
    {
        if (reduce_symptoms(target, heal))
            success = 1;
        if (target->stats.hp < target->stats.maxhp)
        {
            success = 1;
            target->stats.hp += heal;
            if (target->stats.hp > target->stats.maxhp)
                target->stats.hp = target->stats.maxhp;
        }
    /* although last_damage is unsigned, wo put an negative value in it, in draw_client_map2 we convert back to signed int */
        if (target->damage_round_tag != ROUND_TAG)
        {
            target->last_damage = 0;
            target->damage_round_tag = ROUND_TAG;
        }
        target->last_damage -= heal;
    }

    if (success)
    {
        op->speed_left = -ABS(op->speed) * 3;
    }

    if (insert_spell_effect(spells[spell_type].archname, target->map, target->x, target->y))
        LOG(llevDebug, "insert_spell_effect() failed: spell:%d, obj:%s target:%s\n", spell_type, STRING_OBJ_NAME(op),
            STRING_OBJ_NAME(target));
    return success;
}

int cast_change_attr(object_t *op, object_t *caster, object_t *target, int dir, int spell_type)
{
    object_t *tmp = target,
           *this,
           *next,
           *force = NULL;
    int     is_refresh = 0, msg_flag = 1;
    int     atnr = 0, path = 0;        /* see protection spells */
    int     i;

    if (tmp == NULL)
        return 0;

    /* we ID the buff force with spell_type... if we find one, we have old effect.
     * if not, we create a fresh force. */
    FOREACH_OBJECT_IN_OBJECT(this, tmp, next)
    {
        if (this->type == FORCE)
        {
            if (this->weight_limit == spell_type)
            {
                force = this;    /* the old effect will be "refreshed" */
                is_refresh = 1;
                ndi(NDI_UNIQUE, 0, op, "You recast the spell while in effect.");
            }
            else if ((spell_type == SP_BLESS &&
                 this->weight_limit == SP_HOLY_POSSESSION) ||
                (spell_type == SP_HOLY_POSSESSION &&
                 this->weight_limit == SP_BLESS))
            {
                /* both bless AND holy posession are not allowed! */
                ndi(NDI_UNIQUE, 0, op, "No more blessings for you.");
                return 0;
            }
        }
    }

    if (force == NULL)
        force = get_archetype("force");
    force->weight_limit = spell_type;  /* mark this force with the originating spell */
    i = 0;   /* (-> protection spells) */

    switch (spell_type)
    {
        case SP_STRENGTH:
          force->speed_left = -1;
          if (tmp->type != PLAYER)
          {
              if (op->type == PLAYER)
                  ndi(NDI_UNIQUE, 0, op, "You can't cast this kind of spell on your target.");
              return 0;
          }
          else if (op->type == PLAYER && op != tmp)
              ndi(NDI_UNIQUE, 0, tmp, "%s casts strength on you!", op->name ? op->name : "someone");


          if (force->stats.Str < 2)
          {
              force->stats.Str++;
              if (op->type == PLAYER && op != tmp)
                  ndi(NDI_UNIQUE, 0, op, "%s get stronger.", tmp->name ? tmp->name : "someone");
          }
          else
          {
              msg_flag = 0;
              ndi(NDI_UNIQUE, 0, tmp, "You don't grow stronger but the spell is refreshed.");
              if (op->type == PLAYER && op != tmp)
                  ndi(NDI_UNIQUE, 0, op, "%s don't grow stronger but the spell is refreshed.",
                                       tmp->name ? tmp->name : "someone");
          }

          if (insert_spell_effect(spells[SP_STRENGTH].archname, target->map, target->x, target->y))
              LOG(llevDebug, "insert_spell_effect() failed: spell:%d, obj:%s caster:%s target:%s\n", spell_type,
                  STRING_OBJ_NAME(op), STRING_OBJ_NAME(caster), STRING_OBJ_NAME(target));

          break;
    }
    force->speed_left = -1 - SP_level_strength_adjust(op, caster, spell_type) * 0.1f;

    if (!is_refresh)
    {
        SET_FLAG(force, FLAG_APPLIED);
        force = insert_ob_in_ob(force, tmp);
    }
    if (msg_flag)
    {
        if(tmp->type == PLAYER)
            change_abil(tmp, force); /* Mostly to display any messages */
        else
            FIX_PLAYER(tmp ,"cast change attr - bug? bogus call - fix monster?"); /* fix monster? */
    }

    return 1;
}

int remove_curse(object_t *op, object_t *target, int type, SpellTypeFrom src)
{
    object_t *tmp,
           *next;
    int     success = 0;

    if (!op || !target)
        return success;

    if (op != target)
    {
        if (op->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, op, "You cast remove %s on %s.",
                (type == SP_REMOVE_CURSE) ? "curse" : "damnation",
                QUERY_SHORT_NAME(target, op));
        }
        else if (target->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, target, "%s casts remove %s on you.",
                QUERY_SHORT_NAME(op, target),
                (type == SP_REMOVE_CURSE) ? "curse" : "damnation");
        }
    }

    /* Player remove xx only removes applied stuff, npc remove clears ALL */
    FOREACH_OBJECT_IN_OBJECT(tmp, target, next)
    {
        if ((src == spellNPC || QUERY_FLAG(tmp, FLAG_APPLIED))
         && (QUERY_FLAG(tmp, FLAG_CURSED) || (type == SP_REMOVE_DAMNATION && QUERY_FLAG(tmp, FLAG_DAMNED))))
        {
            if (tmp->level <= SK_level(op))
            {
                success++;
                if (type == SP_REMOVE_DAMNATION)
                    CLEAR_FLAG(tmp, FLAG_DAMNED);
                CLEAR_FLAG(tmp, FLAG_CURSED);
                if (!QUERY_FLAG(tmp, FLAG_PERM_CURSED))
                    CLEAR_FLAG(tmp, FLAG_KNOWN_CURSED);
#ifndef USE_OLD_UPDATE
                OBJECT_UPDATE_UPD(tmp, UPD_FLAGS);
#else
                esrv_update_item(UPD_FLAGS, tmp);
#endif
            }
            else /* level of the items is to high for this remove curse */
            {
                if (target->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, target, "The %s's curse is stronger than the prayer!",
                        query_name(tmp, target, ARTICLE_NONE, 0));
                }
                else if (op != target &&
                         op->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, op, "The %s's curse of %s is stronger than your prayer!",
                        query_name(tmp, op, ARTICLE_NONE, 0),
                        QUERY_SHORT_NAME(target, op));
                }
            }
        }
    }

    if (op != target && op->type == PLAYER)
    {
        if (success)
        {
            ndi(NDI_UNIQUE, 0, op, "Your prayer removes some curses.");
        }
        else
        {
            ndi(NDI_UNIQUE, 0, op, "%s's items seem uncursed.",
                QUERY_SHORT_NAME(target, op));
        }
    }

    if (target->type == PLAYER)
    {
        if (success)
            ndi(NDI_UNIQUE, 0, target, "You feel like someone is helping you.");
        else
        {
            if (src == spellNormal)
                ndi(NDI_UNIQUE, 0, target, "You are not using any cursed items.");
            else
                ndi(NDI_UNIQUE, 0, target, "You hear manical laughter in the distance.");
        }
    }

    insert_spell_effect(spells[SP_REMOVE_CURSE].archname, target->map, target->x, target->y);
    return success;
}

/* main identify function to identify objects.
 * mode extension: 0: identify 1 to x items, depending
 * luck & wisdom (nethack style). This is default for player identify.
 * mode 1: identify all unidentified items in the inventory of op.
 * mode 2: identify marked item (not implemented)
 * i added a "identify level" - thats the "power" of the identify spell.
 * if the item has a higher level as the identify then the item
 * can'T be identified from this spell/skills.
 */
int cast_identify(object_t *op, int level, object_t *single_ob, int mode)
{
    object_t *tmp,
           *next;
    int     success = 0, success2 = 0, random_val = 0;
    int     chance  = 8 + op->stats.Wis;

    if (chance < 1)
        chance = 1;

    /* iam to lazy to put the id stuff in own function... */
    if (mode == IDENTIFY_MODE_MARKED)
    {
        tmp = single_ob;
        goto inside_jump1;
    }

    insert_spell_effect(spells[SP_IDENTIFY].archname, op->map, op->x, op->y);

    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        inside_jump1:
        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) &&
            !QUERY_FLAG(tmp, FLAG_SYS_OBJECT) &&
            (need_identify(tmp) ||
             mode ==IDENTIFY_MODE_MARKED))
        {
            success2++;
            if (level < tmp->level)
            {
                if (op->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, op, "%s %s too powerful for this identify!",
                        QUERY_SHORT_NAME(tmp, op), (tmp->nrof > 1) ? "are" : "is");
                }
            }
            else
            {
                identify(tmp);
                if (op->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, op, "You have %s.",
                        QUERY_SHORT_NAME(tmp, op));

                    if (tmp->msg)
                    {
                        ndi(NDI_UNIQUE, 0, op, "The item has a story:\n%s", tmp->msg);
                    }
                }

                if (IDENTIFY_MODE_NORMAL
                 && ((random_val = RANDOM_ROLL(0, chance - 1)) > (chance - ++success - 2)))
                    break;
            }
        }
        if (mode == IDENTIFY_MODE_MARKED)
            break;
    }
    /* If all the power of the spell has been used up, don't go and identify
     * stuff on the floor.  Only identify stuff on the floor if the spell
     * was not fully used.
     */
    /* i disable this... this is useful if we have like in cf 100 dead mobs
     * with 1000 items on the floor - but we don't want have this much junk
     * lot
     *
    if(IDENTIFY_MODE_ALL)
    {
        for (tmp = MSP_GET_LAST(op->map, op->x, op->y); tmp; tmp = tmp->below)
        if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) &&
            !QUERY_FLAG(tmp, FLAG_SYS_OBJECT) &&
            need_identify(tmp))
        {
            identify(tmp);

            if (op->type == PLAYER)
            {
                ndi(NDI_UNIQUE, 0, op, "On the ground %s %s.",
                    (tmp->nrof > 1) ? "are" : "is", QUERY_SHORT_NAME(tmp, op));

                if (tmp->msg)
                {
                    ndi(NDI_UNIQUE, 0, op, "The item has a story:\n%s",
                        tmp->msg);
                }

#ifndef USE_OLD_UPDATE
                OBJECT_UPDATE_REM(tmp);
#else
                esrv_send_item(tmp);
            }
        }
    }
    */
    if (op->type == PLAYER && (!success && !success2))
        ndi(NDI_UNIQUE, 0, op, "You can't reach anything unidentified in your inventory.");

    return success2;
}

int cast_detection(object_t *op, object_t *target, int type)
{
    int       suc = 0;
    msp_t *msp = MSP_KNOWN(op);
    object_t   *this,
             *next;

    if (op->type == PLAYER &&
        target != op)
    {
        ndi(NDI_UNIQUE, 0, op, "You cast detect magic on %s.",
            QUERY_SHORT_NAME(target, op));
    }

    if (target->type != PLAYER) /* only use self or players */
    {
        if (op->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, op, "This spell works only on players.");
        }

        return 0;
    }

    if (target != op)
    {
        ndi(NDI_UNIQUE, 0, target, "%s casts detect magic on you.",
            QUERY_SHORT_NAME(op, target));
    }

    FOREACH_OBJECT_IN_OBJECT(this, target, next)
    {
        suc += Detection(type, this);
    }

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        suc += Detection(type, this);
    }

    if (insert_spell_effect(spells[type].archname, target->map, target->x, target->y))
    {
        LOG(llevInfo, "insert_spell_effect() failed: spell:%d, obj:%s target:%s\n",
            type, STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
    }

    if (op->type == PLAYER)
    {
        ndi(NDI_UNIQUE, 0, op, "The spell detects %s.",
            (suc) ? "something" : "nothing");
    }

    if (target->type == PLAYER && target != op)
    {
        ndi(NDI_UNIQUE, 0, target, "The spell detects %s.",
            (suc) ? "something" : "nothing");
    }

    return 1;
}

static int Detection(int type, object_t *this)
{
    if (QUERY_FLAG(this, FLAG_SYS_OBJECT))
    {
        return 0;
    }

    if (type == SP_DETECT_MAGIC &&
        !QUERY_FLAG(this, FLAG_KNOWN_MAGICAL) &&
        QUERY_FLAG(this, FLAG_IS_MAGICAL))
    {
        SET_FLAG(this, FLAG_KNOWN_MAGICAL);
#ifndef USE_OLD_UPDATE
        OBJECT_UPDATE_UPD(this, UPD_FLAGS);
#else
        esrv_update_item(UPD_FLAGS, this);
#endif
        return 1;
    }
    else if (type == SP_DETECT_CURSE &&
             !QUERY_FLAG(this, FLAG_KNOWN_CURSED) &&
             (QUERY_FLAG(this, FLAG_CURSED) ||
              QUERY_FLAG(this, FLAG_DAMNED)))
    {
        SET_FLAG(this, FLAG_KNOWN_CURSED);
#ifndef USE_OLD_UPDATE
        OBJECT_UPDATE_UPD(this, UPD_FLAGS);
#else
        esrv_update_item(UPD_FLAGS, this);
#endif
        return 1;
    }

    return 0;
}

/* Neutralises effects like poisoning, blindness, and confusion. */
object_t *cure_what_ails_you(object_t *op, uint8 st1)
{
    object_t *tmp = op->inv;

    for (; tmp; tmp = tmp->below)
    {
        if (tmp->type == FORCE &&
            tmp->sub_type1 == st1)
        {
            remove_force(tmp);

            /* We assume only one force. */
            return tmp;
        }
    }

    return NULL;
}

/* if we are here, the arch (spell) we check was able to move
 * to this place. Wall() has failed include reflection.
 * now we look for a target.
 */
void check_fired_arch(object_t *op)
{
    msp_t  *msp;
    object_t    *this,
              *next,
              *owner;
    tag_t      op_tag;

    /* we return here if we have NOTHING blocking here */
    if (!msp_blocked(op, op->map, op->x, op->y))
    {
        return;
    }

    if (op->other_arch)
    {
        explode_object(op);
        return;
    }

    if (op->stats.sp == SP_PROBE &&
        op->type == BULLET)
    {
        probe(op);
        remove_ob(op);
        move_check_off(op, NULL, MOVE_FLAG_VANISHED);
        return;
    }

    if (!(owner = get_owner(op)))
    {
        owner = op;
    }

    op_tag = op->count;
    msp = MSP_KNOWN(op);

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        object_t *head = (this->head) ? this->head : this;
        tag_t   this_tag;
        int     dam;

        /* we need a extra check for pets & golems here later
         * but atm player & npc can't hit other friendly npc */
        if (!IS_LIVE(head) ||
            get_friendship(owner, head) >= FRIENDSHIP_HELP)
        {
            continue;
        }

        this_tag = this->count;
        dam = damage_ob(this, op->stats.dam, op, ENV_ATTACK_CHECK);

        if (!OBJECT_VALID(op, op_tag) ||
            OBJECT_VALID(this, this_tag) ||
            (op->stats.dam -= dam) < 0)
        {
            if (!QUERY_FLAG(op, FLAG_REMOVED))
            {
                remove_ob(op);
                move_check_off(op, NULL, MOVE_FLAG_VANISHED);
                return;
            }
        }
    }
}

void move_fired_arch(object_t *op)
{
    map_t *m = op->map;
    sint16     x = op->x + OVERLAY_X(op->direction),
               y = op->y + OVERLAY_Y(op->direction);
    msp_t  *msp = MSP_GET2(m, x, y);
    tag_t      op_tag  = op->count;

    /* peterm:  added to make comet leave a trail of burnouts
    it's an unadulterated hack, but the effect is cool.    */
    if (op->stats.sp == SP_METEOR)
    {
        replace_insert_ob_in_map("fire_trail", op);

        if (!OBJECT_VALID(op, op_tag))
        {
            return;
        }
    }

    /* the spell has reached a wall and/or the end of its moving points */
    if (msp &&
        (!op->last_sp-- ||
         (!op->direction ||
          MSP_IS_RESTRICTED(msp))))
    {
        if (op->other_arch)
        {
            explode_object(op);
            return;
        }

        remove_ob(op);
        move_check_off(op, NULL, MOVE_FLAG_VANISHED);
        return;
    }

    remove_ob(op);
    move_check_off(op, NULL, MOVE_FLAG_VANISHED);

    if (!msp)
    {
        return;
    }

    op->x = x;
    op->y = y;

    if (!insert_ob_in_map(op, m, op, 0))
    {
        return;
    }

    if (reflwall(msp, op))
    {
        if (op->type == BULLET &&
            op->stats.sp == SP_PROBE)
        {
            if ((msp->flags & (MSP_FLAG_ALIVE | MSP_FLAG_PLAYER)))
            {
                probe(op);
                remove_ob(op);
                move_check_off(op, NULL, MOVE_FLAG_VANISHED);
                return;
            }
        }

        op->direction = absdir(op->direction + 4);
        update_turn_face(op);
    }
    else
    {
        check_fired_arch(op);
    }
}
