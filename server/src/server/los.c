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

    /* This logs the top left quadrant of block[] so that mathematical
     * imbeciles like me can see what is going on. */
#ifdef LOS_DEBUG
    for (y = 0; y <= MAP_CLIENT_Y / 2; y++)
    {
        for (x = 0; x <= MAP_CLIENT_X / 2; x++)
        {
            char buf[TINY_BUF];

            sprintf(buf, "%d=", block[x][y].index);

            for (i = 0; i < block[x][y].index; i++)
            {
                sprintf(strchr(buf, '\0'), "%d,%d ", block[x][y].x[i], block[x][y].y[i]);
            }

            LOG(llevInfo, "%-14s", buf);
        }

        LOG(llevInfo, "\n");
    }
#endif
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
                        pl->blocked_los[dx][dy] |= BLOCKED_LOS_INTERNAL;
                    }
                }
            }
        }
    }

    for (x = 0; x < pl->socket.mapx; x++)
    {
        for (y = 0; y < pl->socket.mapy; y++)
        {
            if (pl->blocked_los[x][y] & BLOCKED_LOS_INTERNAL)
            {
                pl->blocked_los[x][y] &= ~(BLOCKED_LOS_BLOCKED | BLOCKED_LOS_INTERNAL);
            }
        }
    }
}

static sint16 LosOverlay[MAP_CLIENT_Y][MAP_CLIENT_X] =
{
    {281,282,283,284,285,286,287,288,225,226,227,228,229,230,231,232,233,},
    {280,218,219,220,221,222,223,224,169,170,171,172,173,174,175,176,234,},
    {279,217,163,164,165,166,167,168,121,122,123,124,125,126,127,177,235,},
    {278,216,162,116,117,118,119,120, 81, 82, 83, 84, 85, 86,128,178,236,},
    {277,215,161,115, 77, 78, 79, 80, 49, 50, 51, 52, 53, 87,129,179,237,},
    {276,214,160,114, 76, 46, 47, 48, 25, 26, 27, 28, 54, 88,130,180,238,},
    {275,213,159,113, 75, 45, 23, 24,  9, 10, 11, 29, 55, 89,131,181,239,},
    {274,212,158,112, 74, 44, 22,  8,  1,  2, 12, 30, 56, 90,132,182,240,},
    {273,211,157,111, 73, 44, 21,  7,  0,  3, 13, 31, 57, 91,133,183,241,},
    {272,210,156,110, 72, 42, 20,  6,  5,  4, 14, 32, 58, 92,134,184,242,},
    {271,209,155,109, 71, 41, 19, 18, 17, 16, 15, 33, 59, 93,135,185,243,},
    {270,208,154,108, 70, 40, 39, 38, 37, 36, 35, 34, 60, 94,136,186,244,},
    {269,207,153,107, 69, 68, 67, 66, 65, 64, 63, 62, 61, 95,137,187,245,},
    {268,206,152,106,105,104,103,102,101,100, 99, 98, 97, 96,138,188,246,},
    {267,205,151,150,149,148,147,146,145,144,143,142,141,140,139,189,247,},
    {266,204,203,202,201,200,199,198,197,196,195,194,193,192,191,190,248,},
    {265,264,263,262,261,260,259,258,257,256,255,254,253,252,251,250,249,},
};

static sint8 LosX[289] =
{
    0,
    0,1,1,1,0,-1,-1,-1,
    0,1,2,2,2,2,2,1,0,-1,-2,-2,-2,-2,-2,-1,
    0,1,2,3,3,3,3,3,3,3,2,1,0,-1,-2,-3,-3,-3,-3,-3,-3,-3,-2,-1,
    0,1,2,3,4,4,4,4,4,4,4,4,4,3,2,1,0,-1,-2,-3,-4,-4,-4,-4,-4,-4,-4,-4,-4,-3,-2,-1,
    0,1,2,3,4,5,5,5,5,5,5,5,5,5,5,5,4,3,2,1,0,-1,-2,-3,-4,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-4,-3,-2,-1,
    0,1,2,3,4,5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-5,-4,-3,-2,-1,
    0,1,2,3,4,5,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-6,-5,-4,-3,-2,-1,
    0,1,2,3,4,5,6,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-7,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-7,-6,-5,-4,-3,-2,-1,
};

