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

#ifndef __GLOBAL_H
#define __GLOBAL_H

/* DAI_VERSION_INTERIM is a string constant ("a", "b", etc -- longer strings,
 * up to about 16 characters, are possible but it is wisest to stick to single
 * characters) or the empty string. Where it is a constant, it indicates that
 * this is an interim server version. This is necessary because the
 * DAI_VERSION_* defines in protocol.h define the x.y.z version of both client
 * and server and therefore require a new client release for the autoupdater.
 * While it has always been possible to independently patch just the server
 * (so-called 'hotfixes') the problem was that players had no clue, beyond the
 * actual hotfix -- which of course may not really be player-noticeable, that
 * anything at all had changed. Thus confusion reigned.
 *
 * When a new x.y.z release is made, DAI_VERSION_INTERIM must be reset to "".
 * In fact, in trunk, this value should always be "". It (and it is the only
 * bit of code for which this is true) should only ever be directly changed in
 * main.
 *
 * So while trunk is 0.10.6, main may be 0.10.5 or 0.10.5/a to 0.10.5/z. When
 * server-only changes from trunk are merger to main this interim version is
 * bumped on main. Trunk remains at 0.10.6. When a new x.y.z is released (so
 * client too), main is bumped to 0.10.6 (the interim is reset to "") and trunk
 * is bumped to 0.10.7 and the process repeats. */
#define DAI_VERSION_INTERIM ""

/* Nicked from glib.h */
#ifdef __GNUC__
#define DAI_GNUC_PRINTF(format_idx, arg_idx) \
  __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#else
#define DAI_GNUC_PRINTF(format_idx, arg_idx)
#endif

#ifndef EXTERN
#define EXTERN extern
#endif

/* temp. defines to prepare a login server based game server which
 * - don't loads face (daimonin.0 file)
 * - don't allow the request and sending of faces from client
 * - don't generates and sending the client srv_files
 */
#define SERVER_SEND_FACES

#define ASSERT(x)

#if 0
#define ASSERT(x) if (!(x)) { kill(getpid(),11);}
#endif

#if defined(osf1) && !defined(__osf__)
#  define   __osf__
#endif

#if defined(sgi) && !defined(__sgi__)
#  define __sgi__
#endif

#ifdef sun
#  ifndef __sun__
#    define __sun__
#  endif
#endif

#if defined(ultrix) && !defined(__ultrix__)
#  define __ultrix__
#endif

/*
 * Central debug control point to turn on/off special debug setting.
 * debugflags get enabled/disabled by commenting then out or not.
 */

#ifdef DAI_DEVELOPMENT_CODE

//#define SEND_BUFFER_DEBUG

//#define DEBUG_PROCESS_QUEUE

//#define DEBUG_SKILL_UTIL

/* Active list debugging: object.c/activelist_insert_inline() and
 * object.c/activelist_remove_inline(). */
/* #define DEBUG_ACTIVELIST */

/* Log info when inserting/removing beacons: object.c */
/* #define DEBUG_BEACONS */

/* Map load, save, swap, free, and deleŧe: map.c */
#define DEBUG_MAP

/* very have debugging of the whole core server loop - mainly how map data ia send & stored */
/* #define DEBUG_CORE */
/* #define DEBUG_CORE_MAP*/

/* debug the fix_xxxxxx flow */
#define DEBUG_FIX_PLAYER
#define DEBUG_FIX_PLAYER_SKIPPED
#define DEBUG_FIX_MONSTER

/* force traverse loading and process of all player files inside /server/data/players */
/*#define  DEBUG_TRAVERSE_PLAYER_DIR*/

/* Aggro & EXP sharing debugging: aggro.c */
#define DEBUG_AGGRO /* Warning: ALOT debug log lines with this option - disable it for played server */

#define DEBUG_GROUP
//#define DEBUG_GROUP_UPDATE

/*#define DEBUG_FRIENDSHIP_WARNING*/

/* Debug Link Spawns: spawn_point.c */
/* log message when a linked spawn point is added, removed or called */
/* #define DEBUG_LINK_SPAWN */

/* Track & log mempool object using: mempool.c */
/*#define DEBUG_MEMPOOL_OBJECT_TRACKING*/  /* enables a global list of *all* objects
                                            * we have allocated. We can browse them to
                                            * control & debug them. WARNING: Enabling this
                                            * feature will slow down the server *EXTREMLY* and should
                                            * only be done in real debug runs
                                            */

/* Track object garbage collection */
/* #define DEBUG_GC */

