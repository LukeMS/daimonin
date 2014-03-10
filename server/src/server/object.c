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
#ifndef WIN32 /* ---win32 exclude headers */
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#endif /* win32 */

static void RegrowBurdenTree(object *op, sint32 nrof, sint8 mode);
static void RemoveFromEnv(object *op);
static void RemoveFromMap(object *op);

static int static_walk_semaphore = FALSE; /* see walk_off/walk_on functions  */

/* The following arrays contain information necessary to check up to a 7x7 grid
 * of squares. */
/* freearr_x/y contain the x and y offsets for the 48 squares. The array
 * indices map to the squares as follows:
 *   43 44 45 46 47 48 25
 *   42 21 22 23 24 09 26
 *   41 20 07 08 01 10 27
 *   40 19 06 00 02 11 28
 *   39 18 05 04 03 12 29
 *   38 17 16 15 14 13 30
 *   37 36 35 34 33 32 31 */
int freearr_x[SIZEOFFREE] =
{
    0,
    0, 1, 1, 1, 0, -1, -1, -1, /* SIZEOFFREE1 */
    0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1, /* SIZEOFFREE2 */
    0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1 /* SIZEOFFREE */
};

int freearr_y[SIZEOFFREE] =
{
    0,
    -1, -1, 0, 1, 1, 1, 0, -1, /* SIZEOFFREE1 */
    -2, -2, -2, -1, 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, /* SIZEOFFREE2 */
    -3, -3, -3, -3, -2, -1, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3 /* SIZEOFFREE */
};

/* maxfree is now only used by free_dir(). It is used to cut a search short
 * when a wall is encountered. However, it produces some highly suspect
 * results. */
/* TODO: Replace with the same LOS logic used in find_free_spot() -- Smacky 20090121 */
int maxfree[SIZEOFFREE] =
{
    0,
    9, 10, 13, 14, 17, 18, 21, 22, /* SIZEOFFREE1 */
    25, 26, 27, 30, 31, 32, 33, 36, 37, 39, 39, 42, 43, 44, 45, 48, /* SIZEOFFREE2 */
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49 /* SIZEOFFREE */
};

/* freedir is the dir from 00 (the central square) to each index square. */
int freedir[SIZEOFFREE] =
{
    0,
    1, 2, 3, 4, 5, 6, 7, 8, /* SIZEOFFREE1 */
    1, 2, 2, 2, 3, 4, 4, 4, 5, 6, 6, 6, 7, 8, 8, 8, /* SIZEOFFREE2 */
    1, 2, 2, 2, 2, 2, 3, 4, 4, 4, 4, 4, 5, 6, 6, 6, 6, 6, 7, 8, 8, 8, 8, 8 /* SIZEOFFREE */
};

/* freeback and freeback2 are the indices you arrive at when you take one step
 * from the current square towards the central square. */
int freeback[SIZEOFFREE] =
{
    0,
    0, 0, 0, 0, 0, 0, 0, 0, /* SIZEOFFREE1 */
    1, 1, 2, 3, 3, 3, 4, 5, 5, 5, 6, 7, 7, 7, 8, 1, /*SIZEOFFREE1 */
    9, 9, 10, 11, 12, 13, 13, 13, 14, 15, 16, 17, 17, 17, 18, 19, 20, 21, 21, 21, 22, 23, 24, 9 /* SIZEOFFREE */
};

int freeback2[SIZEOFFREE] =
{
    0,
    0, 0, 0, 0, 0, 0, 0, 0, /* SIZEOFFREE1 */
    1, 2, 2, 2, 3, 4, 4, 4, 5, 6, 6, 6, 7, 8, 8, 8, /*SIZEOFFREE1 */
    9, 10, 11, 11, 11, 12, 13, 14, 15, 15, 15, 16, 17, 18, 19, 19, 19, 20, 21, 22, 22, 22, 23, 24 /* SIZEOFFREE */
};

/* find_free_spot(archetype, object, map, x, y, ins_flags, start, stop) will
 * search for a spot at the given map and coordinates which will be able to
 * contain the given archetype.  start and stop specifies how many squares
 * to search (see the freearr_x/y[] definition).
 * It returns a random choice among the alternatives found.
 * start and stop are where to start relative to the free_arr array (1,9
 * does all 4 immediate directions).  This returns the index into the
 * array of the free spot, -1 if no spot available (dir 0 = x,y)
 * Note - this only checks to see if there is space for the head of the
 * object - if it is a multispace object, this should be called for all
 * pieces. */
int find_free_spot(archetype *at, object *op, mapstruct *m, int x, int y, int ins_flags, int start, int stop)
{
    int         i,
                index = 0;
    static int  altern[SIZEOFFREE];
    object     *terrain = (op &&
                           op->terrain_flag &&
                           !(ins_flags & INS_IGNORE_TERRAIN)) ? op : NULL;

    /* Prevent invalid indexing. */
    start = MAX(0, MIN(start, SIZEOFFREE));
    stop = MAX(0, MIN(stop, SIZEOFFREE));

    for (i = start; i < stop; i++)
    {
        int bflag = 0;

        /* The archetype won't fit. */
        if (arch_blocked(at, terrain, m, x + freearr_x[i], y + freearr_y[i]))
            continue;

        if (i > 8 &&
            ins_flags & INS_WITHIN_LOS)
        {
            int j;

            for (j = freeback[i]; j; j = freeback[j])
            {
                if (wall(m, x + freearr_x[j], y + freearr_y[j]))
                {
                    bflag = 1;

                    break;
                }
            }

            if (wall(m, x + freearr_x[freeback2[i]], y + freearr_y[freeback2[i]]))
                bflag = 1;
        }

        if (!bflag)
            altern[index++] = i;
    }

    if (!index)
        return -1;

    return altern[RANDOM() % index];
}

/* find_first_free_spot(archetype, object, mapstruct, x, y) works like
 * find_free_spot(), but it will search max number of squares.
 * But it will return the first available spot, not a random choice.
 * Changed 0.93.2: Have it return -1 if there is no free spot available. */
int find_first_free_spot(archetype *at, object *op, mapstruct *m, int x, int y)
{
    int     i;
    object *terrain = (op && op->terrain_flag) ? op : NULL;

    for (i = 0; i < SIZEOFFREE; i++)
    {
        if (!arch_blocked(at, terrain, m, x + freearr_x[i], y + freearr_y[i]))
            return i;
    }

    return -1;
}

/* find_dir(map, x, y, exclude) will search some close squares in the
 * given map at the given coordinates for live objects.
 * It will not considered the object given as exlude among possible
 * live objects.
 * It returns the direction toward the first/closest live object if finds
 * any, otherwise 0. */
int find_dir(mapstruct *m, int x, int y, object *exclude)
{
    int         i, xt, yt, max = SIZEOFFREE;
    mapstruct  *mt;
    object     *tmp;

    if (exclude && exclude->head)
        exclude = exclude->head;

    for (i = 1; i < max; i++)
    {
        xt = x + freearr_x[i];
        yt = y + freearr_y[i];
        if (wall(m, xt, yt))
            max = maxfree[i];
        else
        {
            if (!(mt = out_of_map(m, &xt, &yt)))
                continue;
            tmp = GET_MAP_OB(mt, xt, yt);
            while (tmp
                != NULL
                && ((tmp != NULL && !QUERY_FLAG(tmp, FLAG_MONSTER) && tmp->type != PLAYER)
                 || (tmp == exclude || (tmp->head && tmp->head == exclude))))
                tmp = tmp->above;
            if (tmp != NULL)
                return freedir[i];
        }
    }
    return 0;
}

/* find_dir_2(delta-x,delta-y) will return a direction in which
 * an object which has subtracted the x and y coordinates of another
 * object, needs to travel toward it. */
int find_dir_2(int x, int y)
{
    int q;
    if (!y)
        q = -300 * x;
    else
        q = x * 100 / y;
    if (y > 0)
    {
        if (q < -242)
            return 3 ;
        if (q < -41)
            return 2 ;
        if (q < 41)
            return 1 ;
        if (q < 242)
            return 8 ;
        return 7 ;
    }
    if (q < -242)
        return 7 ;
    if (q < -41)
        return 6 ;
    if (q < 41)
        return 5 ;
    if (q < 242)
        return 4 ;
    return 3 ;
}

/* absdir(int): Returns a number between 1 and 8, which represent
 * the "absolute" direction of a number (it actually takes care of
 * "overflow" in previous calculations of a direction). */
int absdir(int d)
{
    if (d < 1)
        d +=((uint32)((d/-8)*8))+8;
    if (d > 8)
        d -= ((uint32)((d-1)/8))*8;
    return d;
}

/* dirdiff(dir1, dir2) returns how many 45-degrees differences there is
 * between two directions (which are expected to be absolute (see absdir()) */
int dirdiff(int dir1, int dir2)
{
    int d;
    d = abs(dir1 - dir2);
    if (d > 4)
        d = 8 - d;
    return d;
}

/*
 * Utility functions for inserting & removing objects
 * from activelists
 */
static inline void activelist_remove_inline(object *op)
{
    /* If not already on any active list, don't do anything */
    if(!QUERY_FLAG(op, FLAG_IN_ACTIVELIST))
        return;

#ifdef DEBUG_ACTIVELIST
    LOG(llevDebug, "DEBUG:: activelist_remove(): (%s[%d]) <- %s[%d], type:%d env:%s[%d] map:%s %d, %d -> (%s[%d])\n",
        STRING_OBJ_NAME(op->active_prev), TAG(op->active_prev),
        STRING_OBJ_NAME(op), TAG(op), op->type, STRING_OBJ_NAME(op->env),
        TAG(op->env), STRING_MAP_PATH(op->map), op->x, op->y,
        STRING_OBJ_NAME(op->active_next), TAG(op->active_next));
#endif

    /* If this happens to be the object we will process next,
     * update the next_active_object pointer */
    if(op == next_active_object)
        next_active_object = op->active_next;
    else if(op == inserted_active_objects)
        inserted_active_objects = op->active_next;

    if (op->active_prev)
        op->active_prev->active_next = op->active_next;

    if (op->active_next)
        op->active_next->active_prev = op->active_prev;

    CLEAR_FLAG(op, FLAG_IN_ACTIVELIST);
    op->active_next = NULL;
    op->active_prev = NULL;
}

/* Insert an object into the insertion activelist, it will be
 * moved to its corresponding map's activelist at the next
 * call to process_events() */
static inline void activelist_insert_inline(object *op)
{
    object *tmp;

    /* Do not insert if op is a descendant of a sys_object. */
    for (tmp = op->env; tmp; tmp = tmp->env)
        if (QUERY_FLAG(tmp, FLAG_SYS_OBJECT))
             return;

    /* If already on any active list, don't do anything */
    if(QUERY_FLAG(op, FLAG_IN_ACTIVELIST))
        return;

    /* Since we don't want to process objects twice, we make
     * sure to insert the object in a temporary list until the
     * next process_events() call. */
    op->active_next = inserted_active_objects;
    if (op->active_next != NULL)
        op->active_next->active_prev = op;
    inserted_active_objects = op;
    op->active_prev = NULL;

#ifdef DEBUG_ACTIVELIST
    LOG(llevDebug, "DEBUG:: activelist_insert(): (%s[%d]) <- %s[%d], type:%d env:%s[%d] map:%s %d, %d -> (%s[%d])\n",
        STRING_OBJ_NAME(op->active_prev), TAG(op->active_prev),
        STRING_OBJ_NAME(op), TAG(op), op->type, STRING_OBJ_NAME(op->env),
        TAG(op->env), STRING_MAP_PATH(op->map), op->x, op->y,
        STRING_OBJ_NAME(op->active_next), TAG(op->active_next));
#endif

    SET_FLAG(op, FLAG_IN_ACTIVELIST);
}

void activelist_insert(object *op)
{
    activelist_insert_inline(op);
}

void activelist_remove(object *op)
{
    activelist_remove_inline(op);
}

/* Put an object in the list of removal candidates.
 * If the object has still FLAG_REMOVED set at the end of the
 * server timestep it will be freed
 */
void mark_object_removed(object *ob)
{
    struct mempool_chunk   *mem = MEM_POOLDATA(ob);

    if (OBJECT_FREE(ob))
        LOG(llevBug, "BUG: mark_object_removed() called for free object\n");

    SET_FLAG(ob, FLAG_REMOVED);

    /* Don't mark objects twice */
    if (mem->next != NULL)
        return;

    /* We abuse the mempool freelist here. Need to zero mem->next out
     * before calling return_poolchunk() on the object */
    mem->next = removed_objects;
    removed_objects = mem;
}

/* Go through all objects in the removed list and free the forgotten ones */
void object_gc()
{
    struct mempool_chunk   *current, *next;
    object                 *ob;

#if defined DEBUG_GC
    if(removed_objects != &end_marker)
        LOG(llevDebug, "object_gc():\n");
#endif

    while ((next = removed_objects) != &end_marker)
    {
        removed_objects = &end_marker; /* destroy_object() may free some more objects (inventory items) */
#if defined DEBUG_GC
        LOG(llevDebug, " sweep\n");
#endif

        while (next != &end_marker)
        {
            current = next;
            next = current->next;
            current->next = NULL;

            ob = (object *) MEM_USERDATA(current);
            if (QUERY_FLAG(ob, FLAG_REMOVED))
            {
#if defined DEBUG_GC
                LOG(llevDebug, "  collect obj %s (%d)\n", STRING_OBJ_NAME(ob), ob->count);
#endif
                if (OBJECT_FREE(ob))
                    LOG(llevBug, "BUG: Freed object in remove list: %s\n", STRING_OBJ_NAME(ob));
                else
                {
                    return_poolchunk(ob, pool_object);
                }
            }
        }
    }
}

/* Function examines the 2 objects given to it, and returns true if
 * they can be merged together.
 *
 * Improvements made with merge:  Better checking on potion, and also
 * check weight. */
