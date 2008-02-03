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

/* This file is the one and only DRAWINFO output module. All player
 * communication using drawinfo is handled here - except the few
 * messages we send to the client using DRAWINFO before we had setup
 * any player structure - thats for example when a outdated client
 * logs in and we send "update your client" direct to the info windows.
 * But if the player is loged in - all DRAWINFO are generated here.
 */
#include <global.h>
#include <stdarg.h>

/*
 * new_draw_info:
 *
 * flags is various flags - mostly color, plus a few specials.
 *
 * pri is priority.  It is a little odd - the lower the value, the more
 * important it is.  Thus, 0 gets sent no matter what.  Otherwise, the
 * value must be less than the listening level that the player has set.
 * Unfortunately, there is no clear guideline on what each level does what.
 *
 * pl can be passed as NULL - in fact, this will be done if NDI_ALL is set
 * in the flags.
 *
 */
void new_draw_info(const int flags, const int pri, const object *const pl, const char *const buf)
{
    unsigned char info_string[HUGE_BUF];
    SockList    sl;

    if (!buf) /* should not happen - generate save string and LOG it */
    {
        LOG(llevBug, "BUG:: new_draw_info: NULL string send! %s (%x - %d)\n", query_name(pl), flags, pri);
        return;
    }

    /* here we handle global messages - still not sure i want this here */
    if (flags & NDI_ALL)
    {
        player *tmppl;
        for (tmppl = first_player; tmppl != NULL; tmppl = tmppl->next)
        {
            if(tmppl->state != ST_DEAD && tmppl->state != ST_ZOMBIE && tmppl->socket.status != Ns_Dead)
                new_draw_info((flags & ~NDI_ALL), pri, tmppl->ob, buf);
        }
        return;
    }

    /* Silently refuse messages not to players.
     * This lets us skip all ob->type == PLAYER checks all over the code */
    if (!pl || pl->type != PLAYER)
        return;

    if (!CONTR(pl) || CONTR(pl)->state != ST_PLAYING)
        return;

    if (pri >= CONTR(pl)->listening) /* player don't want this */
        return;

    sl.buf = info_string;
    SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_DRAWINFO2);
    SockList_AddShort(&sl, flags & NDI_FLAG_MASK);
    strcpy((char *)sl.buf + sl.len, buf);
    sl.len += strlen(buf);
    Send_With_Handling(&CONTR(pl)->socket, &sl);

    /*  sprintf(info_string,"X%d %s", flags&NDI_FLAG_MASK, buf);
        Write_String_To_Socket(&CONTR(pl)->socket, BINARY_CMD_DRAWINFO,info_string, strlen(info_string));
        */
}

/* This is a pretty trivial function, but it allows us to use printf style
 * formatting, so instead of the calling function having to do it, we do
 * it here.  IT may also have advantages in the future for reduction of
 * client/server bandwidth (client could keep track of various strings
 */

void new_draw_info_format(const int flags, const int pri, const object *const pl, const char *const format, ...)
{
    char    buf[HUGE_BUF];

    va_list ap;
    va_start(ap, format);

    vsnprintf(buf, HUGE_BUF, format, ap);
    buf[HUGE_BUF-1] = '\0';

    va_end(ap);

    new_draw_info(flags, pri, pl, buf);
}

/* we want give msg to all people on one, specific map
 */
static void new_info_map_all_except(const int color, const mapstruct *const map, const object *const op1,
                                    const object *const op, const char *const str)
{
    object *tmp;

    if (map->player_first)
    {
        for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1)
                new_draw_info(color, 0, tmp, str);
        }
    }
}

