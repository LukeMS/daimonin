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

#include "global.h"

/* Distance must be less than this for the object to be blocked.
 * An object is 1.0 wide, so if set to 0.5, it means the object
 * that blocks half the view (0.0 is complete block) will
 * block view in our tables.
 * .4 or less lets you see through walls.  .5 is about right. */
#define SPACE_BLOCK 0.5

typedef struct blstr
{
    int             x[4], y[4];
    int             index;
} blocks;

static blocks block[MAP_CLIENT_X][MAP_CLIENT_Y];

static void ExpandSight(player_t *pl);
static void BlockVista(player_t *pl, int x, int y);

/*
 * initialises the array used by the LOS routines.
 * This is NOT called for every LOS - only at server start to
 * init the base block struct.
 */

/* since we are only doing the upper left quadrant, only
 * these spaces could possibly get blocked, since these
 * are the only ones further out that are still possibly in the
 * sightline.
 */

void init_block()
{
    int         x, y, dx, dy, i;
    static int  block_x[3] = { - 1, -1, 0}, block_y[3] = { - 1, 0, -1};

    for (x = 0; x < MAP_CLIENT_X; x++)
        for (y = 0; y < MAP_CLIENT_Y; y++)
        {
            block[x][y].index = 0;
        }


    /* The table should be symmetric, so only do the upper left
     * quadrant - makes the processing easier.
     */
    for (x = 1; x <= MAP_CLIENT_X / 2; x++)
    {
        for (y = 1; y <= MAP_CLIENT_Y / 2; y++)
        {
            for (i = 0; i < 3; i++)
            {
                dx = x + block_x[i];
                dy = y + block_y[i];

                /* center space never blocks */
                if (x == MAP_CLIENT_X / 2 && y == MAP_CLIENT_Y / 2)
                    continue;

                /* If its a straight line, its blocked */
                if ((dx == x && x == MAP_CLIENT_X / 2) || (dy == y && y == MAP_CLIENT_Y / 2))
                {
                    /* For simplicity, we mirror the coordinates to block the other
                             * quadrants.
                             */
                    set_block(x, y, dx, dy);
                    if (x == MAP_CLIENT_X / 2)
                    {
                        set_block(x, MAP_CLIENT_Y - y - 1, dx, MAP_CLIENT_Y - dy - 1);
                    }
                    else if (y == MAP_CLIENT_Y / 2)
                    {
                        set_block(MAP_CLIENT_X - x - 1, y, MAP_CLIENT_X - dx - 1, dy);
                    }
                }
                else
                {
                    float   d1, r, s, l;

                    /* We use the algorihm that found out how close the point
                         * (x,y) is to the line from dx,dy to the center of the viewable
                         * area.  l is the distance from x,y to the line.
                         * r is more a curiosity - it lets us know what direction (left/right)
                         * the line is off
                         */

                    d1 = (float) (pow(MAP_CLIENT_X / 2 - dx, 2) + pow(MAP_CLIENT_Y / 2 - dy, 2));
                    r = (float) ((dy - y) * (dy - MAP_CLIENT_Y / 2) - (dx - x) * (MAP_CLIENT_X / 2 - dx)) / d1;
                    s = (float) ((dy - y) * (MAP_CLIENT_X / 2 - dx) - (dx - x) * (MAP_CLIENT_Y / 2 - dy)) / d1;
                    l = (float) FABS(sqrt(d1) * s);

                    if (l <= SPACE_BLOCK)
                    {
                        /* For simplicity, we mirror the coordinates to block the other
                                 * quadrants.
                                 */
                        set_block(x, y, dx, dy);
                        set_block(MAP_CLIENT_X - x - 1, y, MAP_CLIENT_X - dx - 1, dy);
                        set_block(x, MAP_CLIENT_Y - y - 1, dx, MAP_CLIENT_Y - dy - 1);
                        set_block(MAP_CLIENT_X - x - 1, MAP_CLIENT_Y - y - 1, MAP_CLIENT_X - dx - 1,
                                  MAP_CLIENT_Y - dy - 1);
                    }
                }
            }
        }
    }
}

/*
 * Used to initialise the array used by the LOS routines.
 * What this sets if that x,y blocks the view of bx,by
 * This then sets up a relation - for example, something
 * at 5,4 blocks view at 5,3 which blocks view at 5,2
 * etc.  So when we check 5,4 and find it block, we have
 * the data to know that 5,3 and 5,2 and 5,1 should also
 * be blocked.
 */
void set_block(int x, int y, int bx, int by)
{
    int index = block[x][y].index, i;

    /* Due to flipping, we may get duplicates - better safe than sorry.
     */
    for (i = 0; i < index; i++)
    {
        if (block[x][y].x[i] == bx && block[x][y].y[i] == by)
            return;
    }

    block[x][y].x[index] = bx;
    block[x][y].y[index] = by;
    block[x][y].index++;
#ifdef LOS_DEBUG
    LOG(llevInfo, "setblock: added %d %d -> %d %d (%d)\n", x, y, bx, by, block[x][y].index);
#endif
}

