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
/* Eneq(@csd.uu.se): Added weight-modifiers in environment of objects.
   sub/add_weight will transcend the environment updating the carrying
   variable. */
#include <global.h>
#ifndef WIN32 /* ---win32 exclude headers */
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#endif /* win32 */

#include <funcpoint.h>
#include <skillist.h>
#include <loader.h>

#ifdef MEMORY_DEBUG
    int nroffreeobjects = 0;
    int nrofallocobjects = 0;
    #undef OBJ_EXPAND
    #define OBJ_EXPAND 1
#else
    object objarray[STARTMAX]; /* All objects, allocated this way at first */
    int nroffreeobjects = STARTMAX;  /* How many OBs allocated and free (free) */
    int nrofallocobjects = STARTMAX; /* How many OBs allocated (free + used) */
#endif

object *objects;           /* Pointer to the list of used objects */
object *free_objects;      /* Pointer to the list of unused objects */
object *active_objects;	/* List of active objects that need to be processed */


int freearr_x[SIZEOFFREE]=
  {0,0,1,1,1,0,-1,-1,-1,0,1,2,2,2,2,2,1,0,-1,-2,-2,-2,-2,-2,-1,
   0,1,2,3,3,3,3,3,3,3,2,1,0,-1,-2,-3,-3,-3,-3,-3,-3,-3,-2,-1};
int freearr_y[SIZEOFFREE]=
  {0,-1,-1,0,1,1,1,0,-1,-2,-2,-2,-1,0,1,2,2,2,2,2,1,0,-1,-2,-2,
   -3,-3,-3,-3,-2,-1,0,1,2,3,3,3,3,3,3,3,2,1,0,-1,-2,-3,-3,-3};
int maxfree[SIZEOFFREE]=
  {0,9,10,13,14,17,18,21,22,25,26,27,30,31,32,33,36,37,39,39,42,43,44,45,
  48,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49,49};
int freedir[SIZEOFFREE]= {
  0,1,2,3,4,5,6,7,8,1,2,2,2,3,4,4,4,5,6,6,6,7,8,8,8,
  1,2,2,2,2,2,3,4,4,4,4,4,5,6,6,6,6,6,7,8,8,8,8,8};


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

inline int CAN_MERGE(object *ob1, object *ob2) {

    /* A couple quicksanity checks */
    if ((ob1 == ob2) || (ob1->type != ob2->type) ||
        (ob1->material!=ob2->material) || (ob1->material_real!=ob2->material_real) ||
        (ob1->item_quality!=ob2->item_quality) ||(ob1->item_condition!=ob2->item_condition) ||
        (ob1->item_race!=ob2->item_race) ||(ob1->speed != ob2->speed)) 
        return 0;

    /* Note sure why the following is the case - either the object has to
     * be animated or have a very low speed.  Is this an attempted monster
     * check?
     */
    if (!QUERY_FLAG(ob1,FLAG_ANIMATE) && FABS((ob1)->speed) > MIN_ACTIVE_SPEED)
	return 0;


    /* If the objects have been identified, set the BEEN_APPLIED flag.
     * This is to the comparison of the flags below will be OK.  We
     * just can't ignore the been applied or identified flags, as they
     * are not equal - just if it has been identified, the been_applied
     * flags lose any meaning.
     */
    if (QUERY_FLAG(ob1, FLAG_IDENTIFIED))
	SET_FLAG(ob1, FLAG_BEEN_APPLIED);

    if (QUERY_FLAG(ob2, FLAG_IDENTIFIED))
	SET_FLAG(ob2, FLAG_BEEN_APPLIED);


    /* the 0x400000 on flags2 is FLAG_INV_LOCK.  I don't think something
     * being locked in inventory should prevent merging.
     * 0x4 in flags3 is CLIENT_SENT
     */
    if ((ob1->arch != ob2->arch) || 
	(ob1->stats.sp != ob2->stats.sp) ||
	(ob1->flags[0] != ob2->flags[0]) || 
	(ob1->flags[1] != ob2->flags[1]) ||
	((ob1->flags[2] & ~0x400000) != (ob2->flags[2] & ~ 0x400000)) ||
	((ob1->flags[3] & ~0x4) != (ob2->flags[3] & ~0x4)) || 
	(ob1->name != ob2->name) || 
	(ob1->title != ob2->title) ||
	(ob1->msg != ob2->msg) || 
	(ob1->weight != ob2->weight) ||
	(ob1->stats.food != ob2->stats.food) ||
	(memcmp(&ob1->resist, &ob2->resist, sizeof(ob1->resist))!=0) ||
	(ob1->attacktype != ob2->attacktype) ||
	(ob1->magic != ob2->magic) ||
	(ob1->slaying != ob2->slaying) ||
	(ob1->value != ob2->value) ||
	(ob1->animation_id != ob2->animation_id)
	) 
	    return 0;

    switch (ob1->type) {
	case SCROLL:
	    if (ob1->level != ob2->level) return 0;
	    break;

	case RING:
	    /* Don't merge applied rings - easier to keep them seperate, and
	     * it makes more sense (can easily unapply one ring).  Rings are
	     * the only objects that need this special code, as they are the
	     * only objects of the same type in which more than 1 can be
	     * applied at a time.
	     *
	     * Note - there is no break so we fall into the POTION/AMULET
	     * check below.
	     */
	    if (QUERY_FLAG(ob1, FLAG_APPLIED) || QUERY_FLAG(ob2, FLAG_APPLIED))
		return 0;

	case POTION:
	case AMULET:
	    /* This should compare the value of the stats, and not the pointer
	     * itself.  There can be cases were potions seem to loose their
	     * plus
	     */
	    if (memcmp(&ob1->stats,&ob2->stats, sizeof(living))) return 0;
	    break;
    }
    /* Everything passes, must be OK. */
    return 1;
}

/*
 * sum_weight() is a recursive function which calculates the weight
 * an object is carrying.  It goes through in figures out how much
 * containers are carrying, and sums it up.
 */
signed long sum_weight(object *op) {
  signed long sum;
  object *inv;
  for(sum = 0, inv = op->inv; inv != NULL; inv = inv->below) {
    if (inv->inv)
	sum_weight(inv);
    sum += inv->carrying + (inv->nrof ? inv->weight * inv->nrof : inv->weight);
  }
  if (op->type == CONTAINER && op->stats.Str)
    sum = (sum * (100 - op->stats.Str))/100;
  if(op->carrying != sum)
    op->carrying = sum;
  return sum;
}


/*
 * Eneq(@csd.uu.se): Since we can have items buried in a character we need
 * a better check.  We basically keeping traversing up until we can't
 * or find a player.
 */

object *is_player_inv (object *op) { 
    for (;op!=NULL&&op->type!=PLAYER; op=op->env)
      if (op->env==op)
	op->env = NULL;
    return op;
}

/*
 * Used by: Server DM commands: dumpbelow, dump.
 *	Some error messages.
 * The result of the dump is stored in the static global errmsg array.
 */

void dump_object2(object *op) {
  char *cp;
/*  object *tmp;*/

  if(op->arch!=NULL) {
      strcat(errmsg,"arch ");
      strcat(errmsg,op->arch->name?op->arch->name:"(null)");
      strcat(errmsg,"\n");
      if((cp=get_ob_diff(op,&empty_archetype->clone))!=NULL)
	strcat(errmsg,cp);
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
      strcat(errmsg,"end\n");
  } else {
      strcat(errmsg,"Object ");
      if (op->name==NULL) strcat(errmsg, "(null)");
      else strcat(errmsg,op->name);
      strcat(errmsg,"\n");
#if 0
      if((cp=get_ob_diff(op,&empty_archetype->clone))!=NULL)
        strcat(errmsg,cp);
      for (tmp=op->inv; tmp; tmp=tmp->below)
        dump_object2(tmp);
#endif
      strcat(errmsg,"end\n");
  }
}

/*
 * Dumps an object.  Returns output in the static global errmsg array.
 */

void dump_object(object *op) {
  if(op==NULL) {
    strcpy(errmsg,"[NULL pointer]");
    return;
  }
  errmsg[0]='\0';
  dump_object2(op);
}

/* GROS - Dumps an object. Return the result into a string                   */
/* Note that no checking is done for the validity of the target string, so   */
/* you need to be sure that you allocated enough space for it.               */
void dump_me(object *op, char *outstr)
{
    char *cp;

    if(op==NULL)
    {
        strcpy(outstr,"[NULL pointer]");
        return;
    }
    outstr[0]='\0';

    if(op->arch!=NULL)
    {
        strcat(outstr,"arch ");
        strcat(outstr,op->arch->name?op->arch->name:"(null)");
        strcat(outstr,"\n");
        if((cp=get_ob_diff(op,&empty_archetype->clone))!=NULL)
            strcat(outstr,cp);
        strcat(outstr,"end\n");
    }
    else
    {
        strcat(outstr,"Object ");
        if (op->name==NULL)
            strcat(outstr, "(null)");
        else
            strcat(outstr,op->name);
        strcat(outstr,"\n");
        strcat(outstr,"end\n");
    }
}

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

