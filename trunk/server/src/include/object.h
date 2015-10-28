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

#ifndef __OBJECT_H
#define __OBJECT_H

/* Calculates the weight of _WHAT_ including any contents.
 *
 * _WHAT_ may be a container of holding (containers do not stack) or any other
 * object (which may or may not be stacked). */
#define WEIGHT_OVERALL(_WHAT_) \
    ((_WHAT_)->type == CONTAINER && \
     (_WHAT_)->weapon_speed != 1.0) ? \
    ((_WHAT_)->damage_round_tag + (_WHAT_)->weight) : \
    (((_WHAT_)->weight * (((_WHAT_)->nrof > 0) ? (_WHAT_)->nrof : 1)) + \
     (_WHAT_)->carrying)

#define TAG(_O_) (((_O_)) ? (_O_)->count : 0)

/* There are technically three possible types of invisibility: system, gmaster,
 * and normal.
 *
 * In fact, system is a very special case. This is tested by
 * querying FLAG_SYS_OBJECT on the object. System objects are used by the
 * server to maintain the gameworld. Players *never* interact with them
 * directly (the exception here is that SAs can see/examine/pick up/drop them).
 *
 * Gmaster is an invisibility available to GMs and SAs only. *Only* SAs can see
 * such invisibility.
 *
 * Normal is the sort of everyday invisibility you get through magic and so on.
 * Any object can have this sort of invisibility (it can be combined with
 * gmaster). Equally anyone can acquire the ability to see through it (and SAs
 * always can). */
#define IS_GMASTER_INVIS_TO(_WHO_, _WHOM_) \
    (!(GET_GMASTER_MODE((_WHOM_)) & GMASTER_MODE_SA) && \
     IS_GMASTER_INVIS((_WHO_)))

#define IS_NORMAL_INVIS_TO(_WHAT_, _WHO_) \
    (!(GET_GMASTER_MODE((_WHO_)) & GMASTER_MODE_SA) && \
     QUERY_FLAG((_WHAT_), FLAG_IS_INVISIBLE) && \
     !QUERY_FLAG((_WHO_), FLAG_SEE_INVISIBLE))

/* i disabled slow penalty ATM */
#define SLOW_PENALTY(_O_)   0
#define SET_SLOW_PENALTY(_O_, _V_)    (_O_)->stats.exp = (sint32)((_V_) * 1000.0)
#define EXIT_PATH(_O_)         (_O_)->slaying
#define EXIT_DST_PATH(_O_)     (_O_)->race
#define EXIT_POS_FIX(_O_)      (_O_)->last_heal
#define EXIT_POS_RANDOM(_O_)   (_O_)->last_sp
#define EXIT_POS_FREE(_O_)     (_O_)->last_grace
#define EXIT_STATUS(_O_)   (_O_)->last_eat
#define EXIT_LEVEL(_O_)    (_O_)->stats.food
#define EXIT_X(_O_)        (_O_)->stats.hp
#define EXIT_Y(_O_)        (_O_)->stats.sp

#define D_LOCK(_O_) (_O_)->contr->freeze_inv=(_O_)->contr->freeze_look=1;
#define D_UNLOCK(_O_)   (_O_)->contr->freeze_inv=(_O_)->contr->freeze_look=0;

#define ARMOUR_SPEED(_O_)   (_O_)->last_sp
#define ARMOUR_SPELLS(_O_)  (_O_)->last_heal

/* i sorted the members of this struct in 4 byte (32 bit) groups. This will help compiler
 * and cpu to make aligned access of the members, and can (and will) make things smaller
 * and faster - but this depends on compiler & system too.
 */

/* This prevents object stacks from being "too big". When an object stack is
 * larger than this it will cause some problems. */
/* TODO: Over time, add code to check against this (i.e. prevent /create-ing
 * stacks larger). */
// #define MAX_OBJ_NROF 2147483647
/* This is MAX_LONG which bears little resemblence to object.nrof (currently a
 * uint32). In any case it is vastly larger than ever needed (eg, even turning
 * a stack of mith into copper only yields 10000000 copper per mith so we'd
 * need 215 mith to exceed this limit in this highly contrived scenario.
 *
 * So, the max value for uint32 is (2^32-1=)4294967295, an outrageously huge
 * number. It is useful to have some leeway so lets round it down (and still
 * have nearly double the old value) to: */
#define MAX_OBJ_NROF 4290000000

/* Values for the article parameter of query_name() -- see that function for
 * further explanation. */
