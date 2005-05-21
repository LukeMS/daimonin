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
#include <daimonin_game.h>

/*
 * - Game contains the old Daimonin functions and constants
 *   that don't fit into objects
 */

static struct method_decl       Game_methods[]      =
{
    {"TransferMapItems", Game_TransferMapItems}, 
    {"LoadObject", Game_LoadObject},
    {"ReadyMap", Game_ReadyMap}, 
    {"CheckMap", Game_CheckMap}, 
    {"MatchString", Game_MatchString},
    {"FindPlayer", Game_FindPlayer}, 
    {"GetSpellNr", Game_GetSpellNr},
    {"GetSkillNr", Game_GetSkillNr}, 
    {"IsValid", Game_IsValid},
    {"GetTime", Game_GetTime},
    //    {"RegisterCommand", Game_RegisterCommand},
    {NULL, NULL}
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
    {"IDENTIFY_MARKED", 2}, {"CLONE_WITH_INVENTORY", 0}, {"CLONE_WITHOUT_INVENTORY", 1}, {"EXP_AGILITY", 0},
    {"EXP_PERSONAL", 1}, {"EXP_MENTAL", 2}, {"EXP_PHYSICAL", 3}, {"EXP_MAGICAL", 4}, {"EXP_WISDOM", 5},
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
    {"TYPE_SPAWN_POINT"             ,SPAWN_POINT}, {"TYPE_LIGHT_REFILL"            ,TYPE_LIGHT_REFILL},
    {"TYPE_SPAWN_POINT_MOB"         ,SPAWN_POINT_MOB }, {"TYPE_SPAWN_POINT_INFO"        ,SPAWN_POINT_INFO},
    {"TYPE_SPELLBOOK"               ,SPELLBOOK}, {"TYPE_ORGANIC"             ,ORGANIC},
    {"TYPE_CLOAK"                   ,CLOAK}, {"TYPE_CONE"                    ,CONE},
    {"TYPE_AURA"                    ,AURA}, {"TYPE_SPINNER"             ,SPINNER},
    {"TYPE_GATE"                    ,GATE}, {"TYPE_BUTTON"                  ,BUTTON},
    {"TYPE_CF_HANDLE"               ,CF_HANDLE}, {"TYPE_PIT"                 ,PIT},
    {"TYPE_TRAPDOOR"                ,TRAPDOOR}, {"TYPE_WORD_OF_RECALL"          ,WORD_OF_RECALL},
    {"TYPE_PARAIMAGE"               ,PARAIMAGE}, {"TYPE_SIGN"                    ,SIGN},
    {"TYPE_BOOTS"                   ,BOOTS}, {"TYPE_GLOVES"                  ,GLOVES},
    {"TYPE_BASE_INFO"               ,TYPE_BASE_INFO}, {"TYPE_RANDOM_DROP"     ,TYPE_RANDOM_DROP},
    {"TYPE_CONVERTER"               ,CONVERTER}, {"TYPE_BRACERS"             ,BRACERS},
    {"TYPE_POISONING"               ,POISONING}, {"TYPE_SAVEBED"             ,SAVEBED},
    {"TYPE_POISONCLOUD"             ,POISONCLOUD}, {"TYPE_FIREHOLES"               ,FIREHOLES},
    {"TYPE_WAND"                    ,WAND}, {"TYPE_ABILITY"             ,ABILITY},
    {"TYPE_SCROLL"                  ,SCROLL}, {"TYPE_DIRECTOR"                ,DIRECTOR},
    {"TYPE_GIRDLE"                  ,GIRDLE}, {"TYPE_FORCE"                   ,FORCE},
    {"TYPE_POTION_EFFECT"           ,POTION_EFFECT}, {"TYPE_JEWEL"               ,TYPE_JEWEL},
    {"TYPE_NUGGET"                  ,TYPE_NUGGET}, {"TYPE_EVENT_OBJECT"        ,TYPE_EVENT_OBJECT},
    {"TYPE_WAYPOINT_OBJECT"         ,TYPE_WAYPOINT_OBJECT}, {"TYPE_QUEST_CONTAINER" ,TYPE_QUEST_CONTAINER},
    {"TYPE_CLOSE_CON"               ,CLOSE_CON}, {"TYPE_CONTAINER"               ,CONTAINER},
    {"TYPE_ARMOUR_IMPROVER"         ,ARMOUR_IMPROVER}, {"TYPE_WEAPON_IMPROVER"        ,WEAPON_IMPROVER},
    {"TYPE_WEALTH"                  ,TYPE_WEALTH}, {"TYPE_AI"                   ,TYPE_AI},
    {"TYPE_AGGRO_HISTORY"           ,TYPE_AGGRO_HISTORY}, {"TYPE_DAMAGE_INFO"            ,TYPE_DAMAGE_INFO},
    {"TYPE_SKILLSCROLL"             ,SKILLSCROLL}, {"TYPE_DEEP_SWAMP"              ,DEEP_SWAMP},
    {"TYPE_IDENTIFY_ALTAR"          ,IDENTIFY_ALTAR}, {"TYPE_CANCELLATION"            ,CANCELLATION},
    {"TYPE_MENU"                    ,MENU}, {"TYPE_BALL_LIGHTNING"         ,BALL_LIGHTNING},
    {"TYPE_SWARM_SPELL"             ,SWARM_SPELL}, {"TYPE_RUNE"                   ,RUNE},
    {"TYPE_POWER_CRYSTAL"           ,POWER_CRYSTAL,}, {"TYPE_CORPSE"                 ,CORPSE},
    {"TYPE_DISEASE"                 ,DISEASE}, {"TYPE_SYMPTOM"                ,SYMPTOM},
	{"TYPE_QUEST_TRIGGER"                ,TYPE_QUEST_TRIGGER},
	{"TYPE_QUEST_OBJECT"                ,TYPE_QUEST_OBJECT},
    {NULL, 0}
};