/*
 * Returns the object which has the count-variable equal to the argument.
 */

object *find_object(int i) {
  object *op;
  for(op=objects;op!=NULL;op=op->next)
    if(op->count==(tag_t) i)
      break;
 return op;
}

/*
 * Returns the first object which has a name equal to the argument.
 * Used only by the patch command, but not all that useful.
 * Enables features like "patch <name-of-other-player> food 999"
 */

object *find_object_name(char *str) {
  char *name=add_string(str);
  object *op;
  for(op=objects;op!=NULL;op=op->next)
    if(op->name==name)
      break;
  free_string(name);
  return op;
}

void free_all_object_data() {
#ifdef MEMORY_DEBUG
    object *op, *next;

    for (op=free_objects; op!=NULL; ) {
	next=op->next;
	free(op);
	nrofallocobjects--;
	nroffreeobjects--;
	op=next;
    }
#endif
    LOG(llevDebug,"%d allocated objects, %d free objects, STARMAX=%d\n", 
	nrofallocobjects, nroffreeobjects,STARTMAX);
}

/*
 * Returns the object which this object marks as being the owner.
 * A id-scheme is used to avoid pointing to objects which have been
 * freed and are now reused.  If this is detected, the owner is
 * set to NULL, and NULL is returned.
 * (This scheme should be changed to a refcount scheme in the future)
 */

object *get_owner(object *op) {
  if(op->owner==NULL)
    return NULL;
  if(!QUERY_FLAG(op->owner,FLAG_FREED) && op->owner->count==op->ownercount)
    return op->owner;
  op->owner=NULL,op->ownercount=0;
  return NULL;
}

void clear_owner(object *op)
{
    if (!op) return;

    if (op->owner && op->ownercount == op->owner->count)
	op->owner->refcount--;

    op->owner = NULL;
    op->ownercount = 0;
}


/*
 * Sets the owner of the first object to the second object.
 * Also checkpoints a backup id-scheme which detects freeing (and reusage)
 * of the owner object.
 * See also get_owner()
 */

static void set_owner_simple (object *op, object *owner)
{
    /* next line added to allow objects which own objects */ 
    /* Add a check for ownercounts in here, as I got into an endless loop
     * with the fireball owning a poison cloud which then owned the
     * fireball.  I believe that was caused by one of the objects getting
     * freed and then another object replacing it.  Since the ownercounts
     * didn't match, this check is valid and I believe that cause is valid.
     */
    while (owner->owner && owner!=owner->owner && 
	owner->ownercount==owner->owner->count) owner=owner->owner;

    /* IF the owner still has an owner, we did not resolve to a final owner.
     * so lets not add to that.
     */
    if (owner->owner) return;

    op->owner=owner;

    op->ownercount=owner->count;
    owner->refcount++;
}

static void set_skill_pointers (object *op, object *chosen_skill,
	object *exp_obj)
{
    op->chosen_skill = chosen_skill;
    op->exp_obj = exp_obj;

    /* unfortunately, we can't allow summoned monsters skill use
     * because we will need the chosen_skill field to pick the
     * right skill/stat modifiers for calc_skill_exp(). See
     * hit_player() in server/attack.c -b.t.
     */
    CLEAR_FLAG (op, FLAG_CAN_USE_SKILL);
    CLEAR_FLAG (op, FLAG_READY_SKILL);
}


/*
 * Sets the owner and sets the skill and exp pointers to owner's current
 * skill and experience objects.
 */
