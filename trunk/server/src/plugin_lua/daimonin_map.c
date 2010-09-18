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
#include <global.h>
#include <daimonin_map.h>

static struct method_decl Map_methods[] =
{
    {"ActivateConnection",     Map_ActivateConnection},
    {"CreateObject",           Map_CreateObject},
    {"Delete",                 Map_Delete},
    {"GetFirstObjectOnSquare", Map_GetFirstObjectOnSquare},
    {"GetBrightnessOnSquare",  Map_GetBrightnessOnSquare},
    {"HasConnection",          Map_HasConnection},
    {"IsAnyPlayerOnMap",       Map_IsAnyPlayerOnMap},
    {"IsWallOnSquare",         Map_IsWallOnSquare},
    {"MapTileAt",              Map_MapTileAt},
    {"Message",                Map_Message},
    {"PlayersOnMap",           Map_PlayersOnMap},
    {"PlaySound",              Map_PlaySound},
    {"ReadyInheritedMap",      Map_ReadyInheritedMap},
    {"Save",                   Map_Save},
    {"SetDarkness",            Map_SetDarkness},

    {NULL, NULL}
};

static struct attribute_decl Map_attributes[] =
{
    {"name",           FIELDTYPE_CSTR,   offsetof(mapstruct, name),          FIELDFLAG_READONLY, 0},
    {"message",        FIELDTYPE_CSTR,   offsetof(mapstruct, msg),           FIELDFLAG_READONLY, 0},
    {"reset_interval", FIELDTYPE_UINT32, offsetof(mapstruct, reset_timeout), FIELDFLAG_READONLY, 0},
    {"difficulty",     FIELDTYPE_UINT16, offsetof(mapstruct, difficulty),    FIELDFLAG_READONLY, 0},
    {"height",         FIELDTYPE_UINT16, offsetof(mapstruct, height),        FIELDFLAG_READONLY, 0},
    {"width",          FIELDTYPE_UINT16, offsetof(mapstruct, width),         FIELDFLAG_READONLY, 0},
    {"enter_x",        FIELDTYPE_UINT16, offsetof(mapstruct, enter_x),       FIELDFLAG_READONLY, 0},
    {"enter_y",        FIELDTYPE_UINT16, offsetof(mapstruct, enter_y),       FIELDFLAG_READONLY, 0},
    {"darkness",       FIELDTYPE_SINT32, offsetof(mapstruct, darkness),      FIELDFLAG_READONLY, 0},
    {"light_value",    FIELDTYPE_SINT32, offsetof(mapstruct, light_value),   FIELDFLAG_READONLY, 0},
    {"path",           FIELDTYPE_SHSTR,  offsetof(mapstruct, path),          FIELDFLAG_READONLY, 0},
    {"orig_path",      FIELDTYPE_SHSTR,  offsetof(mapstruct, orig_path),     FIELDFLAG_READONLY, 0},
    {"map_status",     FIELDTYPE_UINT32, offsetof(mapstruct, map_status),    FIELDFLAG_READONLY, 0},

    {NULL, 0, 0, 0, 0}
};

static const char *Map_flags[] =
{
    "?f_outdoor",
    "?f_no_save",
    "?f_fixed_rtime",
    "f_nomagic",
    "f_nopriest",
    "f_noharm",
    "f_nosummon",
    "?f_fixed_login",
    "?f_permdeath",
    "?f_ultradeath",
    "?f_ultimatedeath",
    "?f_pvp",

    FLAGLIST_END_MARKER
};

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

/* Compare two objects for equality */
static int Map_eq(struct lua_State *L)
{
    lua_object *lhs = lua_touserdata(L, 1);
    lua_object *rhs = lua_touserdata(L, 2);

    /* Should actually never happen. */
    if ((!lhs || lhs->class->type != LUATYPE_MAP) ||
        (!rhs || rhs->class->type != LUATYPE_MAP))
    {
        LOG(llevBug, "BUG:: %s/Map_eq(): Either/both LHS/RHS not Map objects!\n",
            __FILE__);

        return luaL_error(L, "Map_eq: Either/both LHS/RHS not Map objects!");
    }

    /* Test for LHS invalidity. */
    if (!lhs->class->isValid(lhs))
        return luaL_error(L, "Map_eq: LHS invalid!");

    /* Test for RHS invalidity. */
    if (!rhs->class->isValid(rhs))
        return luaL_error(L, "Map_eq: RHS invalid!");

    /* Compare tags. */
    lua_pushboolean(L, (lhs->tag == rhs->tag));

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
    if ((ptr = strchr(buf, '�')))
        *ptr = '$';

    lua_pushfstring(L, "[%s \"%s\"]", obj->data.map->path, buf);
    return 1;
}

