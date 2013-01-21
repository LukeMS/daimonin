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

/* This containes item logic for client/server.  IT doesn't contain
 * the actual commands that send the data, but does contain
 * the logic for what items should be sent.
 */


#include <global.h>

static object  *esrv_get_ob_from_count_DM(object *pl, tag_t count);
static int      check_container(object *pl, object *con);

/* This is the maximum number of bytes we expect any one item to take up */
#define MAXITEMLEN  300

/* thats combinded with another strlen() in the item code - its an artifact from cf.
 * we will remove and fix this when we rewrite this module.
 */
inline void add_stringlen_to_sockbuf(char *buf, sockbuf_struct *sl)
{
    int len;

    len = strlen(buf);
    if (len > 254)
        len = 254;
    SockBuf_AddChar(sl, len+1);
    SockBuf_AddString(sl, buf, len);
}

/*******************************************************************************
 *
 * Functions related to sending object data to the client.
 *
 ******************************************************************************/

/*
 *  This is a similar to query_name, but returns flags
 *  to be sended to client.
 */
unsigned int query_flags(object *op)
{
    unsigned int flags = 0;

    if (QUERY_FLAG(op, FLAG_APPLIED))
    {
        switch (op->type)
        {
            case BOW:
            case WAND:
            case ROD:
            case HORN:
              flags = a_readied;
              break;
            case WEAPON:
              flags = a_wielded;
              break;
            case SKILL:
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
              flags = a_worn;
              break;
            case CONTAINER:
              flags = a_active;
              break;
            default:
              flags = a_applied;
              break;
        }
    }

    if (op->type == CONTAINER && (op->attacked_by || (!op->env && QUERY_FLAG(op, FLAG_APPLIED))))
        flags |= F_OPEN;

    if (QUERY_FLAG(op, FLAG_IS_TRAPED))
        flags |= F_TRAPED;
    if (QUERY_FLAG(op, FLAG_KNOWN_CURSED))
    {
        if (QUERY_FLAG(op, FLAG_DAMNED))
            flags |= F_DAMNED;
        else if (QUERY_FLAG(op, FLAG_CURSED))
            flags |= F_CURSED;
    }
    if (QUERY_FLAG(op, FLAG_KNOWN_MAGICAL) && is_magical(op))
        flags |= F_MAGIC;
    if (QUERY_FLAG(op, FLAG_UNPAID))
        flags |= F_UNPAID;
    if (QUERY_FLAG(op, FLAG_INV_LOCKED))
        flags |= F_LOCKED;
    if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
        flags |= F_INVISIBLE;
    if (QUERY_FLAG(op, FLAG_IS_ETHEREAL))
        flags |= F_ETHEREAL;
    return flags;
}


/* draw the look window.  Don't need to do animations here
 * This sends all the faces to the client, not just updates.  This is
 * because object ordering would otherwise be inconsistent
 */