void set_owner (object *op, object *owner)
{
    if(owner==NULL||op==NULL)
	return;
    set_owner_simple (op, owner);

    if (owner->type == PLAYER && owner->chosen_skill)
        set_skill_pointers (op, owner->chosen_skill,
                            owner->chosen_skill->exp_obj);
    else if (op->type != PLAYER)
	CLEAR_FLAG (op, FLAG_READY_SKILL);
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
void copy_owner (object *op, object *clone)
{
    object *owner = get_owner (clone);
    if (owner == NULL) {
	/* players don't have owners - they own themselves.  Update
	 * as appropriate.
	 */
	if (clone->type == PLAYER) owner=clone;
	else return;
    }
    set_owner_simple (op, owner);

    if (clone->chosen_skill)
        set_skill_pointers (op, clone->chosen_skill, clone->exp_obj);
    else if (op->type != PLAYER)
	CLEAR_FLAG (op, FLAG_READY_SKILL);
}

/*
 * Resets vital variables in an object
 */

void reset_object(object *op) {
  int i;
  op->name = NULL;
  op->title = NULL;
  op->race = NULL;
  op->slaying = NULL;
  op->msg = NULL;
  for(i=0;i<NR_LOCAL_EVENTS;i++)
  {
    op->event_hook[i] = NULL;
    op->event_plugin[i] = NULL;
    op->event_options[i] = NULL;
  }  
  op->current_weapon_script = NULL;
  clear_object(op);
}
/*
 * clear_object() frees everything allocated by an object, and also
 * clears all variables and flags to default settings.
 */

void clear_object(object *op) {

    /* the memset will clear all these values for us, but we need
     * to reduce the refcount on them.
     */
    if(op->name!=NULL)
	free_string(op->name);
    if(op->title != NULL)
	free_string(op->title);
    if(op->race!=NULL)
	free_string(op->race);
    if(op->slaying!=NULL)
	free_string(op->slaying);
    if(op->msg!=NULL)
	free_string(op->msg);

    /* Using this memset is a lot easier (and probably faster)
     * than explicitly clearing the fields.
     */
    memset((void*)((char*)op + offsetof(object, magic)),
		   0, sizeof(object)-offsetof(object, magic));
     /* Below here, we clear things that are not done by the memset,
     * or set default values that are not zero.
     */

    /* This is more or less true */
    SET_FLAG(op, FLAG_REMOVED);

    op->contr = NULL;
    op->below=NULL;
    op->above=NULL;
    op->inv=NULL;
    op->container=NULL;
    op->env=NULL;
    op->more=NULL;
    op->head=NULL;
    op->map=NULL;
    op->refcount=0;
    /* What is not cleared is next, prev, active_next, active_prev, and count */

	op->anim_enemy_dir = -1;      /* control the facings 25 animations */
	op->anim_moving_dir = -1;     /* the same for movement */
	op->anim_enemy_dir_last = -1;
	op->anim_moving_dir_last = -1;
	op->anim_last_facing = 4;
	op->anim_last_facing_last = -1;

    op->face = blank_face;
    op->attacked_by_count= -1;
#ifdef CASTING_TIME
    op->casting = -1;
#endif
}

/*
 * copy object first frees everything allocated by the second object,
 * and then copies the contends of the first object into the second
 * object, allocating what needs to be allocated.
 */

void copy_object(object *op2, object *op) 
{
	int is_freed=QUERY_FLAG(op,FLAG_FREED),is_removed=QUERY_FLAG(op,FLAG_REMOVED); 

  if(op->name!=NULL)
    free_string(op->name);
  if(op->title!=NULL)
    free_string(op->title);
  if(op->race!=NULL)
    free_string(op->race);
  if(op->slaying!=NULL)
    free_string(op->slaying);
  if(op->msg!=NULL)
    free_string(op->msg);
  (void) memcpy((void *)((char *) op +offsetof(object,magic)),
                (void *)((char *) op2+offsetof(object,magic)),
                sizeof(object)-offsetof(object, magic));
  if(is_freed)
    SET_FLAG(op,FLAG_FREED);
  if(is_removed)
    SET_FLAG(op,FLAG_REMOVED);
  if(op->name!=NULL)
    add_refcount(op->name);
  if(op->title!=NULL)
    add_refcount(op->title);
  if(op->race!=NULL)
    add_refcount(op->race);
  if(op->slaying!=NULL)
    add_refcount(op->slaying);
  if(op->msg!=NULL)
    add_refcount(op->msg);

 	if(QUERY_FLAG(op,FLAG_IDENTIFIED))
	{
		SET_FLAG(op,FLAG_KNOWN_MAGICAL);
		SET_FLAG(op,FLAG_KNOWN_CURSED);
	}
 /* Only alter speed_left when we sure we have not done it before */
  if(op->speed<0 && op->speed_left == op->arch->clone.speed_left) 
	  op->speed_left+=(RANDOM()%90)/100.0f;
  update_ob_speed(op);
}

void copy_object_data(object *op2, object *op) 
{
	int is_freed=QUERY_FLAG(op,FLAG_FREED),is_removed=QUERY_FLAG(op,FLAG_REMOVED); 

  if(op->name!=NULL)
    free_string(op->name);
  if(op->title!=NULL)
    free_string(op->title);
  if(op->race!=NULL)
    free_string(op->race);
  if(op->slaying!=NULL)
    free_string(op->slaying);
  if(op->msg!=NULL)
    free_string(op->msg);
  (void) memcpy((void *)((char *) op +offsetof(object,magic)),
                (void *)((char *) op2+offsetof(object,magic)),
                sizeof(object)-offsetof(object, magic));
  if(is_freed)
    SET_FLAG(op,FLAG_FREED);
  if(is_removed)
    SET_FLAG(op,FLAG_REMOVED);
  if(op->name!=NULL)
    add_refcount(op->name);
  if(op->title!=NULL)
    add_refcount(op->title);
  if(op->race!=NULL)
    add_refcount(op->race);
  if(op->slaying!=NULL)
    add_refcount(op->slaying);
  if(op->msg!=NULL)
    add_refcount(op->msg);

	if(QUERY_FLAG(op,FLAG_IDENTIFIED))
	{
		SET_FLAG(op,FLAG_KNOWN_MAGICAL);
		SET_FLAG(op,FLAG_KNOWN_CURSED);
	}
}

/*
 * expand_objects() allocates more objects for the list of unused objects.
 * It is called from get_object() if the unused list is empty.
 */

void expand_objects() {
  int i;
  object *new;
  new = (object *) CALLOC(OBJ_EXPAND,sizeof(object));

  if(new==NULL)
	LOG(llevError,"ERROR: expand_objects(): OOM.\n");
  free_objects=new;
  new[0].prev=NULL;
  new[0].next= &new[1],
  SET_FLAG(&(new[0]), FLAG_REMOVED);
  SET_FLAG(&(new[0]), FLAG_FREED);

  for(i=1;i<OBJ_EXPAND-1;i++) {
    new[i].next= &new[i+1],
    new[i].prev= &new[i-1],
    SET_FLAG(&(new[i]), FLAG_REMOVED);
    SET_FLAG(&(new[i]), FLAG_FREED);
  }
  new[OBJ_EXPAND-1].prev= &new[OBJ_EXPAND-2],
  new[OBJ_EXPAND-1].next=NULL,
  SET_FLAG(&(new[OBJ_EXPAND-1]), FLAG_REMOVED);
  SET_FLAG(&(new[OBJ_EXPAND-1]), FLAG_FREED);

  nrofallocobjects += OBJ_EXPAND;
  nroffreeobjects += OBJ_EXPAND;
}

/*
 * get_object() grabs an object from the list of unused objects, makes
 * sure it is initialised, and returns it.
 * If there are no free objects, expand_objects() is called to get more.
 */

object *get_object() {
  object *op;

  if(free_objects==NULL) {
    expand_objects();
  }
  op=free_objects;
#ifdef MEMORY_DEBUG
  /* The idea is hopefully by doing a realloc, the memory
   * debugging program will now use the current stack trace to
   * report leaks.
   */
  op = realloc(op, sizeof(object));
  SET_FLAG(op, FLAG_REMOVED);
  SET_FLAG(op, FLAG_FREED);
#endif

  if(!QUERY_FLAG(op,FLAG_FREED))
    LOG(llevError,"ERROR: get_object: Getting busy object.\n");

  free_objects=op->next;
  if(free_objects!=NULL)
    free_objects->prev=NULL;
  op->count= ++ob_count;
  op->name=NULL;
  op->title=NULL;
  op->race=NULL;
  op->slaying=NULL;
  op->msg=NULL;
  op->next=objects;
  op->prev=NULL;
  op->active_next = NULL;
  op->active_prev = NULL;
  if(objects!=NULL)
    objects->prev=op;
  objects=op;
  clear_object(op);
  SET_FLAG(op,FLAG_REMOVED);
  nroffreeobjects--;
  return op;
}

/*
 * If an object with the IS_TURNABLE() flag needs to be turned due
 * to the closest player being on the other side, this function can
 * be called to update the face variable, _and_ how it looks on the map.
 */

void update_turn_face(object *op) {
    if(!QUERY_FLAG(op,FLAG_IS_TURNABLE)||op->arch==NULL)
	return;
	SET_ANIMATION(op, (NUM_ANIMATIONS(op)/NUM_FACINGS(op))*op->direction);
    update_object(op,UP_OBJ_FACE);
}

/*
 * Updates the speed of an object.  If the speed changes from 0 to another
 * value, or vice versa, then add/remove the object from the active list.
 * This function needs to be called whenever the speed of an object changes.
 */

void update_ob_speed(object *op) {
    extern int arch_init;

    /* No reason putting the archetypes objects on the speed list,
     * since they never really need to be updated.
     */

    if (QUERY_FLAG(op, FLAG_FREED) && op->speed) 
	{
		dump_object(op);
		LOG(llevBug,"BUG: Object %s is freed but has speed.\n:%s\n", op->name,errmsg);
		op->speed = 0;
    }
    if (arch_init) {
	return;
    }
    if (FABS(op->speed)>MIN_ACTIVE_SPEED) {
	/* If already on active list, don't do anything */
	if (op->active_next || op->active_prev || op==active_objects)
	    return;

        /* process_events() expects us to insert the object at the beginning
         * of the list. */
	op->active_next = active_objects;
	if (op->active_next!=NULL)
		op->active_next->active_prev = op;
	active_objects = op;
    }
    else {
	/* If not on the active list, nothing needs to be done */
	if (!op->active_next && !op->active_prev && op!=active_objects)
	    return;

	if (op->active_prev==NULL) {
	    active_objects = op->active_next;
	    if (op->active_next!=NULL)
		op->active_next->active_prev = NULL;
	}
	else {
	    op->active_prev->active_next = op->active_next;
	    if (op->active_next)
		op->active_next->active_prev = op->active_prev;
	}
	op->active_next = NULL;
	op->active_prev = NULL;
    }
}


/*
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

void update_object(object *op, int action) {
    int update_now=0, flags;
    
	/*LOG(-1, "update_object: %s (%d,%d) - action %x\n", op->name, op->x, op->y,action);*/
    if (op == NULL)
	{
        /* this should never happen */
        LOG(llevError,"ERROR: update_object() called for NULL object.\n");
		return;
    }
    
    if(op->env!=NULL) {
	/* Animation is currently handled by client, so nothing
	 * to do in this case.
	 */
	return;
    }

    /* If the map is saving, don't do anything as everything is
     * going to get freed anyways.
     */
    if (!op->map || op->map->in_memory == MAP_SAVING) return;
    
    /* make sure the object is within map boundaries */
#ifdef DEBUG
    if (op->x < 0 || op->x >= MAP_WIDTH(op->map) || op->y < 0 || op->y >= MAP_HEIGHT(op->map)) 
	{
        LOG(llevError,"ERROR: update_object() called for object out of map!\n");
		return;
    }
#endif 
	
    flags = GET_MAP_FLAGS(op->map, op->x, op->y);

    SET_MAP_FLAGS(op->map, op->x, op->y, flags | P_NEED_UPDATE);

    if (action == UP_OBJ_INSERT) {
		
		if(flags & P_SET_INV) /* we touch the inv/non inv layer system - force update */
			update_now=1;
        else if (QUERY_FLAG(op, FLAG_ALIVE) && !(flags & P_IS_ALIVE))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_IS_PLAYER) && !(flags & P_IS_PLAYER))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_NO_PASS) && !(flags & P_NO_PASS))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_BLOCKSVIEW) && !(flags & P_BLOCKSVIEW))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_NO_MAGIC) && !(flags & P_NO_MAGIC))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_DAMNED) && !(flags & P_NO_CLERIC))
            update_now=1;
		else if(op->type == CHECK_INV && !(flags & P_CHECK_INV))
            update_now=1;
		else if(op->type == MAGIC_EAR && !(flags & P_MAGIC_EAR))
            update_now=1;

    } else if (action == UP_OBJ_REMOVE) {
		if(flags & P_SET_INV) /* we touch the inv/non inv layer system - force update */
			update_now=1;
        else if (QUERY_FLAG(op, FLAG_ALIVE) && (flags & P_IS_ALIVE))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_IS_PLAYER) && (flags & P_IS_PLAYER))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_NO_PASS) && (flags & P_NO_PASS))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_BLOCKSVIEW) && (flags & P_BLOCKSVIEW))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_NO_MAGIC) && (flags & P_NO_MAGIC))
            update_now=1;
        else if (QUERY_FLAG(op, FLAG_DAMNED) && (flags & P_NO_CLERIC))
            update_now=1;
		else if(op->type == CHECK_INV && (flags & P_CHECK_INV))
            update_now=1;
		else if(op->type == MAGIC_EAR && (flags & P_MAGIC_EAR))
            update_now=1;

    } else if (action == UP_OBJ_CHANGE) {
	update_now=1;
    } else if (action == UP_OBJ_FACE) {
		if(flags & P_SET_INV) /* we touch the inv/non inv layer system - force update */
			update_now=1;
    } else if (action == UP_OBJ_INV) {
		if(op->head)	/* we autoupdate the tails of a multi tile object */
			SET_FLAG(op,QUERY_FLAG(op->head,FLAG_IS_INVISIBLE));
		remove_ob(op);
		insert_ob_in_map(op,op->map,NULL,0);
		update_now=1;
	}
    else {
	LOG(llevError,"ERROR: update_object called with invalid action: %d\n", action);
    }

    if (update_now) {
        SET_MAP_FLAGS(op->map, op->x, op->y, (flags | P_NO_ERROR | P_NEED_UPDATE));
        update_position(op->map, op->x, op->y);
    }

    if(op->more!=NULL)
		update_object(op->more, action);
}


