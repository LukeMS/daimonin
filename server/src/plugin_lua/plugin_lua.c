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

/* First let's include the header file needed                                */

#include <global.h>
#include <plugin_lua.h>
#include <inline.h>

#include <lualib.h>

#include <stdarg.h>

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

/* This one is used to cleanly pass args to the CF core */
CFParm                          GCFP;
CFParm                          GCFP0;
CFParm                          GCFP1;
CFParm                          GCFP2;

/* script context list */
struct lua_context             *first_context;

/* Our global state from which we spawn threads for running scripts */
struct lua_State               *global_state;

/* possibly reference to error handler function */
static int error_handler_ref = LUA_NOREF;
static int globals_metatable_ref = LUA_NOREF;

/*
 * Declarations for the two utility classes
 * - Game contains the old Daimonin functions and constants
 *   that don't fit into objects
 * - Event contains the script-related attributes such as activator,
 *   message and return value.
 */

/* Game methods */
static int  Game_FileUnlink(lua_State *L);
static int  Game_TransferMapItems(lua_State *L);
static int  Game_MatchString(lua_State *L);
static int  Game_ReadyMap(lua_State *L);
static int  Game_FindPlayer(lua_State *L);
static int  Game_GetSpellNr(lua_State *L);
static int  Game_GetSkillNr(lua_State *L);
static int  Game_CheckMap(lua_State *L);
// static int Game_RegisterCommand(lua_State *L);
static int  Game_LoadObject(lua_State *L);

static struct method_decl       Game_methods[]      =
{
    {"FileUnlink", Game_FileUnlink}, {"TransferMapItems", Game_TransferMapItems}, {"LoadObject", Game_LoadObject},
    {"ReadyMap", Game_ReadyMap}, {"CheckMap", Game_CheckMap}, {"MatchString", Game_MatchString},
    {"FindPlayer", Game_FindPlayer}, {"GetSpellNr", Game_GetSpellNr}, {"GetSkillNr", Game_GetSkillNr},
    //    {"RegisterCommand", Game_RegisterCommand},
    {NULL, NULL}
};

static struct attribute_decl    Event_attributes[]  =
{
    {"me", FIELDTYPE_OBJECT, offsetof(struct lua_context, self), FIELDFLAG_READONLY},
    {"activator", FIELDTYPE_OBJECT, offsetof(struct lua_context, activator), FIELDFLAG_READONLY},
    {"other", FIELDTYPE_OBJECT, offsetof(struct lua_context, other), FIELDFLAG_READONLY},
    {"message", FIELDTYPE_SHSTR, offsetof(struct lua_context, text), FIELDFLAG_READONLY},
    {"options", FIELDTYPE_SHSTR, offsetof(struct lua_context, options), FIELDFLAG_READONLY},
    {"returnvalue", FIELDTYPE_SINT32, offsetof(struct lua_context, returnvalue)}, {NULL}
};

/* Useful constants */
static struct constant_decl     Game_constants[]    =
{
    {"NORTH", 1}, {"NORTHEAST", 2}, {"EAST", 3}, {"SOUTHEAST", 4}, {"SOUTH", 5}, {"SOUTHWEST", 6}, {"WEST", 7},
    {"NORTHWEST", 8}, {"MAP_INFO_NORMAL", MAP_INFO_NORMAL}, {"MAP_INFO_ALL", MAP_INFO_ALL}, {"COST_TRUE", F_TRUE},
    {"COST_BUY", F_BUY}, {"COST_SELL", F_SELL}, {"APPLY_TOGGLE", 0}, {"APPLY_ALWAYS", AP_APPLY},
    {"UNAPPLY_ALWAYS", AP_UNAPPLY}, {"UNAPPLY_NO_MERGE", AP_NO_MERGE}, {"UNAPPLY_IGNORE_CURSE", AP_IGNORE_CURSE},
    {"NEUTER", 0}, {"MALE", 1}, {"FEMALE", 2}, {"HERMAPHRODITE", 3}, {"CAST_NORMAL", 0}, {"CAST_POTION", 1},
    {"LEARN", 0}, {"UNLEARN", 1}, {"UNIDENTIFIED", 0}, {"IDENTIFIED", 1}, {"IDENTIFY_NORMAL", 0}, {"IDENTIFY_ALL", 1},
    {"IDENTIFY_MARKED", 2}, {"CLONE_WITH_INVENTORY", 0}, {"CLONE_WITHOUT_INVENTORY", 1}, {"EXP_AGILITY", 1},
    {"EXP_MENTAL", 2}, {"EXP_MAGICAL", 3}, {"EXP_PERSONAL", 4}, {"EXP_PHYSICAL", 5}, {"EXP_WISDOM", 6},
    {"COLOR_ORANGE", NDI_ORANGE}, {"COLOR_WHITE", NDI_WHITE}, {"COLOR_NAVY", NDI_NAVY}, {"COLOR_YELLOW", NDI_YELLOW},
    {"COLOR_BLUE", NDI_BLUE}, {"COLOR_RED", NDI_RED},

