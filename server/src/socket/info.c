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

/* This file is the one and only DRAWINFO output module. All player
 * communication using drawinfo is handled here - except the few
 * messages we send to the client using DRAWINFO before we had setup
 * any player structure - thats for example when a outdated client
 * logs in and we send "update your client" direct to the info windows.
 * But if the player is loged in - all DRAWINFO are generated here.
 */
#include <global.h>
#include <sproto.h>
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
void new_draw_info(int flags,int pri, object *pl, const char *buf)
{
	char info_string[HUGE_BUF];

	if(!buf) /* should not happen - generate save string and LOG it */
	{
		buf = "[NULL]";
		LOG(llevBug,"BUG:: new_draw_info: NULL string send! %s (%x - %d)\n", query_name(pl),flags,pri);
	}

	/* here we handle global messages - still not sure i want this here */
    if (flags & NDI_ALL) {
		player	*tmppl;
		for (tmppl=first_player; tmppl!=NULL; tmppl=tmppl->next)
			new_draw_info((flags & ~NDI_ALL), pri, tmppl->ob, buf);
		return;
    }

	/* here handle some security stuff... a bit overhead for max secure */
    if (!pl || pl->type!=PLAYER)
	{
		LOG(llevBug,"BUG:: new_draw_info: called for object != PLAYER! %s (%x - %d) msg: %s\n", query_name(pl),flags,pri,buf);
		return;
	}
	if(pl->contr==NULL)
	{
		LOG(llevBug,"BUG:: new_draw_info: called for player with contr==NULL! %s (%x - %d) msg: %s\n", query_name(pl),flags,pri,buf);
		return;
	}

    if (pri>=pl->contr->listening) /* player don't want this */
		return;

	sprintf(info_string,"drawinfo %d %s", flags&NDI_COLOR_MASK, buf);
    Write_String_To_Socket(&pl->contr->socket, info_string, strlen(info_string));
}

/* This is a pretty trivial function, but it allows us to use printf style
 * formatting, so instead of the calling function having to do it, we do
 * it here.  IT may also have advantages in the future for reduction of
 * client/server bandwidth (client could keep track of various strings
 */

void new_draw_info_format(int flags, int pri,object *pl, char *format, ...)
{
    char buf[HUGE_BUF];

    va_list ap;
    va_start(ap, format);

    vsprintf(buf, format, ap);

    va_end(ap);

    new_draw_info(flags, pri, pl, buf);
}

/*
 * write to everyone on the map *except* op.  This is useful for emotions.
 */

void new_info_map_except(int color, mapstruct *map, object *op, char *str) {
    player *pl;

    for(pl = first_player; pl != NULL; pl = pl->next)
	if(pl->ob != NULL && pl->ob->map == map && pl->ob != op) {
	    new_draw_info(color, 0, pl->ob, str);
	}
}

/*
 * write to everyone on the map except op1 and op2
 */

void new_info_map_except2(int color, mapstruct *map, object *op1, object *op2,
			  char *str) {
    player *pl;

    for(pl = first_player; pl != NULL; pl = pl->next)
	if(pl->ob != NULL && pl->ob->map == map
	   && pl->ob != op1 && pl->ob != op2) {
	    new_draw_info(color, 0, pl->ob, str);
	}
}

/*
 * write to everyone on the current map
 */

void new_info_map(int color, mapstruct *map, char *str) {
    player *pl;

    for(pl = first_player; pl != NULL; pl = pl->next)
	if(pl->ob != NULL && pl->ob->map == map) {
	    new_draw_info(color, 0, pl->ob, str);
	}
}

