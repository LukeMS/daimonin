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

/*
 * Daimonin pathfinding.
 * (C) 2003 Björn Axelsson, gecko@acc.umu.se
 */

/*
 * Good pathfinding resources:
 *
 * - Amit's Thoughts on Path-Finding and A-Star
 *   http://theory.stanford.edu/~amitp/GameProgramming/
 *
 * - Game AI Resources: Pathfinding
 *   http://www.gameai.com/pathfinding.html
 *
 * - Smart Moves: Intelligent Pathfinding
 *   http://www.gamasutra.com/features/19970801/pathfinding.htm
 */

#include <global.h>
#include <pathfinder.h>

#define HEURISTIC_ERROR -666.

#ifdef DEBUG_PATHFINDING
int         searched_nodes          = 0;
#endif

/* This is the queue used for pathfinding requests from waypoints */
#define PATHFINDER_QUEUE_SIZE 100
static int  pathfinder_queue_first  = 0;
static int  pathfinder_queue_last   = 0;  /* the one after the last actually */
static struct
{
    object_t             *op; /* who asked */
    tag_t               op_count;
} pathfinder_queue[PATHFINDER_QUEUE_SIZE];

/* This is used as static memory for search tree nodes (avoids lots of mallocs) */
#define PATHFINDER_NODEBUF 300
static int          pathfinder_nodebuf_next                 = 0;
static path_node    pathfinder_nodebuf[PATHFINDER_NODEBUF];

static int FindNeighbours(path_node *node, path_node **open_list,
    path_node **closed_list, path_node *start, path_node *goal, object_t *op,
    uint32 id);

/*
 * Possible enhancements (profile and identify need before spending time on):
 *   - Replace the open list with a binary heap or skip list. Will enhance insertion performance.
 *
 * About monster-to-player paths:
 *   - use smaller max-number-of-nodes value (we don't want a mob running away across the map)
 */

/*
 *  Pathfinding scheduling functions
 */

/* enqueue a waypoint for path computation */
int pathfinder_queue_enqueue(object_t *op)
{
    /* Queue full? */
    if (pathfinder_queue_last == pathfinder_queue_first - 1
     || (pathfinder_queue_first == 0 && pathfinder_queue_last == PATHFINDER_QUEUE_SIZE - 1))
        return 0;

    pathfinder_queue[pathfinder_queue_last].op = op;
    pathfinder_queue[pathfinder_queue_last].op_count = op->count;

    if (++ pathfinder_queue_last >= PATHFINDER_QUEUE_SIZE)
        pathfinder_queue_last = 0;

    return 1;
}

/* Get the first waypoint from the queue (or NULL if empty) */
object_t * pathfinder_queue_dequeue(tag_t *count)
{
    object_t *op;

    /* Queue empty? */
    if (pathfinder_queue_last == pathfinder_queue_first)
        return NULL;

    op = pathfinder_queue[pathfinder_queue_first].op;
    *count = pathfinder_queue[pathfinder_queue_first].op_count;

    if (++ pathfinder_queue_first >= PATHFINDER_QUEUE_SIZE)
        pathfinder_queue_first = 0;

    return op;
}

/* Request a new path */
void request_new_path(object_t *op)
{
    if (op == NULL || op->type != MONSTER || MOB_DATA(op) == NULL ||
            QUERY_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_REQUESTED))
        return;

#ifdef DEBUG_PATHFINDING
    LOG(llevDebug, "request_new_path(): enqueing path request for >%s< -> >%s<\n", STRING_OBJ_NAME(op),
        STRING_OBJ_NAME(MOB_PATHDATA(op)->target_ob));
#endif

    if (pathfinder_queue_enqueue(op))
    {
        SET_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_REQUESTED);
        CLEAR_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_FAILED);
    }
}

/* Get the next (valid) mob that have requested a path is requested */
object_t * get_next_requested_path()
{
    object_t *op;
    tag_t   count;

    /* Find next still valid request */
    do
    {
        op = pathfinder_queue_dequeue(&count);
        if (op == NULL)
            return NULL;
    }
    while (!OBJECT_VALID(op, count) || !MOB_DATA(op));

#ifdef DEBUG_PATHFINDING
    LOG(llevDebug, "get_next_requested_path(): dequeued '%s' -> '%s'\n", STRING_OBJ_NAME(op),
        STRING_OBJ_NAME(MOB_PATHDATA(op)->target_ob));
#endif

    CLEAR_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_REQUESTED);

    return op;
}

/*
 * List management functions
 */

