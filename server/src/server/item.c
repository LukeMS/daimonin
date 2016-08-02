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

#include <global.h>

static object_t *CanReach(object_t *who, object_t *what);
static object_t *PickUp(object_t *who, object_t *what, object_t *where, uint32 nrof, object_t *from, object_t *to);
static void    FixAsNeeded(object_t *who, object_t *to, object_t *from, object_t *what);
static object_t *CanPickUp(object_t *who, object_t *what, object_t *where, uint32 nrof);
static object_t *CanDiscard(object_t *who, object_t *what);
static object_t *NoDiscardContainer(object_t *who, object_t *what);

/* describe_resistance generates the visible naming for resistances.
 * returns a static array of the description.  This can return
 * a big buffer.
 * if newline is true, we don't put parens around the description
 * but do put a newline at the end.  Useful when dumping to files
 */
char * describe_resistance(const object_t *const op, int newline)
{
    static char buf[LARGE_BUF];
    char        buf1[LARGE_BUF];
    int         tmpvar, flag = 1;

    buf[0] = 0;

    for (tmpvar = 0; tmpvar < NROFATTACKS; tmpvar++)
    {
        if (op->resist[tmpvar])
        {
            if (flag)
            {
                if (!newline)
                    strcat(buf, "(resists: ");
            }
            else
            {
                if (!newline)
                    strcat(buf, ", ");
            }

            sprintf(buf1, "%s%+d%%%c",
                attack_name[tmpvar].abbr, op->resist[tmpvar],
                (newline) ? '\n' :'\0');

            flag = 0;
            strcat(buf, buf1);
        }
    }
    if (!newline && !flag)
        strcat(buf, ") ");
    return buf;
}

/* describe_attacks generates the visible naming for attack forms.
 * returns a static array of the description.  This can return
 * a big buffer.
 * if newline is true, we don't put parens around the description
 * but do put a newline at the end.  Useful when dumping to files
 */
char * describe_attack(const object_t *const op, int newline)
{
    static char buf[LARGE_BUF];
    char        buf1[LARGE_BUF];
    int         tmpvar, flag = 1;

    buf[0] = 0;

    for (tmpvar = 0; tmpvar < NROFATTACKS; tmpvar++)
    {
        if (op->attack[tmpvar])
        {
            if (flag)
            {
                if (!newline)
                    strcat(buf, "(attacks: ");
            }
            else
            {
                if (!newline)
                    strcat(buf, ", ");
            }

            sprintf(buf1, "%s%+d%%%c",
                attack_name[tmpvar].abbr, op->attack[tmpvar],
                (newline) ? '\n' :'\0');

            flag = 0;
            strcat(buf, buf1);
        }
    }
    if (!newline && !flag)
        strcat(buf, ") ");
    return buf;
}

/* describe terrain flags
 * we use strcat only - prepare the retbuf before call.
 */
static void describe_terrain(const object_t *const op, char *const retbuf)
{
    if (op->terrain_flag & TERRAIN_AIRBREATH)
        strcat(retbuf, "(air breathing)");
    if (op->terrain_flag & TERRAIN_WATERWALK)
        strcat(retbuf, "(water walking)");
    if (op->terrain_flag & TERRAIN_FIREWALK)
        strcat(retbuf, "(fire walking)");
    if (op->terrain_flag & TERRAIN_CLOUDWALK)
        strcat(retbuf, "(cloud walking)");
    if (op->terrain_flag & TERRAIN_WATERBREATH)
        strcat(retbuf, "(water breathing)");
    if (op->terrain_flag & TERRAIN_FIREBREATH)
        strcat(retbuf, "(fire breathing)");
}

/*
 * Returns a pointer to a static buffer which contains a
 * description of the given object.
 * If it is a monster, lots of information about its abilities
 * will be returned.
 * If it is an item, lots of information about which abilities
 * will be gained about its user will be returned.
 * If it is a player, it writes out the current abilities
 * of the player, which is usually gained by the items applied.
 */
/* i rewrite this to describe *every* object in the game.
 * That includes a description of every flag, etc.
 * MT-2003 */
