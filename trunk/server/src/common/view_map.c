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

    The author can be reached via e-mail to info@daimonin.net
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#define GET_CLIENT_FLAGS(_O_)   ((_O_)->flags[0]&0x7f)
#define NO_FACE_SEND        (-1)

/******************************************************************************
 *
 * Start of map related commands.
 *
 ******************************************************************************/
/* Clears a map cell */
#define map_clearcell(_cell_) memset((_cell_), 0, sizeof(MapCell));(_cell_)->count=-1

/* helper functions for draw_client_map */
static inline int get_tiled_map_id(player *pl, struct mapdef *map)
{
    int i;

    if (!pl->last_update)
        return 0;

    /* we assume that last_update, if != NULL, is not swaped out or something.
     * IF we ever put a player on a longer sleep, be sure to nullify last_update
     */
    for (i = 0; i < TILED_MAPS; i++)
    {
            if (pl->last_update-> tile_path[i] == map->path)
                return i+1;
    }
    return 0;
}

static inline void copy_lastmap(NewSocket *ns, int dx, int dy)
{
    struct Map  newmap;
    int         x, y;

    /* the x and y here are coordinates for the new map, i.e. if we moved
     (dx,dy), newmap[x][y] = oldmap[x-dx][y-dy] */
    for (x = 0; x < ns->mapx; x++)
    {
        for (y = 0; y < ns->mapy; y++)
        {
            if (x + dx < 0 || x + dx >= ns->mapx || y + dy < 0 || y + dy >= ns->mapy)
            {
                memset(&(newmap.cells[x][y]), 0, sizeof(MapCell));
                continue;
            }
            memcpy(&(newmap.cells[x][y]), &(ns->lastmap.cells[x + dx][y + dy]), sizeof(MapCell));
        }
    }
    memcpy(&(ns->lastmap), &newmap, sizeof(struct Map));
}