static sint8 LosY[289] =
{
    0,
    -1,-1,0,1,1,1,0,-1,
    -2,-2,-2,-1,0,1,2,2,2,2,2,1,0,-1,-2,-2,
    -3,-3,-3,-3,-2,-1,0,1,2,3,3,3,3,3,3,3,2,1,0,-1,-2,-3,-3,-3,
    -4,-4,-4,-4,-4,-3,-2,-1,0,1,2,3,4,4,4,4,4,4,4,4,4,3,2,1,0,-1,-2,-3,-4,-4,-4,-4,
    -5,-5,-5,-5,-5,-5,-4,-3,-2,-1,0,1,2,3,4,5,5,5,5,5,5,5,5,5,5,5,4,3,2,1,0,-1,-2,-3,-4,-5,-5,-5,-5,-5,
    -6,-6,-6,-6,-6,-6,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,6,6,6,6,6,6,6,6,6,6,6,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-6,-6,-6,-6,-6,
    -7,-7,-7,-7,-7,-7,-7,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-7,-7,-7,-7,-7,-7,-7,
    -8,-8,-8,-8,-8,-8,-8,-8,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,7,6,5,4,3,2,1,0,-1,-2,-3,-4,-5,-6,-7,-8,-8,-8,-8,-8,-8,-8,-8,
};

static sint16 Block[225][3] =
{
    {0,0,0},
    {9,-10,-24},{-10,11,-12},{-12,13,-14},{-14,15,-16},{-16,17,-18},{-18,19,-20},{-20,21,-22},{-22,23,-24},
    {25,-26,-48},{0,-26,-27},{-27,28,-29},{0,-29,-30},{-30,31,-32},{0,-32,-33},{-33,34,-35},{0,-35,-36},{-36,37,-38},{0,-38,-39},{-39,40,-41},{0,-41,-42},{-42,43,-44},{0,-44,-45},{-45,46,-47},{0,-47,-48},
    {49,-50,-80},{0,-50,-51},{0,-51,-52},{-52,53,-54},{0,-54,-55},{0,-55,-56},{-56,57,-58},{0,-58,-59},{0,-59,-60},{-60,61,-62},{0,-62,-63},{0,-63,-64},{-64,65,-66},{0,-66,-67},{0,-67,-68},{-68,69,-70},{0,-70,-71},{0,-71,-72},{-72,73,-74},{0,-74,-75},{0,-75,-76},{-76,77,-78},{0,-78,-79},{0,-79,-80},
    {81,-82,-120},{0,-82,-83},{0,-83,-84},{0,-84,-85},{-85,86,-87},{0,-87,-88},{0,-88,-89},{0,-89,-90},{-90,91,-92},{0,-92,-93},{0,-93,-94},{0,-94,-95},{-95,96,-97},{0,-97,-98},{0,-98,-99},{0,-99,-100},{-100,101,-102},{0,-102,-103},{0,-103,-104},{0,-104,-105},{-105,106,-107},{0,-107,-108},{0,-108,-109},{0,-109,-110},{-110,111,-112},{0,-112,-113},{0,-113,-114},{0,-114,-115},{-115,116,-117},{0,-117,-118},{0,-118,-119},{0,-119,-120},
    {121,-122,-168},{0,-122,-123},{0,-123,-124},{0,-124,-125},{0,-125,-126},{-126,127,-128},{0,-128,-129},{0,-129,-130},{0,-130,-131},{0,-131,-132},{-132,133,-134},{0,-134,-135},{0,-135,-136},{0,-136,-137},{0,-137,-138},{-138,139,-140},{0,-140,-141},{0,-141,-142},{0,-142,-143},{0,-143,-144},{-144,145,-146},{0,-146,-147},{0,-147,-148},{0,-148,-149},{0,-149,-150},{-150,151,-152},{0,-152,-153},{0,-153,-154},{0,-154,-155},{0,-155,-156},{-156,157,-158},{0,-158,-159},{0,-159,-160},{0,-160,-161},{0,-161,-162},{-162,163,-164},{0,-164,-165},{0,-165,-166},{0,-166,-167},{0,-167,-168},
    {169,-170,-224},{0,-170,-171},{0,-171,-172},{0,-172,-173},{0,-173,-174},{0,-174,-175},{-175,176,-177},{0,-177,-178},{0,-178,-179},{0,-179,-180},{0,-180,-181},{0,-181,-182},{-182,183,-184},{0,-184,-185},{0,-185,-186},{0,-186,-187},{0,-187,-188},{0,-188,-189},{-189,190,-191},{0,-191,-192},{0,-192,-193},{0,-193,-194},{0,-194,-195},{0,-195,-196},{-196,197,-198},{0,-198,-199},{0,-199,-200},{0,-200,-201},{0,-201,-202},{0,-202,-203},{-203,204,-205},{0,-205,-206},{0,-206,-207},{0,-207,-208},{0,-208,-209},{0,-209,-210},{-210,211,-212},{0,-212,-213},{0,-213,-214},{0,-214,-215},{0,-215,-216},{0,-216,-217},{-217,218,-219},{0,-219,-220},{0,-220,-221},{0,-221,-222},{0,-222,-223},{0,-223,-224},
    {225,-226,-288},{0,-226,-227},{0,-227,-228},{0,-228,-229},{0,-229,-230},{0,-230,-231},{0,-231,-232},{-232,233,-234},{0,-234,-235},{0,-235,-236},{0,-236,-237},{0,-237,-238},{0,-238,-239},{0,-239,-240},{-240,241,-242},{0,-242,-243},{0,-243,-244},{0,-244,-245},{0,-245,-246},{0,-246,-247},{0,-247,-248},{-248,249,-250},{0,-250,-251},{0,-251,-252},{0,-252,-253},{0,-253,-254},{0,-254,-255},{0,-255,-256},{-256,257,-258},{0,-258,-259},{0,-259,-260},{0,-260,-261},{0,-261,-262},{0,-262,-263},{0,-263,-264},{-264,265,-266},{0,-266,-267},{0,-267,-268},{0,-268,-269},{0,-269,-270},{0,-270,-271},{0,-271,-272},{-272,273,-274},{0,-274,-275},{0,-275,-276},{0,-276,-277},{0,-277,-278},{0,-278,-279},{0,-279,-280},{-280,281,-282},{0,-282,-283},{0,-283,-284},{0,-284,-285},{0,-285,-286},{0,-286,-287},{0,-287,-288},
};

