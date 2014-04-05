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

int command_uskill(object *pl, char *params)
{
    if (!params)
        return 1;

    if (pl->type == PLAYER)
        CONTR(pl)->rest_mode = 0;

    if (!use_skill(pl, params))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }
    else
    {
        return COMMANDS_RTN_VAL_OK;
    }
}

int command_rskill(object *pl, char *params)
{
    int skillno;

    if (!params)
        return 1;

    if (pl->type == PLAYER)
        CONTR(pl)->rest_mode = 0;

    if ((skillno = lookup_skill_by_name(params)) == -1)
    {
        new_draw_info(NDI_UNIQUE, 0, pl, "Couldn't find the skill %s", params);

        return 0;
    }

    if (!change_skill(pl, skillno))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }
    else
    {
        return COMMANDS_RTN_VAL_OK;
    }
}


int command_egobind ( object *pl, char *params)
{
    object *mark;

    if(pl->type != PLAYER || !CONTR(pl))
        return 0;

    if (!(mark = find_marked_object(pl)))
    {
        new_draw_info(NDI_UNIQUE, 0,pl, "First MARK an ego item, then type: /egobind ");

        return 0;
    }

    /* kein egoitem or previous bound */
    if(!QUERY_FLAG(mark, FLAG_IS_EGOITEM) || QUERY_FLAG(mark, FLAG_IS_EGOBOUND))
    {
        new_draw_info(NDI_UNIQUE, 0,pl, "Your marked item %s is not an unbound ego item!", query_name_full(mark, NULL));

        return 0;
    }

    if(!params)
    {
        new_draw_info(NDI_UNIQUE, 0,pl, "To bind %s type: /egobind %d", query_name_full(mark, pl), mark->count);

        return 0;
    }

    /* be sure we REALLY bind the marked and previous announced item! */
    if(mark->count != (uint32) strtoul(params, NULL, 10))
    {
        new_draw_info(NDI_UNIQUE, 0,pl, "The numbers don't match!\nTo bind %s type: /egobind %d", query_name_full(mark, pl), mark->count);

        return 0;
    }

    new_draw_info(NDI_UNIQUE, 0,pl, "You have bound %s!", query_name_full(mark, pl));
    create_ego_item(mark, pl->name, EGO_ITEM_BOUND_PLAYER);
    esrv_update_item(UPD_NAME, mark);
    play_sound_player_only (CONTR(pl), SOUND_LEARN_SPELL, SOUND_NORMAL, 0, 0);

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
        if (QUERY_FLAG(tmp, FLAG_SYS_OBJECT))
        {
            continue;
        }

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
    object *mon = tmp->head ? tmp->head : tmp,
           *walk;
    float   dps;
    char   *gender, *att;
    int     val, val2, i;
    char    buf2[MEDIUM_BUF];

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

    for (walk = mon->inv; walk; walk = walk->below)
    {
        if (walk->type == FORCE &&
            walk->sub_type1 == ST1_FORCE_POISON)
        {
            sprintf(strchr(buf, '\0'), "%s looks very ill.\n", att);
        }
    }

    if(op)
        new_draw_info(NDI_UNIQUE, 0, op, "%s", buf);

    return buf;
}

char * long_desc(object *tmp, object *caller)
{
    static char buf[LARGE_BUF];
    char       *cp;

    if (tmp == NULL)
        return "";
    buf[0] = '\0';

    switch (tmp->type)
    {
        case RING:
        case TYPE_SKILL:
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

              strncat(buf, query_name_full(tmp, caller), LARGE_BUF - 1);

              buf[LARGE_BUF - 1] = 0;
              len = strlen(buf);
              if (len < LARGE_BUF - 5 && ((tmp->type != AMULET && tmp->type != RING) || tmp->title))
              {
                  /* Since we know the length, we save a few cpu cycles by using
                            * it instead of calling strcat */
                  strcpy(buf + len, " ");
                  len++;
                  strncpy(buf + len, cp, LARGE_BUF - len - 1);
                  buf[LARGE_BUF - 1] = 0;
              }
          }
          break;
    }

    if (buf[0] == '\0')
    {
        strncat(buf, query_name_full(tmp, caller), LARGE_BUF - 1);
        buf[LARGE_BUF - 1] = 0;
    }

    return buf;
}