/* Allocate and initialize a node */
static path_node * make_node(map_t *map, sint16 x, sint16 y, uint16 cost, path_node *parent)
{
    path_node  *node;

    /* Out of memory? */
    if (pathfinder_nodebuf_next == PATHFINDER_NODEBUF)
    {
#ifdef DEBUG_PATHFINDING
        LOG(llevDebug, "make_node(): out of static buffer memory (this is not a problem)\n");
#endif
        return NULL;
    }

    node = &pathfinder_nodebuf[pathfinder_nodebuf_next++];

    node->next = NULL;
    node->prev = NULL;
    node->parent = parent;
    node->map = map;
    node->x = x;
    node->y = y;
    node->cost = cost;
    node->heuristic = 0.0;

    return node;
}

/* Remove a node from a list */
static void remove_node(path_node *node, path_node **list)
{
    if (node->prev)
        node->prev->next = node->next;
    else
        *list = node->next;

    if (node->next)
        node->next->prev = node->prev;

    node->next = node->prev = NULL;
}

/* Insert a node first in a list */
static void insert_node(path_node *node, path_node **list)
{
    if (*list)
        (*list)->prev = node;

    node->next = *list;
    node->prev = NULL;

    *list = node;
}

/* Insert a node in a sorted list (lowest heuristic first in list) */
static void insert_priority_node(path_node *node, path_node **list)
{
    path_node  *tmp, *last, *insert_before = NULL;

    /* TODO: make more efficient. use skip list or heaps */
    /* Find node to insert before */
    for (tmp = *list; tmp; tmp = tmp->next)
    {
        last = tmp;
        if (node->heuristic <= tmp->heuristic)
        {
            insert_before = tmp;
            break;
        }
    }

    if (insert_before == *list)
    {
        /* Insert first */
        insert_node(node, list);
    }
    else if (!insert_before)
    {
        /* insert last */
        node->next = NULL;
        node->prev = last;
        last->next = node;
    }
    else
    {
        /* insert in middle */
        node->next = insert_before;
        node->prev = insert_before->prev;
        insert_before->prev = node;
        if (node->prev)
            node->prev->next = node;
    }

    /* Print out the values of the prioqueue -> should be ordered */
    /*
    printf("post: ");
    for(tmp = *list; tmp; tmp = tmp->next)
        printf("%.3f ", tmp->heuristic);
    printf("\n");
    */
}

/*
 * Path-management functions
 */

struct path_segment * encode_path(path_node *path, struct path_segment **last_segment)
{
    struct path_segment*first =     NULL, *last = NULL, *curr;
    path_node                      *tmp;

    for (tmp = path; tmp ; tmp = tmp->next)
    {
        curr = get_poolchunk(pool_path_segment);
        curr->next = NULL;
        curr->x = tmp->x;
        curr->y = tmp->y;
        curr->map = add_refcount(tmp->map->orig_path);

        if (first == NULL)
            first = last = curr;
        else
        {
            last->next = curr;
            last = curr;
        }
    }

    if (last_segment)
        *last_segment = last;

    return first;
}

/* Compress a path by removing redundant segments.
 *
 * Current implementation removes segments that can be traversed by walking in a single direction.
 *
 * Something advanced could be to use a hughes transform / or something smart with cross products
 */
path_node *compress_path(path_node *path)
{
    path_node  *this,
               *next;
    msp_t      *this_msp,
               *next_msp;
    rv_t        rv;
    sint8       last_dir;
#ifdef DEBUG_PATHFINDING
    sint32      removed_nodes = 0,
                total_nodes = 2;
#endif

    /* Rules:
     *  - always leave first and last path nodes
     *  - if the movement direction of node n to n+1 is the same
     *    as for n-1 to n, then remove node n.
     */

    /* Guarantee at least length 3 */
    if (!path ||
        !path->next)
    {
        return path;
    }

    next = path->next;
    this_msp = MSP_KNOWN(path);
    next_msp = MSP_KNOWN(next);
    RV_GET_MSP_TO_MSP(this_msp, next_msp, &rv, RV_MANHATTAN_DISTANCE);
    last_dir = rv.direction;

    for (this = next; this && (next = this->next); this = next)
    {
#ifdef DEBUG_PATHFINDING
        total_nodes++;
#endif
        this_msp = MSP_KNOWN(this);
        next_msp = MSP_KNOWN(next);
        RV_GET_MSP_TO_MSP(this_msp, next_msp, &rv, RV_MANHATTAN_DISTANCE);

        if (last_dir == rv.direction)
        {
            remove_node(this, &path);
#ifdef DEBUG_PATHFINDING
            removed_nodes++;
#endif
        }
        else
        {
            last_dir = rv.direction;
        }
    }

#ifdef DEBUG_PATHFINDING
    LOG(llevDebug, "compress_path(): removed %d nodes of %d (%.0f%%)\n",
        removed_nodes, total_nodes, (float)removed_nodes * 100.0 / (float)total_nodes);
#endif

    return path;
}

