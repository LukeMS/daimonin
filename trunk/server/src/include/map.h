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

/*
 * The mapstruct is allocated each time a new map is opened.
 * It contains pointers (very indirectly) to all objects on the map.
 */

#ifndef MAP_H
#define MAP_H

#define MAP_PLAYER_MAP	1    /* for exit objects: this is a player unique map */
#define MAX_ARCH_LAYERS 7	/* thats our 7 logical layers.
                             * ! for first and last object, we will use 2 more fake layers!
                             */
#define MAP_LAYERS		4	/* thats our 4 physical layers we really show */

/* This is when the map will reset */
#define MAP_WHEN_RESET(m)	((m)->reset_time)

#define MAP_RESET_TIMEOUT(m)	((m)->reset_timeout)
#define MAP_DIFFICULTY(m)		((m)->difficulty)
#define MAP_TIMEOUT(m)			((m)->timeout)
#define MAP_SWAP_TIME(m)		((m)->swap_time)
#define MAP_OUTDOORS(m)			((m)->map_flags & MAP_FLAG_OUTDOOR)
#define MAP_UNIQUE(m)			((m)->map_flags & MAP_FLAG_UNIQUE)
#define MAP_FIXED_RESETTIME(m)	((m)->map_flags & MAP_FLAG_FIXED_RTIME)

#define MAP_NOMAGIC(m)			((m)->map_flags & MAP_FLAG_NOMAGIC)
#define MAP_NOPRIEST(m)			((m)->map_flags & MAP_FLAG_NOPRIEST)
#define MAP_NOHARM(m)			((m)->map_flags & MAP_FLAG_NOHARM)
#define MAP_NOSUMMON(m)			((m)->map_flags & MAP_FLAG_NOSUMMON)
#define MAP_FIXEDLOGIN(m)		((m)->map_flags & MAP_FLAG_FIXED_LOGIN)
#define MAP_PERMDEATH(m)		((m)->map_flags & MAP_FLAG_PERMDEATH)
#define MAP_ULTRADEATH(m)		((m)->map_flags & MAP_FLAG_ULTRADEATH)
#define MAP_ULTIMATEDEATH(m)	((m)->map_flags & MAP_FLAG_ULTIMATEDEATH)
#define MAP_PVP(m)				((m)->map_flags & MAP_FLAG_PVP)

/* mape darkness used to enforce the MAX_DARKNESS value.
 * but IMO, if it is beyond max value, that should be fixed
 * on the map or in the code.
 */
#define MAP_DARKNESS(m)	   	(m)->darkness

#define MAP_WIDTH(m)		(m)->width
#define MAP_HEIGHT(m)		(m)->height
/* Convenient function - total number of spaces is used
 * in many places.
 */
#define MAP_SIZE(m)		((m)->width * (m)->height)

#define MAP_ENTER_X(m)		(m)->enter_x
#define MAP_ENTER_Y(m)		(m)->enter_y

#define MAP_TEMP(m)		(m)->temp
#define MAP_PRESSURE(m)		(m)->pressure
#define MAP_HUMID(m)		(m)->humid
#define MAP_WINDSPEED(m)	(m)->windspeed
#define MAP_WINDDIRECTION(m)	(m)->winddir
#define MAP_SKYCOND(m)		(m)->sky

/* options passed to ready_map_name and load_original_map */
#define MAP_FLUSH	    0x1
#define MAP_PLAYER_UNIQUE   0x2
#define MAP_BLOCK	    0x4
#define MAP_STYLE	    0x8
#define MAP_OVERLAY	    0x10
#define MAP_ARTIFACT	0x20

/* Values for in_memory below.  Should probably be an enumerations */
#define MAP_IN_MEMORY 1
#define MAP_SWAPPED 2
#define MAP_LOADING 3
#define MAP_SAVING 4

/* new macros for map layer system */
#define GET_MAP_SPACE_PTR(M_,X_,Y_)		(&((M_)->spaces[(X_) + (M_)->width * (Y_)]))

#define GET_MAP_SPACE_FIRST(M_)			( (M_)->first )
#define GET_MAP_SPACE_LAST(M_)			( (M_)->last )
#define GET_MAP_SPACE_LAYER(M_,L_)		( (M_)->layer[L_] )
#define GET_MAP_SPACE_CL(M_,L_)			( (M_)->client_mlayer[L_]==-1?NULL:(M_)->layer[(M_)->client_mlayer[L_]])
#define GET_MAP_SPACE_CL_INV(M_,L_)		( (M_)->client_mlayer_inv[L_]==-1?NULL:(M_)->layer[(M_)->client_mlayer_inv[L_]])