/* This turns the "probe" spell into a powerful charm spell as
 * an easy way to aquire pets for testing of the pets code */
/* #define DEBUG_PROBE_IS_CHARM */

/* Controls debugging of the mob behaviours and movement */
#define DEBUG_AI
/*#define DEBUG_AI_ALL */ /* some extra info - enable this for debuging */
/*#define DEBUG_AI_WAYPOINT*/
/*#define DEBUG_AI_NPC_KNOWN*/

/* Uncomment this to enable some verbose pathfinding debug messages */
/* #define DEBUG_PATHFINDING */

/* find_next_object(): object.c */
/*#define DEBUG_FNO*/

/* merge_ob(): object.c */
/*#define DEBUG_MERGE_OB*/

#define DEBUG_CALENDAR

/*#define DEBUG_IPCOMPARE*/

//#define USE_OLD_UPDATE

// #define DEBUG_SHSTR

#endif /* ifdef DAI_DEVELOPMENT_CODE */

#ifdef DEBUG_FIX_PLAYER
#define FIX_PLAYER(_o_, _m_) fix_player(_o_, _m_)
#else
#define FIX_PLAYER(_o_, _m_) fix_player(_o_)
#endif

#ifdef DAI_DEVELOPMENT_CONTENT
#define PLUGIN_LUA_DEBUG       /* give us some general infos out */
#define PLUGIN_LUA_DEBUG_ALL   /* give us more infos out */
#endif

/* Include this first, because it lets us know what we are missing */
#ifdef WIN32 /* ---win32 exclude this, config comes from VC ide */
#include "win32.h"
#else
#include <linux.h>
#endif

/*
 * Daimonin requires ANSI-C, but some compilers "forget" to define it.
 * Thus the prototypes made by cextract don't get included correctly.
 */
#if !defined(__STDC__)
/* Removed # from start of following line.  makedepend was picking it up.
 * The following should still hopefully result in an error.
 */
error - Your ANSI C compiler should be defining __STDC__;
#endif

/** Length of string literal. Don't even think about using this on a pointer */
#define LSTRLEN(s) ((sizeof(s)/sizeof(char))-1)

#define TINY_BUF      64
#define SMALL_BUF     128
#define MEDIUM_BUF    256 /* Used for all kinds of things */
#define LARGE_BUF     1024
#define HUGE_BUF      4096 /* Used for messages - some can be quite long */

#define SOCKET_BUFLEN 4096 /* Max length a packet could be */

#define MAX_ANIMATIONS      256

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if defined(HAVE_TIME_H) && defined(TIME_WITH_SYS_TIME)
#include <time.h>
#endif

/* stddef is for offsetof */
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32 /* ---win32 we define this stuff in win32.h */
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirnet)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#endif

/* Is this accurate? Old comment: offsetof() is part of ANSI C, but many
 * compilers lack it, for example "gcc -ansi". */
/* FIXME: Technically, int should be size_t? */
#if !defined (offsetof)
#   define offsetof(type, member) \
       (int)&(((type *)0)->member)
#endif

/* ABS() outputs the absolute value of its input. */
#define ABS(__a) \
    ((__a) < 0 ? -(__a) : (__a))

/* SGN outputs the sign of its input. */
#define SGN(__a) \
    ((__a) > 0 ? 1 : ((__a) < 0 ? -1 : 0))

/* SQR() outputs its input multiplied by itself. */
#define SQR(__a) \
    ((__a) * (__a))

/* MIN() outputs the minimum value of its inputs. */
#define MIN(__a, __b) \
    ((__a) < (__b) ? (__a) : (__b))

/* MAX() outputs the maximum value of its inputs. */
#define MAX(__a, __b) \
    ((__a) > (__b) ? (__a) : (__b))

/* CLAMP() outputs __c if __a is more than, __b if __a is less than, or __a
 * otherwise. */
#define CLAMP(__a, __b, __c) \
    ((__a) > (__c) ? (__c) : ((__a) < (__b) ? (__b) : (__a)))

/* MALLOC() mallocs __a to size __b, logging a fatal OOM or initializing to
 * 0. */
// Seems Visual C++ cannot handle this style.
//#define MALLOC(__a, __b) \
//    ((!((__a) = malloc((__b)))) ? LOG(llevError, "%s %d: Out of memory!\n", __FILE__, __LINE__) : memset((__a), 0, (__b)))
#define MALLOC(__a, __b) \
    if (!((__a) = malloc((__b)))) \
    { \
        LOG(llevError, "%s %d: Out of memory!\n", __FILE__, __LINE__); \
    } \
    else \
    { \
        memset((__a), 0, (__b)); \
    }

