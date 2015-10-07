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
 * This file contains the actual mob AI behaviours and their
 * utility functions
 */

#include <global.h>

#include <aiconfig.h>

/* Those behaviours are called from other behaviours (ugly, I know...) */
void ai_choose_enemy(object_t *op, struct mob_behaviour_param *params);
void ai_move_towards_owner(object_t *op, struct mob_behaviour_param *params, move_response *response);

/*
 * A few random unsorted utility functions
 */

/* Beginnings of can_see_obj */
/* known_obj is optional but increases efficiency somewhat
 * by using caching data in the known_obj struct
 */
int mob_can_see_obj(object_t *op, object_t *obj, struct mob_known_obj *known_obj)
{
    int     aggro_range,
            stealth_range;

    /* Cache values */
    static tag_t cached_op_tag, cached_obj_tag;
    static uint32 cache_time;
    static int cached_result;

    rv_t   rv, *rv_p = NULL;

    /* Quick answer if possible */
    if (known_obj && known_obj->last_seen == ROUND_TAG)
        return TRUE;

    /* Pets can always see their owners */
    if (get_owner(op) == obj)
        return TRUE;

    /* Try using cache */
    if (cached_op_tag == op->count && cached_obj_tag == obj->count &&
                    cache_time == ROUND_TAG)
        return cached_result;

    /* Gmaster with stealth? */
    if (IS_GMASTER_STEALTH(obj) ||
        IS_GMASTER_INVIS(obj))
    {
        return FALSE;
    }

    /* Invisibility */
    if (IS_NORMAL_INVIS_TO(obj, op))
        return FALSE;

    /* Legal position? */
    if(! obj->map)
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
    else if (RV_GET_OBJ_TO_OBJ(op, obj, &rv, RV_EUCLIDIAN_DISTANCE))
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

/* TODO: make a real behaviour... */
#if 0
void npc_call_for_help(object_t *op) {
  struct mob_known_obj *friend;

  /* TODO: remember to check that the called has MOB_DATA set up before doing anything else... */
  /* TODO: use tmp_friendship? */
  for(friend = MOB_DATA(op)->friends; friend; friend=friend->next) {
      if(friend->friendship >= FRIENDSHIP_HELP && friend->ob->enemy == NULL)  {
          rv_t *rv = get_known_obj_rv(op, friend, MAX_KNOWN_OBJ_RV_AGE);
          if(rv && rv->distance < 4) {
              /* TODO: also check reverse friendship */
              /* TODO: -friendship here dependant on +friendship towards tmp */
              register_npc_known_obj(friend->ob, op->enemy, FRIENDSHIP_ATTACK, 0);
          }
      }
  
}
#endif

/** Callback function for successful moves toward a waypoint.
 * Updates the monster's "home" position to its current position,
 * if it got closer to the waypoint than it has been before */
static void update_home_position_for_wp_move(object_t *op, int last_movement_dir)
{
    /* TODO: this needs more testing */
//    LOG(llevDebug, "%s: best dist %d, last best: %d\n", STRING_OBJ_NAME(op),MOB_PATHDATA(op)->best_distance, MOB_PATHDATA(op)->last_best_distance);
    if(MOB_PATHDATA(op)->best_distance <= 1
            || MOB_PATHDATA(op)->last_best_distance == -1
            || MOB_PATHDATA(op)->best_distance <  MOB_PATHDATA(op)->last_best_distance)
    {
        object_t *base = insert_base_info_object(op);
        base->x = op->x;
        base->y = op->y;
        FREE_AND_ADD_REF_HASH(base->slaying, op->map->orig_path);
//        LOG(llevDebug, "Updating home for '%s'\n", STRING_OBJ_NAME(op));
    }
}

/*
 * Support functions for combat strength and fear calculations
 */

/** Let op to estimate other's combat strength.
 * op's attributes should matter a lot here, for example:
 * - lower level lifeforms shouldn't do any estimation at all
 *   (e.g. puddings, insects, etc)
 * - most animals should only consider physical attributes such
 *   as apparent size and strength
 * - more intelligent mobs should also consider equipment and
 *   use of magic.
 * @return estimated combat strength, always => 0.
 *  A defenseless tiny creature (smoething like a real-life fly)
 *  should have a combat strength of 0.
 */
int estimate_combat_strength(object_t *op, object_t *other)
{
    int level = other->level;
    int health = (op->stats.hp * 100) / op->stats.maxhp; /* health in % */
    int cs;

/* TODO: need an efficient way of detecting poison or (deadly) sickness
 * if(QUERY_FLAG(other, FLAG_POISONED))
        other->health -= 10;
        */
    health = CLAMP(health, 0, 100);

    /* TODO: incredibly basic algorithm for now */
    cs = level + health;
    return MAX(0, cs);
}

/** Give the relative perceived combat strength between
 * op and other in percent.
 * @return around 100 - matching combat strength,
 *   >100 other is stronger (200: other is twice as strong as op),
 *   <100 op is stronger (50: op is twice as strong as other).
 */
int relative_combat_strength(object_t *op, object_t *other)
{
    /* Update own combat strength if needed */
    if(MOB_DATA(op)->combat_strength == -1)
    {
        MOB_DATA(op)->combat_strength = estimate_combat_strength(op, op);
        /* Make sure never <= 0 */
        MOB_DATA(op)->combat_strength = MAX(MOB_DATA(op)->combat_strength, 1);
    }

    return estimate_combat_strength(op, other) * 100 / MOB_DATA(op)->combat_strength;
}

/** Assess tactical situation. Calculates a "tactical center" which is
 * the vector sum of all friends and enemies and our fear/attraction for them.
 * @param op object to calculate the center for
 * @param dx return variable for relative x location of attraction center
 * @param dy return variable for relative y location of attraction center
 * @return attraction sum. if negative, the coordinate is dangerous.
 *         if positive, the coordinate is safe. The size of the value
 *         indicates the relative combat strength around the point.
 */
int assess_tactical_situation(object_t *op, int *dx, int *dy)
{
    struct mob_known_obj *tmp;
    int weight_sum = 0;

    unsigned int max_distance = 10; /* TODO: parameterize? */

    *dx = 0;
    *dy = 0;

    for(tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
    {
        rv_t *rv = get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE);
        if(rv && rv->distance < max_distance)
        {
            /* Let the attraction/fear fall off by the square of the distance */
            int weight = tmp->tmp_attraction - (tmp->tmp_attraction * (int)(rv->distance * rv->distance)) / (max_distance * max_distance);
            *dx += rv->distance_x * weight;
            *dy += rv->distance_y * weight;
            LOG(llevDebug, "%s -> %s, attraction: %d, distance: %d, weight: %d\n",
                    STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp->obj), tmp->tmp_attraction, rv->distance, weight);
            weight_sum += weight;
        }
    }

    if(weight_sum != 0)
    {
        *dx /= weight_sum;
        *dy /= weight_sum;
    }

    return weight_sum;
}

/*
 * Attraction/friendship pseudobehaviours. These are configured like
 * normal behaviours, but called from the sensing functions.
 */

/** Pseudobehaviour to calculate the attraction/repulsion of one monster towards another object.
 * Attraction is controlled by the "attraction" behaviour.
 * @param op the monster to calculate attraction for
 * @param other the object to calculate attraction towards
 * @return a positive (attraction) or negative (repulsion) value, or zero (indifference / "don't care")
 * @todo there's much duplicated code between this and get_npc_mob_attitude().
 * Any suggestions?
 * @see the "attraction" behaviour.
 */
int get_npc_object_attraction(object_t *op, object_t *other)
{
    int attraction = 0;
    struct mob_behaviour_param *attractions;
    struct mob_behaviour_param *tmp;

    if(op->head)
        op = op->head;

    if(op->type != MONSTER)
    {
        LOG(llevBug, "BUG: get_npc_object_attraction() object %s is not a monster (type=%d)\n",
                STRING_OBJ_NAME(op), op->type);
    }

    attractions = MOB_DATA(op)->behaviours->attractions;

    if(attractions == NULL)
        return attraction;

    /* Arch attraction */
    if(attractions[AIPARAM_ATTRACTION_ARCH].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attractions[AIPARAM_ATTRACTION_ARCH]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->arch->name && tmp->stringvalue == other->arch->name)
                attraction += tmp->intvalue;
        }
    }

    /* Named object attitude */
    if(attractions[AIPARAM_ATTRACTION_NAME].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attractions[AIPARAM_ATTRACTION_NAME]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->name && tmp->stringvalue == other->name)
                attraction += tmp->intvalue;
        }
    }

    /* Numbered type attitude */
    if(attractions[AIPARAM_ATTRACTION_TYPE].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attractions[AIPARAM_ATTRACTION_TYPE]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->type == atoi(tmp->stringvalue))
                attraction += tmp->intvalue;
        }
    }

    /* cursed object attitude */
    if(attractions[AIPARAM_ATTRACTION_CURSED].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attractions[AIPARAM_ATTRACTION_CURSED]; tmp != NULL;
                tmp = tmp->next)
        {
            if(is_cursed_or_damned(other))
                attraction += tmp->intvalue;
        }
    }

    /* Numbered type and cursed attitude */
    if(attractions[AIPARAM_ATTRACTION_CURSEDTYPE].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attractions[AIPARAM_ATTRACTION_CURSEDTYPE]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->type == atoi(tmp->stringvalue) && is_cursed_or_damned(other))
                attraction += tmp->intvalue;
        }
    }

    /* Player attitude */
    if(attractions[AIPARAM_ATTRACTION_PLAYER].flags & AI_PARAM_PRESENT)
        if(other->type == PLAYER)
            attraction += attractions[AIPARAM_ATTRACTION_PLAYER].intvalue;

    return attraction;
}


/** Help function to calculate the base friendship/hate ("attitude") of one monster towards another.
 * Attitude is partly controlled by the "friendship" behaviour parameters, but also
 * by "petness". If there's no "friendship" behaviour to get parameters from, it falls back to
 * the alignment ("friendly"/"non-friendly" flag).
 * Friendship is also dynamically modified by mobs' actions, so this function is only part of
 * the puzzle.
 *
 * @param op the monster/player to calculate friendship for
 * @param other the object to calculate friendship towards
 * @return a positive (friendship) or negative (hate) value, or zero (neutral)
 * @see the "friendship" behaviour.
 */
