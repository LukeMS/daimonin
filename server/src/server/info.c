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

/* TODO: Rename file as ndi.c -- ndi has historical significance: draw_info
 * became new_draw_info and is now ndi which is short and unusual/unique
 * enough to be a good prefix. */
/* This file is the one and only DRAWINFO output module. All player
 * communication using drawinfo is handled here - except the few
 * messages we send to the client using DRAWINFO before we had setup
 * any player structure - thats for example when a outdated client
 * logs in and we send "update your client" direct to the info windows.
 * But if the player is loged in - all DRAWINFO are generated here.
 */
#include "global.h"

static sockbuf_struct *ComposeSB(int flags, const char *const str, sockbuf_struct *sb);

/* ndi() sends the format... string to the specified player object (who).
 *
 * flags is various flags.
 *
 * pri is priority.  It is a little odd - the lower the value, the more
 * important it is.  Thus, 0 gets sent no matter what.  Otherwise, the
 * value must be less than the listening level that the player has set.
 * Unfortunately, there is no clear guideline on what each level does what.
 *
 * who can be passed as NULL - in fact, this will be done if NDI_ALL is set
 * in the flags. */
void ndi(const int flags, const int pri, const object_t *const who, const char *const format, ...)
{
    char            buf[HUGE_BUF] = "";
    va_list         ap;
    sockbuf_struct *sb = NULL;
    player_t         *pl;

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
        (!who ||
         who->type != PLAYER ||
         !CONTR(who)))
    {
        return;
    }

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = '\0';
    sb = ComposeSB(flags, buf, sb);

    for (pl = ((flags & NDI_ALL)) ? first_player : CONTR(who); pl;
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

/* CHECKPLAYERS() is a convenience macro used in ndi_map() to avoid large areas
 * of repeat code. Essentially it finds all the players within range of the
 * message and adds the sb to only their sockets. */
#define CHECKPLAYERS(_O_, _E1_, _E2_, _M_, _X_, _Y_, _D_, _SB_) \
    for ((_O_) = (_M_)->player_first; (_O_); (_O_) = CONTR((_O_))->map_above) \
    { \
        if (!(CONTR((_O_))->state & (ST_DEAD | ST_ZOMBIE)) && \
            CONTR((_O_))->socket.status != Ns_Dead && \
            (!(_E1_) || \
             (_O_) != (_E1_)) && \
            (!(_E2_) || \
             (_O_) != (_E2_)) && \
            ((_D_) == MAP_INFO_ALL || \
             SQR((_O_)->x - (_X_)) + POW2((_O_)->y - (_Y_)) <= (_D_))) \
        { \
            SOCKBUF_ADD_TO_SOCKET(&CONTR((_O_))->socket, (_SB_)); \
        } \
    }

/* ndi_map() sends the format... string to every player (that meets certain
 * criteria) on a map (and up to 8 directly tiled maps) within a given range
 * centered around msp.
 *
 * If except1 or except2 are not NULL, these players are NOT included as
 * recipients. */
void ndi_map(const int flags, msp_t *msp, const int dist, const  object_t *const except1, const object_t *const except2, const char *const format, ...)
{
    char            buf[HUGE_BUF] = "";
    va_list         ap;
    sockbuf_struct *sb = NULL;
    object_t       *this;
    map_t          *m,
                   *m2;
    sint16          x,
                    y,
                    x2,
                    y2,
                    d;

    /* Don't send empty string. */
    if (!format ||
        !*format)
    {
        return;
    }

    /* No msp? Quit */
    if (!msp ||
        msp->map->in_memory != MAP_MEMORY_ACTIVE)
    {
        return;
    }

    m = msp->map;
    x = msp->x;
    y = msp->y;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    buf[sizeof(buf) - 1] = '\0';
    sb = ComposeSB(flags, buf, sb);

    if (dist == MAP_INFO_ALL)
    {
        CHECKPLAYERS(this, except1, except2, m, x, y, dist, sb);
        SOCKBUF_COMPOSE_FREE(sb);
        return;
    }

    d = SQR(dist);
    CHECKPLAYERS(this, except1, except2, m, x, y, d, sb);

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTH]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x;
        y2 = y + m2->height;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_EAST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x - m->width;
        y2 = y;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTH]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x;
        y2 = y - m->height;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_WEST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x + m2->width;
        y2 = y;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTHEAST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x - m->width;
        y2 = y + m2->height;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTHEAST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x - m->width;
        y2 = y - m->height;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTHWEST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x + m2->width;
        y2 = y - m->height;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTHWEST]) &&
        m2->in_memory == MAP_MEMORY_ACTIVE &&
        m2->player_first)
    {
        x2 = x + m2->width;
        y2 = y + m2->height;
        CHECKPLAYERS(this, except1, except2, m2, x2, y2, d, sb);
    }

    SOCKBUF_COMPOSE_FREE(sb);
}

#undef CHECKPLAYERS

static sockbuf_struct *ComposeSB(int flags, const char *const str, sockbuf_struct *sb)
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