#define SET_MAP_SPACE_FIRST(M_,O_)			( (M_)->first = (O_ ))
#define SET_MAP_SPACE_LAST(M_,O_)			( (M_)->last = (O_))
#define SET_MAP_SPACE_LAYER(M_,L_,O_)		( (M_)->layer[L_] = (O_))
#define SET_MAP_SPACE_CLID(M_,L_,O_)			( (M_)->client_mlayer[L_] = (sint8) (O_))
#define SET_MAP_SPACE_CLID_INV(M_,L_,O_)		( (M_)->client_mlayer_inv[L_] = (sint8) (O_))


#define GET_MAP_MOVE_FLAGS(M,X,Y)	( (M)->spaces[(X) + (M)->width * (Y)].move_flags )
#define SET_MAP_MOVE_FLAGS(M,X,Y,C)	( (M)->spaces[(X) + (M)->width * (Y)].move_flags = (uint16) C )
#define GET_MAP_FLAGS(M,X,Y)	( (M)->spaces[(X) + (M)->width * (Y)].flags )
#define SET_MAP_FLAGS(M,X,Y,C)	( (M)->spaces[(X) + (M)->width * (Y)].flags = (uint16) C )
#define GET_MAP_LIGHT(M,X,Y)	( (M)->spaces[(X) + (M)->width * (Y)].light )
#define SET_MAP_LIGHT(M,X,Y,L)	( (M)->spaces[(X) + (M)->width * (Y)].light = (sint8) L )

#define GET_MAP_OB(M,X,Y)	( (M)->spaces[(X) + (M)->width * (Y)].first )
#define GET_MAP_OB_LAST(M,X,Y)	( (M)->spaces[(X) + (M)->width * (Y)].last )
#define GET_MAP_OB_LAYER(_M_,_X_,_Y_,_Z_)	( (_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].layer[_Z_] )
#define get_map_ob	GET_MAP_OB

#define SET_MAP_OB(M,X,Y,tmp)	( (M)->spaces[(X) + (M)->width * (Y)].first = (tmp) )
#define SET_MAP_OB_LAST(M,X,Y,tmp)	( (M)->spaces[(X) + (M)->width * (Y)].last = (tmp) )
#define SET_MAP_OB_LAYER(_M_,_X_,_Y_,_Z_,tmp)	( (_M_)->spaces[(_X_) + (_M_)->width * (_Y_)].layer[_Z_] = (tmp) )
#define set_map_ob	SET_MAP_OB

#define SET_MAP_DAMAGE(M,X,Y,tmp)	( (M)->spaces[(X) + (M)->width * (Y)].last_damage = (uint16) (tmp) )
#define GET_MAP_DAMAGE(M,X,Y)	( (M)->spaces[(X) + (M)->width * (Y)].last_damage )

#define SET_MAP_RTAG(M,X,Y,tmp)	( (M)->spaces[(X) + (M)->width * (Y)].round_tag = (uint32) (tmp) )
#define GET_MAP_RTAG(M,X,Y)	( (M)->spaces[(X) + (M)->width * (Y)].round_tag )

/* These are the 'face flags' we grap out of the flags object structure 1:1.
 * I user a macro to get them from the object, doing a fast AND to mask the bigger
 * object flags to a uint8. I had to change to object flag order for it, but it
 * increase the server->client protocol ALOT - we don't need to collect anything.
 */
#define FFLAG_SLEEP     0x01        /* object sleeps */
#define FFLAG_CONFUSED  0x02        /* object is confused */
#define FFLAG_PARALYZED 0x04        /* object is paralyzed */
#define FFLAG_SCARED    0x08        /* object is scared - it will run away */
#define FFLAG_BLINDED   0x10        /* object is blinded */
#define FFLAG_INVISIBLE 0x20        /* object is invisible (can seen with "see invisible" on)*/
#define FFLAG_ETHEREAL  0x40        /* object is etheral */

#define FFLAG_PROBE     0x80        /* object is probed !Flag is set by map2 cmd! */


/* You should really know what you are doing before using this - you
 * should almost always be using out_of_map instead, which takes into account
 * map tiling.
 */
#define OUT_OF_REAL_MAP(M,X,Y) ((X)<0 || (Y)<0 || (X)>=(M)->width || (Y)>=(M)->height)

