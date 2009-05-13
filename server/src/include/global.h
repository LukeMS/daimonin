
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

/* temp. defines to prepare a login server based game server which
 * - don't loads face (daimonin.0 file)
 * - don't allow the request and sending of faces from client
 * - don't generates and sending the client srv_files
 */
#define SERVER_SEND_FACES

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

typedef signed char        sint8;
typedef unsigned char      uint8;
typedef signed short       sint16;
typedef unsigned short     uint16;
typedef signed int         sint32;
typedef unsigned int       uint32;
typedef uint32             tag_t;

/* 64bit definition */
#ifdef WIN32
typedef int64              sint64;
typedef unsigned int64     uint64;
#else /* LINUX and others */
#if SIZEOF_LONG == 8
typedef unsigned long      uint64;
typedef signed long        sint64;
#elif SIZEOF_LONG_LONG == 8
typedef unsigned long long uint64;
typedef signed long long   sint64;
#else
#error Your compiler misses 64-bit support
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

#ifndef WIN32
#ifndef _isatty
#define _isatty(__x) isatty(__x)
#endif
#ifndef _fileno
#define _fileno(__x) fileno(__x)
#endif
#endif

#ifdef USE_CALLOC
# define CALLOC(x,y)    calloc(x,y)
# define CFREE(x)   free(x)
#else
# define CALLOC(x,y)    malloc(x*y)
# define CFREE(x)   free(x)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

/* 0.94.1 - change to GETTIMEOFDAY macro - SNI systems only one one option.
 * rather than have complex #ifdefs throughout the file, lets just figure
 * it out once, here at the top.
 * Have no idea if that is the right symbol to check on for NetBSD,
 * but NetBSD does use 2 params.
 * Move this to global.h from time.c since its also used in arch.c
 */

#ifdef GETTIMEOFDAY_TWO_ARGS
#define GETTIMEOFDAY(last_time) gettimeofday(last_time, NULL);
#else
#define GETTIMEOFDAY(last_time) gettimeofday(last_time);
#endif

#define POW2(x) ((x) * (x))

#define EXIT_NORMAL     0
#define EXIT_ERROR     -1
#define EXIT_SHUTODWN  -2
#define EXIT_RESETMAP  -3

#define ROUND_TAG            pticks /* put this here because the DIFF */

#define WEAPON_SWING_TIME (0.125f)

/* the last_grace attribute is based on ticks instead of real time, so this converts it here */
#define RANGED_DELAY_TIME (0.125f)

#define query_name(_op_) query_name_full(_op_, NULL)

#define ARTIFACTS_FIRST_PASS 1
#define ARTIFACTS_SECOND_PASS 2

/* Give 1 re-roll attempt per artifact and treasure */
#define ARTIFACT_TRIES 2
#define CHANCE_FIX (-1)

/* progressive level damage increase */

#define LEVEL_DAMAGE_MULTIPLIER (0.25f)
#define LEVEL_DAMAGE(__l_) ((((float)(__l_)) * LEVEL_DAMAGE_MULTIPLIER) + 1.0f)

/* to access strings from objects, maps, arches or other system objects,
 * for printf() or others use only this macros to avoid NULL pointer exceptions.
 * Some standard c libaries don't check for NULL in that functions - most times
 * the retail versions.
 */
#define STRING_SAFE(__string__) ((__string__)!=NULL?(__string__):">NULL STR<")
#define PTR_STRING_SAFE(__ptr__, __field__) ((__ptr__)!=NULL?STRING_SAFE((__ptr__)->__field__):">NULL PTR<")

#define STRING_ARCH_NAME(__arch__) PTR_STRING_SAFE((__arch__), name)

