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

#include <global.h>

/* Search the inventory of 'pl' for what matches best with params.
 * we use item_matched_string above - this gives us consistent behaviour
 * between many commands.  Return the best match, or NULL if no match.
 */
object * find_best_object_match(object *pl, char *params)
{
    object *tmp, *best = NULL;
    int     match_val = 0, tmpmatch;

    for (tmp = pl->inv; tmp; tmp = tmp->below)
    {
        if (IS_SYS_INVISIBLE(tmp))
            continue;
        if ((tmpmatch = item_matched_string(pl, tmp, params)) > match_val)
        {
            match_val = tmpmatch;
            best = tmp;
        }
    }
    return best;
}


int command_uskill(object *pl, char *params)
{
    if (!params)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "Usage: use_skill <skill name>");
        return 0;
    }
    if (pl->type == PLAYER)
        CONTR(pl)->rest_mode = 0;
    return use_skill(pl, params);
}

int command_rskill(object *pl, char *params)
{
    int skillno;

    if (!params)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "Usage: ready_skill <skill name>");
        return 0;
    }
    if (pl->type == PLAYER)
        CONTR(pl)->rest_mode = 0;
    skillno = lookup_skill_by_name(params);
    if (skillno == -1)
    {
        new_draw_info_format(NDI_UNIQUE, 0, pl, "Couldn't find the skill %s", params);
        return 0;
    }
    return change_skill(pl, skillno);
}


int command_egobind ( object *pl, char *params)
{
    object *mark;

    if(pl->type != PLAYER || !CONTR(pl))
        return 0;

    mark = find_marked_object(pl);

    if(!mark)
    {
        new_draw_info(NDI_UNIQUE, 0,pl, "First MARK an ego item, then type: /egobind ");
        return 0;
    }

    /* kein egoitem or previous bound */
    if(!QUERY_FLAG(mark, FLAG_IS_EGOITEM) || QUERY_FLAG(mark, FLAG_IS_EGOBOUND))
    {
        new_draw_info_format(NDI_UNIQUE, 0,pl, "Your marked item %s is not an unbound ego item!", query_name(mark));
        return 0;
    }

    if(!params)
    {
        new_draw_info_format(NDI_UNIQUE, 0,pl, "To bind the %s type: /egobind %d", query_name(mark), mark->count);
        return 0;

    }

    /* be sure we REALLY bind the marked and previous announced item! */
    if(mark->count != (uint32) strtoul(params, NULL, 10))
    {
        new_draw_info_format(NDI_UNIQUE, 0,pl, "The numbers don't match!\nTo bind the %s type: /egobind %d", query_name(mark), mark->count);
        return 0;
    }

    new_draw_info_format(NDI_UNIQUE, 0,pl, "You have bound the %s!", query_name(mark));
    create_ego_item(mark, pl->name, EGO_ITEM_BOUND_PLAYER);
    esrv_update_item (UPD_NAME, pl, mark);
    play_sound_player_only (CONTR(pl), SOUND_LEARN_SPELL, SOUND_NORMAL, 0, 0);

    return 1;
}

int command_apply(object *op, char *params)
{
    if (op->type == PLAYER)
        CONTR(op)->rest_mode = 0;
    if (!params)
    {
        player_apply_below(op);
        return 0;
    }
    else
    {
        enum apply_flag aflag   = 0;
        object         *inv;

        while (*params == ' ')
            params++;
        if (!strncmp(params, "-a ", 3))
        {
            aflag = AP_APPLY;
            params += 3;
        }
        if (!strncmp(params, "-u ", 3))
        {
            aflag = AP_UNAPPLY;
            params += 3;
        }
        while (*params == ' ')
            params++;

        inv = find_best_object_match(op, params);
        if (inv)
        {
            player_apply(op, inv, aflag, 0);
        }
        else
            new_draw_info_format(NDI_UNIQUE, 0, op, "Could not find any match to the %s.", params);
    }
    return 0;
}


