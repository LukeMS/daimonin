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

#include <global.h>


/* return the name prefix of an ego item.
* Careful: return is a static char array
*/
char *get_ego_item_name(object *ob)
{
	char *cptr;
	static char name_buf[MAX_BUF];

	if(!ob || !ob->name || !(cptr = strchr(ob->name, '\'')))
		return NULL;


	strncpy(name_buf, ob->name, (int) (cptr-ob->name));
	name_buf[cptr-ob->name]=0;

	return(name_buf);
}

/* check the item is an ego item which can be applied
* return: 0 = OK, 1= EGO unbound, 2= bound wrong person, 3= bound wrong clan
*/
int check_ego_item(object *pl, object *ob)
{
	if(QUERY_FLAG(ob, FLAG_IS_EGOCLAN))
		return EGO_ITEM_BOUND_CLAN;

	if(QUERY_FLAG(ob, FLAG_IS_EGOBOUND))
	{	
		char *tmp_char = get_ego_item_name(ob);

		if(tmp_char && !strcmp(pl->name, tmp_char))
			return EGO_ITEM_BOUND_OK; /* its the right player */		

		return EGO_ITEM_BOUND_PLAYER;
	}

	if(QUERY_FLAG(ob, FLAG_IS_EGOITEM))
		return EGO_ITEM_BOUND_UNBOUND;

	return EGO_ITEM_BOUND_OK;
}

/* create an ego item by changing the name of the object
* and setting the right flags.
* mode: EGO_ITEM_BOUND_CLAN or EGO_ITEM_BOUND_PLAYER
*/
void create_ego_item(object *ob, const char *name, int mode)
{
	char buf[MAX_BUF];

	if(mode == EGO_ITEM_BOUND_CLAN)
		SET_FLAG(ob, FLAG_IS_EGOCLAN);

	SET_FLAG(ob, FLAG_IS_EGOBOUND);

	sprintf(buf, "%s's %s", name, ob->name);
	FREE_AND_COPY_HASH(ob->name, buf);
}

