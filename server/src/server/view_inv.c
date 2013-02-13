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

#include "global.h"

#define FILTER(_PL_, _OP_) \
    ((_OP_) != (_PL_)->ob &&                          /* never see self but otherwise */ \
     (QUERY_FLAG((_PL_)->ob, FLAG_WIZ) ||             /* WIZ gmasters see all */ \
      (!QUERY_FLAG((_OP_), FLAG_SYS_OBJECT) &&        /* can't see sys objects */ \
       (!QUERY_FLAG((_OP_), FLAG_IS_INVISIBLE) ||     /* can't see invisible objects unless */ \
        QUERY_FLAG((_PL_)->ob, FLAG_SEE_INVISIBLE) || /* player has the ability to do so or */ \
        (_OP_)->env == (_PL_)->ob))))                 /* they're in his inventory (simulates feel) */

static void            SendInventory(player *pl, object *op);
static uint8           AddInventory(sockbuf_struct *sb, _server_client_cmd cmd,
                                    uint8 start, uint8 end, object *first);
static void            AddFakeObject(sockbuf_struct *sb, _server_client_cmd cmd,
                                     uint32 tag, uint32 face, char *name);
static uint8           AddName(char *name);
static void            NotifyClients(_server_client_cmd cmd, uint16 flags,
                                     object *op);
static sockbuf_struct *BroadcastItemCmd(sockbuf_struct *sb, _server_client_cmd cmd,
                                        uint16 flags, player *pl, object *op);
static char           *PrepareData(_server_client_cmd cmd, uint16 flags, player *pl,
                                   object *op, char *data);
static uint32          ClientFlags(object *op);
static object  *esrv_get_ob_from_count_DM(object *pl, tag_t count);
static int      check_container(object *pl, object *con);

void esrv_send_below(player *pl)
{
    object             *who;
    mapstruct          *m;
    NewSocket          *ns;
    sockbuf_struct     *sb;
    uint8               sendme;
    _server_client_cmd  cmd = SERVER_CMD_ITEMX;

    /* Sanity checks. */
    if (!pl ||
        (pl->state & (ST_DEAD | ST_ZOMBIE)) ||
        pl->socket.status != Ns_Playing ||
        !(who = pl->ob) ||
        QUERY_FLAG(who, FLAG_REMOVED) ||
        !(m = who->map) ||
        m->in_memory != MAP_ACTIVE ||
        OUT_OF_REAL_MAP(m, who->x, who->y))
    {
        return;
    }

    ns = &pl->socket;
    SOCKBUF_REQUEST_BUFFER(ns, SOCKBUF_DYNAMIC);
    sb = ACTIVE_SOCKBUF(ns);
    SockBuf_AddInt(sb, 0);
    SockBuf_AddInt(sb, 0);

    /* Items already been sent? Send fake 'previous' item. */
    if (ns->look_position)
    {
        AddFakeObject(sb, cmd,
                      0x80000000 | (ns->look_position - NUM_LOOK_OBJECTS),
                      prev_item_face->number, "Apply to see previous items");
        sb = ACTIVE_SOCKBUF(ns);
    }

    sendme = AddInventory(sb, cmd, 0, 0, GET_MAP_OB_LAST(m, who->x, who->y));
    sb = ACTIVE_SOCKBUF(ns);

    if (sendme)
    {
        SOCKBUF_REQUEST_FINISH(ns, cmd, SOCKBUF_DYNAMIC);
    }
    else
    {
        SOCKBUF_REQUEST_RESET(ns);
    }
}

void esrv_send_inventory(player *pl, object *op)
{
    SendInventory(pl, op);
}

void esrv_open_container(player *pl, object *op)
{
    SendInventory(pl, op);
}

void esrv_close_container(player *pl)
{
    SendInventory(pl, NULL);
}

