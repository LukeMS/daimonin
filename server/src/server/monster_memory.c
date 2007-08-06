/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2006 Michael Toennies

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
 * This file contains functions related to AI object/mob memory.
 */

#include <global.h>

#include <aiconfig.h>

/* Those are pseudobehaviours in monster_behaviour.c */
extern int get_npc_object_attraction(object *op, object *other);
extern int get_npc_attitude(object *op, object *other);

/** Purge a single object from list of known mobs/objects */
static inline void remove_mob_known(struct mob_known_obj *tmp, struct mob_known_obj **first, hashtable *ht)
{
    if (tmp->next)
        tmp->next->prev = tmp->prev;

    if (tmp->prev)
        tmp->prev->next = tmp->next;
    else
        *first = tmp->next;

    if(ht)
        hashtable_erase(ht, tmp->obj);

    return_poolchunk(tmp, pool_mob_knownobj);
}

/** Purge invalid and old objects from list of known mobs/objects */
void cleanup_mob_knowns(object *op, struct mob_known_obj **first, hashtable *ht)
{
    struct mob_known_obj   *tmp;
    for (tmp = *first; tmp; tmp = tmp->next)
    {
        if (!OBJECT_VALID(tmp->obj, tmp->obj_count))
            remove_mob_known(tmp, first, ht);
        else if(ROUND_TAG - tmp->last_seen > MAX_KNOWN_OBJ_AGE)
        {
            /* Never forget about our owner */
            if(op->owner != tmp->obj || op->owner_count != tmp->obj->count)
                remove_mob_known(tmp, first, ht);
        }
    }
}

/** Completely clear list of known mobs or objects */
void clear_mob_knowns(object *op, struct mob_known_obj **first, hashtable *ht)
{
    struct mob_known_obj   *tmp;
    /* TODO: can be optimized (clear hashtable, return all chunks, set list
     * pointers to NULL) */
    for (tmp = *first; tmp; tmp = tmp->next)
    {
        /* Don't forget about our owner */
        if(op->owner != tmp->obj || op->owner_count != tmp->obj->count)
            remove_mob_known(tmp, first, ht);
    }
}

/** Update known_obj fields to indicate new knowledge about an object or mob.
 * @param known the already known object we have new info about
 * @param delta_friendship friendship change towards other (for example if other is helping or attacking npc)
 * @param delta_attraction attraction change towards other
 */
void update_npc_known_obj(struct mob_known_obj *known, int delta_friendship, int delta_attraction)
{
    known->friendship += delta_friendship;
    known->attraction += delta_attraction;
    known->tmp_friendship += delta_friendship;
    known->tmp_attraction += delta_attraction;
    known->last_seen = ROUND_TAG;

    if(known->obj->map)
    {
        if(known->last_map != known->obj->map->orig_path)
            FREE_AND_ADD_REF_HASH(known->last_map, known->obj->map->orig_path);
        known->last_x = known->obj->x;
        known->last_y = known->obj->y;
    }
}

/** register a new enemy, friend or interesting object for the NPC.
 * Only call this if you are sure other isn't already known by npc.
 * @param npc the NPC (or mob) that is to know more info.
 * @param other the other object/mob/npc
 * @param friendship initial friendship value or 0 to calculate
 * @param attraction initial attraction value or 0 to calculate
 * @return a mob_known_obj struct for other, or NULL if there was a problem.
 */
struct mob_known_obj *register_npc_known_obj(object *npc, object *other, int friendship, int attraction)
{
    struct mob_known_obj *known;
    int i;
    rv_vector rv;
    int nomap = 0;
    int is_object = 0;

    /* Non-mob object?  */
    if (other->type != PLAYER && !QUERY_FLAG(other, FLAG_ALIVE))
    {
        is_object = 1;

        /* initialize hashtable if needed */
        if(MOB_DATA(npc)->known_objs_ht == NULL)
            MOB_DATA(npc)->known_objs_ht = pointer_hashtable_new(32);
    }

    /* TODO: keep count of objects and push out less
     * important if lists grow too big */

    /* The following code handles new, previously unknown objects */

