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
/*****************************************************************************/
/* Daimonin  plugin support - (C) 2001 by Yann Chachkoff.                    */
/* This code is placed under the GPL.                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Headers needed.                                                           */
/*****************************************************************************/

#ifndef PLUGIN_H_
#define PLUGIN_H_

/*****************************************************************************/
/* This one does not exist under Win32.                                      */
/*****************************************************************************/
#ifndef WIN32
#include <dlfcn.h>
#endif

#undef MODULEAPI
#ifdef WIN32
#ifdef LUA_PLUGIN_EXPORTS
#define MODULEAPI __declspec(dllexport)
#else
#define MODULEAPI __declspec(dllimport)
#endif
#else
#define MODULEAPI
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif


/*****************************************************************************/
/* This one does not exist under Win32.                                      */
/*****************************************************************************/
#ifndef WIN32
#include <dirent.h>
#endif

/*****************************************************************************/
/* Event ID codes. I sorted them to present local events first, but it is    */
/* just a 'cosmetic' thing.                                                  */
/*****************************************************************************/
/*****************************************************************************/
/* Local events. Those are always linked to a specific object.               */
/*****************************************************************************/
#define EVENT_NONE     0  /* No event. This exists only to reserve the "0".  */
#define EVENT_APPLY    1  /* Object applied-unapplied.                       */
#define EVENT_ATTACK   2  /* Monster attacked or Scripted Weapon used.       */
#define EVENT_DEATH    3  /* Player or monster dead.                         */
#define EVENT_DROP     4  /* Object dropped on the floor.                    */
#define EVENT_PICKUP   5  /* Object picked up.                               */
#define EVENT_SAY      6  /* Someone speaks.                                 */
#define EVENT_STOP     7  /* Thrown object stopped.                          */
#define EVENT_TIME     8  /* Triggered each time the object can react/move.  */
#define EVENT_THROW    9  /* Object is thrown.                               */
#define EVENT_TRIGGER  10 /* Button pushed, lever pulled, etc.               */
#define EVENT_CLOSE    11 /* Container closed.                               */
#define EVENT_TIMER    12 /* Timer connected triggered it.                   */
#define EVENT_TALK     13  /* Talk event for npc interface system            */

#define NR_LOCAL_EVENTS 14
#define NR_EVENTS 27

#define EVENT_FLAG_NONE     0x0000
#define EVENT_FLAG_APPLY    0x0001
#define EVENT_FLAG_ATTACK   0x0002
#define EVENT_FLAG_DEATH    0x0004
#define EVENT_FLAG_DROP     0x0008
#define EVENT_FLAG_PICKUP   0x0010
#define EVENT_FLAG_SAY      0x0020
#define EVENT_FLAG_STOP     0x0040
#define EVENT_FLAG_TIME     0x0080
#define EVENT_FLAG_THROW    0x0100
#define EVENT_FLAG_TRIGGER  0x0200
#define EVENT_FLAG_CLOSE    0x0400
#define EVENT_FLAG_TIMER    0x0800
#define EVENT_FLAG_TALK     0x1000


/*****************************************************************************/
/* Global events. Those are never linked to a specific object.               */
/*****************************************************************************/
/* i really dislike this system - scripts/events attached to object is
 * something complete different as attaching a script or event global.
 * The events above are attached to the object - the events below are
 * attached to THE ENGINE. There is no reason to add and bulk the objects
 * with it.
 */
/* dont use this events until this line gets removed!! */
/* Gecko: :-P */

#define EVENT_CLOCK    14 /* Global time event.                              */
#define EVENT_CRASH    15 /* Triggered when the server crashes. Not recursive*/
#define EVENT_GDEATH   16 /* Global Death event                              */
#define EVENT_GKILL    17 /* Triggered when anything got killed by anyone.   */
#define EVENT_LOGIN    18 /* Player login.                                   */
#define EVENT_LOGOUT   19 /* Player logout.                                  */
#define EVENT_MAPENTER 20 /* A player entered a map.                         */
#define EVENT_MAPLEAVE 21 /* A player left a map.                            */
#define EVENT_MAPRESET 22 /* A map is resetting.                             */
#define EVENT_REMOVE   23 /* A Player character has been removed.            */
#define EVENT_SHOUT    24 /* A player 'shout' something.                     */
#define EVENT_TELL     25 /* A player 'tell' something.                      */
#define EVENT_BORN     26 /* A new character has been created.               */

