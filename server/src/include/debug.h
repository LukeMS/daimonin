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

/* Uncomment this to disable all debugging and some sanity
 * checks in core loops */
/* #define PRODUCTION_SYSTEM */

#define ESRV_DEBUG

#ifndef PRODUCTION_SYSTEM
/* Active list debugging: object.c (activelist_insert_inline()) */
/* #define DEBUG_ACTIVELIST_LOG */ /* log message when an object is added or removed from an active list */

/* very have debugging of the whole core server loop - mainly how map data ia send & stored */
/* #define DEBUG_CORE */
/* #define DEBUG_CORE_MAP*/

//#define SEND_BUFFER_DEBUG

/*#define SKILL_UTIL_DEBUG*/

/* debug the fix_xxxxxx flow */
#define DEBUG_FIX_PLAYER
/*#define DEBUG_FIX_PLAYER_SKIPPED */
#define DEBUG_FIX_MONSTER

/* force traverse loading and process of all player files inside /server/data/players */
/*#define  DEBUG_TRAVERSE_PLAYER_DIR*/

/* Aggro & EXP sharing debugging: aggro.c */
#define DEBUG_AGGRO /* Warning: ALOT debug log lines with this option - disable it for played server */

#define DEBUG_GROUP
#define DEBUG_GROUP_UPDATE

/*#define DEBUG_FRIENDSHIP_WARNING*/

/* Debug Link Spawns: spawn_point.c */
#define DEBUG_LINK_SPAWN /* log message when a linked spawn point is added, removed or called */

/* Track & log mempool object using: mempool.c */
/*#define DEBUG_MEMPOOL_OBJECT_TRACKING*/  /* enables a global list of *all* objects
                                            * we have allocated. We can browse them to
                                            * control & debug them. WARNING: Enabling this
                                            * feature will slow down the server *EXTREMLY* and should
                                            * only be done in real debug runs
                                            */

/* Track object garbage collection */
/* #define DEBUG_GC */

/* This turns the "probe" spell into a powerful charm spell as
 * an easy way to aquire pets for testing of the pets code */
/* #define DEBUG_PROBE_IS_CHARM */

/* Controls debugging of the mob behaviours and movement */
#define DEBUG_AI
/*#define DEBUG_AI_ALL */ /* some extra info - enable this for debuging */
/*#define DEBUG_AI_WAYPOINT*/
/*#define DEBUG_AI_NPC_KNOWN*/

/* Uncomment this to enable some verbose pathfinding debug messages */
/* #define DEBUG_PATHFINDING */
#endif /* ifndef PRODUCTION_SYSTEM */

#ifdef DEBUG_FIX_PLAYER
#define FIX_PLAYER(_o_, _m_) fix_player(_o_, _m_)
#else
#define FIX_PLAYER(_o_, _m_) fix_player(_o_)
#endif

#endif