static inline int can_merge(object *ob1, object *ob2)
{
    /* Non-stackable objects of course cannot merge. */
    if (!QUERY_FLAG(ob1, FLAG_CAN_STACK) ||
        !QUERY_FLAG(ob2, FLAG_CAN_STACK))
        return 0;

    /* Coins merge according to arch only. */
    if (ob1->type == MONEY &&
        ob2->type == MONEY &&
        ob1->arch == ob2->arch)
        return 1;

    /* Gecko: Moved out special handling of event object nrof */
    if ((!ob1->nrof || !ob2->nrof) &&
         ob1->type != TYPE_EVENT_OBJECT)
        return 0;

    /* important: don't merge objects with glow_radius set - or we come
    * in heavy side effect situations. Because we really not know what
    * our calling function will do after this merge (and the calling function
    * then must first find out a merge has happen or not). The sense of stacks
    * are to store inactive items. Because glow_radius items can be active even
    * when not apllied, merging is simply wrong here. MT.
    */
    if (ob1->glow_radius ||
        ob2->glow_radius)
        return 0;

    /* just a brain dead long check for things NEVER NEVER should be different
     * this is true under all circumstances for all objects.
     */
    if (ob1->type != ob2->type
     || ob1 == ob2
     || ob1->arch != ob2->arch
     || ob1->sub_type1 != ob2->sub_type1
     || ob1->material != ob2->material
     || ob1->material_real != ob2->material_real
     || ob1->magic != ob2->magic
     || ob1->item_quality != ob2->item_quality
     || ob1->item_condition != ob2->item_condition
     || ob1->item_race != ob2->item_race
     || ob1->speed != ob2->speed
     || ob1->value != ob2->value
     || ob1->weight != ob2->weight
     || ob1->stats.food != ob2->stats.food)
        return 0;

    /* Gecko: added bad special check for event objects
     * Idea is: if inv is identical events only then go ahead and merge)
     * This goes hand in hand with the event keeping addition in get_split_ob()
     */
    if (ob1->inv ||
         ob2->inv)
    {
        object *tmp1 = ob1->inv,
               *tmp2 = ob2->inv;

        if (!tmp1 ||
            !tmp2)
        {
            return 0;
        }

        /* Check that all inv objects are event objects */
        for (; tmp1 && tmp2; tmp1 = tmp1->below, tmp2 = tmp2->below)
        {
            /* I don't understand how this relates to the for loops below.
             * Surely if both invs are event objects only (so we get through
             * this test) the following can_merge() test is redundant?
             * -- Smacky 20100712 */
            if (tmp1->type != TYPE_EVENT_OBJECT ||
                tmp2->type != TYPE_EVENT_OBJECT)
            {
                return 0;
            }
        }

        /* FIXME: This seems wrong. 1) this takes no account of the event types
         * of those objects. Is it OK to stack an object with an APPLY event
         * and one with a TRIGGER event for example. 3) even if the event types
         * are the same, the scripts might not be. 4) even if the events and
         * scripts are the same, the script data might not be.
         * -- Smacky 20100712 */
        /* Same number of events */
        if (tmp1 ||
            tmp2)
        {
            return 0;
        }

        for (tmp1 = ob1->inv; tmp1; tmp1 = tmp1->below)
        {
            for (tmp2 = ob2->inv; tmp2; tmp2 = tmp2->below)
            {
                if (can_merge(tmp1, tmp2))
                {
                    break;
                }
            }

            if (!tmp2) /* Couldn't find something to merge event from ob1 with? */
            {
                return 0;
            }
        }
    }

    /* check the refcount pointer */
    if (ob1->name != ob2->name
     || ob1->title != ob2->title
     || ob1->race != ob2->race
     || ob1->slaying != ob2->slaying
     || ob1->msg != ob2->msg)
        return 0;

    /* compare the static arrays/structs */
    if ((memcmp(&ob1->stats, &ob2->stats, sizeof(living)) != 0)
     || (memcmp(&ob1->resist, &ob2->resist, sizeof(ob1->resist)) != 0)
     || (memcmp(&ob1->attack, &ob2->attack, sizeof(ob1->attack)) != 0))
        return 0;


    /* very special check... we have to ignore the applied flag
     * for stacked items like arrows. This will be enhanced perhaps
     * in the future for more stacked and applyable items.
     */
    if(ob1->flags[2] != ob2->flags[2])
    {
        if(ob1->type == ARROW)
        {
            if((ob1->flags[2] | 0x200000) != (ob2->flags[2] | 0x200000))
                return 0;

        }
        else
            return 0;
    }


    if (ob1->randomitems != ob2->randomitems
     || ob1->other_arch != ob2->other_arch
     || (ob1->flags[0] | 0x70000) != (ob2->flags[0] | 0x70000)
     ||   /* we ignore REMOVED and BEEN_APPLIED */
        ob1->flags[1] != ob2->flags[1]
     || ob1->flags[3] != ob2->flags[3]
     || ob1->flags[4] != ob2->flags[4]
     || ob1->path_attuned != ob2->path_attuned
     || ob1->path_repelled != ob2->path_repelled
     || ob1->path_denied != ob2->path_denied
     || ob1->terrain_type != ob2->terrain_type
     || ob1->terrain_flag != ob2->terrain_flag
     || ob1->weapon_speed != ob2->weapon_speed
     || ob1->magic != ob2->magic
     || ob1->item_level != ob2->item_level
     || ob1->item_skill != ob2->item_skill
     || ob1->level != ob2->level
     || ob1->max_buffs != ob2->max_buffs)
        return 0;

    /* face can be difficult - but inv_face should never different or obj is different! */
    if (ob1->face != ob2->face
     || ob1->inv_face != ob2->inv_face
     || ob1->animation_id != ob2->animation_id
     || ob1->inv_animation_id != ob2->inv_animation_id)
        return 0;

    /* We should avoid merging empty containers too. Gecko 2005-10-09 */
    if (ob1->type == CONTAINER)
        return 0;

    /* some stuff we should not need to test:
     * carrying: because container merge isa big nono - and we tested ->inv before. better no double use here.
     * weight_limit: same reason like carrying - add when we double use for stacking items
     * last_heal;
     * last_sp;
     * last_grace;
     * sint16 last_eat;
     * run_away;
     * stealth;
     * hide;
     * move_type;
     * layer;               this *can* be different for real same item - watch it
     * anim_speed;           this can be interesting...
     */

    return 1; /* can merge! */
}

/*
 * merge_ob(op,tmp):
 *
 * This function goes through all objects above and including tmp, and
 * merges the first matching object with op.
 * If tmp is NULL, it is calculated.
 * A mergeable object must have FLAG_CAN_STACK and nrof > 0. The two objects
 * to merge must then pass the can_merge() test (above). If successful,
 * tmp->nrof is added to op->nrof then set to 0 (meaning tmp is no longer
 * mergeable). tmp is also marked for removal.
 * Returns pointer to op if it succeded in the merge, otherwise NULL
 */
object * merge_ob(object *op, object *tmp)
{
    /* Nothing to merge with or op is non-stackable or has nrof 0, */
    if (!op || !QUERY_FLAG(op, FLAG_CAN_STACK) || !op->nrof)
        return NULL;

    /* Calculate tmp. It is a bug to attempt a merge with an obj in limbo. */
    if (tmp == NULL)
    {
        if (op->map)
            tmp = GET_MAP_OB(op->map, op->x, op->y);
        else if (op->env)
            for (tmp = op->env->inv; tmp && tmp->below; tmp = tmp->below)
                ;
        else
        {
            LOG(llevBug, "BUG:: %s/merge_ob(): op (%s[%d]) has neither map nor environment!",
                __FILE__, STRING_OBJ_NAME(op), TAG(op));

            return NULL;
        }
    }

    /* Ascend through tmps sibling, looking for a match with op. */
    for (; tmp; tmp = tmp->above)
    {
        /* Don't merge with yourself! */
        if (tmp == op)
            continue;

        /* Just merge with the first match. This should be fine as previous
         * merges will have taken care of other matches: there should never be
         * a situation where there are more than two mergeable objects on a
         * square/in an env. */
        if (tmp->nrof && can_merge(op, tmp))
        {
#if DEBUG_MERGE_OB
            LOG(llevInfo,"INFO:: %s:merge_ob(): Merging %s[%d] = %d to %s[%d] = %d!\n",
                __FILE__,
                STRING_OBJ_NAME(tmp), TAG(tmp), tmp->nrof,
                STRING_OBJ_NAME(op), TAG(op), op->nrof);
#endif
            op->nrof = op->nrof + tmp->nrof;
            CLEAR_FLAG(tmp, FLAG_NO_SEND);
            remove_ob(tmp);

            if (op->env &&
                !QUERY_FLAG(op, FLAG_SYS_OBJECT))
            {
                RegrowBurdenTree(op, op->nrof, 1);
            }

            return op;
        }
    }

    return NULL;
}


/* calculates the weight an object is carrying.
 * its not recursive itself but the caller will take care of it.
 * thats for example the recursive flex loader and map.c/LoadObjects().
 */
sint32 sum_weight(object *op)
{
    sint32  sum;
    object *inv;

    for (sum = 0, inv = op->inv; inv != NULL; inv = inv->below)
    {
        if (QUERY_FLAG(inv, FLAG_SYS_OBJECT))
            continue;

        if(inv->type == CONTAINER && inv->weapon_speed != 1.0f)
            sum += inv->damage_round_tag + inv->weight; /* thats the precalculated, modified weight + container weight */
        else
            sum += WEIGHT(inv);
    }

    /* we have a magical container? then precalculate the modified weight */
    if(op->type == CONTAINER && op->weapon_speed != 1.0f)
        op->damage_round_tag = (uint32)((float)sum * op->weapon_speed);

    return sum;
}

/* Changes the carrying value of any envs of op by nrof * op->weight. The
 * direction of change is specified by mode (positive or negative). */
static void RegrowBurdenTree(object *op, sint32 nrof, sint8 mode)
{
    object *where;
    sint32  weight;

    /* If op is not in an env or has no weight, we have nothing to do. */
    if (!(where = op->env) ||
        !(weight = (op->type == CONTAINER &&
                    op->weapon_speed != 1.0)
                   ? op->damage_round_tag + op->weight // no stacking container, ignore nrof
                   : WEIGHT_NROF(op, nrof)))
    {
        return;
    }

    /* Ensure mode is either 1 or -1 then multiply weight by this. */
    mode = (mode >= 0) ? 1 : -1;
    weight *= mode;

    /* Loop through the ancestors (envs) of op, adjusting their carrying and
     * notifying clients as necessary. */
    do
    {
        /* A sys object breaks the chain. */
        if (QUERY_FLAG(where, FLAG_SYS_OBJECT))
        {
            break;
        }

        if (where->type == CONTAINER)
        {
            where->carrying += weight;

            /* A magical container modifying the weight by its magic. */
            if (where->weapon_speed != 1.0)
            {
                weight = where->damage_round_tag;
                where->damage_round_tag = (uint32)((float)where->carrying *
                                                   where->weapon_speed);
                weight = (mode < 0)
                         ? weight - where->damage_round_tag
                         : where->damage_round_tag - weight;
            }
        }
        else
        {
            where->carrying += weight;
        }

        esrv_update_item(UPD_WEIGHT, where);
    }
    while ((where = where->env));
}

/*
 * Eneq(@csd.uu.se): Since we can have items buried in a character we need
 * a better check.  We basically keeping traversing up until we can't
 * or find a player.
 */
/* this function was wrong used in the past. Its only senseful for fix_player() - for
 * example we remove a active force from a player which was inserted in a special
 * force container (for example a exp object). For inventory views, we DON'T need
 * to update the item then! the player only sees his main inventory and *one* container.
 * is this object in a closed container, the player will never notice any change.
 */
object * is_player_inv(object *op)
{
    for (; op != NULL && op->type != PLAYER; op = op->env)
        if (op->env == op)
            op->env = NULL;
    return op;
}

/*
 * Used by: Server DM commands: dumpbelow, dump.
 *  Some error messages.
 * The result of the dump is stored in the static global errmsg array.
 */

#if 0
void dump_object2(const object *op)
{
    char   *cp;
    /*  object *tmp;*/

    if (op &&
        op->arch)
    {
        sprintf(strchr(errmsg, '\0'), "arch %s\n", STRING_OBJ_ARCH_NAME(op));
        sprintf(strchr(errmsg, '\0'), "count %d\n", op->count);
        if ((cp = get_ob_diff(op, &archetype_global._empty_archetype->clone)) != NULL)
            strcat(errmsg, cp);
#if 0
      /* Don't dump player diffs - they are to long, mostly meaningless, and
       * will overflow the buffer.
       * Changed so that we don't dump inventory either.  This may
       * also overflow the buffer.
       */
      if(op->type!=PLAYER && (cp=get_ob_diff(op,&archetype_global._empty_archetype->clone))!=NULL)
        strcat(errmsg,cp);
      for (tmp=op->inv; tmp; tmp=tmp->below)
        dump_object2(tmp);
#endif
        strcat(errmsg, "end\n");
    }
    else
    {
        sprintf(strchr(errmsg, '\0'), "Object %s\n", STRING_OBJ_NAME(op));
#if 0
      if((cp=get_ob_diff(op,&archetype_global._empty_archetype->clone))!=NULL)
        strcat(errmsg,cp);
      for (tmp=op->inv; tmp; tmp=tmp->below)
        dump_object2(tmp);
#endif
        strcat(errmsg, "end\n");
    }
}
#else
void dump_object2(const object *op)
{
    char   *cp;
    /*  object *tmp;*/

    if (op->arch != NULL)
    {
        cp = get_ob_diff(op, &archetype_global._empty_archetype->clone);
        sprintf(strchr(errmsg, '\0'), "arch %s\ncount %d\n%send\n",
                STRING_OBJ_ARCH_NAME(op), op->count, (cp) ? cp : "");
    }
    else
    {
        sprintf(strchr(errmsg, '\0'), "Object %s\nend\n", STRING_OBJ_NAME(op));
    }
}
#endif

/*
 * Dumps an object.  Returns output in the static global errmsg array.
 */

void dump_object(const object *op)
{
    if (op == NULL)
    {
        strcpy(errmsg, "[NULL pointer]");
        return;
    }
    errmsg[0] = '\0';
    dump_object2(op);
}

/* GROS - Dumps an object. Return the result into a string                   */
/* Note that no checking is done for the validity of the target string, so   */
/* you need to be sure that you allocated enough space for it.               */
/* JRG 13-May-2009 Additional parameter tells this function size of buffer.  */
void dump_me(object *op, char *outstr, size_t bufsize)
{
    char   *cp;

    if (op == NULL)
    {
        strcpy(outstr, "[NULL pointer]");
        return;
    }
    outstr[0] = '\0';

    if (op->arch != NULL)
    {
        sprintf(strchr(outstr, '\0'), "arch %s\n", STRING_OBJ_ARCH_NAME(op));
        if ((cp = get_ob_diff(op, &archetype_global._empty_archetype->clone)) != NULL)
        {
            // JRG The 32 here is to allow for the arch/name/end text
            strcat(outstr, ((strlen(cp) + 32) > bufsize) ? "(overflow)" : cp);
        }
        strcat(outstr, "end");
    }
    else
    {
        sprintf(strchr(outstr, '\0'), "Object %s\nend", STRING_OBJ_NAME(op));
    }
}

