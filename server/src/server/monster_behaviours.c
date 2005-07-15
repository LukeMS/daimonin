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
 * This file contains the actual mob AI behaviours and their
 * utility functions
 */

#include <global.h>

#include <aiconfig.h>

void ai_choose_enemy(object *op, struct mob_behaviour_param *params);
    
/* the attribute Str is atm not used from monsters (was used in push code) MT-06.2005 */
rv_vector      *get_known_obj_rv(object *op, struct mob_known_obj *known_obj, int maxage);

/*
 * A few random unsorted utility functions
 */

/* Tests if an object is in the line of sight of another object. */
int obj_in_line_of_sight(object *op, object *obj, rv_vector *rv)
{
    /* Bresenham variables */
    int fraction, dx2, dy2, stepx, stepy;

    /* Stepping variables */
    mapstruct *m = rv->part->map;
    int x = rv->part->x, y = rv->part->y;

    /*
    LOG(llevDebug, "obj_in_line_of_sight(): %s (%d:%d) -> %s (%d:%d)?\n", 
            STRING_OBJ_NAME(op), op->x, op->y, 
            STRING_OBJ_NAME(obj), obj->x, obj->y);
    */
            
    BRESENHAM_INIT(rv->distance_x, rv->distance_y, fraction, stepx, stepy, dx2, dy2);

    while(1) 
    {
        LOG(llevDebug, " (%d:%d)", x, y);
        if(x == obj->x && y == obj->y && m == obj->map) 
        {
//            LOG(llevDebug, "  can see!\n");
            return TRUE;
        }

        if(m == NULL || GET_MAP_FLAGS(m,x,y) & P_BLOCKSVIEW)
        {
//            LOG(llevDebug, "  blocked!\n");
            return FALSE;
        }

        BRESENHAM_STEP(x, y, fraction, stepx, stepy, dx2, dy2);

        m = out_of_map(m, &x, &y);
    }
/*
    LOG(llevDebug, "  out of range!\n");
    return FALSE;*/
}

/* Beginnings of can_see_obj */
/* known_obj is optional but increases efficiency somewhat
 * by using caching data in the known_obj struct
 */
int mob_can_see_obj(object *op, object *obj, struct mob_known_obj *known_obj)
{
    int  aggro_range, stealth_range;

    /* Cache values */
    static tag_t cached_op_tag, cached_obj_tag;
    static uint32 cache_time;
    static int cached_result;

    rv_vector   rv, *rv_p = NULL;

    /* Quick answer if possible */
    if (known_obj && known_obj->last_seen == ROUND_TAG)
        return TRUE;

    /* Try using cache */
    if (cached_op_tag == op->count && cached_obj_tag == obj->count &&
                    cache_time == ROUND_TAG)
        return cached_result;

    /* Invisibility */
    if (QUERY_FLAG(obj, FLAG_IS_INVISIBLE) && !QUERY_FLAG(op, FLAG_SEE_INVISIBLE))
        return FALSE;

    aggro_range = op->stats.Wis; /* wis is basic sensing range */

    /* Extra range to alerted monsters */
    if (op->enemy) {
        aggro_range += 3;
        if (op->enemy == obj)
            aggro_range += 3;
    }

    /* Much less range if asleep or blind */
    if (QUERY_FLAG(op, FLAG_SLEEP) || QUERY_FLAG(op, FLAG_BLIND))
        aggro_range /= 2;

    /* Alternative sensing range for stealthy targets */
    stealth_range = MAX(MIN_MON_RADIUS, aggro_range - 2);

    /* Get the rangevector, trying to use a cached version first */
    if (known_obj)
        rv_p = get_known_obj_rv(op, known_obj, MAX_KNOWN_OBJ_RV_AGE);
    else if (get_rangevector(op, obj, &rv, RV_EUCLIDIAN_DISTANCE))
        rv_p = &rv;

    if (rv_p == NULL)
        cached_result = FALSE;
    else if ((int) rv_p->distance > (QUERY_FLAG(obj, FLAG_STEALTH) ? stealth_range : aggro_range))
        cached_result = FALSE;
    else 
    {
        /* LOS test is _only_ done when we first register a new object,
         * otherwise it is too easy to escape monsters by hiding. */
        cached_result = TRUE;
    }

    cached_op_tag = op->count;
    cached_obj_tag = obj->count;
    cache_time = ROUND_TAG;

    return cached_result;
}

/* TODO: these two new functions are already obsolete and should be replaced
 * with configuration options in the ai_friendship behaviour,
 * but they serve well as a base...
 */
int is_enemy_of(object *op, object *obj)
{
    /* TODO: add a few other odd types here, such as god & golem */
    if (!obj->type == PLAYER || !obj->type == MONSTER || op == obj)
        return FALSE;

    /* Unagressive mobs are never enemies to anything (?) */
    if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE) || QUERY_FLAG(obj, FLAG_UNAGGRESSIVE))
        return FALSE;

    /* TODO: this needs to be sorted out better */
    if (QUERY_FLAG(op, FLAG_FRIENDLY))
    {
        if (QUERY_FLAG(obj, FLAG_MONSTER) && !QUERY_FLAG(obj, FLAG_FRIENDLY))
            return TRUE;
    }
    else
    {
        if (QUERY_FLAG(obj, FLAG_FRIENDLY) || obj->type == PLAYER)
            return TRUE;
    }

    return FALSE;
}