#define ARTICLE_NONE       0
#define ARTICLE_INDEFINITE 1
#define ARTICLE_DEFINITE   2
#define ARTICLE_POSSESSIVE (MAX_OBJ_NROF + 1)

/* QUERY_SHORT_NAME() does query_name() with the appropriate article and no
 * status. It's just a quick'n'easy macro. But note that it's not always
 * appropriate, depending on context. */
#define QUERY_SHORT_NAME(_WHAT_, _WHO_) \
    query_name((_WHAT_), (_WHO_), \
        ((_WHAT_)->nrof > 1 || IS_LIVE((_WHAT_))) ? ARTICLE_DEFINITE : ARTICLE_INDEFINITE, 0)

#if 0
//480
struct object_t
{
    /* These variables are not changed by copy_object(): */
    object_t     *active_next;    /* Next & previous object in the 'active' */
    object_t     *active_prev;    /* List.  This is used in process_events
                                     * so that the entire object list does not
                                     * need to be gone through.
                                     */
    object_t     *below;          /* Pointer to the object stacked below this one */
    object_t     *above;          /* Pointer to the object stacked above this one
                                     * Note: stacked in the *same* environment
                                     */
    object_t     *inv;            /* Pointer to the first object in the inventory */
    object_t     *env;            /* Pointer to the object which is the environment.
                                     * This is typically the container that the object is in.
                                     * if env == NULL then the object is on a map or in the nirvana.
                                     */
    object_t     *more;           /* Pointer to the rest of a large body of objects */
    object_t     *head;           /* Points to the main object of a large body */
    map_t  *map;            /* Pointer to the map in which this object is present */

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
    archetype_t   *arch;           /* Pointer to archetype */
    struct objectlink_t   *randomitems;    /* thats now a linked list of treasurelist */

    /* we can remove chosen_skill & skillgroup by drop here a uint8 with a list of skill
     * numbers. Mobs has no skill and player can grap it from player struct. For exp,
     * i will use skill numbers in golems/ammo and spell objects. So, this can be removed.
     */
    object_t     *chosen_skill;   /* the skill chosen to use */
    object_t     *skillgroup;        /* the exp. obj (category) assoc. w/ this object_t */

    /* now "real" object releated data */
    archetype_t   *other_arch;     /* Pointer used for various things */
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
    object_t     *enemy;          /* Monster/player to follow even if not closest */
    /* (only used by containers now =) */
    tag_t           attacked_by_count;  /* the tag of attacker, so we can be sure */
    object_t     *attacked_by;    /* This object start to attack us! only player & monster */
    tag_t           owner_count;    /* What count the owner had (in case owner has been freed) */
    object_t     *owner;          /* Pointer to the object which controls this one
                                     * Owner should not be referred to directly
                                     * - get_owner() should be used instead.
                                     */

    object_t     *original;       /* Used to store the original, unbuffed version of an item. A buffed
                                     * item will revert to this state when it is saved. */
    uint8           buffed;         // Whether or not the object should contain any BUFF_FORCEs

    /* *map is part of "object head" but this not? hmm */
    sint16          x;              /* X-Position in the map for this object_t */
    sint16          y;              /* Y-Position in the map for this object_t */
#ifdef USE_TILESTRETCHER
    sint16          z;              /* Z-Position in the map (in pixels) for this object_t */
#endif

    uint16          path_attuned;   /* Paths the object is attuned to */
    uint16          path_repelled;  /* Paths the object is repelled from */
    uint16          path_denied;    /* Paths the object is denied access to */

    uint16          last_damage;    /* thats the damage send with map2 */

    uint16          terrain_type;   /* type flags for different enviroment (tile is under water, firewalk,...)
                                     * A object which can be applied GIVES this terrain flags to his owner
                                     */
    uint16          terrain_flag;   /* The object can move over/is unaffected from this terrain type */
    uint16          block_movement; /* The object blocks objects moving in this direction from it's tile */
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

    sint8           max_buffs;          // How many buffs can this item support?

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

    float           speed;              /* The overall speed of this object_t */
    float           speed_left;         /* How much speed is left to spend this round */
    float           weapon_speed;       /* new weapon speed system. swing of weapon */
    float           weapon_speed_left;

    living_t          stats;              /* object stats like hp, sp, grace ... */

#ifdef POSITION_DEBUG
    sint16          ox, oy;             /* For debugging: Where it was last inserted */
#endif