#define STRING_OBJ_NAME(__ob__) PTR_STRING_SAFE((__ob__), name)
#define STRING_OBJ_ARCH_NAME(__ob__) ((__ob__)!=NULL?PTR_STRING_SAFE((__ob__)->arch, name):">NULL OBJ<")
#define STRING_OBJ_MAP_PATH(__ob__) ((__ob__)!=NULL?STRING_MAP_NAME((__ob__)->map):">NULL OBJ<")
#define STRING_OBJ_TITLE(__ob__) PTR_STRING_SAFE((__ob__), title)
#define STRING_OBJ_RACE(__ob__) PTR_STRING_SAFE((__ob__), race)
#define STRING_OBJ_SLAYING(__ob__) PTR_STRING_SAFE((__ob__), slaying)
#define STRING_OBJ_MSG(__ob__) PTR_STRING_SAFE((__ob__), msg)

#define STRING_MAP_PATH(__map__) PTR_STRING_SAFE((__map__), path)
#define STRING_MAP_ORIG_PATH(__map__) PTR_STRING_SAFE((__map__), orig_path)
#define STRING_MAP_TILE_PATH(__map__, __id__) ((__map__)!=NULL?PTR_STRING_SAFE((__map__), tile_path[(__id__)]):">NULL MAP<")
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

#define GET_LEVEL_EXP(_level_) new_levels[_level_]

#define RESTING_DEFAULT_SEC_TIMER 7    /* start rapid regeneration x second after sitting down */
#define REG_DEFAULT_SEC_TIMER 10       /* reg normal over the time all x seconds some points */

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

#define casting_level(__caster_, __spell_type_) SK_level(__caster_)

/* special macro with no {} ! if() FREE_AND_CLEAR_HASH2 will FAIL! */
#define FREE_AND_CLEAR_HASH2(_nv_) if(_nv_){free_string_shared(_nv_);_nv_ =NULL;}

#define LINK_SPAWN_ENEMY        0x01

#define SPAWN_RANDOM_RANGE     10000
#define RANDOM_DROP_RAND_RANGE 1000000

/* for creating treasures (treasure.c) */
#define T_STYLE_UNSET         (-999)
#define ART_CHANCE_UNSET        (-1)

/* ego items */
#define EGO_ITEM_BOUND_OK         0
#define EGO_ITEM_BOUND_UNBOUND    1
#define EGO_ITEM_BOUND_PLAYER     2
#define EGO_ITEM_BOUND_CLAN       3

/* mob defines */
#define MIN_MON_RADIUS            2  /* minimum monster detection radius */
#define MAX_AGGRO_RANGE           9 /* if target of mob is out of this range (or stats.Wis if higher)*/
#define MAX_AGGRO_TIME           12 /* until this time - then it skip target */

#ifdef SERVER_SEND_FACES
/* return values for esrv_send_face() in image.c */
#define SEND_FACE_OK              0
#define SEND_FACE_OUT_OF_BOUNDS   1
#define SEND_FACE_NO_DATA         2
#endif

/* for attack.c and material.c - item damage */
#define HIT_FLAG_WEAPON        1024

/* group define stuff */
#define GROUP_NO                (-1)

#define PMSG_MODE_NOTEXT          1 /* check GROUP_STATUS_NOQUEST, which means "outside maprang"
                                     * in party_message broadcast.
                                     * We simply want avoid kill messages & fighting related
                                     * info to group members out of range.
                                     */

#define GROUP_MAX_MEMBER          6 /* max # of members of a group */

#define GROUP_MODE_JOIN           0 /* allow /invite from all */
#define GROUP_MODE_DENY           1 /* deny /invite from everyone */
#define GROUP_MODE_INVITE         2 /* allow deny from selected player */

#define GROUP_STATUS_FREE         0 /* no group, no pending invite */
#define GROUP_STATUS_INVITE       1 /* pending invite */
#define GROUP_STATUS_GROUP        2 /* player is in group group_id */
#define GROUP_STATUS_NOQUEST      4 /* member get no quests, one drop items ... */

#define GROUP_UPDATE_HP           1
#define GROUP_UPDATE_MAXHP        2
#define GROUP_UPDATE_SP           4
#define GROUP_UPDATE_MAXSP        8
#define GROUP_UPDATE_GRACE       16
#define GROUP_UPDATE_MAXGRACE    32
#define GROUP_UPDATE_LEVEL       64

/* map distance values for draw_info_map functions
 * This value is in tiles
 */