/*
 * free_object() frees everything allocated by an object, removes
 * it from the list of used objects, and puts it on the list of
 * free objects.  The IS_FREED() flag is set in the object.
 * The object must have been removed by remove_ob() first for
 * this function to succeed.
 */

void free_object(object *ob) {
  object *tmp,*op;

    if (!QUERY_FLAG(ob,FLAG_REMOVED)) {
	dump_object(ob);
	LOG(llevBug,"BUG: Free object called with non removed object\n:%s\n",errmsg);
    }
  if(QUERY_FLAG(ob,FLAG_FRIENDLY)) {
	/* This is not a bug - if we not want add/remove it in remove_:ob() (we don't want)
	 * this here is the right place
	dump_object(ob);
    LOG(llevBug,"BUG: tried to free friendly object %s\n:%s\n",ob->name,errmsg);
	*/
    remove_friendly_object(ob);
  }
  if(QUERY_FLAG(ob,FLAG_FREED)) {
    dump_object(ob);
    LOG(llevBug,"BUG: Trying to free freed object.\n%s\n",errmsg);
    return;
  }
  if(ob->more!=NULL) {
    free_object(ob->more);
    ob->more=NULL;
  }
  if (ob->inv) {
    if (ob->map==NULL || ob->map->in_memory!=MAP_IN_MEMORY || wall(ob->map,ob->x,ob->y))
    {
      op=ob->inv;
      while(op!=NULL) {
        tmp=op->below;
        remove_ob(op);
        free_object(op);
        op=tmp;
      }
    }
    else {
		if(ob->type == PLAYER) /* we don't handle players here */
			LOG(llevBug,"BUG: free_object:() - try to drop items of %s\n", ob->name);
		else /* create race corpse and/or drop stuff to floor */
		{
			object *corpse=NULL;

			if((QUERY_FLAG(ob,FLAG_CORPSE) && !QUERY_FLAG(ob,FLAG_STARTEQUIP))
														|| QUERY_FLAG(ob,FLAG_CORPSE_FORCED))
			{
				racelink *race_corpse = find_racelink(ob->race);
				if(race_corpse)
				{
					corpse=arch_to_object(race_corpse->corpse);
				    corpse->x=ob->x;corpse->y=ob->y;corpse->map=ob->map;
					corpse->weight = ob->weight;
				}
			}

			op=ob->inv;
			while(op!=NULL)
			{
				tmp=op->below;
				remove_ob(op);
					/* if we move spawn mobs, we don't want drop their items as free.
					* So, marking the mob itself with "FLAG_STARTEQUIP" will kill
					* all inventory and not dropping it on the map.
					* This also happens when a player slays a to low mob/non exp mob.
					* Don't drop any sys_object in inventory... I can't think about
					* any use... when we do it, a disease needle for example
					* is dropping his disease force and so on.
					*/
					if(QUERY_FLAG(op,FLAG_SYS_OBJECT)|| QUERY_FLAG(ob,FLAG_STARTEQUIP) ||QUERY_FLAG(op,FLAG_STARTEQUIP)
							||QUERY_FLAG(op,FLAG_NO_DROP) || op->type==RUNE)
						free_object(op);
					else 
					{
						op->x=ob->x,op->y=ob->y;

						/* if we have a corpse put the item in it */
						if(corpse)
							insert_ob_in_ob(op, corpse);
						else
							insert_ob_in_map(op,ob->map,NULL,0); /* Insert in same map as the envir */
					}
				op=tmp;
			}

			if(corpse)
			{
				/* drop the corpse when something is in OR corpse_forced is set */
				if(QUERY_FLAG(ob,FLAG_CORPSE_FORCED) || corpse->inv)
				{
					/* ok... we have a corpse AND we insert it.
					 * now check enemy and/or attacker to find a player.
					 * if there is one - personlize this corpse container.
					 * this gives the player the chance to grap this stuff first
					 * - and looter will be stopped.
					 */

					/* carefull, we assume that this virgin slaying field is NULL */
					if(ob->enemy && ob->enemy->type == PLAYER && ob->enemy->count == ob->enemy_count)
						corpse->slaying = add_refcount(ob->enemy->name);

					/* later, we add here other sub type or slaying names for killing groups, clans, etc */
					if(corpse->slaying) /* change sub_type to mark this corpse */
						corpse->sub_type1 = ST1_CONTAINER_CORPSE_player;

					insert_ob_in_map (corpse, ob->map, NULL,0);
				}
				else
				{
					/* if we are here, our corpse mob had something in inv but its nothing to drop */
					if(!QUERY_FLAG(corpse,FLAG_REMOVED) )
						remove_ob(corpse);
					free_object(corpse);
				}

			}
		}
    }
  }
  /* Remove object from the active list */
  ob->speed = 0;
  update_ob_speed(ob);

  SET_FLAG(ob, FLAG_FREED);
  ob->count = 0;
  /* First free the object from the used objects list: */
  if(ob->prev==NULL) {
    objects=ob->next;
    if(objects!=NULL)
      objects->prev=NULL;
  }
  else {
    ob->prev->next=ob->next;
    if(ob->next!=NULL)
      ob->next->prev=ob->prev;
  }
  
  if(ob->name!=NULL) {
    free_string(ob->name);
    ob->name=NULL;
  }
  if(ob->title!=NULL) {
    free_string(ob->title);
    ob->title=NULL;
  }
  if(ob->race!=NULL) {
    free_string(ob->race);
    ob->race=NULL;
  }
  if(ob->slaying!=NULL) {
    free_string(ob->slaying);
    ob->slaying=NULL;
  }
  if(ob->msg!=NULL) {
    free_string(ob->msg);
    ob->msg=NULL;
  }

  /* Now link it with the free_objects list: */
  ob->prev=NULL;
  ob->next=free_objects;
  if(free_objects!=NULL)
    free_objects->prev=ob;
  free_objects=ob;
  nroffreeobjects++;
}

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

/*
 * count_active() returns the number of objects on the list of active objects.
 */

int count_active() {
  int i=0;
  object *tmp=active_objects;
  while(tmp!=NULL)
    tmp=tmp->active_next, i++;
  return i;
}

/*
 * sub_weight() recursively (outwards) subtracts a number from the
 * weight of an object (and what is carried by it's environment(s)).
 */

void sub_weight (object *op, signed long weight) {
  while (op != NULL) {
    if (op->type == CONTAINER) {
      weight=(signed long)(weight*(100-op->stats.Str)/100);
    }
    op->carrying-=weight;
    op = op->env;
  }
}

/* remove_ob(op):
 *   This function removes the object op from the linked list of objects
 *   which it is currently tied to.  When this function is done, the
 *   object will have no environment.  If the object previously had an
 *   environment, the x and y coordinates will be updated to
 *   the previous environment.
 *   if we want remove alot of players inventory items, set
 *   FLAG_NO_FIX_PLAYER to thje player first and call fix_player()
 *   explicit then.
 */

