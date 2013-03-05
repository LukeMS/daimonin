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

/*
 * The mapstruct is allocated each time a new map is opened.
 * It contains pointers (very indirectly) to all objects on the map.
 */

#ifndef __MAP_H
#define __MAP_H

#define MAP_INSTANCE_NUM_INVALID (-1)

/* definition flags of the instance - stored in the player normally */
#define INSTANCE_FLAG_NO_REENTER 1

#define NROF_SLAYERS 7   /* thats our 7 logical layers.
                               * ! for first and last object, we will use 2 more fake layers!
                               */
#define NROF_CLAYERS      4   /* thats our 4 physical layers we really show */

/* Default values for a few non-zero attributes. */
#define MAP_DEFAULT_WIDTH      24
#define MAP_DEFAULT_HEIGHT     24
#define MAP_DEFAULT_DIFFICULTY 1
#define MAP_DEFAULT_DARKNESS   0
#define MAP_MULTI(m)            ((m)->map_status & MAP_STATUS_MULTI)
#define MAP_UNIQUE(m)           ((m)->map_status & MAP_STATUS_UNIQUE)
#define MAP_INSTANCE(m)         ((m)->map_status & MAP_STATUS_INSTANCE)
#define MAP_WHEN_RESET(m)       ((m)->reset_time)
#define MAP_RESET_TIMEOUT(m)    ((m)->reset_timeout)
#define MAP_DIFFICULTY(m)       ((m)->difficulty)
#define MAP_WHEN_SWAP(m)        ((m)->swap_time)
#define MAP_SWAP_TIMEOUT(m)     ((m)->swap_timeout)
#define MAP_OUTDOORS(m)         ((m)->map_flags & MAP_FLAG_OUTDOOR)
#define MAP_FIXED_RESETTIME(m)  ((m)->map_flags & MAP_FLAG_FIXED_RTIME)
#define MAP_NOSAVE(m)           ((m)->map_flags & MAP_FLAG_NO_SAVE)
#define MAP_NOMAGIC(m)          ((m)->map_flags & MAP_FLAG_NOMAGIC)
#define MAP_NOPRIEST(m)         ((m)->map_flags & MAP_FLAG_NOPRIEST)
#define MAP_NOHARM(m)           ((m)->map_flags & MAP_FLAG_NOHARM)
#define MAP_NOSUMMON(m)         ((m)->map_flags & MAP_FLAG_NOSUMMON)
#define MAP_FIXEDLOGIN(m)       ((m)->map_flags & MAP_FLAG_FIXED_LOGIN)
#define MAP_PERMDEATH(m)        ((m)->map_flags & MAP_FLAG_PERMDEATH)
#define MAP_ULTRADEATH(m)       ((m)->map_flags & MAP_FLAG_ULTRADEATH)
#define MAP_ULTIMATEDEATH(m)    ((m)->map_flags & MAP_FLAG_ULTIMATEDEATH)
#define MAP_PVP(m)              ((m)->map_flags & MAP_FLAG_PVP)
#define MAP_MANUAL_RESET(m)     ((m)->map_flags & MAP_FLAG_MANUAL_RESET)
#define MAP_RELOAD(m)           ((m)->map_flags & MAP_FLAG_RELOAD)

/* mape darkness used to enforce the MAX_DARKNESS value.
 * but IMO, if it is beyond max value, that should be fixed
 * on the map or in the code.
 */
#define MAP_DARKNESS(m)     (m)->darkness
#define MAP_LIGHT_VALUE(m)  (m)->light_value

#define MAP_WIDTH(m)        (m)->width
#define MAP_HEIGHT(m)       (m)->height
/* Convenient function - total number of spaces is used
 * in many places.
 */
#define MAP_SIZE(m)         ((m)->width * (m)->height)

#define MAP_ENTER_X(m)      (m)->enter_x
#define MAP_ENTER_Y(m)      (m)->enter_y

/* flags passed to enter_map_xx(), ready_map_name, load_map and other map related functions */
#define MAP_STATUS_MULTI            0x01 /* normal multiuser map (saved in /tmp) */
#define MAP_STATUS_UNIQUE           0x02 /* like apartment - (map is inside /players folder) */
#define MAP_STATUS_INSTANCE         0x04 /* this is an instance (map is inside /instance) */
#define MAP_STATUS_STYLE            0x08 /* we load a special random map style map - we don't set speed for objects inside! */
#define MAP_STATUS_ORIGINAL         0x10 /* map is an original map - generate treasures! */