/* do some checks, map name and LOS and then draw the map */
void draw_client_map(object *plobj)
{
    player *pl = CONTR(plobj);
    int redraw_below=FALSE;

    if (plobj->type != PLAYER)
    {
        LOG(llevBug, "BUG: draw_client_map called with non player/non eric-server (%s)\n", plobj->name);
        return;
    }

    /* IF player is just joining the game, he isn't on a map,
    * If so, don't try to send them a map.  All will
     * be OK once they really log in.
     */
    if (!plobj->map || plobj->map->in_memory != MAP_IN_MEMORY)
        return;

    /* if we has changed somewhere the map - prepare map data */
    pl->map_update_cmd = MAP_UPDATE_CMD_SAME;
    if (pl->last_update != plobj->map)
    {
        int tile_map = get_tiled_map_id(pl, plobj->map);

        if(!pl->last_update || !tile_map) /* we are on a new map or set? */
        {
            pl->map_update_cmd = MAP_UPDATE_CMD_NEW;
            memset(&(pl->socket.lastmap), 0, sizeof(struct Map));
            pl->last_update = plobj->map;
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
                    pl->map_off_x = plobj->x - pl->map_tile_x;
                    pl->map_off_y = -(pl->map_tile_y + (MAP_HEIGHT(plobj->map) - plobj->y));
                break;

                case 1:
                    pl->map_off_y = plobj->y - pl->map_tile_y;
                    pl->map_off_x = (MAP_WIDTH(plobj->map) - pl->map_tile_x) + plobj->x;
                break;

                case 2:
                    pl->map_off_x = plobj->x - pl->map_tile_x;
                    pl->map_off_y = (MAP_HEIGHT(plobj->map) - pl->map_tile_y) + plobj->y;
                break;

                case 3:
                    pl->map_off_y = plobj->y - pl->map_tile_y;
                    pl->map_off_x = -(pl->map_tile_x + (MAP_WIDTH(plobj->map) - plobj->x));
                break;

                case 4:
                    pl->map_off_y = -(pl->map_tile_y + (MAP_HEIGHT(plobj->map) - plobj->y));
                    pl->map_off_x = (MAP_WIDTH(plobj->map) - pl->map_tile_x) + plobj->x;
                break;

                case 5:
                    pl->map_off_x = (MAP_WIDTH(plobj->map) - pl->map_tile_x) + plobj->x;
                    pl->map_off_y = (MAP_HEIGHT(plobj->map) - pl->map_tile_y) + plobj->y;
                break;

                case 6:
                    pl->map_off_y = (MAP_HEIGHT(plobj->map) - pl->map_tile_y) + plobj->y;
                    pl->map_off_x = -(pl->map_tile_x + (MAP_WIDTH(plobj->map) - plobj->x));
                break;

                case 7:
                    pl->map_off_x = -(pl->map_tile_x + (MAP_WIDTH(plobj->map) - plobj->x));
                    pl->map_off_y = -(pl->map_tile_y + (MAP_HEIGHT(plobj->map) - plobj->y));
                break;
            }
            /*LOG(llevDebug, "**** Connected: %d - %d,%d\n",  tile_map-1, pl->map_off_x, pl->map_off_y);*/
            copy_lastmap(&pl->socket, pl->map_off_x, pl->map_off_y);
            pl->last_update = plobj->map;
        }
    }
    else /* check still on the same postion */
    {
            if(pl->map_tile_x != plobj->x || pl->map_tile_y != plobj->y)
            {
                copy_lastmap(&pl->socket, plobj->x-pl->map_tile_x, plobj->y-pl->map_tile_y);
                redraw_below=TRUE;
            }
    }

    if(redraw_below) /* redraw below windows? (and backbuffer now position) */
    {
        /* backbuffer position so we can determinate we have moved or not */
        pl->map_tile_x = plobj->x;
        pl->map_tile_y = plobj->y;
        pl->socket.below_clear = 1; /* draw_client_map2() has implicit cleared belowed - don't send a clear again */
        pl->socket.update_tile = 0; /* redraw it */
        pl->socket.look_position = 0; /* reset the "show only x items of all items belows" counter */
    }

    /* do LOS after calls to update_position */
    if (!QUERY_FLAG(plobj, FLAG_WIZ) && pl->update_los)
    {
        update_los(plobj);
        pl->update_los = 0;
    }

    draw_client_map2(plobj);
}


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

static int  darkness_table[]    =
{
    0, 10, 30, 60, 120, 260, 480, 960
};