/* update_los() recalculates the array which specifies what is visible for pl.
 * In summary, it does this by first resetting each element to 0 so every
 * square is visible. For gmaster_wizpass, this is all so just return. For
 * others step through the array, comparing each element to the corresponding
 * msp. */
void update_los(player_t *pl)
{
    if (!pl->use_old_los)
    {
    object_t *who = pl->ob;
    sint16    i,
              j,
              ax,
              ay;

    TPR_START();
#ifdef DEBUG_CORE
    LOG(llevDebug, "LOS - %s\n", STRING_OBJ_NAME(who));
#endif

    /* Players with wizpass, can see all (use IGNORE rather than VISIBLE to
     * skip pointless updates in view_map.c:draw_map()). */
    if (pl->gmaster_wizpass)
    {
        (void)memset((void *)pl->blocked_los, BLOCKED_LOS_IGNORE, sizeof(pl->blocked_los));
        TPR_BREAK("LOS new wizpass");
        return;
    }
    /* A blind player can see nothing except the square he is on. */
    else if (QUERY_FLAG(who, FLAG_BLIND))
    {
        (void)memset((void *)pl->blocked_los, BLOCKED_LOS_BLOCKED, sizeof(pl->blocked_los));
        pl->blocked_los[pl->socket.mapx_2][pl->socket.mapy_2] = BLOCKED_LOS_IGNORE;
        TPR_BREAK("LOS new blind");
        return;
    }

    /* Reset the array -- all msps are visible by default. */
    (void)memset((void *)pl->blocked_los, BLOCKED_LOS_VISIBLE, sizeof(pl->blocked_los));

    /* The central position (where the player is) must be handled specially.
     * This is because it is never considered to be blocksview so the 8 squares
     * around it are never blocked. This means that, eg, an ethereal player
     * passing though a wall (normally blocksview) has his immediate view
     * unhindered. */
    ax = pl->socket.mapx_2;
    ay = pl->socket.mapy_2;
    pl->blocked_los[ax][ay] = BLOCKED_LOS_IGNORE;

    for (i = 1; i < MAP_CLIENT_X * MAP_CLIENT_Y; i++)
    {
        map_t  *m = who->map;
        sint16  x = who->x + LosX[i],
                y = who->y + LosY[i];
        msp_t  *msp = MSP_GET(m, x, y);

        ax = MAP_CLIENT_X / 2 + LosX[i];
        ay = MAP_CLIENT_Y / 2 + LosY[i];

        if (!msp ||
            i >= pl->socket.mapx * pl->socket.mapy)
        {
            pl->blocked_los[ax][ay] = BLOCKED_LOS_OUT_OF_MAP;
            continue;
        }

        if ((msp->flags & MSP_FLAG_BLOCKSVIEW))
        {
            pl->blocked_los[ax][ay] |= BLOCKED_LOS_BLOCKSVIEW;
            pl->blocked_los[ax][ay] &= ~BLOCKED_LOS_OBSCURED;
        }
        else if ((msp->flags & MSP_FLAG_OBSCURESVIEW))
        {
            pl->blocked_los[ax][ay] |= BLOCKED_LOS_OBSCURESVIEW;
        }
        else if ((msp->flags & MSP_FLAG_ALLOWSVIEW))
        {
            pl->blocked_los[ax][ay] |= BLOCKED_LOS_ALLOWSVIEW;
            pl->blocked_los[ax][ay] &= ~BLOCKED_LOS_OBSCURED;
        }

        if (i >= (pl->socket.mapx - 2) * (pl->socket.mapy - 2))
        {
            pl->blocked_los[ax][ay] |= BLOCKED_LOS_IGNORE;
            continue;
        }

        if ((pl->blocked_los[ax][ay] & (BLOCKED_LOS_BLOCKSVIEW | BLOCKED_LOS_BLOCKED)))
        {
            for (j = 0; j <= 2; j++)
            {
                sint16 k = Block[i][j],
                       dx,
                       dy;

                if (!k)
                {
                    continue;
                }

                dx = pl->socket.mapx_2 + LosX[ABS(k)];
                dy = pl->socket.mapy_2 + LosY[ABS(k)];

                if (k < 0)
                {
                    pl->blocked_los[dx][dy] |= ((pl->blocked_los[dx][dy] & BLOCKED_LOS_OBSCURED)) ? BLOCKED_LOS_BLOCKED : BLOCKED_LOS_OBSCURED;
                }
                else if (k > 0)
                {
                    pl->blocked_los[dx][dy] |= BLOCKED_LOS_BLOCKED;
                }
            }
        }
        else if ((pl->blocked_los[ax][ay] & (BLOCKED_LOS_OBSCURESVIEW | BLOCKED_LOS_OBSCURED)))
        {
            for (j = 0; j <= 2; j++)
            {
                sint16 k = Block[i][j],
                       dx,
                       dy;

                if (!k)
                {
                    continue;
                }

                dx = pl->socket.mapx_2 + LosX[ABS(k)];
                dy = pl->socket.mapy_2 + LosY[ABS(k)];
                pl->blocked_los[dx][dy] |= BLOCKED_LOS_OBSCURED;
            }
        }
    }

    for (i = 1; i < (pl->socket.mapx - 2) * (pl->socket.mapy - 2); i++)
    {
        ax = MAP_CLIENT_X / 2 + LosX[i];
        ay = MAP_CLIENT_Y / 2 + LosY[i];

        if (!(pl->blocked_los[ax][ay] & (BLOCKED_LOS_BLOCKSVIEW | BLOCKED_LOS_BLOCKED | BLOCKED_LOS_OUT_OF_MAP)))
        {
            for (j = 1; j <= 8; j++)
            {
                sint16 dx = ax + LosX[j],
                       dy = ay + LosY[j];

                if ((pl->blocked_los[dx][dy] & (BLOCKED_LOS_ALLOWSVIEW | BLOCKED_LOS_BLOCKSVIEW)))
                {
                    pl->blocked_los[dx][dy] &= ~BLOCKED_LOS_BLOCKED;
                }
            }
        }
    }

    /* If the player has xray vision, unblock nearby blocked squares. */
    /* TODO: Currently this is a fixed 2 square radius. Why not make it
     * variable so that we can have greater and lesser demon eyes?
     *
     * -- Smacky 20150812 */
    if (QUERY_FLAG(who, FLAG_XRAYS))
    {
        for (j = 1; j <= 24; j++)
        {
            ax = pl->socket.mapx_2 + LosX[j],
            ay = pl->socket.mapy_2 + LosY[j];
            pl->blocked_los[ax][ay] &= ~BLOCKED_LOS_BLOCKED;
        }
    }

    TPR_STOP("LOS new full");
    }
    else
    {
    object_t *who = pl->ob;
    sint16    i,
              j;

    TPR_START();
#ifdef DEBUG_CORE
    LOG(llevDebug, "LOS - %s\n", STRING_OBJ_NAME(who));
#endif

    /* Reset the array -- all msps are visible. */
    (void)memset((void *)pl->blocked_los, BLOCKED_LOS_VISIBLE, sizeof(pl->blocked_los));

    /* For wizpass, that is all. */
    if (pl->gmaster_wizpass)
    {
        TPR_BREAK("LOS old wizpass");
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

    TPR_STOP("LOS old full");
    }
}

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

/* los_find_target() attempts to find the next appropriate target in player's
 * LoS. If nothing can be found, default self. */
void los_find_target(player_t *pl, uint8 mode, sint16 start, sint16 stop, sint16 ox, sint16 oy)
{
    sint16    i;
    object_t *who = pl->ob;

    /* Loop i from start + 1 to stop to start or 0. */
    for (i = start + 1; i > start; i = (i < stop) ? i + 1 : ((start < 0) ? start : 0))
    {
        sint16    ax = MAP_CLIENT_X / 2 + ox + LosX[i],
                  ay = MAP_CLIENT_Y / 2 + oy + LosY[i];
        map_t    *m;
        sint16    x,
                  y;
        msp_t    *msp;
        object_t *this,
                 *next;

        /* Can't see this square for whatever reason? It's obviously not in
         * contention for targeting then. */
        if (ax < 0 ||
            ax >= pl->socket.mapx ||
            ay < 0 ||
            ay >= pl->socket.mapy ||
            (pl->blocked_los[ax][ay] & (BLOCKED_LOS_BLOCKED | BLOCKED_LOS_OUT_OF_MAP)))
        {
            continue;
        }

        m = who->map;
        x = who->x + ox + LosX[i];
        y = who->y + oy + LosY[i];

        msp = MSP_GET2(m, x, y);

        if (!msp)
        {
            LOG(llevBug, "BUG:: %s:los_find_target(): missing msp?\n", __FILE__);
            LOS_SET_TARGET(pl, who, LOS_TARGET_SELF, 0);
            return;
        }

        /* Only monsters and players are targetable so we can immediately
         * discount this msp if it lackks both flags. */
        if (!(msp->flags & (MSP_FLAG_ALIVE | MSP_FLAG_PLAYER)))
        {
            continue;
        }

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            if (this != who &&
                IS_LIVE(this))
            {
                object_t *target = (this->head) ? this->head : this;

                if (target != pl->target_ob &&
                    LOS_VALIDATE_TARGET(pl, target, 0))
                {
                    sint32 friendship = get_friendship(who, target);
                    uint8  target_mode = LOS_GET_REAL_TARGET_MODE(target, mode, friendship);

                    LOS_SET_TARGET(pl, target, target_mode, LosOverlay[ay][ax]);
                    return;
                }
            }
        }
    }

    LOS_SET_TARGET(pl, who, LOS_TARGET_SELF, 0);
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
