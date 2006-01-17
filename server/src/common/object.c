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
#ifndef WIN32 /* ---win32 exclude headers */
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#endif /* win32 */


static int              static_walk_semaphore                               = FALSE; /* see walk_off/walk_on functions  */


int                     freearr_x[SIZEOFFREE]                               =
{
    0, 0, 1, 1, 1, 0, -1, -1, -1, 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3,
    2, 1, 0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1
};
int                     freearr_y[SIZEOFFREE]                               =
{
    0, -1, -1, 0, 1, 1, 1, 0, -1, -2, -2, -2, -1, 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -3, -3, -3, -3, -2, -1, 0, 1,
    2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3
};
int                     maxfree[SIZEOFFREE]                                 =
{
    0, 9, 10, 13, 14, 17, 18, 21, 22, 25, 26, 27, 30, 31, 32, 33, 36, 37, 39, 39, 42, 43, 44, 45, 48, 49, 49, 49, 49,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49
};
int                     freedir[SIZEOFFREE]                                 =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 2, 2, 3, 4, 4, 4, 5, 6, 6, 6, 7, 8, 8, 8, 1, 2, 2, 2, 2, 2, 3, 4, 4, 4, 4, 4, 5, 6,
    6, 6, 6, 6, 7, 8, 8, 8, 8, 8
};

/** Object management functions **/


/*
 * Utility functions for inserting & removing objects
 * from activelists
 */
static inline void activelist_remove_inline(object *op)
{
    /* If not already on any active list, don't do anything */
    if(!QUERY_FLAG(op, FLAG_IN_ACTIVELIST))
        return;

#ifdef DEBUG_ACTIVELIST_LOG
    LOG(llevDebug,"ACTIVE_rem: %s (%d) @%s (%x - %x - %x)\n", query_name(op), op->count, STRING_MAP_PATH(op->map),
															op, op->active_prev, op->active_next);
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
    /* If already on any active list, don't do anything */
    if(QUERY_FLAG(op, FLAG_IN_ACTIVELIST))
        return;

#ifdef DEBUG_ACTIVELIST_LOG
    LOG( llevDebug,"ACTIVE_add: %s (type:%d count:%d) env:%s map:%s (%d,%d) (%x - %x - %x)\n", query_name(op), op->type, op->count,
         query_name(op->env), op->map?STRING_SAFE(op->map->path):"NULL", op->x, op->y, op, op->active_prev, op->active_next);
#endif

    /* Since we don't want to process objects twice, we make
     * sure to insert the object in a temporary list until the
     * next process_events() call. */
    op->active_next = inserted_active_objects;
    if (op->active_next != NULL)
        op->active_next->active_prev = op;
    inserted_active_objects = op;
    op->active_prev = NULL;

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

    mem->next = removed_objects;
    removed_objects = mem;
}

/* Go through all objects in the removed list and free the forgotten ones */
void object_gc()
{
    struct mempool_chunk   *current, *next;
    object                 *ob;

    while ((next = removed_objects) != &end_marker)
    {
        removed_objects = &end_marker; /* destroy_object() may free some more objects (inventory items) */
        while (next != &end_marker)
        {
            current = next;
            next = current->next;
            current->next = NULL;

            ob = (object *) MEM_USERDATA(current);
            if (QUERY_FLAG(ob, FLAG_REMOVED))
            {
                if (OBJECT_FREE(ob))
                    LOG(llevBug, "BUG: Freed object in remove list: %s\n", STRING_OBJ_NAME(ob));
                else
                    return_poolchunk(ob, pool_object);
            }
        }
    }
}

/* Moved this out of define.h and in here, since this is the only file
 * it is used in.  Also, make it an inline function for cleaner
 * design.
 *
 * Function examines the 2 objects given to it, and returns true if
 * they can be merged together.
 *
 * Note that this function appears a lot longer than the macro it
 * replaces - this is mostly for clarity - a decent compiler should hopefully
 * reduce this to the same efficiency.
 *
 * Check nrof variable *before* calling CAN_MERGE()
 *
 * Improvements made with merge:  Better checking on potion, and also
 * check weight
 */

