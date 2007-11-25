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
        LOG(llevDebug, "BUG: calc_direction_towards(): '%s' not on a map\n", STRING_OBJ_NAME(op));
        return 0;
    }

    if (map == NULL)
    {
        LOG(llevDebug, "BUG: calc_direction_towards(): invalid destination map for '%s'\n", STRING_OBJ_NAME(op));
        return 0;
    }

    /* Get general direction and distance to target */
    if(! get_rangevector_full(
            op, op->map, op->x, op->y,
            NULL, map, x, y, &target_rv,
            RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE))
    {
        LOG(llevDebug, "BUG: calc_direction_towards(): unhandled rv failure '%s'\n", STRING_OBJ_NAME(op));
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

    pf->last_best_distance = pf->best_distance;

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
        /* TODO: here we can see if an earlier pathfinding attempt failed
         * and decide whether or not it is worth trying again.
         * We need some way to see if anything changed
         * (our position, the target position etc).
         * Also (or alternatively), we should use a nice backoff algo like
         * the TCP backoff to exponentially increase the time between
         * pathfinding attempts */
        /*if(QUERY_FLAG(pf, PATHFINDFLAG_PATH_FAILED))
        {
        }*/

        if (!QUERY_FLAG(pf, PATHFINDFLAG_PATH_REQUESTED))
        {
            /* request new path */
            pf->target_obj = target;
            if (target)
            {
                pf->target_count = target->count;
                FREE_AND_CLEAR_HASH(pf->target_map);
            }
            else
            {
                FREE_AND_ADD_REF_HASH(pf->target_map, map->orig_path);
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

    path_map = ready_inherited_map(op->map, pf->path->map, 1);

    /* Walk towards next precomputed coordinate */
    if(path_map == NULL || ! get_rangevector_full(
            op, op->map, op->x, op->y,
            NULL, path_map, pf->path->x, pf->path->y,
            &segment_rv, RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE))
    {
        LOG(llevDebug, "calc_direction_towards(): segment rv failure for '%s' @(%s:%d,%d) -> (%s (%s):%d:%d)\n",
                STRING_OBJ_NAME(op),
                STRING_MAP_PATH(op->map), op->x, op->y,
                STRING_MAP_PATH(path_map), STRING_SAFE(pf->path->map), pf->path->x, pf->path->y);

        /* Discard invalid path. This will force a new path request later */
        free_path(pf->path);
        pf->path = NULL;

        return 0;
    }

    /* throw away segment if we are finished with it */
    if (segment_rv.distance <= 1 && pf->path != NULL)
    {
        free_string_shared(pf->path->map);
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
            FREE_AND_ADD_REF_HASH(pf->target_map, map->orig_path);
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
     && (target->map->orig_path != MOB_PATHDATA(op)->goal_map
      || target->x != MOB_PATHDATA(op)->goal_x
      || target->y != MOB_PATHDATA(op)->goal_y))
    {
        rv_vector   rv_goal, rv_target;
        mapstruct  *goal_map    = ready_inherited_map(op->map, MOB_PATHDATA(op)->goal_map, 1);

        /* TODO if we can't see the object, goto its last known position
         * (also have to separate between well-known objects that we can find
         * without seeing, and other objects that we have to search or track */
        /* TODO make sure maps are loaded (here and everywhere else) */

        if (!goal_map)
        {
            /* This can happen if target moves into another instance. We take it there has been a lot of movement */
            LOG(llevDebug, "calc_direction_towards_object(): goal_map == NULL (%s <->%s, op->map: %s, target map: %s)\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(target),
                STRING_MAP_PATH(op->map), STRING_SAFE(MOB_PATHDATA(op)->goal_map));

            /* Request new path */
            free_path(MOB_PATHDATA(op)->path);
            MOB_PATHDATA(op)->path = NULL;
        }
        else if (get_rangevector_full(
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
    if(WP_BEACON(wp))
    {
        object *beacon = locate_beacon(WP_BEACON(wp));
        if(beacon)
        {
            while(beacon->env)
                beacon = beacon->env;
            return calc_direction_towards(op, wp, beacon->map, beacon->x, beacon->y);
        }
        else
            return 0; /* TODO: what to do? */
    } else
    {
        mapstruct *map;

        if(WP_MAP(wp) && *WP_MAP(wp) != '\0')
        {
            map = ready_inherited_map(op->map, WP_MAP(wp), 0);
            if(map && map->orig_path != WP_MAP(wp))
                FREE_AND_ADD_REF_HASH(WP_MAP(wp), map->orig_path);
        } else
            map = op->map;
        return calc_direction_towards(op, wp, map, WP_X(wp), WP_Y(wp));
    }
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
#ifdef DEBUG_AI_WAYPOINT
          LOG(llevDebug,"dir_from_response(): '%s' -> '%s' (waypoint; %d:%d)\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(response->data.target.obj), response->data.target.obj->x, response->data.target.obj->y);
#endif
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

    /* NOTE: remember that the monster may die during move_object */
    if (!(forbidden & (1 << dir)) && move_object(op, dir)) /* Can the monster move directly toward waypoint? */
        return TRUE;

    m = (RANDOM() & 2) ? 1 : -1;          /* Try left or right first? */
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
 * Mob stats related
 */

/* Not really AI related, but here anyway */
static inline void regenerate_stats(object *op)
{
    /*  generate hp, if applicable */
    if (op->stats.Con && op->stats.hp < op->stats.maxhp)
    {
        /* only reg when mob has no ememy or hp reg in fight is set */
        if(QUERY_FLAG(op, FLAG_FIGHT_HPREG) ||
                (!op->enemy && !QUERY_FLAG(op, FLAG_NO_ATTACK) && !QUERY_FLAG(op, FLAG_SURRENDERED)))
        {
            if (++op->last_heal > 5)
            {
                op->last_heal = 0;
                op->stats.hp += op->stats.Con;

                if (op->stats.hp > op->stats.maxhp)
                    op->stats.hp = op->stats.maxhp;
            }
        }

        /* if the monster has gained enough HP that they are no longer afraid */
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

    /* Count down spell casting delay */
    if (op->last_grace)
        op->last_grace--;
}

/*
 * Waypoint utility functions
 */

/** Find a monster's currently active waypoint, if any */
object * get_active_waypoint(object *op)
{
    object *wp  = NULL;

    for (wp = op->inv; wp != NULL; wp = wp->below)
        if (wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, WP_FLAG_ACTIVE))
            break;

    return wp;
}

/** Find a monster's current return-home wp, if any */
object * get_return_waypoint(object *op)
{
    object *wp  = NULL;

    for (wp = op->inv; wp != NULL; wp = wp->below)
        if (wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, FLAG_REFLECTING))
            break;

    return wp;
}

/** Find a monster's waypoint by name (used for getting the next waypoint).
 * @param op operand mob
 * @param name must be a shared string or NULL to find any waypoint */
object * find_waypoint(object *op, const char *name)
{
    object *wp  = NULL;

    for (wp = op->inv; wp != NULL; wp = wp->below)
        if (wp->type == TYPE_WAYPOINT_OBJECT && (name == NULL || wp->name == name))
            break;

    return wp;
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
    if(obj->map == NULL || obj->env != NULL)
        return FALSE;
    if(QUERY_FLAG(obj,FLAG_STAND_STILL) || QUERY_FLAG(obj,FLAG_ROOTED))
        return FALSE;
    return TRUE;
}


/**  Move-monster returns 1 if the object has been freed, otherwise 0.  */
int move_monster(object *op, int mode)
{
    move_response           response;
    int                     dir;
    int                     success = 0;
    struct mob_behaviour   *behaviour;
    int                     did_move = 0, did_action = 0;
    int                     old_speed_factor;

    if (op == NULL || op->type != MONSTER)
    {
        LOG(llevBug, "BUG: move_monster(): Called for non-monster object '%s'\n", STRING_OBJ_NAME(op));
        if(op->type == SPAWN_POINT_MOB)
        {
            LOG(llevMapbug, "MAPBUG: move_monster(): Found Spawn Point Mob on map! Removing '%s'\n", STRING_OBJ_NAME(op));
            remove_ob(op);
        }
        return 0;
    }

    if(QUERY_FLAG(op,FLAG_PARALYZED))
        return 0;

    /* Set up mob data if missing */
    if (MOB_DATA(op) == NULL)
    {
        LOG(llevDebug, "DEBUG: move_monster(): mob '%s' without AI, is this really possible?\n", STRING_OBJ_NAME(op));
        op->custom_attrset = get_poolchunk(pool_mob_data);
        MOB_DATA(op)->behaviours = setup_behaviours(op);
    }

    old_speed_factor = MOB_DATA(op)->move_speed_factor;

    /* we only have a valid weapon swing - no move */
    if(mode == FALSE)
        goto jump_move_monster_action;

    MOB_DATA(op)->move_speed_factor = 2;

    /*
     * First, some general monster-management
     */

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
    cleanup_mob_knowns(op, &MOB_DATA(op)->known_mobs, NULL);
    cleanup_mob_knowns(op, &MOB_DATA(op)->known_objs, MOB_DATA(op)->known_objs_ht);

    regenerate_stats(op); /* Regenerate if applicable */

    /* Mark own combat strength as needing recalculation */
    MOB_DATA(op)->combat_strength = -1;

    /*
     * Internal thought and sensing behaviours
     * All of those are always executed
     */
    for (behaviour = MOB_DATA(op)->behaviours->behaviours[BEHAVIOURCLASS_PROCESSES];
         behaviour != NULL; behaviour = behaviour->next)
    {
        ((void(*) (object *, struct mob_behaviour_param *)) behaviour->declaration->func) (op, behaviour->parameters);
    }

    /* Only do movement if we are actually on a map
     * (jumping out from a container should be an action) */
    if(ai_obj_can_move(op))
    {
        /*
         * Normal-priority movement behaviours. The first to return
         * a movement disables the rest
         */
        response.type = MOVE_RESPONSE_NONE; /* Clear the movement response */
        response.forbidden = 0;
        response.success_callback = NULL;

        for (behaviour = MOB_DATA(op)->behaviours->behaviours[BEHAVIOURCLASS_MOVES];
                behaviour != NULL;
                behaviour = behaviour->next)
        {
            ((void(*) (object *, struct mob_behaviour_param *, move_response *)) behaviour->declaration->func)
                (op, behaviour->parameters, & response);
            if (response.type != MOVE_RESPONSE_NONE) {
                MOB_DATA(op)->last_movement_behaviour = behaviour->declaration;
                break;
            }
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
        if((dir == 0 || success == 0) && (response.forbidden & (1 << 0)) 
                && !QUERY_FLAG(op, FLAG_REMOVED))
        {
            success = do_move_monster(op, (RANDOM()%8)+1, response.forbidden);
        }

        /* Moving may have killed the monster */
        if(QUERY_FLAG(op, FLAG_REMOVED))
        {
            LOG(llevDebug, "move_monster(): %s (%d) died when moving.\n",  STRING_OBJ_NAME(op), op->count);
            return 0;
        }

        if(success) {
            did_move = 1;
            if(response.success_callback != NULL)
                response.success_callback(op, dir);
        }
    }

    /* Clear anim indicators if we didn't do anything.
     * This enables idle animations */
    if(!did_move)
        op->anim_moving_dir = -1;

    /*
     * Other mutually exclusive action commands
     * First to return TRUE disables the rest
     * TODO: some monsters can do multiple attacks? make the number of iterations here a parameter
     * TODO: either shuffle these randomly or use some sort of priority system
     * TODO: maybe separate into two parts: decision (gives an action and a priority) and
     *       execution (which done on the highest-prioritized action after all decisions are finished)
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
        
    /* The action may have killed the monster */
    if(QUERY_FLAG(op, FLAG_REMOVED))
    {
        LOG(llevDebug, "move_monster(): %s (%d) died when performing action.\n",  STRING_OBJ_NAME(op), op->count);
        return 0;
    }

    /* Update the idle counter */
    if(did_move || did_action)
        MOB_DATA(op)->idle_time = 0;
    else
        MOB_DATA(op)->idle_time++;

    /* Enable/disable attack animation */
    if(MOB_DATA(op)->enemy == NULL)
        op->anim_enemy_dir = -1;

    /* Change gears? */
    if(MOB_DATA(op)->move_speed_factor != old_speed_factor)
        set_mobile_speed(op, 0);

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
    object     *goal_object = NULL;
    mapstruct  *goal_map = NULL;
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
        /* Move towards a specific coordinate */
        goal_x = MOB_PATHDATA(op)->target_x;
        goal_y = MOB_PATHDATA(op)->target_y;
        goal_map = ready_inherited_map(op->map, MOB_PATHDATA(op)->target_map, 0);
    }
    else if (target->type == TYPE_WAYPOINT_OBJECT)
    {
        if(WP_BEACON(target))
            goal_object = locate_beacon(WP_BEACON(target));
        else
        {
            /* Default map is current map */
            goal_x = WP_X(target);
            goal_y = WP_Y(target);
            if(WP_MAP(target) && *WP_MAP(target) != '\0')
            {
                goal_map = ready_inherited_map(op->map, WP_MAP(target), 0);
                if(goal_map && goal_map->orig_path != WP_MAP(target))
                    FREE_AND_ADD_REF_HASH(WP_MAP(target), goal_map->orig_path);
            }
            else
                goal_map = op->map;

            FREE_AND_CLEAR_HASH(MOB_PATHDATA(op)->goal_map);
        }
    }
    else
    {
        goal_object = target;
    }

    if(goal_object)
    {
        if (goal_object->type == TYPE_BASE_INFO)
        {
            goal_map = ready_inherited_map(op->map, goal_object->slaying, 0);
/*            LOG(llevDebug, "source: %s, map %s (%p), target %s map %s (%p)\n",
                    STRING_OBJ_NAME(op), STRING_MAP_PATH(op->map), op->map,
                    STRING_OBJ_NAME(target), STRING_MAP_PATH(goal_map), goal_map);*/
        }
        else
        {
            while(goal_object->env)
                goal_object = goal_object->env;
            goal_map = goal_object->map;
        }
        goal_x = goal_object->x;
        goal_y = goal_object->y;

        if(goal_map)
        {
            /* Keep track of targets that may move */
            FREE_AND_ADD_REF_HASH(MOB_PATHDATA(op)->goal_map, goal_map->orig_path);
            MOB_PATHDATA(op)->goal_x = goal_x;
            MOB_PATHDATA(op)->goal_y = goal_y;
        }
    }

    if(goal_map == NULL)
    {
        LOG(llevDebug, "object_accept_path(): NULL goal map. op=%s, map %s, target=%s\n",
                STRING_OBJ_NAME(op), STRING_MAP_PATH(op->map), STRING_OBJ_NAME(target));
        return;
    }

    /* Early exit if we are already close enough */
    if(get_rangevector_full(op, op->map, op->x, op->y,
                NULL, goal_map, goal_x, goal_y,
                &v, RV_DIAGONAL_DISTANCE) && 
            v.distance <= 1)
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
            LOG(llevDebug, "object_accept_path(): '%s' new path -> [object '%s' / coordinate %d:%d@%s])\n",
                STRING_OBJ_NAME(op),
                STRING_OBJ_NAME(MOB_PATHDATA(op)->target_obj),
                MOB_PATHDATA(op)->target_x, MOB_PATHDATA(op)->target_y,
                STRING_SAFE(MOB_PATHDATA(op)->target_map));
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
        LOG(llevDebug, "object_accept_path(): no path to destination ('%s' -> [object '%s' / coordinate %d:%d@%s])\n",
                STRING_OBJ_NAME(op),
                STRING_OBJ_NAME(MOB_PATHDATA(op)->target_obj),
                MOB_PATHDATA(op)->target_x, MOB_PATHDATA(op)->target_y,
                STRING_SAFE(MOB_PATHDATA(op)->target_map));
        LOG(llevDebug, "  last movement behaviour of '%s': '%s'\n",
                STRING_OBJ_NAME(op), MOB_DATA(op)->last_movement_behaviour->name);
        SET_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_FAILED);
    }
}

/*
 * Dump to standard out the abilities of all monsters.
 */
void dump_abilities(void)
{
    archetype  *at;
    for (at = first_archetype; at; at = at->next)
    {
        const char *ch, *gen_name = "";
        archetype  *gen;

        if (!QUERY_FLAG(&at->clone, FLAG_MONSTER))
            continue;

        /* Get rid of e.g. multiple black puddings */
        if (QUERY_FLAG(&at->clone, FLAG_CHANGING))
            continue;

        for (gen = first_archetype; gen; gen = gen->next)
        {
            if (gen->clone.other_arch && gen->clone.other_arch == at)
            {
                gen_name = gen->name;
                break;
            }
        }

        ch = describe_item(&at->clone);
        LOG(llevInfo, "%-16s|%6d|%4d|%3d|%s|%s|%s\n", at->clone.name, at->clone.stats.exp, at->clone.stats.hp,
            at->clone.stats.ac, ch, at->name, gen_name);
    }
}

/*
 * As dump_abilities(), but with an alternative way of output.
 */

void print_monsters(void)
{
    archetype  *at;
    object     *op;
    int         i;

    LOG(llevInfo,
        "               |     |   |    |    |                                  attacks/ resistances                                                                                              |\n");
    LOG(llevInfo,
        "monster        | hp  |dam| ac | wc | phy mag fir ele cld cfs acd drn wmg ght poi slo par tim fea cnc dep dth chs csp gpw hwd bln int lst sla cle pie net son dem psi |  exp   | new exp |\n");
    LOG(llevInfo,
        "-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    for (at = first_archetype; at != NULL; at = at->next)
    {
        op = arch_to_object(at);
        if (QUERY_FLAG(op, FLAG_MONSTER))
        {
            LOG(llevInfo, "%-15s|%5d|%3d|%4d|%4d|", op->arch->name, op->stats.maxhp, op->stats.dam, op->stats.ac,
                op->stats.wc);
            for (i = 0; i < NROFATTACKS; i++)
                LOG(llevInfo, "%4d", op->attack[i]);
            LOG(llevInfo, " |\n               |     |   |    |    |");
            for (i = 0; i < NROFATTACKS; i++)
                LOG(llevInfo, "%4d", op->resist[i]);
            LOG(llevInfo, " |%8d|\n", op->stats.exp);
        }
    }
}