void free_all_object_data()
{
    unsigned int allo = *pool_object->nrof_allocated,
                 free = *pool_object->nrof_free;

    LOG(llevDebug, "%u allocated objects, %u free objects\n",
        allo, free);
}

/*
 * Returns the object which this object marks as being the owner.
 * A id-scheme is used to avoid pointing to objects which have been
 * freed and are now reused.  If this is detected, the owner is
 * set to NULL, and NULL is returned.
 * (This scheme should be changed to a refcount scheme in the future)
 */

object * get_owner(object *op)
{
    if (!op || op->owner == NULL)
        return NULL;
    if (!OBJECT_FREE(op) && op->owner->count == op->owner_count)
        return op->owner;
    op->owner = NULL,op->owner_count = 0;
    return NULL;
}

void clear_owner(object *op)
{
    if (!op)
        return;

    /*
       if (op->owner && op->owner_count == op->owner->count)
    op->owner->refcount--;
       */

    op->owner = NULL;
    op->owner_count = 0;
}


/*
 * Sets the owner of the first object to the second object.
 * Also checkpoints a backup id-scheme which detects freeing (and reusage)
 * of the owner object.
 * See also get_owner()
 */

static void set_owner_simple(object *op, object *owner)
{
    /* next line added to allow objects which own objects */
    /* Add a check for owner_counts in here, as I got into an endless loop
     * with the fireball owning a poison cloud which then owned the
     * fireball.  I believe that was caused by one of the objects getting
     * freed and then another object replacing it.  Since the owner_counts
     * didn't match, this check is valid and I believe that cause is valid.
     */
    while (owner->owner && owner != owner->owner && owner->owner_count == owner->owner->count)
        owner = owner->owner;

    /* IF the owner still has an owner, we did not resolve to a final owner.
     * so lets not add to that.
     */
    if (owner->owner)
        return;

    op->owner = owner;

    op->owner_count = owner->count;
    /*owner->refcount++;*/
}


/*
 * Sets the owner and sets the skill and exp pointers to owner's current
 * skill and experience objects.
 */
void set_owner(object * const op, object * const owner)
{
    if (owner == NULL || op == NULL)
        return;
    set_owner_simple(op, owner);

    if (owner->type == PLAYER && owner->chosen_skill)
    {
        op->chosen_skill = owner->chosen_skill;
        op->skillgroup = owner->chosen_skill->skillgroup;
    }
}

/* Set the owner to clone's current owner and set the skill and experience
 * objects to clone's objects (typically those objects that where the owner's
 * current skill and experience objects at the time when clone's owner was
 * set - not the owner's current skill and experience objects).
 *
 * Use this function if player created an object (e.g. fire bullet, swarm
 * spell), and this object creates further objects whose kills should be
 * accounted for the player's original skill, even if player has changed
 * skills meanwhile.
 */
void copy_owner(object *op, object *clone)
{
    object *owner   = get_owner(clone);
    if (owner == NULL)
    {
        /* players don't have owners - they own themselves.  Update
         * as appropriate.
         */
        if (clone->type == PLAYER)
            owner = clone;
        else
            return;
    }
    set_owner_simple(op, owner);

    if (clone->chosen_skill)
    {
        op->chosen_skill = clone->chosen_skill;
        op->skillgroup = clone->skillgroup;
    }
}

/*
 * initialize_object().
 * We don't should have here any valid ptr - having still valid
 * pointer when our object is inside the memorypool should be a bug.
 * initializes all variables and flags to default settings.
 * If used on any object structures, be sure all pointers of the
 * base object are cleared until they are wiped with this function
 */
void initialize_object(object *op)
{
    memset(op, 0, sizeof(object));
    op->name = op->title = op->race = op->slaying = op->msg = NULL;
    /* Set some values that should not be 0 by default */
    op->anim_enemy_dir = -1;      /* control the facings 25 animations */
    op->anim_moving_dir = -1;     /* the same for movement */
    op->anim_enemy_dir_last = -1;
    op->anim_moving_dir_last = -1;
    op->anim_last_facing = 4;
    op->anim_last_facing_last = -1;

    op->face = blank_face;
    op->attacked_by_count = -1;

    /* give the object a new (unique) count tag */
    op->count = ++ob_count;
    op->count_debug=op->count;
}

/*
 * copy object first frees everything allocated by the second object,
 * and then copies the contends of the first object into the second
 * object, allocating what needs to be allocated.
 */
void copy_object(object *src, object *dest)
{
    int is_removed  = QUERY_FLAG(dest, FLAG_REMOVED);

    /* remove because perhaps speed will change when we copy */
    if(QUERY_FLAG(dest, FLAG_IN_ACTIVELIST))
        activelist_remove_inline(dest);

    FREE_ONLY_HASH(dest->name);
    FREE_ONLY_HASH(dest->title);
    FREE_ONLY_HASH(dest->race);
    FREE_ONLY_HASH(dest->slaying);
    FREE_ONLY_HASH(dest->msg);

    /* unlink old treasurelist if needed */
    if (dest->randomitems)
        unlink_treasurelists(dest->randomitems, 0);

    (void) memcpy((void *) ((char *) dest + offsetof(object, name)), (void *) ((char *) src + offsetof(object, name)),
                  sizeof(object) - offsetof(object, name));

    if (is_removed)
        SET_FLAG(dest, FLAG_REMOVED);

    ADD_REF_NOT_NULL_HASH(dest->name);
    ADD_REF_NOT_NULL_HASH(dest->title);
    ADD_REF_NOT_NULL_HASH(dest->race);
    ADD_REF_NOT_NULL_HASH(dest->slaying);
    ADD_REF_NOT_NULL_HASH(dest->msg);

#if 0
/* Buggers up merging. These are 3 distinct flags and should be tested for
 * separately. As such, IDENTIFIED should not set the other two in any case.
 * Especially, in a function whose purpose is to copy an object, we should not
 * then modify the copy so it is no longer identical to the original.
 * -- Smacky 20090312 */
    if (QUERY_FLAG(dest, FLAG_IDENTIFIED))
    {
        if (is_magical(dest))
            SET_FLAG(dest, FLAG_KNOWN_MAGICAL);

        if (is_cursed_or_damned(dest))
            SET_FLAG(dest, FLAG_KNOWN_CURSED);
    }
#endif

    /* perhaps we have a custom treasurelist. Then we need to
     * increase the refcount here.
     */
    if (dest->randomitems && (dest->randomitems->flags & OBJLNK_FLAG_REF))
        dest->randomitems->ref_count++;

    /* We set the custom_attrset pointer to NULL to avoid
     * really bad problems. TODO. this needs to be handled better
     * but it works until its only the player struct.
     */
    dest->custom_attrset = NULL;

    /* Only alter speed_left when we sure we have not done it before */
    if (dest->speed < 0 && dest->speed_left == dest->arch->clone.speed_left)
        dest->speed_left += (RANDOM() % 90) / 100.0f;

    CLEAR_FLAG(dest, FLAG_IN_ACTIVELIST); /* perhaps our source is on active list - ignore! */
    update_ob_speed(dest);
}

/* Same as above, but not touching the active list */
void copy_object_data(object *op2, object *op)
{
    int is_removed  = QUERY_FLAG(op, FLAG_REMOVED);

    FREE_ONLY_HASH(op->name);
    FREE_ONLY_HASH(op->title);
    FREE_ONLY_HASH(op->race);
    FREE_ONLY_HASH(op->slaying);
    FREE_ONLY_HASH(op->msg);

    /* unlink old treasurelist if needed */
    if (op->randomitems)
        unlink_treasurelists(op->randomitems, 0);

    (void) memcpy((void *) ((char *) op + offsetof(object, name)), (void *) ((char *) op2 + offsetof(object, name)),
                  sizeof(object) - offsetof(object, name));

    if (is_removed)
        SET_FLAG(op, FLAG_REMOVED);

    ADD_REF_NOT_NULL_HASH(op->name);
    ADD_REF_NOT_NULL_HASH(op->title);
    ADD_REF_NOT_NULL_HASH(op->race);
    ADD_REF_NOT_NULL_HASH(op->slaying);
    ADD_REF_NOT_NULL_HASH(op->msg);

#if 0
/* Buggers up merging. These are 3 distinct flags and should be tested for
 * separately. As such, IDENTIFIED should not set the other two in any case.
 * Especially, in a function whose purpose is to copy an object, we should not
 * then modify the copy so it is no longer identical to the original.
 * -- Smacky 20090312 */
    if (QUERY_FLAG(op, FLAG_IDENTIFIED))
    {
        if (is_magical(op))
            SET_FLAG(op, FLAG_KNOWN_MAGICAL);

        if (is_cursed_or_damned(op))
            SET_FLAG(op, FLAG_KNOWN_CURSED);
    }
#endif

    /* perhaps we have a custom treasurelist. Then we need to
     * increase the refcount here.
     */
    if (op->randomitems && (op->randomitems->flags & OBJLNK_FLAG_REF))
        op->randomitems->ref_count++;

    /* We set the custom_attrset pointer to NULL to avoid
     * really bad problems. TODO. this needs to be handled better */
    op->custom_attrset = NULL;
}

/*
 * get_object() grabs an object from the list of unused objects, makes
 * sure it is initialised, and returns it.
 * If there are no free objects, expand_objects() is called to get more.
 */

object * get_object()
{
    object *new_obj = (object *) get_poolchunk(pool_object);
    mark_object_removed(new_obj);
    return new_obj;
}

/*
 * If an object with the IS_TURNABLE() flag needs to be turned due
 * to the closest player being on the other side, this function can
 * be called to update the face variable, _and_ how it looks on the map.
 */

void update_turn_face(object *op)
{
    if (!QUERY_FLAG(op, FLAG_IS_TURNABLE) || op->arch == NULL)
        return;
    SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
    update_object(op, UP_OBJ_FACE);
}

/*
 * Updates the speed of an object.  If the speed changes from 0 to another
 * value, or vice versa, then add/remove the object from the active list.
 * This function needs to be called whenever the speed of an object changes.
 */

void update_ob_speed(object *op)
{
    /* No reason putting the archetypes objects on the speed list,
     * since they never really need to be updated.
     */


    if (OBJECT_FREE(op) && op->speed)
    {
        dump_object(op);
        LOG(llevBug, "BUG: Object %s is freed but has speed.\n:%s\n", op->name, errmsg);
        op->speed = 0;
    }

    /* these are special case objects - they have speed set, but should not put
     * on the active list.
     */
    if (arch_init || op->type == SPAWN_POINT_MOB)
        return;

    if (FABS(op->speed) > MIN_ACTIVE_SPEED)
        activelist_insert_inline(op);
    else
        activelist_remove_inline(op);
}

/* OLD NOTES
 * update_object() updates the array which represents the map.
 * It takes into account invisible objects (and represent squares covered
 * by invisible objects by whatever is below them (unless it's another
 * invisible object, etc...)
 * If the object being updated is beneath a player, the look-window
 * of that player is updated (this might be a suboptimal way of
 * updating that window, though, since update_object() is called _often_)
 *
 * action is a hint of what the caller believes need to be done.
 * For example, if the only thing that has changed is the face (due to
 * an animation), we don't need to call update_position until that actually
 * comes into view of a player.  OTOH, many other things, like addition/removal
 * of walls or living creatures may need us to update the flags now.
 * current action are:
 * UP_OBJ_INSERT: op was inserted
 * UP_OBJ_REMOVE: op was removed
 * UP_OBJ_CHANGE: object has somehow changed.  In this case, we always update
 *  as that is easier than trying to look at what may have changed.
 * UP_OBJ_FACE: only the objects face has changed.
 */
/* i want use this function as one and only way to decide what we update in a tile
 * and how - and what not. As smarter this function is, as better. This function MUST
 * be called now from everything what does a noticable change to a object. We can
 * pre-decide its needed to call but normally we must call this.
 */