lua_class Game =
{
    LUATYPE_GAME, "Game", 0, NULL, NULL, Game_methods, Game_constants
};

/****************************************************************************/
/*                                                                          */
/*                          Game module functions                           */
/*                                                                          */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : Game_TransferMapItems                                            */
/* Lua    : game:TransferMapItems(map_old, map_new, x, y)                     */
/* Info   : Transfer all items with "no_pick 0" setting from map_old         */
/*          to position x,y on map new.                                      */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Game_TransferMapItems(lua_State *L)
{
    lua_object *map_new, *map_old, *self;
    int         x, y;

    get_lua_args(L, "GMMii", &self, &map_old, &map_new, &x, &y);

    GCFP.Value[0] = (void *) (map_old->data.map);
    GCFP.Value[1] = (void *) (map_new->data.map);
    GCFP.Value[2] = (void *) (&x);
    GCFP.Value[3] = (void *) (&y);

    (PlugHooks[HOOK_MAPTRANSERITEMS]) (&GCFP);

    return 0;
}


/*****************************************************************************/
/* Name   : Game_LoadObject                                                  */
/* Lua    : game:LoadObject(string)                                          */
/* Status : Untested                                                         */
/*****************************************************************************/
static int Game_LoadObject(lua_State *L)
{
    object *whoptr;
    char   *dumpob;
    CFParm *CFR;
    lua_object *self;

    get_lua_args(L, "Gs", &self, &dumpob);

    /* First step: We create the object */
    GCFP.Value[0] = (void *) (dumpob);
    CFR = (PlugHooks[HOOK_LOADOBJECT]) (&GCFP);
    whoptr = (object *) (CFR->Value[0]);
    free(CFR);

    return push_object(L, &GameObject, whoptr);
}

/*****************************************************************************/
/* Name   : Game_MatchString                                                 */
/* Lua    : game:MatchString(firststr, secondstr)                            */
/* Info   : Case insensitive string comparision. Returns 1 if the two        */
/*          strings are the same, or 0 if they differ.                       */
/*          secondstring can contain regular expressions.                    */
/* Status : Stable                                                           */
/*****************************************************************************/
static int Game_MatchString(lua_State *L)
{
    char   *premiere;
    char   *seconde;
    lua_object *self;

    get_lua_args(L, "Gss", &self, &premiere, &seconde);

    lua_pushboolean(L, (hooks->re_cmp(premiere, seconde) != NULL));
    return 1;
}

/*****************************************************************************/
/* Name   : Game_ReadyMap                                                    */
/* Lua    : game:ReadyMap(name, unique)                                      */
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
    lua_object *self;

    get_lua_args(L, "Gsi|O", &self, &mapname, &flags, &obptr);

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
/* Lua    : game:CheckMap(arch, map_path, x, y)                              */
/* Info   :                                                                  */
/* Status : Unfinished. DO NOT USE!                                          */
/*****************************************************************************/