void draw_client_map2(object *pl)
{
    static uint32   map2_count  = 0;
    NewSocket        *ns;
    sockbuf_struct    *sbptr;
    MapCell        *mp;
    MapSpace       *msp;
    New_Face       *face;
    mapstruct      *m;
    player         *pl_ptr = CONTR(pl);
    object         *tmp = NULL, *tmph = NULL, *pname2 = NULL, *pname3 = NULL, *pname4 = NULL;
    int             x, y, ax, ay, d, nx, ny, probe_tmp;
    int             x_start, personal_light = 0;
    int             dark, flag_tmp, special_vision;
    int             quick_pos_1, quick_pos_2, quick_pos_3;
    int             inv_flag    = QUERY_FLAG(pl, FLAG_SEE_INVISIBLE) ? 0 : 1;
    uint16          face_num0, face_num1, face_num2, face_num3, face_num1m, face_num2m, face_num3m;
    uint16          mask;
    int pname_flag, ext_flag, dmg_flag;
    int dmg_layer2, dmg_layer1, dmg_layer0;
    int wdark;
#ifdef USE_TILESTRETCHER
    sint16 z1;
#endif

#ifdef DEBUG_CORE_MAP
    int tile_count  = 0;
#endif

    if (pl_ptr->personal_light)
        personal_light = global_darkness_table[pl_ptr->personal_light];

    wdark = darkness_table[world_darkness];
    special_vision = (QUERY_FLAG(pl, FLAG_XRAYS) ? 1 : 0) | (QUERY_FLAG(pl, FLAG_SEE_IN_DARK) ? 2 : 0);

    map2_count++;      /* we need this to decide quickly we have updated a object before here */

    ns = &pl_ptr->socket;
    SOCKBUF_REQUEST_BUFFER(ns, SOCKET_SIZE_MEDIUM);
    sbptr = ACTIVE_SOCKBUF(ns);

    SockBuf_AddChar(sbptr, pl_ptr->map_update_cmd); /* marker */
    if(pl_ptr->map_update_cmd != MAP_UPDATE_CMD_SAME)
    {
#if 0
        SockBuf_AddString(sbptr, pl->map->name, strlen(pl->map->name));
#else
        char buf[MAX_BUF];

        strcpy(buf, pl->map->name);

        if (pl->map->music)
        {
           strcat(buf, "§");
           strcat(buf, pl->map->music);
        }

        SockBuf_AddString(sbptr, buf, strlen(buf));
#endif

        if(pl_ptr->map_update_cmd == MAP_UPDATE_CMD_CONNECTED)
        {
            SockBuf_AddChar(sbptr, pl_ptr->map_update_tile);
            SockBuf_AddChar(sbptr, pl_ptr->map_off_x);
            SockBuf_AddChar(sbptr, pl_ptr->map_off_y);
        }
        else
        {
            SockBuf_AddChar(sbptr, pl->map->width);
            SockBuf_AddChar(sbptr, pl->map->height);
        }
    }

    SockBuf_AddChar(sbptr, pl->x);
    SockBuf_AddChar(sbptr, pl->y);
    /* x,y are the real map locations.  ax, ay are viewport relative
     * locations.
     */

    /* i don't trust all compilers to optimize it in this BIG loop */
    x_start = (pl->x + (pl_ptr->socket.mapx + 1) / 2) - 1;

    for (ay = pl_ptr->socket.mapy - 1,y = (pl->y + (pl_ptr->socket.mapy + 1) / 2) - 1;
         y >= pl->y - pl_ptr->socket.mapy_2;
         y--,ay--)
    {
        ax = pl_ptr->socket.mapx - 1;
        for (x = x_start; x >= pl->x - pl_ptr->socket.mapx_2; x--,ax--)
        {
            d = pl_ptr->blocked_los[ax][ay];
            mask = (ax & 0x1f) << 11 | (ay & 0x1f) << 6;

            /* space is out of map OR blocked.  Update space and clear values if needed */
            if (d & (BLOCKED_LOS_OUT_OF_MAP | BLOCKED_LOS_BLOCKED))
            {
                if (pl_ptr->socket.lastmap.cells[ax][ay].count != -1)
                {
#ifdef DEBUG_CORE_MAP
                    tile_count++;
#endif
                    SockBuf_AddShort(sbptr, mask); /* a position mask without any flags = clear cell */
                    map_clearcell(&pl_ptr->socket.lastmap.cells[ax][ay]); /* sets count to -1 too */
                }
                continue;
            }

            /* it IS a valid map -but which? */
            nx = x;ny = y;
            if (!(m = out_of_map(pl->map, &nx, &ny)))
            {
                /* this should be catched in LOS function... so its a glitch,
                         * except we are in DM mode - there we skip all this LOS stuff.
                         */
                if (!QUERY_FLAG(pl, FLAG_WIZ))
                    LOG(llevDebug, "BUG: draw_client_map2() out_of_map for player <%s> map:%s (%,%d)\n", query_name(pl),
                        pl->map->path ? pl->map->path : "<no path?>", x, y);

                if (pl_ptr->socket.lastmap.cells[ax][ay].count != -1)
                {
#ifdef DEBUG_CORE_MAP
                    tile_count++;
#endif
                    SockBuf_AddShort(sbptr, mask);
                    map_clearcell(&pl_ptr->socket.lastmap.cells[ax][ay]);/* sets count to -1 too */
                }
                continue;
            }

            msp = GET_MAP_SPACE_PTR(m, nx, ny);

            /* we need to rebuild the layer first? */
            if (msp->flags & P_NEED_UPDATE)
            {
#ifdef DEBUG_CORE
                LOG(llevDebug, "P_NEED_UPDATE (%s) pos:(%d,%d)\n", query_name(pl), nx, ny);
#endif
                msp->flags &= ~P_FLAGS_ONLY;
                update_position(m, NULL, nx, ny);
            }

            /* lets check for changed blocksview - but only tile which have
                 * an impact to our LOS.
                 */
            if (!(d & BLOCKED_LOS_IGNORE)) /* border tile, we can ignore every LOS change */
            {
                if (msp->flags & P_BLOCKSVIEW) /* tile has blocksview set? */
                {
                    if (!d) /* now its visible? */
                    {
                        /*LOG(-1,"SET_BV(%d,%d): was bv is now %d\n", nx,ny,d);*/
                        pl_ptr->update_los = 1;
                    }
                }
                else
                {
                    if (d & BLOCKED_LOS_BLOCKSVIEW)
                    {
                        /*LOG(-1,"SET_BV(%d,%d): was visible is now %d\n", nx,ny,d);*/
                        pl_ptr->update_los = 1;
                    }
                }
            }

                /* this space is viewable */
                pname_flag = 0,ext_flag = 0, dmg_flag = 0;
                dmg_layer2 = 0,dmg_layer1 = 0,dmg_layer0 = 0;
                dark = NO_FACE_SEND;
#ifdef USE_TILESTRETCHER
                z1 = 0;
#endif
                /* lets calc the darkness/light value for this tile.*/
                if (MAP_OUTDOORS(m) && wdark <= m->light_value)
                {
                    d = msp->light_value + ((!personal_light) ? wdark : personal_light);
                }
                else
                {
                    d = msp->light_value + ((!personal_light) ? m->light_value : personal_light);
                }

                if (d <= 0) /* tile is not normal visible */
                {
                    /* (xray) or (infravision with mobile(aka alive) or player on a tile)? */
                    if (special_vision & 1 || (special_vision & 2 && msp->flags & (P_IS_PLAYER | P_IS_ALIVE)))
                        d = 100; /* make spot visible again */
                    else
                    {
                        if (pl_ptr->socket.lastmap.cells[ax][ay].count != -1)
                        {
#ifdef DEBUG_CORE_MAP
                            tile_count++;
#endif
                            SockBuf_AddShort(sbptr, mask);
                            map_clearcell(&pl_ptr->socket.lastmap.cells[ax][ay]);/* sets count to -1 too */
                        }
                        continue;
                    }
                }
                /* when we arrived here, this tile IS visible - now lets collect the data of it
                     * and update the client when something has changed.
                     */
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

                mp = &(pl_ptr->socket.lastmap.cells[ax][ay]);

                if (mp->count != d)
                {
                    mask |= 0x10;    /* darkness bit */
                    dark = d;
                    mp->count = d;
                }

                /* floor layer */
                face_num0 = 0;
                if(msp->floor_face)
                {
                    face_num0 = msp->floor_face->number;
#ifdef USE_TILESTRETCHER
                    if (msp->floor_z != 0 )
                     {
                       /*pname_flag |=0x80; */ /*  This floor has a height offset */
                       z1 = msp->floor_z;
#ifdef DEBUG_CORE_MAP
                       LOG(llevDebug,"Z1 = %d, pname = %d\n",z1,pname_flag);
#endif
                     }
#endif
                }
                /* layer 1 (floor) is really free now - we only have here
                 * type 68 SHOP_FLOOR left using layer 1. That special floor
                 * will be removed when we add real shop interfaces.
                 */
                else
                {
                    if (inv_flag)
                        tmp = GET_MAP_SPACE_CL(msp, 0);
                    else
                        tmp = GET_MAP_SPACE_CL_INV(msp, 0);
                    if (tmp)
                        face_num0 = tmp->face->number ;
                }
                if (mp->faces[3] != face_num0)
                {
                    mask |= 0x8;    /* this is the floor layer - 0x8 is floor bit */
                    mp->faces[3] = face_num0;       /* this is our backbuffer which we control */
                }

                /* LAYER 2 */
                /* ok, lets explain on one layer what we do */
                /* First, we get us the normal or invisible layer view */
/*
                if (inv_flag)
                    tmp = GET_MAP_SPACE_CL(msp, 1);
                else
                    tmp = GET_MAP_SPACE_CL_INV(msp, 1);
*/
                flag_tmp = 0;
                probe_tmp = 0;
                tmp = tmph = NULL;
                quick_pos_1 = 0;
                face = msp->mask_face;

                /* if we have no legal visual to send, skip it */
                if (!face || face == blank_face)
                {
                    flag_tmp = 0;probe_tmp = 0;
                    quick_pos_1 = 0;
                    face_num1m = face_num1 = 0;
                }
                else
                {
                    /* show target to player (this is personlized data)*/
                    if (tmph && pl_ptr->target_object_count == tmph->count)
                    {
                        flag_tmp |= FFLAG_PROBE;
                        if (tmph->stats.hp)
                            probe_tmp = (int) ((double) tmph->stats.hp / ((double) tmph->stats.maxhp / 100.0));
                        /* we don't carew about 0. If the client gots probe flag and value 0, he change it
                           to one... if some is here, it is alive, so what?
                         */
                    }
                    /* face_num1 is original, face_num1m is a copy */
                    face_num1m = face_num1 = face->number;
                    /* if a monster, we mark face_num1m - this goes to client */
                    if (tmp && (QUERY_FLAG(tmp, FLAG_MONSTER) || tmp->type == PLAYER))
                        face_num1m |= 0x8000;
                }
                /* ok - lets check we NEED to send this all to client */
                if (mp->faces[0] != face_num1 || mp->quick_pos[0] != quick_pos_1)
                {
                    mask |= 0x4;    /* this is the floor layer - 0x4 is floor bit */
                    if (tmp && tmp->type == PLAYER)
                    {
                        pname_flag |= 0x04; /* we have a player as object - send name too */
                        pname2 = tmp;
                    }
                    mp->faces[0] = face_num1;       /* this is our backbuffer which we control */
                    mp->quick_pos[0] = quick_pos_1; /* what we have send to client before */
                    if (quick_pos_1) /* if a multi arch */
                        ext_flag |= 0x4; /* mark multi arch - we need this info when sending this layer to client*/
                }
                /* thats our extension flags! like blind, confusion, etc */
                /* extensions are compared to all sended map update data very rare */
                if (flag_tmp != mp->fflag[0] || probe_tmp != mp->ff_probe[0])
                {
                    if (face_num1) /* the client delete the ext/probe values if face== 0 */
                        ext_flag |= 0x20; /* floor ext flags */
                    if (probe_tmp != mp->ff_probe[0] && flag_tmp & FFLAG_PROBE) /* ugly, but we must test it twice to submit implicit changes right */
                        flag_tmp |= FFLAG_PROBE;
                    mp->fflag[0] = flag_tmp;
                    mp->ff_probe[0] = probe_tmp;
                }

                /* LAYER 1 */
                if (inv_flag)
                    tmp = GET_MAP_SPACE_CL(msp, 2);
                else
                    tmp = GET_MAP_SPACE_CL_INV(msp, 2);
                probe_tmp = 0;
                if (tmp)
                {
                    /* Well, i have no idea how to send for each player his own face without this.
                     * The way we can avoid this is to lets draw the player by the client
                     * only and just to tell the client what direction and animation the player now
                     * has... but Daimonin/CF can't handle client map animation atm... Even it should
                     * not hard to be done. MT
                     */
/*
                    if (pl->x == nx && pl->y == ny && tmp->layer == 6)
                        tmp = pl;
*/
                    flag_tmp = GET_CLIENT_FLAGS(tmp);
                    tmph = tmp;
                    face = tmp->face;

                    if (tmp->last_damage != 0 && tmp->damage_round_tag == ROUND_TAG)
                    {
                        if (tmp==pl)
                        {
                            dmg_flag |= 0x4;
                            dmg_layer2 = tmp->last_damage;
                        }
                        else
                        {
                            dmg_flag |= 0x2;
                            dmg_layer1 = tmp->last_damage;
                        }
                    }
                    quick_pos_2 = tmp->quick_pos;
                    if (quick_pos_2) /* if we have a multipart object */
                    {
                        if ((tmph = tmp->head)) /* tail tile */
                        {
                            if (tmp->head->update_tag == map2_count)
                                face = 0; /* skip */
                            else
                            {
                                tmp->head->update_tag = map2_count;
                                face = tmp->head->face;
                            }
                        }
                        else /* a head */
                        {
                            if (tmp->update_tag == map2_count)
                                face = 0; /* we have send it this round before */
                            else
                            {
                                tmp->update_tag = map2_count;
                                face = tmp->face;
                            }
                        }
                    }
                }
                else
                {
                    face = NULL;
                    quick_pos_2 = 0;
                }

                if (!face || face == blank_face)
                {
                    flag_tmp = 0;probe_tmp = 0;
                    quick_pos_2 = 0;
                    face_num2m = face_num2 = 0;
                }
                else
                {
                    /* show target to player (this is personlized data)*/
                    if (tmph && pl_ptr->target_object_count == tmph->count)
                    {
                        flag_tmp |= FFLAG_PROBE;
                        if (tmph->stats.hp)
                            probe_tmp = (int) ((double) tmph->stats.hp / ((double) tmph->stats.maxhp / 100.0));
                        /* we don't carew about 0. If the client gots probe flag and value 0, he change it
                                    to one... if some is here, it is alive, so what?
                                     */
                    }
                    face_num2m = face_num2 = face->number;
                    if (tmp && (QUERY_FLAG(tmp, FLAG_MONSTER) || tmp->type == PLAYER))
                        face_num2m |= 0x8000;
                }

                if (mp->faces[1] != face_num2 || mp->quick_pos[1] != quick_pos_2)
                {
                    mask |= 0x2;    /* middle bit */
                    if (tmp && tmp->type == PLAYER)
                    {
                        pname_flag |= 0x02; /* we have a player as object - send name too */
                        pname3 = tmp;
                    }
                    mp->faces[1] = face_num2;
                    mp->quick_pos[1] = quick_pos_2;
                    if (quick_pos_2) /* if a multi arch */
                        ext_flag |= 0x2;
                }
                /* check, set and buffer ext flag */
                if (flag_tmp != mp->fflag[1] || probe_tmp != mp->ff_probe[1])
                {
                    if (face_num2) /* the client delete the ext/probe values if face== 0 */
                        ext_flag |= 0x10; /* floor ext flags */
                    if (probe_tmp != mp->ff_probe[1] && flag_tmp & FFLAG_PROBE) /* ugly, but we must test it twice to submit implicit changes right */
                        flag_tmp |= FFLAG_PROBE;
                    mp->fflag[1] = flag_tmp;
                    mp->ff_probe[1] = probe_tmp;
                }

                if (inv_flag)
                    tmp = GET_MAP_SPACE_CL(msp, 3);
                else
                    tmp = GET_MAP_SPACE_CL_INV(msp, 3);
                probe_tmp = 0;
                if (tmp)
                {
                    if (pl->x == nx && pl->y == ny && tmp->layer == 6)
                        tmp = pl;
                    flag_tmp = GET_CLIENT_FLAGS(tmp);
                    face = tmp->face;
                    tmph = tmp;
                    if (tmp->last_damage != 0 && tmp->damage_round_tag == ROUND_TAG)
                    {
                        if (tmp==pl)
                        {
                            dmg_flag |= 0x4;
                            dmg_layer2 = tmp->last_damage;
                        }
                        else
                        {
                            dmg_flag |= 0x1;
                            dmg_layer0 = tmp->last_damage;
                        }
                    }
                    quick_pos_3 = tmp->quick_pos;
                    if (quick_pos_3) /* if we have a multipart object */
                    {
                        if ((tmph = tmp->head)) /* tail tile */
                        {
                            if (tmph->update_tag == map2_count)
                                face = 0; /* skip */
                            else
                            {
                                tmph->update_tag = map2_count;
                                face = tmph->face;
                            }
                        }
                        else /* head */
                        {
                            if (tmp->update_tag == map2_count)
                                face = 0; /* we have send it this round before */
                            else
                            {
                                tmp->update_tag = map2_count;
                                face = tmp->face;
                            }
                        }
                    }
                }
                else
                {
                    face = NULL;
                    quick_pos_3 = 0;
                }

                if (!face || face == blank_face)
                {
                    flag_tmp = 0;probe_tmp = 0;
                    face_num3m = face_num3 = 0;
                    quick_pos_3 = 0;
                }
                else
                {
                    /* show target to player (this is personlized data)*/
                    if (tmph && pl_ptr->target_object_count == tmph->count)
                    {
                        flag_tmp |= FFLAG_PROBE;
                        if (tmph->stats.hp)
                            probe_tmp = (int) ((double) tmph->stats.hp / ((double) tmph->stats.maxhp / (double) 100.0));
                        /* we don't carew about 0. If the client gots probe flag and value 0, he change it
                                    to one... if some is here, it is alive, so what?
                                     */
                    }
                    face_num3m = face_num3 = face->number;
                    if (tmp && (QUERY_FLAG(tmp, FLAG_MONSTER) || tmp->type == PLAYER))
                        face_num3m |= 0x8000;
                }

                if (mp->faces[2] != face_num3 || mp->quick_pos[2] != quick_pos_3)
                {
                    mask |= 0x1;    /* top bit */
                    if (tmp && tmp->type == PLAYER)
                    {
                        pname_flag |= 0x01; /* we have a player as object - send name too */
                        pname4 = tmp;
                    }
                    if (quick_pos_3) /* if a multi arch */
                        ext_flag |= 0x1;
                    mp->faces[2] = face_num3;
                    mp->quick_pos[2] = quick_pos_3;
                }
                /* check, set and buffer ext flag */
                if (flag_tmp != mp->fflag[2] || probe_tmp != mp->ff_probe[2])
                {
                    if (face_num3) /* the client delete the ext/probe values if face== 0 */
                        ext_flag |= 0x08; /* floor ext flags */
                    if (probe_tmp != mp->ff_probe[2] && flag_tmp & FFLAG_PROBE) /* ugly, but we must test it twice to submit implicit changes right */
                        flag_tmp |= FFLAG_PROBE;
                    mp->fflag[2] = flag_tmp;
                    mp->ff_probe[2] = probe_tmp;
                }

                /* perhaps we smashed some on this map position */
                /* object is gone but we catch the damage we have here done */
                if (GET_MAP_RTAG(m, nx, ny) == ROUND_TAG)
                    dmg_flag |= 0x08; /* position (kill) damage */

                if (pname_flag)
                    ext_flag |= 0x80; /* we have one or more player names in this map node*/
                if (dmg_flag)
                    ext_flag |= 0x40; /* we have a dmg animation */

                if (ext_flag)
                {
                    mask |= 0x20;    /* mark ext flag as valid - LAST time we set mask! */
                    SockBuf_AddShort(sbptr, mask);
                    SockBuf_AddChar(sbptr, (char) ext_flag); /* push the ext_flagbyte */
                }
                else
                {
                    /* well.. IS there something we have to send? */
                    if (!(mask & 0x3f)) /* check all bits except the position */
                        continue;
#ifdef DEBUG_CORE_MAP
                    else
                        tile_count++;
#endif
                    SockBuf_AddShort(sbptr, mask); /* mask only */
                }

                if (pname_flag)
                {
                    SockBuf_AddChar(sbptr, (char) pname_flag);
                    /*if (pname_flag & 0x08)
                        SockList_AddString(&sl, CONTR(pname1)->quick_name, strlen(CONTR(pname1)->quick_name));*/
                    if (pname_flag & 0x04)
                        SockBuf_AddString(sbptr, CONTR(pname2)->quick_name, strlen(CONTR(pname2)->quick_name));
                    if (pname_flag & 0x02)
                        SockBuf_AddString(sbptr, CONTR(pname3)->quick_name, strlen(CONTR(pname3)->quick_name));
                    if (pname_flag & 0x01)
                        SockBuf_AddString(sbptr, CONTR(pname4)->quick_name, strlen(CONTR(pname4)->quick_name));
                }

                /* fire & forget layer animation tags */
                if (dmg_flag)
                {
                    /*LOG(llevDebug,"Send dmg_flag (%x): %x (%d %d %d) (%s)\n", mask,dmg_flag,dmg_layer2,dmg_layer1,dmg_layer0,pl->name);*/
                    SockBuf_AddChar(sbptr, (char) dmg_flag);
                    /*thats the special one - the red kill spot the client shows */
                    /* remember we put the damage value in the map because at the time
                     * we are here at run time, the object is dead since some ticks and
                     * perhaps some else is moved on this spot and/or the old object deleted */
                    if (dmg_flag & 0x08)
                    {
                        SockBuf_AddShort(sbptr, (sint16) GET_MAP_DAMAGE(m, nx, ny));
                    }
                    if (dmg_flag & 0x04)
                    {
                        SockBuf_AddShort(sbptr, (sint16) dmg_layer2);
                    }
                    if (dmg_flag & 0x02)
                    {
                        SockBuf_AddShort(sbptr, (sint16) dmg_layer1);
                    }
                    if (dmg_flag & 0x01)
                    {
                        SockBuf_AddShort(sbptr, (sint16) dmg_layer0);
                    }
                }


                /* client additional layer animations */
                if (ext_flag & 0x38)
                {
                    if (ext_flag & 0x20)
                    {
                        SockBuf_AddChar(sbptr, (char) mp->fflag[0]);
                        if (mp->fflag[0] & FFLAG_PROBE)
                        {
                            SockBuf_AddChar(sbptr, mp->ff_probe[0]);
                        }
                    }
                    if (ext_flag & 0x10)
                    {
                        SockBuf_AddChar(sbptr, (char) mp->fflag[1]);
                        if (mp->fflag[1] & FFLAG_PROBE)
                        {
                            SockBuf_AddChar(sbptr, mp->ff_probe[1]);
                        }
                    }
                    if (ext_flag & 0x08)
                    {
                        SockBuf_AddChar(sbptr, (char) mp->fflag[2]); /* and all the face flags if there */
                        if (mp->fflag[2] & FFLAG_PROBE)
                        {
                            SockBuf_AddChar(sbptr, mp->ff_probe[2]);
                        }
                    }
                }

                if (dark != NO_FACE_SEND)
                {
                    SockBuf_AddChar(sbptr, (char) dark);
                }
                if (mask & 0x08)
                {
                    SockBuf_AddShort(sbptr, face_num0);
#ifdef USE_TILESTRETCHER
                    SockBuf_AddShort(sbptr, z1);   /*  <--- sending height every time... */
                    /* should be reworked to send only when !=0 */
#endif
                }
                if (mask & 0x04)
                {
                    SockBuf_AddShort(sbptr, face_num1m);
                    if (ext_flag & 0x4)
                    {
                        SockBuf_AddChar(sbptr, (char) quick_pos_1);
                    }
                }
                if (mask & 0x02)
                {
                    SockBuf_AddShort(sbptr, face_num2m);
                    if (ext_flag & 0x2)
                    {
                        SockBuf_AddChar(sbptr, (char) quick_pos_2);
                    }
                }
                if (mask & 0x01)
                {
                    SockBuf_AddShort(sbptr, face_num3m);
                    if (ext_flag & 0x1)
                    {
                        SockBuf_AddChar(sbptr, (char) quick_pos_3);
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
        SOCKBUF_REQUEST_FINISH(ns, BINARY_CMD_MAP2, SOCKBUF_DYNAMIC);
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
void set_personal_light(player *pl, int value)
{
    if (!pl)
        return;

    if (value < 0)
        value += (((value / -MAX_DARKNESS) * MAX_DARKNESS)) + MAX_DARKNESS;

    if (value > MAX_DARKNESS)
        value -= (((value - 1) / MAX_DARKNESS)) * MAX_DARKNESS;

    pl->personal_light = (uint32)value;
}
