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

/** Maximum number of ticks we trust an older rangevector to a known object */
#define MAX_KNOWN_OBJ_RV_AGE 1

/** Maximum number of ticks a mob remembers an object that it can't see */
#define MAX_KNOWN_OBJ_AGE 80 /* 80 = 10 seconds */

/** A path request has been enqueued for this mob */
#define PATHFINDFLAG_PATH_REQUESTED 0
/** Last pathfinding failed */
#define PATHFINDFLAG_PATH_FAILED    1
/** Number of defined pathfinding flags */
#define NROF_PATHFIND_FLAGS         2

/** Struct for pathfinding related data. Each mob has one of these */
struct mobdata_pathfinding
{
    /** either an obj on map or a waypoint
     * (or a mob base info object) (or NULL)
     * @{ */
    object                 *target_obj;
    tag_t                   target_count;
    /** @} */

    /** target is those coords if target_obj is NULL
     * @{ */
    const char             *target_map;
    int                     target_x, target_y;
    /** @} */

    /** Original goal. Used to keep track of moving objects
     * @{ */
    const char             *goal_map;
    int                     goal_x, goal_y;
    /** @} */

    struct path_segment    *path; /**< Precomputed path from pathfinder.c   */

    uint16                  goal_delay_counter; /**< compared to wp->goal_delay */
    sint16                  best_distance; /**< to subgoal */
    sint16                  last_best_distance; /**< to subgoal */
    uint8                   tried_steps; /**< steps that didn't get us closer to subgoal */

    uint32                  flags[ (NROF_PATHFIND_FLAGS/32)+1 ];
};

/** The object is known to make use of distance attacks */
#define AI_OBJFLAG_USES_DISTANCE_ATTACK 0
/** The object is the missile or similar from a distance attack */
#define AI_OBJFLAG_IS_MISSILE 1
/** Total number of flags in the mob_known_obj struct */
#define NROF_AI_KNOWN_OBJ_FLAGS 2

/** Keeps track of other objects known to a mob
 * (enemies, friends and nearby objects). Works as a mob's short-time
 * memory about other objects.
 */
struct mob_known_obj
{
    struct mob_known_obj   *next, *prev; /** < linked list */

    /** The actual object we remember
     * @{ */
    object                 *obj;
    tag_t                   obj_count;
    /** @} */

    uint32                  last_seen;  /**< tick that this thing was last seen. Used for timeout */
    /** Last known position.
     * @{ */
    const char             *last_map;
    int                     last_x, last_y;
    /** @} */

    /** this stored rv saves some CPU at the cost of some memory, is it really worth it? */
    rv_vector               rv;
    uint32                  rv_time;  /**< Last time the rv was recalculated (or 0 for invalid) */

    /** Cumulative friendship and fear values
     * (negative friendship is enemosity and negative attraction is fear) */
    int                     friendship, attraction;
    /** Temporary values recalculated every now and then */
    int                     tmp_friendship, tmp_attraction;

    /** Knowledge bits */
    uint32                  flags[ (NROF_AI_KNOWN_OBJ_FLAGS/32)+1 ];
};

/** Convenience macros for accessing parameters in behaviour functions
 * @{ */
#define AI_PARAM_PRESENT   1 /* The parameter is present */

/** Is the param present? */
#define AIPARAM_PRESENT(param) (params[(param)].flags & AI_PARAM_PRESENT)
/** Retrieve the param integer value */
#define AIPARAM_INT(param) params[(param)].intvalue
/** Retrieve the param string value */
#define AIPARAM_STRING(param) params[(param)].stringvalue
/** @} */

/** A behaviour parameter */
struct mob_behaviour_param
{
    struct mob_behaviour_param *next;        /**< Linked list of multiple definitions*/
    const char                 *stringvalue; /**< Parameter value as (shared) string */
    long                        intvalue;    /**< Integer value */
    int                         flags;
};

/** Call info (presence and parameter list) for a behaviour */
struct mob_behaviour
{
    struct behaviour_decl      *declaration; /**< static info about this behaviour */
    struct mob_behaviour       *next;        /**< Linked list */
    struct mob_behaviour_param *parameters;  /**< Parameter array */
};