void remove_ob(object *op) {
	MapSpace *msp;
    object *tmp,*last=NULL;
    object *otmp;
    tag_t tag;
    int check_walk_off;
    

    if(QUERY_FLAG(op,FLAG_REMOVED)) 
	{
		dump_object(op);
		LOG(llevBug,"BUG: Trying to remove removed object.\n:%s\n", errmsg);
		return;
    }

    if(op->more!=NULL)
		remove_ob(op->more);

    SET_FLAG(op, FLAG_REMOVED);

    /* 
     * In this case, the object to be removed is in someones
     * inventory.
     */
    if(op->env!=NULL)
	{
		if(op->nrof)
			sub_weight(op->env, op->weight*op->nrof);
		else
			sub_weight(op->env, op->weight+op->carrying);

		/* NO_FIX_PLAYER is set when a great many changes are being
		* made to players inventory.  If set, avoiding the call to save cpu time.
		* the flag is set from outside... perhaps from a drop_all() function.
		*/
		if ((otmp=is_player_inv(op->env))!=NULL && otmp->contr && !QUERY_FLAG(otmp,FLAG_NO_FIX_PLAYER))
		    fix_player(otmp);

		if(op->above!=NULL)
			op->above->below=op->below;
		else
			op->env->inv=op->below;

		if(op->below!=NULL)
			op->below->above=op->above;

		/* we set up values so that it could be inserted into
		* the map, but we don't actually do that - it is up
		* to the caller to decide what we want to do.
		*/
		op->x=op->env->x,op->y=op->env->y;

		#ifdef POSITION_DEBUG
			op->ox=op->x,op->oy=op->y;
		#endif

		op->map=op->env->map;
		op->above=NULL,op->below=NULL;
		op->env=NULL;
		return;
    }

    /* If we get here, we are removing it from a map */
    if (op->map == NULL)
		return;

    if (!op->map)
		LOG(llevBug,"BUG: remove_ob called when object was on map but appears to not be within valid coordinates? %s (%d,%d)\n",
															    op->map->path, op->x, op->y);
 
    /* Re did the following section of code - it looks like it had
     * lots of logic for things we no longer care about
     */

	/* if this is the base layer object, we assign the next object to be it if it is from same layer type */
	msp=GET_MAP_SPACE_PTR(op->map,op->x,op->y);
	if(QUERY_FLAG(op,FLAG_IS_INVISIBLE) )		
		msp->flags|=P_SET_INV;	/* we play around with invisible objects - tell it update */

	if(op->layer)
	{
		if(GET_MAP_SPACE_LAYER(msp,op->layer-1) == op)
		{
			/* well, don't kick the inv objects of this layer to normal layer */
		    if (op->above && op->above->layer == op->layer && GET_MAP_SPACE_LAYER(msp,op->layer+6)!= op->above)
				SET_MAP_SPACE_LAYER(msp,op->layer-1, op->above);
			else
				SET_MAP_SPACE_LAYER(msp,op->layer-1, NULL);
		}
		else if(GET_MAP_SPACE_LAYER(msp,op->layer+6) == op) /* inv layer? */
		{
		    if (op->above && op->above->layer == op->layer)
				SET_MAP_SPACE_LAYER(msp,op->layer+6, op->above);
			else
				SET_MAP_SPACE_LAYER(msp,op->layer+6, NULL);
		}
	}

    /* link the object above us */
    if (op->above)
		op->above->below=op->below;
	else
		SET_MAP_SPACE_LAST(msp,op->below); /* assign below as last one */

    /* Relink the object below us, if there is one */
    if(op->below)
		op->below->above=op->above;
    else
		SET_MAP_SPACE_FIRST(msp,op->above);  /* first object goes on above it. */

    op->above=NULL;                      
    op->below=NULL;

	/* this is triggered when a map is swaped out and the objects on it get removed too */
	if (op->map->in_memory == MAP_SAVING)
		return;

    tag = op->count;
    check_walk_off = ! QUERY_FLAG (op, FLAG_NO_APPLY);
    for(tmp=GET_MAP_OB(op->map,op->x,op->y);tmp!=NULL;tmp=tmp->above)
	{
		/* No point updating the players look faces if he is the object
		* being removed.
		*/

		if(tmp->type==PLAYER && tmp!=op) 
		{
			/* If a container that the player is currently using somehow gets
			* removed (most likely destroyed), update the player view
			* appropriately.
			*/
			if (tmp->container==op)
			{
				CLEAR_FLAG(op, FLAG_APPLIED);
				tmp->container=NULL;
			}
			tmp->contr->socket.update_look=1;
		}

		if (check_walk_off && (QUERY_FLAG (op, FLAG_FLYING) ?
			QUERY_FLAG (tmp, FLAG_FLY_OFF) : QUERY_FLAG (tmp, FLAG_WALK_OFF)))
		{
	    
			move_apply_func (tmp, op, NULL);
			if (was_destroyed (op, tag))
			{
				/* hmm... why is this a bug??.. this is like a trap
				 * which instant kills you when you move away... not fair
				 * but really not a bug. Of course not smart to handle this
				 * AFTER the remove.
				 */
				LOG (llevBug, "BUG: remove_ob(): name %s, archname %s destroyed leaving object\n", tmp->name, tmp->arch->name);
			}
		}

		/* Eneq(@csd.uu.se): Fixed this to skip tmp->above=tmp */

		if(tmp->above == tmp)
			tmp->above = NULL;
		last=tmp;
    }

	update_object(op, UP_OBJ_REMOVE);		

	/* this can be optimized too - check for blocksview on this map square before */
    if(QUERY_FLAG(op,FLAG_BLOCKSVIEW)|| (op->glow_radius>0)) 
		update_all_los(op->map, op->x, op->y);
}

/*
 * merge_ob(op,top):
 *
 * This function goes through all objects below and including top, and
 * merges op to the first matching object.
 * If top is NULL, it is calculated.
 * Returns pointer to object if it succeded in the merge, otherwise NULL
 */

object *merge_ob(object *op, object *top) {
  if(!op->nrof)
    return 0;
  if(top==NULL)
    for(top=op;top!=NULL&&top->above!=NULL;top=top->above);
  for(;top!=NULL;top=top->below) {
    if(top==op)
      continue;
    if (CAN_MERGE(op,top))
    {
      top->nrof+=op->nrof;
/*      CLEAR_FLAG(top,FLAG_STARTEQUIP);*/
      op->weight = 0; /* Don't want any adjustements now */
      remove_ob(op);
      free_object(op);
      return top;
    }
  }
  return NULL;
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
 *   new object if 'op' was merged with other object
 *   NULL if 'op' was destroyed
 *   just 'op' otherwise
 */

object *insert_ob_in_map (object *op, mapstruct *m, object *originator, int flag)
{
    object *tmp, *top, *floor=NULL;
	MapSpace *mc;
    int x,y,lt,layer, layer_inv;

    if (QUERY_FLAG (op, FLAG_FREED))
	{
		dump_object(op);
		LOG (llevBug, "BUG: Trying to insert freed object %s in map %s!\n:%s\n", op->name, m->name,errmsg);
		return NULL;
    }
    if(m==NULL)
	{
		dump_object(op);
		LOG(llevBug,"BUG: Trying to insert object %s in null-map!\n%s\n",op->name, errmsg);
		return op;
    }
		
	if(!QUERY_FLAG(op,FLAG_REMOVED))
	{
		dump_object(op);
		LOG(llevBug,"BUG: Trying to insert non removed object %s in map %s.\n%s\n", op->name, m->name, errmsg);
		return op;
    }

    if(op->more!=NULL)
	{
		if (insert_ob_in_map(op->more,m,originator,flag) == NULL) 
		{
			if ( ! op->head)
				LOG (llevBug, "BUG: insert_ob_in_map(): inserting op->more killed op %s in map %s\n",op->name, m->name);
			return NULL;
		}
    }
    CLEAR_FLAG(op,FLAG_REMOVED);

#ifdef POSITION_DEBUG
    /* Debugging information so you can see the last coordinates this object had */
    op->ox=op->x;
    op->oy=op->y;
#endif
    x = op->x;
    y = op->y;

    if(!(m=out_of_map(m, &x, &y)))
	{
		dump_object(op);
		LOG(llevBug,"BUG: Trying to insert object outside the map.\n%s\n", errmsg);
		return op;
    }

        /* Ideally, the caller figures this out */
    if (op->map != m) {
	/* coordinates should not change unless map also changes */
	op->x = x;
	op->y = y;
	op->map = m;
#if 0
	LOG(llevDebug,"insert_ob_in_map not called with proper tiled map: %s != %s, orig coord = %d, %d\n",
	    op->map->path, m->path, op->ox, op->oy);
#endif
    }

	/* hm, i not checked this, but is it not smarter to remove op instead and return? MT */
    if(op->nrof && !(flag & INS_NO_MERGE)) 
	{
		for(tmp=GET_MAP_OB(m, x, y);tmp!=NULL;tmp=tmp->above)
		{
			if (CAN_MERGE(op,tmp)) 
			{
				op->nrof+=tmp->nrof;
				remove_ob(tmp);
				free_object(tmp);
			}
		}
    }

