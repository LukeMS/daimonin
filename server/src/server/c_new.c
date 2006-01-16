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

    The author can be reached via e-mail to daimonin@nord-com.net
*/


/* NOTE: I have added the new commands as (slow) string stuff.
 * The only reason is that they are simple to debug.
 * We have at, 2 command system - this one and the stuff in commands.c.
 * I plan to rework the command system - in 2 steps (new protocol and then later UDP socket)
 * or one step - new protocol, based on UDP.
 */

/* This file deals with administrative commands from the client. */
#include <global.h>

#ifndef tolower
#define tolower(C)      (((C) >= 'A' && (C) <= 'Z')? (C) - 'A' + 'a': (C))
#endif

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

#define NROF_MAP_NODE (sizeof(map_pos_array) /(sizeof(int)*2))


static int compare_A(const void *a, const void *b)
{
    return strcmp(((CommArray_s *) a)->name, ((CommArray_s *) b)->name);
}

CommArray_s * find_command_element(char *cmd, CommArray_s *commarray, int commsize)
{
    CommArray_s    *asp, dummy;
    char           *cp;

    for (cp = cmd; *cp; cp++)
        *cp = tolower(*cp);

    dummy.name = cmd;
    asp = (CommArray_s *) bsearch((void *) &dummy, (void *) commarray, commsize, sizeof(CommArray_s), compare_A);
    return asp;
}

/* This function is called from the new client/server code.
 * pl is the player who is issuing the command, command is the
 * command.
 */
int execute_newserver_command(object *pl, char *command)
{
    CommArray_s    *csp;
    char           *cp;

    /* remove the command from the parameters */
    cp = strchr(command, ' ');
    if (cp)
    {
        *(cp++) = '\0';
        cp = cleanup_string(cp);
        if (cp && *cp == '\0')
            cp = NULL;
    }

    csp = find_plugin_command(command, pl);

    if (!csp)
        csp = find_command_element(command, Commands, CommandsSize);

    if (!csp)
        csp = find_command_element(command, CommunicationCommands, CommunicationCommandSize);

    if (!csp && QUERY_FLAG(pl, FLAG_WIZ))
        csp = find_command_element(command, WizCommands, WizCommandsSize);

    if (csp == NULL)
    {
        new_draw_info_format(NDI_UNIQUE, 0, pl, "'%s' is not a valid command.", command);
        return 0;
    }

    pl->speed_left -= csp->time;

    return csp->func(pl, cp);
}

int command_run(object *op, char *params)
{
    CONTR(op)->run_on = 1;
    return (move_player(op, params ? atoi(params) : 0));
}

int command_run_stop(object *op, char *params)
{
    CONTR(op)->run_on = 0;
    return 1;
}


void send_target_command(player *pl)
{
    int     aim_self_flag   = FALSE;
    char    tmp[256];

    if (!pl->ob->map)
        return;

    tmp[0] = BINARY_CMD_TARGET;
    tmp[1] = pl->combat_mode;
    tmp[2] = 0; /* color mode */

    pl->ob->enemy = NULL;
    pl->ob->enemy_count = 0;
    /* target still legal? */
    if (!pl->target_object || !OBJECT_ACTIVE(pl->target_object) || pl->target_object == pl->ob) /* thats we self */
        aim_self_flag = TRUE;
    else if (pl->target_object_count == pl->target_object->count)
    {
        /* ok, a last check... i put it here to have clear code:
             * perhaps we have legal issues why we can't aim or attack
             * our target anymore... invisible & stuff are handled here.
             * stuff like a out of pvp area moved player are handled different.
             * we HOLD the target - perhaps the guy moved back.
             * this special stuff is handled deeper in attack() functions.
             */
        if (QUERY_FLAG(pl->target_object, FLAG_SYS_OBJECT)
         || (QUERY_FLAG(pl->target_object, FLAG_IS_INVISIBLE) && !QUERY_FLAG(pl->ob, FLAG_SEE_INVISIBLE)))
            aim_self_flag = TRUE;
        else
        {
            if (pl->target_object->type == PLAYER || QUERY_FLAG(pl->target_object, FLAG_FRIENDLY))
                tmp[3] = 2; /* friend */
            else
            {
                tmp[3] = 1; /* enemy */
                pl->ob->enemy = pl->target_object;
                pl->ob->enemy_count = pl->target_object_count;
            }
            if (pl->target_object->name)
                strcpy(tmp + 4, pl->target_object->name);
            else
                strcpy(tmp + 4, "(null)");
        }
    }
    else
        aim_self_flag = TRUE;

    /* ok... at last, target self */
    if (aim_self_flag)
    {
        tmp[3] = 0; /* self */
        strcpy(tmp + 4, pl->ob->name);
        pl->target_object = pl->ob;
        pl->target_object_count = 0;
        pl->target_map_pos = 0;
    }

    /* now we have a target - lets calculate the color code.
     * we can do it easy and send the real level to client and
     * let calc it there but this will allow to spoil that
     * data on client side.
     */
    if (pl->target_object->level < level_color[pl->ob->level].yellow) /* target is lower */
    {
        /* if < the green border value, the mob is grey */
        if (pl->target_object->level < level_color[pl->ob->level].green) /* grey */
            tmp[2] = NDI_GREY;
        else /* calc green or blue */
        {
            if (pl->target_object->level < level_color[pl->ob->level].blue)
                tmp[2] = NDI_GREEN;
            else
                tmp[2] = NDI_BLUE;
        }
    }
    else /* target is higher or as yellow min. range */
    {
        if (pl->target_object->level >= level_color[pl->ob->level].purple)
            tmp[2] = NDI_PURPLE;
        else if (pl->target_object->level >= level_color[pl->ob->level].red)
            tmp[2] = NDI_RED;
        else if (pl->target_object->level >= level_color[pl->ob->level].orange)
            tmp[2] = NDI_ORANGE;
        else
            tmp[2] = NDI_YELLOW;
    }

    /* some nice extra info for DM's */
    if (QUERY_FLAG(pl->ob, FLAG_WIZ))
    {
        char    buf[64];
        sprintf(buf, "(lvl %d)", pl->target_object->level);
        strcat(tmp + 4, buf);
    }
    pl->target_level = pl->target_object->level;
    Write_String_To_Socket(&pl->socket, BINARY_CMD_TARGET, tmp, strlen(tmp + 4) + 4);
}

