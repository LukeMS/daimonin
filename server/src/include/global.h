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
 
typedef unsigned int	uint32;
typedef signed int	sint32;
typedef unsigned short	uint16;
typedef signed short	sint16;
typedef unsigned char	uint8;
typedef signed char	sint8;
typedef unsigned short Fontindex;
typedef unsigned int tag_t;


#define POW2(x) ((x) * (x))

/* map distance values for draw_info_map functions 
 * This value is in tiles
 */
#define MAP_INFO_NORMAL 12
#define MAP_INFO_ALL 9999

/* use this macros *only* to access the global hash table!
 * Note: there is a 2nd hash table for the arch list - thats a static
 * list BUT the arch names are inserted in the global hash to - so every
 * archlist name has 2 entries!
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

/* number of connected maps from a tiled map */
#define TILED_MAPS 8

/* global stuff used by new skill/experience system -b.t.
 * Needed before player.h
 */
#define MAX_EXP_CAT 7 		/* This should be => # of exp obj in the game 
				 * remember to include the "NULL" exp object  
			         * EXP_NONE as part of the overall tally. 
				 */
#define EXP_NONE (MAX_EXP_CAT - 1)  /* "NULL" exp. object. This is the last 
				     * experience obj always.*/ 


#define MAXLEVEL      110
extern uint32 new_levels[MAXLEVEL+2];
extern float lev_exp[MAXLEVEL+1];

extern uint32 global_map_tag; /* our global map_tag value for the server (map.c)*/

/*
 * So far only used when dealing with artifacts.
 * (now used by alchemy and other code too. Nov 95 b.t).
 */
typedef struct linked_char {
  const char *name;
  struct linked_char *next;
} linked_char;

#include "face.h"
#include "attack.h" /* needs to be before material.h */
#include "material.h"
#include "living.h"
#include "object.h"
#include "map.h"
#include "tod.h"
#include "pathfinder.h"

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

/* pointer for the glue.c interface between crosslib and server */
#include "funcpoint.h"

/*****************************************************************************
 * GLOBAL VARIABLES:							     *
 *****************************************************************************/
/* Special potions are now identifed by the last_eat value.
 * last_eat == 0 is no special potion - means they are used
 * as spell effect carrier. 
 */
#define special_potion(__op_sp) (__op_sp)->last_eat

/* arch.c - sysinfo for lowlevel */
extern int arch_cmp;		/* How many strcmp's */
extern int arch_search;	/* How many searches */

#define move_object(__op, __dir) move_ob(__op,__dir,__op)

#define is_magical(__op_) QUERY_FLAG(__op_,FLAG_IS_MAGICAL)

#define NUM_COLORS		13

extern char *colorname[NUM_COLORS];

extern New_Face *new_faces;

/*
 * These are the beginnings of linked lists:
 */
EXTERN player *first_player;
EXTERN mapstruct *first_map;
EXTERN treasurelist *first_treasurelist;
EXTERN artifactlist *first_artifactlist;
EXTERN archetype *first_archetype;
EXTERN objectlink *first_friendly_object;	/* Objects monsters will go after */
EXTERN godlink *first_god;
EXTERN racelink *first_race;

#define NROF_COMPRESS_METHODS 4
EXTERN char *uncomp[NROF_COMPRESS_METHODS][3];

/*
 * Variables set by different flags (see init.c):
 */
EXTERN long init_done;			/* Ignores signals until init_done is true */
EXTERN long trying_emergency_save;	/* True when emergency_save() is reached */
EXTERN long nroferrors;		/* If it exceeds MAX_ERRORS, call fatal() */

extern long pticks;		/* used by various function to determine */
				/* how often to save the character */
/*
 * Misc global variables:
 */
EXTERN FILE *logfile;			/* Used by server/daemon.c */
EXTERN int exiting;			/* True if the game is about to exit */
EXTERN long nroftreasures;		/* Only used in malloc_info() */
EXTERN long nrofartifacts;		/* Only used in malloc_info() */
EXTERN long nrofallowedstr;		/* Only used in malloc_info() */

EXTERN short nrofexpcat;	/* Current number of experience categories in the game */
EXTERN object *exp_cat[MAX_EXP_CAT];	/* Array of experience objects in the game */ 

EXTERN archetype *aggro_wp_archetype;	/* Nice to have fast access to it */
EXTERN archetype *empty_archetype;	/* Nice to have fast access to it */
EXTERN archetype *base_info_archetype;	/* Nice to have fast access to it */
EXTERN archetype *map_archeytpe;
EXTERN char first_map_path[MAX_BUF];	/* The start-level */

EXTERN char errmsg[HUGE_BUF];
EXTERN long ob_count;

EXTERN struct timeval last_time;        /* Used for main loop timing */

/*
 * Used in treasure.c
 */
EXTERN const char *undead_name;	/* Used in hit_player() in main.c */

EXTERN Animations *animations;
EXTERN int num_animations,animations_allocated, bmaps_checksum;

