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
	{"SetUniqueMap", Map_SetUniqueMap},
    {"Save", Map_Save}, {"Delete", Map_Delete},
    {"GetFirstObjectOnSquare", Map_GetFirstObjectOnSquare},
    {"GetBrightnessOnSquare", Map_GetBrightnessOnSquare},
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
    { "darkness", FIELDTYPE_SINT32, offsetof(mapstruct, darkness), FIELDFLAG_READONLY },
    { "light_value", FIELDTYPE_SINT32, offsetof(mapstruct, light_value), FIELDFLAG_READONLY },
    { "path", FIELDTYPE_SHSTR, offsetof(mapstruct, path), FIELDFLAG_READONLY },
    {NULL}
};

static const char              *Map_flags[]         =
{
    "?f_outdoor", "?f_unique", "?f_fixed_rtime", "f_nomagic", "f_nopriest", "f_noharm", "f_nosummon", "?f_fixed_login",
    "?f_permdeath", "?f_ultradeath", "?f_ultimatedeath", "?f_pvp", FLAGLIST_END_MARKER
};

/****************************************************************************/
/*                          Map methods                                     */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : Map_SetUniqueMap                                                 */
/* Lua    : map:SetUniqueMap(0, object)                                      */
/* Status : Stable                                                           */
/* Info   : Make a map unique - This is used to transfer apartments between  */
/*        : players or change normal loaded maps                             */ 
/*****************************************************************************/
static int Map_SetUniqueMap(lua_State *L)
{
	const char *tmp_string;
	int         flags;
	lua_object *obptr   = NULL;
	lua_object *map;

	get_lua_args(L, "MiO", &map, &flags, &obptr);

	if (!map->data.map || map->data.map->in_memory != MAP_IN_MEMORY)
		return 0;

	/* we ignore flags ATM - 0 is default and means player apartment */
	if(!obptr || !obptr->data.object)
		return 0;

	tmp_string = hooks->create_unique_path(map->data.map->path, obptr->data.object);
	FREE_AND_COPY_HASH(map->data.map->path, tmp_string);
	map->data.map->map_flags |= MAP_FLAG_UNIQUE;

	return 0;
}

/*****************************************************************************/
/* Name   : Map_GetFirstObjectOnSquare                                       */
/* Lua    : map:GetFirstObjectOnSquare(x, y)                                 */
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
/* Name   : Map_GetBrightnessOnSquare                                        */
/* Lua    : map:GetBrightnessOnSquare(x, y, mode)                            */
/* Info   : Returns the brightness level on the specified square. If         */
/*          mode == 0 (default) the brightness is returned as a value between*/
/*          0 and 7 as in map.darkness. If mode is 1, the value returned is  */
/*          in the internal higher-resolution scale, usually somewhere       */
/*          between 0-1280 (compare with map.light_level.) Since this scale  */
/*          is uncapped, it is possible to sense a light source in full      */
/*          daylight (map.darkness=7) with mode==1                           */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Map_GetBrightnessOnSquare(lua_State *L)
{
    int         x, y, mode = 0;
    lua_object *map;
    int brightness, i;

    get_lua_args(L, "Mii|i", &map, &x, &y, &mode);

    brightness = hooks->map_brightness(map->data.map, x, y);

    if(mode == 0)
    {
        /* Convert brightness to 0-MAX_DARKNESS scale */
        for(i=0; i<MAX_DARKNESS; i++)
        {
            if(brightness < hooks->global_darkness_table[i]) {
                brightness = MAX(0,i-1);
                break;
            }
        }
        if(i==MAX_DARKNESS) /* Didn't find it? */
            brightness = i;
    }

    lua_pushnumber(L, brightness);

    return 1;
}

