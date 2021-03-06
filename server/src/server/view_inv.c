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

#define FILTER(_PL_, _O_) \
    (!QUERY_FLAG((_O_), FLAG_NO_SEND) &&              /* some objects are only map effects */ \
     ((_O_) != (_PL_)->ob &&                           /* never see self but otherwise */ \
      (((_PL_)->gmaster_mode & GMASTER_MODE_SA) ||       /* SAs see all but others */ \
       (!QUERY_FLAG((_O_), FLAG_SYS_OBJECT) &&           /* can't see sys objects */ \
        !IS_GMASTER_INVIS((_O_)) &&                      /* can't see gmaster invis */ \
        (!QUERY_FLAG((_O_), FLAG_IS_INVISIBLE) ||        /* can't see invisible objects unless */ \
         QUERY_FLAG((_PL_)->ob, FLAG_SEE_INVISIBLE) ||     /* player has the ability to do so or */ \
         (_O_)->env == (_PL_)->ob)))))                    /* they're in his inventory (simulates feel) */

static void            SendInventory(player_t *pl, object_t *op);
static uint8           AddInventory(sockbuf_struct *sb, _server_client_cmd cmd,
                                    uint8 start, uint8 end, object_t *first);
static void            AddFakeObject(sockbuf_struct *sb, _server_client_cmd cmd,
                                     uint32 tag, uint32 face, char *name);
static void            NotifyClients(_server_client_cmd cmd, uint16 flags,
                                     object_t *op);
static sockbuf_struct *BroadcastItemCmd(sockbuf_struct *sb, _server_client_cmd cmd,
                                        uint16 flags, player_t *pl, object_t *op);
static char           *PrepareData(_server_client_cmd cmd, uint16 flags, player_t *pl,
                                   object_t *op, char *data);
static uint32          ClientFlags(object_t *op);
static object_t         *GetObjFromCount(object_t *what, tag_t count);

