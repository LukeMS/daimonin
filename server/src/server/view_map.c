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

/* This block is basically taken from socket.c - I assume if it works there,
 * it should work here.
 */
#ifndef WIN32 /* ---win32 exclude unix headers */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif /* win32 */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

/* helper functions for draw_client_map */
static inline int get_tiled_map_id(player_t *pl, struct map_t *map)
{
    int i;

    if (!pl->last_update)
        return 0;

    /* we assume that last_update, if != NULL, is not swaped out or something.
     * IF we ever put a player on a longer sleep, be sure to nullify last_update
     */
    for (i = 0; i < TILING_DIRECTION_NROF; i++)
    {
            if (pl->last_update-> tiling.tile_path[i] == map->path)
                return i+1;
    }
    return 0;
}

static inline void copy_lastmap(NewSocket *ns, int dx, int dy)
{
    view_map_t  newmap;
    int         x, y;

    /* the x and y here are coordinates for the new map, i.e. if we moved
     (dx,dy), newmap[x][y] = oldmap[x-dx][y-dy] */
    for (x = 0; x < ns->mapx; x++)
    {
        for (y = 0; y < ns->mapy; y++)
        {
            if (x + dx < 0 || x + dx >= ns->mapx || y + dy < 0 || y + dy >= ns->mapy)
            {
                memset(&(newmap.cells[x][y]), 0, sizeof(view_msp_t));
                continue;
            }
            memcpy(&(newmap.cells[x][y]), &(ns->lastmap.cells[x + dx][y + dy]), sizeof(view_msp_t));
        }
    }
    memcpy(&(ns->lastmap), &newmap, sizeof(view_map_t));
}


/* do some checks, map name and LOS and then draw the map */
void draw_client_map(player_t *pl)
{
    object_t *who  = pl->ob;
    int       redraw_below = FALSE;

    /* IF player is just joining the game, he isn't on a map,
    * If so, don't try to send them a map.  All will
     * be OK once they really log in.
     */
    if (!who->map || who->map->in_memory != MAP_MEMORY_ACTIVE)
        return;

    /* if we has changed somewhere the map - prepare map data */
    pl->map_update_cmd = MAP_UPDATE_CMD_SAME;
    if (pl->last_update != who->map)
    {
        int tile_map = get_tiled_map_id(pl, who->map);

        if(!pl->last_update || !tile_map) /* we are on a new map or set? */
        {
            pl->map_update_cmd = MAP_UPDATE_CMD_NEW;
            memset(&(pl->socket.lastmap), 0, sizeof(view_map_t));
            pl->last_update = who->map;
            redraw_below=TRUE;
        }
        else /* because tile_map can never != 0 if pl->last_update is NULL, tile_map always valid here */
        {
            pl->map_update_cmd = MAP_UPDATE_CMD_CONNECTED;
            pl->map_update_tile = tile_map;
            redraw_below=TRUE;
            /* be here means, we have moved to a known, connected map related to our last position!
             * lets calculate the offsets to our last position,
             * which is: pl->last_update - pos: map_tile_x,map_tile_y
             */
            switch(tile_map-1)
            {
                case 0:
                    pl->map_off_x = who->x - pl->map_tile_x;
                    pl->map_off_y = -(pl->map_tile_y + (MAP_HEIGHT(who->map) - who->y));
                break;

                case 1:
                    pl->map_off_y = who->y - pl->map_tile_y;
                    pl->map_off_x = (MAP_WIDTH(who->map) - pl->map_tile_x) + who->x;
                break;

                case 2:
                    pl->map_off_x = who->x - pl->map_tile_x;
                    pl->map_off_y = (MAP_HEIGHT(who->map) - pl->map_tile_y) + who->y;
                break;

                case 3:
                    pl->map_off_y = who->y - pl->map_tile_y;
                    pl->map_off_x = -(pl->map_tile_x + (MAP_WIDTH(who->map) - who->x));
                break;

                case 4:
                    pl->map_off_y = -(pl->map_tile_y + (MAP_HEIGHT(who->map) - who->y));
                    pl->map_off_x = (MAP_WIDTH(who->map) - pl->map_tile_x) + who->x;
                break;

                case 5:
                    pl->map_off_x = (MAP_WIDTH(who->map) - pl->map_tile_x) + who->x;
                    pl->map_off_y = (MAP_HEIGHT(who->map) - pl->map_tile_y) + who->y;
                break;

                case 6:
                    pl->map_off_y = (MAP_HEIGHT(who->map) - pl->map_tile_y) + who->y;
                    pl->map_off_x = -(pl->map_tile_x + (MAP_WIDTH(who->map) - who->x));
                break;

                case 7:
                    pl->map_off_x = -(pl->map_tile_x + (MAP_WIDTH(who->map) - who->x));
                    pl->map_off_y = -(pl->map_tile_y + (MAP_HEIGHT(who->map) - who->y));
                break;
            }
            /*LOG(llevDebug, "**** Connected: %d - %d,%d\n",  tile_map-1, pl->map_off_x, pl->map_off_y);*/
            copy_lastmap(&pl->socket, pl->map_off_x, pl->map_off_y);
            pl->last_update = who->map;
        }
    }
    else /* check still on the same postion */
    {
            if(pl->map_tile_x != who->x || pl->map_tile_y != who->y)
            {
                copy_lastmap(&pl->socket, who->x-pl->map_tile_x, who->y-pl->map_tile_y);
                redraw_below=TRUE;
            }
    }

    /* do LOS after calls to msp_update */
    if (pl->update_los)
    {
        update_los(pl);
        pl->update_los = 0;
    }

    draw_client_map2(pl);

    if (redraw_below) /* redraw below windows? (and backbuffer now position) */
    {
        /* backbuffer position so we can determinate we have moved or not */
        pl->map_tile_x = who->x;
        pl->map_tile_y = who->y;
        pl->socket.look_position = pl->socket.look_flag = 0;
        esrv_send_below(pl);
    }
}

