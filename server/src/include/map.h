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

#ifndef __TILING_H
#define __TILING_H

/* The OUT_OF_(REAL_)MAP() macros are *really* important. They basically allow
 * tiling to work. As with all macros there is no inherent type-checking of the
 * parameters so the caller must ensure they're sensible on entry (M is
 * non-NULL map_t *, X and Y are sint16). */
/* OUT_OF_REAL_MAP() is true if X or Y are not within M's width and height
 * respectively, or false otherwise. This simply calculates if we are looking
 * at an msp in M or not. Usually that is only half the story and if we aren't,
 * we then want to know where our coordinates point on the tileset (see
 * OUT_OF_MAP()). */
#define OUT_OF_REAL_MAP(_M_, _X_, _Y_) \
    ((_X_) < 0 || \
     (_X_) >= MAP_WIDTH((_M_)) || \
     (_Y_) < 0 || \
     (_Y_) >= MAP_HEIGHT((_M_)))

/* OUT_OF_MAP() checks to which map the coordinates M, X, Y point, loading the
 * map and updating M, X, Y as necessary. Note that this means that M, X, Y may
 * be different values after the macro than before: if the coordinates point
 * to an msp not in any valid map, M will be NULL on exit, X and Y unchanged;
 * if the coordinates point to an msp on some tiled map, all are updated to
 * this msp. The possibly updated value of M is returned. */
#if 0
#define OUT_OF_MAP(_M_, _X_, _Y_) \
    ((!LOG(llevDebug, ">>>>FILE=%s,%d IN=%s,%d,%d", __FILE__, __LINE__, STRING_MAP_PATH((_M_)), (_X_), (_Y_)) && \
      (!OUT_OF_REAL_MAP((_M_), (_X_), (_Y_)) || \
       ((_M_) = out_of_map((_M_), &(_X_), &(_Y_)))) && \
      !LOG(llevDebug, " OUT=%s,%d,%d\n", STRING_MAP_PATH((_M_)), (_X_), (_Y_))) \
     ? (_M_) : NULL)
#else
#define OUT_OF_MAP(_M_, _X_, _Y_) \
    ((!OUT_OF_REAL_MAP((_M_), (_X_), (_Y_)) || \
      ((_M_) = out_of_map((_M_), &(_X_), &(_Y_)))) ? (_M_) : NULL)
#endif

/* OUT_OF_MAP2() is similar to OUT_OF_MAP() but won't load tiled maps not yet
 * in memory. It's only really intended for things like managing light masks.
 * You should use OUT_OF_MAP(). */
#define OUT_OF_MAP2(_M_, _X_, _Y_) \
    ((!OUT_OF_REAL_MAP((_M_), (_X_), (_Y_)) || \
      ((_M_) = out_of_map2((_M_), &(_X_), &(_Y_)))) ? (_M_) : NULL)

enum tiling_direction_t
{
    TILING_DIRECTION_NORTH,
    TILING_DIRECTION_EAST,
    TILING_DIRECTION_SOUTH,
    TILING_DIRECTION_WEST,
    TILING_DIRECTION_NORTHEAST,
    TILING_DIRECTION_SOUTHEAST,
    TILING_DIRECTION_SOUTHWEST,
    TILING_DIRECTION_NORTHWEST,
    TILING_DIRECTION_NROF
};

struct tiling_t
{
    shstr_t       *tile_path[TILING_DIRECTION_NROF];  /* path to adjoining maps (shared strings) */
    shstr_t       *orig_tile_path[TILING_DIRECTION_NROF]; /* sic! */
    map_t         *tile_map[TILING_DIRECTION_NROF];   /* Next map, linked list */
    uint32         tileset_id;              /* All map tiles that are connected to each other with tiling
                                               are on the same tileset. 0 means unknown */
    sint16         tileset_x;    /* This maps coordinates in the tileset. Only valid if tileset_id > 0 */
    sint16         tileset_y;    /* This maps coordinates in the tileset. Only valid if tileset_id > 0 */
};

extern map_t  *out_of_map(map_t *m, sint16 *x, sint16 *y);
extern map_t  *out_of_map2(map_t *m, sint16 *x, sint16 *y);
extern sint8   on_same_tileset(map_t *m1, map_t *m2);
static sint8   on_same_instance(map_t *m1, map_t *m2);

#endif /* ifndef __TILING_H */

#ifndef __RV_H
#define __RV_H

/* These flags are used in the flags parameter of rv_get() (in practical terms
 * that really means as the final (_F_) input of the RV_GET_*() macros).
 * Although these are used and set up as a bitmask, in fact the lower few
 * (*_D) behave more as a mode. That is, where multiple such flags are given
 * only the first one found (in the order they are defined below, which is
 * according to speed from slow to fast) is used. If none are given, neither
 * distance nor direction are calculated, which i s obviously fastest. */
#define RV_FLAG_EUCLIDIAN_D       (1 << 0) // straight line 
#define RV_FLAG_FAST_EUCLIDIAN_D  (1 << 1) // squared straight line
#define RV_FLAG_MANHATTAN_D       (1 << 2) // manhattan
#define RV_FLAG_DIAGONAL_D        (1 << 3) // diagonal
/* (1 << 4) is free. */
/* (1 << 5) is free. */
#define RV_FLAG_IGNORE_MULTIPART  (1 << 6) // don't translate for closest body part
#define RV_FLAG_RECURSIVE_SEARCH  (1 << 7) // needs work -- DO NOT USE

/* Maximum number of tiles to search in relative_tile_position() before giving up */
/* 8 => 1 level deep, 24 => 2 levels, 48 =>3 levels */
#define MAX_SEARCH_MAP_TILES 24

/* RV_GET_OBJ_TO_OBJ() is a convenience macro for calculating the rv between
 * two objects. To keep the macro simple and embedable in conditions, it is the
 * caller's reponsibility to ensure both objects exist and are on maps.*/