int is_friend_of(object *op, object *obj)
{
    /* TODO: add a few other odd types here, such as god & golem */
    if (!obj->type == PLAYER || !obj->type == MONSTER || !op->type == PLAYER || !op->type == MONSTER || op == obj)
        return FALSE;

    /* TODO: this needs to be sorted out better */
    if (QUERY_FLAG(op, FLAG_FRIENDLY) || op->type == PLAYER)
    {
        if (!QUERY_FLAG(obj, FLAG_MONSTER) || QUERY_FLAG(obj, FLAG_FRIENDLY) || obj->type == PLAYER)
        {
            return TRUE;
        }
    }
    else
    {
        if (!QUERY_FLAG(obj, FLAG_FRIENDLY) && obj->type != PLAYER)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * Get the rangevector to a known object. If an earlier calculated rangevector is
 * older than maxage then we calculate a new one (set maxage to 0 to force update).
 * Returns a pointer to the rangevector, or NULL if get_rangevector() failed.
 */
rv_vector * get_known_obj_rv(object *op, struct mob_known_obj *known_obj, int maxage)
{
    /* TODO: added checks for NULL maps here (happens if monster is picked up, for example).
     * Actually, it would be slightly more interesting if we could get the coordinates for the
     * container of the mob, so that mobs can for example hide in containers until the enemy
     * is far enough away. Or reversly, hide in container and later jump out and attack enemy. *
     * Gecko 2005-05-08 */
    if ( op == NULL || op->map == NULL || op->env || 
            known_obj == NULL || known_obj->obj->map == NULL || known_obj->obj->env)
        return NULL;

    if (ROUND_TAG - known_obj->rv_time >= (uint32) maxage || known_obj->rv_time == 0 || maxage == 0)
    {
        /*
        if(!mob_can_see_obj(op, known_obj->obj, NULL)) {
            mapstruct *map = ready_map_name(known_obj->last_map, MAP_NAME_SHARED);
            if(get_rangevector_full(op, op->map, op->x, op->y,
                        known_obj->obj, map, known_obj->last_x, known_obj->last_y,
                        &known_obj->rv, RV_EUCLIDIAN_DISTANCE))
            {
                known_obj->rv_time = global_round_tag;
            } else
            {
                known_obj->rv_time = 0;
                return NULL;
            }
        }
        */

        if (get_rangevector(op, known_obj->obj, &known_obj->rv, 0))
        {
            known_obj->rv_time = ROUND_TAG;
        }
        else
        {
            known_obj->rv_time = 0;
            return NULL;
        }
    }

    /* hotfix for this bug. part should here NOT be NULL */
    if (!known_obj->rv.part)
    {
        LOG(-1, "CRASHBUG: rv->part == NULL for %s on map %s with enemy %s and map %s\n", query_name(op),
            op->map ? STRING_SAFE(op->map->path) : "NULL", query_name(known_obj->obj),
            known_obj->obj ? STRING_SAFE(known_obj->obj->map ? STRING_SAFE(known_obj->obj->map->path) : "NULL") : "NULL");
        return NULL;
    }
    return &known_obj->rv;
}

/* TODO: make a real behaviour... */
#if 0
void npc_call_for_help(object *op) {
  struct mob_known_obj *friend;

  /* TODO: remember to check that the called has MOB_DATA set up before doing anything else... */
  /* TODO: use tmp_friendship? */
  for(friend = MOB_DATA(op)->friends; friend; friend=friend->next) {
      if(friend->friendship >= FRIENDSHIP_HELP && friend->ob->enemy == NULL)  {
          rv_vector *rv = get_known_obj_rv(op, friend, MAX_KNOWN_OBJ_RV_AGE);
          if(rv && rv->distance < 4) {
              /* TODO: also check reverse friendship */
              /* TODO: -friendship here dependant on +friendship towards tmp */
              register_npc_known_obj(friend->ob, op->enemy, FRIENDSHIP_ATTACK);
          }
      }
  }
}
#endif

int calc_friendship_from_attitude(object *op, object *other)
{
    int friendship = 0;
    struct mob_behaviour_param *attitudes = MOB_DATA(op)->behaviours->attitudes;
    struct mob_behaviour_param *tmp;

    if(attitudes == NULL)
        return 0;

    /* Race attitude */
    if(attitudes[AIPARAM_ATTITUDE_RACE].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attitudes[AIPARAM_ATTITUDE_RACE]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->race && tmp->stringvalue == other->race)
                friendship += tmp->intvalue;
        }
    }

    /* Arch attitude */
    if(attitudes[AIPARAM_ATTITUDE_ARCH].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attitudes[AIPARAM_ATTITUDE_ARCH]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->arch->name && tmp->stringvalue == other->arch->name)
                friendship += tmp->intvalue;
        }
    }

    /* Named object attitude */
    if(attitudes[AIPARAM_ATTITUDE_NAME].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attitudes[AIPARAM_ATTITUDE_NAME]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->name && tmp->stringvalue == other->name)
                friendship += tmp->intvalue;
        }
    }

    /* Named group attitude */
    if(attitudes[AIPARAM_ATTITUDE_GROUP].flags & AI_PARAM_PRESENT)
    {
        /* Make sure other is a monster that belongs to one or more groups */
        if(other->type == MONSTER && MOB_DATA(other) &&
                MOB_DATA(other)->behaviours->groups &&
                MOB_DATA(other)->behaviours->groups[AIPARAM_GROUPS_NAME].flags & AI_PARAM_PRESENT)
        {
            /* Match my group attitudes to the other's group memberships */
            for(tmp = &attitudes[AIPARAM_ATTITUDE_ARCH]; tmp != NULL;
                    tmp = tmp->next)
            {
                struct mob_behaviour_param *group;
                for(group = &MOB_DATA(op)->behaviours->groups[AIPARAM_GROUPS_NAME];
                        group != NULL; group = group->next)
                {
                    if(tmp->stringvalue == group->stringvalue)
                        friendship += tmp->intvalue;
                }
            }
        }
    }

    /* Player attitude */
    if(attitudes[AIPARAM_ATTITUDE_PLAYER].flags & AI_PARAM_PRESENT)
    {
        if(other->type == PLAYER)
            friendship += attitudes[AIPARAM_ATTITUDE_PLAYER].intvalue;
    }

    LOG(llevDebug, "Attitude friendship modifier: %d (%s->%s)\n", friendship, STRING_OBJ_NAME(op), STRING_OBJ_NAME(other));

    return friendship;
}

/* register a new enemy or friend for the NPC */
struct mob_known_obj * register_npc_known_obj(object *npc, object *other, int friendship)
{
    struct mob_known_obj   *tmp;
    struct mob_known_obj   *last    = NULL;
    int i;
    rv_vector rv;

    if (npc == NULL)
    {
#ifdef AI_DEBUG
        LOG(llevDebug, "register_npc_known_obj(): Called with NULL npc obj\n");
#endif
        return NULL;
    }

    if (other == NULL)
    {
#ifdef AI_DEBUG
        LOG(llevDebug, "register_npc_known_obj(): Called with NULL other obj\n");
#endif
        return NULL;
    }

    if (npc == other)
    {
#ifdef AI_DEBUG
        LOG(llevDebug, "register_npc_known_obj(): Called for itself '%s'\n", STRING_OBJ_NAME(npc));
#endif
        return NULL;
    }

   /*
    * this is really needed.
    * a hitter object can be a "system object" ...even a object
    * in the inventory of npc (like a disease). These objects have
    * usually no map.
    *
    * Gecko: Hmm... I have to fix this to be able to handle non-mob objects
    */
    if (other->type != PLAYER && !QUERY_FLAG(other, FLAG_ALIVE))
    {
#ifdef AI_DEBUG
        LOG(llevDebug, "register_npc_known_obj(): Called for non PLAYER/IS_ALIVE '%s'\n", STRING_OBJ_NAME(npc));
#endif
        return NULL;
    }

    if (npc->type != MONSTER)
    {
#ifdef AI_DEBUG
        LOG(llevDebug, "register_npc_known_obj(): Called on non-mob object '%s' type %d\n", STRING_OBJ_NAME(npc),
            npc->type);
#endif
        return NULL;
    }

    /* this check will hopefully be unnecessary in the future */
    if (MOB_DATA(npc) == NULL)
    {
#ifdef AI_DEBUG
        LOG(llevDebug, "register_npc_known_obj(): No mobdata (yet) for '%s'\n", STRING_OBJ_NAME(npc));
#endif
        return NULL;
    }

    /* Ignore mobs that are inside containers or not on maps */
    if(other->map == NULL || other->env != NULL)
    {
        LOG(llevDebug, "register_npc_known_obj(): '%s' trying to register object '%s' not on a map\n", STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other));
        return NULL;
    }
    
    /* TODO: get rid of flag_unaggressive and use only friendship */
    if (friendship < 0 && QUERY_FLAG(npc, FLAG_UNAGGRESSIVE))
    {
        CLEAR_FLAG(npc, FLAG_UNAGGRESSIVE);
        friendship += FRIENDSHIP_ATTACK;
    }

    /* Does npc already know this other? */
    for (tmp = MOB_DATA(npc)->known_mobs; tmp; tmp = tmp->next)
    {
        if (tmp->obj == other && tmp->obj_count == other->count)
        {
            tmp->last_seen = ROUND_TAG;
            FREE_AND_ADD_REF_HASH(tmp->last_map, other->map->path);
            tmp->last_x = other->x;
            tmp->last_y = other->y;
            tmp->friendship += friendship;
            /*            if(friendship)
                            LOG(llevDebug,"register_npc_known_obj(): '%s' changed mind about '%s'. friendship: %d -> %d\n",  STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other), tmp->friendship - friendship, tmp->friendship);*/
            return tmp;
        }
        last = tmp;
    }

    /* TODO: keep count of enemies and push out less
     * important if new ones are added beyond a reasonable max number */

    /* It was a new, previously unknown object */
    if(! get_rangevector(npc, other, &rv, RV_EUCLIDIAN_DISTANCE) || !rv.part)
    {
        LOG(llevBug, "BUG: register_npc_known_obj(): '%s' can't get rv to '%s'\n", STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other));
        return NULL;
    }        
    
    /* We check LOS here, only if we are registering a new object */
    /* Also, we only check against players, and not if we have 
     * been hit or helped by them. */
    if(other->type == PLAYER && friendship == 0 && 
            !obj_in_line_of_sight(npc, other, &rv))
        return NULL;

    tmp = get_poolchunk(pool_mob_knownobj);
    tmp->next = NULL;
    tmp->prev = last;
    tmp->obj = other;
    tmp->obj_count = other->count;

    tmp->last_map = add_refcount(other->map->path);
    tmp->last_x = other->x;
    tmp->last_y = other->y;

    tmp->last_seen = ROUND_TAG; /* If we got here, we have seen it */