void esrv_send_item(object *op)
{
    _server_client_cmd cmd = (op->map) ? SERVER_CMD_ITEMX : SERVER_CMD_ITEMY;
    uint16             flags = UPD_FLAGS | UPD_WEIGHT | UPD_FACE |
                               UPD_DIRECTION | UPD_NAME | UPD_ANIM |
                               UPD_ANIMSPEED | UPD_NROF;

    NotifyClients(cmd, flags, op);
}

void esrv_update_item(uint16 flags, object *op)
{
    NotifyClients(SERVER_CMD_UPITEM, flags, op);
}

void esrv_del_item(object *op)
{
    NotifyClients(SERVER_CMD_DELITEM, 0, op);
}

/* Sends the inventory of op to pl. This function is in fact used in three
 * circumstances: (1) !op -- the player's container is closed; (2) op ==
 * pl->container the player's contaiiner is opened (and it's inventory sent);
 * (3) otherwise -- the inventory of op is sent. */
static void SendInventory(player *pl, object *op)
{
    NewSocket          *ns;
    sockbuf_struct     *sb;
    _server_client_cmd  cmd = SERVER_CMD_ITEMY;

    /* Sanity checks. */
    if (!pl ||
        (pl->state & (ST_DEAD | ST_ZOMBIE)) ||
        pl->socket.status != Ns_Playing)
    {
        return;
    }

    ns = &pl->socket;
    SOCKBUF_REQUEST_BUFFER(ns, SOCKBUF_DYNAMIC);
    sb = ACTIVE_SOCKBUF(ns);

    /* Close container. */
    if (!op)
    {
        SockBuf_AddInt(sb, -1);
        SockBuf_AddInt(sb, -1);
    }
    else
    {
        /* Open container. */
        if (pl->container == op)
        {
            SockBuf_AddInt(sb, -1);
            SockBuf_AddInt(sb, op->count);
        }
        /* Default. */
        else
        {
            SockBuf_AddInt(sb, op->count);
            SockBuf_AddInt(sb, op->count);
        }

        (void)AddInventory(sb, cmd, 0, 0, op->inv);
        sb = ACTIVE_SOCKBUF(ns);
    }

    SOCKBUF_REQUEST_FINISH(ns, cmd, SOCKBUF_DYNAMIC);
}

/* Adds the objects including and below first to sb (inventory cmds are always
 * sent to a single client in a working buffer). cmd is either SERVER_CMD_ITEMX
 * for below windoow objects or SERVER_CMD_ITEMY otherwise. start and end are
 * only used when first and its siblings have no env/are directly on a map and
 * restrict the amount of data sent in one go to a client. WIZ gmasters also
 * get sent sys objects and the full inventories of each object (ie, EVERYTHING
 * below first). */
static uint8 AddInventory(sockbuf_struct *sb, _server_client_cmd cmd,
                          uint8 start, uint8 end, object *first)
{
    NewSocket *ns = sb->ns;
    player    *pl = ns->pl;
    object    *this;
    uint8      sendme = 0;

    for (this = first; this; this = this->below)
    {
        if (this->type == SHOP_FLOOR || // TODO: Remove this type/arch
            !FILTER(pl, this))
        {
            continue;
        }
        else if (this->env ||
                 ++start >= ns->look_position) // skip already sent items
        {
            sendme = 1;

            /* Too many items? Send fake 'next' item. */
            if (!this->env &&
                ++end > NUM_LOOK_OBJECTS)
            {
                AddFakeObject(sb, cmd,
                              0x80000000 | (ns->look_position + NUM_LOOK_OBJECTS),
                              next_item_face->number, "Apply to see next items");
                sb = ACTIVE_SOCKBUF(ns);

                break;
            }
            else
            {
                char    data[MEDIUM_BUF],
                       *cp = data;
                uint16  flags = UPD_FLAGS | UPD_WEIGHT | UPD_FACE |
                                UPD_DIRECTION | UPD_NAME | UPD_ANIM |
                                UPD_ANIMSPEED | UPD_NROF;

                cp = PrepareData(cmd, flags, pl, this, data);
                SockBuf_AddStringNonTerminated(sb, data, cp - data);

                if (QUERY_FLAG(pl->ob, FLAG_WIZ) &&
                    this->inv)
                {
                    AddFakeObject(sb, cmd, 0xc0000000 | this->count,
                                  blank_face->number, "start inventory");
                    sb = ACTIVE_SOCKBUF(ns);
                    (void)AddInventory(sb, cmd, start, end, this->inv);
                    sb = ACTIVE_SOCKBUF(ns);
                    AddFakeObject(sb, cmd, 0xe0000000 | this->count,
                                  blank_face->number, "end inventory");
                    sb = ACTIVE_SOCKBUF(ns);
                }
            }
        }
    }

    return sendme;
}

