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

#include <global.h>
#include <plugin_lua.h>

#include <lualib.h>

#include <stdarg.h>

#undef MODULEAPI
#ifdef WIN32
#define MODULEAPI __declspec(dllexport)
#else
#define MODULEAPI
#endif

/* Global data objects */

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
CFParm                         *PlugProps;
f_plugin                        PlugHooks[1024];

/* New-styled hooks */
struct plugin_hooklist         *hooks;

/* script context list */
struct lua_context             *first_context;
/* Tag counter for contexts */
tag_t lua_context_tag_counter;

/* Our global state from which we spawn threads for running scripts */
struct lua_State               *global_state;

/* possibly reference to error handler function */
static int error_handler_ref = LUA_NOREF;
static int globals_metatable_ref = LUA_NOREF;

/* Memory pool for our contexts */
struct mempool *pool_luacontext;

/*
 * - Event contains the script-related attributes such as activator,
 *   message and return value.
 *   It also carries (invisible to scripts) the move_response data for
 *   movement responses.
 */

static int  Event_isValid(lua_State *L, lua_object *obj);

static struct attribute_decl    Event_attributes[]  =
{
    {"me", FIELDTYPE_OBJECTREF, offsetof(struct lua_context, self), FIELDFLAG_READONLY, offsetof(struct lua_context, self_tag)},
    {"activator", FIELDTYPE_OBJECTREF, offsetof(struct lua_context, activator), FIELDFLAG_READONLY, offsetof(struct lua_context, activator_tag)},
    {"other", FIELDTYPE_OBJECTREF, offsetof(struct lua_context, other), FIELDFLAG_READONLY, offsetof(struct lua_context, other_tag)},
    {"message", FIELDTYPE_SHSTR, offsetof(struct lua_context, text), FIELDFLAG_READONLY},
    {"options", FIELDTYPE_SHSTR, offsetof(struct lua_context, options), FIELDFLAG_READONLY},
    {"returnvalue", FIELDTYPE_SINT32, offsetof(struct lua_context, returnvalue), 0},
    {NULL}
};

/* Basic script classes */
lua_class Event =
{
    LUATYPE_EVENT, "Event", 0, NULL, Event_attributes, NULL,
    NULL, NULL, NULL, NULL, NULL, Event_isValid
};

/*
 * Other globals
 */

#if 0
/* Commands management part */
Lua   Cmd CustomCommand[NR_CUSTOM_CMD];
int NextCustomCommand;
#endif

/*****************************************************************************/
/* Some lua runtime functions                                                */
/*****************************************************************************/

/* Our error handler. Tries to call a lua error handler called "_error" */
static int luaError(lua_State *L)
{
    if (error_handler_ref != LUA_NOREF)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, error_handler_ref);
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_setfenv(L, -2); /* Make sure it runs in the thread environment */
        lua_pushvalue(L, 1);
        lua_call(L,1,1);
        return 1;
    }
    else
        return 1;
}

/* Try to find the file 'file' with the help of the
   variable LUA_PATH in the global table of 'L' */
static int luaFindFile(lua_State *L, const char *filename, const char **path)
{
    static char buf[MAX_BUF];
    char        lua_path[MAX_BUF];
    const char *part;
    int         i, j, part_size = 0, replace = 0;
    struct stat stat_buf;
    size_t      size;

    /* Get LUA_PATH */
    lua_pushstring(L, "LUA_PATH");
    lua_gettable(L, LUA_GLOBALSINDEX);
    strncpy(lua_path, lua_tostring(L, -1), MAX_BUF);
    lua_pop(L, 1);

    /* Check if this file was already found */
    lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
    lua_pushstring(L, filename);
    lua_rawget(L, -2);

    if (lua_isfunction(L, -1))
    {
        /* The filename is already a valid path */
        *path = filename;
        lua_pop(L, 2);
        return 0;
    }
    else if (lua_isstring(L, -1))
    {
        /* The path for this filename was already found */
        *path = lua_tostring(L, -1);
        lua_pop(L, 2);
        return 0;
    }
    lua_pop(L, 2);

    /* Set part and size */
    part = lua_path;
    size = strlen(filename);

    for (; ;)
    {
        /* Reset part_size */
        part_size = 0;

        /* Get the size of the next part */
        while (part[part_size] != ';' && part[part_size] != '\0' && part_size != MAX_BUF - 2)
        {
            /* Count how many replacements in this part must be done */
            if (part[part_size] == '?')
                ++replace;
            ++part_size;
        }

        /* Go to the next part if the current is empty */
        if (!part_size)
        {
            part += 1;
            continue;
        }

        /* Reset buf, i and j */
        memset(buf, 0, MAX_BUF);
        i = 0;
        j = 0;

        /* Do we must replace something in this part? */
        if (replace)
        {
            /* Copy every character and insert instead of the '?'
               the filename until there are no '?'s left or the
               limit of the buffer is reached */
            for (; replace && j != MAX_BUF - 2; ++i, ++j)
            {
                if (part[i] != '?')
                    buf[j] = part[i];
                else if (j + size < MAX_BUF - 2)
                {
                    strncpy(buf + j, filename, size);
                    j += size - 1;
                    --replace;
                }
            }
        }

        /* Now simply copy (the rest of) the part into the buffer
           until the buffer is full or the whole part is copied */
        if (i != part_size)
        {
            strncpy(buf + j, part + i, part_size - i);
            j += part_size - i;
        }

        /* Try to find the file */
        if (!stat(buf, &stat_buf))
        {
            i = 0;
            break;
        }

        if (part[part_size] != '\0')
            part += part_size + 1;
        else
        {
            i = 1;
            break;
        }
    }

    /* If we couldn't find the file push an error
       message on the stack and return */
    if (i)
    {
        lua_pushfstring(L, "couldn't find script file %s", filename);
        return LUA_ERRFILE;
    }

    *path = buf;

    return 0;
}