void update_object(object *op, int action)
{
    MapSpace   *msp;
    int         flags, newflags;

    /*LOG(llevNoLog, "update_object: %s (%d,%d) - action %x\n", op->name, op->x, op->y,action);*/
    if (op == NULL)
    {
        /* this should never happen */
        LOG(llevError, "ERROR: update_object() called for NULL object.\n");
        return;
    }

    if (op->env != NULL)
    {
        // Send the player the new inventory information if needed.
        if (op->env->type == PLAYER)
        {
            esrv_update_item(UPD_FACE | UPD_ANIM | UPD_ANIMSPEED, op);
        }
        return;
    }

    if (!op->map || op->map->in_memory == MAP_SAVING)
    {
        return;
    }

    /* make sure the object is within map boundaries */
    /*
       if (op->x < 0 || op->x >= MAP_WIDTH(op->map) || op->y < 0 || op->y >= MAP_HEIGHT(op->map))
    {
           LOG(llevError,"ERROR: update_object() called for object out of map!\n");
        return;
       }
    */

    msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y);

    if (action == UP_OBJ_FACE) /* no need to change anything except the map update counter */
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_FACE - %s\n", query_name(op));
#endif
        esrv_update_item(UPD_FACE | UPD_ANIM | UPD_ANIMSPEED, op);

        return;
    }

    newflags = msp->flags;
    flags = newflags & ~P_NEED_UPDATE;

    switch(action)
    {
        case UP_OBJ_INSERT: /* always resort layer - but not always flags */
#ifdef DEBUG_CORE
            LOG(llevDebug, "UO_INS - %s\n", query_name(op));
#endif
            newflags |= P_NEED_UPDATE; /* force layer rebuild */

            if (!QUERY_FLAG(op, FLAG_NO_SEND))
            {
                esrv_send_item(op);
            }

            /* handle lightning system */
            if (op->glow_radius)
                adjust_light_source(op->map, op->x, op->y, op->glow_radius);

            /* this is handled a bit more complex, we must always loop the flags! */
            if (QUERY_FLAG(op, FLAG_NO_PASS) || QUERY_FLAG(op, FLAG_PASS_THRU) || QUERY_FLAG(op, FLAG_PASS_ETHEREAL))
                newflags |= P_FLAGS_UPDATE;
            else /* ok, we don't must use flag loop - we can set it by hand! */
            {
                if (op->type == CHECK_INV)
                    newflags |= P_CHECK_INV;
                else if (op->type == MAGIC_EAR)
                    newflags |= P_MAGIC_EAR;
                else if (op->type == GRAVESTONE)
                    newflags |= P_PLAYER_GRAVE;

                if (QUERY_FLAG(op, FLAG_ALIVE))
                {
                    newflags |= P_IS_ALIVE;
                    if(op->type==MONSTER && OBJECT_VALID(op->owner, op->owner_count)
                            && op->owner->type == PLAYER)
                        newflags |= P_IS_PLAYER_PET;
                }
                if (QUERY_FLAG(op, FLAG_IS_PLAYER))
                    newflags |= P_IS_PLAYER;
                if (QUERY_FLAG(op, FLAG_PLAYER_ONLY))
                    newflags |= P_PLAYER_ONLY;
                if (QUERY_FLAG(op, FLAG_BLOCKSVIEW))
                    newflags |= P_BLOCKSVIEW;
                if (QUERY_FLAG(op, FLAG_NO_MAGIC))
                    newflags |= P_NO_MAGIC;
                if (QUERY_FLAG(op, FLAG_NO_CLERIC))
                    newflags |= P_NO_CLERIC;
                if (QUERY_FLAG(op, FLAG_WALK_ON))
                    newflags |= P_WALK_ON;
                if (QUERY_FLAG(op, FLAG_FLY_ON))
                    newflags |= P_FLY_ON;
                if (QUERY_FLAG(op, FLAG_WALK_OFF))
                    newflags |= P_WALK_OFF;
                if (QUERY_FLAG(op, FLAG_FLY_OFF))
                    newflags |= P_FLY_OFF;
                if (QUERY_FLAG(op, FLAG_DOOR_CLOSED))
                    newflags |= P_DOOR_CLOSED;
                if (QUERY_FLAG(op, FLAG_CAN_REFL_SPELL))
                    newflags |= P_REFL_SPELLS;
                if (QUERY_FLAG(op, FLAG_CAN_REFL_MISSILE))
                    newflags |= P_REFL_MISSILE;
            }
            break;
        case UP_OBJ_REMOVE:
#ifdef DEBUG_CORE
            LOG(llevDebug, "UO_REM - %s\n", query_name(op));
#endif
            newflags |= P_NEED_UPDATE; /* force layer rebuild */

            if (!QUERY_FLAG(op, FLAG_NO_SEND))
            {
                esrv_del_item(op);
            }

            /* we don't handle floor tile light/darkness setting here -
             * we assume we don't remove a floor tile ever before dropping
             * the map.
             */

            /* handle lightning system */
            if (op->glow_radius)
                adjust_light_source(op->map, op->x, op->y, -(op->glow_radius));

            /* we must rebuild the flags when one of this flags is touched from our object */
            if (QUERY_FLAG(op, FLAG_ALIVE)
             || QUERY_FLAG(op, FLAG_IS_PLAYER)
             || QUERY_FLAG(op, FLAG_BLOCKSVIEW)
             || QUERY_FLAG(op, FLAG_DOOR_CLOSED)
             || QUERY_FLAG(op, FLAG_PASS_THRU)
             || QUERY_FLAG(op, FLAG_PASS_ETHEREAL)
             || QUERY_FLAG(op, FLAG_NO_PASS)
             || QUERY_FLAG(op, FLAG_PLAYER_ONLY)
             || QUERY_FLAG(op, FLAG_NO_MAGIC)
             || QUERY_FLAG(op, FLAG_NO_CLERIC)
             || QUERY_FLAG(op, FLAG_WALK_ON)
             || QUERY_FLAG(op, FLAG_FLY_ON)
             || QUERY_FLAG(op, FLAG_WALK_OFF)
             || QUERY_FLAG(op, FLAG_FLY_OFF)
             || QUERY_FLAG(op, FLAG_CAN_REFL_SPELL)
             || QUERY_FLAG(op, FLAG_CAN_REFL_MISSILE)
             || op->type == CHECK_INV
             || op->type == MAGIC_EAR
             || op->type == GRAVESTONE)
                newflags |= P_FLAGS_UPDATE; /* force flags rebuild */
            break;
        case UP_OBJ_FLAGS:
#ifdef DEBUG_CORE
            LOG(llevDebug, "UO_FLAGS - %s\n", query_name(op));
#endif
            newflags |= P_FLAGS_UPDATE; /* force flags rebuild but no tile counter*/
            break;
        case UP_OBJ_FLAGFACE:
#ifdef DEBUG_CORE
            LOG(llevDebug, "UO_FLAGFACE - %s\n", query_name(op));
#endif
            newflags |= P_FLAGS_UPDATE; /* force flags rebuild */
            esrv_update_item(UPD_FACE | UPD_ANIM | UPD_ANIMSPEED, op);
            break;
        case UP_OBJ_LAYER:
#ifdef DEBUG_CORE
            LOG(llevDebug, "UO_LAYER - %s\n", query_name(op));
#endif
            map_set_slayers(msp, op, 1); // must have uptodate slayers if we want to rebuild the clayers!
            newflags |= P_NEED_UPDATE; /* rebuild layers - most common when we change visibility of the object */

            if (!QUERY_FLAG(op, FLAG_NO_SEND))
            {
                esrv_send_or_del_item(op);
            }

            break;
        case UP_OBJ_ALL:
#ifdef DEBUG_CORE
            LOG(llevDebug, "UO_ALL - %s\n", query_name(op));
#endif
            newflags |= (P_FLAGS_UPDATE | P_NEED_UPDATE); /* force full tile update */
            break;
        default:
            LOG(llevError, "ERROR: update_object called with invalid action: %d\n", action);
            return;
    }

    if (flags != newflags)
    {
        if (newflags & (P_FLAGS_UPDATE))  /* rebuild flags */
        {
            msp->flags |= (newflags | P_NO_ERROR | P_FLAGS_ONLY);
            update_position(op->map, NULL, op->x, op->y);
        }
        else
            msp->flags |= newflags;
    }

    if (op->more != NULL)
        update_object(op->more, action);
}

/* Drops the inventory of ob into ob's current environment. */
/* Makes some decisions whether to actually drop or not, and/or to
 * create a corpse for the stuff */
void drop_ob_inv(object *ob)
{
    player *pl      = NULL;
    object *corpse  = NULL;
    object *tmp_op  = NULL;
    object *gtmp, *tmp     = NULL;

    if (ob->type == PLAYER)
    {
        /* we don't handle players here */
        LOG(llevBug, "BUG: drop_ob_inv() - try to drop items of %s\n",
            ob->name);

        return;
    }

    if (!ob->env &&
        (!ob->map ||
         ob->map->in_memory != MAP_ACTIVE))
    {
        /* TODO */
        LOG(llevDebug, "BUG: drop_ob_inv() - can't drop inventory of objects not in map yet: %s (%s)\n",
            STRING_OBJ_NAME(ob), STRING_OBJ_MAP_PATH(ob));

        return;
    }

    if (ob->enemy &&
        ob->enemy->type == PLAYER &&
        ob->enemy_count == ob->enemy->count)
    {
        pl = CONTR(ob->enemy);
    }

    /* create corpse and/or drop stuff to floor */
    if ((QUERY_FLAG(ob, FLAG_CORPSE) &&
         !QUERY_FLAG(ob, FLAG_STARTEQUIP)) ||
        QUERY_FLAG(ob, FLAG_CORPSE_FORCED))
    {
        char buf[MEDIUM_BUF];

        /* Create the corpse object. */
        /* TODO: Change the corpse attribute from boolean to an arch name so
         * that the corpse arch can be variable (ie, different capacities for a
         * gnat and a demon) and not hardwired in to the code.
         * -- Smacky 20090302 */
        corpse = arch_to_object(archetype_global._corpse_default);

        /* Give thie corpse the correct face -- this is the 0th entry of the
         * mob's animation (corpses are not animated). */
        /* TODO: The check for dummy.111 is temporary -- it's to suppress
         * ingame errors (showing the dummy.111 face) while the arches are
         * being changed. */
        strcpy(buf, new_faces[animations[ob->animation_id].faces[0]].name);

        if (strcmp(buf, "dummy.111"))
        {
            corpse->face = &new_faces[animations[ob->animation_id].faces[0]];
        }

        /* The corpse will go on the same square as (m)ob->head. */
        corpse->map = ob->map;
        corpse->x = ob->x;
        corpse->y = ob->y;
        /* ALL corpses are half the weight of the living mob! */
        corpse->weight = (sint32)(ob->weight * 0.50);
    }

    for (tmp_op = ob->inv; tmp_op; tmp_op = tmp)
    {
        tmp = tmp_op->below;
        remove_ob(tmp_op); /* Inv-no check off / This will be destroyed in next loop of object_gc() */

        /* if we recall spawn mobs, we don't want drop their items as free.
         * So, marking the mob itself with "FLAG_STARTEQUIP" will kill
         * all inventory and not dropping it on the map.
         * This also happens when a player slays a to low mob/non exp mob.
         * Don't drop any sys_object in inventory... I can't think about
         * any use... when we do it, a disease needle for example
         * is dropping his disease force and so on. */
        if(tmp_op->type == TYPE_QUEST_TRIGGER)
        {
            /* legal, non freed enemy */
            if (pl)
            {
                if (!(pl->group_status & GROUP_STATUS_GROUP))
                {
                    insert_quest_item(tmp_op, pl->ob); /* single player */
                }
                else
                {
                    for(gtmp = pl->group_leader; gtmp; gtmp = CONTR(gtmp)->group_next)
                    {
                        /* check for out of (kill) range */
                        if(!(CONTR(gtmp)->group_status & GROUP_STATUS_NOQUEST))
                        {
                           insert_quest_item(tmp_op, gtmp); /* give it to member */
                        }
                    }
                }
            }
        }
        else if (!(QUERY_FLAG(ob, FLAG_STARTEQUIP) ||
                   (tmp_op->type != RUNE &&
                    (QUERY_FLAG(tmp_op, FLAG_SYS_OBJECT) ||
                     QUERY_FLAG(tmp_op, FLAG_STARTEQUIP) ||
                     QUERY_FLAG(tmp_op, FLAG_NO_DROP)))))
        {
            tmp_op->x = ob->x;
            tmp_op->y = ob->y;
            CLEAR_FLAG(tmp_op, FLAG_APPLIED);

            /* if we have a corpse put the item in it */
            if (corpse)
            {
                if(!tmp_op->item_level &&
                   !tmp_op->level &&
                   tmp_op->type != RING &&
                   tmp_op->type != AMULET)
                {
                    SET_FLAG(tmp_op, FLAG_IDENTIFIED);

                    if (is_magical(tmp_op))
                    {
                        SET_FLAG(tmp_op, FLAG_KNOWN_MAGICAL);
                    }

                    if (is_cursed_or_damned(tmp_op))
                    {
                        SET_FLAG(tmp_op, FLAG_KNOWN_CURSED);
                    }
                }

                insert_ob_in_ob(tmp_op, corpse);
            }
            else
            {
                /* don't drop traps from a container to the floor.
                 * removing the container where a trap is applied will
                 * neutralize the trap too
                 * Also not drop it in env - be safe here */
                if (tmp_op->type != RUNE)
                {
                    if (!tmp_op->item_level &&
                        !tmp_op->level &&
                        tmp_op->type != RING &&
                        tmp_op->type != AMULET)
                    {
                        SET_FLAG(tmp_op, FLAG_IDENTIFIED);

                        if (is_magical(tmp_op))
                        {
                            SET_FLAG(tmp_op, FLAG_KNOWN_MAGICAL);
                        }

                        if (is_cursed_or_damned(tmp_op))
                        {
                            SET_FLAG(tmp_op, FLAG_KNOWN_CURSED);
                        }
                    }

                    if (ob->env)
                    {
                        insert_ob_in_ob(tmp_op, ob->env);
                    }
                    else
                    {
                        /* Insert in same map as the env*/
                        insert_ob_in_map(tmp_op, ob->map, NULL, 0);
                    }
                }
            }
        }
    }

    if (corpse)
    {
        /* drop the corpse when something is in OR corpse_forced is set */
        /* i changed this to drop corpse always even they have no items
         * inside (player get confused when corpse don't drop. To avoid
         * clear corpses, change below "||corpse " to "|| corpse->inv" */
        if (QUERY_FLAG(ob, FLAG_CORPSE_FORCED) ||
            corpse)
        {
            /* ok... we have a corpse AND we insert something in.
             * now check enemy and/or attacker to find a player.
             * if there is one - personlize this corpse container.
             * this gives the player the chance to grap this stuff first
             * - and looter will be stopped. */
            if (pl)
            {
                FREE_AND_ADD_REF_HASH(corpse->slaying, pl->ob->name);
            }
            else if (QUERY_FLAG(ob, FLAG_CORPSE_FORCED)) /* && no player */
            {
                /* normallly only player drop corpse. But in some cases
                 * npc can do it too. Then its smart to remove that corpse fast.
                 * It will not harm anything because we never deal for NPC with
                 * bounty. */
                corpse->stats.food = 6;
            }

            if (corpse->slaying) /* change sub_type to mark this corpse */
            {
                if(pl->group_status & GROUP_STATUS_GROUP)
                {
                    corpse->stats.maxhp = CONTR(pl->group_leader)->group_id;
                    corpse->sub_type1 = ST1_CONTAINER_CORPSE_group;
                }
                else
                {
                    corpse->sub_type1 = ST1_CONTAINER_CORPSE_player;
                }
            }

            if (ob->env)
            {
                insert_ob_in_ob(corpse, ob->env);
            }
            else
            {
                insert_ob_in_map(corpse, ob->map, NULL, 0);
            }
        }
        else /* disabled */
        {
            /* if we are here, our corpse mob had something in inv but its nothing to drop */
            if (!QUERY_FLAG(corpse, FLAG_REMOVED))
            {
                remove_ob(corpse); /* no check off - not put in the map here */
            }
        }
    }
}

/** Frees all data belonging to an object, but doesn't
 * care about the object itself. This can be used for
 * non-GC objects like archetype clone objects */