/* Adds a fake object to sb, as defined by the other parameters. */
static void AddFakeObject(sockbuf_struct *sb, _server_client_cmd cmd,
                          uint32 tag, uint32 face, char *name)
{
    char  buf[SMALL_BUF];
    uint8 len;

    SockBuf_AddInt(sb, tag);
    SockBuf_AddInt(sb, 0);
    SockBuf_AddInt(sb, -1);
    SockBuf_AddInt(sb, face);
    SockBuf_AddChar(sb, 0);

    if (cmd == SERVER_CMD_ITEMY)
    {
       SockBuf_AddChar(sb, 0);
       SockBuf_AddChar(sb, 0);
       SockBuf_AddChar(sb, 0);
       SockBuf_AddChar(sb, 0);
       SockBuf_AddChar(sb, 0);
       SockBuf_AddChar(sb, 0);
    }

    sprintf(buf, name);
    len = AddName(buf);
    SockBuf_AddChar(sb, len + 1);
    SockBuf_AddString(sb, buf, len);
    SockBuf_AddShort(sb, 0);
    SockBuf_AddChar(sb, 0);
    SockBuf_AddInt(sb, 0);
}

/* Ensures name is less than 128 characters long and returns its length. */
static uint8 AddName(char *name)
{
    uint8 len = MIN(127, strlen(name));

    *(name + len) = '\0';

    return len;
}

/* Finds the clients interested in op (which may be on a map or in an env) and
 * attaches a broadcast sockbuf to them. */
static void NotifyClients(_server_client_cmd cmd, uint16 flags, object *op)
{
    object         *who = NULL,
                   *where = op->env;
    sockbuf_struct *sb = NULL;

    /* When no-one is playing, there's nothing to do. */
    if (!first_player)
    {
        return;
    }

    if (where)
    {
        /* Loop through the envs of op, sending cmd to each valid client. */
        while (where &&
               where->type != TYPE_VOID_CONTAINER)
        {
            if (where->type == PLAYER)
            {
                who = where;
                where = NULL; // players cant have envs
            }
            else if (where->type == CONTAINER)
            {
                who = (!who) ? where->attacked_by : CONTR(who)->container_above;
            }

            /* No (more) players? Move on to where's parent. */
            if (!who)
            {
                where = where->env;
            }
            else
            {
                sb = BroadcastItemCmd(sb, cmd, flags, CONTR(who), op);
            }
        };
    }
    else
    {
        /* Send cmd to each valid client on the square. */
        for (who = GET_MAP_OB(op->map, op->x, op->y); who; who = who->above)
        {
            if (who->type != PLAYER)
            {
                continue;
            }

            sb = BroadcastItemCmd(sb, cmd, flags, CONTR(who), op);
        }
    }

    if (sb)
    {
        SOCKBUF_COMPOSE_FREE(sb);
    }
}

/* First creates if necessary, then attaches a broadcast sockbuf (sb) of
 * cmd/flags/op to the client pl. */