/* TODO: The following defines should be moved to protocol.h and the client
 * code that receives this cmd (commands.c:Map2Cmd()) should be updated
 * accordingly.
 *
 * -- Smacky 20140831 */
/* mask is 16 bits with 6-10 being the y and 11-15 being the x. This means a
 * max possible viewable map of 31x31 (Dai has always had a fixed 17x17). */
#define MASK_FLAG_OVER  (1 << 0)
#define MASK_FLAG_UNDER (1 << 1)
#define MASK_FLAG_FMASK (1 << 2)
#define MASK_FLAG_FLOOR (1 << 3)
#define MASK_FLAG_ALPHA (1 << 4)
#define MASK_FLAG_EXTRA (1 << 5)

#define EXT_FLAG_MPART_OVER  (1 << 0)
#define EXT_FLAG_MPART_UNDER (1 << 1)
#define EXT_FLAG_FLAGS_OVER  (1 << 3)
#define EXT_FLAG_FLAGS_UNDER (1 << 4)
#define EXT_FLAG_DMG         (1 << 6)
#define EXT_FLAG_PNAME       (1 << 7)

/* PNAME_FLAG_* and DMG_FLAG_* could be merged meaning we only sned 1 extra
 * byte of flags instead of 2 for msps where a player is damaged/healed. Would
 * require x.Y.z update. */
#define PNAME_FLAG_OVER  (1 << 0)
#define PNAME_FLAG_UNDER (1 << 1)

#define DMG_FLAG_OVER  (1 << 0)
#define DMG_FLAG_UNDER (1 << 1)
#define DMG_FLAG_SELF  (1 << 2)
#define DMG_FLAG_KILL  (1 << 3)

/* The problem to "personalize" a map view is that we have to access here the objects
 * we want to draw. This means alot of memory access in different areas. Iam not
 * sure about the speed increase when we put all this info in the map node. First, this
 * will be some static memory more per node. Second, we have to force to draw it for
 * every object... Here is some we can optimize, but it should happen very careful.
 */
/* this kind of map update is overused and outdated. We need for all this special stuff
 * to send the object ID to the client - and we use then the object ID to attach more data
 * when we update the object.
 */

