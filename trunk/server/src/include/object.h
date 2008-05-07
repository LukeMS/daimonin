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

#define decrease_ob(xyz) decrease_ob_nr(xyz,1)

#define WEIGHT(op) ((op->nrof ? op->weight * op->nrof : (unsigned int) (op->weight) + op->carrying))
#define WEIGHT_NROF(op, nrof) ((nrof ? op->weight * nrof : op->weight) + op->carrying)

/* move_apply() function call flags */
#define MOVE_APPLY_DEFAULT  0
#define MOVE_APPLY_WALK_ON  1
#define MOVE_APPLY_FLY_ON   2
#define MOVE_APPLY_WALK_OFF 4
#define MOVE_APPLY_FLY_OFF  8
#define MOVE_APPLY_MOVE     16 /* means: our object makes a step in/out of this tile */
#define MOVE_APPLY_VANISHED 32 /* when a player logs out, the player char not "move" out of a tile
                                * but it "turns to nothing on the spot". This sounds senseless but for
                               * example a move out can trigger a teleporter action. This flag prevents
                               * a loging out/exploding object is teleported after removing it from the spot.
                               */
#define MOVE_APPLY_SAVING   64 /* move_apply() called from saving function */

/* WALK ON/OFF function return flags */
#define CHECK_WALK_OK        0
#define CHECK_WALK_DESTROYED 1
#define CHECK_WALK_MOVED     2


#define ARCH_MAX_TYPES       512 /* important - this must be higher as max type number! */
/* i sorted the members of this struct in 4 byte (32 bit) groups. This will help compiler
 * and cpu to make aligned access of the members, and can (and will) make things smaller
 * and faster - but this depends on compiler & system too.
 */