    void           *custom_attrset;     /* Type-dependant extra data. */
};
#else
//480
struct object_t
{
    /* These variables are not changed by copy_object(): */
    object_t     *active_next;    /* Next & previous object in the 'active' */
    object_t     *active_prev;    /* List.  This is used in process_events
                                     * so that the entire object list does not
                                     * need to be gone through.
                                     */
    object_t     *below;          /* Pointer to the object stacked below this one */
    object_t     *above;          /* Pointer to the object stacked above this one
                                     * Note: stacked in the *same* environment
                                     */
    object_t     *inv;            /* Pointer to the first object in the inventory */
    object_t     *env;            /* Pointer to the object which is the environment.
                                     * This is typically the container that the object is in.
                                     * if env == NULL then the object is on a map or in the nirvana.
                                     */
    object_t     *more;           /* Pointer to the rest of a large body of objects */
    object_t     *head;           /* Points to the main object of a large body */
    map_t  *map;            /* Pointer to the map in which this object is present */

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
    shstr_t     *name;           /* The name of the object, obviously... */
    shstr_t     *title;          /* Of foo, etc */
    shstr_t     *race;           /* human, goblin, dragon, etc */
    shstr_t     *slaying;        /* Which race to do double damage to
                                     * If this is an exit, this is the filename */
    shstr_t     *msg;            /* If this is a book/sign/magic mouth/etc */

    /* here starts copy_object() releated data */

    /* these are some internals */
    archetype_t   *arch;           /* Pointer to archetype */
    objectlink_t   *randomitems;    /* thats now a linked list of treasurelist */

    /* we can remove chosen_skill & skillgroup by drop here a uint8 with a list of skill
     * numbers. Mobs has no skill and player can grap it from player struct. For exp,
     * i will use skill numbers in golems/ammo and spell objects. So, this can be removed.
     */
    object_t     *chosen_skill;   /* the skill chosen to use */
    object_t     *skillgroup;        /* the exp. obj (category) assoc. w/ this object_t */

    /* now "real" object releated data */
    archetype_t   *other_arch;     /* Pointer used for various things */
    New_Face       *face;           /* struct ptr to the 'face' - the picture(s) */
    New_Face       *inv_face;       /* struct ptr to the inventory 'face' - the picture(s) */
    void           *custom_attrset;     /* Type-dependant extra data. */
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
    object_t     *enemy;          /* Monster/player to follow even if not closest */
    /* (only used by containers now =) */
    tag_t           attacked_by_count;  /* the tag of attacker, so we can be sure */
    object_t     *attacked_by;    /* This object start to attack us! only player & monster */
    tag_t           owner_count;    /* What count the owner had (in case owner has been freed) */
    object_t     *owner;          /* Pointer to the object which controls this one
                                     * Owner should not be referred to directly
                                     * - get_owner() should be used instead.
                                     */
    object_t     *original;       /* Used to store the original, unbuffed version of an item. A buffed
                                     * item will revert to this state when it is saved. */
    uint8           buffed;         // Whether or not the object should contain any BUFF_FORCEs

    /* *map is part of "object head" but this not? hmm */
    sint16          x;              /* X-Position in the map for this object_t */
    sint16          y;              /* Y-Position in the map for this object_t */
#ifdef USE_TILESTRETCHER
    sint16          z;              /* Z-Position in the map (in pixels) for this object_t */
#endif

    uint16          path_attuned;   /* Paths the object is attuned to */
    uint16          path_repelled;  /* Paths the object is repelled from */
    uint16          path_denied;    /* Paths the object is denied access to */

    uint16          last_damage;    /* thats the damage send with map2 */

    uint16          terrain_type;   /* type flags for different enviroment (tile is under water, firewalk,...)
                                     * A object which can be applied GIVES this terrain flags to his owner
                                     */
    uint16          terrain_flag;   /* The object can move over/is unaffected from this terrain type */
    uint16          block_movement; /* The object blocks objects moving in this direction from it's tile */
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

    sint8           max_buffs;          // How many buffs can this item support?

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
    float           speed;              /* The overall speed of this object_t */
    float           speed_left;         /* How much speed is left to spend this round */
    float           weapon_speed;       /* new weapon speed system. swing of weapon */
    float           weapon_speed_left;
    living_t          stats;              /* object stats like hp, sp, grace ... */
#ifdef POSITION_DEBUG
    sint16          ox, oy;             /* For debugging: Where it was last inserted */
#endif
};
#endif

#define CONTR(ob) ((player_t *)((ob)->custom_attrset))

/* Used by update_object to know if the object being passed is
 * being added or removed.
 */
