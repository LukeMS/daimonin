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

#include <global.h>

/* given that 'pet' is a friendly object, this function returns a
 * monster the pet should attack, NULL if nothing appropriate is
 * found.  it basically looks for nasty things around the owner
 * of the pet to attack.
 * this is now tilemap aware.
 */

object * get_pet_enemy(object *pet, rv_vector *rv)
{
    object     *owner, *tmp;
    int         i, x, y;
    mapstruct  *nm;

    if ((owner = get_owner(pet)) != NULL)
    {
        /* If the owner has turned on the pet, make the pet
         * unfriendly.
         */
        /* TODO deactivated while cleaning up AI code */
        /*  if ((check_enemy(owner,rv)) == pet) {
                CLEAR_FLAG(pet, FLAG_FRIENDLY);
                remove_friendly_object(pet);
                pet->move_type &=~PETMOVE;
                return owner;
            }*/
    }
    else
    {
        /* else the owner is no longer around, so the
         * pet no longer needs to be friendly.
         */
        CLEAR_FLAG(pet, FLAG_FRIENDLY);
        remove_friendly_object(pet);
        pet->move_type &= ~PETMOVE;
        return NULL;
    }
    /* If they are not on the same map, the pet won't be agressive */
    if (!on_same_map(pet, owner))
        return NULL;

    /* We basically look for anything nasty around the owner that this
     * pet should go and attack.
     */
    for (i = 0; i < SIZEOFFREE; i++)
    {
        x = owner->x + freearr_x[i];
        y = owner->y + freearr_y[i];
        if (!(nm = out_of_map(owner->map, &x, &y)))
            continue;
        /* Only look on the space if there is something alive there. */
        /* here we need to tweak a bit for PvP - pets should attack player /golems then.
             * well, this can wait until i include the arena/pvp areas
             */
        if (GET_MAP_FLAGS(nm, x, y) & P_IS_ALIVE)
        {
            for (tmp = get_map_ob(nm, x, y); tmp != NULL; tmp = tmp->above)
            {
                object *tmp2    = tmp->head == NULL ? tmp : tmp->head;
                if (QUERY_FLAG(tmp2, FLAG_ALIVE)
                 && !QUERY_FLAG(tmp2, FLAG_FRIENDLY)
                 && !QUERY_FLAG(tmp2,
                                                                                                    FLAG_UNAGGRESSIVE)
                 && tmp2 != owner
                 && tmp2->type != PLAYER)
                    return tmp2;
            } /* for objects on this space */
        } /* if there is something living on this space */
    } /* for loop of spaces around the owner */

    /* Didn't find anything - return NULL */
    return NULL;
}

void terminate_all_pets(object *owner)
{
    objectlink *obl, *next;
    for (obl = first_friendly_object; obl != NULL; obl = next)
    {
        object *ob  = obl->objlink.ob;
        next = obl->next;
        if (get_owner(ob) == owner)
        {
            remove_friendly_object(ob);
            if (!QUERY_FLAG(ob, FLAG_REMOVED))
            {
                remove_ob(ob);
                check_walk_off(ob, NULL, MOVE_APPLY_VANISHED);
            }
        }
    }
}

/*
 * Unfortunately, sometimes, the owner of a pet is in the
 * process of entering a new map when this is called.
 * Thus the map isn't loaded yet, and we have to remove
 * the pet...
 * Interesting enough, we don't use the passed map structure in
 * this function.
 */

void remove_all_pets(mapstruct *map)
{
    objectlink *obl, *next;
    object     *owner;

    for (obl = first_friendly_object; obl != NULL; obl = next)
    {
        next = obl->next;
        if (obl->objlink.ob->type != PLAYER
         && QUERY_FLAG(obl->objlink.ob, FLAG_FRIENDLY)
         && (owner = get_owner(obl->objlink.ob)) != NULL
         && owner->map != obl->objlink.ob->map)
        {
            /* follow owner checks map status for us */
            follow_owner(obl->objlink.ob, owner);
            /* bug: follow can kill the pet here ... */
            if (QUERY_FLAG(obl->objlink.ob, FLAG_REMOVED) && FABS(obl->objlink.ob->speed) > MIN_ACTIVE_SPEED)
            {
                object *ob  = obl->objlink.ob;
                LOG(llevMonster, "(pet failed to follow)");
                remove_friendly_object(ob);
            }
        }
    }
}