static int Game_CheckMap(lua_State *L)
{
    char   *what;
    char   *mapstr;
    int     x, y;
    lua_object *self;
    /*    object* foundob; */

    /* Gecko: replaced coordinate tuple with separate x and y coordinates */
    get_lua_args(L, "Gssii", &self, &what, &mapstr, &x, &y);

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
/* Lua    : game:FindPlayer(name)                                            */
/* Status : Tested                                                           */
/*****************************************************************************/

static int Game_FindPlayer(lua_State *L)
{
    player *foundpl;
    object *foundob = NULL;
    CFParm *CFR;
    char   *txt;
    lua_object *self;

    get_lua_args(L, "Gs", &self, &txt);

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
/* Lua    : game:GetSpellNr(name)                                            */
/* Info   : Gets the number of the named spell. -1 if no such spell exists   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int Game_GetSpellNr(lua_State *L)
{
    char   *spell;
    lua_object *self;

    get_lua_args(L, "Gs", &self, &spell);

    lua_pushnumber(L, hooks->look_up_spell_name(spell));
    return 1;
}

/*****************************************************************************/
/* Name   : Game_GetSkillNr                                                  */
/* Lua    : game:GetSkillNr(name)                                            */
/* Info   : Gets the number of the named skill. -1 if no such skill exists   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int Game_GetSkillNr(lua_State *L)
{
    char   *skill;
    lua_object *self;

    get_lua_args(L, "Gs", &self, &skill);

    lua_pushnumber(L, hooks->lookup_skill_by_name(skill));
    return 1;
}

/*****************************************************************************/
/* Name   : Game_IsValid                                                     */
/* Lua    : game:IsValid(what)                                               */
/* Info   : Test if a Map, Event or GameObject is still valid.               */
/*          (Useful for datastore and coroutine usage).                      */
/*          This is the only lua function that doesn't generate an error if  */
/*          given an invalid object.                                         */
/* Status : Tested                                                           */
/*****************************************************************************/
static int Game_IsValid(lua_State *L)
{
    lua_object *obj;

    if(lua_gettop(L) != 2)
        luaL_error(L, "wrong number of arguments to game:IsValid() (2 expected, got %d)", lua_gettop(L));

    if(lua_isnil(L, -1))
    {
        /* Nil is never valid */
        lua_pushboolean(L, 0);
    }
    else if(! (obj = lua_touserdata(L, -1)))
    {
        luaL_error(L, "parameter is not nil or an object");
    }
    else
    {
       lua_pushboolean(L, obj->class->isValid(L, obj));
    }

    return 1;
}

/*****************************************************************************/
/* Name   : Game_GetTime                                                     */
/* Lua    : game:GetTime()                                                   */
/* Info   : Return a table with values on the current game time.             */
/*          The table will have the following fields:                        */
/*          year - year number                                               */
/*          month - month number                                             */
/*          day - day number in month                                        */
/*          dayofweek - day number in week                                   */
/*          hour - current time                                              */
/*          minute - current time                                            */
/*          weekofmonth - week in month                                      */
/*          season - season number                                           */
/*          dayofweek_name - weekday as string                               */
/*          month_name - month as string                                     */
/*          season_name - season as string                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
static int Game_GetTime(lua_State *L)
{
    timeofday_t tod;
    lua_object *self;

    get_lua_args(L, "G", &self);

    hooks->get_tod(&tod);

    lua_newtable(L);
    
    lua_pushliteral(L, "year");
    lua_pushnumber(L, (lua_Number) tod.year);
    lua_rawset(L, -3);
    lua_pushliteral(L, "month");
    lua_pushnumber(L, (lua_Number) tod.month);
    lua_rawset(L, -3);
    lua_pushliteral(L, "day");
    lua_pushnumber(L, (lua_Number) tod.day);
    lua_rawset(L, -3);
    lua_pushliteral(L, "dayofweek");
    lua_pushnumber(L, (lua_Number) tod.dayofweek);
    lua_rawset(L, -3);
    lua_pushliteral(L, "hour");
    lua_pushnumber(L, (lua_Number) tod.hour);
    lua_rawset(L, -3);
    lua_pushliteral(L, "minute");
    lua_pushnumber(L, (lua_Number) tod.minute);
    lua_rawset(L, -3);
    lua_pushliteral(L, "weekofmonth");
    lua_pushnumber(L, (lua_Number) tod.weekofmonth);
    lua_rawset(L, -3);
    lua_pushliteral(L, "season");
    lua_pushnumber(L, (lua_Number) tod.season);
    lua_rawset(L, -3);
    
    lua_pushliteral(L, "dayofweek_name");
    lua_pushstring(L, tod.dayofweek_name);
    lua_rawset(L, -3);
    lua_pushliteral(L, "month_name");
    lua_pushstring(L, tod.month_name);
    lua_rawset(L, -3);
    lua_pushliteral(L, "season_name");
    lua_pushstring(L, tod.season_name);
    lua_rawset(L, -3);
    
    return 1;
}

/* FUNCTIONEND -- End of the Lua plugin functions. */

/*
 * Class & Object handling
 */

int Game_init(lua_State *L)
{
    init_class(L, &Game);

    return 0;
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
