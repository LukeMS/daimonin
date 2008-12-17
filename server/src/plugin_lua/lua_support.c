/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Application

    Copyright (C) 2001 Michael Toennies
    lua_support.c Copyright (C) 2005 Björn Axelsson

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
 * The functions in this file are mostly support functions for our object model
 * and minor extensions to the Lua C API
 */

#include <global.h>
#include <plugin_lua.h>

/* Index to the lua table we use for script caching */
int                 cache_ref   = LUA_NOREF;

static int  get_attribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib);
static int  set_attribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib);

/* Internally used pseudo-classes, not accessible from scripts */
static lua_class    Attribute   =
{
    LUATYPE_ATTRIBUTE, "Attribute", 0, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0
};
static lua_class    Method      =
{
    LUATYPE_METHOD, "Method", 0, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0
};
static lua_class    Constant    =
{
    LUATYPE_CONSTANT, "Constant", 0, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0
};
static lua_class    Flag        =
{
    LUATYPE_FLAG, "Flag", 0, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0
};

/*
 * Debug functions
 */

/* Lua debug function */
void dumpStack(lua_State *L)
{
    int i, n = lua_gettop(L);
    LOG(llevDebug, "Lua stack contents:\n");
    for (i = 1; i <= n; i++)
    {
        LOG(llevDebug, " %d - %s: \"%s\"\n", i, lua_typename(L, lua_type(L, i)),
            lua_tostring(L, i) ? lua_tostring(L, i) : "?");
    }
}

/*
 * luaCFunctions
 */

/* Generic "tostring" metamethod for our object model,
 * can be overridden by classes
 */
static int toString_generic(struct lua_State *L)
{
    lua_object *obj = lua_touserdata(L, 1);

    if (obj)
    {
        lua_pushfstring(L, "(%s)", obj->class->name);
    }
    else
    {
        lua_pushstring(L, "(error)");
    }

    return 1;
}

/* Generic "eq" metamethod for our object model,
 * can be overridden by classes
 */
static int eq_generic(struct lua_State *L)
{
    lua_object *obj1 = lua_touserdata(L, 1);
    lua_object *obj2 = lua_touserdata(L, 2);

    if (!obj1 || !obj2)
        luaL_error(L, "eq: Not an object");

    lua_pushboolean(L, memcmp(obj1, obj2, sizeof(lua_object)) == 0);
    return 1;
}

/* luaCFunction for the "index" metamethod of our object model */
static int getObjectMember(lua_State *L)
{
    int nargs   = lua_gettop(L);

    if (nargs == 2)
    {
        lua_object *obj              = lua_touserdata(L, 1);
        const char *key                 = lua_tostring(L, 2);
        lua_class *class    = obj->class;

        if (obj && key)
        {
            lua_object *member;

            lua_rawgeti(L, LUA_REGISTRYINDEX, class->meta); /* Fetch the class */
            lua_pushstring(L, key);
            lua_rawget(L, -2); /* Get the attribute */
            /* stack: object, key, class table, class member */

            if ((member = lua_touserdata(L, -1)))
            {
                if(! obj->class->isValid(L, obj))
                    luaL_error(L, "Invalid %s object", obj->class->name);

                switch (member->class->type)
                {
                    case LUATYPE_ATTRIBUTE:
                      return get_attribute(L, obj, member->data.attribute);

                    case LUATYPE_METHOD:
                      lua_pushcclosure(L, member->data.method->func, 0);
                      return 1;

                    case LUATYPE_CONSTANT:
                      lua_pushnumber(L, member->data.constant->value);
                      return 1;

                    case LUATYPE_FLAG:
                      if (class->getFlag)
                          return class->getFlag(L, obj, member->data.flag.index);
                      luaL_error(L, "Can't get flags of %s", class->name);

                    default:
                      /* Do nothing */
                      break;
                }
            }

            luaL_error(L, "No such class member: %s.%s", obj->class->name, key);
        }
        else
            luaL_error(L, "BUG: Wrong parameter types");
    }
    else
        luaL_error(L, "BUG: Wrong number of parameters");

    return 0;
}