    /* Argh, the object types. Make sure to keep up-to date if any are added/removed */
    {"TYPE_PLAYER"                  ,PLAYER}, {"TYPE_BULLET"                  ,BULLET},
    {"TYPE_ROD"                     ,ROD}, {"TYPE_TREASURE"                ,TREASURE},
    {"TYPE_POTION"                  ,POTION}, {"TYPE_FOOD"                    ,FOOD},
    {"TYPE_POISON"                  ,POISON}, {"TYPE_BOOK"                    ,BOOK},
    {"TYPE_CLOCK"                   ,CLOCK}, {"TYPE_FBULLET"                 ,FBULLET},
    {"TYPE_FBALL"                   ,FBALL}, {"TYPE_LIGHTNING"               ,LIGHTNING},
    {"TYPE_ARROW"                   ,ARROW}, {"TYPE_BOW"                     ,BOW},
    {"TYPE_WEAPON"                  ,WEAPON}, {"TYPE_ARMOUR"                  ,ARMOUR},
    {"TYPE_PEDESTAL"                ,PEDESTAL}, {"TYPE_ALTAR"                   ,ALTAR},
    {"TYPE_CONFUSION"               ,CONFUSION}, {"TYPE_LOCKED_DOOR"             ,LOCKED_DOOR},
    {"TYPE_SPECIAL_KEY"             ,SPECIAL_KEY}, {"TYPE_MAP"                     ,MAP},
    {"TYPE_DOOR"                    ,DOOR}, {"TYPE_KEY"                     ,KEY},
    {"TYPE_MMISSILE"                ,MMISSILE}, {"TYPE_TIMED_GATE"              ,TIMED_GATE},
    {"TYPE_TRIGGER"                 ,TRIGGER}, {"TYPE_GRIMREAPER"              ,GRIMREAPER},
    {"TYPE_MAGIC_EAR"               ,MAGIC_EAR}, {"TYPE_TRIGGER_BUTTON"          ,TRIGGER_BUTTON},
    {"TYPE_TRIGGER_ALTAR"           ,TRIGGER_ALTAR}, {"TYPE_TRIGGER_PEDESTAL"        ,TRIGGER_PEDESTAL},
    {"TYPE_SHIELD"                  ,SHIELD}, {"TYPE_HELMET"                  ,HELMET},
    {"TYPE_HORN"                    ,HORN}, {"TYPE_MONEY"                   ,MONEY},
    {"TYPE_CLASS"                   ,CLASS}, {"TYPE_GRAVESTONE"              ,GRAVESTONE},
    {"TYPE_AMULET"                  ,AMULET}, {"TYPE_PLAYERMOVER"             ,PLAYERMOVER},
    {"TYPE_TELEPORTER"              ,TELEPORTER}, {"TYPE_CREATOR"                 ,CREATOR},
    {"TYPE_SKILL"                   ,SKILL}, {"TYPE_EXPERIENCE"              ,EXPERIENCE},
    {"TYPE_EARTHWALL"               ,EARTHWALL}, {"TYPE_GOLEM"                   ,GOLEM},
    {"TYPE_BOMB"                    ,BOMB}, {"TYPE_THROWN_OBJ"              ,THROWN_OBJ},
    {"TYPE_BLINDNESS"               ,BLINDNESS}, {"TYPE_GOD"                     ,GOD},
    {"TYPE_DETECTOR"                ,DETECTOR}, {"TYPE_SPEEDBALL"               ,SPEEDBALL},
    {"TYPE_DEAD_OBJECT"             ,DEAD_OBJECT}, {"TYPE_DRINK"                   ,DRINK},
    {"TYPE_MARKER"                  ,MARKER}, {"TYPE_HOLY_ALTAR"              ,HOLY_ALTAR},
    {"TYPE_PLAYER_CHANGER"          ,PLAYER_CHANGER}, {"TYPE_BATTLEGROUND"            ,BATTLEGROUND},
    {"TYPE_PEACEMAKER"              ,PEACEMAKER}, {"TYPE_GEM"                     ,GEM},
    {"TYPE_FIRECHEST"               ,FIRECHEST}, {"TYPE_FIREWALL"                ,FIREWALL},
    {"TYPE_ANVIL"                   ,ANVIL}, {"TYPE_CHECK_INV"               ,CHECK_INV},
    {"TYPE_MOOD_FLOOR"              ,MOOD_FLOOR}, {"TYPE_EXIT"                    ,EXIT},
    {"TYPE_AGE_FORCE"               ,TYPE_AGE_FORCE}, {"TYPE_SHOP_FLOOR"              ,SHOP_FLOOR},
    {"TYPE_SHOP_MAT"                ,SHOP_MAT}, {"TYPE_RING"                    ,RING},
    {"TYPE_FLOOR"                   ,FLOOR}, {"TYPE_FLESH"                   ,FLESH},
    {"TYPE_INORGANIC"               ,INORGANIC}, {"TYPE_LIGHT_APPLY"             ,TYPE_LIGHT_APPLY },
    {"TYPE_LIGHTER"                 ,LIGHTER}, {"TYPE_TRAP_PART"               ,TRAP_PART},
    {"TYPE_WALL"                    ,WALL}, {"TYPE_LIGHT_SOURCE"            ,LIGHT_SOURCE},
    {"TYPE_MISC_OBJECT"             ,MISC_OBJECT}, {"TYPE_MONSTER"                 ,MONSTER},
    /* TODO: keep replaceing with the true defines... */
    {"TYPE_SPAWN_POINT"             ,81}, {"TYPE_LIGHT_REFILL"            ,82}, {"TYPE_SPAWN_POINT_MOB"     ,83 },
    {"TYPE_SPAWN_POINT_INFO"        ,84}, {"TYPE_SPELLBOOK"               ,85}, {"TYPE_ORGANIC"             ,86},
    {"TYPE_CLOAK"                   ,87}, {"TYPE_CONE"                    ,88}, {"TYPE_AURA"                   ,89},
    {"TYPE_SPINNER"             ,90}, {"TYPE_GATE"                    ,91}, {"TYPE_BUTTON"                  ,92},
    {"TYPE_CF_HANDLE"               ,93}, {"TYPE_PIT"                 ,94}, {"TYPE_TRAPDOOR"                ,95},
    {"TYPE_WORD_OF_RECALL"          ,96}, {"TYPE_PARAIMAGE"               ,97}, {"TYPE_SIGN"                    ,98},
    {"TYPE_BOOTS"                   ,99}, {"TYPE_GLOVES"                  ,100}, {"TYPE_BASE_INFO"           ,101},
    {"TYPE_RANDOM_DROP"     ,102}, {"TYPE_CONVERTER"               ,103}, {"TYPE_BRACERS"             ,104},
    {"TYPE_POISONING"               ,105 }, {"TYPE_SAVEBED"             ,106}, {"TYPE_POISONCLOUD"         ,107},
    {"TYPE_FIREHOLES"               ,108}, {"TYPE_WAND"                    ,109}, {"TYPE_ABILITY"             ,110},
    {"TYPE_SCROLL"                  ,111}, {"TYPE_DIRECTOR"                ,112}, {"TYPE_GIRDLE"                  ,113},
    {"TYPE_FORCE"                   ,114}, {"TYPE_POTION_EFFECT"         ,115}, {"TYPE_JEWEL"               ,116},
    {"TYPE_NUGGET"          ,117}, {"TYPE_EVENT_OBJECT"        ,118}, {"TYPE_WAYPOINT_OBJECT" ,119},
    {"TYPE_QUEST_CONTAINER" ,120}, {"TYPE_CLOSE_CON"               ,121}, {"TYPE_CONTAINER"               ,122},
    {"TYPE_ARMOUR_IMPROVER"        ,123}, {"TYPE_WEAPON_IMPROVER"        ,124}, {"TYPE_WEALTH"        ,125},
    {"TYPE_SKILLSCROLL"         ,130}, {"TYPE_DEEP_SWAMP"              ,138}, {"TYPE_IDENTIFY_ALTAR"          ,139},
    {"TYPE_CANCELLATION"            ,141}, {"TYPE_MENU"                    ,150}, {"TYPE_BALL_LIGHTNING"         ,151},
    {"TYPE_SWARM_SPELL"            ,153}, {"TYPE_RUNE"                   ,154}, {"TYPE_POWER_CRYSTAL"          ,156},
    {"TYPE_CORPSE"                 ,157}, {"TYPE_DISEASE"                ,158}, {"TYPE_SYMPTOM"                ,159},
    {NULL, 0}
};

