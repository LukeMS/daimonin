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


/* NOTE: I have added the new commands as (slow) string stuff.
 * The only reason is that they are simple to debug.
 * We have at, 2 command system - this one and the stuff in commands.c.
 * I plan to rework the command system - in 2 steps (new protocol and then later UDP socket)
 * or one step - new protocol, based on UDP.
 */

/* This file deals with administrative commands from the client. */
#include <global.h>

#define MAP_POS_X 0
#define MAP_POS_Y 1

static int  map_pos_array[][2]  =
{
    {0,0}, {0, -1}, {1, -1}, {1,0}, {1,1}, {0,1}, { - 1,1}, { - 1,0}, { - 1, -1}, {0, -2}, {1, -2}, {2, -2}, {2, -1},
    {2,0}, {2,1}, {2,2}, {1,2}, {0,2}, { - 1,2}, { - 2,2}, { - 2,1}, { - 2,0}, { - 2, -1}, { - 2, -2}, { - 1, -2},
    {0, -3}, {1, -3}, {2, -3}, {3, -3}, {3, -2}, {3, -1}, {3,0}, {3,1}, {3,2}, {3,3}, {2,3}, {1,3}, {0,3}, { - 1,3},
    { - 2,3}, { - 3,3}, { - 3,2}, { - 3,1}, { - 3,0}, { - 3, -1}, { - 3, -2}, { - 3, -3}, { - 2, -3}, { - 1, -3},
    {0, -4}, {1, -4}, {2, -4}, {3, -4}, {4, -4}, {4, -3}, {4, -2}, {4, -1}, {4,0}, {4,1}, {4,2}, {4,3}, {4,4}, {3,4},
    {2,4}, {1,4}, {0,4}, { - 1,4}, { - 2,4}, { - 3,4}, { - 4,4}, { - 4,3}, { - 4,2}, { - 4,1}, { - 4,0}, { - 4, -1},
    { - 4, -2}, { - 4, -3}, { - 4, -4}, { - 3, -4}, { - 2, -4}, { - 1, -4}, {0, -5}, {1, -5}, {2, -5}, {3, -5}, {4, -5},
    {5, -5}, {5, -4}, {5, -3}, {5, -2}, {5, -1}, {5,0}, {5,1}, {5,2}, {5,3}, {5,4}, {5,5}, {4,5}, {3,5}, {2,5}, {1,5},
    {0,5}, { - 1,5}, { - 2,5}, { - 3,5}, { - 4,5}, { - 5,5}, { - 5,4}, { - 5,3}, { - 5,2}, { - 5,1}, { - 5,0},
    { - 5, -1}, { - 5, -2}, { - 5, -3}, { - 5, -4}, { - 5, -5}, { - 4, -5}, { - 3, -5}, { - 2, -5}, { - 1, -5}, {0, -6},
    {1, -6}, {2, -6}, {3, -6}, {4, -6}, {5, -6}, {6, -6}, {6, -5}, {6, -4}, {6, -3}, {6, -2}, {6, -1}, {6,0}, {6,1},
    {6,2}, {6,3}, {6,4}, {6,5}, {6,6}, {5,6}, {4,6}, {3,6}, {2,6}, {1,6}, {0,6}, { - 1,6}, { - 2,6}, { - 3,6}, { - 4,6},
    { - 5,6}, { - 6,6}, { - 6,5}, { - 6,4}, { - 6,3}, { - 6,2}, { - 6,1}, { - 6,0}, { - 6, -1}, { - 6, -2}, { - 6, -3},
    { - 6, -4}, { - 6, -5}, { - 6, -6}, { - 5, -6}, { - 4, -6}, { - 3, -6}, { - 2, -6}, { - 1, -6}, {0, -7}, {1, -7},
    {2, -7}, {3, -7}, {4, -7}, {5, -7}, {6, -7}, {7, -7}, {7, -6}, {7, -5}, {7, -4}, {7, -3}, {7, -2}, {7, -1}, {7,0},
    {7,1}, {7,2}, {7,3}, {7,4}, {7,5}, {7,6}, {7,7}, {6,7}, {5,7}, {4,7}, {3,7}, {2,7}, {1,7}, {0,7}, { - 1,7},
    { - 2,7}, { - 3,7}, { - 4,7}, { - 5,7}, { - 6,7}, { - 7,7}, { - 7,6}, { - 7,5}, { - 7,4}, { - 7,3}, { - 7,2},
    { - 7,1}, { - 7,0}, { - 7, -1}, { - 7, -2}, { - 7, -3}, { - 7, -4}, { - 7, -5}, { - 7, -6}, { - 7, -7}, { - 6, -7},
    { - 5, -7}, { - 4, -7}, { - 3, -7}, { - 2, -7}, { - 1, -7}, {0, -8}, {1, -8}, {2, -8}, {3, -8}, {4, -8}, {5, -8},
    {6, -8}, {7, -8}, {8, -8}, {8, -7}, {8, -6}, {8, -5}, {8, -4}, {8, -3}, {8, -2}, {8, -1}, {8,0}, {8,1}, {8,2},
    {8,3}, {8,4}, {8,5}, {8,6}, {8,7}, {8,8}, {7,8}, {6,8}, {5,8}, {4,8}, {3,8}, {2,8}, {1,8}, {0,8}, { - 1,8},
    { - 2,8}, { - 3,8}, { - 4,8}, { - 5,8}, { - 6,8}, { - 7,8}, { - 8,8}, { - 8,7}, { - 8,6}, { - 8,5}, { - 8,4},
    { - 8,3}, { - 8,2}, { - 8,1}, { - 8,0}, { - 8, -1}, { - 8, -2}, { - 8, -3}, { - 8, -4}, { - 8, -5}, { - 8, -6},
    { - 8, -7}, { - 8, -8}, { - 7, -8}, { - 6, -8}, { - 5, -8}, { - 4, -8}, { - 3, -8}, { - 2, -8}, { - 1, -8}
};

