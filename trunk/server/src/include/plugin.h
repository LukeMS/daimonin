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
#define EVENT_EXAMINE  12 /* Object was examined                             */
#define EVENT_TALK     13  /* Talk event for npc interface system            */

/* AI events don't use event objects, but are still linked to a mob object */
#define EVENT_AI_BEHAVIOUR 14 /* Behaviour event for the AI system           */

#define NR_LOCAL_EVENTS 15
#define NR_EVENTS 28

#define EVENT_FLAG(x) (1 << (x - 1))

#define EVENT_FLAG_NONE     0
#define EVENT_FLAG_APPLY    EVENT_FLAG(EVENT_APPLY)
#define EVENT_FLAG_ATTACK   EVENT_FLAG(EVENT_ATTACK)
#define EVENT_FLAG_DEATH    EVENT_FLAG(EVENT_DEATH)
#define EVENT_FLAG_DROP     EVENT_FLAG(EVENT_DROP)
#define EVENT_FLAG_PICKUP   EVENT_FLAG(EVENT_PICKUP)
#define EVENT_FLAG_SAY      EVENT_FLAG(EVENT_SAY)
#define EVENT_FLAG_STOP     EVENT_FLAG(EVENT_STOP)
#define EVENT_FLAG_TIME     EVENT_FLAG(EVENT_TIME)
#define EVENT_FLAG_THROW    EVENT_FLAG(EVENT_THROW)
#define EVENT_FLAG_TRIGGER  EVENT_FLAG(EVENT_TRIGGER)
#define EVENT_FLAG_CLOSE    EVENT_FLAG(EVENT_CLOSE)
#define EVENT_FLAG_EXAMIN   EVENT_FLAG(EVENT_EXAMINE)
#define EVENT_FLAG_TALK     EVENT_FLAG(EVENT_TALK)

/* special flag for quest_triggers - internal use */
#define EVENT_FLAG_SPECIAL_QUEST    EVENT_FLAG(NR_LOCAL_EVENTS)

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

#define EVENT_CLOCK    15 /* Global time event.                              */
#define EVENT_CRASH    16 /* Triggered when the server crashes. Not recursive*/
#define EVENT_GDEATH   17 /* Global Death event                              */
#define EVENT_GKILL    18 /* Triggered when anything got killed by anyone.   */
#define EVENT_LOGIN    19 /* Player login.                                   */
#define EVENT_LOGOUT   20 /* Player logout.                                  */
#define EVENT_MAPENTER 21 /* A player entered a map.                         */
#define EVENT_MAPLEAVE 22 /* A player left a map.                            */
#define EVENT_MAPRESET 23 /* A map is resetting.                             */
#define EVENT_REMOVE   24 /* A Player character has been removed.            */
#define EVENT_SHOUT    25 /* A player 'shout' something.                     */
#define EVENT_TELL     26 /* A player 'tell' something.                      */
#define EVENT_BORN     27 /* A new character has been created.               */

/*****************************************************************************/
/* Hook codes. A hook is a function pointer passed from the server to the    */
/* plugin, so the plugin can call a server/crosslib functionality. Some may  */
/* call them "callbacks", although I don't like that term, which is too      */
/* closely bound to C and pointers.                                          */
/* I didn't add comments for all those hooks, but it should be quite easy to */
/* find out to what function they are pointing at. Also consult the plugins.c*/
/* source file in the server subdirectory to see the hook "wrappers".        */
/*****************************************************************************/

/* Gecko 2005-05-14: This old hook system is being phased out, use the
 * plugin_hooklist struct below instead */
#define HOOK_NONE               0

#define HOOK_SENDCUSTOMCOMMAND   1
#define HOOK_MAPDELETE           2
#define HOOK_CREATEOBJECT        3

#define HOOK_OUTOFMAP            4

#define HOOK_CMDRSKILL          5
#define HOOK_BECOMEFOLLOWER     6
#define HOOK_PICKUP             7
#define HOOK_GETMAPOBJECT       8

#define HOOK_COMMUNICATE        9
#define HOOK_FINDPLAYER         10
#define HOOK_MANUALAPPLY        11
#define HOOK_CMDDROP            12
#define HOOK_CMDTAKE            13