/* Basic script classes */
lua_class                       Event               =
{
    LUATYPE_EVENT, "Event", 0, NULL, Event_attributes,
};
lua_class                       Game                =
{
    LUATYPE_GAME, "Game", 0, NULL, NULL, Game_methods, Game_constants
};

/*
 * Other globals
 */

#if 0
/* Commands management part */
PythonCmd CustomCommand[NR_CUSTOM_CMD];
int NextCustomCommand;
#endif

/****************************************************************************/
/*                                                                          */
/*                          Game module functions                           */
/*                                                                          */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : Game_FileUnlink(path)                                            */
/* Lua    : game.FileUnlink(path)                                            */
/* Info   : Unlink the file (delete is physically).                          */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Game_FileUnlink(lua_State *L)
{
    char   *fname;

    get_lua_args(L, "s", &fname);

    unlink(fname);

    return 0;
}

/*****************************************************************************/
/* Name   : Game_TransferMapItems                                            */
/* Lua    : game.TranserMapItems(map_old, map_new, x, y)                     */
/* Info   : Transfer all items with "no_pick 0" setting from map_old         */
/*          to position x,y on map new.                                      */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Game_TransferMapItems(lua_State *L)
{
    lua_object *map_new, *map_old;
    int         x, y;

    get_lua_args(L, "MMii", &map_old, &map_new, &x, &y);

    GCFP.Value[0] = (void *) (map_old->data.map);
    GCFP.Value[1] = (void *) (map_new->data.map);
    GCFP.Value[2] = (void *) (&x);
    GCFP.Value[3] = (void *) (&y);

    (PlugHooks[HOOK_MAPTRANSERITEMS]) (&GCFP);

    return 0;
}