/*
 * Actual pathfinding code
 */

/* Heuristic used for A* search. Should return an estimate of the "cost" (number of moves)
 * needed to get from current to goal. start is the initial start position and should be
 * given too.
 *
 * op1 and op2 are optional, but recommended for precise handling
 * of multipart objects.
 *
 * If this function overestimates, we are not guaranteed an optimal path.
 *
 * rv_get() can fail here if there is no maptile path between start and goal.
 * if it does, we have to return an error value (HEURISTIC_ERROR)
 */
float distance_heuristic(path_node *start, path_node *current, path_node *goal, object_t *op1, object_t *op2)
{
    msp_t *start_msp = MSP_KNOWN(start),
          *current_msp = MSP_KNOWN(current),
          *goal_msp = MSP_KNOWN(goal);
    rv_t   rv1,
           rv2;
    float  h;

    /* Diagonal distance (not manhattan distance or euclidian distance!) */
    if (!rv_get(op1, current_msp, op2, goal_msp, &rv1, RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE))
    {
        return HEURISTIC_ERROR;
    }

    /* TODO: really need to negate distances? */
    rv1.distance_x = -rv1.distance_x;
    rv1.distance_y = -rv1.distance_y;
    h = (float)rv1.distance;

    /* Add straight-line preference by calculating cross product   */
    /* (gives better performance on open areas _and_ nicer-looking paths) */
    if (!rv_get(op1, start_msp, op2, goal_msp, &rv2, RV_RECURSIVE_SEARCH | RV_NO_DISTANCE))
    {
        return HEURISTIC_ERROR;
    }

    rv2.distance_x = -rv2.distance_x;
    rv2.distance_y = -rv2.distance_y;
    h += ABS(rv1.distance_x * rv2.distance_y - rv2.distance_x * rv1.distance_y) * 0.001f;
    return h;
}

/* Find a path for op from location (x1,y1) on map1 to location (x2,y2) on map2 */
path_node * find_path(object_t *op, map_t *map1, int x1, int y1, map_t *map2, int x2, int y2)
{
    /* Closed nodes have been examined. Open are to be examined */
    path_node      *open_list, *closed_list;
    path_node      *found_path      = NULL;
    path_node       start, goal;
    static uint32   traversal_id    = 0;

    /* sanity check */
    if(!map1 || !map2)
        return NULL;

    /* Avoid overflow of traversal_id */
    if (traversal_id == 4294967295U /* UINT_MAX */)
    {
        map_t  *m;
        for (m = first_map; m != NULL; m = m->next)
            m->pathfinding_id = 0;
        traversal_id = 0;
        LOG(llevDebug, "find_path(): resetting traversal id\n");
    }

    traversal_id++;
    pathfinder_nodebuf_next = 0;
    start.x = x1;
    start.y = y1;
    start.map = map1;
    goal.x = x2;
    goal.y = y2;
    goal.map = map2;

    /* The initial tile */
    open_list = make_node(map1, (sint16) x1, (sint16) y1, 0, NULL);
    open_list->heuristic = distance_heuristic(&start, open_list, &goal, op, NULL);
    closed_list = NULL;

    if (map1->pathfinding_id != traversal_id)
    {
        map1->pathfinding_id = traversal_id;
        memset(map1->bitmap, 0, ((MAP_WIDTH(map1) + 31) / 32) * MAP_HEIGHT(map1) * sizeof(uint32));
    }

    map1->bitmap[x1 / 32 + ((MAP_WIDTH(map1) + 31) / 32) * y1] |= (1U << (x1 % 32));

    if (open_list->heuristic == HEURISTIC_ERROR)
    {
#ifdef DEBUG_PATHFINDING
        LOG(llevDebug, "find_path(): Failed to find path between targets. Aborting!\n");
#endif
        return NULL;
    }

    while (open_list != NULL /* && searched_nodes < 100 */)
    {
        /* pick best node from open_list */
        path_node  *tmp = open_list;

        /* Move node from open list to closed list */
        remove_node(tmp, &open_list);
        insert_node(tmp, &closed_list);

        /* Reached the goal? (Or at least the tile next to it?) */
        /* if(tmp->x == x2 && tmp->y == y2 && tmp->map == map2) { */
        if (tmp->heuristic <= 1.2)
        {
            path_node  *tmp2;

            /* Move all nodes used in the path to the path list */
            for (tmp2 = tmp; tmp2; tmp2 = tmp2->parent)
            {
                remove_node(tmp2, &closed_list);
                insert_node(tmp2, &found_path);
            }
            break;
        }
        else
        {
            if (!FindNeighbours(tmp, &open_list, &closed_list, &start, &goal, op, traversal_id))
                break;
        }
    }

    /* If no path was found, pick the path to the node closest to the target */
    if (found_path == NULL)
    {
        path_node  *best_node   = NULL;
        path_node  *tmp;

        /* Move best from the open list to the closed list */
        if (open_list)
        {
            tmp = open_list;
            remove_node(tmp, &open_list);
            insert_node(tmp, &closed_list);
        }

        /* Scan through the closed list for a closer tile */
        for (tmp = closed_list; tmp; tmp = tmp->next)
            if (best_node == NULL
             || tmp->heuristic < best_node->heuristic
             || (tmp->heuristic == best_node->heuristic && tmp->cost < best_node->cost))
                best_node = tmp;

        /* Create a path if we found anything */
        if (best_node)
            for (tmp = best_node; tmp; tmp = tmp->parent)
            {
                remove_node(tmp, &closed_list);
                insert_node(tmp, &found_path);
            }
    }

#ifdef DEBUG_PATHFINDING
    LOG(llevDebug, "find_path(): explored %d tiles, stored %d.\n", searched_nodes, pathfinder_nodebuf_next);
    searched_nodes = 0;

    /* This writes out the explored tiles on the source map. Useful for heuristic tweaking */
    /*
    {
        sint16 y,
               x;

        for (y = 0; y < MAP_HEIGHT(map1); y++)
        {
            for (x = 0; x < MAP_HEIGHT(map1); x++)
            {
                LOG(llevInfo, "%c", (map1->bitmap[y] & (1U << x)) ? 'X' : '-');
            }

            LOG(llevInfo, "\n");
        }
    }
    */
#endif

    return found_path;
}

