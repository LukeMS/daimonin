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

#ifndef __MONSTER_H
#define __MONSTER_H

#include <aiconfig.h>

/* Maximum number of ticks we trust an older rangevector to a known object */
#define MAX_KNOWN_OBJ_RV_AGE 1

/* Maximum number of ticks a mob remembers an object that it can't see */
#define MAX_KNOWN_OBJ_AGE 200

/** Monster extended data **/

#define PATHFINDFLAG_PATH_REQUESTED 0 /* A path request has been
                                            enqueued for this mob */
#define PATHFINDFLAG_PATH_FAILED    1 /* Last pathfinding failed */

#define NROF_PATHFIND_FLAGS         2

struct mobdata_pathfinding
{
    object                 *target_obj;  /* either an obj on map or a waypoint */
    tag_t                   target_count; /* (or a mob base info object) (or NULL) */

    const char             *target_map;   /* target is those coords if target_obj is  */
    int                     target_x, target_y;   /* NULL */

    struct path_segment    *path; /* Precomputed path from pathfinder.c   */

    const char             *goal_map;   /* Original goal. Used to keep track of */
    int                     goal_x, goal_y;        /* moving objects */

    uint16                  goal_delay_counter; /* compared to wp->goal_delay */
    sint16                  best_distance; /* to subgoal */
    uint8                   tried_steps; /* steps that didn't get us closer to subgoal */

    uint32                  flags[ (NROF_PATHFIND_FLAGS/32)+1 ];
};

#define AI_OBJFLAG_USES_DISTANCE_ATTACK 0 /* The object is known
                                             to make use of distance
                                             attacks */
#define NROF_AI_KNOWN_OBJ_FLAGS 1

/* Keeps track of other objects known to a mob
 * (enemies, friends and nearby objects). Works as a mob's short-time
 * memory about other objects.
 */
struct mob_known_obj
{
    struct mob_known_obj   *next, *prev;
    object                 *obj;
    tag_t                   obj_count; /* Tag for object */

    uint32                  last_seen; /* tick that this thing was last seen. Used for timeout */
    const char             *last_map;   /* Last known position */
    int                     last_x, last_y;

    /* this stored rv saves some CPU at the cost of some memory, is it really worth it? */
    rv_vector               rv;    /* Stored vector to the object */
    uint32                  rv_time;   /* Last time the rv was recalculated (or 0 for invalid) */

    int                     friendship, attraction;   /* Cumulative friendship and fear values
                                                         (negative friendship is enemosity and
                                                          negative attraction is fear) */
    int                     tmp_friendship, tmp_attraction; /* Temporary values */

    uint32                  flags[ (NROF_AI_KNOWN_OBJ_FLAGS/32)+1 ];
};

/* Flags for parameters */
#define AI_PARAM_PRESENT   1 /* The parameter is present */

#define AIPARAM_PRESENT(param) (params[(param)].flags & AI_PARAM_PRESENT)
#define AIPARAM_INT(param) params[(param)].intvalue
#define AIPARAM_STRING(param) params[(param)].stringvalue

/* A behaviour parameter */
struct mob_behaviour_param
{
    struct mob_behaviour_param *next; /* Linked list of multiple definitions*/
    const char                 *stringvalue;      /* Parameter value as string */
    int                         intvalue;                 /* Integer value */
    int                         flags;
};

/* Call info for a behaviour */
struct mob_behaviour
{
    struct behaviour_decl      *declaration; /* static info about this behaviour */
    struct mob_behaviour       *next; /* Linked list */
    struct mob_behaviour_param *parameters; /* Parameter array */
};

struct mob_behaviourset
{
    struct mob_behaviourset    *prev, *next; /* Linked list */
    int                         refcount;                         /* Nr of active mobs using this def */
    const char                 *definition;               /* The string definition */
    uint32                      bghash;                        /* Hash for generated behaviours */

    struct mob_behaviour       *behaviours[NROF_BEHAVIOURCLASSES];

    struct mob_behaviour_param *attitudes;  /* Quicklink to behaviours["ATTITUDE"]->parameters */
    struct mob_behaviour_param *groups;     /* Quicklink to behaviours["GROUPS"]->parameters */
};

struct mobdata
{
    struct mobdata_pathfinding  pathfinding;

    struct mob_known_obj       *known_mobs; /* TODO optimization for search: binary heap */
    struct mob_known_obj       *known_objs; /* TODO optimization for search: binary heap */

    struct mob_known_obj       *owner, *enemy;

    struct mob_behaviourset    *behaviours;

    object                     *spawn_info; /* quick pointer to spawn info (and so to its spawn point - if one) */

    uint8 idle_time;            /* How long have we been standing still
                                   not doing anything */

    /* DEBUG DATA STORAGE */
    struct behaviour_decl  *last_movement_behaviour;
};

#define MOB_DATA(ob) ((struct mobdata *)((ob)->custom_attrset))
#define MOB_PATHDATA(ob) (&(((struct mobdata *)((ob)->custom_attrset))->pathfinding))

/* A few friendship delta values */
#define FRIENDSHIP_ENEMY_BONUS  -50 /* Bonus to help focus current enemy */
#define FRIENDSHIP_ATTACK      -100 /* Added if attacked */
#define FRIENDSHIP_TRY_ATTACK   -50 /* Added if attacked but failed */
#define FRIENDSHIP_PUSH         -10 /* Added if pushed */
#define FRIENDSHIP_NEUTRAL        0
#define FRIENDSHIP_DIST_MAX      50 /* Max effect of distance */
#define FRIENDSHIP_HELP         100 /* Added if helped */
#define FRIENDSHIP_PET         5000 /* Base for pets */

typedef enum
{
    MOVE_RESPONSE_NONE,
    /* No response, let someone else decide */
    MOVE_RESPONSE_DIR,
    /* simple move in a direction, dir 0 is stand still */
    MOVE_RESPONSE_DIRS,
    /* Move in any of the given directions. one is picked randomly */
    MOVE_RESPONSE_WAYPOINT,
    /* move towards a (permanent) waypoint */
    MOVE_RESPONSE_COORD,
    /* move towards a generic coordinate */
    MOVE_RESPONSE_OBJECT         /* move towards a (possibly moving) object */
}    move_response_type;

typedef struct behaviour_move_response
{
    move_response_type  type;
    uint16              forbidden; /* bitmap of forbidden directions */
    union
    {
        int direction;
        int directions; /* bitmap of selected directions */

        struct
        {
            int         x, y;
            mapstruct  *map;
        } coord;

        struct
        {
            object *obj;
            tag_t   obj_count;
        } target;
    } data;
} move_response;

#endif