#define NROF_MAP_NODE ((int)(sizeof(map_pos_array) /(sizeof(int)*2)))


int command_run(object_t *op, char *params)
{
    CONTR(op)->run_on = 1;
    move_player(op, params ? atoi(params) : 0, TRUE);

    return 0;
}

int command_run_stop(object_t *op, char *params)
{
    CONTR(op)->run_on = 0;

    return 0;
}

int command_combat(object_t *op, char *params)
{
    if (!op || !op->map || op->type != PLAYER || !CONTR(op))
        return 0;

    CONTR(op)->rest_sitting = CONTR(op)->rest_mode = 0;

    if (CONTR(op)->combat_mode)
        CONTR(op)->combat_mode = 0;
    else

        CONTR(op)->combat_mode = 1;

    update_pets_combat_mode(op);

    send_target_command(CONTR(op));

    return 0;
}

void target_self(object_t *op)
{
    CONTR(op)->target_object = op;
    CONTR(op)->target_level = op->level;
    CONTR(op)->target_object_count = op->count;
    CONTR(op)->target_map_pos = 0;
}

/** Filter for valid targets */
static int valid_new_target(object_t *op, object_t *candidate)
{
    /* TODO: how about golems? */
    if (IS_LIVE(candidate))
    {
        if (candidate == CONTR(op)->target_object ||
            QUERY_FLAG(candidate, FLAG_SYS_OBJECT) ||
            IS_GMASTER_INVIS_TO(candidate, op) ||
            IS_NORMAL_INVIS_TO(candidate, op))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

/* enter combat mode and attack the object in front of us - IF we are in combat
 * and have a enemy/target, skip it and stop attacking.
 */
/* TODO: at some time, we should move the target stuff to the client. but for this,
 * we need a better and smarter client information strategy - MT2003
 */
/* this function needs a rework... its a bit bulky after adding all this exceptions MT-2004 */
/*
 * Currently, "target friend" targets mobs with friendship >= FRIENDSHIP_HELP,
 * which is basically anyone of the same alignment or players on non-pvp maps.
 * "target enemy" targets anything with friendship < FRIENDSHIP_HELP which
 * includes neutral NPCs and PvP players. This ensures that "target enemy"
 * includes all potential targets but gives a big risk for unintentional attacks
 * on neutral targets. Gecko 2006-05-01
 */
/* "target talk" targets both enemies and friends (first come, first served),
 * but exempts player. BTW this entire targetting system, this function
 * particularly, needs a rewrite.
 * -- Smacky 20090502 */
/* The "target friend" code works differently on PvP maps only - After the target
 * moves to the furthest away friend, it'll begin the loop again, whereas in the other 
 * code, it'll target self. The target friend code is very odd anyway. I'll add to the 
 * clamor and say that this function needs a rewrite (and also possibly moved to the client),
 * but I can't do it now, too much other stuff I want to work on. -- _people_*/

int command_target(object_t *op, char *params)
{
    player_t   *pl = NULL;
    object_t   *tmp,
               *next,
               *head;
    int         jump_in, jump_in_n = 0, get_ob_flag;
    int         n, nt;
    sint16      x2,
                y2;

    if (!op ||
        !op->map ||
        op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return 0;
    }

    if (!params || params[0] == '\0')
        return 1;

    x2 = pl->socket.mapx_2,
    y2 = pl->socket.mapy_2;

    /* !x y = mouse map target */
    if (params[0] == '!')
    {
        int     xstart, ystart;
        char   *ctmp;

        xstart = atoi(params + 1);
        ctmp = strchr(params + 1, ' ');
        if (!ctmp) /* bad format.. skip */
            return 0;

        ystart = atoi(ctmp + 1);

        for (n = 0; n < OVERLAY_7X7; n++)
        {
            map_t *m = op->map;
            /* thats the trick: we get  op map pos, but we have 2 offsets:
             * the offset from the client mouse click - can be
             * +- pl->socket.mapx/2 - and the freearr_x/y offset for
             * the search. */
            sint16     xx = OVERLAY_X(n) + xstart,
                       yy = OVERLAY_Y(n) + ystart,
                       x = op->x + xx,
                       y = op->y + yy;
            msp_t  *msp;

            if (xx < -x2 ||
                xx > x2 ||
                yy < -y2 ||
                yy > y2 ||
                pl->blocked_los[xx + x2][yy + y2] > BLOCKED_LOS_BLOCKSVIEW)
            {
                continue;
            }

            msp = MSP_GET(m, x, y);

            if (!msp)
            {
                continue;
            }

            /* we can have more as one possible target
             * on a square - but i try this first without
             * handle it.
             */
            FOREACH_OBJECT_IN_MSP(tmp, msp, next)
            {
                /* this is a possible target */
                head = (tmp->head) ? tmp->head : tmp;

                if (valid_new_target(op, head) &&
                    head != pl->target_object)
                {
                    pl->target_object = head;
                    pl->target_level = head->level;
                    pl->target_object_count = head->count;
                    pl->target_map_pos = n;
                    send_target_command(pl);
                    return 0;
                }
            }
        }
    }
    else if (params[0] == '0')
    {
        // Loop through the map_pos_array and try to find the nearest enemy to the op.
        for (n = pl->target_map_pos; n < NROF_MAP_NODE; n++)
        {
            map_t *m = op->map;
            /* thats the trick: we get  op map pos, but we have 2 offsets:
             * the offset from the client mouse click - can be
             * +- pl->socket.mapx/2 - and the freearr_x/y offset for
             * the search. */
            sint16     xx = map_pos_array[n][MAP_POS_X],
                       yy = map_pos_array[n][MAP_POS_Y],
                       x = op->x + xx,
                       y = op->y + yy;
            msp_t  *msp;

            if (xx < -x2 ||
                xx > x2 ||
                yy < -y2 ||
                yy > y2 ||
                pl->blocked_los[xx + x2][yy + y2] > BLOCKED_LOS_BLOCKSVIEW)
            {
                continue;
            }

            msp = MSP_GET(m, x, y);

            if (!msp)
            {
                continue;
            }

            /* we can have more as one possible target
             * on a square - but i try this first without
             * handle it.
             */
            FOREACH_OBJECT_IN_MSP(tmp, msp, next)
            {
                /* this is a possible target */
                head = (tmp->head) ? tmp->head : tmp;

                if (valid_new_target(op, head)
                    && get_friendship(op, head) <= FRIENDSHIP_ATTACK
                    && head != pl->target_object)
                {
                    pl->target_object = head;
                    pl->target_level = head->level;
                    pl->target_object_count = head->count;
                    pl->target_map_pos = n;
                    send_target_command(pl);
                    return 0;
                }
            }
        }
    }
    else if (params[0] == '1') /* friend */
    {
        /* if /target friend but old target was enemy - target self first */
        if (OBJECT_VALID(pl->target_object, pl->target_object_count) &&
            get_friendship(op, pl->target_object) < FRIENDSHIP_HELP)
        {
            target_self(op);
        }
        else /* ok - search for a friendly object now! */
        {
            /* if our target before was a non enemy, start new search
             * if it was an enemy, use old value.
             */
            n = 0;
            nt = -1;
            /* lets search for last friendly object position! */
            if (pl->target_object == op)
            {
                get_ob_flag = 0;
                jump_in = 1;
                jump_in_n = n;
                tmp = op->above;
            }
            else if (OBJECT_VALID(pl->target_object, pl->target_object_count)
                  && get_friendship(op, pl->target_object) >= FRIENDSHIP_HELP)
            {
                get_ob_flag = 0;
                jump_in = 1;
                n = pl->target_map_pos;
                jump_in_n = n;
                tmp = pl->target_object->above;
            }
            else
            {
                n = 1;
                pl->target_object = NULL;
                jump_in = 0;
                get_ob_flag = 1;
            }

            for (; n < NROF_MAP_NODE && n != nt; n++)
            {
                map_t *m = op->map;
                sint16     xx,
                           yy,
                           x,
                           y;
                msp_t  *msp;

                if (nt == -1)
                {
                    nt = n;
                }
dirty_jump_in1:
                xx = map_pos_array[n][MAP_POS_X];
                yy = map_pos_array[n][MAP_POS_Y];
                x = op->x + xx;
                y = op->y + yy;

                msp = MSP_GET(m, x, y);

                if (!msp ||
                    pl->blocked_los[xx + x2][yy + y2] > BLOCKED_LOS_BLOCKSVIEW)
                {
                    if ((n + 1) == NROF_MAP_NODE)
                        n = -1;
                    continue;
                }

                /* we can have more as one possible target
                 * on a square - but i try this first without
                 * handle it.  */
                if (get_ob_flag)
                {
                    tmp = msp->last;
                }

                for (get_ob_flag = 1; tmp; tmp = tmp->below)
                {
                    /* this is a possible target */
                    head = (tmp->head) ? tmp->head : tmp;

                    if(valid_new_target(op, head)
                            && get_friendship(op, head) >= FRIENDSHIP_HELP
                            && head != pl->target_object)
                    {
                        pl->target_object = head;
                        pl->target_level = head->level;
                        pl->target_object_count = head->count;
                        pl->target_map_pos = n;
                        send_target_command(pl);
                        return 0;
                    }
                }
                if ((n + 1) == NROF_MAP_NODE) /* force a full loop */
                    n = -1;
            }

            if (jump_in) /* force another dirty jump ;) */
            {
                n = jump_in_n;
                jump_in = 0;
                if ((n + 1) == NROF_MAP_NODE)
                    nt = -1;
                else
                    nt = n;
                goto dirty_jump_in1;
            }
        }
    }
    // If params[0] == '2', then someone wants to target themself. This is ignored until the bottom of the code, which catches all cases.
    else if (params[0] == '3') /* talk */
    {
        /* If we have a valid target already, search from there, else start
         * afresh. */
        n = (OBJECT_VALID(pl->target_object,
                          pl->target_object_count)) ?
            pl->target_map_pos : 0;
        nt = -1;

        /* The target was invalid so make sure it is nullified. */
        if (!n)
            pl->target_object = NULL;

        for (; n < NROF_MAP_NODE && n != nt; n++)
        {
            map_t *m = op->map;
            sint16     xx = map_pos_array[n][MAP_POS_X],
                       yy = map_pos_array[n][MAP_POS_Y],
                       x = op->x + xx,
                       y = op->y + yy;
            msp_t  *msp;

            if (nt == -1)
            {
                nt = n;
            }

            xx += x2;
            yy += y2;

            msp = MSP_GET(m, x, y);

            if (!msp ||
                pl->blocked_los[xx][yy] > BLOCKED_LOS_BLOCKSVIEW)
            {
                if ((n + 1) == NROF_MAP_NODE)
                    n = -1;

                continue;
            }

            /* we can have more as one possible target
             * on a square - but i try this first without
             * handle it.
             */
            FOREACH_OBJECT_IN_MSP(tmp, msp, next)
            {
                head = (tmp->head) ? tmp->head : tmp;

                if (head->type != PLAYER && // talk is for player-mob only.
                    valid_new_target(op, head))
                {
                    /* This can happen when our old target has moved to next
                     * position. */
                    pl->target_object = head;
                    pl->target_level = head->level;
                    pl->target_object_count = head->count;
                    pl->target_map_pos = n;
                    send_target_command(pl);
                    return 0;
                }
            }

            if ((n + 1) == NROF_MAP_NODE) /* force a full loop */
                n = -1;
        }
    }

    // Haven't found a target yet, so target self.
    target_self(op);
    send_target_command(pl);
    return 0;
}

/* generate_ext_title() - get name and grap race/gender/profession from force objects */
void generate_ext_title(player_t *pl)
{
    object_t *walk,
           *next;
    char   *gender, *gmaster;
    char    prof[32]    = "";
    char    title[32]   = "";
    char    rank[32]    = "";
    char    align[32]   = "";

    /* collect all information from the force objects. Just walk one time through them*/
    FOREACH_OBJECT_IN_OBJECT(walk, pl->ob, next)
    {
        if (walk->name == shstr_cons.GUILD_FORCE &&
            walk->arch == archetype_global._guild_force)
        {
            if (walk->slaying)
                strcpy(prof, walk->slaying);
            if (walk->title)
            {
                strcpy(title, " the ");
                strcat(title, walk->title);
            }
        }
        else if (walk->name == shstr_cons.RANK_FORCE &&
                 walk->arch == archetype_global._rank_force)
        {
            if (walk->title)
            {
                strcpy(rank, walk->title);
                strcat(rank, " ");
            }
        }
        else if (walk->name == shstr_cons.ALIGNMENT_FORCE &&
                 walk->arch == archetype_global._alignment_force)
        {
            if (walk->title)
                strcpy(align, walk->title);
        }
    }

    if (QUERY_FLAG(pl->ob, FLAG_IS_MALE))
        gender = QUERY_FLAG(pl->ob, FLAG_IS_FEMALE) ? "hermaphrodite" : "male";
    else if (QUERY_FLAG(pl->ob, FLAG_IS_FEMALE))
        gender = "female";
    else
        gender = "neuter";

    if ((pl->gmaster_mode & GMASTER_MODE_SA))
    {
        gmaster = "[SA]";
    }
    else if ((pl->gmaster_mode & GMASTER_MODE_MM))
    {
        gmaster = "[MM]";
    }
    else if ((pl->gmaster_mode & GMASTER_MODE_MW))
    {
        gmaster = "[MW]";
    }
    else if ((pl->gmaster_mode & GMASTER_MODE_GM))
    {
        gmaster = "[GM]";
    }
    else if ((pl->gmaster_mode & GMASTER_MODE_VOL))
    {
        gmaster = "[VOL]";
    }
    else
    {
        gmaster = "";
    }

    sprintf(pl->quick_name, "%s%s%s", rank, pl->ob->name, gmaster);
    sprintf(pl->ext_title, "%s\n%s%s %s\n%s\n%s\n%s\n%s\n%c\n",
            rank, pl->ob->name, gmaster, title, pl->ob->race, prof, align,
            determine_god(pl->ob), *gender);
}

