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

/*
 * Central debug control point to turn on/off special debug setting.
 * debugflags get enabled/disabled by commenting then out or not.
 */
 
#ifndef DEBUG_H
#define DEBUG_H

/* Active list debugging: object.c (activelist_insert_inline()) */
#define DEBUG_ACTIVELIST_LOG /* log message when an object is added or removed from an active list */

/* Aggro & EXP sharing debugging: aggro.c */
#define DEBUG_AGGRO /* Warning: ALOT debug log lines with this option - disable it for played server */

/* Debug Link Spawns: spawn_point.c */
#define DEBUG_LINK_SPAWN /* log message when a linked spawn point is added, removed or called */

/* Track & log mempool object using: mempool.c */
/*#define DEBUG_MEMPOOL_OBJECT_TRACKING*/  /* enables a global list of *all* objects
                                            * we have allocated. We can browse them to
                                            * control & debug them. WARNING: Enabling this
                                            * feature will slow down the server *EXTREMLY* and should
                                            * only be done in real debug runs
                                            */


#endif