#define RV_GET_OBJ_TO_OBJ(_O1_, _O2_, _RV_, _F_) \
    rv_get((_O1_), MSP_KNOWN((_O1_)), (_O2_), MSP_KNOWN((_O2_)), (_RV_), (_F_))

/* RV_GET_MSP_TO_MSP() is a convenience macro for calculating the rv between
 * two msps. rv_get() will cope if either msp is NULL (out of map) but this is
 * a guaranteed immediate failure so really the caller should check first. */
#define RV_GET_MSP_TO_MSP(_MSP1_, _MSP2_, _RV_, _F_) \
    rv_get(NULL, (_MSP1_), NULL, (_MSP2_), (_RV_), (_F_))

/* RV_GET_OBJ_TO_MSP() is a convenience macro for calculating the rv between
 * two objects. To keep the macro simple and embedable in conditions, it is the
 * caller's reponsibility to ensure both objects exist and are on maps.*/
#define RV_GET_OBJ_TO_MSP(_O_, _MSP_, _RV_, _F_) \
    rv_get((_O_), MSP_KNOWN((_O_)), NULL, (_MSP_), (_RV_), (_F_))

/* RV_GET_OBJ_TO_OBJ() is a convenience macro for calculating the rv between
 * two objects. To keep the macro simple and embedable in conditions, it is the
 * caller's reponsibility to ensure both objects exist and are on maps.*/
#define RV_GET_MSP_TO_OBJ(_MSP_, _O_, _RV_, _F_) \
    rv_get(NULL, (_MSP_), (_O_), MSP_KNOWN((_O_)), (_RV_), (_F_))

#define RV_TEST_MELEE(_RV_) \
    (ABS((_RV_).distance_x) < 2 && \
     ABS((_RV_).distance_y) < 2)

            /* exact 45 deg */
#define RV_TEST_MISSILE_EXACT(_RV_) \
    ((_RV_).distance_x == 0 || \
     (_RV_).distance_y == 0 || \
     ABS((_RV_).distance_x) - ABS((_RV_).distance_y) == 0)

          /* 45 deg +- one tile */
#define RV_TEST_MISSILE_APPROX(_RV_) \
    (ABS((_RV_).distance_x) <= 1 || \
     ABS((_RV_).distance_y) <= 1 || \
     ABS(ABS((_RV_).distance_x) - ABS((_RV_).distance_y)) <= 1)
/* This is used by get_rangevector to determine where the other
 * creature is.  get_rangevector takes into account map tiling,
 * so you just can not look the the map coordinates and get the
 * righte value.  distance_x/y are distance away, which
 * can be negativbe.  direction is the crossfire direction scheme
 * that the creature should head.  part is the part of the
 * monster that is closest.
 * Note: distance should be always >=0. I changed it to UINT. MT
 */
/* Unaligned: 20/24
 * Internal padding: 0/0
 * Trailing padding: 0/0
 * Cache line optimized: no
 * The four integer members could probably be uint16, sint16, sint16, sint8 (+1
 * padding for an overall saving of 8.
 *
 * -- Smacky 20140822 */
struct rv_t
{
    object_t     *head;
    object_t     *part;
    unsigned int  distance;
    int           distance_x;
    int           distance_y;
    int           direction;
};

struct rv_cache_t
{
    shstr_t *path; // cached map path (TODO: why path? why not just map?)
    sint16   x;    // cached x
    sint16   y;    // cached y
};

/* This is used when we try to find vectors between map tiles in relative_tile_position() */
struct mapsearch_node
{
    map_t              *map;
    int                     dx, dy;
    struct mapsearch_node  *next;
};

extern sint8 rv_get(object_t *op1, msp_t *msp1, object_t *op2, msp_t *msp2, rv_t *retval, uint8 flags);

#endif /* ifndef __RV_H */

#ifndef __MAP_H
#define __MAP_H

/* for server->client map protocol. Tell client how to handle its local map cache */
#define MAP_UPDATE_CMD_SAME 0
#define MAP_UPDATE_CMD_NEW 1
#define MAP_UPDATE_CMD_CONNECTED 2

#define MAP_INSTANCE_NUM_INVALID (-1)

/* definition flags of the instance - stored in the player normally */
#define MAP_INSTANCE_FLAG_NO_REENTER 1

/* Default values for a few non-zero attributes. */
#define MAP_DEFAULT_WIDTH      24
#define MAP_DEFAULT_HEIGHT     24
#define MAP_DEFAULT_DIFFICULTY 1
#define MAP_DEFAULT_DARKNESS   0

/* Values for in_memory below.  Should probably be an enumerations */
enum map_memory_t
{
    MAP_MEMORY_FREED,
    MAP_MEMORY_ACTIVE,
    MAP_MEMORY_SWAPPED,
    MAP_MEMORY_LOADING,
    MAP_MEMORY_SAVING
};

/* Map status values for map_t.status.
 *
 * Map status holds info on how the map is loaded, whether a manual
 * reset/reload has been requested, and so on. Note that this does not hold
 * info on whether/how the map is in memory -- for that see map_memory_t. */
#define MAP_STATUS_MULTI        (1 << 0)  // normal multiuser map (saved in /tmp)
#define MAP_STATUS_UNIQUE       (1 << 1)  // like apartment - (map is inside /players folder)
#define MAP_STATUS_INSTANCE     (1 << 2)  // this is an instance (map is inside /instance)
#define MAP_STATUS_STYLE        (1 << 3)  // we load a special random map style map - we don't set speed for objects inside!
#define MAP_STATUS_ORIGINAL     (1 << 4)  // map is an original map - generate treasures!
#define MAP_STATUS_NO_FALLBACK  (1 << 5) // when map loading fails, don't try to load savebed, emergency or another fallback map
#define MAP_STATUS_LOAD_ONLY    (1 << 6) // signal for map loader func to return after successful read_map_name()
#define MAP_STATUS_FIXED_LOGIN  (1 << 7) // same as MAP_FIXEDLOGIN() - useful for dynamic setting
#define MAP_STATUS_MANUAL_RESET (1 << 8) // a gmaster/script has scheduled this map to reset
#define MAP_STATUS_RELOAD       (1 << 9) // this map will reload (only has meaning on a manual reset)