int get_npc_attitude(object_t *op, object_t *other)
{
    int friendship = 0;
    object_t *owner;
    struct mob_behaviour_param *attitudes;
    struct mob_behaviour_param *tmp;

    if(op == NULL || other == NULL)
    {
        LOG(llevBug, "BUG: get_npc_mob_attitude('%s', '%s') with NULL parameter\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(other));
        return 0;
    }

    if(op->head)
        op = op->head;
    if(other->head)
        other = other->head;

    /* If we are asked about a player's attitude to something,
     * see if we can do a reverse lookup */
    if(op->type != MONSTER)
    {
        LOG(llevBug, "BUG: get_npc_attitude('%s','%s') called for non-monster\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(other));
    }
    else if(MOB_DATA(op) == NULL)
    {
        LOG(llevDebug, "Warning: AI not initialized when requesting attitude of monster '%s' towards '%s'.\n",
            STRING_OBJ_NAME(op), STRING_OBJ_NAME(other));
        return 0;
    }

    /* pet to pet and pet to owner attitude */
    if ((owner = get_owner(op)))
    {
        if (owner == get_owner(other))
        {
            friendship += FRIENDSHIP_HELP;
        }
        else if  (owner == other &&
                  owner->count == other->count)
        {
            friendship += FRIENDSHIP_PET;
        }
    }

    attitudes = MOB_DATA(op)->behaviours->attitudes;

    if(attitudes == NULL)
    {
        /* Default alignment handling */
        if (QUERY_FLAG(op, FLAG_FRIENDLY) == QUERY_FLAG(other, FLAG_FRIENDLY))
            friendship += FRIENDSHIP_HELP;
        else
            friendship += FRIENDSHIP_ATTACK;

        return friendship;
    }

    /* Configurable alignment attitudes */
    if (QUERY_FLAG(op, FLAG_FRIENDLY) == QUERY_FLAG(other, FLAG_FRIENDLY))
        friendship += attitudes[AIPARAM_FRIENDSHIP_SAME_ALIGNMENT].intvalue;
    else
        friendship += attitudes[AIPARAM_FRIENDSHIP_OPPOSITE_ALIGNMENT].intvalue;

    /* Race attitude */
    if(attitudes[AIPARAM_FRIENDSHIP_RACE].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attitudes[AIPARAM_FRIENDSHIP_RACE]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->race && tmp->stringvalue == other->race)
                friendship += tmp->intvalue;
        }
    }

    /* Arch attitude */
    if(attitudes[AIPARAM_FRIENDSHIP_ARCH].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attitudes[AIPARAM_FRIENDSHIP_ARCH]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->arch->name && tmp->stringvalue == other->arch->name)
                friendship += tmp->intvalue;
        }
    }

    /* Named object attitude */
    if(attitudes[AIPARAM_FRIENDSHIP_NAME].flags & AI_PARAM_PRESENT)
    {
        for(tmp = &attitudes[AIPARAM_FRIENDSHIP_NAME]; tmp != NULL;
                tmp = tmp->next)
        {
            if(other->name && tmp->stringvalue == other->name)
                friendship += tmp->intvalue;
        }
    }

    /* Named group attitude */
    if(attitudes[AIPARAM_FRIENDSHIP_GROUP].flags & AI_PARAM_PRESENT)
    {
        /* Make sure other is a monster that belongs to one or more groups */
        if(other->type == MONSTER && MOB_DATA(other) &&
                MOB_DATA(other)->behaviours->groups &&
                MOB_DATA(other)->behaviours->groups[AIPARAM_GROUPS_NAME].flags & AI_PARAM_PRESENT)
        {
            /* Match my group attitudes to the other's group memberships */
            for(tmp = &attitudes[AIPARAM_FRIENDSHIP_GROUP]; tmp != NULL;
                    tmp = tmp->next)
            {
                struct mob_behaviour_param *group;

                for(group = &MOB_DATA(other)->behaviours->groups[AIPARAM_GROUPS_NAME];
                        group != NULL; group = group->next)
                {
                    if(tmp->stringvalue == group->stringvalue)
                        friendship += tmp->intvalue;
                }
            }
        }
    }

    /* Player attitude */
    if(attitudes[AIPARAM_FRIENDSHIP_PLAYER].flags & AI_PARAM_PRESENT)
    {
        if(other->type == PLAYER)
            friendship += attitudes[AIPARAM_FRIENDSHIP_PLAYER].intvalue;
    }

    /* Named god attitude */
    if(attitudes[AIPARAM_FRIENDSHIP_GOD].flags & AI_PARAM_PRESENT)
    {
        /* For now the god test is only done on players.
         * calling determine_god() on a generic mob will give that mob a
         * random god, and we don't want that to happen.
         */
        if(other->type == PLAYER)
        {
            const char *godname = determine_god(other);
            for(tmp = &attitudes[AIPARAM_FRIENDSHIP_GOD]; tmp != NULL;
                    tmp = tmp->next)
            {
                if(godname == tmp->stringvalue)
                    friendship += tmp->intvalue;
            }
        }
    }

//    LOG(llevDebug, "Attitude friendship modifier: %d (%s->%s)\n", friendship, STRING_OBJ_NAME(op), STRING_OBJ_NAME(other));

    return friendship;
}



/** Request the friendship between op and other.
* Calculates the friendship between any two players, npcs and/or
* monsters. Takes monster memory, AI settings, pet status and PvP status
* in consideration.
* @param op object to calculate friendship for
* @param other object to calculate friendship towards
* @return if friendship <= FRIENDSHIP_ATTACK then other is an enemy of op
*         if friendship >= FRIENDSHIP_HELP, then other is a friend of op
*         the values inbetween are considered neutral
* @todo this belongs to another file, but which one?
* @todo handle groups
*/
int get_friendship(object_t *op, object_t *other)
{
    struct mob_known_obj *known;

    if (!op ||
        !other)
    {
        LOG(llevBug, "BUG:: %s:get_friendship('%s', '%s'): NULL parameter!\n",
            __FILE__, STRING_OBJ_NAME(op), STRING_OBJ_NAME(other));
        return 0;
    }

    if(op->head)
        op = op->head;
    if(other->head)
        other = other->head;

    if (op == other)
    {
        return FRIENDSHIP_HELP;
    }

    switch (op->type)
    {
        case PLAYER:
        if (other->type == PLAYER)
        {
// If PvP is enabled, see if op and other are friends (same group).
#ifdef USE_PVP
            msp_t *msp_op = MSP_KNOWN(op),
                  *msp_other = MSP_KNOWN(other);

            if ((msp_op->flags & MSP_FLAG_PVP) &&
                (msp_other->flags & MSP_FLAG_PVP))
            {
                // Loop through the group and if other is in it, op and other are friends.
                if ((CONTR(op)->group_status & GROUP_STATUS_GROUP))
                {
                    object_t *member;

                    for (member = CONTR(op)->group_leader; member; member = CONTR(member)->group_next)
                    {
                        if (member == other)
                        {
                            return FRIENDSHIP_HELP;
                        }
                    }
                }

                // Otherwise, let them kill each other.
                return FRIENDSHIP_ATTACK;
            }
#endif
            return FRIENDSHIP_HELP;
        }
        /* Try reverse lookup */
        else if (other->type == MONSTER)
        {
            object_t *tmp = op;

            op = other;
            other = tmp;
        }
        else
        {
            return 0;
        }

        case MONSTER:
        /* Do we already know this other? */
        for (known = MOB_DATA(op)->known_mobs; known; known = known->next)
        {
            if (known->obj == other &&
                known->obj_count == other->count)
            {
                return known->friendship;
            }
        }

        /* Calculate it then */
        return get_npc_attitude(op, other);
    }

#ifdef DEBUG_FRIENDSHIP_WARNING
    LOG(llevDebug, "DEBUG:: %s:get_friendship() between %s[%d] (%d) and %s[%d] (%d)!\n",
        __FILE__, STRING_OBJ_NAME(op), TAG(op), op->type,
        STRING_OBJ_NAME(other), TAG(other), other->type);
#endif
    return 0;
}


/*
 * Mutually exclusive movement behaviours
 */

void ai_stand_still(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    if(op->owner)
    {
        ai_move_towards_owner(op, NULL, response);
        return;
    }

    response->type = MOVE_RESPONSE_DIR;
    response->data.direction = 0;
}

void ai_sleep(object_t *op, struct mob_behaviour_param *params, move_response *response)
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


#define dir_turn(dir, n) ((((dir) - 1 + (n)) & 7) + 1)