char *examine(object *op, object *tmp, int flag)
{
    char    *buf_out = global_string_buf4096;
    char    buf[LARGE_BUF];
    char    tmp_buf[TINY_BUF];
    int     i;

    if (tmp == NULL || tmp->type == CLOSE_CON)
        return NULL;

    *buf_out='\0';
    if(op)
    {
        if (trigger_object_plugin_event(EVENT_EXAMINE, tmp, op, NULL, NULL, 0,
                                        0, 0, 0) &&
             !IS_GMASTER_WIZ(op))
        {
            return NULL;
        }
    }

    /* Only quetzals can see the resistances on flesh. To realize
    this, we temporarily flag the flesh with SEE_INVISIBLE */
    if (op && op->type == PLAYER && tmp->type == FLESH && is_dragon_pl(op))
        SET_FLAG(tmp, FLAG_SEE_INVISIBLE);

    *buf='\0';
    if(flag)
    {
        sprintf(buf, "That is %s", long_desc(tmp, op));
    }

    if (op && op->type == PLAYER && tmp->type == FLESH)
        CLEAR_FLAG(tmp, FLAG_SEE_INVISIBLE);

    /* only add this for usable items, not for objects like walls or floors for example */
    if (!QUERY_FLAG(tmp, FLAG_IDENTIFIED) &&
        need_identify(tmp))
    {
        sprintf(strchr(buf, '\0'), " (unidentified)");
    }

    if (tmp->type == TYPE_LIGHT_APPLY)
    {
        sprintf(strchr(buf, '\0'), " (%s)",
                (tmp->glow_radius) ? "lit" : "extinguished");
    }

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
                    STRING_SAFE(CONTR(op)->skillgroup_ptr[tmp->item_skill-1]->name));
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

    if (QUERY_FLAG(tmp, FLAG_NO_DROP))
    {
        sprintf(buf, "** ~NO-DROP item%s~ **\n", tmp->nrof > 1 ? "s" : "");
        strcat(buf_out, buf);
    }

    if (!IS_LIVE(tmp))
    {
        if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            sprintf(buf, "You value %s at ~%s~.\n",
                (tmp->nrof > 1) ? "them" : "it",
                (tmp->value) ? query_cost_string(tmp, op, F_TRUE, COSTSTRING_SHORT) : "nothing");
            strcat(buf_out, buf);
        }

        if (tmp->type != MONEY)
        {
            MapSpace *msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y);
            object *shop;

            GET_MAP_SPACE_SYS_OBJ(msp, SHOP_FLOOR, shop);

            if (shop)
            {
                if (QUERY_FLAG(tmp, FLAG_UNPAID))
                {
                    sprintf(buf, "You can buy %s for ~%s~ from %s.\n",
                        (tmp->nrof > 1) ? "them" : "it",
                        query_cost_string(tmp, op, F_BUY, COSTSTRING_SHORT),
                        query_name_full(shop, NULL));
                }
                else
                {
                    sprintf(buf, "You can sell %s for ~%s~ to %s.\n",
                        (tmp->nrof > 1) ? "them" : "it",
                        query_cost_string(tmp, op, F_SELL, COSTSTRING_SHORT),
                        query_name_full(shop, NULL));
                }

                strcat(buf_out, buf);
            }
        }
    }

    if (tmp->msg)
    {
        /* Don't show message for all objects. */
        if (!(tmp->type == EXIT ||
              tmp->type == GRAVESTONE ||
              tmp->type == BOOK ||
              tmp->type == CORPSE) ||
            (!QUERY_FLAG(tmp, FLAG_WALK_ON) &&
             tmp->type == SIGN))
        {
            /* This is just a hack so when identifying hte items, we print
             * out the extra message.  */
            if (need_identify(tmp) &&
                QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            {
                /* For non-applyable applyable lights we *do* want the
                 * message but *do not* want the story line. */
                if (tmp->type != TYPE_LIGHT_APPLY ||
                    !QUERY_FLAG(tmp, FLAG_NO_FIX_PLAYER))
                {
                    sprintf(strchr(buf_out, '\0'), "The object has a story:\n");
                }

                sprintf(strchr(buf_out, '\0'), "%s", tmp->msg);
            }
        }
    }
    /* Applyable applyable lights with no msg have default instructions. */
    else if (tmp->type == TYPE_LIGHT_APPLY &&
             !QUERY_FLAG(tmp, FLAG_NO_FIX_PLAYER))
    {
        if (!tmp->glow_radius)
        {
            sprintf(strchr(buf_out, '\0'), "Apply to attempt to light.");
        }
        else if (tmp->env &&
                 tmp->env->type == PLAYER &&
                 !QUERY_FLAG(tmp, FLAG_APPLIED))
        {
            sprintf(strchr(buf_out, '\0'), "Apply to attempt to use as your light source");
        }
        else
        {
            sprintf(strchr(buf_out, '\0'), "Apply to attempt to extinguish.");
        }
    }

    if(op)
    {
        new_draw_info(NDI_UNIQUE, 0, op, "%s", buf_out);

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

        if (IS_GMASTER_WIZ(op))
        {
            dump_object(tmp);
            new_draw_info(NDI_UNIQUE, 0, op, "%s", errmsg);
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
        if ((!QUERY_FLAG(tmp, FLAG_SYS_OBJECT) &&
             (inv == NULL ||
              inv->type == CONTAINER ||
              QUERY_FLAG(tmp, FLAG_APPLIED))) ||
            (!op ||
             IS_GMASTER_WIZ(op)))
        {
            items++;
        }

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

    if (op &&
        IS_GMASTER_WIZ(op))
    {
        for (tmp = (inv) ? inv->inv : op->inv; tmp; tmp = tmp->below)
        {
            if ((QUERY_FLAG(tmp, FLAG_SYS_OBJECT) ||
                 (inv &&
                  inv->type != CONTAINER &&
                  !QUERY_FLAG(tmp, FLAG_APPLIED))))
            {
                continue;
            }

            new_draw_info(NDI_UNIQUE, 0, op, "%s- %-*.*s (%5d) %-8s",
                          in, length, length, query_name_full(tmp, op), tmp->count,
                          query_weight(tmp));
        }

        if (!inv)
        {
            new_draw_info(NDI_UNIQUE, 0, op, "%-*s %-8s", 41, "Total weight :", query_weight(op));
        }
    }
}