char * describe_item(const object_t *const op)
{
    object_t     *tmp,
               *next;
    int         attr, val, more_info = 0, id_true = 0;
    char        buf[MEDIUM_BUF];
    static char retbuf[LARGE_BUF*3];

    retbuf[0] = '\0';

    /* we start with living objects like mobs */
    if (op->type == PLAYER)
    {
        player_t *pl = CONTR(op);

        describe_terrain(op, retbuf);
        sprintf(strchr(retbuf, '\0'), "(regen: hp %+d, mana %+d, grace %+d)",
            pl->gen_hp, pl->gen_sp, pl->gen_grace);
    }
    else if (op->type == MONSTER)
    {
        describe_terrain(op, retbuf);

        if (QUERY_FLAG(op, FLAG_UNDEAD))
            strcat(retbuf, "(undead)");
        if (QUERY_FLAG(op, FLAG_INVULNERABLE))
            strcat(retbuf, "(invulnerable)");
        if (QUERY_FLAG(op, FLAG_NO_ATTACK))
            strcat(retbuf, "(never attacks)");
        if (QUERY_FLAG(op, FLAG_CAN_PASS_THRU))
            strcat(retbuf, "(pass through doors)");
        if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
            strcat(retbuf, "(see invisible)");

        if (QUERY_FLAG(op, FLAG_USE_WEAPON))
            strcat(retbuf, "(melee attack)");
        if (QUERY_FLAG(op, FLAG_READY_BOW))
            strcat(retbuf, "(range attack)");
        if (QUERY_FLAG(op, FLAG_USE_ARMOUR))
            strcat(retbuf, "(wear armour)");
        if (QUERY_FLAG(op, FLAG_USE_RING))
            strcat(retbuf, "(wear ring)");
        if (QUERY_FLAG(op, FLAG_FRIENDLY))
            strcat(retbuf, "(NPC)");
        if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE))
            strcat(retbuf, "(unaggressive)");
        else
            strcat(retbuf, "(aggressive)");
        if (QUERY_FLAG(op, FLAG_HITBACK))
            strcat(retbuf, "(hitback)");
        if (QUERY_FLAG(op, FLAG_READY_SPELL))
            strcat(retbuf, "(spellcaster)");
        if (QUERY_FLAG(op, FLAG_CAN_OPEN_DOOR))
            strcat(retbuf, "(open doors)");
        if (QUERY_FLAG(op, FLAG_RANDOM_MOVE))
            strcat(retbuf, "(wandering)");

        /* lets check the inv for spell ABILITY objects.
             * This was previous a randomitems search, but that
             * was wrong because its possible we use a random list to
             * generate different instances of this mob/item
             */
        FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
        {
            if (tmp && (tmp->type == ABILITY))
            {
                strcat(retbuf, "(");
                strcat(retbuf, STRING_SAFE(tmp->name));
                strcat(retbuf, ")");
            }
        }

        if (FABS(op->speed) > MIN_ACTIVE_SPEED)
        {
            switch ((int) ((FABS(op->speed)) * 15))
            {
                case 0:
                  strcat(retbuf, "(very slow)");
                  break;
                case 1:
                  strcat(retbuf, "(slow)");
                  break;
                case 2:
                  strcat(retbuf, "(normal speed)");
                  break;
                case 3:
                case 4:
                  strcat(retbuf, "(fast)");
                  break;
                case 5:
                case 6:
                  strcat(retbuf, "(very fast)");
                  break;
                case 7:
                case 8:
                case 9:
                case 10:
                  strcat(retbuf, "(extremely fast)");
                  break;
                default:
                  strcat(retbuf, "(lightning fast)");
                  break;
            }
        }
    }
    else /* here we handle items... */
    {
        if (QUERY_FLAG(op, FLAG_IDENTIFIED) || QUERY_FLAG(op, FLAG_BEEN_APPLIED) || !need_identify(op))
            id_true = 1; /* we only need calculate this one time */

        if (op->item_level)
        {
            if (op->item_skill)
            {
                sprintf(strchr(retbuf, '\0'), "(req. level %d in %s)",
                    op->item_level,
                    STRING_OBJ_NAME(skillgroups[op->item_skill - 1]));
            }
            else
            {
                sprintf(strchr(retbuf, '\0'), "(req. level %d)",
                    op->item_level);
            }
        }

        /* terrain flags have no double use... if valid, show them */
        if (id_true && op->terrain_type)
            describe_terrain(op, retbuf);

        /* now lets deal with special cases */
        switch (op->type)
        {
            case WAND:
            case ROD:
            case HORN:
              if (id_true)
              {
                  sprintf(buf, "(%1.2f sec)", ((float) op->last_grace / pticks_second));
                  strcat(retbuf, buf);
              }
              break;

              /* Armor type objects */
            case ARMOUR:
            case HELMET:
            case SHOULDER:
            case LEGS:
            case SHIELD:
            case BOOTS:
            case GLOVES:
            case GIRDLE:
            case BRACERS:
            case CLOAK:
              if (id_true)
              {
                  if (ARMOUR_SPEED(op))
                  {
                      sprintf(buf, "(encumbrance%+.1f%%)", ARMOUR_SPEED(op) / 10.0);
                      strcat(retbuf, buf);
                  }
                  /* Do this in all cases - otherwise it gets confusing - does that
                             * item have no penality, or is it not fully identified for example.
                             */
                  if (ARMOUR_SPELLS(op))
                  {
                      sprintf(buf, "(casting penalty%+.1f%%)", ARMOUR_SPELLS(op)/10.0);
                      strcat(retbuf, buf);
                  }
              }
            case WEAPON:
            case RING:
            case AMULET:
            case FORCE:
             if (op->stats.hp ||
                 op->stats.sp ||
                 op->stats.grace)
             {
                 more_info = 1;
             }

            case BOW:
            case ARROW:
              if (id_true)
              {
                  float dps_swing = 0.0f;

                  if (op->type == BOW || op->type == ARROW)
                      dps_swing = ((float) op->last_grace * WEAPON_SWING_TIME);

                  if (op->stats.dam)
                  {
                      int i, tmp_dam=0;

                      /* Show DPS */
                      /* These are "direct" damage dealers where we have a swing speed & dmg */
                      if(op->type == WEAPON || (op->type == ARROW && op->sub_type1 > 127) || op->type == BOW ||
                          op->type == PLAYER || op->type == MONSTER)
                      {
                          float dps = (float)op->stats.dam;

                          /* adjust damage with quality. DON'T adjust by /10 - this will
                          * give us a better value. Docs has to explain that the DPS of weapons
                          * is shown for a (ideal) level 10 char. Note that ->magic will be added
                          * to dam AND wc - for dam it will be handled as 0.1 additional damage.
                          * For this items this is only the "base DPS"
                          */
                          if(op->type == WEAPON || op->type == ARROW || op->type == BOW)
                              dps = (dps * (op->item_quality / 100.0f)) + op->magic;

                          if(!dps_swing) /* not a bow or arrow then use weapon_speed as swing speed */
                              dps_swing = op->weapon_speed;

                          if(dps_swing)
                              dps /= dps_swing;

                          /* we can have more as one attack - and so more as 100% damage (or less)
                          * so we need to "collect" and multiply the attack damage
                          * ONLY use the real damage attack forms
                          */
                          for(i=0;i<=LAST_ATNR_ATTACK;i++)
                              tmp_dam += op->attack[i];
                          tmp_dam += op->attack[ATNR_INTERNAL];

                          if(tmp_dam)
                              dps *= ((float)tmp_dam)/100.0f;

                          sprintf(buf, "(dps %.1f)", dps);
                      }
                      /* all what a player can apply has dam*10 value */
                      else if( op->type == AMULET || op->type == RING ||
                          op->type == BOOTS || op->type == HELMET || op->type == BRACERS || op->type == GIRDLE ||
                          op->type == CLOAK || op->type == ARMOUR || op->type == SHIELD || op->type == GLOVES ||
                          op->type == SHOULDER || op->type == LEGS || (op->type == ARROW && op->sub_type1 < 127))
                          sprintf(buf, "(dam%+.1f)", ((float)op->stats.dam)/10.0f);
                      else
                          sprintf(buf, "(dam%+d)", op->stats.dam);
                      strcat(retbuf, buf);
                  }

                  if (op->stats.wc)
                  {
                      sprintf(buf, "(wc%+d)", op->stats.wc);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.thac0)
                  {
                      sprintf(buf, "(hit chancet%+d)", op->stats.thac0);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.thacm)
                  {
                      sprintf(buf, "(fumble%+d)", op->stats.thacm);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.ac)
                  {
                      sprintf(buf, "(ac%+d)", op->stats.ac);
                      strcat(retbuf, buf);
                  }

                  if (op->type == ARROW || op->type == BOW)
                  {
                      if(op->last_grace)
                      {
                          sprintf(buf, "(%1.2f sec)", dps_swing);
                          strcat(retbuf, buf);
                      }

                      if (op->last_sp)
                      {
                          sprintf(buf, "(range%+d)", op->last_sp);
                          strcat(retbuf, buf);
                      }
                  }
                  else if (op->type == WEAPON)
                  {
                      sprintf(buf, "(%1.2f sec)", op->weapon_speed);
                      strcat(retbuf, buf);

                      if (op->level > 0)
                      {
                          sprintf(buf, "(improved %d/%d)", op->last_eat, op->level);
                          strcat(retbuf, buf);
                      }
                  }
              }

			  sprintf(buf, "%s", describe_resistance(op, 0));
			  strcat(retbuf, buf);

			  sprintf(buf, "%s", describe_attack(op, 0));
			  strcat(retbuf, buf);
              break;

            case FOOD:
            case DRINK:
              if (id_true)
              {
                  if (op->stats.thac0)
                  {
                      sprintf(strchr(retbuf, '\0'), "(hit%+d)", op->stats.thac0);
                  }

                  if (op->stats.thacm)
                  {
                      sprintf(strchr(retbuf, '\0'), "(miss%+d)", op->stats.thacm);
                  }

                  if (op->last_eat <= 0)
                  {
                      strcat(retbuf, "(no restorative qualities)");
                  }
                  else
                  {
                      sprintf(strchr(retbuf, '\0'), "(health per second for %d seconds: hp %+d, mana %+d, grace %+d)",
                          op->last_eat, op->stats.hp, op->stats.sp, op->stats.grace);
                  }
              }
              break;

            case POTION:
              if (id_true)
              {
                  if (op->stats.thac0)
                  {
                      sprintf(buf, "(hit%+d)", op->stats.thac0);
                      strcat(retbuf, buf);
                  }
                  if (op->stats.thacm)
                  {
                      sprintf(buf, "(miss%+d)", op->stats.thacm);
                      strcat(retbuf, buf);
                  }
                  if (op->last_sp)
                  {
                      sprintf(buf, "(range%+d)", op->last_sp);
                      strcat(retbuf, buf);
                  }
              }
              /* potions, scrolls, ...*/
            default:
              /* no more infos for all items we don't have handled in the switch */
              return retbuf;
        }

        /* these counts for every "normal" item a player deals with - most times equipement */
        if (id_true)
        {
            for (attr = 0; attr < 7; attr++)
            {
                if ((val = get_stat_value(&(op->stats), attr)) != 0)
                {
                    sprintf(buf, "(%s%+d)", short_stat_name[attr], val);
                    strcat(retbuf, buf);
                }
            }

            if (op->stats.exp)
            {
                sprintf(buf, "(speed %+d)", op->stats.exp);
                strcat(retbuf, buf);
            }
        }
    }

    /* some special info for some kind of identified items */
    if (id_true && more_info)
    {
        sprintf(strchr(retbuf, '\0'), "(regen: hp %+d, mana %+d, grace %+d)",
            op->stats.hp, op->stats.sp, op->stats.grace);
    }

    /* here we deal with all the special flags */
    if (id_true || op->type == MONSTER || op->type == PLAYER)
    {
        if (QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
            strcat(retbuf, "(see invisible)");
        if (QUERY_FLAG(op, FLAG_MAKE_ETHEREAL))
            strcat(retbuf, "(makes ethereal)");
        if (QUERY_FLAG(op, FLAG_IS_ETHEREAL))
            strcat(retbuf, "(ethereal)");
        if (QUERY_FLAG(op, FLAG_MAKE_INVISIBLE))
            strcat(retbuf, "(makes invisible)");
        if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
            strcat(retbuf, "(invisible)");
        if (QUERY_FLAG(op, FLAG_XRAYS))
            strcat(retbuf, "(xray-vision)");
        if (QUERY_FLAG(op, FLAG_SEE_IN_DARK))
            strcat(retbuf, "(infravision)");
        if (QUERY_FLAG(op, FLAG_LIFESAVE))
            strcat(retbuf, "(lifesaving)");
        if (QUERY_FLAG(op, FLAG_REFL_CASTABLE))
            strcat(retbuf, "(reflect castables)");
        if (QUERY_FLAG(op, FLAG_REFL_MISSILE))
            strcat(retbuf, "(reflect missiles)");
        if (QUERY_FLAG(op, FLAG_STEALTH))
            strcat(retbuf, "(stealth)");
        if (QUERY_FLAG(op, FLAG_FLYING))
            strcat(retbuf, "(flying)");
        if (QUERY_FLAG(op, FLAG_LEVITATE))
            strcat(retbuf, "(levitate)");
    }
    if (id_true)
    {
        if (op->slaying != NULL)
        {
            strcat(retbuf, "(slay ");
            strcat(retbuf, op->slaying);
            strcat(retbuf, ")");
        }

        strcat(retbuf, describe_attack(op, 0));
        DESCRIBE_PATH(retbuf, op->path_attuned, "Attuned");
        DESCRIBE_PATH(retbuf, op->path_repelled, "Repelled");
        DESCRIBE_PATH(retbuf, op->path_denied, "Denied");

        if ((op->type != HORN &&
             op->type != ROD &&
             op->type != WAND) &&
            (op->stats.maxhp ||
             op->stats.maxsp ||
             op->stats.maxgrace))
        {
            sprintf(strchr(retbuf, '\0'), "(health: hp %+d, mana %+d, grace %+d)",
                op->stats.maxhp, op->stats.maxsp, op->stats.maxgrace);
        }
    }
    return retbuf;
}