/* MALLOC_STRING() mallocs __a to size strlen(__b) + 1, logging a fatal OOM or
 * initializing to the string array __b. */
// Seems Visual C++ cannot handle this style.
//#define MALLOC_STRING(__a, __b) \
//    ((!((__a) = malloc(strlen((__b)) + 1))) ? LOG(llevError, "%s %d: Out of memory!\n", __FILE__, __LINE__) : sprintf((__a), "%s", (__b)))
#define MALLOC_STRING(__a, __b) \
    if (!((__a) = malloc(strlen((__b)) + 1))) \
    { \
        LOG(llevError, "%s %d: Out of memory!\n", __FILE__, __LINE__); \
    } \
    else \
    { \
        sprintf((__a), "%s", (__b)); \
    }

/* FREE() frees __a and sets it to NULL. */
/* NOTE: This is generally the case with most *FREE*() macros: they call
 * *free*() and then assign NULL to the pointer. This iw because the functions
 * themselves do not reassign the pointer. After all, perhaps you want to
 * reallocate it immediately. Often the pointer is being freed as part of a
 * deallocation of a larger structure in which case NULLing each component is
 * not rreally necessary. Or at server shutdown all the allocated memory is
 * freed but we don't then care that the pointers are now garbage. Still, the
 * bigger issue is that referring to an unreassigned freed pointer will crash
 * the program. So although sometimes circumstances make it a bit redundant, do
 * it by default. */
#define FREE(__a) \
    free((__a)); \
    (__a) = NULL;

/* GETTIMEOFDAY() is simply a wrapper for the gettimeofday() library function
 * which apparently may tske one or two arguments depending on
 * implementation. */
/* Seems that the second argument is obsolete anyway, but currently it is
 * defined under Linux and our Windows implementation also allows for it. */
#ifdef GETTIMEOFDAY_TWO_ARGS
#   define GETTIMEOFDAY(__a) \
        gettimeofday((__a), NULL);
#else
#   define GETTIMEOFDAY(__a) \
        gettimeofday((__a));
#endif

/* Here we typedef *all* the data types used in the non-3rd party server code.
 * TODO: There are a lot of unupdated typedefs yet.
 *
 * By doing it here this means we can use the alias *anywhere* in the code.
 * This is particularly useful for the complex types such as structs as it
 * means that we can declare self-referential members like this:
 *   struct object_t
 *   {
 *       object_t *next; // no explicit struct keyword
 *       ...
 *   };
 *
 * And more importantly we can declare functions with a struct parameter or
 * return again without explicit keywords or worrying about whether the actual
 * struct has been declared yet (which means we can declare functions in
 * specific headers and include those headers in any order and therefore not
 * need unwieldy ...proto.h headers. */
#ifndef __TYPEDEFS_H
#   define __TYPEDEFS_H

/* SCALAR TYPES (integers of specific byte sizes, etc):
 *
 * Type defines for specific signed/unsigned variables of a certain number
 * of bits.  If a certain number of bits is required, these type defines
 * should then be used.  This will make porting to systems that have different
 * sized data types easier.
 *
 * Note:  When using something that is normally stored in a character (ie,
 * strings), don't use the uint8/sint8 typdefs, use 'char' instead. The
 * signedness for char is probably not universal, and using char will
 * probably be more portable than sint8/unit8.
 *
 * TODO: Change these to eg int8_t/uint8_t and only define if stdint.h (C99) is
 * unavailable. Although it might be easier/better to just forget about C99
 * entirely (still not sure how well supported it is on Windows) and just
 * suffix these with _t. */
typedef signed char        sint8;
typedef unsigned char      uint8;
typedef signed short       sint16;
typedef unsigned short     uint16;
typedef signed int         sint32;
typedef unsigned int       uint32;
#   ifdef WIN32
typedef __int64            sint64;
typedef unsigned __int64   uint64;
#   elif SIZEOF_LONG == 8
typedef unsigned long      uint64;
typedef signed long        sint64;
#   elif SIZEOF_LONG_LONG == 8
typedef unsigned long long uint64;
typedef signed long long   sint64;
#   else
#       error Your compiler misses 64-bit support
#   endif

typedef const char         shstr_t;
typedef sint8              stat_t;
typedef uint32             tag_t;