#define UP_OBJ_INSERT   1   /* object was inserted in a map */
#define UP_OBJ_REMOVE   2   /* object was removed from a map tile */
#define UP_OBJ_FLAGS    3   /* critical object flags has been changed, rebuild tile flags but NOT increase tile counter */
#define UP_OBJ_FACE     4   /* Only thing that changed was the face */
#define UP_OBJ_FLAGFACE 5   /* update flags & face (means increase tile update counter */
#define UP_OBJ_SLICE    6   /* object layer was changed, rebuild layer systen - used from invisible for example */
#define UP_OBJ_ALL      7   /* force full update */

/* Macro for the often used object validity test (verify an pointer/count pair) */
#define OBJECT_VALID(_ob_, _count_) ((_ob_) && (_ob_)->count == ((tag_t)_count_) && !QUERY_FLAG((_ob_), FLAG_REMOVED) && !OBJECT_FREE(_ob_))

/* Less strict version of above */
#define OBJECT_VALID_OR_REMOVED(_ob_, _count_) ((_ob_) && (_ob_)->count == ((tag_t)_count_) && !OBJECT_FREE(_ob_))

/* Test the object is not removed nor freed - but no count test */
#define OBJECT_ACTIVE(_ob_) (!QUERY_FLAG((_ob_),FLAG_REMOVED) && !OBJECT_FREE(_ob_))

/* Test if an object is in the free-list */
#define OBJECT_FREE(_ob_) ((_ob_)->count==0 && CHUNK_FREE(_ob_))

/* These are flags passed to insert_ob_in_map() and insert_ob_in_ob(). */
#define INS_NO_MERGE       (1 << 0)
#define INS_NO_WALK_ON     (1 << 1)
/* used intern from insert_xx to track multiarch problems - don't use! */
#define INS_TAIL_MARKER    (1 << 2)

/* Waypoint macros */
#define WP_MOVE_TRIES 4 /* number of retries to get closer to (local) target before giving up */

/*find_next_object modes */
#define FNO_MODE_INV_ONLY   0
#define FNO_MODE_CONTAINERS 1
#define FNO_MODE_ALL        2

/* Goes through I's /full/ inv. At the end O either is the last item in the inv
 * or NULL if I has no inv. */
#define GET_INV_BOTTOM(I, O) \
        for ((O) = (I)->inv; (O) && (O)->below; (O) = (O)->below)

#define BUFF_ADD_SUCCESS      1  // Nothing went wrong.
#define BUFF_ADD_EXISTS       2  // The buff exists, but that does not mean it failed.
#define BUFF_ADD_LIMITED      4  // Too many of the same buff.
#define BUFF_ADD_MAX_EXCEEDED 8  // item->max_buffs exceeded.
#define BUFF_ADD_BAD_PARAMS   16 // item or buff == NULL
#define BUFF_ADD_NO_INSERT    32 // Something went wrong in insert_ob_in_ob

#define MODE_INVENTORY    0
#define MODE_NO_INVENTORY 1

/* OBJECT_FULLY_IDENTIFY() sets the object's IDENTIFIED flag and sets or clears
 * the KNOWN_* flags as appropriate to fully identify it. */
#define OBJECT_FULLY_IDENTIFY(_O_) \
    SET_FLAG((_O_), FLAG_IDENTIFIED); \
    SET_OR_CLEAR_FLAG((_O_), FLAG_KNOWN_MAGICAL, (QUERY_FLAG((_O_), FLAG_IS_MAGICAL))); \
    SET_OR_CLEAR_FLAG((_O_), FLAG_KNOWN_CURSED, (QUERY_FLAG((_O_), FLAG_CURSED) || QUERY_FLAG((_O_), FLAG_DAMNED)));

#ifndef USE_OLD_UPDATE
/* The OBJECT_UPDATE_*() macros are intended to make the process of updating
 * the server and/or relevant clients as simple, foolproof, and fast as
 * possible.
 *
 * These macros operate on single objects only. So for a multipart it is the
 * caller's responsibility to work over the head and body as necessary by, for
 * example, embedding the macro in a FOREACH_PART_OF_OBJECT() loop. Remember
 * that multiparts in envs are decapitated anyway and see the UPD comment
 * below. */
/* OBJECT_UPDATE_INS() is used only when an object is (re)inserted into a map
 * or env as by insert_ob_in_map() and insert_ob_in_ob(). As such only the
 * choice to send client data is handled in the macro. */
