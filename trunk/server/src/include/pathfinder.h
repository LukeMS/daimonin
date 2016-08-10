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

#ifndef __PATHFINDER_H
#define __PATHFINDER_H

/* Uncomment this to enable more intelligent use of CPU time for path finding */
#define LEFTOVER_CPU_FOR_PATHFINDING

typedef struct astar_node
{
    struct astar_node  *next;    /* Next node in linked list */
    struct astar_node  *prev;    /* Previous node in linked list */

    struct astar_node  *parent;  /* Node this was reached from */

    struct map_t      *map;
    sint16              x;                  /* X-Position in the map for this node */
    sint16              y;                  /* Y-Position in the map for this object_t */

    uint16              cost;                /* Cost of reaching this node (distance from origin) */
    float               heuristic;            /* Estimated cost of reaching the goal from this node */
} path_node;

struct path_segment
{
    struct path_segment *next;
    const char          *map;
    sint16               x;
    sint16               y;
};

/* PATHFINDER_FREE_PATH() frees and assigns NULL to its input. */
#define PATHFINDER_FREE_PATH(__a) \
    pathfinder_free_path((__a)); \
    (__a) = NULL;

extern int                  pathfinder_queue_enqueue(object_t *waypoint);
extern object_t            *pathfinder_queue_dequeue(tag_t *count);
extern void                 request_new_path(object_t *op);
extern object_t            *get_next_requested_path(void);
extern struct path_segment *encode_path(path_node *path, struct path_segment **last_segment);
extern int                  get_path_next(const char *buf, sint16 *off, const char **mappath, map_t **map, int *x, int *y);
extern path_node           *compress_path(path_node *path);
extern float                distance_heuristic(path_node *start, path_node *current, path_node *goal, object_t *op1, object_t *op2);
extern path_node           *find_path(object_t *op, map_t *map1, int x1, int y1, map_t *map2, int x2, int y2);
extern void                 pathfinder_free_path(struct path_segment *p);

#endif /* ifndef __PATHFINDER_H */