/* luaCFunction for the "newindex" metamethod of our object model */
static int setObjectMember(lua_State *L)
{
    int nargs   = lua_gettop(L);

    if (nargs == 3)
    {
        lua_object *obj  = lua_touserdata(L, 1);
        const char *key     = lua_tostring(L, 2);
        /* stack: object, key, value */

        if (obj && key)
        {
            lua_object *member;

            lua_rawgeti(L, LUA_REGISTRYINDEX, obj->class->meta); /* Fetch the class */
            lua_pushstring(L, key);
            lua_rawget(L, -2); /* Get the member */
            /* stack: object, key, value, class table, class member */

            if ((member = lua_touserdata(L, -1)))
            {
                if(! obj->class->isValid(L, obj))
                    luaL_error(L, "Invalid %s object", obj->class->name);

                switch (member->class->type)
                {
                    case LUATYPE_ATTRIBUTE:
                      if (!(member->data.attribute->flags & FIELDFLAG_READONLY))
                      {
                          lua_pop(L, 2); /* get rid of class table and member*/
                          set_attribute(L, obj, member->data.attribute);
                          return 0;
                      } /* else: fall trough... */
                    case LUATYPE_METHOD:
                    case LUATYPE_CONSTANT:
                      luaL_error(L, "Readonly member %s.%s", obj->class->name, key);

                    case LUATYPE_FLAG:
                      lua_pop(L, 2); /* get rid of class table and member*/
                      if (obj->class->setFlag && !member->data.flag.readonly)
                          return obj->class->setFlag(L, obj, member->data.flag.index);
                      luaL_error(L, "Readonly flag %s.%s", obj->class->name, key);

                    default:
                      /* Do nothing */
                      break;
                }
            }
            luaL_error(L, "No such class member: %s.%s", obj->class->name, key);
        }
        else
            luaL_error(L, "BUG: wrong parameter types");
    }
    else
        luaL_error(L, "BUG: wrong number of parameters");

    return 0;
}

/*
 * Support functions
 */

/* get an attribute from a lua_object and push it onto the stack */
static int get_attribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib)
{
    void   *field_ptr   = (void *) ((char *) obj->data.anything + attrib->offset);
    char   *str;
    void   *field_ptr2;
    tag_t   tag;
    object *tmp;

    switch (attrib->type)
    {
        case FIELDTYPE_SHSTR:
        case FIELDTYPE_CSTR:
          str = *(char * *) field_ptr;
          lua_pushstring(L, str ? str : "");
          return 1;
        case FIELDTYPE_UINT8:
          lua_pushnumber(L, *(uint8 *) field_ptr);
          return 1;
        case FIELDTYPE_SINT8:
          lua_pushnumber(L, *(sint8 *) field_ptr);
          return 1;
        case FIELDTYPE_UINT16:
          lua_pushnumber(L, *(uint16 *) field_ptr);
          return 1;
        case FIELDTYPE_SINT16:
          lua_pushnumber(L, *(sint16 *) field_ptr);
          return 1;
        case FIELDTYPE_UINT32:
          lua_pushnumber(L, *(uint32 *) field_ptr);
          return 1;
        case FIELDTYPE_SINT32:
            lua_pushnumber(L, *(sint32 *) field_ptr);
            return 1;
        case FIELDTYPE_SINT64:
            /* warning: we can have data loss by casting sint64 to double by high sint64!
             * This issue will become urgent when we want compile a real 64bit version
             * of the server.
             */
            lua_pushnumber(L, (lua_Number) (*(sint64 *) field_ptr));
          return 1;
        case FIELDTYPE_FLOAT:
          lua_pushnumber(L, *(float *) field_ptr);
          return 1;
        case FIELDTYPE_MAP:
          /* Can return nil */
          if((*(mapstruct **)field_ptr) == NULL || (*(mapstruct **)field_ptr)->in_memory != MAP_IN_MEMORY )
          {
              lua_pushnil(L);
              return 1;
          }
          return push_object(L, &Map, *(mapstruct * *) field_ptr);
        case FIELDTYPE_OBJECT:
          return push_object(L, &GameObject, *(object * *) field_ptr);
        case FIELDTYPE_OBJECTREF:
          /* returns nil if objectref is invalid */
          field_ptr2 = (void *) ((char *) obj->data.anything + attrib->extra_data);
          tmp = *(object * *) field_ptr;
          tag = *(tag_t *) field_ptr2;
          return push_object(L, &GameObject, OBJECT_VALID_OR_REMOVED(tmp, tag) ? tmp : NULL);

        default:
          luaL_error(L, "BUG: unknown attribute type %d", attrib->type);
          /* lua_error never returns */
          break;
    }

    return 0;
}