void free_object_data(object *ob, int free_static_data)
{
    /* This should be very rare... */
    if (QUERY_FLAG(ob, FLAG_IS_LINKED))
        remove_button_link(ob);

    activelist_remove_inline(ob);

    if (ob->type == CONTAINER && ob->attacked_by)
        container_unlink(NULL, ob);

    /* unlink old treasurelist if needed */
    if (ob->randomitems)
        unlink_treasurelists(ob->randomitems, free_static_data ? OBJLNK_FLAG_STATIC : 0);
    ob->randomitems = NULL;

    /* Remove object from the active list */
//    ob->speed = 0;
//    update_ob_speed(ob);
    /*LOG(llevDebug,"FO: a:%s %x >%s< (#%d)\n", ob->arch?(ob->arch->name?ob->arch->name:""):"", ob->name, ob->name?ob->name:"",ob->name?query_refcount(ob->name):0);*/

    /* Free attached attrsets */
    if (ob->custom_attrset)
    {
        /*      LOG(llevDebug,"destroy_object() custom attrset found in object %s (type %d)\n",
                      STRING_OBJ_NAME(ob), ob->type);*/

#ifdef DAI_DEVELOPMENT_CODE /* Avoid this check on performance-critical servers */
        if(ob->head)
        {
            LOG(llevDebug, "DEBUG:: %s/free_object_data(): Custom attrset found in object %s[%d] subpart (type %d)\n",
                __FILE__, STRING_OBJ_NAME(ob), TAG(ob), ob->type);
        }
#endif

        switch (ob->type)
        {
            case PLAYER:
            case DEAD_OBJECT:
              /* Players are changed into DEAD_OBJECTs when they logout */
              return_poolchunk(ob->custom_attrset, pool_player);
              break;

            case MONSTER:
              return_poolchunk(ob->custom_attrset, pool_mob_data);
              break;

            case TYPE_BEACON:
              {
                  object *registered = hashtable_find(beacon_table, ob->custom_attrset);

#ifdef DEBUG_BEACONS
                  /* the original object name is stored in custom_attrset */
                  LOG(llevInfo, "Removing beacon (%s): ", (char *)ob->custom_attrset);

                  if (registered != ob)
                  {
                      LOG(llevInfo, "another beacon has replaced it. Not deregistering!\n");
                  }
                  else
                  {
                      LOG(llevInfo, "deregistering!\n");
                      hashtable_erase(beacon_table, ob->custom_attrset);
                  }
#else
                  if (registered == ob)
                  {
                      hashtable_erase(beacon_table, ob->custom_attrset);
                  }
#endif

                  FREE_ONLY_HASH(ob->custom_attrset);
              }
              break;

            case TYPE_QUEST_UPDATE: // since r7336 a string, so avoid default
            break;

            default:
              LOG(llevBug, "BUG: destroy_object() custom attrset found in unsupported object %s (type %d)\n",
                  STRING_OBJ_NAME(ob), ob->type);
        }
        ob->custom_attrset = NULL;
    }

    FREE_AND_CLEAR_HASH2(ob->name);
    FREE_AND_CLEAR_HASH2(ob->title);
    FREE_AND_CLEAR_HASH2(ob->race);
    FREE_AND_CLEAR_HASH2(ob->slaying);
    FREE_AND_CLEAR_HASH2(ob->msg);
}

/* destroy and delete recursive the inventory of an destroyed object. */
static void destroy_ob_inv(object *op)
{
    object *tmp, *tmp2;

#if defined DEBUG_GC
    if(op->inv)
        LOG(llevDebug, "  destroy_ob_inv(%s (%d))\n", STRING_OBJ_NAME(op), op->count);
#endif

    for (tmp = op->inv; tmp; tmp = tmp2)
    {
        tmp2 = tmp->below;

        /* For some reason sometimes a dmg info in an aggro history will be
         * freed with return_poolchunk() in object_gc() above but not entirely
         * removed by the time we get here (ie, during map swap). We therefore
         * sometimes try to mark_object_removed() a freed object, which is a
         * bug. Although IDK why this happens, so it needs investigating, this
         * *only* happens in this one case so I assume it is a timing glitch.
         * In any case, it seems pointless to let it happen so here we skip
         * such freed objects and log a DEBUG.
         * -- Smacky 20101115 */
        if (OBJECT_FREE(tmp))
        {
#if defined DEBUG_GC
            LOG(llevDebug, "DEBUG:: %s/destroy_ob_inv(): Skipping freed object found in inv of %s[%d] during gc!\n",
                __FILE__, STRING_OBJ_NAME(op), TAG(op));
#endif

            continue;
        }

#if defined DEBUG_GC
        LOG(llevDebug, "    removing %s (%d)\n", STRING_OBJ_NAME(tmp), tmp->count);
#endif

        if (tmp->inv)
            destroy_ob_inv(tmp);

        mark_object_removed(tmp); /* Enqueue for gc */
    }
}

/** frees everything allocated by an object, removes
 * it from the list of used objects, and puts it on the list of
 * free objects.
 *
 * This function is called automatically to free unused objects
 * (it is called from return_poolchunk() during garbage collection in object_gc() ).
 * The object must have been removed by remove_ob() first for this function to succeed.
 * @note Due to the tricky free/active/remove-list handling of objects, don't ever call this manually.
 */
void destroy_object(object *ob)
{
    if (OBJECT_FREE(ob))
    {
        dump_object(ob);
        LOG(llevBug, "BUG: Trying to destroy freed object.\n%s\n", errmsg);
        return;
    }

    if (!QUERY_FLAG(ob, FLAG_REMOVED))
    {
        dump_object(ob);
        LOG(llevBug, "BUG: Destroy object called with non removed object\n:%s\n", errmsg);
    }

#if defined DEBUG_GC
    LOG(llevDebug, "  destroy_object(%s)\n", STRING_OBJ_NAME(ob));
#endif

    /* Make sure to get rid of the inventory, too. */
    destroy_ob_inv(ob);

    free_object_data(ob, 0);

    ob->map = NULL;

    ob->count = 0; /* mark object as "do not use" and invalidate all references to it */
}

#if 0
/*
 * count_free() returns the number of objects on the list of free objects.
 */

int count_free() {
  int i=0;
  object *tmp=free_objects;
  while(tmp!=NULL)
    tmp=tmp->next, i++;
  return i;
}

/*
 * count_used() returns the number of objects on the list of used objects.
 */

int count_used() {
  int i=0;
  object *tmp=objects;
  while(tmp!=NULL)
    tmp=tmp->next, i++;
  return i;
}
#endif

/* Physically kill/destroy an object, creating corpse and/or
 * dropping any inventory on the floor */
void destruct_ob(object *op)
{
    drop_ob_inv(op);
    remove_ob(op);
    check_walk_off(op, NULL, MOVE_APPLY_DEFAULT);

    /* Notify player that a pet has died */
    /* TODO: maybe this should be in kill_object() */
    if(op->type == MONSTER && OBJECT_VALID(op->owner, op->owner_count) && op->owner->type == PLAYER)
        new_draw_info(NDI_UNIQUE, 0, op->owner, "Your %s was killed", query_name(op));
}

/* remove_ob(op):
 *   This function removes the object op from the linked list of objects
 *   which it is currently tied to.  When this function is done, the
 *   object will have no environment.  If the object previously had an
 *   environment, the x and y coordinates will be updated to
 *   the previous environment.
 *   if we want remove alot of players inventory items, set
 *   FLAG_NO_FIX_PLAYER to the player first and call fix_player()
 *   explicit then.
 */
void remove_ob(object *op)
{
    if (QUERY_FLAG(op, FLAG_REMOVED))
    {
        /*dump_object(op)*/;
        LOG(llevBug, "BUG:: %s:remove_ob(): Trying to remove removed object %s[%d] at %s (%d,%d)!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op), STRING_OBJ_MAP_PATH(op),
            op->x, op->y);

        return;
    }

    if (op->more)
    {
        remove_ob(op->more); // check off is handled outside here
    }

    mark_object_removed(op);
    SET_FLAG(op, FLAG_OBJECT_WAS_MOVED);

    if (op->env)
    {
        RemoveFromEnv(op);
    }
    else if (op->map)
    {
        RemoveFromMap(op);
    }
    else
    {
        LOG(llevBug, "BUG:: %s:remove_ob(): object %s[%d] has neither map nor env!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op));
    }
}

static void RemoveFromEnv(object *op)
{
    object *env = op->env;

    /* When the object being removed is an open container, close it. */
    if (op->type == CONTAINER &&
        op->attacked_by)
    {
        container_unlink(NULL, op);
    }

    /* Notify clients. */
    if (!QUERY_FLAG(op, FLAG_NO_SEND))
    {
        esrv_del_item(op);
    }

    /* Recalc the chain of weights. Remember that the above is client info
     * only -- the object has not actually gone yet. */
    if (!QUERY_FLAG(op, FLAG_SYS_OBJECT))
    {
        RegrowBurdenTree(op, op->nrof, -1);
    }

    /* Sort out the revised object chain. */
    if (op->above)
    {
        op->above->below = op->below;
    }
    else
    {
        op->env->inv = op->below;
    }

    if (op->below)
    {
        op->below->above = op->above;
    }

#ifdef POSITION_DEBUG
    op->ox = op->x;
    op->oy = op->y;
#endif

    op->above = op->below = op->env = NULL;
    op->map = NULL;

    if (env->type == PLAYER &&
        !QUERY_FLAG(env, FLAG_NO_FIX_PLAYER))
    {
        FIX_PLAYER(env, "remove_ob()");
    }
}

static void RemoveFromMap(object *op)
{
    MapSpace *msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y);

    /* Sort out map space slayer. */
    if (op->layer)
    {
        map_set_slayers(msp, op, 0);
    }

    /* Sort out object chain. Don't NULL op->map (still needed). */
    if (op->above)
    {
        op->above->below = op->below;
    }
    else
    {
        SET_MAP_SPACE_LAST(msp, op->below); // assign below as last one
    }

    if (op->below)
    {
        op->below->above = op->above;
    }
    else
    {
        SET_MAP_SPACE_FIRST(msp, op->above);  // first object goes on above it
    }

    op->above = NULL;
    op->below = NULL;

    /* When a map is swapped out and the objects on it get removed too. */
    if (op->map->in_memory == MAP_SAVING)
    {
        return;
    }

    if (op->type == PLAYER)
    {
        player *pl = CONTR(op);

        /* now we remove us from the local map player chain */
        if (pl->map_below)
        {
            CONTR(pl->map_below)->map_above = pl->map_above;
        }
        else
        {
            op->map->player_first = pl->map_above;
        }

        if (pl->map_above)
        {
            CONTR(pl->map_above)->map_below = pl->map_below;
        }

        pl->map_below = pl->map_above = NULL;
        pl->update_los = 1;

        /* a open container NOT in our player inventory = unlink (close) when we move */
        if (pl->container &&
            pl->container->env != op)
        {
            container_unlink(pl, NULL);
        }
    }

    update_object(op, UP_OBJ_REMOVE);
    op->env = NULL;
}

/* Recursively remove the inventory of op. */
void remove_ob_inv(object *op)
{
    object *this,
           *next;

    for (this = op->inv; this; this = next)
    {
        next = this->below;

        if (this->inv)
        {
            remove_ob_inv(this);
        }

        remove_ob(this);
    }
}

/*
 * insert_ob_in_map (op, map, originator, flag):
 * This function inserts the object in the two-way linked list
 * which represents what is on a map.
 * The second argument specifies the map, and the x and y variables
 * in the object about to be inserted specifies the position.
 *
 * originator: Player, monster or other object that caused 'op' to be inserted
 * into 'map'.  May be NULL.
 *
 * flag is a bitmask about special things to do (or not do) when this
 * function is called.  see the object.h file for the INS_ values.
 * Passing 0 for flag gives proper default values, so flag really only needs
 * to be set if special handling is needed.
 *
 * Return value:
 *   NULL if 'op' was destroyed
 *   just 'op' otherwise
 *   When a trap (like a trapdoor) has moved us here, op will returned true.
 *   The caller function must handle it and controlling ->map, ->x and ->y of op
 *
 * I reworked the FLY/MOVE_ON system - it should now very solid and faster. MT-2004.
 * Notice that the FLY/WALK_OFF stuff is removed from remove_ob() and must be called
 * explicit when we want make a "move/step" for a object which can trigger it.
 */

object *insert_ob_in_map(object *const op, mapstruct *m, object *const originator, const int flag)
{
    object*tmp =    NULL, *top;
    MapSpace       *msp;
    int             x, y, lt, layer, layer_inv;
    mapstruct      *old_map = op->map;

    /* some tests to check all is ok... some cpu ticks
     * which tracks we have problems or not
     */
    if (OBJECT_FREE(op))
    {
        dump_object(op);
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert freed object %s in map %s!\n:%s\n", query_name(op),
            m->name, errmsg);
        return NULL;
    }
    if (m == NULL)
    {
        dump_object(op);
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert object %s in null-map!\n%s\n", query_name(op), errmsg);
        return NULL;
    }

    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        dump_object(op);
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert non removed object %s in map %s.\n%s\n", query_name(op),
            m->name, errmsg);
        return NULL;
    }

    /* tail, but no INS_TAIL_MARKER: we had messed something outside! */
    if (op->head && !(flag & INS_TAIL_MARKER))
    {
        LOG(llevBug,
            "BUG: insert_ob_in_map(): inserting op->more WITHOUT INS_TAIL_MARKER! OB:%s (ARCH: %s) (MAP: %s (%d,%d))\n",
            query_name(op), op->arch->name, m->path, op->x, op->y);
        return NULL;
    }

    if (op->more)
    {
        if (insert_ob_in_map(op->more, op->more->map, originator, flag | INS_TAIL_MARKER) == NULL)
        {
            if (!op->head)
                LOG(llevBug, "BUG: insert_ob_in_map(): inserting op->more killed op %s in map %s\n", query_name(op),
                    m->name);
            return NULL;
        }
    }

    /* A stackable object with nrof 0 is junk.
     * We do this because otherwise objects marked for removal by merge_ob()
     * can be un-removed here. */
    if (QUERY_FLAG(op, FLAG_CAN_STACK) && !op->nrof)
        return NULL;

#ifdef POSITION_DEBUG
    op->ox = op->x;
    op->oy = op->y;