/* ENUM TYPES:
 *
 * Unfortunately typedefs don't work quite so smoothly with enums in C, so eg
 * 'typedef enum log_t log_t;' here with 'enum log_t { ... };' later in a
 * specific header (treating enums as directly analogous to structs, see below,
 * does not work as expected). So instead we typedef the alias as an int (C
 * stores enums internally as ints) here and declare the enum later.
 *
 * In theory I think this should prevent the useful behaviour of the compiler
 * choking on variables of the enum type being assigned values not declared in
 * the enumeration. In practice this doesn't seem to work anyway (GCC bug or me
 * bug?), so no big loss. */
typedef int attack_envmode_t;
typedef int attack_nr_t;
typedef int log_t;
typedef int map_memory_t;
typedef int msp_slice_t;
typedef int msp_clayer_t;
typedef int msp_slayer_t;
typedef int stat_nr_t;
typedef int tiling_direction_t;

/* NON-SCALAR TYPES (structs and unions):
 *
 * We use the _t suffix for both struct, etc declarator and typedef identifier
 * for ease of use. There is no conflict as they use different namespaces.
 *
 * Note: While this is actually relevant tooo the struct declarationss, not the
 * typedefs, this is a nice central place to mention it.
 *
 * Use (at least) basic structure packing techniques to squeeze structures into
 * as small a memory foo;;tprint as possible. Here is a very readable resource
 * on this subject: http://www.catb.org/esr/structure-packing/
 *
 * I think two things about this are important to stress.
 *
 * Firstly, while it is definitely worthwhile to remove/reduce unnecessary
 * padding on very often-used structures (such as map_t, msp_t, and object_t),
 * the saving per structure is only going to be a few bytes and this may seem
 * pointless when a new feature (such as msp_t being able to back reference its
 * map, x, y in 0.10.7) can add this and more, but that's useful, padding
 * isn't.
 *
 * Secondly, structure packing is an art, not a science. A very tightly packed
 * structure that is totally unreadable/unmaintainable as a result is of
 * dubious benefit. Also, keep in mind the memory cache. This is a complex
 * subject in detail but the basics seem simple enough: when a program first
 * accesses a specific address in memory (ie, a struct), a given block of that
 * memory -- the cache line, allegedly usually 32 or 64 bytes, depending on
 * architecture -- is copied into the cache. Subsequent operations work on this
 * cache, if available, first. Therefore it improves access times to put the
 * most commonly accessed members within the cache line.
 *
 * Older code tended (inconsistently) to use #pragma pack (on Windows). Don't
 * do this. */
typedef struct archetype_t       archetype_t;
typedef struct artifact_t        artifact_t;
typedef struct artifactlist_t    artifactlist_t;
typedef struct attack_name_t     attack_name_t;
typedef struct ban_t             ban_t;
typedef struct hashtable_t       hashtable_t;
typedef struct living_t          living_t;
typedef struct map_t             map_t;
typedef struct mob_pathfinding_t mob_pathfinding_t;
typedef struct moneyblock_t      moneyblock_t;
typedef struct msp_t             msp_t;
typedef struct object_t          object_t;
typedef struct objectlink_t      objectlink_t;
typedef struct player_t          player_t;
typedef struct player_template_t player_template_t;
typedef struct rv_t              rv_t;
typedef struct rv_cache_t        rv_cache_t;
typedef struct settings_t        settings_t;
typedef struct shstr_linked_t    shstr_linked_t;
typedef struct tiling_t          tiling_t;
typedef struct treasure_t        treasure_t;
typedef struct treasurelist_t    treasurelist_t;
typedef struct view_map_t        view_map_t;
typedef struct view_msp_t        view_msp_t;

#endif /* ifndef  __TYPEDEFS_H */

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

/* The TPR_*() macros are for timing code execution (TPR stands for Time Per
 * Repetition).
 *
 * There are four macros, TPR_START(), TPR_STOP(), TPR_BREAK(), and
 * TPR_GETDEVIATION(). Of these, you must use the first two. The third is
 * optional but may be used multiple times, and the fOurth is for internal use
 * only (you never call it directly).
 *
 * Usage is pretty simple. The macros are designed to time a function or part
 * thereof. Of course this may (likely will) call other functions but the point
 * is, each 'set' of calls must be within the same function.
 *
 * At the head of the function but after any function-wide automatic variable
 * declarations, or from wherever you want to start timing execution, call
 * TPR_START().
 *
 * At the foot of the function but before the final return, or wherever you
 * want to finish timing exeecution, call TPR_STOP(S).
 * 
 * Between these two, 0-n times, you may call TPR_BREAK(S).
 *
 * STOP and BREAK both log a message which gives some basic info and the time
 * (in seconds and microseconds) that has elapsed between when you called START
 * and now (the deviation). S is a string to help you identify what is going
 * on.
 *
 * STOP and BREAK are actually functionally nearly identical ATM. Nevertheless
 * you should use the appropriate0 one as described above.
 *
 * There is a very small overhead in using these macros so they probably should
 * not generally be used in public non-development servers (ie, main). */