void esrv_draw_look(object *pl)
{
    NewSocket  *ns = &CONTR(pl)->socket;
    sockbuf_struct *sbptr;
    char       *tmp_sp;
    object     *head, *tmp, *last;
    int         len, flags, got_one = 0, anim_speed, start_look = 0, end_look = 0;
    char        buf[MEDIUM_BUF];

    /* change out_of_map to OUT_OF_REAL_MAP(). we don't even think here about map crossing */
    if (QUERY_FLAG(pl, FLAG_REMOVED)
     || pl->map == NULL
     || pl->map->in_memory != MAP_ACTIVE
     || OUT_OF_REAL_MAP(pl->map,pl->x,pl->y))
        return;

    /*LOG(llevNoLog,"send look of: %s\n", query_name(pl));*/
    /* another layer feature: grap last (top) object without browsing the objects */
    tmp = GET_MAP_OB_LAST(pl->map, pl->x, pl->y);

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);
    sbptr = ACTIVE_SOCKBUF(ns); /* we have *some* to do here, perhaps the compiler can use a native ptr better */

    SockBuf_AddInt(sbptr, 0);
    SockBuf_AddInt(sbptr, 0);

    if (ns->look_position)
    {
        SockBuf_AddInt(sbptr, 0x80000000 | (ns->look_position - NUM_LOOK_OBJECTS));
        SockBuf_AddInt(sbptr, 0);
        SockBuf_AddInt(sbptr, -1);
        SockBuf_AddInt(sbptr, prev_item_face->number);
        SockBuf_AddChar(sbptr, 0);
        sprintf(buf, "A'pply (click) to see %d previous items", NUM_LOOK_OBJECTS);
        add_stringlen_to_sockbuf(buf, sbptr);
        sbptr = ACTIVE_SOCKBUF(ns);
        SockBuf_AddShort(sbptr, 0);
        SockBuf_AddChar(sbptr, 0);
        SockBuf_AddInt(sbptr, 0);
    }

    for (last = NULL; tmp != last; tmp = tmp->below)
    {
        if (tmp == pl)
            continue;

        /* Shop floors should not be seen in below window */
        if (tmp->type == SHOP_FLOOR)
            continue;

        /* skip map mask, sys_objects and invisible objects when we can't see them */
        if (tmp->layer <= 0
         || tmp->layer == 7 // Layer 7 isn't "below" the player, so it shouldn't be in the "below" window.
         || IS_SYS_INVISIBLE(tmp)
         || (!QUERY_FLAG(pl, FLAG_SEE_INVISIBLE) && QUERY_FLAG(tmp, FLAG_IS_INVISIBLE)))
        {
            /* but only when we are not a active DM */
            if (!QUERY_FLAG(pl, FLAG_WIZ))
                continue;
        }

        /* skip all items we had send before of the 'max shown items of a tile space' */
        if (++start_look < ns->look_position)
            continue;
        /* if we have to much items to send, send a 'next group' object and leave here */
        if (++end_look > NUM_LOOK_OBJECTS)
        {
            SockBuf_AddInt(sbptr, 0x80000000 | (ns->look_position + NUM_LOOK_OBJECTS));
            SockBuf_AddInt(sbptr, 0);
            SockBuf_AddInt(sbptr, -1);
            SockBuf_AddInt(sbptr, next_item_face->number);
            SockBuf_AddChar(sbptr, 0);
            sprintf(buf, "A'pply (click) to see next group of items");
            add_stringlen_to_sockbuf(buf, sbptr);
            sbptr = ACTIVE_SOCKBUF(ns);
            SockBuf_AddShort(sbptr, 0);
            SockBuf_AddChar(sbptr, 0);
            SockBuf_AddInt(sbptr, 0);
            break;
        }

        /* ok, now we start sending this item here */
        flags = query_flags(tmp);
        if (QUERY_FLAG(tmp, FLAG_NO_PICK))
            flags |= F_NOPICK;

        SockBuf_AddInt(sbptr, tmp->count);
        SockBuf_AddInt(sbptr, flags);

        if(QUERY_FLAG(tmp, FLAG_NO_PICK))
        {
            SockBuf_AddInt(sbptr, -1);
        }
        else
        {
            if(tmp->type == CONTAINER && tmp->weapon_speed != 1.0f) /* magical containers */
            {
                SockBuf_AddInt(sbptr, tmp->damage_round_tag + tmp->weight);
            }
            else
            {
                SockBuf_AddInt(sbptr, WEIGHT(tmp) );
            }
        }
        if (tmp->head)
        {
            if (tmp->head->inv_face && QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            {
                SockBuf_AddInt(sbptr, tmp->head->inv_face->number);
            }
            else
            {
                SockBuf_AddInt(sbptr, tmp->head->face->number);
            }
        }
        else
        {
            if (tmp->inv_face && QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            {
                SockBuf_AddInt(sbptr,tmp->inv_face->number);
            }
            else
            {
                SockBuf_AddInt(sbptr, tmp->face->number);
            }
        }
        SockBuf_AddChar(sbptr, tmp->facing);

        if (tmp->head)
            head = tmp->head;
        else
            head = tmp;

        len = (int)strlen((tmp_sp = query_base_name(head, pl)))+1;
        if (len > 128)
        {
            len = 128; /* 127 chars + 0 marker */
            SockBuf_AddChar(sbptr, len);
            SockBuf_AddString(sbptr, tmp_sp, len-1);
        }
        else
        {
            SockBuf_AddChar(sbptr, len+1);
            SockBuf_AddString(sbptr, tmp_sp, len);
        }

        /* handle animations... this will change 100% when we add client
             * sided animations.
             */
        SockBuf_AddShort(sbptr, tmp->animation_id);
        anim_speed = 0;
        if (QUERY_FLAG(tmp, FLAG_ANIMATE))
        {
            if (tmp->anim_speed)
                anim_speed = tmp->anim_speed;
            else
            {
                if (FABS(tmp->speed) < 0.001)
                    anim_speed = 255;
                else if (FABS(tmp->speed) >= 1.0)
                    anim_speed = 1;
                else
                    anim_speed = (int) (1.0 / FABS(tmp->speed));
            }
            if (anim_speed > 255)
                anim_speed = 255;
        }
        SockBuf_AddChar(sbptr, anim_speed);
        SockBuf_AddInt(sbptr, tmp->nrof);
        got_one++;

        /* We do a special for DMs - forcing the
             * inventory of ALL objects we send here... This is a
             * wonderful feature for controling & find bugs.
             */
        if (QUERY_FLAG(pl, FLAG_WIZ))
        {
            if (tmp->inv)
            {
               got_one = esrv_draw_DM_inv(pl, tmp);
               /* 2008-08-28 Alderan:
                * Fix double mempool-free bug
                * We MUST re-get the pointer to the buffer, because the function could
                * (and will in some circumstances) resize the buffer.
                */
               sbptr = ACTIVE_SOCKBUF(ns);
            }
        }
    } /* for loop */

    if (got_one || (!got_one && !ns->below_clear))
    {
        SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_ITEMY, SOCKBUF_DYNAMIC);
        ns->below_clear=0;
    }
    else
        SOCKBUF_REQUEST_RESET(ns);
}