#define OBJECT_UPDATE_INS(_O_) \
    if (!QUERY_FLAG((_O_), FLAG_NO_SEND)) \
    { \
        esrv_send_item((_O_)); \
    }

/* OBJECT_UPDATE_REM() is used only when an object is removed from a map or env
 * as by remove_ob(). As such only the choice to delete client data is handled
 * in the macro. */
#define OBJECT_UPDATE_REM(_O_) \
    if (!QUERY_FLAG((_O_), FLAG_NO_SEND)) \
    { \
        esrv_del_item((_O_)); \
    }

/* OBJECT_UPDATE_VIS() is used when an object changes some form of visibility.
 * The server handling, while not complicated, is a bit trick to pin down to
 * consistent code. As such only the choice to send client data is handled in
 * the macro. */
#define OBJECT_UPDATE_VIS(_O_) \
    if (!QUERY_FLAG((_O_), FLAG_NO_SEND)) \
    { \
        esrv_send_or_del_item((_O_)); \
    }

/* OBJECT_UPDATE_UPD() is used when an object changes in some way (other than
 * visibility). _F_ is some combination of the UPD_* flags. These manage which
 * data is updated to the client. Also, if _O_->map and _F_ & UPD_SERVERFLAGS,
 * the msp has its flags rebuilt.
 *
 * In at least MOST cases only the head of multiparts needs updating */
#define OBJECT_UPDATE_UPD(_O_, _F_) \
    if ((_O_)->map && \
        ((_F_) & UPD_SERVERFLAGS)) \
    { \
        MSP_UPDATE(MSP_KNOWN((_O_)), (_O_)) \
    } \
    if (!QUERY_FLAG((_O_), FLAG_NO_SEND)) \
    { \
        esrv_update_item(((_F_) & ~UPD_SERVERFLAGS), (_O_)); \
    }

/* OBJECT_REQUIRES_MSP_UPDATE() queries whether _O_ is of a type or has certain
 * flags which will necessitate an msp update. Note that this is only used in 3
 * cases: when a map is freshly loaded into memory (in this case updating is
 * deferred until all map objects have been inserted and then at the end each
 * msp is updated once for each object on it which necessitates an update); by
 * insert_ob_on_map() (which causes a single update based on _O_only; and by
 * remove_ob() (which causes a complete rebuilding of the msp's flags based on
 * all remaining objects on the msp. */
#define OBJECT_REQUIRES_MSP_UPDATE(_O_) \
    (((_O_)->type == CHECK_INV || \
      (_O_)->type == MAGIC_EAR || \
      (_O_)->type == GRAVESTONE || \
      QUERY_FLAG((_O_), FLAG_ALIVE) || \
      QUERY_FLAG((_O_), FLAG_IS_PLAYER) || \
      QUERY_FLAG((_O_), FLAG_OBSCURESVIEW) || \
      QUERY_FLAG((_O_), FLAG_ALLOWSVIEW) || \
      QUERY_FLAG((_O_), FLAG_BLOCKSVIEW) || \
      QUERY_FLAG((_O_), FLAG_DOOR_CLOSED) || \
      QUERY_FLAG((_O_), FLAG_PASS_THRU) || \
      QUERY_FLAG((_O_), FLAG_PASS_ETHEREAL) || \
      QUERY_FLAG((_O_), FLAG_NO_PASS) || \
      QUERY_FLAG((_O_), FLAG_NO_SPELLS) || \
      QUERY_FLAG((_O_), FLAG_NO_PRAYERS) || \
      QUERY_FLAG((_O_), FLAG_WALK_ON) || \
      QUERY_FLAG((_O_), FLAG_FLY_ON) || \
      QUERY_FLAG((_O_), FLAG_WALK_OFF) || \
      QUERY_FLAG((_O_), FLAG_FLY_OFF) || \
      QUERY_FLAG((_O_), FLAG_REFL_CASTABLE) || \
      QUERY_FLAG((_O_), FLAG_REFL_MISSILE)) ? 1 : 0)
#endif

#endif /* ifndef __OBJECT_H */

#ifndef __PETS_H
#define __PETS_H

#define MAX_PETS 2      /* Maximum number of pets at any time */
#define MAX_PERMAPETS 2 /* Maximum number of non-temporary pets */

#define PET_VALID(pet_ol, _owner_) \
    (OBJECT_VALID((pet_ol)->objlink.ob, (pet_ol)->id) && \
     (pet_ol)->objlink.ob->owner == (_owner_) && (pet_ol)->objlink.ob->owner_count == (_owner_)->count)

#endif /* ifndef __PETS_H */