/* Map flag values for map_t.flags. */
#define MAP_FLAG_OUTDOOR       (1 << 0)  // outdoor map - daytime effects are on
#define MAP_FLAG_NO_SAVE       (1 << 1)  // don't save maps - atm only used with unique maps
#define MAP_FLAG_FIXED_RTIME   (1 << 2)  // reset time is not affected by players entering/exiting map
#define MAP_FLAG_NO_SPELLS     (1 << 3)  // no spells
#define MAP_FLAG_NO_PRAYERS    (1 << 4)  // no prayers
#define MAP_FLAG_NO_HARM       (1 << 5)  // no harmful spells/prayers
#define MAP_FLAG_NO_SUMMON     (1 << 6)  // no summoning spells/prayers
#define MAP_FLAG_FIXED_LOGIN   (1 << 7)  // player login forced to enter_x/enter_y
#define MAP_FLAG_PERMDEATH     (1 << 8)  // perm death map
#define MAP_FLAG_ULTRADEATH    (1 << 9)  // ultra death map
#define MAP_FLAG_ULTIMATEDEATH (1 << 10) // ultimate death map
#define MAP_FLAG_PVP           (1 << 11) // PvP is possible on this map

#ifndef USE_OLD_UPDATE
#else
/* These are special flags used internally by certain functions to manage the
 * use of the other map flags. */
#define MAP_FLAG_NO_UPDATE     (1 << 31) // ?
#endif

/* Macros to access the map_t.
 *
 * In general, code should always use these macros (or functions in map.c) to
 * access many of the values in the map structure. Failure to do this will
 * almost certainly break various features. You may think it is safe to look at
 * width and height values directly (or even through the macros), but doing so
 * will completely break map tiling. */
/* I don't really understand the above comment. Especially the last sentence
 * just isn't true. Giving it the benefit of the doubt, perhaps the comment
 * predates out_of_map() and the concern was that code would try to fudge that
 * functionality directly? Still, lets use the macros (except in map.c) as they
 * improve code readability.
 *
 * -- Smacky 20140811 */
#define MAP_STATUS_TYPE(_F_)     ((_F_) & (MAP_STATUS_MULTI | MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE | MAP_STATUS_STYLE))
#define MAP_FIXED_RESETTIME(_M_) ((_M_)->flags & MAP_FLAG_FIXED_RTIME)
#define MAP_NOSAVE(_M_)          ((_M_)->flags & MAP_FLAG_NO_SAVE)
#define MAP_FIXEDLOGIN(_M_)      ((_M_)->flags & MAP_FLAG_FIXED_LOGIN)
#define MAP_WHEN_SWAP(_M_)       (_M_)->swap_time
#define MAP_SWAP_TIMEOUT(_M_)    (_M_)->swap_timeout
#define MAP_WHEN_RESET(_M_)      (_M_)->reset_time
#define MAP_RESET_TIMEOUT(_M_)   (_M_)->reset_timeout
#define MAP_DIFFICULTY(_M_)      (_M_)->difficulty
#define MAP_DARKNESS(_M_)        (_M_)->ambient_darkness
#define MAP_LIGHT_VALUE(_M_)     (_M_)->ambient_brightness
#define MAP_WIDTH(_M_)           (_M_)->width
#define MAP_HEIGHT(_M_)          (_M_)->height
#define MAP_SIZE(_M_)            (MAP_WIDTH(_M_) * MAP_HEIGHT(_M_))
#define MAP_ENTER_X(_M_)         (_M_)->enter_x
#define MAP_ENTER_Y(_M_)         (_M_)->enter_y

/* MAP_SET_WHEN_SWAP() sets the swap time for M to the absolute value of T
 * seconds when T is non-zero, or never when T is zero. */
#define MAP_SET_WHEN_SWAP(_M_, _T_) \
    if ((_T_) > 0) \
    { \
        MAP_WHEN_SWAP((_M_)) = (ROUND_TAG - ROUND_TAG % \
                                (long unsigned int)MAX(1, pticks_second)) / \
                               pticks_second + (_T_); \
    } \
    else if ((_T_) == 0) \
    { \
        MAP_WHEN_SWAP((_M_)) = 0; \
    } \
    else \
    { \
        MAP_WHEN_SWAP((_M_)) = (ROUND_TAG - ROUND_TAG % \
                                (long unsigned int)MAX(1, pticks_second)) / \
                               pticks_second - (_T_); \
    }

/* MAP_SET_WHEN_RESET() sets the reset time for M to T seconds when T is
 * positive, or immediate when T is negative, or never when T is zero. */
#define MAP_SET_WHEN_RESET(_M_, _T_) \
    if ((_T_) > 0) \
    { \
        MAP_WHEN_RESET((_M_)) = (ROUND_TAG - ROUND_TAG % \
                                 (long unsigned int)MAX(1, pticks_second)) / \
                                pticks_second + (_T_); \
    } \
    else if ((_T_) == 0) \
    { \
        MAP_WHEN_RESET((_M_)) = 0; \
    } \
    else \
    { \
        MAP_WHEN_RESET((_M_)) = (ROUND_TAG - ROUND_TAG % \
                                 (long unsigned int)MAX(1, pticks_second)) / \
                                pticks_second; \
    }

/* MAP_SET_PLAYER_MAP_INFO_DEFAULT() sets the map info fields in _PL_'s player
 * structure to default values. */