#endif

    /* this is now a key part of this function, because
     * we adjust multi arches here when they cross map boarders!
     */
    x = op->x;
    y = op->y;
    op->map = m;

    if (!(m = out_of_map(m, &x, &y)))
    {
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert object %s outside the map %s (%d,%d).\n\n",
            query_name(op), op->map->path, op->x, op->y);
        return NULL;
    }

    /* x and y will only change when we change the map too - so check the map */
    if (op->map != m)
    {
        op->map = m;
        op->x = x;
        op->y = y;
    }

    if (!(flag & INS_NO_MERGE))
        (void)merge_ob(op, NULL);

    CLEAR_FLAG(op, FLAG_REMOVED);
    SET_FLAG(op, FLAG_OBJECT_WAS_MOVED); /* we need this for FLY/MOVE_ON/OFF */
    CLEAR_FLAG(op, FLAG_APPLIED);       /* nothing on the floor can be applied */
    CLEAR_FLAG(op, FLAG_INV_LOCKED);    /* or locked */

    /* map layer system */
    /* we don't test for sys object because we ALWAYS set the layer of a sys object
     * to 0 when we load a sys_object (0 is default, but server loader will warn when
     * we set a layer != 0). We will do the check in the arch load and
     * in the map editor, so we don't need to mess with it anywhere at runtime.
     * Note: even the inserting process is more complicate and more code as the crossfire
     * on, we should speed up things alot - with more object more cpu time we will safe.
     * Also, see that we don't need to access in the inserting or sorting the old objects.
     * no FLAG_xx check or something - all can be done by the cpu in cache.
     */
    msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y); /* for fast access - we will not change the node here */
    /* Layer 0 (system) objects always go to the first object on the square --
     * everything else is ->above it -- and are never visible on the client map
     * (so no SET_MAP_SPACE_SLAYER()) -- except to gmaster_matrix players but
     * this is handled specially. */
    if (op->layer == 0)
    {
        if ((top = GET_MAP_SPACE_FIRST(msp)))
        {
            top->below = op;
            op->above = top;
        }
        else
        {
            SET_MAP_SPACE_LAST(msp, op);
        }

        SET_MAP_SPACE_FIRST(msp, op);
    }
    /* Other layers are non-system objects always go to the last object on the
     * square -- everything else is ->below it. */
    else
    {
        if ((top = GET_MAP_SPACE_LAST(msp)))
        {
            top->above = op;
            op->below = top;
        }
        else
        {
            SET_MAP_SPACE_FIRST(msp, op);
        }

        SET_MAP_SPACE_LAST(msp, op);
        map_set_slayers(msp, op, 1);
    }

    /* lets set some specials for our players
     * we adjust the ->player map variable and the local
     * map player chain.
     */
    if (op->type == PLAYER)
    {
        player *pl = CONTR(op);

        /* Bug #000120, Make sure we have a valid player object here */
        if(!pl)
        {
            LOG(llevBug, "BUG:: %s:insert_ob_in_map(): Player object %s[%d] without controller in map %s\n",
                __FILE__, STRING_OBJ_NAME(op), TAG(op),
                STRING_MAP_PATH(op->map));
            CLEAR_FLAG(op, FLAG_IS_PLAYER);
            op->type = MISC_OBJECT;
        }
        else
        {
            if (op->map->player_first)
            {
                CONTR(op->map->player_first)->map_below = op;
                pl->map_above = op->map->player_first;
            }

            op->map->player_first = op;
            pl->update_los = 1;
        }
    }

    if(!(op->map->map_flags & MAP_FLAG_NO_UPDATE))
    {
        update_object(op, UP_OBJ_INSERT);
    }

    /* See if op moved between maps */
    if(op->speed)
    {
        if(op->map != old_map)
        {
            // LOG(llevDebug, "Object moved between maps: %s (%s -> %s)\n", STRING_OBJ_NAME(op), STRING_MAP_PATH(old_map), STRING_MAP_PATH(op->map));
            activelist_remove_inline(op);
        }

        activelist_insert_inline(op);
    }

    /* Help pets to catch up if player entered another mapset or
     * went too far away */
    if(op->type == PLAYER && op->map != old_map && CONTR(op)->pets)
        pets_follow_owner(op);

    /* check walk on/fly on flag if not canceled AND there is some to move on.
     * Note: We are first inserting the WHOLE object/multi arch - then we check all
     * part for traps. This ensures we don't must do nasty hacks with half inserted/removed
     * objects - for example when we hit a teleporter trap.
     * Check only for single tiles || or head but ALWAYS for heads.
     */
    if (!(flag & INS_NO_WALK_ON) && (msp->flags & (P_WALK_ON | P_FLY_ON) || op->more) && !op->head)
    {
        for (tmp = op; tmp; tmp = tmp->more)
        {
            int event;

            msp = GET_MAP_SPACE_PTR(tmp->map, tmp->x, tmp->y);

            /* tmp is flying/levitating but no fly event here */
            if (IS_AIRBORNE(tmp)) /* Old code queried op only, but as
                                   * check_walk_on() may be called on tmp and
                                   * queries it for the flying flags, we must
                                   * do so here too -- Smacky 20080926 */
            {
                if (!(msp->flags & P_FLY_ON))
                    continue;
            }
            /* tmp is walkimg but no walk event here */
            else
            {
                if (!(msp->flags & P_WALK_ON))
                    continue;
            }

            /* there is a move event appropriate to tmp's move type here */
            if ((event = check_walk_on(tmp, originator, MOVE_APPLY_MOVE)))
            {
                if (event == CHECK_WALK_MOVED)
                    return op; /* don't return NULL - we are valid but we was moved */
                else
                    return NULL; /* CHECK_WALK_DESTROYED */
            }
        }
    }
    return op;
}

/* this function inserts an object in the map, but if it
 *  finds an object of its own type, it'll remove that one first.
 *  op is the object to insert it under:  supplies x and the map.
 */
void replace_insert_ob_in_map(char *arch_string, object *op)
{
    object   *this,
             *next;
    MapSpace *msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y);

    /* Search for itself and remove any old instances */
    for (this = GET_MAP_SPACE_FIRST(msp); this; this = next)
    {
        next = this->above;

        if (!strcmp(this->arch->name, arch_string)) /* same archetype */
        {
            remove_ob(this); /* no move off here... should be ok, this is a technical function */
        }
    }

    /* Insert a replacement. */
    this = get_archetype(arch_string);
    this->x = op->x;
    this->y = op->y;
    insert_ob_in_map(this, op->map, op, 0);
}

/*
 * get_split_ob(ob,nr) splits up ob into two parts.  The part which
 * is returned contains nr objects, and the remaining parts contains
 * the rest (or is removed and freed if that number is 0).
 * On failure, NULL is returned.
 */

object * get_split_ob(object *orig_ob, uint32 nr)
{
    object *newob;
    object *tmp, *event;

    if(!orig_ob)
        return NULL;

    if (orig_ob->nrof < nr)
    {
        LOG(llevDebug, "get_split_ob(): There are only %d %ss.", orig_ob->nrof ? orig_ob->nrof : 1, query_name(orig_ob));
        return NULL;
    }

    if (orig_ob->env == NULL && orig_ob->map->in_memory != MAP_ACTIVE)
    {
        LOG(llevDebug, "get_split_ob(): Tried to split object whose map is not in memory.\n");
        return NULL;
    }

    newob = get_object();
    copy_object(orig_ob, newob);

    /* Gecko: copy inventory (event objects)  */
    for (tmp = orig_ob->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == TYPE_EVENT_OBJECT)
        {
            event = get_object();
            copy_object(tmp, event);
            insert_ob_in_ob(event, newob);
        }
    }

    newob->nrof = nr;
    (void)decrease_ob_nr(orig_ob, nr);

    return newob;
}

/*
 * decrease_ob_nr(object, number) decreases a specified number from
 * the amount of an object.  If the amount reaches 0, the object
 * is subsequently removed and freed.
 *
 * Return value: 'op' if something is left, NULL if the amount reached 0
 */

object * decrease_ob_nr(object *op, uint32 i)
{
    if (i == 0) // objects with op->nrof require this check
    {
        return op;
    }
    else if (i > op->nrof) // can't decrease by more than there are
    {
        i = op->nrof;
    }

    /* If object is already removed everything is already handled elsewhere. */
    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        if (op->env)
        {
            /* There are still some left. */
            if (i < op->nrof)
            {
                uint16 flags = UPD_NROF;

                op->nrof -= i;

                if (!QUERY_FLAG(op, FLAG_SYS_OBJECT))
                {
                    RegrowBurdenTree(op, i, -1);
                    fix_player_weight(is_player_inv(op->env));
                    flags |= UPD_WEIGHT;
                }

/* An update would be ideal as it only sends a few bytes of data to each
 * client (the changed nrof and weights) whereas the alternative sends the
 * entire client-side object. However, due to some buggy code in at least
 * 0.10.6 and earlier clients the former causes the item to appear in the
 * below window whereas the latter works.
 * -- Smacky 20140311 */
//                esrv_update_item(flags, op);
                esrv_send_item(op);
            }
            /* We removed all! */
            else
            {
                remove_ob(op);
                op->nrof = 0; // after remove_ob() so RegrowBurdenTree() works
                esrv_del_item(op);
            }
        }
        else // if (op->map)
        {
            op->nrof -= i;
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_VANISHED);

            /* Anything left? Reinsert it at head of inv. */
            if (op->nrof)
            {
                insert_ob_in_map(op, op->map, op, 0);
            }
        }
    }

    if (op->nrof)
    {
        return op;
    }
    else
    {
        return NULL;
    }
}

/* Inserts op in the linked list inside where.
 *
 * The function returns now pointer to inserted item, and return value can
 * be != op, if items are merged. */
object *insert_ob_in_ob(object *op, object *where)
{
    mapstruct *m;
    object    *merged;

    if (!op)
    {
        LOG(llevBug, "BUG:: %s:insert_ob_in_ob(): Attempt to insert nothing!\n",
            __FILE__);

        return NULL;
    }
    else if (!where)
    {
        LOG(llevBug, "BUG:: %s:insert_ob_in_ob(): Attempt to insert %s[%d] in nowhere!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op));

        return op;
    }
    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        LOG(llevBug, "BUG:: %s:insert_ob_in_ob(): Attempt to insert non-removed %s[%d]!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op));

        return op;
    }
    else if (op->more)
    {
        LOG(llevBug, "BUG:: %s:insert_ob_in_ob(): Attempt to insert multipart %s[%d]!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op));

        return op;
    }
    else if (where->head)
    {
        where = where->head;
        LOG(llevBug, "BUG:: %s:insert_ob_in_ob(): Attempt to insert %s[%d] in multipart's body, redirecting to head %s[%d]!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op), STRING_OBJ_NAME(where),
            TAG(where));
    }

    /* A stackable object with nrof 0 is junk.
     * We do this because otherwise objects marked for removal by merge_ob()
     * can be un-removed here. */
    if (QUERY_FLAG(op, FLAG_CAN_STACK) &&
        !op->nrof)
    {
        return NULL;
    }

    /* Sort out the revised object chain. */
    m = op->map;    // if op was removed from a map op->map != NULL
    op->map = NULL; // op->map must be nulled for merge_ob()
    op->env = where;
    op->above = op->below = NULL;
#ifdef POSITION_DEBUG
    op->ox = 0;
    op->oy = 0;
#endif

    /* We only care about merged as an indicator of whether it did or not
     * (whether merged != NULL) so we know whether or not to call
     * RegrowBurdenTree() below. Either way, op is still our real object being
     * inserted. */
    merged = merge_ob(op, NULL);
    CLEAR_FLAG(op, FLAG_REMOVED);
    SET_FLAG(op, FLAG_OBJECT_WAS_MOVED);

    if (!where->inv)
    {
        where->inv = op;
    }
    else
    {
        op->below = where->inv;
        op->below->above = where->inv = op;
    }

    /* See if op moved between maps/containers. */
    if (op->speed &&
        m)
    {
        activelist_remove_inline(op);
    }

    /* Recalc the chain of weights. */
    if (!merged &&
        !QUERY_FLAG(op, FLAG_SYS_OBJECT))
    {
        RegrowBurdenTree(op, op->nrof, 1);
    }

    /* For event objects set the new env's event flags. */
    if (op->type == TYPE_EVENT_OBJECT &&
        op->sub_type1)
    {
        where->event_flags |= (1U << (op->sub_type1 - 1));
    }
    /* For normal or item quest triggers being inserted into a container set
     * the container's event flag. */
    else if (op->type == TYPE_QUEST_TRIGGER &&
             (op->sub_type1 == ST1_QUEST_TRIGGER_NORMAL ||
              op->sub_type1 == ST1_QUEST_TRIGGER_ITEM) &&
             where->type == CONTAINER)
    {
        where->event_flags |= EVENT_FLAG_SPECIAL_QUEST;
    }

    update_ob_speed(op);

    if (!QUERY_FLAG(op, FLAG_NO_SEND))
    {
        esrv_send_item(op);
    }

    if (op->env->type == PLAYER &&
        !QUERY_FLAG(op->env, FLAG_NO_FIX_PLAYER))
    {
        FIX_PLAYER(op->env, "insert_ob_in_ob()");
    }

    return op;
}

/*
 * Checks if any objects which has the WALK_ON() (or FLY_ON() if the
 * object is flying) flag set, will be auto-applied by the insertion
 * of the object into the map (applying is instantly done).
 * Any speed-modification due to SLOW_MOVE() of other present objects
 * will affect the speed_left of the object.
 *
 * originator: Player, monster or other object that caused 'op' to be inserted
 * into 'map'.  May be NULL.
 *
 * Return value: 1 if 'op' was destroyed, 0 otherwise.
 *
 * 4-21-95 added code to check if appropriate skill was readied - this will
 * permit faster movement by the player through this terrain. -b.t.
 *
 * MSW 2001-07-08: Check all objects on space, not just those below
 * object being inserted.  insert_ob_in_map may not put new objects
 * on top.
 */

int check_walk_on(object *const op, object *const originator, int flags)
{
    uint8     fly,
              local_walk_semaphore = 0; // 1= root call for static_walk_semaphore setting
    tag_t     tag;
    MapSpace *msp;
    object   *this;

    if (QUERY_FLAG(op, FLAG_NO_APPLY))
    {
        return 0;
    }

    /* This flag ensures we notice when a moving event has appeared! Because
     * the functions who set/clear the flag can be called recursive from this
     * function and walk_off() we need a static, global semaphor like flag to
     * ensure we don't clear the flag except in the mother call. */
    if (!static_walk_semaphore)
    {
        local_walk_semaphore = 1;
        static_walk_semaphore = 1;
        CLEAR_FLAG(op, FLAG_OBJECT_WAS_MOVED);
    }

    if ((fly = IS_AIRBORNE(op)))
    {
        flags |= MOVE_APPLY_FLY_ON;
    }
    else
    {
        flags |= MOVE_APPLY_WALK_ON;
    }

    tag = op->count;
    msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y);

    for (this = GET_MAP_SPACE_FIRST(msp); this; this = this->above)
    {
        /* Can't apply yourself. */
        if (this == op)
        {
            continue;
        }

        if ((fly)
            ? QUERY_FLAG(this, FLAG_FLY_ON)
            : QUERY_FLAG(this, FLAG_WALK_ON))
        {
            move_apply(this, op, originator, flags); /* apply_func must handle multi arch parts....
                                                     * NOTE: move_apply() can be heavy recursive and recall
                                                     * this function too.*/

            if (was_destroyed(op, tag)) // we got killed, removed or whatever
            {
                if (local_walk_semaphore)
                {
                    static_walk_semaphore = 0;
                }

                return CHECK_WALK_DESTROYED;
            }

            /* and here a remove_ob() or insert_xx() was triggered - we MUST stop now */
            if (QUERY_FLAG(op, FLAG_OBJECT_WAS_MOVED))
            {
                if (local_walk_semaphore)
                {
                    static_walk_semaphore = 0;
                }

                return CHECK_WALK_MOVED;
            }
        }
    }

    if (local_walk_semaphore)
    {
        static_walk_semaphore = 0;
    }

    return CHECK_WALK_OK;
}

/* Different to check_walk_on() this must be called explicit and its
 * handles muti arches at once.
 * There are some flags notifiying move_apply() about the kind of event
 * we have.
 */
int check_walk_off(object *op, object *originator, int flags)
{
    MapSpace   *mc;
    object     *tmp, *part;
    int         local_walk_semaphore    = FALSE; /* when TRUE, this function is root call for static_walk_semaphore setting */
    int         fly;
    tag_t       tag;


    if (!op || !op->map) /* no map, no walk off - item can be in inventory and/or ... */
        return CHECK_WALK_OK; /* means "nothing happens here" */

    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        LOG(llevBug, "BUG: check_walk_off: object %s is not removed when called\n", query_name(op));
        return CHECK_WALK_OK;
    }