void draw_client_map2(player_t *pl)
{
    static uint32   map2_count  = 0;
    object_t       *who = pl->ob;
    msp_slice_t     msp_slice;
    NewSocket      *ns = &pl->socket;
    sockbuf_struct *sbptr;
    uint8           show = (pl->gmaster_matrix) ? pl->gmaster_matrix : ((1 << MSP_CLAYER_NROF) - 1);
    sint16          x, y, ax, ay, x_start;
    uint32          xrays = QUERY_FLAG(who, FLAG_XRAYS),
                    infra = QUERY_FLAG(who, FLAG_SEE_IN_DARK);
#ifdef DEBUG_CORE_MAP
    int tile_count  = 0;
#endif
 
    map2_count++;      /* we need this to decide quickly we have updated a object before here */

    if ((pl->gmaster_mode & GMASTER_MODE_SA))
    {
        msp_slice = MSP_SLICE_GMASTER;
    }
    else if (QUERY_FLAG(who, FLAG_SEE_INVISIBLE))
    {
        msp_slice = MSP_SLICE_INVISIBLE;
    }
    else
    {
        msp_slice = MSP_SLICE_VISIBLE;
    }

    get_tad(who->map->tadnow, who->map->tadoffset);
    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);
    sbptr = ACTIVE_SOCKBUF(ns);
    SockBuf_AddChar(sbptr, pl->map_update_cmd); /* marker */

    if(pl->map_update_cmd != MAP_UPDATE_CMD_SAME)
    {
        SockBuf_AddString(sbptr, who->map->name, strlen(who->map->name));
        SockBuf_AddString(sbptr, STRING_SAFE(who->map->music),
                          strlen(STRING_SAFE(who->map->music)));

        if(pl->map_update_cmd == MAP_UPDATE_CMD_CONNECTED)
        {
            SockBuf_AddChar(sbptr, pl->map_update_tile);
            SockBuf_AddChar(sbptr, pl->map_off_x);
            SockBuf_AddChar(sbptr, pl->map_off_y);
        }
        else
        {
            SockBuf_AddChar(sbptr, MAP_WIDTH(who->map));
            SockBuf_AddChar(sbptr, MAP_HEIGHT(who->map));
        }
    }

    SockBuf_AddChar(sbptr, who->x);
    SockBuf_AddChar(sbptr, who->y);
    /* x,y are the real map locations.  ax, ay are viewport relative
     * locations.
     */

    /* i don't trust all compilers to optimize it in this BIG loop */
    x_start = (who->x + (pl->socket.mapx + 1) / 2) - 1;
    ay = pl->socket.mapy - 1;

    for (y = (who->y + (pl->socket.mapy + 1) / 2) - 1; y >= who->y - pl->socket.mapy_2; y--, ay--)
    {
        ax = pl->socket.mapx - 1;

        for (x = x_start; x >= who->x - pl->socket.mapx_2; x--, ax--)
        {
            int           blos = pl->blocked_los[ax][ay];
            view_msp_t   *view_msp = &pl->socket.lastmap.cells[ax][ay];
            map_t        *m;
            sint16        nx,
                          ny;
            msp_t        *msp;
            object_t     *clayer_under = NULL,
                         *clayer_over = NULL;
            char         *pname_under = NULL,
                         *pname_over = NULL;
            uint16        mask = (ax & 0x1f) << 11 | (ay & 0x1f) << 6,
                          face_floor = 0,
                          face_fmask = 0,
                          face_under = 0,
                          face_over = 0;
             uint8        mpart_under = 0,
                          mpart_over = 0,
                          flag_under = 0,
                          flag_over = 0,
                          probe_under = 0,
                          probe_over = 0,
                          ext_flag = 0,
                          dmg_flag = 0;
             sint16       dmg_self = 0,
                          dmg_under = 0,
                          dmg_over = 0;
#ifdef USE_TILESTRETCHER
            sint16        z1 = 0;
#endif
            int           d;

            /* space is out of map OR blocked.  Update space and clear values if needed */
            if ((blos & (BLOCKED_LOS_OUT_OF_MAP | BLOCKED_LOS_BLOCKED)))
            {
                if (view_msp->count != -1)
                {
#ifdef DEBUG_CORE_MAP
                    tile_count++;
#endif
                    SockBuf_AddShort(sbptr, mask); /* a position mask without any flags = clear cell */
                    memset(view_msp, 0, sizeof(view_msp_t));
                    view_msp->count = -1;
                }

                continue;
            }

            /* it IS a valid map -but which? */
            m = who->map;
            nx = x;
            ny = y;
            msp = MSP_GET(m, nx, ny);

            /* this should be caught in LOS function... so its a glitch, except
             * we are gmaster_wizpass -- there we skip all this LOS stuff. */
            if (!msp)
            {
                if (!pl->gmaster_wizpass)
                {
                    LOG(llevInfo, "INFO:: draw_client_map2() out_of_map for player <%s> map:%s (%d,%d)\n",
                        STRING_OBJ_NAME(who), STRING_MAP_PATH(m), x, y);
                }

                if (view_msp->count != -1)
                {
#ifdef DEBUG_CORE_MAP
                    tile_count++;
#endif
                    SockBuf_AddShort(sbptr, mask);
                    memset(view_msp, 0, sizeof(view_msp_t));
                    view_msp->count = -1;
                }

                continue;
            }

            /* Lets check for changed blocksview - but only msps which have an
             * impact to our LOS. */
            if (!(blos & BLOCKED_LOS_IGNORE)) // border msp, we can ignore every LOS change
            {
                if ((msp->flags & MSP_FLAG_OBSCURESVIEW))
                {
                    if (!(blos & BLOCKED_LOS_OBSCURESVIEW))
                    {
                        pl->update_los = 1;
                    }
                }
                else
                {
                    if ((blos & BLOCKED_LOS_OBSCURESVIEW))
                    {
                        pl->update_los = 1;
                    }
                }

                if ((msp->flags & MSP_FLAG_BLOCKSVIEW))
                {
                    if (!(blos & BLOCKED_LOS_BLOCKSVIEW))
                    {
                        pl->update_los = 1;
                    }
                }
                else
                {
                    if ((blos & BLOCKED_LOS_BLOCKSVIEW))
                    {
                        pl->update_los = 1;
                    }
                }
            }

            /* lets calc the darkness/light value for this tile.*/
            d = (pl->personal_light) ?
                msp->flooding_brightness + brightness[pl->personal_light] :
                MSP_GET_REAL_BRIGHTNESS(msp);

            if (d <= 0) /* tile is not normal visible */
            {
                /* (xray) or (infravision with mobile(aka alive) or player on a tile)? */
                if (xrays ||
                    (infra &&
                     (msp->flags & (MSP_FLAG_PLAYER | MSP_FLAG_ALIVE))))
                {
                    d = 100; /* make spot visible again */
                }
                else
                {
                    if (view_msp->count != -1)
                    {
#ifdef DEBUG_CORE_MAP
                         tile_count++;
#endif
                        SockBuf_AddShort(sbptr, mask);
                        memset(view_msp, 0, sizeof(view_msp_t));
                        view_msp->count = -1;
                    }
                    continue;
                }
            }
            /* when we arrived here, this tile IS visible - now lets collect the data of it
                 * and update the client when something has changed.
                 */
            if ((blos & BLOCKED_LOS_OBSCURED))
            {
                d /= 4;
            }

            /* we should do this with a table */
            if (d > 640)
                d = 210;
            else if (d > 320)
                d = 180;
            else if (d > 160)
                d = 150;
            else if (d > 80)
                d = 120;
            else if (d > 40)
                d = 90;
            else if (d > 20)
                d = 60;
            else
                d = 30;

            if (view_msp->count != d)
            {
                mask |= MASK_FLAG_ALPHA;
                view_msp->count = d;
            }

            /* If the slice is not synced for this msp (means the clayers do
             * not reflect the slayers), sync it now. */
            if (!(msp->slices_synced & (1 << msp_slice)))
            {
                msp_clayer_t c;
                msp_slayer_t s;

#ifdef DEBUG_CORE
                LOG(llevDebug, "SYNC SLICE %d of %s %d,%d\n",
                    msp_slice, STRING_MAP_PATH(m), nx, ny);

#endif
                /* First NULL the unsynced msp->clayers. */
                for (c = MSP_CLAYER_NROF - MSP_CLAYER_UNSLICED - 2; c >= 0; c--)
                {
                    msp->clayer[msp_slice][c] = NULL;
                }

                /* Set clayer_over and clayer_under as necessary. */
                for (s = MSP_SLAYER_NROF - MSP_SLAYER_UNSLICED - 2; s >= 0; s--)
                {
                    object_t *slayer = msp->slayer[msp_slice][s];

                    if (slayer)
                    {
                        clayer_over = slayer;

                        while (--s >= 0)
                        {
                            if ((slayer = msp->slayer[msp_slice][s]))
                            {
                                clayer_under = slayer;
                                break;
                            }
                        }

                        break;
                    }
                }

#ifdef USE_SLAYER_MONSTER
                /* If the msp holds an ITEMA/ITEMB and a FEATURE (and no other
                 * MSP_SLAYERs) these will be assigned to under and over
                 * respectively. Here we swap them round. This means the item
                 * is drawn over the feature. */
                if (clayer_under && // implies clayer_over is non-NULL
                    clayer_over->layer == MSP_SLAYER_FEATURE) // implie clayer_under->layer is ITEMA/ITEMB
                {
                    object_t *o = clayer_under;

                    clayer_under = clayer_over;
                    clayer_over = o;
                }

#endif

                /* Update the msp->clayers and mark this slice as synced. */
                msp->clayer[msp_slice][MSP_CLAYER_UNDER - MSP_CLAYER_UNSLICED - 1] = clayer_under;
                msp->clayer[msp_slice][MSP_CLAYER_OVER - MSP_CLAYER_UNSLICED - 1] = clayer_over;
                msp->slices_synced |= (1 << msp_slice);
            }

            /* Assign msp->clayers to clayer_over/under. */
            clayer_under = msp->clayer[msp_slice][MSP_CLAYER_UNDER - MSP_CLAYER_UNSLICED - 1];
            clayer_over = msp->clayer[msp_slice][MSP_CLAYER_OVER - MSP_CLAYER_UNSLICED - 1];

            /* Always see self. */
            /* TODO: This block needs to be looked at carefully. Should be a
             * simple task...
             *
             * -- Smacky 20150127 */
            if (MSP_KNOWN(who) == msp)
            {
#if 0
                /* If there is MSP_SLAYER_EFFECT object in clayer_over put who
                 * in clayer_under otherwise move clayer_over (which may be
                 * NULL) to clayer_under and put who in clayer_over. */
                if (clayer_over &&
                    clayer_over->layer == MSP_SLAYER_EFFECT)
                {
                    clayer_under = who;
                }
                else
                {
                    clayer_under = clayer_over;
                    clayer_over = who;
                }
#else
                if (clayer_over &&
                    clayer_over->type == PLAYER)
                {
                    clayer_over = who;
                }
                else if (clayer_under &&
                         clayer_under->type == PLAYER)
                {
                    clayer_under = who;
                }
#endif
            }

            /* CLAYER 0 - floor. */
            if ((show & (1 << MSP_CLAYER_FLOOR)))
            {
            if (pl->gmaster_matrix &&
                !msp->floor_face)
            {
                face_floor = no_floor_face->number;
            }
            else if (msp->floor_face)
            {
                face_floor = msp->floor_face->number;
#ifdef USE_TILESTRETCHER

                if (msp->floor_z != 0 )
                {
                   z1 = msp->floor_z;
# ifdef DEBUG_CORE_MAP
                   LOG(llevDebug,"Z1 = %d\n", z1);
# endif
                }
#endif
            }
            else
            {
                face_floor = 0;
            }
            }

            /* CLAYER 1 - fmask. */
            if ((show & (1 << MSP_CLAYER_FMASK)))
            {
            if (pl->gmaster_matrix &&
                msp->first &&
                msp->first->layer == MSP_SLAYER_SYSTEM)
            {
                face_fmask = msp->first->face->number;
            }
            else if (msp->mask_face)
            {
                face_fmask = msp->mask_face->number;
            }
            else
            {
                face_fmask = 0;
            }
            }

            /* CLAYER 2 - thing under. */
            if ((show & (1 << MSP_CLAYER_UNDER)))
            {
            if (clayer_under)
            {
                object_t *head = (clayer_under->head) ? clayer_under->head : clayer_under;

                face_under = clayer_under->face->number;
                mpart_under = clayer_under->quick_pos;

                if (mpart_under)
                {
                    if (head->update_tag == map2_count)
                    {
                        clayer_under = NULL;
                    }
                    else
                    {
                        head->update_tag = map2_count;
                        face_under = head->face->number;
                    }
                }

                if (face_under == blank_face->number)
                {
                    clayer_under = NULL;
                }

                if (!clayer_under)
                {
                    face_under = 0;
                    mpart_under = 0;
                    flag_under = 0;
                    probe_under = 0;
                }
                else
                {
                    /* Well, i have no idea how to send for each player his own
                     * face without this. The way we can avoid this is to lets
                     * draw the player by the client only and just to tell the
                     * client what direction and animation the player now has.
                     * Currently we can't handle client map animation, even it
                     * should not hard to be done. MT */
                    flag_under = (clayer_under->flags[0] & 0x7f) |
                        ((IS_GMASTER_INVIS(clayer_under)) ? FFLAG_INVISIBLE : 0);

                    if (clayer_under->last_damage != 0 &&
                        clayer_under->damage_round_tag == ROUND_TAG)
                    {
                        if (clayer_under == who)
                        {
                            dmg_flag |= 0x4;
                            dmg_self = clayer_under->last_damage;
                        }
                        else
                        {
                            dmg_flag |= 0x1;
                            dmg_under = clayer_under->last_damage;
                        }
                    }

                    /* show target to player (this is personlized data)*/
                    if (pl->target_object_count == head->count)
                    {
                        flag_under |= FFLAG_PROBE;

                        if (head->stats.hp)
                        {
                            probe_under = (uint8)((double)head->stats.hp / ((double)head->stats.maxhp / (double)100.0));
                        }
                    }
                }
            }
            }

            /* CLAYER 3 - thing over. */
            if ((show & (1 << MSP_CLAYER_OVER)))
            {
            if (clayer_over)
            {
                object_t *head = (clayer_over->head) ? clayer_over->head : clayer_over;

                face_over = clayer_over->face->number;
                mpart_over = clayer_over->quick_pos;

                if (mpart_over)
                {
                    if (head->update_tag == map2_count)
                    {
                        clayer_over = NULL;
                    }
                    else
                    {
                        head->update_tag = map2_count;
                        face_over = head->face->number;
                    }
                }

                if (face_over == blank_face->number)
                {
                    clayer_over = NULL;
                }

                if (!clayer_over)
                {
                    face_over = 0;
                    mpart_over = 0;
                    flag_over = 0;
                    probe_over = 0;
                }
                else
                {
                    /* Well, i have no idea how to send for each player his own
                     * face without this. The way we can avoid this is to lets
                     * draw the player by the client only and just to tell the
                     * client what direction and animation the player now has.
                     * Currently we can't handle client map animation, even it
                     * should not hard to be done. MT */
                    flag_over = (clayer_over->flags[0] & 0x7f) |
                        ((IS_GMASTER_INVIS(clayer_over)) ? FFLAG_INVISIBLE : 0);

                    if (clayer_over->last_damage != 0 &&
                        clayer_over->damage_round_tag == ROUND_TAG)
                    {
                        if (clayer_over == who)
                        {
                            dmg_flag |= 0x4;
                            dmg_self = clayer_over->last_damage;
                        }
                        else
                        {
                            dmg_flag |= 0x1;
                            dmg_over = clayer_over->last_damage;
                        }
                    }

                    /* show target to player (this is personlized data)*/
                    if (pl->target_object_count == head->count)
                    {
                        flag_over |= FFLAG_PROBE;

                        if (head->stats.hp)
                        {
                            probe_over = (uint8)((double)head->stats.hp / ((double)head->stats.maxhp / (double)100.0));
                        }
                    }
                }
            }
            }

            if (view_msp->faces[MSP_CLAYER_FLOOR] != face_floor)
            {
                mask |= MASK_FLAG_FLOOR;
                view_msp->faces[MSP_CLAYER_FLOOR] = face_floor;
            }

            if (view_msp->faces[MSP_CLAYER_FMASK] != face_fmask)
            {
                mask |= MASK_FLAG_FMASK;
                view_msp->faces[MSP_CLAYER_FMASK] = face_fmask;
            }

            if (view_msp->faces[MSP_CLAYER_UNDER] != face_under ||
                view_msp->quick_pos[MSP_CLAYER_UNDER] != mpart_under)
            {
                mask |= MASK_FLAG_UNDER;

                if (clayer_under &&
                    clayer_under->type == PLAYER)
                {
                    ext_flag |= EXT_FLAG_PNAME;
                    pname_under = CONTR(clayer_under)->quick_name;
                }

                if (mpart_under)
                {
                    ext_flag |= EXT_FLAG_MPART_UNDER;
                }

                view_msp->faces[MSP_CLAYER_UNDER] = face_under;
                view_msp->quick_pos[MSP_CLAYER_UNDER] = mpart_under;
            }

            /* check, set and buffer ext flag */
            if (view_msp->fflag[MSP_CLAYER_UNDER] != flag_under ||
                view_msp->ff_probe[MSP_CLAYER_UNDER] != probe_under)
            {
                /* This was changed in r7818/7819 to a test on flag_under
                 * which fixes the 'ghost target' bug. In itself this probably
                 * is correct but unfortunately the 0.10.z client also tests
                 * face meaning that a sc mismatch here means targeting does
                 * not loop but gets stuck on the last map object. Tweaking the
                 * client code too should fix this but while 0.10.7 is in
i                * development it is easiest to leave the 'ghost target' bug in
                 * and at least have targeting looping (and no, testing both
                 * face and flag does not work). */
                if (face_under)
                {
                    ext_flag |= EXT_FLAG_FLAGS_UNDER;
                }

                /* Does this make sense? I get the first part because we'll
                 * have skipped the above ff_probe test if we passed the fflag
                 * test. Well I sort of get it.  As ff_probe is reset here come
                 * what may who cares if we test or not? And then the second
                 * part means we turn on FFLAG_PROBE but only if it already is
                 * on... */
                if (view_msp->ff_probe[MSP_CLAYER_UNDER] != probe_under && 
                    (flag_under & FFLAG_PROBE)) /* ugly, but we must test it twice to submit implicit changes right */
                {
                    flag_under |= FFLAG_PROBE;
                }

                view_msp->fflag[MSP_CLAYER_UNDER] = flag_under;
                view_msp->ff_probe[MSP_CLAYER_UNDER] = probe_under;
            }

            if (view_msp->faces[MSP_CLAYER_OVER] != face_over ||
                view_msp->quick_pos[MSP_CLAYER_OVER] != mpart_over)
            {
                mask |= MASK_FLAG_OVER;

                if (clayer_over &&
                    clayer_over->type == PLAYER)
                {
                    ext_flag |= EXT_FLAG_PNAME;
                    pname_over = CONTR(clayer_over)->quick_name;
                }

                if (mpart_over)
                {
                    ext_flag |= EXT_FLAG_MPART_OVER;
                }

                view_msp->faces[MSP_CLAYER_OVER] = face_over;
                view_msp->quick_pos[MSP_CLAYER_OVER] = mpart_over;
            }

            /* check, set and buffer ext flag */
            if (view_msp->fflag[MSP_CLAYER_OVER] != flag_over ||
                view_msp->ff_probe[MSP_CLAYER_OVER] != probe_over)
            {
                /* This was changed in r7818/7819 to a test on flag_over
                 * which fixes the 'ghost target' bug. In itself this probably
                 * is correct but unfortunately the 0.10.z client also tests
                 * face meaning that a sc mismatch here means targeting does
                 * not loop but gets stuck on the last map object. Tweaking the
                 * client code too should fix this but while 0.10.7 is in
i                * development it is easiest to leave the 'ghost target' bug in
                 * and at least have targeting looping (and no, testing both
                 * face and flag does not work). */
                if (face_over)
                {
                    ext_flag |= EXT_FLAG_FLAGS_OVER;
                }

                /* Does this make sense? I get the first part because we'll
                 * have skipped the above ff_probe test if we passed the fflag
                 * test. Well I sort of get it.  As ff_probe is reset here come
                 * what may who cares if we test or not? And then the second
                 * part means we turn on FFLAG_PROBE but only if it already is
                 * on... */
                if (view_msp->ff_probe[MSP_CLAYER_OVER] != probe_over && 
                    (flag_over & FFLAG_PROBE)) /* ugly, but we must test it twice to submit implicit changes right */
                {
                    flag_over |= FFLAG_PROBE;
                }

                view_msp->fflag[MSP_CLAYER_OVER] = flag_over;
                view_msp->ff_probe[MSP_CLAYER_OVER] = probe_over;
            }

            /* perhaps we smashed some on this map position */
            /* object is gone but we catch the damage we have here done */
            if (msp->round_tag == ROUND_TAG)
            {
                dmg_flag |= DMG_FLAG_KILL;
            }

            if (dmg_flag)
            {
                ext_flag |= EXT_FLAG_DMG;
            }

            if (ext_flag)
            {
                mask |= MASK_FLAG_EXTRA;
                SockBuf_AddShort(sbptr, mask);
                SockBuf_AddChar(sbptr, ext_flag); /* push the ext_flagbyte */
            }
            else
            {
                /* well.. IS there something we have to send? */
                if (!(mask & 0x3f)) /* check all bits except the position */
                {
                    continue;
                }

#ifdef DEBUG_CORE_MAP
                tile_count++;
#endif
                SockBuf_AddShort(sbptr, mask); /* mask only */
            }

            if (pname_under)
            {
                SockBuf_AddChar(sbptr, PNAME_FLAG_UNDER);
                SockBuf_AddString(sbptr, pname_under, strlen(pname_under));
            }
            else if (pname_over)
            {
                SockBuf_AddChar(sbptr, PNAME_FLAG_OVER);
                SockBuf_AddString(sbptr, pname_over, strlen(pname_over));
            }

            /* fire & forget layer animation tags */
            if (dmg_flag)
            {
                SockBuf_AddChar(sbptr, dmg_flag);

                /*thats the special one - the red kill spot the client shows */
                /* remember we put the damage value in the map because at the time
                 * we are here at run time, the object is dead since some ticks and
                 * perhaps some else is moved on this spot and/or the old object deleted */
                if ((dmg_flag & DMG_FLAG_KILL))
                {
                    SockBuf_AddShort(sbptr, (sint16)(msp->last_damage));
                }
                if ((dmg_flag & DMG_FLAG_SELF))
                {
                    SockBuf_AddShort(sbptr, dmg_self);
                }
                if ((dmg_flag & DMG_FLAG_UNDER))
                {
                    SockBuf_AddShort(sbptr, dmg_under);
                }
                if ((dmg_flag & DMG_FLAG_OVER))
                {
                    SockBuf_AddShort(sbptr, dmg_over);
                }
            }

            if ((ext_flag & EXT_FLAG_FLAGS_UNDER))
            {
                SockBuf_AddChar(sbptr, flag_under);

                if ((flag_under & FFLAG_PROBE))
                {
                    SockBuf_AddChar(sbptr, probe_under);
                }
            }

            if ((ext_flag & EXT_FLAG_FLAGS_OVER))
            {
                SockBuf_AddChar(sbptr, flag_over);

                if ((flag_over & FFLAG_PROBE))
                {
                    SockBuf_AddChar(sbptr, probe_over);
                }
            }

            if ((mask & MASK_FLAG_ALPHA))
            {
                SockBuf_AddChar(sbptr, (char)view_msp->count);
            }

            if ((mask & MASK_FLAG_FLOOR))
            {
                SockBuf_AddShort(sbptr, face_floor);
#ifdef USE_TILESTRETCHER
                SockBuf_AddShort(sbptr, z1);   /*  <--- sending height every time... */
                /* should be reworked to send only when !=0 */
#endif
            }

            if ((mask & MASK_FLAG_FMASK))
            {
                SockBuf_AddShort(sbptr, face_fmask);
            }

            if ((mask & MASK_FLAG_UNDER))
            {
                /* TODO: I don't like this. Always marking living objects in
                 * this way can allow (fairly minor) client exploits such as
                 * faking infravision (but not in total darkness). I have a
                 * semi-formulated better approach to the question of special
                 * vision anyway but it will require an x.Y.z update.
                 *
                 * -- Smacky 20140831 */
                if (clayer_under &&
                    (clayer_under->type == MONSTER ||
                     clayer_under->type == PLAYER))
                {
                    face_under |= 0x8000;
                }

                SockBuf_AddShort(sbptr, face_under);

                if ((ext_flag & EXT_FLAG_MPART_UNDER))
                {
                    SockBuf_AddChar(sbptr, mpart_under);
                }
            }

            if ((mask & MASK_FLAG_OVER))
            {
                /* TODO: I don't like this. Always marking living objects in
                 * this way can allow (fairly minor) client exploits such as
                 * faking infravision (but not in total darkness). I have a
                 * semi-formulated better approach to the question of special
                 * vision anyway but it will require an x.Y.z update.
                 *
                 * -- Smacky 20140831 */
                if (clayer_over &&
                    (clayer_over->type == MONSTER ||
                     clayer_over->type == PLAYER))
                {
                    face_over |= 0x8000;
                }

                SockBuf_AddShort(sbptr, face_over);

                if ((ext_flag & EXT_FLAG_MPART_OVER))
                {
                    SockBuf_AddChar(sbptr, mpart_over);
                }
            }
        } /* for x loop */
    } /* for y loop */

    /* Verify that we in fact do need to send this */
    if (SOCKBUF_REQUEST_BUFSIZE(sbptr) > 3)
    {
#ifdef DEBUG_CORE_MAP
        LOG(llevDebug, "MAP2: (%d) %d %d send tiles: %d \n", SOCKBUF_REQUEST_BUFSIZE(sbptr), sbptr->len, sbptr->request_len, tile_count);
#endif
        SOCKBUF_REQUEST_FINISH(ns, SERVER_CMD_MAP2, SOCKBUF_DYNAMIC);
    }
    else
        SOCKBUF_REQUEST_RESET(ns);
}

/* set_personal_light() sets pl->personal_light to 0 <= value <= MAX_DARKNESS.
 * Note that on an outdoors map personal_light kills the *visible* variable
 * lighting (for pl only) (this was intentional when personal_light (then
 * dm_light) was just for MM use as during map testing variable lighting is
 * rarely useful; now however normal players can have personal_light so it's
 * not so useful = FIXME ;)). */
void set_personal_light(player_t *pl, int value)
{
    if (!pl)
        return;

    if (value < 0)
        value += (((value / -MAX_DARKNESS) * MAX_DARKNESS)) + MAX_DARKNESS;

    if (value > MAX_DARKNESS)
        value -= (((value - 1) / MAX_DARKNESS)) * MAX_DARKNESS;

    pl->personal_light = (uint32)value;
}