/* value is on top of stack */
static int set_attribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib)
{
    void       *field_ptr   = (void *) ((char *) obj->data.anything + attrib->offset);
    const char *str;

    /* Call any class hooks */
    if (obj->class->setAttribute_Hook)
        obj->class->setAttribute_Hook(L, obj, attrib, 1);

    /* First check type */
    switch (attrib->type)
    {
        case FIELDTYPE_SHSTR:
          if (!lua_isstring(L, -1) && !lua_isnil(L, -1))
              luaL_error(L, "Illegal type %s for string field %s.%s", lua_typename(L, lua_type(L, -1)),
                         obj->class->name, attrib->name);

          /* Check against max allowed Daimonin string length (see loader.l) */
          if(lua_strlen(L, -1) > HUGE_BUF - 16)
              luaL_error(L, "String too long: %d chars (max allowed=%d)\n", lua_strlen(L, -1),HUGE_BUF-16);
          break;

        case FIELDTYPE_UINT8:
        case FIELDTYPE_SINT8:
        case FIELDTYPE_UINT16:
        case FIELDTYPE_SINT16:
        case FIELDTYPE_UINT32:
        case FIELDTYPE_SINT32:
        case FIELDTYPE_SINT64:
        case FIELDTYPE_FLOAT:
          if (!lua_isnumber(L, -1))
              luaL_error(L, "Illegal type %s for numeric field %s.%s", lua_typename(L, lua_type(L, -1)),
                         obj->class->name, attrib->name);
          break;

        default:
          luaL_error(L, "Unhandled attribute type %d for %s.%s", attrib->type, obj->class->name, attrib->name);
    }

    /* Do the actual setting */
    switch (attrib->type)
    {
        case FIELDTYPE_SHSTR:
          str = lua_tostring(L, -1);
          if (*(char * *) field_ptr != NULL)
              FREE_AND_CLEAR_HASH(*(char * *) field_ptr);
          if (str && strcmp(str, ""))
              FREE_AND_COPY_HASH(*(const char * *) field_ptr, str);
          break;

        case FIELDTYPE_UINT8:
          *(uint8 *) field_ptr = (uint8) lua_tonumber(L, -1); break;
        case FIELDTYPE_SINT8:
          *(sint8 *) field_ptr = (sint8) lua_tonumber(L, -1); break;
        case FIELDTYPE_UINT16:
          *(uint16 *) field_ptr = (uint16) lua_tonumber(L, -1); break;
        case FIELDTYPE_SINT16:
          *(sint16 *) field_ptr = (sint16) lua_tonumber(L, -1); break;
        case FIELDTYPE_UINT32:
          *(uint32 *) field_ptr = (uint32) lua_tonumber(L, -1); break;
        case FIELDTYPE_SINT32:
          *(sint32 *) field_ptr = (sint32) lua_tonumber(L, -1); break;
        case FIELDTYPE_SINT64:
          *(sint64 *) field_ptr = (sint64) lua_tonumber(L, -1); break;
        case FIELDTYPE_FLOAT:
          *(float *) field_ptr = (float) lua_tonumber(L, -1); break;

        default:
          luaL_error(L, "Unhandled attribute type %d for %s.%s", attrib->type, obj->class->name, attrib->name);
    }

    /* Call any class hooks */
    if (obj->class->setAttribute_Hook)
        obj->class->setAttribute_Hook(L, obj, attrib, 0);

    /* pop value */
    lua_pop(L, 1);

    return 0;
}