#define MAP_SET_PLAYER_MAP_INFO_DEFAULT(_PL_) \
    SHSTR_FREE_AND_ADD_REF((_PL_)->maplevel, shstr_cons.start_mappath); \
    SHSTR_FREE_AND_ADD_REF((_PL_)->orig_map, shstr_cons.start_mappath); \
    (_PL_)->status = FALLBACK_START_MAP_STATUS; \
    (_PL_)->map_x = FALLBACK_START_MAP_X; \
    (_PL_)->map_y = FALLBACK_START_MAP_Y;

/* MAP_SET_PLAYER_MAP_INFO_CURRENT() sets the map info fields in _PL_'s player
 * structure to the current map's values. */
#define MAP_SET_PLAYER_MAP_INFO_CURRENT(_PL_) \
    SHSTR_FREE_AND_ADD_REF((_PL_)->maplevel, (_PL_)->ob->map->path); \
    SHSTR_FREE_AND_ADD_REF((_PL_)->orig_map, (_PL_)->ob->map->orig_path); \
    (_PL_)->status = (_PL_)->ob->map->status; \
    (_PL_)->map_x = (_PL_)->ob->x; \
    (_PL_)->map_y = (_PL_)->ob->y;

/* MAP_SET_PLAYER_BED_INFO_DEFAULT() sets the bed info fields in _PL_'s player
 * structure to default values. */
#define MAP_SET_PLAYER_BED_INFO_DEFAULT(_PL_) \
    SHSTR_FREE_AND_ADD_REF((_PL_)->savebed_map, shstr_cons.bind_mappath); \
    SHSTR_FREE_AND_ADD_REF((_PL_)->orig_savebed_map, shstr_cons.bind_mappath); \
    (_PL_)->bed_status = BIND_MAP_STATUS; \
    (_PL_)->bed_x = BIND_MAP_X; \
    (_PL_)->bed_y = BIND_MAP_Y;

/* MAP_SET_PLAYER_BED_INFO_CURRENT() sets the bed info fields in _PL_'s player
 * structure to the current map's values. */
#define MAP_SET_PLAYER_BED_INFO_CURRENT(_PL_) \
    SHSTR_FREE_AND_ADD_REF((_PL_)->savebed_map, (_PL_)->ob->map->path); \
    SHSTR_FREE_AND_ADD_REF((_PL_)->orig_savebed_map, (_PL_)->ob->map->orig_path); \
    (_PL_)->bed_status = (_PL_)->ob->map->status; \
    (_PL_)->bed_x = (_PL_)->ob->x; \
    (_PL_)->bed_y = (_PL_)->ob->y;

/* Unaligned: 248/412
 * Internal padding: 0/0
 * Trailing padding: 0/4
 * Cache line optimized: no
 * Members like difficulty, height, width and ambient_darkness can probably be
 * reduced from uint16 to uint8, tileset_id from uint32 to uint16, and
 * cached_dist_x and cached_dist_y from sint32 to sint16.
 *
 * -- Smacky 20140822 */
struct map_t
{
    map_t         *last;                   /* map before, if NULL we are first_map */
    map_t         *next;                   /* Next map, linked list */
    msp_t         *spaces;                 /* Array of spaces on this map */
    msp_t         *first_light;            /* list of tiles spaces with light sources in */
    objectlink_t  *buttons;                /* Linked list of linked lists of buttons */
    uint32        *bitmap;                 /* Bitmap used for marking visited tiles in pathfinding */
    char          *tmpname;                /* Name of temporary file (note: This will be by nature a unique
                                             * string - using our hash string system would be senseless.
                                             * Option is to add is as array, but then it must be from strlen
                                             * MAXPATHLEN which is 256 bytes or more - but normally this
                                             * path string is only around 20 bytes long. So we use malloc() ) */
    shstr_t       *name;                   /* Name of map as given by its creator */
    shstr_t       *music;                  /* Filename of background music file */
    shstr_t       *msg;                    /* Message map creator may have left */
    shstr_t       *path;                   /* Filename of the map (shared string now) */
    shstr_t       *orig_path;              /* same as above but the original pathes - non unique or instanced */
    tiling_t       tiling;
    rv_cache_t     rv_cache;
    shstr_t       *reference;              /* Reference for unique or instance maps (a player name) */
    objectlink_t  *linked_spawn_list;      /* list pointer of the linked list */
    object_t      *player_first;           /* chained list of player on this map */
    object_t      *active_objects;         /* linked list of active objects */
    timeanddate_t *tadnow;
    sint32         tadoffset;
    /* See comment in map.c:GetLinkedMap(). */
    tag_t          tag;                    /* Unique identifier of this map object. Same as object tags */
    uint32         map_tag;                /* to identify maps for fixed_login */
    map_memory_t   in_memory;              /* If not true, the map has been freed and must
                                            * be loaded before used.  The map,omap and map_ob
                                            * arrays will be allocated when the map is loaded */

    uint32         status;             /* flags for the internal status and use of the map */
    uint32         flags;              /* mag flags for various map settings (set from outside) */
    uint32         swap_time;              /* when this map should be swapped */
    uint32         swap_timeout;           /* How many seconds must elapse before this map should be swapped */
    uint32         reset_time;             /* when this map should reset */
    uint32         reset_timeout;          /* How many seconds must elapse before this map should be reset */
    uint32         pathfinding_id;         /* For which traversal bitmap is valid */
    uint32         traversed;               /* Used by relative_tile_position() to mark visited maps */
    uint16         perm_load;               /* This is a counter - used for example from NPC's which have
                                             * and the npc/object with perm_load flag will stay in game.
                                             */
    uint16         difficulty;              /* What level the player should be to play here */
    uint16         height;                  /* Width and height of map. */
    uint16         width;
    sint16         enter_x;                 /* enter_x and enter_y are default entrance location */
    sint16         enter_y;                 /* on the map if none are set in the exit */
    sint16         ambient_brightness;     // the brightness when this map is not marked as outdoors
    sint16         ambient_darkness;       // the darkness when this map is not marked as outdoors
};

