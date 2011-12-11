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

/* This file is the one and only DRAWINFO output module. All player
 * communication using drawinfo is handled here - except the few
 * messages we send to the client using DRAWINFO before we had setup
 * any player structure - thats for example when a outdated client
 * logs in and we send "update your client" direct to the info windows.
 * But if the player is loged in - all DRAWINFO are generated here.
 */
#include "global.h"

static sockbuf_struct *NewDrawInfo(int flags, const char *const str,
                                   sockbuf_struct *sb);

/* new_draw_info:
 *
 * flags is various flags.
 *
 * pri is priority.  It is a little odd - the lower the value, the more
 * important it is.  Thus, 0 gets sent no matter what.  Otherwise, the
 * value must be less than the listening level that the player has set.
 * Unfortunately, there is no clear guideline on what each level does what.
 *
 * op can be passed as NULL - in fact, this will be done if NDI_ALL is set
 * in the flags. */
void new_draw_info(const int flags, const int pri, const object *const op,
                   const char *const format, ...)
{
    char            buf[HUGE_BUF] = "";
    va_list         ap;
    sockbuf_struct *sb;
    player         *pl;

    /* Don't send empty string. */
    if (!format ||
        !*format)
    {
        return;
    }

    /* Silently refuse non-NDI_ALL messages not to players. This lets us skip
     * all ob->type == PLAYER checks all over the code */
    /* Neater maybe, but surely slower as we then go through all the NDI
     * rigmarole for nothing (ie, in common calling funcs which work for both
     * players and monsters.
     * -- Smacky 20100619 */
    if (!(flags & NDI_ALL) &&
        (!op ||
         op->type != PLAYER ||
         !CONTR(op)))
    {
        return;
    }

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = '\0';
    sb = NewDrawInfo(flags, buf, sb);

    for (pl = ((flags & NDI_ALL)) ? first_player : CONTR(op); pl;
         pl = ((flags & NDI_ALL)) ? pl->next : NULL)
    {
        if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
            pl->socket.status != Ns_Dead &&
            pri <= pl->listening)
        {
            SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
        }
    }

    SOCKBUF_COMPOSE_FREE(sb);
}