#define MAP_INFO_NORMAL          12
#define MAP_INFO_ALL           9999

/* client interface communication */
#define NPC_INTERFACE_MODE_NO    -1
#define NPC_INTERFACE_MODE_NPC    1
#define NPC_INTERFACE_MODE_QLIST  2

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

#define MAXLEVEL           110

#define MONEYSTRING_NOTHING  0
#define MONEYSTRING_AMOUNT   1
#define MONEYSTRING_ALL     -1

#define COSTSTRING_SHORT  0
#define COSTSTRING_FULL   1
#define COSTSTRING_APPROX 2

/* Those are used by plugin events (argument fixthem) */
#define EVENT_MULTIPLE_TRIGGERS  4 // Allow multiple event triggers per round
#define SCRIPT_FIX_ACTIVATOR     2
#define SCRIPT_FIX_ALL           1
#define SCRIPT_FIX_NOTHING       0

#define special_potion(__op_sp) (__op_sp)->last_eat
#define move_object(__op, __dir) move_ob(__op,__dir,__op)

#define is_magical(__op_) QUERY_FLAG(__op_,FLAG_IS_MAGICAL)
#define is_cursed_or_damned(__op_) (QUERY_FLAG(__op_, FLAG_CURSED) || QUERY_FLAG(__op_, FLAG_DAMNED))

#define NUM_COLORS          13

/** number of darkness level. Add +1 for "total dark" */
#define MAX_DARKNESS         7

/* define from shstr.h - hash table dump */
#define SS_DUMP_TOTALS       1

/* global typedefs.
 * which needs defined before header loading.
 */

/**
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

#include "protocol.h" /* this is a shared header between server & client! defines & macros only! */
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
#include "spells.h"
#include "map.h"
#include "tod.h"
#include "pathfinder.h"
#include "gmaster.h"
#include "timeutils.h"

/* statistical events */
#include "stats.h"

/* Pull in the socket structure - used in the player structure */
#include "newserver.h"
#include "newclient.h"

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

/* Monster AI and mobdata structs */
#include "monster.h"

#ifdef USE_CHANNELS
/* Channel System */
#include "channel.h"
#endif

typedef struct _money_block
{
    /** 0, 1, or -1: see get_money_from_string() */
    int     mode;
    sint64   mithril;
    sint64   gold;
    sint64   silver;
    sint64   copper;
}_money_block;

/** ban node - see ban.c */
typedef struct ban_struct
{
    const char *name;       /* if != NULL, we have banned an name */
    int          ticks_init; /* how long is the ban */
    int          ticks_left; /* how long left */
    uint32       ticks;     /* (starting) pticks + ticks_left */
    char        *ip;        /* if name is == NULL, we have a ip */
} _ban_struct;