#if 0
/* update_los() recalculates the array which specifies what is
 * visible for the given player. */
void update_los(player_t *pl)
{
    object_t *who = pl->ob;
    sint16    i,
              j;

#ifdef DEBUG_CORE
    LOG(llevDebug, "LOS - %s\n", STRING_OBJ_NAME(who));
#endif

    /* Reset the array -- all msps are visible. */
    (void)memset((void *)pl->blocked_los, BLOCKED_LOS_VISIBLE, sizeof(pl->blocked_los));

    /* For wizpass, that is all. */
    if (pl->gmaster_wizpass)
    {
        return;
    }

    /* Work through the array, determining what is visible or not based on what
     * is actually on the map. */
    for (i = (MAP_CLIENT_X - pl->socket.mapx) / 2; i < (MAP_CLIENT_X + pl->socket.mapx) / 2; i++)
    {
        for (j = (MAP_CLIENT_Y - pl->socket.mapy) / 2; j < (MAP_CLIENT_Y + pl->socket.mapy) / 2; j++)
        {
            map_t  *m = who->map;
            sint16  x = who->x + i - MAP_CLIENT_X / 2,
                    y = who->y + j - MAP_CLIENT_Y / 2,
                    ax = i - (MAP_CLIENT_X - pl->socket.mapx) / 2,
                    ay = j - (MAP_CLIENT_Y - pl->socket.mapy) / 2;
            msp_t  *msp = MSP_GET(m, x, y);

            /* this skips the "edges" of view area, the border tiles.
             * Naturally, this tiles can't block any view - there is
             * nothing behind them. */
            if (!block[i][j].index)
            {
                /* to handle the "blocksview update" right, we give this special
                 * tiles a "never use it to trigger a los_update()" flag.
                 * blockview changes to this tiles will have no effect. */
                if (!msp)
                {
                    pl->blocked_los[ax][ay] = BLOCKED_LOS_OUT_OF_MAP;
                }
                else
                {
                    pl->blocked_los[ax][ay] |= BLOCKED_LOS_IGNORE;
                }

                continue;
            }

            /* If the converted coordinates are outside the viewable
             * area for the client, return now. */
            if (ax < 0 ||
                ax >= pl->socket.mapx ||
                ay < 0 ||
                ay >= pl->socket.mapy)
            {
                continue;
            }

            /*LOG(llevNoLog,"SET_LOS: %d,%d\n", ax,ay);*/
            /* If this space is already blocked, prune the processing - presumably
             * whatever has set this space to be blocked has done the work and already
             * done the dependency chain.
             * but check for out_of_map to speedup our client map draw function. */
            if (pl->blocked_los[ax][ay] & (BLOCKED_LOS_BLOCKED | BLOCKED_LOS_OUT_OF_MAP))
            {
                if (pl->blocked_los[ax][ay] & BLOCKED_LOS_BLOCKED)
                {
                    if (!msp)
                    {
                        pl->blocked_los[ax][ay] = BLOCKED_LOS_OUT_OF_MAP;
                    }
                    else if ((msp->flags & MSP_FLAG_BLOCKSVIEW))
                    {
                        pl->blocked_los[ax][ay] |= BLOCKED_LOS_BLOCKSVIEW;
                    }
                }

                continue;
            }

            if (!msp)
            {
                BlockVista(pl, i, j);
                pl->blocked_los[ax][ay] = BLOCKED_LOS_OUT_OF_MAP;
            }
            else if ((msp->flags & MSP_FLAG_BLOCKSVIEW))
            {
                BlockVista(pl, i, j);
                pl->blocked_los[ax][ay] |= BLOCKED_LOS_BLOCKSVIEW;
            }
        }
    }

    ExpandSight(pl);

    /* If the player has xray vision, unblock nearby blocked squares. */
    /* TODO: Currently this is a fixed 9x9 area. Why not make it variable so
     * that we can have greater and lesser demon eyes?
     *
     * -- Smacky 20150812 */
    if (QUERY_FLAG(who, FLAG_XRAYS))
    {
        sint16 dx = pl->socket.mapx_2,
               dy = pl->socket.mapy_2;

        for (i = -4; i <= 4; i++)
        {
            for (j = -4; j <= 4; j++)
            {
                if (pl->blocked_los[dx + i][dy + j] & BLOCKED_LOS_BLOCKED)
                {
                    pl->blocked_los[dx + i][dy + j] &= ~BLOCKED_LOS_BLOCKED;
                }
            }
        }
    }
}

/* Used to initialise the array used by the LOS routines.
 * x,y are indexes into the blocked[][] array.
 * This recursively sets the blocked line of sight view.
 * From the blocked[][] array, we know for example
 * that if some particular space is blocked, it blocks
 * the view of the spaces 'behind' it, and those blocked
 * spaces behind it may block other spaces, etc.
 * In this way, the chain of visibility is set. */