/* Extended variant of luaL_typerror that is aware of our object model */
static inline void param_type_err(lua_State *L, int pos, const char *expected)
{
    static char buf[64];

    lua_object *obj = lua_touserdata(L, pos);
    sprintf(buf, "%s expected, got %s", expected, obj ? obj->class->name : lua_typename(L, lua_type(L, pos)));

    luaL_argerror(L, pos, buf);
}

/* Validate and get an object argument of the specified class */
static inline lua_object * get_object_arg(lua_State *L, int pos, lua_class *class)
{
    lua_object *obj;

    if ((obj = lua_touserdata(L, pos)))
    {
        if(! obj->class->isValid(L, obj))
            luaL_error(L, "Invalid %s object", obj->class->name);

        return obj;
    }

    param_type_err(L, pos, class->name);
    return NULL;
}

/* Parse function/method arguments (similar to PyArg_ParseTuple())
 * fmt codes:
 *  b - boolean
 *  s - string
 *  i - integer (int)
 *  I - integer (int64)
 *  f - float
 *  d - double
 *  O - GameObject
 *  M - Map
 *  G - the "game" singleton object
 *  E - event object
 *  A - AI object
 *  | - the following arguments are optional
 *  ? - the next argument may be nil
 *
 * Hm, if we have here a i type integer and we have a uint32,
 * can we run in trouble by miscasting it with (int)?
 * we should observe that issue.
 */
void get_lua_args(lua_State *L, const char *fmt, ...)
{
    va_list     ap;
    const char *p;
    int         optional = 0, nextnil = 0;
    int         pos     = 1;
    int         nargs   = lua_gettop(L);

    va_start(ap, fmt);
    for (p = fmt; *p; p++)
    {
        /* Metastuff */
        if (*p == '|')
        {
            optional = 1;
            continue;
        }
        else if (*p == '?')
        {
            nextnil = 1;
            continue;
        }

        /* Did we run out of supplied arguments? */
        if (pos > nargs)
        {
            if (optional)
                return;
            else
            {
                lua_Debug   ar;
                lua_getstack(L, 0, &ar);
                lua_getinfo(L, "n", &ar);
                if (strcmp(ar.namewhat, "method") == 0)
                    nargs--;  /* do not count `self' */
                luaL_error(L, "Too few arguments to %s (%d)", ar.name ? ar.name : "?", nargs);
            }
        }

        if (nextnil && lua_isnil(L, pos))
        {
            *va_arg(ap, void * *) = NULL;
            pos++;
            continue;
        }

        switch (*p)
        {
            case 'O':
              /* GameObject */
              *va_arg(ap, lua_object * *) = get_object_arg(L, pos, &GameObject);
              break;

            case 'M':
              /* Map */
              *va_arg(ap, lua_object * *) = get_object_arg(L, pos, &Map);
              break;

            case 'G':
              /* Game */
              *va_arg(ap, lua_object * *) = get_object_arg(L, pos, &Game);
              break;

            case 'E':
              /* Event */
              *va_arg(ap, lua_object * *) = get_object_arg(L, pos, &Event);
              break;

            case 'A':
              /* AI */
              *va_arg(ap, lua_object * *) = get_object_arg(L, pos, &AI);
              break;

            case 'i':
              /* integer (int) */
              luaL_checknumber(L, pos);
              *va_arg(ap, int *) = (int) lua_tonumber(L, pos);
              break;

            case 'I':
                /* integer (int64) */
                luaL_checknumber(L, pos);
                *va_arg(ap, sint64 *) = (sint64) lua_tonumber(L, pos);
                break;

            case 'f':
              /* float */
              luaL_checknumber(L, pos);
              *va_arg(ap, float *) = (float) lua_tonumber(L, pos);
              break;

            case 'd':
              /* double */
              luaL_checknumber(L, pos);
              *va_arg(ap, double *) = (double) lua_tonumber(L, pos);
              break;

            case 's':
              /* string */
              luaL_checklstring(L, pos, NULL);
              *va_arg(ap, const char * *) = lua_tostring(L, pos);
              break;

            case 'b':
              /* boolean */
              luaL_checknumber(L, pos);
              *va_arg(ap, int *) = lua_toboolean(L, pos);
              break;

            default:
              luaL_error(L, "BUG: unknown type code '%c'", *p);
              break;
        }
        pos++;
        nextnil = 0;
    }

    /* Did we get too many supplied arguments? */
    if (nargs >= pos)
    {
        lua_Debug   ar;
        lua_getstack(L, 0, &ar);
        lua_getinfo(L, "n", &ar);
        if (strcmp(ar.namewhat, "method") == 0)
        {
            nargs--;  /* do not count `self' */
            pos--;  /* do not count `self' */
        }
        luaL_error(L, "Too many arguments to %s (%d expected, got %d)", ar.name ? ar.name : "?", pos - 1, nargs);
    }
    va_end(ap);
}