/* used for a active DM - implicit sending the inventory of all
 * items we see in inventory & in below. For controling & debug.
 * Do a examine cmd over the item and you will see a dump.
 */
int esrv_draw_DM_inv(object *pl, object *op)
{
    NewSocket  *ns = &CONTR(pl)->socket;
    sockbuf_struct *sbptr = ACTIVE_SOCKBUF(ns);
    char   *tmp_sp, *tmp_in_inv = "in inventory", *tmp_end_inv = "end of inventory";
    object *tmp, *head;
    int     got_one = 0, flags, len, anim_speed;

    SockBuf_AddInt(sbptr, 0);
    SockBuf_AddInt(sbptr, 0);
    SockBuf_AddInt(sbptr, -1);
    SockBuf_AddInt(sbptr, blank_face->number);
    len = strlen(tmp_in_inv);
    SockBuf_AddChar(sbptr, len+1);
    add_stringlen_to_sockbuf(tmp_in_inv, sbptr);
    sbptr = ACTIVE_SOCKBUF(ns);
    SockBuf_AddShort(sbptr, 0);
    SockBuf_AddChar(sbptr, 0);
    SockBuf_AddInt(sbptr, 0);

    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        flags = query_flags(tmp);
        if (QUERY_FLAG(tmp, FLAG_NO_PICK))
            flags |= F_NOPICK;

        SockBuf_AddInt(sbptr, tmp->count);
        SockBuf_AddInt(sbptr, flags);

        if(QUERY_FLAG(tmp, FLAG_NO_PICK))
        {
            SockBuf_AddInt(sbptr, -1);
        }
        else
        {
            if(tmp->type == CONTAINER && tmp->weapon_speed != 1.0f)
            {
                SockBuf_AddInt(sbptr, tmp->damage_round_tag + tmp->weight);
            }
            else
            {
                SockBuf_AddInt(sbptr, WEIGHT(tmp) );
            }
        }

        if (tmp->head)
        {
            SockBuf_AddInt(sbptr, tmp->head->face->number);
        }
        else
        {
            SockBuf_AddInt(sbptr, tmp->face->number);
        }
        SockBuf_AddChar(sbptr, tmp->facing);

        if (tmp->head)
            head = tmp->head;
        else
            head = tmp;

        len = strlen((tmp_sp = query_base_name(head, pl))) + 1; /* +1 = 0 marker for string end */
        if (len > 128)
        {
            len = 128; /* 127 chars + 0 marker */
            SockBuf_AddChar(sbptr, len);
            SockBuf_AddString(sbptr, tmp_sp, len-1);
        }
        else
        {
            SockBuf_AddChar(sbptr, len+1);
            SockBuf_AddString(sbptr, tmp_sp, len);
        }

        /* handle animations... this will change 100% when we add client
             * sided animations.
             */
        SockBuf_AddShort(sbptr, tmp->animation_id);
        anim_speed = 0;
        if (QUERY_FLAG(tmp, FLAG_ANIMATE))
        {
            if (tmp->anim_speed)
                anim_speed = tmp->anim_speed;
            else
            {
                if (FABS(tmp->speed) < 0.001)
                    anim_speed = 255;
                else if (FABS(tmp->speed) >= 1.0)
                    anim_speed = 1;
                else
                    anim_speed = (int) (1.0 / FABS(tmp->speed));
            }
            if (anim_speed > 255)
                anim_speed = 255;
        }
        SockBuf_AddChar(sbptr, anim_speed);
        SockBuf_AddInt(sbptr, tmp->nrof);
        got_one++;

        if (tmp->inv) /* oh well... another container to flush */
        {
            got_one = esrv_draw_DM_inv(pl, tmp);
            /* 2008-08-28 Alderan: fix for double mempool-free bug */
            sbptr = ACTIVE_SOCKBUF(ns);
        }
    } /* for loop */

    SockBuf_AddInt(sbptr, 0);
    SockBuf_AddInt(sbptr, 0);
    SockBuf_AddInt(sbptr, -1);
    SockBuf_AddInt(sbptr, blank_face->number);
    len = strlen(tmp_end_inv);
    SockBuf_AddChar(sbptr, len+1);
    add_stringlen_to_sockbuf(tmp_end_inv, sbptr);
    sbptr = ACTIVE_SOCKBUF(ns);
    SockBuf_AddShort(sbptr, 0);
    SockBuf_AddChar(sbptr, 0);
    SockBuf_AddInt(sbptr, 0);
    return got_one;
}

