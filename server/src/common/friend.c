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

/* MT, 10.2002
 * Hm, this friendly list concept is very useful - but NOT NOT in this way!
 * First, it is broken in the way as players and npcs/pets are added to the same
 * list. Players are not friendly objects - they are players (the friendly flag
 * is cleared for players in some sections of the code frequently). I added them
 * to list so we can use it like it should work.
 * And second, the friendly list is a GLOBAL list - for ALL maps!
 * Assuming 100 players on 75 map will bulk the list with information that are only
 * useful to 1-2% for every player.
 *
 *
 * The real way to reimplement this list is a 2 way strategie:
 * First, pets should only allowed for players. NPCS/mob should be able to have a golem
 * but no pets. And even golems are a problem for npcs because we have no AI which can
 * move npc golems useful.
 * But golems are stored atm in the object structure, so we have always a direct control
 * about it.
 * For pets, we should implement a linked pet list in the player struct. This give us the
 * power to handle pets more intelligent and fast.
 * 
 * But the biggest part is to remove the friendly list as global list - the friendly list
 * must be attached to the map header. So, every map has then its own "friendly object" list.
 * This will work very fine with the tiled map system, because we can easy ask "is there a 
 * target in our map? no? then perhaps in a attached? no? then we must not search anymore
 * because it is always out of range."
 * 
 * This code is a MUST because a global list will counter our system we want install with
 * the tiled maps - a capsuled area which is loaded and which only interact which the needed
 * parts of our world (= content = objects).
 */

static int  friendly_list_count = 0;
/*
 * Add a new friendly object to the linked list of friendly objects.
 * No checking to see if the object is already in the linked list is done.
 */

void add_friendly_object(object *op)
{
    objectlink *ol;

    /* only add senseful here - give a note ... i use it as info -
     * this will change when we chain the friendly list to maps 
     */
    if (op->type != PLAYER && op->type != MONSTER && !QUERY_FLAG(op, FLAG_ALIVE) && !QUERY_FLAG(op, FLAG_MONSTER))
    {
        LOG(llevDebug, "DEBUG: friendly list called for non mob/player (%s - %s)\n", query_name(op), op->arch->name);
        return;
    }
    /* special case: player can be or not friendly - if in logon process, we don't gave
     * them friendly to avoid illegal (player is not on map but in login limbus) on this
     * list. This is not a bug but we will do it more clever in map attached friendly lists.
     */
    if (op->type == PLAYER && !QUERY_FLAG(op, FLAG_FRIENDLY))
        return;
    /* Add some error checking.  This shouldn't happen, but the friendly
     * object list usually isn't very long, and remove_friendly_object
     * won't remove it either.  Plus, it is easier to put a breakpoint in
     * the debugger here and see where the problem is happening. 
     */
    for (ol = first_friendly_object; ol != NULL; ol = ol->next)
    {
        if (ol->objlink.ob == op)
        {
            LOG(llevBug, "BUG: (bad bug!) add_friendly_object: Trying to add object already on list (%s)\n",
                query_name(op));
            return;
        }
    }

    friendly_list_count++;
    /*  LOG(llevDebug,"add f_obj %s (c:%d).\n",query_name(op), friendly_list_count);*/

    ol = first_friendly_object;
    first_friendly_object = get_objectlink(OBJLNK_FLAG_OB);
    first_friendly_object->objlink.ob = op;
    first_friendly_object->id = op->count;
    first_friendly_object->next = ol;
}

/*
 * Removes the specified object from the linked list of friendly objects.
 */

void remove_friendly_object(object *op)
{
    objectlink *this;

    /* only add senseful here - give a note ... i use it as info -
     * this will change when we chain the friendly list to maps 
     */
    if (op->type != PLAYER && op->type != MONSTER && !QUERY_FLAG(op, FLAG_ALIVE) && !QUERY_FLAG(op, FLAG_MONSTER))
    {
        LOG(llevDebug, "DEBUG: friendly list called for remove non mob/player (%s - %s)\n", query_name(op),
            op->arch->name);
        return;
    }

    /* special case: player can be or not friendly - if in logon process, we don't gave
     * them friendly to avoid illegal (player is not on map but in login limbus) on this
     * list. This is not a bug but we will do it more clever in map attached friendly lists.
     */
    if (op->type == PLAYER && !QUERY_FLAG(op, FLAG_FRIENDLY))
        return;

    CLEAR_FLAG(op, FLAG_FRIENDLY);
    if (!first_friendly_object)
    {
        LOG(llevBug, "BUG: remove_friendly_object called with empty friendly list, remove ob=%s\n", query_name(op));
        return;
    }
    /* if the first object happens to be the one, processing is pretty
     * easy.
     */
    if (first_friendly_object->objlink.ob == op)
    {
        this = first_friendly_object;
        first_friendly_object = this->next;
        free_objectlink_simple(this);
    }
    else
    {
        objectlink *prev    = first_friendly_object;

        for (this = first_friendly_object->next; this != NULL; this = this->next)
        {
            if (this->objlink.ob == op)
                break;
            prev = this;
        }
        if (this)
        {
            /* This should not happen.  But if it does, presumably the
               * call to remove it is still valid.
               */
            if (this->id != op->count)
            {
                LOG(llevBug, "BUG: remove_friendly_object, tags do no match, %s, %d != %d\n", query_name(op), op->count,
                    this->id);
            }
            prev->next = this->next;
            free_objectlink_simple(this);
        }
    }

    friendly_list_count--;
    /*  LOG(llevDebug,"remove f_obj %s (c:%d).\n",query_name(op), friendly_list_count);*/
}

/*
 * Dumps all friendly objects.  Invoked in DM-mode with the G key.
 */

void dump_friendly_objects()
{
    objectlink *ol;

    for (ol = first_friendly_object; ol != NULL; ol = ol->next)
        LOG(llevInfo, "%s (count: %d)\n", query_name(ol->objlink.ob), ol->objlink.ob->count);
}

/* New function, MSW 2000-1-14
 * It traverses the friendly list removing objects that should not be here
 * (ie, do not have friendly flag set, freed, etc)
 */
void clean_friendly_list()
{
    objectlink *this_link, *prev = NULL, *next;

    for (this_link = first_friendly_object; this_link; this_link = next)
    {
        next = this_link->next;
        if (!OBJECT_VALID(this_link->objlink.ob, this_link->id)
         || (!QUERY_FLAG(this_link->objlink.ob, FLAG_FRIENDLY) && this_link->objlink.ob->type != PLAYER))
        {
            if (prev)
            {
                prev->next = this_link->next;
            }
            else
            {
                first_friendly_object = this_link->next;
            }
            LOG(llevDebug, "clean_friendly_list: Removed bogus link: %s\n", query_name(this_link->objlink.ob));
            free_objectlink_simple(this_link);
        }
        /* If we removed the object, then prev is still valid.  */
        else
            prev = this_link;
    }
}