    /* Special handling of mobs inside containers or not on maps */
    if(other->map == NULL || other->env != NULL || npc->map == NULL || npc->env != NULL)
    {
#ifdef DEBUG_AI_NPC_KNOWN
        LOG(llevDebug, "register_npc_known_obj(): '%s' trying to register object '%s' and at least one not on a map\n", STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other));
#endif
        nomap = 1;
    }
    else
    {
        if(! get_rangevector(npc, other, &rv, RV_EUCLIDIAN_DISTANCE) || !rv.part)
        {
            LOG(llevBug, "BUG: register_npc_known_obj(): '%s' can't get rv to '%s'\n", STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other));
            return NULL;
        }

        /* We check LOS here, only if we are registering a new object */
        /* Also, we only check against players, and not if we have
         * been hit or helped by them, or if they own us */
        if(other->type == PLAYER && friendship == 0 && npc->owner != other)
            if(!obj_in_line_of_sight(npc, other, &rv))
              return NULL;
    }

    /* Calculate attitude and/or attraction if needed */
    if(attraction == 0)
        attraction = get_npc_object_attraction(npc, other);
    if(!is_object && friendship == 0)
        friendship = get_npc_attitude(npc, other);

    /* We do a last attempt at LOS test here, for mob to mob detection,
     * but only if the two are enemies */
    if(nomap == 0 && !is_object && other->type != PLAYER && friendship < 0)
    {
        if(!obj_in_line_of_sight(npc, other, &rv))
        {
#ifdef DEBUG_AI_NPC_KNOWN
            LOG(llevDebug,"register_npc_known_obj(): '%s' can't see '%s'. friendship: %d, attraction: %d\n",  STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other), friendship, attraction);
#endif
            return NULL;
        }
    }

    /* Initialize the known_obj at last */
    known = get_poolchunk(pool_mob_knownobj);
    known->next = NULL;
    known->prev = NULL;

    known->last_map = NULL;

    known->obj = other;
    known->obj_count = other->count;

    known->tmp_friendship = 0;
    known->tmp_attraction = 0;
    known->friendship = 0;
    known->attraction = 0;

    for(i=0; i<=NROF_AI_KNOWN_OBJ_FLAGS/32; i++)
        known->flags[i] = 0;

    if(nomap)
        known->rv_time = 0; /* No cached rv */
    else
    {
        known->rv_time = ROUND_TAG;   /* Cache the rv we calculated above. */
        known->rv = rv;
    }

    /* Initial friendship and attitude */
    update_npc_known_obj(known, friendship, attraction);

    /* Add to list and possibly hashtable */
    if(is_object)
    {
        if(MOB_DATA(npc)->known_objs)
            MOB_DATA(npc)->known_objs->prev = known;
        known->next = MOB_DATA(npc)->known_objs;
        MOB_DATA(npc)->known_objs = known;

        hashtable_insert(MOB_DATA(npc)->known_objs_ht, other, known);
    }
    else
    {
        if(MOB_DATA(npc)->known_mobs)
            MOB_DATA(npc)->known_mobs->prev = known;
        known->next = MOB_DATA(npc)->known_mobs;
        MOB_DATA(npc)->known_mobs = known;
    }

    //    LOG(llevDebug,"register_npc_known_obj(): '%s' detected '%s'. friendship: %d, attraction: %d\n",  STRING_OBJ_NAME(npc), STRING_OBJ_NAME(other), known->friendship, known->attraction);

    return known;
}

/** Update npc's knowledge about another mob or object with changes in friendship and/or attraction.
 * This is the external interface to the mob memory system, to be used in attack code etc.
 * @param npc the NPC (or mob) that is to know more info.
 * @param other the other object/mob/npc
 * @param delta_friendship friendship change towards other (for example if other is helping or attacking npc)
 * @param delta_attraction attraction change towards other
 */
struct mob_known_obj *update_npc_knowledge(object *npc, object *other, int delta_friendship, int delta_attraction)
{
    int is_object = 0; /* We differ between objects and mobs */
    struct mob_known_obj *known = NULL; /* Did we already know this other */

    if (npc == NULL)
    {
#ifdef DEBUG_AI_NPC_KNOWN
        LOG(llevDebug, "update_npc_knowledge(): Called with NULL npc obj\n");
#endif
        return NULL;
    }

    if (other == NULL)
    {
#ifdef DEBUG_AI_NPC_KNOWN
        LOG(llevDebug, "update_npc_knowledge(): Called with NULL other obj\n");
#endif
        return NULL;
    }

