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
 * Daimonin pathfinding. 
 * (C) 2003 Bj�rn Axelsson, gecko@acc.umu.se
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
#ifndef __CEXTRACT__
#include <sproto.h>
#endif

#ifdef DEBUG_PATHFINDING
int searched_nodes = 0;
#endif

/* This is the queue used for pathfinding requests from waypoints */
#define PATHFINDER_QUEUE_SIZE 100
static int pathfinder_queue_first = 0;
static int pathfinder_queue_last = 0;  /* the one after the last actually */
static struct {
    object *waypoint;
    tag_t wp_count;
} pathfinder_queue[PATHFINDER_QUEUE_SIZE];

/* This is used as static memory for search tree nodes (avoids lots of mallocs) */
#define PATHFINDER_NODEBUF 300
static int pathfinder_nodebuf_next = 0;
static path_node pathfinder_nodebuf[PATHFINDER_NODEBUF];

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
int pathfinder_queue_enqueue(object *waypoint) {    
    /* Queue full? */
    if(pathfinder_queue_last == pathfinder_queue_first-1 || 
            (pathfinder_queue_first == 0 && pathfinder_queue_last == PATHFINDER_QUEUE_SIZE - 1))
        return FALSE;
        
    pathfinder_queue[pathfinder_queue_last].waypoint = waypoint;
    pathfinder_queue[pathfinder_queue_last].wp_count = waypoint->count;

    if(++ pathfinder_queue_last >= PATHFINDER_QUEUE_SIZE)
        pathfinder_queue_last = 0;

    return TRUE;
}

/* Get the first waypoint from the queue (or NULL if empty) */
object *pathfinder_queue_dequeue(int *count) {    
    object *waypoint;
    
    /* Queue empty? */
    if(pathfinder_queue_last == pathfinder_queue_first)
        return NULL;
    
    waypoint = pathfinder_queue[pathfinder_queue_first].waypoint;
    *count =  pathfinder_queue[pathfinder_queue_first].wp_count;
    
    if(++ pathfinder_queue_first >= PATHFINDER_QUEUE_SIZE)
        pathfinder_queue_first = 0;
    
    return waypoint;
}

/* Request a new path */
void request_new_path(object *waypoint)
{
    if(waypoint == NULL || QUERY_FLAG(waypoint, FLAG_WP_PATH_REQUESTED))
        return;
    
#ifdef DEBUG_PATHFINDING    
    LOG(llevDebug,"request_new_path(): enqueing path request for >%s< -> >%s<\n", waypoint->env->name, waypoint->name);
#endif    

    if(pathfinder_queue_enqueue(waypoint)) {
        SET_FLAG(waypoint, FLAG_WP_PATH_REQUESTED);
        waypoint->owner = waypoint->env;
        waypoint->ownercount = waypoint->env->count;
    }
}

/* Get the next (valid) waypoint for which a path is requested */
object *get_next_requested_path()
{
    object *waypoint;
    tag_t count;
    
    do {
        waypoint = pathfinder_queue_dequeue(&count);
        if(waypoint == NULL)
            return NULL;
        
        /* verify the waypoint and its monster */
        if(!OBJECT_VALID(waypoint, count) || !OBJECT_VALID(waypoint->owner, waypoint->ownercount) ||
                !(QUERY_FLAG(waypoint, FLAG_CURSED) || QUERY_FLAG(waypoint, FLAG_DAMNED)) ||
                (QUERY_FLAG(waypoint, FLAG_DAMNED) && !OBJECT_VALID(waypoint->enemy, waypoint->enemy_count)))
            waypoint = NULL;
    } while(waypoint == NULL);
    
#ifdef DEBUG_PATHFINDING    
    LOG(llevDebug, "get_next_requested_path(): dequeued '%s' -> '%s'\n", waypoint->owner->name, waypoint->name);
#endif

    CLEAR_FLAG(waypoint, FLAG_WP_PATH_REQUESTED);
    return waypoint;    
}   