int ai_only_keep_possible_dirs(int aDirWeight[9], object_t *op, struct mob_behaviour_param *params){
object_t *base;
map_t *basemap;
rv_t rv;
int nXRange = 0, nYRange = 0;
unsigned int nLimitXY;
int nret;

    base = insert_base_info_object(op);

    nLimitXY = 0;

    if(params[AIPARAM_MOVE_RANDOMLY_XLIMIT].flags & AI_PARAM_PRESENT){
        nXRange = params[AIPARAM_MOVE_RANDOMLY_XLIMIT].intvalue;
        nLimitXY = 2;
    }
    
    if(params[AIPARAM_MOVE_RANDOMLY_YLIMIT].flags & AI_PARAM_PRESENT){
        nYRange = params[AIPARAM_MOVE_RANDOMLY_YLIMIT].intvalue;
        nLimitXY |= 1;
    }

        if (!(basemap = map_is_ready(base->slaying)))
        {
            basemap = ready_inherited_map(op->map, base->slaying);
        }

        if (basemap &&
            !RV_GET_MSP_TO_OBJ(MSP_GET2(basemap, base->x, base->y), op, &rv, RV_NO_DISTANCE)) /* the mob is not a controled map */
        {
            aDirWeight[0] = 0;
            aDirWeight[1] = 0;
            aDirWeight[2] = 0;
            aDirWeight[3] = 0;
            aDirWeight[4] = 0;
            aDirWeight[5] = 0;
            aDirWeight[6] = 0;
            aDirWeight[7] = 0;
            aDirWeight[8] = 0;

            switch(op->direction){
            case 0:
                break;
            case 1:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_SOUTH] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_SOUTH] == basemap->tiling.tile_map[TILING_DIRECTION_NORTH]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTH] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTH] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHWEST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;
            case 2:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_SOUTHWEST] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_SOUTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTH]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHWEST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_EAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHEAST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;
            case 3:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_WEST] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_WEST] == basemap->tiling.tile_map[TILING_DIRECTION_EAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_WEST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_WEST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHEAST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;
            case 4:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_NORTHWEST] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_NORTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTH]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_EAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHWEST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHWEST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;
            case 5:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_NORTH] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_NORTH] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTH]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTH] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTH] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHWEST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;
            case 6:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_NORTHEAST] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_NORTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTH]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHWEST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_WEST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_NORTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHWEST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;                
            case 7:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_EAST] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_EAST] == basemap->tiling.tile_map[TILING_DIRECTION_WEST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_EAST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHWEST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_EAST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHWEST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;
            case 8:
                if(    op->map->tiling.tile_map[TILING_DIRECTION_SOUTHEAST] &&
                    ((op->map->tiling.tile_map[TILING_DIRECTION_SOUTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTH]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHEAST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_SOUTHWEST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_WEST]) ||
                    (op->map->tiling.tile_map[TILING_DIRECTION_SOUTHEAST] == basemap->tiling.tile_map[TILING_DIRECTION_NORTHWEST]))){
                    aDirWeight[dir_turn(op->direction, 4)] = 1;
                    return 1;
                }
                break;
            }
            nLimitXY = 0;
    }

    aDirWeight[0] = 1;
    aDirWeight[1] = 1;
    aDirWeight[2] = 1;
    aDirWeight[3] = 1;
    aDirWeight[4] = 1;
    aDirWeight[5] = 1;
    aDirWeight[6] = 1;
    aDirWeight[7] = 1;
    aDirWeight[8] = 1;

    if(nLimitXY & 2){
        if(rv.distance_x + 1 > nXRange){
            aDirWeight[2] = 0;
            aDirWeight[3] = 0;
            aDirWeight[4] = 0;
        }
        if(rv.distance_x - 1 < -nXRange){
            aDirWeight[6] = 0;
            aDirWeight[7] = 0;
            aDirWeight[8] = 0;
        }
    }

    if(nLimitXY & 1){
        if(rv.distance_y + 1 > nYRange){
            aDirWeight[4] = 0;
            aDirWeight[5] = 0;
            aDirWeight[6] = 0;
        }
        if(rv.distance_y - 1 < -nYRange){
            aDirWeight[1] = 0;
            aDirWeight[2] = 0;
            aDirWeight[8] = 0;
        }
    }
    nret = aDirWeight[0] + aDirWeight[1] + aDirWeight[2] + aDirWeight[3] + aDirWeight[4];
    nret += aDirWeight[5] + aDirWeight[6] + aDirWeight[7] +    aDirWeight[8];
    return nret;
}

/*	fill aDirWeight[9] with possible directions 
	returns the max random number + 1
*/
int ai_move_randomly_behaviour(int aDirWeight[9], object_t *op, struct mob_behaviour_param *params){
int nStandStill = 246; /* todo: get this value from params */
int nLastDir;
int nret;

    if(!(nLastDir = op->direction)){ /* Last move is stand still */
        if((RANDOM() % 256) <= nStandStill){ /* stand still */
            aDirWeight[0] = 1;
            aDirWeight[1] = 0;
            aDirWeight[2] = 0;
            aDirWeight[3] = 0;
            aDirWeight[4] = 0;
            aDirWeight[5] = 0;
            aDirWeight[6] = 0;
            aDirWeight[7] = 0;
            aDirWeight[8] = 0;
            return 1;
        }
        if(nret = ai_only_keep_possible_dirs(aDirWeight, op, params) - 1){ /* evaluate moves in any direction */
            aDirWeight[0] = 0;
            return nret;
        }
        /* no movement possible, so don't move */
        return 1;
    } else { /* the mob was moving */
        ai_only_keep_possible_dirs(aDirWeight, op, params); /* evaluate moves in any direction */

        nret = aDirWeight[nLastDir] *= 128; /* same direction */

        nret += aDirWeight[dir_turn(nLastDir, 1)] *= 32;  /* turn on the right once */

        nret += aDirWeight[dir_turn(nLastDir, -1)] *= 32; /* turn on the left once */
        
        nret += aDirWeight[dir_turn(nLastDir, 2)] *= 4; /* turn on the right twice */

        nret += aDirWeight[dir_turn(nLastDir, -2)] *= 4; /* turn on the left twice */

        nret += aDirWeight[dir_turn(nLastDir, 3)] *= 1; /* turn on the right 3 times */

        nret += aDirWeight[dir_turn(nLastDir, -3)] *= 1; /* turn on the left 3 times */

        nret += aDirWeight[dir_turn(nLastDir, 4)] *= 1; /* turn on the right 4 times */

        nret += aDirWeight[0] *= 4; /* don't move */
        
        return nret;
    }
}

void ai_move_randomly(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    int aDirWeight[9]; /* 8 directions + stand still */
    int nMaxRand;
    int n, i;

    if (op->owner)
    {
        ai_move_towards_owner(op, NULL, response);
        return;
    }

    nMaxRand = ai_move_randomly_behaviour(aDirWeight, op, params);

    while (nMaxRand)
    {
        n = RANDOM() % nMaxRand;

        for (i = 0; i < 9; i++)
        {
            if ((n -= aDirWeight[i]) < 0)
            {
                if (!msp_blocked(op, NULL, OVERLAY_X(i), OVERLAY_Y(i)))
                {
                    response->type = MOVE_RESPONSE_DIR;
                    response->data.direction = i;
                    /* LOG(llevDebug, "move_randomly(): name: %s, dir: %d\n", op->name, i);*/
                    //            LOG(llevDebug, "move_randomly(): i=%d, dirs={%d,%d,%d,%d,%d,%d,%d,%d}\n",
                    //                    i, dirs[0], dirs[1], dirs[2], dirs[3], dirs[4], dirs[5], dirs[6], dirs[7]);
                    return;
                }

                nMaxRand -= aDirWeight[i];
                aDirWeight[i] = 0;
                break;
            }
        }
    }

    response->type = MOVE_RESPONSE_DIR;
    response->data.direction = 0;
}


/* This behaviour is also called from some terminal move behaviours to
 * allow charming of normal monsters without changing their behaviours */
void ai_move_towards_owner(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    rv_t *rv;
    object_t *base = insert_base_info_object(op);

    if(! OBJECT_VALID(op->owner, op->owner_count) || MOB_DATA(op)->owner == NULL)
    {
        if(op->owner)
            op->owner = NULL;
        return;
    }

    rv = get_known_obj_rv(op, MOB_DATA(op)->owner, MAX_KNOWN_OBJ_RV_AGE);
    if(! rv)
        return;

    /* TODO: parameterize */
    if(rv->distance < 8)
    {
        /* If very close to owner, possibly rest a little */
        if(rv->distance < 4)
        {
            int rest_chance;
            if(op->anim_moving_dir == -1)
                /* Already resting? high chance of staying. */
                rest_chance = 85;
            else
                rest_chance = 40 - rv->distance * 10;

            if((RANDOM() % 100) < rest_chance)
            {
                response->type = MOVE_RESPONSE_DIR;
                response->data.direction = 0;
                return;
            }
        }

        /* The further from owner, the lesser chance to
         * stroll randomly */
        if((RANDOM() % (8-rv->distance)) > 2)
        {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = RANDOM() % 8 + 1;
            return;
        }
    }

    response->type = MOVE_RESPONSE_OBJECT;
    response->data.target.obj = op->owner;
    response->data.target.obj_count = op->owner_count;

    /* Update the pet's home position to the owner's current position */
    base->x = op->owner->x;
    base->y = op->owner->y;
    FREE_AND_ADD_REF_HASH(base->slaying, op->owner->map->orig_path);
}

void ai_move_towards_home(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    object_t *base;

    if(op->owner)
    {
        ai_move_towards_owner(op, NULL, response);
        return;
    }

    /* TODO: optimization: pointer to the base ob in mob_data */
    if ((base = insert_base_info_object(op)) && base->slaying)
    {
        /* If mob isn't already home */
        if (op->x != base->x || op->y != base->y || op->map->orig_path != base->slaying)
        {
            map_t *map;

            if (!(map = map_is_ready(base->slaying)))
            {
                map = ready_inherited_map(op->map, base->slaying);
            }

            response->type = MOVE_RESPONSE_COORD;
            response->data.coord.x = base->x;
            response->data.coord.y = base->y;
            response->data.coord.map = map;
            MOB_DATA(op)->move_speed_factor=4; /* Move quickly towards home */
        }
    }
}

/** Investigate the nearest(?), most attractive item */
void ai_investigate_attraction(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    int max_attraction = 0;
    struct mob_known_obj *max_attractor = NULL, *tmp;

    if(MOB_DATA(op)->known_objs == NULL)
        return;

    /* Find known_obj with highest attraction. TODO: consider distance too */
    for(tmp = MOB_DATA(op)->known_objs; tmp; tmp = tmp->next)
    {
        if(tmp->attraction > max_attraction && OBJECT_VALID(tmp->obj, tmp->obj_count))
        {
            max_attraction = tmp->attraction;
            max_attractor = tmp;
        }
    }

    if(max_attractor)
    {
        rv_t  *rv  = get_known_obj_rv(op, max_attractor, MAX_KNOWN_OBJ_RV_AGE);
        LOG(llevDebug, "  %s investigating %s\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(max_attractor->obj));

        if(rv)
        {
            if (rv->distance <= 1)
            {
                response->type = MOVE_RESPONSE_DIR;
                response->data.direction = 0;

                if(MOB_DATA(op)->idle_time > 4)
                    max_attractor->attraction = 0;
            }
            else
            {
                response->type = MOVE_RESPONSE_OBJECT;
                response->data.target.obj = max_attractor->obj;
                response->data.target.obj_count = max_attractor->obj_count;
            }
        }
    }
}

/** Avoid stepping on repulsive items.
 * @todo also parameterize trigger distance
 */
void ai_avoid_repulsive_items(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    struct mob_known_obj *tmp;

    /* Find nearby repulsive known_objs */
    for(tmp = MOB_DATA(op)->known_objs; tmp; tmp = tmp->next)
    {
        if(tmp->attraction < 0)
        {
            rv_t *rv = get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE);
            if(rv && rv->distance <= 1)
            {
                response->forbidden |= (1 << rv->direction);
            }
        }
    }
}