/* write to everyone on the current map in a defined area. */
void new_info_map(const int flags, const mapstruct *const map, const int x,
                  const int y, const int dist, const char *const format, ...)
{
    char            buf[HUGE_BUF] = "";
    va_list         ap;
    sockbuf_struct *sb;
    object         *tmp;
    int             xt,
                    yt,
                    d;

    /* Don't send empty string. */
    if (!format ||
        !*format)
    {
        return;
    }

    /* No map? Quit */
    if (!map ||
        map->in_memory != MAP_IN_MEMORY)
    {
        return;
    }

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = '\0';
    sb = NewDrawInfo(flags, buf, sb);

    if (dist == MAP_INFO_ALL)
    {
        for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }

        SOCKBUF_COMPOSE_FREE(sb);

        return;
    }

    d = POW2(dist);

    for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
    {
        player *pl = CONTR(tmp);

        if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
            pl->socket.status != Ns_Dead &&
            (POW2(tmp->x - x) + POW2(tmp->y - y)) <= d)
        {
            SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
        }
    }

    if (map->tile_map[TILED_MAPS_NORTH] &&
        map->tile_map[TILED_MAPS_NORTH]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_NORTH]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTH]);

        for (tmp = map->tile_map[TILED_MAPS_NORTH]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_EAST] &&
        map->tile_map[TILED_MAPS_EAST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_EAST]->player_first)
    {
        xt = x - MAP_WIDTH(map);

        for (tmp = map->tile_map[TILED_MAPS_EAST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_SOUTH] &&
        map->tile_map[TILED_MAPS_SOUTH]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_SOUTH]->player_first)
    {
        yt = y - MAP_HEIGHT(map);

        for (tmp = map->tile_map[TILED_MAPS_SOUTH]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_WEST] &&
        map->tile_map[TILED_MAPS_WEST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_WEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_WEST]);

        for (tmp = map->tile_map[TILED_MAPS_WEST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_NORTHEAST] &&
        map->tile_map[TILED_MAPS_NORTHEAST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_NORTHEAST]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTHEAST]);
        xt = x - MAP_WIDTH(map);

        for (tmp = map->tile_map[TILED_MAPS_NORTHEAST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_SOUTHEAST] &&
        map->tile_map[TILED_MAPS_SOUTHEAST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_SOUTHEAST]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        yt = y - MAP_HEIGHT(map);

        for (tmp = map->tile_map[TILED_MAPS_SOUTHEAST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_SOUTHWEST] &&
        map->tile_map[TILED_MAPS_SOUTHWEST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_SOUTHWEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_SOUTHWEST]);
        yt = y - MAP_HEIGHT(map);

        for (tmp = map->tile_map[TILED_MAPS_SOUTHWEST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_NORTHWEST] &&
        map->tile_map[TILED_MAPS_NORTHWEST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_NORTHWEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_NORTHWEST]);
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTHWEST]);

        for (tmp = map->tile_map[TILED_MAPS_NORTHWEST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    SOCKBUF_COMPOSE_FREE(sb);
}

/* write to everyone on the map *except* op and op1.  This is useful for emotions. */
void new_info_map_except(const int flags, const mapstruct *const map,
                         const int x, const int y, const int dist,
                         const  object *const op1, const object *const op,
                         const char *const format, ...)
{
    char            buf[HUGE_BUF] = "";
    va_list         ap;
    sockbuf_struct *sb;
    object         *tmp;
    int             xt,
                    yt,
                    d;

    /* Don't send empty string. */
    if (!format ||
        !*format)
    {
        return;
    }

    /* No map? Quit */
    if (!map ||
        map->in_memory != MAP_IN_MEMORY)
    {
        return;
    }

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = '\0';
    sb = NewDrawInfo(flags, buf, sb);

    if (dist != MAP_INFO_ALL)
    {
        d = POW2(dist);
    }
    else
    {
        for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op &&
                tmp != op1)
            {
                player *pl = CONTR(tmp);

                if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                    pl->socket.status != Ns_Dead)
                {
                    SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
                }
            }
        }

        SOCKBUF_COMPOSE_FREE(sb);

        return;
    }

    if (map->player_first) /* any player on this map? */
    {
        for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - x) + POW2(tmp->y - y)) <= d)
            {
                player *pl = CONTR(tmp);

                if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                    pl->socket.status != Ns_Dead)
                {
                    SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
                }
            }
        }
    }

    if (map->tile_map[TILED_MAPS_NORTH] &&
        map->tile_map[TILED_MAPS_NORTH]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_NORTH]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTH]);

        for (tmp = map->tile_map[TILED_MAPS_NORTH]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_EAST] &&
        map->tile_map[TILED_MAPS_EAST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_EAST]->player_first)
    {
        xt = x - MAP_WIDTH(map);

        for (tmp = map->tile_map[TILED_MAPS_EAST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_SOUTH] &&
        map->tile_map[TILED_MAPS_SOUTH]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_SOUTH]->player_first)
    {
        yt = y - MAP_HEIGHT(map);

        for (tmp = map->tile_map[TILED_MAPS_SOUTH]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_WEST] &&
        map->tile_map[TILED_MAPS_WEST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_WEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_WEST]);

        for (tmp = map->tile_map[TILED_MAPS_WEST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_NORTHEAST] &&
        map->tile_map[TILED_MAPS_NORTHEAST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_NORTHEAST]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTHEAST]);
        xt = x - MAP_WIDTH(map);

        for (tmp = map->tile_map[TILED_MAPS_NORTHEAST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_SOUTHEAST] &&
        map->tile_map[TILED_MAPS_SOUTHEAST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_SOUTHEAST]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        yt = y - MAP_HEIGHT(map);

        for (tmp = map->tile_map[TILED_MAPS_SOUTHEAST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_SOUTHWEST] &&
        map->tile_map[TILED_MAPS_SOUTHWEST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_SOUTHWEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_SOUTHWEST]);
        yt = y - MAP_HEIGHT(map);

        for (tmp = map->tile_map[TILED_MAPS_SOUTHWEST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    if (map->tile_map[TILED_MAPS_NORTHWEST] &&
        map->tile_map[TILED_MAPS_NORTHWEST]->in_memory == MAP_IN_MEMORY &&
        map->tile_map[TILED_MAPS_NORTHWEST]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[TILED_MAPS_NORTHWEST]);
        yt = y + MAP_HEIGHT(map->tile_map[TILED_MAPS_NORTHWEST]);

        for (tmp = map->tile_map[TILED_MAPS_NORTHWEST]->player_first; tmp;
             tmp = CONTR(tmp)->map_above)
        {
            player *pl = CONTR(tmp);

            if (!(pl->state & (ST_DEAD | ST_ZOMBIE)) &&
                pl->socket.status != Ns_Dead &&
                tmp != op &&
                tmp != op1 &&
                (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
            {
                SOCKBUF_ADD_TO_SOCKET(&pl->socket, sb);
            }
        }
    }

    SOCKBUF_COMPOSE_FREE(sb);
}

static sockbuf_struct *NewDrawInfo(int flags, const char *const str, sockbuf_struct *sb)
{
    const int  len = strlen(str);
    char      *buf;

    MALLOC(buf, len + 3); // uint16 flags +  string length + '\0'
    *((uint16 *)buf) = (uint16)(flags & (~NDI_ALL | NDI_FLAG_MASK));
    sprintf(buf + 2, "%s", str);
    sb = SOCKBUF_COMPOSE(SERVER_CMD_DRAWINFO2, buf, len + 2, 0);
    FREE(buf);

    return sb;
}