/* Load 'file' */
static int luaLoadFile(lua_State *L, const char *file)
{
    const char *path = hooks->create_mapdir_pathname(file);
    int         res = 0;

    if ((res = luaFindFile(L, file, &path)) == 0)
    {
        if ((res = load_file_cache(L, path)) == 0)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
            lua_pushstring(L, file);
            lua_rawget(L, -2);

            if (!lua_isfunction(L, -1))
            {
                lua_pop(L, 1);
                lua_pushstring(L, file);
                lua_pushstring(L, path);
                lua_rawset(L, -3);
            }

            lua_pop(L, 1);
        }
    }

    return res;
}

/* Our own 'require' function that supports auto-compilation and
 * caching.
 */
static int luaRequire(lua_State *L)
{
    const char *file;
    int         res;

    get_lua_args(L, "s", &file);

    if ((res = luaLoadFile(L, file)) == 0)
    {
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_setfenv(L, -2);

        lua_call(L, 0, 0);
    }
    else
        lua_error(L);

    return 0;
}

/* Our replacement "type" function that supports our object model */
static int luaType(lua_State *L)
{
    luaL_checkany(L, 1);
    if(lua_isuserdata(L, 1))
    {
        lua_object *obj = lua_touserdata(L, 1);
        lua_pushstring(L, obj->class->name);
    } else
    {
        lua_pushstring(L, lua_typename(L, lua_type(L, 1)));
    }
    return 1;
}

/*****************************************************************************/
/* Detached scripts handling                                                 */
/*****************************************************************************/

/* Insert a context in the global list */
void lua_context_insert(struct lua_context *context)
{
    context->next = first_context;
    first_context = context;
    if(context->next)
        context->next->prev = context;
}

/* Remove a context from the global list */
void lua_context_remove(struct lua_context *context)
{
    if(first_context == context)
        first_context = context->next;
    if(context->prev)
        context->prev->next = context->next;
    if(context->next)
        context->next->prev = context->prev;
    context->next = context->prev = NULL;
}

void detach_lua_context(struct lua_context *context, int resume_time)
{
#ifdef LUA_DEBUG
    LOG(llevDebug, "LUA - Detaching context (%s)\n", context->file);
#endif
    context->resume_time = resume_time;
    if(first_context == NULL)
    {
        /* Register for global tick events */
        CFParm CFP;
        int evt = EVENT_CLOCK;
        CFP.Value[0] = (void *)(&evt);
        CFP.Value[1] = (void *)PLUGIN_NAME;
        (PlugHooks[HOOK_REGISTEREVENT])(&CFP);
    }

    lua_context_insert(context);
}