#define HOOK_FINDMARKEDOBJECT   14
#define HOOK_TRANSFEROBJECT     15
#define HOOK_KILLOBJECT         16
#define HOOK_LEARNSPELL         17

#define HOOK_IDENTIFYOBJECT     18
#define HOOK_CHECKFORSPELL      19
#define HOOK_DESTRUCTOBJECT     20
#define HOOK_CLONEOBJECT        21
#define HOOK_INTERFACE          22
#define HOOK_UPDATESPEED        23
#define HOOK_UPDATEOBJECT       24
#define HOOK_FINDANIMATION      25
#define HOOK_TELEPORTOBJECT      26
#define HOOK_LEARNSKILL          27

#define HOOK_READYMAPNAME       28
#define HOOK_ADDEXP             29
#define HOOK_DETERMINEGOD       30
#define HOOK_FINDGOD            31
#define HOOK_REGISTEREVENT      32
#define HOOK_UNREGISTEREVENT    33
#define HOOK_DUMPOBJECT         34
#define HOOK_LOADOBJECT         35
#define HOOK_REMOVEOBJECT       36
#define HOOK_MAPTRANSERITEMS     37
#define HOOK_MAPSAVE             38

#define NR_OF_HOOKS              39


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
    int (*map_brightness)(mapstruct *map, int x, int y);
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
    struct mob_known_obj * (*register_npc_known_obj)(object *npc, object *enemy, int friendship);
    int (*get_rangevector)(object *op1, object *op2, rv_vector *retval, int flags);
    int (*get_rangevector_from_mapcoords)(mapstruct *map1, int x1, int y1, mapstruct *map2, int x2, int y2, rv_vector *retval, int flags);
    object * (*get_archetype)(const char *name);
    void (*play_sound_player_only)(player *pl, int soundnum, int soundtype, int x, int y);
    void (*add_money_to_player)(object *pl, int c, int s, int g, int m);
    void (*drop_ob_inv)(object *ob);
    object * (*decrease_ob_nr)(object *op, int i);
    void (*add_quest_containers)(object *op);
    void (*add_quest_trigger)(object *who, object *trigger);
    void (*set_quest_status)(struct obj *trigger, int q_status, int q_type);
    void (*spring_trap)(object *trap, object *victim);
    int  (*cast_spell)(object *op, object *caster, int dir, int type, int ability, SpellTypeFrom item, char *stringarg);
    void (*play_sound_map)(mapstruct *map, int x, int y, int sound_num, int sound_type);
    object * (*find_skill)(object *op, int skillnr);
    int  (*find_animation)(char *name);
    int  (*find_face)(const char *name, int error);
    void (*get_tod)(struct _timeofday *tod);
    sint64 (*query_money)(object *op);
    sint64 (*query_cost)(object *tmp, object *who, int flag);
    char* (*cost_string_from_value)(sint64 cost, int mode);
    int (*pay_for_item)(object *op, object *pl);
    int (*pay_for_amount)(sint64 to_pay, object *pl);
    char* (*get_word_from_string)(char *str, int *pos);
    int (*get_money_from_string)(char *text, struct _money_block *money);
    void (*sell_item)(object *op, object *pl, sint64 value);
    int (*query_money_type)(object *op, int value);
    sint64 (*remove_money_type)(object *who, object *op, sint64 value, sint64 amount);
    void (*insert_money_in_player)(object *pl, object *money, uint32 nrof);
    int (*add_pet)(object *owner, object *pet, int mode);
	sint64 (*material_repair_cost)(object *item, object *owner);
	void (*material_repair_item)(object *item, int skill_value);
	char * (*query_short_name)(object *op, object *caller);
	artifact * (*find_artifact)(const char *name);
	void (*give_artifact_abilities)(object *op, artifact *art);
	const char * (*find_string)(const char *str);
	int (*get_nrof_quest_item)(const struct obj *target, const char *aname, const char *name, const char *title);
	object * (*is_player_inv)(object *op);

    /* Global variables */
    Animations **animations;
    New_Face **new_faces;
    int *global_darkness_table;
    archetype **coins_arch;
    struct shstr_constants *shstr_cons;
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
#define    PLUGINS_MAX_NROF 32

extern CFPlugin             PlugList[PLUGINS_MAX_NROF];
extern int                  PlugNR;

#endif /*PLUGIN_H_*/