/** A unique collection of behaviours. Possibly shared by multiple mobs */
struct mob_behaviourset
{
    struct mob_behaviourset    *prev, *next; /**< Linked list */
    int                         refcount;    /**< Nr of active mobs using this def */
    const char                 *definition;  /**< The string definition (for manual behaviours) */
    uint32                      bghash;      /**< Hash for generated behaviours */

    /** one linked list of behaviours for each behaviour class */
    struct mob_behaviour       *behaviours[NROF_BEHAVIOURCLASSES];

    struct mob_behaviour_param *attitudes;  /**< Quicklink to behaviours["ATTITUDE"]->parameters */
    struct mob_behaviour_param *attractions;/**< Quicklink to behaviours["ATTRACTION"]->parameters */
    struct mob_behaviour_param *groups;     /**< Quicklink to behaviours["GROUPS"]->parameters */
};

/** The mob "mind". This contains everything the mob knows and understands */
struct mobdata
{
    struct mobdata_pathfinding  pathfinding;   /**< Pathfinding data */

    struct mob_known_obj       *known_mobs;    /**< List of recently detected mobs */
    struct mob_known_obj       *known_objs;    /**< List of recently detected objects */
    hashtable                  *known_objs_ht; /**< another view of known_objs. @note can be NULL */

    struct mob_known_obj       *owner, *enemy; /**< Important other mobs */

    struct mob_behaviourset    *behaviours;    /**< This mob's behaviours */

    object                     *spawn_info;    /**< quick pointer to spawn info (and so to its spawn point - if one) */

    /** Antilure timer */
    int antiluring_timer;

    /** Self-estimated combat strength */
    int combat_strength;

    uint8 idle_time;            /**< How long have we been standing still not doing anything */
    uint8 move_speed_factor;    /**< Wanted speed factor. 2 is normal speed. @see set_mobile_speed() */

    /** DEBUG DATA STORAGE */
    struct behaviour_decl  *last_movement_behaviour;
};

#define MOB_DATA(ob) ((struct mobdata *)((ob)->custom_attrset))
#define MOB_PATHDATA(ob) (&(((struct mobdata *)((ob)->custom_attrset))->pathfinding))

/** A few friendship delta values
 * @{ */
#define FRIENDSHIP_ENEMY_BONUS  -50 /**< Bonus to help focus current enemy */
#define FRIENDSHIP_ATTACK      -100 /**< Added if attacked */
#define FRIENDSHIP_TRY_ATTACK   -50 /**< Added if attacked but failed */
#define FRIENDSHIP_PUSH         -10 /**< Added if pushed */
#define FRIENDSHIP_NEUTRAL        0
#define FRIENDSHIP_DIST_MAX      50 /**< Max effect of distance */
#define FRIENDSHIP_HELP         100 /**< Added if helped */
#define FRIENDSHIP_PET         5000 /**< Base for pets */
/** @} */

/* Similar values for attraction/fear */
#define ATTRACTION_NEUTRAL        0
#define ATTRACTION_HOME        1000 /* Home sweet home (or pet owner) */

/** Possible movement behaviour response types */
typedef enum
{
    /** No response, let someone else decide */
    MOVE_RESPONSE_NONE,
    /** simply move in a direction, dir 0 is stand still */
    MOVE_RESPONSE_DIR,
    /** Move in any of the given directions. one is picked randomly */
    MOVE_RESPONSE_DIRS,
    /** move towards a (permanent) waypoint */
    MOVE_RESPONSE_WAYPOINT,
    /** move towards a generic coordinate */
    MOVE_RESPONSE_COORD,
    /** move towards a (possibly moving) object */
    MOVE_RESPONSE_OBJECT
}    move_response_type;

/** Data for movement behaviour responses */
typedef struct behaviour_move_response
{
    move_response_type  type; /**< @see move_response_type */
    uint16              forbidden; /**< bitmap of forbidden directions */
    void (*success_callback)(object *op, int dir); /**< callback function in case the movement turned out successful */
    union
    {
        int direction;  /**< single direction to move in */
        int directions; /**< bitmap of selected directions */

        /** Move to a target coordinate */
        struct
        {
            int         x, y;
            mapstruct  *map;
        } coord;

        /** Move towards a target object */
        struct
        {
            object *obj;
            tag_t   obj_count;
        } target;
    } data;
} move_response;

#endif
