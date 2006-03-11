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

    The author can be reached via e-mail to info@daimonin.net
*/

#include <global.h>

static archetype *at_guild_force = NULL;

/* get the guild object from a player */
object *guild_get(player *pl, char *name)
{
	object     *walk = pl->guild_force;

	if(!pl->ob)
		return NULL;
	
	if (walk && name) /* we have a guild force - check its our guild name */
	{
		if (!walk->slaying || strcmp(walk->slaying, name)) /* we are not in this guild - search old guild info */
		{
			for (walk = walk->inv; walk != NULL; walk = walk->below)
			{
				if (walk->slaying && !strcmp(walk->slaying, name)) /* thats the old guild info */
					break;
			}
		}
	} 

	return walk;
}

/* join a guild and return the new and/or updated guild object */
object *guild_join(player *pl, char *name, int s1_group, int s1_value, int s2_group, int s2_value, int s3_group, int s3_value)
{
	object *guild;

	if(!pl->ob)
		return NULL;
	
	if(pl->guild_force && pl->guild_force->slaying)	/* some sanity checks */
	{
		if(!strcmp(pl->guild_force->slaying, name) ) /* double join?? */
			return pl->guild_force;
		
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
			if(!at_guild_force)
				at_guild_force = find_archetype(shstr_cons.guild_force);
			pl->guild_force = insert_ob_in_ob(arch_to_object(at_guild_force), pl->ob);
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

	return guild;
}

/* leave your current guild, move the guild info in the guild object inventory and neutralize the force */
void guild_leave(player *pl)
{
	object	*old, *walk= pl->guild_force;

	if(!pl->ob)
		return;

	if(!walk || !walk->slaying) /* we can't leave where we are not in */
		return;

	for (old = walk->inv; old != NULL; old = old->below)
	{
		if (old->slaying == walk->slaying) /* thats the old guild info */
			break;
	}
	
	if(!old)
	{	
		if(!at_guild_force)
			at_guild_force = find_archetype(shstr_cons.guild_force);
		old= insert_ob_in_ob(arch_to_object(at_guild_force), walk);
	}
	
	/* we have now an old or new created guild force inside the main info */
	copy_object(walk,old);
	old->sub_type1 = ST1_GUILD_OLD;

	/* neutralize the guild and update the infos */
	FREE_AND_CLEAR_HASH(walk->slaying); /* no name, no guild */
	walk->sub_type1 = ST1_GUILD_IN; /* we are "in guild of nothing" - slaying is NULL and the tag */
	walk->last_eat = SKILLGROUP_PHYSIQUE; /* fix_player() should be called after this */
	walk->last_sp = SKILLGROUP_AGILITY;
	walk->last_heal = SKILLGROUP_WISDOM;
	walk->last_grace = walk->magic = walk->state = 100;
	pl->socket.ext_title_flag = 1;
}