/* Tests if a Map object is valid */
static int Map_isValid(lua_object *obj)
{
    return (obj->tag == obj->data.map->tag && obj->data.map->in_memory == MAP_IN_MEMORY);
}

/* Declare the map class */
lua_class Map =
{
    LUATYPE_MAP,
    "Map",
    0,
    Map_eq,
    Map_toString,
    Map_attributes,
    Map_methods,
    NULL,
    Map_flags,
    Map_getFlag,
    Map_setFlag,
    NULL,
    Map_isValid,
    0
};

/* Initialize the map class */
int Map_init(lua_State *s)
{
    init_class(s, &Map);

    return 0;
}


/****************************************************************************/
/*                          Map methods                                     */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */
/*****************************************************************************/
/* Name   : Map_ReadyInheritedMap                                            */
/* Lua    : map:ReadyInheritedMap(map_path, flags)                           */
/* Info   : Loads the map from map_path into memory, unless already loaded.  */
/*          Will load the new map as the same type (multi,unique or instance)*/
/*          as the old map.                                                  */
/*          See also object:ReadyUniqueMap(), object:StartNewInstance() and  */
/*          game:ReadyMap()                                                  */
/*          flags:                                                           */
/*            game.MAP_CHECK - don't load the map if it isn't in memory,     */
/*                             returns nil if the map wasn't in memory.      */
/*            game.MAP_NEW - delete the map from memory and force a reset    */
/*                           (if it existed in memory or swap)               */
/* Return : map pointer to map, or nil                                       */
/* Status : Unfinished (flags not handled yet)                               */
/*****************************************************************************/
static int Map_ReadyInheritedMap(lua_State *L)
{
    lua_object *self;
    char       *mapname;
    const char *orig_path_sh,
               *path_sh = NULL;
    int         flags = 0;
    mapstruct  *new_map = NULL;

    get_lua_args(L, "Ms|i", &self, &mapname, &flags);

    /* TODO: handle flags like game:ReadyMap() */

    /* we need a valid map status to know how to handle the map file */
    if(MAP_STATUS_TYPE(WHERE->map_status))
    {
        orig_path_sh = hooks->create_safe_mapname_sh(mapname);

        /* create the path prefix (./players/.. or ./instance/.. ) for non multi maps */
        if(WHERE->map_status & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE))
        {
            char tmp_path[MAXPATHLEN];

            path_sh = hooks->add_string( hooks->normalize_path_direct(WHERE->path,
                                         hooks->path_to_name(orig_path_sh), tmp_path));
        }

        new_map = hooks->ready_map_name(path_sh?path_sh:orig_path_sh, orig_path_sh, MAP_STATUS_TYPE(WHERE->map_status), WHERE->reference);

        FREE_ONLY_HASH(orig_path_sh);
        if(path_sh)
            FREE_ONLY_HASH(path_sh);
    }

    return push_object(L, &Map, new_map);
}

/*****************************************************************************/
/* Name   : Map_Delete                                                       */
/* Lua    : map:Delete(flags)                                                */
/* Info   : Remove the map from memory and map list. Release all objects.    */
/*          if flag is non-zero the map is physically deleted too! For multi */
/*          the effect is the same as for false but unique or instance maps  */
/*          are physically deleted.                                          */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Map_Delete(lua_State *L)
{
    lua_object *self;
    char const *path_sh = NULL;
    int         map_player = FALSE,
                flags = 0;

    get_lua_args(L, "M|i", &self, &flags);

    /* sanity checks... we don't test "in_memory" because
     * we want remove perhaps swapped out maps too
     */
    if (!WHERE || !MAP_STATUS_TYPE(WHERE->map_status))
        return 0;

    if(flags) /* caller wants physical remove of the file too */
    {
        /* it only makes sense for unique or instance maps */
        if(!(WHERE->map_status & (MAP_STATUS_UNIQUE|MAP_STATUS_INSTANCE)) )
            flags = FALSE; /* no remove for wrong map types */
        else
        {
            if(!WHERE->path || !(*WHERE->path == '.')) /* last sanity test */
            {
                LOG(llevDebug, "Map_Delete(): non MULTI map without '.' path = %s\n", STRING_SAFE(WHERE->path));
                flags = FALSE;
            }
            else
                path_sh = hooks->add_refcount(WHERE->path);
        }
    }

    if(WHERE->player_first) /* we have players on the map? */
    {
        hooks->map_to_player_unlink(WHERE); /* remove player from map */
        map_player = TRUE;
    }

    /* remove map from /tmp and from memory */
    hooks->clean_tmp_map(WHERE);
    hooks->delete_map(WHERE);

    /* transfer players to the emergency map - why emergency?
     * because our bind point CAN be the same map we killed above!
     */
    if(map_player)
        hooks->map_to_player_link(NULL, -1, -1, TRUE);

    /* handle the file delete */
    if(flags && path_sh)
    {
        unlink(path_sh);
        FREE_ONLY_HASH(path_sh);
    }

    return 0;
}