/* need_identify returns true if the item should be identified.  This
 * function really should not exist - by default, any item not identified
 * should need it.
 */

int need_identify(const object_t *const op)
{
    switch (op->type)
    {
        case RING:
        case WAND:
        case ROD:
        case HORN:
        case SCROLL:
        case TYPE_SKILL:
        case SPELLBOOK:
        case FOOD:
        case POTION:
        case BOW:
        case ARROW:
        case WEAPON:
        case ARMOUR:
        case SHIELD:
        case HELMET:
        case SHOULDER:
        case LEGS:
        case AMULET:
        case BOOTS:
        case GLOVES:
        case BRACERS:
        case GIRDLE:
        case CONTAINER:
        case DRINK:
        case INORGANIC:
        case CLOAK:
        case GEM:
        case TYPE_PEARL:
        case TYPE_JEWEL:
        case TYPE_NUGGET:
        case POWER_CRYSTAL:
        case POISON:
        case BOOK:
        case TYPE_LIGHT_APPLY:
        case TYPE_LIGHT_REFILL:
          return 1;
    }

    return 0;
}


/*
 * Supposed to fix face-values as well here, but later.
 */

void identify(object_t *op)
{
    if (!op)
    {
        return;
    }

    OBJECT_FULLY_IDENTIFY(op);
    CLEAR_FLAG(op, FLAG_NO_SKILL_IDENT);

    if (op->type == POTION &&
        op->arch)
    {
        /*op->face = op->arch->clone.face; */
        FREE_AND_ADD_REF_HASH(op->name, op->arch->clone.name);
    }
    else if (op->type == SPELLBOOK && op->slaying != NULL)
    {
        if ((op->stats.sp = look_up_spell_name(op->slaying)) < 0)
        {
            char    buf[256];
            op->stats.sp = -1;
            sprintf(buf, "Spell formula for %s", op->slaying);
            FREE_AND_COPY_HASH(op->name, buf);
        }
        else
        {
            /* clear op->slaying since we no longer need it */
            FREE_AND_CLEAR_HASH(op->slaying);
        }
    }

#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(op, UPD_FLAGS | UPD_NAME);
#else
    if (op->map) /* The shop identifies items before they hit the ground */
        /* I don't think identification will change anything but face */
        update_object(op, UP_OBJ_FACE);
    else
    {
        /* A lot of the values can change from an update - might as well send
         * it all. */
        esrv_send_item(op);
    }
#endif
}