/** Useful if mob is much slower than enemy?
 * @note experimental. not finished, tested or used */
void ai_step_back_after_swing(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    if (op->weapon_speed_left > 0 && OBJECT_VALID(op->enemy, op->enemy_count)
            && mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
    {
        rv_t  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);

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

void ai_avoid_line_of_fire(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    struct mob_known_obj *tmp;

    /* Disabled for multi-tile mobs. TODO: add support */
    if(op->more)
        return;

    /* Disable this behaviour if we are in a melee fight */
    if (OBJECT_VALID(op->enemy, op->enemy_count))
    {
        rv_t  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);
        if (rv && rv->distance <= 1)
            return;
    }

    /* Find relevant missiles */
    for(tmp = MOB_DATA(op)->known_objs; tmp; tmp = tmp->next)
    {
        object_t *missile,
                 *owner;
        int       i;
        rv_t      rv,
                 *rv_p;

        if(! QUERY_FLAG(tmp, AI_OBJFLAG_IS_MISSILE))
            continue;

        missile = tmp->obj;

        /* This happens when the owner is removed (eg, a mob archer -- or his
         * spawn point -- is on a map which times out or is killed while his
         * missile is in flight). */
        /* TODO: I'm not totally convinced there in't also a bug here so lets
         * leave a technical log message. */
        if (!(owner = get_owner(missile)))
        {
            if (!QUERY_FLAG(missile, FLAG_REMOVED))
            {
                LOG(llevInfo, "Ownerless missile (%s[%d] on %s %d,%d) removed!\n",
                    STRING_OBJ_NAME(missile), TAG(missile),
                    STRING_MAP_PATH(missile->map), missile->x, missile->y);
                remove_ob(missile);
                check_walk_off(missile, NULL, MOVE_APPLY_VANISHED);
            }

            return;
        }

        /* If the mob can't see the player who shot it, or the missile itself, don't dodge. */
        if (!mob_can_see_obj(missile, op, MOB_DATA(op)->known_objs) ||
            !mob_can_see_obj(owner, op, MOB_DATA(op)->known_objs))
        {
            return;
        }

        /* Don't always dodge - Put it to a test of WC vs. AC */
        if (random_roll(0, owner->stats.Dex) + missile->stats.wc >= op->stats.ac)
        {
            return;
        }

        switch(tmp->obj->type) {
            /* Straight-line-like missiles */
            case ARROW:
            case BULLET:
            case THROWN_OBJ:
#if 0
//            case MMISSILE:
//            case FBULLET:
//            case FBALL:
#endif
                /* Don't stand still in line of fire.
                 * TODO: should possibly do a random throw against the mob's reaction
                 * time, intelligence or somesuch */
                /* TODO: smart mobs should ignore weak missiles or anything it is immune against */
                if (RV_GET_OBJ_TO_OBJ(tmp->obj, op, &rv, RV_DIAGONAL_DISTANCE) &&
                    RV_TEST_MISSILE_EXACT(rv) &&
                    tmp->obj->direction == rv.direction)
                {
                    response->forbidden |= (1 << 0);
                }

                /* Don't move into line of fire (note: this may fail if
                 * firing into the back of an enemy. That is a good thing :) */
                for (i = -2; i <= 2; i++)
                {
                    int     d = absdir(op->direction + i);
                    map_t  *m = op->map;
                    sint16  x = op->x + OVERLAY_X(d),
                            y = op->y + OVERLAY_Y(d);
                    msp_t  *msp = MSP_GET2(m, x, y);

                    /* Avoid moving into line of fire */
                    if (msp &&
                        RV_GET_OBJ_TO_MSP(tmp->obj, msp, &rv, RV_DIAGONAL_DISTANCE) &&
                        RV_TEST_MISSILE_EXACT(rv) &&
                        tmp->obj->direction == rv.direction)
                    {
                        response->forbidden |= (1 << d);
                    }
                }

                break;
/* Currently there is no good way I can think of for cone dodging.
 * the way it is currently handled causes the mob to freeze if it can't
 * find a safe way to dodge, which makes it too easy to kill and 
 * slows the server down. */
#if 0
            /* Area-like "missiles" */
            case CONE:
                if ((rv_p = get_known_obj_rv(op, tmp, 0)))
                {
                    if(rv_p->distance == 0) {
                        response->forbidden |= (1 << 0);
                    } else {
                        /* stats.sp is the cone's direction. We check if the mob is in a
                         * cone of the found object_t */
                        if(tmp->obj->stats.sp == 0 ||
                                dirdiff(absdir(rv->direction + 4), tmp->obj->stats.sp) <= 1)
                        {
                            /* Simply try to retreat out of the way. */
                            /* TODO: add sidestepping */
                            response->forbidden |= (1 << 0);
                            response->forbidden |= (1 << absdir(rv_p->direction-2));
                            response->forbidden |= (1 << absdir(rv_p->direction-1));
                            response->forbidden |= (1 << rv_p->direction);
                            response->forbidden |= (1 << absdir(rv_p->direction+1));
                            response->forbidden |= (1 << absdir(rv_p->direction+2));
                        }
                    }
                }
                break;
#endif

            /* Note: untested and probably not very smart... */
            case POISONCLOUD:
            case LIGHTNING:
            case BOMB:
                /* TODO: Really step out of the way, not only out of the area */
                if ((rv_p = get_known_obj_rv(op, tmp, 0)))
                {
                    if (rv_p->distance <= 1)
                    {
                        response->forbidden |= (1 << rv_p->direction);
                        response->forbidden |= (1 << absdir(rv_p->direction - 1));
                        response->forbidden |= (1 << absdir(rv_p->direction + 1));
                        response->forbidden |= (1 << 0);
                    }
                }
                break;
        }
    }
}

void ai_optimize_line_of_fire(object_t *op, struct mob_behaviour_param *params, move_response *response)
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
            rv_t *rv_p;
            sint8 base_dir;

            rv_p = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);

            /* Too close or too far to care? */
            if (rv_p->distance <= 2 ||
                rv_p->distance > 8)
            {
                return;
            }

            /* Already perfect? */
            if (RV_TEST_MISSILE_EXACT(*rv_p))
            {
                good_directions = (1 << 0);
            }
            else
            {
                response->forbidden |= (1 << 0); /* Don't stay in a bad spot */
            }

            base_dir = rv_p->direction;

            /* Find a nearby good spot */
            /* TODO: can probably be calculated instead of searched for */
            /* TODO: with this algorithm there is a certain state where
             * the mob starts zipping between two "half-good" spots */
            for(i = -2; i <= 2; i++)
            {
                sint8   dir = absdir(base_dir + i);
                map_t  *m = op->map;
                sint16  x = op->x + OVERLAY_X(dir),
                        y = op->y + OVERLAY_Y(dir);
                msp_t  *msp = MSP_GET2(m, x, y);
                rv_t    rv;

                /* Find a spot in or near line of fire, and forbid movements to other spots */
                if (msp &&
                    RV_GET_OBJ_TO_MSP(op, msp, &rv, RV_DIAGONAL_DISTANCE))
                {
                    if (RV_TEST_MISSILE_EXACT(rv))
                    {
                        good_directions |= (1 << dir);
                    }
                    else if (RV_TEST_MISSILE_APPROX(rv))
                    {
                        ok_directions |= (1 << dir);
                    }
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

void ai_move_towards_enemy(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    rv_t  *rv;

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
#if 0
        LOG(llevDebug, "ai_move_towards_enemy(): %s can't get to %s, downgrading its enemy status\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));
        /* TODO: The current solution also totally disregards archers
         * and magic users that don't have to reach the target by walking */
        /* TODO: this gives some crazy results together with attitudes,
         * see the group test in the AI testmap for an example. */
        MOB_DATA(op)->enemy->friendship /= 2;
        MOB_DATA(op)->enemy->tmp_friendship = 0;

        /* Note: this may eventually make the mob forget about the enemy and go home,
         * but e.g. linked spawns will get reaggroed by their friends. */
#else
        return;
#endif
    }

    response->type = MOVE_RESPONSE_OBJECT;
    response->data.target.obj = op->enemy;
    response->data.target.obj_count = op->enemy_count;
}

void ai_keep_distance_to_enemy(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    if (OBJECT_VALID(op->enemy, op->enemy_count) && mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
    {
        rv_t  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);

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
                op->speed_left-=0.5f; /* we move backwards - do it a bit slower */
            }
            else if (rv->distance < (unsigned int) AIPARAM_INT(AIPARAM_KEEP_DISTANCE_TO_ENEMY_MAX_DIST))
            {
//                response->type = MOVE_RESPONSE_DIR;
//                response->data.direction = 0;
                response->forbidden |= (1 << rv->direction);
                response->forbidden |= (1 << absdir(rv->direction+1));
                response->forbidden |= (1 << absdir(rv->direction-1));
                op->anim_enemy_dir = rv->direction;
                op->speed_left-=0.5f; /* we move backwards - do it a bit slower */
            }
        }
    }
}