/* The first five flags below are in fact technically unnecessary (see
 * move.c/check_insertion_allowed()) but are retained for backwards
 * compatibility. */
#define MAP_STATUS_FIXED_POS        0x20 /* fixed location */
#define MAP_STATUS_RANDOM_POS_1     0x40 /* random location, 1 square radius */
#define MAP_STATUS_RANDOM_POS_2     0x80 /* random location, 2 squares radius */
#define MAP_STATUS_RANDOM_POS_3     0x100 /* random location, 3 squares radius */
#define MAP_STATUS_RANDOM_POS       0x200 /* random location, progressive radius */
#define MAP_STATUS_FREE_POS_ONLY    0x400 /* free spot only */

#define MAP_STATUS_NO_FALLBACK      0x800 /* when map loading fails, don't try to load savebed, emergency or another fallback map */
#define MAP_STATUS_LOAD_ONLY        0x1000 /* signal for map loader func to return after successful read_map_name() */
#define MAP_STATUS_FIXED_LOGIN      0x2000 /* same as MAP_FIXEDLOGIN() - useful for dynamic setting */

#define MAP_STATUS_ARTIFACT         0x4000 /* unusued: tells load_object we load a artifact object/block */
/* used to mask out map_status to get the map type */
#define MAP_STATUS_TYPE(_f)     (_f&(MAP_STATUS_MULTI|MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE|MAP_STATUS_STYLE))

/* Values for in_memory below.  Should probably be an enumerations */
#define MAP_ACTIVE   1
#define MAP_SWAPPED     2
#define MAP_LOADING     3
#define MAP_SAVING      4

/* new macros for map layer system */
#define GET_MAP_SPACE_PTR(_M_, _X_, _Y_)              (&((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)]))
#define GET_MAP_SPACE_FIRST(_M_)                      ((_M_)->first)
#define GET_MAP_SPACE_LAST(_M_)                       ((_M_)->last)
#define SET_MAP_SPACE_FIRST(_M_, _O_)                 ((_M_)->first = (_O_))
#define SET_MAP_SPACE_LAST(_M_, _O_)                  ((_M_)->last = (_O_))
#define GET_MAP_SPACE_VISIBLE_SLAYER(_M_, _L_)        ((_M_)->visible_slayer[(_L_)])
#define GET_MAP_SPACE_INVISIBLE_SLAYER(_M_, _L_)      ((_M_)->invisible_slayer[(_L_)])
#define GET_MAP_SPACE_GMASTER_SLAYER(_M_, _L_)        ((_M_)->gmaster_slayer[(_L_)])
#define GET_MAP_SPACE_VISIBLE_CLAYER(_M_, _L_)        ((_M_)->visible_clayer[(_L_)])
#define GET_MAP_SPACE_INVISIBLE_CLAYER(_M_, _L_)      ((_M_)->invisible_clayer[(_L_)])
#define GET_MAP_SPACE_GMASTER_CLAYER(_M_, _L_)        ((_M_)->gmaster_clayer[(_L_)])
#define SET_MAP_SPACE_VISIBLE_SLAYER(_M_, _L_, _O_)   ((_M_)->visible_slayer[(_L_)] = (_O_))
#define SET_MAP_SPACE_INVISIBLE_SLAYER(_M_, _L_, _O_) ((_M_)->invisible_slayer[(_L_)] = (_O_))
#define SET_MAP_SPACE_GMASTER_SLAYER(_M_, _L_, _O_)   ((_M_)->gmaster_slayer[(_L_)] = (_O_))
#define SET_MAP_SPACE_VISIBLE_CLAYER(_M_, _L_, _O_)   ((_M_)->visible_clayer[(_L_)] = (_O_))
#define SET_MAP_SPACE_INVISIBLE_CLAYER(_M_, _L_, _O_) ((_M_)->invisible_clayer[(_L_)] = (_O_))
#define SET_MAP_SPACE_GMASTER_CLAYER(_M_, _L_, _O_)   ((_M_)->gmaster_clayer[(_L_)] = (_O_))
#define GET_MAP_MOVE_FLAGS(_M_, _X_, _Y_)             ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].move_flags)
#define SET_MAP_MOVE_FLAGS(_M_, _X_, _Y_, _V_)        ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].move_flags = (_V_))
#define GET_MAP_FLAGS(_M_, _X_, _Y_)                  ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].flags)
#define SET_MAP_FLAGS(_M_, _X_, _Y_, _V_)             ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].flags = (_V_))
#define GET_MAP_LIGHT(_M_, _X_, _Y_)                  ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].light)
#define SET_MAP_LIGHT(_M_, _X_, _Y_, _V_)             ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].light = (sint8)(_V_))
#define GET_MAP_LIGHT_VALUE(_M_, _X_, _Y_)            ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].light_value)
#define SET_MAP_FACE_MASK(_M_, _X_, _Y_, _V_)         ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].mask_face = (_V_))
#define GET_MAP_FLOOR_FLAGS(_M_, _X_, _Y_)            ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].floor_flags)
#define GET_MAP_OB(_M_, _X_, _Y_)                     ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].first)
#define SET_MAP_DAMAGE(_M_, _X_, _Y_, tmp)            ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].last_damage = (uint16)(tmp))
#define GET_MAP_DAMAGE(_M_, _X_, _Y_)                 ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].last_damage)
#define SET_MAP_RTAG(_M_, _X_, _Y_, tmp)              ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].round_tag = (uint32)(tmp))
#define GET_MAP_RTAG(_M_, _X_, _Y_)                   ((_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].round_tag)