/*****************************************************************************/
/* Name   : Game_LoadObject                                                  */
/* Lua    : game.LoadObject(string)                                          */
/* Status : Untested                                                         */
/*****************************************************************************/
static int Game_LoadObject(lua_State *L)
{
    object *whoptr;
    char   *dumpob;
    CFParm *CFR;

    get_lua_args(L, "s", &dumpob);

    /* First step: We create the object */
    GCFP.Value[0] = (void *) (dumpob);
    CFR = (PlugHooks[HOOK_LOADOBJECT]) (&GCFP);
    whoptr = (object *) (CFR->Value[0]);
    free(CFR);

    return push_object(L, &GameObject, whoptr);
}

/*****************************************************************************/
/* Name   : Game_MatchString                                                 */
/* Lua    : game.MatchString(firststr, secondstr)                            */
/* Info   : Case insensitive string comparision. Returns 1 if the two        */
/*          strings are the same, or 0 if they differ.                       */
/*          secondstring can contain regular expressions.                    */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Game_MatchString(lua_State *L)
{
    char   *premiere;
    char   *seconde;

    get_lua_args(L, "ss", &premiere, &seconde);

    lua_pushboolean(L, (hooks->re_cmp(premiere, seconde) != NULL));
    return 1;
}

/*****************************************************************************/
/* Name   : Game_ReadyMap                                                    */
/* Lua    : game.ReadyMap(name, unique)                                      */
/* Info   : Make sure the named map is loaded into memory. unique _must_ be  */
/*          1 if the map is unique (f_unique = 1).                           */
/*          IF flags | 1, the map path is already the right unique one.      */
/*          If flags | 2, the map path is original and must be changed .     */
/*          If flags | 4  *unique maps only* unique map gets DELETED  and    */
/*                        fresh reloaded!                                    */
/*          Default value for unique is 0                                    */
/* Status : Stable                                                           */
/*****************************************************************************/