/* check a object marked with
 * FLAG_IS_TRAPED has still a known
 * trap in it!
 */
void set_traped_flag(object_t *op)
{
    object_t *tmp,
           *next;
    int     flag;

    if (!op)
        return;

    /* player & monsters are not marked */
    if (op->type == PLAYER || op->type == MONSTER)
        return;

    flag = QUERY_FLAG(op, FLAG_IS_TRAPED);

    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        /* must be a rune AND visible */
        if (tmp->type == RUNE && tmp->stats.Cha <= 1)
        {
            SET_FLAG(op, FLAG_IS_TRAPED);
            if (!flag)
                goto set_traped_view;
            return;
        }
    }

    CLEAR_FLAG(op, FLAG_IS_TRAPED); /* clean */
    if (!flag)
        return;

    set_traped_view:
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(op, UPD_FLAGS);
#else
    if (!op->env) /* env object is on map */
        update_object(op, UP_OBJ_FACE);
    else /* somewhere else - if visible, update */
    {
        esrv_update_item(UPD_FLAGS, op);
    }
#endif
}

/* We don't want allow to put a magical container inside another
 * magical container. This function returns 0 when op can be
 * put in env of 1 when both are magical.
 */
int check_magical_container(const object_t *op, const object_t *env)
{
    /* is op a magical container? */
    if(!op || op->type != CONTAINER || op->weapon_speed == 1.0f)
    {
        return 0;
    }

    for(;env;env = env->env)
    {
        if (env->type == CONTAINER && env->weapon_speed != 1.0f)
        {
            return 1;
        }
    }

    return 0;
}

/* pick_up() is called when who attempts to pick up nrof * what. If where is
 * non-NULL it is a container into which what will be put (pick up to
 * container) otherwise what is moved into who's inv (pick up to inventory).
 *
 * As a special case, when what is of type LOOT we loop through the inventory
 * of what, picking up each of those objects. The empty loot object is then
 * removed (but if who is a player he is messaged that he picked up the loot
 * object, not each of its contents). In this way we can pick up many items
 * with one call.
 *
 * The return is what (but possibly with a different ->nrof, certainly with a
 * different ->env, etc than before the function) if what was picked up or NULL
 * if it wasn't. */