/*****************************************************************************/
/* Name   : Map_IsWallOnSquare                                               */
/* Lua    : map:IsWallOnSquare(x, y)                                         */
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
/* Lua    : map:MapTileAt(x, y)                                              */
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
/* Lua    : map:Save(flag)                                                   */
/* Status : Stable                                                           */
/* Info   : Save the map. If flag is 1, unload map from memory               */
/*****************************************************************************/
static int Map_Save(lua_State *L)
{
    int         flags = 0;
    lua_object *map;

    get_lua_args(L, "M|i", &map, &flags);

	if (!map->data.map || map->data.map->in_memory != MAP_IN_MEMORY)
		return 0;

	if (hooks->new_save_map(map->data.map, 0) == -1)
		LOG(llevDebug, "MapSave(): failed to save map %s\n", STRING_SAFE(map->data.map->path));

	if (flags)
		map->data.map->in_memory = MAP_IN_MEMORY;

    return 0;
}

/*****************************************************************************/
/* Name   : Map_Delete                                                       */
/* Lua    : map:Delete(flags)                                                */
/* Status : Stable                                                           */
/* Info   : Remove the map from memory and map list. Release all objects.    */
/*****************************************************************************/
static int Map_Delete(lua_State *L)
{
    int         flags = 0;
    lua_object *map;

    get_lua_args(L, "M|i", &map, &flags);

	if (!map->data.map || !map->data.map->in_memory)
		return 0;

	if (flags) /* really delete the map */
	{
		hooks->free_map(map->data.map, 1);
		hooks->delete_map(map->data.map);
	}
	else /* just swap it out */
		hooks->free_map(map->data.map, 1);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_PlaySound                                                    */
/* Lua    : map:PlaySound(x, y, soundnumber, soundtype)                      */
/* Info   : play the sound on the map, sounding like it comes from the given */
/*          x,y coordinates.                                                 */
/*          If soundtype is game.SOUNDTYPE_NORMAL (the default), then        */
/*          soundnumber should be one of the game.SOUND_xxx constants        */
/*          If soundtype is game.SOUNDTYPE_SPELL, then the sound number      */
/*          should be a spell number, to play the sound of that spell        */
/* Status : Tested                                                           */
/*****************************************************************************/
static int Map_PlaySound(lua_State *L)
{
    int         x, y, soundnumber, soundtype = SOUND_NORMAL;
    lua_object *map;

    get_lua_args(L, "Miii|i", &map, &x, &y, &soundnumber, &soundtype);

    hooks->play_sound_map(map->data.map, x, y, soundnumber, soundtype);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_Message                                                      */
/* Lua    : map:Message(x, y, distance, messagem,color)                      */
/* Info   : Writes a message to all players on a map                         */
/*          Starting point x,y for all players in distance                   */
/*          color should be one of the game.COLOR_xxx constants.             */
/*          default color is game.COLOR_BLUE | game.COLOR_UNIQUE             */
/* Status : Tested                                                           */
/*****************************************************************************/

static int Map_Message(lua_State *L)
{
    int             color = NDI_BLUE |NDI_UNIQUE, x, y, d;
    char        *message;
    lua_object    *map;

    get_lua_args(L, "Miiis|i", &map, &x, &y, &d, &message, &color);

    hooks->new_info_map(color, map->data.map, x, y, d, message);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_CreateObject                                                 */
/* Lua    : map:CreateObject(arch_name, x, y)                                */
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

/* pushes flag value on top of stack */
static int Map_setFlag(lua_State *L, lua_object *obj, uint32 flagno)
{
    int     value;

    if (lua_isnumber(L, -1))
        value = (int) lua_tonumber(L, -1);
    else
        value = lua_toboolean(L, -1);

    if(value)
        obj->data.map->map_flags |= (1 << flagno);
    else
        obj->data.map->map_flags &= ~(1 << flagno);

    return 0;
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

/* Tests if a Map object is valid */
static int Map_isValid(lua_State *L, lua_object *obj)
{
    /* TODO: also check map tag */
    return obj->data.map->in_memory == MAP_IN_MEMORY;
}

/* Declare the map class */
lua_class   Map =
{
    LUATYPE_MAP, "Map", 0, Map_toString, Map_attributes, Map_methods,
    NULL, Map_flags, Map_getFlag, Map_setFlag, NULL, Map_isValid
};

/* Initialize the map class */
int Map_init(lua_State *s)
{
    init_class(s, &Map);

    return 0;
}