//    tmp->rv_time = 0;           /* Makes cached rv invalid */
    tmp->rv_time = ROUND_TAG;   /* Cache the rv we calculated above. */
    tmp->rv = rv;

    /* Initial friendship and attitude */
    tmp->friendship = friendship + calc_friendship_from_attitude(npc, other);
    tmp->attraction = 0;
    tmp->tmp_friendship = 0;
    tmp->tmp_attraction = 0;

    for(i=0; i<=NROF_AI_KNOWN_OBJ_FLAGS/32; i++)
        tmp->flags[i] = 0;

    /* Insert last in list of known objects */
    if (last)
        last->next = tmp;
    else
        MOB_DATA(npc)->known_mobs = tmp;

    //    LOG(llevDebug,"register_npc_known_obj(): '%s' detected '%s'. friendship: %d\n",  STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other), tmp->friendship);

    return tmp;
}

/*
 * Waypoint utility functions
 */

/* Find a monster's currently active waypoint, if any */
object * get_active_waypoint(object *op)
{
    object *wp  = NULL;

    for (wp = op->inv; wp != NULL; wp = wp->below)
        if (wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, WP_FLAG_ACTIVE))
            break;

    return wp;
}

/* Find a monster's current return-home wp, if any */
object * get_return_waypoint(object *op)
{
    object *wp  = NULL;

    for (wp = op->inv; wp != NULL; wp = wp->below)
        if (wp->type == TYPE_WAYPOINT_OBJECT && QUERY_FLAG(wp, FLAG_REFLECTING))
            break;

    return wp;
}

/* Find a monster's waypoint by name (used for getting the next waypoint) */
object * find_waypoint(object *op, const char *name)
{
    object *wp  = NULL;

    if (name == NULL)
        return NULL;

    for (wp = op->inv; wp != NULL; wp = wp->below)
        if (wp->type == TYPE_WAYPOINT_OBJECT && strcmp(wp->name, name) == 0)
            break;

    return wp;
}

/*
 * Some other utility functions
 */

int can_hit_melee(object *ob1, object *ob2, rv_vector *rv)
{
    if (QUERY_FLAG(ob1, FLAG_CONFUSED) && !(RANDOM() % 3))
        return 0;
    return abs(rv->distance_x) < 2 && abs(rv->distance_y) < 2;
}

/* modes:
 * 1 - exact 45 deg
 * 2 - 45 deg +- one tile
 * 3 - free 360 deg LOF
 *
 * TODO: rename to is_in_line_of_fire
 */
int can_hit_missile(object *ob1, object *ob2, rv_vector *rv, int mode)
{
    /* TODO: actually perform a rough line of sight calculation */

    switch (mode)
    {
        case 1:
          /* exact 45 deg */
        default:
          return rv->distance_x == 0 || rv->distance_y == 0 || abs(rv->distance_x) - abs(rv->distance_y) == 0;
        case 2:
          /* 45 deg +- one tile */
          return abs(rv->distance_x) <= 1
              || abs(rv->distance_y) <= 1
              || abs(abs(rv->distance_x) - abs(rv->distance_y)) <= 1;

        case 3:
          /* free 360 deg line of fire */
          return TRUE;
    }
}

/* Ugly hack for now... */
int mapcoord_in_line_of_fire(object *op1, mapstruct *map, int x, int y, int mode)
{
    rv_vector rv;
    get_rangevector_full(op1, op1->map, op1->x, op1->y, NULL, map, x, y, &rv, RV_DIAGONAL_DISTANCE);
    return can_hit_missile(op1, NULL, &rv, mode);
}

/* Normalize a given map path and make sure it is valid and
 * that the map is loaded. Can return NULL in case of failure */
mapstruct *normalize_and_ready_map(mapstruct *defmap, const char **path)
{
    /* Default map is current map */
    if (path == NULL || *path == NULL || **path == '\0')
        return defmap;

    /* If path not normalized: normalize it */
    if (**path != '/')
    {
        char    temp_path[HUGE_BUF];
        normalize_path(defmap->path, *path, temp_path);
        FREE_AND_COPY_HASH(*path, temp_path);
    }

    /* check if we are already on the map */
    if (*path == defmap->path)
        return defmap;
    else
        return ready_map_name(*path, MAP_NAME_SHARED);
}

/* scary function - need rework. even in crossfire its changed now */
void monster_check_apply(object *mon, object *item)
{
    /* this function is simply to bad - for example will potions applied
     * not depending on the situation... why applying a heal potion when
     * full hp? firestorm potion when standing next to own people?
     * IF we do some AI stuff here like using items we must FIRST
     * add a AI - then doing the things. Think first, act later!
     */
}

/*
 * Mutually exclusive movement behaviours
 */

void ai_stand_still(object *op, struct mob_behaviour_param *params, move_response *response)
{
    response->type = MOVE_RESPONSE_DIR;
    response->data.direction = 0;
}

void ai_sleep(object *op, struct mob_behaviour_param *params, move_response *response)
{
    if (QUERY_FLAG(op, FLAG_SLEEP))
    {
        if (op->enemy)
            CLEAR_FLAG(op, FLAG_SLEEP);
        else
        {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = 0;
        }
    }
}

void ai_move_randomly(object *op, struct mob_behaviour_param *params, move_response *response)
{
    int     i, r;
    object *base    = find_base_info_object(op);

    /* Give up to 8 chances for a monster to move randomly */
    for (i = 0; i < 8; i++)
    {
        r = RANDOM() % 8 + 1;

        /* TODO: doesn't handle map borders */
        /* check x and y direction of possible move */
        if (params[AIPARAM_MOVE_RANDOMLY_XLIMIT].flags & AI_PARAM_PRESENT)
            if (abs(op->x + freearr_x[r] - base->x) > params[AIPARAM_MOVE_RANDOMLY_XLIMIT].intvalue)
                continue;
        if (params[AIPARAM_MOVE_RANDOMLY_YLIMIT].flags & AI_PARAM_PRESENT)
            if (abs(op->y + freearr_y[r] - base->y) > params[AIPARAM_MOVE_RANDOMLY_YLIMIT].intvalue)
                continue;

        if (!blocked_link(op, freearr_x[r], freearr_y[r]))
        {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = r;
            return;
        }
    }
}

void ai_move_towards_home(object *op, struct mob_behaviour_param *params, move_response *response)
{
    /* TODO: optimization: pointer to the base ob in mob_data */
    object *base    = insert_base_info_object(op);

    if (base && base->slaying)
    {
        /* If mob isn't already home */
        if (op->x != base->x || op->y != base->y || op->map->path != base->slaying)
        {
            mapstruct  *map = normalize_and_ready_map(op->map, &base->slaying);

            response->type = MOVE_RESPONSE_COORD;
            response->data.coord.x = base->x;
            response->data.coord.y = base->y;
            response->data.coord.map = map;
        }
    }
}

/* Useful if mob is much slower than enemy? */
void ai_step_back_after_swing(object *op, struct mob_behaviour_param *params, move_response *response)
{
    if (op->weapon_speed_left > 0 && OBJECT_VALID(op->enemy, op->enemy_count) && mob_can_see_obj(op, op->enemy,
                                                                                                 MOB_DATA(op)->enemy))
    {
        rv_vector  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);

        if (rv && rv->distance < (unsigned int) AIPARAM_INT(AIPARAM_KEEP_DISTANCE_TO_ENEMY_MIN_DIST))
        {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = absdir(rv->direction + 4);
            op->anim_enemy_dir = response->data.direction;
        }
        else if (rv->distance == (unsigned int) AIPARAM_INT(AIPARAM_KEEP_DISTANCE_TO_ENEMY_MAX_DIST))
        {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = 0;
            op->anim_enemy_dir = rv->direction;
        }
    }
}