/* creates a new lua_object to wrap the data in */
int push_object(lua_State *L, lua_class *class, void *data)
{
    lua_object *obj;

    if (data == NULL)
    {
        lua_pushnil(L);
        return 1;
    }

    obj = lua_newuserdata(L, sizeof(lua_object));
    obj->class = class;
    obj->data.anything = data;

    /* Setup tag to make invalidation of weak references possible */
    switch(class->type) {
        case LUATYPE_MAP:
            obj->tag = obj->data.map->tag;
            break;
        case LUATYPE_OBJECT:
        case LUATYPE_AI:
            obj->tag = obj->data.object->count;
            break;
        case LUATYPE_EVENT:
            obj->tag = obj->data.context->tag;
            break;
        default:
            obj->tag = 0;
            break;
    }

    /* Fetch and attach metatable */
    lua_rawgeti(L, LUA_REGISTRYINDEX, class->meta);
    lua_setmetatable(L, -2);

    class->obcount++;
    //    LOG(llevDebug, "pushed a %s (count=%d)\n", obj->class->name, obj->class->obcount);

    return 1;
}

/* Add a member from our object model to a class */
/* the class table is to be on stack top */
static void add_class_member(struct lua_State *L, const char *key, lua_class *class,  lua_object *object)
{
    lua_object *obj;

    lua_pushstring(L, key);
    obj = lua_newuserdata(L, sizeof(lua_object));
    obj->class = class;
    obj->data = object->data;

    lua_settable(L, -3);
}

/* Gc metamethod for debugging memory leaks */
int gc(struct lua_State *L)
{
    lua_object *obj = lua_touserdata(L, 1);
    if (obj)
    {
        obj->class->obcount--;
        LOG(llevDebug, "gc on %s (count=%d)\n", obj->class->name, obj->class->obcount);
    }
    else
    {
        luaL_error(L, "gc on wierd obj");
    }
    return 0;
}

/* Default object validator, always returns true */
static int default_object_validator(struct lua_State *L, lua_object *obj)
{
    return TRUE;
}

/* Set up new class data in the registry */
int init_class(struct lua_State *L, lua_class *class)
{
    int i;
    lua_object tmp;

    /* TODO:
     * - add optional set/get function pointers to the attribute definitions
     * - allow lua scripts to replace/add methods and add attributes
     *   (requires change in the object model)
     */

    /* Set up class metatable */
    lua_newtable(L);

    lua_pushstring(L, "__index");
    lua_pushcclosure(L, getObjectMember, 0);
    lua_rawset(L, -3);     /* stack: metatable */

    lua_pushstring(L, "__newindex");
    lua_pushcclosure(L, setObjectMember, 0);
    lua_rawset(L, -3);     /* stack: metatable */

    lua_pushstring(L, "__eq");
    lua_pushcclosure(L, eq_generic, 0);
    lua_rawset(L, -3);     /* stack: metatable */

    lua_pushstring(L, "__tostring");
    if (class->toString)
        lua_pushcclosure(L, class->toString, 0);
    else
        lua_pushcclosure(L, toString_generic, 0);
    lua_rawset(L, -3);     /* stack: metatable */

    // TODO: a concat metamethod

#if 0
    lua_pushstring(L, "__gc");
    lua_pushcclosure(L, gc, 0);
    lua_rawset(L, -3);
#endif

    /* Set up class members */
    if (class->attributes)
        for (i = 0; class->attributes[i].name != NULL; i++)
        {
            tmp.data.attribute = &class->attributes[i];
            add_class_member(L, class->attributes[i].name, &Attribute, &tmp);
        }

    if (class->methods)
        for (i = 0; class->methods[i].name != NULL; i++)
        {
            tmp.data.method = &class->methods[i];
            add_class_member(L, class->methods[i].name, &Method, &tmp);
        }

    if (class->constants)
        for (i = 0; class->constants[i].name != NULL; i++)
        {
            tmp.data.constant = &class->constants[i];
            add_class_member(L, class->constants[i].name, &Constant, &tmp);
        }

    if (class->flags)
    {
        for (i = 0; class->flags[i] == NULL || strcmp(class->flags[i], FLAGLIST_END_MARKER) != 0; i++)
        {
            const char *flagname = class->flags[i];
            if (flagname != NULL) {
                tmp.data.flag.index = i;
                if (flagname[0] == '?') {
                    tmp.data.flag.readonly = 1;
                    flagname++;
                } else
                    tmp.data.flag.readonly = 0;
                add_class_member(L, flagname, &Flag, &tmp);
            }
        }
    }

    class->meta = luaL_ref(L, LUA_REGISTRYINDEX); /* store class metatable in registry */

    if(class->isValid == NULL)
        class->isValid = default_object_validator;

    return 0;
}