static void BlockVista(player_t *pl, int x, int y)
{
    sint32  i;
    sint16  x2 = (MAP_CLIENT_X - pl->socket.mapx) / 2,
            y2 = (MAP_CLIENT_Y - pl->socket.mapy) / 2;

    for (i = 0; i < block[x][y].index; i++)
    {
        sint16 dx = block[x][y].x[i],
               dy = block[x][y].y[i],
               ax = dx - x2,
               ay = dy - y2;

        if (ax < 0 ||
            ax >= pl->socket.mapx ||
            ay < 0 ||
            ay >= pl->socket.mapy)
        {
            continue;
        }

#if 0
        LOG(llevInfo ,"blocked %d %d -> %d %d\n",dx, dy, ax, ay);
#endif
        /* we need to adjust to the fact that the socket
         * code wants the los to start from the 0,0
         * and not be relative to middle of los array.
         */
        if (!(pl->blocked_los[ax][ay] & BLOCKED_LOS_OUT_OF_MAP))
        {
            pl->blocked_los[ax][ay] |= BLOCKED_LOS_BLOCKED; /* this tile can't be seen */
        }

        BlockVista(pl, dx, dy);
    }
}

/*
 * ExpandSight goes through the array of what the given player is
 * able to see, and expands the visible area a bit, so the player will,
 * to a certain degree, be able to see into corners.
 * This is somewhat suboptimal, would be better to improve the formula.
 */
/* thats true: we should migrate this function asap in the los - a bit better
 * "pre calculated" LOS function.
 * It should be easy to do a better, optimized precalculation. MT-2004
 */
static void ExpandSight(player_t *pl)
{
    int i, x, y, dx, dy;

    for (x = 1; x < pl->socket.mapx - 1; x++)    /* loop over inner squares */
    {
        for (y = 1; y < pl->socket.mapy - 1; y++)
        {
#if 0
        LOG(llevInfo ,"ExpandSight x,y = %d, %d  blocksview = %d, %d\n",
            x, y, op->x-pl->socket.mapx_2+x, op->y-pl->socket.mapy_2+y);
#endif
            if (pl->blocked_los[x][y] <= BLOCKED_LOS_BLOCKSVIEW &&  /* if visible */
                !(pl->blocked_los[x][y] & BLOCKED_LOS_BLOCKSVIEW))  /* and not blocksview */
            {
                /* mark all directions */
                for (i = 1; i <= 8; i += 1)
                {
                    dx = x + OVERLAY_X(i);
                    dy = y + OVERLAY_Y(i);

                    if (dx < 0 ||
                        dx > pl->socket.mapx ||
                        dy < 0 ||
                        dy > pl->socket.mapy)
                    {
                        continue;
                    }

                    if (pl->blocked_los[dx][dy] & BLOCKED_LOS_BLOCKED)
                    {
                        pl->blocked_los[dx][dy] |= BLOCKED_LOS_EXPAND;
                    }
                }
            }
        }
    }

    for (x = 0; x < pl->socket.mapx; x++)
    {
        for (y = 0; y < pl->socket.mapy; y++)
        {
            if (pl->blocked_los[x][y] & BLOCKED_LOS_EXPAND)
            {
                pl->blocked_los[x][y] &= ~(BLOCKED_LOS_BLOCKED | BLOCKED_LOS_EXPAND);
            }
        }
    }
}
#else
/* update_los() recalculates the array which specifies what is
 * visible for the given player. */
