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
#define HOOK_REMOVEOBJECT        2
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
#define HOOK_KILLOBJECT         15
#define HOOK_LEARNSPELL         16

#define HOOK_IDENTIFYOBJECT     17
#define HOOK_CHECKFORSPELL      18
#define HOOK_DESTRUCTOBJECT     19
#define HOOK_CLONEOBJECT        20
#define HOOK_LOADOBJECT         21
#define HOOK_UPDATESPEED        22
#define HOOK_UPDATEOBJECT       23
#define HOOK_FINDANIMATION      24

#define HOOK_LEARNSKILL         25

#define HOOK_DUMPOBJECT         26
#define HOOK_ADDEXP             27
#define HOOK_DETERMINEGOD       28
#define HOOK_FINDGOD            29
#define HOOK_REGISTEREVENT      30
#define HOOK_UNREGISTEREVENT    31

#define NR_OF_HOOKS              32


/*****************************************************************************/
/* CFParm is the data type used to pass informations between the server and  */
/* the plugins. Using CFParm allows a greater flexibility, at the cost of a  */
/* "manual" function parameters handling and the need of "wrapper" functions.*/
/* Each CFParm can contain up to 15 different values, stored as (void *).    */
/*****************************************************************************/
typedef struct _CFParm
{
    const int     Type[15];   /* Currently unused, but may prove useful later.     */
    void   *Value[15];  /* The values contained in the CFParm structure.     */
} CFParm;


/*****************************************************************************/
/* Generic plugin function prototype. All hook functions follow this.        */
/*****************************************************************************/
typedef CFParm*(*f_plugin)    (CFParm *PParm);

typedef int (*f_plugin_event) (CFParm *PParm);
typedef int (*f_plugin_prop)  (CFParm *PParm, CommArray_s *RTNCmd);
typedef int (*f_plugin_init)  (CFParm *PParm, const char **name, const char **version);


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
    f_plugin_event    eventfunc;          /* Event Handler function            */
    f_plugin_init     initfunc;           /* Plugin Initialization function.   */
    f_plugin          pinitfunc;          /* Plugin Post-Init. function.       */
    f_plugin          removefunc;         /* Plugin Closing function.          */
    f_plugin          hookfunc;           /* Plugin CF-funct. hooker function  */
    f_plugin_prop     propfunc;           /* Plugin getProperty function       */
    LIBPTRTYPE        libptr;             /* Pointer to the plugin library     */
    shstr             *id;                /* Plugin identification string      */
    shstr             *fullname;          /* Plugin full name                  */
    int               gevent[NR_EVENTS];  /* Global events registered          */
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
    char*(*create_mapdir_pathname)(const char *);
    char*(*re_cmp)(char *, char *);
    void (*new_draw_info)(const int, const int, const object *const , const char *const);
    void (*new_draw_info_format)(const int, const int, const object *const, const char *const, ...);
#ifdef USE_CHANNELS
    void (*lua_channel_message)(char *channelname, const char *name, char *message, int mode);
#endif
    void (*new_info_map)(const int, const mapstruct *const, const int, const int, const int, const char *const);
    void (*new_info_map_except)(const int, const mapstruct *const, const int, const int, const int,
          const object *const, const object *const, const char *const);
    int (*map_brightness)(mapstruct *, int, int);
    int (*wall)(mapstruct *, int, int);
    void (*free_string_shared)(const char *);
    const char*(*add_string)(const char *);
    const char*(*add_refcount)(const char *);
#ifdef DEBUG_FIX_PLAYER
    void (*fix_player)(object *, char *msg);
#else
    void (*fix_player)(object *);
