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
     (((_PL_)->gmaster_mode & GMASTER_MODE_SA) ||      /* SAs see all but others */ \
      (!QUERY_FLAG((_OP_), FLAG_SYS_OBJECT) &&          /* can't see sys objects */ \
       !IS_GMASTER_INVIS((_OP_)) &&                     /* can't see gmaster invis */ \
       (!QUERY_FLAG((_OP_), FLAG_IS_INVISIBLE) ||       /* can't see invisible objects unless */ \
        QUERY_FLAG((_PL_)->ob, FLAG_SEE_INVISIBLE) ||    /* player has the ability to do so or */ \
        (_OP_)->env == (_PL_)->ob))))                    /* they're in his inventory (simulates feel) */

static void            SendInventory(player *pl, object *op);
static uint8           AddInventory(sockbuf_struct *sb, _server_client_cmd cmd,
                                    uint8 start, uint8 end, object *first);
static void            AddFakeObject(sockbuf_struct *sb, _server_client_cmd cmd,
                                     uint32 tag, uint32 face, char *name);
static void            NotifyClients(_server_client_cmd cmd, uint16 flags,
                                     object *op);
static sockbuf_struct *BroadcastItemCmd(sockbuf_struct *sb, _server_client_cmd cmd,
                                        uint16 flags, player *pl, object *op);
static char           *PrepareData(_server_client_cmd cmd, uint16 flags, player *pl,
                                   object *op, char *data);
static uint32          ClientFlags(object *op);
static object         *GetObjFromCount(object *what, tag_t count);

void esrv_send_below(player *pl)
{
    object             *who;
    mapstruct          *m;
    NewSocket          *ns;
    sockbuf_struct     *sb;
    MapSpace           *msp;
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

    msp = GET_MAP_SPACE_PTR(m, who->x, who->y);
    sendme = AddInventory(sb, cmd, 0, 0, GET_MAP_SPACE_LAST(msp));
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
    /* Due to some buggy code in at least 0.10.6 and earlier clients updates
     * for items in non-player inventories cause the item to appear in the
     * below window so we must resend the full item details (which implicitly
     * deletes the old instance client-side).
     *
     * -- Smacky 20140311 */
    if (op->env &&
        op->env->type != PLAYER)
    {
        NotifyClients(SERVER_CMD_ITEMY, UPD_FLAGS | UPD_WEIGHT | UPD_FACE |
            UPD_DIRECTION | UPD_NAME | UPD_ANIM | UPD_ANIMSPEED | UPD_NROF,
            op);
    }
    else
    {
        NotifyClients(SERVER_CMD_UPITEM, flags, op);
    }
}

void esrv_del_item(object *op)
{
    NotifyClients(SERVER_CMD_DELITEM, 0, op);
}