typedef struct Settings
{
    int                             max_cons_from_one_ip; /* Maximum number of concurrent connections from a single IP address, default = 2 */
    int                             player_races;       /* number of player race arches in client_settings */
    int                             mutelevel;          /* default FALSE - if TRUE player < level 2 can't shout */
    int                             login_allow;        /* if set to FALSE, login_ip is checked */
    char                           *login_ip;           /* ip for login_allow */
    char                           *logfilename;        /* logfile to use */
    uint16                          csport;             /* port for new client/server */
    LogLevel                        debug;              /* Default debugging level */
    uint8                           dumpvalues;         /* Set to dump various values/tables */
    char                           *dumparg;            /* additional argument for some dump functions */
    uint8                           daemonmode;         /* If true, detach and become daemon */
    int                             argc;               /* parameters that were passed to the program */
    char                           **argv;              /* Only used by xio.c, so will go away at some time */
    char                           *datadir;            /* read only data files */
    char                           *localdir;           /* read/write data files */
    char                           *accountdir;         /* Where the player files are */
    char                           *playerdir;          /* Where the player files are */
    char                           *instancedir;        /* Where the instance map files are */
    char                           *mapdir;             /* Where the map files are */
    char                           *archetypes;         /* name of the archetypes file - libdir is prepended */
    char                           *treasures;          /* location of the treasures file. */
    char                           *uniquedir;          /* directory for the unique items */
    char                           *tmpdir;             /* Directory to use for temporary files */
    char                           *statsdir;           /* Directory for active logs of statistical events */
    char                           *statsarchivedir;    /* Directory for logs, ready for further processing */
    uint8                           stat_loss_on_death; /* If true, chars lose a random stat when they die */
    uint8                           balanced_stat_loss; /* If true, Death stat depletion based on level etc */
    int                             reset_loc_time;     /* Number of seconds to put player back at home */

    /* The meta_ is information for the metaserver.  These are set in
     * the lib/settings file.
     */
    int                             meta_on : 1;        /* True if we should send updates */
    char                            meta_server[MAX_BUF]; /* Hostname/ip addr of the metaserver */
    char                            meta_name[MAX_BUF];   /* Servername listed in the meta server list */
    char                            meta_host[MAX_BUF]; /* Hostname of this host */
    uint16                          meta_port;          /* Port number to use for updates */
    char                            meta_comment[MAX_BUF]; /* Comment we send to the metaserver */
    uint32                          worldmapstartx;     /* starting x tile for the worldmap */
    uint32                          worldmapstarty;     /* starting y tile for the worldmap */
    uint32                          worldmaptilesx;     /* number of tiles wide the worldmap is */
    uint32                          worldmaptilesy;     /* number of tiles high the worldmap is */
    uint32                          worldmaptilesizex;  /* number of squares wide in a wm tile */
    uint32                          worldmaptilesizey;  /* number of squares high in a wm tile */
    uint16                          dynamiclevel;       /* how dynamic is the world? */
} Settings;

typedef struct _player_arch_template
{
    archetype   *p_arch[4];
    int   str;  /* these stats points overrule the arch settings for easy player customizing */
    int   dex;
    int   con;
    int   intel;
    int   wis;
    int   pow;
    int   cha;
} player_arch_template;

/* increase when you add more as 12 player races to client_settings */
#define MAX_PLAYER_ARCH     (12*4)

/*****************************************************************************
 * GLOBAL VARIABLES:                                                         *
 *****************************************************************************/
/* these variables are direct initialized in their modules. So we
 * can't use EXTERN.
 */
extern sint32                   new_levels[MAXLEVEL + 2];
extern float                    lev_exp[MAXLEVEL + 1];
extern int                      freearr_x[SIZEOFFREE];
extern int                      freearr_y[SIZEOFFREE];
extern int                      maxfree[SIZEOFFREE];
extern int                      freedir[SIZEOFFREE];
extern int                      freeback[SIZEOFFREE];
extern int                      freeback2[SIZEOFFREE];
extern Settings                 settings;
extern player_arch_template     player_arch_list[MAX_PLAYER_ARCH];

extern int                      global_darkness_table[MAX_DARKNESS + 1];
extern spell                    spells[NROFREALSPELLS];

/* EXTERN is pre-defined in common/init.c as #define EXTERN - so the
 * variables are bind in there. In every other module, EXTERN is
 * defined as #define EXTERN extern.
 */

/* lists of the active ingame gmasters */
EXTERN objectlink               *gmaster_list;
EXTERN objectlink               *gmaster_list_MW;
EXTERN objectlink               *gmaster_list_VOL;
EXTERN objectlink               *gmaster_list_GM;
EXTERN objectlink               *gmaster_list_MM;

EXTERN objectlink               *ban_list_player; /* see ban.c */
EXTERN objectlink               *ban_list_ip;     /* see ban.c */

EXTERN archetype               *global_aggro_history_arch;
EXTERN archetype               *global_dmg_info_arch;

EXTERN object                  *active_objects; /* List of active objects that need to be processed */
EXTERN object                  *inserted_active_objects; /* List of active objects that will be inserted into active_objects */
EXTERN object                  *next_active_object; /* Loop index for process_events(), might be modified during the loop */
EXTERN struct mempool_chunk    *removed_objects; /* List of objects that have been removed
                                                  * during the last server timestep
                                                  */

/** Intialization functions for the different object types */
EXTERN void (*object_initializers[256])(object *);