object_t *pick_up(object_t *who, object_t *what, object_t *where, uint32 nrof)
{
    player_t    *pl = NULL;
    object_t    *from,
                *to = who,
                *outof;
    uint32      tmp_nrof;

    /* Sanity checks: must be a who and only players and monsters can pick
     * things up; must be a what; if there is a where, it must be a
     * container (and for non-gmaster wiz players, it must be their linked
     * container). */
    if (!who ||
        (who->type != MONSTER &&
         (who->type != PLAYER ||
          !(pl = CONTR(who)))) ||
        !what ||
        (where &&
         (((!pl ||
            (pl->gmaster_mode & GMASTER_MODE_SA)) &&
           where->type != CONTAINER) ||
          (pl &&
           where != pl->container))))
    {
        return NULL;
    }

    /* Make sure who can reach what (and where if there is one) -- see
     * CanReach(). */
    if (!(from = CanReach(who, what)) ||
        (where &&
         !(to = CanReach(who, where))))
    {
        return NULL;
    }

    outof = what->env;

    /* Ensure nrof is a sensible value. */
    nrof = MAX(1, MIN(nrof, MAX_OBJ_NROF));

    if (nrof > what->nrof)
    {
        /* In shops any number of unpaid items can be picked up off the floor,
         * regardless of stack size. */
        if (!MSP_KNOWN(who) ||
            !what->map ||
            !QUERY_FLAG(what, FLAG_UNPAID))
        {
            nrof = what->nrof;
        }
    }

    /* When what is loot we go through a special process. */
    if (what->type == LOOT)
    {
        object_t *looted,
                 *next;

        if (pl)
        {
            SET_FLAG(who, FLAG_NO_FIX_PLAYER);
        }

        FOREACH_OBJECT_IN_OBJECT(looted, what, next)
        {
            /* Nested loots will be thrown away. */
            if (looted->type == LOOT)
            {
                remove_ob(looted);
            }
            /* Attempt to pick up each of the contents of the loot. */
            else
            {
                uint32 nr = MAX(1, MIN(looted->nrof, MAX_OBJ_NROF));

                (void)PickUp(who, looted, where, nr, from, to);
            }
        }

        /* If there is still loot we couldn't pick up... */
        if (what->inv)
        {
            /* When the loot is being given we want to give the script a chance
             * to deal with anything left over. */
            if (QUERY_FLAG(from, FLAG_IS_GIVING))
            {
                /* If who is a player, keep him uptodate with what is going on
                 * (he already knows why).  */
                if (pl)
                {
                    pl = NULL; // prevents futher 'you pick up...' messages
                    ndi(NDI_UNIQUE, 0, who, "You receive as much of %s as you can.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }
            }
            /* Otherwise... */
            else
            {
                /* If who is a player, keep him uptodate with what is going on
                 * (he already knows why).  */
                if (pl)
                {
                    pl = NULL; // prevents futher 'you pick up...' messages
                    ndi(NDI_UNIQUE, 0, who, "You take what you can of %s and leave the rest.",
                        query_name(what, who, ARTICLE_DEFINITE, 0));
                }

                /* Insert all the remaining contents of the loot in wherever the
                 * loot itself is. */
                if (what->map)
                {
                    FOREACH_OBJECT_IN_OBJECT(looted, what, next)
                    {
                        remove_ob(looted);
                        looted->x = what->x;
                        looted->y = what->y;
                        (void)insert_ob_in_map(looted, what->map, NULL, INS_NO_WALK_ON);
                    }
                }
                else if (what->env)
                {
                    FOREACH_OBJECT_IN_OBJECT(looted, what, next)
                    {
                        remove_ob(looted);
                        (void)insert_ob_in_ob(looted, what->env);
                    }
                }
            }
        }

        if (!(QUERY_FLAG(what, FLAG_REMOVED)))
        {
            remove_ob(what);
        }
    }
    /* When what is not loot, try to pick it up -- see PickUp(). If this fails,
     * just return NULL. */
    else if (!(what = PickUp(who, what, where, nrof, from, to)))
    {
        return NULL;
    }

    /* Give players an appropriate message to tell them what they've done. */
    /* TODO: Currently the client plays a pick up sound whenever a pick up is
     * attempted. This means when the server decides the attempt has failed, a
     * sound is still played. This should be moved server-side. Then also the
     * client can handle the 'you pick up...' messages on receipt of the sound
     * cmd. */
    if (pl)
    {
        char    buf[LARGE_BUF];
        object_t *into = (what->env &&
            what->env->type == CONTAINER) ? what->env : where;

        tmp_nrof = what->nrof;
        what->nrof = nrof;

        if (from != who &&
            QUERY_FLAG(from, FLAG_IS_GIVING))
        {
            sprintf(buf, "%s %s you %s",
                QUERY_SHORT_NAME(from, who),
                (from->nrof > 1) ? "give" : "gives",
                query_name(what, who, ARTICLE_DEFINITE, 0));

            if (into)
            {
                sprintf(strchr(buf, '\0'), " and you put %s into %s",
                    (nrof > 1) ? "them" : "it",
                    query_name(into, who, ARTICLE_DEFINITE, 0));
            }
        }
        else
        {
            if (outof &&
                outof != who)
            {
                sprintf(buf, "Taking %s out of %s, you ",
                    (nrof > 1) ? "them" : "it",
                    query_name(outof, who, ARTICLE_DEFINITE, 0));
            }
            else
            {
                sprintf(buf, "You ");
            }

            if (from == who)
            {
                sprintf(strchr(buf, '\0'), "%s %s into %s",
                    (to == who) ? "transfer" : "put",
                    query_name(what, who, ARTICLE_DEFINITE, 0),
                    (into) ? query_name(into, who, ARTICLE_DEFINITE, 0) : "your inventory");
            }
            else
            {
                sprintf(strchr(buf, '\0'), "pick up %s",
                    query_name(what, who, ARTICLE_DEFINITE, 0));

                if (into)
                {
                    sprintf(strchr(buf, '\0'), " and put %s into %s",
                        (nrof > 1) ? "them" : "it",
                        query_name(into, who, ARTICLE_DEFINITE, 0));
                }
            }
        }

        what->nrof = tmp_nrof;

        ndi(NDI_UNIQUE, 0, who, "%s.", buf);
    }

    /* Fix any involved players/monsters who need it -- see FixAsNeeded(). */
    FixAsNeeded(who, to, from, what);
    return what;
}

/* CanReach() ascertains if what is close enough that who can touch it. That
 * means what must be either (in a container) in who's inventory or (in a
 * container) on the same map square as who -- see below for some exceptions to
 * this basic rule.
 *
 * The return is NULL is what is not in reach of who or, if it is, an object
 * which is one of: what if what is directly on a map; a player or monster
 * (which may be who) if what is (in a container) in that creature's inv; a
 * container which is directly on a map.
 *
 * If who is a player an ndi is sent to the player when what is out of
 * reach. */
static object_t *CanReach(object_t *who, object_t *what)
{
    object_t *this;

    /* Walk up what's environments until the one before the map. In a textbook
     * scenario if what is (in a container) in who's inv then this should be
     * who (players and monters must not have envs).
     *
     * Afterwards this points to an object, one of: what if what is directly on
     * a map; a player or monster (which may be who) if what is (in a
     * container) in that creature's inv; a container which is directly on a
     * map. */
    for (this = what; this->env; this = this->env)
    {
        ;
    }

    /* If either who or this are airborne and the other is not (and what is not
     * in who's inventory), who can't reach what. */
    /* TODO: This makes no distinction between flying/levitating. */
    if (this != who &&
        IS_AIRBORNE(who) != IS_AIRBORNE(this))
    {
        if (who->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, who, "%s is floating out of your reach!",
                query_name(what, who, ARTICLE_DEFINITE, 0));
        }

        return NULL;
    }

    /* SAs can reach things in other creature's invs (including in containers
     * in those invs). All others can only fiddle with themselves, unless this
     * is actively giving. */
    if (this->type == PLAYER ||
        (this != what &&
         this->type == MONSTER))
    {
        if (this != who &&
            !QUERY_FLAG(this, FLAG_IS_GIVING) &&
            !(GET_GMASTER_MODE(who) & GMASTER_MODE_SA))
        {
            this = NULL;
        }
    }
    /* SAs can reach things in any container (open or closed). For convenience
     * so can monsters. Other players are limited to their linked container. */
    else if (this != what &&
             this->type == CONTAINER)
    {
        if (who->type == PLAYER &&
            !(CONTR(who)->gmaster_mode & GMASTER_MODE_SA) &&
            this != CONTR(who)->container)
        {
            this = NULL;
        }
    }

    /* If this does not point to who and is not giving, it must be directly on
     * a map so check that it occupies the same space as who. */
    if (this &&
        this != who &&
        !QUERY_FLAG(this, FLAG_IS_GIVING))
    {
        if (this->map != who->map ||
            this->x != who->x ||
            this->y != who->y)
        {
            this = NULL;
        }
    }

    if (!this &&
        who->type == PLAYER)
    {
        ndi(NDI_UNIQUE, 0, who, "%s is out of your reach!",
            query_name(what, who, ARTICLE_DEFINITE, 0));
    }

    return this;
}

/* PickUp() is the business end of picking up an object -- see pick_up(). */
static object_t *PickUp(object_t *who, object_t *what, object_t *where, uint32 nrof, object_t *from, object_t *to)
{
    player_t   *pl = (who->type == PLAYER) ? CONTR(who) : NULL;
    msp_t *msp;
    sint32 weight, tmp_nrof;

    /* For a pick up to inv (ie, where is NULL), who might have an appropriate
     * readied container in his inventory, so check that. */
    if (!where &&
        from != who)
    {
        object_t *this,
               *next;
        sint32  extra_weight = WEIGHT_OVERALL(what);

        FOREACH_OBJECT_IN_OBJECT(this, who, next)
        {
            /* The container must be applied (ready/open). */
            if (this->type == CONTAINER &&
                QUERY_FLAG(this, FLAG_APPLIED))
            {
                /* It must have enough space for what. */
                if (this->weight_limit >= this->carrying + extra_weight)
                {
                    /* Some containers can only hold specific classes of item
                     * (eg, pouches for valuables) while others can hold any.
                     * If we find a specific container that matches the item
                     * class being picked up, use that. Otherwise, use the
                     * first generic one found. */
                    /* TODO: Improve and expand these specific containers. */
                    if (!where &&
                        !this->race)
                    {
                        where = this;
                    }
                    else if (this->race == what->race)
                    {
                        where = this;
                        break;
                    }
                }
            }
        }
    }

    /* Check that who actually can pick up nrof * what. There are several
     * reasons why what can't be picked up -- see CanPickUp(). */
    if (!CanPickUp(who, what, where, nrof))
    {
        return NULL;
    }

    /* At this point we've determined that its theoretically OK for who to pick
     * up nrof * what. Now we need to determine the type of pick up (to
     * inventory or to container), either of which involves further side
     * effects/checks. */
    /* If where is (still) NULL (see above) we're trying to pick up what to
     * who's inv. */
    if (!where)
    {
        /* If what is a container be sure to unlink (close) it for all players
         * viewing it. */
        if (what->type == CONTAINER)
        {
            container_unlink(NULL, what);
        }
    }
    /* When where is non-NULL we're trying to put what in it. */
    else
    {
        tmp_nrof = what->nrof;
        what->nrof = nrof;
        weight = (sint32)(WEIGHT_OVERALL(what));
        what->nrof = tmp_nrof;

        /* Containers can't be put in other containers. */
        if (what->type == CONTAINER)
        {
            if (pl)
            {
                ndi(NDI_UNIQUE, 0, who, "Containers can't be put in other containers!");
            }

            return NULL;
        }
        /* where may be a container which cannot hold what. */
        else if ((where->sub_type1 & 1) != ST1_CONTAINER_CORPSE &&
                 where->race &&
                 where->race != what->race)
        {
            if (pl)
            {
                ndi(NDI_UNIQUE, 0, who, "Only %s can be put into %s!",
                    where->race, query_name(where, who, ARTICLE_DEFINITE, 0));
            }

            return NULL;
        }

        /* Check that where has enough space for what. */
        /* TODO: If nrof > 1 perhaps split what to fit? */
        else if (where->weight_limit < where->carrying + weight)
        {
            if (pl)
            {
                ndi(NDI_UNIQUE, 0, who, "%s %s too heavy to fit in %s!",
                   query_name(what, who, ARTICLE_DEFINITE, 0),
                   (nrof > 1) ? "are" : "is",
                   query_name(where, who, ARTICLE_DEFINITE, 0));
            }

            return NULL;
        }

        /* If an applyable light is lit (has glow_radius) force it to be
         * FLAG_APPLIED so we can unapply it below. This prevents players storing
         * lit torches in bags. */
        if (what->type == TYPE_LIGHT_APPLY &&
            what->glow_radius)
        {
            SET_FLAG(what, FLAG_APPLIED);
            apply_light(who, what);
        }

        /* Picking up to a container not in who's inventory is similar to
         * dropping in some respects so do the necessary checks -- see
         * CanDiscard(). */
        if (to != who &&
            !CanDiscard(who, what))
        {
            return NULL;
        }
    }

    /* Now we've determined the pick up is possible, so lets do it! */
    /* When a monster is picked up, it becomes a homeless mob. */
    if (what->type == MONSTER &&
        !QUERY_FLAG(what, FLAG_HOMELESS_MOB))
    {
        make_mob_homeless(what);
    }

    /* what can have a PICKUP script on it which, if it returns true, aborts
     * the actual pick up. */
    if (trigger_object_plugin_event(EVENT_PICKUP, what, who, where, NULL,
            (int *)&nrof, NULL, NULL, SCRIPT_FIX_ALL))
    {
        return NULL;
    }

    /* where can also have a PICKUP script on it which, if it returns true,
     * aborts the actual pick up. */
    if (where &&
        trigger_object_plugin_event(EVENT_PICKUP, where, who, what, NULL,
            (int *)&nrof, NULL, NULL, SCRIPT_FIX_ALL))
    {
        return NULL;
    }

    /* Picking things up from or to outside of who's inventory interrupts
     * resting. */
    if ((from != who ||
         to != who) &&
        pl)
    {
        pl->rest_mode = 0;
    }

    /* Below here is the actual code to move what to a new environment (either
     * where or who). Also we send some appropriate messages to the client. */
    /* Picking up unpaid items from a shop floor is preliminary to buying
     * them. */
    if ((msp = MSP_KNOWN(who)) &&
        what->map &&
        QUERY_FLAG(what, FLAG_UNPAID) &&
        !QUERY_FLAG(what, FLAG_SYS_OBJECT) &&
        what->layer)
    {
        what = clone_object(what, MODE_NO_INVENTORY);
        what->nrof = nrof;

        if (pl)
        {
            object_t *shop;

            MSP_GET_SYS_OBJ(msp, SHOP_FLOOR, shop);
            ndi(NDI_UNIQUE, 0, who, "You can buy %s for ~%s~ from %s.\n",
                query_name(what, who, ARTICLE_DEFINITE, 0),
                query_cost_string(what, who, F_BUY, COSTSTRING_SHORT),
                query_name(shop, who, ARTICLE_DEFINITE, 0));
        }
    }
    else
    {
        if (nrof < what->nrof)
        {
            if (pl)
            {
                SET_FLAG(who, FLAG_NO_FIX_PLAYER);
            }

            what = get_split_ob(what, nrof);
        }
        else if (!QUERY_FLAG(what, FLAG_REMOVED))
        {
            if (what->map)
            {
                remove_ob(what);

                if (move_check_off(what, NULL, MOVE_FLAG_VANISHED) > MOVE_RETURN_SUCCESS)
                {
                    return NULL;
                }
            }
            else
            {
                if (pl)
                {
                    SET_FLAG(who, FLAG_NO_FIX_PLAYER);
                }

                remove_ob(what);
            }
        }
    }

    /* As long as what is not LOOT, insert it into its new environment. */
    if (what->type != LOOT)
    {
        what = insert_ob_in_ob(what, (where) ? where : who);
    }

    return what;
}

/* CanPickUp() determines if who can pick up (if appropriate, nrof *) what
 * (into where if not NULL)>
 *
 * The return is what if who can pick it up, or NULL if who can't.
 *
 * If who is a player an ndi is sent to the player when what can't be picked
 * up. */
static object_t *CanPickUp(object_t *who, object_t *what, object_t *where, uint32 nrof)
{
    sint32 tmp_nrof;
    sint32 weight;

    /* Picking up players would cause mayhem so it's not allowed. */
    if (what->type == PLAYER)
    {
        if (who->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, who, "%s really would not like being picked up!",
                QUERY_SHORT_NAME(what, who));
        }

        return NULL;
    }

    /* Multiparts are absolute no-nos because the server cannot cope with such
     * objects having an environment. Also it would be logically ridiculous to
     * have such physically large objects being pocketed. */
    /* TODO: This restriction will be lifted fo gmaster wiz players. */
    if (what->more ||
        what->head)
    {
        if (who->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, who, "%s %s too large for you to pick up!",
                query_name(what, who, ARTICLE_DEFINITE, 0),
                (what->nrof > 1) ? "are" : "is");
        }

        return NULL;
    }

    /* Normal players and mobs cannot pick up these items but MW/MM/SAs can. */