#if 0
/* Command will drop all items that have not been locked */
int command_dropall(object *op, char *params)
{
    object *curinv, *nextinv;

    if (op->inv == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Nothing to drop!");
        return 0;
    }

    curinv = op->inv;

    /*
      This is the default.  Drops everything not locked or considered
      not something that should be dropped.
    */
    /*
      Care must be taken that the next item pointer is not to money as
      the drop() routine will do unknown things to it when dropping
      in a shop. --Tero.Pelander@utu.fi
    */

    if (params == NULL)
    {
        while (curinv != NULL)
        {
            nextinv = curinv->below;
            while (nextinv && nextinv->type == MONEY)
                nextinv = nextinv->below;
            if (!QUERY_FLAG(curinv, FLAG_INV_LOCKED)
             && curinv->type != MONEY
             && curinv->type != FOOD
             && curinv->type != KEY
             && curinv->type != SPECIAL_KEY
             && (curinv->type != TYPE_PEARL && curinv->type != GEM && curinv->type != TYPE_JEWEL && curinv->type != TYPE_NUGGET)
             && !IS_SYS_INVISIBLE(curinv)
             && (curinv->type != CONTAINER || (op->type == PLAYER && CONTR(op)->container != curinv)))
            {
                if (QUERY_FLAG(op, FLAG_STARTEQUIP))
                    drop(op, curinv);
            }
            curinv = nextinv;
        }
    }
    else if (strcmp(params, "weapons") == 0)
    {
        while (curinv != NULL)
        {
            nextinv = curinv->below;
            while (nextinv && nextinv->type == MONEY)
                nextinv = nextinv->below;
            if (!QUERY_FLAG(curinv, FLAG_INV_LOCKED)
             && ((curinv->type == WEAPON) || (curinv->type == BOW) || (curinv->type == ARROW)))
            {
                if (QUERY_FLAG(op, FLAG_STARTEQUIP))
                    drop(op, curinv);
            }
            curinv = nextinv;
        }
    }
    else if (strcmp(params, "armor") == 0 || strcmp(params, "armour") == 0)
    {
        while (curinv != NULL)
        {
            nextinv = curinv->below;
            while (nextinv && nextinv->type == MONEY)
                nextinv = nextinv->below;
            if (!QUERY_FLAG(curinv, FLAG_INV_LOCKED)
             && ((curinv->type == ARMOUR) || curinv->type == SHIELD || curinv->type == HELMET))
            {
                if (QUERY_FLAG(op, FLAG_STARTEQUIP))
                    drop(op, curinv);
            }
            curinv = nextinv;
        }
    }
    else if (strcmp(params, "misc") == 0)
    {
        while (curinv != NULL)
        {
            nextinv = curinv->below;
            while (nextinv && nextinv->type == MONEY)
                nextinv = nextinv->below;
            if (!QUERY_FLAG(curinv, FLAG_INV_LOCKED) && !QUERY_FLAG(curinv, FLAG_APPLIED))
            {
                switch (curinv->type)
                {
                    case HORN:
                    case BOOK:
                    case SPELLBOOK:
                    case GIRDLE:
                    case AMULET:
                    case RING:
                    case CLOAK:
                    case BOOTS:
                    case GLOVES:
                    case BRACERS:
                    case SCROLL:
                    case ARMOUR_IMPROVER:
                    case WEAPON_IMPROVER:
                    case WAND:
                    case ROD:
                    case POTION:
                      if (QUERY_FLAG(op, FLAG_STARTEQUIP))
                          drop(op, curinv);
                      curinv = nextinv;
                      break;
                    default:
                      curinv = nextinv;
                      break;
                }
            }
            curinv = nextinv;
        }
    }

    return 0;
}
#endif

/* Object op wants to drop object(s) params.  params can be a
 * comma seperated list.
 */

int command_drop(object *op, char *params)
{
    object *tmp, *next;
    int     did_one = 0;

    if (!params)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Drop what?");
        return 0;
    }
    else
    {
        for (tmp = op->inv; tmp; tmp = next)
        {
            next = tmp->below;
            if (QUERY_FLAG(tmp, FLAG_NO_DROP) || IS_SYS_INVISIBLE(tmp))
                continue;
            if (item_matched_string(op, tmp, params))
            {
                drop(op, tmp);
                did_one = 1;
            }
        }
        if (!did_one)
            new_draw_info(NDI_UNIQUE, 0, op, "Nothing to drop.");
    }
    if (op->type == PLAYER)
        CONTR(op)->count = 0;
    return 0;
}