/*****************************************************************************/
/* Hook codes. A hook is a function pointer passed from the server to the    */
/* plugin, so the plugin can call a server/crosslib functionality. Some may  */
/* call them "callbacks", although I don't like that term, which is too      */
/* closely bound to C and pointers.                                          */
/* I didn't add comments for all those hooks, but it should be quite easy to */
/* find out to what function they are pointing at. Also consult the plugins.c*/
/* source file in the server subdirectory to see the hook "wrappers".        */
/*****************************************************************************/
#define HOOK_NONE               0
#define HOOK_LOG                1
#define HOOK_NEWINFOMAP         2
#define HOOK_SPRINGTRAP         3
#define HOOK_CASTSPELL          4
#define HOOK_CMDRSKILL          5
#define HOOK_BECOMEFOLLOWER     6
#define HOOK_PICKUP             7
#define HOOK_GETMAPOBJECT       8
#define HOOK_ESRVSENDITEM       9
#define HOOK_FINDPLAYER         10
#define HOOK_MANUALAPPLY        11
#define HOOK_CMDDROP            12
#define HOOK_CMDTAKE            13
#define HOOK_CMDTITLE           14
#define HOOK_TRANSFEROBJECT     15
#define HOOK_KILLOBJECT         16
#define HOOK_LEARNSPELL         17
#define HOOK_CHECKFORSPELLNAME  18
#define HOOK_CHECKFORSPELL      19
#define HOOK_ESRVSENDINVENTORY  20
#define HOOK_CREATEARTIFACT     21
#define HOOK_GETARCHETYPE       22
#define HOOK_UPDATESPEED        23
#define HOOK_UPDATEOBJECT       24
#define HOOK_FINDANIMATION      25
#define HOOK_GETARCHBYOBJNAME   26
#define HOOK_INSERTOBJECTINMAP  27
#define HOOK_READYMAPNAME       28
#define HOOK_ADDEXP             29
#define HOOK_DETERMINEGOD       30
#define HOOK_FINDGOD            31
#define HOOK_REGISTEREVENT      32
#define HOOK_UNREGISTEREVENT    33
#define HOOK_DUMPOBJECT         34
#define HOOK_LOADOBJECT         35
#define HOOK_REMOVEOBJECT       36
#define HOOK_ADDSTRING          37
#define HOOK_FREESTRING         38
#define HOOK_ADDREFCOUNT        39
#define HOOK_GETFIRSTMAP        40
#define HOOK_GETFIRSTPLAYER     41
#define HOOK_GETFIRSTARCHETYPE  42
#define HOOK_QUERYCOST          43
#define HOOK_QUERYMONEY         44
#define HOOK_PAYFORITEM         45
#define HOOK_PAYFORAMOUNT       46
#define HOOK_NEWDRAWINFO        47
#define HOOK_SENDCUSTOMCOMMAND  48
#define HOOK_CFTIMERCREATE      49
#define HOOK_CFTIMERDESTROY     50
#define HOOK_MOVEPLAYER         51
#define HOOK_MOVEOBJECT         52
#define HOOK_SETANIMATION        53
#define HOOK_COMMUNICATE         54
#define HOOK_FINDBESTOBJECTMATCH 55
#define HOOK_APPLYBELOW          56
#define HOOK_DESTRUCTOBJECT      57
#define HOOK_CLONEOBJECT         58
#define HOOK_TELEPORTOBJECT      59
#define HOOK_LEARNSKILL          60
#define HOOK_FINDMARKEDOBJECT    61
#define HOOK_IDENTIFYOBJECT      62
#define HOOK_CHECKFORSKILLNAME   63
#define HOOK_FINDSKILL           64
#define HOOK_NEWINFOMAPEXCEPT    65
#define HOOK_INSERTOBJECTINOB    66
#define HOOK_FIXPLAYER           67
#define HOOK_PLAYSOUNDMAP        68
#define HOOK_OUTOFMAP            69
#define HOOK_CREATEOBJECT        70
#define HOOK_SHOWCOST            71
#define HOOK_DEPOSIT             72
#define HOOK_WITHDRAW            73
#define HOOK_MAPTRANSERITEMS     74
#define HOOK_MAPSAVE             75
#define HOOK_MAPDELETE           76
#define HOOK_INTERFACE           77

#define NR_OF_HOOKS              78

/*****************************************************************************/
/* CFParm is the data type used to pass informations between the server and  */
/* the plugins. Using CFParm allows a greater flexibility, at the cost of a  */
/* "manual" function parameters handling and the need of "wrapper" functions.*/
/* Each CFParm can contain up to 15 different values, stored as (void *).    */
/*****************************************************************************/
typedef struct _CFParm
{
    int     Type[15];   /* Currently unused, but may prove useful later.     */
    void   *Value[15];  /* The values contained in the CFParm structure.     */
} CFParm;