int CAN_MERGE(object *ob1, object *ob2)
{
    /* just some quick hack */
    if (ob1->type == MONEY && ob1->type == ob2->type && ob1->arch == ob2->arch)
        return 1;

    /* Gecko: Moved out special handling of event object nrof */
    /* important: don't merge objects with glow_radius set - or we come
     * in heavy side effect situations. Because we really not know what
     * our calling function will do after this merge (and the calling function
     * then must first find out a merge has happen or not). The sense of stacks
     * are to store inactive items. Because glow_radius items can be active even
     * when not apllied, merging is simply wrong here. MT.
     */
    if (((!ob1->nrof || !ob2->nrof) && ob1->type != TYPE_EVENT_OBJECT) || ob1->glow_radius || ob2->glow_radius)
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
    if (ob1->inv || ob2->inv)
    {
        object *tmp1, *tmp2;
        if (!ob1->inv || !ob2->inv)
            return 0;

        /* Check that all inv objects are event objects */
        for (tmp1 = ob1->inv, tmp2 = ob2->inv; tmp1 && tmp2; tmp1 = tmp1->below, tmp2 = tmp2->below)
            if (tmp1->type != TYPE_EVENT_OBJECT || tmp2->type != TYPE_EVENT_OBJECT)
                return 0;
        if (tmp1 || tmp2) /* Same number of events */
            return 0;

        for (tmp1 = ob1->inv; tmp1; tmp1 = tmp1->below)
        {
            for (tmp2 = ob2->inv; tmp2; tmp2 = tmp2->below)
                if (CAN_MERGE(tmp1, tmp2))
                    break;
            if (!tmp2) /* Couldn't find something to merge event from ob1 with? */
                return 0;
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


    if (ob1->randomitems != ob2->randomitems
     || ob1->other_arch != ob2->other_arch
     || (ob1->flags[0] | 0x300) != (ob2->flags[0] | 0x300)
   ||   /* we ignore REMOVED and BEEN_APPLIED */
        ob1->flags[1] != ob2->flags[1]
     || ob1->flags[2] != ob2->flags[2]
     || ob1->flags[3] != ob2->flags[3]
     || ob1->path_attuned != ob2->path_attuned
     || ob1->path_repelled != ob2->path_repelled
     || ob1->path_denied != ob2->path_denied
     || ob1->terrain_type != ob2->terrain_type
     || ob1->terrain_flag != ob2->terrain_flag
     || ob1->weapon_speed != ob2->weapon_speed
     || ob1->magic != ob2->magic
     || ob1->item_level != ob2->item_level
     || ob1->item_skill != ob2->item_skill
     || ob1->glow_radius != ob2->glow_radius
     || ob1->level != ob2->level)
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
 * merge_ob(op,top):
 *
 * This function goes through all objects below and including top, and
 * merges op to the first matching object.
 * If top is NULL, it is calculated.
 * Returns pointer to object if it succeded in the merge, otherwise NULL
 */

object * merge_ob(object *op, object *top)
{
    if (!op->nrof)
        return 0;
    if (top == NULL)
        for (top = op; top != NULL && top->above != NULL; top = top->above)
            ;
    for (; top != NULL; top = top->below)
    {
        if (top == op)
            continue;
        if (CAN_MERGE(op, top))
        {
            top->nrof += op->nrof;
            op->weight = 0; /* Don't want any adjustements now */
            remove_ob(op); /* this is right: no check off */
            return top;
        }
    }
    return NULL;
}

/*
 * sum_weight() is a recursive function which calculates the weight
 * an object is carrying.  It goes through in figures out how much
 * containers are carrying, and sums it up.
 */
signed long sum_weight(object *op)
{
    sint32  sum;
    object *inv;

    for (sum = 0, inv = op->inv; inv != NULL; inv = inv->below)
    {
        if (QUERY_FLAG(inv, FLAG_SYS_OBJECT))
            continue;

        if (inv->inv)
            sum_weight(inv);
        sum += inv->carrying + (inv->nrof ? inv->weight * inv->nrof : inv->weight);
    }

    /* because we avoid calculating for EVERY item in the loop above
     * the weight adjustment for magic containers, we can run here in some
     * rounding problems... in the worst case, we can remove a item from the
     * container but we are not able to put it back because rounding.
     * well, a small prize for saving *alot* of muls in player houses for example.
     */
    if (op->type == CONTAINER && op->weapon_speed != 1.0f)
        sum = (sint32) ((float) sum * op->weapon_speed);

    op->carrying = sum;

    return sum;
}

/*
 * add_weight(object, weight) adds the specified weight to an object,
 * and also updates how much the environment(s) is/are carrying.
 */

void add_weight(object *op, sint32 weight)
{
    if(QUERY_FLAG(op, FLAG_SYS_OBJECT))
        return;
    
    while (op != NULL)
    {
        /* only *one* time magic can effect the weight of objects */
        if (op->type == CONTAINER && op->weapon_speed != 1.0f)
            weight = (sint32) ((float) weight * op->weapon_speed);
        op->carrying += weight;
        op = op->env;
    }
}

/*
 * sub_weight() recursively (outwards) subtracts a number from the
 * weight of an object (and what is carried by it's environment(s)).
 */
void sub_weight(object *op, sint32 weight)
{
    if(QUERY_FLAG(op, FLAG_SYS_OBJECT))
        return;
    
    while (op != NULL)
    {
        /* only *one* time magic can effect the weight of objects */
        if (op->type == CONTAINER && op->weapon_speed != 1.0f)
            weight = (sint32) ((float) weight * op->weapon_speed);
        op->carrying -= weight;
        op = op->env;
    }
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

void dump_object2(object *op)
{
    char   *cp;
    /*  object *tmp;*/

    if (op->arch != NULL)
    {
        strcat(errmsg, "arch ");
        strcat(errmsg, op->arch->name ? op->arch->name : "(null)");
        strcat(errmsg, "\n");
        sprintf(errmsg, "%scount %d\n", errmsg, op->count);
        if ((cp = get_ob_diff(op, &empty_archetype->clone)) != NULL)
            strcat(errmsg, cp);
#if 0
      /* Don't dump player diffs - they are too long, mostly meaningless, and
       * will overflow the buffer.
       * Changed so that we don't dump inventory either.  This may
       * also overflow the buffer.
       */
      if(op->type!=PLAYER && (cp=get_ob_diff(op,&empty_archetype->clone))!=NULL)
        strcat(errmsg,cp);
      for (tmp=op->inv; tmp; tmp=tmp->below)
        dump_object2(tmp);
#endif
        strcat(errmsg, "end\n");
    }
    else
    {
        strcat(errmsg, "Object ");
        if (op->name == NULL)
            strcat(errmsg, "(null)");
        else
            strcat(errmsg, op->name);
        strcat(errmsg, "\n");
#if 0
      if((cp=get_ob_diff(op,&empty_archetype->clone))!=NULL)
        strcat(errmsg,cp);
      for (tmp=op->inv; tmp; tmp=tmp->below)
        dump_object2(tmp);
#endif
        strcat(errmsg, "end\n");
    }
}

/*
 * Dumps an object.  Returns output in the static global errmsg array.
 */

void dump_object(object *op)
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
void dump_me(object *op, char *outstr)
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
        strcat(outstr, "arch ");
        strcat(outstr, op->arch->name ? op->arch->name : "(null)");
        strcat(outstr, "\n");
        if ((cp = get_ob_diff(op, &empty_archetype->clone)) != NULL)
            strcat(outstr, cp);
        strcat(outstr, "end\n");
    }
    else
    {
        strcat(outstr, "Object ");
        if (op->name == NULL)
            strcat(outstr, "(null)");
        else
            strcat(outstr, op->name);
        strcat(outstr, "\n");
        strcat(outstr, "end\n");
    }
}

/* Gecko: outdated dumping functions */
#if 0
/*
 * This is really verbose...Can be triggered by the P key while in DM mode.
 * All objects are dumped to stderr (or alternate logfile, if in server-mode)
 */

void dump_all_objects() {
  object *op;
  for(op=objects;op!=NULL;op=op->next) {
    dump_object(op);
    LOG(llevInfo, "Object %d\n:%s\n", op->count, errmsg);
  }
}

/*
 * get_nearest_part(multi-object, object 2) returns the part of the
 * multi-object 1 which is closest to the second object.
 * If it's not a multi-object, it is returned.
 */

/* OUTDATED !!!
object *get_nearest_part(object *op,object *pl) {

  object *tmp,*closest;
  int last_dist,i;
  if(op->more==NULL)
    return op;
  for(last_dist=distance(op,pl),closest=op,tmp=op->more;tmp!=NULL;tmp=tmp->more)
    if((i=distance(tmp,pl))<last_dist)
      closest=tmp,last_dist=i;
  return closest;
}
*/
#endif

/* Gecko: we could at least search through the active, friend and player lists here... */

/*
 * Returns the object which has the count-variable equal to the argument.
 */

object * find_object(int i)
{
    return NULL;
    /* object *op;

    for(op=objects;op!=NULL;op=op->next)
      if(op->count==(tag_t) i)
        break;
    return op;*/
}

/*
 * Returns the first object which has a name equal to the argument.
 * Used only by the patch command, but not all that useful.
 * Enables features like "patch <name-of-other-player> food 999"
 */

object * find_object_name(char *str)
{
    return NULL;

    /* if find_string() can't find the string -
     * then its impossible that op->name will match.
     * if we get a string - its the hash table
     * ptr for this string.
     */
    /*
      const char *name=find_string(str);
      object *op;

      if(name==NULL)
          return NULL;

      for(op=objects;op!=NULL;op=op->next)
      {
        if(op->name==name)
            break;
      }

      return op;*/
}

void free_all_object_data()
{
    LOG(llevDebug, "%d allocated objects, %d free objects\n", pool_object->nrof_allocated,
        pool_object->nrof_free);
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
void set_owner(object *op, object *owner)
{
    if (owner == NULL || op == NULL)
        return;
    set_owner_simple(op, owner);

    if (owner->type == PLAYER && owner->chosen_skill)
    {
        op->chosen_skill = owner->chosen_skill;
        op->exp_obj = owner->chosen_skill->exp_obj;
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
        op->exp_obj = clone->exp_obj;
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
void copy_object(object *op2, object *op)
{
    int is_removed  = QUERY_FLAG(op, FLAG_REMOVED);

	/* remove because perhaps speed will change when we copy */
    if(QUERY_FLAG(op, FLAG_IN_ACTIVELIST))
	    activelist_remove_inline(op);

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

    if (QUERY_FLAG(op, FLAG_IDENTIFIED))
    {
        SET_FLAG(op, FLAG_KNOWN_MAGICAL);
        SET_FLAG(op, FLAG_KNOWN_CURSED);
    }

    /* perhaps we have a custom treasurelist. Then we need to
     * increase the refcount here.
     */
    if (op->randomitems && (op->randomitems->flags & OBJLNK_FLAG_REF))
        op->randomitems->ref_count++;

    /* We set the custom_attrset pointer to NULL to avoid
     * really bad problems. TODO. this needs to be handled better
    * but it works until its only the player struct.
    */
    op->custom_attrset = NULL;

    /* Only alter speed_left when we sure we have not done it before */
    if (op->speed < 0 && op->speed_left == op->arch->clone.speed_left)
        op->speed_left += (RANDOM() % 90) / 100.0f;

	CLEAR_FLAG(op, FLAG_IN_ACTIVELIST); /* perhaps our source is on active list - ignore! */
    update_ob_speed(op);
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

    if (QUERY_FLAG(op, FLAG_IDENTIFIED))
    {
        SET_FLAG(op, FLAG_KNOWN_MAGICAL);
        SET_FLAG(op, FLAG_KNOWN_CURSED);
    }    
    
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

    /*LOG(-1, "update_object: %s (%d,%d) - action %x\n", op->name, op->x, op->y,action);*/
    if (op == NULL)
    {
        /* this should never happen */
        LOG(llevError, "ERROR: update_object() called for NULL object.\n");
        return;
    }

    if (op->env != NULL || !op->map || op->map->in_memory == MAP_SAVING)
        return;

    /* make sure the object is within map boundaries */
    /*
       if (op->x < 0 || op->x >= MAP_WIDTH(op->map) || op->y < 0 || op->y >= MAP_HEIGHT(op->map))
    {
           LOG(llevError,"ERROR: update_object() called for object out of map!\n");
        return;
       }
    */

    if (action == UP_OBJ_FACE) /* no need to change anything except the map update counter */
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_FACE - %s\n", query_name(op));
#endif
        INC_MAP_UPDATE_COUNTER(op->map, op->x, op->y);
        return;
    }

    msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y);
    newflags = msp->flags;
    flags = newflags & ~P_NEED_UPDATE;

    if (action == UP_OBJ_INSERT) /* always resort layer - but not always flags */
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_INS - %s\n", query_name(op));
#endif
        newflags |= P_NEED_UPDATE; /* force layer rebuild */
        msp->update_tile++;

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
    }
    else if (action == UP_OBJ_REMOVE)
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_REM - %s\n", query_name(op));
#endif
        newflags |= P_NEED_UPDATE; /* force layer rebuild */
        msp->update_tile++;

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
         || op->type == MAGIC_EAR)
            newflags |= P_FLAGS_UPDATE; /* force flags rebuild */
    }
    else if (action == UP_OBJ_FLAGS)
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_FLAGS - %s\n", query_name(op));
#endif
        newflags |= P_FLAGS_UPDATE; /* force flags rebuild but no tile counter*/
    }
    else if (action == UP_OBJ_FLAGFACE)
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_FLAGFACE - %s\n", query_name(op));
#endif
        newflags |= P_FLAGS_UPDATE; /* force flags rebuild */
        msp->update_tile++;
    }
    else if (action == UP_OBJ_LAYER)
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_LAYER - %s\n", query_name(op));
#endif
        newflags |= P_NEED_UPDATE; /* rebuild layers - most common when we change visibility of the object */
        msp->update_tile++;
    }
    else if (action == UP_OBJ_ALL)
    {
#ifdef DEBUG_CORE
        LOG(llevDebug, "UO_ALL - %s\n", query_name(op));
#endif
        newflags |= (P_FLAGS_UPDATE | P_NEED_UPDATE); /* force full tile update */
        msp->update_tile++;
    }
    else
    {
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
        LOG(llevBug, "BUG: drop_ob_inv() - try to drop items of %s\n", ob->name);
        return;
    }

    if (ob->env == NULL && (ob->map == NULL || ob->map->in_memory != MAP_IN_MEMORY))
    {
        /* TODO */
        LOG(llevDebug, "BUG: drop_ob_inv() - can't drop inventory of objects not in map yet: %s (%x)\n", ob->name,
            ob->map);
        return;
    }

    if(ob->enemy && ob->enemy->type == PLAYER && ob->enemy_count == ob->enemy->count)
        pl = CONTR(ob->enemy);

    /* create race corpse and/or drop stuff to floor */
    if ((QUERY_FLAG(ob, FLAG_CORPSE) && !QUERY_FLAG(ob, FLAG_STARTEQUIP)) || QUERY_FLAG(ob, FLAG_CORPSE_FORCED))
    {
        racelink   *race_corpse = find_racelink(ob->race);
        if (race_corpse)
        {
            corpse = arch_to_object(race_corpse->corpse);
            corpse->x = ob->x;corpse->y = ob->y;corpse->map = ob->map;
            corpse->weight = ob->weight;
        }
    }

    tmp_op = ob->inv;
    while (tmp_op != NULL)
    {
        tmp = tmp_op->below;
        remove_ob(tmp_op); /* Inv-no check off / This will be destroyed in next loop of object_gc() */
        /* if we recall spawn mobs, we don't want drop their items as free.
         * So, marking the mob itself with "FLAG_STARTEQUIP" will kill
         * all inventory and not dropping it on the map.
         * This also happens when a player slays a to low mob/non exp mob.
         * Don't drop any sys_object in inventory... I can't think about
         * any use... when we do it, a disease needle for example
         * is dropping his disease force and so on.
         */

        if(tmp_op->type==TYPE_QUEST_TRIGGER)
        {
            /* legal, non freed enemy */
            if (pl)
            {
                if(!(pl->group_status & GROUP_STATUS_GROUP))
                    insert_quest_item(tmp_op, pl->ob); /* single player */
                else
                {
                    for(gtmp=pl->group_leader;gtmp;gtmp=CONTR(gtmp)->group_next)
                    {
                        /* check for out of (kill) range */
                        if(!(CONTR(gtmp)->group_status & GROUP_STATUS_NOEXP))
                           insert_quest_item(tmp_op, gtmp); /* give it to member */
                    }
                }
            }
        }
        else if (!(QUERY_FLAG(ob, FLAG_STARTEQUIP)
                || (tmp_op->type != RUNE
                 && (QUERY_FLAG(tmp_op, FLAG_SYS_OBJECT)
                  || QUERY_FLAG(tmp_op, FLAG_STARTEQUIP)
                  || QUERY_FLAG(tmp_op, FLAG_NO_DROP)))))
        {
            tmp_op->x = ob->x,tmp_op->y = ob->y;
            CLEAR_FLAG(tmp_op, FLAG_APPLIED);

            /* if we have a corpse put the item in it */
            if (corpse)
                insert_ob_in_ob(tmp_op, corpse);
            else
            {
                /* don't drop traps from a container to the floor.
                 * removing the container where a trap is applied will
                 * neutralize the trap too
                * Also not drop it in env - be safe here
                 */
                if (tmp_op->type != RUNE)
                {
                    if (ob->env)
                    {
                        insert_ob_in_ob(tmp_op, ob->env);
                        /* this should handle in future insert_ob_in_ob() */
                        if (ob->env->type == PLAYER)
                            esrv_send_item(ob->env, tmp_op);
                        else if (ob->env->type == CONTAINER)
                            esrv_send_item(ob->env, tmp_op);
                    }
                    else
                        insert_ob_in_map(tmp_op, ob->map, NULL, 0); /* Insert in same map as the env*/
                }
            }
        }
        tmp_op = tmp;
    }

    if (corpse)
    {
        /* drop the corpse when something is in OR corpse_forced is set */
        /* i changed this to drop corpse always even they have no items
         * inside (player get confused when corpse don't drop. To avoid
         * clear corpses, change below "||corpse " to "|| corpse->inv"
         */
        if (QUERY_FLAG(ob, FLAG_CORPSE_FORCED) || corpse)
        {
            /* ok... we have a corpse AND we insert something in.
             * now check enemy and/or attacker to find a player.
             * if there is one - personlize this corpse container.
             * this gives the player the chance to grap this stuff first
             * - and looter will be stopped.
             */

            if (pl)
            {
                FREE_AND_ADD_REF_HASH(corpse->slaying, pl->ob->name);
            }
            else if (QUERY_FLAG(ob, FLAG_CORPSE_FORCED)) /* && no player */
            {
                /* normallly only player drop corpse. But in some cases
                 * npc can do it too. Then its smart to remove that corpse fast.
                 * It will not harm anything because we never deal for NPC with
                 * bounty.
                 */
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
                    corpse->sub_type1 = ST1_CONTAINER_CORPSE_player;
            }

            if (ob->env)
            {
                insert_ob_in_ob(corpse, ob->env);
                /* this should handle in future insert_ob_in_ob() */
                if (ob->env->type == PLAYER)
                    esrv_send_item(ob->env, corpse);
                else if (ob->env->type == CONTAINER)
                    esrv_send_item(ob->env, corpse);
            }
            else
                insert_ob_in_map(corpse, ob->map, NULL, 0);
        }
        else /* disabled */
        {
            /* if we are here, our corpse mob had something in inv but its nothing to drop */
            if (!QUERY_FLAG(corpse, FLAG_REMOVED))
                remove_ob(corpse); /* no check off - not put in the map here */
        }
    }
}

