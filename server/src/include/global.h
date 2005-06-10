
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

#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef EXTERN
#define EXTERN extern
#endif

#define ESRV_DEBUG

#include "includes.h"


/* Type defines for specific signed/unsigned variables of a certain number
 * of bits.  Not really used anyplace, but if a certain number of bits
 * is required, these type defines should then be used.  This will make
 * porting to systems that have different sized data types easier.
 *
 * Note: The type defines should just mean that the data type has at
 * least that many bits.  if a uint16 is actually 32 bits, no big deal,
 * it is just a waste of space.
 *
 * Note2:  When using something that is normally stored in a character
 * (ie strings), don't use the uint8/sint8 typdefs, use 'char' instead.
 * The signedness for char is probably not universal, and using char
 * will probably be more portable than sint8/unit8
 */

typedef signed char     sint8;
typedef unsigned char   uint8;
typedef signed short    sint16;
typedef unsigned short  uint16;
typedef signed int      sint32;
typedef unsigned int    uint32;
typedef uint32			tag_t;

/* 64bit definition */
#ifdef WIN32
typedef int64					sint64;
typedef unsigned int64			uint64;

#else /* LINUX and others */
#if SIZEOF_LONG == 8
typedef unsigned long			uint64;
typedef signed long				sint64;
#elif SIZEOF_LONG_LONG == 8
typedef unsigned long long		uint64;
typedef signed long long 		sint64;

#else
#error Your compiler miss int64 support
#endif
#endif

#ifdef CALLOC
#undef CALLOC
#endif

/* porting stuff for file handle function names. */
#ifndef _fstat
#define _fstat(__x,__y) fstat(__x,__y)
#endif
#ifndef _stat
#define _stat stat
#endif
#ifndef _isatty
#define _isatty(__x) isatty(__x)
#endif
#ifndef _fileno
#define _fileno(__x) fileno(__x)
#endif

#ifdef USE_CALLOC
# define CALLOC(x,y)    calloc(x,y)
# define CFREE(x)   free(x)
#else
# define CALLOC(x,y)    malloc(x*y)
# define CFREE(x)   free(x)
#endif

/* 0.94.1 - change to GETTIMEOFDAY macro - SNI systems only one one option.
 * rather than have complex #ifdefs throughout the file, lets just figure
 * it out once, here at the top.
 * Have no idea if that is the right symbol to check on for NetBSD,
 * but NetBSD does use 2 params.
 * Move this to global.h from time.c since its also used in arch.c
 */

#ifdef GETTIMEOFDAY_TWO_ARGS
#define GETTIMEOFDAY(last_time) gettimeofday(last_time, (struct timezone *) NULL);
#else
#define GETTIMEOFDAY(last_time) gettimeofday(last_time);
#endif

#define POW2(x) ((x) * (x))

#define EXIT_NORMAL    0
#define EXIT_ERROR     -1
#define EXIT_SHUTODWN  -2

#define ROUND_TAG            pticks /* put this here because the DIFF */

#define query_name(_op_) query_name_full(_op_, NULL)

/* to access strings from objects, maps, arches or other system objects,
 * for printf() or others use only this macros to avoid NULL pointer exceptions.
 * Some standard c libaries don't check for NULL in that functions - most times
 * the retail versions.
 */
#define STRING_SAFE(__string__) ((__string__)?(__string__):">NULL STR<")
#define PTR_STRING_SAFE(__ptr__, __field__) ((__ptr__)?STRING_SAFE((__ptr__)->__field__):">NULL PTR<")

#define STRING_ARCH_NAME(__arch__) PTR_STRING_SAFE((__arch__), name)

#define STRING_OBJ_NAME(__ob__) PTR_STRING_SAFE((__ob__), name)
#define STRING_OBJ_ARCH_NAME(__ob__) ((__ob__)?PTR_STRING_SAFE((__ob__)->arch, name):">NULL OBJ<")
#define STRING_OBJ_MAP_PATH(__ob__) ((__ob__)?STRING_MAP_NAME((__ob__)->map):">NULL OBJ<")
#define STRING_OBJ_TITLE(__ob__) PTR_STRING_SAFE((__ob__), title)
#define STRING_OBJ_RACE(__ob__) PTR_STRING_SAFE((__ob__), race)
#define STRING_OBJ_SLAYING(__ob__) PTR_STRING_SAFE((__ob__), slaying)
#define STRING_OBJ_MSG(__ob__) PTR_STRING_SAFE((__ob__), msg)