#define TPR_START() \
    { \
        struct timeval tpr_a, \
                       tpr_b; \
        double         tpr_d; \
        GETTIMEOFDAY(&tpr_a);

#define TPR_BREAK(_S_) \
        TPR_GETDEVIATION(); \
        LOG(llevInfo, "TPR BREAK: %s:: %f\n", (_S_), tpr_d);

#define TPR_STOP(_S_) \
        TPR_GETDEVIATION(); \
        LOG(llevInfo, "TPR STOP: %s:: %f\n", (_S_), tpr_d); \
    }

#define TPR_GETDEVIATION() \
        GETTIMEOFDAY(&tpr_b); \
        if (tpr_b.tv_usec < tpr_a.tv_usec) \
        { \
          int tpr_n = (tpr_a.tv_usec - tpr_b.tv_usec) / 1000000 + 1; \
          tpr_a.tv_usec -= 1000000 * tpr_n; \
          tpr_a.tv_sec += tpr_n; \
        } \
        if (tpr_b.tv_usec - tpr_a.tv_usec > 1000000) \
        { \
          int tpr_n = (tpr_b.tv_usec - tpr_a.tv_usec) / 1000000; \
          tpr_a.tv_usec += 1000000 * tpr_n; \
          tpr_a.tv_sec -= tpr_n; \
        } \
        tpr_d = (double)(tpr_b.tv_sec - tpr_a.tv_sec) + (double)(tpr_b.tv_usec - tpr_a.tv_usec) / 1000000.0;

/* Various exit codes. While there does not seem to be a real standard for
 * these things, I have adopted the accepted bash usage. The essnce of this is
 * 0 means all OK, 1 is a general error (I've assigned it to shutdown), 2 is 
 * reserved and not really applicable here, 3-125 are free (I've assigned the
 * ones we use), 126-128 are reserved and not really applicable here, 128+n are
 * for POSIX signal n (signals do have standard values).
 *
 * -- Smacky 20150626 */
#define SERVER_EXIT_NORMAL   0
#define SERVER_EXIT_SHUTDOWN 1
#define SERVER_EXIT_RESTART  3
#define SERVER_EXIT_ARTIFACT 4
#define SERVER_EXIT_TREASURE 5
#define SERVER_EXIT_FATAL    6
#define SERVER_EXIT_FLOOD    7
#define SERVER_EXIT_SIGHUP   (128 + 1)
#define SERVER_EXIT_SIGINT   (128 + 2)
#define SERVER_EXIT_SIGQUIT  (128 + 3)
#define SERVER_EXIT_SIGBUS   (128 + 7)
#define SERVER_EXIT_SIGSEGV  (128 + 11)
#define SERVER_EXIT_SIGPIPE  (128 + 13)
#define SERVER_EXIT_SIGTERM  (128 + 15)

#define ROUND_TAG            pticks /* put this here because the DIFF */

#define WEAPON_SWING_TIME (0.125f)

/* the last_grace attribute is based on ticks instead of real time, so this converts it here */
#define RANGED_DELAY_TIME (0.125f)

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
#define STRING_MAP_TILE_PATH(__map__, __id__) ((__map__)!=NULL?PTR_STRING_SAFE((__map__), tiling.tile_path[(__id__)]):">NULL MAP<")
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

#define casting_level(__caster_, __spell_type_) SK_level(__caster_)

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

typedef enum
{
    BANTYPE_PLAYER,
    BANTYPE_ACCOUNT,
    BANTYPE_CHAR,
    BANTYPE_IP
} ENUM_BAN_TYPE;

#define MONEY_MODE_NOTHING 0
#define MONEY_MODE_AMOUNT  1
#define MONEY_MODE_ALL     2

#define COSTSTRING_SHORT  0
#define COSTSTRING_FULL   1
#define COSTSTRING_APPROX 2

/* Those are used by plugin events (argument fixthem) */
#define EVENT_MULTIPLE_TRIGGERS  4 // Allow multiple event triggers per round
#define SCRIPT_FIX_ACTIVATOR     2
#define SCRIPT_FIX_ALL           1
#define SCRIPT_FIX_NOTHING       0

