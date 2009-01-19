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

/* Utility functions for pets management. */

#include <global.h>

#define MAX_PETS 2      /* Maximum number of pets at any time */
#define MAX_PERMAPETS 2 /* Maximum number of non-temporary pets */

#define PET_VALID(pet_ol, _owner_) \
    (OBJECT_VALID((pet_ol)->objlink.ob, (pet_ol)->id) && \
     (pet_ol)->objlink.ob->owner == (_owner_) && (pet_ol)->objlink.ob->owner_count == (_owner_)->count)

/* force: 1 to force, 0 for normal usage
   If force is 0, a normal pet addition is done, which may fail for
   several reasons. If force is 1, the pet is forced which may still
   fail but not as often.
   Normally you should use 0, but in some cases where for example a
   quest requires a specific pet you could try 1.
   Returns 0 on success, -1 on failure.
*/
int add_pet(object *owner, object *pet, int force)
{
    int nrof_pets = 0, nrof_permapets = 0;
    objectlink *ol, *next_ol;
    struct mob_known_obj *tmp;
    object *spawninfo;

    if(owner == NULL || pet == NULL || owner->type != PLAYER || pet->type != MONSTER)
    {
        LOG(llevBug, "BUG: add_pet(): Illegal owner (%s) or pet (%s)\n", STRING_OBJ_NAME(owner), STRING_OBJ_NAME(pet));
        return -1;
    }

    /* Handle multipart objects */
    if(pet->head)
        pet = pet->head;

    if(pet->owner == owner || (OBJECT_VALID(pet->owner, pet->owner_count) && !force))
    {
        new_draw_info_format(NDI_UNIQUE, 0, owner, "%s is already taken", query_name(pet));
        return -1;
    }

    /* Count number of pets, remove invalid links */
    for(ol = CONTR(owner)->pets; ol; ol = next_ol)
    {
        next_ol = ol->next;
        if(PET_VALID(ol, owner))
            nrof_pets++;
        else
            objectlink_unlink(&CONTR(owner)->pets, NULL, ol);
        /*
        if(! QUERY_FLAG(ol->objlink.ob, FLAG_IS_USED_UP))
            nrof_permapets++;
        */
    }

    if((nrof_pets >= MAX_PETS || nrof_permapets >= MAX_PERMAPETS) && !force)
    {
        new_draw_info_format(NDI_UNIQUE, 0, owner, "You have too many pets to handle %s", query_name(pet));
        return -1;
    }

    /* Make pet forget all old friendship values (TODO really?) */
    for (tmp = MOB_DATA(pet)->known_mobs; tmp; tmp = tmp->next)
        return_poolchunk(tmp, pool_mob_knownobj);
    MOB_DATA(pet)->known_mobs = NULL;
    /* In effect, make the pet a whole new object, so that all other
     * mobs forgets about it */
    /* TODO: efficient, but ugly solution. watch for side effects */
    pet->count = ++ob_count;

    /* Set pet owner */
    pet->owner = owner;
    pet->owner_count = owner->count;
    if(! (MOB_DATA(pet)->owner = update_npc_knowledge(pet, owner, 0, 0)))
    {
        LOG(llevBug, "BUG: add_pet(): Couldn't register owner (%s) of pet (%s)\n", STRING_OBJ_NAME(owner), STRING_OBJ_NAME(pet));
        return -1;
    }

    SET_OR_CLEAR_FLAG(pet, FLAG_FRIENDLY, QUERY_FLAG(owner, FLAG_FRIENDLY)); /* Brainwash */
    pet->enemy = NULL;
    MOB_DATA(pet)->enemy = NULL;

    /* Follow owner combat mode */
    SET_OR_CLEAR_FLAG(pet, FLAG_UNAGGRESSIVE, !CONTR(owner)->combat_mode);

    /* Insert link in owner's pet list */
    ol = get_objectlink(OBJLNK_FLAG_OB);
    ol->objlink.ob = pet;
    ol->id = pet->count;
    ol->ref_count = 1; /* TODO: How can this be used? */
    objectlink_link(&CONTR(owner)->pets, NULL, NULL, CONTR(owner)->pets, ol);

    /* Need to release/unlink spawned mobs */
    /* TODO: move to a release_spawn_mob() function in spawn.c */
    if((spawninfo = MOB_DATA(pet)->spawn_info))
    {
        if(OBJECT_VALID(spawninfo->owner, spawninfo->owner_count) &&
                spawninfo->owner->enemy == pet)
        {
            spawninfo->owner->enemy = NULL;
            LOG(llevDebug, "add_pet(): removing %s from spawn point control\n", STRING_OBJ_NAME(pet));
        }
        remove_ob(spawninfo);
        MOB_DATA(pet)->spawn_info = NULL;
        CLEAR_MULTI_FLAG(pet, FLAG_SPAWN_MOB);
    }

    return 0;
}

void update_pets_combat_mode(object *owner)
{
    objectlink *ol;

    for(ol = CONTR(owner)->pets; ol; ol = ol->next)
    {
        if(PET_VALID(ol, owner))
            SET_OR_CLEAR_FLAG(ol->objlink.ob, FLAG_UNAGGRESSIVE, !CONTR(owner)->combat_mode);
    }
}