void esrv_close_container(object *op)
{
    NewSocket *ns = &CONTR(op)->socket;

    /*LOG(llevNoLog,"close container of: %s\n", query_name(op));*/
    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_SMALL);
    SockBuf_AddInt(ACTIVE_SOCKBUF(ns), -1); /* container mode flag */
    SockBuf_AddInt(ACTIVE_SOCKBUF(ns), -1);
    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_ITEMX, SOCKBUF_DYNAMIC);
}


static int esrv_send_inventory_DM(object *pl, object *op)
{
    sockbuf_struct *sbptr = ACTIVE_SOCKBUF(&CONTR(pl)->socket);
    object *tmp;
    int     flags, got_one = 0, anim_speed, len;
    char    item_n[MEDIUM_BUF];

    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        flags = query_flags(tmp);
        if (QUERY_FLAG(tmp, FLAG_NO_PICK))
            flags |= F_NOPICK;

        SockBuf_AddInt(sbptr, tmp->count);
        SockBuf_AddInt(sbptr, flags);

        if(QUERY_FLAG(tmp, FLAG_NO_PICK))
        {
            SockBuf_AddInt(sbptr, -1);
        }
        else
        {
            if(tmp->type == CONTAINER && tmp->weapon_speed != 1.0f)
            {
                SockBuf_AddInt(sbptr, tmp->damage_round_tag + tmp->weight);
            }
            else
            {
                SockBuf_AddInt(sbptr, WEIGHT(tmp) );
            }
        }

        if (tmp->inv_face && QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            SockBuf_AddInt(sbptr, tmp->inv_face->number);
        }
        else
        {
            SockBuf_AddInt(sbptr, tmp->face->number);
        }

        SockBuf_AddChar(sbptr, tmp->facing);
        SockBuf_AddChar(sbptr, tmp->type);
        SockBuf_AddChar(sbptr, tmp->sub_type1);
        if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
        {
            SockBuf_AddChar(sbptr, tmp->item_quality);
            SockBuf_AddChar(sbptr, tmp->item_condition);
            SockBuf_AddChar(sbptr, tmp->item_level);
            SockBuf_AddChar(sbptr, tmp->item_skill);
        }
        else
        {
            SockBuf_AddInt(sbptr, 0xffffffff);
        }
        strncpy(item_n, query_base_name(tmp, pl), 127);
        item_n[127] = 0;
        len = strlen(item_n);
        SockBuf_AddChar(sbptr, len+1);
        SockBuf_AddString(sbptr, item_n, len);
        if (tmp->inv_animation_id)
        {
            SockBuf_AddShort(sbptr, tmp->inv_animation_id);
        }
        else
        {
            SockBuf_AddShort(sbptr, tmp->animation_id);
        }
        /* i use for both the same anim_speed - when we need a different,
             * i adding inv_anim_speed.
             */
        anim_speed = 0;
        if (QUERY_FLAG(tmp, FLAG_ANIMATE))
        {
            if (tmp->anim_speed)
                anim_speed = tmp->anim_speed;
            else
            {
                if (FABS(tmp->speed) < 0.001)
                    anim_speed = 255;
                else if (FABS(tmp->speed) >= 1.0)
                    anim_speed = 1;
                else
                    anim_speed = (int) (1.0 / FABS(tmp->speed));
            }
            if (anim_speed > 255)
                anim_speed = 255;
        }
        SockBuf_AddChar(sbptr, anim_speed);
        SockBuf_AddInt(sbptr, tmp->nrof);
        got_one++;
    }
    return got_one;
}

/* send_inventory send the inventory for the player BUT also the inventory for
 * items. When the player obens a chest on the ground, this function is called to
 * send the inventory for the chest - and the items are shown in below. The client
 * should take care, that the items in the below windows shown can be changed here
 * too without calling the function for the look window.
 */
/* we have here no invisible flag - we can "see" invisible when we have it in the inventory.
 * that simulate the effect that we can "feel" the object in our hands.
 * but when we drop it, it rolls out of sight and vanish...
 */