#define NUM_COLORS          13

/** number of darkness levels. 0 means total darkness. */
#define MAX_DARKNESS   7
#define MAX_BRIGHTNESS 1280

extern sint16 brightness[MAX_DARKNESS + 1];

/* The FOREACH macros provide a convenient and understandable way to write a
 * code block which steps through each object in a given context.
 *
 * FOREACH_OBJECT_IN_OBJECT() will go through every object in the inventory of
 * another.
 *
 * FOREACH_OBJECT_IN_MSP() will go through every object in a map square. Note
 * that this begins the loop at the real object (that is not system object) end
 * of the list. (Almost?) Always when looking for a system object on a map
 * square, you're only interested in if there is a specific one or not and it
 * is better to use MSP_GET_SYS_OBJECT().
 *
 * FOREACH_PART_OF_OBJECT() will go through each part of a multipart object
 * (working its way towards the tail).
 *
 * All macros take 3 arguments. The 1st, _OC_, and 3rd, _ON_, are object_t *. 
 * These values will be overwritten each loop, with _OC_ pointing to the
 * child object under consideration or NULL and _ON_ pointing to the next one
 * or NULL. Note that it is fine to remove_ob() the child object each iteration
 * (but do not destroy the heirarchy further than the current child or the loop
 * will fail).
 *
 * The 2nd argument of FOREACH_OBJECT_IN_OBJECT(), _OP_, is also object_t *.
 * This is the parent object whose inventory we are looping through. Note that
 * the macro is not recursive; if _OC_ has an inventory we do not loop through
 * it (such behaviour is very easy to write in specific situations though).
 *
 * The 2nd argument of FOREACH_OBJECT_IN_MSP(), _MSP_, is msp_t *. This is
 * the map square whose contents we are looping through. Note that _OC_/_ON_
 * point to actual objects in _MSP_ and do not determine if they are singlepart
 * or part of a multipart. This is very easy to write in specific situations
 * though.
 *
 * The 2nd argument of FOREACH_PART_OF_OBJECT(), _OP_, is an object_t *. This
 * is the part (usually the head) from which we start browsing.
 *
 * So you will often find code like this (where what is a preset object_t *):
 *
 * object_t *this,
 *          *next;
 *
 * FOREACH_OBJECT_IN_OBJECT(this, what, next)
 * {
 *     ...
 * }
 *
 * Or this (where msp is a preset msp_t *):
 *
 * object_t *this,
 *          *next;
 *
 * FOREACH_OBJECT_IN_MSP(this, msp, next)
 * {
 *     ...
 * }
 *
 * Or this (where what is a preset object_t *):
 *
 * object_t *this,
 *          *next;
 *
 * FOREACH_PART_OF_OBJECT(this, what, next)
 * {
 *     ...
 * }
 * */
#define FOREACH_OBJECT_IN_OBJECT(_OC_, _OP_, _ON_) \
    for ((_OC_) = (_OP_)->inv, (_ON_) = (_OC_) ? (_OC_)->below : NULL; \
        (_OC_); \
        (_OC_) = (_ON_), (_ON_) = (_OC_) ? (_OC_)->below : NULL)
#define FOREACH_OBJECT_IN_MSP(_OC_, _MSP_, _ON_) \
    for ((_OC_) = (_MSP_)->last, (_ON_) = (_OC_) ? (_OC_)->below : NULL; \
        (_OC_); \
        (_OC_) = (_ON_), (_ON_) = (_OC_) ? (_OC_)->below : NULL)
#define FOREACH_PART_OF_OBJECT(_OC_, _OP_, _ON_) \
    for ((_OC_) = (_OP_), (_ON_) = (_OC_) ? (_OC_)->more : NULL; \
        (_OC_); \
        (_OC_) = (_ON_), (_ON_) = (_OC_) ? (_OC_)->more : NULL)

/* global typedefs.
 * which needs defined before header loading.
 */

#include "hashtable.h"
#include "hashfunc.h"
#include "shstr_ng.h"
#include "config.h"
#include "define.h"
#include "logger.h"
#include "ipcompare.h"
#include "protocol.h" /* this is a shared header between server & client! defines & macros only! */
#include "loader.h"
#include "face.h"
#include "aggro.h"
#include "attack.h" /* needs to be before material.h */
#include "material.h"
#include "living.h"
#include "mempool.h"
#include "object.h"
#include "links.h"
#include "arch.h"
#include "spells.h"
#include "calendar.h"
#include "map.h"
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

