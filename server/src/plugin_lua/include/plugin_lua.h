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
/* This is a remake of CFPython module from Crossfire. The big changes done  */
/* are the addition of real object and map objects with methods and attribs. */
/* The attributes made it possible to remove almost all Set and Get          */
/* and the separation of functions into class methods led to the split into  */
/* three c files for better overview.                                        */
/*****************************************************************************/
/* Daimonin Lua Plugin 0.1 - Jan 2005                                        */
/* Björn Axelsson                                                            */
/* Contact: gecko-at-acc.umu.se                                              */
/*****************************************************************************/

#ifndef PLUGIN_LUA_H
#define PLUGIN_LUA_H

/* First the required header files - only the module interface and lua */
#include <lua.h> 
#include <lauxlib.h> 
#include <plugin.h>

#undef MODULEAPI
#ifdef WIN32
#ifdef LUA_PLUGIN_EXPORTS
#define MODULEAPI __declspec(dllexport)
#else
#define MODULEAPI __declspec(dllimport)
#endif /* ifdef LUA_PLUGIN_EXPORTS */
#else
#define MODULEAPI
#endif /* ifdef WIN32 */

#define LUA_DEBUG   /* give us some general infos out */

#define PLUGIN_NAME    "Lua"
#define PLUGIN_VERSION "Daimonin Lua Plugin 0.1"

#define LUA_PATH "/lua"
#define LUA_INITSCRIPT "/lua/plugin_init.lua"

/* The plugin properties and hook functions. A hook function is a pointer to */
/* a CF function wrapper. Basically, most CF functions that could be of any  */
/* use to the plugin have "wrappers", functions that all have the same args  */
/* and all returns the same type of data (CFParm); pointers to those functs. */
/* are passed to the plugin when it is initialized. They are what I called   */
/* "Hooks". It means that using any server function is slower from a plugin  */
/* than it is from the "inside" of the server code, because all arguments    */
/* need to be passed back and forth in a CFParm structure, but the difference*/
/* is not a problem, unless for time-critical code sections. Using such hooks*/
/* may of course sound complicated, but it allows much greater flexibility.  */
extern CFParm          *PlugProps;
extern f_plugin         PlugHooks[1024];
extern int              cache_ref;

/* Some practical stuff, often used in the plugin */
#define WHO (self->data.object)
#define WHAT (whatptr->data.object)
#define WHERE (whereptr->data.object)

/* The declarations for the plugin interface. Every plugin should have those.*/
extern MODULEAPI CFParm    *registerHook(CFParm *PParm);
extern MODULEAPI CFParm    *triggerEvent(CFParm *PParm);
extern MODULEAPI CFParm    *initPlugin(CFParm *PParm);
extern MODULEAPI CFParm    *postinitPlugin(CFParm *PParm);
extern MODULEAPI CFParm    *removePlugin(CFParm *PParm);
extern MODULEAPI CFParm    *getPluginProperty(CFParm *PParm);

/* This one is used to cleanly pass args to the CF core */
extern CFParm           GCFP;
extern CFParm           GCFP0;
extern CFParm           GCFP1;
extern CFParm           GCFP2;

/* Those are used to handle the events. The first one is used when a player  */
/* attacks with a "scripted" weapon. HandleEvent is used for all other events*/
extern MODULEAPI int        HandleUseWeaponEvent(CFParm *CFP);
extern MODULEAPI int        HandleEvent(CFParm *CFP);
extern MODULEAPI int        HandleGlobalEvent(CFParm *CFP);

/* Called to start the Lua interpreter. */        
extern MODULEAPI void       init_Daimonin_Lua();

/* This is the new-style hook data */
struct plugin_hooklist *hooks;
/* And a macro to hookify LOG: */
#undef LOG
#define LOG hooks->LOG

struct lua_context
{
    struct lua_context *next, *prev;
    lua_State          *state; /* The actual lua environment */
    int                 threadidx; /* ref to thread object in the registry */
    // TODO: more stuff, like callback function and interval counter

    object             *self, *activator, *other; /* Involved objects */
    const char         *text;             /* Text for SAY events */

    const char         *options;          /* Options from event object */
    const char         *file;             /* Script file (not normalized) */
    int                 parm1, parm2, parm3, parm4;  /* Parameters from event */
    int                 returnvalue;              /* Return value from script */
};

enum lua_object_type
{
    LUATYPE_EVENT,
    LUATYPE_GAME,
    LUATYPE_MAP,
    LUATYPE_OBJECT,
    /* These are never exposed outside the engine */
    LUATYPE_ATTRIBUTE,
    LUATYPE_METHOD,
    LUATYPE_CONSTANT,
    LUATYPE_FLAG
};