#define MAP_SET_WHEN_SWAP(_M_, _T_) \
    MAP_WHEN_SWAP((_M_)) = (ROUND_TAG - ROUND_TAG % \
                            (long unsigned int)MAX(1, pticks_second)) / \
                           pticks_second + (_T_)

#ifdef MAP_RESET // _T_ > 0, _T_ secs from now, _T_ == 0:  never, _T_ < 0: now
# define MAP_SET_WHEN_RESET(_M_, _T_) \
    if ((_T_) > 0) \
    { \
        MAP_WHEN_RESET((_M_)) = (ROUND_TAG - ROUND_TAG % (long unsigned int)MAX(1, pticks_second)) / pticks_second + (_T_); \
    } \
    else if ((_T_) == 0) \
    { \
        MAP_WHEN_RESET((_M_)) = 0; \
    } \
    else \
    { \
        MAP_WHEN_RESET((_M_)) = (ROUND_TAG - ROUND_TAG % (long unsigned int)MAX(1, pticks_second)) / pticks_second; \
    }
#else // maps never reset so ignore _T_
# define MAP_SET_WHEN_RESET(_M_, _T_) \
    MAP_WHEN_RESET((_M_)) = 0
#endif

/* You should really know what you are doing before using this - you
 * should almost always be using out_of_map instead, which takes into account
 * map tiling.
 */
#define OUT_OF_REAL_MAP(M,X,Y) ((X)<0 || (Y)<0 || (X)>=(M)->width || (Y)>=(M)->height)

/* These are used in the MapLook flags element.  They are not used in
 * in the object flags structure.
 */

#define P_BLOCKSVIEW    0x01
#define P_NO_MAGIC      0x02        /* Spells (some) can't pass this object */
#define P_NO_PASS       0x04        /* Nothing can pass (wall() is true) */
#define P_IS_PLAYER     0x08        /* there is one or more player on this tile */