extern map_t  *map_is_in_memory(shstr_t *name);
extern map_t  *map_is_ready(shstr_t *path_sh);
extern char   *create_mapdir_pathname(const char *name);
extern sint8   check_path(const char *name, uint8 prepend_dir);
extern char   *normalize_path(const char *src, const char *dst, char *path);
extern char   *normalize_path_direct(const char *src, const char *dst, char *path);
extern map_t  *ready_inherited_map(map_t *orig_map, shstr_t *new_map_path);
extern map_t  *map_save(map_t *m);
extern void    delete_map(map_t *m);
extern shstr_t  *create_safe_path_sh(const char *path);
extern shstr_t  *create_unique_path_sh(shstr_t *reference, shstr_t *orig_path_sh);
extern shstr_t  *create_instance_path_sh(player_t *pl, shstr_t *orig_path_sh, uint32 flags);
extern map_t  *ready_map_name(shstr_t *path_sh, shstr_t *orig_path_sh, uint32 flags, shstr_t *reference);
extern void    clean_tmp_map(map_t *m);
extern void    free_all_maps(void);
extern uint16  map_player_unlink(map_t *m, shstr_t *path_sh);
extern void    map_player_link(map_t *m, sint16 x, sint16 y, uint8 flag);
extern void    read_map_log(void);
extern void    swap_map(map_t *map, int force_flag);
extern void    map_check_in_memory(map_t *m);
extern int     players_on_map(map_t *m);
extern void    map_transfer_apartment_items(map_t *map_old, map_t *map_new, sint16 x, sint16 y);

#endif /* ifndef __MAP_H */

#ifndef __MSP_H
#define __MSP_H

/* MSP_RAW() calculates the msp at M, X, Y. It is the caller's responsibility
 * to check that M, X, Y are valid (that means X, Y are absolute coordinates to
 * a square within M -- no bounday checking or tile loading is done). */
#define MSP_RAW(_M_, _X_, _Y_) \
    (&((_M_)->spaces[(_X_) + MAP_WIDTH((_M_)) * (_Y_)]))

/* MSP_GET() calls OUT_OF_MAP() to update M, X, Y as necessary then returns a
 * pointer to the resultant msp or NULL. As this calls OUT_OF_MAP(), M, X, Y
 * may be different on exit than entry. */
#define MSP_GET(_M_, _X_, _Y_) \
    (((_M_) && \
      OUT_OF_MAP((_M_), (_X_), (_Y_))) ? MSP_RAW((_M_), (_X_), (_Y_)) : NULL)

/* MSP_GET2() is similar to MSP_GET() but uses OUT_OF_MAP2() so does not load
 * nearby surrounding maps into memory. If the appropriate map is not already in
 * memory the return is NULL. */
#define MSP_GET2(_M_, _X_, _Y_) \
    (((_M_) && \
      OUT_OF_MAP2((_M_), (_X_), (_Y_))) ? MSP_RAW((_M_), (_X_), (_Y_)) : NULL)

/* MSP_KNOWN() is used instead of MSP_GET() where it is already known that the
 * msp is valid (guaranteed to not be out of map) but we just want to know what
 * it is (the return). This is always the case for an object on a map (O, it is
 * the caller's responsibility to check that O actually is on a map). */
#define MSP_KNOWN(_O_) \
    (MSP_RAW((_O_)->map, (_O_)->x, (_O_)->y))

/* MSP_GET_REAL_BRIGHTNESS() returns the actual brightness on _MSP_. This is a
 * combination of the flooding brightness from nearby light-emitting objects,
 * the floor brightness, and either the current daylight brightness or the
 * map's own general brightness depending on whether _MSP_ is in daylight or
 * not. */
#define MSP_GET_REAL_BRIGHTNESS(_MSP_) \
    ((_MSP_)->flooding_brightness + (_MSP_)->floor_brightness + \
     (((_MSP_)->flags & MSP_FLAG_DAYLIGHT) ? \
      (_MSP_)->map->tadnow->daylight_brightness : \
      (_MSP_)->map->ambient_brightness))

/* MSP_IS_APPROPRIATE_BRIGHTNESS() returns 1 or 0 to indicate if _MSP_ has an
 * appropriate brightness level (_B_). If _B_ is negative, we look for a
 * brightness on _MSP_ lower or equal to this absolute value. Otherwise, we
 * look for a brightness on _MSP_ higher or equal to this value. */
#define MSP_IS_APPROPRIATE_BRIGHTNESS(_MSP_, _B_) \
    (((_B_) < 0 && \
      MSP_GET_REAL_BRIGHTNESS((_MSP_)) <= ABS((_B_))) || \
     ((_B_) >= 0 && \
      MSP_GET_REAL_BRIGHTNESS((_MSP_)) >= (_B_)))

/* MSP_IS_RESTRICTED() returns 1 or 0 to indicate if _MSP_ has some flag which
 * might prevent entry. This is VERY general. For objects such as monsters and
 * players you should always use blocked(), not this, but it may be a
 * convenient and fast solution for spells, etc. */
/* TODO: This actually is not that useful since 0.10.7 so will likely be
 * removed some time.
 *
 * -- Smacky 20140515 */
#define MSP_IS_RESTRICTED(_MSP_) \
    (((_MSP_)->flags & (MSP_FLAG_DOOR_CLOSED | MSP_FLAG_PLAYER_ONLY | MSP_FLAG_NO_PASS | MSP_FLAG_PASS_THRU | MSP_FLAG_PASS_ETHEREAL)) ? 1 : 0)

/* MSP_GET_SYS_OBJ() sets _O_ to the first system object on _MSP_ that is of
 * type _T_. Layer 0/sys objs are always stored at the ->first end of a square
 * so we can loop through them by checking ->above and break out as soon as a
 * non-zero layer is encountered. If the required type is not found, _O_ is set
 * to NULL. */
