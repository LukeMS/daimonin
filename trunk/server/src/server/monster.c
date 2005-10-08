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
 * This file contains the monster AI core and its support functions
 */

#include <global.h>

#include <aiconfig.h>

/*
 * Support functions for move_monster()
 */

/* Get a direction from object op to object target, using precomputed paths
 * if available, and request path finding if needed */
static int calc_direction_towards(object *op, object *target, mapstruct *map, int x, int y)
{
    struct mobdata_pathfinding *pf;
    mapstruct                  *path_map;
    rv_vector                   target_rv, segment_rv;

    target_rv.direction = 1234543;
    segment_rv.direction = 1234542;

    pf = MOB_PATHDATA(op);

    if (op->map == NULL)
    {
        LOG(llevBug, "BUG: calc_direction_towards(): '%s' not on a map\n", STRING_OBJ_NAME(op));
        return 0;
    }

    if (map == NULL)
    {
        LOG(llevBug, "BUG: calc_direction_towards(): invalid destination map for '%s'\n", STRING_OBJ_NAME(op));
        return 0;
    }

    /* Get general direction and distance to target */
    if(! get_rangevector_full(
            op, op->map, op->x, op->y,
            NULL, map, x, y, &target_rv,
            RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE))
    {
        LOG(llevBug, "BUG: calc_direction_towards(): unhandled rv failure '%s'\n", STRING_OBJ_NAME(op));
        /* TODO: verify results */
        /* if not on same map (or close) do something else... */
        return 0;
    }

    /* Close enough already? */
    if (target_rv.distance <= 1)
    {
        if (target_rv.distance == 0)
            return 0;
        else
            return target_rv.direction;
    }

#ifdef DEBUG_PATHFINDING
    LOG(llevDebug, "calc_direction_towards() '%s'->'%s' (distance = %d)\n",
            STRING_OBJ_NAME(op), STRING_OBJ_NAME(target), target_rv.distance);
#endif

    /* Clean up old path */
    if (pf->path)
    {
        if (pf->target_obj != target
         || (target && pf->target_count != target->count)
         || (!target && (pf->target_map != map->path || pf->target_x != x || pf->target_y != y)))
        {
            free_path(pf->path);
            pf->path = NULL;
        }
    }

    /* No precomputed path (yet) ? */
    if (pf->path == NULL)
    {
        /* request new path */
        if (!QUERY_FLAG(pf, PATHFINDFLAG_PATH_REQUESTED))
        {
            pf->target_obj = target;
            if (target)
            {
                pf->target_count = target->count;
                FREE_AND_CLEAR_HASH(pf->target_map);
            }
            else
            {
                FREE_AND_ADD_REF_HASH(pf->target_map, map->path);
                pf->target_x = x;
                pf->target_y = y;
            }

#ifdef DEBUG_PATHFINDING
            LOG(llevDebug, "calc_direction_towards() path=NULL '%s'->'%s'\n", STRING_OBJ_NAME(op),
                STRING_OBJ_NAME(target));
#endif
            request_new_path(op);
        }

        /* Take a first guesstimate step */
        return target_rv.direction;
    }

    path_map = ready_map_name(pf->path->map, MAP_NAME_SHARED);
    /* Walk towards next precomputed coordinate */
    if(! get_rangevector_full(
            op, op->map, op->x, op->y,
            NULL, path_map, pf->path->x, pf->path->y,
            &segment_rv, RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE))
    {
        LOG(llevDebug, "calc_direction_towards(): segment rv failure for '%s' @(%s:%d,%d) -> (%s:%d:%d)\n",
                STRING_OBJ_NAME(op),
                STRING_MAP_NAME(op->map), op->x, op->y,
                STRING_MAP_NAME(path_map), pf->path->x, pf->path->y);

        /* Discard invalid path. This will force a new path request later */
        free_path(pf->path);
        pf->path = NULL;

        return 0;
    }

    /* throw away segment if we are finished with it */
    if (segment_rv.distance <= 1 && pf->path != NULL)
    {
        return_poolchunk(pf->path, pool_path_segment);
        pf->path = pf->path->next; /* assuming poolchunk is still valid */
        pf->tried_steps = 0;
        pf->best_distance = -1;
    }

    if ((int) segment_rv.distance < pf->best_distance || pf->best_distance == -1)
    {
        /* If we got closer: store closest distance & reset timeout */
        pf->best_distance = segment_rv.distance;
        pf->tried_steps = 0;
    }
    else if (pf->tried_steps++ > WP_MOVE_TRIES)
    {
        /* If not got closer for a while: ask for a new path */
        pf->target_obj = target;
        if (target)
        {
            FREE_AND_CLEAR_HASH(pf->target_map);
            pf->target_count = target->count;
        }
        else
        {
            FREE_AND_ADD_REF_HASH(pf->target_map, map->path);
            pf->target_x = x;
            pf->target_y = y;
        }

        if (!QUERY_FLAG(pf, PATHFINDFLAG_PATH_REQUESTED))
        {
#ifdef DEBUG_PATHFINDING
            LOG(llevDebug, "calc_direction_towards() timeout '%s'->'%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
#endif
            request_new_path(op);
        }
    }

    return segment_rv.direction;
}

static int calc_direction_towards_coord(object *op, mapstruct *map, int x, int y)
{
    return calc_direction_towards(op, NULL, map, x, y);
}

static int calc_direction_towards_object(object *op, object *target)
{
    if(op->map == NULL) {
        LOG(llevBug, "BUG: calc_direction_towards_object(): op->map == NULL (%s <->%s)\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
        return 0;
    }

    if(target->map == NULL) {
        LOG(llevBug, "BUG: calc_direction_towards_object(): target->map == NULL (%s <->%s)\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
        return 0;
    }

    /* Request new path if target has moved too much */
    if (MOB_PATHDATA(op)->path
     && MOB_PATHDATA(op)->goal_map
     && (target->map->path != MOB_PATHDATA(op)->goal_map
      || target->x != MOB_PATHDATA(op)->goal_x
      || target->y != MOB_PATHDATA(op)->goal_y))
    {
        rv_vector   rv_goal, rv_target;
        mapstruct  *goal_map    = ready_map_name(MOB_PATHDATA(op)->goal_map, MAP_NAME_SHARED);

        if (!goal_map)
        {
            LOG(llevDebug, "BUGBUG: calc_direction_towards_object(): goal_map == NULL (%s <->%s)\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(target));
            return 0;
        }

        /* TODO if we can't see the object, goto its last known position
         * (also have to separate between well-known objects that we can find
         * without seeing, and other objects that we have to search or track */
        /* TODO make sure maps are loaded (here and everywhere else) */
        if (get_rangevector_full(
                    target, target->map, target->x, target->y,
                    NULL, goal_map, MOB_PATHDATA(op)->goal_x, MOB_PATHDATA(op)->goal_y,
                    &rv_goal, RV_DIAGONAL_DISTANCE)
                && get_rangevector_full(
                    op, op->map, op->x, op->y,
                    target, target->map, target->x, target->y,
                    &rv_target, RV_DIAGONAL_DISTANCE))
        {
            /* Heuristic: if dist(target, path goal) > dist(target, self)
             * then get a new path */
            if (rv_target.distance > 1 && rv_goal.distance * 2 > rv_target.distance)
            {
#ifdef DEBUG_PATHFINDING
                LOG(llevDebug, "calc_direction_towards_object(): %s's target '%s' has moved\n", STRING_OBJ_NAME(op),
                    STRING_OBJ_NAME(target));
#endif
                free_path(MOB_PATHDATA(op)->path);
                MOB_PATHDATA(op)->path = NULL;
            }
        }
    }

    return calc_direction_towards(op, target, target->map, target->x, target->y);
}

/* Get a direction towards the target stored in the waypoint object wp
 * tries to use precomputed path if available or request path finding if needed */
static int calc_direction_towards_waypoint(object *op, object *wp)
{
    return calc_direction_towards(op, wp, normalize_and_ready_map(op->map, &WP_MAP(wp)), WP_X(wp), WP_Y(wp));
}

int choose_direction_from_bitmap(object *op, int bitmap)
{
    int numdirs=0, dirs[9], i;

    for(i=0; i<9; i++)
    {
        if(bitmap & (1 << i))
        {
            dirs[numdirs] = i;
            numdirs++;
        }
    }

    if(numdirs == 0)
        return 0;

    return dirs[RANDOM()%(numdirs)];
}

/* Calculate a movement direction given a movement response */
static inline int direction_from_response(object *op, move_response *response)
{
    switch (response->type)
    {
        case MOVE_RESPONSE_DIR:
          return response->data.direction;
        case MOVE_RESPONSE_DIRS:
          return choose_direction_from_bitmap(op, response->data.directions);
        case MOVE_RESPONSE_OBJECT:
//          LOG(llevDebug,"dir_from_response(): '%s' -> '%s' (object; %d:%d)\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(response->data.target.obj), response->data.target.obj->x, response->data.target.obj->y);
          return calc_direction_towards_object(op, response->data.target.obj);
        case MOVE_RESPONSE_WAYPOINT:
//          LOG(llevDebug,"dir_from_response(): '%s' -> '%s' (waypoint; %d:%d)\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(response->data.target.obj), response->data.target.obj->x, response->data.target.obj->y);
          return calc_direction_towards_waypoint(op, response->data.target.obj);
        case MOVE_RESPONSE_COORD:
//          LOG(llevDebug,"dir_from_response(): '%s' -> %d:%d\n", STRING_OBJ_NAME(op), response->data.coord.x, response->data.coord.y);
          return calc_direction_towards_coord(op, response->data.coord.map, response->data.coord.x,
                                              response->data.coord.y);

        default:
          return 0;
    }
}

/* Actually move the monster in the specified direction. If there is something blocking,
 * try to go on either side of it */
static int do_move_monster(object *op, int dir, uint16 forbidden)
{
    int m;

    /* Confused monsters need a small adjustment */
    if (QUERY_FLAG(op, FLAG_CONFUSED)) {
        dir = absdir(dir + RANDOM() % 3 + RANDOM() % 3 - 2);
        forbidden = 0;
    }

    if (!(forbidden & (1 << dir)) && move_object(op, dir)) /* Can the monster move directly toward waypoint? */
        return TRUE;

    m = 1 - (RANDOM() & 2);          /* Try left or right first? */
    /* try different detours */
    if ((!(forbidden & (1 << absdir(dir + m))) && move_object(op, absdir(dir + m)))
     || (!(forbidden & (1 << absdir(dir - m))) && move_object(op, absdir(dir - m)))
     || (!(forbidden & (1 << absdir(dir + m * 2))) && move_object(op, absdir(dir + m * 2)))
     || (!(forbidden & (1 << absdir(dir - m * 2))) && move_object(op, absdir(dir - m * 2))))
        return TRUE;

    /* Couldn't move at all... */
    return FALSE;
}

/*
 * Mob maintainance functions
 */

/* Purge invalid and old mobs from list of known mobs */
static inline void remove_mob_known(struct mob_known_obj *tmp, struct mob_known_obj **first)
{
    if (tmp->next)
        tmp->next->prev = tmp->prev;

    if (tmp->prev)
        tmp->prev->next = tmp->next;
    else
        *first = tmp->next;

    return_poolchunk(tmp, pool_mob_knownobj);
}

static inline void cleanup_mob_knowns(object *op, struct mob_known_obj **first)
{
    struct mob_known_obj   *tmp;
    for (tmp = *first; tmp; tmp = tmp->next)
    {
        if (!OBJECT_VALID(tmp->obj, tmp->obj_count))
            remove_mob_known(tmp, first);
        else if(ROUND_TAG - tmp->last_seen > MAX_KNOWN_OBJ_AGE)
        {
            /* Never forget about our owner */
            if(op->owner != tmp->obj || op->owner_count != tmp->obj->count)
                remove_mob_known(tmp, first);
        }
    }
}

/* Not really AI related, but here anyway */
static inline void regenerate_stats(object *op)
{
    /*  generate hp, if applicable */
    if (op->stats.Con && op->stats.hp < op->stats.maxhp)
    {
        if (++op->last_heal > 5)
        {
            op->last_heal = 0;
            op->stats.hp += op->stats.Con;

            if (op->stats.hp > op->stats.maxhp)
                op->stats.hp = op->stats.maxhp;
        }

        /* So if the monster has gained enough HP that they are no longer afraid */
        /* TODO: should be handled elsewhere */
        if (QUERY_FLAG(op, FLAG_RUN_AWAY)
         && op->stats.hp >= (signed short) (((float) op->run_away / (float) 100) * (float) op->stats.maxhp))
            CLEAR_FLAG(op, FLAG_RUN_AWAY);
    }

    /* generate sp, if applicable */
    if (op->stats.Pow && op->stats.sp < op->stats.maxsp)
    {
        op->last_sp += (int) ((float) (8 * op->stats.Pow) / FABS(op->speed));
        op->stats.sp += op->last_sp / 128;  /* causes Pow/16 sp/tick */
        op->last_sp %= 128;
        if (op->stats.sp > op->stats.maxsp)
            op->stats.sp = op->stats.maxsp;
    }

    /* I think this is spell casting delay... */
    if (op->last_grace)
        op->last_grace--;
}

/* Placeholder function for some special processes */
void ai_fake_process(object *op, struct mob_behaviour_param *params)
{
    LOG(llevBug, "BUG: ai_fake_process() should never be called\n");
}

/*
 * Main AI function
 */

/* decide mob can move or not.
* Reasons it can't move: STAND_STILL set, paralyzed, stuck, rooted,
* mesmerized....
*/
inline int ai_obj_can_move(object *obj)
{
    if(QUERY_FLAG(obj,FLAG_STAND_STILL))
        return FALSE;
    return TRUE;
}


/* Move-monster returns 1 if the object has been freed, otherwise 0.  */
int move_monster(object *op, int mode)
{
    move_response           response;
    int                     dir, tmp_dir;
    int                     success = 0;
    struct mob_behaviour   *behaviour;
    int                     did_move = 0, did_action = 0;

    if (op == NULL || op->type != MONSTER)
    {
        LOG(llevBug, "move_monster(): Called for non-monster object '%s'\n", STRING_OBJ_NAME(op));
        return 0;
    }

    if(QUERY_FLAG(op,FLAG_PARALYZED))
        return 0;

    /* Set up mob data if missing */
    if (MOB_DATA(op) == NULL)
    {
        op->custom_attrset = get_poolchunk(pool_mob_data);
        MOB_DATA(op)->behaviours = setup_behaviours(op);
    }
    
    /* we only have a valid weapon swing - no move */
    if(mode == FALSE)
        goto jump_move_monster_action;
    
    /*
     * First, some general monster-management
     */
    tmp_dir = op->anim_enemy_dir;
    op->anim_enemy_dir = -1;      /* control the facings 25 animations */
    op->anim_moving_dir = -1;     /* the same for movement */

    /* Pets temporarily stored inside a player gets a chance to escape */
    if(op->env && op->env->type == PLAYER && QUERY_FLAG(op, FLAG_SYS_OBJECT))
    {
        if(op->owner == NULL)
            if(add_pet(op->env, op, 0))
                return 0;
        
        pet_follow_owner(op);

        /* We won't do anything with them unless they actually got out */
        if(op->map == NULL)
            return 0;
    }

    /* Purge invalid and old mobs from list of known mobs */
    cleanup_mob_knowns(op, &MOB_DATA(op)->known_mobs);
    cleanup_mob_knowns(op, &MOB_DATA(op)->known_objs);

    regenerate_stats(op); /* Regenerate if applicable */

    /*
     * Internal thought and sensing behaviours
     * All of those are always executed
     */
    for (behaviour = MOB_DATA(op)->behaviours->behaviours[BEHAVIOURCLASS_PROCESSES];
         behaviour != NULL; behaviour = behaviour->next)
    {
        /* TODO: find a slightly more efficient way of handling
         * non-executable "fake" processes */
        if(behaviour->declaration->func != ai_fake_process)
            ((void(*) (object *, struct mob_behaviour_param *)) behaviour->declaration->func) (op, behaviour->parameters);
    }

    /* Only do movement if we are actually on a map
     * (jumping out from a container should be an action) */
    if(op->map && op->env == NULL && !QUERY_FLAG(op,FLAG_ROOTED))
    {
        /*
         * Normal-priority movement behaviours. The first to return
         * a movement disables the rest
         */
        response.type = MOVE_RESPONSE_NONE; /* Clear the movement response */
        response.forbidden = 0;

        if(ai_obj_can_move(op))
        {
            for (behaviour = MOB_DATA(op)->behaviours->behaviours[BEHAVIOURCLASS_MOVES];
                behaviour != NULL;
                behaviour = behaviour->next)
            {
                ((void(*) (object *, struct mob_behaviour_param *, move_response *)) behaviour->declaration->func)
                (op, behaviour->parameters, & response);
                if (response.type != MOVE_RESPONSE_NONE)
                    break;
            }

            /* TODO move_home alternative: move_towards_friend */
            /* TODO make it possible to move _away_ from waypoint or object */

            /* Calculate direction from response needed and execute movement */
            dir = direction_from_response(op, &response);
            if (dir > 0)
            {
                success = do_move_monster(op, dir, response.forbidden);
                /* TODO: handle success=0 and precomputed paths/giving up */
            } 

            /* Try to avoid standing still if we aren't allowed to */
            if((dir == 0 || success == 0) && (response.forbidden & (1 << 0)))
            {
                success = do_move_monster(op, (RANDOM()%8)+1, response.forbidden);
            }

            if(success)
                did_move = 1;
        }
    }

    /*
     * Other mutually exclusive action commands
     * First to return TRUE disables the rest
     * TODO: some monsters can do multiple attacks? make the number of iterations here a parameter
     * TODO: either shuffle these randomly or use some sort of priority system
     * TODO: maybe separate into two parts: decision (gives an action and a priority) and
     *       execution (which done on the highest-prioritized action after all decisions are finished)
     * TODO: in original rules DEX has influence over whether to try any of these or not
     */
    jump_move_monster_action:
    for (behaviour = MOB_DATA(op)->behaviours->behaviours[BEHAVIOURCLASS_ACTIONS];
         behaviour != NULL;
         behaviour = behaviour->next)
    {
        if (((int(*) (object *, struct mob_behaviour_param *)) behaviour->declaration->func) (op, behaviour->parameters))
        {
            did_action = 1;
            break;
        }
    }

    /* Update the idle counter */
    if(did_move || did_action)
        MOB_DATA(op)->idle_time = 0;
    else
        MOB_DATA(op)->idle_time++;

    return 0;
}

/*
 * Pathfinding "callback"
 */

/* A request for path finding has been accepted and we must now find out
 *   1) where we actually wanted to go, and
 *   2) how to get there.
 */
void object_accept_path(object *op)
{
    mapstruct  *goal_map;
    int         goal_x, goal_y;
    path_node  *path;
    object     *target;
    rv_vector v;

    /* make sure we have a valid target obj or map */
    if (op->type != MONSTER
     || MOB_DATA(op) == NULL
     || (!OBJECT_VALID(MOB_PATHDATA(op)->target_obj, MOB_PATHDATA(op)->target_count) && !MOB_PATHDATA(op)->target_map)
     || !op->map)
        return;

    /* 1: Where do we want to go? */
    target = MOB_PATHDATA(op)->target_obj;

    /* Is target our real target, is it a waypoint which stores the target
     * coords? Or is our target a coordinate in the target_* values? */
    if (target == NULL)
    {
        /* Move towards a spcific coordinate */
        goal_x = MOB_PATHDATA(op)->target_x;
        goal_y = MOB_PATHDATA(op)->target_y;
        goal_map = normalize_and_ready_map(op->map, &MOB_PATHDATA(op)->target_map);
    }
    else if (target->type == TYPE_WAYPOINT_OBJECT)
    {
        /* Default map is current map */
        goal_x = WP_X(target);
        goal_y = WP_Y(target);
        goal_map = normalize_and_ready_map(op->map, &WP_MAP(target));

        FREE_AND_CLEAR_HASH(MOB_PATHDATA(op)->goal_map);
    }
    else
    {
        goal_x = target->x;
        goal_y = target->y;
        if (target->type == TYPE_BASE_INFO)
        {
            goal_map = normalize_and_ready_map(op->map, &target->slaying);
/*            LOG(llevDebug, "source: %s, map %s (%p), target %s map %s (%p)\n",
                    STRING_OBJ_NAME(op), STRING_MAP_PATH(op->map), op->map,
                    STRING_OBJ_NAME(target), STRING_MAP_PATH(goal_map), goal_map);*/
        }
        else
            goal_map = target->map;

        /* Keep track of targets that may move */
        FREE_AND_ADD_REF_HASH(MOB_PATHDATA(op)->goal_map, goal_map->path);
        MOB_PATHDATA(op)->goal_x = goal_x;
        MOB_PATHDATA(op)->goal_y = goal_y;
    }

    /* Make sure we aren't already close enough */
    get_rangevector_full(op, op->map, op->x, op->y,
            NULL, goal_map, goal_x, goal_y,
            &v, RV_DIAGONAL_DISTANCE);
    if(v.distance <= 1)
        return;

    /* 2) Do the actual pathfinding: find a path and compress it */
    path = compress_path(find_path(op, op->map, op->x, op->y, goal_map, goal_x, goal_y));

    if (path && path->next)
    {
        /* Skip the first path element (always the starting position) */
        path = path->next;

#ifdef DEBUG_PATHFINDING
        {
            path_node  *tmp;
            LOG(llevDebug, "object_accept_path(): '%s' new path -> '%s': ", STRING_OBJ_NAME(op),
                STRING_OBJ_NAME(MOB_PATHDATA(op)->target_obj));
            for (tmp = path; tmp; tmp = tmp->next)
                LOG(llevDebug, "(%d,%d) ", tmp->x, tmp->y);
            LOG(llevDebug, "\n");
        }
#endif
        /* Free any old precomputed path */
        if (MOB_PATHDATA(op)->path)
            free_path(MOB_PATHDATA(op)->path);

        /* And store the new one */
        MOB_PATHDATA(op)->path = encode_path(path, NULL);

        /* Clear counters and stuff */
        MOB_PATHDATA(op)->best_distance = -1;
        MOB_PATHDATA(op)->tried_steps = 0;
        CLEAR_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_FAILED);
    }
    else
    {
        LOG(llevDebug, "object_accept_path(): no path to destination ('%s' -> '%s')\n", STRING_OBJ_NAME(op),
            STRING_OBJ_NAME(MOB_PATHDATA(op)->target_obj));
        SET_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_FAILED);
    }
}