    if (npc == other)
    {
#ifdef DEBUG_AI_NPC_KNOWN
        LOG(llevDebug, "update_npc_knowledge(): Called for itself '%s'\n", STRING_OBJ_NAME(npc));
#endif
        return NULL;
    }

    if (npc->type != MONSTER)
    {
#ifdef DEBUG_AI_NPC_KNOWN
        LOG(llevDebug, "update_npc_knowledge(): Called on non-mob object '%s' type %d\n", STRING_OBJ_NAME(npc),
            npc->type);
#endif
        return NULL;
    }

    /* this check will hopefully be unnecessary in the future */
    if (MOB_DATA(npc) == NULL)
    {
#ifdef DEBUG_AI_NPC_KNOWN
        LOG(llevDebug, "update_npc_knowledge(): No mobdata (yet) for '%s'\n", STRING_OBJ_NAME(npc));
#endif
        return NULL;
    }

    /* never register anything when surrendered.
     * A surrendered mob doesn't deal in friends, enemies or objects.
     */
    if (QUERY_FLAG(npc, FLAG_SURRENDERED))
        return NULL;

    /* Non-mob object?  */
    if (other->type != PLAYER && !QUERY_FLAG(other, FLAG_ALIVE))
        is_object = 1;

    /* Does npc already know this other? In that case known = old_known_obj */
    if(is_object && MOB_DATA(npc)->known_objs_ht)
        known = (struct mob_known_obj *)hashtable_find(MOB_DATA(npc)->known_objs_ht, other);
    else if(! is_object)
    {
        for (known = MOB_DATA(npc)->known_mobs; known; known = known->next)
        {
            if (known->obj == other)
                break;
        }
    }

    /* Previously unknown other */
    if(known == NULL)
        known = register_npc_known_obj(npc, other, 0, 0);

    /* Update values */
    if(known)
    {
        update_npc_known_obj(known, delta_friendship, delta_attraction);

        /* If an unaggressive mob was attacked, it now turns aggressive forever */
        if (!is_object && delta_friendship < 0 && QUERY_FLAG(npc, FLAG_UNAGGRESSIVE))
        {
            CLEAR_FLAG(npc, FLAG_UNAGGRESSIVE);
            known->friendship += FRIENDSHIP_ATTACK;
        }
    }

    return known;
}

/** Get the rangevector to a known object. If an earlier calculated rangevector is
 * older than maxage then we calculate a new one (set maxage to 0 to force update).
 * Returns a pointer to the rangevector, or NULL if get_rangevector() failed.
 * @param op origin mob
 * @param known_obj object we want the vector to
 * @param maxage maximum age of cached vector
 * @return a vector. possibly an older cached vector. possibly NULL if a vector
 * couldn't be calculated.
 */
rv_vector * get_known_obj_rv(object *op, struct mob_known_obj *known_obj, int maxage)
{
    /* TODO: added checks for NULL maps here (happens if monster is picked up, for example).
     * Actually, it would be slightly more interesting if we could get the coordinates for the
     * container of the mob, so that mobs can for example hide in containers until the enemy
     * is far enough away. Or reversly, hide in container and later jump out and attack enemy.
     * Gecko 2005-05-08 */
    if ( op == NULL || op->map == NULL || op->env ||
            known_obj == NULL || known_obj->obj->map == NULL || known_obj->obj->env)
        return NULL;

    if (ROUND_TAG - known_obj->rv_time >= (uint32) maxage || known_obj->rv_time == 0 || maxage == 0)
    {
        /* Recalculating the rv might very well fail */
        if (! get_rangevector(op, known_obj->obj, &known_obj->rv, 0))
        {
            known_obj->rv_time = 0;
            return NULL;
        }

        known_obj->rv_time = ROUND_TAG;
    }

    /* hotfix for this bug. part should here NOT be NULL */
    if (!known_obj->rv.part)
    {
        LOG(-1, "CRASHBUG: rv->part == NULL for %s on map %s with enemy %s and map %s\n", query_name(op),
            op->map ? STRING_SAFE(op->map->orig_path) : "NULL", query_name(known_obj->obj),
            known_obj->obj ? STRING_SAFE(known_obj->obj->map ? STRING_SAFE(known_obj->obj->map->orig_path) : "NULL") : "NULL");
        return NULL;
    }
    return &known_obj->rv;
}