/* Find untraversed neighbours of the node and add to the open_list
 *
 * Returns 0 if we ran into a limit of any kind and cannot continue,
 * or 1 if everything was ok. */
static int FindNeighbours(path_node *node, path_node **open_list,
    path_node **closed_list, path_node *start, path_node *goal, object_t *op,
    uint32 id)
{
    uint8 i;

    for (i = 1; i < 9; i++)
    {
        map_t *m = node->map;
        sint16     x = node->x + OVERLAY_X(i),
                   y = node->y + OVERLAY_Y(i);
        msp_t  *msp = MSP_GET2(m, x, y);

        if (!msp)
        {
            continue;
        }

        if (m->pathfinding_id != id ||
            !(m->bitmap[x / 32 + ((MAP_WIDTH(m) + 31) / 32) * y] & (1U << (x % 32))))
        {
            uint32 block;

            if (m->pathfinding_id != id)
            {
                m->pathfinding_id = id;
                memset(m->bitmap, 0, ((MAP_WIDTH(m) + 31) / 32) * MAP_HEIGHT(m) * sizeof(uint32));
            }

            m->bitmap[x / 32 + ((MAP_WIDTH(m) + 31) / 32) * y] |= (1U << (x % 32));
#ifdef DEBUG_PATHFINDING
            searched_nodes++;
#endif
            block = msp_blocked(op, NULL, OVERLAY_X(i), OVERLAY_Y(i));

            /* msp_blocked() returnis MSP_FLAG_DOOR_CLOSED if the msp is
             * blocked ONLY by a door which op CAN open. */
            /* TODO: increase path cost if we have to open doors? */
            if (!block ||
                block == MSP_FLAG_DOOR_CLOSED)
            {
                path_node *new_node;

                if ((new_node = make_node(m, x, y, (uint16) (node->cost + 1), node)))
                {
                    new_node->heuristic = distance_heuristic(start, new_node, goal, op, NULL);

                    if (new_node->heuristic == HEURISTIC_ERROR)
                    {
                        return 0;
                    }

                    insert_priority_node(new_node, open_list);
                }
                else
                {
                    return 0;
                }
            }

            /* TODO: might need to reopen neighbour nodes if their cost can be
             * lowered from the new node --requires pointers to the msp instead
             * of bitmap but probably only necessary if we add different path
             * costs to different terrains, or support for doors/teleporters) */
        }
    }

    return 1;
}

void free_path(struct path_segment *p)
{
    for (; p; p = p->next)
    {
        free_string_shared(p->map);
        return_poolchunk(p, pool_path_segment);
        /* assumes poolchunk is still valid */
    }
}
