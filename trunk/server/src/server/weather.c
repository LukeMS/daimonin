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
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

const int season_timechange[5][HOURS_PER_DAY] = {

    {   0, 0,  0,  0, 0, -1, -1,-1, -1, -1, 0, 0,
        0, 0,  0,  0,  0,  1,  1, 1, 1, 1, 0, 0 },

    {   0, 0,  0,  0, -1, -1, -1,-1, -1, 0, 0, 0,
    0, 0,  0,  0,  0,  1,  1, 1, 1, 1, 0, 0 },
    
    
    {   0, 0,  0,  -1, -1, -1, -1,-1, 0, 0, 0, 0,
        0, 0,  0,  0,  0,  1,  1, 1, 1, 1, 0, 0 },
    
    {   0, 0,  -1, -1, -1, -1, -1, 0, 0, 0, 0, 0,
        0, 0,  0,  0,  0,  0,  1, 1, 1, 1, 1, 0 },
    

    {   0, 0,  -1, -1, -1, -1, -1, 0, 0, 0, 0, 0,
        0, 0,  0,  0,  0,  0,  0, 1, 1, 1, 1, 1 }
};
 
void init_word_darkness(void)
{
    int i;
    timeofday_t tod;
    
    world_darkness=MAX_DARKNESS;
    get_tod(&tod);

    for (i = HOURS_PER_DAY/2; i < HOURS_PER_DAY; i++)
        world_darkness -=season_timechange[tod.season][i];
    for (i = 0; i <= tod.hour; i++) /* must be <= and not < ... */
        world_darkness -=season_timechange[tod.season][i];
}

/*
 * This performs the basic function of advancing the clock one tick
 * forward.  Every 20 ticks, the clock is saved to disk.  It is also
 * saved on shutdown.  Any time dependant functions should be called
 * from this function, and probably be passed tod as an argument.
 * Please don't modify tod in the dependant function.
 */

void tick_the_clock()
{
    timeofday_t tod ;

    todtick++;
    if (todtick%20 == 0)
	write_todclock();
    get_tod(&tod);
    world_darkness -=season_timechange[tod.season][tod.hour];
}
