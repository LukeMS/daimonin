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

#ifndef OBJECT_H
#define OBJECT_H

#ifdef WIN32
#pragma pack(push,1)
#endif

/* i sorted the members of this struct in 4 byte (32 bit) groups. This will help compiler
 * and cpu to make aligned access of the members, and can (and will) make things smaller 
 * and faster - but this depends on compiler & system too.
 */
typedef struct obj 
{
	/* These variables are not changed by copy_object(): */
	struct pl_player *contr;			/* Pointer to the player which control this object */
	struct obj *next;			/* Pointer to the next object in the free/used list */
	struct obj *prev;			/* Pointer to the previous object in the free/used list*/
	struct obj *active_next;	/* Next & previous object in the 'active' */
	struct obj *active_prev;	/* List.  This is used in process_events 
								 * so that the entire object list does not
								 * need to be gone through. 
								 */
	struct obj *below;			/* Pointer to the object stacked below this one */
	struct obj *above;			/* Pointer to the object stacked above this one
								 * Note: stacked in the *same* environment
							     */
	struct obj *inv;			/* Pointer to the first object in the inventory */
	struct obj *container;		/* Current container being used.  I think this
								 * is only used by the player right now.
								 */
	struct obj *env;			/* Pointer to the object which is the environment.
								 * This is typically the container that the object is in.
								 */
	struct obj *more;			/* Pointer to the rest of a large body of objects */
	struct obj *head;			/* Points to the main object of a large body */
	struct mapdef *map;			/* Pointer to the map in which this object is present */

	tag_t count;				/* Which nr. of object created this is. */
								/* hmmm... unchanged? this count should be new
								 * set every time a freed object is used again
								 * this count refers the logical object. MT.
								 */

    /* These get an extra add_refcount(), after having been copied by memcpy().
     * All fields beow this point are automatically copied by memcpy.  If
     * adding something that needs a refcount updated, make sure you modify
     * copy_object to do so.  Everything below here also gets cleared
     * by clear_object() 
	 */
	char *name;					/* The name of the object, obviously... */
	char *title;				/* Of foo, etc */
	char *race;					/* human, goblin, dragon, etc */
	char *slaying;				/* Which race to do double damage to 
								 * If this is an exit, this is the filename
								 */
	char *msg;					/* If this is a book/sign/magic mouth/etc */
	
	/* here starts copy_object() releated data */

	/* these are some internals */
	struct archt *arch;						/* Pointer to archetype */
	struct treasureliststruct *randomitems; /* Items to be generated */

	/* we can remove chosen_skill & exp_obj by drop here a uint8 with a list of skill
	 * numbers. Mobs has no skill and player can grap it from player struct. For exp,
	 * i will use skill numbers in golems/ammo and spell objects. So, this can be removed.
	 */
	struct obj *chosen_skill;	/* the skill chosen to use */
	struct obj *exp_obj;		/* the exp. obj (category) assoc. w/ this object */
	uint32		event_flags;    /* flags matching events of event objects inside object ->inv */
	/*struct obj *event_ptr;*/ /* needed when we want chain script direct without browsing
						    * the objects inventory (this is needed when we want mutiple
							* scripts of same kind in one object).
							*/

	/* now "real" object releated data */
	struct archt *other_arch;	/* Pointer used for various things */
	New_Face *face;				/* struct ptr to the 'face' - the picture(s) */
	New_Face *inv_face;			/* struct ptr to the inventory 'face' - the picture(s) */

	sint32 weight;				/* Attributes of the object - the weight */
	uint32 weight_limit;		/* Weight-limit of object */
	sint32 carrying;			/* How much weight this object contains (of objects in inv) */
	uint32 path_attuned;		/* Paths the object is attuned to */
	uint32 path_repelled;		/* Paths the object is repelled from */
	uint32 path_denied; 		/* Paths the object is denied access to */
	sint32 value;				/* How much money it is worth (or contains) */
	uint32 nrof;				/* How many of the objects */
	uint32 damage_round_tag;	/* neede for the damage info for client in map2 */
	uint32 update_tag;			/* this is used from map2 update! */ 
	uint32 flags[NUM_FLAGS_32]; /* various flags */

	tag_t enemy_count;			/* What count the enemy has */
	struct obj *enemy;			/* Monster/player to follow even if not closest */
	tag_t attacked_by_count;	/* the tag of attacker, so we can be sure */
	struct obj *attacked_by;	/* This object start to attack us! only player & monster */
	tag_t ownercount;			/* What count the owner had (in case owner has been freed) */
	struct obj *owner;			/* Pointer to the object which controls this one
								 * Owner should not be referred to directly
								 * - get_owner() should be used instead.
								 */
	/* *map is part of "object head" but this not? hmm */
	sint16 x;					/* X-Position in the map for this object */
	sint16 y;					/* Y-Position in the map for this object */

	sint16 attacked_by_distance;/* needed to target the nearest enemy */
	uint16 last_damage;			/* thats the damage send with map2 */

	uint16 terrain_type;		/* type flags for different enviroment (tile is under water, firewalk,...) 
	                             * A object which can be applied GIVES this terrain flags to his owner
	                             */
	uint16 terrain_flag;		/* The object can move over/is unaffected from this terrain type */


	uint16 material;      		/* What materials this object consist of */
	sint16 material_real;		/* This hold the real material value like what kind of steel */

	sint16 last_heal;			/* Last healed. Depends on constitution */
	sint16 last_sp;				/* As last_heal, but for spell points */

	sint16 last_grace;			/* as last_sp, except for grace */
	sint16 last_eat;			/* How long since we last ate */

    uint16 animation_id;		/* An index into the animation array */
    uint16 inv_animation_id;	/* An index into the animation array for the client inv */

	/* some stuff for someone coming softscrolling / smooth animations */
	/*sint8 tile_xoff;*/			/* x-offset of position of an object inside a tile */
	/*sint8 tile_yoff;*/			/* same for y-offset */
	sint8 magic;				/* Any magical bonuses to this item */
	uint8 state;				/* How the object was last drawn (animation) */

	sint8 level;				/* the level of this object (most used for mobs & player) */
	sint8 direction;			/* Means the object is moving that way. */
	sint8 facing;				/* Object is oriented/facing that way. */
	uint8 quick_pos;			/* quick pos is 0 for single arch, xxxx0000 for a head
								 * or x/y offset packed to 4 bits for a tail 
								 * warning: change this when include > 15x15 monster
								 */

	uint8 type; 				/* PLAYER, BULLET, etc.  See define.h */
	uint8 sub_type1;			/* sub type definition - this will be send to client too */
	uint8 item_quality;			/* quality of a item in range from 0-100 */
	uint8 item_condition;		/* condition of repair of an item - from 0 to 100% item_quality */

	uint8 item_race;			/* item crafted from race x. "orcish xxx", "dwarven xxxx" */
	uint8 item_level;			/* level needed to use or apply this item */
	uint8 item_skill;			/* if set and item_level, item_level in this skill is needed */
	sint8 glow_radius;			/* indicates the glow radius of the object */

	sint8 move_status;			/* What stage in attack mode */
    uint8 move_type;			/* What kind of attack movement */
	sint8 anim_enemy_dir;       /* special shadow variable: show dir to targeted enemy */
	sint8 anim_moving_dir;      /* sic: shows moving dir or -1 when object do something else */

	sint8 anim_enemy_dir_last;	/* if we change facing in movement, we must test for update the anim*/
	sint8 anim_moving_dir_last; /* sic:*/
	sint8 anim_last_facing;     /* the last direction this monster was facing */
	sint8 anim_last_facing_last;/* the last direction this monster was facing backbuffer*/
  
    uint8 anim_speed;			/* animation speed in ticks */
	uint8 last_anim;			/* ticks between animation-frames */
	uint8 will_apply;			/* See crossfire.doc */
	uint8 run_away;				/* Monster runs away if it's hp goes below this percentage. */

	uint8 pick_up;				/* pickup mode - See crossfire.doc */  
	sint8 stealth;				/* the "stealth" value. from -100 to +100.
								 * -100 means the object will make ALOT of noice when moving,
								 * +100 means it will move like a ghost 
								 */
	uint8 hide;					/* The object is hidden. We don't use a flag here because
								 * the range from 0-255 tells us the quality of the hide
								 */
	uint8 layer;				/* the layer in a map, this object will be sorted in */

	sint8 resist[NROFATTACKS];	/* Intrinsic resist against damage - range from -125 to +125 */

	uint8 attack[NROFATTACKS];	/* our attack values - range from 0%-125%. (negative values makes no sense).
	                             * Note: we can in theory allow 300% damage for a attacktype.
	                             * all we need is to increase sint8 to sint16. Thats true for
	                             * resist & protection too. But it will be counter the damage
	                             * calculation. Think about a way a player deals only 10 damage
	                             * at base but can grap so many items that he does 3000% damage.
	                             * thats not how this should work. More damage should come from
	                             * the stats.dmg value - NOT from this source.
								 * The "125% max border" should work nice and the 25% over 100%
								 * should give a little boost. I think about to give player crafters
								 * the power to boost items to 100%+.
	                             */

	sint8 protection[NROFPROTECTIONS];	/* Resistance against attacks in % - range from 125-125*/

	float speed;				/* The overall speed of this object */
	float speed_left;			/* How much speed is left to spend this round */
	float weapon_speed;			/* new weapon speed system. swing of weapon */
	float weapon_speed_left;
	float weapon_speed_add;

	living stats;				/* object stats like hp, sp, grace ... */

	uint32	attacktype;			/* REMOVE IS IN PROCESS */


#ifdef CASTING_TIME
	sint16 casting;				/* time left before spell goes off */
	uint16 spell_state;
	uint16 start_holding;
	struct spell_struct *spell;
	uint32 spelltype;
	char *spellarg;
#endif

#ifdef POSITION_DEBUG
	sint16 ox,oy;				/* For debugging: Where it was last inserted */
#endif

} object;