#define P_IS_ALIVE      0x10        /* something alive is on this space */
#define P_NO_CLERIC     0x20        /* no clerical spells cast here */
#define P_PLAYER_ONLY   0x40        /* Only players are allowed to enter this space. This excludes mobs,
                                    * pets and golems but also spell effects and throwed/fired items.
                                    * it works like a no_pass for players only (pass_thru don't work for it).
                                    */
#define P_DOOR_CLOSED   0x80        /* a closed door is blocking this space - if we want approach, we must first
                                    * check its possible to open it.
                                    */


#define P_CHECK_INV     0x100       /* we have something like inventory checker in this tile node.
                                    * if set, me must blocked_tile(), to see what happens to us
                                    */
#define P_IS_PVP        0x200       /* This is ARENA flag - NOT PvP area flags - area flag is in mapheader */
#define P_PASS_THRU     0x400       /* same as NO_PASS - but objects with PASS_THRU set can cross it.
                                     * Note: If a node has NO_PASS and P_PASS_THRU set, there are 2 objects
                                     * in the node, one with pass_thru and one with real no_pass - then
                                     * no_pass will overrule pass_thru
                                     */
#define P_PASS_ETHEREAL 0x800       /* same as PASS_THRU - but for ethereal objects. This is pass_thru light */

#define P_WALK_ON       0x1000      /* this 4 flags are for moving objects and what happens when they enter */
#define P_WALK_OFF      0x2000      /* or leave a map tile */
#define P_FLY_OFF       0x4000
#define P_FLY_ON        0x8000

#define P_REFL_SPELLS   0x10000     /* something on the tile reflect spells */
#define P_REFL_MISSILE  0x20000     /* something on the tile reflect missiles */

#define P_MAGIC_EAR     0x40000     /* we have a magic ear in this map tile... later we should add a map
                                    * pointer where we attach as chained list this stuff - no search
                                    * or flags then needed.
                                    */
#define P_IS_PLAYER_PET 0x80000     /* The alive thing here is someone's pet */
#define P_PLAYER_GRAVE  0x100000    /* There is a player gravestone here */

#define P_OUT_OF_MAP    0x4000000   /* of course not set for map tiles but from blocked_xx()
                                     * function where the out_of_map() fails to grap a valid
                                     * map or tile.
                                     */
/* these are special flags to control how and what the update_position()
 * functions updates the map space.
 */
#define P_FLAGS_ONLY    0x08000000  /* skip the layer update, do flags only */
#define P_FLAGS_UPDATE  0x10000000  /* if set, update the flags by looping the map objects */
#define P_NEED_UPDATE   0x20000000  /* resort the layer when updating */
#define P_NO_ERROR      0x40000000  /* Purely temporary - if set, update_position
                                     * does not complain if the flags are different.
                                     */

#define P_NO_TERRAIN    0x80000000 /* DON'T USE THIS WITH SET_MAP_FLAGS... this is just to mark for
                                    * return values of blocked()...
                                    */

/* for server->client map protocol. Tell client how to handle its local map cache */
#define MAP_UPDATE_CMD_SAME 0
#define MAP_UPDATE_CMD_NEW 1
#define MAP_UPDATE_CMD_CONNECTED 2

#ifdef WIN32
#pragma pack(push,1)
#endif

typedef struct MapCell_struct
{
    int     count;
    short   faces[NROF_CLAYERS];
    uint16  fflag[NROF_CLAYERS];
    uint8   ff_probe[NROF_CLAYERS];
    char    quick_pos[NROF_CLAYERS];
} MapCell;

struct Map
{
    struct MapCell_struct   cells[MAP_CLIENT_X][MAP_CLIENT_Y];
};

#define MAP_FLOOR_FLAG_NO_PASS 2
#define MAP_FLOOR_FLAG_PLAYER_ONLY 4

/* This represents a single atomic map tile (aka square, hex etc) */
typedef struct MapSpace_s
{
    struct MapSpace_s  *prev_light;                     // used to create chained light source list
    struct MapSpace_s  *next_light;                     // used to create chained light source list

    /* XXX: To my mind first and last are backwards. first has ->below = NULL
     * and last has ->above = NULL.
     *
     * -- Smacky 20130225 */
    object             *first;                          // first object in this square
    object             *last;                           // last object in this square
    object             *visible_slayer[NROF_SLAYERS];   // array of visible slayer objects
    object             *invisible_slayer[NROF_SLAYERS]; // array of invisible slayer objects
    object             *gmaster_slayer[NROF_SLAYERS];   // array of gmaster slayer objects
    object             *visible_clayer[NROF_CLAYERS];   // array of visible clayer objects
    object             *invisible_clayer[NROF_CLAYERS]; // array of invisible clayer objects
    object             *gmaster_clayer[NROF_CLAYERS];   // array of gmaster clayer objects

    uint32              round_tag;                      /* tag for last_damage */
    sint32              light_source;                   /* light source counter - as higher as brighter light source here */
    sint32              light_value;                    /* how much light is in this tile. 0 = total dark
                                                         * 255+ = full daylight.
                                                         */
    New_Face           *mask_face;                      /* here we need the face for masks, because it can be turnable */
    int                 flags;                          /* flags about this space (see the P_ values above) */
    New_Face           *floor_face;                     /* here we need the face for the floor because it can be turnable */
    uint16              floor_terrain;
    sint16              floor_light;
    uint16              floor_direction_block;          /* direction blocking flag */
#ifdef USE_TILESTRETCHER
    sint16              floor_z;                        /* height of floor */
#endif
    uint16              last_damage;                    /* last_damage tmp backbuffer */
    uint16              move_flags;                     /* terrain type flags (water, underwater,...) */
    sint16              mask;                           /* picture/object ID for the floor mask of this tile */
    uint8               light;                          /* How much light this space provides */
    uint8               floor_flags;                    /* floor data: flags */
} MapSpace;

#ifdef WIN32
#pragma pack(pop)
#endif

/* map flags for global map settings - used in ->map_flags */
#define MAP_FLAG_NOTHING       0
#define MAP_FLAG_OUTDOOR       (1 << 0)  // outdoor map - daytime effects are on
#define MAP_FLAG_NO_SAVE       (1 << 1)  // don't save maps - atm only used with unique maps
#define MAP_FLAG_FIXED_RTIME   (1 << 2)  // reset time is not affected by players entering/exiting map
#define MAP_FLAG_NOMAGIC       (1 << 3)  // no spells
#define MAP_FLAG_NOPRIEST      (1 << 4)  // no prayers
#define MAP_FLAG_NOHARM        (1 << 5)  // no harmful spells/prayers
#define MAP_FLAG_NOSUMMON      (1 << 6)  // no summoning spells/prayers
#define MAP_FLAG_FIXED_LOGIN   (1 << 7)  // player login forced to enter_x/enter_y
#define MAP_FLAG_PERMDEATH     (1 << 8)  // perm death map
#define MAP_FLAG_ULTRADEATH    (1 << 9) // ultra death map
#define MAP_FLAG_ULTIMATEDEATH (1 << 10) // ultimate death map
#define MAP_FLAG_PVP           (1 << 11) // PvP is possible on this map
#define MAP_FLAG_MANUAL_RESET  (1 << 12) // a gmaster/script has scheduled this map to reset
#define MAP_FLAG_RELOAD        (1 << 13) // this map will reload (only has meaning on a manual reset)
#define MAP_FLAG_NO_UPDATE     (1 << 31) // ?

#define SET_MAP_TILE_VISITED(m, x, y, id) { \
    if((m)->pathfinding_id != (id)) { \
        (m)->pathfinding_id = (id); \
        memset((m)->bitmap, 0, ((MAP_WIDTH(m)+31)/32) * MAP_HEIGHT(m) * sizeof(uint32)); } \
    (m)->bitmap[(x)/32 + ((MAP_WIDTH(m)+31)/32)*(y)] |= (1U << ((x) % 32)); }