int command_examine(object *op, char *params)
{
    if (op->type == PLAYER)
        CONTR(op)->rest_mode = 0;
    if (!params)
    {
        object *tmp = op->below;
        while (tmp && !LOOK_OBJ(tmp))
            tmp = tmp->below;
        if (tmp)
            examine(op, tmp, TRUE);
    }
    else
    {
        object *tmp = find_best_object_match(op, params);
        if (tmp)
            examine(op, tmp, TRUE);
        else
            new_draw_info_format(NDI_UNIQUE, 0, op, "Could not find an object that matches %s", params);
    }
    return 0;
}

/* Gecko: added a recursive part to search so that we also search in containers */
static object * find_marked_object_rec(object *op, object **marked, uint32 *marked_count)
{
    object *tmp, *tmp2;

    /* TODO: wouldn't it be more efficient to search the other way? That is:
     * start with the marked item, and search outwards through its env
     * until we find the player? Isn't env always cleared when an object
     * is removed from its container? */

    /* This may seem like overkill, but we need to make sure that they
     * player hasn't dropped the item.  We use count on the off chance that
     * an item got reincarnated at some point.
     */
    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        if (IS_SYS_INVISIBLE(tmp))
            continue;
        if (tmp == *marked)
        {
            if (tmp->count == *marked_count)
                return tmp;
            else
            {
                *marked = NULL;
                *marked_count = 0;
                return NULL;
            }
        }
        else if (tmp->inv)
        {
            tmp2 = find_marked_object_rec(tmp, marked, marked_count);
            if (tmp2)
                return tmp2;
            if (*marked == NULL)
                return NULL;
        }
    }
    return NULL;
}

/* op should be a player.
 * we return the object the player has marked with the 'mark' command
 * below.  If no match is found (or object has changed), we return
 * NULL.  We leave it up to the calling function to print messages if
 * nothing is found.
 */
object * find_marked_object(object *op)
{
    if (op->type != PLAYER)
        return NULL;

    if (!op || !CONTR(op))
        return NULL;
    if (!CONTR(op)->mark)
        return NULL;

    return find_marked_object_rec(op, &CONTR(op)->mark, &CONTR(op)->mark_count);
}


/* op is the player
 * tmp is the monster being examined.
 */