/*
 * List management functions 
 */

/* Allocate and initialize a node */
static path_node *make_node(mapstruct *map, sint16 x, sint16 y, uint16 cost, path_node *parent)
{
    path_node *node;
    
    /* Out of memory? */
    if(pathfinder_nodebuf_next == PATHFINDER_NODEBUF) {
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
static void remove_node(path_node *node, path_node**list)
{
    if(node->prev) 
        node->prev->next = node->next;
    else
        *list = node->next;

    if(node->next) 
        node->next->prev = node->prev;

    node->next = node->prev = NULL;
}

/* Insert a node first in a list */
static void insert_node(path_node *node, path_node **list)
{
    if(*list) 
        (*list)->prev = node;

    node->next = *list;
    node->prev = NULL;
        
    *list = node;
}

/* Insert a node in a sorted list (lowest heuristic first in list) */
static void insert_priority_node(path_node *node, path_node **list)
{
    path_node *tmp, *last, *insert_before = NULL;
    
    /* TODO: make more efficient. use skip list or heaps */
    /* Find node to insert before */
    for(tmp = *list; tmp; tmp = tmp->next) {
        last = tmp;
        if(node->heuristic <= tmp->heuristic) {
            insert_before = tmp;       
            break;
        }
    }
    
    if(insert_before == *list) {  /* Insert first */
        insert_node(node, list);
    } else if (! insert_before) {  /* insert last */
        node->next = NULL;
        node->prev = last;
        last->next = node;
    } else {                   /* insert in middle */
        node->next = insert_before;
        node->prev = insert_before->prev;
        insert_before->prev = node;
        if(node->prev)
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

/* Generate a string representation of a path (returns a shared string!) 
 * 
 * Ideas on how to store paths:
 *   a) store path as real waypoint objects (might be a lot of objects...)
 *   b) store path as field in waypoints 
 *      b1) linked list in i.e. ob->enemy (needs special free() call when removing object)
 *      b2) in ascii in waypoint->msg (will even be saved out =)
 *        b2.1) direction list (e.g. 1234155532, compact but fragile)
 *        b2.2) map / coordinate list: (/dev/testmaps:13,12 14,12 ...)
 *              (human-readable (and editable), complex parsing)
 *              Approx: 600 steps in one 4096 bytes msg field
 *        b2.2.1) hex (/dev/testmaps/xxx D,C E,C ...)
 *              (harder to read and write, more compact)
 *              Approx: 1000 steps in one 4096 bytes msg field
 */
const char *encode_path(path_node *path)
{
    char buf[HUGE_BUF];
    char *bufptr = buf;
    mapstruct *last_map = NULL;
    path_node *tmp;

    /* TODO: buffer overflow checking */
    for(tmp = path; tmp ; tmp = tmp->next) {
        if(tmp->map != last_map) {
            bufptr += sprintf(bufptr, "%s%s", last_map ? "\n" : "", tmp->map->path);
            last_map = tmp->map;
        }
        bufptr += sprintf(bufptr, " %d,%d", tmp->x, tmp->y);
    }

    return add_string(buf);
}

/* Get the next location from a textual path description (generated by encode_path) starting
 * from the character index indicated by off.
 *
 * map should be initialized with whatever map the object we are working on currently lives on
 * (to handle paths without map strings)
 *
 * If a location is found, the function will return TRUE and update map, x, y and off. 
 * (off will be set to the index to use for the next call to this function)
 * Otherwise FALSE will be returned and the values of map, x and y will be undefined and off 
 * will not be touched.
 *
 * Example text path description:
 * /dev/testmaps/testmap_waypoints 17,7 17,10 18,11 19,10 20,10
 * /dev/testmaps/testmap_waypoints2 8,22
 * /dev/testmaps/testmap_waypoints3 0,22 1,23
 * /dev/testmaps/testmap_waypoints4 1,1 2,2
 */
int get_path_next(const char *buf, sint16 *off, const char **mappath, mapstruct **map, int *x, int *y)
{
    const char *coord_start = buf + *off, *coord_end, *map_def = coord_start;
 
    /* TODO: I want to get rid of this strlen call -> could store buflen in a separate waypoint field... */
    if(buf == NULL || *map == NULL || *off >= (int)strlen(buf)) {
        LOG(llevBug,"get_path_next: Illegal parameters: %s %p %d\n", buf, *map, *off);
        return FALSE;
    }
    
    /* TODO: hmm... I don't think this is necessary anymore, since we have the store path name */
    if(*mappath == NULL || **mappath == '\0') {
        /* Scan backwards from requested offset to previous linebreak or start of string */
        for(map_def = coord_start; map_def > buf && *(map_def -1) != '\n'; map_def--) 
            ;
    }
    
    /* Extract map path if any at the current position (this part is only used when we go between
     * map tiles, or when we extract the first step) */
    if(! isdigit(*map_def)) {
        char map_name[HUGE_BUF];                   /* Temporary buffer for map path extraction */
        const char *mapend = strchr(map_def, ' '); /* Find the end of the map path */

        if(mapend == NULL) {
            LOG(llevBug,"get_path_next: No delimeter after map name in path description '%s' off %d\n", buf, *off);
            return FALSE;
        }            

        strncpy(map_name, map_def, mapend - map_def);
        map_name[mapend - map_def] = '\0';

        FREE_AND_COPY_HASH(*mappath, map_name); /* store the new map path in the given shared string */
        /* Adjust coordinate pointer to point after map path */
        if(! isdigit(*coord_start))
            coord_start = mapend + 1;
    }
        
    /* Select the map we are aiming at */
    /* TODO: handle unique maps? */
    if(*mappath)
        *map = ready_map_name(*mappath, MAP_NAME_SHARED); /* We assume map name is already normalized */

    if(*map == NULL) {
        LOG(llevBug,"get_path_next: Couldn't load map from description '%s' off %d\n", buf, *off);
        return FALSE;
    }            

    /* Get the requested coordinate pair */
    coord_end = coord_start + strcspn(coord_start, " \n");    
    if(coord_end == coord_start || sscanf(coord_start, "%d,%d", x, y) != 2) {
        LOG(llevBug,"get_path_next: Illegal coordinate pair in '%s' off %d\n", buf, *off);
        return FALSE;
    }

    /* Adjust coordinates to be on the safe side */
    *map = out_of_map(*map, x, y);
    if(*map == NULL) {
        LOG(llevBug,"get_path_next: Location (%d, %d) is out of map\n", *x, *y);
        return FALSE;
    }            
    
    /* Adjust the offset */
    *off = coord_end - buf + (*coord_end ? 1 : 0);
    
    return TRUE;
}

/* Compress a path by removing redundant segments.
 *
 * Current implementation removes segments that can be traversed by walking in a single direction.
 * 
 * Something advanced could be to use a hughes transform / or something smart with cross products
 */
path_node *compress_path(path_node *path)
{
    path_node *tmp, *next;
    int last_dir;
    rv_vector v;
    
#ifdef DEBUG_PATHFINDING
    int removed_nodes = 0, total_nodes = 2;
#endif

    /* Rules: 
     *  - always leave first and last path nodes
     *  - if the movement direction of node n to n+1 is the same 
     *    as for n-1 to n, then remove node n.
     */

    /* Guarantee at least length 3 */
    if(path == NULL || path->next == NULL)
        return path;
    
    next = path->next;    
    
    get_rangevector_from_mapcoords(path->map, path->x, path->y, next->map, next->x, next->y, &v, 0); 
    last_dir = v.direction;
    
    for(tmp = next; tmp && tmp->next; tmp = next) {        
        next = tmp->next;
        
#ifdef DEBUG_PATHFINDING
        total_nodes++;
#endif
        get_rangevector_from_mapcoords(tmp->map, tmp->x, tmp->y, next->map, next->x, next->y, &v, 0); 
        if(last_dir == v.direction) {
            remove_node(tmp, &path);
#ifdef DEBUG_PATHFINDING
            removed_nodes++;
#endif
        } else 
            last_dir = v.direction;
    }

#ifdef DEBUG_PATHFINDING
    LOG(llevDebug,"compress_path(): removed %d nodes of %d (%.0f%%)\n", removed_nodes, total_nodes, (float)removed_nodes * 100.0 / (float)total_nodes);
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
 * If this function overestimates, we are not guaranteed an optimal path.
 */
float distance_heuristic(path_node *start, path_node *current, path_node *goal)
{
    rv_vector v1, v2;
    float h;
    
    /* Diagonal distance (not manhattan distance or euclidian distance!) */
    if(goal->map == current->map) {
        /* Avoid a function call in simple case */
        v1.distance_x = current->x - goal->x;
        v1.distance_y = current->y - goal->y;
        v1.distance = MAX(abs(v1.distance_x),abs(v1.distance_y));
    } else 
        get_rangevector_from_mapcoords(goal->map, goal->x, goal->y, current->map, current->x, current->y, &v1, 2|8);
    h = (float)v1.distance;
    
    /* Add straight-line preference by calculating cross product   */
    /* (gives better performance on open areas _and_ nicer-looking paths) */
    if(goal->map == start->map) {
        /* Avoid a function call in simple case */
        v2.distance_x = start->x - goal->x;
        v2.distance_y = start->y - goal->y;
    } else 
        get_rangevector_from_mapcoords(goal->map, goal->x, goal->y, start->map, start->x, start->y, &v2, 2|4|8);
    
    h += abs(v1.distance_x*v2.distance_y - v2.distance_x*v1.distance_y) * 0.001f;
    
    return h;
}

/* Find untraversed neighbours of the node and add to the open_list 
 * 
 * Returns FALSE if we ran into a limit of any kind and cannot continue,
 * or TRUE if everything was ok.
 */
int find_neighbours(path_node *node, path_node **open_list, path_node **closed_list,  
        path_node *start, path_node *goal, object *op, uint32 id)
{
    int i, x2, y2;
    mapstruct *map;
    int block;

    for(i = 1; i<9; i++) {
        x2 = node->x + freearr_x[i]; 
        y2 = node->y + freearr_y[i];

        map = out_of_map(node->map, &x2, &y2);

        if(map && !QUERY_MAP_TILE_VISITED(map, x2, y2, id)) {
            SET_MAP_TILE_VISITED(map, x2, y2, id);

#ifdef DEBUG_PATHFINDING
            searched_nodes++;
#endif    

            /* Multi-arch or not? (blocked_link_2 works for normal archs too, but is more expensive) */
            if(op->head || op->more)
                block = blocked_link_2(op, map, x2, y2);
                /* TODO: handle doors for multi-archs. Will require some modification to 
                 * blocked_link_2 i guess: (don't return as soon as we find a block: if
                 * that block is P_DOOR_CLOSED, keep searching and add P_DOOR_CLOSED to the return
                 * value.)
                 */
            else {
                block = blocked(op, map, x2, y2, op->terrain_flag);
                /* Check for possible door openening */
                /* TODO: increase path cost if we have to open doors? */
                if(block == P_DOOR_CLOSED && open_door(op, map, x2, y2, 0))
                    block = 0;
            }
            
            if(! block) {  
                path_node *new_node;
                if((new_node = make_node(map, (sint16)x2, (sint16)y2, (uint16)(node->cost + 1), node))) {
                    new_node->heuristic = distance_heuristic(start, new_node, goal);
                    insert_priority_node(new_node, open_list);
                } else
                    return FALSE;
            }

            /* TODO: might need to reopen neighbour nodes if their cost can be lowered from the new node.
             * (This requires us to store pointers to the tiles instead of bitmap.)
             * (Probably only required if we add different path costs to different terrains,
             * or support for doors/teleporters)
             */
        }
    }

    return TRUE;
}

/* Find a path for op from location (x1,y1) on map1 to location (x2,y2) on map2 */
path_node * find_path(object *op, 
        mapstruct *map1, int x1, int y1, 
        mapstruct *map2, int x2, int y2)
{
    /* Closed nodes have been examined. Open are to be examined */
    path_node *open_list, *closed_list;
    path_node *found_path = NULL;
    path_node start, goal;
    
    static uint32 traversal_id = 0;
    
    /* Avoid overflow of traversal_id */
    if(traversal_id == 4294967295U /* UINT_MAX */) {
        mapstruct *m;
        for(m = first_map; m != NULL; m=m->next) 
            m->pathfinding_id = 0;
        traversal_id = 0;
        LOG(llevDebug,"find_path(): resetting traversal id\n");
    }
    traversal_id++;

    pathfinder_nodebuf_next = 0;
    
    start.x = x1; start.y = y1; start.map = map1;
    goal.x = x2; goal.y = y2; goal.map = map2;
    
    /* The initial tile */
    open_list = make_node(map1, (sint16)x1, (sint16)y1, 0, NULL);
    open_list->heuristic = distance_heuristic(&start, open_list, &goal);
    closed_list = NULL;
    SET_MAP_TILE_VISITED(map1, x1, y1, traversal_id);
    
    while(open_list != NULL /* && searched_nodes < 100 */)  {
        /* pick best node from open_list */
        path_node *tmp = open_list;
        
        /* Move node from open list to closed list */
        remove_node(tmp, &open_list);
        insert_node(tmp, &closed_list);

        /* Reached the goal? (Or at least the tile next to it?) */
        /* if(tmp->x == x2 && tmp->y == y2 && tmp->map == map2) { */
        if(tmp->heuristic <= 1.2) {
            path_node *tmp2;

            /* Move all nodes used in the path to the path list */
            for(tmp2 = tmp; tmp2; tmp2 = tmp2->parent) {
                remove_node(tmp2, &closed_list);
                insert_node(tmp2, &found_path);
            }
            break;
        } else {
            if(! find_neighbours(tmp, &open_list, &closed_list, &start, &goal, op, traversal_id))
                break;
        }
    }

    /* If no path was found, pick the path to the node closest to the target */
    if(found_path == NULL) {
        path_node *best_node = NULL;
        path_node *tmp;
        
        /* Move best from the open list to the closed list */
        if(open_list) {
            tmp = open_list;
            remove_node(tmp, &open_list);
            insert_node(tmp, &closed_list);
        }
        
        /* Scan through the closed list for a closer tile */
        for(tmp = closed_list; tmp; tmp = tmp->next) 
            if(best_node == NULL || 
                    tmp->heuristic < best_node->heuristic || 
                    (tmp->heuristic == best_node->heuristic && tmp->cost < best_node->cost)) 
                best_node = tmp;

        /* Create a path if we found anything */
        if(best_node)
            for(tmp = best_node; tmp; tmp = tmp->parent) {
                remove_node(tmp, &closed_list);
                insert_node(tmp, &found_path);
            }
    }

#ifdef DEBUG_PATHFINDING
    LOG(llevDebug,"find_path(): explored %d tiles, stored %d.\n", searched_nodes, pathfinder_nodebuf_next);
    searched_nodes = 0;
 
    /* This writes out the explored tiles on the source map. Useful for heuristic tweaking */
    /*
    {
        int y, x;
        for(y=0; y<map1->height; y++) {
            for(x=0; x<map1->height; x++) 
                printf("%c", (map1->bitmap[y] & (1U << x)) ? 'X' : '-');
            printf("\n");
        }
    }
    */
#endif
    
    return found_path;
}