#define STRING_MAP_PATH(__map__) PTR_STRING_SAFE((__map__), path)
#define STRING_MAP_TILE_PATH(__map__, __id__) ((__map__)?PTR_STRING_SAFE((__map__), tile_path[(__id__)]):">NULL MAP<")
#define STRING_MAP_NAME(__map__) PTR_STRING_SAFE((__map__), name)
#define STRING_MAP_TMPNAME(__map__) PTR_STRING_SAFE((__map__), tmpname)
#define STRING_MAP_MSG(__map__) PTR_STRING_SAFE((__map__), msg)

/* Rotate right from bsd sum. This is used in various places for checksumming */
#define ROTATE_RIGHT(c) if ((c) & 01) (c) = ((c) >>1) + 0x80000000; else (c) >>= 1;


#define SET_ANIMATION(ob,newanim) ob->face=&new_faces[animations[ob->animation_id].faces[newanim]]
#define GET_ANIMATION(ob,anim) (animations[ob->animation_id].faces[anim])
#define GET_ANIM_ID(ob) (ob->animation_id)

#define SET_INV_ANIMATION(ob,newanim) ob->face=&new_faces[animations[ob->inv_animation_id].faces[newanim]]
#define GET_INV_ANIMATION(ob,anim) (animations[ob->inv_animation_id].faces[anim])
#define GET_INV_ANIM_ID(ob) (ob->inv_animation_id)


#define MAX_PLAYER_NAME		12
#define MAX_PLAYER_PASSWORD	16

/* used for eric_server() */
#define SOCKET_UPDATE_PLAYER 1
#define SOCKET_UPDATE_CLIENT 2

/* NUM_ANIMATIONS returns the number of animations allocated.  The last
 * usuable animation will be NUM_ANIMATIONS-1 (for example, if an object
 * has 8 animations, NUM_ANIMATIONS will return 8, but the values will
 * range from 0 through 7.
 */
#define NUM_ANIMATIONS(ob) (animations[ob->animation_id].num_animations)
#define NUM_FACINGS(ob) (animations[ob->animation_id].facings)

#define FREE_AND_NULL_PTR(_xyz_) {if(_xyz_){free(_xyz_); _xyz_=NULL; }}

/* use *only* these macros to access the global hash table!
 * Note: there is a 2nd hash table for the arch list - thats a static
 * list BUT the arch names are inserted in the global hash too - so every
 * archlist name has 2 entries (so you can't always use == for string comparison!)
 */
#define FREE_AND_COPY_HASH(_sv_,_nv_) { if (_sv_) free_string_shared(_sv_); _sv_=add_string(_nv_); }
#define FREE_AND_ADD_REF_HASH(_sv_,_nv_) { if (_sv_) free_string_shared(_sv_); _sv_=add_refcount(_nv_); }
#define FREE_AND_CLEAR_HASH(_nv_) {if(_nv_){free_string_shared(_nv_);_nv_ =NULL;}}
#define FREE_ONLY_HASH(_nv_) if(_nv_)free_string_shared(_nv_);

#define ADD_REF_NOT_NULL_HASH(_nv_) if(_nv_!=NULL)add_refcount(_nv_);

/* special macro with no {} ! if() FREE_AND_CLEAR_HASH2 will FAIL! */
#define FREE_AND_CLEAR_HASH2(_nv_) if(_nv_){free_string_shared(_nv_);_nv_ =NULL;}

#define SPAWN_RANDOM_RANGE 10000
#define RANDOM_DROP_RAND_RANGE 1000000

/* for creating treasures (treasure.c) */
#define T_STYLE_UNSET (-2)
#define ART_CHANCE_UNSET (-1)

/* mob defines */
#define MIN_MON_RADIUS 2  /* minimum monster detection radius */
#define MAX_AGGRO_RANGE 9 /* if target of mob is out of this range (or stats.Wis if higher)*/
#define MAX_AGGRO_TIME 12 /* until this time - then it skip target */

/* return values for esrv_send_face() in image.c */
#define SEND_FACE_OK 0
#define SEND_FACE_OUT_OF_BOUNDS 1
#define SEND_FACE_NO_DATA 2

/* group define stuff */
#define GROUP_NO (-1)