/*
 * destroy_object() frees everything allocated by an object, removes
 * it from the list of used objects, and puts it on the list of
 * free objects. This function is called automatically to free unused objects
 * (it is called from return_poolchunk() during garbage collection in object_gc() ).
 * The object must have been removed by remove_ob() first for
 * this function to succeed.
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

    /* This should be very rare... */
    if (QUERY_FLAG(ob, FLAG_IS_LINKED))
        remove_button_link(ob);

    activelist_remove_inline(ob);

    if (ob->type == CONTAINER && ob->attacked_by)
        container_unlink(NULL, ob);

    /* unlink old treasurelist if needed */
    if (ob->randomitems)
        unlink_treasurelists(ob->randomitems, 0);
    ob->randomitems = NULL;

    /* Make sure to get rid of the inventory, too. It will be destroy()ed at the next gc */
    /* TODO: maybe destroy() it here too? */
    remove_ob_inv(ob);

    /* Remove object from the active list */
//    ob->speed = 0;
//    update_ob_speed(ob);
    /*LOG(llevDebug,"FO: a:%s %x >%s< (#%d)\n", ob->arch?(ob->arch->name?ob->arch->name:""):"", ob->name, ob->name?ob->name:"",ob->name?query_refcount(ob->name):0);*/

    /* Free attached attrsets */
    if (ob->custom_attrset)
    {
        /*      LOG(llevDebug,"destroy_object() custom attrset found in object %s (type %d)\n",
                      STRING_OBJ_NAME(ob), ob->type);*/

#ifndef PRODUCTION_SYSTEM /* Avoid this check on performance-critical servers */
        if(ob->head) /* TODO: this shouldn't be compiled into a production server */
            LOG(llevDebug, "BUG: destroy_object() custom attrset found in object %s subpart (type %d)\n",
                    STRING_OBJ_NAME(ob), ob->type);
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
    if (op->inv)
        drop_ob_inv(op);
    remove_ob(op);
    check_walk_off(op, NULL, MOVE_APPLY_DEFAULT);

    /* Notify player that a pet has died */
    /* TODO: maybe this should be in kill_object() */
    if(op->type == MONSTER && OBJECT_VALID(op->owner, op->owner_count) && op->owner->type == PLAYER)
        new_draw_info_format(NDI_UNIQUE, 0, op->owner, "Your %s was killed", query_name(op));
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
    MapSpace   *msp;
    object     *otmp;

    if (QUERY_FLAG(op, FLAG_REMOVED))
    {
        /*dump_object(op)*/;
        LOG(llevBug, "BUG: Trying to remove removed object.:<%s> (<%d> %d %x) map:%s (%d,%d)\n", query_name(op),
				op->arch?STRING_SAFE(op->arch->name):"NOARCH", op->type, op->count, 
				op->map ? (op->map->path ? op->map->path : "op->map->path == NULL") : "op->map==NULL", op->x, op->y);
        return;
    }

    if (op->more != NULL)
        remove_ob(op->more); /* check off is handled outside here */

    mark_object_removed(op);
    SET_FLAG(op, FLAG_OBJECT_WAS_MOVED);

    /*
     * In this case, the object to be removed is in someones
     * inventory.
     */
    if (op->env != NULL)
    {
        /* this is not enough... when we for example remove money from a pouch
         * which is in a sack (which is itself in the player inv) then the weight
         * of the sack is not calculated right. This is only a temporary effect but
         * we need to fix it here a recursive ->env chain.
         */
        if(! QUERY_FLAG(op, FLAG_SYS_OBJECT))
        {
            if (op->nrof)
                sub_weight(op->env, op->weight * op->nrof);
            else
                sub_weight(op->env, op->weight + op->carrying);
        }

        /* NO_FIX_PLAYER is set when a great many changes are being
         * made to players inventory.  If set, avoiding the call to save cpu time.
         * the flag is set from outside... perhaps from a drop_all() function.
         */
        if ((otmp = is_player_inv(op->env)) != NULL && CONTR(otmp) && !QUERY_FLAG(otmp, FLAG_NO_FIX_PLAYER))
            fix_player(otmp);

        if (op->above != NULL)
            op->above->below = op->below;
        else
            op->env->inv = op->below;

        if (op->below != NULL)
            op->below->above = op->above;

        /* we set up values so that it could be inserted into
            * the map, but we don't actually do that - it is up
            * to the caller to decide what we want to do.
            */
        /* BUG: if we do this, active list for maps will fail!!
         * op->map set with a valid ptr will tell the server that this
         * object is on a map!! This is even without it a bug!
         */
        /*
        op->map = op->env->map;
        op->x = op->env->x,op->y = op->env->y;
        */

#ifdef POSITION_DEBUG
        op->ox = op->x,op->oy = op->y;
#endif

        op->above = NULL,op->below = NULL;
        op->map = NULL;
        op->env = NULL;
        return;
    }

    /* If we get here, we are removing it from a map */
    if (!op->map)
    {
        LOG(llevBug, "BUG: remove_ob(): object %s (%s) not on map or env.\n", query_short_name(op, NULL),
            op->arch ? (op->arch->name ? op->arch->name : "<nor arch name!>") : "<no arch!>");
        return;
    }

    /* lets first unlink this object from map*/

    /* if this is the base layer object, we assign the next object to be it if it is from same layer type */
    msp = GET_MAP_SPACE_PTR(op->map, op->x, op->y);

    if (op->layer)
    {
        if (GET_MAP_SPACE_LAYER(msp, op->layer - 1) == op)
        {
            /* well, don't kick the inv objects of this layer to normal layer */
            if (op->above && op->above->layer == op->layer && GET_MAP_SPACE_LAYER(msp, op->layer + 6) != op->above)
                SET_MAP_SPACE_LAYER(msp, op->layer - 1, op->above);
            else
                SET_MAP_SPACE_LAYER(msp, op->layer - 1, NULL);
        }
        else if (GET_MAP_SPACE_LAYER(msp, op->layer + 6) == op) /* inv layer? */
        {
            if (op->above && op->above->layer == op->layer)
                SET_MAP_SPACE_LAYER(msp, op->layer + 6, op->above);
            else
                SET_MAP_SPACE_LAYER(msp, op->layer + 6, NULL);
        }
    }

    /* link the object above us */
    if (op->above)
        op->above->below = op->below;
    else
        SET_MAP_SPACE_LAST(msp, op->below); /* assign below as last one */

    /* Relink the object below us, if there is one */
    if (op->below)
        op->below->above = op->above;
    else
        SET_MAP_SPACE_FIRST(msp, op->above);  /* first object goes on above it. */

    op->above = NULL;
    op->below = NULL;

    /* this is triggered when a map is swaped out and the objects on it get removed too */
    if (op->map->in_memory == MAP_SAVING)
        return;

    msp->update_tile++; /* we updated something here - mark this tile as changed! */

    /* some player only stuff.
     * we adjust the ->player map variable and the local map player chain.
     */
    if (op->type == PLAYER)
    {
        struct pl_player   *pltemp  = CONTR(op);

        /* now we remove us from the local map player chain */
        if (pltemp->map_below)
            CONTR(pltemp->map_below)->map_above = pltemp->map_above;
        else
            op->map->player_first = pltemp->map_above;

        if (pltemp->map_above)
            CONTR(pltemp->map_above)->map_below = pltemp->map_below;

        pltemp->map_below = pltemp->map_above = NULL;

        pltemp->update_los = 1; /* thats always true when touching the players map pos. */

        /* a open container NOT in our player inventory = unlink (close) when we move */
        if (pltemp->container && pltemp->container->env != op)
            container_unlink(pltemp, NULL);
    }

    update_object(op, UP_OBJ_REMOVE);

    op->env = NULL;
}