#include "quest.h"
#include "exp.h"

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

#include "book.h"

/* increase when you add more as 12 player races to client_settings */
#define MAX_PLAYER_ARCH     (12*4)

/*****************************************************************************
 * GLOBAL VARIABLES:                                                         *
 *****************************************************************************/
/* TODO: need init.h. */
struct settings_t
{
    int                             max_cons_from_one_ip; /* Maximum number of concurrent connections from a single IP address, default = 2 */
    int                             player_races;       /* number of player race arches in client_settings */
    int                             mutelevel;          /* default 0 - if 1 player < level 2 can't shout */
    int                             login_allow;        /* if set to 0, login_ip is checked */
    char                           *login_ip;           /* ip for login_allow */
    char                           *tlogfilename;       /* tlogfile to use */
    char                           *clogfilename;       /* clogfile to use */
    uint16                          csport;             /* port for new client/server */
    log_t                        debug;              /* Default debugging level */
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
    sint8                           stat_loss;          /* If not 0, players lose random stats when they die. */

    /* The meta_ is information for the metaserver.  These are set in
     * the lib/settings file.
     */
    int                             meta_on : 1;        /* True if we should send updates */
    char                            meta_server[MEDIUM_BUF]; /* Hostname/ip addr of the metaserver */
    char                            meta_name[MEDIUM_BUF];   /* Servername listed in the meta server list */
    char                            meta_host[MEDIUM_BUF]; /* Hostname of this host */
    uint16                          meta_port;          /* Port number to use for updates */
    char                            meta_comment[MEDIUM_BUF]; /* Comment we send to the metaserver */
    uint32                          worldmapstartx;     /* starting x tile for the worldmap */
    uint32                          worldmapstarty;     /* starting y tile for the worldmap */
    uint32                          worldmaptilesx;     /* number of tiles wide the worldmap is */
    uint32                          worldmaptilesy;     /* number of tiles high the worldmap is */
    uint32                          worldmaptilesizex;  /* number of squares wide in a wm tile */
    uint32                          worldmaptilesizey;  /* number of squares high in a wm tile */
    uint16                          dynamiclevel;       /* how dynamic is the world? */
};

extern settings_t settings;

struct player_template_t
{
    archetype_t *p_arch[4];
    int          str;   // overrules the arch settings for easy customisation
    int          dex;   // overrules the arch settings for easy customisation
    int          con;   // overrules the arch settings for easy customisation
    int          intel; // overrules the arch settings for easy customisation
    int          wis;   // overrules the arch settings for easy customisation
    int          pow;   // overrules the arch settings for easy customisation
    int          cha;   // overrules the arch settings for easy customisation
};

extern player_template_t player_template[MAX_PLAYER_ARCH];

/* TODO: need money.h. */
struct moneyblock_t
{
    uint8  mode;
    uint32 mithril;
    uint32 gold;
    uint32 silver;
    uint32 copper;
};

/* TODO: Need ban.h. */
struct ban_t
{
    shstr_t  *name;       /* if != NULL, we have banned an name */
    shstr_t  *account;
    int     ticks_init; /* how long is the ban */
    int     ticks_left; /* how long left */
    uint32  ticks;     /* (starting) pticks + ticks_left */
    char   *ip;        /* if name is == NULL, we have a ip */
};

extern spell                    spells[NROFREALSPELLS];

/* EXTERN is pre-defined in common/init.c as #define EXTERN - so the
 * variables are bind in there. In every other module, EXTERN is
 * defined as #define EXTERN extern.
 */

EXTERN objectlink_t               *ban_list_player;   /* see ban.c */
EXTERN objectlink_t               *ban_list_ip;       /* see ban.c */
EXTERN objectlink_t               *ban_list_account;  /* see ban.c */

EXTERN object_t                  *active_objects; /* List of active objects that need to be processed */
EXTERN object_t                  *inserted_active_objects; /* List of active objects that will be inserted into active_objects */
EXTERN object_t                  *next_active_object; /* Loop index for process_events(), might be modified during the loop */
EXTERN struct mempool_chunk    *removed_objects; /* List of objects that have been removed
                                                  * during the last server timestep
                                                  */

/** Intialization functions for the different object types */
EXTERN void (*object_initializers[256])(object_t *);

EXTERN _srv_client_files        SrvClientFiles[SRV_CLIENT_FILES];
EXTERN Socket_Info              socket_info;