EXTERN _srv_client_files        SrvClientFiles[SRV_CLIENT_FILES];
EXTERN Socket_Info              socket_info;

EXTERN int                      global_exit_return; /* return value for exit() */
EXTERN long                     global_instance_id; /* every instance has a base ID at server runtime */
EXTERN int                      global_instance_num; /* every instance has an unique tag/number */
EXTERN uint32                   global_group_tag; /* every group gets an unique group tag identifier */
EXTERN uint32                   global_map_tag; /* our global map_tag value for the server (map.c)*/
EXTERN New_Face                *new_faces;
EXTERN archetype               *coins_arch[NUM_COINS+1];
EXTERN char                     global_version_msg[32];

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
EXTERN int                      player_active_meta;
EXTERN mapstruct               *first_map;
EXTERN treasurelist            *first_treasurelist;
EXTERN artifactlist            *first_artifactlist;
EXTERN godlink                 *first_god;
EXTERN racelink                *first_race;

/*
 * Variables set by different flags (see init.c):
 */
EXTERN long                     init_done;          /* Ignores signals until init_done is true */
EXTERN long                     nroferrors;     /* If it exceeds MAX_ERRORS, call fatal() */

EXTERN unsigned long            pticks;                 /* this is the global round counter. Every main loop pticks=pticks+1 */
EXTERN long                     pticks_ums;             /* how many micro seconds has one pticks */
EXTERN float                    pticks_second;          /* how many pticks in one second */
EXTERN uint32                   pticks_socket_idle;     /* 3 idle counter we use for idle sockets in socket/loop.c */
EXTERN uint32                   pticks_player_idle1;
EXTERN uint32                   pticks_player_idle2;
/*
 * Misc global variables:
 */
EXTERN FILE                    *logfile;            /* Used by server/daemon.c */
EXTERN int                      exiting;            /* True if the game is about to exit */
EXTERN long                     nroftreasures;      /* Only used in malloc_info() */
EXTERN long                     nrofartifacts;      /* Only used in malloc_info() */
EXTERN long                     nrofallowedstr;     /* Only used in malloc_info() */

EXTERN object                   void_container; /* Container for objects without env or map */

EXTERN char                     global_string_buf4096[HUGE_BUF];
EXTERN char                     errmsg[HUGE_BUF];
EXTERN long                     ob_count;

EXTERN int                      global_race_counter; /* global race counter */


EXTERN struct timeval           last_time;        /* Used for main loop timing */

/* constant shared string pointers */
EXTERN struct shstr_constants
{
    const char *undead;
    const char *none;
    const char *NONE;
    const char *quarterstaff;
    const char *battleground;
    const char *clawing;
    const char *dragon_skin_force;
    const char *dragon_ability_force;
    const char *dragon;
    const char *town_portal_destination;
    const char *existing_town_portal;
    const char *player;
    const char *money;
    const char *RANK_FORCE;
    const char *rank_force;
    const char *ALIGNMENT_FORCE;
    const char *alignment_force;
    const char *GUILD_FORCE;
    const char *guild_force;
    const char *player_info;
    const char *special_prayer;
    const char *grace_limit;
    const char *restore_grace;
    const char *restore_hitpoints;
    const char *restore_spellpoints;
    const char *heal_spell;
    const char *remove_curse;
    const char *remove_damnation;
    const char *heal_depletion;
    const char *message;
    const char *enchant_weapon;
    const char *Eldath;
    const char *the_Tabernacle;
    const char *poisonous_food;
    const char *starvation;
    const char *drowning;
    const char *emergency_mappath;
    const char *start_mappath;
    const char *bind_mappath;
} shstr_cons;

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
EXTERN archetype               *level_up_arch; /* a global animation arch we use it in 2 modules, so not static */

/* hashtable of beacons */
EXTERN hashtable               *beacon_table;

/* include some global project headers */
#include "plugin.h"

#include "libproto.h"
#include "sockproto.h"
#include "sproto.h"
#include "testproto.h"

//#include <../random_maps/random_map.h>
//#include <../random_maps/rproto.h>



#endif /* GLOBAL_H */