void terminate_lua_context(struct lua_context *context)
{
#ifdef LUA_DEBUG_ALL
    LOG(llevDebug, "LUA - Terminating context (%s)\n", context->file);
#endif
    if(context->prev || context->next || first_context == context) {
        lua_context_remove(context);
        if(first_context == NULL)
        {
            /* Unregister for global tick events */
            CFParm CFP;
            int evt = EVENT_CLOCK;
            CFP.Value[0] = (void *)(&evt);
            CFP.Value[1] = (void *)PLUGIN_NAME;
            (PlugHooks[HOOK_UNREGISTEREVENT])(&CFP);
        }
    }

    /* Get rid of the thread object, which should leave the thread for gc */
    luaL_unref(global_state, LUA_REGISTRYINDEX, context->threadidx);
    context->tag = 0;

    FREE_ONLY_HASH(context->text);
    FREE_ONLY_HASH(context->file);
    FREE_ONLY_HASH(context->options);

    return_poolchunk(context, pool_luacontext);
}

/* TODO: More efficient would be to keep the contexts sorted by time
 * to resume (like an event queue). Then at every tick (or by using
 * timers, if they work (?)) just pick the n first contexts that
 * have triggered, not even traversing the rest.
 */
void resume_detached_contexts()
{
    struct lua_context *context, *next_context = NULL;
    for(context = first_context; context != NULL; context = next_context)
    {
        next_context = context->next;
        context->resume_time--;
        if(context->resume_time < 0) {
            lua_State *L = context->state;
            int res;
#ifdef LUA_DEBUG
    LOG(llevDebug, "LUA - Resuming detached script: %s\n", context->file);
#endif
            res = lua_resume(L, 0);

            if (res == 0)
            {
                /* Handle scripts that just wants to yield, not end */
                if(lua_isnumber(L, -1) || lua_isstring(L, -1))  {
                    context->resume_time = (int)(lua_tonumber(L, -1)* (lua_Number)(1000000 / MAX_TIME));
                }
            } else
            {
                const char *error;

                if ((error = lua_tostring(L, -1)))
                    LOG(llevDebug, "LUA - %s\n", error);
                else
                    LOG(llevDebug, "LUA - unknown error %d type %s\n", res,
                            lua_typename(L, lua_type(L, -1)));
            }

            if(context->resume_time < 0) {
                terminate_lua_context(context);
            }
        }
    }
}

/*****************************************************************************/
/* The Plugin Management Part.                                               */
/* Most of the functions below should exist in any CF plugin. They are used  */
/* to glue the plugin to the server core. All functions follow the same      */
/* declaration scheme (taking a CFParm* arg, returning a CFParm) to make the */
/* plugin interface as general as possible. And since the loading of modules */
/* isn't time-critical, it is never a problem. It could also make using      */
/* programming languages other than C to write plugins a little easier, but  */
/* this has yet to be proven.                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Called whenever a Hook Function needs to be connected to the plugin.      */
/*****************************************************************************/
MODULEAPI CFParm * registerHook(CFParm *PParm)
{
    int         Pos;
    f_plugin    Hook;
    Pos = *(int *) (PParm->Value[0]);
    Hook = (f_plugin) (PParm->Value[1]);
    PlugHooks[Pos] = Hook;
    return NULL;
}

/*****************************************************************************/
/* Called to send the hooks struct to the plugin.                            */
/*****************************************************************************/
MODULEAPI void registerHooks(struct plugin_hooklist *hooklist)
{
    hooks = hooklist;
}


/*****************************************************************************/
/* Called whenever an event is triggered, both Local and Global ones.        */
/*****************************************************************************/
/* Two types of events exist in CF:                                          */
/* - Local events: They are triggered by a single action on a single object. */
/*                 Nearly any object can trigger a local event               */
/*                 To warn the plugin of a local event, the map-maker needs  */
/*                 to use the event... tags in the objects of their maps.    */
/* - Global events: Those are triggered by actions concerning CF as a whole. */
/*                 Those events may or may not be triggered by a particular  */
/*                 object; they can't be specified by event... tags in maps. */
/*                 The plugin should register itself for all global events it*/
/*                 wants to be aware of.                                     */
/* Why those two types ? Local Events are made to manage interactions between*/
/* objects, for example to create complex scenarios. Global Events are made  */
/* to allow logging facilities and server management. Global Events tends to */
/* require more CPU time than Local Events, and are sometimes difficult to   */
/* bind to any specific object.                                              */
/*****************************************************************************/
MODULEAPI int triggerEvent(CFParm *PParm)
{
    int eventcode;
    int result;

    eventcode = *(int *) (PParm->Value[0]);
    switch (eventcode)
    {
        case EVENT_NONE:
          LOG(llevDebug, "LUA - Warning - EVENT_NONE requested\n");
          break;
        case EVENT_ATTACK:
        case EVENT_APPLY:
        case EVENT_DEATH:
        case EVENT_DROP:
        case EVENT_PICKUP:
        case EVENT_SAY:
        case EVENT_TALK:
        case EVENT_STOP:
        case EVENT_TELL:
        case EVENT_TIME:
        case EVENT_THROW:
        case EVENT_TRIGGER:
        case EVENT_CLOSE:
        case EVENT_EXAMINE:
        case EVENT_AI_BEHAVIOUR:
#ifdef LUA_DEBUG_ALL
    LOG(llevDebug, "LUA - triggerEvent:: eventcode %d\n", eventcode);
#endif
          result = HandleEvent(PParm);
          break;
        case EVENT_BORN:
        case EVENT_CRASH:
        case EVENT_LOGIN:
        case EVENT_LOGOUT:
        case EVENT_REMOVE:
        case EVENT_SHOUT:
        case EVENT_MAPENTER:
        case EVENT_MAPLEAVE:
        case EVENT_CLOCK:
        case EVENT_MAPRESET:
          result = HandleGlobalEvent(PParm);
          break;
    }

    return result;
}