void update_los(player_t *pl)
{
    object_t *who = pl->ob;
    sint16    i,
              j;

#ifdef DEBUG_CORE
    LOG(llevDebug, "LOS - %s\n", STRING_OBJ_NAME(who));
#endif

    /* Reset the array -- all msps are visible. */
    (void)memset((void *)pl->blocked_los, BLOCKED_LOS_VISIBLE, sizeof(pl->blocked_los));

    /* For wizpass, that is all. */
    if (pl->gmaster_wizpass)
    {
        return;
    }

    /* Work through the array, determining what is visible or not based on what
     * is actually on the map. */
    for (i = (MAP_CLIENT_X - pl->socket.mapx) / 2; i < (MAP_CLIENT_X + pl->socket.mapx) / 2; i++)
    {
        for (j = (MAP_CLIENT_Y - pl->socket.mapy) / 2; j < (MAP_CLIENT_Y + pl->socket.mapy) / 2; j++)
        {
            map_t  *m = who->map;
            sint16  x = who->x + i - MAP_CLIENT_X / 2,
                    y = who->y + j - MAP_CLIENT_Y / 2,
                    ax = i - (MAP_CLIENT_X - pl->socket.mapx) / 2,
                    ay = j - (MAP_CLIENT_Y - pl->socket.mapy) / 2;
            msp_t  *msp = MSP_GET(m, x, y);

            /* this skips the "edges" of view area, the border tiles.
             * Naturally, this tiles can't block any view - there is
             * nothing behind them. */
            if (!block[i][j].index)
            {
                /* to handle the "blocksview update" right, we give this special
                 * tiles a "never use it to trigger a los_update()" flag.
                 * blockview changes to this tiles will have no effect. */
                if (!msp)
                {
                    pl->blocked_los[ax][ay] |= BLOCKED_LOS_OUT_OF_MAP;
                }
                else
                {
                    pl->blocked_los[ax][ay] |= BLOCKED_LOS_IGNORE;
                }

                continue;
            }

            /* If the converted coordinates are outside the viewable
             * area for the client, return now. */
            if (ax < 0 ||
                ax >= pl->socket.mapx ||
                ay < 0 ||
                ay >= pl->socket.mapy)
            {
                continue;
            }

            /*LOG(llevNoLog,"SET_LOS: %d,%d\n", ax,ay);*/
            /* If this space is already blocked, prune the processing - presumably
             * whatever has set this space to be blocked has done the work and already
             * done the dependency chain.
             * but check for out_of_map to speedup our client map draw function. */
            if (!(pl->blocked_los[ax][ay] & BLOCKED_LOS_OUT_OF_MAP))
            {
                if (!msp)
                {
                    if (!(pl->blocked_los[ax][ay] & BLOCKED_LOS_BLOCKED))
                    {
                        BlockVista(pl, i, j);
                    }

                    pl->blocked_los[ax][ay] |= BLOCKED_LOS_OUT_OF_MAP;
                }
                else if ((msp->flags & MSP_FLAG_BLOCKSVIEW))
                {
                    if (!(pl->blocked_los[ax][ay] & BLOCKED_LOS_BLOCKED))
                    {
                        BlockVista(pl, i, j);
                    }

                    pl->blocked_los[ax][ay] |= BLOCKED_LOS_BLOCKSVIEW;
                }
            }
        }
    }

    ExpandSight(pl);

    /* If the player has xray vision, unblock nearby blocked squares. */
    /* TODO: Currently this is a fixed 9x9 area. Why not make it variable so
     * that we can have greater and lesser demon eyes?
     *
     * -- Smacky 20150812 */
    if (QUERY_FLAG(who, FLAG_XRAYS))
    {
        sint16 dx = pl->socket.mapx_2,
               dy = pl->socket.mapy_2;

        for (i = -4; i <= 4; i++)
        {
            for (j = -4; j <= 4; j++)
            {
                if (pl->blocked_los[dx + i][dy + j] & BLOCKED_LOS_BLOCKED)
                {
                    pl->blocked_los[dx + i][dy + j] &= ~BLOCKED_LOS_BLOCKED;
                }
            }
        }
    }
}

/* Used to initialise the array used by the LOS routines.
 * x,y are indexes into the blocked[][] array.
 * This recursively sets the blocked line of sight view.
 * From the blocked[][] array, we know for example
 * that if some particular space is blocked, it blocks
 * the view of the spaces 'behind' it, and those blocked
 * spaces behind it may block other spaces, etc.
 * In this way, the chain of visibility is set. */
static void BlockVista(player_t *pl, int x, int y)
{
    sint32  i;
    sint16  x2 = (MAP_CLIENT_X - pl->socket.mapx) / 2,
            y2 = (MAP_CLIENT_Y - pl->socket.mapy) / 2;

    for (i = 0; i < block[x][y].index; i++)
    {
        sint16 dx = block[x][y].x[i],
               dy = block[x][y].y[i],
               ax = dx - x2,
               ay = dy - y2;

        if (ax < 0 ||
            ax >= pl->socket.mapx ||
            ay < 0 ||
            ay >= pl->socket.mapy)
        {
            continue;
        }

#if 0
        LOG(llevInfo ,"blocked %d %d -> %d %d\n",dx, dy, ax, ay);
#endif
        /* we need to adjust to the fact that the socket
         * code wants the los to start from the 0,0
         * and not be relative to middle of los array.
         */
        if (!(pl->blocked_los[ax][ay] & BLOCKED_LOS_OUT_OF_MAP))
        {
            pl->blocked_los[ax][ay] |= BLOCKED_LOS_BLOCKED; /* this tile can't be seen */
        }

        BlockVista(pl, dx, dy);
    }
}

/*
 * ExpandSight goes through the array of what the given player is
 * able to see, and expands the visible area a bit, so the player will,
 * to a certain degree, be able to see into corners.
 * This is somewhat suboptimal, would be better to improve the formula.
 */
/* thats true: we should migrate this function asap in the los - a bit better
 * "pre calculated" LOS function.
 * It should be easy to do a better, optimized precalculation. MT-2004
 */