void esrv_send_inventory(object *pl, object *op)
{
    NewSocket  *ns = &CONTR(pl)->socket;
    sockbuf_struct *sbptr;
    object     *tmp;
    int         flags, got_one = 0, anim_speed, len;
    char        item_n[MEDIUM_BUF];

    /*LOG(llevDebug,"send inventory of: %s\n", query_name(op));*/
    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);
    sbptr = ACTIVE_SOCKBUF(ns);

    if (pl != op) /* in this case we send a container inventory! */
    {
        SockBuf_AddInt(sbptr, -1); /* container mode flag */
    }
    else
    {
        SockBuf_AddInt(sbptr, op->count);
    }

    SockBuf_AddInt(sbptr, op->count);

    for (tmp = op->inv; tmp; tmp = tmp->below)
    {
        if (!QUERY_FLAG(pl, FLAG_SEE_INVISIBLE) && QUERY_FLAG(tmp, FLAG_IS_INVISIBLE))
        {
            /* skip this for DMs */
            if (!QUERY_FLAG(pl, FLAG_WIZ))
                continue;
        }

        if (LOOK_OBJ(tmp) || QUERY_FLAG(pl, FLAG_WIZ))
        {
            flags = query_flags(tmp);
            if (QUERY_FLAG(tmp, FLAG_NO_PICK))
                flags |= F_NOPICK;


            SockBuf_AddInt(sbptr, tmp->count);
            SockBuf_AddInt(sbptr, flags);

            if(QUERY_FLAG(tmp, FLAG_NO_PICK))
            {
                SockBuf_AddInt(sbptr, -1);
            }
            else
            {
                if(tmp->type == CONTAINER && tmp->weapon_speed != 1.0f)
                {
                    SockBuf_AddInt(sbptr, tmp->damage_round_tag  + tmp->weight);
                }
                else
                {
                    SockBuf_AddInt(sbptr, WEIGHT(tmp) );
                }
            }

            if (tmp->inv_face && QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            {
                SockBuf_AddInt(sbptr, tmp->inv_face->number);
            }
            else
            {
                SockBuf_AddInt(sbptr, tmp->face->number);
            }

            SockBuf_AddChar(sbptr, tmp->facing);
            SockBuf_AddChar(sbptr, tmp->type);
            SockBuf_AddChar(sbptr, tmp->sub_type1);
            if (QUERY_FLAG(tmp, FLAG_IDENTIFIED))
            {
                SockBuf_AddChar(sbptr, tmp->item_quality);
                SockBuf_AddChar(sbptr, tmp->item_condition);
                SockBuf_AddChar(sbptr, tmp->item_level);
                SockBuf_AddChar(sbptr, tmp->item_skill);
            }
            else
            {
                SockBuf_AddInt(sbptr, 0xffffffff);
            }
            strncpy(item_n, query_base_name(tmp, pl), 127);
            item_n[127] = 0;
            len = strlen(item_n);
            SockBuf_AddChar(sbptr, len+1);
            SockBuf_AddString(sbptr, item_n, len);
            if (tmp->inv_animation_id)
            {
                SockBuf_AddShort(sbptr, tmp->inv_animation_id);
            }
            else
            {
                SockBuf_AddShort(sbptr, tmp->animation_id);
            }
            /* i use for both the same anim_speed - when we need a different,
                 * i adding inv_anim_speed.
                 */
            anim_speed = 0;
            if (QUERY_FLAG(tmp, FLAG_ANIMATE))
            {
                if (tmp->anim_speed)
                    anim_speed = tmp->anim_speed;
                else
                {
                    if (FABS(tmp->speed) < 0.001)
                        anim_speed = 255;
                    else if (FABS(tmp->speed) >= 1.0)
                        anim_speed = 1;
                    else
                        anim_speed = (int) (1.0 / FABS(tmp->speed));
                }
                if (anim_speed > 255)
                    anim_speed = 255;
            }
            SockBuf_AddChar(sbptr, anim_speed);
            SockBuf_AddInt(sbptr, tmp->nrof);
            got_one++;

            if (QUERY_FLAG(pl, FLAG_WIZ))
            {
                if (tmp->inv && tmp->type != CONTAINER)
                {
                    got_one = esrv_send_inventory_DM(pl, tmp);
                    /* 2008-08-28 Alderan: Always re-get the local pointer after a call to a function which puts stuff in the sockbuf! */
                    sbptr = ACTIVE_SOCKBUF(ns);
                }
            }
        }
    }

    if (got_one || pl != op) /* container can be empty... */
        SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_ITEMY, SOCKBUF_DYNAMIC);
    else
        SOCKBUF_REQUEST_RESET(ns);
}