static int Game_ReadyMap(lua_State *L)
{
    char       *mapname;
    mapstruct  *mymap;
    lua_object *obptr   = NULL;
    int         flags   = 0;
    CFParm     *CFR;
    get_lua_args(L, "si|O", &mapname, &flags, &obptr);

    GCFP.Value[0] = (void *) (mapname);
    GCFP.Value[1] = (void *) (&flags);
    GCFP.Value[2] = NULL;
    if (obptr)
        GCFP.Value[2] = (void *) (obptr->data.object);

    CFR = (PlugHooks[HOOK_READYMAPNAME]) (&GCFP);
    mymap = (mapstruct *) (CFR->Value[0]);
    free(CFR);

    return push_object(L, &Map, mymap);
}

/*****************************************************************************/
/* Name   : Game_CheckMap                                                    */
/* Lua    : game.CheckMap(arch, map_path, x, y)                              */
/* Info   :                                                                  */
/* Status : Unfinished. DO NOT USE!                                          */
/*****************************************************************************/

static int Game_CheckMap(lua_State *L)
{
    char   *what;
    char   *mapstr;
    int     x, y;
    /*    object* foundob; */

    /* Gecko: replaced coordinate tuple with separate x and y coordinates */
    get_lua_args(L, "ssii", &what, &mapstr, &x, &y);

    luaL_error(L, "CheckMap() is not finished!");

    /*    foundob = present_arch(
            find_archetype(what),
            has_been_loaded(mapstr),
            x,y
        );
        return wrap_object(foundob);*/

    return 0;
}


/*****************************************************************************/
/* Name   : Game_FindPlayer                                                  */
/* Lua    : game.FindPlayer(name)                                            */
/* Status : Tested                                                           */
/*****************************************************************************/

static int Game_FindPlayer(lua_State *L)
{
    player *foundpl;
    object *foundob = NULL;
    CFParm *CFR;
    char   *txt;

    get_lua_args(L, "s", &txt);

    GCFP.Value[0] = (void *) (txt);
    CFR = (PlugHooks[HOOK_FINDPLAYER]) (&GCFP);
    foundpl = (player *) (CFR->Value[0]);
    free(CFR);

    if (foundpl != NULL)
        foundob = foundpl->ob;

    return push_object(L, &GameObject, foundob);
}

/*****************************************************************************/
/* Name   : Game_GetSpellNr                                                  */
/* Lua    : game.GetSpellNr(name)                                            */
/* Info   : Gets the number of the named spell. -1 if no such spell exists   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int Game_GetSpellNr(lua_State *L)
{
    char   *spell;
    CFParm *CFR;
    int     value;

    get_lua_args(L, "s", &spell);

    GCFP.Value[0] = (void *) (spell);
    CFR = (PlugHooks[HOOK_CHECKFORSPELLNAME]) (&GCFP);
    value = *(int *) (CFR->Value[0]);

    lua_pushnumber(L, value);
    return 1;
}

/*****************************************************************************/
/* Name   : Game_GetSkillNr                                                  */
/* Lua    : game.GetSkillNr(name)                                            */
/* Info   : Gets the number of the named skill. -1 if no such skill exists   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int Game_GetSkillNr(lua_State *L)
{
    char   *skill;
    CFParm *CFR;
    int     value;

    get_lua_args(L, "s", &skill);

    GCFP.Value[0] = (void *) (skill);
    CFR = (PlugHooks[HOOK_CHECKFORSKILLNAME]) (&GCFP);
    value = *(int *) (CFR->Value[0]);

    lua_pushnumber(L, value);
    return 1;
}

/* FUNCTIONEND -- End of the Lua plugin functions. */

/* Our error handler. Tries to call a lua error handler called "_error" */
static int luaError(lua_State *L)
{
    if(error_handler_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, error_handler_ref);
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_setfenv(L, -2); /* Make sure it runs in the thread environment */
        lua_pushvalue(L, 1);
        lua_call(L,1,1);
        return 1;
    } else
        return 1;
}