#define QUERY_MAP_TILE_VISITED(m, x, y, id) \
    ((m)->pathfinding_id == (id) && ((m)->bitmap[(x)/32 + ((MAP_WIDTH(m)+31)/32)*(y)] & (1U << ((x) % 32))))

/* In general, code should always use the macros
 * above (or functions in map.c) to access many of the
 * values in the map structure.  Failure to do this will
 * almost certainly break various features.  You may think
 * it is safe to look at width and height values directly
 * (or even through the macros), but doing so will completely
 * break map tiling.
 */
typedef struct mapdef
{
    struct mapdef  *last;                   /* map before, if NULL we are first_map */
    struct mapdef  *next;                   /* Next map, linked list */
    tag_t           tag;                    /* Unique identifier of this map object. Same as object tags */

    MapSpace       *spaces;                 /* Array of spaces on this map */
    MapSpace       *first_light;            /* list of tiles spaces with light sources in */
    oblinkpt       *buttons;                /* Linked list of linked lists of buttons */
    uint32         *bitmap;                 /* Bitmap used for marking visited tiles in pathfinding */
    char           *tmpname;                /* Name of temporary file (note: This will be by nature a unique
                                             * string - using our hash string system would be senseless.
                                             * Option is to add is as array, but then it must be from strlen
                                             * MAXPATHLEN which is 256 bytes or more - but normally this
                                             * path string is only around 20 bytes long. So we use malloc() )
                                             */

    /* hash strings... */
    shstr          *name;                   /* Name of map as given by its creator */
    shstr          *music;                  /* Filename of background music file */
    shstr          *msg;                    /* Message map creator may have left */
    shstr          *path;                   /* Filename of the map (shared string now) */
    shstr          *orig_path;              /* same as above but the original pathes - non unique or instanced */
    shstr          *tile_path[TILED_MAPS];  /* path to adjoining maps (shared strings) */
    shstr          *orig_tile_path[TILED_MAPS]; /* sic! */
    shstr          *cached_dist_map;         /* With which other map was relative_tile_position() last used? */
    shstr          *reference;              /* Reference for unique or instance maps (a player name) */
    /* hash strings end */

    struct mapdef  *tile_map[TILED_MAPS];   /* Next map, linked list */
    objectlink     *linked_spawn_list;      /* list pointer of the linked list */
    object         *player_first;           /* chained list of player on this map */
    object         *active_objects;         /* linked list of active objects */

    uint32          pathfinding_id;         /* For which traversal bitmap is valid */
    sint32          darkness;               /* indicates the base light value in this map.
                                             * This value is only used when the map is not
                                             * marked as outdoor.
                                             * 0= totally dark. 7= daylight
                                             */
    sint32          light_value;            /* the real light_value, build out from darkness
                                             * and possibly other factors.
                                             * This value is only used when the
                                             * map is not marked as outdoor.
                                             * 0 = totally dark, > 1000(?) = daylight
                                             */
    uint32          map_status;             /* flags for the internal status and use of the map */
    uint32          map_flags;              /* mag flags for various map settings (set from outside) */
    uint32          reset_time;             /* when this map should reset */
    uint32          reset_timeout;          /* How many seconds must elapse before this map should be reset */
    uint32          map_tag;                /* to identify maps for fixed_login */
    uint32          swap_time;              /* when this map should be swapped */
    uint32          swap_timeout;           /* How many seconds must elapse before this map should be swapped */
    uint32          in_memory;              /* If not true, the map has been freed and must
                                             * be loaded before used.  The map,omap and map_ob
                                             * arrays will be allocated when the map is loaded */

    uint32          traversed;               /* Used by relative_tile_position() to mark visited maps */
    sint32          cached_dist_x, cached_dist_y; /* Cached relative_tile_position() */

    uint16          perm_load;               /* This is a counter - used for example from NPC's which have
                                              * a global function. If this counter is != 0, map will not swap
                                              * and the npc/object with perm_load flag will stay in game.
                                              */
    uint16          difficulty;              /* What level the player should be to play here */
    uint16          height;                  /* Width and height of map. */
    uint16          width;
    sint16          enter_x;                 /* enter_x and enter_y are default entrance location */
    sint16          enter_y;                 /* on the map if none are set in the exit */

    uint32          tileset_id;              /* All map tiles that are connected to each other with tiling
                                                are on the same tileset. 0 means unknown */
    sint16          tileset_x, tileset_y;    /* This maps coordinates in the tileset. Only valid if tileset_id > 0 */
} mapstruct;