static void esrv_update_item_send(int flags, object *pl, object *op)
{
    NewSocket        *ns = &CONTR(pl)->socket;
    sockbuf_struct    *sbptr;

    /*LOG(llevDebug,"update item: %s\n", query_name(op));*/
    /* If we have a request to send the player item, skip a few checks. */
    if (op != pl)
    {
        if (!LOOK_OBJ(op) && !QUERY_FLAG(pl, FLAG_WIZ))
            return;
    }

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);
    sbptr = ACTIVE_SOCKBUF(ns);

    SockBuf_AddShort(sbptr, flags);
    SockBuf_AddInt(sbptr, op->count);

    if (flags & UPD_LOCATION)
    {
        SockBuf_AddInt(sbptr, op->env ? op->env->count : 0);
    }
    if (flags & UPD_FLAGS)
    {
        SockBuf_AddInt(sbptr, query_flags(op));
    }
    if (flags & UPD_WEIGHT)
    {
        if(op->type == CONTAINER && op->weapon_speed != 1.0f)
        {
            SockBuf_AddInt(sbptr, op->damage_round_tag  + op->weight);
        }
        else
        {
            SockBuf_AddInt(sbptr, WEIGHT(op));
        }
    }
    if (flags & UPD_FACE)
    {
        if (op->inv_face && QUERY_FLAG(op, FLAG_IDENTIFIED))
        {
            SockBuf_AddInt(sbptr, op->inv_face->number);
        }
        else
        {
            SockBuf_AddInt(sbptr, op->face->number);
        }
    }
    if (flags & UPD_DIRECTION)
    {
        SockBuf_AddChar(sbptr, op->facing);
    }
    if (flags & UPD_NAME)
    {
        int     len;
        char    item_n[MEDIUM_BUF];

        strncpy(item_n, query_base_name(op, pl), 127);
        item_n[127] = 0;
        len = strlen(item_n);
        SockBuf_AddChar(sbptr, len + 1);
        SockBuf_AddString(sbptr, item_n, len);
    }
    if (flags & UPD_ANIM)
    {
        if (op->inv_animation_id)
        {
            SockBuf_AddShort(sbptr, op->inv_animation_id);
        }
        else
        {
            SockBuf_AddShort(sbptr, op->animation_id);
        }
    }
    if (flags & UPD_ANIMSPEED)
    {
        int anim_speed  = 0;
        if (QUERY_FLAG(op, FLAG_ANIMATE))
        {
            if (op->anim_speed)
                anim_speed = op->anim_speed;
            else
            {
                if (FABS(op->speed) < 0.001)
                    anim_speed = 255;
                else if (FABS(op->speed) >= 1.0)
                    anim_speed = 1;
                else
                    anim_speed = (int) (1.0 / FABS(op->speed));
            }
            if (anim_speed > 255)
                anim_speed = 255;
        }
        SockBuf_AddChar(sbptr, anim_speed);
    }
    if (flags & UPD_NROF)
    {
        SockBuf_AddInt(sbptr, op->nrof);
    }
    if (flags & UPD_QUALITY)
    {
        SockBuf_AddChar(sbptr, op->item_quality);
        SockBuf_AddChar(sbptr, op->item_condition);
    }

    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_UPITEM, SOCKBUF_DYNAMIC);
}

/* Updates object *op for player *pl.  flags is a list of values to update
 * to the client (as defined in newclient.h - might as well use the
 * same value both places.
 */
void esrv_update_item(int flags, object *pl, object *op)
{
    object *tmp;

    /* special case: update something in a container.
     * we don't care about where the container is,
     * because always is the container link list valid!
     */
    if (op->env && op->env->type == CONTAINER)
    {
        for (tmp = op->env->attacked_by; tmp; tmp = CONTR(tmp)->container_above)
            esrv_update_item_send(flags, tmp, op);
        return;
    }

    /* Integrating the is_player_inv() test here saves us many tests in the code.
     * TODO: remove the "pl" parameter for this function call
     * Gecko 2007-01-19 */
    if((tmp = is_player_inv(op)))
        esrv_update_item_send(flags, tmp, op);
}


