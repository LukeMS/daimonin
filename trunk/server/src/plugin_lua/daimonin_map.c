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
#include <daimonin_map.h>

/* Global data objects */

static struct method_decl       Map_methods[]       =
{
    {"Save", Map_Save}, {"Delete", Map_Delete}, 
    {"GetFirstObjectOnSquare", Map_GetFirstObjectOnSquare},
    {"IsWallOnSquare", Map_IsWallOnSquare},
    {"PlaySound", Map_PlaySound}, {"Message", Map_Message}, {"MapTileAt",  Map_MapTileAt},
    {"CreateObject", Map_CreateObject}, {NULL, NULL}
};

static struct attribute_decl    Map_attributes[]    =
{
    { "name",    FIELDTYPE_CSTR, offsetof(mapstruct, name), FIELDFLAG_READONLY },
    { "message", FIELDTYPE_CSTR, offsetof(mapstruct, msg), FIELDFLAG_READONLY },
    { "reset_interval", FIELDTYPE_UINT32, offsetof(mapstruct, reset_timeout), FIELDFLAG_READONLY },
    { "difficulty", FIELDTYPE_UINT16, offsetof(mapstruct, difficulty), FIELDFLAG_READONLY },
    { "height", FIELDTYPE_UINT16, offsetof(mapstruct, height), FIELDFLAG_READONLY },
    { "width", FIELDTYPE_UINT16, offsetof(mapstruct, width), FIELDFLAG_READONLY },
    { "darkness", FIELDTYPE_UINT8, offsetof(mapstruct, darkness), FIELDFLAG_READONLY },
    { "path", FIELDTYPE_SHSTR, offsetof(mapstruct, path), FIELDFLAG_READONLY }
};

static const char              *Map_flags[]         =
{
    "f_outdoor", "f_unique", "f_fixed_rtime", "f_nomagic", "f_nopriest", "f_noharm", "f_nosummon", "f_fixed_login",
    "f_permdeath", "f_ultradeath", "f_ultimatedeath", "f_pvp", FLAGLIST_END_MARKER
};

/****************************************************************************/
/*                          Map methods                                     */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : Map_GetFirstObjectOnSquare                                       */
/* Lua    : map.GetFirstObjectOnSquare(x, y)                                 */
/* Info   : Gets the bottom object on the tile. Use obj.above to browse objs */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Map_GetFirstObjectOnSquare(lua_State *L)
{
    int         x, y;
    object     *val;
    CFParm     *CFR;
    lua_object *map;

    get_lua_args(L, "Mii", &map, &x, &y);

    GCFP.Value[0] = map->data.map;
    GCFP.Value[1] = (void *) (&x);
    GCFP.Value[2] = (void *) (&y);
    CFR = (PlugHooks[HOOK_GETMAPOBJECT]) (&GCFP);
    val = (object *) (CFR->Value[0]);

    return push_object(L, &GameObject, val);
}

/*****************************************************************************/
/* Name   : Map_IsWallOnSquare                                               */
/* Lua    : map.IsWallOnSquare(x, y)                                         */
/* Info   : returns true if the square at x,y is a wall                      */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Map_IsWallOnSquare(lua_State *L)
{
    int         x, y;
    lua_object *map;

    get_lua_args(L, "Mii", &map, &x, &y);

    lua_pushboolean(L, hooks->wall(map->data.map, x, y));
    return 1;
}

/*****************************************************************************/
/* Name   : Map_MapTileAt                                                    */
/* Lua    : map.MapTileAt(x, y)                                              */
/* Status : untested                                                         */
/* TODO   : do someting about the new modified coordinates too?              */
/*****************************************************************************/
static int Map_MapTileAt(lua_State *L)
{
    int         x, y;
    CFParm     *CFR;
    mapstruct  *result;
    lua_object *map;

    get_lua_args(L, "Mii", &map, &x, &y);

    GCFP.Value[0] = map->data.map;
    GCFP.Value[1] = (void *) (&x);
    GCFP.Value[2] = (void *) (&y);
    CFR = (PlugHooks[HOOK_OUTOFMAP]) (&GCFP);
    result = (mapstruct *) (CFR->Value[0]);

    return push_object(L, &Map, result);
}