/* These are used in the MapLook flags element.  They are not used in
 * in the object flags structure.
 */

#define P_BLOCKSVIEW	0x01
#define P_NO_MAGIC      0x02	/* Spells (some) can't pass this object */
#define P_NO_PASS       0x04	/* Nothing can pass (wall() is true) */
#define P_IS_PLAYER		0x08	/* there is one or more player on this tile */
#define P_IS_ALIVE      0x10	/* something alive is on this space */
#define P_NO_CLERIC     0x20	/* no clerical spells cast here */
#define P_NEED_UPDATE	0x40	/* this space is out of date */
#define P_NO_ERROR      0x80	/* Purely temporary - if set, update_position
                                 * does not complain if the flags are different.
                                 */
#define P_CHECK_INV		0x100   /* we have something like inventory checker in this tile node.
								 * if set, me must blocked_tile(), to see what happens to us
								 */
#define P_SET_INV		0x200	/* rebuild the invisible client_layer array */
#define P_IS_PVP		0x400	/* This is ARENA flag - NOT PvP area flags - area flag is in mapheader */
#define P_PASS_THRU		0x800	/* same as NO_PASS - but objects with PASS_THRU set can cross it.
								 * Note: If a node has NO_PASS and P_PASS_THRU set, there are 2 objects
								 * in the node, one with pass_thru and one with real no_pass - then
								 * no_pass will overrule pass_thru 
								 */
#define P_MAGIC_EAR		0x1000   /* we have a magic ear in this map tile... later we should add a map
								  * pointer where we attach as chained list this stuff - no search
								  * or flags then needed.
								  */