static void esrv_send_item_send(object *pl, object *op)
{
    NewSocket        *ns = &CONTR(pl)->socket;
    sockbuf_struct    *sbptr;
    int                anim_speed, len;
    char            item_n[MEDIUM_BUF];


    if(!pl ||!op)
        return;

    /* If this is not the player object, do some more checks */
    if (op != pl)
    {
        /* We only send 'visible' objects to the client */
        if (!LOOK_OBJ(op) &&
            !QUERY_FLAG(pl, FLAG_WIZ))
        {
            return;
        }
    }

    /*LOG(llevNoLog,"send item: %s\n", query_name(op));*/

    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);
    sbptr = ACTIVE_SOCKBUF(ns);

    SockBuf_AddInt(sbptr, -4); /* no delinv */
    SockBuf_AddInt(sbptr, (op->env ? op->env->count : 0));
    SockBuf_AddInt(sbptr, op->count);
    SockBuf_AddInt(sbptr, query_flags(op));

    if(op->type == CONTAINER && op->weapon_speed != 1.0f)
    {
        SockBuf_AddInt(sbptr, op->damage_round_tag  + op->weight);
    }
    else
    {
        SockBuf_AddInt(sbptr, WEIGHT(op));
    }

    if (op->head)
    {
        if (op->head->inv_face && QUERY_FLAG(op, FLAG_IDENTIFIED))
        {
            SockBuf_AddInt(sbptr, op->head->inv_face->number);
        }
        else
        {
            SockBuf_AddInt(sbptr, op->head->face->number);
        }
    }
    else
    {
        if (op->inv_face && QUERY_FLAG(op, FLAG_IDENTIFIED))
        {
            SockBuf_AddInt(sbptr, op->inv_face->number);
        }
        else
        {
            SockBuf_AddInt(sbptr, op->face->number);
        }
    }

    SockBuf_AddChar(sbptr, op->facing);
    if (op->env) /* if not below */
    {
        SockBuf_AddChar(sbptr, op->type);
        SockBuf_AddChar(sbptr, op->sub_type1);
        if (QUERY_FLAG(op, FLAG_IDENTIFIED))
        {
            SockBuf_AddChar(sbptr, op->item_quality);
            SockBuf_AddChar(sbptr, op->item_condition);
            SockBuf_AddChar(sbptr, op->item_level);
            SockBuf_AddChar(sbptr, op->item_skill);
        }
        else
        {
            SockBuf_AddInt(sbptr, 0xffffffff);
        }
    }
    strncpy(item_n, query_base_name(op, pl), 127);
    item_n[127] = 0;
    len = strlen(item_n);
    SockBuf_AddChar(sbptr, len+1);
    SockBuf_AddString(sbptr, item_n, len);

    if (op->env && op->inv_animation_id)
    {
        SockBuf_AddShort(sbptr, op->inv_animation_id);
    }
    else
    {
        SockBuf_AddShort(sbptr, op->animation_id);
    }
    anim_speed = 0;
    if (QUERY_FLAG(op, FLAG_ANIMATE))
    {
        if (op->anim_speed)
            anim_speed = op->anim_speed;
        else
        {
            if (FABS(op->speed) < 0.001)
                anim_speed = 255;
            else if (FABS(op->speed) >= 1.0)
                anim_speed = 1;
            else
                anim_speed = (int) (1.0 / FABS(op->speed));
        }
        if (anim_speed > 255)
            anim_speed = 255;
    }
    SockBuf_AddChar(sbptr, anim_speed);
    SockBuf_AddInt(sbptr, op->nrof);

    SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_ITEMX, SOCKBUF_DYNAMIC);
}

void esrv_send_item(object *pl, object *op)
{
    object *tmp;

    if(!pl ||!op)
        return;

    /* special case: update something in a container.
     * we don't care about where the container is,
     * because always is the container link list valid!
     */
    if (op->env && op->env->type == CONTAINER)
    {
        for (tmp = op->env->attacked_by; tmp; tmp = CONTR(tmp)->container_above)
            esrv_send_item_send(tmp, op);
        return;
    }

    /* Integrating the is_player_inv() test here saves us many tests in the code.
     * TODO: remove the "pl" parameter for this function call
     * Gecko 2007-01-19 */
    if((tmp = is_player_inv(op)))
        esrv_send_item_send(tmp, op);
}


static inline void esrv_del_item_send(player *pl, int tag)
{
    SOCKBUF_REQUEST_BUFFER(&pl->socket, SOCKET_SIZE_SMALL);
    SockBuf_AddInt(ACTIVE_SOCKBUF(&pl->socket), tag);
    SOCKBUF_REQUEST_FINISH(&pl->socket, SERVER_CMD_DELITEM, SOCKBUF_DYNAMIC);
}

/* Tells the client to delete an item.
 * cont is the container - it must be seperated
 * from tag because the "tag" object can be destroyed
 * at this point on the server - we need to notify it
 * to the clients now.
 */
void esrv_del_item(player *pl, int tag, object *cont)
{
    /* FIXME: Do we really need this special handling for open containers here?
     * 
     * -- Smacky 20130121 */
    /* Note that this meahs containers must be deleted client-side BEFORE they
     * are unlinked (closed). */
    if (cont &&
        cont->type == CONTAINER &&
        cont->attacked_by)
    {
        object *whose;

        for (whose = cont->attacked_by; whose;
             whose = CONTR(whose)->container_above)
        {
            esrv_del_item_send(CONTR(whose), tag);
        }
    }
    else if (pl)
    {
        esrv_del_item_send(pl, tag);
    }
}


/*******************************************************************************
 *
 * Client has requested us to do something with an object.
 *
 ******************************************************************************/

/* Takes a player and object count (tag) and returns the actual object
 * pointer, or null if it can't be found.
 */

object * esrv_get_ob_from_count(object *pl, tag_t count)
{
    object *op, *tmp;

    if (pl->count == count)
        return pl;

    /* this is special case... We can examine deep inside every inventory
     * even from non containers.
     */
    if (QUERY_FLAG(pl, FLAG_WIZ))
    {
        for (op = pl->inv; op; op = op->below)
        {
            if (op->count == count)
                return op;
            else if (op->inv)
            {
                if ((tmp = esrv_get_ob_from_count_DM(op->inv, count)))
                    return tmp;
            }
        }
        if(pl->map)
        {
            for (op = GET_MAP_OB(pl->map, pl->x, pl->y); op; op = op->above)
            {
                if (op->count == count)
                    return op;
                else if (op->inv)
                {
                    if ((tmp = esrv_get_ob_from_count_DM(op->inv, count)))
                        return tmp;
                }
            }
        }
        return NULL;
    }