#define MSP_GET_SYS_OBJ(_MSP_, _T_, _O_) \
    for ((_O_) = (_MSP_)->first; (_O_); (_O_) = (_O_)->above) \
    { \
        if ((_O_)->layer) \
        { \
            (_O_) = NULL; \
            break; \
        } \
        else if ((_O_)->type == (_T_)) \
        { \
            break; \
        } \
    }

#ifndef USE_OLD_UPDATE
#define MSP_UPDATE(_MSP_, _O_) \
    if (QUERY_FLAG((_O_), FLAG_NO_PASS)) \
    { \
        if ((_MSP_)->flags & MSP_FLAG_NO_PASS) \
        { \
            if (!QUERY_FLAG((_O_), FLAG_PASS_THRU)) \
            { \
                (_MSP_)->flags &= ~MSP_FLAG_PASS_THRU; \
            } \
            if (!QUERY_FLAG((_O_), FLAG_PASS_ETHEREAL)) \
            { \
                (_MSP_)->flags &= ~MSP_FLAG_PASS_ETHEREAL; \
            } \
        } \
        else \
        { \
            (_MSP_)->flags |= MSP_FLAG_NO_PASS; \
            if (QUERY_FLAG((_O_), FLAG_PASS_THRU)) \
            { \
                (_MSP_)->flags |= MSP_FLAG_PASS_THRU; \
            } \
            if (QUERY_FLAG((_O_), FLAG_PASS_ETHEREAL)) \
            { \
                (_MSP_)->flags |= MSP_FLAG_PASS_ETHEREAL; \
            } \
        } \
    } \
    else \
    { \
        if (!((_MSP_)->flags & MSP_FLAG_NO_PASS)) \
        { \
            if (QUERY_FLAG((_O_), FLAG_PASS_THRU)) \
            { \
                (_MSP_)->flags |= MSP_FLAG_PASS_THRU; \
            } \
            if (QUERY_FLAG((_O_), FLAG_PASS_ETHEREAL)) \
            { \
                (_MSP_)->flags |= MSP_FLAG_PASS_ETHEREAL; \
            } \
        } \
        else \
        { \
            (_MSP_)->flags &= ~MSP_FLAG_NO_PASS; \
            if (!QUERY_FLAG((_O_), FLAG_PASS_THRU)) \
            { \
                (_MSP_)->flags &= ~MSP_FLAG_PASS_THRU; \
            } \
            if (!QUERY_FLAG((_O_), FLAG_PASS_ETHEREAL)) \
            { \
                (_MSP_)->flags &= ~MSP_FLAG_PASS_ETHEREAL; \
            } \
        } \
    } \
    if ((_O_)->type == CHECK_INV) \
    { \
        (_MSP_)->flags |= MSP_FLAG_CHECK_INV; \
    } \
    else if ((_O_)->type == GRAVESTONE) \
    { \
        (_MSP_)->flags |= MSP_FLAG_PLAYER_GRAVE; \
    } \
    else if ((_O_)->type == MAGIC_EAR) \
    { \
        (_MSP_)->flags |= MSP_FLAG_MAGIC_EAR; \
    } \
    else \
    { \
        if (QUERY_FLAG((_O_), FLAG_IS_PLAYER)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_PLAYER; \
        } \
        if (QUERY_FLAG((_O_), FLAG_DOOR_CLOSED)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_DOOR_CLOSED; \
        } \
        if (QUERY_FLAG((_O_), FLAG_NO_SPELLS)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_NO_SPELLS; \
        } \
        if (QUERY_FLAG((_O_), FLAG_NO_PRAYERS)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_NO_PRAYERS; \
        } \
        if (QUERY_FLAG((_O_), FLAG_OBSCURESVIEW)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_OBSCURESVIEW; \
        } \
        if (QUERY_FLAG((_O_), FLAG_ALLOWSVIEW)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_ALLOWSVIEW; \
        } \
        if (QUERY_FLAG((_O_), FLAG_BLOCKSVIEW)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_BLOCKSVIEW; \
        } \
        if (QUERY_FLAG((_O_), FLAG_REFL_CASTABLE)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_REFL_CASTABLE; \
        } \
        if (QUERY_FLAG((_O_), FLAG_REFL_MISSILE)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_REFL_MISSILE; \
        } \
        if (QUERY_FLAG((_O_), FLAG_WALK_ON)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_WALK_ON; \
        } \
        if (QUERY_FLAG((_O_), FLAG_WALK_OFF)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_WALK_OFF; \
        } \
        if (QUERY_FLAG((_O_), FLAG_FLY_ON)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_FLY_ON; \
        } \
        if (QUERY_FLAG((_O_), FLAG_FLY_OFF)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_FLY_OFF; \
        } \
        if (QUERY_FLAG((_O_), FLAG_ALIVE)) \
        { \
            (_MSP_)->flags |= MSP_FLAG_ALIVE; \
            if((_O_)->type==MONSTER && \
               OBJECT_VALID((_O_)->owner, (_O_)->owner_count) && \
               (_O_)->owner->type == PLAYER) \
            { \
                (_MSP_)->flags |= MSP_FLAG_PLAYER_PET; \
            } \
        } \
    }