    CLEAR_FLAG(op,FLAG_APPLIED); /* hack for fixing F_APPLIED in items of dead people */
    CLEAR_FLAG(op, FLAG_INV_LOCKED);
    if (!QUERY_FLAG(op, FLAG_ALIVE))
	CLEAR_FLAG(op, FLAG_NO_STEAL);

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
	mc = GET_MAP_SPACE_PTR(op->map,op->x,op->y); /* for fast access - we will not change the node here */
	if(op->layer) /* so we have a non system object */
	{
		layer = op->layer-1;
		layer_inv = layer+7;
		if(!QUERY_FLAG(op,FLAG_IS_INVISIBLE)) /* not invisible? */
		{
			/* have we another object of this layer? */
			if((top=GET_MAP_SPACE_LAYER(mc, layer))==NULL && (top=GET_MAP_SPACE_LAYER(mc, layer_inv))==NULL) 
			{
				/* no, we are the first of this layer - lets search something above us we can chain with.*/
				for(lt=op->layer;lt<MAX_ARCH_LAYERS&&(top=GET_MAP_SPACE_LAYER(mc,lt))==NULL&&(top=GET_MAP_SPACE_LAYER(mc,lt+7))==NULL;lt++)
					;
			}

			/* now, if top != NULL, thats the object BEFORE we want chain. This can be
			 * the base layer, the inv base layer or a object from a upper layer.
			 * If it NULL, we are the only object in this tile OR 
			 * ->last holds the object BEFORE ours.
			 */
			SET_MAP_SPACE_LAYER(mc,layer,op); /* we always go in front */
			if(top) /* easy - we chain our object before this one */
			{
				if(top->below)
					top->below->above=op;
				else /* if no object before ,we are new starting object */
					SET_MAP_SPACE_FIRST(mc,op); /* first object */
				op->below = top->below;
				top->below = op;
				op->above = top;
			}
			else /* we are first object here or one is before us  - chain to it */
			{
				if((top=GET_MAP_SPACE_LAST(mc))!=NULL)
				{
					top->above = op;
					op->below = top;

				}
				else /* a virgin! we set first and last object */
					SET_MAP_SPACE_FIRST(mc,op); /* first object */

				SET_MAP_SPACE_LAST(mc,op); /* last object */
			}
		}
		else /* invisible object */
		{
			int tmp_flag;
			mc->flags|=P_SET_INV;	/* we play around with invisible objects - tell it update */

			/* check the layers */
			if((top=GET_MAP_SPACE_LAYER(mc, layer_inv))!=NULL)
				tmp_flag = 1;
			else if((top=GET_MAP_SPACE_LAYER(mc, layer))!=NULL) 
			{
				/* in this case, we have 1 or more normal objects in this layer,
				 * so we must skip all of them. easiest way is to get a upper layer
				 * valid object.
				 */
				for(lt=op->layer;lt<MAX_ARCH_LAYERS&&(tmp=GET_MAP_SPACE_LAYER(mc,lt))==NULL&&(tmp=GET_MAP_SPACE_LAYER(mc,lt+7))==NULL;lt++)
					;
				tmp_flag = 2;
			}
			else
			{
				/* no, we are the first of this layer - lets search something above us we can chain with.*/
				for(lt=op->layer;lt<MAX_ARCH_LAYERS&&(top=GET_MAP_SPACE_LAYER(mc,lt))==NULL&&(top=GET_MAP_SPACE_LAYER(mc,lt+7))==NULL;lt++)
					;
				tmp_flag = 3;
			}

			SET_MAP_SPACE_LAYER(mc,layer_inv,op); /* in all cases, we are the new inv base layer */
			if(top) /* easy - we chain our object before this one - well perhaps */
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

				if(tmp_flag == 2)
				{
					if(tmp)
					{
						tmp->below->above=op; /* we can't be first, there is always one before us */
						op->below = tmp->below;
						tmp->below = op; /* and one after us... */
						op->above = tmp;
					}
					else
					{
						tmp = GET_MAP_SPACE_LAST(mc); /* there is one before us, so this is always valid */
						SET_MAP_SPACE_LAST(mc,op); /* new last object */
						op->below = tmp;
						tmp->above=op;
					}
				}
				else /* tmp_flag 1 & tmp_flag 3 are done the same way */
				{
					if(top->below)
						top->below->above=op;
					else /* if no object before ,we are new starting object */
						SET_MAP_SPACE_FIRST(mc,op); /* first object */
					op->below = top->below;
					top->below = op;
					op->above = top;
				}
			}
			else /* we are first object here or one is before us  - chain to it */
			{
				if((top=GET_MAP_SPACE_LAST(mc))!=NULL) /* there is something down we don't care what */
				{
					top->above = op; /* just chain to it */
					op->below = top;

				}
				else /* a virgin! we set first and last object */
					SET_MAP_SPACE_FIRST(mc,op); /* first object */

				SET_MAP_SPACE_LAST(mc,op); /* last object */
			}
		}
	}
	else /* op->layer == 0 - lets just put this object in front of all others */
	{
		/* is there some else? */
		if((top=GET_MAP_SPACE_FIRST(mc))!=NULL)
		{
			/* easy chaining */
			top->below = op;
			op->above = top;
		}
		else /* no ,we are last object too */
			SET_MAP_SPACE_LAST(mc,op); /* last object */

		SET_MAP_SPACE_FIRST(mc,op); /* first object */
	}


	/* lets set some specials for our players */
    if(op->type==PLAYER)
		op->contr->do_los=1;

    /* If we have a floor, we know the player, if any, will be above
     * it, so save a few ticks and start from there.
     */
	/* all players are layer 6 this can not be changed - so we can skip objects of different layer type */
	/* wizs can be layer 0, but we will autoforce a wizs look */
    for(tmp=GET_MAP_OB_LAYER(op->map,op->x,op->y,5);tmp!=NULL;tmp=tmp->above)
	{
		if (tmp->type == PLAYER)
		    tmp->contr->socket.update_look=1;
    }
	/* and the same for invisible players ... */
    for(tmp=GET_MAP_OB_LAYER(op->map,op->x,op->y,5+MAX_ARCH_LAYERS);tmp!=NULL;tmp=tmp->above)
	{
		if (tmp->type == PLAYER)
		    tmp->contr->socket.update_look=1;
    }

    /* Don't know if moving this to the end will break anything.  However,
     * we want to have update_look set above before calling this.
     *
     * check_walk_on() must be after this because code called from
     * check_walk_on() depends on correct map flags (so functions like
     * blocked() and wall() work properly), and these flags are updated by
     * update_object().
     */

    /* updates flags (blocked, alive, no magic, etc) for this map space */
    update_object(op,UP_OBJ_INSERT);

    /* If this object glows, it may affect lighting conditions that are
     * visible to others on this map.  But update_all_los is really
     * an inefficient way to do this, as it means los for all players
     * on the map will get recalculated.  The players could very well
     * be far away from this change and not affected in any way -
     * this should get redone to only look for players within range,
     * or just updating the P_NEED_UPDATE for spaces within this area
     * of effect may be sufficient.
     */
    if(MAP_DARKNESS(op->map) && op->glow_radius>0) 
		update_all_los(op->map, op->x, op->y);


    /* if this is not the head or flag has been passed, don't check walk on status */

    if (!(flag & INS_NO_WALK_ON) && !op->head) {
        if (check_walk_on(op, originator))
	    return NULL;

        /* If we are a multi part object, lets work our way through the check
         * walk on's.
         */
        for (tmp=op->more; tmp!=NULL; tmp=tmp->more)
            if (check_walk_on (tmp, originator))
		return NULL;
    }
    return op;
}

/* this function inserts an object in the map, but if it
 *  finds an object of its own type, it'll remove that one first. 
 *  op is the object to insert it under:  supplies x and the map.
 */
void replace_insert_ob_in_map(char *arch_string, object *op) {
    object *tmp;
    object *tmp1;

    /* first search for itself and remove any old instances */

    for(tmp=GET_MAP_OB(op->map,op->x,op->y); tmp!=NULL; tmp=tmp->above) {
	if(!strcmp(tmp->arch->name,arch_string)) /* same archetype */ {
	    remove_ob(tmp);
	    free_object(tmp);
	}
    }

    tmp1=arch_to_object(find_archetype(arch_string));

  
    tmp1->x = op->x; tmp1->y = op->y;
    insert_ob_in_map(tmp1,op->map,op,0);
}        

/*
 * get_split_ob(ob,nr) splits up ob into two parts.  The part which
 * is returned contains nr objects, and the remaining parts contains
 * the rest (or is removed and freed if that number is 0).
 * On failure, NULL is returned, and the reason put into the
 * global static errmsg array.
 */

object *get_split_ob(object *orig_ob,int nr) {
    object *newob;
    int is_removed = (QUERY_FLAG (orig_ob, FLAG_REMOVED) != 0);