#ifdef DAI_DEVELOPMENT_CONTENT
    if (!(GET_GMASTER_MODE(who) & (GMASTER_MODE_MW | GMASTER_MODE_MM | GMASTER_MODE_SA)))
#else
    if (!(GET_GMASTER_MODE(who) & (GMASTER_MODE_MM | GMASTER_MODE_SA)))
#endif
    {
        int     ego_mode;
        object_t *from;

        /* If what is somebody else's ego item then who can't pick it up. */
        if ((ego_mode = check_ego_item(who, what)) &&
            ego_mode == EGO_ITEM_BOUND_PLAYER)
        {
            if (who->type == PLAYER)
            {
                ndi (NDI_UNIQUE, 0, who, "%s %s not your ego item!",
                    query_name(what, who, ARTICLE_DEFINITE, 0),
                    (what->nrof > 1) ? "are" : "is");
            }

            return NULL;
        }

        /* No_picks can't be picked up. */
        if (QUERY_FLAG(what, FLAG_NO_PICK) ||
        /* If you can't see it, you can't pick it up. */
            IS_NORMAL_INVIS_TO(what, who))
        {
            if (who->type == PLAYER)
            {
                ndi(NDI_UNIQUE, 0, who, "%s %s not something you can pick up!",
                    query_name(what, who, ARTICLE_DEFINITE, 0),
                    (what->nrof > 1) ? "are" : "is");
            }

            return NULL;
        }

        for (from = what; from->env; from = from->env)
        {
            ;
        }

        /* When what is not already in (a container in) who's inv, check who
         * can lift it. */
        if (from != who)
        {
            sint32 who_limit = (who->type == PLAYER) ?
                (sint32)CONTR(who)->weight_limit :
                ((who->weight_limit > 0) ? who->weight_limit : 20000);

            // Hack to allow WEIGHT_OVERALL to not account for full stack.
            // TODO: Change WEIGHT_OVERALL
            tmp_nrof = what->nrof;
            what->nrof = nrof;
            weight = (sint32)(WEIGHT_OVERALL(what));
            what->nrof = tmp_nrof;

            if (who_limit < who->carrying + weight)
            {
                if (who->type == PLAYER)
                {
                    ndi(NDI_UNIQUE, 0, who, "%s %s too heavy to pick up!",
                        query_name(what, who, ARTICLE_DEFINITE, 0),
                        (what->nrof > 1) ? "are" : "is");
                }

                return NULL;
            }
        }
    }

    return what;
}