static void ExpandSight(player_t *pl)
{
    int i, x, y, dx, dy;

    for (x = 1; x < pl->socket.mapx - 1; x++)    /* loop over inner squares */
    {
        for (y = 1; y < pl->socket.mapy - 1; y++)
        {
#if 0
        LOG(llevInfo ,"ExpandSight x,y = %d, %d  blocksview = %d, %d\n",
            x, y, op->x-pl->socket.mapx_2+x, op->y-pl->socket.mapy_2+y);
#endif
            if (pl->blocked_los[x][y] <= BLOCKED_LOS_BLOCKSVIEW &&  /* if visible */
                !(pl->blocked_los[x][y] & BLOCKED_LOS_BLOCKSVIEW))  /* and not blocksview */
            {
                /* mark all directions */
                for (i = 1; i <= 8; i += 1)
                {
                    dx = x + OVERLAY_X(i);
                    dy = y + OVERLAY_Y(i);

                    if (dx < 0 ||
                        dx > pl->socket.mapx ||
                        dy < 0 ||
                        dy > pl->socket.mapy)
                    {
                        continue;
                    }

                    if ((pl->blocked_los[dx][dy] & BLOCKED_LOS_BLOCKED))
                    {
                        pl->blocked_los[dx][dy] |= BLOCKED_LOS_EXPAND;
                    }
                }
            }
        }
    }

    for (x = 0; x < pl->socket.mapx; x++)
    {
        for (y = 0; y < pl->socket.mapy; y++)
        {
            if ((pl->blocked_los[x][y] & BLOCKED_LOS_EXPAND))
            {
                pl->blocked_los[x][y] &= ~(BLOCKED_LOS_BLOCKED | BLOCKED_LOS_EXPAND);
            }
        }
    }
}
#endif

/*
 * Debug-routine which dumps the array which specifies the visible
 * area of a player.  Triggered by the z key in DM mode.
 */

void print_los(object_t *op)
{
    int     x, y;
    char    buf[50], buf2[10];

    strcpy(buf, "   ");
    for (x = 0; x < CONTR(op)->socket.mapx; x++)
    {
        sprintf(buf2, "%2d", x);
        strcat(buf, buf2);
    }
    ndi(NDI_UNIQUE, 0, op, "%s", buf);
    for (y = 0; y < CONTR(op)->socket.mapy; y++)
    {
        sprintf(buf, "%2d:", y);
        for (x = 0; x < CONTR(op)->socket.mapx; x++)
        {
            sprintf(buf2, " %1d", CONTR(op)->blocked_los[x][y]);
            strcat(buf, buf2);
        }
        ndi(NDI_UNIQUE, 0, op, "%s", buf);
    }
}

/*
 * make_sure_seen: The object is supposed to be visible through walls, thus
 * check if any players are nearby, and edit their LOS array.
 */
void make_sure_seen(object_t *op)
{
}

/*
 * make_sure_not_seen: The object which is supposed to be visible through
 * walls has just been removed from the map, so update the los of any
 * players within its range
 */

void make_sure_not_seen(object_t *op)
{
}

/** Tests if an object is in the line of sight of another object.
 * Relatively slow. Do not use for players, since they always have
 * a precalculated area of sight which can be used instead.
 * @param op the looking object
 * @param obj the object to look for
 * @param rv pre-calculated rv from op to obj
 * @return TRUE if there's an unblocked line of sight from op to obj, FALSE otherwise
 */
int obj_in_line_of_sight(object_t *op, object_t *obj, rv_t *rv)
{
    /* Bresenham variables */
    int fraction, dx2, dy2, stepx, stepy;

    /* Stepping variables */
    map_t *m = rv->part->map;
    sint16     x = rv->part->x,
               y = rv->part->y;

    /*
    LOG(llevDebug, "obj_in_line_of_sight(): %s (%d:%d) -> %s (%d:%d)?\n",
            STRING_OBJ_NAME(op), op->x, op->y,
            STRING_OBJ_NAME(obj), obj->x, obj->y);
    */

    BRESENHAM_INIT(rv->distance_x, rv->distance_y, fraction, stepx, stepy, dx2, dy2);

    while(1)
    {
        msp_t *msp;

//        LOG(llevDebug, " (%d:%d)", x, y);
        if(m == obj->map &&
           x == obj->x &&
           y == obj->y)
        {
//            LOG(llevDebug, "  can see!\n");
            return TRUE;
        }

        msp = MSP_GET(m, x, y);

        // Can't see if view blocked, unless mob has x-ray vision
        // and object (player) is in range.
        if (!msp ||
            ((msp->flags & MSP_FLAG_BLOCKSVIEW) &&
             (!QUERY_FLAG(op, FLAG_XRAYS) ||
              !mob_can_see_obj(op, obj, NULL))))
        {
//            LOG(llevDebug, "  blocked!\n");
            return FALSE;
        }

        BRESENHAM_STEP(x, y, fraction, stepx, stepy, dx2, dy2);
    }
/*
    LOG(llevDebug, "  out of range!\n");
    return FALSE;*/
}

#include "global.h"