#endif
    void (*esrv_send_item)(object *, object *);
    void (*esrv_send_inventory)(object *, object *);
    void (*esrv_update_item)(int flags, object *, object *);
    int (*lookup_skill_by_name)(char *);
    int (*look_up_spell_name)(const char *);
    object*(*insert_ob_in_ob)(object *, object *);
    object*(*insert_ob_in_map)(object * const, mapstruct *, object *const, const int);
    int (*move_ob)(object *, int, object *);
    void (*free_mempool)(struct mempool *);
    struct mempool * (*create_mempool)(const char *description, uint32 expand, uint32 size,
        uint32 flags, chunk_initialisator initialisator, chunk_deinitialisator deinitialisator,
        chunk_constructor constructor, chunk_destructor destructor);
    uint32 (*nearest_pow_two_exp)(uint32);
    void (*return_poolchunk_array_real)(void *, uint32, struct mempool *);
    void*(*get_poolchunk_array_real)(struct mempool *, uint32);
    object * (*arch_to_object)(archetype *);
    archetype * (*find_archetype)(const char *);
    struct mob_known_obj * (*update_npc_knowledge)(object *, object *, int, int);
    int (*get_rangevector)(object *, object *, rv_vector *, int);
    int (*get_rangevector_from_mapcoords)(mapstruct *, int, int, mapstruct *, int, int, rv_vector *, int);
    object * (*get_archetype)(const char *);
    void (*play_sound_player_only)(player *, int, int, int, int);
    void (*add_money_to_player)(object *, int, int, int, int);
    void (*drop_ob_inv)(object *);
    object * (*decrease_ob_nr)(object *, uint32);
    void (*add_quest_containers)(object *);
    void (*add_quest_trigger)(object *, object *);
    void (*set_quest_status)(struct obj *, int, int);
    void (*spring_trap)(object *, object *);
    int  (*cast_spell)(object *, object *, int, int, int, SpellTypeFrom, char *);
    void (*play_sound_map)(mapstruct *, int, int, int, int);
    object * (*find_skill)(object *, int);
    int  (*find_animation)(char *);
    int  (*find_face)(const char *, int);
    void (*get_tod)(struct _timeofday *);
    sint64 (*query_money)(object *);
    sint64 (*query_cost)(object *, object *, int);
    char* (*cost_string_from_value)(sint64, int);
    int (*pay_for_item)(object *, object *);
    int (*pay_for_amount)(sint64, object *);
    char* (*get_word_from_string)(char *, int *);
    int (*get_money_from_string)(char *, struct _money_block *);
    void (*sell_item)(object *, object *, sint64);
    int (*query_money_type)(object *, int);
    sint64 (*remove_money_type)(object *, object *, sint64, sint64);
    void (*insert_money_in_player)(object *, object *, uint32);
    int (*add_pet)(object *, object *, int);
    sint64 (*material_repair_cost)(object *, object *);
    void (*material_repair_item)(object *, int);
    char * (*query_short_name)(const object *const, const object *const);
    char * (*query_base_name)(object *, object *);
    artifact * (*find_artifact)(const char *);
    void (*give_artifact_abilities)(object *, artifact *);
    const char * (*find_string)(const char *);
    uint32 (*get_nrof_quest_item)(const struct obj *, const char *, const char *, const char *);
    object * (*is_player_inv)(object *);
    void (*gui_interface)(object *, int, const char *, const char *);
    int (*quest_count_pending)(const struct obj *);
    struct obj *(*quest_find_name)(const struct obj *, const char *);
    object * (*guild_get)(player *, char *);
    object * (*guild_join)(player *, char *, int, int, int, int, int, int);
    void (*guild_leave)(player *);
    object *(*locate_beacon)(shstr *);
    void (*free_map)(mapstruct *m, int flag);
    void (*delete_map)(mapstruct *m);
    void (*map_transfer_apartment_items)(mapstruct *map_old, mapstruct * map_new, int x, int y);
    int (*new_save_map)(mapstruct *m, int flag);
    mapstruct* (*ready_map_name)(const char *name_path,const char *src_path, int flags, shstr *reference);
    const char* (*create_unique_path_sh)(const object *op, const char *name);
    void  (*reload_behaviours)(object *op);
    void  (*clear_mob_knowns)(object *op, struct mob_known_obj **first, hashtable *ht);
    int (*command_target)(object *, char *);
    int (*get_friendship)(object *, object *); 
    int (*command_combat)(object *, char *);
    object *(*find_next_object)(object *, uint8, uint8, object *);
    sint32 (*sum_weight)(object *);

    hashtable *(*hashtable_new)(hashtable_size_t (*hash_func)(const hashtable_const_key_t), int (*equals_func)(const hashtable_const_key_t, const hashtable_const_key_t), hashtable_const_key_t deleted_key, hashtable_const_key_t empty_key, hashtable_size_t num_buckets);
    void (*hashtable_delete)(hashtable *ht);
    void (*hashtable_clear)(hashtable *ht);
    hashtable_value_t (*hashtable_find)(const hashtable *const ht, const hashtable_const_key_t key);
    int (*hashtable_insert)(hashtable *const ht, const hashtable_const_key_t key, const hashtable_value_t obj);
    int (*hashtable_erase)(hashtable *const ht, const hashtable_const_key_t key);
    hashtable_iterator_t (*hashtable_iterator)(const hashtable *const ht);
    hashtable_iterator_t (*hashtable_iterator_next)(const hashtable *const ht, hashtable_iterator_t i);

    char *(*normalize_path)(const char *src, const char *dst, char *path);
    mapstruct *(*enter_map_by_name)(object *op, const char *path, const char *src_path, int x, int y, int flags);
    int (*enter_map_by_exit)(object *op, object *exit_ob);
    int (*enter_map)(object *op, object *originator, mapstruct *newmap, int x, int y, int flags);
    const char *(*create_instance_path_sh)(player * const pl, const char * const name, int flags);
    void (*clean_tmp_map)(mapstruct *m);
    int (*map_to_player_unlink)(mapstruct *m);
    void (*map_to_player_link)(mapstruct *m, int x, int y, int flag);
    const char* (*create_safe_mapname_sh)(char const *mapname);
    char* (*normalize_path_direct)(const char *src, const char *dst, char *path);
    const char* (*path_to_name)(const char *file);
    void (*reset_instance_data)(player *pl);

    /* Global variables */
    Animations **animations;
    New_Face **new_faces;
    int *global_darkness_table;
    archetype **coins_arch;
    struct shstr_constants *shstr_cons;
    struct behaviourclass_decl *behaviourclasses;
    long *global_instance_id;
    unsigned long *pticks;
};

/*****************************************************************************/
/* Exportable functions. Any plugin should define all those.                 */
/* initPlugin        is called when the plugin initialization process starts.*/
/* removePlugin      is called before the plugin gets unloaded from memory.  */
/* getPluginProperty is currently unused.                                    */
/* registerHook      is used to transmit hook pointers from server to plugin.*/
/* triggerEvent      is called whenever an event occurs.                     */
/*****************************************************************************/
/*
extern MODULEAPI CFParm    *initPlugin(CFParm *PParm);
extern MODULEAPI CFParm    *removePlugin(CFParm *PParm);
extern MODULEAPI CFParm    *getPluginProperty(CFParm *PParm);
extern MODULEAPI CFParm    *registerHook(CFParm *PParm);
extern MODULEAPI CFParm    *triggerEvent(CFParm *PParm);
*/

/* Table of all loaded plugins */
#define    PLUGINS_MAX_NROF 32

extern CFPlugin             PlugList[PLUGINS_MAX_NROF];
extern int                  PlugNR;

#endif /*PLUGIN_H_*/