char *examine_monster(object *op, object *tmp, char *buf, int flag)
{
    object *mon = tmp->head ? tmp->head : tmp;
    float   dps;
    char   *gender, *att;
    int     val, val2, i;
    char    buf2[MAX_BUF];

    if (QUERY_FLAG(mon, FLAG_IS_MALE))
    {
        if (QUERY_FLAG(mon, FLAG_IS_FEMALE))
        {
            gender = "hermaphrodite";
            att = "It";
        }
        else
        {
            gender = "male";
            att = "He";
        }
    }
    else if (QUERY_FLAG(mon, FLAG_IS_FEMALE))
    {
        gender = "female";
        att = "She";
    }
    else
    {
        gender = "neuter";
        att = "It";
    }

    if (QUERY_FLAG(mon, FLAG_IS_GOOD))
    {
        sprintf(buf2,"%s is a good aligned %s %s%s.\n", att, gender, QUERY_FLAG(mon,FLAG_UNDEAD)?"undead ":"", mon->race);
        strcat(buf,buf2);
    }
    else if (QUERY_FLAG(mon, FLAG_IS_EVIL))
    {
        sprintf(buf2,"%s is a evil aligned %s %s%s.\n", att, gender, QUERY_FLAG(mon,FLAG_UNDEAD)?"undead ":"", mon->race);
        strcat(buf,buf2);
    }
    else if (QUERY_FLAG(mon, FLAG_IS_NEUTRAL))
    {
        sprintf(buf2,"%s is a neutral aligned %s %s%s.\n", att, gender, QUERY_FLAG(mon,FLAG_UNDEAD)?"undead ":"", mon->race);
        strcat(buf,buf2);
    }
    else
    {
        sprintf(buf2,"%s is a %s %s%s.\n", att, gender, QUERY_FLAG(mon,FLAG_UNDEAD)?"undead ":"", mon->race);
        strcat(buf,buf2);
    }

    if(flag)
    {
        if (mon->type == PLAYER)
        {
            sprintf(buf2,"%s is level %d and %d years old%s.\n", att, mon->level, CONTR(mon)->age,
                             QUERY_FLAG(mon, FLAG_IS_AGED) ? " (magically aged)" : "");
            strcat(buf,buf2);
        }
        else
        {
            sprintf(buf2,"%s is level %d%s.\n", att, mon->level,
                             QUERY_FLAG(mon, FLAG_IS_AGED) ? " and unnaturally aged" : "");
            strcat(buf,buf2);
        }
    }

    if (mon->type == PLAYER)
        dps = ((float)CONTR(mon)->dps)/10.0f;
    else /* calc it for a monster */
    {
        int tmp_dam=0;

        dps = (float)mon->stats.dam;
        if(mon->weapon_speed)
            dps /= mon->weapon_speed;
        for(i=0;i<=LAST_ATNR_ATTACK;i++)
            tmp_dam += mon->attack[i];
        tmp_dam += mon->attack[ATNR_INTERNAL];

        if(tmp_dam)
            dps *= ((float)tmp_dam)/100.0f;
    }

    sprintf(buf2,"%s attacks with %.1f dps and has %d hp", att, dps, mon->stats.maxhp);
    strcat(buf,buf2);
    if(QUERY_FLAG(mon,FLAG_READY_SPELL) || mon->type == PLAYER)
    {
        sprintf(buf2,",\nsp of %d and a sp recovery of %d", mon->stats.maxsp, mon->stats.Pow);
        strcat(buf,buf2);
    }
    sprintf(buf2,".\n%s has a wc of %d and an ac of %d.\n", att, mon->stats.wc, mon->stats.ac);
    strcat(buf,buf2);

    for (val = val2 = -1,i = 0; i < NROFATTACKS; i++)
    {
        if (mon->resist[i] > 0)
            val = i;
        else if (mon->resist[i] < 0)
            val = i;
    }
    if (val != -1)
    {
        sprintf(buf2,"%s can naturally resist some attacks.\n", att);
        strcat(buf,buf2);
    }
    if (val2 != -1)
    {
        sprintf(buf2, "%s is naturally vulnerable to some attacks.\n", att);
        strcat(buf,buf2);
    }

    if(flag && (mon->stats.hp + 1)) /* we use this also for general arch description */
    {
        switch ((mon->stats.hp + 1) * 4 / (mon->stats.maxhp + 1))
        {
            /* From 1-4 */
            case 1:
              sprintf(buf2,"%s is in a bad shape.\n", att);
              strcat(buf,buf2);
              break;
            case 2:
              sprintf(buf2,"%s is hurt.\n", att);
              strcat(buf,buf2);
              break;
            case 3:
              sprintf(buf2,"%s is somewhat hurt.\n", att);
              strcat(buf,buf2);
              break;
            default:
              sprintf(buf2,"%s is in excellent shape.\n", att);
              strcat(buf,buf2);
              break;
        }
    }
    if (present_in_ob(POISONING, mon) != NULL)
    {
        sprintf(buf2,"%s looks very ill.\n", att);
        strcat(buf,buf2);
    }

    if(op)
        new_draw_info_format(NDI_UNIQUE, 0, op, buf);

    return buf;
}

char * long_desc(object *tmp, object *caller)
{
    static char buf[VERY_BIG_BUF];
    char       *cp;

    if (tmp == NULL)
        return "";
    buf[0] = '\0';

    switch (tmp->type)
    {
        case RING:
        case SKILL:
        case WEAPON:
        case ARMOUR:
        case BRACERS:
        case HELMET:
        case SHOULDER:
        case LEGS:
        case SHIELD:
        case BOOTS:
        case GLOVES:
        case AMULET:
        case GIRDLE:
        case POTION:
        case BOW:
        case ARROW:
        case CLOAK:
        case FOOD:
        case DRINK:
        case HORN:
        case WAND:
        case ROD:
        case FLESH:
        case CONTAINER:
          if (*(cp = describe_item(tmp)) != '\0')
          {
              int   len;

              strncat(buf, query_name(tmp), VERY_BIG_BUF - 1);

              buf[VERY_BIG_BUF - 1] = 0;
              len = strlen(buf);
              if (len < VERY_BIG_BUF - 5 && ((tmp->type != AMULET && tmp->type != RING) || tmp->title))
              {
                  /* Since we know the length, we save a few cpu cycles by using
                            * it instead of calling strcat */
                  strcpy(buf + len, " ");
                  len++;
                  strncpy(buf + len, cp, VERY_BIG_BUF - len - 1);
                  buf[VERY_BIG_BUF - 1] = 0;
              }
          }
          break;
    }

    if (buf[0] == '\0')
    {
        strncat(buf, query_name_full(tmp, caller), VERY_BIG_BUF - 1);
        buf[VERY_BIG_BUF - 1] = 0;
    }

    return buf;
}