/* This is much like the SIZEOFFREE stuff (see object.c) but operates on a max
 * 9x9 grid rather than 7x7. It *might* be worthwhile combining the two systems.
 *
 * Although I have added spacing to aid readability, the LightMasks[] is not
 * the most obvious thing in the world. The masks are arranged so that the
 * indices spiral outwards from the central point (index 0). A more readable/
 * maintainable arrangement might be so that the tables themselves represent a
 * flat mao of the masks (ie, basically so the central point (currently index
 * 0) is actually central to the array (so index 40). But the current
 * arrangement is probably faster in execution whi is the most important thing
 * (light masks change very frequently).
 *
 * -- Smacky 20140511 */
#define MAX_MASK_SIZE 81
#define NR_LIGHT_MASK 10
#define MAX_LIGHT_SOURCE 13

#if 0
static int LightMaskX[MAX_MASK_SIZE] =
{
    0,
    0, 1, 1, 1, 0, -1, -1, -1,
    0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1,
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1,
    0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0, -1, -2, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -2, -1
};

static int LightMaskY[MAX_MASK_SIZE] =
{
    0,
    -1, -1, 0, 1, 1, 1, 0, -1,
    -2, -2, -2, -1, 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2,
    -3, -3, -3, -3, -2, -1, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3,
    4, 4, 4, 4, 4, 3, 2, 1, 0, -1, -2, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -2, -1, 0, 1, 2, 3, 4, 4, 4, 4
};
#endif

static uint8 LightMask[MAX_LIGHT_SOURCE + 1] =
{
    0, 1, 2, 3, 4, 5, 6, 6, 7, 7, 8, 8, 8, 9
};

static int LightMaskWidth[NR_LIGHT_MASK] =
{
    0, 1, 2, 2, 3, 3, 3, 4, 4, 4
};

static int LightMaskSize[NR_LIGHT_MASK] =
{
    0, 9, 25, 25, 49, 49, 49, 81, 81, 81
};

static int LightMasks[NR_LIGHT_MASK][OVERLAY_9X9] =
{
    {0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0},
    {40  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0},
    {80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0},
    {160 , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0},
    {160 , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0},
    {320 , 
     160 , 160 , 160 , 160 , 160 , 160 , 160 , 160 , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0},
    {320 , 
     160 , 160 , 160 , 160 , 160 , 160 , 160 , 160 , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 
     0   , 0   , 0   , 0   , 0   , 0   , 0   , 0},
    {320 , 
     160 , 160 , 160 , 160 , 160 , 160 , 160 , 160 , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20  , 
     20  , 20  , 20  , 20  , 20  , 20  , 20  , 20},
    {640 , 
     320 , 320 , 320 , 320 , 320 , 320 , 320 , 320 , 
     160 , 160 , 160 , 160 , 160 , 160 , 160 , 160 , 
     160 , 160 , 160 , 160 , 160 , 160 , 160 , 160 , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40},
    {1280,
     640 , 640 , 640 , 640 , 640 , 640 , 640 , 640 , 
     160 , 160 , 160 , 160 , 160 , 160 , 160 , 160 , 
     160 , 160 , 160 , 160 , 160 , 160 , 160 , 160 , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     80  , 80  , 80  , 80  , 80  , 80  , 80  , 80  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40  , 
     40  , 40  , 40  , 40  , 40  , 40  , 40  , 40}
};

static inline sint8 GetRealLightSourceValue(int l);
static inline void  RemoveLightMask(map_t *map, sint16 x, sint16 y, sint8 id);
static inline uint8 AddLightMask(map_t *map, sint16 x, sint16 y, sint8 id);
static inline void  RestoreLightMask(map_t *restore_map, map_t *map, sint16 x, sint16 y, sint8 id);
static inline void  RemoveLightMaskOther(map_t *map, int x, int y, int id);

sint16 brightness[MAX_DARKNESS + 1] =
{
    0, 20, 40, 80, 160, 320, 640, 1280
};

/* returns true if op carries one or more lights
 * This is a trivial function now days, but it used to
 * be a bit longer.  Probably better for callers to just
 * check the op->glow_radius instead of calling this. */
int has_carried_lights(object_t *op)
{
    /* op is a light source or glowing */
    if (op->glow_radius > 0)
        return 1;

    return 0;
}

/* we add or remove a light source to a map space.
 * we adjust the light source map counter
 * and apply the area of light it invokes around it.
 */