#define PMSG_MODE_NOEXP     1   /* check GROUP_STATUS_NOEXP in party_message broadcast.
                                 * We simply want avoid kill messages & fighting related
                                 * info to group members out of range.
                                 */

#define GROUP_MAX_MEMBER	6   /* max # of members of a group */

#define GROUP_MODE_JOIN		0	/* allow /invite from all */
#define GROUP_MODE_DENY		1	/* deny /invite from everyone */
#define GROUP_MODE_INVITE	2	/* allow deny from selected player */

#define GROUP_STATUS_FREE	0   /* no group, no pending invite */
#define GROUP_STATUS_INVITE 1	/* pending invite */
#define GROUP_STATUS_GROUP	2	/* player is in group group_id */
#define GROUP_STATUS_NOEXP  4   /* member get no exp/no quest items/etc */

#define GROUP_UPDATE_HP         1
#define GROUP_UPDATE_MAXHP      2
#define GROUP_UPDATE_SP         4
#define GROUP_UPDATE_MAXSP      8
#define GROUP_UPDATE_GRACE      16
#define GROUP_UPDATE_MAXGRACE   32
#define GROUP_UPDATE_LEVEL      64

/* map distance values for draw_info_map functions
 * This value is in tiles
 */
#define MAP_INFO_NORMAL 12
#define MAP_INFO_ALL 9999

/* client interface communication */
#define NPC_INTERFACE_MODE_NO  -1
#define NPC_INTERFACE_MODE_NPC  1

/* number of connected maps from a tiled map */
#define TILED_MAPS 8

typedef enum
{
    SKILLGROUP_AGILITY,
    SKILLGROUP_PERSONAL,
    SKILLGROUP_MENTAL,
    SKILLGROUP_PHYSIQUE,
    SKILLGROUP_MAGIC,
    SKILLGROUP_WISDOM,
    SKILLGROUP_MISC,
    NROFSKILLGROUPS
} ENUM_SKILL_GROUPS;

/* have a marker to all exp groups which define the real player level */
#define NROFSKILLGROUPS_ACTIVE SKILLGROUP_MISC

#define MAXLEVEL      110

#define MONEYSTRING_NOTHING 0
#define MONEYSTRING_AMOUNT 1
#define MONEYSTRING_ALL -1

/* GROS: Those are used by plugin events (argument fixthem) */
#define SCRIPT_FIX_ACTIVATOR 2
#define SCRIPT_FIX_ALL 1
#define SCRIPT_FIX_NOTHING 0

#define special_potion(__op_sp) (__op_sp)->last_eat
#define move_object(__op, __dir) move_ob(__op,__dir,__op)

#define is_magical(__op_) QUERY_FLAG(__op_,FLAG_IS_MAGICAL)

#define NUM_COLORS      13

#define MAX_DARKNESS 7 /* number of darkness level. Add +1 for "total dark" */

/* define from shstr.h - hash table dump */
#define SS_DUMP_TOTALS  1

/* global typedefs.
 * which needs defined before header loading.
 */

/*
 * So far only used when dealing with artifacts.
 * (now used by alchemy and other code too. Nov 95 b.t).
 * This is used in readable.c, recipe.c and treasure.c .
 * its used in statical structures loaded at startup.
 * NEVER use this in dynamical way.
 */
typedef struct linked_char
{
    const char         *name;
    struct linked_char *next;
} linked_char;

#include "loader.h"
#include "face.h"
#include "aggro.h"
#include "quest.h"
#include "attack.h" /* needs to be before material.h */
#include "material.h"
#include "living.h"
#include "mempool.h"
#include "object.h"
#include "links.h"
#include "arch.h"
#include "map.h"
#include "tod.h"
#include "pathfinder.h"
#include "gmaster.h"

/* Pull in the socket structure - used in the player structure */
#include "newserver.h"

/* add skills.h global */
#include "skills.h"

/* Pull in the player structure */
#include "player.h"

/* pull in treasure structure */
#include "treasure.h"

#include "commands.h"

/* Pull in artifacts */
#include "artifact.h"

/* Now for gods */
#include "god.h"

/* Now for races */
#include "race.h"

#include "sounds.h"

/* Now for recipe/alchemy */
#include "recipe.h"

/* Now for spells */
#include "spells.h"

/* Monster AI and mobdata structs */
#include "monster.h"