/*****************************************************************************/
/* Generic plugin function prototype. All hook functions follow this.        */
/*****************************************************************************/
typedef CFParm*(*f_plugin) (CFParm *PParm);

/*****************************************************************************/
/* CFPlugin contains all pertinent informations about one plugin. The server */
/* maintains a list of CFPlugins in memory. Note that the library pointer is */
/* a (void *) in general, but a HMODULE under Win32, due to the specific DLL */
/* management.                                                               */
/*****************************************************************************/
#ifndef WIN32
#define LIBPTRTYPE void*
#else
#define LIBPTRTYPE HMODULE
#endif
typedef struct _CFPlugin
{
    f_plugin    eventfunc;          /* Event Handler function            */
    f_plugin    initfunc;           /* Plugin Initialization function.   */
    f_plugin    pinitfunc;          /* Plugin Post-Init. function.       */
    f_plugin    removefunc;         /* Plugin Closing function.          */
    f_plugin    hookfunc;           /* Plugin CF-funct. hooker function  */
    f_plugin    propfunc;           /* Plugin getProperty function       */
    LIBPTRTYPE  libptr;             /* Pointer to the plugin library     */
    char        id[MAX_BUF];        /* Plugin identification string      */
    char        fullname[MAX_BUF];  /* Plugin full name                  */
    int         gevent[NR_EVENTS];  /* Global events registered          */
} CFPlugin;

/* Test of a new, more efficient hook system */
/* The new way to use hooks is better, because
 * - we don't need to copy arguments to and from the CFParm structs
 * - we can choose to call the hooked function directly instead of through a wrapper
 * - we can still use wrappers if we want to
 * - we get type safety and a cleaner code
 */
struct plugin_hooklist
{
    void (*LOG)(LogLevel, char *, ...);
    char*(*create_pathname)(const char *);
    char*(*re_cmp)(char *, char *);

    void (*new_draw_info)(int flags, int pri, object *pl, const char *buf);
    void (*new_draw_info_format)(int flags, int pri, object *pl, char *format, ...);
    void (*new_info_map)(int color, mapstruct *map, int x, int y, int dist, const char *str);
    void (*new_info_map_except)(int color, mapstruct *map, int x, int y, int dist, object *op1, object *op, const char *str);
    int (*wall)(mapstruct *map, int x, int y);

    void (*free_string_shared)(const char *str);
    const char*(*add_string)(const char *str);
    const char*(*add_refcount)(const char *str);

    void (*fix_player)(object *op);
    void (*esrv_send_item)(object *pl, object *op);
    void (*esrv_send_inventory)(object *pl, object *op);

    int (*lookup_skill_by_name)(char *string);
    int (*look_up_spell_name)(const char *spname);

    object*(*insert_ob_in_ob)(object *op, object *where);
    object*(*insert_ob_in_map)(object *op, mapstruct *m, object *originator, int flag);
    int (*move_ob)(object *op, int dir, object *originator);

    void (*free_mempool)(struct mempool *pool);
    struct mempool*(*create_mempool)(const char *description, uint32 expand, uint32 size,
                                       uint32 flags, chunk_constructor constructor,
                                       chunk_destructor destructor);
    uint32 (*nearest_pow_two_exp)(uint32 n);
    void (*return_poolchunk_array_real)(void *data, uint32 arraysize_exp, struct mempool *pool);
    void*(*get_poolchunk_array_real)(struct mempool *pool, uint32 arraysize_exp);
	object * (*arch_to_object)(archetype *at);
	archetype * (*find_archetype)(const char *name);
};

/*****************************************************************************/
/* Exportable functions. Any plugin should define all those.                 */
/* initPlugin        is called when the plugin initialization process starts.*/
/* removePlugin      is called before the plugin gets unloaded from memory.  */
/* getPluginProperty is currently unused.                                    */
/* registerHook      is used to transmit hook pointers from server to plugin.*/
/* triggerEvent      is called whenever an event occurs.                     */
/*****************************************************************************/
extern MODULEAPI CFParm    *initPlugin(CFParm *PParm);
extern MODULEAPI CFParm    *removePlugin(CFParm *PParm);
extern MODULEAPI CFParm    *getPluginProperty(CFParm *PParm);
extern MODULEAPI CFParm    *registerHook(CFParm *PParm);
extern MODULEAPI CFParm    *triggerEvent(CFParm *PParm);


/* Table of all loaded plugins */
extern CFPlugin             PlugList[34];
extern int                  PlugNR;

#endif /*PLUGIN_H_*/