void adjust_light_source(msp_t *msp, int light)
{
    sint8   olm,
            nlm;
    map_t  *m = msp->map;
    sint16  x = msp->x,
            y = msp->y;

    /* this happens, we don't change the intense of the old light mask */
    olm = GetRealLightSourceValue(msp->light_source); /* old mask */
    msp->light_source += light;
    nlm = GetRealLightSourceValue(msp->light_source); /* new mask */

    if (nlm == olm) /* old mask same as new one? */
    {
        return; /* not much to do */
    }

    if (olm)
    {
        RemoveLightMask(m, x, y, olm); /* remove the old light mask */

        /* remember - perhaps we are in this list - perhaps we are not! */
        if (msp->prev_light)
        {
            msp->prev_light->next_light = msp->next_light;
        }
        /* we are the list head */
        else if (m->first_light == msp)
        {
            m->first_light = msp->next_light;
        }

        if (msp->next_light) /* handle next link */
        {
            msp->next_light->prev_light = msp->prev_light; /* NULL or prev */
        }

        msp->prev_light = msp->next_light = NULL;
    }

    if (nlm)
    {
        if (AddLightMask(m, x, y, nlm)) /* add new light mask */
        {
            /* don't chain if we are chained previous */
            if (msp->next_light ||
                msp->prev_light ||
                m->first_light == msp)
            {
                return;
            }

            /* we should be always unlinked here - so link it now */
            msp->next_light = m->first_light;

            if (m->first_light)
            {
                msp->next_light->prev_light = msp;
            }

            m->first_light = msp;
        }
    }
}

static inline sint8 GetRealLightSourceValue(int l)
{
    if (l > MAX_LIGHT_SOURCE)
    {
        return LightMask[MAX_LIGHT_SOURCE];
    }
    else if (l < -MAX_LIGHT_SOURCE)
    {
        return -LightMask[MAX_LIGHT_SOURCE];
    }
    else if (l < 0)
    {
        return -LightMask[-l];
    }

    return LightMask[l];
}

static inline void RemoveLightMask(map_t *map, sint16 x, sint16 y, sint8 id)
{
    uint8 mask,
          mlen,
          i;

    if (id > 0) /* light masks */
    {
        mask = 1;
    }
    else
    {
        mask = 0;
        id = -id;
    }

    mlen = LightMaskSize[id];

    for (i = 0; i < mlen; i++)
    {
        map_t  *m2 = map;
        sint16      x2 = x + OVERLAY_X(i),
                    y2 = y + OVERLAY_Y(i);
        msp_t   *msp = MSP_GET2(m2, x2, y2);

        if (!msp)
        {
            continue;
        }

        x2 = LightMasks[id][i];
        msp->flooding_brightness += (mask) ? -x2 : x2;
    }
}

static inline uint8 AddLightMask(map_t *map, sint16 x, sint16 y, sint8 id)
{
    uint8 mask,
          mlen,
          i,
          map_flag = 0;

    if (id > 0) /* light masks */
    {
        mask = 1;
    }
    else
    {
        mask = 0;
        id = -id;
    }

    mlen = LightMaskSize[id];

    for (i = 0; i < mlen; i++)
    {
        map_t  *m2 = map;
        sint16      x2 = x + OVERLAY_X(i),
                    y2 = y + OVERLAY_Y(i);
        msp_t   *msp = MSP_GET2(m2, x2, y2);

        if (!msp)
        {
            if (x2)
            {
                map_flag = 1;
            }

            continue;
        }

        if (m2 != map)   /* this light mask cross some tiled map borders */
        {
            map_flag = 1;
        }

        x2 = LightMasks[id][i];
        msp->flooding_brightness += (mask) ? x2 : -x2;
    }

    return map_flag;
}

#define CHECKLSL(_MSP_, _X_, _Y_, _ID_) \
    if (!(_MSP_)->first) \
    { \
        LOG(llevBug, "BUG:: %s:check_light_source_list(): no object in msp of light source on %s %d %d!\n", \
            __FILE__, STRING_MAP_PATH((_MSP_)->map), (_MSP_)->x, (_MSP_)->y); \
        continue; \
    } \
    (_X_) = (_MSP_)->first->x; \
    (_Y_) = (_MSP_)->first->y; \
    (_ID_) = GetRealLightSourceValue((_MSP_)->light_source);