typedef struct obj
{
    /* These variables are not changed by copy_object(): */
    struct obj     *active_next;    /* Next & previous object in the 'active' */
    struct obj     *active_prev;    /* List.  This is used in process_events
                                     * so that the entire object list does not
                                     * need to be gone through.
                                     */
    struct obj     *below;          /* Pointer to the object stacked below this one */
    struct obj     *above;          /* Pointer to the object stacked above this one
                                     * Note: stacked in the *same* environment
                                     */
    struct obj     *inv;            /* Pointer to the first object in the inventory */
    struct obj     *env;            /* Pointer to the object which is the environment.
                                     * This is typically the container that the object is in.
                                     * if env == NULL then the object is on a map or in the nirvana.
                                     */
    struct obj     *more;           /* Pointer to the rest of a large body of objects */
    struct obj     *head;           /* Points to the main object of a large body */
    struct mapdef  *map;            /* Pointer to the map in which this object is present */

    tag_t           count_debug;
    tag_t           count;          /* Which nr. of object created this is. */
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
    const char     *name;           /* The name of the object, obviously... */
    const char     *title;          /* Of foo, etc */
    const char     *race;           /* human, goblin, dragon, etc */
    const char     *slaying;        /* Which race to do double damage to
                                     * If this is an exit, this is the filename */
    const char     *msg;            /* If this is a book/sign/magic mouth/etc */

    /* here starts copy_object() releated data */

    /* these are some internals */
    struct archt   *arch;           /* Pointer to archetype */
    struct oblnk   *randomitems;    /* thats now a linked list of treasurelist */

    /* we can remove chosen_skill & exp_obj by drop here a uint8 with a list of skill
     * numbers. Mobs has no skill and player can grap it from player struct. For exp,
     * i will use skill numbers in golems/ammo and spell objects. So, this can be removed.
     */
    struct obj     *chosen_skill;   /* the skill chosen to use */
    struct obj     *exp_obj;        /* the exp. obj (category) assoc. w/ this object */

    /* now "real" object releated data */
    struct archt   *other_arch;     /* Pointer used for various things */
    New_Face       *face;           /* struct ptr to the 'face' - the picture(s) */
    New_Face       *inv_face;       /* struct ptr to the inventory 'face' - the picture(s) */

    sint64          value;          /* How much money it is worth (or contains) */
    uint32          event_flags;    /* flags matching events of event objects inside object ->inv */
    sint32          weight;         /* Attributes of the object - the weight */
    sint32          weight_limit;   /* Weight-limit of object - player and container should have this... perhaps we can substitute it?*/
    sint32          carrying;       /* How much weight this object contains (of objects in inv) */
    uint32          nrof;           /* How many of the objects */
    uint32          damage_round_tag;   /* needed for the damage info for client in map2 */
    uint32          update_tag;     /* this is used from map2 update! */
    uint32          flags[NUM_FLAGS_32]; /* various flags */

    tag_t           enemy_count;    /* What count the enemy has */
    struct obj     *enemy;          /* Monster/player to follow even if not closest */
    /* (only used by containers now =) */
    tag_t           attacked_by_count;  /* the tag of attacker, so we can be sure */
    struct obj     *attacked_by;    /* This object start to attack us! only player & monster */
    tag_t           owner_count;    /* What count the owner had (in case owner has been freed) */
    struct obj     *owner;          /* Pointer to the object which controls this one
                                     * Owner should not be referred to directly
                                     * - get_owner() should be used instead.
                                     */
    /* *map is part of "object head" but this not? hmm */
    sint16          x;              /* X-Position in the map for this object */
    sint16          y;              /* Y-Position in the map for this object */
#ifdef USE_TILESTRETCHER
    sint16          z;              /* Z-Position in the map (in pixels) for this object */
#endif

    uint16          path_attuned;   /* Paths the object is attuned to */
    uint16          path_repelled;  /* Paths the object is repelled from */
    uint16          path_denied;    /* Paths the object is denied access to */

    uint16          last_damage;    /* thats the damage send with map2 */

    uint16          terrain_type;   /* type flags for different enviroment (tile is under water, firewalk,...)
                                     * A object which can be applied GIVES this terrain flags to his owner
                                     */
    uint16          terrain_flag;   /* The object can move over/is unaffected from this terrain type */


    uint16          material;           /* What materials this object consist of */
    sint16          material_real;      /* This hold the real material value like what kind of steel */

    sint16          last_heal;          /* Last healed. Depends on constitution */
    sint16          last_sp;            /* As last_heal, but for spell points */

    sint16          last_grace;         /* as last_sp, except for grace */
    sint16          last_eat;           /* How long since we last ate */

    uint16          animation_id;       /* An index into the animation array */
    uint16          inv_animation_id;   /* An index into the animation array for the client inv */

    sint8           glow_radius;        /* object is a light source */
    /* some stuff for someone coming softscrolling / smooth animations */
    /*sint8 tile_xoff;*/            /* x-offset of position of an object inside a tile */
    /*sint8 tile_yoff;*/            /* same for y-offset */
    sint8           magic;              /* Any magical bonuses to this item */
    uint8           state;              /* How the object was last drawn (animation) */

    sint8           level;              /* the level of this object (most used for mobs & player) */
    sint8           direction;          /* Means the object is moving that way. */
    sint8           facing;             /* Object is oriented/facing that way. */
    uint8           quick_pos;          /* quick pos is 0 for single arch, xxxx0000 for a head
                                         * or x/y offset packed to 4 bits for a tail
                                         * warning: change this when include > 15x15 monster
                                         */

    uint8           type;               /* PLAYER, BULLET, etc.  See define.h - must be < ARCH_MAX_TYPES */
    uint8           sub_type1;          /* sub type definition - this will be send to client too */
    uint8           item_quality;       /* quality of a item in range from 0-100 */
    uint8           item_condition;     /* condition of repair of an item - from 0 to 100% item_quality */

    uint8           item_race;          /* item crafted from race x. "orcish xxx", "dwarven xxxx" */
    uint8           item_level;         /* level needed to use or apply this item */
    uint8           item_skill;         /* if set and item_level, item_level in this skill is needed */

    sint8           anim_enemy_dir;     /**< special shadow variable: show dir to targeted enemy
                                          for mobs: activate attack animation
                                          for the given direction unless == -1 */
    sint8           anim_moving_dir;    /* sic: shows moving dir or -1 when object do something else */

    sint8           anim_enemy_dir_last; /* if we change facing in movement, we must test for update the anim*/
    sint8           anim_moving_dir_last; /* sic:*/
    sint8           anim_last_facing;     /* the last direction this monster was facing */
    sint8           anim_last_facing_last;/* the last direction this monster was facing backbuffer*/

    uint8           anim_speed;         /* animation speed in ticks */
    uint8           last_anim;          /* ticks between animation-frames */
    /* TODO: get rid of this one with AI system change */
    uint8           run_away;           /* Monster runs away if it's hp goes below this percentage. */

    uint8           hide;               /* The object is hidden. We don't use a flag here because
                                         * the range from 0-255 tells us the quality of the hide
                                         */
    uint8           layer;              /* the layer in a map, this object will be sorted in */

    /* TODO: get rid of using attrsets? */
    sint8           resist[NROFATTACKS];/* Intrinsic resist against damage - range from -125 to +125 */

    uint8           attack[NROFATTACKS];/* our attack values (only positiv ones */

    float           speed;              /* The overall speed of this object */
    float           speed_left;         /* How much speed is left to spend this round */
    float           weapon_speed;       /* new weapon speed system. swing of weapon */
    float           weapon_speed_left;

    living          stats;              /* object stats like hp, sp, grace ... */

#ifdef POSITION_DEBUG
    sint16          ox, oy;             /* For debugging: Where it was last inserted */
#endif

    void           *custom_attrset;     /* Type-dependant extra data. */
} object;

