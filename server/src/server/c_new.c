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

/* comman_combat() toggles combat mode. */
int command_combat(object_t *op, char *params)
{
    player_t     *pl;
    objectlink_t *ol;

    if (!op ||
        op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    pl->combat_mode = !pl->combat_mode;
    pl->update_target = 1;
    pl->rest_sitting = pl->rest_mode = 0;

    /* Player's pets take their cue from their master. */
    for (ol = pl->pets; ol; ol = ol->next)
    {
        if (PET_VALID(ol, op))
        {
            SET_OR_CLEAR_FLAG(ol->objlink.ob, FLAG_UNAGGRESSIVE, !pl->combat_mode);
        }
    }

    return COMMANDS_RTN_VAL_OK;
}

/* command_target() selects the next visible target based on its parameters and
 * the player's previous target. */
/* TODO: In non-PvP areas multiple players can occupy the same msp so ideally
 * they should be targetable in sequence. But this is actually quite difficult
 * to achieve under the 0.10.z protocol. In fact, previous versions  -- despite
 * some spaghetti code -- did not manage this. This code I think will at least
 * alternate between he top 2 players.
 *
 * In fact this points to a larger problem -- that targeting should not be done
 * server-side but client-side. This would make the multiple-targets-on-one-msp
 * problem trivial and free up a fair bit of server time. But ATM the client
 * does not have enough data to do this properly and I do not want to do the
 * required client/x.Y.z update yet.
 *
 * -- Smacky 20150921 */
int command_target(object_t *op, char *params)
{
    player_t *pl;

    if (!op ||
        !params ||
        op->type != PLAYER ||
        !(pl = CONTR(op)))
    {
        return COMMANDS_RTN_VAL_ERROR;
    }

    if (params[0] == '0') // enemy
    {
        /* if old target mode was FRIEND, start loop again.. */
        if (pl->target_mode == LOS_TARGET_FRIEND)
        {
            pl->target_index = 0;
        }

        los_find_target(pl, LOS_TARGET_ENEMY, pl->target_index - 1, pl->socket.mapx * pl->socket.mapy - 1, 0, 0);
    }
    else if (params[0] == '1') // friend
    {
        /* if old target mode was ENEMY, start loop again.. */
        if (pl->target_mode == LOS_TARGET_ENEMY)
        {
            pl->target_index = 0;
        }

        los_find_target(pl, LOS_TARGET_FRIEND, pl->target_index - 1, pl->socket.mapx * pl->socket.mapy - 1, 0, 0);
    }
    else if (params[0] == '2') // self
    {
        LOS_SET_TARGET(pl, op, LOS_TARGET_SELF, 0);
    }
    else if (params[0] == '3') // talk
    {
        los_find_target(pl, LOS_TARGET_TALK, pl->target_index - 1, pl->socket.mapx * pl->socket.mapy - 1, 0, 0);
    }
    else if (params[0] == '!') // !x y = mouse map target
    {
        sint16  x,
                y;
        char   *cp;

        x = atoi(params + 1);
        
        if (!(cp = strchr(params + 1, ' ')))
        {
            LOG(llevBug, "BUG:: %s:command_target(): Bad format!\n", __FILE__);
            return COMMANDS_RTN_VAL_ERROR;
        }

        y = atoi(cp + 1);
        los_find_target(pl, LOS_TARGET_MOUSE, -1, OVERLAY_7X7 - 1, x, y);
    }
    else
    {
        LOG(llevBug, "BUG:: %s:command_target(): Unrecognized target command '%c'!\n",
            __FILE__, params[0]);
        return COMMANDS_RTN_VAL_ERROR;
    }

    pl->update_target = 1;
    return COMMANDS_RTN_VAL_OK;
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