char *examine(object *op, object *tmp, int flag)
{
    char    *buf_out = global_string_buf4096;
    char    buf[VERY_BIG_BUF];
    char    tmp_buf[64];
    int     i;

    if (tmp == NULL || tmp->type == CLOSE_CON)
        return NULL;

    *buf_out='\0';
    if(op)
    {
        if(trigger_object_plugin_event(EVENT_EXAMINE,tmp,op,NULL,NULL,0,0,0,0)  && !QUERY_FLAG(op, FLAG_WIZ))
            return NULL;
    }

    /* Only quetzals can see the resistances on flesh. To realize
    this, we temporarily flag the flesh with SEE_INVISIBLE */
    if (op && op->type == PLAYER && tmp->type == FLESH && is_dragon_pl(op))
        SET_FLAG(tmp, FLAG_SEE_INVISIBLE);

    *buf='\0';
    if(flag)
    {
        strcpy(buf, "That is ");
        strncat(buf, long_desc(tmp, op), VERY_BIG_BUF - strlen(buf) - 1);
        buf[VERY_BIG_BUF - 1] = 0;
    }

    if (op && op->type == PLAYER && tmp->type == FLESH)
        CLEAR_FLAG(tmp, FLAG_SEE_INVISIBLE);

    /* only add this for usable items, not for objects like walls or floors for example */
    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) && need_identify(tmp))
        strncat(buf, " (unidentified)", VERY_BIG_BUF - strlen(buf) - 1);
    buf[VERY_BIG_BUF - 1] = 0;

    strcat(buf_out, buf);
    strcat(buf_out, "\n");
    buf[0] = '\0';

    if (QUERY_FLAG(tmp, FLAG_MONSTER) || (tmp && tmp->type == PLAYER))
    {
        strcat(buf_out, describe_item(tmp->head ? tmp->head : tmp));
        strcat(buf_out, "\n");
        examine_monster(NULL, tmp, buf_out, flag);
    }
    /* we don't double use the item_xxx arch commands, so they are always valid */
    else if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
    {
        /* if one of this is set, we have a ego item */
        if (QUERY_FLAG(tmp, FLAG_IS_GOOD))
            strcat(buf_out, "It is good aligned\n.");
        else if (QUERY_FLAG(tmp, FLAG_IS_EVIL))
            strcat(buf_out, "It is evil aligned\n.");
        else if (QUERY_FLAG(tmp, FLAG_IS_NEUTRAL))
            strcat(buf_out, "It is neutral aligned\n.");

        if (tmp->item_level)
        {
            if (tmp->item_skill)
            {
                sprintf(buf, "It needs a level of %d in %s to use.\n", tmp->item_level,
                    STRING_SAFE(CONTR(op)->exp_obj_ptr[tmp->item_skill-1]->name));
                strcat(buf_out, buf);
            }
            else
            {
                sprintf(buf, "It needs a level of %d to use.\n", tmp->item_level);
                strcat(buf_out, buf);
            }
        }

        if (tmp->item_quality)
        {
            int tmp_flag = 0;

            sprintf(buf, "Qua: %d Con: %d.\n", tmp->item_quality, tmp->item_condition);
            strcat(buf_out, buf);

            if (QUERY_FLAG(tmp, FLAG_PROOF_PHYSICAL))
                tmp_flag +=1;
            if (QUERY_FLAG(tmp, FLAG_PROOF_ELEMENTAL))
                tmp_flag +=2;
            if (QUERY_FLAG(tmp, FLAG_PROOF_MAGICAL))
                tmp_flag +=4;
            if (QUERY_FLAG(tmp, FLAG_PROOF_SPHERICAL))
                tmp_flag +=8;

            if(tmp_flag)
            {
                strcpy(buf, "It is ");

                if(tmp_flag == 15)
                {
                    strcat(buf, "indestructible");
                }
                else
                {
                    int ft  = 0;

                    if (QUERY_FLAG(tmp, FLAG_PROOF_PHYSICAL))
                    {
                        strcat(buf, "physical");
                        ft = 1;
                    }
                    if (QUERY_FLAG(tmp, FLAG_PROOF_ELEMENTAL))
                    {
                        if (ft)
                            strcat(buf, ", ");
                        strcat(buf, "elemental");
                        ft = 1;
                    }
                    if (QUERY_FLAG(tmp, FLAG_PROOF_MAGICAL))
                    {
                        if (ft)
                            strcat(buf, ", ");
                        strcat(tmp_buf, "magical");
                        ft = 1;
                    }
                    if (QUERY_FLAG(tmp, FLAG_PROOF_SPHERICAL))
                    {
                        if (ft)
                            strcat(buf, ", ");
                        strcat(tmp_buf, "spherical");
                        ft = 1;
                    }
                    strcat(buf, " proof");
                }
                strcat(buf, ".\n");
                strcat(buf_out, buf);
            }
            buf[0] = '\0';
        }
    }
    else /* not identified */
    {
        if (tmp->item_quality)
        {
            sprintf(buf, "Qua: ?? Con: %d.\n", tmp->item_condition);
            strcat(buf_out, buf);
        }
        buf[0] = '\0';
    }

    if (tmp->item_quality && !tmp->item_condition)
        strcat(buf_out, "Item is broken!\n");

    switch (tmp->type)
    {
        case SPELLBOOK:
          if (QUERY_FLAG(tmp, FLAG_IDENTIFIED) && tmp->stats.sp >= 0 && tmp->stats.sp <= NROFREALSPELLS)
          {
              if (tmp->sub_type1 == ST1_SPELLBOOK_CLERIC)
                  sprintf(buf, "%s is a %d level prayer.", spells[tmp->stats.sp].name, spells[tmp->stats.sp].level);
              else
                  sprintf(buf, "%s is a %d level spell.", spells[tmp->stats.sp].name, spells[tmp->stats.sp].level);
          }
          break;

        case BOOK:
          if (tmp->msg != NULL)
             sprintf(buf, "It is written in %s.\n", get_language(tmp->weight_limit));
          break;

        case CONTAINER:
          if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
          {
              if (tmp->race != NULL)
              {
                  if (tmp->weight_limit >0)
                      sprintf(buf, "It can hold only %s and its weight limit is %.1f kg.\n", tmp->race,
                              (float) tmp->weight_limit / 1000.0f);
                  else
                      sprintf(buf, "It can hold only %s.\n", tmp->race);

                  if (tmp->weapon_speed != 1.0f) /* has magic modifier? */
                  {
                      strcat(buf_out, buf);
                      if (tmp->weapon_speed > 1.0f) /* bad */
                          sprintf(buf, "It increases the weight of items inside by %.1f%%.\n", tmp->weapon_speed * 100.0f);
                      else /* good */
                          sprintf(buf, "It decreases the weight of items inside by %.1f%%.\n",
                                  100.0f - (tmp->weapon_speed * 100.0f));
                  }
              }
              else
              {
                  if (tmp->weight_limit > 0)
                  {
                      sprintf(buf, "Its weight limit is %.1f kg.\n", (float) tmp->weight_limit / 1000.0f);
                  }

                  if (tmp->weapon_speed != 1.0f) /* has magic modifier? */
                  {
                      strcat(buf_out, buf);
                      if (tmp->weapon_speed > 1.0f) /* bad */
                          sprintf(buf, "It increases the weight of items inside by %.1f%%.\n", tmp->weapon_speed * 100.0f);
                      else /* good */
                          sprintf(buf, "It decreases the weight of items inside by %.1f%%.\n",
                                  100.0f - (tmp->weapon_speed * 100.0f));
                  }
              }
              if (buf[0] != '\0')
                  strcat(buf_out, buf);
          }
          if(tmp->weapon_speed != 1.0f)
              sprintf(buf, "It contains %.1f kg reduced to %.1f kg.\n", (float) tmp->carrying / 1000.0f,
                      (float) tmp->damage_round_tag / 1000.0f);
          else
              sprintf(buf, "It contains %.1f kg.\n", (float) tmp->carrying / 1000.0f);
          break;

        case WAND:
          if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
              sprintf(buf, "It has %d charges left.\n", tmp->stats.food);
          break;

        case TYPE_LIGHT_APPLY:
          if (!tmp->last_sp || (tmp->last_eat && !tmp->stats.food))
              sprintf(buf, "%s no fuel remaining!\n", (tmp->nrof > 1) ? "They each have" : "It has");
          else if (!tmp->last_eat)
              sprintf(buf, "%s limitless fuel!\n", (tmp->nrof > 1) ? "They each have" : "It has");
          else
          {
              int minutes = tmp->stats.food / 36 + 1;
              sprintf(buf, "%s less than %d minute%s of fuel remaining!\n", (tmp->nrof > 1) ? "They each have" : "It has",
                      minutes, (minutes > 1) ? "s" : "");
          }
          break;
    }

    if (buf[0] != '\0')
        strcat(buf_out, buf);

    if (tmp->material && (need_identify(tmp) && QUERY_FLAG(tmp, FLAG_IDENTIFIED)))
    {
        strcpy(buf, "It's made of: ");
        for (i = 0; i < NROFMATERIALS; i++)
        {
            if (tmp->material & (1 << i))
            {
                strcat(buf, material[i].name);
                strcat(buf, " ");
            }
        }
        strcat(buf, "\n");
        strcat(buf_out, buf);
    }

    if (tmp->weight)
    {
        sprintf(buf, tmp->nrof > 1 ? "They weigh %3.3f kg.\n" : "It weighs %3.3f kg.\n",
            (float) (tmp->nrof ? tmp->weight * (sint32)tmp->nrof : tmp->weight) / 1000.0f);

        strcat(buf_out, buf);
    }

    if (QUERY_FLAG(tmp, FLAG_STARTEQUIP))
    {
        if (QUERY_FLAG(tmp, FLAG_UNPAID)) /* thats a unpaid clone shop item */
        {
            sprintf(buf, "%s would cost you %s.\n", tmp->nrof > 1 ? "They" : "It", query_cost_string(tmp, op, F_BUY, COSTSTRING_SHORT));
            strcat(buf_out, buf);
        }
        else /* it is a real one drop item */
        {
            sprintf(buf, "** ~NO-DROP item%s~ **\n", tmp->nrof > 1 ? "s" : "");
            strcat(buf_out, buf);
            if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            {
                if (tmp->value)
                    sprintf(buf, "But %s worth %s.\n", tmp->nrof > 1 ? "they are" : "it is",
                                         query_cost_string(tmp, op, F_TRUE, COSTSTRING_SHORT));
                else
                    sprintf(buf, "%s worthless.\n", tmp->nrof > 1 ? "They are" : "It is");
                strcat(buf_out, buf);
            }
        }
    }
    else if (tmp->value && !IS_LIVE(tmp))
    {
        if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            if (QUERY_FLAG(tmp, FLAG_UNPAID))
            {
                sprintf(buf, "%s would cost you %s.\n", tmp->nrof > 1 ? "They" : "It",
                                     query_cost_string(tmp, op, F_BUY, COSTSTRING_SHORT));
                strcat(buf_out, buf);
            }
            else
            {
                sprintf(buf, "%s worth %s.\n", tmp->nrof > 1 ? "They are" : "It is",
                                     query_cost_string(tmp, op, F_BUY, COSTSTRING_SHORT));
                strcat(buf_out, buf);
                goto dirty_little_jump1;
			}
        }
        else
        {
            object *floor;
            dirty_little_jump1 : floor = GET_MAP_OB_LAYER(op->map, op->x, op->y, 0);
            if (floor && floor->type == SHOP_FLOOR && tmp->type != MONEY)
            {/* disabled CHA effect for b4
				sprintf(buf, "This shop will pay you %s (%0.1f%%).",
					query_cost_string(tmp, op, F_SELL, COSTSTRING_SHORT), 20.0f + 100.0f);
			  */
				sprintf(buf, "This shop will pay you %s.", query_cost_string(tmp, op, F_SELL, COSTSTRING_SHORT));
                strcat(buf_out, buf);
            }
        }
    }
    else if (!IS_LIVE(tmp))
    {
        if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            if (QUERY_FLAG(tmp, FLAG_UNPAID))
                sprintf(buf, "%s would cost nothing.\n", tmp->nrof > 1 ? "They" : "It");
            else
                sprintf(buf, "%s worthless.\n", tmp->nrof > 1 ? "They are" : "It is");
            strcat(buf_out, buf);
        }
    }

    /* Does the object have a message?  Don't show message for all object
     * types - especially if the first entry is a match
     */
    if (tmp->msg && tmp->type != EXIT
     && tmp->type != GRAVESTONE
     && tmp->type != SIGN
     && tmp->type != BOOK
     && tmp->type != CORPSE
     && !QUERY_FLAG(tmp, FLAG_WALK_ON)
     && strncasecmp(tmp->msg, "@match", 7))
    {
        /* This is just a hack so when identifying hte items, we print
         * out the extra message
         */
        if (need_identify(tmp) && QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            strcat(buf_out, "The object has a story:\n");
            strcat(buf_out, tmp->msg);
        }
    }

    if(op)
    {
        new_draw_info(NDI_UNIQUE, 0, op, buf_out);

        /* Examining signs also applies them (so the player can see what is on
         * them). This is because for signs their whole raison d'etre is to be
         * read and it is logical to equate players examining a sign with them
         * actually reading it. So it is not a useful UI feature to expect
         * players to differentiate between apply and examine in this case.
         * However, internally they are still separate actions. A player
         * examining a sign will generate an EXAMINE event followed by an APPLY
         * event, but a player applying a sign will only generate an APPLY
         * event. -- Smacky 20080426 */
        if (tmp->type == SIGN ||
            tmp->type == GRAVESTONE)
            manual_apply(op, tmp, 0);

        if (QUERY_FLAG(op, FLAG_WIZ))
        {
            dump_object(tmp);
            new_draw_info(NDI_UNIQUE, 0, op, errmsg);
        }
    }
    return buf_out;
}