/* FixAsNeeded() carries out any necessary fixing post-pick up/drop. */
/* TODO: While I think this works well enough, ALL fixing needs
 * reworking/making more efficient. But that is a separate job. */
/* TODO: Currently monster encumbrance probably isn't handled. */
static void FixAsNeeded(object_t *who, object_t *to, object_t *from, object_t *what)
{
    object_t *needfixing[4];
    uint8   i;
    int     is_sys_object = QUERY_FLAG(what, FLAG_SYS_OBJECT);

    if (from == who ||
        from == to)
    {
        from = NULL;
    }

    if (to == who)
    {
        to = NULL;
    }

    needfixing[0] = who;
    needfixing[1] = to;
    needfixing[2] = from;
    needfixing[3] = NULL;

    for (i = 0; i <= 3; i++)
    {
        object_t *this = needfixing[i];

        if (!this)
        {
            return;
        }

        if (this->type == PLAYER)
        {
            CLEAR_FLAG(this, FLAG_NO_FIX_PLAYER);
        }

        if (is_sys_object)
        {
            FIX_PLAYER(this, "pick up/drop"); // also fixes monsters
        }
        else if (this->type == PLAYER)
        {
            fix_player_weight(this);
        }
    }
}

/* drop_to_floor() is called when who attempts to drop nrof * what.
 *
 * The return is what (but possibly with a different ->nrof, certainly with a
 * different ->env and ->map, etc than before the function) if what was dropped
 * or NULL if it wasn't. */
