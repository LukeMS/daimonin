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

#include <global.h>

static void Free(objectlink_t *ol);

/*
 *   I changed the objectlink module so it use not only the mempool
 *   system but also uses one struct for objectlink and objectlinkpt.
 *   This should not only speed up things and avoid memory fragmentation.
 *   The new union{} part of the objectlink will turn this link object now
 *   in a fast, easy to use and global used link for nearly every
 *   game structure, dynamic or static. MT-2004
 */

/*
 * Allocates a new objectlink structure, initialises it, and returns
 * a pointer to it.
 */

objectlink_t * objectlink_get(int id)
{
    objectlink_t *ol  = (objectlink_t *) get_poolchunk(pool_objectlink);
    memset(ol, 0, sizeof(objectlink_t));
    ol->flags = id;
    return ol;
}

/*
 * Recursively frees all linked list of objectlink pointers
 * WARNING: only call for with FLAG_IS_LINKED used
 * lists - friendly list or others handle their
 * objectlink malloc/free native.
*/

void objectlink_free(objectlink_t *ol)
{
    if (ol->next)
    {
        objectlink_free(ol->next);
    }

    if (ol->objlink.link)
    {
        Free(ol->objlink.link);
    }
}

/*
 * Recursively frees all objectlinks.
 * WARNING: only call for with FLAG_IS_LINKED used
 * lists - friendly list or others handle their
 * objectlink malloc/free native.
*/
/* free objectlink
 * and clean up linked objects
 */

static void Free(objectlink_t *ol)
{
    if (ol->next)
    {
        Free(ol->next);
    }

    if (OBJECT_VALID(ol->objlink.ob, ol->id))
        CLEAR_FLAG(ol->objlink.ob, FLAG_IS_LINKED);
    return_poolchunk(ol, pool_objectlink);
}


/* generic link function for objectlinks
 * update a start & end ptr is there is one.
 */
objectlink_t *objectlink_link(objectlink_t **startptr, objectlink_t **endptr,
                            objectlink_t *afterptr, objectlink_t *beforeptr, objectlink_t *objptr )
{
    if(!beforeptr) /* link it behind afterptr */
    {
        if(afterptr) /* if not, we just have to update startptr & endptr */
        {
            if(afterptr->next) /* link between something? */
            {
                objptr->next = afterptr->next;
                afterptr->next->prev = objptr;
            }
            afterptr->next = objptr;
            objptr->prev = afterptr;
        }

        if(startptr && !*startptr)
            *startptr = objptr;
        if(endptr && (!*endptr || *endptr == afterptr))
            *endptr = objptr;
    }
    else if(!afterptr) /* link it before beforeptr */
    {
        if(beforeptr->prev)
        {
            objptr->prev = beforeptr->prev;
            beforeptr->prev->next = objptr;
        }
        beforeptr->prev = objptr;
        objptr->next = beforeptr;

        /* we can't be endptr but perhaps start */
        if(startptr && (!*startptr || *startptr == beforeptr))
            *startptr = objptr;
    }
    else /* special: link together 2 lists/objects */
    {
        beforeptr->prev = objptr;
        afterptr->next = objptr;
        objptr->next = beforeptr;
        objptr->prev = afterptr;
    }

    return objptr;
}

/* generic unlink a objectlink from a list
 * update a end & start ptr if there is one.
 */
objectlink_t *objectlink_unlink(objectlink_t **startptr, objectlink_t **endptr, objectlink_t *objptr)
{
    if(startptr && *startptr == objptr)
        *startptr = objptr->next;
    if(endptr && *endptr == objptr)
        *endptr = objptr->prev;

    if(objptr->prev)
        objptr->prev->next = objptr->next;
    if(objptr->next)
        objptr->next->prev = objptr->prev;

    return objptr;
}
