/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2006 Michael Toennies

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

/* get the guild object from a player */
/* FIXME: Why is name not shstr_t *? */
object_t *guild_get(player_t *pl, char *name)
{
    if (pl->guild_force &&
        name) /* we have a guild force - check its our guild name */
    {
        object_t *walk = pl->guild_force,
               *next;

        if (!walk->slaying ||
            strcmp(walk->slaying, name)) /* we are not in this guild - search old guild info */
        {
            FOREACH_OBJECT_IN_OBJECT(walk, walk, next)
            {
                if (walk->slaying &&
                    !strcmp(walk->slaying, name)) /* thats the old guild info */
                {
                    return walk;
                }
            }
        }
    }

    return NULL;
}

/* join a guild and return the new and/or updated guild object_t */
object_t *guild_join(player_t *pl, char *name, int s1_group, int s1_value, int s2_group, int s2_value, int s3_group, int s3_value)
{
    object_t *guild;

    if(!pl->ob)
    {
        return NULL;
    }

    if(pl->guild_force && pl->guild_force->slaying) /* some sanity checks */
    {
        if(!strcmp(pl->guild_force->slaying, name)) /* double join?? */
        {
            return pl->guild_force;
        }

        guild_leave(pl); /* force a guild leave of previous guild */
    }

    if((guild = guild_get(pl, name))) /* we have an old guild, was in a guild or want/must we rejoin ? */
    {
        if(guild->sub_type1 == ST1_GUILD_OLD) /* rejoin */
        {
            copy_object(guild, pl->guild_force);
            guild = pl->guild_force;
        }
    }
    else /* our first guild or new guild - we are ALWAYS guildless on this point */
    {
        if(!pl->guild_force)
        {
            pl->guild_force = insert_ob_in_ob(arch_to_object(archetype_global._guild_force),
                                              pl->ob);
        }

        guild = pl->guild_force;
    }

    if(guild->sub_type1 != ST1_GUILD_OLD)
    {
        if(name)
        {
            FREE_AND_COPY_HASH(guild->slaying, name);
        }
        else
        {
            FREE_AND_CLEAR_HASH(guild->slaying);
        }

        guild->last_eat = s1_group;
        guild->last_sp = s2_group;
        guild->last_heal = s3_group;

        guild->last_grace = s1_value;
        guild->magic = s2_value;
        guild->state = s3_value;
    }

    guild->sub_type1 = ST1_GUILD_IN;
    pl->socket.ext_title_flag = 1;
    FIX_PLAYER(pl->ob, "guild join");
    return guild;
}

/* Leave the current guild, move the guild info in the guild
 * object inventory and neutralize the force.
 */
void guild_leave(player_t *pl)
{
    object_t *old,
           *walk = pl->guild_force,
           *next;

    if(!pl->ob)
    {
        return;
    }

    if(!walk || !walk->slaying) /* we can't leave where we are not in */
    {
        return;
    }

    FOREACH_OBJECT_IN_OBJECT(old, walk, next)
    {
        if (old->slaying == walk->slaying) /* thats the old guild info */
        {
            break;
        }
    }

    if(!old)
    {
        old = insert_ob_in_ob(arch_to_object(archetype_global._guild_force),
                              walk);
    }

    /* we have now an old or new created guild force inside the main info */
    copy_object(walk,old);
    old->sub_type1 = ST1_GUILD_OLD;

    /* neutralize the guild and update the infos */
    /* FIXME: We should just read the guild_force arch defaults here. */
    FREE_AND_CLEAR_HASH(walk->slaying); /* no name, no guild */
    walk->sub_type1 = ST1_GUILD_IN; /* we are "in guild of nothing" - slaying is NULL and the tag */
    /* As this is a default, only give 50% of the exp that you would actually
     * get if you'd really trained those skills -- see aggro.c. Guild members
     * get a better exchange rate (encourages guild membership).
     * -- Smacky 20100113 */
    walk->last_eat = SKILLGROUP_PHYSIQUE; /* fix_player() should be called after this */
    walk->last_sp = SKILLGROUP_AGILITY;
    walk->last_heal = SKILLGROUP_WISDOM;
    walk->last_grace = 25;
    walk->magic = 15;
    walk->state = 10;
    walk->level = 1;
    SET_FLAG(walk, FLAG_RUN_AWAY);
    pl->socket.ext_title_flag = 1;
    FIX_PLAYER(pl->ob, "guild leave");
}