    for (op = pl->inv; op; op = op->below)
        if (op->count == count)
            return op;
        else if (op->type == CONTAINER && CONTR(pl)->container == op)
            for (tmp = op->inv; tmp; tmp = tmp->below)
                if (tmp->count == count)
                    return tmp;

    if(pl->map)
    {
        for (op = GET_MAP_OB(pl->map, pl->x, pl->y); op; op = op->above)
            if (op->count == count)
                return op;
            else if (op->type == CONTAINER && CONTR(pl)->container == op)
                for (tmp = op->inv; tmp; tmp = tmp->below)
                    if (tmp->count == count)
                        return tmp;
    }
    return NULL;
}

/* rekursive function for DM access to non container inventories */
static object * esrv_get_ob_from_count_DM(object *pl, tag_t count)
{
    object *tmp, *op;

    for (op = pl; op; op = op->below)
    {
        if (op->count == count)
            return op;
        else if (op->inv)
        {
            if ((tmp = esrv_get_ob_from_count_DM(op->inv, count)))
                return tmp;
        }
    }
    return NULL;
}

/* Move an object to a new lcoation */
void esrv_move_object(object *pl, tag_t to, tag_t tag, long nrof)
{
    object *op, *env;
    int     tmp;

    /*LOG(llevDebug,"Move item %d (nrof=%d) to %d.\n", tag, nrof,to);*/

    op = esrv_get_ob_from_count(pl, tag);
    if (!op) /* latency effect - we have moved before we applied this (or below from player changed) */
        return;

    if (!to) /* drop it to the ground */
    {
        if (op->map && !op->env)
            return;

        /*LOG(llevNoLog,"drop it... (%d)\n",check_container(pl,op));*/
        CLEAR_FLAG(pl, FLAG_INV_LOCKED); /* funny trickm see check container */
        if ((tmp = check_container(pl, op)))
            new_draw_info(NDI_UNIQUE, 0, pl, "Remove first all ~NO-DROP~ items from this container!");
        else if (QUERY_FLAG(pl, FLAG_INV_LOCKED))
            new_draw_info(NDI_UNIQUE, 0, pl, "You can't drop a container with locked items inside!");
        else
            drop_object(pl, op, nrof);
        return;
    }
    else if (to == pl->count || (to == op->count && !op->env)) /* pick it up to the inventory */
    {
        /* return if player has already picked it up */
        if (op->env == pl)
            return;

        CONTR(pl)->count = nrof;
        /*LOG(llevNoLog,"pick up...\n");*/
        pick_up(pl, op); /* it goes in player inv or readied container */
        return ;
    }
    /* If not dropped or picked up, we are putting it into a sack */
    env = esrv_get_ob_from_count(pl, to);
    if (!env)
        return;

    /* put_object_in_sack presumes that necessary sanity checking
     * has already been done (eg, it can be picked up and fits in
     * in a sack, so check for those things.  We should also check
     * an make sure env is in fact a container for that matter.
     */
    /* player have for example a opend container in the inventory */
    if (env->type == CONTAINER && can_pick(pl, op) && sack_can_hold(pl, env, op, nrof))
    {
        /*LOG(llevNoLog,"put in sack...\n");*/
        CLEAR_FLAG(pl, FLAG_INV_LOCKED); /* funny trickm see check container */
        tmp = check_container(pl, op);
        if (QUERY_FLAG(pl, FLAG_INV_LOCKED) && env->env != pl)
            new_draw_info(NDI_UNIQUE, 0, pl, "You can't drop a container with locked items inside!");
        else if (tmp && env->env != pl)
            new_draw_info(NDI_UNIQUE, 0, pl, "Remove first all ~NO-DROP~ items from this container!");
        else if (QUERY_FLAG(op, FLAG_STARTEQUIP) && env->env != pl)
            new_draw_info(NDI_UNIQUE, 0, pl, "You can't store ~NO-DROP~ items outside your inventory!");
        else
            put_object_in_sack(pl, env, op, nrof);
        return;
    }
}


/* thats the safest rule: you can't drop containers which holds
 * a startequip item or a container holding one.
 * return is the number of one drops in this container chain.
 */
static int check_container(object *pl, object *con)
{
    object *current, *next;
    int     ret = 0;

    if (con->type != CONTAINER) /* only check stuff *inside* a container */
        return ret;

    for (current = con->inv; current != NULL; current = next)
    {
        next = current->below;
        ret += check_container(pl, current);

        if (QUERY_FLAG(current, FLAG_STARTEQUIP))
            ret += 1;
        if (QUERY_FLAG(current, FLAG_INV_LOCKED))
            SET_FLAG(pl, FLAG_INV_LOCKED);
    }
    return ret;
}