/*
 * write to everyone on the current map in a defined area
 *
*/
void new_info_map(const int color, const mapstruct *const map, const int x, const int y, const int dist, const char *const str)
{
    int     xt, yt, d;
    object *tmp;

    if (!map || map->in_memory != MAP_IN_MEMORY)
        return;

    if (dist != MAP_INFO_ALL)
        d = POW2(dist);
    else
    {
        new_info_map_all_except(color, map, NULL, NULL, str); /* we want all on this map */
        return;
    }

    if (map->player_first) /* any player on this map? */
    {
        for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - x) + POW2(tmp->y - y)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }

    if (map->tile_map[0] && map->tile_map[0]->in_memory == MAP_IN_MEMORY && map->tile_map[0]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[0]);
        for (tmp = map->tile_map[0]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[1] && map->tile_map[1]->in_memory == MAP_IN_MEMORY && map->tile_map[1]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        for (tmp = map->tile_map[1]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[2] && map->tile_map[2]->in_memory == MAP_IN_MEMORY && map->tile_map[2]->player_first)
    {
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[2]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[3] && map->tile_map[3]->in_memory == MAP_IN_MEMORY && map->tile_map[3]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[3]);
        for (tmp = map->tile_map[3]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[4] && map->tile_map[4]->in_memory == MAP_IN_MEMORY && map->tile_map[4]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[4]);
        xt = x - MAP_WIDTH(map);
        for (tmp = map->tile_map[4]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[5] && map->tile_map[5]->in_memory == MAP_IN_MEMORY && map->tile_map[5]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[5]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[6] && map->tile_map[6]->in_memory == MAP_IN_MEMORY && map->tile_map[6]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[6]);
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[6]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[7] && map->tile_map[7]->in_memory == MAP_IN_MEMORY && map->tile_map[7]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[7]);
        yt = y + MAP_HEIGHT(map->tile_map[7]);
        for (tmp = map->tile_map[7]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if ((POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
}

/*
 * write to everyone on the map *except* op and op1.  This is useful for emotions.
 */
void new_info_map_except(const int color, const mapstruct *const map, const int x, const int y, const int dist,
                         const  object *const op1, const object *const op, const char *const str)
{
    int     xt, yt, d;
    object *tmp;

    if (!map || map->in_memory != MAP_IN_MEMORY)
        return;

    if (dist != MAP_INFO_ALL)
        d = POW2(dist);
    else
    {
        new_info_map_all_except(color, map, op1, op, str); /* we want all on this map */
        return;
    }

    if (map->player_first) /* any player on this map? */
    {
        for (tmp = map->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - x) + POW2(tmp->y - y)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }

    if (map->tile_map[0] && map->tile_map[0]->in_memory == MAP_IN_MEMORY && map->tile_map[0]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[0]);
        for (tmp = map->tile_map[0]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[1] && map->tile_map[1]->in_memory == MAP_IN_MEMORY && map->tile_map[1]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        for (tmp = map->tile_map[1]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[2] && map->tile_map[2]->in_memory == MAP_IN_MEMORY && map->tile_map[2]->player_first)
    {
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[2]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - x) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[3] && map->tile_map[3]->in_memory == MAP_IN_MEMORY && map->tile_map[3]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[3]);
        for (tmp = map->tile_map[3]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - xt) + POW2(tmp->y - y)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[4] && map->tile_map[4]->in_memory == MAP_IN_MEMORY && map->tile_map[4]->player_first)
    {
        yt = y + MAP_HEIGHT(map->tile_map[4]);
        xt = x - MAP_WIDTH(map);
        for (tmp = map->tile_map[4]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[5] && map->tile_map[5]->in_memory == MAP_IN_MEMORY && map->tile_map[5]->player_first)
    {
        xt = x - MAP_WIDTH(map);
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[5]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[6] && map->tile_map[6]->in_memory == MAP_IN_MEMORY && map->tile_map[6]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[6]);
        yt = y - MAP_HEIGHT(map);
        for (tmp = map->tile_map[6]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
    if (map->tile_map[7] && map->tile_map[7]->in_memory == MAP_IN_MEMORY && map->tile_map[7]->player_first)
    {
        xt = x + MAP_WIDTH(map->tile_map[7]);
        yt = y + MAP_HEIGHT(map->tile_map[7]);
        for (tmp = map->tile_map[7]->player_first; tmp; tmp = CONTR(tmp)->map_above)
        {
            if (tmp != op && tmp != op1 && (POW2(tmp->x - xt) + POW2(tmp->y - yt)) <= d)
                new_draw_info(color, 0, tmp, str);
        }
    }
}

/* same as above but as formated string version */
void new_info_map_format(const int color, const mapstruct *const map, const int x, const int y, const int dist, const char *const format, ...)
{
    char    buf[HUGE_BUF];
    va_list ap;

    va_start(ap, format);

    vsnprintf(buf, HUGE_BUF, format, ap);
    buf[HUGE_BUF-1] = '\0';

    va_end(ap);

    new_info_map(color,map, x, y, dist, buf);
}

/* same as above but as formated string version */
void new_info_map_except_format(const int color, const mapstruct *const map, const int x, const int y, const int dist, const object *const op1, const object *const op, const char *const format, ...)
{
    char    buf[HUGE_BUF];
    va_list ap;

    va_start(ap, format);

    vsnprintf(buf, HUGE_BUF, format, ap);
    buf[HUGE_BUF-1] = '\0';

    va_end(ap);

    new_info_map_except(color, map, x, y, dist, op1, op, buf);
}