void esrv_send_below(player_t *pl)
{
    object_t             *who;
    map_t          *m;
    NewSocket          *ns;
    sockbuf_struct     *sb;
    msp_t           *msp;
    uint8               sendme;
    _server_client_cmd  cmd = SERVER_CMD_ITEMX;

    /* Sanity checks. */
    if (!pl ||
        (pl->state & (ST_DEAD | ST_ZOMBIE)) ||
        pl->socket.status != Ns_Playing ||
        !(who = pl->ob) ||
        QUERY_FLAG(who, FLAG_REMOVED) ||
        !(m = who->map) ||
        m->in_memory != MAP_MEMORY_ACTIVE ||
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

    msp = MSP_GET2(m, who->x, who->y);
    sendme = AddInventory(sb, cmd, 0, 0, msp->last);
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

void esrv_send_inventory(player_t *pl, object_t *op)
{
    SendInventory(pl, op);
}

void esrv_open_container(player_t *pl, object_t *op)
{
    SendInventory(pl, op);
}

void esrv_close_container(player_t *pl)
{
    SendInventory(pl, NULL);
}

void esrv_send_item(object_t *op)
{
    _server_client_cmd cmd = (op->map) ? SERVER_CMD_ITEMX : SERVER_CMD_ITEMY;
    uint16             flags = UPD_FLAGS | UPD_WEIGHT | UPD_FACE |
                               UPD_DIRECTION | UPD_NAME | UPD_ANIM |
                               UPD_ANIMSPEED | UPD_NROF;

    NotifyClients(cmd, flags, op);
}

void esrv_update_item(uint16 flags, object_t *op)
{
    /* Lets not play about -- updating items is borked so redirect to a full
     * send as new.
     *
     * -- Smacky 20150610 */
#if 0
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
    /* Seems a similar bug updating items on maps.
     *
     * -- Smacky 20150610 */
    else if (op->map)
    {
        NotifyClients(SERVER_CMD_ITEMX, UPD_FLAGS | UPD_WEIGHT | UPD_FACE |
            UPD_DIRECTION | UPD_NAME | UPD_ANIM | UPD_ANIMSPEED | UPD_NROF,
            op);
    }
    else
    {
        NotifyClients(SERVER_CMD_UPITEM, flags, op);
    }
#else
    esrv_send_item(op);
#endif
}

void esrv_del_item(object_t *op)
{
    NotifyClients(SERVER_CMD_DELITEM, 0, op);
}

void esrv_send_or_del_item(object_t *op)
{
    uint16 flags = UPD_FLAGS | UPD_WEIGHT | UPD_FACE | UPD_DIRECTION |
                   UPD_NAME | UPD_ANIM | UPD_ANIMSPEED | UPD_NROF;

    NotifyClients(0, flags, op);
}

/* Sends the inventory of op to pl. This function is in fact used in three
 * circumstances: (1) !op -- the player's container is closed; (2) op ==
 * pl->container the player's contaiiner is opened (and it's inventory sent);
 * (3) otherwise -- the inventory of op is sent. */
static void SendInventory(player_t *pl, object_t *op)
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
                          uint8 start, uint8 end, object_t *first)
{
    NewSocket *ns = sb->ns;
    player_t    *pl = ns->pl;
    object_t    *this;
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
static void NotifyClients(_server_client_cmd cmd, uint16 flags, object_t *op)
{
    object_t       *this;
    map_t          *m;
    object_t       *where;
    msp_t          *msp;
    object_t       *next;
    sockbuf_struct *sb = NULL;

    /* Sanity checks. All we're interested in is clients interested in op. That
     * means players on the same msp as op or carrying op (or whatever it is in
     * in both cases), Therefore if no players are online there is nothing to
     * do (see final check too because if anyone is online we need to consider
     * if they have an interest). Also check that op is non-NULL and has not
     * been freed. */
    if (!first_player ||
        !op ||
        OBJECT_FREE(op)) 
    {
        return;
    }

    /* Starting at op, ascend through its parents until an object directly on a
     * map is reached. On exit, this will be that object, where will be NULL,
     * and m will be the map (guaranteed to be non-NULL). Notwithstanding the
     * sanity check, this must always be the case. If not, something
     * fundamental is broken within the object system; which is both beyond the
     * scope of this function to fix and probably means we never get this far
     * anyway so we never even address it.*/
    this = op;
    m = op->map;
    where = op->env;
//LOG(llevDebug,">>>>%s[%d] %s %d,%d %s (%c%c)\n",this->name,this->count,(m)?m->path:"NOMAP",this->x,this->y,(where)?where->name:"NOENV",QUERY_FLAG(this,FLAG_INSERTED)?'I':' ',QUERY_FLAG(this,FLAG_REMOVED)?'R':' ');

    while (!m)
    {
        /* Another sanity check. Neither ->map nor ->env means either this is
         * fully removed or has never been inserted in the first place. Either
         * way, we can't carry on (and there can't be any players involved
         * anyway) so return. */
        if (!where)
        {
            return;
        }

        this = where;
        m = where->map;
        where = where->env;
//LOG(llevDebug,"++++%s[%d] %s %d,%d %s (%c%c)\n",this->name,this->count,(m)?m->path:"NOMAP",this->x,this->y,(where)?where->name:"NOENV",QUERY_FLAG(this,FLAG_INSERTED)?'I':' ',QUERY_FLAG(this,FLAG_REMOVED)?'R':' ');
    };

    /* TODO: This is a temp workaround. ATM spawning mobs set ->map out of
     * sequence. So here we double check that even though m is non-NULL this is
     * *really* inserted. Otherwise eventually the server will crash.
     *
     * -- Smacky 20160921 */
    if (this->type == MONSTER &&
        !QUERY_FLAG(this, FLAG_INSERTED))
    {
        return;
    }

    /* So get the msp. */
    msp = MSP_KNOWN(this);

    /* Final sanity check. This saves a LOT of time -- no players here, nothing
     * to do. */
    if (!(msp->flags & MSP_FLAG_PLAYER))
    {
        return;
    }

    /* Browse msp for players, sending cmd to each valid client as
     * appropriate. */
    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        player_t *pl;

        /* Once we reach the first sys object there can be no more players, so
         * abort. */
        if (QUERY_FLAG(this, FLAG_SYS_OBJECT))
        {
            break;
        }

        /* Found a player/client? If not, continue. Alsso, no need to update
         * the client about its own avatar. */
        if (!(pl = (this->type == PLAYER) ? CONTR(this) : NULL) ||
            pl->ob == op)
        {
            continue;
        }

//LOG(llevDebug,">>>>%s[%d] %s %d,%d %s (%c%c)\n",op->name,op->count,(op->map)?op->map->path:"NOMAP",op->x,op->y,(op->env)?op->env->name:"NOENV",QUERY_FLAG(op,FLAG_INSERTED)?'I':' ',QUERY_FLAG(op,FLAG_REMOVED)?'R':' ');

        /* If op (not this) is on a map... */
        if (op->map)
        {
            /* If pl is not viewing a long list of items, just send a normal
             * broadcast sockbuf. */
            if (!pl->socket.look_flag)
            {
                sb = BroadcastItemCmd(sb, cmd, flags, pl, op);
            }
            /* Otherwise there's a bit of specific work to do so send THIS
             * client a new below window. */
            else
            {
                esrv_send_below(pl);
            }
        }
        /* Otherwise op must be in an env. If pl is an SA (who can see all,
         * including inside other objects, whatever depth) OR op is directly
         * inside pl's linked (means open) container or inventory, send a
         * broadcast sockbuf. */
        else if ((pl->gmaster_mode & GMASTER_MODE_SA) ||
            pl->container == op->env ||
            pl->ob == op->env)
        {
            sb = BroadcastItemCmd(sb, cmd, flags, pl, op);
        }
    }

    /* Tidy up our toys (if any). */
    SOCKBUF_COMPOSE_FREE(sb);
}

/* First creates if necessary, then attaches a broadcast sockbuf (sb) of
 * cmd/flags/op to the client pl. */
static sockbuf_struct *BroadcastItemCmd(sockbuf_struct *sb, _server_client_cmd cmd,
                                        uint16 flags, player_t *pl, object_t *op)
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
static char *PrepareData(_server_client_cmd cmd, uint16 flags, player_t *pl,
                         object_t *op, char *data)
{
    char *cp = data;

    *((uint32 *)cp) = op->count;
    cp += 4;

    /* For DELITEM that's it. For others it depends also on flags. */
    if (cmd != SERVER_CMD_DELITEM)
    {
        object_t *where = op->env,
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
            char    name[SMALL_BUF];
            object_t *who = (pl) ? pl->ob : NULL;
            size_t len;

            if (snprintf(name, SMALL_BUF, "%s", query_name(op, who, ARTICLE_NONE, (op->type == RING || op->type == AMULET) ? 1 : 0)) < 0)
            {
                strcpy(name + SMALL_BUF - 4, "...");
            }

            name[SMALL_BUF - 1] = '\0';

            len = strlen(name);
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
                    float speed = ABS(op->speed);

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
static uint32 ClientFlags(object_t *op)
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
        QUERY_FLAG(op, FLAG_IS_MAGICAL))
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
object_t *esrv_get_ob_from_count(object_t *who, tag_t count)
{
    player_t *pl;
    object_t *this,
           *next,
           *that,
           *next2;

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
        FOREACH_OBJECT_IN_OBJECT(this, who, next)
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
            msp_t *msp = MSP_KNOWN(who);

            FOREACH_OBJECT_IN_MSP(this, msp, next)
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
        FOREACH_OBJECT_IN_OBJECT(this, who, next)
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
                FOREACH_OBJECT_IN_OBJECT(that, this, next2)
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

        if (who->map)
        {
            msp_t *msp = MSP_KNOWN(who);

            FOREACH_OBJECT_IN_MSP(this, msp, next)
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
                    FOREACH_OBJECT_IN_OBJECT(that, this, next2)
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
static object_t *GetObjFromCount(object_t *what, tag_t count)
{
    object_t *this;

    for (this = what; this; this = this->below)
    {
        if (this->count == count)
        {
            return this;
        }
        else if (this->inv)
        {
            object_t *that = GetObjFromCount(this->inv, count);

            if (that)
            {
                return that;
            }
        }
    }

    return NULL;
}