void save_pet(object *pet)
{
    if (!QUERY_FLAG(pet, FLAG_REMOVED))
    {
        remove_ob(pet);
        if (check_walk_off(pet, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
        {
            new_draw_info_format(NDI_UNIQUE, 0, pet->owner, "Your %s has disappeared.", query_name(pet));
            return;
        }
    }

    SET_FLAG(pet, FLAG_SYS_OBJECT);
    insert_ob_in_ob(pet, pet->owner);
}

/* Warp a pet close to its owner. If that is impossible, temporarily store
 * the pet in the owner until there's somewhere to move out */
/* TODO: the pathfinding system needs to be updated to handle
 * warping/teleporting of mobs */
void pet_follow_owner(object *pet)
{
    object *tmp;
    int     dir;

    if (!QUERY_FLAG(pet, FLAG_REMOVED))
    {
        remove_ob(pet);
        if (check_walk_off(pet, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
        {
            new_draw_info_format(NDI_UNIQUE, 0, pet->owner, "Your %s has disappeared.", query_name(pet));
            return;
        }
    }

    /*
     * Unfortunately, sometimes, the owner of a pet is in the
     * process of entering a new map when this is called.
     * Thus the map isn't loaded yet, and we have to remove
     * the pet.
     * The player can also be removed from a map when called
     * (as in the case of /resetmap).
     * We should solve this by storing pet with player
     * until player is on an acceptable map. The goal is to preserve
     * the pets as much as possible.
     */

    if (pet->owner->map == NULL || QUERY_FLAG(pet->owner, FLAG_REMOVED))
    {
//        new_draw_info_format(NDI_UNIQUE, 0, pet->owner, "Your %s has disappeared (no map).", query_name(pet));
//        LOG(llevBug, "BUG: Can't follow owner: no map.\n");
        save_pet(pet);
        return;
    }

    if (pet->owner->map->in_memory != MAP_IN_MEMORY)
    {
//        new_draw_info_format(NDI_UNIQUE, 0, pet->owner, "Your %s has disappeared (map not loaded).", query_name(pet));
//        LOG(llevBug, "BUG: Owner of the pet not on a map in memory!?\n");
        save_pet(pet);
        return;
    }

    dir = find_free_spot(pet->arch, pet, pet->owner->map, pet->owner->x, pet->owner->y, INS_WITHIN_LOS, 1, SIZEOFFREE - 1);
    if (dir == -1)
    {
//        new_draw_info_format(NDI_UNIQUE, 0, pet->owner, "Your %s has disappeared (no space).", query_name(pet));
//        LOG(llevBug, "BUG: No space for pet to follow, freeing %s.\n", STRING_OBJ_NAME(pet));
        save_pet(pet);
        return; /* Will be freed since it's removed */
    }

    for (tmp = pet; tmp != NULL; tmp = tmp->more)
    {
        tmp->x = pet->owner->x + freearr_x[dir] + tmp->arch->clone.x;
        tmp->y = pet->owner->y + freearr_y[dir] + tmp->arch->clone.y;
    }

    CLEAR_FLAG(pet, FLAG_SYS_OBJECT);

    if (!insert_ob_in_map(pet, pet->owner->map, NULL, 0))
        new_draw_info_format(NDI_UNIQUE, 0, pet->owner, "Your %s has disappeared.", query_name(pet));
    else
        new_draw_info_format(NDI_UNIQUE, 0, pet->owner, "Your %s appears next to you", query_name(pet));
}

/* Warp owner's distant pets towards him */
/* TODO: this could also be used by a skill or spell "recall pets" */
void pets_follow_owner(object *owner)
{
    objectlink *ol;

    for(ol = CONTR(owner)->pets; ol; ol = ol->next)
        if(PET_VALID(ol, owner) && !on_same_tileset(ol->objlink.ob, owner))
            pet_follow_owner(ol->objlink.ob);
}

/* Called when a map is swapped out or reloaded. Should warp
 * all pets on a map to their owner */
void remove_all_pets(mapstruct *map)
{
    object *tmp, *next_tmp;

    /* TODO: with a little better org of the active list (mobs first,
     * then other objects) we can make this a little faster */

    /* TODO: it is possible that this has the same problems as the
     * traversal of the active list. Be careful. */
    for(tmp = map->active_objects->active_next; tmp; tmp = next_tmp)
    {
        next_tmp = tmp->active_next;

        /* FIXME: This is a temporary extra check */
        if(tmp->map != map)
            LOG(llevError, "ERROR: remove_all_pets(): object %s (%d) in activelist of map %s really on map %s\n", STRING_OBJ_NAME(tmp), tmp->count, STRING_MAP_NAME(map), STRING_MAP_NAME(tmp->map));

        if(tmp->type == MONSTER && OBJECT_VALID_OR_REMOVED(tmp->owner, tmp->owner_count))
            pet_follow_owner(tmp);
    }
}

/* Make all pets disappear */
void terminate_all_pets(object *owner)
{
    objectlink *ol;

    for(ol = CONTR(owner)->pets; ol; ol = ol->next)
    {
        if(PET_VALID(ol, owner))
        {
            remove_ob(ol->objlink.ob);
            check_walk_off(ol->objlink.ob, NULL, MOVE_APPLY_VANISHED);
        }
    }
}

/* called from save_object for players */
void save_all_pets(FILE *fp, object *owner, int flag)
{
    objectlink *ol;

    for(ol = CONTR(owner)->pets; ol; ol = ol->next)
    {
        if(PET_VALID(ol, owner))
        {
            object *pet = ol->objlink.ob;

            /* Don't save twice */
            if(pet->env == owner)
                continue;

            SET_FLAG(pet, FLAG_SYS_OBJECT);
            save_object(fp, pet, 2);
            CLEAR_FLAG(pet, FLAG_SYS_OBJECT);
        }
    }
}