/*****************************************************************************/
/* Name   : Map_Save                                                         */
/* Lua    : map.Save(flag)                                                   */
/* Status : Stable                                                           */
/* Info   : Save the map. If flag is 1, unload map from memory               */
/*****************************************************************************/
static int Map_Save(lua_State *L)
{
    int         x;
    lua_object *map;

    get_lua_args(L, "M|i", &map, &x);

    GCFP.Value[0] = map->data.map;
    GCFP.Value[1] = (void *) (&x);
    (PlugHooks[HOOK_MAPSAVE]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_Delete                                                       */
/* Lua    : map.Delete(flags)                                                */
/* Status : Stable                                                           */
/* Info   : Remove the map from memory and map list. Release all objects.    */
/*****************************************************************************/
static int Map_Delete(lua_State *L)
{
    int         x;
    lua_object *map;

    get_lua_args(L, "M|i", &map, &x);

    GCFP.Value[0] = map->data.map;
    GCFP.Value[1] = (void *) (&x);
    (PlugHooks[HOOK_MAPDELETE]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_PlaySound                                                    */
/* Lua    : map.PlaySound(x, y, soundnumber, soundtype)                      */
/* Status : Tested                                                           */
/* TODO   : supply constants for the sounds                                  */
/*****************************************************************************/
static int Map_PlaySound(lua_State *L)
{
    int         x, y, soundnumber, soundtype;
    lua_object *map;

    get_lua_args(L, "Miiii", &map, &x, &y, &soundnumber, &soundtype);

    GCFP.Value[0] = (void *) (map->data.map);
    GCFP.Value[1] = (void *) (&x);
    GCFP.Value[2] = (void *) (&y);
    GCFP.Value[3] = (void *) (&soundnumber);
    GCFP.Value[4] = (void *) (&soundtype);
    (PlugHooks[HOOK_PLAYSOUNDMAP]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_Message                                                      */
/* Lua    : map.Message(x, y, distance, messagem,color)                      */
/* Info   : Writes a message to all players on a map                         */
/*          Starting point x,y for all players in distance                   */
/*          default color is game.COLOR_BLUE | game.COLOR_UNIQUE             */
/* Status : Tested                                                           */
/*****************************************************************************/

static int Map_Message(lua_State *L)
{
    int   color = NDI_BLUE |NDI_UNIQUE, x, y, d;
    char                   *message;
    lua_object             *map;

    get_lua_args(L, "Miiis|i", &map, &x, &y, &d, &message, &color);

    GCFP.Value[0] = (void *) (&color);
    GCFP.Value[1] = (void *) (map->data.map);
    GCFP.Value[2] = (void *) (&x);
    GCFP.Value[3] = (void *) (&y);
    GCFP.Value[4] = (void *) (&d);
    GCFP.Value[5] = (void *) (message);

    (PlugHooks[HOOK_NEWINFOMAP]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_CreateObject                                                 */
/* Lua    : map.CreateObject(arch_name, x, y)                                */
/* Info   :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static int Map_CreateObject(lua_State *L)
{
    char       *txt;
    int         x, y;
    CFParm     *CFR;
    lua_object *map;

    get_lua_args(L, "Msii", &map, &txt, &x, &y);

    GCFP.Value[0] = (void *) (txt);
    GCFP.Value[1] = (void *) (map->data.map);
    GCFP.Value[2] = (void *) (&x);
    GCFP.Value[3] = (void *) (&y);
    CFR = (PlugHooks[HOOK_CREATEOBJECT]) (&GCFP);

    return push_object(L, &GameObject, (object *) (CFR->Value[0]));
}


/* FUNCTIONEND -- End of the Lua plugin functions. */

/****************************************************************************/
/* Map object management                                                    */
/****************************************************************************/

/* pushes flag value on top of stack */
static int Map_getFlag(lua_State *L, lua_object *obj, uint32 flagno)
{
    lua_pushboolean(L, (obj->data.map->map_flags & (1 << flagno)));
    return 1;
}

/* Return a string representation of this object (useful for debugging) */
static int Map_toString(lua_State *L)
{
    lua_object *obj = lua_touserdata(L, 1);
    char        buf[HUGE_BUF], *ptr;

    if (obj == NULL || obj->class->type != LUATYPE_MAP)
        luaL_error(L, "Not a Map object");

    strncpy(buf, obj->data.map->name, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    /* We can't send this special char to the client message thingie,
     * it will mess the text up */
    if ((ptr = strchr(buf, '§')))
        *ptr = '$';

    lua_pushfstring(L, "[%s \"%s\"]", obj->data.map->path, buf);
    return 1;
}

/* Declare the map class */
lua_class   Map =
{
    LUATYPE_MAP, "Map", 0, Map_toString, Map_attributes, Map_methods, NULL, Map_flags, Map_getFlag, NULL
};

/* Initialize the map class */
int Map_init(lua_State *s)
{
    init_class(s, &Map);

    return 0;
}