/*
 * inventory prints object's inventory. If inv==NULL then print player's
 * inventory.
 * [ Only items which are applied are showed. Tero.Haatanen@lut.fi ]
 */
void inventory(object *op, object *inv)
{
    object *tmp;
    char   *in;
    int     items = 0, length;

    if (inv == NULL && op == NULL)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "Inventory of what object?");
        return;
    }
    tmp = inv ? inv->inv : op->inv;

    while (tmp)
    {
        if ((!IS_SYS_INVISIBLE(tmp) && (inv == NULL || inv->type == CONTAINER || QUERY_FLAG(tmp, FLAG_APPLIED)))
         || (!op || QUERY_FLAG(op, FLAG_WIZ)))
            items++;
        tmp = tmp->below;
    }
    if (inv == NULL)
    {
        /* player's inventory */
        if (items == 0)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "You carry nothing.");
            return;
        }
        else
        {
            length = 28;
            in = "";
            new_draw_info(NDI_UNIQUE, 0, op, "Inventory:");
        }
    }
    else
    {
        if (items == 0)
            return;
        else
        {
            length = 28;
            in = "  ";
        }
    }
    for (tmp = inv ? inv->inv : op->inv; tmp; tmp = tmp->below)
    {
        if ((!op || !QUERY_FLAG(op, FLAG_WIZ))
         && (IS_SYS_INVISIBLE(tmp) || (inv && inv->type != CONTAINER && !QUERY_FLAG(tmp, FLAG_APPLIED))))
            continue;
        if ((!op || QUERY_FLAG(op, FLAG_WIZ)))
            new_draw_info_format(NDI_UNIQUE, 0, op, "%s- %-*.*s (%5d) %-8s", in, length, length, query_name(tmp),
                                 tmp->count, query_weight(tmp));
        else
            new_draw_info_format(NDI_UNIQUE, 0, op, "%s- %-*.*s %-8s", in, length + 8, length + 8, query_name(tmp),
                                 query_weight(tmp));
    }
    if (!inv && op)
    {
        new_draw_info_format(NDI_UNIQUE, 0, op, "%-*s %-8s", 41, "Total weight :", query_weight(op));
    }
}