/*
 * Bytecode cache
 */


int init_file_cache(struct lua_State *L)
{
    /* Set up cache table */
    lua_newtable(L);
    cache_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

/* Leaves the function or an error on the stack top
 * TODO: purge old entries when cache starts getting big
 */
int load_file_cache(struct lua_State *L, const char *file)
{
    struct stat stat_buf;
    int         load = 0, res = 0;
    char *suffix;

    /* Make sure we are allowed to load file */
    suffix = strrchr(file, '.');
    if(suffix == NULL || (strcmp(suffix, ".lua") && strcmp(suffix, ".lc")))
    {
        lua_pushfstring(L, "Not a legal script file `%s'", file);
        return LUA_ERRFILE;
    }

    /* Get file modification time */
    if (stat(file, &stat_buf))
    {
        lua_pushfstring(L, "couldn't find script file `%s'", file);
        return LUA_ERRFILE;
    }

    /* First, check the cache table for this script */
    lua_rawgeti(L, LUA_REGISTRYINDEX, cache_ref);
    lua_pushstring(L, file);
    lua_pushvalue(L, -1);
    lua_rawget(L, -3);

    /* stack: cache table, path, function/nil */

    if (lua_isfunction(L, -1))
    {
        time_t  load_time;

        /* Get the file load time */
        lua_pushvalue(L, -1); /* it is stored with the function as key */
        lua_rawget(L, -4);
        /* stack: cache, path, function, time */

        load_time = (time_t) lua_tonumber(L, -1);
        lua_pop(L, 1); /* get rid of time */

        /* File changed since we loaded it? */
        if (load_time < stat_buf.st_mtime)
        {
#ifdef LUA_DEBUG_ALL
            LOG(llevDebug, "LUA - Cached version old, loading '%s' from file\n", file);
#endif
            lua_pop(L, 1); /* get rid of old chunk */
            load = 1;
        }
        else
        {
            load = 0;
        }
    }
    else
    {
#ifdef LUA_DEBUG_ALL
        LOG(llevDebug, "LUA - Script not in cache, loading '%s' from file\n", file);
#endif
        lua_pop(L, 1); /* throw away nil */
        load = 1;
    }

    if (load)
    {
        /* stack: cache, path */
        res = luaL_loadfile(L, file);
        if (res == 0)
        {
            /* stack: cache, path, f */

            /* store the load time */
            lua_pushvalue(L, -1);
            lua_pushnumber(L, (lua_Number) time(NULL));
            lua_rawset(L, -5);

            /* Store the function */
            lua_pushvalue(L, -2); /* path */
            lua_pushvalue(L, -2); /* function */
            lua_rawset(L, -5);
        }
    }
/* stack: cache, path, function/error */

    /* cleanup */
    lua_remove(L, -2); /* path */
    lua_remove(L, -2); /* cache */

    return res;
}