/* Compiles the Lua script 'file' */
static int luaCompile(lua_State *L, const char **file)
{
    static char buf[MAX_BUF];
    FILE       *fp;
    int         res;
    size_t      size;
    char       *suffix;
    struct stat s1, s2;

    size = strlen(*file);

    if(size > MAX_BUF - 4)
    {
        lua_pushfstring(L, "the filename '%s' is longer than %d bytes", *file, MAX_BUF - 4);
        return LUA_ERRMEM;
    }

    strncpy(buf, *file, size + 1);

    if (stat(*file, &s1))
    {
        lua_pushfstring(L, "couldn't find script file %s", *file);
        return LUA_ERRFILE;
    }

    if ((suffix = strrchr(buf, '.')) == NULL || (strrchr(buf, '/')) > suffix)
        memset(suffix = buf + (size + 1), 0, 4);
    else if ((strncmp(suffix, ".lc", 4)) == 0)
    {
        strncpy(suffix, ".lua", 5);

        if (stat(buf, &s2) || s2.st_mtime <= s1.st_mtime)
            return 0;

        if ((res = luaL_loadfile(L, buf)))
            return res;

        strncpy(suffix, ".lc", 4);
    }
    else
    {
        if ((strncmp(suffix, ".lua", 5)) == 0)
            memset(suffix, 0, 4);
        else
            suffix = buf + size + 1;

        strncpy(suffix, ".lc", 4);

        if (!stat(buf, &s2) && s2.st_mtime >= s1.st_mtime)
        {
            *file = buf;
            return 0;
        }

        if ((res = luaL_loadfile(L, *file)))
            return res;
    }

#ifdef LUA_DEBUG
    LOG(llevDebug, "LUA - Compiling file '%s' -> '%s'\n", *file, buf);
#endif

    lua_pushliteral(L, "string");
    lua_gettable(L, LUA_GLOBALSINDEX);
    lua_pushliteral(L, "dump");
    lua_rawget(L, -2);
    lua_remove(L, -2);
    lua_pushvalue(L, -2);

    lua_call(L, 1, 1);

    errno = 0;
    if((fp = fopen(buf, "wb")) == NULL)
    {
        lua_pop(L, 2);
        lua_pushfstring(L, "Couldn't create or open file %s\n%s\n", buf, errno ? strerror(errno) : "Unkown error");
        errno = 0;
        return LUA_ERRFILE;
    }

    fwrite(lua_tostring(L, -1), lua_strlen(L, -1), sizeof(char), fp);
    lua_pop(L, 2);

    if(ferror(fp))
    {
        lua_pushfstring(L, "An error occured while writing to file %s\n%s\n", buf, errno ? strerror(errno) : "Unkown error");
        errno = 0;
        return LUA_ERRFILE;
    }
    fclose(fp);

    *file = buf;

    return 0;
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
    const char *path = hooks->create_pathname(file);
    int         res = 0;

    if ((res = luaFindFile(L, file, &path)) == 0)
    {
//        if ((res = luaCompile(L, &path)) == 0)
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
/* Called to send the hooks struct to the plugin.                             */
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
MODULEAPI CFParm * triggerEvent(CFParm *PParm)
{
    /*CFParm *CFP; */
    int         eventcode;
    static int  result;

    eventcode = *(int *) (PParm->Value[0]);
#ifdef LUA_DEBUG
    LOG(llevDebug, "LUA - triggerEvent:: eventcode %d\n", eventcode);
#endif
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
        case EVENT_STOP:
        case EVENT_TELL:
        case EVENT_TIME:
        case EVENT_THROW:
        case EVENT_TRIGGER:
        case EVENT_CLOSE:
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
    GCFP.Value[0] = (void *) (&result);

    return &GCFP;
}

/*****************************************************************************/
/* Handles standard global events.                                            */
/*****************************************************************************/
MODULEAPI int HandleGlobalEvent(CFParm *PParm)
{
    LOG(llevDebug, "Unimplemented for now\n");

#if 0
    switch(*(int *)(PParm->Value[0]))
    {
        case EVENT_CRASH:
            LOG(llevDebug, "Unimplemented for now\n");
            break;
        case EVENT_BORN:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event BORN generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */
            RunPythonScript("python/python_born.py");
            break;
        case EVENT_LOGIN:
            StackActivator[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackWho[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event LOGIN generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */
            /*LOG(llevDebug, "IP is %s\n", (char *)(PParm->Value[2])); */
            RunPythonScript("python/python_login.py");
            break;
        case EVENT_LOGOUT:
            StackActivator[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackWho[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event LOGOUT generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */
            RunPythonScript("python/python_logout.py");
            break;
        case EVENT_REMOVE:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event REMOVE generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            RunPythonScript("python/python_remove.py");
            break;
        case EVENT_SHOUT:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event SHOUT generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            /*LOG(llevDebug, "Message shout is %s\n",StackText[StackPosition]); */
            RunPythonScript("python/python_shout.py");
            break;
        case EVENT_MAPENTER:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event MAPENTER generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            RunPythonScript("python/python_mapenter.py");
            break;
        case EVENT_MAPLEAVE:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event MAPLEAVE generated by %s\n",STRING_OBJ_NAME(StackActivator[StackPosition])); */

            RunPythonScript("python/python_mapleave.py");
            break;
        case EVENT_CLOCK:
            /* LOG(llevDebug, "Event CLOCK generated\n"); */
            RunPythonScript("python/python_clock.py");
            break;
        case EVENT_MAPRESET:
            StackText[StackPosition] = (char *)(PParm->Value[1]);/* Map name/path */
            LOG(llevDebug, "Event MAPRESET generated by %s\n", StackText[StackPosition]);

            RunPythonScript("python/python_mapreset.py");
            break;
    }
    StackPosition--;
#endif

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

    /* "next" and "ipairs" need to be in the local env */
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

    /* Set up the error handler function */
    lua_pushcclosure(L, luaError, 0);

    /* Load the actual script function */
    res = luaLoadFile(L, context->file);
    if (res == 0)
    {
        /* First set up environment for the function. */
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_setfenv(L, -2);

        /* Loadfile puts the loaded chunk on top of the stack as a function,
         * we call it without parameters and not caring about return values */
        res = lua_pcall(L, 0, 0, -2);
        // res = lua_resume(L, 0);

        if (res == 0)
        {
            /* TODO: if script just wants to yield, not end. store its
             * global environment somewhere so we can continue later */

            return 0;
        }
    }

    error = lua_tostring(L, -1);
    if (error)
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
        *(int *) (PParm->Value[5]), *(int *) (PParm->Value[6]), *(int *) (PParm->Value[7]), *(int *) (PParm->Value[8]));
#endif

    context = malloc(sizeof(struct lua_context));

    context->next = context->prev = NULL;
    context->state = lua_newthread(global_state);
    context->threadidx = luaL_ref(global_state, LUA_REGISTRYINDEX);

    /* And all the event parameters */
    context->activator = (object *) (PParm->Value[1]);
    context->self = (object *) (PParm->Value[2]);
    context->other = (object *) (PParm->Value[3]);
    context->text = (const char *) (PParm->Value[4]);
    context->parm1 = *(int *) (PParm->Value[5]);
    context->parm2 = *(int *) (PParm->Value[6]);
    context->parm3 = *(int *) (PParm->Value[7]);
    context->parm4 = *(int *) (PParm->Value[8]);
    context->file = (const char *) (PParm->Value[9]);
    context->options = (const char *) (PParm->Value[10]);
    context->returnvalue = 0;

    res = RunLuaScript(context);

    /* Get rid of the thread object, and hope the thread gets collected */
    /* TODO: make it possible to keep the thread & context alive,
     * possibly using yield() and resume() */
    luaL_unref(global_state, LUA_REGISTRYINDEX, context->threadidx);

    if (res)
    {
        free(context);
        return 0;
    }

#ifdef LUA_DEBUG
    LOG(llevDebug, "fixing. ");
#endif

    if (context->parm4 == SCRIPT_FIX_ALL)
    {
        if (context->other != NULL)
            fix_player_hook(context->other);
        if (context->self != NULL)
            fix_player_hook(context->self);
        if (context->activator != NULL)
            fix_player_hook(context->activator);
    }
    else if (context->parm4 == SCRIPT_FIX_ACTIVATOR)
    {
        fix_player_hook(context->activator);
    }

    ret = context->returnvalue;
    free(context);

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
MODULEAPI CFParm * initPlugin(CFParm *PParm)
{
    LOG(llevDebug, "    Daimonin Lua Plugin loading.....\n");

    init_Daimonin_Lua();

    GCFP.Value[0] = (void *) PLUGIN_NAME;
    GCFP.Value[1] = (void *) PLUGIN_VERSION;
    return &GCFP;
}

/*****************************************************************************/
/* Used to do cleanup before killing the plugin.                             */
/*****************************************************************************/
MODULEAPI CFParm * removePlugin(CFParm *PParm)
{
    lua_close(global_state);

    return NULL;
}

/*****************************************************************************/
/* This function is called to ask various informations to the plugin.        */
/*****************************************************************************/
MODULEAPI CFParm * getPluginProperty(CFParm *PParm)
{
    double  dblval  = 0.0;
    //    int i;
    if (PParm != NULL)
    {
        if (PParm->Value[0] && !strcmp((char *) (PParm->Value[0]), "command?"))
        {
            if (PParm->Value[1] && !strcmp((char *) (PParm->Value[1]), PLUGIN_NAME))
            {
                GCFP.Value[0] = PParm->Value[1];
                GCFP.Value[1] = &cmd_aboutLua;
                GCFP.Value[2] = &dblval;
                return &GCFP;
            }
            else
            {
#if 0
                for (i=0;i<NR_CUSTOM_CMD;i++)
                {
                    if (CustomCommand[i].name)
                    {
                        if (!strcmp(CustomCommand[i].name,(char *)(PParm->Value[1])))
                        {
                            LOG(llevDebug, "LUA - Running command %s\n",CustomCommand[i].name);
                            GCFP.Value[0] = PParm->Value[1];
                            GCFP.Value[1] = cmd_customPython;
                            GCFP.Value[2] = &(CustomCommand[i].speed);
                            NextCustomCommand = i;
                            return &GCFP;
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
    return NULL;
}

#if 0
MODULEAPI int cmd_customPython(object *op, char *params)
{
#ifdef PYTHON_DEBUG
    LOG(llevDebug, "PYTHON - cmd_customPython called:: script file: %s\n",CustomCommand[NextCustomCommand].script);
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

    RunPythonScript(CustomCommand[NextCustomCommand].script);

    return StackReturn[StackPosition--];
}
#endif

MODULEAPI int cmd_aboutLua(object *op, char *params)
{
    /* ehm... a map info version msg??? should be drawinfo, or missed i something? MT-18.02.04*/
    /*
       int color = NDI_BLUE|NDI_UNIQUE;
       char message[1024];
       sprintf(message,"%s (Pegasus)\n(C)2001 by Gros. The Plugin code is under GPL.",PLUGIN_VERSION);
       GCFP.Value[0] = (void *)(&color);
       GCFP.Value[1] = (void *)(op->map);
       GCFP.Value[2] = (void *)(message);
       (PlugHooks[HOOK_NEWINFOMAP])(&GCFP);
    */
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
    /* see how useful they could be for the Python stuff.*/
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
    struct timeval  new_time;
    (void) GETTIMEOFDAY(&new_time);

    LOG(llevDebug, "LUA - Start postinitPlugin.\n");

    /*    GCFP.Value[1] = (void *)(add_string_hook(PLUGIN_NAME));*/
    GCFP.Value[1] = (void *) PLUGIN_NAME;
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
/* Initializes the Python Interpreter.                                       */
/*****************************************************************************/
MODULEAPI void init_Daimonin_Lua()
{
    int     res;
    char    lua_path[MAX_BUF];
    char   *map_path;

    strcpy(lua_path, hooks->create_pathname(LUA_PATH));
    map_path = hooks->create_pathname("");

    global_state = lua_open();

    /* Initialize the libs */
    luaopen_base(global_state);
    luaopen_string(global_state);
    luaopen_table(global_state);
    luaopen_math(global_state);
    luaopen_io(global_state);

    /* Initialize the classes */
    init_class(global_state, &Event);
    init_class(global_state, &Game);
    GameObject_init(global_state);
    Map_init(global_state);

    /* Set up the global Game object  */
    lua_pushliteral(global_state, "game");
    push_object(global_state, &Game, "Daimonin");
    lua_rawset(global_state, LUA_GLOBALSINDEX);

    /* Set up module search path; prefer compiled files */
    lua_pushliteral(global_state, "LUA_PATH");
    lua_pushfstring(global_state, "%s?;%s?.lc;%s?.lua;%s/?;%s/?.lc;%s/?.lua;?;?.lc;?.lua", map_path, map_path, map_path,
                    lua_path, lua_path, lua_path);
    lua_rawset(global_state, LUA_GLOBALSINDEX);

    init_file_cache(global_state);

    /* Call the initialization script */
    res = luaL_loadfile(global_state, hooks->create_pathname(LUA_INITSCRIPT));
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