void ai_move_towards_enemy_last_known_pos(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    if (OBJECT_VALID(op->enemy, op->enemy_count) && MOB_DATA(op)->enemy->last_map)
    {
        rv_t               rv;
        struct mob_known_obj   *enemy   = MOB_DATA(op)->enemy;
        map_t              *map     = map_is_ready(enemy->last_map);

        if (map &&
            RV_GET_OBJ_TO_MSP(op, MSP_GET2(map, enemy->last_x, enemy->last_y), &rv, RV_EUCLIDIAN_DISTANCE))
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
void ai_search_for_lost_enemy(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    if (OBJECT_VALID(op->enemy, op->enemy_count) && MOB_DATA(op)->enemy->last_map)
    {
        int i, r;

        /* Give up to 8 chances for a monster to move randomly */
        for (i = 0; i < 8; i++)
        {
            r = RANDOM() % 8 + 1;

            if (!msp_blocked(op, NULL, OVERLAY_X(r), OVERLAY_Y(r)))
            {
                response->type = MOVE_RESPONSE_DIR;
                response->data.direction = r;
                return;
            }
        }
    }
}

void ai_move_towards_waypoint(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    object_t     *wp;
    rv_t   rv;
    int         try_next_wp = 0;
    object_t     *target = NULL;

    wp = get_active_waypoint(op);
    if (wp)
    {
        map_t  *destmap = NULL;
        sint16  wp_x,
                wp_y;

        if(wp->race)
        {
            target = locate_beacon(wp->race);
            if(target)
            {
                while(target->env)
                    target = target->env;
                wp_x = target->x;
                wp_y = target->y;
                destmap = target->map;
            }
            else
            {
                LOG(llevMapbug, "MAPBUG:: Couldn't find beacon %s for waypoint %s of %s[%s %d %d]!\n",
                    wp->race, STRING_OBJ_NAME(wp), STRING_OBJ_NAME(op),
                    STRING_MAP_PATH(op->map), op->x, op->y);
                CLEAR_FLAG(wp, FLAG_CURSED); /* disable this waypoint */
                try_next_wp = 1;
            }
        }
        else
        {
            wp_x = wp->stats.hp;
            wp_y = wp->stats.sp;

            if(wp->slaying)
            {
                char path[MAXPATHLEN];

                /* map_is_ready() bugs if not fed an absolute path, so
                 * make one if necessary. */
                if (*wp->slaying != '/')
                {
                    FREE_AND_COPY_HASH(wp->slaying,
                                       normalize_path(op->map->path,
                                                      wp->slaying, path));
                }

                if (!(destmap = map_is_ready(wp->slaying)))
                {
                    destmap = ready_inherited_map(op->map, wp->slaying);
                }

                if(destmap && destmap->orig_path != wp->slaying)
                    FREE_AND_ADD_REF_HASH(wp->slaying, destmap->orig_path);
            }
            else
                destmap = op->map;
        }

#ifdef DEBUG_AI_WAYPOINT
        LOG(llevDebug, "FOUND waypoint(): '%s' has active waypoint '%s'\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(wp));
#endif
        if (destmap)
        {
            /* We know which map we want to. Can we figure out where that
             * map lies relative to current position? */

            if (!RV_GET_OBJ_TO_MSP(op, MSP_GET2(destmap, wp_x, wp_y), &rv, RV_RECURSIVE_SEARCH | RV_DIAGONAL_DISTANCE))
            {
                /* Problem: we couldn't find a relative direction between the
                 * maps. Usually it means that they are in different mapsets
                 * or too far away from each other. */
                LOG(llevMapbug, "MAPBUG:: No connection between maps: '%s' and '%s'!\n",
                    STRING_MAP_PATH(destmap), STRING_MAP_PATH(op->map));
                CLEAR_FLAG(wp, FLAG_CURSED); /* disable this waypoint */
                try_next_wp = 1;
            }
            else
            {
                /* Good, we know general distance and direction to wp target */

                /* Are we close enough to accept the wp? */
                if (rv.distance <= (unsigned int) wp->stats.grace)
                {
                    /* Trigger plugin event as soon as we reach the waypoint */
                    if(MOB_PATHDATA(op)->goal_delay_counter == 0)
                        trigger_object_plugin_event(EVENT_TRIGGER,
                                wp, op, NULL,
                                NULL, NULL, NULL, NULL, SCRIPT_FIX_NOTHING);

                    /* Should we wait a little while? */
                    if (MOB_PATHDATA(op)->goal_delay_counter < wp->stats.wc)
                    {
                        MOB_PATHDATA(op)->goal_delay_counter++;
                    }
                    else
                    {
#ifdef DEBUG_AI_WAYPOINT
                        LOG(llevDebug, "ai_move_towards_waypoint(): '%s' reached destination '%s'\n",
                            STRING_OBJ_NAME(op), STRING_OBJ_NAME(wp));
#endif
                        MOB_PATHDATA(op)->goal_delay_counter = 0;
                        MOB_PATHDATA(op)->best_distance = -1;
                        MOB_PATHDATA(op)->last_best_distance = -1;
                        MOB_PATHDATA(op)->tried_steps = 0;
                        CLEAR_FLAG(wp, FLAG_CURSED);
                        try_next_wp = 1;
                    }
                }
            }
        }
        else
        {
            LOG(llevMapbug, "MAPBUG:: Couldn't find map '%s' for waypoint %s of %s[%s %d %d]!\n",
                STRING_OBJ_SLAYING(wp), STRING_OBJ_NAME(wp), STRING_OBJ_NAME(op),
                STRING_MAP_PATH(op->map), op->x, op->y);
            CLEAR_FLAG(wp, FLAG_CURSED);
            try_next_wp = 1;
        }
    }

    /* If we reached or gave up on the current waypoint */
    if (try_next_wp && wp)
    {
        if((wp = get_next_waypoint(op, wp)))
        {
#ifdef DEBUG_AI_WAYPOINT
            LOG(llevDebug, "ai_move_towards_waypoint(): '%s' next WP: '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(wp));
#endif
            SET_FLAG(wp, FLAG_CURSED); /* activate new waypoint */
            MOB_PATHDATA(op)->best_distance = -1;
            MOB_PATHDATA(op)->last_best_distance = -1;
            MOB_PATHDATA(op)->tried_steps = 0;
        }
        else
        {
#ifdef DEBUG_AI
            LOG(llevDebug, "ai_move_towards_waypoint(): '%s' no next WP\n", STRING_OBJ_NAME(op));
#endif
        }
    }

    if (wp)
    {
        response->type = MOVE_RESPONSE_WAYPOINT;
        response->data.target.obj = wp;
        response->data.target.obj_count = wp->count;
        response->success_callback = update_home_position_for_wp_move;
    }
}

/* Makes sure the mob doesn't stand still to long */
void ai_dont_stand_still(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    if(MOB_DATA(op)->idle_time >= AIPARAM_INT(AIPARAM_DONT_STAND_STILL_MAX_IDLE_TIME))
        response->forbidden |= (1 << 0);
}

/*
 * Runs away from enemy if scared
 * Sets scared if low hp
 * Clears scared if high enough hp OR after a random time
 */
void ai_run_away_from_enemy(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    rv_t  *rv;

    /* Become scared or regain senses according to hp. */
    if (op->stats.maxhp &&
        AIPARAM_PRESENT(AIPARAM_RUN_AWAY_FROM_ENEMY_HP_THRESHOLD))
    {
         uint8 is_scared = ((op->stats.hp * 100) / op->stats.maxhp <
                            AIPARAM_INT(AIPARAM_RUN_AWAY_FROM_ENEMY_HP_THRESHOLD));

         SET_OR_CLEAR_FLAG(op, FLAG_SCARED, is_scared);
    }
    /* Else low hp is not the problem so if we're scared we can stop being
     * afraid after a random delay */
    else if (QUERY_FLAG(op, FLAG_SCARED) &&
             !(RANDOM() % 20))
    {
        CLEAR_FLAG(op, FLAG_SCARED);
    }

    /* No longer scared, or no enemy (so nothing to run away from)? */
    if (!QUERY_FLAG(op, FLAG_SCARED) ||
        !op->enemy)
    {
        return;
    }

    /* Run away! Run away! */
    if ((rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
    {
        /* TODO: more intelligent: use pathfinding to find the
         * most distant point from enemy */
        response->type = MOVE_RESPONSE_DIR;
        response->data.direction = absdir(rv->direction + 4);
        op->speed_left -= 0.5f;/* let him move away "scared" - with weak legs */
    }
    else
    {
        response->type = MOVE_RESPONSE_DIR;
        response->data.direction = RANDOM() % 8 + 1; /* Run randomly */
    }
}

/*
 * Runs away from repulsive items.
 * Sets scared if repulsion is > repulsion_threshold and distance < distance_threshold
 * Clears scared a some random time after distance > distance_threshold
 */
void ai_run_away_from_repulsive_object(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    rv_t  *rv = NULL;
    struct mob_known_obj *tmp, *most_repulsive = NULL;

    for(tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
        if(most_repulsive == NULL || tmp->tmp_attraction < most_repulsive->tmp_attraction)
            most_repulsive = tmp;
    for(tmp = MOB_DATA(op)->known_objs; tmp; tmp = tmp->next)
        if(most_repulsive == NULL || tmp->tmp_attraction < most_repulsive->tmp_attraction)
            most_repulsive = tmp;

    if(!QUERY_FLAG(op, FLAG_SCARED))
    {
        if(most_repulsive &&
                most_repulsive->tmp_attraction <= AIPARAM_INT(AIPARAM_RUN_AWAY_FROM_REPULSIVE_OBJECT_REPULSION_THRESHOLD) &&
                (rv = get_known_obj_rv(op, most_repulsive, MAX_KNOWN_OBJ_RV_AGE)) &&
                (int)rv->distance <= AIPARAM_INT(AIPARAM_RUN_AWAY_FROM_REPULSIVE_OBJECT_DISTANCE_THRESHOLD))
            SET_FLAG(op, FLAG_SCARED);
    }

    if(QUERY_FLAG(op, FLAG_SCARED))
    {
        if(most_repulsive && (rv ||
                (rv = get_known_obj_rv(op, most_repulsive, MAX_KNOWN_OBJ_RV_AGE))))
        {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = absdir(rv->direction + 4);
        }
        else
        {
            response->type = MOVE_RESPONSE_DIR;
            response->data.direction = RANDOM() % 8 + 1; /* Run randomly */
        }

        /* Regain senses? FIXME: doesn't play very well with the run_away_from_enemy behaviour */
        if (!rv || (int)rv->distance > AIPARAM_INT(AIPARAM_RUN_AWAY_FROM_REPULSIVE_OBJECT_DISTANCE_THRESHOLD))
            if (!(RANDOM() % 4))
                CLEAR_FLAG(op, FLAG_SCARED);
    }
}

/*
 * Ensures the mob doesn't move too far away from its home position. Not meant for antilure
 * usage.
 */
void ai_stay_near_home(object_t *op, struct mob_behaviour_param *params, move_response *response)
{
    object_t *base;
    rv_t rv;
    int distflags;
    int maxdist = AIPARAM_INT(AIPARAM_STAY_NEAR_HOME_MAX_DIST);
    map_t  *map;

    /* Disabled for pets */
    if(op->owner)
        return;

    /* TODO: optimization: pointer to the base ob in mob_data */
    if(!(base = insert_base_info_object(op)) || !base->slaying)
        return;

    /* If mob isn't already home */
    if (op->x == base->x && op->y == base->y && op->map->orig_path == base->slaying)
        return;

    if (!(map = map_is_ready(base->slaying)))
    {
        if (!(map = ready_inherited_map(op->map, base->slaying)))
        {
            return;
        }
    }

    if(AIPARAM_INT(AIPARAM_STAY_NEAR_HOME_EUCLIDIAN_DISTANCE)) {
        maxdist *= maxdist;
        distflags = RV_FAST_EUCLIDIAN_DISTANCE;
    } else
        distflags = RV_DIAGONAL_DISTANCE;

    if (!RV_GET_OBJ_TO_MSP(op, MSP_GET2(map, base->x, base->y), &rv, distflags))
    {
        return;
    }

    if((int)rv.distance >= maxdist) {
        response->forbidden |= (1 << absdir(rv.direction+2));
        response->forbidden |= (1 << absdir(rv.direction+3));
        response->forbidden |= (1 << absdir(rv.direction+4));
        response->forbidden |= (1 << absdir(rv.direction+5));
        response->forbidden |= (1 << absdir(rv.direction+6));
    }
}

/* AI <-> plugin interface for movement behaviours */
void ai_plugin_move(object_t *op, struct mob_behaviour_param *params, move_response *response)
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
 * Processes (misc. behaviours)
 */

/* Dummy function for some special processes */
void ai_fake_process(object_t *op, struct mob_behaviour_param *params)
{
}

/** Scans the nearby area for interesting (non-zero attraction) items.
 * @todo optimization - only look at newly discovered squares after a move
 * (for example, only look at the new squares to the east after having moved east).
 * But sometimes (random interval?) look at old squares to detect dropped/thrown/spawned
 * objects without moving. Also rescan all squares if having been teleported or newly spawned.
 * We also must remember to periodically check the already known objects to see if they
 * have moved or disappeared and to update their memory timeout
 * XXX: currently very experimental */
void ai_look_for_objects(object_t *op, struct mob_behaviour_param *params)
{
    int sense_range;
    map_t *m;
    sint16 dx, dy, x, y;

    /* initialize hashtable if needed */
    if(MOB_DATA(op)->known_objs_ht == NULL)
        MOB_DATA(op)->known_objs_ht = pointer_hashtable_new(32);

    /* The "real" sense range calculation is in mob_can_see_ob(), this is
     * a simplified version */
    sense_range = op->stats.Wis;
    if (QUERY_FLAG(op, FLAG_SLEEP) || QUERY_FLAG(op, FLAG_BLIND))
        sense_range /= 2;
    sense_range = MAX(MIN_MON_RADIUS, sense_range);

    /* TODO: change to a more circular scanning area */
    for(dx = -sense_range; dx <= sense_range; dx++)
    {
        for(dy = -sense_range; dy <= sense_range; dy++)
        {
            msp_t *msp;
            object_t   *tmp,
                     *next;

            m = op->map;
            x = op->x + dx;
            y = op->y + dy;
            msp = MSP_GET(m, x, y);

            if (!msp)
            {
                continue;
            }

            FOREACH_OBJECT_IN_MSP(tmp, msp, next)
            {
                struct mob_known_obj *known;
                /* TODO: filter out pointless objects
                 * (monster, player, sys_invisible, decoration, etc) */
                if(tmp->type == MONSTER || tmp->type == PLAYER ||
                        QUERY_FLAG(tmp, FLAG_SYS_OBJECT) || tmp == op)
                    continue;
                /* TODO: what is best - to first see if the object is
                 * "interesting" or to first see if we already know it? */
                known = hashtable_find(MOB_DATA(op)->known_objs_ht, tmp);
                if(! known)
                {
                    int attraction = get_npc_object_attraction(op, tmp);
                    if(attraction) {
                        register_npc_known_obj(op, tmp, 0, attraction, 1);
#if defined DEBUG_AI
                        LOG(llevDebug, "attraction of '%s' -> '%s': %d\n",
                                STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp), attraction);
#endif
                    }

                } else
                    update_npc_known_obj(known, 0, 0);
            }
            
        }
    }
}

static void maps_in_range(object_t *op, int range, int *maps)
{
    if (op->y - range < 0)
        maps[0] = 1; /* North */
    if (op->y + range >= MAP_HEIGHT(op->map))
        maps[2] = 1; /* South */
    if (op->x - range < 0) {
        maps[3] = 1; /* West */
        if (maps[0])
            maps[7] = 1; /* Northwest */
        if (maps[2])
            maps[6] = 1; /* Southwest */
    }
    if (op->x + range >= MAP_WIDTH(op->map)) {
        maps[1] = 1; /* East */
        if (maps[0])
            maps[4] = 1; /* Northeast */
        if (maps[2])
            maps[5] = 1; /* Southeast */
    }
}

/** Scans the nearby area for enemy missiles, bullets, storms etc.
 * XXX: currently very experimental */
void ai_look_for_enemy_missiles(object_t *op, struct mob_behaviour_param *params)
{
    int sense_range;
    int tilenr;
    int check_maps[8] = {0,0,0,0,0,0,0,0}; /* nearby map tiles to scan */

    if(!op->map)
        return;

    /* Disable this behaviour if we are in a melee fight */
    if (OBJECT_VALID(op->enemy, op->enemy_count))
    {
        rv_t  *rv  = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE);
        if (rv && rv->distance <= 1)
            return;
    }

    /* initialize hashtable if needed */
    if(MOB_DATA(op)->known_objs_ht == NULL)
        MOB_DATA(op)->known_objs_ht = pointer_hashtable_new(32);

    /* The "real" sense range calculation is in mob_can_see_ob(), this is
     * a simplified version */
    sense_range = op->stats.Wis;
    if (QUERY_FLAG(op, FLAG_SLEEP) || QUERY_FLAG(op, FLAG_BLIND))
        sense_range /= 2;
    sense_range = MAX(MIN_MON_RADIUS * 4, sense_range);

    maps_in_range(op, sense_range, check_maps);

    /* Scan for acvtive enemy missiles in each marked map */
    for (tilenr=0; tilenr < TILING_DIRECTION_NROF + 1; tilenr++)
    {
        object_t *obj;
        if(tilenr == TILING_DIRECTION_NROF)
            obj = op->map->active_objects->active_next; /* Always scan op's map */
        else if (op->map->tiling.tile_map[tilenr] && op->map->tiling.tile_map[tilenr]->in_memory == MAP_MEMORY_ACTIVE && check_maps[tilenr])
            obj = op->map->tiling.tile_map[tilenr]->active_objects->active_next;
        else
            continue;

        for (; obj; obj = obj->active_next)
        {
            switch(obj->type) {
                case BULLET:
                case THROWN_OBJ:
                case ARROW:
                case LIGHTNING:
                case CONE:
#if 0
//                case MMISSILE:
//                case FBULLET:
//                case FBALL:
//                case POISONCLOUD:
//                case BOMB:
#endif
                    if (obj->owner == NULL || get_friendship(op, obj->owner) <= FRIENDSHIP_ATTACK)
                    {
                        struct mob_known_obj *known = hashtable_find(MOB_DATA(op)->known_objs_ht, obj);
                        if(! known)
                        {
                            rv_t rv;

                            if (RV_GET_OBJ_TO_OBJ(op, obj, &rv, RV_DIAGONAL_DISTANCE) &&
                                (int)rv.distance <= sense_range)
                            {
                                known = register_npc_known_obj(op, obj, 0, -10, 1);
                                /* TODO: configurable attraction value */
                            }
                        } else
                            update_npc_known_obj(known, 0, 0);
                        if(known)
                            SET_FLAG(known, AI_OBJFLAG_IS_MISSILE);
                    }
            }
        }
    }
}

void ai_look_for_other_mobs(object_t *op, struct mob_behaviour_param *params)
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
            STRING_OBJ_NAME(op), op->env?op->env->x:-1, op->env?op->env->y:-1);
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

    maps_in_range(op, sense_range, check_maps);

    /* Scan for mobs and players in each marked map */
    for (tilenr=0; tilenr < TILING_DIRECTION_NROF + 1; tilenr++)
    {
        object_t *obj;
        if(tilenr == TILING_DIRECTION_NROF)
            obj = op->map->active_objects->active_next; /* Always scan op's map */
        else if (op->map->tiling.tile_map[tilenr] && op->map->tiling.tile_map[tilenr]->in_memory == MAP_MEMORY_ACTIVE && check_maps[tilenr])
            obj = op->map->tiling.tile_map[tilenr]->active_objects->active_next;
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
                    && obj != op
                    && mob_can_see_obj(op, obj, NULL))
            {
                struct mob_known_obj *tmp;
                /* See if we already know this mob */
                for(tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
                    if(tmp->obj == obj && tmp->obj_count == obj->count)
                        break;
                if(tmp)
                    update_npc_known_obj(tmp, 0, 0);
                else
                    register_npc_known_obj(op, obj, 0, 0, 1);
                /* TODO: get rid of double rv calculation
                 * (both can_see_obj() and register_npc_known_obj)
                 */
            }
        }
    }
}

/** Update friendship level of each known mob */
void ai_friendship(object_t *op, struct mob_behaviour_param *params)
{
    struct mob_known_obj   *tmp;
    object_t *owner_enemy = NULL;
    struct mob_known_obj   *known_owner_enemy = NULL;

    if(OBJECT_VALID(op->owner, op->owner_count) && op->owner->enemy)
        owner_enemy = op->owner->enemy;

    for (tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
    {
        tmp->tmp_friendship = tmp->friendship;

        /* Helps us focusing on a single enemy */
        if (tmp == MOB_DATA(op)->enemy)
            tmp->tmp_friendship += FRIENDSHIP_ENEMY_BONUS;

        /* Let pets attack player's targets */
        if(!op->enemy && tmp->obj == owner_enemy && tmp->obj_count == op->owner->enemy_count)
            known_owner_enemy = tmp;

        /* Now factor in distance  */
        if (get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE))
        {
            tmp->tmp_friendship += (FRIENDSHIP_DIST_MAX / (int) MAX(tmp->rv.distance, 1.0)) * SGN(tmp->tmp_friendship);
        }
        /* TODO: test last_seen aging */
        //        tmp->tmp_friendship /= MAX(global_round_tag - tmp->last_seen, 1);
        //        LOG(llevDebug,"ai_friendship(): '%s' -> '%s'. friendship: %d\n",  STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp->obj), tmp->tmp_friendship);
    }

    /* Learn about owner's enemy if we didn't know of it */
    if(owner_enemy && !op->enemy && !known_owner_enemy)
        register_npc_known_obj(op, owner_enemy, 0, 0, 0);
}

/** Update attraction/fear level of each known mob */
void ai_attraction(object_t *op, struct mob_behaviour_param *params)
{
    struct mob_known_obj *tmp;
    object_t               *owner;

    for (tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
    {
        tmp->tmp_attraction = tmp->attraction;

        /* Undeads don't deal with dynamic attractions or fears */
        if(op->race == shstr_cons.undead)
            continue;

        /* pets are attracted to owners */
        if ((owner = get_owner(op)) &&
            owner == tmp->obj)
        {
            tmp->tmp_attraction += ATTRACTION_HOME;
        }

        /* Attraction/fear for other mobs is calculated from the
         * perceived relative combad strength */
        if(tmp->tmp_friendship > FRIENDSHIP_HELP)
        {
            if(tmp->tmp_attraction >= 0)
                tmp->tmp_attraction += relative_combat_strength(op, tmp->obj);
        }
        else if(tmp->tmp_friendship < FRIENDSHIP_ATTACK)
        {
            if(tmp->tmp_attraction <= 0)
                tmp->tmp_attraction -= relative_combat_strength(op, tmp->obj);
        }
#if 0
        LOG(llevDebug, "ai_attraction(): %s attraction towards %s: %d\n",
                STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp->obj), tmp->tmp_attraction);
#endif
    }

    /* Tactical awareness work-in-progress temporarily disabled. Gecko 2006-04-19 */
#if 0
    {
        int dx, dy, sum;
        sum = assess_tactical_situation(op, &dx, &dy);
        LOG(llevDebug, "ai_attraction(): %s (%d) tacsit: %d@%d,%d\n",
                STRING_OBJ_NAME(op), op->count, sum, dx, dy);
    }
#endif
}

/* TODO: parameterize MAX_IDLE_TIME */
#define MAX_IDLE_TIME 5
#define ANTILURE_TIMER_MAX 5
void ai_choose_enemy(object_t *op, struct mob_behaviour_param *params)
{
    object_t                 *oldenemy    = op->enemy;
    struct mob_known_obj   *tmp, *worst_enemy = NULL;
    int antilure_dist_2 = AIPARAM_INT(AIPARAM_CHOOSE_ENEMY_ANTILURE_DISTANCE);
    object_t *base = NULL;
    map_t *base_map = NULL;

    /* We won't look for enemies if we are unaggressive */
    if(QUERY_FLAG(op, FLAG_UNAGGRESSIVE) || QUERY_FLAG(op, FLAG_SURRENDERED))
    {
        if(op->enemy)
            op->enemy = NULL;
        return;
    }

    /* Enable anti-luring protection? (distance -1 = disable)*/
    if(antilure_dist_2 >= 0)
    {
        /* Try to find a legal home map */
        base = insert_base_info_object(op);

        /* crashed on testserver - give log mesg */
        if(!base)
            LOG(llevDebug, "BUG: ai_choose_enemy(%s)> no base info object!\n", STRING_OBJ_NAME(op));
        else
        {
            if(base->slaying)
                if ((base_map = map_is_in_memory(base->slaying)))
                    if (base_map->in_memory != MAP_MEMORY_ACTIVE)
                        base_map = NULL;

            /* square distance for fast euclidian distance comparisons */
            antilure_dist_2 = antilure_dist_2 * antilure_dist_2;
        }
    }

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
                rv_t  *rv  = get_known_obj_rv(op, tmp, MAX_KNOWN_OBJ_RV_AGE);
                if (! rv)
                    continue;

                /* Ignore enemy if too far from home position */
                if(base_map && tmp->obj->map && tmp->obj->map->in_memory == MAP_MEMORY_ACTIVE)
                {
                    msp_t *tmp_msp = MSP_KNOWN(tmp->obj),
                          *base_msp = MSP_GET2(base_map, base->x, base->y);
                    rv_t   base_rv;

                    /* TODO: actually use tmp->last_map, last_x, last_y */
                    if(RV_GET_MSP_TO_MSP(tmp_msp, base_msp, &base_rv, RV_FAST_EUCLIDIAN_DISTANCE))
                    {
                        if((int)base_rv.distance > antilure_dist_2)
                        {
                            MOB_DATA(op)->antiluring_timer--;
                            if(MOB_DATA(op)->antiluring_timer <= 0)
                            {
#ifdef DEBUG_AI_ALL
                                LOG(llevDebug, "ai_choose_enemy() '%s' ignoring '%s' - too far from home\n",
                                        STRING_OBJ_NAME(op), STRING_OBJ_NAME(tmp->obj));
#endif
                                continue;
                            }
                        } else
                            MOB_DATA(op)->antiluring_timer = ANTILURE_TIMER_MAX;
                    }
                }

                op->anim_enemy_dir = rv->direction;
                worst_enemy = tmp;
            }
        }
    }

    /* Did we find an enemy? */
    if (worst_enemy)
    {
        /* conservative use of the linked spawns - if linked spawns, give enemy signal to all */
        /* only kick the signal here in, when we have a new target */
        if (MOB_DATA(op)->spawn_info && MOB_DATA(op)->spawn_info->owner->slaying)
            send_link_spawn_signal(MOB_DATA(op)->spawn_info->owner, worst_enemy->obj, LINK_SPAWN_ENEMY);

        //        LOG(llevDebug,"ai_choose_enemy(): %s's worst enemy is '%s', friendship: %d\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(worst_enemy->ob), worst_enemy->tmp_friendship);
        op->enemy = worst_enemy->obj;
        MOB_DATA(op)->enemy = worst_enemy;
        op->enemy_count = worst_enemy->obj_count;
        MOB_DATA(op)->move_speed_factor = 4; /* Attack speed */
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
            object_t *owner;

            if (!QUERY_FLAG(op, FLAG_FRIENDLY) && op->map)
                play_sound_map(MSP_KNOWN(op), SOUND_GROWL, SOUND_NORMAL);

            /* Notify player about target */
            if (op->type == MONSTER &&
                (owner = get_owner(op)) &&
                owner->type == PLAYER)
            {
                ndi(NDI_UNIQUE, 0, owner, "%s is attacking %s.",
                    query_name(op, owner, ARTICLE_POSSESSIVE, 0),
                    QUERY_SHORT_NAME(op->enemy, owner));
            }

            /* The unaggressives look after themselves 8) */
            /* TODO: Make a separate behaviour... */
            //            if(QUERY_FLAG(op,FLAG_UNAGGRESSIVE)) {
            //                CLEAR_FLAG(op, FLAG_UNAGGRESSIVE);
            //            npc_call_for_help(op);
            //            }
        }
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
void ai_plugin_process(object_t *op, struct mob_behaviour_param *params)
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
int ai_melee_attack_enemy(object_t *op, struct mob_behaviour_param *params)
{
    rv_t  *rv;

    if (!OBJECT_VALID(op->enemy, op->enemy_count)
     || QUERY_FLAG(op, FLAG_SCARED)
     || QUERY_FLAG(op, FLAG_UNAGGRESSIVE)
     || op->weapon_speed_left > 0
     || op->map == NULL)
        return FALSE;

    /* TODO: choose another enemy if this fails */
    if (!(rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    if (!RV_TEST_MELEE(*rv) ||
        !mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        return FALSE;

    //    LOG(llevDebug,"ai_melee_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));

    /* TODO: the following test should be done in skill_attack! */
    /* TODO: what if wc underflows? */
    op->anim_enemy_dir = rv->direction;
    if (QUERY_FLAG(op, FLAG_RUN_AWAY))
        rv->part->stats.wc -= 10;
    skill_attack(op->enemy, rv->part, 0, NULL);
    op->weapon_speed_left += FABS(op->weapon_speed);
    if (QUERY_FLAG(op, FLAG_RUN_AWAY))
        rv->part->stats.wc += 10;

    return TRUE;
}

int ai_bow_attack_enemy(object_t *op, struct mob_behaviour_param *params)
{
    object_t    *bow,
              *next,
              *arrow,
              *target = op->enemy;
    rv_t *rv;
    int        direction;

    if (!OBJECT_VALID(op->enemy, op->enemy_count) ||
        QUERY_FLAG(op, FLAG_UNAGGRESSIVE) ||
        QUERY_FLAG(op, FLAG_SCARED) ||
        !QUERY_FLAG(op, FLAG_READY_BOW) ||
        op->weapon_speed_left > 0 ||
        !op->map)
    {
        return 0;
    }

    /* TODO: choose another target if this or next test fails */
    if (!(rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
    {
        return 0;
    }

    /* TODO: also check distance and LOS */
    if (!RV_TEST_MISSILE_APPROX(*rv) ||
        !mob_can_see_obj(op, target, MOB_DATA(op)->enemy))
    {
        return 0;
    }

    //    LOG(llevDebug,"ai_distance_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));
    op->anim_enemy_dir = rv->direction;
    direction = (QUERY_FLAG(op, FLAG_CONFUSED)) ? absdir(rv->direction + RANDOM() % 5 - 2) : rv->direction;

    /* Find the applied bow */
    FOREACH_OBJECT_IN_OBJECT(bow, op, next)
    {
        if (bow->type == BOW &&
            QUERY_FLAG(bow, FLAG_APPLIED))
        {
            /* Select suitable arrows */
            arrow = (bow->sub_type1 == 128) ? bow->inv : find_arrow(op, bow->race);

            if(!arrow)
            {
                /* Out of arrows , unapply bow */
                manual_apply(op, bow, 0);
                bow = NULL;
            }

            break;
        }
    }

    if (!bow)
    {
        /*LOG(llevBug, "BUG: Monster %s (%d) has READY_BOW without bow.\n", STRING_OBJ_NAME(op), op->count);*/
        CLEAR_FLAG(op, FLAG_READY_BOW);
        return 0;
    }

    /* thats a infinitve arrow! dupe it. */
    if (QUERY_FLAG(arrow, FLAG_SYS_OBJECT))
    {
        object_t *new_arrow = get_object();

        copy_object(arrow, new_arrow);
        CLEAR_FLAG(new_arrow, FLAG_SYS_OBJECT);
        new_arrow->nrof = 1;

        /* now setup the self destruction */
        new_arrow->stats.food = 20;
        arrow = new_arrow;
    }
    else
    {
        arrow = get_split_ob(arrow, 1);
    }

    /* setup arrow stats basing on the mob op */
    if (!(arrow = create_missile(op, bow, arrow, rv->direction)))
    {
        return 0;
    }

    op->weapon_speed_left += FABS(op->weapon_speed);

    /* hack: without this, a monster with a bow is invinsible by a non range monster
     * with same speed. It simply runs away, can't be catched but will range kill
     * the other. Thats not what we want.
     * To remove this speed thingy, we need a flag for the AI which skips the movement
     * phase after a cast/arrow action.
     * At last we must skip a action which brings the mob out of range ... so, a movement
     * to the enemy should be allowed. This is not a question of reality of not - this will
     * destroy not only game play but also every map design and is a critical misbehaviour.
     */
    op->speed_left -= 2.0;
    return 1;
}

#define MAX_KNOWN_SPELLS 20

/* TODO: slightly rework this */
object_t * monster_choose_random_spell(object_t *monster)
{
    object_t *altern[MAX_KNOWN_SPELLS];
    object_t *tmp,
           *next;
    spell  *spell;
    int     i = 0, j;

    FOREACH_OBJECT_IN_OBJECT(tmp, monster, next)
    {
        if (tmp->type == ABILITY ||
            tmp->type == SPELLBOOK)
        {
            /*  Check and see if it's actually a useful spell */
            if ((spell = find_spell(tmp->stats.sp)) &&
                !(spell->path & (PATH_ABJURATION | PATH_TRANSMUTATION | PATH_LIGHT)))
            {
                if (tmp->stats.maxsp)
                {
                    for (j = 0; i < MAX_KNOWN_SPELLS && j < tmp->stats.maxsp; j++)
                    {
                        altern[i++] = tmp;
                    }
                }
                else
                {
                    altern[i++] = tmp;
                }

                if (i == MAX_KNOWN_SPELLS)
                {
                    break;
                }
            }
        }
    }

    if (i)
    {
        return altern[RANDOM() % i];
    }

    return NULL;
}

/* op is the basic mob doing anything
 * caster is op or the multipart part that the spell will come from
 * dir is the cast direction for directional spells
 * target is (an optional) target object
 * spell_item is the spell info item
 * sp_type is the id of the spell (a spell_item may have several spells
 * for different distances etc)
 */
static int monster_cast_spell(object_t *op, object_t *part, int dir, object_t *target, object_t *spell_item, int sp_type)
{
    spell  *sp;
    object_t *tmp_enemy = NULL;
    tag_t   tmp_enemy_tag = 0;
    int     ability, sp_cost;

    if ((sp = find_spell(sp_type)) == NULL)
    {
        LOG(llevDebug, "monster_cast_spell(): Can't find spell #%d for mob %s (%s) (%d,%d)\n", sp_type,
            STRING_OBJ_NAME(op), STRING_MAP_NAME(op->map), op->x, op->y);
        return FALSE;
    }

    sp_cost = SP_level_spellpoint_cost(op, op, sp_type);
    if (op->stats.sp < sp_cost)
        return FALSE;

    ability = (spell_item->type == ABILITY && QUERY_FLAG(spell_item, FLAG_IS_MAGICAL));

    /* add default cast time from spell force to monster.
     * we want make spell casting (ability) action independent from
     * speed - which will be really movement/physically action orientated.
     * With the casting delay counter, we are independent from speed &
     * weapon_speed - thats needed for heavy spells with, lets say, a 10 second delay
     * or even more. mob->magic is the default mob speed delay, spell->last_grace the
     * delay for this spell.
     * the last_grace counter is decreased in regenerate_stats().
     */
    op->last_grace += (op->magic + spell_item->last_grace);

    /* If we cast a spell, only use up casting_time speed.
     * outdated. we want use the casting delay counter above now! (MT-07.2005)
     */
    //op->speed_left += (float) 1.0 - (float) sp->time / (float) 20.0 * (float) FABS(op->speed);

    op->stats.sp -= sp_cost;

    /* The casting code uses op->enemy for target, but we don't always
     * target our current enemy. */
    if(target && target != op->enemy)
    {
        tmp_enemy = op->enemy;
        tmp_enemy_tag = op->enemy_count;
        op->enemy = target;
        op->enemy_count = target->count;
    }

//    LOG(llevNoLog,"CAST2 %s: spell_item=%s, dir=%d, target=%s\n",STRING_OBJ_NAME(op), STRING_OBJ_NAME(spell_item), dir, STRING_OBJ_NAME(target) );

    /* TODO: what does the return value of cast_spell do ? */
    cast_spell(part, part, dir, sp_type, ability, spellNormal, NULL);

    if(target && target == tmp_enemy)
    {
        op->enemy = tmp_enemy;
        op->enemy_count = tmp_enemy_tag;
    }

    op->speed_left-=2;/* hack: see bow behaviour! */

    return TRUE;
}

int ai_spell_attack_enemy(object_t *op, struct mob_behaviour_param *params)
{
    int direction, sp_type;
    rv_t  *rv;
    object_t     *spell_item;

    if (!OBJECT_VALID(op->enemy, op->enemy_count)
     || QUERY_FLAG(op, FLAG_SCARED)
     || !QUERY_FLAG(op, FLAG_READY_SPELL)
     || QUERY_FLAG(op, FLAG_UNAGGRESSIVE)
     // || op->weapon_speed_left > 0
     || op->last_grace > 0
     || op->map == NULL)
        return FALSE;

    /* TODO: choose another target if this or next test fails */
    if (!(rv = get_known_obj_rv(op, MOB_DATA(op)->enemy, MAX_KNOWN_OBJ_RV_AGE)))
        return FALSE;
    /* TODO: also check distance and LOS */
    /* TODO: should really check type of spell (area or missile) */
    if (!RV_TEST_MISSILE_APPROX(*rv) ||
        !mob_can_see_obj(op, op->enemy, MOB_DATA(op)->enemy))
        return FALSE;

    //    LOG(llevDebug,"ai_spell_attack_enemy(): '%s' -> '%s'\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(op->enemy));

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

    return monster_cast_spell(op, rv->part, direction, op->enemy, spell_item, sp_type);
}

int ai_heal_friend(object_t *op, struct mob_behaviour_param *params)
{
    object_t *tmp,
           *next;
    /* Selected target and spell for healing */
    object_t *target = NULL, *spell = NULL;
    /* Spell items for the different spell types */
    object_t *heal = NULL, *cure_poison = NULL, *cure_disease = NULL;

    int best_friendship = 0;

    if (QUERY_FLAG(op, FLAG_SCARED)
     || !QUERY_FLAG(op, FLAG_READY_SPELL)
     || op->weapon_speed_left > 0
     || op->last_grace > 0
     || op->map == NULL)
        return FALSE;

    /* TODO: shouldn't do this every tick. It think setting up a
     * bitmap of known spells at mob creation should be enough
     * (will still need to search to find the actual spell_item,
     * but we can that delay that until we know we have a target) */

    /* See what spells we actually know... */
    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
        if (tmp->type == ABILITY ||
            tmp->type == SPELLBOOK)
        {
            switch(tmp->stats.sp)
            {
                case SP_MINOR_HEAL:
                heal = tmp;
                break;

                case SP_CURE_POISON:
                cure_poison = tmp;
                break;

                case SP_CURE_DISEASE:
                cure_disease = tmp;
                break;
            }
        }
    }

    if(heal == NULL &&  cure_poison == NULL && cure_disease == NULL)
        return FALSE;

    /* TODO: actually support curing. For that I need an efficient method
     * to see if a mob is poisoned or diseased */

    /* Do we need to heal ourself? */
    if(op->stats.hp < op->stats.maxhp / 2 && heal) {
        target = op;
        spell = heal;
    }
    else
    {
        struct mob_known_obj *tmp;

        /* Go through list of known mobs and look for hurt friends */
        for (tmp = MOB_DATA(op)->known_mobs; tmp; tmp = tmp->next)
        {
            if(tmp->tmp_friendship > best_friendship)
            {
                if (AIPARAM_PRESENT(AIPARAM_HEAL_FRIEND_HEALING_MIN_FRIENDSHIP) &&
                        tmp->tmp_friendship >= AIPARAM_INT(AIPARAM_HEAL_FRIEND_HEALING_MIN_FRIENDSHIP) &&
                        tmp->obj->stats.hp < tmp->obj->stats.maxhp / 2)
                {
                    target = tmp->obj;
                    spell = heal;
                    best_friendship = tmp->tmp_friendship;
                }
            }
        }
    }

    if(spell && target)
        return monster_cast_spell(op, op, 0, target, spell, spell->stats.sp);
    else
        return FALSE;
}


/* AI <-> plugin interface for actions */
int ai_plugin_action(object_t *op, struct mob_behaviour_param *params)
{
    int ret = 0;
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
    CFP.Value[9] = (char *) AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_BEHAVIOUR);   /* file */
    CFP.Value[10] = (char *) AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_OPTIONS);
    CFP.Value[11] = NULL;
    ret = PlugList[findPlugin(AIPARAM_STRING(AIPARAM_PLUGIN_PROCESS_PLUGIN))].eventfunc (&CFP);
#endif

    return ret;
}