/*****************************************************************************/
/* Name   : Map_GetFirstObjectOnSquare                                       */
/* Lua    : map:GetFirstObjectOnSquare(x, y)                                 */
/* Info   : Gets the bottom object on the tile. Use obj.above to browse objs */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Map_GetFirstObjectOnSquare(lua_State *L)
{
    lua_object *self;
    int         x,
                y;
    object     *val;
    CFParm     *CFR,
                CFP;

    get_lua_args(L, "Mii", &self, &x, &y);

    CFP.Value[0] = WHERE;
    CFP.Value[1] = (void *) (&x);
    CFP.Value[2] = (void *) (&y);
    CFR = (PlugHooks[HOOK_GETMAPOBJECT]) (&CFP);
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
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Map_GetBrightnessOnSquare(lua_State *L)
{
    lua_object *self;
    int         x,
                y,
                mode = 0,
                brightness,
                i;

    get_lua_args(L, "Mii|i", &self, &x, &y, &mode);

    brightness = hooks->map_brightness(WHERE, x, y);

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
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Map_IsWallOnSquare(lua_State *L)
{
    lua_object *self;
    int         x,
                y;

    get_lua_args(L, "Mii", &self, &x, &y);

    lua_pushboolean(L, hooks->wall(WHERE, x, y));

    return 1;
}

/*****************************************************************************/
/* Name   : Map_IsAnyPlayerOnMap                                             */
/* Lua    : map:IsAnyPlayerOnMap(tiling)                                     */
/* Info   : Checks if at least one player is on the map.                     */
/*          Tiling is optional. If true we also check the upto eight         */
/*          surrounding maps.                                                */
/*          Returns true if any player is found, false otherwise.            */
/* Status : Tested/Stable                                                    */
/*                                                                           */
/* A looping script, or a script triggered repeatedly by a timer, will stop  */
/* the map from being saved, even if all players have left the map. This     */
/* means the script continues forever and the map remains in memory. This    */
/* function allows the script to clean up and exit (or to kill the timer)    */
/* when all players have left.                                               */
/*****************************************************************************/
static int Map_IsAnyPlayerOnMap(lua_State *L)
{
    lua_object *self;
    int         tiling = 0,
                isthere,
                i;

    get_lua_args(L, "M|b", &self, &tiling);

    isthere = (WHERE->player_first) ? 1 : 0;

    if (!isthere && tiling)
        for (i = 0; i < TILED_MAPS; i++)
        {
            isthere = (WHERE->tile_map[i] &&
                       WHERE->tile_map[i]->player_first) ? 1 : 0;

            /* We only care if there is *any* player on the tiles, so at the
             * first positive quit the loop. */
            if (isthere)
                break;
        }

    lua_pushboolean(L, isthere);

    return 1;
}

/*****************************************************************************/
/* Name   : Map_MapTileAt                                                    */
/* Lua    : map:MapTileAt(x, y)                                              */
/* Status : Tested/Stable                                                    */
/* TODO   : do someting about the new modified coordinates too?              */
/*****************************************************************************/
static int Map_MapTileAt(lua_State *L)
{
    lua_object *self;
    int         x,
                y;
    CFParm     *CFR,
                CFP;
    mapstruct  *result;

    get_lua_args(L, "Mii", &self, &x, &y);

    CFP.Value[0] = WHERE;
    CFP.Value[1] = (void *) (&x);
    CFP.Value[2] = (void *) (&y);
    CFR = (PlugHooks[HOOK_OUTOFMAP]) (&CFP);
    result = (mapstruct *) (CFR->Value[0]);

    return push_object(L, &Map, result);
}

/*****************************************************************************/
/* Name   : Map_Save                                                         */
/* Lua    : map:Save(flag)                                                   */
/* Info   : Save the map. If flag is 1, unload map from memory               */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Map_Save(lua_State *L)
{
    lua_object *self;
    int         flags = 0;

    get_lua_args(L, "M|i", &self, &flags);

    if (!WHERE || WHERE->in_memory != MAP_IN_MEMORY)
        return 0;

    if (hooks->new_save_map(WHERE, 0) == -1)
        LOG(llevDebug, "MapSave(): failed to save map %s\n", STRING_SAFE(WHERE->path));

    if (flags)
        WHERE->in_memory = MAP_IN_MEMORY;

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
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Map_PlaySound(lua_State *L)
{
    lua_object *self;
    int         x,
                y,
                soundnumber,
                soundtype = SOUND_NORMAL;

    get_lua_args(L, "Miii|i", &self, &x, &y, &soundnumber, &soundtype);

    hooks->play_sound_map(WHERE, x, y, soundnumber, soundtype);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_Message                                                      */
/* Lua    : map:Message(x, y, distance, message, color, except1, except2)    */
/* Info   : Writes a message to all players on a map, except the two         */
/*          exceptions if specified.                                         */
/*          Starting point x,y for all players in distance                   */
/*          color should be one of the game.COLOR_xxx constants.             */
/*          default color is game.COLOR_BLUE | game.COLOR_UNIQUE             */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/

static int Map_Message(lua_State *L)
{
    lua_object *self,
               *except1,
               *except2;
    int         x,
                y,
                d,
                color;
    char       *message;

    color = NDI_BLUE;
    except1 = NULL;
    except2 = NULL;
    get_lua_args(L, "Miiis|iOO", &self, &x, &y, &d, &message, &color, &except1,
                 &except2);

    /* No point mucking about with an empty message. */
    if (*message)
    {
        hooks->new_info_map_except(NDI_UNIQUE | color, WHERE, x, y, d,
                                   (except1) ? except1->data.object : NULL,
                                   (except2) ? except2->data.object : NULL,
                                   "%s", message);
    }

    return 0;
}

/*****************************************************************************/
/* Name   : Map_CreateObject                                                 */
/* Lua    : map:CreateObject(arch_name, x, y)                                */
/* Info   :                                                                  */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Map_CreateObject(lua_State *L)
{
    lua_object *self;
    char       *txt;
    int         x,
                y;
    CFParm     *CFR,
                CFP;
    object     *new_ob;

    get_lua_args(L, "Msii", &self, &txt, &x, &y);

    CFP.Value[0] = (void *) (txt);
    CFP.Value[1] = (void *) (WHERE);
    CFP.Value[2] = (void *) (&x);
    CFP.Value[3] = (void *) (&y);
    CFR = (PlugHooks[HOOK_CREATEOBJECT]) (&CFP);
    new_ob =  (object *) (CFR->Value[0]);
    free(CFR);

    return push_object(L, &GameObject, new_ob);
}

/*****************************************************************************/
/* Name   : Map_PlayersOnMap                                                 */
/* Lua    : map:PlayersOnMap()                                               */
/* Info   : Returns a table of all the players on map, or nil.               */
/* Status : Untested/Stable                                                  */
/*****************************************************************************/
static int Map_PlayersOnMap(lua_State *L)
{
    lua_object *self;
    object     *ob;
    int         i;

    get_lua_args(L, "M", &self);

    /* No-one on this map, so return nil. */
    if (!WHERE->player_first)
        return 0;

    /* Build up our table. */
    lua_newtable(L);

    for (ob = WHERE->player_first, i = 1; ob;
         ob = CONTR(ob)->map_above, i++)
    {
        push_object(L, &GameObject, ob);
        lua_rawseti(L, -2, i);
    }

    return 1;
}

/*****************************************************************************/
/* Name   : Map_SetDarkness                                                  */
/* Lua    : map:SetDarkness()                                                */
/* Info   : Sets the map-wide 'illumination' to value, normalised to the     */
/*          range 0 <= value <= MAX_DARKNESS).                               */
/*          value should be one of the game.MAP_DARKNESS_* constants, where: */
/*            game.MAP_DARKNESS_TOTAL : 'true' darkness. Info about parts of */
/*                                      the map the player cannot see is not */
/*                                      even sent to the client, meaning this*/
/*                                      is the only kind of map darkness that*/
/*                                      can be used as a gameplay element;   */
/*                                      all others are prone to client-side  */
/*                                      'cheats'.                            */
/*            game.MAP_DARKNESS_MIN : the darkest map darkness other than    */
/*                                    true darkness.                         */
/*            game.MAP_DARKNESS_MAX : the brightest map darkness (full       */
/*                                    daylight).                             */
/* Status : Untested/Stable                                                  */
/* TODO   : Decide how to handle outdoors maps (currently we just ignore the */
/*          flag, meaning the darkness will be set and becomes the new       */
/*          maximum 'illumination' for variable lighting according to tad).  */
/*****************************************************************************/
static int Map_SetDarkness(lua_State *L)
{
    lua_object *self;
    int         value;

    get_lua_args(L, "Mi", &self, &value);

    hooks->set_map_darkness(WHERE, value);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_ActivateConnection                                           */
/* Lua    : map:ActivateConnection(connected, activator, originator)         */
/* Info   : Activates a connection on another map.                           */
/*          map is (of course) the map to activate on.                       */
/*          connected is the object (lever, etc) which started the process.  */
/*          activator and originator (both optional) are objects which caused*/
/*          connected to be activated (eg, activator might be the player who */
/*          pulled the lever).                                               */
/*          The connection activated on map will be the same one that        */
/*          connected is connected to on its map (eg, if connected has       */
/*          connection 1, connection 1 will be activated on map.             */
/*          Note that this method does /not/ actually trigger connected. It  */
/*          is assumed that that has already been done (ie, by a player).    */
/*          Instead, this method merely passes on the fact that connected has*/
/*          been triggered to the connection network on another map.         */
/* Status : Untested/Stable                                                  */
/*****************************************************************************/
static int Map_ActivateConnection(lua_State *L)
{
    lua_object *self,
               *connected,
               *activator,
               *originator;
    int         connection;

    activator = NULL;
    originator = NULL;
    get_lua_args(L, "MO|OO", &self, &connected, &activator, &originator);

    if (!(connection = hooks->get_button_value(connected->data.object)))
        return luaL_error(L, "map:ActivateConnection(): Arg #1 must be connected!");
    else
    {
        oblinkpt *oblp;
        int       flag;

        for (oblp = WHERE->buttons, flag = FALSE; oblp;
             oblp = oblp->next)
        {
            if (oblp->value == connection)
            {
                flag = TRUE;
                break;
            }
        }

        if (!flag)
            return luaL_error(L, "map:ActivateConnection(): Arg #1 has no connection on map!");
    }

    if (WHERE == connected->data.object->map)
        return luaL_error(L, "map:ActivateConnection(): Arg #1 must be on different map!");

    hooks->signal_connection(connected->data.object, (activator) ?
                             activator->data.object : NULL, (originator) ?
                             originator->data.object : NULL, WHERE);

    return 0;
}

/*****************************************************************************/
/* Name   : Map_HasConnection                                                */
/* Lua    : map:HasConnection(connection)                                    */
/* Info   : Checks if map has connection.                                    */
/*          If it does, a numerical table of the connected objects is        */
/*          If it doesn't, the return is nil.                                */
/* Status : Untested/Stable                                                  */
/*****************************************************************************/
static int Map_HasConnection(lua_State *L)
{
    lua_object *self;
    int         connection;
    oblinkpt   *oblp;

    get_lua_args(L, "Mi", &self, &connection);

    for (oblp = WHERE->buttons; oblp; oblp = oblp->next)
    {
        if (oblp->value == connection)
        {
            objectlink *olp;
            int         i;

            lua_newtable(L);

            for (olp = oblp->objlink.link, i = 1; olp; olp = olp->next, i++)
            {
                push_object(L, &GameObject, olp->objlink.ob);
                lua_rawseti(L, -2, i);
            }

            return 1;
        }
    }

    return 0;
}


/* FUNCTIONEND -- End of the Lua plugin functions. */