    if (QUERY_FLAG(op, FLAG_NO_APPLY))
        return CHECK_WALK_OK;

    tag = op->count;
    if ((fly = IS_AIRBORNE(op)))
        flags |= MOVE_APPLY_FLY_OFF;
    else
        flags |= MOVE_APPLY_WALK_OFF;

    for (part = op; part; part = part->more) /* check single and multi arches */
    {
        mc = GET_MAP_SPACE_PTR(part->map, part->x, part->y);
        if (!(mc->flags & (P_WALK_OFF | P_FLY_OFF))) /* no event on this tile */
            continue;

        /* This flags ensures we notice when a moving event has appeared!
         * Because the functions who set/clear the flag can be called recursive
         * from this function and walk_off() we need a static, global semaphor
         * like flag to ensure we don't clear the flag except in the mother call.
         */
        if (!static_walk_semaphore)
        {
            local_walk_semaphore = TRUE;
            static_walk_semaphore = TRUE;
            CLEAR_FLAG(op, FLAG_OBJECT_WAS_MOVED);
        }

        for (tmp = mc->first; tmp != NULL; tmp = tmp->above) /* ok, check objects here... */
        {
            if (tmp == part) /* its the ob part in this space... better not >1 part in same space of same arch */
                continue;

            if (fly ? QUERY_FLAG(tmp, FLAG_FLY_OFF) : QUERY_FLAG(tmp, FLAG_WALK_OFF)) /* event */
            {
                move_apply(tmp, part, originator, flags);

                if (OBJECT_FREE(part) || tag != op->count)
                {
                    if (local_walk_semaphore)
                        static_walk_semaphore = FALSE;
                    return CHECK_WALK_DESTROYED;
                }

                /* and here a insert_xx() was triggered - we MUST stop now */
                if (!QUERY_FLAG(part, FLAG_REMOVED) || QUERY_FLAG(part, FLAG_OBJECT_WAS_MOVED))
                {
                    if (local_walk_semaphore)
                        static_walk_semaphore = FALSE;
                    return CHECK_WALK_MOVED;
                }
            }
        }
        if (local_walk_semaphore)
        {
            local_walk_semaphore = FALSE;
            static_walk_semaphore = FALSE;
        }
    }

    if (local_walk_semaphore)
        static_walk_semaphore = FALSE;
    return CHECK_WALK_OK;
}


/*
 * present_arch(arch, map, x, y) searches for any objects with
 * a matching archetype at the given map and coordinates.
 * The first matching object is returned, or NULL if none.
 */

object * present_arch(archetype *at, mapstruct *m, int x, int y)
{
    object *tmp;
    if (!(m = out_of_map(m, &x, &y)))
    {
        LOG(llevError, "ERROR: Present_arch called outside map.\n");
        return NULL;
    }
    for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
        if (tmp->arch == at)
            return tmp;
    return NULL;
}

/*
 * present(type, map, x, y) searches for any objects with
 * a matching type variable at the given map and coordinates.
 * The first matching object is returned, or NULL if none.
 */

object * present(unsigned char type, mapstruct *m, int x, int y)
{
    object *tmp;
    if (!(m = out_of_map(m, &x, &y)))
    {
        LOG(llevError, "ERROR: Present called outside map.\n");
        return NULL;
    }
    for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
        if (tmp->type == type)
            return tmp;
    return NULL;
}

/*
 * present_in_ob(type, object) searches for any objects with
 * a matching type variable in the inventory of the given object.
 * The first matching object is returned, or NULL if none.
 */

object * present_in_ob(unsigned char type, object *op)
{
    object *tmp;
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        if (tmp->type == type)
            return tmp;
    return NULL;
}

/*
 * present_arch_in_ob(archetype, object) searches for any objects with
 * a matching archetype in the inventory of the given object.
 * The first matching object is returned, or NULL if none.
 */

object *present_arch_in_ob(archetype *at, object *op)
{
    object *tmp;
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        if (tmp->arch == at)
            return tmp;
    return NULL;
}

/*
* present_arch_in_ob_temp(archetype, object) searches for any objects with
* a matching archetype in the inventory of the given object which
* has the flag FLAG_IS_USED_UP set.
* The first matching object is returned, or NULL if none.
*/
object *present_arch_in_ob_temp(archetype *at, object *op)
{
    object *tmp;
    for (tmp = op->inv; tmp != NULL; tmp = tmp->below)
        if (tmp->arch == at && QUERY_FLAG(tmp,FLAG_IS_USED_UP))
            return tmp;
    return NULL;
}

/*
 * can_pick(picker, item): finds out if an object is possible to be
 * picked up by the picker.  Returnes 1 if it can be
 * picked up, otherwise 0.
 *
 * Cf 0.91.3 - don't let WIZ's pick up anything - will likely cause
 * core dumps if they do.
 *
 * Add a check so we can't pick up invisible objects (0.93.8)
 */
/* Prevent multiparts being picked up.
 * Make function readable.
 * -- Smacky 20090826 */

int can_pick(object *who, object *item)
{
    /* Multiparts can never be picked up. */
    if (item->more || item->head)
        return 0;

    /* Weightless objects currently can't be picked up. */
    /* I am not sure about that weight >0... */
    if (item->weight <= 0)
        return 0;

    /* Layer 0/sys objects can't be picked up (really the one should imply the
     * other but JIC). TODO: Perhaps be strict about all layers? Really only
     * layers 3 and 4 should be pickable. */
    if (!item->layer ||
        QUERY_FLAG(item, FLAG_SYS_OBJECT))
    {
        return 0;
    }

    /* If you can't see it, you can't pick it up. */
    /* Is this sensible?
     * -- Smacky 20090826 */
    if (IS_NORMAL_INVIS_TO(item, who))
    {
        return 0;
    }

    /* Non-players can't pick up objects which weigh more than 1/3 of their
     * body weight. */
    /* Nice idea, but shouldn't Str be involved?
     * -- Smacky 20090826 */
    /* I misinterpreted this. I had it only applying to players. The original
     * code only applies it to non-players, so I have changed it back. But this
     * seems illogical and I see no gameplay benefit either -- a hill giant
     * can't pick up heavy stuff but a player can?
     * -- Smacky 20090829 */
    if (who->type != PLAYER &&
        item->weight >= who->weight / 3)
        return 0;

    /* Normally no_picks can't be picked up, but unpaid no_picks can. */
    /* This seems dodgy to me.
     * -- Smacky 20090826 */
    if (QUERY_FLAG(item, FLAG_NO_PICK) &&
        !(who->type == PLAYER &&
          QUERY_FLAG(item, FLAG_UNPAID)))
        return 0;

    return 1;
}


/*
 * create clone from object to another
 */
object * ObjectCreateClone(object *asrc)
{
    object*dst =    NULL, *tmp, *src, *part, *prev, *item;

    if (!asrc)
        return NULL;
    src = asrc;
    if (src->head)
        src = src->head;

    prev = NULL;
    for (part = src; part; part = part->more)
    {
        tmp = get_object();
        copy_object(part, tmp);
        tmp->x -= src->x;
        tmp->y -= src->y;
        if (!part->head)
        {
            dst = tmp;
            tmp->head = NULL;
        }
        else
        {
            tmp->head = dst;
        }
        tmp->more = NULL;
        if (prev)
            prev->more = tmp;
        prev = tmp;
    }
    /*** copy inventory ***/
    for (item = src->inv; item; item = item->below)
    {
        (void) insert_ob_in_ob(ObjectCreateClone(item), dst);
    }

    return dst;
}

int was_destroyed(const object *const op, const tag_t old_tag)
{
    /* checking for OBJECT_FREE isn't necessary, but makes this function more
     * robust */
    /* Gecko: redefined "destroyed" a little broader: included removed objects.
     * -> need to make sure this is never a problem with temporarily removed objects */
    return (QUERY_FLAG(op, FLAG_REMOVED) || (op->count != old_tag) || OBJECT_FREE(op));
}

/* GROS - Creates an object using a string representing its content.         */
/* Basically, we save the content of the string to a temp file, then call    */
/* load_object on it. I admit it is a highly inefficient way to make things, */
/* but it was simple to make and allows reusing the load_object function.    */
/* Remember not to use load_object_str in a time-critical situation.         */
/* Also remember that multiparts objects are not supported for now.          */
/* Rewritten to read the object deefinition from a string as the name suggests
 * rather than a file (uses LO_MEMORYMODE), which should avoid the above speed
 * issues and potential problems with the file being overwritten by a
 * concurrent call to load_object_str() (ie, via scripts using
 * game:LoadObject()). Also, multipart object loading is supported in Dai.
 * -- Smacky 20090314 */
object * load_object_str(char *obstr)
{
    object *ob;

    /* Basic checks that obstr is reasonable. */
    if (strncmp(obstr, "arch ", 5) ||
        strcmp(obstr + strlen(obstr) - 4, "\nend"))
    {
        LOG(llevBug, "BUG:: %s/load_object_str(): invalid form of object definition string: >%s<\n",
            __FILE__, obstr);

        return NULL;
    }

    ob = get_object();

    if (!load_object(obstr, ob, NULL, LO_MEMORYMODE, 0))
    {
        LOG(llevBug, "BUG:: %s/load_object_str(): load_object() failed!",
            __FILE__);
        remove_ob(ob);

        return NULL;
    }

    return ob;
}


int auto_apply(object *op)
{
    object*tmp =    NULL, *tmp2;
    int             i, level;

    /* because auto_apply will be done only *one* time
     * when a new, base map is loaded, we always clear
     * the flag now.
     */
    CLEAR_FLAG(op, FLAG_AUTO_APPLY);
    switch (op->type)
    {
        case SHOP_FLOOR:
          if (op->randomitems == NULL)
              return 0;
          do
          {
              i = 10; /* let's give it 10 tries */
              level = get_enviroment_level(op);
              while ((tmp = generate_treasure(op->randomitems, level)) == NULL && --i);
              if (tmp == NULL)
                  return 0;

              if (is_cursed_or_damned(tmp))
              {
                  tmp = NULL;
              }
          }
          while (!tmp);

          tmp->x = op->x,tmp->y = op->y;
          SET_FLAG(tmp, FLAG_UNPAID);
          SET_FLAG(tmp, FLAG_NO_PICK);  /* our new shop code: applying a nopick/unpaid object generate a new object of this in the players inventory */
          insert_ob_in_map(tmp, op->map, NULL, INS_NO_MERGE | INS_NO_WALK_ON);
          identify(tmp);
          break;

        case TREASURE:
          level = get_enviroment_level(op);
          create_treasure_list(op->randomitems, op, op->map ? GT_ENVIRONMENT : 0, level, ART_CHANCE_UNSET, 0);

          /* If we generated on object and put it in this object inventory,
           * move it to the parent object as the current object is about
           * to disappear.  An example of this item is the random_* stuff
           * that is put inside other objects.
          * i fixed this - old part only copied one object instead all.
           */
          for (tmp = op->inv; tmp; tmp = tmp2)
          {
              tmp2 = tmp->below;
              remove_ob(tmp);
              if (op->env)
                  insert_ob_in_ob(tmp, op->env);
          }
          remove_ob(op); /* no move off needed */
          break;
    }

    return tmp ? 1 : 0;
}

/** Tries to locate a beacon.
 * @return the named beacon object if it was in memory,
 *         or NULL otherwise */
object *locate_beacon(shstr *id)
{
    if(id == NULL)
    {
        LOG(llevBug, "locate_beacon(NULL)\n");
        return NULL;
    }
    return (object *)hashtable_find(beacon_table, id);
}

/** Intializer function for TYPE_BEACON objects.
 * Ensures the beacon is added to the beacon hashtable.
 */