    if((int) orig_ob->nrof<nr) {
	sprintf(errmsg,"There are only %d %ss.",
            orig_ob->nrof?orig_ob->nrof:1, orig_ob->name);
	return NULL;
    }
    newob=get_object();
    copy_object(orig_ob,newob);
    if((orig_ob->nrof-=nr)<1) {
	if ( ! is_removed)
            remove_ob(orig_ob);
	free_object(orig_ob);
    }
    else if ( ! is_removed) {
	if(orig_ob->env!=NULL)
	    sub_weight (orig_ob->env,orig_ob->weight*nr);
	if (orig_ob->env == NULL && orig_ob->map->in_memory!=MAP_IN_MEMORY) {
	    strcpy(errmsg, "Tried to split object whose map is not in memory.");
	    LOG(llevDebug,
		    "Error, Tried to split object whose map is not in memory.\n");
	    return NULL;
	}
    }
    newob->nrof=nr;
    return newob;
}

/*
 * decrease_ob_nr(object, number) decreases a specified number from
 * the amount of an object.  If the amount reaches 0, the object
 * is subsequently removed and freed.
 *
 * Return value: 'op' if something is left, NULL if the amount reached 0
 */

object *decrease_ob_nr (object *op, int i)
{
    object *tmp;
    player *pl;

    if (i == 0)   /* objects with op->nrof require this check */
        return op;

    if (i > (int)op->nrof)
        i = (int)op->nrof;

    if (QUERY_FLAG (op, FLAG_REMOVED))
    {
        op->nrof -= i;
    }
    else if (op->env != NULL)
    {
	/* is this object in the players inventory, or sub container
	 * therein?
	 */
        tmp = is_player_inv (op->env);
	/* nope.  Is this a container the player has opened?
	 * If so, set tmp to that player.
	 * IMO, searching through all the players will mostly
	 * likely be quicker than following op->env to the map,
	 * and then searching the map for a player.
	 */
	if (!tmp) {
	    for (pl=first_player; pl; pl=pl->next)
		if (pl->ob->container == op->env) break;
	    if (pl) tmp=pl->ob;
	    else tmp=NULL;
	}

        if (i < (int)op->nrof) {
            sub_weight (op->env, op->weight * i);
            op->nrof -= i;
            if (tmp) {
                (*esrv_send_item_func) (tmp, op);
                (*esrv_update_item_func) (UPD_WEIGHT, tmp, tmp);
            }
        } else {
            remove_ob (op);
            op->nrof = 0;
            if (tmp) {
                (*esrv_del_item_func) (tmp->contr, op->count);
                (*esrv_update_item_func) (UPD_WEIGHT, tmp, tmp);
            }
        }
    }
    else 
    {
	object *above = op->above;

        if (i < (int)op->nrof) {
            op->nrof -= i;
        } else {
            remove_ob (op);
            op->nrof = 0;
        }
	/* Since we just removed op, op->above is null */
        for (tmp = above; tmp != NULL; tmp = tmp->above)
            if (tmp->type == PLAYER) {
                if (op->nrof)
                    (*esrv_send_item_func) (tmp, op);
                else
                    (*esrv_del_item_func) (tmp->contr, op->count);
            }
    }

    if (op->nrof) {
        return op;
    } else {
        free_object (op);
        return NULL;
    }
}

/*
 * add_weight(object, weight) adds the specified weight to an object,
 * and also updates how much the environment(s) is/are carrying.
 */