object_t *drop_to_floor(object_t *who, object_t *what, uint32 nrof)
{
    player_t   *pl = NULL;
    msp_t *msp;
    object_t   *shop,
             *from,
             *outof;
    int       reinsert = 1;

    /* Sanity checks: must be a who and only players and monsters can drop
     * things; must be a what. */
    if (!who ||
        (who->type != MONSTER &&
         (who->type != PLAYER ||
          !(pl = CONTR(who)))) ||
        !what)
    {
        return NULL;
    }

    for (from = what; from->env; from = from->env)
    {
        ;
    }

    /* who can't drop what unless it is in (a container in) who's inventory. */
    if (from != who)
    {
        return NULL;
    }

    outof = what->env;
    msp = MSP_KNOWN(who);
    MSP_GET_SYS_OBJ(msp, SHOP_FLOOR, shop);

    /* In a shop a container that is not empty cannot be dropped. */
    if (shop &&
        what->type == CONTAINER &&
        what->inv)
    {
        if (pl)
        {
            ndi(NDI_UNIQUE, 0, who, "First take everything out of %s!",
                query_name(what, who, ARTICLE_DEFINITE, 0));
        }

        return NULL;
    }

    /* Sometimes who cannot discard what -- see CanDiscard(). */
    if (!CanDiscard(who, what))
    {
        return NULL;
    }

    /* Now we've determined the drop is possible, so lets do it! */
    /* If what is a container be sure to unlink (close) it for all players
     * viewing it. */
    if (what->type == CONTAINER)
    {
        container_unlink(NULL, what);
    }

    /* what can have a DROP script on it which, if it returns true, aborts the
     * actual drop. */
    if (trigger_object_plugin_event(EVENT_DROP, what, who, NULL, NULL,
            (int *)&nrof, NULL, NULL, SCRIPT_FIX_ALL))
    {
        return NULL;
    }

    /* Dropping things interrupts resting. */
    if (pl)
    {
        pl->rest_mode = 0;
    }

    /* Below here is the actual code to move what to the map (specifically the
     * sqaure beneath who's feet). Also we send some appropriate messages to
     * the client. */
    if (nrof &&
        what->nrof != nrof)
    {
        what = get_split_ob(what, nrof);
    }
    else if (!QUERY_FLAG(what, FLAG_REMOVED))
    {
        remove_ob(what);
    }

    /* Give players an appropriate message to tell them what they've done. */
    /* TODO: Currently the client plays a drop sound whenever a drop is
     * attempted. This means when the server decides the attempt has failed, a
     * sound is still played. This should be moved server-side. Then also the
     * client can handle the 'you drop...' message on receipt of the sound
     * cmd. */
    if (pl)
    {
        char buf[MEDIUM_BUF];

        if (outof &&
            outof != who)
        {
            sprintf(buf, "Taking %s out of %s, you",
                (nrof > 1) ? "them" : "it", query_name(outof, who, ARTICLE_DEFINITE, 0));
        }
        else
        {
            sprintf(buf, "You ");
        }

        ndi(NDI_UNIQUE, 0, who, "%s drop %s.",
            buf, query_name(what, who, ARTICLE_DEFINITE, 0));
    }

    /* No drops vanish for non-SAs. */
    if (pl &&
        !(pl->gmaster_mode & GMASTER_MODE_SA) &&
        QUERY_FLAG(what, FLAG_NO_DROP))
    {
        ndi(NDI_UNIQUE, 0, who, "~NO-DROP~: %s vanishes to nowhere!",
            query_name(what, who, ARTICLE_DEFINITE, 0));
        reinsert = 0;
    }
    /* In a shop there are special rules. */
    else if (shop &&
             !QUERY_FLAG(what, FLAG_SYS_OBJECT) &&
             what->layer)
    {
        sint64 price;

        /* An unpaid item is just put back in the shop. */
        if (QUERY_FLAG(what, FLAG_UNPAID))
        {
            if (pl)
            {
                ndi(NDI_UNIQUE, 0, who, "The shop magic puts %s back in storage.",
                    query_name(what, who, ARTICLE_DEFINITE, 0));
            }

            reinsert = 0;
        }
        /* Coins and worthless items just drop to the floor like normal but
         * everything else is sold to the shop. */
        else if (what->type != MONEY &&
                 (price = query_cost(what, who, F_SELL)) > 0)
        {
            moneyblock_t  money;
            object_t       *loot;

            (void)enumerate_coins(price, &money);
            loot = create_financial_loot(&money, who, MODE_NO_INVENTORY);

            if (pl)
            {
                ndi(NDI_UNIQUE, 0, who, "You sell %s for ~%s~.",
                    query_name(what, who, ARTICLE_DEFINITE, 0),
                    query_cost_string(what, who, F_SELL, COSTSTRING_FULL));
            }

            FREE_AND_COPY_HASH(loot->name, "your fee");
            (void)pick_up(who, loot, NULL, 1);
            reinsert = 0;
        }
    }

    /* Yay! we're finally done. Insert what into its new map if necessary. */
    if (reinsert)
    {
        what->x = who->x;
        what->y = who->y;
        what = insert_ob_in_map(what, who->map, who, 0);

        /* Reinsert who at the top of the object list. */
        SET_FLAG(who, FLAG_NO_APPLY);
        remove_ob(who);
        (void)insert_ob_in_map(who, who->map, who, INS_NO_MERGE | INS_NO_WALK_ON);
        CLEAR_FLAG(who, FLAG_NO_APPLY);
    }

    /* Fix any involved players/monsters who need it -- see FixAsNeeded(). */
    FixAsNeeded(who, NULL, NULL, what);

    return what;
}

/* CanDiscard() determines if who can discard what.
 *
 * The return is what if who can discard it, or NULL if who can't.
 *
 * If who is a player an ndi is sent to the player when what can't be
 * discarded. */
static object_t *CanDiscard(object_t *who, object_t *what)
{
    /* Players have a few specific rules. */
    if (who->type == PLAYER)
    {
        /* Locked items can't be dropped. */
        if (QUERY_FLAG(what, FLAG_INV_LOCKED))
        {
            ndi(NDI_UNIQUE, 0, who, "%s %s locked!",
                query_name(what, who, ARTICLE_DEFINITE, 0),
                (what->nrof > 1) ? "are" : "is");
            return NULL;
        }

        /* Under some circumstances a player cannot discard a container with
         * something in it. */
        if (what->type == CONTAINER)
        {
            /* Always, certain contents prevent dropping. */
            if (NoDiscardContainer(who, what))
            {
                return NULL;
            }
        }
    }
    /* Monsters can't drop no drops. */
    else if (QUERY_FLAG(what, FLAG_NO_DROP))
    {
        return NULL;
    }

    /* If what is applied and can't be unapplied (eg, it's cursed) then it
     * can't be discarded. */
    if (QUERY_FLAG(what, FLAG_APPLIED))
    {
        int cantunapply = apply_equipment(who, what, AP_UNAPPLY);

        if (cantunapply)
        {
            return NULL;
        }

        if (!QUERY_FLAG(what, FLAG_SYS_OBJECT)) // else called later
        {
            FIX_PLAYER(who, "CanDiscard");
        }
    }

    return what;
}

/* NoDiscardContainer() recursively checks containers for no drops and locked
 * items inside, returning a pointer to the first such object found (container
 * cannot be discarded) or NULL if none are found (container can be
 * discarded).
 *
 * An ndi is sent to who (this function is only called for players) when the
 * container cannot be discarded. */
static object_t *NoDiscardContainer(object_t *who, object_t *what)
{
    object_t *this,
           *next;

    FOREACH_OBJECT_IN_OBJECT(this, what, next)
    {
        if (QUERY_FLAG(this, FLAG_SYS_OBJECT))
        {
            continue;
        }

        if (this->type == CONTAINER &&
            this->inv)
        {
            object_t *that = NoDiscardContainer(who, this);

            if (that)
            {
                return that;
            }
        }

        if (QUERY_FLAG(this, FLAG_NO_DROP))
        {
            ndi(NDI_UNIQUE, 0, who, "First remove all ~NO-DROP~ items from %s!",
               query_name(what, who, ARTICLE_DEFINITE, 0));
            return this;
        }
        else if (QUERY_FLAG(this, FLAG_INV_LOCKED))
        {
            ndi(NDI_UNIQUE, 0, who, "First remove all locked items from %s!",
               query_name(what, who, ARTICLE_DEFINITE, 0));
            return this;
        }
    }

    return NULL;
}