#else
#define MSP_SET_FLAGS_BY_OBJECT(_F_, _O_) \
    if ((_O_)->type == CHECK_INV) \
    { \
        (_F_) |= MSP_FLAG_CHECK_INV; \
    } \
    else if ((_O_)->type == GRAVESTONE) \
    { \
        (_F_) |= MSP_FLAG_PLAYER_GRAVE; \
    } \
    else if ((_O_)->type == MAGIC_EAR) \
    { \
        (_F_) |= MSP_FLAG_MAGIC_EAR; \
    } \
    else \
    { \
        if (QUERY_FLAG((_O_), FLAG_IS_PLAYER)) \
        { \
            (_F_) |= MSP_FLAG_PLAYER; \
        } \
        if (QUERY_FLAG((_O_), FLAG_DOOR_CLOSED)) \
        { \
            (_F_) |= MSP_FLAG_DOOR_CLOSED; \
        } \
        if (QUERY_FLAG((_O_), FLAG_NO_SPELLS)) \
        { \
            (_F_) |= MSP_FLAG_NO_SPELLS; \
        } \
        if (QUERY_FLAG((_O_), FLAG_NO_PRAYERS)) \
        { \
            (_F_) |= MSP_FLAG_NO_PRAYERS; \
        } \
        if (QUERY_FLAG((_O_), FLAG_OBSCURESVIEW)) \
        { \
            (_F_) |= MSP_FLAG_OBSCURESVIEW; \
        } \
        if (QUERY_FLAG((_O_), FLAG_ALLOWSVIEW)) \
        { \
            (_F_) |= MSP_FLAG_ALLOWSVIEW; \
        } \
        if (QUERY_FLAG((_O_), FLAG_BLOCKSVIEW)) \
        { \
            (_F_) |= MSP_FLAG_BLOCKSVIEW; \
        } \
        if (QUERY_FLAG((_O_), FLAG_REFL_CASTABLE)) \
        { \
            (_F_) |= MSP_FLAG_REFL_CASTABLE; \
        } \
        if (QUERY_FLAG((_O_), FLAG_REFL_MISSILE)) \
        { \
            (_F_) |= MSP_FLAG_REFL_MISSILE; \
        } \
        if (QUERY_FLAG((_O_), FLAG_WALK_ON)) \
        { \
            (_F_) |= MSP_FLAG_WALK_ON; \
        } \
        if (QUERY_FLAG((_O_), FLAG_WALK_OFF)) \
        { \
            (_F_) |= MSP_FLAG_WALK_OFF; \
        } \
        if (QUERY_FLAG((_O_), FLAG_FLY_ON)) \
        { \
            (_F_) |= MSP_FLAG_FLY_ON; \
        } \
        if (QUERY_FLAG((_O_), FLAG_FLY_OFF)) \
        { \
            (_F_) |= MSP_FLAG_FLY_OFF; \
        } \
        if (QUERY_FLAG((_O_), FLAG_ALIVE)) \
        { \
            (_F_) |= MSP_FLAG_ALIVE; \
            if((_O_)->type==MONSTER && \
               OBJECT_VALID((_O_)->owner, (_O_)->owner_count) && \
               (_O_)->owner->type == PLAYER) \
            { \
                (_F_) |= MSP_FLAG_PLAYER_PET; \
            } \
        } \
    }
#endif

/* used in blocked() when we only want know about blocked by something */
#define TERRAIN_ALL     0xffff

/* msp_t.flags mark various aspects of an msp, often according to one or more
 * objects on it.
 *
 * The first 16 bits are also potentially used in msp_t.floor_flags. */
#define MSP_FLAG_DAYLIGHT      (1 << 0)  // this msp is in daylight
#define MSP_FLAG_PVP           (1 << 1)  // PvP is allowed in this msp
#define MSP_FLAG_NO_SPELLS     (1 << 2)  // spells can't be cast on this msp
#define MSP_FLAG_NO_PRAYERS    (1 << 3)  // prayers can't be invoked on this msp
#define MSP_FLAG_NO_HARM       (1 << 4)  // no harmful castables on this msp
#define MSP_FLAG_PLAYER_ONLY   (1 << 5)  // only players can enter this msp (so no monsters, spell effects, thrown/fired items ,etc)
#define MSP_FLAG_NO_PASS       (1 << 6)  // nothing can enter this msp
#define MSP_FLAG_PASS_THRU     (1 << 7)  // only objects with FLAG_PASS_THRU can enter this msp -- overridden by MSP_FLAG_NO_PASS
#define MSP_FLAG_PASS_ETHEREAL (1 << 8)  // only objects with FLAG_ETHEREAL can enter this msp -- overridden by MSP_FLAG_NO_PASS
#define MSP_FLAG_CHECK_INV     (1 << 9)  // there is an inventory checker in this msp
#define MSP_FLAG_MAGIC_EAR     (1 << 10) // one or more magic ears are on this msp
#define MSP_FLAG_PLAYER_GRAVE  (1 << 11) // there is a gravestone on this msp
#define MSP_FLAG_ALIVE         (1 << 12) // there is one or more players or a monster on this msp
#define MSP_FLAG_PLAYER        (1 << 13) // there is one or more player on this msp
#define MSP_FLAG_PLAYER_PET    (1 << 14) // there is a pet on this msp
#define MSP_FLAG_DOOR_CLOSED   (1 << 15) // a closed door is blocking this msp
#define MSP_FLAG_REFL_CASTABLE (1 << 16) // one or more objects on this msp reflect spells/prayers
#define MSP_FLAG_REFL_MISSILE  (1 << 17) // one or more objects on this msp reflect missiles
#define MSP_FLAG_OBSCURESVIEW  (1 << 18) // one or more objects on this msp obscure los
#define MSP_FLAG_ALLOWSVIEW    (1 << 19) // one or more objects on this msp allow los
#define MSP_FLAG_BLOCKSVIEW    (1 << 20) // one or more objects on this msp block los
#define MSP_FLAG_WALK_ON       (1 << 21) // one or more objects on this msp react when something enters the msp by 'walking'
#define MSP_FLAG_WALK_OFF      (1 << 22) // one or more objects on this msp react when something leaves the msp by 'walking'
#define MSP_FLAG_FLY_ON        (1 << 23) // one or more objects on this msp react when something leaves the msp by flying
#define MSP_FLAG_FLY_OFF       (1 << 24) // one or more objects on this msp react when something enters the msp by flying
/* (1 << 25) is free */
/* (1 << 26) is free */
/* (1 << 27) is free */
/* (1 << 28) is free */
#ifndef USE_OLD_UPDATE
/* (1 << 29) is free */
/* (1 << 30) is free */
#define MSP_FLAG_OUT_OF_MAP    (1 << 31)
#else
/* These are special flags used internally by certain functions to manage the
 * use of the other MSP_FLAGs. */