void follow_owner(object *ob, object *owner)
{
    object *tmp;
    int     dir;

    if (!QUERY_FLAG(ob, FLAG_REMOVED))
    {
        remove_ob(ob);
        if (check_walk_off(ob, NULL, MOVE_APPLY_VANISHED) != CHECK_WALK_OK)
            return;
    }
    if (owner->map == NULL)
    {
        LOG(llevBug, "BUG: Can't follow owner: no map.\n");
        return;
    }
    if (owner->map->in_memory != MAP_IN_MEMORY)
    {
        LOG(llevBug, "BUG: Owner of the pet not on a map in memory!?\n");
        return;
    }
    dir = find_free_spot(ob->arch, owner->map, owner->x, owner->y, 1, SIZEOFFREE + 1);
    if (dir == -1)
    {
        LOG(llevMonster, "No space for pet to follow, freeing %s.\n", ob->name);
        return; /* Will be freed since it's removed */
    }
    for (tmp = ob; tmp != NULL; tmp = tmp->more)
    {
        tmp->x = owner->x + freearr_x[dir] + (tmp->arch == NULL ? 0 : tmp->arch->clone.x);
        tmp->y = owner->y + freearr_y[dir] + (tmp->arch == NULL ? 0 : tmp->arch->clone.y);
    }
    if (!insert_ob_in_map(ob, owner->map, NULL, 0))
    {
        if (owner->type == PLAYER) /* Uh, I hope this is always true... */
            new_draw_info(NDI_UNIQUE, 0, owner, "Your pet has disappeared.");
    }
    else if (owner->type == PLAYER) /* Uh, I hope this is always true... */
        new_draw_info(NDI_UNIQUE, 0, owner, "Your pet magically appears next to you");
    return;
}

void pet_move(object *ob)
{
    int         dir, tag, xt, yt;
    object     *ob2, *owner;
    mapstruct  *mt;

    /* Check to see if player pulled out */
    if ((owner = get_owner(ob)) == NULL)
    {
        remove_ob(ob); /* Will be freed when returning */
        remove_friendly_object(ob);
        check_walk_off(ob, NULL, MOVE_APPLY_VANISHED);
        LOG(llevMonster, "Pet: no owner, leaving.\n");
        return;
    }

    /* move monster into the owners map if not in the same map */
    if (ob->map != owner->map)
    {
        follow_owner(ob, owner);
        return;
    }
    /* Calculate Direction */
    dir = find_dir_2(ob->x - ob->owner->x, ob->y - ob->owner->y);
    ob->direction = dir;

    tag = ob->count;
    if (!(move_ob(ob, dir, ob)))
    {
        object *part;

        /* the failed move_ob above may destroy the pet, so check here */
        if (was_destroyed(ob, tag))
            return;

        for (part = ob; part != NULL; part = part->more)
        {
            xt = part->x + freearr_x[dir];
            yt = part->y + freearr_y[dir];
            if (!(mt = out_of_map(part->map, &xt, &yt)))
                return;

            for (ob2 = get_map_ob(mt, xt, yt); ob2 != NULL; ob2 = ob2->above)
            {
                object *new_ob;
                new_ob = ob2->head ? ob2->head : ob2;
                if (new_ob == ob)
                    break;
                if (new_ob == ob->owner)
                    return;
                if (get_owner(new_ob) == ob->owner)
                    break;
                if (QUERY_FLAG(new_ob, FLAG_ALIVE)
                 && !QUERY_FLAG(ob, FLAG_UNAGGRESSIVE)
                 && !QUERY_FLAG(new_ob,
                                FLAG_UNAGGRESSIVE)
                 && !QUERY_FLAG(new_ob,
                                FLAG_FRIENDLY))
                {
                    register_npc_known_obj(ob, new_ob, FRIENDSHIP_PUSH);
                    register_npc_known_obj(new_ob, ob, FRIENDSHIP_PUSH);
                    return;
                }
                else if (new_ob->type == PLAYER)
                {
                    new_draw_info(NDI_UNIQUE, 0, new_ob, "You stand in the way of someones pet.");
                    return;
                }
            }
        }
        dir = absdir(dir + 4 - (RANDOM() % 5) - (RANDOM() % 5));
        (void) move_ob(ob, dir, ob);
    }
    return;
}