typedef struct _money_block
{
    int     mode; /* 0, 1, or -1: see get_money_from_string() */
    long    mithril;
    long    gold;
    long    silver;
    long    copper;
}_money_block;

#define BAN_STRUCT_TAG 64

/* ban node - see ban.c */
typedef struct ban_struct
{
	int mode;
	int ticks_init; /* how long is the ban */
	int ticks_left; /* how long left */
	uint32 ticks;		/* (starting) pticks + ticks_left */
	uint32 ip;
	char tag[BAN_STRUCT_TAG];
} _ban_struct;

typedef struct Settings
{
    char                           *logfilename;   /* logfile to use */
    uint16                          csport;     /* port for new client/server */
    LogLevel                        debug;      /* Default debugging level */
    uint8                           dumpvalues;     /* Set to dump various values/tables */
    char                           *dumparg;       /* additional argument for some dump functions */
    uint8                           daemonmode;     /* If true, detach and become daemon */
    int                             argc;       /* parameters that were passed to the program */
    char                           **argv;      /* Only used by xio.c, so will go away at some time */
    char                           *datadir;        /* read only data files */
    char                           *localdir;       /* read/write data files */
    char                           *playerdir;      /* Where the player files are */
    char                           *mapdir;     /* Where the map files are */
    char                           *archetypes;    /* name of the archetypes file - libdir is prepended */
    char                           *treasures;      /* location of the treasures file. */
    char                           *uniquedir;      /* directory for the unique items */
    char                           *tmpdir;     /* Directory to use for temporary files */
    uint8                           stat_loss_on_death; /* If true, chars lose a random stat when they die */
    uint8                           balanced_stat_loss; /* If true, Death stat depletion based on level etc */
    int                             reset_loc_time; /* Number of seconds to put player back at home */

    /* The meta_ is information for the metaserver.  These are set in
     * the lib/settings file.
     */
    uint8                           meta_on : 1;            /* True if we should send updates */
    char                            meta_server[MAX_BUF];   /* Hostname/ip addr of the metaserver */
    char                            meta_host[MAX_BUF];     /* Hostname of this host */
    uint16                          meta_port;          /* Port number to use for updates */
    char                            meta_comment[MAX_BUF];  /* Comment we send to the metaserver */
    uint32                          worldmapstartx;     /* starting x tile for the worldmap */
    uint32                          worldmapstarty;     /* starting y tile for the worldmap */
    uint32                          worldmaptilesx;     /* number of tiles wide the worldmap is */
    uint32                          worldmaptilesy;     /* number of tiles high the worldmap is */
    uint32                          worldmaptilesizex;      /* number of squares wide in a wm tile */
    uint32                          worldmaptilesizey;      /* number of squares high in a wm tile */
    uint16                          dynamiclevel;       /* how dynamic is the world? */
} Settings;


/*****************************************************************************
 * GLOBAL VARIABLES:                                                         *
 *****************************************************************************/
/* these varaibles are direct initialized in their modules. So we
 * can't use EXTERN.
 */
extern uint32                   new_levels[MAXLEVEL + 2];
extern float                    lev_exp[MAXLEVEL + 1];
extern int                      freearr_x[SIZEOFFREE];
extern int                      freearr_y[SIZEOFFREE];
extern int                      maxfree[SIZEOFFREE];
extern int                      freedir[SIZEOFFREE];
extern Settings                 settings;

extern int                      global_darkness_table[MAX_DARKNESS + 1];
extern spell                    spells[NROFREALSPELLS];

/* EXTERN is pre-defined in common/init.c as #define EXTERN - so the
 * variables are bind in there. In every other module, EXTERN is
 * defined as #define EXTERN extern.
 */
#ifdef CS_LOGSTATS
EXTERN CS_Stats                 cst_tot, cst_lst;
#endif

/* lists of the active ingame gmasters */
EXTERN objectlink			   *gmaster_list;
EXTERN objectlink			   *gmaster_list_VOL;
EXTERN objectlink			   *gmaster_list_GM;
EXTERN objectlink			   *gmaster_list_DM;

EXTERN objectlink			   *ban_list_player; /* see ban.c */
EXTERN objectlink			   *ban_list_ip;	/* see ban.c */


EXTERN SockList                 global_sl;
EXTERN archetype               *global_aggro_history_arch;
EXTERN archetype               *global_dmg_info_arch;