/* Rotate right from bsd sum. This is used in various places for checksumming */
#define ROTATE_RIGHT(c) if ((c) & 01) (c) = ((c) >>1) + 0x80000000; else (c) >>= 1;


#define SET_ANIMATION(ob,newanim) ob->face=&new_faces[animations[ob->animation_id].faces[newanim]]
#define GET_ANIMATION(ob,anim) (animations[ob->animation_id].faces[anim])
#define GET_ANIM_ID(ob) (ob->animation_id)

#define SET_INV_ANIMATION(ob,newanim) ob->face=&new_faces[animations[ob->inv_animation_id].faces[newanim]]
#define GET_INV_ANIMATION(ob,anim) (animations[ob->inv_animation_id].faces[anim])
#define GET_INV_ANIM_ID(ob) (ob->inv_animation_id)
/* NUM_ANIMATIONS returns the number of animations allocated.  The last
 * usuable animation will be NUM_ANIMATIONS-1 (for example, if an object
 * has 8 animations, NUM_ANIMATIONS will return 8, but the values will
 * range from 0 through 7.
 */
#define NUM_ANIMATIONS(ob) (animations[ob->animation_id].num_animations)
#define NUM_FACINGS(ob) (animations[ob->animation_id].facings)

extern int freearr_x[SIZEOFFREE], freearr_y[SIZEOFFREE];
extern int maxfree[SIZEOFFREE], freedir[SIZEOFFREE];

extern New_Face *blank_face, *next_item_face, *prev_item_face;
extern MapLook blank_look;

extern long max_time;	/* loop time */
extern NewSocket *init_sockets;

extern unsigned long todtick;
extern int world_darkness;
extern archetype *level_up_arch;

#define decrease_ob(xyz) decrease_ob_nr(xyz,1)

#define FREE_AND_NULL_PTR(_xyz_) {if(_xyz_){free(_xyz_); _xyz_=NULL; }}

#ifdef CALLOC
#undef CALLOC
#endif

#ifdef USE_CALLOC
# define CALLOC(x,y)	calloc(x,y)
# define CFREE(x)	free(x)
#else
# define CALLOC(x,y)	malloc(x*y)
# define CFREE(x)	free(x)
#endif


typedef struct Settings {
    char    *logfilename;   /* logfile to use */
    uint16  csport;	    /* port for new client/server */
    LogLevel debug;	    /* Default debugging level */
    uint8   dumpvalues;	    /* Set to dump various values/tables */
    char    *dumparg;       /* additional argument for some dump functions */
    uint8   daemonmode;     /* If true, detach and become daemon */
    int	    argc;	    /* parameters that were passed to the program */
    char    **argv;	    /* Only used by xio.c, so will go away at some time */
    char    *datadir;	    /* read only data files */
    char    *localdir;	    /* read/write data files */
    char    *playerdir;	    /* Where the player files are */
    char    *mapdir;	    /* Where the map files are */
    char    *archetypes;    /* name of the archetypes file - libdir is prepended */
    char    *treasures;	    /* location of the treasures file. */
    char    *uniquedir;	    /* directory for the unique items */
    char    *tmpdir;	    /* Directory to use for temporary files */
    uint8   stat_loss_on_death;	/* If true, chars lose a random stat when they die */
    uint8   use_permanent_experience; /* If true, players can gain perm exp */
    uint8   balanced_stat_loss; /* If true, Death stat depletion based on level etc */
    int	    reset_loc_time; /* Number of seconds to put player back at home */

    /* The meta_ is information for the metaserver.  These are set in 
     * the lib/settings file.
     */
    uint8   meta_on:1;		    /* True if we should send updates */
    char    meta_server[MAX_BUF];   /* Hostname/ip addr of the metaserver */
    char    meta_host[MAX_BUF];	    /* Hostname of this host */
    uint16  meta_port;		    /* Port number to use for updates */
    char    meta_comment[MAX_BUF];  /* Comment we send to the metaserver */
    uint32  worldmapstartx;	    /* starting x tile for the worldmap */
    uint32  worldmapstarty;	    /* starting y tile for the worldmap */
    uint32  worldmaptilesx;	    /* number of tiles wide the worldmap is */
    uint32  worldmaptilesy;	    /* number of tiles high the worldmap is */
    uint32  worldmaptilesizex;	    /* number of squares wide in a wm tile */
    uint32  worldmaptilesizey;	    /* number of squares high in a wm tile */
    uint16  dynamiclevel;	    /* how dynamic is the world? */
} Settings;

extern Settings settings;

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

/* GROS: Those are used by plugin events (argument fixthem) */
#define SCRIPT_FIX_ACTIVATOR 2
#define SCRIPT_FIX_ALL 1
#define SCRIPT_FIX_NOTHING 0

/* include some global project headers */
#ifndef __CEXTRACT__
#include "libproto.h"
#include "sockproto.h"
#endif

#include "plugin.h"


#endif /* GLOBAL_H */