/* An object for the lua engine. Used as full userdata
 * to wrap a server object/map/context and its type */
typedef struct lua_object_s
{
    struct  lua_class_s *class;
    union
    {
        struct lua_context     *context;
        object                 *object;
        mapstruct              *map;
        void                   *game;

        uint32                  flagno;
        struct attribute_decl  *attribute;
        struct method_decl     *method;
        struct constant_decl   *constant;

        void                   *anything;
    } data;
} lua_object;

typedef struct lua_class_s
{
    enum lua_object_type    type; 
    const char             *name;
    int                     meta;    /* ref to the metatable for class objects */

    lua_CFunction           toString;

    struct attribute_decl  *attributes;
    struct method_decl     *methods;
    struct constant_decl   *constants;
    const char             **flags;

    int (*getFlag)(lua_State *, struct lua_object_s *, uint32);
    int (*setFlag)(lua_State *, struct lua_object_s *, uint32);
    int (*setAttribute_Hook)(lua_State *, struct lua_object_s *, struct attribute_decl *, int);

    int                     obcount;
} lua_class;


/* Types used in objects and maps structs */
typedef enum
{
    FIELDTYPE_SHSTR,
    /* Pointer to shared string */
    FIELDTYPE_CSTR,
    /* Pointer to C string */
    FIELDTYPE_CARY,
    /* C string (array directly in struct) */
    FIELDTYPE_UINT8,
    FIELDTYPE_SINT8,
    FIELDTYPE_UINT16,
    FIELDTYPE_SINT16,
    FIELDTYPE_UINT32,
    FIELDTYPE_SINT32,
    FIELDTYPE_FLOAT,
    FIELDTYPE_OBJECT,
    FIELDTYPE_MAP,
    FIELDTYPE_OBJECTREF /* object pointer + tag */
}    field_type;

/* Type used for numeric constants */
struct constant_decl
{
    const char *name;
    const int   value;
};

struct method_decl
{
    const char *name;
    void       *func; /* TODO: function type */
};

struct attribute_decl
{
    const char             *name;
    field_type              type;
    uint32                  offset;    /* Offset in object struct */
    uint32                  flags;     /* flags for special handling */
    uint32                  extra_data; /* extra data for some special fields */
};

#define FLAGLIST_END_MARKER "$end$"

/* Special flags for object attribute access */
#define FIELDFLAG_READONLY        1 /* changing value not allowed */
#define FIELDFLAG_PLAYER_READONLY 2 /* changing value is not allowed if object is a player */
#define FIELDFLAG_PLAYER_FIX      4 /* fix player or monster after change */

/* Utility functions in lua_support.c */
void                    dumpStack(lua_State *L);
int                     init_class(struct lua_State *s, lua_class *class);
int                     push_object(lua_State *s, lua_class *class, void *data);
void                    get_lua_args(lua_State *L, const char *fmt, ...);
int                     init_file_cache(struct lua_State *L);
int                     load_file_cache(struct lua_State *L, const char *file);

extern lua_class        GameObject;
extern lua_class        Map;
extern int              GameObject_init(lua_State *s);
extern int              Map_init(lua_State *s);

/*****************************************************************************/
/* Commands management part.                                                 */
/* It is now possible to add commands to crossfire. The following stuff was  */
/* created to handle such commands.                                          */
/*****************************************************************************/

/* The "About Lua" stuff. Bound to "python" command. */
extern MODULEAPI int    cmd_aboutLua(object *op, char *params);
/* The following one handles all custom Python command calls. */
extern MODULEAPI int    cmd_customLua(object *op, char *params);

#if 0
/* This structure is used to define one python-implemented crossfire command.*/
typedef struct PythonCmdStruct
{
    char *name;    /* The name of the command, as known in the game.    */
    char *script;  /* The name of the script file to bind.              */
    double speed;   /* The speed of the command execution.                   */
} PythonCmd;

/* This plugin allows up to 1024 custom commands.                            */
#define NR_CUSTOM_CMD 1024
extern PythonCmd CustomCommand[NR_CUSTOM_CMD];
/* This one contains the index of the next command that needs to be run. I do*/
/* not like the use of such a global variable, but it is the most convenient */
/* way I found to pass the command index to cmd_customPython.                */
extern int NextCustomCommand;
#endif
#endif /*PLUGIN_PYTHON_H*/