void ai_avoid_line_of_fire(object *op, struct mob_behaviour_param *params, move_response *response)
{
    if (OBJECT_VALID(op->enemy, op->enemy_count))
    {
        /* TODO: not correct for multi-tile mobs, the in_line_of_fire() functions simply don't
         * work for them. Possible solutions: 1) disable for multi-tile mobs (what do big monsters care
         * about puny missiles, anyway?  2) fix the line-of-fire functions (can be very expensive) */
        /* Disabled for multi-tile mobs */
        if(op->more)
            return;

        /* TODO: mobs will not approach enemy through narrow corridors, as they can't
         * avoid missiles there. It also means they can get stuck in the middle of a corridor as
         * a sitting duck for any distance attacks. Possible fixes: 1) only activate if the enemy if
         * known to use missiles (maybe easy with upcoming mob damage memory) 2) temporarily disable
         * if we get stuck somewhere. 3) detect getting stuck and either flee or charge.
         */

        /* Disable behaviour if we don't think enemy uses missiles */
        if(! QUERY_FLAG(MOB_DATA(op)->enemy, AI_OBJFLAG_USES_DISTANCE_ATTACK))
        {
            if(op->enemy->type == PLAYER)
            {
                /* Nasty hack for quick detection of possible distance-attack
                 * skills */
                /* TODO: should preferably be using observed behaviour instead of
                 * chosen skill, but this is quite cheap. */
                char is_distance_skill[NROFSKILLS] = {
                    0,0,0,0,0, 0,0,0,0,0,
                    0,0,0,0,0, 0,0,0,1,0, /* Flame touch ? */
                    0,0,0,0,1, 1,1,0,0,1,
                    1,0,0,0,1, 1,0,0,0,0,
                    0,0 };

                if(op->enemy->chosen_skill == NULL ||
                        ! is_distance_skill[op->enemy->chosen_skill->stats.sp])
                    return;
                SET_FLAG(MOB_DATA(op)->enemy, AI_OBJFLAG_USES_DISTANCE_ATTACK);
            }
            else if (op->enemy->type == MONSTER)
            {
                if(! QUERY_FLAG(op->enemy, FLAG_READY_SPELL) &&
                        ! QUERY_FLAG(op->enemy, FLAG_READY_BOW))
                    return;
                SET_FLAG(MOB_DATA(op)->enemy, AI_OBJFLAG_USES_DISTANCE_ATTACK);
            }
        }

        /* Behaviour core */
        if(mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        {
            rv_vector  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);

            if (rv->distance > 2 && rv->distance < 8)
            {
                int i;

                   /* Avoid staying in line of fire */
                if(can_hit_missile(op->enemy, op, rv, 1))
                       response->forbidden |= (1 << 0);

                for(i=-3; i<=3; i++) {
                    mapstruct *m;
                    int d = absdir(rv->direction + i);
                    int x = op->x + freearr_x[d];
                    int y = op->y + freearr_y[d];

                    /* Avoid moving into line of fire */
                    if ((m = out_of_map(op->map, &x, &y)))
                    {
                        if(mapcoord_in_line_of_fire(op->enemy, m, x, y, 1))
                            response->forbidden |= (1 << d);
                    }
                }
            }
        }
    }
}

void ai_optimize_line_of_fire(object *op, struct mob_behaviour_param *params, move_response *response)
{
    /* TODO: not correct for multi-tile mobs, the in_line_of_fire() functions simply don't
     * work for them. Possible solutions: 1) disable for multi-tile mobs (what do big monsters care
     * about puny missiles, anyway?  2) fix the line-of-fire functions (can be very expensive) */
    /* Disabled for multi-tile mobs */
    if(op->more)
        return;

    if (OBJECT_VALID(op->enemy, op->enemy_count))
    {
        /* Behaviour core */
        if(mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        {
            int i;
            int good_directions = 0, ok_directions = 0;

            rv_vector  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);

            /* Too close or too far to care? */
            if(rv->distance <= 2 || rv->distance > 8)
                return;

            /* Already perfect? */
            if(can_hit_missile(op, op->enemy, rv, 1))
                good_directions = (1 << 0);
            else
                response->forbidden |= (1 << 0); /* Don't stay in a bad spot */

            /* Find a nearby good spot */
            /* TODO: can probably be calculated instead of searched for */
            /* TODO: with this algorithm there is a certain state where
             * the mob starts zipping between two "half-good" spots */
            for(i=-2; i<=2; i++)
            {
                mapstruct *m;
                int dir = absdir(rv->direction+i);
                int x = op->x + freearr_x[dir];
                int y = op->y + freearr_y[dir];

                /* Find a spot in or near line of fire, and forbid movements to other spots */
                if ((m = out_of_map(op->map, &x, &y)))
                {
                    if(mapcoord_in_line_of_fire(op->enemy, m, x, y, 1)) /* good spot? */
                        good_directions |= (1 << dir);
                    else if(mapcoord_in_line_of_fire(op->enemy, m, x, y, 2)) /* ok spot? */
                        ok_directions |= (1 << dir);
                }
            }

            /* See if we have a movement response... */
            good_directions &= ~response->forbidden;
            if(good_directions) {
                response->data.directions = good_directions;
                response->type = MOVE_RESPONSE_DIRS;
            }
            else {
                ok_directions &= ~response->forbidden;
                if(ok_directions) {
                    response->data.directions = ok_directions;
                    response->type = MOVE_RESPONSE_DIRS;
                }
            }
        }
    }
}