#ifdef WIN32
#pragma pack(pop)
#endif

typedef struct oblnk { /* Used to link together several objects */
  object *ob;
  struct oblnk *next;
  int id;
} objectlink;

typedef struct oblinkpt { /* Used to link together several object links */
  struct oblnk *link;
  long value;		/* Used as connected value in buttons/gates */
  struct oblinkpt *next;
} oblinkpt;

/*
 * The archetype structure is a set of rules on how to generate and manipulate
 * objects which point to archetypes.
 * This probably belongs in arch.h, but there really doesn't appear to
 * be much left in the archetype - all it really is is a holder for the
 * object and pointers.  This structure should get removed, and just replaced
 * by the object structure
 */

typedef struct archt {
    char *name;					/* More definite name, like "generate_kobold" */
    struct archt *next;			/* Next archetype in a linked list */
    struct archt *head;			/* The main part of a linked object */
    struct archt *more;			/* Next part of a linked object */
	object		 *base_clone;	/* used by artifacts list: if != NULL,
								 * this object is the base object and clone is
								 * the modified artifacts object.
								 * we use base_clone for unidentified objects 
								 * (to get unified "non identified" values),
								 * or it is used to get a base object when we
								 * remove the artifacts changes (cancellation, dispel...)
								 */
    object		 clone;			/* An object from which to do copy_object() */
} archetype;