void esrv_send_or_del_item(object *op)
{
    uint16 flags = UPD_FLAGS | UPD_WEIGHT | UPD_FACE | UPD_DIRECTION |
                   UPD_NAME | UPD_ANIM | UPD_ANIMSPEED | UPD_NROF;

    NotifyClients(0, flags, op);
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
        if (!FILTER(pl, this))
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
                ns->look_flag = 1;

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

                if ((pl->gmaster_mode & GMASTER_MODE_SA) &&
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
    SockBuf_AddInt(sb, 0);
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

    sprintf(buf, "%s", name);
    len = (uint8)strlen(buf);
    SockBuf_AddChar(sb, len + 1);
    SockBuf_AddString(sb, buf, len);
    SockBuf_AddShort(sb, 0);
    SockBuf_AddChar(sb, 0);
    SockBuf_AddInt(sb, 0);
}

/* Finds the clients interested in op (which may be on a map or in an env) and
 * attaches a broadcast sockbuf to them. */
static void NotifyClients(_server_client_cmd cmd, uint16 flags, object *op)
{
    object         *who = NULL;
    sockbuf_struct *sb = NULL;

    /* When no-one is playing, there's nothing to do. */
    if (!first_player)
    {
        return;
    }

    if (op->env)
    {
        object *where = op->env;

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
    else if (op->map)
    {
        /* Send cmd to each valid client on the square. */
        for (who = GET_MAP_OB(op->map, op->x, op->y); who; who = who->above)
        {
            player *pl;

            if (who->type != PLAYER)
            {
                continue;
            }

            pl = CONTR(who);

            if (!pl->socket.look_flag)
            {
                sb = BroadcastItemCmd(sb, cmd, flags, pl, op);
            }
            else
            {
                esrv_send_below(pl);
            }
        }
    }

    SOCKBUF_COMPOSE_FREE(sb);
}

/* First creates if necessary, then attaches a broadcast sockbuf (sb) of
 * cmd/flags/op to the client pl. */
static sockbuf_struct *BroadcastItemCmd(sockbuf_struct *sb, _server_client_cmd cmd,
                                        uint16 flags, player *pl, object *op)
{
    /* Only send the cmd to a valid client and when filter is passed. */
    if (pl &&
        !(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
        pl->socket.status == Ns_Playing)
    {
        /* This really defeats the whole purpose of a broadcast sockbuf (cos it
         * is rebuilt every time) but as it is only used rarely (when an inv or
         * below item gains or loses visibiliity), it's not a big deal. */
        if (!cmd)
        {
            SOCKBUF_COMPOSE_FREE(sb);

            if (FILTER(pl, op))
            {
                cmd = (op->map) ? SERVER_CMD_ITEMX : SERVER_CMD_ITEMY;
            }
            else
            {
                cmd = SERVER_CMD_DELITEM;
            }
        }
        else if (!FILTER(pl, op))
        {
            return sb;
        }

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
                *((uint32 *)cp) = WEIGHT_OVERALL(op);
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
            char   *name;
            object *who = (pl) ? pl->ob : NULL;
            uint8   len;

            name = query_name(op, who, ARTICLE_NONE, 0);

            for (len = 0; len <= 127; len++)
            {
                if (*(name + len) == '\0')
                {
                    break;
                }
            }

            *(name + len) = '\0'; 
            *((uint8 *)cp++) = len + 1;
            sprintf(cp, "%s", name);
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

            case TYPE_SKILL:
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
 * pointer, or NULL if it can't be found or it can't be manipulated (that is,
 * seen) by the player. We account for all forms of invisibility in deciding
 * what can and cannot be manipulated. */
object *esrv_get_ob_from_count(object *who, tag_t count)
{
    player *pl;
    object *this,
           *that;

    /* Sanity check. */
    if (!(pl = CONTR(who)))
    {
        return NULL;
    }

    /* Players can always play with themselves. */
    if (who->count == count)
    {
        return who;
    }

    /* this is special case... We can examine deep inside every inventory
     * even from non containers. */
    if ((pl->gmaster_mode & GMASTER_MODE_SA))
    {
        for (this = who->inv; this; this = this->below)
        {
            if (this->count == count)
            {
                return this;
            }
            else if (this->inv)
            {
                if ((that = GetObjFromCount(this->inv, count)))
                {
                    return that;
                }
            }
        }

        if(who->map)
        {
            for (this = GET_MAP_OB(who->map, who->x, who->y); this; this = this->above)
            {
                if (this->count == count)
                {
                    return this;
                }
                else if (this->inv)
                {
                    if ((that = GetObjFromCount(this->inv, count)))
                    {
                        return that;
                    }
                }
            }
        }
    }
    else
    {
        for (this = who->inv; this; this = this->below)
        {
            /* We know pl is not an SA so cannot manipulate... */
            if (QUERY_FLAG(this, FLAG_SYS_OBJECT))
            {
                continue;
            }

            if (this->count == count)
            {
                return this;
            }
            else if (this->type == CONTAINER &&
                     pl->container == this)
            {
                for (that = this->inv; that; that = that->below)
                {
                    /* We know pl is not an SA so cannot manipulate... */
                    if (QUERY_FLAG(that, FLAG_SYS_OBJECT))
                    {
                        continue;
                    }

                    if (that->count == count)
                    {
                        return that;
                    }
                }
            }
        }

        if(who->map)
        {
            for (this = GET_MAP_OB(who->map, who->x, who->y); this; this = this->above)
            {
                /* We know pl is not an SA so cannot manipulate... */
                if (QUERY_FLAG(this, FLAG_SYS_OBJECT) ||
                    IS_GMASTER_INVIS(this) ||
                    (QUERY_FLAG((this), FLAG_IS_INVISIBLE) &&
                     !QUERY_FLAG((who), FLAG_SEE_INVISIBLE)))
                    
                {
                    continue;
                }

                if (this->count == count)
                {
                    return this;
                }
                else if (this->type == CONTAINER &&
                         pl->container == this)
                {
                    for (that = this->inv; that; that = that->below)
                    {
                        /* We know pl is not an SA so cannot manipulate... */
                        if (QUERY_FLAG(that, FLAG_SYS_OBJECT) ||
                            (QUERY_FLAG((that), FLAG_IS_INVISIBLE) &&
                             !QUERY_FLAG((who), FLAG_SEE_INVISIBLE)))
                        {
                            continue;
                        }

                        if (that->count == count)
                        {
                            return that;
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

/* Recursive function for SA access to non container inventories. */
static object *GetObjFromCount(object *what, tag_t count)
{
    object *this;

    for (this = what; this; this = this->below)
    {
        if (this->count == count)
        {
            return this;
        }
        else if (this->inv)
        {
            object *that = GetObjFromCount(this->inv, count);

            if (that)
            {
                return that;
            }
        }
    }

    return NULL;
}