static void beacon_initializer(object *op)
{
    object *parent;

    /* Beacons must be unique so do not register beacons in SPAWN_POINT_MOBs */
    for (parent = op; parent->env; parent = parent->env)
    {
        if (parent->type == SPAWN_POINT_MOB)
            return;
    }

    /* No env and no map means the beacon is in a mob's loot container
     * (during the spawn process) so ignore it). */
    if (!parent->map)
        return;

    /* At this point, parent must be on a map so check its type. Beacons do not
     * get registered -- in fact, get removed -- on instance and unique maps
     * (which by their nature can have multiple examples loaded at the same
     * time for different players. */
    /* TODO Perhaps a better way would be to add
     * the instance path to the beacon name here, so beacons can be used on
     * instances and are still unique. However, an allowance for this also
     * needs to be built into locate_beacon() and I am too lazy ATM.
     * -- Smacky 20081211 */
    /*  The above would not work on uniques.
     *  -- Smacky 20090414 */
    /* It seems we are trying to remove the beacon before it has been inserted,
     * so lets not actually remove it. This means the object is still there on
     * the map, which IIMO is not great, but it is not registered as a beacon.
     * -- Smacky 20091024 */
    if (!MAP_MULTI(parent->map))
    {
        LOG(llevMapbug, "MAPBUG:: Ignoring beacon on instance (%s[%s %d %d])!\n",
            STRING_OBJ_NAME(op), STRING_MAP_PATH(parent->map), parent->x,
            parent->y);
        //remove_ob(op);
        return;
    }

#ifdef DEBUG_BEACONS
    LOG(llevDebug, "DEBUG:: %s/beacon_initializer(): Initializing beacon (%s[%d])!\n",
        __FILE__, STRING_OBJ_NAME(op), TAG(op));
#endif

    if (op->custom_attrset)
    {
        LOG(llevMapbug, "MAPBUG:: Ignoring second initialization of single beacon (%s[%s %d %d])!\n",
            STRING_OBJ_NAME(op), STRING_MAP_PATH(parent->map), parent->x,
            parent->y);
        return;
    }
    else if (!op->name ||
             op->name == shstr_cons.beacon_default)
    {
        LOG(llevMapbug, "MAPBUG:: Ignoring initialization of beacon with NULL/default name (%s[%s %d %d])!\n",
            STRING_OBJ_NAME(op), STRING_MAP_PATH(parent->map), parent->x,
            parent->y);
        return;
    }

    /* Store original name in the attrset, so that a name change doesn't mess
     * things up */
    op->custom_attrset = (void *)op->name;
    add_refcount(op->name);

    if (!hashtable_insert(beacon_table, op->custom_attrset, op))
    {
        /* Replace existing entry TODO: speed up with hashtable_replace() or
         * something similar */
#ifdef DEBUG_BEACONS
        LOG(llevDebug, "DEBUG:: %s/beacon_initializer(): Replacing already registered beacon (%s[%d])!\n",
            __FILE__, STRING_OBJ_NAME(op), TAG(op));
#endif
        hashtable_erase(beacon_table, op->custom_attrset);
        hashtable_insert(beacon_table, op->custom_attrset, op);
    }
}

static void monster_initializer(object *op)
{
    /* We only want to initialize monsters inside objects here. Monsters on maps
     * are handled by map.c:map.c/LoadObjects() after they have been inserted onto the map */
    if(op->env)
        fix_monster(op);
}

/** Initialize the table of object initializers. */
void init_object_initializers()
{
    object_initializers[TYPE_BEACON] = beacon_initializer;
    object_initializers[MONSTER] = monster_initializer;
}

/* Returns the next object in the local object tree (according to mode and
 * root) of op which matches.
 * An object matches when it has the specified type.
 * A local object tree is defined as *up to* all the ancestors, descendants,
 * and siblings of op. This scope is modified according to mode and root.
 * If mode == FNO_MODE_INV_ONLY, only the immediate inventory of op is
 * included.
 * If mode == FNO_MODE_CONTAINERS, the inventories of all CONTAINER type
 * descendants is included as well.
 * If mode == FNO_MODE_ALL, the inventories of all descendants, regardless of
 * type, is included.
 * If root == NULL, all ancestors below the 'natural' root are included.
 * If root != NULL, only ancestors below the specified root are included.
 * The natural root is the ancestor which is on a map (the assumption is that
 * players and monsters must always be directly on a map, not in an
 * environment). */
object *find_next_object(object *op, uint8 type, uint8 mode, object *root)
{
    object *next;

#ifdef DEBUG_FNO
    LOG(llevDebug, "DEBUG:: %s/find_next_object(%s[%d], %d, %d, %s[%d]):",
        __FILE__,
        STRING_OBJ_NAME(op), TAG(op), type,
        mode, STRING_OBJ_NAME(root), TAG(root));
#endif

    next = op;

    do
    {
        if (next->inv &&
            ((mode == FNO_MODE_CONTAINERS &&
              (next->type == PLAYER ||
               next->type == MONSTER ||
               next->type == CONTAINER ||
               next->type == SPAWN_POINT_MOB)) ||
             mode == FNO_MODE_ALL))
        {
            next = next->inv;
#ifdef DEBUG_FNO
            LOG(llevDebug, "\n v");
#endif
        }
        else if (next->env && !next->below)
        {
            object *tmp = NULL;

            while (!tmp && next->env)
            {
                if (next == root)
                    break;
                next = next->env;
                tmp = (next->env) ? next->below : NULL;
#ifdef DEBUG_FNO
                LOG(llevDebug, "\n ^ %s[%d]",
                    STRING_OBJ_NAME(next), TAG(next));
#endif
            }

            if (next != root)
            {
                next = tmp;

                if (mode != FNO_MODE_ALL)
                    while (next && QUERY_FLAG(next, FLAG_SYS_OBJECT))
                        if (root && (next = next->below) == root)
                            break;
            }
#ifdef DEBUG_FNO
            LOG(llevDebug, "->");
#endif
        }
        else
        {
            if ((next = next->below) != root)
            {
                if (mode != FNO_MODE_ALL)
                    while (next && QUERY_FLAG(next, FLAG_SYS_OBJECT))
                        if ((next = next->below) == root && root)
                            break;
            }
#ifdef DEBUG_FNO
            LOG(llevDebug, " >");
#endif
        }

        if (next)
        {
#ifdef DEBUG_FNO
            LOG(llevDebug, " %s[%d]",
                STRING_OBJ_NAME(next), TAG(next));
#endif

            if (root && (next == root || next == root->env))
            {
#ifdef DEBUG_FNO
                LOG(llevDebug, " ROOT!\n");
#endif
                next = NULL;
            }
            else if (!type || next->type == type)
            {
#ifdef DEBUG_FNO
                LOG(llevDebug, " MATCHES!\n");
#endif
                break;
            }
        }
        else
        {
#ifdef DEBUG_FNO
            LOG(llevDebug, " NULL!\n");
#endif
        }
    }
    while (next);

    return next;
}

/*
 * Changes an item back to its original/unbuffed state so that it can be saved
 * or buff effects can be recalculated.
 */
void revert_buff_stats(object *item)
{
    object *original;

    if (item && item->original)
    {
        original = item->original;

        // The original version will still have the coordinates from when it was copied.
        original->x = item->x;
        original->y = item->y;

        copy_object(item->original, item);
        item->original = original;
    }
}

/* Copy stats from all BUFF_FORCEs in item's inventory to the item. It will also
 * store the original version of that object in item->original so that it can
 * easily be reverted or compared.
 */
void fix_buff_stats(object *item)
{
    int i = 0;
    uint32 n = 0;
    object *inv;
	object *below;
	uint8 buffs = 0; // Only tracks if the object still has *any* buffs

    if (!item)
    {
        LOG(llevBug, "fix_buff_stats() called without item!\n");
        return;
    }

    if (!item->buffed)
    {
        return;
    }

    // Try to create a duplicate of the object to make changing stats relative to the original.
    if (!item->original)
    {
        item->original = get_object();
        copy_object(item, item->original);

        // Even though the object isn't on a map or inside another, it shouldn't be removed on cleanup.
        CLEAR_FLAG(item->original, FLAG_REMOVED);

        if (!item->original)
        {
            LOG(llevDebug, "fix_buff_stats() failed - could not copy original object\n");
            return;
        }
    } else
    {
        // If orig was found, item should be different than orig, so revert item to orig.
        revert_buff_stats(item);
    }

	for (inv = item->inv; inv != NULL; inv = below)
    {
        below = inv->below;
        buffs++;

        if (inv->type != BUFF_FORCE)
        {
            continue;
        }

        // Buff is disabled?
        if (!QUERY_FLAG(inv, FLAG_APPLIED))
        {
            continue;
        }

        n = inv->nrof;

        item->stats.ac += inv->stats.ac * n;
        item->stats.Cha += inv->stats.Cha * n;
        item->stats.Con += inv->stats.Con * n;
        item->stats.dam += inv->stats.dam * n;
        item->stats.Dex += inv->stats.Dex * n;
        item->stats.food += inv->stats.food * n;
        item->stats.grace += inv->stats.grace * n;
        item->stats.hp += inv->stats.hp * n;
        item->stats.Int += inv->stats.Int * n;
        item->stats.maxgrace += inv->stats.maxgrace * n;
        item->stats.maxhp += inv->stats.maxhp * n;
        item->stats.maxsp += inv->stats.maxsp * n;
        item->stats.Pow += inv->stats.Pow * n;
        item->stats.sp += inv->stats.sp * n;
        item->stats.Str += inv->stats.Str * n;
        item->stats.thac0 += inv->stats.thac0 * n;
        item->stats.thacm += inv->stats.thacm * n;
        item->stats.wc += inv->stats.wc * n;
        item->stats.Wis += inv->stats.Wis * n;

        item->item_condition += inv->item_condition * n;

        item->item_level += inv->item_level;
        item->item_skill = inv->item_skill;

        item->level += inv->level;
        item->magic += inv->magic * n;

        item->path_attuned |= inv->path_attuned;
        item->path_denied |= inv->path_denied;
        item->path_repelled |= inv->path_repelled;
        item->terrain_flag |= inv->terrain_flag;

        item->value += inv->value * n;
        item->weapon_speed += inv->weapon_speed * n;
        item->weight += inv->weight * n;
        item->weight_limit += inv->weight_limit * n;

        for (i = 0; i < NROFATTACKS; i++)
        {
            item->attack[i] += inv->attack[i] * n;
            item->resist[i] += inv->resist[i] * n;
        }
    }

    // Make sure the player notices the change if they're wearing the item.
    if (QUERY_FLAG(item, FLAG_APPLIED) && item->env && item->env->type == PLAYER)
    {
        // Only items in the root of players' inventories can be applied.
        FIX_PLAYER(item->env, "buff applied");
    }

    if (buffs == 0)
    {
        item->buffed = FALSE;
    }
}

/*
 * See if the item will exceed its buff limit when we add a buff.
 * This takes buff->nrof and op->max_buffs into account. Returns
 * FALSE when the object cannot accept this buff, TRUE when it can.
 * This does not take into account if the object has already taken
 * this buff.
 */
uint8 check_buff_limit(object *op, int nr)
{
    sint8 real_max = 0; // The max after considering item_condition
    sint8 buffs = 0;
    object *inv;

    // -1 always means this item can never be buffed.
    if (op->max_buffs == -1)
    {
        return FALSE;
    }

	// Max buffs not specified so use qua/con instead.
    if (op->max_buffs == 0)
    {
        // Items with a higher condition are already powerful enough, so
        // they shouldn't be as enhanceable.
        real_max = (88 - op->item_condition) / 2;

        if (real_max <= 0)
        {
            // But give it at least 1 slot.
            real_max = 1;
        }

    } else
    {
        real_max = op->max_buffs;
    }

    if (op->inv)
    {
        for (inv = op->inv; inv; inv = inv->below)
        {
            if (inv->type == BUFF_FORCE)
            {
                buffs += inv->nrof;
            }
        }

    }

    if (buffs + nr > real_max)
    {
        return FALSE;
    }

    return TRUE;

}

object * check_buff_exists(object *item, const char *name)
{
    object *inv;

    if (!item || !name)
    {
        LOG(llevDebug, "check_buff_exists() called without item or name!\n");
        return NULL;
    }

    for (inv = item->inv; inv; inv = inv->below)
    {
        if (inv->type == BUFF_FORCE && strcmp(inv->name, name) == 0)
        {
            return inv;
        }
    }

    return NULL;
}

/*
 * Returns: 0 = success
 *          1 = item or buff == NULL
 *          2 = max_buffs exceeded
 *          3 = max of that particular buff exceeded
 *          4 = for whatever reason we can't put the buff in the item
 *
 * Multiple flags can be applied to better inform the user of why the buff wasn't applied.
 */
int add_item_buff(object *item, object *buff, short just_checking)
{
    object *oldbuff;
    uint8 ret = BUFF_ADD_SUCCESS;

    if (!item || !buff)
    {
		ret = BUFF_ADD_BAD_PARAMS;
        return ret; // No point in going on, only bad things will come.
    }

    // Make sure the item won't exceed its numerical buff limit.
    if (!check_buff_limit(item, buff->last_sp))
    {
        ret |= BUFF_ADD_LIMITED;
        ret &= ~BUFF_ADD_SUCCESS;
    }

    oldbuff = check_buff_exists(item, buff->name);

    if (oldbuff)
    {
        ret |= BUFF_ADD_EXISTS;

        if (oldbuff->last_sp + buff->last_sp > oldbuff->max_buffs)
        {
            ret |= BUFF_ADD_MAX_EXCEEDED;
            ret &= ~BUFF_ADD_SUCCESS;

            return ret;
        }

        // If we are actually adding the buff and it's still successful
        // Just increase the nrof since we already have the buff force.
        if (!just_checking && (ret & BUFF_ADD_SUCCESS))
        {
            oldbuff->last_sp += buff->last_sp;
        }
    } else
    {
        if (!just_checking && (ret & BUFF_ADD_SUCCESS))
        {
            buff = insert_ob_in_ob(buff, item);

            // Something has gone wrong - insert_ob_in_ob() will
            // return null if it shouldn't happen.
            if (!buff)
            {
                ret |= BUFF_ADD_NO_INSERT;
                ret &= ~BUFF_ADD_SUCCESS;
            }
        }
    }

    if ((ret & BUFF_ADD_SUCCESS) && !just_checking)
    {
        SET_FLAG(buff, FLAG_APPLIED);

        /* Copy stat boni from the buff force to the object. This uses buff
         * instead of oldbuff so that we can just merge the changes instead
         * of unmerging all and then remerging all.
         */
        item->buffed = TRUE;
        fix_buff_stats(item);
    }

    return ret;
}

int remove_item_buff(object *item, char *name, uint32 nrof)
{
    object *oldbuff;

    if (!item || !name)
    {
        return BUFF_ADD_BAD_PARAMS;
    }

    oldbuff = check_buff_exists(item, name);

    if (!oldbuff)
    {
        return BUFF_ADD_EXISTS;
    }

    decrease_ob_nr(oldbuff, (nrof > 0) ? nrof : oldbuff->nrof);

    fix_buff_stats(item);

    return BUFF_ADD_SUCCESS;
}