int command_combat(object *op, char *params)
{
    if (!op || !op->map || op->type != PLAYER || !CONTR(op))
        return 1;

	CONTR(op)->damage_timer = PLAYER_HPGEN_DELAY;
    if (CONTR(op)->combat_mode)
        CONTR(op)->combat_mode = 0;
    else
    {
        CONTR(op)->combat_mode = 1;
        CONTR(op)->praying = 0;
    }
    update_pets_combat_mode(op);

    send_target_command(CONTR(op));
    return 1;
}

/* enter combat mode and attack the object in front of us - IF we are in combat
 * and have a enemy/target, skip it and stop attacking.
 */
/* TODO: at some time, we should move the target stuff to the client. but for this,
 * we need a better and smarter client information strategy - MT2003
 */
/* this function needs a rework... its a bit bulky after adding all this exceptions MT-2004 */
int command_target(object *op, char *params)
{
    mapstruct  *m;
    object     *tmp, *head;
    int         jump_in, jump_in_n, get_ob_flag;
    int         n, nt, xt, yt, block, pvp_flag = FALSE;

    if (!op || !op->map || op->type != PLAYER || !CONTR(op) || !params || params[0] == 0)
        return 1;

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

        for (n = 0; n < SIZEOFFREE; n++)
        {
            int xx, yy;

            /* thats the trick: we get  op map pos, but we have 2 offsets:
                     * the offset from the client mouse click - can be
                     * +- CONTR(op)->socket.mapx/2 - and the freearr_x/y offset for
                     * the search.
                     */
            xt = op->x + (xx = freearr_x[n] + xstart);
            yt = op->y + (yy = freearr_y[n] + ystart);

            if (xx <-(int) (CONTR(op)->socket.mapx_2)
             || xx>(int)(CONTR(op)->socket.mapx_2)
             || yy <-(int) (CONTR(op)->socket.mapy_2)
             || yy>(int)(CONTR(op)->socket.mapy_2))
                continue;

            block = CONTR(op)->blocked_los[xx + CONTR(op)->socket.mapx_2][yy + CONTR(op)->socket.mapy_2];
            if (block > BLOCKED_LOS_BLOCKSVIEW || !(m = out_of_map(op->map, &xt, &yt)))
                continue;

            /* we can have more as one possible target
                     * on a square - but i try this first without
                     * handle it.
                     */
            for (tmp = get_map_ob(m, xt, yt); tmp != NULL; tmp = tmp->above)
            {
                /* this is a possible target */
                tmp->head != NULL ? (head = tmp->head) : (head = tmp); /* ensure we have head */
                if (QUERY_FLAG(head, FLAG_MONSTER) || QUERY_FLAG(head, FLAG_FRIENDLY) || head->type == PLAYER)
                {
                    /* this can happen when our old target has moved to next position */
                    if (QUERY_FLAG(head, FLAG_SYS_OBJECT)
                     || (QUERY_FLAG(head, FLAG_IS_INVISIBLE) && !QUERY_FLAG(op, FLAG_SEE_INVISIBLE)))
                        continue;
                    CONTR(op)->target_object = head;
                    CONTR(op)->target_level = head->level;
                    CONTR(op)->target_object_count = head->count;
                    CONTR(op)->target_map_pos = n;
                    goto found_target;
                }
            }
        }
    }
    else if (params[0] == '0')
    {
        /* if our target before was a non enemy, start new search
             * if it was an enemy, use old value.
             */
        n = 0;
        nt = -1;

        /* lets search for enemy object! */
        if (CONTR(op)->target_object
         && OBJECT_ACTIVE(CONTR(op)->target_object)
         && CONTR(op)->target_object_count
         == CONTR(op)->target_object->count
         && !QUERY_FLAG(CONTR(op)->target_object,
                        FLAG_FRIENDLY))
            n = CONTR(op)->target_map_pos;
        else
            CONTR(op)->target_object = NULL;

        /* now check where we are. IF we are on a PvP map or in a PvP area - then we can
            * target players on a PvP area too... TODO: group check for group PvP
            */
        if (GET_MAP_FLAGS(op->map, op->x, op->y) & P_IS_PVP || op->map->map_flags & MAP_FLAG_PVP)
            pvp_flag = TRUE;

        for (; n < NROF_MAP_NODE && n != nt; n++)
        {
            int xx, yy;
            if (nt == -1)
                nt = n;
            xt = op->x + (xx = map_pos_array[n][MAP_POS_X]);
            yt = op->y + (yy = map_pos_array[n][MAP_POS_Y]);
            block = CONTR(op)->blocked_los[xx + CONTR(op)->socket.mapx_2][yy + CONTR(op)->socket.mapy_2];
            if (block > BLOCKED_LOS_BLOCKSVIEW || !(m = out_of_map(op->map, &xt, &yt)))
            {
                if ((n + 1) == NROF_MAP_NODE)
                    n = -1;
                continue;
            }
            /* we can have more as one possible target
                 * on a square - but i try this first without
                 * handle it.
                 */
            for (tmp = get_map_ob(m, xt, yt); tmp != NULL; tmp = tmp->above)
            {
                /* this is a possible target */
                tmp->head != NULL ? (head = tmp->head) : (head = tmp); /* ensure we have head */
                if ((QUERY_FLAG(head, FLAG_MONSTER) && !QUERY_FLAG(head, FLAG_FRIENDLY))
                 || (pvp_flag
                  && (head->type == PLAYER && ((GET_MAP_FLAGS(m, xt, yt) & P_IS_PVP) || (m->map_flags & MAP_FLAG_PVP)))))
                {
                    /* this can happen when our old target has moved to next position */
                    if (head == CONTR(op)->target_object
                     || head == op
                     || QUERY_FLAG(head, FLAG_SYS_OBJECT)
                     || (QUERY_FLAG(head, FLAG_IS_INVISIBLE) && !QUERY_FLAG(op, FLAG_SEE_INVISIBLE)))
                        continue;
                    CONTR(op)->target_object = head;
                    CONTR(op)->target_level = head->level;
                    CONTR(op)->target_object_count = head->count;
                    CONTR(op)->target_map_pos = n;
                    goto found_target;
                }
            }
            if ((n + 1) == NROF_MAP_NODE) /* force a full loop */
                n = -1;
        }
    }
    else if (params[0] == '1') /* friend */
    {
        /* if /target friend but old target was enemy - target self first */
        if (CONTR(op)->target_object
         && OBJECT_ACTIVE(CONTR(op)->target_object)
         && CONTR(op)->target_object_count
         == CONTR(op)->target_object->count
         && !QUERY_FLAG(CONTR(op)->target_object,
                        FLAG_FRIENDLY))
        {
            CONTR(op)->target_object = op;
            CONTR(op)->target_level = op->level;
            CONTR(op)->target_object_count = op->count;
            CONTR(op)->target_map_pos = 0;
        }
        else /* ok - search for a friendly object now! */
        {
            /* if our target before was a non enemy, start new search
                     * if it was an enemy, use old value.
                     */
            n = 0;
            nt = -1;
            /* lets search for last friendly object position! */
            if (CONTR(op)->target_object == op)
            {
                get_ob_flag = 0;
                jump_in = 1;
                jump_in_n = n;
                tmp = op->above;
            }
            else if (OBJECT_VALID(CONTR(op)->target_object, CONTR(op)->target_object_count)
                  && (QUERY_FLAG(CONTR(op)->target_object, FLAG_FRIENDLY) || CONTR(op)->target_object->type == PLAYER))
            {
                get_ob_flag = 0;
                jump_in = 1;
                n = CONTR(op)->target_map_pos;
                jump_in_n = n;
                tmp = CONTR(op)->target_object->above;
            }
            else
            {
                n = 1;
                CONTR(op)->target_object = NULL;
                jump_in = 0;
                get_ob_flag = 1;
            }

            /* now check where we are. IF we are on a PvP map or in a PvP area - then we can
                     * target players on a PvP area too... TODO: group check for group PvP
                     */
            if (GET_MAP_FLAGS(op->map, op->x, op->y) & P_IS_PVP || op->map->map_flags & MAP_FLAG_PVP)
                pvp_flag = TRUE;


            for (; n < NROF_MAP_NODE && n != nt; n++)
            {
                int xx, yy;
                if (nt == -1)
                    nt = n;
                dirty_jump_in1 : xt = op->x + (xx = map_pos_array[n][MAP_POS_X]);
                yt = op->y + (yy = map_pos_array[n][MAP_POS_Y]);
                block = CONTR(op)->blocked_los[xx + CONTR(op)->socket.mapx_2][yy + CONTR(op)->socket.mapy_2];
                if (block > BLOCKED_LOS_BLOCKSVIEW || !(m = out_of_map(op->map, &xt, &yt)))
                {
                    if ((n + 1) == NROF_MAP_NODE)
                        n = -1;
                    continue;
                }
                /* we can have more as one possible target
                        * on a square - but i try this first without
                        * handle it.
                        */
                if (get_ob_flag)
                    tmp = get_map_ob(m, xt, yt);

                for (get_ob_flag = 1; tmp != NULL; tmp = tmp->above)
                {
                    /* this is a possible target */
                    tmp->head != NULL ? (head = tmp->head) : (head = tmp); /* ensure we have head */
                    if (QUERY_FLAG(head, FLAG_FRIENDLY)
                     || (!pvp_flag
                      && head->type == PLAYER
                      && !(GET_MAP_FLAGS(m, xt, yt) & P_IS_PVP)
                      && !(m->map_flags & MAP_FLAG_PVP)))
                    {
                        /* this can happen when our old target has moved to next position
                                             * i have no tmp == op here to allow self targeting in the friendly chain
                                             */
                        if (head == CONTR(op)->target_object
                         || QUERY_FLAG(head, FLAG_SYS_OBJECT)
                         || (QUERY_FLAG(head, FLAG_IS_INVISIBLE) && !QUERY_FLAG(op, FLAG_SEE_INVISIBLE)))
                            continue;
                        CONTR(op)->target_object = head;
                        CONTR(op)->target_level = head->level;
                        CONTR(op)->target_object_count = head->count;
                        CONTR(op)->target_map_pos = n;
                        goto found_target;
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
    else if (params[0] == '2') /* self */
    {
        CONTR(op)->target_object = op;
        CONTR(op)->target_level = op->level;
        CONTR(op)->target_object_count = op->count;
        CONTR(op)->target_map_pos = 0;
    }
    else /* TODO: ok... try to use params as a name */
    {
        /* still not sure we need this.. perhaps for groups? */
        CONTR(op)->target_object = NULL; /* dummy */
    }

    found_target : send_target_command(CONTR(op));
    return 1;
}

/* This loads the first map an puts the player on it. */
static void set_first_map(object *op)
{
    object *current;

    strcpy(CONTR(op)->maplevel, EXIT_PATH(&map_archeytpe->clone));
    op->x = map_archeytpe->clone.stats.hp;
    op->y = map_archeytpe->clone.stats.sp;

    if (!strcmp(EXIT_PATH(&map_archeytpe->clone), "/tutorial"))
    {
        current = get_object();
        FREE_AND_COPY_HASH(EXIT_PATH(current), EXIT_PATH(&map_archeytpe->clone));
        EXIT_X(current) = map_archeytpe->clone.stats.hp;
        EXIT_Y(current) = map_archeytpe->clone.stats.sp;
        current->last_eat = MAP_PLAYER_MAP;
        enter_exit(op, current);
    }
    else
    {
        enter_exit(op, NULL);
    }
}


/* we *SHOULD* grap this info from server/client setting file
 * but i have no time and code it hard here!
 */
/* min_ is also the "start" value */
typedef struct _new_char_template
{
    char                       *name;
    int                         max_p;
    int                         min_Str;
    int                         max_Str;
    int                         min_Dex;
    int                         max_Dex;
    int                         min_Con;
    int                         max_Con;
    int                         min_Int;
    int                         max_Int;
    int                         min_Wis;
    int                         max_Wis;
    int                         min_Pow;
    int                         max_Pow;
    int                         min_Cha;
    int                         max_Cha;
}_new_char_template;

static _new_char_template   new_char_template[] =
{
    {"human_male", 5, 12, 14, 12, 14, 12 ,14, 12, 14 ,12, 14 , 12, 14 ,12 ,14},
    {"human_female", 5, 12, 14, 12, 14, 12 ,14, 12, 14 ,12, 14 , 12, 14 ,12 ,14},
    {"half_elf_male", 5, 12, 14, 13, 15, 11 ,13, 12, 14 ,11, 13 , 13, 15 ,12 ,14},
    {"half_elf_female", 5, 12, 14, 13, 15, 11 ,13, 12, 14 ,11, 13 , 13, 15 ,12 ,14},
    {NULL, 5, 12, 14, 12, 14, 12 ,14, 12, 14 ,12, 14 , 12, 14 ,12 ,14}
};

/* client send us a new char creation.
 * at this point we know for *pl the name and
 * the password but nothing about his (player char)
 * base arch.
 * This command tells us which the player has selected
 * and how he has setup the stats.
 * We need to control the stats *CAREFUL*.
 * If *whatever* is not correct here - kill this socket!
 */
void command_new_char(char *params, int len, player *pl)
{
	int					skillnr[]       = {SK_SLASH_WEAP, SK_MELEE_WEAPON, SK_CLEAVE_WEAP, SK_PIERCE_WEAP};
    archetype          *p_arch          = NULL;
    const char         *name_tmp        = NULL;
    object             *objtmp, *op              = pl->ob;
    int x = pl->ob->    x, y = pl->ob->y;
    int                 stats[8], i, v;
#ifdef PLUGINS
    int                 evtid;
    CFParm              CFP;
#endif
    char                name[HUGE_BUF]  = "";
    char                buf[HUGE_BUF]   = "";
	char				*skillitem[] = {"shortsword", "mstar_small", "axe_small", "dagger_large"};

    if (CONTR(op)->state != ST_CREATE_CHAR)
    {
        LOG(llevDebug, "SHACK:: %s: command_new_char send at from time\n", query_name(pl->ob));
        pl->socket.status = Ns_Dead; /* killl socket */
        return;
    }

    if (!params || len > MAX_BUF)
    {
        pl->socket.status = Ns_Dead; /* killl socket */
        return;
    }

    sscanf(params, "%s %d %d %d %d %d %d %d %d\n", name, &stats[0], &stats[1], &stats[2], &stats[3], &stats[4], &stats[5],
           &stats[6],&stats[7]);

    /* now: we have to verify every *bit* of what the client has send us */
    /* invalid player arch? */
    if (!(p_arch = find_archetype(name)) || p_arch->clone.type != PLAYER)
    {
        pl->socket.status = Ns_Dead; /* killl socket */
        LOG(llevSystem, "SHACK: %s: invalid player arch!\n", query_name(pl->ob));
        return;
    }


    LOG(llevDebug, "NewChar: %s:: ARCH: %s (%d %d %d %d %d %d %d %d)\n", query_name(pl->ob), name, stats[0], stats[1],
        stats[2], stats[3], stats[4], stats[5], stats[6],stats[7]);


    for (i = 0; new_char_template[i].name != NULL; i++)
    {
        if (!strcmp(name, new_char_template[i].name))
            break;
    }

    if (!new_char_template[i].name)
    {
        LOG(llevDebug, "BUG:: %s: NewChar %s not in def table!\n", query_name(pl->ob), name);
        pl->socket.status = Ns_Dead; /* killl socket */
        return;
    }

    v = new_char_template[i].min_Str
      + new_char_template[i].min_Dex
      + new_char_template[i].min_Con
      + new_char_template[i].min_Int
      + new_char_template[i].min_Wis
      + new_char_template[i].min_Pow
      + new_char_template[i].min_Cha
      + new_char_template[i].max_p;

    if (v != (stats[0] + stats[1] + stats[2] + stats[3] + stats[4] + stats[5] + stats[6]) /* all boni put on the player? */
     || stats[0]
      < new_char_template[i].min_Str
     || stats[0]
      > new_char_template[i].max_Str
     || stats[1]
      < new_char_template[i].min_Dex
     || stats[1]
      > new_char_template[i].max_Dex
     || stats[2]
      < new_char_template[i].min_Con
     || stats[2]
      > new_char_template[i].max_Con
     || stats[3]
      < new_char_template[i].min_Int
     || stats[3]
      > new_char_template[i].max_Int
     || stats[4]
      < new_char_template[i].min_Wis
     || stats[4]
      > new_char_template[i].max_Wis
     || stats[5]
      < new_char_template[i].min_Pow
     || stats[5]
      > new_char_template[i].max_Pow
     || stats[6]
      < new_char_template[i].min_Cha
     || stats[6]
      > new_char_template[i].max_Cha)
    {
        LOG(llevDebug, "SHACK:: %s: tried to hack NewChar! (%d - %d)\n", query_name(pl->ob), i,
            stats[0] + stats[1] + stats[2] + stats[3] + stats[4] + stats[5] + stats[6]);
        pl->socket.status = Ns_Dead; /* killl socket */
        return;
    }
	
	if(stats[7] < 0 || stats[7] > 3) /* selected weapon skill */
	{
        LOG(llevDebug, "SHACK:: %s: tried to hack NewChar! (weapon skill %d)\n", query_name(pl->ob), stats[6]);
        pl->socket.status = Ns_Dead; /* killl socket */
        return;
	} 

    /* all is ok - now lets create this sucker */
    /* the stats of a player a saved in pl struct and copied to the object */

    FREE_AND_ADD_REF_HASH(name_tmp, op->name); /* need to copy the name to new arch */
    copy_object(&p_arch->clone, op);
    op->custom_attrset = pl;
    pl->ob = op;
    CONTR(op)->last_value = -1;
    FREE_AND_CLEAR_HASH2(op->name);
    op->name = name_tmp;
    op->x = x;
    op->y = y;
    SET_ANIMATION(op, 4 * (NUM_ANIMATIONS(op) / NUM_FACINGS(op)));     /* So player faces south */

    pl->orig_stats.Str = stats[0];
    pl->orig_stats.Dex = stats[1];
    pl->orig_stats.Con = stats[2];
    pl->orig_stats.Int = stats[3];
    pl->orig_stats.Wis = stats[4];
    pl->orig_stats.Pow = stats[5];
    pl->orig_stats.Cha = stats[6];

    SET_FLAG(op, FLAG_NO_FIX_PLAYER);
    /* this must before then initial items are given */
    esrv_new_player(CONTR(op), op->weight + op->carrying);
#ifdef PLUGINS
    /* GROS : Here we handle the BORN global event */
    evtid = EVENT_BORN;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = (void *) (op);
    GlobalEvent(&CFP);

    /* GROS : We then generate a LOGIN event */
    evtid = EVENT_LOGIN;
    CFP.Value[0] = (void *) (&evtid);
    CFP.Value[1] = (void *) (CONTR(op));
    CFP.Value[2] = (void *) (CONTR(op)->socket.ip_host);
    GlobalEvent(&CFP);
#endif

    CONTR(op)->state = ST_PLAYING;
    FREE_AND_CLEAR_HASH2(op->msg);

    /* We create this now because some of the unique maps will need it
     * to save here.
     */
    sprintf(buf, "%s/%s/%s/%s", settings.localdir, settings.playerdir, get_subdir(op->name), op->name);
    make_path_to_file(buf);

#ifdef AUTOSAVE
    CONTR(op)->last_save_tick = ROUND_TAG;
#endif

    display_motd(op);
    if (!CONTR(op)->dm_stealth)
    {
        new_draw_info_format(NDI_UNIQUE | NDI_ALL, 5, op, "%s entered the game.", op->name);

        if(gmaster_list_DM || gmaster_list_GM)
        {
            objectlink *ol;
            char buf_dm[64];

            sprintf(buf_dm,"DM: new player! %d now playing.", player_active);

            for(ol = gmaster_list_DM;ol;ol=ol->next)
                new_draw_info(NDI_UNIQUE, 0,ol->objlink.ob, buf_dm);

            for(ol = gmaster_list_GM;ol;ol=ol->next)
                new_draw_info(NDI_UNIQUE, 0,ol->objlink.ob, buf_dm);
        }
    }
    CLEAR_FLAG(op, FLAG_WIZ);
    give_initial_items(op, op->randomitems);
    link_player_skills(op);
	learn_skill(op, NULL, NULL, skillnr[stats[7]], 0);

	objtmp = get_archetype(skillitem[stats[7]]);
    objtmp = insert_ob_in_ob(objtmp, op);
    SET_FLAG(objtmp, FLAG_IDENTIFIED);
    SET_FLAG(objtmp, FLAG_KNOWN_MAGICAL);
    SET_FLAG(objtmp, FLAG_KNOWN_CURSED);
	objtmp->value = 1;

    CLEAR_FLAG(op, FLAG_NO_FIX_PLAYER);
    CONTR(op)->last_stats.exp = 1;          /* force send of skill exp data to client */
    strcpy(CONTR(op)->title, op->race);     /* no title - just what we born */
    fix_player(op);                         /* THATS our first fix_player() when we create a new char
                                                                 * add this time, hp and sp will be set
                                                                 */
    esrv_update_item(UPD_FACE, op, op);
    esrv_send_inventory(op, op);

    /* NOW we set our new char in the right map - we have a 100% right init player */
    set_first_map(op);
    SET_FLAG(op, FLAG_FRIENDLY);

    CONTR(op)->socket.update_tile = 0;
    CONTR(op)->socket.look_position = 0;
    CONTR(op)->socket.look_position_container = 0;
    CONTR(op)->socket.ext_title_flag = 1;
    esrv_new_player(CONTR(op), op->weight + op->carrying);
    send_skilllist_cmd(op, NULL, SPLIST_MODE_ADD);
    send_spelllist_cmd(op, NULL, SPLIST_MODE_ADD);
}

void command_face_request(char *params, int len, NewSocket *ns)
{
    int i, count;

    if (!params)
        return;
    count = *(uint8 *) params;

    for (i = 0; i < count; i++)
    {
        if (esrv_send_face(ns, *((short *) (params + 1) + i), 0) == SEND_FACE_OUT_OF_BOUNDS)
        {
            LOG(llevInfo, "CLIENT BUG: command_face_request (%d) out of bounds. host: %s. close connection.\n",
                *((short *) (params + 1) + i), STRING_SAFE(ns->ip_host));
            ns->status = Ns_Dead; /* killl socket */
            return;
        }
    }
}

void command_fire(char *params, int len, player *pl)
{
    int     dir = 0, type, tag1, tag2;
    object *op  = pl->ob;

    if (!params)
        return;

    CONTR(op)->fire_on = 1;

    /* i submit all this as string for testing. if stable, we change this to a short
    * and fancy binary format. MT-11-2002
    */
    sscanf(params, "%d %d %d %d", &dir, &type, &tag1, &tag2);

    if (type == FIRE_MODE_SPELL)
    {
        char   *tmp;
        tag2 = -1;
        tmp = strchr(params, ' ');
        tmp = strchr(tmp + 1, ' ');
        tmp = strchr(tmp + 1, ' ');
        if (strlen(tmp + 1) > 60)
        {
            LOG(llevDebug, "DEBUG: Player %s has send to long fire command: %s\n", query_name(pl->ob), tmp + 1);
            return;
        }
        strncpy(CONTR(op)->firemode_name, tmp + 1, 60);
        if (!fire_cast_spell(op, CONTR(op)->firemode_name))
        {
            CONTR(op)->fire_on = 0;
            CONTR(op)->firemode_type = -1; /* marks no client fire action */
            return;
        }
    }
    else if (type == FIRE_MODE_SKILL)
    {
        char   *tmp;
        tag2 = -1;
        tmp = strchr(params, ' ');
        tmp = strchr(tmp + 1, ' ');
        tmp = strchr(tmp + 1, ' ');
        if (strlen(tmp + 1) > 60)
        {
            LOG(llevDebug, "DEBUG: Player %s has send to long fire command: %s\n", query_name(pl->ob), tmp + 1);
            return;
        }
        strncpy(CONTR(op)->firemode_name, tmp + 1, 60);
    }

    CONTR(op)->firemode_type = type; /* only here will this value be set */
    CONTR(op)->firemode_tag1 = tag1;
    CONTR(op)->firemode_tag2 = tag2;

    move_player(op, dir);
    CONTR(op)->fire_on = 0;
    CONTR(op)->firemode_type = -1; /* marks no client fire action */
}

void send_spelllist_cmd(object *op, char *spellname, int mode)
{
    char    tmp[1024 * 10]; /* we should careful set a big enough buffer here */

    sprintf(tmp, "X%d ", mode);
    if (spellname) /* send single name */
    {
        strcat(tmp, "/");
        strcat(tmp, spellname);
    }
    else
    {
        int i, spnum;

        for (i = 0; i < (QUERY_FLAG(op, FLAG_WIZ) ? NROFREALSPELLS : CONTR(op)->nrofknownspells); i++)
        {
            if (QUERY_FLAG(op, FLAG_WIZ))
                spnum = i;
            else
                spnum = CONTR(op)->known_spells[i];

            strcat(tmp, "/");
            strcat(tmp, spells[spnum].name);
        }
    }
    Write_String_To_Socket(&CONTR(op)->socket, BINARY_CMD_SPELL_LIST, tmp, strlen(tmp));
}

void send_skilllist_cmd(object *op, object *skillp, int mode)
{
    object *tmp2;
    char    buf[256];
    char    tmp[1024 * 5]; /* we should careful set a big enough buffer here */

    if (skillp)
    {
        if (skillp->last_eat == 1) /* normal skills */
            sprintf(tmp, "X%d /%s|%d|%d", mode, skillp->name, skillp->level, skillp->stats.exp);
        else if (skillp->last_eat == 2) /* "buy level" skills */
            sprintf(tmp, "X%d /%s|%d|-2", mode, skillp->name, skillp->level);
        else /* no level skills */
            sprintf(tmp, "X%d /%s|%d|-1", mode, skillp->name, skillp->level);
    }
    else
    {
        sprintf(tmp, "X%d ", mode);
        for (tmp2 = op->inv; tmp2; tmp2 = tmp2->below)
        {
            if (tmp2->type == SKILL && IS_SYS_INVISIBLE(tmp2))
            {
                if (tmp2->last_eat == 1)
                    sprintf(buf, "/%s|%d|%d", tmp2->name, tmp2->level, tmp2->stats.exp);
                else if (tmp2->last_eat == 2)
                    sprintf(buf, "/%s|%d|-2", tmp2->name, tmp2->level);
                else
                    sprintf(buf, "/%s|%d|-1", tmp2->name, tmp2->level);

                strcat(tmp, buf);
            }
        }
    }
    Write_String_To_Socket(&CONTR(op)->socket, BINARY_CMD_SKILL_LIST, tmp, strlen(tmp));
}

/* all this functions are not really bulletproof. filling tmp[] can be easily produce
 * a stack overflow. Doing here some more intelligent is needed. I do this here
 * with sprintf() only for fast beta implementation */

void send_ready_skill(object *op, char *skillname)
{
    char    tmp[256]; /* we should careful set a big enough buffer here */

    sprintf(tmp, "X%s", skillname);
    Write_String_To_Socket(&CONTR(op)->socket, BINARY_CMD_SKILLRDY, tmp, strlen(tmp));
}

/* send to the client the golem face & name. Note, that this is only cosmetical
 * information to fill the range menu in the client.
 */
void send_golem_control(object *golem, int mode)
{
    char    tmp[256]; /* we should careful set a big enough buffer here */

    if (mode == GOLEM_CTR_RELEASE)
        sprintf(tmp, "X%d %d %s", mode, 0, golem->name);
    else
        sprintf(tmp, "X%d %d %s", mode, golem->face->number, golem->name);
    Write_String_To_Socket(&CONTR(golem->owner)->socket, BINARY_CMD_GOLEMCMD, tmp, strlen(tmp));
}

/* generate_ext_title() - get name and grap race/gender/profession from force objects */
void generate_ext_title(player *pl)
{
    object *walk;
    char   *gender, *tmp;
    char    prof[32]    = "";
    char    title[32]   = "";
    char    rank[32]    = "";
    char    align[32]   = "";

    /* collect all information from the force objects. Just walk one time through them*/
    for (walk = pl->ob->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name == shstr_cons.GUILD_FORCE && walk->arch->name == shstr_cons.guild_force)
        {
            if (walk->slaying)
                strcpy(prof, walk->slaying);
            if (walk->title)
            {
                strcpy(title, " the ");
                strcat(title, walk->title);
            }
        }
        else if (walk->name == shstr_cons.RANK_FORCE && walk->arch->name == shstr_cons.rank_force)
        {
            if (walk->title)
            {
                strcpy(rank, walk->title);
                strcat(rank, " ");
            }
        }
        else if (walk->name == shstr_cons.ALIGNMENT_FORCE && walk->arch->name == shstr_cons.alignment_force)
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

    /* get a possible special tag for DM/GM/VOL */
    tmp = (pl->gmaster_mode==GMASTER_MODE_NO?"":(pl->gmaster_mode==GMASTER_MODE_DM ? " [DM]" : (pl->gmaster_mode==GMASTER_MODE_GM ?" [GM]" : " [VOL]")));

    strcpy(pl->quick_name, rank);
    strcat(pl->quick_name, pl->ob->name);
    strcat(pl->quick_name, tmp);

    /*strcat(pl->quick_name, title);*/
    sprintf( pl->ext_title, "%s\n%s%s %s\n%s\n%s\n%s\n%s\n%c\n", rank, pl->ob->name,tmp,
             title, pl->ob->race, prof, align, determine_god(pl->ob), *gender);
}