EXTERN const char              *global_string_none; /* for quick search for string "none" */
EXTERN object                  *active_objects; /* List of active objects that need to be processed */
EXTERN struct mempool_chunk    *removed_objects; /* List of objects that have been removed
                                                 * during the last server timestep
                                                 */

EXTERN _srv_client_files        SrvClientFiles[SRV_CLIENT_FILES];
EXTERN Socket_Info              socket_info;

EXTERN uint32                   global_group_tag; /* every group gets a unique group tag identifier */
EXTERN uint32                   global_map_tag; /* our global map_tag value for the server (map.c)*/
EXTERN New_Face                *new_faces;
EXTERN archetype			   *coins_arch[NUM_COINS];


/* arch.c - sysinfo for lowlevel */
EXTERN int                      arch_init;
EXTERN int                      arch_cmp;       /* How many strcmp's */
EXTERN int                      arch_search;    /* How many searches */
/*
* These are the beginnings of linked lists:
*/
EXTERN player                  *first_player;
EXTERN player                  *last_player;
EXTERN int                      player_active;
EXTERN mapstruct               *first_map;
EXTERN treasurelist            *first_treasurelist;
EXTERN artifactlist            *first_artifactlist;
EXTERN objectlink              *first_friendly_object;  /* Objects monsters will go after */
EXTERN godlink                 *first_god;
EXTERN racelink                *first_race;

/*
 * Variables set by different flags (see init.c):
 */
EXTERN long                     init_done;          /* Ignores signals until init_done is true */
EXTERN long                     trying_emergency_save;  /* True when emergency_save() is reached */
EXTERN long                     nroferrors;     /* If it exceeds MAX_ERRORS, call fatal() */

EXTERN uint32                   pticks;                 /* this is the global round counter. Every main loop pticks=pticks+1 */
EXTERN long                     pticks_ums;             /* how many micro seconds has one pticks */
EXTERN float                    pticks_second;          /* how many pticks in one second */

/*
 * Misc global variables:
 */
EXTERN FILE                    *logfile;            /* Used by server/daemon.c */
EXTERN int                      exiting;            /* True if the game is about to exit */
EXTERN long                     nroftreasures;      /* Only used in malloc_info() */
EXTERN long                     nrofartifacts;      /* Only used in malloc_info() */
EXTERN long                     nrofallowedstr;     /* Only used in malloc_info() */

EXTERN object                   void_container; /* Container for objects without env or map */

EXTERN char                     first_map_path[MAX_BUF];    /* The start-level */

EXTERN char                     global_string_buf4096[HUGE_BUF];
EXTERN char                     errmsg[HUGE_BUF];
EXTERN long                     ob_count;

EXTERN int                      global_race_counter; /* global race counter */


EXTERN struct timeval           last_time;        /* Used for main loop timing */

/*
 * Used in treasure.c
 */
EXTERN const char              *undead_name;    /* Used in hit_player() in main.c */

EXTERN Animations              *animations;
EXTERN int                      num_animations, animations_allocated, bmaps_checksum;

/* Used in image.c */
/* nroffiles is the actual number of bitmaps defined.
 * nrofpixmaps is the higest numbers bitmap that is loaded.  With
 * the automatic generation of the bmaps file, this is now equal
 * to nroffiles.
 *
 */
EXTERN int                     nroffiles, nrofpixmaps;

/* only used in loader.c, to go from the numeric image id (which is
 * used throughout the program) backto the standard name.
 */
EXTERN MapLook                  blank_look;
EXTERN New_Face                *blank_face, *next_item_face, *prev_item_face;

EXTERN NewSocket               *init_sockets;

EXTERN unsigned long            todtick; /* time of the day tick counter */
EXTERN int                      world_darkness; /* daylight value. 0= totally dark. 7= daylight */

EXTERN archetype               *wp_archetype;   /* Nice to have fast access to it */
EXTERN archetype               *empty_archetype;    /* Nice to have fast access to it */
EXTERN archetype               *base_info_archetype;    /* Nice to have fast access to it */
EXTERN archetype               *map_archeytpe;
EXTERN archetype               *level_up_arch; /* a global animation arch we use it in 2 modules, so not static */

/* include some global project headers */
#include "plugin.h"

#include "libproto.h"
#include "sockproto.h"
#include "sproto.h"

//#include <../random_maps/random_map.h>
//#include <../random_maps/rproto.h>



#endif /* GLOBAL_H */
