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

static void RegrowBurdenTree(object_t *op, sint32 nrof, sint8 mode);
static void RemoveFromEnv(object_t *op);
static void RemoveFromMap(object_t *op);
static void Copy(object_t *from, object_t *to);
static void DropLoot(object_t *ob);

static int static_walk_semaphore = FALSE; /* see walk_off/walk_on functions  */

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
static inline void activelist_remove_inline(object_t *op)
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
static inline void activelist_insert_inline(object_t *op)
{
    object_t *tmp;

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

void activelist_insert(object_t *op)
{
    activelist_insert_inline(op);
}

void activelist_remove(object_t *op)
{
    activelist_remove_inline(op);
}

/* Put an object in the list of removal candidates.
 * If the object has still FLAG_REMOVED set at the end of the
 * server timestep it will be freed
 */
void mark_object_removed(object_t *ob)
{
    struct mempool_chunk   *mem = MEM_POOLDATA(ob);

    if (OBJECT_FREE(ob))
        LOG(llevBug, "BUG: mark_object_removed() called for free object\n");

    SET_FLAG(ob, FLAG_REMOVED);

    /* Don't mark objects twice */
    if (mem->next != NULL)
        return;

    /* We abuse the mempool freelist here. Need to zero mem->next out
     * before calling return_poolchunk() on the object_t */
    mem->next = removed_objects;
    removed_objects = mem;
}

/* Go through all objects in the removed list and free the forgotten ones */
void object_gc()
{
    struct mempool_chunk   *current, *next;
    object_t                 *ob;

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

            ob = (object_t *) MEM_USERDATA(current);
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
static inline int can_merge(object_t *ob1, object_t *ob2)
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
        object_t *tmp1 = ob1->inv,
               *next1,
               *tmp2 = ob2->inv,
               *next2;

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

        FOREACH_OBJECT_IN_OBJECT(tmp1, ob1, next1)
        {
            FOREACH_OBJECT_IN_OBJECT(tmp2, ob2, next2)
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
    if ((memcmp(&ob1->stats, &ob2->stats, sizeof(living_t)) != 0)
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
object_t * merge_ob(object_t *op, object_t *tmp)
{
    /* Nothing to merge with or op is non-stackable or has nrof 0, */
    if (!op || !QUERY_FLAG(op, FLAG_CAN_STACK) || !op->nrof)
        return NULL;

    /* Calculate tmp. It is a bug to attempt a merge with an obj in limbo. */
    if (tmp == NULL)
    {
        if (op->map)
        {
            msp_t *msp = MSP_KNOWN(op);

            tmp = msp->last;
        }
        else if (op->env)
        {
            for (tmp = op->env->inv; tmp && tmp->below; tmp = tmp->below)
            {
                ;
            }
        }
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
sint32 sum_weight(object_t *op)
{
    sint32  sum = 0;
    object_t *inv,
           *next;

    FOREACH_OBJECT_IN_OBJECT(inv, op, next)
    {
        if (QUERY_FLAG(inv, FLAG_SYS_OBJECT))
            continue;

        sum += WEIGHT_OVERALL(inv);
    }

    /* we have a magical container? then precalculate the modified weight */
    if(op->type == CONTAINER && op->weapon_speed != 1.0f)
        op->damage_round_tag = (uint32)((float)sum * op->weapon_speed);

    return sum;
}

/* Changes the carrying value of any envs of op by nrof * op->weight. The
 * direction of change is specified by mode (positive or negative). */
static void RegrowBurdenTree(object_t *op, sint32 nrof, sint8 mode)
{
    sint32  tmp_nrof,
            weight;
    object_t *where;

    /* This bit of ugliness is because WEIGHT_OVERALL() assumes op->nrof is the
     * real nrof. */
    tmp_nrof = op->nrof;
    op->nrof = nrof;
    weight = WEIGHT_OVERALL(op);
    op->nrof = tmp_nrof;

    /* If op is not in an env or has no weight, we have nothing to do. */
    if (!(where = op->env) ||
        !(weight))
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
object_t * is_player_inv(object_t *op)
{
    for (; op != NULL && op->type != PLAYER; op = op->env)
        if (op->env == op)
            op->env = NULL;
    return op;
}

/* Used by: Server DM commands: dumpbelow, dump.
 *  Some error messages.
 * The result of the dump is stored in the static global errmsg array. */
void dump_object2(const object_t *op)
{
    char *cp;

    if (op->arch)
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

/*
 * Dumps an object.  Returns output in the static global errmsg array.
 */

void dump_object(const object_t *op)
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
void dump_me(object_t *op, char *outstr, size_t bufsize)
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

object_t * get_owner(object_t *op)
{
    if (!op || op->owner == NULL)
        return NULL;
    if (!OBJECT_FREE(op) && op->owner->count == op->owner_count)
        return op->owner;
    op->owner = NULL,op->owner_count = 0;
    return NULL;
}

void clear_owner(object_t *op)
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

static void set_owner_simple(object_t *op, object_t *owner)
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
void set_owner(object_t * const op, object_t * const owner)
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
void copy_owner(object_t *op, object_t *clone)
{
    object_t *owner   = get_owner(clone);
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
void initialize_object(object_t *op)
{
    memset(op, 0, sizeof(object_t));
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

/* clone_object() clones original (multiparts are correctly handled). If mode
 * is zero it also clones any inventory. The return is the clone. */
object_t *clone_object(object_t *original, uint8 mode)
{
    object_t *clone = NULL,
           *prev = NULL,
           *this,
           *next;

    /* Sanity check: No original = nothing to do. */
    if (!original)
    {
        return NULL;
    }

    /* If original is multipart, point to its head. */
    if (original->head)
    {
        original = original->head;
    }

    /* Clone original to clone, being sure to correctly clone all parts of a
     * multipart. */
    for (this = original; this; this = next)
    {
        object_t *that;

        /* Here we test if the being-cloned object is a multipart or, failing
         * that, if it's archetype is a multipart. In this way we 'clone'
         * multiparts from invs where only the arch head is stored (eg, spawn
         * point mobs in spawn points) as complete objects). */
        if (!(next = this->more) &&
            !(next = this->arch->clone.more))
        {
            next = NULL;
        }

        that = get_object();
        copy_object(this, that);

        if (original->env)
        {
            that->env = original->env;
        }
        else if (original->map)
        {
            map_t  *m = original->map;
            sint16  x = original->x + this->arch->clone.x,
                    y = original->y + this->arch->clone.y;

            /* out_of_map() will check and adjust map,x,y so that spawn points on
             * or near the edge of a map will spawn a mob with the correct values
             * on a tiled map (ie, the point is at map_0000 5 0 and the mob spawns
             * on the northeast square -- this is map_0001 6 23, not map_0000 6 -1).
             * -- Smacky 20131205 */
            that->map = OUT_OF_MAP(m, x, y);
            that->x = x;
            that->y = y;
        }

        if (!this->head)
        {
            clone = that;
            that->head = NULL;
        }
        else
        {
            that->head = clone;
        }

        that->more = NULL;

        if (prev)
        {
            prev->more = that;
        }

        prev = that;
    }

    if (mode == MODE_INVENTORY)
    {
        FOREACH_OBJECT_IN_OBJECT(this, original, next)
        {
            object_t *that = clone_object(this, MODE_INVENTORY);

            (void)insert_ob_in_ob(that, clone);
        }
    }

    /* Cloned players become monsters. */
    if (clone->type == PLAYER ||
        QUERY_FLAG(clone, FLAG_IS_PLAYER))
    {
        clone->type = MONSTER;
        CLEAR_FLAG(clone, FLAG_IS_PLAYER);
    }

    return clone;
}

/* copy_object first frees everything allocated to to and then copies the
 * contents of from into it, allocating what needs to be allocated. */
void copy_object(object_t *from, object_t *to)
{
    /* remove because perhaps speed will change when we copy */
    if (QUERY_FLAG(to, FLAG_IN_ACTIVELIST))
    {
        activelist_remove_inline(to);
    }

    Copy(from, to);

    /* Only alter speed_left when we sure we have not done it before */
    if (to->speed < 0 &&
        to->speed_left == to->arch->clone.speed_left)
    {
        to->speed_left += (RANDOM() % 90) / 100.0;
    }

    CLEAR_FLAG(to, FLAG_IN_ACTIVELIST); // perhaps our source is on active list - ignore!
    update_ob_speed(to);
}

/* Same as above, but not touching the active list. */
void copy_object_data(object_t *from, object_t *to)
{
    Copy(from, to);
}

static void Copy(object_t *from, object_t *to)
{
    int is_removed  = QUERY_FLAG(to, FLAG_REMOVED);

    /* unlink old treasurelist if needed */
    if (to->randomitems)
    {
        unlink_treasurelists(to->randomitems, 0);
    }

    FREE_AND_CLEAR_HASH2(to->name);
    FREE_AND_CLEAR_HASH2(to->title);
    FREE_AND_CLEAR_HASH2(to->race);
    FREE_AND_CLEAR_HASH2(to->slaying);
    FREE_AND_CLEAR_HASH2(to->msg);
    (void)memcpy((void *)((char *)to + offsetof(object_t, name)),
        (void *)((char *)from + offsetof(object_t, name)),
        sizeof(object_t) - offsetof(object_t, name));
    ADD_REF_NOT_NULL_HASH(to->name);
    ADD_REF_NOT_NULL_HASH(to->title);
    ADD_REF_NOT_NULL_HASH(to->race);
    ADD_REF_NOT_NULL_HASH(to->slaying);
    ADD_REF_NOT_NULL_HASH(to->msg);

    if (is_removed)
    {
        SET_FLAG(to, FLAG_REMOVED);
    }

#if 0
/* Buggers up merging. These are 3 distinct flags and should be tested for
 * separately. As such, IDENTIFIED should not set the other two in any case.
 * Especially, in a function whose purpose is to copy an object, we should not
 * then modify the copy so it is no longer identical to the original.
 * -- Smacky 20090312 */
    if (QUERY_FLAG(to, FLAG_IDENTIFIED))
    {
        if (is_magical(to))
            SET_FLAG(to, FLAG_KNOWN_MAGICAL);

        if (is_cursed_or_damned(to))
            SET_FLAG(to, FLAG_KNOWN_CURSED);
    }
#endif

    /* perhaps we have a custom treasurelist. Then we need to
     * increase the refcount here. */
    if (to->randomitems &&
        (to->randomitems->flags & OBJLNK_FLAG_REF))
    {
        to->randomitems->ref_count++;
    }

    /* We set the custom_attrset pointer to NULL to avoid
     * really bad problems. TODO. this needs to be handled better
     * but it works until its only the player struct. */
    to->custom_attrset = NULL;
}

/*
 * get_object() grabs an object from the list of unused objects, makes
 * sure it is initialised, and returns it.
 * If there are no free objects, expand_objects() is called to get more.
 */

object_t * get_object()
{
    object_t *new_obj = (object_t *) get_poolchunk(pool_object);
    mark_object_removed(new_obj);
    return new_obj;
}

/*
 * If an object with the IS_TURNABLE() flag needs to be turned due
 * to the closest player being on the other side, this function can
 * be called to update the face variable, _and_ how it looks on the map.
 */

void update_turn_face(object_t *op)
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

void update_ob_speed(object_t *op)
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
 * an animation), we don't need to call msp_update until that actually
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
void update_object(object_t *op, int action)
{
    map_t *m;
    sint16     x,
               y;
    msp_t  *msp;

    /*LOG(llevNoLog, "update_object: %s (%d,%d) - action %x\n", op->name, op->x, op->y,action);*/
    if (!op)
    {
        /* this should never happen */
        LOG(llevBug, "BUG:: update_object() called for NULL object.\n");
        return;
    }

    if (op->env)
    {
        // Send the player the new inventory information if needed.
        if (op->env->type == PLAYER)
        {
            esrv_update_item(UPD_FACE | UPD_ANIM | UPD_ANIMSPEED, op);
        }
        return;
    }

    if (!op->map ||
        op->map->in_memory == MAP_MEMORY_SAVING)
    {
        return;
    }

    m = op->map;
    x = op->x;
    y = op->y;
    msp = MSP_KNOWN(op);

    switch (action)
    {
        case UP_OBJ_INSERT:
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_INS - %s\n", STRING_OBJ_NAME(op));

#endif
        if (!QUERY_FLAG(op, FLAG_NO_SEND))
        {
            esrv_send_item(op);
        }

        if (op->glow_radius)
        {
            adjust_light_source(msp, op->glow_radius);
        }

        /* this is handled a bit more complex, we must always loop the flags! */
        if (QUERY_FLAG(op, FLAG_NO_PASS) ||
            QUERY_FLAG(op, FLAG_PASS_THRU) ||
            QUERY_FLAG(op, FLAG_PASS_ETHEREAL))
        {
            msp->flags |= (MSP_FLAG_NO_ERROR | MSP_FLAG_UPDATE);
            msp_update(m, NULL, x, y);
        }
        else /* ok, we don't must use flag loop - we can set it by hand! */
        {
            uint32 flags = 0;

            MSP_SET_FLAGS_BY_OBJECT(flags, op);
            msp->flags |= flags;
        }

        break;

        case UP_OBJ_REMOVE:
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_REM - %s\n", STRING_OBJ_NAME(op));

#endif
        if (!QUERY_FLAG(op, FLAG_NO_SEND))
        {
            esrv_del_item(op);
        }

        /* we don't handle floor tile light/darkness setting here -
         * we assume we don't remove a floor tile ever before dropping
         * the map. */
        if (op->glow_radius)
        {
            adjust_light_source(msp, -(op->glow_radius));
        }

        /* we must rebuild the flags when one of this flags is touched from our object_t */
        if (op->type == CHECK_INV ||
            op->type == MAGIC_EAR ||
            op->type == GRAVESTONE ||
            QUERY_FLAG(op, FLAG_ALIVE) ||
            QUERY_FLAG(op, FLAG_IS_PLAYER) ||
            QUERY_FLAG(op, FLAG_BLOCKSVIEW) ||
            QUERY_FLAG(op, FLAG_DOOR_CLOSED) ||
            QUERY_FLAG(op, FLAG_PASS_THRU) ||
            QUERY_FLAG(op, FLAG_PASS_ETHEREAL) ||
            QUERY_FLAG(op, FLAG_NO_PASS) ||
            QUERY_FLAG(op, FLAG_NO_SPELLS) ||
            QUERY_FLAG(op, FLAG_NO_PRAYERS) ||
            QUERY_FLAG(op, FLAG_WALK_ON) ||
            QUERY_FLAG(op, FLAG_FLY_ON) ||
            QUERY_FLAG(op, FLAG_WALK_OFF) ||
            QUERY_FLAG(op, FLAG_FLY_OFF) ||
            QUERY_FLAG(op, FLAG_REFL_CASTABLE) ||
            QUERY_FLAG(op, FLAG_REFL_MISSILE))
        {
            msp->flags |= (MSP_FLAG_NO_ERROR | MSP_FLAG_UPDATE);
            msp_update(m, NULL, x, y);
        }

        break;

        case UP_OBJ_FLAGS:
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_FLAGS - %s\n", STRING_OBJ_NAME(op));
#endif
        msp->flags |= (MSP_FLAG_NO_ERROR | MSP_FLAG_UPDATE);
        msp_update(m, NULL, x, y);
        break;

        case UP_OBJ_FACE: /* no need to change anything except the map update counter */
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_FACE - %s\n", STRING_OBJ_NAME(op));
#endif
        esrv_update_item(UPD_FACE | UPD_ANIM | UPD_ANIMSPEED, op);
        return;

        case UP_OBJ_FLAGFACE:
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_FLAGFACE - %s\n", STRING_OBJ_NAME(op));
#endif
        msp->flags |= (MSP_FLAG_NO_ERROR | MSP_FLAG_UPDATE);
        msp_update(m, NULL, x, y);
        esrv_update_item(UPD_FACE | UPD_ANIM | UPD_ANIMSPEED, op);
        break;

        case UP_OBJ_SLICE:
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_SLICE - %s\n", STRING_OBJ_NAME(op));
#endif
        msp->flags |= (MSP_FLAG_NO_ERROR | MSP_FLAG_UPDATE);
        msp_update(m, NULL, x, y);

        if (!QUERY_FLAG(op, FLAG_NO_SEND))
        {
            esrv_send_or_del_item(op);
        }

        break;

        case UP_OBJ_ALL:
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_ALL - %s\n", STRING_OBJ_NAME(op));
#endif
        msp->flags |= (MSP_FLAG_NO_ERROR | MSP_FLAG_UPDATE);
        msp_update(m, NULL, x, y);
        break;

        default:
        LOG(llevBug, "BUG:: update_object called with invalid action: %d\n", action);
        return;
    }

    /* Update all parts of a multipart. */
    if (op->more)
    {
        update_object(op->more, action);
    }
}

/** Frees all data belonging to an object, but doesn't
 * care about the object itself. This can be used for
 * non-GC objects like archetype clone objects */
void free_object_data(object_t *ob, int free_static_data)
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
                  object_t *registered = hashtable_find(beacon_table, ob->custom_attrset);

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
static void destroy_ob_inv(object_t *op)
{
    object_t *tmp,
           *next;

#if defined DEBUG_GC
    if(op->inv)
        LOG(llevDebug, "  destroy_ob_inv(%s (%d))\n", STRING_OBJ_NAME(op), op->count);
#endif

    FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
    {
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
void destroy_object(object_t *ob)
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
  object_t *tmp=free_objects;
  while(tmp!=NULL)
    tmp=tmp->next, i++;
  return i;
}

/*
 * count_used() returns the number of objects on the list of used objects.
 */

int count_used() {
  int i=0;
  object_t *tmp=objects;
  while(tmp!=NULL)
    tmp=tmp->next, i++;
  return i;
}
#endif

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
void remove_ob(object_t *op)
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

static void RemoveFromEnv(object_t *op)
{
    object_t *env = op->env;

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

static void RemoveFromMap(object_t *op)
{
    msp_t *msp = MSP_KNOWN(op);

    /* Sort out object chain. Don't NULL op->map (still needed). */
    if (op->above)
    {
        op->above->below = op->below;
    }
    else
    {
        msp->last = op->below;
    }

    if (op->below)
    {
        op->below->above = op->above;
    }
    else
    {
        msp->first = op->above;
    }

    op->above = NULL;
    op->below = NULL;

    /* When a map is swapped out and the objects on it get removed too. */
    if (op->map->in_memory == MAP_MEMORY_SAVING)
    {
        return;
    }

    if (op->type == PLAYER)
    {
        player_t *pl = CONTR(op);

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

    if (op->layer > MSP_SLAYER_UNSLICED)
    {
        msp_rebuild_slices_without(msp, op);
    }

    update_object(op, UP_OBJ_REMOVE);
    op->env = NULL;
}

/* Recursively remove the inventory of op. */
void remove_ob_inv(object_t *op)
{
    object_t *this,
           *next;

    FOREACH_OBJECT_IN_OBJECT(this, op, next)
    {
        if (this->inv)
        {
            remove_ob_inv(this);
        }

        remove_ob(this);
    }
}

/* kill_object() causws victim to be 'killed' by killer. killer may be NULL in
 * which case victim just dies (of natural causes). It is possible that victim
 * has a DEATH script wh */
object_t *kill_object(object_t *victim, object_t *killer)
{
    uint32 tag = victim->count;

    /* A DEATH script may return true to cheat death (or otherwise handle it).
     * If it does then return victim or NULL depending on whether victim still
     * exists afterwards. If it returns false then return NULL if victim no
     * longer exists afterwards or carry on otherwise. */
    if (trigger_object_plugin_event(EVENT_DEATH, victim, killer, victim, NULL, NULL, NULL, NULL, SCRIPT_FIX_ALL))
    {
        if (OBJECT_VALID(victim, tag))
        {
            return victim;
        }
    }

    if (!OBJECT_VALID(victim, tag))
    {
        return NULL;
    }

    /* Show Damage System for clients
     * whatever is dead now, we check map. If it on map, we redirect last_damage
     * to map space, giving player the chance to see the last hit damage they had
     * done. If there is more as one object killed on a single map tile, we overwrite
     * it now. This visual effect works pretty good. MT */
    /* no pet/player/monster checking now, perhaps not needed */
    if (victim->map)
    {
        msp_t *msp = MSP_KNOWN(victim);

        if (victim->damage_round_tag == ROUND_TAG)
        {
            msp->last_damage = victim->last_damage;
            msp->round_tag = ROUND_TAG;
        }

        play_sound_map(msp, SOUND_PLAYER_KILLS, SOUND_NORMAL);
    }

    /* Notify player that a pet has died */
    if (victim->type == MONSTER)
    {
        object_t *victim_owner = get_owner(victim);

        if (victim_owner &&
            victim_owner->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, victim_owner, "%s was killed!",
                query_name(victim, victim_owner, ARTICLE_POSSESSIVE, 0));
        }
    }

    /* aggroless kill (eg, script mob:Kill()) -- force empty corpse */
    if (!killer)
    {
        if (victim->type == MONSTER)
        {
            /* ...have no enemy (corpse will be noone's bounty and noone
             * gets a kill credit). */
            victim->enemy = NULL;

            /* ...leave empty corpses (as long as the mob leaves a corpse
             * at all AND unless the mob already has a forced corpse. */
            if (QUERY_FLAG(victim, FLAG_CORPSE) &&
                !QUERY_FLAG(victim, FLAG_CORPSE_FORCED))
            {
                SET_FLAG(victim, FLAG_CORPSE_FORCED);
                SET_FLAG(victim, FLAG_NO_DROP);
            }
        }
    }
    /* The killer nay be a pet or spell or missile or such so we need to know
     * the owner to calculate exp/loot. */
    else
    {
        object_t *owner = get_owner(killer);

        if (!owner)
        {
            owner = killer;
        }

        /* Create kill message */
        if (owner->type == PLAYER)
        {
            char      buf[MEDIUM_BUF] = "";
            object_t *corpse_owner;

            if (owner != killer)
            {
                if (killer->type == MONSTER)
                {
                    ndi(NDI_WHITE, 0, owner, "%s killed %s!",
                        query_name(killer, NULL, ARTICLE_POSSESSIVE, 1),
                        QUERY_SHORT_NAME(victim, NULL));
                    sprintf(buf, "%s's %s killed %s.",
                        QUERY_SHORT_NAME(owner, NULL),
                        query_name(killer, NULL, ARTICLE_NONE, 1),
                        QUERY_SHORT_NAME(victim, NULL));
                }
                else
                {
                    ndi(NDI_WHITE, 0, owner, "You killed %s with %s!",
                        QUERY_SHORT_NAME(victim, NULL),
                        query_name(killer, NULL, ARTICLE_NONE, 1));
                    sprintf(buf, "%s killed %s with %s.",
                        QUERY_SHORT_NAME(owner, NULL),
                        QUERY_SHORT_NAME(victim, NULL),
                        query_name(killer, NULL, ARTICLE_NONE, 1));
                }

                owner->skillgroup = killer->skillgroup;
            }
            else
            {
                ndi(NDI_WHITE, 0, owner, "You killed %s!",
                    QUERY_SHORT_NAME(victim, NULL));
                sprintf(buf, "%s killed %s.",
                    QUERY_SHORT_NAME(owner, NULL),
                    QUERY_SHORT_NAME(victim, NULL));
            }

            /* Give exp and create the corpse. Decide we get a loot or not. */
            corpse_owner = aggro_calculate_exp(victim, owner, (buf[0] != '\0') ? buf : NULL);

            if (victim->type == MONSTER)
            {
#if 0
                if (!corpse_owner)
                {
                    corpse_owner = owner;
                }

#endif
                victim->enemy = corpse_owner;
                victim->enemy_count = corpse_owner->count;
            }
        }
        /* mob/npc kill - force a droped corpse without items */
        else
        {
            if (victim->type == MONSTER)
            {
                victim->enemy = NULL;
                SET_FLAG(victim, FLAG_CORPSE_FORCED);
                SET_FLAG(victim, FLAG_NO_DROP);
            }
        }
    }

    victim->speed = 0;
    update_ob_speed(victim); /* remove from active list (if on) */
    DropLoot(victim);
    remove_ob(victim);
    check_walk_off(victim, NULL, MOVE_APPLY_DEFAULT);
    return NULL;
}

/* DropLoot() needs a rewrite. */
static void DropLoot(object_t *ob)
{
    player_t *pl      = NULL;
    object_t *corpse  = NULL;
    object_t *tmp_op,
           *next;
    object_t *gtmp, *tmp     = NULL;

    if (!ob->env &&
        (!ob->map ||
         ob->map->in_memory != MAP_MEMORY_ACTIVE))
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
         !QUERY_FLAG(ob, FLAG_NO_DROP)) ||
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

    FOREACH_OBJECT_IN_OBJECT(tmp_op, ob, next)
    {
        remove_ob(tmp_op); /* Inv-no check off / This will be destroyed in next loop of object_gc() */

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
        /* if we recall spawn mobs, we don't want drop their items as free.
         * So, marking the mob itself with "FLAG_NO_DROP" will kill
         * all inventory and not dropping it on the map.
         * This also happens when a player slays a to low mob/non exp mob.
         * Don't drop any sys_object in inventory... I can't think about
         * any use... when we do it, a disease needle for example
         * is dropping his disease force and so on. */
        else if (!(QUERY_FLAG(ob, FLAG_NO_DROP) ||
                   (tmp_op->type != RUNE &&
                    (QUERY_FLAG(tmp_op, FLAG_SYS_OBJECT) ||
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

object_t *insert_ob_in_map(object_t *const op, map_t *m, object_t *const originator, const int flag)
{
    object_t         *tmp = NULL,
                   *top;
    sint16          x,
                    y;
    msp_t       *msp;
    int             lt, layer, layer_inv;
    map_t      *old_map = op->map;

    /* some tests to check all is ok... some cpu ticks
     * which tracks we have problems or not */
    if (OBJECT_FREE(op))
    {
        dump_object(op);
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert freed object %s in map %s!\n:%s\n",
            STRING_OBJ_NAME(op), m->name, errmsg);
        return NULL;
    }

    if (!m)
    {
        dump_object(op);
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert object %s in null-map!\n%s\n",
            STRING_OBJ_NAME(op), errmsg);
        return NULL;
    }

    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        dump_object(op);
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert non removed object %s in map %s.\n%s\n",
            STRING_OBJ_NAME(op), m->name, errmsg);
        return NULL;
    }

    /* tail, but no INS_TAIL_MARKER: we had messed something outside! */
    if (op->head &&
        !(flag & INS_TAIL_MARKER))
    {
        LOG(llevBug, "BUG: insert_ob_in_map(): inserting op->more WITHOUT INS_TAIL_MARKER! OB:%s (ARCH: %s) (MAP: %s (%d,%d))\n",
            STRING_OBJ_NAME(op), op->arch->name, m->path, op->x, op->y);
        return NULL;
    }

    if (op->more)
    {
        if (!insert_ob_in_map(op->more, op->more->map, originator, flag | INS_TAIL_MARKER))
        {
            if (!op->head)
                LOG(llevBug, "BUG: insert_ob_in_map(): inserting op->more killed op %s in map %s\n",
                    STRING_OBJ_NAME(op), m->name);
            return NULL;
        }
    }

    /* A stackable object with nrof 0 is junk.
     * We do this because otherwise objects marked for removal by merge_ob()
     * can be un-removed here. */
    if (QUERY_FLAG(op, FLAG_CAN_STACK) &&
        !op->nrof)
        return NULL;

#ifdef POSITION_DEBUG
    op->ox = op->x;
    op->oy = op->y;
#endif

#if 0
    /* this is now a key part of this function, because
     * we adjust multi arches here when they cross map boarders!
     */
    x = op->x;
    y = op->y;
    op->map = m;
    msp = MSP_GET(m, x, y);

    if (!msp)
    {
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert object %s outside the map %s (%d,%d).\n\n",
            STRING_OBJ_NAME(op), op->map->path, op->x, op->y);
        return NULL;
    }

    /* x and y will only change when we change the map too - so check the map */
    if (op->map != m)
    {
        op->map = m;
        op->x = x;
        op->y = y;
    }
#else
    x = op->x;
    y = op->y;
    op->map = m;
    msp = MSP_GET2(m, x, y);

    if (!msp)
    {
        LOG(llevBug, "BUG: insert_ob_in_map(): Trying to insert object %s outside the map %s (%d,%d).\n\n",
            STRING_OBJ_NAME(op), op->map->path, op->x, op->y);
        return NULL;
    }

    /* x and y will only change when we change the map too - so check the map */
    if (op->map != m)
    {
        op->map = m;
        op->x = x;
        op->y = y;
    }
#endif

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
    /* Layer 0 (system) objects always go to the first object on the square --
     * everything else is ->above it -- and are never visible on the client map
     * -- except to gmaster_matrix players but this is handled specially. */
    if (op->layer == MSP_SLAYER_SYSTEM)
    {
        if ((top = msp->first))
        {
            top->below = op;
            op->above = top;
        }
        else
        {
            msp->last = op;
        }

        msp->first = op;
    }
    /* Other layers are non-system objects always go to the last object on the
     * square -- everything else is ->below it. */
    else
    {
        if ((top = msp->last))
        {
            top->above = op;
            op->below = top;
        }
        else
        {
            msp->first = op;
        }

        msp->last = op;
    }

    /* lets set some specials for our players
     * we adjust the ->player map variable and the local
     * map player chain.
     */
    if (op->type == PLAYER)
    {
        player_t *pl = CONTR(op);

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

    if (op->layer > MSP_SLAYER_UNSLICED)
    {
        msp_rebuild_slices_with(msp, op);
    }

    if(!(op->map->flags & MAP_FLAG_NO_UPDATE))
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
    if (op->type == PLAYER)
    {
        if (op->map != old_map &&
            CONTR(op)->pets)
        {
            pets_follow_owner(op);
        }
    }
    /* For exits we NULL ->race, which is the destination path. This forces it
     * to be recalculated next time the exit is used. Which means that an
     * inherited exit can be moved to a different (status) map and still
     * function correctly as an exit. */
    else if (op->type == EXIT ||
             op->type == TELEPORTER ||
             op->type == PIT ||
             op->type == TRAPDOOR)
    {
        FREE_AND_CLEAR_HASH(op->race);
    }

    /* check walk on/fly on flag if not canceled AND there is some to move on.
     * Note: We are first inserting the WHOLE object/multi arch - then we check all
     * part for traps. This ensures we don't must do nasty hacks with half inserted/removed
     * objects - for example when we hit a teleporter trap.
     * Check only for single tiles || or head but ALWAYS for heads.
     */
    if (!(flag & INS_NO_WALK_ON) && (msp->flags & (MSP_FLAG_WALK_ON | MSP_FLAG_FLY_ON) || op->more) && !op->head)
    {
        for (tmp = op; tmp; tmp = tmp->more)
        {
            int event;

            msp = MSP_KNOWN(tmp);

            /* tmp is flying/levitating but no fly event here */
            if (IS_AIRBORNE(tmp)) /* Old code queried op only, but as
                                   * check_walk_on() may be called on tmp and
                                   * queries it for the flying flags, we must
                                   * do so here too -- Smacky 20080926 */
            {
                if (!(msp->flags & MSP_FLAG_FLY_ON))
                    continue;
            }
            /* tmp is walkimg but no walk event here */
            else
            {
                if (!(msp->flags & MSP_FLAG_WALK_ON))
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
void replace_insert_ob_in_map(char *arch_string, object_t *op)
{
    map_t *m = op->map;
    sint16     x = op->x,
               y = op->y;
    msp_t  *msp = MSP_KNOWN(op);
    object_t    *this,
              *next;

    /* Search for itself and remove any old instances */
    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (!strcmp(this->arch->name, arch_string)) /* same archetype */
        {
            remove_ob(this); /* no move off here... should be ok, this is a technical function */
        }
    }

    /* Insert a replacement. */
    this = get_archetype(arch_string);
    this->x = x;
    this->y = y;
    insert_ob_in_map(this, m, op, 0);
}

/*
 * get_split_ob(ob,nr) splits up ob into two parts.  The part which
 * is returned contains nr objects, and the remaining parts contains
 * the rest (or is removed and freed if that number is 0).
 * On failure, NULL is returned.
 */

object_t * get_split_ob(object_t *orig_ob, uint32 nr)
{
    object_t *newob,
           *tmp,
           *next,
           *event;

    if(!orig_ob)
        return NULL;

    if (orig_ob->nrof < nr)
    {
        LOG(llevDebug, "get_split_ob(): There are only %d %ss.", orig_ob->nrof ? orig_ob->nrof : 1, STRING_OBJ_NAME(orig_ob));
        return NULL;
    }

    if (orig_ob->env == NULL && orig_ob->map->in_memory != MAP_MEMORY_ACTIVE)
    {
        LOG(llevDebug, "get_split_ob(): Tried to split object whose map is not in memory.\n");
        return NULL;
    }

    newob = get_object();
    copy_object(orig_ob, newob);

    /* Gecko: copy inventory (event objects)  */
    FOREACH_OBJECT_IN_OBJECT(tmp, orig_ob, next)
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

object_t * decrease_ob_nr(object_t *op, uint32 i)
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

                esrv_update_item(flags, op);
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
object_t *insert_ob_in_ob(object_t *op, object_t *where)
{
    map_t *m;
    object_t    *merged;

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

int check_walk_on(object_t *const op, object_t *const originator, int flags)
{
    uint8      fly,
               local_walk_semaphore = 0; // 1= root call for static_walk_semaphore setting
    tag_t      tag;
    msp_t  *msp;
    object_t    *this,
              *next;

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
    msp = MSP_KNOWN(op);

    FOREACH_OBJECT_IN_MSP(this, msp, next)
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

            if (!OBJECT_VALID(op, tag)) // we got killed, removed or whatever
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
int check_walk_off(object_t *op, object_t *originator, int flags)
{
    object_t     *part;
    int         local_walk_semaphore    = FALSE; /* when TRUE, this function is root call for static_walk_semaphore setting */
    int         fly;
    tag_t       tag;


    if (!op || !op->map) /* no map, no walk off - item can be in inventory and/or ... */
        return CHECK_WALK_OK; /* means "nothing happens here" */

    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        LOG(llevBug, "BUG: check_walk_off: object %s is not removed when called\n", STRING_OBJ_NAME(op));
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
        map_t    *m = op->map;
        sint16    x = part->x,
                  y = part->y;
        msp_t    *msp = MSP_GET(m, x, y);
        object_t *tmp,
                 *next;

        if (!(msp->flags & (MSP_FLAG_WALK_OFF | MSP_FLAG_FLY_OFF))) /* no event on this tile */
        {
            continue;
        }

        /* This flags ensures we notice when a moving event has appeared!
         * Because the functions who set/clear the flag can be called recursive
         * from this function and walk_off() we need a static, global semaphor
         * like flag to ensure we don't clear the flag except in the mother
         * call. */
        if (!static_walk_semaphore)
        {
            local_walk_semaphore = TRUE;
            static_walk_semaphore = TRUE;
            CLEAR_FLAG(op, FLAG_OBJECT_WAS_MOVED);
        }

        FOREACH_OBJECT_IN_MSP(tmp, msp, next)
        {
            if (tmp == part) /* its the ob part in this space... better not >1 part in same space of same arch */
            {
                continue;
            }

            if ((fly) ? QUERY_FLAG(tmp, FLAG_FLY_OFF) : QUERY_FLAG(tmp, FLAG_WALK_OFF)) /* event */
            {
                move_apply(tmp, part, originator, flags);

                if (OBJECT_FREE(part) ||
                    tag != op->count)
                {
                    if (local_walk_semaphore)
                    {
                        static_walk_semaphore = FALSE;
                    }

                    return CHECK_WALK_DESTROYED;
                }

                /* and here a insert_xx() was triggered - we MUST stop now */
                if (!QUERY_FLAG(part, FLAG_REMOVED) ||
                    QUERY_FLAG(part, FLAG_OBJECT_WAS_MOVED))
                {
                    if (local_walk_semaphore)
                    {
                        static_walk_semaphore = FALSE;
                    }

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
    {
        static_walk_semaphore = FALSE;
    }

    return CHECK_WALK_OK;
}

/* present_arch(arch, map, x, y) searches for any objects with
 * a matching archetype at the given map and coordinates.
 * The first matching object is returned, or NULL if none. */
object_t *present_arch(archetype_t *at, map_t *m, sint16 x, sint16 y)
{
    msp_t *msp = MSP_GET(m, x, y);
    object_t   *this,
             *next;

    if (!msp)
    {
        LOG(llevBug, "BUG:: %s/present_arch(): outside map!\n", __FILE__);
        return NULL;
    }

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (this->arch == at)
        {
            return this;
        }
    }

    return NULL;
}

/* present(type, map, x, y) searches for any objects with
 * a matching type variable at the given map and coordinates.
 * The first matching object is returned, or NULL if none. */
object_t *present(unsigned char type, map_t *m, sint16 x, sint16 y)
{
    msp_t *msp = MSP_GET(m, x, y);
    object_t   *this,
             *next;

    if (!msp)
    {
        LOG(llevBug, "BUG:: %s/present(): outside map!\n", __FILE__);
        return NULL;
    }

    FOREACH_OBJECT_IN_MSP(this, msp, next)
    {
        if (this->type == type)
        {
            return this;
        }
    }

    return NULL;
}

/* present_in_ob(type, object) searches for any objects with
 * a matching type variable in the inventory of the given object.
 * The first matching object is returned, or NULL if none. */
object_t *present_in_ob(unsigned char type, object_t *op)
{
    object_t *this,
           *next;

    FOREACH_OBJECT_IN_OBJECT(this, op, next)
    {
        if (this->type == type)
        {
            return this;
        }
    }

    return NULL;
}

/* present_arch_in_ob(archetype, object) searches for any objects with
 * a matching archetype in the inventory of the given object.
 * The first matching object is returned, or NULL if none. */
object_t *present_arch_in_ob(archetype_t *at, object_t *op)
{
    object_t *this,
           *next;

    FOREACH_OBJECT_IN_OBJECT(this, op, next)
    {
        if (this->arch == at)
        {
            return this;
        }
    }

    return NULL;
}

/* present_arch_in_ob_temp(archetype, object) searches for any objects with
 * a matching archetype in the inventory of the given object which
 * has the flag FLAG_IS_USED_UP set.
 * The first matching object is returned, or NULL if none. */
object_t *present_arch_in_ob_temp(archetype_t *at, object_t *op)
{
    object_t *this,
           *next;

    FOREACH_OBJECT_IN_OBJECT(this, op, next)
    {
        if (this->arch == at &&
            QUERY_FLAG(this, FLAG_IS_USED_UP))
        {
            return this;
        }
    }

    return NULL;
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
object_t * load_object_str(char *obstr)
{
    object_t *ob;

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


int auto_apply(object_t *op)
{
    object_t *tmp,
           *next;
    int     i,
            level;

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
          FOREACH_OBJECT_IN_OBJECT(tmp, op, next)
          {
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
object_t *locate_beacon(shstr_t *id)
{
    if(id == NULL)
    {
        LOG(llevBug, "locate_beacon(NULL)\n");
        return NULL;
    }
    return (object_t *)hashtable_find(beacon_table, id);
}

/** Intializer function for TYPE_BEACON objects.
 * Ensures the beacon is added to the beacon hashtable.
 */
static void beacon_initializer(object_t *op)
{
    object_t *parent;

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
    if (!(parent->map->status & MAP_STATUS_MULTI))
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

static void monster_initializer(object_t *op)
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
object_t *find_next_object(object_t *op, uint8 type, uint8 mode, object_t *root)
{
    object_t *next;

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
            object_t *tmp = NULL;

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
void revert_buff_stats(object_t *item)
{
    object_t *original;

    if (item && item->original)
    {
        original = item->original;

        // The original version will still have the coordinates from when it was copied.
        original->x = item->x;
        original->y = item->y;

        // Null the item's "original" so it's not copied in copy_object.
        item->original = NULL;

        // Copy the original over the real item
        copy_object(original, item);

        // Get rid of the original object.
        remove_ob(original);
    }
}

/* Copy stats from all BUFF_FORCEs in item's inventory to the item. It will also
 * store the original version of that object in item->original so that it can
 * easily be reverted or compared.
 */
void fix_buff_stats(object_t *item)
{
    int i = 0;
    uint32 n = 0;
    object_t *inv,
           *next;
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
    if (item->original)
    {
        // If orig was found, item should be different than orig, so revert item to orig.
        revert_buff_stats(item);
    }

    // Try to create a duplicate of the object to make changing stats relative to the original.
    item->original = get_object();
    copy_object(item, item->original);

    // Even though the object isn't on a map or inside another, it shouldn't be removed on cleanup.
    CLEAR_FLAG(item->original, FLAG_REMOVED);

    if (!item->original)
    {
        LOG(llevDebug, "fix_buff_stats() failed - could not copy original object\n");
        return;
     }

    FOREACH_OBJECT_IN_OBJECT(inv, item, next)
    {
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
uint8 check_buff_limit(object_t *op, int nr)
{
    sint8 real_max = 0; // The max after considering item_condition
    sint8 buffs = 0;
    object_t *inv,
           *next;

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

    FOREACH_OBJECT_IN_OBJECT(inv, op, next)
    {
        if (inv->type == BUFF_FORCE)
        {
            buffs += inv->nrof;
        }
    }

    if (buffs + nr > real_max)
    {
        return FALSE;
    }

    return TRUE;

}

object_t * check_buff_exists(object_t *item, const char *name)
{
    object_t *inv,
           *next;

    if (!item || !name)
    {
        LOG(llevDebug, "check_buff_exists() called without item or name!\n");
        return NULL;
    }

    FOREACH_OBJECT_IN_OBJECT(inv, item, next)
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
int add_item_buff(object_t *item, object_t *buff, short just_checking)
{
    object_t *oldbuff;
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

int remove_item_buff(object_t *item, char *name, uint32 nrof)
{
    object_t *oldbuff;

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

/* query_name() returns a pointer to a string which is the qualified name of
 * what.
 *
 * Qualified name means (often) much more than just what->name.
 *
 * If who is non-NULL thennn what will be named from who's perspective, where
 * appropriate.
 *
 * The article of the name has a somewhat broader, and narrower at the same
 * time, meaning than usual. article determines if we prefix the name both with
 * an article and what->nrof if this is > 1:
 *   ARTICLE_NONE means no article and no nrof.
 *   ARTICLE_INDEFINITE means the indefinite article ("a") and nrof.
 *   ARTICLE_DEFINITE means the definite article ("the") and nrof.
 *   ARTICLE_POSSESSIVE means the possessive article ("your") and nrof.
 *
 * While ARTICLE_NONE and ARTICLE_POSSESSIVE force the above, both
 * ARTICLE_INDEFINITE and ARTICLE_DEFINITE can be overidden if who is specified
 * and what is an applied item in who's inventory or who is the owner of what.
 *
 * Furthermore, the last three are overridden if what is named, in which case
 * no article but yes nrof.
 *
 * TODO: status will probably be removed and handled client-side in future.
 * 
 * NOTE: Capitalisation, pluralisation and other string mungeing will be
 * handled client-side. */
char *query_name(object_t *what, object_t *who, uint32 article, uint8 status)
{
    static char   buf[5][MEDIUM_BUF];
    static uint8  n = 0;
    char         *cp;

    if (!what)
    {
        return ">NONAME<";
    }

    /* We use 5 alternating static buffers to hold the qualified name. This
     * means  we can have up to 5 query_name()s in the same format string
     * (which should be ample; I think the most currently used is 3). */
    *(cp = buf[(n = (n + 1) % 5)]) = '\0';

    if (article != ARTICLE_NONE)
    {
        /* Named objects override the article but we still check for quantity. */
        if (!QUERY_FLAG(what, FLAG_IS_NAMED))
        {
            /* Possessive. */
            if (article == ARTICLE_POSSESSIVE ||
                who &&
                (what->env == who &&
                 QUERY_FLAG(what, FLAG_APPLIED) ||
                 get_owner(what) == who))
            {
                sprintf(cp, "your ");
            }
            /* Indefinite. */
            else if (article == ARTICLE_INDEFINITE)
            {
                sprintf(cp, "a ");
            }
            /* Definite. */
            else // if (article == ARTICLE_DEFINITE)
            {
                sprintf(cp, "the ");
            }
        }

        /* Quantity if > 1 (ie, don't call with article = 1 if what->nrof > 1). */
        if (what->nrof > 1)
        {
            sprintf(strchr(cp, '\0'), "%u ", what->nrof);
        }
    }

    if (what->sub_type1 == ARROW &&
        what->type == MISC_OBJECT) /* special neutralized arrow! */
    {
        sprintf(strchr(cp, '\0'), "broken ");
    }

    if(!QUERY_FLAG(what, FLAG_IS_NAMED)) /* named items has no prefix */
    {
        const char *prefix;

        /* add the item race name */
        if (!IS_LIVE(what) &&
            what->type != TYPE_BASE_INFO &&
            (prefix = item_race_table[what->item_race].name) != "")
        {
            sprintf(strchr(cp, '\0'), "%s", prefix);
        }

        /* we add the real material name as prefix. Because real_material == 0 is
         * "" (clear string) we don't must check item types for adding something here
         * or not (artifacts for example has normally no material prefix) */
        if (what->material_real > 0 &&
            QUERY_FLAG(what, FLAG_IDENTIFIED) &&
            (prefix = material_real[what->material_real].name) != "")
        {
            sprintf(strchr(cp, '\0'), "%s", prefix);
        }
    }

    if (what->type == PLAYER)
    {
        sprintf(strchr(cp, '\0'), "|%s|", STRING_OBJ_NAME(what));
    }
    else
    {
        sprintf(strchr(cp, '\0'), "%s", STRING_OBJ_NAME(what));
    }

    if (QUERY_FLAG(what, FLAG_IDENTIFIED) ||
        !need_identify(what))
    {
        switch (what->type)
        {
            case SPELLBOOK:
            if (!what->title)
            {
                if (what->slaying)
                {
                    sprintf(strchr(cp, '\0'), " of %s", what->slaying);
                }
                else
                {
                    sprintf(strchr(cp, '\0'), " of %s",
                        (what->stats.sp == SP_NO_SPELL) ? "nothing" : spells[what->stats.sp].name);
                }
            }
            else
            {
                sprintf(strchr(cp, '\0'), " %s", what->title);
            }
            break;

            case SCROLL:
            case WAND:
            case ROD:
            case HORN:
            case POTION:
            if (!what->title)
            {
                sprintf(strchr(cp, '\0'), " of %s",
                    (what->stats.sp == SP_NO_SPELL) ? "nothing" : spells[what->stats.sp].name);
            }
            else
            {
                sprintf(strchr(cp, '\0'), " %s", what->title);
            }

            sprintf(strchr(cp, '\0'), " (lvl %d)", what->level);
            break;

            case PLAYER:
            case MONSTER:
            case TYPE_BASE_INFO:
            if (what->title)
            {
                sprintf(strchr(cp, '\0'), " %s", what->title);
            }
            break;

            default:
            if (what->magic)
            {
                sprintf(strchr(cp, '\0'), " %+d", what->magic);
            }

            if (what->title)
            {
                sprintf(strchr(cp, '\0'), " %s", what->title);
            }

            if ((what->type == ARROW ||
                 (what->type == MISC_OBJECT &&
                  what->sub_type1 == ARROW)) && // special neutralized arrow
                what->slaying)
            {
                sprintf(strchr(cp, '\0'), " %s", what->slaying);
            }
        }
    }

    if (what->type == CONTAINER)
    {
        if (what->sub_type1 >= ST1_CONTAINER_NORMAL_group)
        {
            if (what->sub_type1 == ST1_CONTAINER_CORPSE_group)
            {
                if (!who)
                {
                    sprintf(strchr(cp, '\0'), " (bounty of a group)");
                }
                else if ((CONTR(who)->group_status & GROUP_STATUS_GROUP) &&
                    CONTR(CONTR(who)->group_leader)->group_id == what->stats.maxhp)
                {
                    sprintf(strchr(cp, '\0'), " (bounty of your group%s)", 
                        (QUERY_FLAG(what, FLAG_BEEN_APPLIED)) ? ", SEARCHED" : "");
                }
                else
                {
                    sprintf(strchr(cp, '\0'), " (bounty of another group)");
                }
            }
        }
        else if (what->sub_type1 >= ST1_CONTAINER_NORMAL_player)
        {
            if (what->sub_type1 == ST1_CONTAINER_CORPSE_player)
            {
                if (what->slaying)
                {
                    sprintf(strchr(cp, '\0'), " (bounty of %s%s)",
                        what->slaying,
                        ((who &&
                          who->name == what->slaying) &&
                          QUERY_FLAG(what, FLAG_BEEN_APPLIED)) ? ", SEARCHED" : "");
                }
                else if (QUERY_FLAG(what, FLAG_BEEN_APPLIED))
                {
                    sprintf(strchr(cp, '\0'), " (SEARCHED)");
                }
            }
        }
    }

    if (!QUERY_FLAG(what, FLAG_NO_PICK))
    {
        if (QUERY_FLAG(what, FLAG_UNPAID))
        {
            sprintf(strchr(cp, '\0'), " |U|");
        }

        if (QUERY_FLAG(what, FLAG_INV_LOCKED))
        {
            sprintf(strchr(cp, '\0'), " |*|");
        }

        if (QUERY_FLAG(what, FLAG_NO_DROP))
        {
            sprintf(strchr(cp, '\0'), " (~no-drop~)");
        }

        if (QUERY_FLAG(what, FLAG_ONE_DROP))
        {
            sprintf(strchr(cp, '\0'), " (~one-drop~)");
        }
        else if (QUERY_FLAG(what, FLAG_QUEST_ITEM))
        {
            sprintf(strchr(cp, '\0'), " (~quest~)");
        }

        if (QUERY_FLAG(what, FLAG_KNOWN_CURSED))
        {
            if (QUERY_FLAG(what, FLAG_PERM_DAMNED))
            {
                sprintf(strchr(cp, '\0'), " (perm. damned)");
            }
            else if (QUERY_FLAG(what, FLAG_DAMNED))
            {
                sprintf(strchr(cp, '\0'), " (damned)");
            }
            else if (QUERY_FLAG(what, FLAG_PERM_CURSED))
            {
                sprintf(strchr(cp, '\0'), " (perm. cursed)");
            }
            else if (QUERY_FLAG(what, FLAG_CURSED))
            {
                sprintf(strchr(cp, '\0'), " (cursed)");
            }
        }
    }

    /* No status so we're done. */
    if (!status)
    {
        return cp;
    }

    /* TODO: This will be handled differently in future. */
    switch (what->type)
    {
        case RING:
        case WEAPON:
        case ARMOUR:
        case BRACERS:
        case HELMET:
        case SHOULDER:
        case LEGS:
        case SHIELD:
        case BOOTS:
        case GLOVES:
        case AMULET:
        case GIRDLE:
        case BOW:
        case ARROW:
        case CLOAK:
        case FOOD:
        case DRINK:
        case FLESH:
        case CONTAINER:
        if (!what->title)
        {
            sprintf(strchr(cp, '\0'), " %s", describe_item(what));
        }
        break;
    }

    /* TODO: I believe this is only of interest if (who && what->env == who).
     * This means that this info is better added client-side.
     *
     * -- Smacky 20140407 */
    if (QUERY_FLAG(what, FLAG_APPLIED))
    {
        switch (what->type)
        {
            case BOW:
            case WAND:
            case ROD:
            case HORN:
            sprintf(strchr(cp, '\0'), " (readied)");
            break;
            case WEAPON:
            sprintf(strchr(cp, '\0'), " (wielded)");
            break;
            case ARMOUR:
            case SHOULDER:
            case LEGS:
            case HELMET:
            case SHIELD:
            case RING:
            case BOOTS:
            case GLOVES:
            case AMULET:
            case GIRDLE:
            case BRACERS:
            case CLOAK:
            sprintf(strchr(cp, '\0'), " (worn)");
            break;
            case CONTAINER:
            if (what->attacked_by &&
                what->attacked_by->type == PLAYER)
            {
                sprintf(strchr(cp, '\0'), " (open)");
            }
            else
            {
                sprintf(strchr(cp, '\0'), " (ready)");
            }
            break;
            case TYPE_SKILL:
            default:
            sprintf(strchr(cp, '\0'), " (applied)");
        }
    }

    return cp;
}

map_t *parent_map(object_t *what)
{
    map_t *m;

    while (what)
    {
        if ((m = what->map))
        {
            break;
        }

        what = what->env;
    }

    return m;
}