/* delete and remove recursive the inventory of an object.
 */
void remove_ob_inv(object *op)
{
    object *tmp, *tmp2;

    for (tmp = op->inv; tmp; tmp = tmp2)
    {
        tmp2 = tmp->below; /* save ptr, gets NULL in remove_ob */
        if (tmp->inv)
            remove_ob_inv(tmp);
        remove_ob(tmp);  /* no map, no check off */
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

object * insert_ob_in_map(object *op, mapstruct *m, object *originator, int flag)
{
    object*tmp =    NULL, *top;
    MapSpace       *mc;
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

    CLEAR_FLAG(op, FLAG_REMOVED);

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

    /* hm, i not checked this, but is it not smarter to remove op instead and return? MT */
    if (op->nrof && !(flag & INS_NO_MERGE))
    {
        for (tmp = GET_MAP_OB(m, x, y); tmp != NULL; tmp = tmp->above)
        {
            if (CAN_MERGE(op, tmp))
            {
                op->nrof += tmp->nrof;
                /* a bit tricky remove_ob() without check off.
                 * technically, this happens: arrow x/y is falling on the stack
                 * of perhaps 10 arrows. IF a teleporter is called, the whole 10
                 * arrows are teleported.Thats a right effect.
                 */
                remove_ob(tmp);
            }
        }
    }

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
    mc = GET_MAP_SPACE_PTR(op->map, op->x, op->y); /* for fast access - we will not change the node here */
    if (op->layer) /* so we have a non system object */
    {
        layer = op->layer - 1;
        layer_inv = layer + 7;
        if (!QUERY_FLAG(op, FLAG_IS_INVISIBLE)) /* not invisible? */
        {
            /* have we another object of this layer? */
            if ((top = GET_MAP_SPACE_LAYER(mc, layer)) == NULL && (top = GET_MAP_SPACE_LAYER(mc, layer_inv)) == NULL)
            {
                /* no, we are the first of this layer - lets search something above us we can chain with.*/
                for (lt = op->layer;
                     lt < MAX_ARCH_LAYERS
                  && (top = GET_MAP_SPACE_LAYER(mc, lt)) == NULL
                  && (top = GET_MAP_SPACE_LAYER(mc, lt + 7)) == NULL;
                     lt++)
                    ;
            }

            /* now, if top != NULL, thats the object BEFORE we want chain. This can be
                     * the base layer, the inv base layer or a object from a upper layer.
                     * If it NULL, we are the only object in this tile OR
                     * ->last holds the object BEFORE ours.
                     */
            SET_MAP_SPACE_LAYER(mc, layer, op); /* we always go in front */
            if (top) /* easy - we chain our object before this one */
            {
                if (top->below)
                    top->below->above = op;
                else /* if no object before ,we are new starting object */
                    SET_MAP_SPACE_FIRST(mc, op); /* first object */
                op->below = top->below;
                top->below = op;
                op->above = top;
            }
            else /* we are first object here or one is before us  - chain to it */
            {
                if ((top = GET_MAP_SPACE_LAST(mc)) != NULL)
                {
                    top->above = op;
                    op->below = top;
                }
                else /* a virgin! we set first and last object */
                    SET_MAP_SPACE_FIRST(mc, op); /* first object */

                SET_MAP_SPACE_LAST(mc, op); /* last object */
            }
        }
        else /* invisible object */
        {
            int tmp_flag;

            /* check the layers */
            if ((top = GET_MAP_SPACE_LAYER(mc, layer_inv)) != NULL)
                tmp_flag = 1;
            else if ((top = GET_MAP_SPACE_LAYER(mc, layer)) != NULL)
            {
                /* in this case, we have 1 or more normal objects in this layer,
                         * so we must skip all of them. easiest way is to get a upper layer
                         * valid object.
                         */
                for (lt = op->layer;
                     lt < MAX_ARCH_LAYERS
                  && (tmp = GET_MAP_SPACE_LAYER(mc, lt)) == NULL
                  && (tmp = GET_MAP_SPACE_LAYER(mc, lt + 7)) == NULL;
                     lt++)
                    ;
                tmp_flag = 2;
            }
            else
            {
                /* no, we are the first of this layer - lets search something above us we can chain with.*/
                for (lt = op->layer;
                     lt < MAX_ARCH_LAYERS
                  && (top = GET_MAP_SPACE_LAYER(mc, lt)) == NULL
                  && (top = GET_MAP_SPACE_LAYER(mc, lt + 7)) == NULL;
                     lt++)
                    ;
                tmp_flag = 3;
            }

            SET_MAP_SPACE_LAYER(mc, layer_inv, op); /* in all cases, we are the new inv base layer */
            if (top) /* easy - we chain our object before this one - well perhaps */
            {
                /* ok, now the tricky part.
                         * if top is set, this can be...
                         * - the inv layer of same layer id (and tmp_flag will be 1)
                         * - the normal layer of same layer id (tmp_flag == 2)
                         * - the inv OR normal layer of a upper layer (tmp_flag == 3)
                         * if tmp_flag = 1, its easy - just we get in front of top and use
                         * the same links.
                         * if tmp_flag = 2 AND tmp is set, tmp is the object we chain before.
                         * is tmp is NULL, we get ->last and chain after it.
                         * if tmp_flag = 3, we chain all times to top (before).
                         */

                if (tmp_flag == 2)
                {
                    if (tmp)
                    {
                        tmp->below->above = op; /* we can't be first, there is always one before us */
                        op->below = tmp->below;
                        tmp->below = op; /* and one after us... */
                        op->above = tmp;
                    }
                    else
                    {
                        tmp = GET_MAP_SPACE_LAST(mc); /* there is one before us, so this is always valid */
                        SET_MAP_SPACE_LAST(mc, op); /* new last object */
                        op->below = tmp;
                        tmp->above = op;
                    }
                }
                else /* tmp_flag 1 & tmp_flag 3 are done the same way */
                {
                    if (top->below)
                        top->below->above = op;
                    else /* if no object before ,we are new starting object */
                        SET_MAP_SPACE_FIRST(mc, op); /* first object */
                    op->below = top->below;
                    top->below = op;
                    op->above = top;
                }
            }
            else /* we are first object here or one is before us  - chain to it */
            {
                if ((top = GET_MAP_SPACE_LAST(mc)) != NULL) /* there is something down we don't care what */
                {
                    top->above = op; /* just chain to it */
                    op->below = top;
                }
                else /* a virgin! we set first and last object */
                    SET_MAP_SPACE_FIRST(mc, op); /* first object */

                SET_MAP_SPACE_LAST(mc, op); /* last object */
            }
        }
    }
    else /* op->layer == 0 - lets just put this object in front of all others */
    {
        /* is there some else? */
        if ((top = GET_MAP_SPACE_FIRST(mc)) != NULL)
        {
            /* easy chaining */
            top->below = op;
            op->above = top;
        }
        else /* no ,we are last object too */
            SET_MAP_SPACE_LAST(mc, op); /* last object */

        SET_MAP_SPACE_FIRST(mc, op); /* first object */
    }


    /* lets set some specials for our players
     * we adjust the ->player map variable and the local
     * map player chain.
     */
    if (op->type == PLAYER)
    {
        /* Bug #000120, Make sure we have a valid player object here */
        if(!CONTR(op))
        {
            LOG(llevBug, "BUG: Player object %s without controller in map %s\n",
                    STRING_OBJ_NAME(op), STRING_MAP_PATH(op->map));
            CLEAR_FLAG(op, FLAG_IS_PLAYER);
            op->type = MISC_OBJECT;
        }
        else
        {
            CONTR(op)->socket.update_tile = 0;
            CONTR(op)->update_los = 1; /* thats always true when touching the players map pos. */

            if (op->map->player_first)
            {
                CONTR(op->map->player_first)->map_below = op;
                CONTR(op)->map_above = op->map->player_first;
            }
            op->map->player_first = op;
        }
    }

	if(!(op->map->map_flags & MAP_FLAG_NO_UPDATE))
	{
	    mc->update_tile++; /* we updated something here - mark this tile as changed! */
		/* updates flags (blocked, alive, no magic, etc) for this map space */
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
    if (!(flag & INS_NO_WALK_ON) && (mc->flags & (P_WALK_ON | P_FLY_ON) || op->more) && !op->head)
    {
        int event;

        /* we want reuse mc here... bad enough we need to check it double for multi arch */
        if (QUERY_FLAG(op, FLAG_FLY_ON))
        {
            if (!(mc->flags & P_FLY_ON)) /* we are flying but no fly event here */
                goto check_walk_loop;
        }
        else /* we are not flying - check walking only */
        {
            if (!(mc->flags & P_WALK_ON))
                goto check_walk_loop;
        }

        if ((event = check_walk_on(op, originator, MOVE_APPLY_MOVE)))
        {
            if (event == CHECK_WALK_MOVED)
                return op; /* don't return NULL - we are valid but we was moved */
            else
                return NULL; /* CHECK_WALK_DESTROYED */
        }

        /* TODO: check event */

        check_walk_loop:
        for (tmp = op->more; tmp != NULL; tmp = tmp->more)
        {
            mc = GET_MAP_SPACE_PTR(tmp->map, tmp->x, tmp->y);

            /* object is flying/levitating */
            if (QUERY_FLAG(op, FLAG_FLY_ON))  /* trick: op is single tile OR always head! */
            {
                if (!(mc->flags & P_FLY_ON)) /* we are flying but no fly event here */
                    continue;
            }
            else /* we are not flying - check walking only */
            {
                if (!(mc->flags & P_WALK_ON))
                    continue;
            }

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
    object *tmp;
    object *tmp1;

    /* first search for itself and remove any old instances */

    for (tmp = GET_MAP_OB(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above)
    {
        if (!strcmp(tmp->arch->name, arch_string)) /* same archetype */
        {
            remove_ob(tmp); /* no move off here... should be ok, this is a technical function */
        }
    }

    tmp1 = arch_to_object(find_archetype(arch_string));


    tmp1->x = op->x; tmp1->y = op->y;
    insert_ob_in_map(tmp1, op->map, op, 0);
}

/*
 * get_split_ob(ob,nr) splits up ob into two parts.  The part which
 * is returned contains nr objects, and the remaining parts contains
 * the rest (or is removed and freed if that number is 0).
 * On failure, NULL is returned, and the reason put into the
 * global static errmsg array.
 */

object * get_split_ob(object *orig_ob, int nr)
{
    object *newob;
    object *tmp, *event;
    int     is_removed  = (QUERY_FLAG(orig_ob, FLAG_REMOVED) != 0);

    if ((int) orig_ob->nrof < nr)
    {
        sprintf(errmsg, "There are only %d %ss.", orig_ob->nrof ? orig_ob->nrof : 1, query_name(orig_ob));
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
    /*    if(QUERY_FLAG(orig_ob, FLAG_UNPAID) && QUERY_FLAG(orig_ob, FLAG_NO_PICK))*/
    /*      ;*/ /* clone objects .... */
    /*  else*/
    orig_ob->nrof -= nr;

    if (orig_ob->nrof < 1)
    {
        if (!is_removed)
            remove_ob(orig_ob);
        check_walk_off(orig_ob, NULL, MOVE_APPLY_VANISHED);
    }
    else if (!is_removed)
    {
        if (orig_ob->env != NULL && !QUERY_FLAG(orig_ob, FLAG_SYS_OBJECT))
            sub_weight(orig_ob->env, orig_ob->weight * nr);
        if (orig_ob->env == NULL && orig_ob->map->in_memory != MAP_IN_MEMORY)
        {
            strcpy(errmsg, "Tried to split object whose map is not in memory.");
            LOG(llevDebug, "Error, Tried to split object whose map is not in memory.\n");
            return NULL;
        }
    }

    newob->nrof = nr;
    return newob;
}

/*
 * decrease_ob_nr(object, number) decreases a specified number from
 * the amount of an object.  If the amount reaches 0, the object
 * is subsequently removed and freed.
 *
 * Return value: 'op' if something is left, NULL if the amount reached 0
 */

object * decrease_ob_nr(object *op, int i)
{
    object *tmp;

    if (i == 0)   /* objects with op->nrof require this check */
        return op;

    if (i > (int) op->nrof)
        i = (int) op->nrof;

    if (QUERY_FLAG(op, FLAG_REMOVED))
    {
        op->nrof -= i;
    }
    else if (op->env && OBJECT_ACTIVE(op->env))
    {
        /* is this object in the players inventory? */
        if (!(tmp = is_player_inv(op->env)))
        {
            /* No... But perhaps tmp is in a container viewed by a player?
                     * Lets check the container is linked to a player.
                     */
            if (op->env->type == CONTAINER
             && op->env->attacked_by
             && CONTR(op->env->attacked_by)
             && CONTR(op->env->attacked_by)->container == op->env)
                tmp = op->env->attacked_by;
        }

        if (i < (int) op->nrof) /* there are still some */
        {
            if(! QUERY_FLAG(op, FLAG_SYS_OBJECT))
                sub_weight(op->env, op->weight * i);
            op->nrof -= i;
            if (tmp)
            {
                esrv_send_item(tmp, op);
                esrv_update_item(UPD_WEIGHT, tmp, tmp);
            }
        }
        else /* we removed all! */
        {
            op->nrof = 0;
            if (tmp)
            {
                if (tmp->type != CONTAINER)
                    esrv_del_item(CONTR(tmp), op->count, op->env);
                else
                    esrv_del_item(NULL, op->count, op->env);
            }
            remove_ob(op);
        }
    }
    else
    {
        object *above   = op->above;

        if (i < (int) op->nrof)
        {
            op->nrof -= i;
        }
        else
        {
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
            op->nrof = 0;
        }
        /* Since we just removed op, op->above is null */
        for (tmp = above; tmp != NULL; tmp = tmp->above)
        {
            if (tmp->type == PLAYER)
            {
                if (op->nrof)
                    esrv_send_item(tmp, op);
                else
                    esrv_del_item(CONTR(tmp), op->count, op->env);
            }
        }
    }

    if (op->nrof)
        return op;
    else
        return NULL;
}

/*
 * insert_ob_in_ob(op,environment):
 *   This function inserts the object op in the linked list
 *   inside the object environment.
 *
 * Eneq(@csd.uu.se): Altered insert_ob_in_ob to make things picked up enter
 * the inventory at the last position or next to other objects of the same
 * type.
 * Frank: Now sorted by type, archetype and magic!
 *
 * The function returns now pointer to inserted item, and return value can
 * be != op, if items are merged. -Tero
 */

object * insert_ob_in_ob(object *op, object *where)
{
    object *tmp, *otmp;
    mapstruct *old_map = op->map;

    if (!QUERY_FLAG(op, FLAG_REMOVED))
    {
        dump_object(op);
        LOG(llevBug, "BUG: Trying to insert (ob) inserted object.\n%s\n", errmsg);
        return op;
    }
    if (where == NULL)
    {
        dump_object(op);
        LOG(llevBug, "BUG: Trying to put object in NULL.\n%s\n", errmsg);
        return op;
    }
    if (where->head)
    {
        LOG(llevBug, "BUG: Tried to insert object wrong part of multipart object.\n");
        where = where->head;
    }
    if (op->more)
    {
        LOG(llevError, "ERROR: Tried to insert multipart object %s (%d)\n", query_name(op), op->count);
        return op;
    }

    CLEAR_FLAG(op, FLAG_REMOVED);

    if (op->nrof)
    {
        for (tmp = where->inv; tmp != NULL; tmp = tmp->below)
            if (CAN_MERGE(tmp, op))
            {
                /* return the original object and remove inserted object
                          (client needs the original object) */
                tmp->nrof += op->nrof;

                /* Weight handling gets pretty funky.  Since we are adding to
                 * tmp->nrof, we need to increase the weight.
                 */
                if(! QUERY_FLAG(op, FLAG_SYS_OBJECT))
                    add_weight(where, op->weight * op->nrof);
                /* FIXME: is the weight added twice for these objects? 
                 * Both here and below? - Gecko 20050806 */

                /* Make sure we get rid of the old object */
                SET_FLAG(op, FLAG_REMOVED);

                op = tmp;
                remove_ob(op); /* and fix old object's links (we will insert it further down)*/
                CLEAR_FLAG(op, FLAG_REMOVED); /* Just kidding about previous remove */
                break;
            }


        /* I assume stackable objects have no inventory
         * We add the weight - this object could have just been removed
         * (if it was possible to merge).  calling remove_ob will subtract
         * the weight, so we need to add it in again, since we actually do
         * the linking below
         */
        if(! QUERY_FLAG(op, FLAG_SYS_OBJECT))
            add_weight(where, op->weight * op->nrof);
    }
    else if(! QUERY_FLAG(op, FLAG_SYS_OBJECT))
        add_weight(where, (op->weight + op->carrying));

    SET_FLAG(op, FLAG_OBJECT_WAS_MOVED);
    op->map = NULL;
    op->env = where;
    op->above = NULL;
    op->below = NULL;
    //  op->x=0,op->y=0; XXX clearing these values makes them impossible
    //  to use for home_x and home_y in base_info objects
#ifdef POSITION_DEBUG
    op->ox = 0,op->oy = 0;
#endif

    /* See if op moved between maps/containers */
    if(op->speed && op->map != old_map)
    {
        // LOG(llevDebug, "Object moved between maps: %s (%s -> %s)\n", STRING_OBJ_NAME(op), STRING_MAP_PATH(old_map), STRING_MAP_PATH(op->map));
        activelist_remove_inline(op);
    } 

    /* Client has no idea of ordering so lets not bother ordering it here.
     * It sure simplifies this function...
     */
    if (where->inv == NULL)
        where->inv = op;
    else
    {
        op->below = where->inv;
        op->below->above = op;
        where->inv = op;
    }

    /* check for event object and set the owner object
     * event flags.
     */
    if (op->type == TYPE_EVENT_OBJECT && op->sub_type1)
        where->event_flags |= (1U << (op->sub_type1 - 1));
    else if(op->type == TYPE_QUEST_TRIGGER && op->sub_type1 == ST1_QUEST_TRIGGER_CONT && where->type == CONTAINER)
        where->event_flags |= EVENT_FLAG_SPECIAL_QUEST;

    /* if player, adjust one drop items and fix player if not
     * marked as no fix.
     */
    otmp = is_player_inv(where);
    if (otmp && CONTR(otmp) != NULL)
    {
        if (!QUERY_FLAG(otmp, FLAG_NO_FIX_PLAYER))
            fix_player(otmp);
    }

    /* Make sure the object is on an activelist if needed */
    update_ob_speed(op);

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

int check_walk_on(object *op, object *originator, int flags)
{
    object     *tmp /*, *head=op->head?op->head:op*/;
    int         local_walk_semaphore    = FALSE; /* when TRUE, this function is root call for static_walk_semaphore setting */
    tag_t       tag;
    int fly;

    if (QUERY_FLAG(op, FLAG_NO_APPLY))
        return 0;

    fly = (QUERY_FLAG(op, FLAG_FLYING)|QUERY_FLAG(op, FLAG_LEVITATE));
    if (fly)
        flags |= MOVE_APPLY_FLY_ON;
    else
        flags |= MOVE_APPLY_WALK_ON;
    tag = op->count;

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

    for (tmp = GET_MAP_OB(op->map, op->x, op->y); tmp != NULL; tmp = tmp->above)
    {
        if (tmp == op)
            continue;    /* Can't apply yourself */

        if (fly ? QUERY_FLAG(tmp, FLAG_FLY_ON) : QUERY_FLAG(tmp, FLAG_WALK_ON))
        {
            move_apply(tmp, op, originator, flags); /* apply_func must handle multi arch parts....
                                                                  * NOTE: move_apply() can be heavy recursive and recall
                                                                  * this function too.*/

            if (was_destroyed(op, tag)) /* this means we got killed, removed or whatever! */
            {
                if (local_walk_semaphore)
                    static_walk_semaphore = FALSE;
                return CHECK_WALK_DESTROYED;
            }

            /* and here a remove_ob() or insert_xx() was triggered - we MUST stop now */
            if (QUERY_FLAG(op, FLAG_OBJECT_WAS_MOVED))
            {
                if (local_walk_semaphore)
                    static_walk_semaphore = FALSE;
                return CHECK_WALK_MOVED;
            }
        }
    }
    if (local_walk_semaphore)
        static_walk_semaphore = FALSE;
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
    fly = (QUERY_FLAG(op, FLAG_FLYING)|QUERY_FLAG(op, FLAG_LEVITATE));
    if (fly)
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
 * set_cheat(object) sets the cheat flag (WAS_WIZ) in the object and in
 * all it's inventory (recursively).
 * If checksums are used, a player will get set_cheat called for
 * him/her-self and all object carried by a call to this function.
 */

void set_cheat(object *op)
{
    /*
     object *tmp;
     if(op->inv)
       for(tmp = op->inv; tmp != NULL; tmp = tmp->below)
         set_cheat(tmp);
    */
}

/*
 * find_free_spot(archetype, map, x, y, start, stop) will search for
 * a spot at the given map and coordinates which will be able to contain
 * the given archetype.  start and stop specifies how many squares
 * to search (see the freearr_x/y[] definition).
 * It returns a random choice among the alternatives found.
 * start and stop are where to start relative to the free_arr array (1,9
 * does all 4 immediate directions).  This returns the index into the
 * array of the free spot, -1 if no spot available (dir 0 = x,y)
 * Note - this only checks to see if there is space for the head of the
 * object - if it is a multispace object, this should be called for all
 * pieces.
 */

int find_free_spot(archetype *at, mapstruct *m, int x, int y, int start, int stop)
{
    int         i, index = 0;
    static int  altern[SIZEOFFREE];
    for (i = start; i < stop; i++)
    {
        if (!arch_blocked(at, NULL, m, x + freearr_x[i], y + freearr_y[i]))
            altern[index++] = i;
        else if (wall(m, x + freearr_x[i], y + freearr_y[i]) && maxfree[i] < stop)
            stop = maxfree[i];
    }
    if (!index)
        return -1;
    return altern[RANDOM() % index];
}

/*
 * find_first_free_spot(archetype, mapstruct, x, y) works like
 * find_free_spot(), but it will search max number of squares.
 * But it will return the first available spot, not a random choice.
 * Changed 0.93.2: Have it return -1 if there is no free spot available.
 */

int find_first_free_spot(archetype *at, mapstruct *m, int x, int y)
{
    int i;
    for (i = 0; i < SIZEOFFREE; i++)
    {
        if (!arch_blocked(at, NULL, m, x + freearr_x[i], y + freearr_y[i]))
            return i;
    }
    return -1;
}

int find_first_free_spot2(archetype *at, mapstruct *m, int x, int y, int start, int range)
{
    int i;
    for (i = start; i < range; i++)
    {
        if (!arch_blocked(at, NULL, m, x + freearr_x[i], y + freearr_y[i]))
            return i;
    }
    return -1;
}
/*
 * find_dir(map, x, y, exclude) will search some close squares in the
 * given map at the given coordinates for live objects.
 * It will not considered the object given as exlude among possible
 * live objects.
 * It returns the direction toward the first/closest live object if finds
 * any, otherwise 0.
 */

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

/*
 * find_dir_2(delta-x,delta-y) will return a direction in which
 * an object which has subtracted the x and y coordinates of another
 * object, needs to travel toward it.
 */

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

/*
 * absdir(int): Returns a number between 1 and 8, which represent
 * the "absolute" direction of a number (it actually takes care of
 * "overflow" in previous calculations of a direction).
 */

int absdir(int d)
{
    while (d < 1)
        d += 8;
    while (d > 8)
        d -= 8;
    return d;
}

/*
 * dirdiff(dir1, dir2) returns how many 45-degrees differences there is
 * between two directions (which are expected to be absolute (see absdir())
 */

int dirdiff(int dir1, int dir2)
{
    int d;
    d = abs(dir1 - dir2);
    if (d > 4)
        d = 8 - d;
    return d;
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

int can_pick(object *who, object *item)
{
    return ((who->type == PLAYER && QUERY_FLAG(item, FLAG_NO_PICK) && QUERY_FLAG(item, FLAG_UNPAID))
        ||  /* iam not sure about that weight >0... */
      (item->weight > 0
    && !QUERY_FLAG(item, FLAG_NO_PICK)
    && (!IS_INVISIBLE(item, who) || QUERY_FLAG(who, FLAG_SEE_INVISIBLE))
    && (who->type == PLAYER || item->weight < who->weight / 3)));
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

int was_destroyed(object *op, tag_t old_tag)
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
object * load_object_str(char *obstr)
{
    object *op;
    FILE   *tempfile;
    void   *mybuffer;
    char    filename[MAX_BUF];
    sprintf(filename, "%s/cfloadobstr2044", settings.tmpdir);
    tempfile = fopen(filename, "w+");
    if (tempfile == NULL)
    {
        LOG(llevError, "ERROR: load_object_str(): Unable to access load object temp file\n");
        return NULL;
    };
    fprintf(tempfile, obstr);

    op = get_object();

    rewind(tempfile);
    mybuffer = create_loader_buffer(tempfile);
    load_object(tempfile, op, mybuffer, LO_REPEAT, 0);
    delete_loader_buffer(mybuffer);
    LOG(llevDebug, " load str completed, object=%s\n", query_name(op));
    fclose(tempfile);
    return op;
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

              if (QUERY_FLAG(tmp, FLAG_CURSED) || QUERY_FLAG(tmp, FLAG_DAMNED))
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
