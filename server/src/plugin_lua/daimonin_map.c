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
    {"name",               FIELDTYPE_CSTR,   offsetof(map_t, name),               FIELDFLAG_READONLY, 0},
    {"message",            FIELDTYPE_CSTR,   offsetof(map_t, msg),                FIELDFLAG_READONLY, 0},
    {"reset_interval",     FIELDTYPE_UINT32, offsetof(map_t, reset_timeout),      FIELDFLAG_READONLY, 0},
    {"difficulty",         FIELDTYPE_UINT16, offsetof(map_t, difficulty),         FIELDFLAG_READONLY, 0},
    {"height",             FIELDTYPE_UINT16, offsetof(map_t, height),             FIELDFLAG_READONLY, 0},
    {"width",              FIELDTYPE_UINT16, offsetof(map_t, width),              FIELDFLAG_READONLY, 0},
    {"enter_x",            FIELDTYPE_UINT16, offsetof(map_t, enter_x),            FIELDFLAG_READONLY, 0},
    {"enter_y",            FIELDTYPE_UINT16, offsetof(map_t, enter_y),            FIELDFLAG_READONLY, 0},
    {"ambient_darkness",   FIELDTYPE_SINT8,  offsetof(map_t, ambient_darkness),   FIELDFLAG_READONLY, 0},
    {"ambient_brightness", FIELDTYPE_SINT16, offsetof(map_t, ambient_brightness), FIELDFLAG_READONLY, 0},
    {"path",               FIELDTYPE_SHSTR,  offsetof(map_t, path),               FIELDFLAG_READONLY, 0},
    {"orig_path",          FIELDTYPE_SHSTR,  offsetof(map_t, orig_path),          FIELDFLAG_READONLY, 0},
    {"status",             FIELDTYPE_UINT32, offsetof(map_t, status),             FIELDFLAG_READONLY, 0},

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
    "f_pvp",

    FLAGLIST_END_MARKER
};

/* pushes flag value on top of stack */
static int Map_getFlag(lua_State *L, lua_object *obj, uint32 flagno)
{
    lua_pushboolean(L, (obj->data.map->flags & (1 << flagno)));
    return 1;
}