/*****************************************************************************/
/* Handles standard global events.                                            */
/*****************************************************************************/
MODULEAPI int HandleGlobalEvent(CFParm *PParm)
{
    CFParm parm;

    switch(*(int *)(PParm->Value[0]))
    {
        case EVENT_CLOCK:
            resume_detached_contexts();
            break;

        case EVENT_LOGOUT:
            memcpy(&parm, PParm, sizeof(parm));
            parm.Value[9] = "/lua/event_logout.lua";
            HandleEvent(&parm);
            break;

        default:
            LOG(llevDebug, "Unimplemented for now\n");
            break;
#if 0
        case EVENT_CRASH:
            LOG(llevDebug, "Unimplemented for now\n");
            break;
        case EVENT_BORN:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event BORN generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */
            RunLua   Script("python/python_born.py");
            break;
        case EVENT_LOGIN:
            StackActivator[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackWho[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event LOGIN generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */
            /*LOG(llevDebug, "IP is %s\n", (char *)(PParm->Value[2])); */
            RunLua   Script("python/python_login.py");
            break;
        case EVENT_LOGOUT:
            StackActivator[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackWho[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event LOGOUT generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */
            RunLua   Script("python/python_logout.py");
            break;
        case EVENT_REMOVE:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event REMOVE generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            RunLua   Script("python/python_remove.py");
            break;
        case EVENT_SHOUT:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event SHOUT generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            /*LOG(llevDebug, "Message shout is %s\n",StackText[StackPosition]); */
            RunLua   Script("python/python_shout.py");
            break;
        case EVENT_MAPENTER:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event MAPENTER generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            RunLua   Script("python/python_mapenter.py");
            break;
        case EVENT_MAPLEAVE:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event MAPLEAVE generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            RunLua   Script("python/python_mapleave.py");
            break;
        case EVENT_CLOCK:
            /* LOG(llevDebug, "Event CLOCK generated\n"); */
            RunLua   Script("python/python_clock.py");
            break;
        case EVENT_MAPRESET:
            StackText[StackPosition] = (char *)(PParm->Value[1]);/* Map name/path */
            LOG(llevDebug, "Event MAPRESET generated by %s\n", StackText[StackPosition]);

            RunLua   Script("python/python_mapreset.py");
            break;
#endif
    }

    return 0;
}

/********************************************************************/
/* Execute a script, handling loading, parsing and caching          */
/********************************************************************/

static int RunLuaScript(struct lua_context *context)
{
    struct lua_State   *L   = context->state;
    int                 res = 0;
    const char         *error;

    /* Set up a new, empty env for the new thread */
    lua_newtable(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, globals_metatable_ref);
    lua_setmetatable(L, -2);
    lua_replace(L, LUA_GLOBALSINDEX);

    /* "next" and "ipairs" _must_ be available in the local env */
    lua_pushliteral(global_state, "next");
    lua_pushvalue(global_state, -1);
    lua_rawget(global_state, LUA_GLOBALSINDEX);
    lua_xmove(global_state, L, 2);
    lua_rawset(L, LUA_GLOBALSINDEX);
    lua_pushliteral(global_state, "ipairs");
    lua_pushvalue(global_state, -1);
    lua_rawget(global_state, LUA_GLOBALSINDEX);
    lua_xmove(global_state, L, 2);
    lua_rawset(L, LUA_GLOBALSINDEX);

    /* Set up the event object for this call */
    lua_pushliteral(L, "event");
    push_object(L, &Event, context);
    lua_rawset(L, LUA_GLOBALSINDEX);

    /* Load the actual script function */
    res = luaLoadFile(L, context->file);
    if (res == 0)
    {
        /* First set up environment for the function. */
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_setfenv(L, -2);

        lua_pushcfunction(L, luaError);
        lua_pushvalue(L, -2);
        lua_remove(L, -3);

        /* Call the function as a coroutine */
        res = lua_coroutine(L, 0, -2);

        if (res == 0)
        {
            /* Handle scripts that just wants to yield, not end */
            if(lua_isnumber(L, -1) || lua_isstring(L, -1))
                detach_lua_context(context, (int)(lua_tonumber(L, -1) * (lua_Number)(1000000 / MAX_TIME)));

            return 0;
        }
    }

    if ((error = lua_tostring(L, -1)))
        LOG(llevDebug, "LUA - %s\n", error);
    else
        LOG(llevDebug, "LUA - unknown error %d type %s\n", res,
                lua_typename(L, lua_type(L, -1)));

    return -1;
}

/*****************************************************************************/
/* Handles standard local events.                                            */
/*****************************************************************************/
MODULEAPI int HandleEvent(CFParm *PParm)
{
    struct lua_context *context;
    int                 ret, res;

#ifdef LUA_DEBUG
    LOG(llevDebug, "LUA - HandleEvent:: start script file >%s<\n", (char *) (PParm->Value[9]));
    LOG(llevDebug, "LUA - call data:: o1:>%s< o2:>%s< o3:>%s< text:>%s< i1:%d i2:%d i3:%d i4:%d\n",
        STRING_OBJ_NAME((object *) (PParm->Value[1])), STRING_OBJ_NAME((object *) (PParm->Value[2])),
        STRING_OBJ_NAME((object *) (PParm->Value[3])), STRING_SAFE((char *) (PParm->Value[4])),
        PParm->Value[5] ? *(int *) (PParm->Value[5]) : 0,
        PParm->Value[6] ? *(int *) (PParm->Value[6]) : 0,
        PParm->Value[7] ? *(int *) (PParm->Value[7]) : 0,
        PParm->Value[8] ? *(int *) (PParm->Value[8]) : 0);
#endif

    if(PParm->Value[9] == NULL)
    {
        LOG(llevBug, "LUA - event triggered without script path");
        return 0;
    }

    context = get_poolchunk(pool_luacontext);

    context->tag = ++lua_context_tag_counter;
    context->next = context->prev = NULL;
    context->resume_time = -1;
    context->state = lua_newthread(global_state);
    context->threadidx = luaL_ref(global_state, LUA_REGISTRYINDEX);

    /* And all the event parameters */
    context->activator = (object *) (PParm->Value[1]);
    context->activator_tag = context->activator ? context->activator->count : 0;
    context->self = (object *) (PParm->Value[2]);
    context->self_tag = context->self ? context->self->count : 0;
    context->other = (object *) (PParm->Value[3]);
    context->other_tag = context->other ? context->other->count : 0;
    context->text = PParm->Value[4] ? hooks->add_string((const char *) (PParm->Value[4])) : NULL;
    context->parm1 = PParm->Value[5] ? *(int *) (PParm->Value[5]) : 0;
    context->parm2 = PParm->Value[6] ? *(int *) (PParm->Value[6]) : 0;
    context->parm3 = PParm->Value[7] ? *(int *) (PParm->Value[7]) : 0;
    context->parm4 = PParm->Value[8] ? *(int *) (PParm->Value[8]) : 0;
    context->options = PParm->Value[10] ? hooks->add_string((const char *) (PParm->Value[10])) : NULL;
    context->returnvalue = 0;

    /* Try to normalize file name if needed */
    if(((const char *)PParm->Value[9])[0] == '/')
        context->file = hooks->add_string((const char *) (PParm->Value[9]));
    else
    {
        /* We need a base path. */
        char buf[HUGE_BUF];
        object *outermost = context->self;
        while(outermost && outermost->env)
            outermost = outermost->env;
        if(outermost == NULL || outermost->map == NULL || outermost->map->path == NULL)
        {
            LOG(llevBug, "LUA: script path %s of %s in container %s is relative but no map is available for path reference\n", (const char *) (PParm->Value[9]), STRING_OBJ_NAME(context->self), STRING_OBJ_NAME(outermost));
            context->file = NULL;
            terminate_lua_context(context);
            return 0;
        }
        hooks->normalize_path(outermost->map->orig_path, (const char *) (PParm->Value[9]), buf);
        context->file = hooks->add_string(buf);
#ifdef LUA_DEBUG
        LOG(llevDebug, "LUA: normalized script path: %s\n", context->file);
#endif
    }


    if(*(int *)PParm->Value[0] == EVENT_AI_BEHAVIOUR)
        context->move_response = PParm->Value[11];
    else
        context->move_response = NULL;


    res = RunLuaScript(context);

    if (res)
    {
        terminate_lua_context(context);
        return 0;
    }

#ifdef LUA_DEBUG
    LOG(llevDebug, "fixing. ");
#endif

    if (context->parm4 == SCRIPT_FIX_ALL)
    {
        if (context->other && context->other->type == PLAYER &&
                QUERY_FLAG(context->other, FLAG_FIX_PLAYER))
        {
            CLEAR_FLAG(context->other, FLAG_FIX_PLAYER);
            hooks->FIX_PLAYER(context->other, "LUA: script fix - other");
        }
        if (context->self && context->self->type == PLAYER &&
                QUERY_FLAG(context->self, FLAG_FIX_PLAYER))
        {
            CLEAR_FLAG(context->self, FLAG_FIX_PLAYER);
            hooks->FIX_PLAYER(context->self, "LUA: script fix - self");
        }
        if (context->activator && context->activator->type == PLAYER &&
                QUERY_FLAG(context->activator, FLAG_FIX_PLAYER))
        {
            CLEAR_FLAG(context->activator, FLAG_FIX_PLAYER);
            hooks->FIX_PLAYER(context->activator, "LUA: script fix - activator");
        }
    }
    else if (context->parm4 == SCRIPT_FIX_ACTIVATOR &&
            context->activator && context->activator->type == PLAYER &&
            QUERY_FLAG(context->activator, FLAG_FIX_PLAYER))
    {
        CLEAR_FLAG(context->activator, FLAG_FIX_PLAYER);
        hooks->FIX_PLAYER(context->activator, "LUA: script fix - activator2");
    }

    ret = context->returnvalue;

    if(context->resume_time == -1)
        terminate_lua_context(context);

#ifdef LUA_DEBUG
    LOG(llevDebug, "done (returned: %d)!\n", ret);
#endif

    return ret;
}

/*****************************************************************************/
/* Plugin initialization.                                                    */
/*****************************************************************************/
/* It is required that:                                                      */
/* - The first returned value of the CFParm structure is the "internal" name */
/*   of the plugin, used by objects to identify it.                          */
/* - The second returned value is the name "in clear" of the plugin, used for*/
/*   information purposes.                                                   */
/*****************************************************************************/
MODULEAPI int initPlugin(CFParm *PParm, const char **name, const char **version)
{
    LOG(llevDebug, "    Daimonin Lua Plugin loading.....\n");

    init_Daimonin_Lua();

    *name = PLUGIN_NAME;
    *version = PLUGIN_VERSION;
    return 0;
}

/*****************************************************************************/
/* Used to do cleanup before killing the plugin.                             */
/*****************************************************************************/
MODULEAPI CFParm * removePlugin(CFParm *PParm)
{
    LOG(llevDebug, "    Daimonin Lua Plugin unloading.....\n");

    /* Try to find the function '_shutdown' and run it */
    lua_pushliteral(global_state, "_shutdown");
    lua_rawget(global_state, LUA_GLOBALSINDEX);
    if (lua_isfunction(global_state, -1))
        lua_call(global_state, 0, 0);

    /* TODO: Terminate all detached threads */

    lua_close(global_state);

    hooks->free_mempool(pool_luacontext);

    return NULL;
}

/*****************************************************************************/
/* This function is called to ask various informations to the plugin.        */
/*****************************************************************************/
MODULEAPI int getPluginProperty(CFParm *PParm, CommArray_s *RTNCmd)
{
    if (PParm != NULL)
    {
        if (PParm->Value[0] && !strcmp((char *) (PParm->Value[0]), "command?"))
        {
            if (PParm->Value[1] && !strcmp((char *) (PParm->Value[1]), PLUGIN_NAME))
            {
                RTNCmd->name = PParm->Value[1];
                RTNCmd->func = cmd_aboutLua;
                RTNCmd->time = 0.0;
                return 1;
            }
            else
            {
#if 0
                int i;
                for (i=0;i<NR_CUSTOM_CMD;i++)
                {
                    if (CustomCommand[i].name)
                    {
                        if (!strcmp(CustomCommand[i].name,(char *)(PParm->Value[1])))
                        {
                            RTNCmd->name = PParm->Value[1];
                            RTNCmd->func = cmd_customLua;
                            RTNCmd->time = CustomCommand[i].speed;
                            NextCustomCommand = i;
                            return 1;
                        }
                    }
                }
#endif
            }
        }
        else
        {
            LOG(llevDebug, "LUA - Unknown property tag: %s\n", (char *) (PParm->Value[0]));
        }
    }
    return 0;
}

#if 0
MODULEAPI int cmd_customLua   (object *op, char *params)
{
#ifdef PYTHON_DEBUG
    LOG(llevDebug, "PYTHON - cmd_customLua    called:: script file: %s\n",CustomCommand[NextCustomCommand].script);
#endif
    if (StackPosition == MAX_RECURSIVE_CALL)
    {
        LOG(llevDebug, "PYTHON - Can't execute script - No space left of stack\n");
        return 0;
    }
    StackPosition++;
    StackActivator[StackPosition]   = op;
    StackWho[StackPosition]         = op;
    StackOther[StackPosition]       = op;
    StackText[StackPosition]        = params;
    StackReturn[StackPosition]      = 0;

    RunLua   Script(CustomCommand[NextCustomCommand].script);

    return StackReturn[StackPosition--];
}
#endif

MODULEAPI int cmd_aboutLua(object *op, char *params)
{
    return 0;
}

/*****************************************************************************/
/* The postinitPlugin function is called by the server when the plugin load  */
/* is complete. It lets the opportunity to the plugin to register some events*/
/*****************************************************************************/
MODULEAPI CFParm * postinitPlugin(CFParm *PParm)
{
    /*    int i; */
    /* We can now register some global events if we want */
    /* We'll only register the global-only events :      */
    /* BORN, CRASH, LOGIN, LOGOUT, REMOVE, and SHOUT.    */
    /* The events APPLY, ATTACK, DEATH, DROP, PICKUP, SAY*/
    /* STOP, TELL, TIME, THROW and TRIGGER are already   */
    /* handled on a per-object basis and I simply don't  */
    /* see how useful they could be for the Lua    stuff.*/
    /* Registering them as local would be probably useful*/
    /* for extended logging facilities.                  */

    /* this is a extrem silly code part to remove a linker warning
    * from VS c++ 6.x build. The optimizer will drop a warning that
    * a function (timeGettime() ) is not used inside gettimeofday() and
    * so he can remove the whole system .lib where it is in. This also means
    * its not needed to load the .dll at runtime and thats what it tell us.
    * this force a call and remove the warning from build. Its redundant
    * code to give us a warning free build... without using any #ifdef
    * or pragma.
    */
    /*
    struct timeval  new_time;
    (void) GETTIMEOFDAY(&new_time);
    */
    CFParm CFP;
    int i;

    LOG(llevDebug, "LUA - Start postinitPlugin.\n");

    CFP.Value[1] = (void *)PLUGIN_NAME;
    
    /* Register for logout events (needed by the datastore system) */
    i = EVENT_LOGOUT;
    CFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&CFP);

    /*
       i = EVENT_BORN;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
       i = EVENT_CRASH;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
       i = EVENT_LOGIN;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
       i = EVENT_LOGOUT;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
       i = EVENT_REMOVE;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
       i = EVENT_SHOUT;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
       i = EVENT_MAPENTER;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
       i = EVENT_MAPLEAVE;
       GCFP.Value[0] = (void *)(&i);
       (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
    */
    /*    i = EVENT_CLOCK; */
    /*    GCFP.Value[0] = (void *)(&i); */
    /*    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP); */
    /*
        i = EVENT_MAPRESET;
        GCFP.Value[0] = (void *)(&i);
        (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
    */
    return NULL;
}

/*****************************************************************************/
/* Initializes the Lua interpreter.                                          */
/*****************************************************************************/

/* Tests if an event object is valid */
static int Event_isValid(lua_State *L, lua_object *obj)
{
    return obj->data.context->tag == obj->tag;
}


MODULEAPI void init_Daimonin_Lua()
{
    int     res;
    char    lua_path[MAX_BUF];
    char   *map_path;

    strcpy(lua_path, hooks->create_mapdir_pathname(LUA_PATH));
    map_path = hooks->create_mapdir_pathname("");

    pool_luacontext = hooks->create_mempool("lua contexts", 5, sizeof(struct lua_context), 0, NULL, NULL, NULL, NULL);

    global_state = lua_open();

    /* Initialize the libs */
    luaopen_base(global_state);
    luaopen_string(global_state);
    luaopen_table(global_state);
    luaopen_math(global_state);
    luaopen_io(global_state);
    luaopen_debug(global_state);

    /* Initialize the classes */
    init_class(global_state, &Event);
    Game_init(global_state);
    GameObject_init(global_state);
    Map_init(global_state);
    AI_init(global_state);

    /* Set up the global Game object  */
    lua_pushliteral(global_state, "game");
    push_object(global_state, &Game, "Daimonin");
    lua_rawset(global_state, LUA_GLOBALSINDEX);

    /* Add our own 'type' function */
    lua_pushstring(global_state, "type");
    lua_pushcclosure(global_state, luaType, 0);
    lua_rawset(global_state, LUA_GLOBALSINDEX);

    /* Set up module search path; prefer compiled files */
    lua_pushliteral(global_state, "LUA_PATH");
    lua_pushfstring(global_state, "%s?;%s?.lc;%s?.lua;%s/?;%s/?.lc;%s/?.lua;?;?.lc;?.lua", map_path, map_path, map_path,
                    lua_path, lua_path, lua_path);
    lua_rawset(global_state, LUA_GLOBALSINDEX);

    init_file_cache(global_state);

    /* Call the initialization script */
    res = luaL_loadfile(global_state, hooks->create_mapdir_pathname(LUA_INITSCRIPT));
    if (res == 0)
    {
        /* Loadfile puts the loaded chunk on top of the stack as a function,
         * we call it without parameters and not caring about return values */
        res = lua_pcall(global_state, 0, 0, 0);

        if(res == 0) /* everything ok? */
        {
            /* See if we got an error handler? */
            lua_pushliteral(global_state, "_error");
            lua_rawget(global_state, LUA_GLOBALSINDEX);
            if(lua_isfunction(global_state, -1))
                error_handler_ref = luaL_ref(global_state, LUA_REGISTRYINDEX);
            else
                lua_pop(global_state, 1);
        } else
            lua_pop(global_state, 1);
    }
    else
    {
        if (lua_tostring(global_state, -1))
            LOG(llevDebug, "LUA - %s\n", lua_tostring(global_state, -1));
        lua_pop(global_state, 1);
    }

    /* Add our own 'require' function */
    lua_pushstring(global_state, "require");
    lua_pushcclosure(global_state, luaRequire, 0);
    lua_rawset(global_state, LUA_GLOBALSINDEX);

    /* Set up a metatable for thread global environments */
    lua_newtable(global_state);
    lua_pushliteral(global_state, "__index");
    lua_pushvalue(global_state, LUA_GLOBALSINDEX);
    lua_rawset(global_state, -3);
    globals_metatable_ref = luaL_ref(global_state, LUA_REGISTRYINDEX);

#if 0
        for (i=0;i<NR_CUSTOM_CMD;i++)
        {
            CustomCommand[i].name   = NULL;
            CustomCommand[i].script = NULL;
            CustomCommand[i].speed  = 0.0;
        }
#endif
}

/*
 * Old stuff for possible future reimplementation
 */

#if 0
/*****************************************************************************/
/* Name   : Game_RegisterCommand                                             */
/* Lua    : game.RegisterCommand(cmdname, scriptname, speed)                 */
/* Status : Untested                                                         */
/*****************************************************************************/
/* pretty untested... */
static int Game_RegisterCommand(lua_State *L)
{
    char *cmdname;
    char *scriptname;
    double cmdspeed;
    int i;

    if (!PyArg_ParseTuple(args, "ssd",&cmdname,&scriptname,&cmdspeed))
        return NULL;

    for (i=0;i<NR_CUSTOM_CMD;i++)
    {
        if (CustomCommand[i].name)
        {
            if (!strcmp(CustomCommand[i].name,cmdname))
            {
                LOG(llevDebug, "PYTHON - This command is already registered !\n");
                RAISE("This command is already registered");
            }
        }
    }
    for (i=0;i<NR_CUSTOM_CMD;i++)
    {
        if (CustomCommand[i].name == NULL)
        {
            CustomCommand[i].name = (char *)(malloc(sizeof(char)*strlen(cmdname)));
            CustomCommand[i].script = (char *)(malloc(sizeof(char)*strlen(scriptname)));
            strcpy(CustomCommand[i].name,cmdname);
            strcpy(CustomCommand[i].script,scriptname);
            CustomCommand[i].speed = cmdspeed;
            i = NR_CUSTOM_CMD;
        }
    }

    return 0;
}
#endif