#define MSP_FLAG_OUT_OF_MAP    (1 << 29)
#define MSP_FLAG_UPDATE        (1 << 30) // update the flags by looping the map objects
#define MSP_FLAG_NO_ERROR      (1 << 31) // msp_update (does not complain if the flags are different)
#endif

/* Slayers, or server layers, are used by the server to number object layers
 * (that is primarily msp_t.slayer and object_t.layer). */
enum msp_slayer_t
{
    MSP_SLAYER_SYSTEM,  // system object
    MSP_SLAYER_FLOOR,   // floor
    MSP_SLAYER_FMASK,   // floor mask
    MSP_SLAYER_ITEMA,   // pickable item
    MSP_SLAYER_ITEMB,   // non-pickable item/scenery
    MSP_SLAYER_FEATURE, // non-pickable, applyable map feature/scenery
#ifdef USE_SLAYER_MONSTER
    MSP_SLAYER_MONSTER, // monster
#endif
    MSP_SLAYER_PLAYER,  // player
    MSP_SLAYER_EFFECT,  // visual effect (eg, liquid, gas, magic, etc)
    MSP_SLAYER_NROF
};

/* Clayers, or client layers, are used to number the object 'layers' when
 * sending a MAP2 sc cmd. */
enum msp_clayer_t
{
    MSP_CLAYER_FLOOR, // floor
    MSP_CLAYER_FMASK, // floor mask
    MSP_CLAYER_UNDER, // 2nd-highest slayer object
    MSP_CLAYER_OVER,  // highest slayer object
    MSP_CLAYER_NROF
};

/* Each msp maintains MSP_SLICE_NROF lists of objects on it, or slices. */
enum msp_slice_t
{
    MSP_SLICE_VISIBLE,   // objects in the visible spectrum (ie, nearly all)
    MSP_SLICE_INVISIBLE, // also objects in the magical invisible spectrum
    MSP_SLICE_GMASTER,   // also invisible GM/SAs (ie, everything)
    MSP_SLICE_NROF
};

#define MSP_SLAYER_UNSLICED MSP_SLAYER_FMASK
#define MSP_CLAYER_UNSLICED MSP_CLAYER_FMASK

/* Unaligned: 196/356
 * Internal padding: 0/4
 * Trailing padding: 0/0
 * Cache line optimized: sort of
 *
 * -- Smacky 20140822 */
struct msp_t
{
    map_t    *map;
    sint16    x;
    sint16    y;
    uint32    flags;                          // flags about this space (see the MSP_FLAG_ values above)
    msp_t    *prev_light;                     // used to create chained light source list
    msp_t    *next_light;                     // used to create chained light source list
    object_t *first;                          // first object in this square
    object_t *last;                           // last object in this square
    object_t *slayer[MSP_SLICE_NROF][MSP_SLAYER_NROF - MSP_SLAYER_UNSLICED - 1]; // array of slayer objects
    object_t *clayer[MSP_SLICE_NROF][MSP_CLAYER_NROF - MSP_CLAYER_UNSLICED - 1]; // array of clayer objects
    New_Face *mask_face;                      // here we need the face for masks, because it can be turnable
    New_Face *floor_face;                     // here we need the face for the floor because it can be turnable
    sint32    light_source;                   // light source counter - as higher as brighter light source here
    sint32    flooding_brightness;            // how much brightness from neaby light-emitting objects
    uint32    round_tag;                      // tag for last_damage
    uint16    last_damage;                    // last_damage tmp backbuffer
    uint16    move_flags;                     // terrain type flags (water, underwater,...)
    uint16    floor_flags;                    // floor data: flags
    uint16    floor_terrain;
    uint16    floor_direction_block;          // direction blocking flag
    sint16    floor_darkness;
    sint16    floor_brightness;
#ifdef USE_TILESTRETCHER
    sint16    floor_z;                        // height of floor
#endif
    uint8     slices_synced;
};

extern void    msp_rebuild_slices_without(msp_t *msp, object_t *op);
extern void    msp_rebuild_slices_with(msp_t *msp, object_t *op);
#ifndef USE_OLD_UPDATE
#else
extern void    msp_update(map_t *m, msp_t *mspace, sint16 x, sint16 y);
#endif
extern uint32  msp_blocked(object_t *op, map_t *map, sint16 x, sint16 y);

#endif /* ifndef __MSP_H */

#ifndef __OVERLAY_H
#define __OVERLAY_H

#define OVERLAY_3X3 9
#define OVERLAY_5X5 25
#define OVERLAY_7X7 49
#define OVERLAY_9X9 81
#define OVERLAY_MAX OVERLAY_9X9

#define OVERLAY_IGNORE_TERRAIN  (1 << 0)
#define OVERLAY_WITHIN_LOS      (1 << 1)
#define OVERLAY_FORCE           (1 << 2)
#define OVERLAY_FIRST_AVAILABLE (1 << 3)
#define OVERLAY_FIXED           (1 << 4)
#define OVERLAY_RANDOM          (1 << 5)
#define OVERLAY_SPECIAL         (1 << 6)

extern sint8 overlay_x[OVERLAY_MAX];
extern sint8 overlay_y[OVERLAY_MAX];

#define OVERLAY_X(_V_) overlay_x[(_V_)]
#define OVERLAY_Y(_V_) overlay_y[(_V_)]

extern sint8  overlay_find_free(msp_t *msp, object_t *what, sint8 start, sint8 stop, uint8 ins_flags);
extern sint8  overlay_find_free_by_flags(msp_t *msp, object_t *who, uint8 oflags);
extern sint8  overlay_find_dir(msp_t *msp, object_t *exclude);
extern uint32 overlay_is_back_blocked(sint8 index, msp_t *msp, uint32 flags);

#endif /* ifndef __OVERLAY_H */