void ai_move_towards_enemy(object *op, struct mob_behaviour_param *params, move_response *response)
{
    rv_vector  *rv;
    
    if (!OBJECT_VALID(op->enemy, op->enemy_count) || !mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        return;

    rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);
    if(rv == NULL)
        return;

    op->anim_enemy_dir = rv->direction;

    if (rv->distance <= 1)
    {
        /* Stay where we are */
        response->type = MOVE_RESPONSE_DIR;
        response->data.direction = 0;
        return;        
    }

    /* If we can't even find a way to the enemy, downgrade it */
    if(MOB_PATHDATA(op)->target_obj == op->enemy &&
            QUERY_FLAG(MOB_PATHDATA(op), PATHFINDFLAG_PATH_FAILED))
    {
        LOG(llevDebug, "ai_move_towards_enemy(): %s can't get to %s, downgrading its enemy status\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));
        MOB_DATA(op)->enemy->friendship = 0;
        MOB_DATA(op)->enemy->tmp_friendship = 0;

        /* Go through the mob list yet again (should only be done once) */
        /* TODO: keep track of second_worst_enemy instead... */
        ai_choose_enemy(op, params);
        
//        LOG(llevDebug, "ai_move_towards_enemy(): %s chose new enemy: %s\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));
        if(op->enemy == NULL)
            return;
    }
        
    response->type = MOVE_RESPONSE_OBJECT;
    response->data.target.obj = op->enemy;
    response->data.target.obj_count = op->enemy_count;
}

void ai_keep_distance_to_enemy(object *op, struct mob_behaviour_param *params, move_response *response)
{
    if (OBJECT_VALID(op->enemy, op->enemy_count) && mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
    {
        rv_vector  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);

        if (rv)
        {
            /* keep distance is something different as run away.
             * But a endless "keep distance" is or at last looks the same.
             * So, we must avoid it.
             * With the "action movement delay, this should work very well.
             * We should also handle this more tricky in the AI behaviour itself.
             */
            if(rv->distance <= 1)
            {
                ai_move_towards_enemy(op, params, response);
                return;
            }

            if (rv->distance < (unsigned int) AIPARAM_INT(AIPARAM_KEEP_DISTANCE_TO_ENEMY_MIN_DIST))
            {
                response->type = MOVE_RESPONSE_DIR;
                response->data.direction = absdir(rv->direction + 4);
                op->anim_enemy_dir = response->data.direction;
            }
            else if (rv->distance < (unsigned int) AIPARAM_INT(AIPARAM_KEEP_DISTANCE_TO_ENEMY_MAX_DIST))
            {
//                response->type = MOVE_RESPONSE_DIR;
//                response->data.direction = 0;
                response->forbidden |= (1 << rv->direction);
                response->forbidden |= (1 << absdir(rv->direction+1));
                response->forbidden |= (1 << absdir(rv->direction-1));
                op->anim_enemy_dir = rv->direction;
            }
        }
    }
}

void ai_move_towards_enemy_last_known_pos(object *op, struct mob_behaviour_param *params, move_response *response)
{
    if (OBJECT_VALID(op->enemy, op->enemy_count) && MOB_DATA(op)->enemy->last_map)
    {
        rv_vector               rv;
        struct mob_known_obj   *enemy   = MOB_DATA(op)->enemy;
        mapstruct              *map     = ready_map_name(enemy->last_map, MAP_NAME_SHARED);

        if (get_rangevector_full(op, op->map, op->x, op->y, 
                    enemy->obj, map, enemy->last_x, enemy->last_y, 
                    &rv, RV_EUCLIDIAN_DISTANCE))
        {
            op->anim_enemy_dir = rv.direction;
            if (rv.distance > 3)
            {
                response->type = MOVE_RESPONSE_COORD;
                response->data.coord.x = enemy->last_x;
                response->data.coord.y = enemy->last_y;
                response->data.coord.map = map;
            }
        }
    }
}

/* Stupid behaviour that moves around randomly looking for a lost enemy */
void ai_search_for_lost_enemy(object *op, struct mob_behaviour_param *params, move_response *response)
{
    if (OBJECT_VALID(op->enemy, op->enemy_count) && MOB_DATA(op)->enemy->last_map)
    {
        int i, r;

        /* Give up to 8 chances for a monster to move randomly */
        for (i = 0; i < 8; i++)
        {
            r = RANDOM() % 8 + 1;

            if (!blocked_link(op, freearr_x[r], freearr_y[r]))
            {
                response->type = MOVE_RESPONSE_DIR;
                response->data.direction = r;
                return;
            }
        }
    }
}

void ai_move_towards_waypoint(object *op, struct mob_behaviour_param *params, move_response *response)
{
    object     *wp;
    rv_vector   rv;
    int         try_next_wp = 0;

    wp = get_active_waypoint(op);
    if (wp)
    {
        mapstruct  *destmap = normalize_and_ready_map(op->map, &WP_MAP(wp));
        if (destmap)
        {
            /* We know which map we want to. Can we figure out where that
             * map lies relative to current position? */

            /* This rv may be computed several times, this is generally
             * not a performance problem, since the cache in the recursive
             * search usually catches that.
             * TODO: extend cache in recursive search with longer memory
             */
            if (!get_rangevector_full(
                        op, op->map, op->x, op->y, 
                        NULL, destmap, WP_X(wp), WP_Y(wp), &rv,
                        RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE))
            {
                /* Problem: we couldn't find a relative direction between the
                 * maps. Usually it means that they are in different mapsets
                 * or too far away from each other. */
                LOG(llevDebug, "BUG: ai_move_towards_waypoint(): No connection between maps: '%s' and '%s'\n",
                    STRING_MAP_PATH(destmap), STRING_MAP_PATH(op->map));
                CLEAR_FLAG(wp, WP_FLAG_ACTIVE); /* disable this waypoint */
                try_next_wp = 1;
            }
            else
            {
                /* Good, we know general distance and direction to wp target */

                /* Are we close enough to accept the wp? */
                if (rv.distance <= (unsigned int) wp->stats.grace)
                {
                    /* Should we wait a little while? */
                    if (MOB_PATHDATA(op)->goal_delay_counter < WP_DELAYTIME(wp))
                    {
                        MOB_PATHDATA(op)->goal_delay_counter++;
                    }
                    else
                    {
#ifdef AI_DEBUG
                        LOG(llevDebug, "ai_move_towards_waypoint(): '%s' reached destination '%s'\n",
                            STRING_OBJ_NAME(op), STRING_OBJ_NAME(wp));
#endif

#ifdef PLUGINS
                        /* GROS: Handle for plugin trigger event */
                        if (wp->event_flags & EVENT_FLAG_TRIGGER)
                        {
                            CFParm  CFP;
                            CFParm *CFR;
                            int     k, l, m;
                            int     rtn_script  = 0;
                            object *event_obj   = get_event_object(wp, EVENT_TRIGGER);
                            m = 0;

                            k = EVENT_TRIGGER;
                            l = SCRIPT_FIX_NOTHING;
                            CFP.Value[0] = &k;
                            CFP.Value[1] = op;
                            CFP.Value[2] = wp;
                            CFP.Value[3] = NULL;
                            CFP.Value[4] = NULL;
                            CFP.Value[5] = &m;
                            CFP.Value[6] = &m;
                            CFP.Value[7] = &m;
                            CFP.Value[8] = &l;
                            CFP.Value[9] = (char *) event_obj->race;
                            CFP.Value[10] = (char *) event_obj->slaying;
                            if (findPlugin(event_obj->name) >= 0)
                            {
                                CFR = (PlugList[findPlugin(event_obj->name)].eventfunc) (&CFP);
                                rtn_script = *(int *) (CFR->Value[0]);
                                if (rtn_script != 0)
                                    return;
                            }
                        }
#endif

                        MOB_PATHDATA(op)->goal_delay_counter = 0;
                        MOB_PATHDATA(op)->best_distance = -1;
                        MOB_PATHDATA(op)->tried_steps = 0;
                        CLEAR_FLAG(wp, WP_FLAG_ACTIVE);
                        try_next_wp = 1;
                    }
                }
            }
        }
        else
        {
            LOG(llevDebug, "BUG: ai_move_towards_waypoint(): '%s' ('%s') no such map: '%s'\n", STRING_OBJ_NAME(op),
                STRING_OBJ_NAME(wp), STRING_WP_MAP(wp));
            CLEAR_FLAG(wp, WP_FLAG_ACTIVE);
            try_next_wp = 1;
        }
    }

    /* If we reached or gave up on the current waypoint */
    if (try_next_wp && wp)
    {
        if (WP_NEXTWP(wp) && (wp = find_waypoint(op, WP_NEXTWP(wp))))
        {
#ifdef AI_DEBUG
            LOG(llevDebug, "ai_move_towards_waypoint(): '%s' next WP: '%s'\n", STRING_OBJ_NAME(op), STRING_WP_NEXTWP(wp));
#endif
            SET_FLAG(wp, WP_FLAG_ACTIVE); /* activate new waypoint */
            MOB_PATHDATA(op)->best_distance = -1;
            MOB_PATHDATA(op)->tried_steps = 0;
        }
        else
        {
#ifdef AI_DEBUG
            LOG(llevDebug, "ai_move_towards_waypoint(): '%s' no next WP\n", STRING_OBJ_NAME(op));
#endif
            wp = NULL;
        }
    }

    if (wp)
    {
        response->type = MOVE_RESPONSE_WAYPOINT;
        response->data.target.obj = wp;
        response->data.target.obj_count = wp->count;
    }
}

/*
 * Runs away from enemy if scared
 * Sets scared if low hp
 * Clears scared if high enough hp OR after a random time
 */
void ai_run_away_from_enemy(object *op, struct mob_behaviour_param *params, move_response *response)
{
    rv_vector  *rv;

    /* Is scared? */
    if (QUERY_FLAG(op, FLAG_SCARED) && op->enemy)
    {
        if ((rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        {
            /* TODO: more intelligent: use pathfinding to find the
             * most distant point from enemy */
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = absdir(rv->direction + 4);
        }
        else
        {
            /* TODO: run around randomly? */
        }

        /* Regain senses? */
        if (op->stats.maxhp && AIPARAM_PRESENT(AIPARAM_RUN_AWAY_FROM_ENEMY_HP_THRESHOLD))
        {
            /* Gecko: I added a slight hysteresis treshold here
             * (stay afraid until hp reaches 2*runaway % of maxhp) */
            if (op->stats.hp == op->stats.maxhp
             || (op->stats.hp * 100) / op->stats.maxhp > AIPARAM_INT(AIPARAM_RUN_AWAY_FROM_ENEMY_HP_THRESHOLD) * 2)
            {
                CLEAR_FLAG(op, FLAG_SCARED);
            }
        }
        else
        {
            /* If we aren't scared because of low hp, we can stop
             * being afraid after a random delay */
            if (!RANDOM() % 20)
                CLEAR_FLAG(op, FLAG_SCARED);
        }
    }
    else
    {
        /* Become scared? */
        if (op->stats.maxhp
         && AIPARAM_PRESENT(AIPARAM_RUN_AWAY_FROM_ENEMY_HP_THRESHOLD)
         && (op->stats.hp * 100) / op->stats.maxhp < AIPARAM_INT(AIPARAM_RUN_AWAY_FROM_ENEMY_HP_THRESHOLD))
        {
            SET_FLAG(op, FLAG_SCARED);
        }
    }
}

/* AI <-> plugin interface for movement behaviours */
void ai_plugin_move(object *op, struct mob_behaviour_param *params, move_response *response)
{
#ifdef PLUGINS
    CFParm  CFP;
    int     k, l, m;
    k = EVENT_AI_BEHAVIOUR;
    l = 0; /* SCRIPT_FIX_ALL; */ /* Script fix none */
    m = 0;
    CFP.Value[0] = &k;   /* Event type */
    CFP.Value[1] = NULL; /* Activator */
    CFP.Value[2] = op;   /* Me */
    CFP.Value[3] = NULL; /* Other */
    CFP.Value[4] = NULL; /* Message */
    CFP.Value[5] = &m;
    CFP.Value[6] = &m;
    CFP.Value[7] = &m;
    CFP.Value[8] = &l; /* Fix settings */
    CFP.Value[9] = (char *)AIPARAM_STRING(AIPARAM_PLUGIN_MOVE_BEHAVIOUR);
    CFP.Value[10] = (char *)AIPARAM_STRING(AIPARAM_PLUGIN_MOVE_OPTIONS);
    CFP.Value[11] = (void *) response;
    PlugList[findPlugin(AIPARAM_STRING(AIPARAM_PLUGIN_MOVE_PLUGIN))].eventfunc (&CFP);
#endif
}

/*
 * Misc behaviours
 */


void ai_look_for_other_mobs(object *op, struct mob_behaviour_param *params)
{
    int tilenr;
    int sense_range;
    int check_maps[8] = {0,0,0,0,0,0,0,0}; /* nearby map tiles to scan */

    /* Lets check the mob has a valid map. 
     * Monsters should be able to live in containers and sense what
     * is going on around them.
     * TODO: but for now we just do nothing, waiting for someone to
     * open the container.
     */
    if(!op->map)
    {
        /* Removed the BUG info and object removal. This isn't a bug - Gecko 
        LOG(llevDebug,"BUG:: ai_look_for_other_mobs(): Mob %s without map - deleting it (%d,%d)\n", 
            query_name(op), op->env?op->env->x:-1, op->env?op->env->y:-1);
        remove_ob(op);
        */
        return;
    }
    
    /* TODO possibility for optimization: if we already have enemies there
     * is no need to look for new ones every timestep... */
    /* TODO: optimization: maybe first look through nearest squares to see if something interesting is there,
     * then search the active list */
    /* TODO: adaptive algo: if many objects in nearby maps' active lists (approx > r^2, r = sense range)
     * then it is probably faster to scan through the map cells, especially if using the
     * IS_ALIVE flag on map cell level to see if it is useful to scan that cell */

    /* Find out which other map tiles are within our sense range */
    
    /* The "real" sense range calculation is in mob_can_see_ob(), this is 
     * a simplified version */
    sense_range = op->stats.Wis;
    if (op->enemy)
        sense_range += 6;
    if (QUERY_FLAG(op, FLAG_SLEEP) || QUERY_FLAG(op, FLAG_BLIND))
        sense_range /= 2;
    
    if (op->y - sense_range < 0) 
        check_maps[0] = 1; /* North */
    if (op->y + sense_range >= MAP_HEIGHT(op->map)) 
        check_maps[2] = 1; /* South */
    if (op->x - sense_range < 0) {
        check_maps[3] = 1; /* West */
        if (check_maps[0])
            check_maps[7] = 1; /* Northwest */
        if (check_maps[2])
            check_maps[6] = 1; /* Southwest */
    }
    if (op->x + sense_range >= MAP_WIDTH(op->map)) {
        check_maps[1] = 1; /* East */
        if (check_maps[0])
            check_maps[4] = 1; /* Northeast */
        if (check_maps[2])
            check_maps[5] = 1; /* Southeast */
    }
    
    /* Scan for mobs and players in each marked map */
    for (tilenr=0; tilenr < TILED_MAPS + 1; tilenr++)
    {
        object *obj;
        if(tilenr == TILED_MAPS)
            obj = op->map->active_objects; /* Always scan op's map */
        else if (op->map->tile_map[tilenr] && op->map->tile_map[tilenr]->in_memory == MAP_IN_MEMORY && check_maps[tilenr])
            obj = op->map->tile_map[tilenr]->active_objects;
        else
            continue;

        /* TODO: swap in nearby maps? (that might cascade in turn if the loaded maps contain mobs!) */
        /* Normally, we should never do swap in maps for mobs. Because the main feature of the 
         * engine is, to have parts swaped out. But we need a "FLAG_SWAP_LOCK" flag for special
         * mobs (for example quest mobs) who run around. We don't want that the swap function swaped
         * them out before the quest is finished. These flaged mobs should handled as players in
         * map questions - that means too to swap maps in. MT-07.2005
         */
        for (; obj; obj = obj->active_next)
        {
            if ((QUERY_FLAG(obj, FLAG_ALIVE) || obj->type == PLAYER)
            //        && on_same_map(obj, op) // This is implicit from scanning only nearby maps
                    && obj != op
                    && mob_can_see_obj(op, obj, NULL))
            {
                /* TODO: get rid of double rv calculation
                 * (both can_see_obj() and register_npc_known_obj)
                 */
                register_npc_known_obj(op, obj, 0);
            }
        }
    }
}

/* Calculate friendship level of each known mob */
void ai_friendship(object *op, struct mob_behaviour_param *params)
{
    struct mob_known_obj   *tmp;

    for (tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
    {
        tmp->tmp_friendship = tmp->friendship;

        /* Replace with flexible behaviour parameters */
        if (is_enemy_of(op, tmp->obj))
            tmp->tmp_friendship += FRIENDSHIP_ATTACK;
        else if (is_friend_of(op, tmp->obj))
            tmp->tmp_friendship += FRIENDSHIP_HELP;

        /* Helps us focusing on a single enemy */
        if (tmp == MOB_DATA(op)->enemy)
            tmp->tmp_friendship += FRIENDSHIP_ENEMY_BONUS;

        /* Now factor in distance  */
        if (get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE))
        {
            tmp->tmp_friendship += (FRIENDSHIP_DIST_MAX / (int) MAX(tmp->rv.distance, 1.0)) * SGN(tmp->tmp_friendship);
        }
        /* TODO: test last_seen aging */
        //        tmp->tmp_friendship /= MAX(global_round_tag - tmp->last_seen, 1);
        //        LOG(llevDebug,"ai_friendship(): '%s' -> '%s'. friendship: %d\n",  STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp->obj), tmp->tmp_friendship);
    }
}

/* TODO: parameterize MAX_IDLE_TIME */
#define MAX_IDLE_TIME 5
void ai_choose_enemy(object *op, struct mob_behaviour_param *params)
{
    object                 *oldenemy    = op->enemy;
    struct mob_known_obj   *tmp, *worst_enemy = NULL;

    /* Go through list of known mobs and choose the most hated
     * that we can get to.
     */
    for (tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
    {
        if (tmp->tmp_friendship < 0)
        {
            /* Most hated enemy so far? */
            if ((worst_enemy == NULL || tmp->tmp_friendship < worst_enemy->tmp_friendship))
            {
                /* Ignore if we can't get to it at all */
                /* TODO: should probably use get_last_known_obj_rv() */
                rv_vector  *rv  = get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE);
                if (rv)
                {
                    op->anim_enemy_dir = rv->direction;
                    worst_enemy = tmp;
                }
            }
        }
    }

    /* Did we find an enemy? */
    if (worst_enemy)
    {
        //        LOG(llevDebug,"ai_choose_enemy(): %s's worst enemy is '%s', friendship: %d\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(worst_enemy->ob), worst_enemy->tmp_friendship);
        op->enemy = worst_enemy->obj;
        MOB_DATA(op)->enemy = worst_enemy;
        op->enemy_count = worst_enemy->obj_count;
    }
    else
    {
        op->enemy = NULL;
        MOB_DATA(op)->enemy = NULL;
    }

    /* Handle enemy switching (growl, speed up/down) */
    /* TODO: separate into another behaviour? */
    if (op->enemy != oldenemy)
    {
        MOB_DATA(op)->idle_time = 0;
        if (op->enemy)
        {
            // Is this actually referenced to anywhere else? - Gecko 20050713
//            op->last_eat = 0;   /* important: thats our "we lose aggro count" - reset to zero here */
            if (!QUERY_FLAG(op, FLAG_FRIENDLY) && op->map)
                play_sound_map(op->map, op->x, op->y, SOUND_GROWL, SOUND_NORMAL);

            /* The unaggressives look after themselves 8) */
            /* TODO: Make a separate behaviour... */
            //            if(QUERY_FLAG(op,FLAG_UNAGGRESSIVE)) {
            //                CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
            //            npc_call_for_help(op);
            //            }
        }
        set_mobile_speed(op, 0);
    }
    
/* TODO: disabled until somone comes up with a test case where this 
 * actually happens before pathfinding fails. */
#if 0    
    else if(MOB_DATA(op)->enemy && MOB_DATA(op)->idle_time > MAX_IDLE_TIME && MOB_DATA(op)->enemy->tmp_friendship < 0)
    {
        LOG(llevDebug, "ai_choose_enemy(): %s too bored getting to %s, downgrading its enemy status\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));
        MOB_DATA(op)->enemy->tmp_friendship = 0;
        MOB_DATA(op)->enemy->friendship = 0;
        /* Go through the mob list yet again (should only be done once) */
        /* TODO: keep track of second_worst_enemy instead... */
        ai_choose_enemy(op, params);
    }
#endif    
}

/* AI <-> plugin interface for processes */
void ai_plugin_process(object *op, struct mob_behaviour_param *params)
{
#ifdef PLUGINS
    CFParm  CFP;
    int     k, l, m;
    k = EVENT_AI_BEHAVIOUR;
    l = 0; /* SCRIPT_FIX_ALL; */ /* Script fix none */
    m = 0;
    CFP.Value[0] = &k;   /* Event type */
    CFP.Value[1] = NULL; /* Activator */
    CFP.Value[2] = op;   /* Me */
    CFP.Value[3] = NULL; /* Other */
    CFP.Value[4] = NULL; /* Message */
    CFP.Value[5] = &m;
    CFP.Value[6] = &m;
    CFP.Value[7] = &m;
    CFP.Value[8] = &l; /* Fix settings */
    CFP.Value[9] = (char *)AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_BEHAVIOUR);   /* file */
    CFP.Value[10] = (char *)AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_OPTIONS);
    CFP.Value[11] = NULL;
    PlugList[findPlugin(AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_PLUGIN))].eventfunc (&CFP);
#endif
}

/*
 * Attack behaviours
 */
int ai_melee_attack_enemy(object *op, struct mob_behaviour_param *params)
{
    rv_vector  *rv;

    if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE)
     || QUERY_FLAG(op, FLAG_SCARED)
     || !OBJECT_VALID(op->enemy, op->enemy_count)
     || op->weapon_speed_left > 0
     || op->map == NULL)
        return FALSE;

    /* TODO: choose another enemy if this fails */
    if (!(rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    if (!can_hit_melee(rv->part, op->enemy, rv) || !mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        return FALSE;

    //    LOG(llevDebug,"ai_melee_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));

    /* TODO: the following test should be done in skill_attack! */
    /* TODO: what if wc underflows? */
    op->anim_enemy_dir = rv->direction;
    if (QUERY_FLAG(op, FLAG_RUN_AWAY))
        rv->part->stats.wc -= 10;
    skill_attack(op->enemy, rv->part, 0, NULL);
    op->weapon_speed_left += FABS((int) op->weapon_speed_left) + 1;
    if (QUERY_FLAG(op, FLAG_RUN_AWAY))
        rv->part->stats.wc += 10;

    return TRUE;
}

int ai_bow_attack_enemy(object *op, struct mob_behaviour_param *params)
{
    object*bow =    NULL, *arrow = NULL, *target = op->enemy;
    rv_vector      *rv;
    int             tag;
    int             direction;

    if (!QUERY_FLAG(op, FLAG_READY_BOW)
     || QUERY_FLAG(op, FLAG_UNAGGRESSIVE)
     || QUERY_FLAG(op, FLAG_SCARED)
     || !OBJECT_VALID(op->enemy,
                      op->enemy_count)
     || op->weapon_speed_left > 0
     || op->map == NULL)
        return FALSE;

    /* TODO: choose another target if this or next test fails */
    if (!(rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    /* TODO: also check distance and LOS */
    if (!can_hit_missile(op, target, rv, 2) || !mob_can_see_obj(op, target, MOB_DATA(op)->enemy))
        return FALSE;

    //    LOG(llevDebug,"ai_distance_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));

    op->anim_enemy_dir = rv->direction;
    direction = rv->direction;
    if (QUERY_FLAG(op, FLAG_CONFUSED))
        direction = absdir(direction + RANDOM() % 5 - 2);

    /* Find the applied bow */
    for (bow = op->inv; bow != NULL; bow = bow->below)
	{
		if (bow->type == BOW && QUERY_FLAG(bow, FLAG_APPLIED))
		{
			/* Select suitable arrows */
		    if(bow->sub_type1 == 128) /* ability bow type */
				arrow = bow->inv;
			else
				arrow = find_arrow(op, bow->race);

			if(!arrow)
			{
				/* Out of arrows , unapply bow */
			    manual_apply(op, bow, 0);
				bow = NULL;
			}
		    break;
		}
	}

    if (bow == NULL)
    {
        /*LOG(llevBug, "BUG: Monster %s (%d) has READY_BOW without bow.\n", STRING_OBJ_NAME(op), op->count);*/
        CLEAR_FLAG(op, FLAG_READY_BOW);
        return 0;
    }

    /* thats a infinitve arrow! dupe it. */
    if (QUERY_FLAG(arrow, FLAG_SYS_OBJECT))
    {
        object *new_arrow   = get_object();
        copy_object(arrow, new_arrow);
        CLEAR_FLAG(new_arrow, FLAG_SYS_OBJECT);
        new_arrow->nrof = 0;

        /* now setup the self destruction */
        new_arrow->stats.food = 20;
        arrow = new_arrow;
    }
    else
        arrow = get_split_ob(arrow, 1);

    /* ugly nasty arrow-setting-upping block */
    set_owner(arrow, op);
    arrow->direction = direction;
    arrow->x = rv->part->x,arrow->y = rv->part->y;
    arrow->speed = 1;
    update_ob_speed(arrow);
    arrow->speed_left = 0;
    SET_ANIMATION(arrow, (NUM_ANIMATIONS(arrow) / NUM_FACINGS(arrow)) * rv->direction);
    arrow->level = op->level;
    arrow->last_heal = arrow->stats.wc; /* save original wc and dam */
    arrow->stats.hp = arrow->stats.dam;
    arrow->stats.dam += bow->stats.dam + bow->magic + arrow->magic; /* NO_STRENGTH */
    arrow->stats.dam = FABS((int) ((float) (arrow->stats.dam * lev_damage[op->level])));
    arrow->stats.wc = 10 + (bow->magic + bow->stats.wc + arrow->magic + arrow->stats.wc + op->level);
    arrow->map = op->map;
    arrow->last_sp = 12; /* we use fixed value for mobs */
    SET_FLAG(arrow, FLAG_FLYING);
    SET_FLAG(arrow, FLAG_IS_MISSILE);
    SET_FLAG(arrow, FLAG_FLY_ON);
    SET_FLAG(arrow, FLAG_WALK_ON);
    tag = arrow->count;
    arrow->stats.grace = arrow->last_sp;
    arrow->stats.maxgrace = 60 + (RANDOM() % 12);

    if (insert_ob_in_map(arrow, op->map, op, 0))
    {
        play_sound_map(arrow->map, arrow->x, arrow->y, SOUND_THROW, SOUND_NORMAL);
        move_arrow(arrow);
    }

    op->weapon_speed_left += FABS((int) op->weapon_speed_left) + 1;

    /* hack: without this, a monster with a bow is invinsible by a non range monster
     * with same speed. It simply runs away, can't be catched but will range kill
     * the other. Thats not what we want.
     * To remove this speed thingy, we need a flag for the AI which skips the movement
     * phase after a cast/arrow action.
     * At last we must skip a action which brings the mob out of range ... so, a movement
     * to the enemy should be allowed. This is not a question of reality of not - this will
     * destroy not only game play but also every map design and is a critical misbehaviour.
     */
    op->speed_left--;
    return 1;
}

#define MAX_KNOWN_SPELLS 20

/* TODO: slightly rework this */
object * monster_choose_random_spell(object *monster)
{
    object *altern[MAX_KNOWN_SPELLS];
    object *tmp;
    spell  *spell;
    int     i = 0, j;

    for (tmp = monster->inv; tmp != NULL; tmp = tmp->below)
        if (tmp->type == ABILITY || tmp->type == SPELLBOOK)
        {
            /*  Check and see if it's actually a useful spell */
            if ((spell = find_spell(tmp->stats.sp)) != NULL
             && !(spell->path & (PATH_ABJURATION | PATH_TRANSMUTATION | PATH_LIGHT)))
            {
                if (tmp->stats.maxsp)
                    for (j = 0; i < MAX_KNOWN_SPELLS && j < tmp->stats.maxsp; j++)
                        altern[i++] = tmp;
                else
                    altern[i++] = tmp;
                if (i == MAX_KNOWN_SPELLS)
                    break;
            }
        }

    if (!i)
        return NULL;

    return altern[RANDOM() % i];
}

int ai_spell_attack_enemy(object *op, struct mob_behaviour_param *params)
{
    object     *target  = op->enemy;
    rv_vector  *rv;
    spell      *sp;
    object     *spell_item;
    int         sp_type, ability, direction, sp_cost;

    if (QUERY_FLAG(op, FLAG_UNAGGRESSIVE)
     || QUERY_FLAG(op, FLAG_SCARED)
     || !QUERY_FLAG(op, FLAG_READY_SPELL)
     || !OBJECT_VALID(op->enemy,
                      op->enemy_count)
     || op->weapon_speed_left > 0
     || op->last_grace > 0
     || op->map == NULL)
        return FALSE;

    /* TODO: choose another target if this or next test fails */
    if (!(rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    /* TODO: also check distance and LOS */
    /* TODO: should really check type of spell (area or missile) */
    if (!can_hit_missile(op, target, rv, 2) || !mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        return FALSE;

    //    LOG(llevDebug,"ai_distance_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));

    op->anim_enemy_dir = rv->direction;
    direction = rv->direction;
    if (QUERY_FLAG(op, FLAG_CONFUSED))
        direction = absdir(direction + RANDOM() % 5 - 2);

    /* Find a reasonable spell  */
    if ((spell_item = monster_choose_random_spell(op)) == NULL)
    {
        LOG(llevDebug, "ai_spell_attack_enemy() No spell found! Turned off spells in %s (%s) (%d,%d)\n",
            STRING_OBJ_NAME(op), STRING_MAP_NAME(op->map), op->x, op->y);
        CLEAR_FLAG(op, FLAG_READY_SPELL); /* Will be turned on when picking up book */
        return 0;
    }

    if (spell_item->stats.hp)
    {
        /* Alternate long-range spell: check how far away enemy is */
        if (rv->distance > 6)
            sp_type = spell_item->stats.hp;
        else
            sp_type = spell_item->stats.sp;
    }
    else
        sp_type = spell_item->stats.sp;

    if ((sp = find_spell(sp_type)) == NULL)
    {
        LOG(llevDebug, "ai_spell_attack_enemy(): Can't find spell #%d for mob %s (%s) (%d,%d)\n", sp_type,
            STRING_OBJ_NAME(op), STRING_MAP_NAME(op->map), op->x, op->y);
        return 0;
    }

    sp_cost = SP_level_spellpoint_cost(op, op, sp_type);
    if (op->stats.sp < sp_cost)
        return FALSE;

    /* TODO: this kind of spells shouldn't be handled here... */
    if (sp->flags & SPELL_DESC_SELF) /* Spell should be cast on caster (ie, heal, strength) */
        direction = 0;

    ability = (spell_item->type == ABILITY && QUERY_FLAG(spell_item, FLAG_IS_MAGICAL));

    /* If we cast a spell, only use up casting_time speed */
    op->speed_left += (float) 1.0 - (float) spells[sp_type].time / (float) 20.0 * (float) FABS(op->speed);

    op->stats.sp -= sp_cost;

    /* add default cast time from spell force to monster */
    /* TODO: what is this? */
    op->last_grace += spell_item->last_grace;

    /*LOG(-1,"CAST2: dir:%d (%d)- target:%s\n", dir, rv->direction, query_name(op->enemy) );*/
    /* TODO: what does the return value of cast_spell do ? */
    cast_spell(rv->part, rv->part, direction, sp_type, ability, spellNormal, NULL);

    op->speed_left--;/* hack: see bow bahaviour! */
    return TRUE;
}

/* AI <-> plugin interface for actions */
int ai_plugin_action(object *op, struct mob_behaviour_param *params)
{
    int ret = 0;
#ifdef PLUGINS
    CFParm  CFP, * retCFP;
    int     k, l, m;
    k = EVENT_AI_BEHAVIOUR;
    l = 0; /* SCRIPT_FIX_ALL; */ /* Script fix none */
    m = 0;
    CFP.Value[0] = &k;   /* Event type */
    CFP.Value[1] = NULL; /* Activator */
    CFP.Value[2] = op;   /* Me */
    CFP.Value[3] = NULL; /* Other */
    CFP.Value[4] = NULL; /* Message */
    CFP.Value[5] = &m;
    CFP.Value[6] = &m;
    CFP.Value[7] = &m;
    CFP.Value[8] = &l; /* Fix settings */
    CFP.Value[9] = (char *) AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_BEHAVIOUR);   /* file */
    CFP.Value[10] = (char *) AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_OPTIONS);
    CFP.Value[11] = NULL;
    retCFP = PlugList[findPlugin(AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_PLUGIN))].eventfunc (&CFP);
    ret = *(int *)retCFP->Value[0];
#endif

    return ret;
}