#define P_PLAYER_ONLY	0x2000   /* Only players are allowed to enter this space. This excludes mobs,
								  * pets and golems but also spell effects and throwed/fired items.
								  * it works like a no_pass for players only (pass_thru don't work for it).
								  */
#define P_DOOR_CLOSED   0x4000	 /* a closed door is blocking this space - if we want approach, we must first
								  * check its possible to open it.
								  */

#define P_NO_TERRAIN    0x10000 /* DON'T USE THIS WITH SET_MAP_FLAGS... this is just to mark for return
								 * values of blocked...
								 */

/* Can't use MapCell as that is used in newserver.h
 * Instead of having numerous arrays that have information on a
 * particular space (was map, floor, floor2, map_ob),
 * have this structure take care of that information.
 * This puts it all in one place, and should also make it easier
 * to extend information about a space.
 */

#ifdef WIN32
#pragma pack(push,1)
#endif

/* well, we can also use instead a object* as client_layer a sint8 index, which is used
 * for layer[] - but its faster & easier for a few bytes more 
 */

typedef struct MapSpace {
	object  *first;							/* start of the objects in this map tile */
	object	*layer[MAX_ARCH_LAYERS*2];		/* array of visible layer objects + for invisible (*2)*/
	sint8	client_mlayer[MAP_LAYERS];		/* index for layer[] - this will send to player */
	sint8	client_mlayer_inv[MAP_LAYERS];	/* same for invisible objects */
	object  *last;							/* last object in this list */
    uint32  round_tag;						/* tag for last_damage */
    uint16  last_damage;					/* last_damage tmp backbuffer */
    uint16  move_flags;						/* terrain type flags (water, underwater,...) */
    uint16	flags;							/* flags about this space (see the P_ values above) */
    sint8	light;							/* How much light this space provides */
} MapSpace;

#ifdef WIN32
#pragma pack(pop)
#endif

/*
 * this is an overlay structure of the whole world.  It exists as a simple
 * high level map, which doesn't contain the full data of the underlying map.
 * in this map, only things such as weather are recorded.  By doing so, we
 * can keep the entire world parameters in memory, and act as a whole on
 * them at once.  We can then, in a separate loop, update the actual world
 * with the new values we have assigned.
 */

typedef struct wmapdef {
    char path[HUGE_BUF];	/* Filename of the map */
    char	*tmpname;	/* Name of temporary file */
    char 	*name;		/* Name of map as given by its creator */
    sint16	temp;		/* base temperature of this tile (F) */
    sint16	pressure;	/* barometric pressure (mb) */
    sint8	humid;		/* humitidy of this tile */
    sint8	windspeed;	/* windspeed of this tile */
    sint8	winddir;	/* direction of wind */
    sint8	sky;		/* sky conditions */
    sint32	avgelev;	/* average elevation */
    uint8 	darkness;	/* indicates level of darkness of map */
} weathermap_t;

/* map flags for global map settings - used in ->map_flags */
#define MAP_FLAG_NOTHING			0
#define MAP_FLAG_OUTDOOR			1		/* map is outdoor map - daytime effects are on */
#define MAP_FLAG_UNIQUE				2		/* special unique map - see docs */
#define MAP_FLAG_FIXED_RTIME		4		/* if true, reset time is not affected by
											 * players entering/exiting map
											 */
#define MAP_FLAG_NOMAGIC			8		/* no sp based spells */
#define MAP_FLAG_NOPRIEST			16		/* no grace baes spells allowed */
#define MAP_FLAG_NOHARM				32		/* allow only no attack, no debuff spells 
                                             * this is city default setting - heal for example
											 * is allowed on you and others but no curse or 
											 * fireball or abusing stuff like darkness or create walls 
											 */
#define MAP_FLAG_NOSUMMON			64		/* don't allow any summon/pet summon spell.
											 * this includes "call summons" for calling pets from other maps 
											 */
#define MAP_FLAG_FIXED_LOGIN		128		/* when set, a player login on this map will forced
											 * to default enter_x/enter_y of this map.
											 * this avoid map stucking and treasure camping
											 */
#define MAP_FLAG_PERMDEATH			256		/* this map is a perm death. */
#define MAP_FLAG_ULTRADEATH			1024	/* this map is a ultra death map */
#define MAP_FLAG_ULTIMATEDEATH		2048	/* this map is a ultimate death map */
#define MAP_FLAG_PVP				4096	/* PvP is possible on this map */

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
typedef struct mapdef {
    struct mapdef *next;			/* Next map, linked list */
    char *name;     				/* Name of map as given by its creator */
    char *tmpname;					/* Name of temporary file */
    char *msg;						/* Message map creator may have left */

    /* The following two are used by the pathfinder algorithm in pathfinder.c */
    uint32 *bitmap;                 /* Bitmap used for marking visited tiles in pathfinding */
    uint32 pathfinding_id;          /* For which traversal is the above valid */
    
    MapSpace *spaces;				/* Array of spaces on this map */
    oblinkpt *buttons;				/* Linked list of linked lists of buttons */

    const char *tile_path[TILED_MAPS];			/* path to adjoining maps */
    struct mapdef *tile_map[TILED_MAPS];		/* Next map, linked list */


	uint32 map_flags;				/* mag flags for various map settings */
    uint32 reset_time;				/* when this map should reset */
    uint32 reset_timeout;			/* How many seconds must elapse before this map
									 * should be reset
									 */
	uint32 map_tag;					/* to identify maps for fixed_login */
    sint32 timeout;					/* swapout is set to this */
    sint32 swap_time;				/* When it reaches 0, the map will be swapped out */
    uint32 in_memory;				/* If not true, the map has been freed and must
									 * be loaded before used.  The map,omap and map_ob
									 * arrays will be allocated when the map is loaded */

    sint16 players;					/* How many players are on this level right now */
    uint16 difficulty;				/* What level the player should be to play here */
	uint16 height;					/* Width and height of map. */
    uint16 width;
    uint16 enter_x;					/* enter_x and enter_y are default entrance location */
    uint16 enter_y;					/* on the map if none are set in the exit */

    sint16	temp;					/* base temperature of this tile (F) */
    sint16  pressure;				/* barometric pressure (mb) */

    uint8 darkness;    				/* indicates level of darkness of map - run time value */
    uint8 darkness_def;    			/* indicates level of darkness of map - default value */
									/* note: when we load a new map, we load darkness in 
									 * darkness and darkness_def. Then we process darkness
									 * with daytime, spells, etc. to get the current darkness
									 * level. When we save, we save darkness_def (tmp maps!).
									 */
    uint8 compressed;				/* Compression method used */ 
    uint32 traversed;               /* Used by relative_tile_position() to mark visited maps */

    sint8 humid;					/* humitidy of this tile */
    sint8 windspeed;				/* windspeed of this tile */
    sint8 winddir;					/* direction of wind */
    sint8 sky;						/* sky conditions */
    char path[HUGE_BUF];			/* Filename of the map, hm, dynamic malloc should ok,too */

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
typedef struct rv_vector {
    unsigned int	    distance;
    int	    distance_x;
    int	    distance_y;
    int	    direction;
    object  *part;
} rv_vector;

#endif