static sockbuf_struct *BroadcastItemCmd(sockbuf_struct *sb, _server_client_cmd cmd,
                                        uint16 flags, player *pl, object *op)
{
    /* Only send the cmd to a valid client and when filter is passed. */
    if (pl &&
        !(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
        pl->socket.status == Ns_Playing &&
        FILTER(pl, op))
    {
        /* If we've not yet made a sockbuf, make one now. */
        if (!sb)
        {
            char  data[MEDIUM_BUF],
                 *cp = data;

            /* The first few bytes depend on the cmd. */
            if (cmd == SERVER_CMD_ITEMX ||
                cmd == SERVER_CMD_ITEMY)
            {
                *((uint32 *)cp) = -4;
                cp += 4;
                *((uint32 *)cp) = (op->env) ? op->env->count : 0;
                cp += 4;
            }
            else if (cmd == SERVER_CMD_UPITEM)
            {
                *((uint16 *)cp) = flags;
                cp += 2;
            }

            cp = PrepareData(cmd, flags, pl, op, cp);
            sb = SOCKBUF_COMPOSE(cmd, data, cp - data, 0);
        }

        SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
    }

    return sb;
}

/* Depending on cmd and flags, puts data about op into the buffer, data (which
 * must be large enough -- well under 200 bytes), returning a pointer to the
 * end of the used part of the buffer. If pl != NULL this is used to
 * personalise certain data (currently only if flags & UPD_NAME and op is a
 * corpse). The buffer is suitable to be attached to a working or broadcast
 * sockbuf. */
static char *PrepareData(_server_client_cmd cmd, uint16 flags, player *pl,
                         object *op, char *data)
{
    char *cp = data;

    *((uint32 *)cp) = op->count;
    cp += 4;

    /* For DELITEM that's it. For others it depends also on flags. */
    if (cmd != SERVER_CMD_DELITEM)
    {
        object *where = op->env,
               *head = (op->head) ? op->head : op;

        if ((flags & UPD_LOCATION))
        {
            *((uint32 *)cp) = (where) ? where->count : 0;
            cp += 4;
        }

        if ((flags & UPD_FLAGS))
        {
            *((uint32 *)cp) = ClientFlags(op);
            cp += 4;
        }

        if ((flags & UPD_WEIGHT))
        {
            if (!where &&
                QUERY_FLAG(op, FLAG_NO_PICK))
            {
                *((uint32 *)cp) = (uint32)-1;
            }
            else
            {
                *((uint32 *)cp) = (op->type == CONTAINER &&
                                   op->weapon_speed != 1.0)
                                  ? op->damage_round_tag + op->weight
                                  : WEIGHT(op);
            }

            cp += 4;
        }

        if ((flags & UPD_FACE))
        {
            *((uint32 *)cp) = (where &&
                               head->inv_face &&
                               QUERY_FLAG(op, FLAG_IDENTIFIED)) // why?
                              ? head->inv_face->number
                              : head->face->number;
            cp += 4;
        }

        if ((flags & UPD_DIRECTION))
        {
            *((uint8 *)cp++) = op->facing;
        }

        /* When we're sending an item and it is in an env, add any identified
         * info .*/
        /* TODO: Several issues here. */
        if (//cmd == SERVER_CMD_ITEMX || // ITEMX means below
            cmd == SERVER_CMD_ITEMY)
        {
            /* TODO: type and subtype should always be sent for ITEMX/Y,
             * regardless of where. However, 0.10.z clients assume the values
             * are not sent for below items so to do so requires a Y update. */
            *((uint8 *)cp++) = op->type;
            *((uint8 *)cp++) = op->sub_type1;

            if (QUERY_FLAG(op, FLAG_IDENTIFIED))
            {
                *((uint8 *)cp++) = op->item_quality;
                *((uint8 *)cp++) = op->item_condition;
                *((uint8 *)cp++) = op->item_level;
                *((uint8 *)cp++) = op->item_skill;
            }
            else
            {
                *((uint32 *)cp) = 0xffffffff;
                cp += 4;
            }
        }

        if ((flags & UPD_NAME))
        {
            char    buf[SMALL_BUF];
            object *who = (pl) ? pl->ob : NULL;
            uint8   len;

            sprintf(buf, "%s", query_base_name(op, who));
            len = AddName(buf);
            *((uint8 *)cp++) = len + 1;
            sprintf(cp, "%s", buf);
            cp += len + 1;
        }

        if ((flags & UPD_ANIM))
        {
            *((uint16 *)cp) = (where &&
                               head->inv_animation_id)
                              ? head->inv_animation_id
                              : head->animation_id;
            cp += 2;
        }

        if ((flags & UPD_ANIMSPEED))
        {
            sint32 animspeed;

            if (QUERY_FLAG(op, FLAG_ANIMATE))
            {
                if (op->anim_speed)
                {
                    animspeed = op->anim_speed;
                }
                else
                {
                    float speed = FABS(op->speed);

                    if (speed < 0.001)
                    {
                        animspeed = 255;
                    }
                    else if (speed >= 1.0)
                    {
                        animspeed = 1;
                    }
                    else
                    {
                        animspeed = (sint32)(1.0 / speed);
                    }
                }

                animspeed = MAX(0, MIN(animspeed, 255));
            }
            else
            {
                animspeed = 0;
            }

            *((uint8 *)cp++) = (uint8)animspeed;
        }

        if ((flags & UPD_NROF))
        {
            *((uint32 *)cp) = op->nrof;
            cp += 4;
        }

        if ((flags & UPD_QUALITY))
        {
            *((uint8 *)cp++) = op->item_quality;
            *((uint8 *)cp++) = op->item_condition;
        }
    }

    return cp;
}

/* Returns client-side flags depending on status of object. */
static uint32 ClientFlags(object *op)
{
    uint32 flags = 0;

    if (QUERY_FLAG(op, FLAG_APPLIED))
    {
        switch (op->type)
        {
            case BOW:
            case WAND:
            case ROD:
            case HORN:
              flags |= A_READIED;

              break;

            case WEAPON:
              flags |= A_WIELDED;

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
              flags |= A_WORN;

              break;

            case CONTAINER:
              flags |= A_ACTIVE;

              break;

            default:
              flags |= A_APPLIED;
        }

        flags |= F_APPLIED;
    }

    if (QUERY_FLAG(op, FLAG_IS_ETHEREAL))
    {
        flags |= F_ETHEREAL;
    }

    if (QUERY_FLAG(op, FLAG_IS_INVISIBLE))
    {
        flags |= F_INVISIBLE;
    }

    if (QUERY_FLAG(op, FLAG_UNPAID))
    {
        flags |= F_UNPAID;
    }

    if (QUERY_FLAG(op, FLAG_KNOWN_MAGICAL) &&
        is_magical(op))
    {
        flags |= F_MAGIC;
    }

    if (QUERY_FLAG(op, FLAG_KNOWN_CURSED))
    {
        if (QUERY_FLAG(op, FLAG_CURSED))
        {
            flags |= F_CURSED;
        }

        if (QUERY_FLAG(op, FLAG_DAMNED))
        {
            flags |= F_DAMNED;
        }
    }

    if (op->type == CONTAINER &&
        (op->attacked_by ||
         (!op->env &&
          QUERY_FLAG(op, FLAG_APPLIED))))
    {
        flags |= F_OPEN;
    }

    if (QUERY_FLAG(op, FLAG_NO_PICK))
    {
        flags |= F_NOPICK;
    }

    if (QUERY_FLAG(op, FLAG_INV_LOCKED))
    {
        flags |= F_LOCKED;
    }

    if (QUERY_FLAG(op, FLAG_IS_TRAPED))
    {
        flags |= F_TRAPED;
    }

    return flags;
}

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

