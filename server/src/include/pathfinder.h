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

#ifndef PATHFINDER_H
#define PATHFINDER_H

typedef struct astar_node 
{
    struct astar_node *next;    /* Next node in linked list */
    struct astar_node *prev;    /* Previous node in linked list */
    
    struct astar_node *parent;  /* Node this was reached from */
        
    struct mapdef *map;
	sint16 x;					/* X-Position in the map for this node */
	sint16 y;					/* Y-Position in the map for this object */
    
    uint16 cost;                /* Cost of reaching this node (distance from origin) */
    float heuristic;            /* Estimated cost of reaching the goal from this node */
} path_node;

/* Psuedo-flag used to mark waypoints as "has requested path" */
/* Reuses a non-saved flag                                    */
#define FLAG_WP_PATH_REQUESTED FLAG_PARALYZED

/* Uncomment this to enable some verbose pathfinding debug messages */
/*#define DEBUG_PATHFINDING */

/* Uncomment this to enable more intelligent use of CPU time for path finding */
#define LEFTOVER_CPU_FOR_PATHFINDING    
#endif