/* pushes flag value on top of stack */
static int Map_setFlag(lua_State *L, lua_object *obj, uint32 flagno, int before)
{
    int value = lua_toboolean(L, -1);
    map_t *m = obj->data.map;

    if (before)
    {
    }
    else
    {
        if (value)
        {
            m->flags |= (1 << flagno);
        }
        else
        {
            m->flags &= ~(1 << flagno);
        }
    }

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
    char        buf[HUGE_BUF];

    if (obj == NULL || obj->class->type != LUATYPE_MAP)
        luaL_error(L, "Not a Map object");

    strncpy(buf, obj->data.map->name, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    lua_pushfstring(L, "[%s \"%s\"]", obj->data.map->path, buf);
    return 1;
}

/* Tests if a Map object is valid */
static int Map_isValid(lua_object *obj)
{
    return (obj->tag == obj->data.map->tag);
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
/* Lua    : map:ReadyInheritedMap(path, mode)                                */
/* Info   : Loads the map pointed to by path into memory with the same status*/
/*          (multi, unique, or instance) as the old map.                     */
/*                                                                           */
/*          path is a required string. TODO: This is likely broken for       */
/*          for advanced use, for now it should always be an absolute path). */
/*                                                                           */
/*          mode is an optional number. Use one of:                          */
/*            game.MAP_CHECK - don't load the map if it isn't in memory,     */
/*                             returns nil if the map wasn't in memory.      */
/*            game.MAP_NEW - if the map is already in memory, force an       */
/*                           immediate reset; then (re)load it.              */
/* Return : map pointer to map, or nil                                       */
/*****************************************************************************/
static int Map_ReadyInheritedMap(lua_State *L)
{
    lua_object *self;
    const char *path;
    int         mode = 0;
    shstr_t      *orig_path_sh = NULL,
               *path_sh = NULL;
    map_t  *m = NULL;

    get_lua_args(L, "Ms|i", &self, &path, &mode);

    if (!MAP_STATUS_TYPE(WHERE->status))
    {
        return luaL_error(L, "map:ReadyInheritedMap(): Self must have a status!");
    }

    /* Absolute (multi). */
    if (*path == '/')
    {
        if (hooks->check_path(path, 1) != -1)
        {
            orig_path_sh = hooks->create_safe_path_sh(path);
        }
    }
    /* Unique/instance. */
    else if (!strncmp(path, LOCALDIR "/" PLAYERDIR, LSTRLEN(LOCALDIR "/" PLAYERDIR)) ||
             !strncmp(path, LOCALDIR "/" INSTANCEDIR, LSTRLEN(LOCALDIR "/" INSTANCEDIR)))
    {
        if (hooks->check_path(path, 0) != -1)
        {
            orig_path_sh = hooks->create_safe_path_sh(path);
        }
    }
    /* Relative (multi). */
    else
    {
        char buf[MAXPATHLEN];

        if (hooks->check_path(hooks->normalize_path(WHERE->orig_path, path, buf), 1) != -1)
        {
            orig_path_sh = hooks->create_safe_path_sh(buf);
        }
    }

    if (!orig_path_sh)
    {
        return luaL_error(L, "map:ReadyInheritedMap() could not verify the supplied path: >%s<!",
                          STRING_SAFE(path));
    }

    if ((WHERE->status & (MAP_STATUS_UNIQUE | MAP_STATUS_INSTANCE)))
    {
        char path[MAXPATHLEN];

        FREE_AND_COPY_HASH(path_sh, hooks->normalize_path_direct(WHERE->path,
                           orig_path_sh, path));
    }

    m = hooks->map_is_in_memory((path_sh) ? path_sh : orig_path_sh);

    if (mode == PLUGIN_MAP_NEW &&
        m)
    {
        m->status |= MAP_STATUS_MANUAL_RESET | MAP_STATUS_RELOAD;
        MAP_SET_WHEN_RESET(m, -1);
        hooks->map_check_in_memory(m);
    }
    else if (mode != PLUGIN_MAP_CHECK &&
             (!m ||
              (m->in_memory != MAP_MEMORY_LOADING &&
               m->in_memory != MAP_MEMORY_ACTIVE)))
    {
        m = hooks->ready_map_name((path_sh) ? path_sh : orig_path_sh,
                                  orig_path_sh,
                                  MAP_STATUS_TYPE(WHERE->status),
                                  WHERE->reference);
    }

    FREE_AND_CLEAR_HASH(orig_path_sh);
    FREE_AND_CLEAR_HASH(path_sh);
    push_object(L, &Map, m);

    return 1;
}

/*****************************************************************************/
/* Name   : Map_Delete                                                       */
/* Lua    : map:Delete(seconds, reload)                                      */
/* Info   : Resets the map (remove it from memory, release all objects,      */
/*          teleport any players on it back to their respawn).               */
/*                                                                           */
/*          seconds is an optional number. A positive value causes the map to*/
/*          reset in that many seconds (players on the map will get a        */
/*          countdown every 30s or every s for the last 10). A negative value*/
/*          causes the map to reset according to it's normal timeout). Zero  */
/*          (default) causes an immediate reset.                             */
/*                                                                           */
/*          reload is an optional boolean. True causes the map to instantly  */
/*          reload when it resets (and players will not go to their respawn).*/
/* Return : nil                                                              */
/*****************************************************************************/
static int Map_Delete(lua_State *L)
{
    lua_object *self;
    int         secs = 0,
                reload = 0;

    get_lua_args(L, "M|ib", &self, &secs, &reload);
    WHERE->status |= MAP_STATUS_MANUAL_RESET;

    if (reload)
    {
        WHERE->status |= MAP_STATUS_RELOAD;
    }
    else
    {
        WHERE->status &= ~MAP_STATUS_RELOAD;
    }

    if (secs > 0) // delay X secs
    {
        MAP_SET_WHEN_RESET(WHERE, secs);
    }
    else if (secs < 0) // default for that map
    {
        MAP_SET_WHEN_RESET(WHERE, MAP_RESET_TIMEOUT(WHERE));
    }
    else // immediate
    {
        MAP_SET_WHEN_RESET(WHERE, -1);
        hooks->map_check_in_memory(WHERE);
    }

    return 0;
}

/*****************************************************************************/
/* Name   : Map_GetFirstObjectOnSquare                                       */
/* Lua    : map:GetFirstObjectOnSquare(x, y)                                 */
/* Info   : Gets the bottom object on the tile. Use obj.above to browse objs */
/*****************************************************************************/
static int Map_GetFirstObjectOnSquare(lua_State *L)
{
    lua_object *self;
    sint32      x,
                y;
    map_t      *m2;
    sint16      x2,
                y2;
    msp_t      *msp;
    object_t   *o = NULL;

    get_lua_args(L, "Mii", &self, &x, &y);
    m2 = WHERE;
    x2 = x;
    y2 = y;

    if ((msp = MSP_GET(m2, x2, y2)))
    {
        o = msp->last;
    }

    push_object(L, &GameObject, o);
    return 1;
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
/*****************************************************************************/
static int Map_GetBrightnessOnSquare(lua_State *L)
{
    lua_object *self;
    sint32      x,
                y,
                mode = 0;
    map_t      *m2;
    sint16      x2,
                y2;
    msp_t      *msp;
    uint16      n = 0;

    get_lua_args(L, "Mii|i", &self, &x, &y, &mode);
    m2 = WHERE;
    x2 = x;
    y2 = y;

    if ((msp = MSP_GET(m2, x2, y2)))
    {
        hooks->get_tad(m2->tadnow, m2->tadoffset);
        n = MSP_GET_REAL_BRIGHTNESS(msp);

        if (mode == 0)
        {
            uint8 i;

            /* Convert brightness to 0-MAX_DARKNESS scale */
            for(i = 0; i < MAX_DARKNESS; i++)
            {
                if (n < hooks->brightness[i])
                {
                    n = MAX(0, i - 1);
                    break;
                }
            }

            if (i == MAX_DARKNESS) /* Didn't find it? */
            {
                n = i;
            }
        }
    }

    lua_pushnumber(L, (lua_Number)n);
    return 1;
}

/*****************************************************************************/
/* Name   : Map_IsWallOnSquare                                               */
/* Lua    : map:IsWallOnSquare(x, y)                                         */
/* Info   : returns true if the square at x,y is a wall                      */
/*****************************************************************************/
static int Map_IsWallOnSquare(lua_State *L)
{
    lua_object *self;
    sint32      x,
                y;
    map_t      *m2;
    sint16      x2,
                y2;
    msp_t      *msp;

    get_lua_args(L, "Mii", &self, &x, &y);
    m2 = WHERE;
    x2 = x;
    y2 = y;
    msp = MSP_GET(m2, x2, y2);
    lua_pushboolean(L, (!msp || MSP_IS_RESTRICTED(msp)) ? 1 : 0);
    return 1;
}

/*****************************************************************************/
/* Name   : Map_IsAnyPlayerOnMap                                             */
/* Lua    : map:IsAnyPlayerOnMap(tiling)                                     */
/* Info   : Checks if at least one player is on the map.                     */
/*          Tiling is optional. If true we also check the upto eight         */
/*          surrounding maps.                                                */
/*          Returns true if any player is found, false otherwise.            */
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
        for (i = 0; i < TILING_DIRECTION_NROF; i++)
        {
            isthere = (WHERE->tiling.tile_map[i] &&
                       WHERE->tiling.tile_map[i]->player_first) ? 1 : 0;

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
/*****************************************************************************/
static int Map_MapTileAt(lua_State *L)
{
    lua_object *self;
    sint32      x,
                y;
    map_t      *m2;
    sint16      x2,
                y2;

    get_lua_args(L, "Mii", &self, &x, &y);
    x2 = x;
    y2 = y;
    m2 = OUT_OF_MAP(WHERE, x2, y2);
    push_object(L, &Map, m2);
    lua_pushnumber(L, x2);
    lua_pushnumber(L, y2);
    return 3;
}

/*****************************************************************************/
/* Name   : Map_Save                                                         */
/* Lua    : map:Save()                                                       */
/* Info   : Save the map.                                                    */
/* Return : nil                                                              */
/*****************************************************************************/
static int Map_Save(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "M", &self);

    if (!hooks->map_save(WHERE))
    {
        return luaL_error(L, "MapSave(): failed!");
    }

    WHERE->in_memory = MAP_MEMORY_ACTIVE;

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
/*****************************************************************************/
static int Map_PlaySound(lua_State *L)
{
    lua_object *self;
    int         x,
                y,
                soundnumber,
                soundtype = SOUND_NORMAL;

    get_lua_args(L, "Miii|i", &self, &x, &y, &soundnumber, &soundtype);
    hooks->play_sound_map(MSP_RAW(WHERE, x, y), soundnumber, soundtype);

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
        hooks->ndi_map(NDI_UNIQUE | color, MSP_RAW(WHERE, x, y), d,
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
/*****************************************************************************/
static int Map_CreateObject(lua_State *L)
{
    lua_object *self;
    char       *txt;
    int         x,
                y;
    CFParm     *CFR,
                CFP;
    object_t     *new_ob;

    get_lua_args(L, "Msii", &self, &txt, &x, &y);

    CFP.Value[0] = (void *) (txt);
    CFP.Value[1] = (void *) (WHERE);
    CFP.Value[2] = (void *) (&x);
    CFP.Value[3] = (void *) (&y);
    CFR = (PlugHooks[HOOK_CREATEOBJECT]) (&CFP);
    new_ob =  (object_t *) (CFR->Value[0]);
    free(CFR);

    return push_object(L, &GameObject, new_ob);
}

/*****************************************************************************/
/* Name   : Map_PlayersOnMap                                                 */
/* Lua    : map:PlayersOnMap()                                               */
/* Info   : Returns a table of all the players on map, or nil.               */
/*****************************************************************************/
static int Map_PlayersOnMap(lua_State *L)
{
    lua_object *self;
    object_t     *ob;
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
/* Info   : Sets the map-wide darkness to value.                             */
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
/* TODO   : This needs an overhaul. Probably it should be Mi|ii: where 2 args*/
/*          sets the map's ambient light levels (as now) and 4 args sets the */
/*          msp's floor light levels.                                        */
/*****************************************************************************/
static int Map_SetDarkness(lua_State *L)
{
    lua_object *self;
    int         value;

    get_lua_args(L, "Mi", &self, &value);

    if (value < 0||
        value > MAX_DARKNESS)
    {
        value = MAX_DARKNESS;
    }

    WHERE->ambient_darkness = value;
    WHERE->ambient_brightness = hooks->brightness[value];
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
        objectlink_t *oblp;
        int       flag;

        for (oblp = WHERE->buttons, flag = 0; oblp;
             oblp = oblp->next)
        {
            if (oblp->value == connection)
            {
                flag = 1;
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
/*****************************************************************************/
static int Map_HasConnection(lua_State *L)
{
    lua_object *self;
    int         connection;
    objectlink_t   *oblp;

    get_lua_args(L, "Mi", &self, &connection);

    for (oblp = WHERE->buttons; oblp; oblp = oblp->next)
    {
        if (oblp->value == connection)
        {
            objectlink_t *olp;
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