#ifdef WIN32
#pragma pack(pop)
#endif

#define CONTR(ob) ((player *)((ob)->custom_attrset))

/* This returns TRUE if the object is somethign that
 * should be displayed in the look window
 */
#define LOOK_OBJ(_ob) (!IS_SYS_INVISIBLE(_ob) && _ob->type!=PLAYER)

/* Used by update_object to know if the object being passed is
 * being added or removed.
 */
#define UP_OBJ_INSERT   1   /* object was inserted in a map */
#define UP_OBJ_REMOVE   2   /* object was removed from a map tile */
#define UP_OBJ_FLAGS    3   /* critical object flags has been changed, rebuild tile flags but NOT increase tile counter */
#define UP_OBJ_FACE     4   /* Only thing that changed was the face */
#define UP_OBJ_FLAGFACE 5   /* update flags & face (means increase tile update counter */
#define UP_OBJ_ALL      6   /* force full update */
#define UP_OBJ_LAYER    7   /* object layer was changed, rebuild layer systen - used from invisible for example */

/* Macro for the often used object validity test (verify an pointer/count pair) */
#define OBJECT_VALID(_ob_, _count_) ((_ob_) && (_ob_)->count == ((tag_t)_count_) && !QUERY_FLAG((_ob_), FLAG_REMOVED) && !OBJECT_FREE(_ob_))

/* Less strict version of above */
#define OBJECT_VALID_OR_REMOVED(_ob_, _count_) ((_ob_) && (_ob_)->count == ((tag_t)_count_) && !OBJECT_FREE(_ob_))

/* Test the object is not removed nor freed - but no count test */
#define OBJECT_ACTIVE(_ob_) (!QUERY_FLAG((_ob_),FLAG_REMOVED) && !OBJECT_FREE(_ob_))

/* Test if an object is in the free-list */
#define OBJECT_FREE(_ob_) ((_ob_)->count==0 && CHUNK_FREE(_ob_))

/* These are flags passed to insert_ob_in_map and
 * insert_ob_in_ob.  Note that all flags may not be meaningful
 * for both functions.
 */
#define INS_NO_MERGE        0x0001
#define INS_NO_WALK_ON      0x0002
#define INS_TAIL_MARKER     0x0004 /* used intern from insert_xx to track multi
                                       * arch problems - don't use!
                                   */

/* Waypoint macros */
#define WP_FLAG_ACTIVE FLAG_CURSED
#define WP_FLAG_BESTEFFORT FLAG_NO_ATTACK

#define WP_MAP(wp) (wp)->slaying
#define WP_X(wp) (wp)->stats.hp
#define WP_Y(wp) (wp)->stats.sp
#define WP_ACCEPTDIST(wp) (wp)->stats.grace
#define WP_DELAYTIME(wp) (wp)->stats.wc
#define WP_NEXTWP(wp) (wp)->title
#define WP_BEACON(wp) (wp)->race

#define STRING_WP_MAP(wp) STRING_OBJ_SLAYING(wp)
#define STRING_WP_NEXTWP(wp) STRING_OBJ_RACE(wp)

#define WP_MOVE_TRIES 4 /* number of retries to get closer to (local) target before giving up */

#endif