EXTERN long                     global_instance_id; /* every instance has a base ID at server runtime */
EXTERN int                      global_instance_num; /* every instance has an unique tag/number */
EXTERN uint32                   global_group_tag; /* every group gets an unique group tag identifier */
EXTERN uint32                   global_map_tag; /* our global map_tag value for the server (map.c)*/
EXTERN New_Face                *new_faces;
EXTERN archetype_t               *coins_arch[NUM_COINS+1];
EXTERN char                     global_version_msg[32];

/* arch.c - sysinfo for lowlevel */
EXTERN int                      arch_init;
EXTERN int                      arch_cmp;       /* How many strcmp's */
EXTERN int                      arch_search;    /* How many searches */
/*
* These are the beginnings of linked lists:
*/
EXTERN player_t                  *first_player;
EXTERN player_t                  *last_player;
EXTERN int                      player_active;
EXTERN int                      player_active_meta;
EXTERN map_t               *first_map;
EXTERN treasurelist_t            *first_treasurelist;
EXTERN artifactlist_t            *first_artifactlist;
EXTERN godlink                 *first_god;
EXTERN racelink                *first_race;

/*
 * Variables set by different flags (see init.c):
 */
EXTERN long                     init_done;          /* Ignores signals until init_done is true */
EXTERN long                     nroferrors;     /* If it exceeds MAX_ERRORS, call fatal() */

EXTERN uint32                   pticks;                 /* this is the global round counter. Every main loop pticks=pticks+1 */
EXTERN uint32                   pticks_ums;             /* how many micro seconds has one pticks */
EXTERN uint32                   pticks_second;          /* how many pticks in one second */

/*
 * Misc global variables:
 */
EXTERN FILE                    *tlogfile;           /* Used by server/daemon.c */
EXTERN FILE                    *clogfile;           /* Used by server/daemon.c */
EXTERN int                      exiting;            /* True if the game is about to exit */
EXTERN long                     nroftreasures;      /* Only used in malloc_info() */
EXTERN long                     nrofartifacts;      /* Only used in malloc_info() */
EXTERN long                     nrofallowedstr;     /* Only used in malloc_info() */

EXTERN object_t                   void_container; /* Container for objects without env or map */

EXTERN char                     global_string_buf4096[HUGE_BUF];
EXTERN char                     errmsg[HUGE_BUF*6]; /* Must be at least as large as buf in get_ob_dif JRG 13-May-2009 */
EXTERN long                     ob_count;

EXTERN int                      global_race_counter; /* global race counter */


EXTERN struct timeval           last_time;        /* Used for main loop timing */

/* constant shared string pointers */
EXTERN struct shstr_constants
{
    shstr_t *undead;
    shstr_t *none;
    shstr_t *NONE;
    shstr_t *quarterstaff;
    shstr_t *clawing;
    shstr_t *town_portal_destination;
    shstr_t *existing_town_portal;
    shstr_t *player;
    shstr_t *money;
    shstr_t *RANK_FORCE;
    shstr_t *rank_force;
    shstr_t *ALIGNMENT_FORCE;
    shstr_t *GUILD_FORCE;
    shstr_t *stat_strength;
    shstr_t *stat_dexterity;
    shstr_t *stat_constitution;
    shstr_t *stat_intelligence;
    shstr_t *stat_wisdom;
    shstr_t *stat_power;
    shstr_t *stat_charisma;
    shstr_t *grace_limit;
    shstr_t *restore_grace;
    shstr_t *restore_hitpoints;
    shstr_t *restore_spellpoints;
    shstr_t *heal_spell;
    shstr_t *remove_curse;
    shstr_t *remove_damnation;
    shstr_t *heal_depletion;
    shstr_t *message;
    shstr_t *enchant_weapon;
    shstr_t *Eldath;
    shstr_t *the_Tabernacle;
    shstr_t *poisonous_food;
    shstr_t *starvation;
    shstr_t *drowning;
    shstr_t *emergency_mappath;
    shstr_t *start_mappath;
    shstr_t *bind_mappath;
    shstr_t *nopass;
    shstr_t *beacon_default;
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
EXTERN New_Face                *blank_face, *next_item_face, *prev_item_face, *no_floor_face;

EXTERN NewSocket               *init_sockets;

/* hashtable of beacons */
EXTERN hashtable_t               *beacon_table;

/* include some global project headers */
#include "plugin.h"

#include "libproto.h"
#include "portproto.h"
#include "sockproto.h"
#include "sproto.h"
#include "testproto.h"

//#include <../random_maps/random_map.h>
//#include <../random_maps/rproto.h>

#endif /* ifndef __GLOBAL_H */