void add_weight (object *op, signed long weight) {
  while (op!=NULL) {
    if (op->type == CONTAINER) {
      weight=(signed long)(weight*(100-op->stats.Str)/100);
    }
    op->carrying+=weight;
    op=op->env;
  }
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

object *insert_ob_in_ob(object *op,object *where) {
  object *tmp, *otmp;

  if(!QUERY_FLAG(op,FLAG_REMOVED)) {
    dump_object(op);
    LOG(llevBug,"BUG: Trying to insert (ob) inserted object.\n%s\n", errmsg);
    return op;
  }
  if(where==NULL) {
    dump_object(op);
    LOG(llevBug,"BUG: Trying to put object in NULL.\n%s\n", errmsg);
    return op;
  }
  if (where->head) {
    LOG(llevBug, "BUG: Tried to insert object wrong part of multipart object.\n");
    where = where->head;
  }
  if (op->more) {
    LOG(llevError, "ERROR: Tried to insert multipart object %s (%d)\n", op->name, op->count);
    return op;
  }
  CLEAR_FLAG(op, FLAG_OBJ_ORIGINAL);
  CLEAR_FLAG(op, FLAG_REMOVED);
  if(op->nrof) {
    for(tmp=where->inv;tmp!=NULL;tmp=tmp->below)
      if ( CAN_MERGE(tmp,op) ) {
	/* return the original object and remove inserted object
           (client needs the original object) */
        tmp->nrof += op->nrof;
	/* Weight handling gets pretty funky.  Since we are adding to
	 * tmp->nrof, we need to increase the weight.
	 */
	add_weight (where, op->weight*op->nrof);
        SET_FLAG(op, FLAG_REMOVED);
        free_object(op); /* free the inserted object */
        op = tmp;
        remove_ob (op); /* and fix old object's links */
        CLEAR_FLAG(op, FLAG_REMOVED);
	break;
      }

    /* I assume combined objects have no inventory
     * We add the weight - this object could have just been removed
     * (if it was possible to merge).  calling remove_ob will subtract
     * the weight, so we need to add it in again, since we actually do
     * the linking below
     */
    add_weight (where, op->weight*op->nrof);
  } else
    add_weight (where, (op->weight+op->carrying));

  otmp=is_player_inv(where);
  if (otmp&&otmp->contr!=NULL) {
    if (!QUERY_FLAG(otmp,FLAG_NO_FIX_PLAYER))
      fix_player(otmp);
  }

  op->map=NULL;
  op->env=where;
  op->above=NULL;
  op->below=NULL;
  op->x=0,op->y=0;
#ifdef POSITION_DEBUG
  op->ox=0,op->oy=0;
#endif
  /* reset the light list and los of the players on the map */
  if(op->glow_radius>0&&where->map)
  {
#ifdef DEBUG_LIGHTS
      LOG(llevDebug, " insert_ob_in_ob(): got %s to insert in map/op\n",
	op->name);
#endif /* DEBUG_LIGHTS */ 
      if (MAP_DARKNESS(where->map)) update_all_los(where->map, where->x, where->y);
  }

  /* Client has no idea of ordering so lets not bother ordering it here.
   * It sure simplifies this function...
   */
  if (where->inv==NULL)
      where->inv=op;
  else {
      op->below = where->inv;
      op->below->above = op;
      where->inv = op;
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

int check_walk_on (object *op, object *originator)
{
    object *tmp;
    tag_t tag;
    mapstruct *m=op->map;
    int x=op->x, y=op->y;

    if(QUERY_FLAG(op,FLAG_NO_APPLY))
	return 0;

    tag = op->count;

    /* The objects have to be checked from top to bottom.
     * Hence, we first go to the top: */
    for (tmp=GET_MAP_OB(op->map, op->x, op->y); tmp!=NULL &&
	 tmp->above!=NULL; tmp=tmp->above) {
	/* Trim the search when we find the first other spell effect 
	 * this helps performance so that if a space has 50 spell objects,
	 * we don't need to check all of them.
	 */
	if (QUERY_FLAG(tmp, FLAG_FLYING) && QUERY_FLAG(tmp, FLAG_NO_PICK)) break;
    }
    
    for(;tmp!=NULL; tmp=tmp->below) {
	if (tmp == op) continue;    /* Can't apply yourself */

	/* Slow down creatures moving over rough terrain */
	if(QUERY_FLAG(tmp,FLAG_SLOW_MOVE)&&!QUERY_FLAG(op,FLAG_FLYING)) {
	    float diff;

	    diff=(float) (SLOW_PENALTY(tmp)*FABS(op->speed));
	    if (op->type==PLAYER) {
		/* ARGH - we need quick flags for this... this is insane skill check every move */
		if ((QUERY_FLAG(tmp,FLAG_IS_HILLY) && find_skill(op,SK_CLIMBING)) ||
		    (QUERY_FLAG(tmp,FLAG_IS_WOODED) && find_skill(op,SK_WOODSMAN)))  {
			diff=diff/4.0f;
		}
	    }
	    op->speed_left -= diff;
	}
	if(QUERY_FLAG(op,FLAG_FLYING)?QUERY_FLAG(tmp,FLAG_FLY_ON):
	   QUERY_FLAG(tmp,FLAG_WALK_ON)) {
	    move_apply_func (tmp, op, originator);
            if (was_destroyed (op, tag))
              return 1;
	    /* what the person/creature stepped onto has moved the object
	     * someplace new.  Don't process any further - if we did,
	     * have a feeling strange problems would result.
	     */
	    if (op->map != m || op->x != x || op->y != y) return 0;
	}
    }
    return 0;
}

/*
 * present_arch(arch, map, x, y) searches for any objects with
 * a matching archetype at the given map and coordinates.
 * The first matching object is returned, or NULL if none.
 */

object *present_arch(archetype *at, mapstruct *m, int x, int y) {
  object *tmp;
  if(!(m=out_of_map(m,&x,&y))) {
    LOG(llevError,"ERROR: Present_arch called outside map.\n");
    return NULL;
  }
  for(tmp=GET_MAP_OB(m,x,y); tmp != NULL; tmp = tmp->above)
    if(tmp->arch == at)
      return tmp;
  return NULL;
}

/*
 * present(type, map, x, y) searches for any objects with
 * a matching type variable at the given map and coordinates.
 * The first matching object is returned, or NULL if none.
 */

object *present(unsigned char type,mapstruct *m, int x,int y) {
  object *tmp;
  if(!(m=out_of_map(m,&x,&y))) {
    LOG(llevError,"ERROR: Present called outside map.\n");
    return NULL;
  }
  for(tmp=GET_MAP_OB(m,x,y);tmp!=NULL;tmp=tmp->above)
    if(tmp->type==type)
      return tmp;
  return NULL;
}

/*
 * present_in_ob(type, object) searches for any objects with
 * a matching type variable in the inventory of the given object.
 * The first matching object is returned, or NULL if none.
 */

object *present_in_ob(unsigned char type,object *op) {
  object *tmp;
  for(tmp=op->inv;tmp!=NULL;tmp=tmp->below)
    if(tmp->type==type)
      return tmp;
  return NULL;
}

/*
 * present_arch_in_ob(archetype, object) searches for any objects with
 * a matching archetype in the inventory of the given object.
 * The first matching object is returned, or NULL if none.
 */

object *present_arch_in_ob(archetype *at, object *op)  {
  object *tmp;
  for(tmp=op->inv;tmp!=NULL;tmp=tmp->below)
    if( tmp->arch == at)
      return tmp;
  return NULL;
}

/*
 * set_cheat(object) sets the cheat flag (WAS_WIZ) in the object and in
 * all it's inventory (recursively).
 * If checksums are used, a player will get set_cheat called for
 * him/her-self and all object carried by a call to this function.
 */

void set_cheat(object *op) {
  object *tmp;
  SET_FLAG(op, FLAG_WAS_WIZ);
  if(op->inv)
    for(tmp = op->inv; tmp != NULL; tmp = tmp->below)
      set_cheat(tmp);
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

int find_free_spot(archetype *at, mapstruct *m,int x,int y,int start,int stop) {
  int i,index=0;
  static int altern[SIZEOFFREE];
  for(i=start;i<stop;i++) {
    /* Surprised the out_of_map check was missing. Without it, we may
     * end up accessing garbage, which may say a space is free
     */
    if (arch_out_of_map(at, m, x+freearr_x[i],y+freearr_y[i])) continue;
    if(!arch_blocked(at,NULL, m,x+freearr_x[i],y+freearr_y[i]))
      altern[index++]=i;
    else if(wall(m,x+freearr_x[i],y+freearr_y[i])&&maxfree[i]<stop)
      stop=maxfree[i];
  }
  if(!index) return -1;
  return altern[RANDOM()%index];
}

/*
 * find_first_free_spot(archetype, mapstruct, x, y) works like
 * find_free_spot(), but it will search max number of squares.
 * But it will return the first available spot, not a random choice.
 * Changed 0.93.2: Have it return -1 if there is no free spot available.
 */

int find_first_free_spot(archetype *at, mapstruct *m,int x,int y) {
  int i;
  for(i=0;i<SIZEOFFREE;i++) {
    if(!arch_blocked(at,NULL,m,x+freearr_x[i],y+freearr_y[i]))
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

int find_dir(mapstruct *m, int x, int y, object *exclude) {
  int i, xt, yt, max=SIZEOFFREE;
  mapstruct *mt;
  object *tmp;
  
  if (exclude && exclude->head)
    exclude = exclude->head;

  for(i=1;i<max;i++) {
	  xt = x+freearr_x[i];
	  yt = y+freearr_y[i];
    if(wall(m, xt,yt))
      max=maxfree[i];
    else {
		if(!(mt=out_of_map(m,&xt,&yt)))
			continue;
      tmp=GET_MAP_OB(mt,xt,yt);
      while(tmp!=NULL && ((tmp!=NULL&&!QUERY_FLAG(tmp,FLAG_MONSTER)&&
	tmp->type!=PLAYER) || (tmp == exclude || (tmp->head && tmp->head == exclude))))
	        tmp=tmp->above;
      if(tmp!=NULL)
        return freedir[i];
    }
  }
  return 0;
}

/*
 * distance(object 1, object 2) will return the square of the
 * distance between the two given objects.
 */

int distance(object *ob1,object *ob2) {
  int i;
  i= (ob1->x - ob2->x)*(ob1->x - ob2->x)+
         (ob1->y - ob2->y)*(ob1->y - ob2->y);
  return i;
}

/*
 * find_dir_2(delta-x,delta-y) will return a direction in which
 * an object which has subtracted the x and y coordinates of another
 * object, needs to travel toward it.
 */

int find_dir_2(int x, int y) {
  int q;
  if(!y)
    q= -300*x;
  else
    q=x*100/y;
  if(y>0) {
    if(q < -242)
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

int absdir(int d) {
  while(d<1) d+=8;
  while(d>8) d-=8;
  return d;
}

/*
 * dirdiff(dir1, dir2) returns how many 45-degrees differences there is
 * between two directions (which are expected to be absolute (see absdir())
 */

int dirdiff(int dir1, int dir2) {
  int d;
  d = abs(dir1 - dir2);
  if(d>4)
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

int can_pick(object *who,object *item) {
  return ((who->type==PLAYER && QUERY_FLAG(item,FLAG_NO_PICK) && QUERY_FLAG(item,FLAG_UNPAID)) || /* clone shop - we assume that items are never invisible here*/
	  (item->weight>0&&!QUERY_FLAG(item,FLAG_NO_PICK)&& !IS_INVISIBLE(item,who) && 
	  !QUERY_FLAG(item,FLAG_ALIVE)&& (who->type==PLAYER||item->weight<who->weight/3)));
}


/*
 * create clone from object to another
 */
object *ObjectCreateClone (object *asrc) {
    object *dst = NULL,*tmp,*src,*part,*prev, *item;

    if(!asrc) return NULL;
    src = asrc;
    if(src->head)
        src = src->head;

    prev = NULL;
    for(part = src; part; part = part->more) {
        tmp = get_object();
        copy_object(part,tmp);
        tmp->x -= src->x;
        tmp->y -= src->y;
        if(!part->head) {
            dst = tmp;
            tmp->head = NULL;
        } else {
            tmp->head = dst;
        }
        tmp->more = NULL;
        if(prev) 
            prev->more = tmp;
        prev = tmp;
    }
    /*** copy inventory ***/
    for(item = src->inv; item; item = item->below) {
	(void) insert_ob_in_ob(ObjectCreateClone(item),dst);
    }

    return dst;
}

int was_destroyed (object *op, tag_t old_tag)
{
    /* checking for FLAG_FREED isn't necessary, but makes this function more
     * robust */
    return op->count != old_tag || QUERY_FLAG (op, FLAG_FREED);
}

/* GROS - Creates an object using a string representing its content.         */
/* Basically, we save the content of the string to a temp file, then call    */
/* load_object on it. I admit it is a highly inefficient way to make things, */
/* but it was simple to make and allows reusing the load_object function.    */
/* Remember not to use load_object_str in a time-critical situation.         */
/* Also remember that multiparts objects are not supported for now.          */
object* load_object_str(char *obstr)
{
    object *op;
    FILE *tempfile;
    char filename[MAX_BUF];
    sprintf(filename,"%s/cfloadobstr2044",settings.tmpdir);
    tempfile=fopen(filename,"w");
    if (tempfile == NULL)
    {
        LOG(llevError,"ERROR: load_object_str(): Unable to access load object temp file\n");
        return NULL;
    };
    fprintf(tempfile,obstr);
    fclose(tempfile);

    op=get_object();

    tempfile=fopen(filename,"r");
    if (tempfile == NULL)
    {
        LOG(llevError,"ERROR: Unable to read object temp file\n");
        return NULL;
    };
    load_object(tempfile,op,LO_NEWFILE,0);
    LOG(llevDebug," load str completed, object=%s\n",op->name);
    CLEAR_FLAG(op,FLAG_REMOVED);
    fclose(tempfile);
    return op;
}



/*** end of object.c ***/