void check_light_source_list(map_t *m)
{
    map_t  *m2;
    sint16      x,
                y;
    msp_t   *msp;
    sint8       id;

    /*LOG(llevNoLog,"CHECK ALL LS for map:>%s<\n", m->path);*/

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTH]) &&
        (m2->in_memory == MAP_MEMORY_ACTIVE ||
         m2->in_memory == MAP_MEMORY_LOADING) &&
        m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s< (%d - %d)\n", x,y,m2->path,y+LightMaskWidth[id],m2->height);*/
            /* only light sources reaching in this map */
            if (y + LightMaskWidth[ABS(id)] < m2->height)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_EAST])
     && (m2->in_memory == MAP_MEMORY_ACTIVE || m2->in_memory == MAP_MEMORY_LOADING)
     && m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            /* only light sources reaching in this map */
            if (x - LightMaskWidth[ABS(id)] >= 0)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTH])
     && (m2->in_memory == MAP_MEMORY_ACTIVE || m2->in_memory == MAP_MEMORY_LOADING)
     && m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            /* only light sources reaching in this map */
            if (y - LightMaskWidth[ABS(id)] >= 0)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_WEST])
     && (m2->in_memory == MAP_MEMORY_ACTIVE || m2->in_memory == MAP_MEMORY_LOADING)
     && m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            /* only light sources reaching in this map */
            if (x + LightMaskWidth[ABS(id)] < m2->width)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTHEAST])
     && (m2->in_memory == MAP_MEMORY_ACTIVE || m2->in_memory == MAP_MEMORY_LOADING)
     && m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            /* only light sources reaching in this map */
            if ((y + LightMaskWidth[ABS(id)]) < m2->height || (x - LightMaskWidth[ABS(id)]) >= 0)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTHEAST])
     && (m2->in_memory == MAP_MEMORY_ACTIVE || m2->in_memory == MAP_MEMORY_LOADING)
     && m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            /* only light sources reaching in this map */
            if ((x - LightMaskWidth[ABS(id)]) >= 0 || (y - LightMaskWidth[ABS(id)]) >= 0)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_SOUTHWEST])
     && (m2->in_memory == MAP_MEMORY_ACTIVE || m2->in_memory == MAP_MEMORY_LOADING)
     && m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            /* only light sources reaching in this map */
            if ((y - LightMaskWidth[ABS(id)]) >= 0 || (x + LightMaskWidth[ABS(id)]) < m2->width)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }

    if ((m2 = m->tiling.tile_map[TILING_DIRECTION_NORTHWEST])
     && (m2->in_memory == MAP_MEMORY_ACTIVE || m2->in_memory == MAP_MEMORY_LOADING)
     && m2->first_light)
    {
        /* check this light source list */
        for (msp = m2->first_light; msp; msp = msp->next_light)
        {
            CHECKLSL(msp, x, y, id);

            /*LOG(llevNoLog,"check LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            /* only light sources reaching in this map */
            if ((y + LightMaskWidth[ABS(id)]) < m2->height
             || (x + LightMaskWidth[ABS(id)]) < m2->width)
            {
                continue;
            }

            /*LOG(llevNoLog,"restore LSL: %d,%d map:>%s<\n", x,y,m2->path);*/
            RestoreLightMask(m, m2, x, y, id);
        }
    }
}

#undef CHECKLSL

/* after loading a map, we check here all possible connected
 * maps for overlapping light sources. When we find one, we
 * adding the overlapping area to our new loaded map.
 */
static inline void RestoreLightMask(map_t *restore_map, map_t *map, sint16 x, sint16 y, sint8 id)
{
    uint8 mask,
          mlen,
          i;

    if (id > 0) /* light masks */
    {
        mask = 1;
    }
    else
    {
        mask = 0;
        id = -id;
    }

    mlen = LightMaskSize[id];

    for (i = 0; i < mlen; i++)
    {
        map_t  *m2 = map;
        sint16      x2 = x + OVERLAY_X(i),
                    y2 = y + OVERLAY_Y(i);
        msp_t   *msp = MSP_GET2(m2, x2, y2);

        if (!msp ||
            restore_map != m2)
        {
            continue;
        }

        x2 = LightMasks[id][i];
        msp->flooding_brightness += (mask) ? x2 : -x2;
    }
}

void remove_light_source_list(map_t *map)
{
    msp_t   *tmp;

    /*LOG(llevNoLog,"REMOVE LSL-LIST of map:>%s<\n",map->path);*/
    for (tmp = map->first_light; tmp; tmp = tmp->next_light)
    {
        /* again - there MUST be at last ONE object in this map space */
        if (!tmp->first)
        {
            LOG(llevBug, "BUG: remove_light_source_list() map:>%s< - no object in mapspace of light source!\n",
                map->path ? map->path : "NO MAP PATH?");
            continue;
        }
        /*LOG(llevNoLog,"remove LSL: %d,%d ,map:>%s<\n", tmp->first->x,tmp->first->y,map->path);*/
        RemoveLightMaskOther(map, tmp->first->x, tmp->first->y, GetRealLightSourceValue(tmp->light_source));
    }
    map->first_light = NULL;
}

/* only remove mask part from OTHER in memory maps! */
static inline void RemoveLightMaskOther(map_t *map, int x, int y, int id)
{
    uint8 mask,
          mlen,
          i;

    if (id > 0) /* light masks */
    {
        mask = 1;
    }
    else
    {
        mask = 0;
        id = -id;
    }

    mlen = LightMaskSize[id];

    for (i = 0; i < mlen; i++)
    {
        map_t  *m2 = map;
        sint16      x2 = x + OVERLAY_X(i),
                    y2 = y + OVERLAY_Y(i);
        msp_t   *msp = MSP_GET2(m2, x2, y2);

        if (!msp ||
            m2 == map) /* only legal OTHER maps */

        {
            continue;
        }

        x2 = LightMasks[id][i];
        msp->flooding_brightness += (mask) ? -x2 : x2;
    }
}
