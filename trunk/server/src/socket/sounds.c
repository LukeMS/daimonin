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
#include <sproto.h>
#include <sounds.h>

/* This is only used for new client/server sound.  If the sound source
 * on the map is farther away than this, we don't sent it to the client.
 */
#define MAX_SOUND_DISTANCE 12

void play_sound_player_only(player *pl, int soundnum,  int soundtype, int x, int y)
{
    SockList sl;
	char buf[32];

    if (!pl->socket.sound) /* player has disabled sound */
		return;

    sl.buf=buf;
		
	SOCKET_SET_BINARY_CMD(&sl, BINARY_CMD_SOUND);
	/*
    strcpy((char*)sl.buf, "sound ");
    sl.len=strlen((char*)sl.buf);
	*/
    SockList_AddChar(&sl, (char)x);
    SockList_AddChar(&sl, (char)y);
    SockList_AddShort(&sl, (uint16)soundnum);
    SockList_AddChar(&sl, (char)soundtype);
    Send_With_Handling(&pl->socket, &sl);
}

#define POW2(x) ((x) * (x))

/* Plays some sound on map at x,y.  */
void play_sound_map(mapstruct *map, int x, int y, int sound_num, int sound_type)
{
    player *pl;

    /* TODO: a linked list of all players of one map which we can grap fast
     * from this map for uses like this 
     */

    for (pl = first_player; pl; pl = pl->next) {

	if (pl->state==ST_PLAYING && pl->ob->map == map) {
        /* ARGH... doing 2 pows for all players on this map... 
         * there must be a smarter way.  
         * We calc this in the client again, to get the volume/pan */
	    int distance=isqrt(POW2(pl->ob->x - x) + POW2(pl->ob->y - y));

	    if (distance<=MAX_SOUND_DISTANCE) {
			play_sound_player_only(pl, sound_num, sound_type, x-pl->ob->x, y-pl->ob->y);
	    }
	}
    }
}