/* This is used by get_rangevector to determine where the other
 * creature is.  get_rangevector takes into account map tiling,
 * so you just can not look the the map coordinates and get the
 * righte value.  distance_x/y are distance away, which
 * can be negativbe.  direction is the crossfire direction scheme
 * that the creature should head.  part is the part of the
 * monster that is closest.
 * Note: distance should be always >=0. I changed it to UINT. MT
 */
typedef struct rv_vector_s
{
    unsigned int    distance;
    int             distance_x;
    int             distance_y;
    int             direction;
    object         *part;
} rv_vector;

/* constants for the flags for get_rangevector_*() functions */
#define RV_IGNORE_MULTIPART         0x01
#define RV_RECURSIVE_SEARCH         0x02

#define RV_DIAGONAL_DISTANCE        0x00
#define RV_MANHATTAN_DISTANCE       0x04
#define RV_EUCLIDIAN_DISTANCE       0x08
#define RV_FAST_EUCLIDIAN_DISTANCE  0x10
#define RV_NO_DISTANCE             (0x08|0x04|0x10)

extern int global_darkness_table[MAX_DARKNESS + 1];

/* Maximum number of tiles to search in relative_tile_position() before giving up */
/* 8 => 1 level deep, 24 => 2 levels, 48 =>3 levels */
#define MAX_SEARCH_MAP_TILES 24

/* This is used when we try to find vectors between map tiles in relative_tile_position() */
struct mapsearch_node
{
    mapstruct              *map;
    int                     dx, dy;
    struct mapsearch_node  *next;
};

#endif /* ifndef __MAP_H */