extern object *objects;
extern object *active_objects;
extern object *free_objects;
extern object objarray[STARTMAX];

extern int nrofallocobjects;
extern int nroffreeobjects;

/* This returns TRUE if the object is somethign that
 * should be displayed in the look window
 */
#define LOOK_OBJ(_ob) (!IS_SYS_INVISIBLE(_ob) && _ob->type!=PLAYER)

/* Used by update_object to know if the object being passed is
 * being added or removed.
 */
#define UP_OBJ_INSERT   1
#define UP_OBJ_REMOVE   2
#define UP_OBJ_CHANGE   3
#define UP_OBJ_FACE     4   /* Only thing that changed was the face */
#define UP_OBJ_INV		5	/* object has changed invisibility - adjust map sorting */

/* These are flags passed to insert_ob_in_map and 
 * insert_ob_in_ob.  Note that all flags may not be meaningful
 * for both functions.
 */
#define INS_NO_MERGE		0x0001
#define INS_ABOVE_FLOOR_ONLY	0x0002
#define INS_NO_WALK_ON		0x0004
/* Always put object on top.  Generally only needed when loading files
 * from disk and ordering needs to be preserved.  Note that INS_ABOVE_FLOOR_ONLY
 * and INS_ON_TOP are really mutually exclusive.
 */
#define INS_ON_TOP		0x0008


#endif
