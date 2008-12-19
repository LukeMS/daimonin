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

static struct method_decl Game_methods[] =
{
    {"EnumerateCoins",   Game_EnumerateCoins},
    {"FindPlayer",       Game_FindPlayer},
    {"GetSkillNr",       Game_GetSkillNr},
    {"GetSpellNr",       Game_GetSpellNr},
    {"GetTime",          Game_GetTime},
    {"IsValid",          Game_IsValid},
    {"LoadObject",       Game_LoadObject},
    {"LocateBeacon",     Game_LocateBeacon},
    {"Log",              Game_Log},
    {"MatchString",      Game_MatchString},
    {"ReadyMap",         Game_ReadyMap},
/*  {"RegisterCommand",  Game_RegisterCommand}, */
    {"UpgradeApartment", Game_UpgradeApartment},

    {NULL, NULL}
};

static struct constant_decl Game_constants[] =
{
    /* directions (constants not defined by server) */
    {"NORTH",     1},
    {"NORTHEAST", 2},
    {"EAST",      3},
    {"SOUTHEAST", 4},
    {"SOUTH",     5},
    {"SOUTHWEST", 6},
    {"WEST",      7},
    {"NORTHWEST", 8},

    /* map info modes (global.h) */
    {"MAP_INFO_NORMAL", MAP_INFO_NORMAL},
    {"MAP_INFO_ALL",    MAP_INFO_ALL},

    /* costs by transaction (define.h) */
    {"COST_TRUE", F_TRUE},
    {"COST_BUY",  F_BUY},
    {"COST_SELL", F_SELL},

    /* apply modes (define.h) */
    {"APPLY_TOGGLE",         AP_NULL},
    {"APPLY_ALWAYS",         AP_APPLY},
    {"UNAPPLY_ALWAYS",       AP_UNAPPLY},
    {"UNAPPLY_NO_MERGE",     AP_NO_MERGE},
    {"UNAPPLY_IGNORE_CURSE", AP_IGNORE_CURSE},

    /* genders (constants not defined by server) */
    {"NEUTER",        0},
    {"MALE",          1},
    {"FEMALE",        2},
    {"HERMAPHRODITE", 3},

    /* spell delivery modes (based on spellEnumerate in spells.h but we cheat for simplicity) */
    {"CAST_NORMAL", 0},
    {"CAST_POTION", 1},

    /* skill/spell learning modes (constants not defined by server) */
    {"LEARN",   0},
    {"UNLEARN", 1},

    /* identification status (constants not defined by server) */
    {"UNIDENTIFIED", 0},
    {"IDENTIFIED",   1},

    /* identification modes (define.h) */
    {"IDENTIFY_NORMAL", IDENTIFY_MODE_NORMAL},
    {"IDENTIFY_ALL",    IDENTIFY_MODE_ALL},
    {"IDENTIFY_MARKED", IDENTIFY_MODE_MARKED},

    /* cloning modes (constants not defined by server) */
    {"CLONE_WITH_INVENTORY",    0},
    {"CLONE_WITHOUT_INVENTORY", 1},

    /* experience categories (deprecate in favour of SKILLGROUP_*?) */
    {"EXP_AGILITY",  0},
    {"EXP_PERSONAL", 1},
    {"EXP_MENTAL",   2},
    {"EXP_PHYSICAL", 3},
    {"EXP_MAGICAL",  4},
    {"EXP_WISDOM",   5},

    /* colours (newclient.h) */
    {"COLOR_WHITE",   NDI_WHITE},
    {"COLOR_ORANGE",  NDI_ORANGE},
    {"COLOR_NAVY",    NDI_NAVY},
    {"COLOR_RED",     NDI_RED},
    {"COLOR_GREEN",   NDI_GREEN},
    {"COLOR_BLUE",    NDI_BLUE},
    {"COLOR_GREY",    NDI_GREY},
    {"COLOR_BROWN",   NDI_BROWN},
    {"COLOR_PURPLE",  NDI_PURPLE},
    {"COLOR_FLESH",   NDI_FLESH},
    {"COLOR_YELLOW",  NDI_YELLOW},
    {"COLOR_DK_NAVY", NDI_DK_NAVY},

    /* message modes (newclient.h) */
    {"VIM_MSG",      NDI_VIM},
    {"UNIQUE_MSG",   NDI_UNIQUE},

    /* equipment slots of the player->equipment array (player.h) */
    {"EQUIP_MAIL",     PLAYER_EQUIP_MAIL},
    {"EQUIP_GAUNTLET", PLAYER_EQUIP_GAUNTLET},
    {"EQUIP_BRACER",   PLAYER_EQUIP_BRACER},
    {"EQUIP_HELM",     PLAYER_EQUIP_HELM},
    {"EQUIP_SHOULDER", PLAYER_EQUIP_SHOULDER},
    {"EQUIP_LEGS",     PLAYER_EQUIP_LEGS},
    {"EQUIP_BOOTS",    PLAYER_EQUIP_BOOTS},
    {"EQUIP_CLOAK",    PLAYER_EQUIP_CLOAK},
    {"EQUIP_GIRDLE",   PLAYER_EQUIP_GIRDLE},
    {"EQUIP_SHIELD",   PLAYER_EQUIP_SHIELD},
    {"EQUIP_RRING",    PLAYER_EQUIP_RRING},
    {"EQUIP_LRING",    PLAYER_EQUIP_LRING},
    {"EQUIP_AMULET",   PLAYER_EQUIP_AMULET},
    {"EQUIP_WEAPON1",  PLAYER_EQUIP_WEAPON1},
    {"EQUIP_BOW",      PLAYER_EQUIP_BOW},
    {"EQUIP_AMUN",     PLAYER_EQUIP_AMUN},
    {"EQUIP_MAX",      PLAYER_EQUIP_MAX},

    /* skill groups used by quests (constants not define by server) */
    /* why not just use game.SKILLGROUP_* and add 1 in the relevant functions? */
    /* this would be invisible to scripters and reduce the number of constants used */
    {"ITEM_SKILL_NO",          0},
    {"ITEM_SKILL_AGILITY",     1},
    {"ITEM_SKILL_PERSONALITY", 2},
    {"ITEM_SKILL_MENTAL",      3},
    {"ITEM_SKILL_PHYSICAL",    4},
    {"ITEM_SKILL_MAGIC",       5},
    {"ITEM_SKILL_WISDOM",      6},

    /* map loading modes (plugin_lua/include/plugin_lua.h) */
    {"MAP_CHECK", PLUGIN_MAP_CHECK},
    {"MAP_NEW",   PLUGIN_MAP_NEW},

    /* map status flags (also some are used by GameObject:SetPosition()) (map.h) */
    {"INSTANCE_NO_REENTER", INSTANCE_FLAG_NO_REENTER},
    {"MFLAG_FIXED_POS",     MAP_STATUS_FIXED_POS},
    {"MFLAG_RANDOM_POS",    MAP_STATUS_RANDOM_POS},
    {"MFLAG_FREE_POS_ONLY", MAP_STATUS_FREE_POS_ONLY},
    {"MFLAG_NO_FALLBACK",   MAP_STATUS_NO_FALLBACK },
    {"MFLAG_LOAD_ONLY",     MAP_STATUS_LOAD_ONLY},
    {"MFLAG_FIXED_LOGIN",   MAP_STATUS_FIXED_LOGIN},

    /* quest trigger sub types (define.h) */
    {"QUEST_NORMAL",   ST1_QUEST_TRIGGER_NORMAL},
    {"QUEST_KILL",     ST1_QUEST_TRIGGER_KILL},
    {"QUEST_KILLITEM", ST1_QUEST_TRIGGER_KILL_ITEM},
    {"QUEST_ITEM",     ST1_QUEST_TRIGGER_ITEM},

    /* quest steps (constants not defined by server) */
    {"QSTAT_NO",       1},
    {"QSTAT_ACTIVE",   2},
    {"QSTAT_SOLVED",   3},
    {"QSTAT_DONE",     4},
    {"QSTAT_DISALLOW", 5},

    /* guild force sub types (define.h) */
    {"GUILD_NO",  0},
    {"GUILD_IN",  ST1_GUILD_IN},
    {"GUILD_OLD", ST1_GUILD_OLD},

    /* soundtypes (sounds.h) */
    {"SOUNDTYPE_NORMAL", SOUND_NORMAL},
    {"SOUNDTYPE_SPELL",  SOUND_SPELL},

    /* normal sounds (sounds.h) */
    {"SOUND_LEVEL_UP",       SOUND_LEVEL_UP},
    {"SOUND_FIRE_ARROW",     SOUND_FIRE_ARROW},
    {"SOUND_LEARN_SPELL",    SOUND_LEARN_SPELL},
    {"SOUND_FUMBLE_SPELL",   SOUND_FUMBLE_SPELL},
    {"SOUND_WAND_POOF",      SOUND_WAND_POOF},
    {"SOUND_OPEN_DOOR",      SOUND_OPEN_DOOR},
    {"SOUND_PUSH_PLAYER",    SOUND_PUSH_PLAYER},
    {"SOUND_HIT_IMPACT",     SOUND_HIT_IMPACT},
    {"SOUND_HIT_CLEAVE",     SOUND_HIT_CLEAVE},
    {"SOUND_HIT_SLASH",      SOUND_HIT_SLASH},
    {"SOUND_HIT_PIERCE",     SOUND_HIT_PIERCE},
    {"SOUND_MISS_BLOCK",     SOUND_MISS_BLOCK},
    {"SOUND_MISS_HAND",      SOUND_MISS_HAND},
    {"SOUND_MISS_MOB",       SOUND_MISS_MOB},
    {"SOUND_MISS_PLAYER",    SOUND_MISS_PLAYER},
    {"SOUND_PET_IS_KILLED",  SOUND_PET_IS_KILLED},
    {"SOUND_PLAYER_DIES",    SOUND_PLAYER_DIES},
    {"SOUND_OB_EVAPORATE",   SOUND_OB_EVAPORATE},
    {"SOUND_OB_EXPLODE",     SOUND_OB_EXPLODE},
    {"SOUND_PLAYER_KILLS",   SOUND_PLAYER_KILLS},
    {"SOUND_TURN_HANDLE",    SOUND_TURN_HANDLE},
    {"SOUND_FALL_HOLE",      SOUND_FALL_HOLE},
    {"SOUND_DRINK_POISON",   SOUND_DRINK_POISON},
    {"SOUND_DROP_THROW",     SOUND_DROP_THROW},
    {"SOUND_LOSE_SOME",      SOUND_LOSE_SOME},
    {"SOUND_THROW",          SOUND_THROW},
    {"SOUND_GATE_OPEN",      SOUND_GATE_OPEN},
    {"SOUND_GATE_CLOSE",     SOUND_GATE_CLOSE},
    {"SOUND_OPEN_CONTAINER", SOUND_OPEN_CONTAINER},
    {"SOUND_GROWL",          SOUND_GROWL},
    {"SOUND_ARROW_HIT",      SOUND_ARROW_HIT},
    {"SOUND_DOOR_CLOSE",     SOUND_DOOR_CLOSE},
    {"SOUND_TELEPORT",       SOUND_TELEPORT},
    {"SOUND_CLICK",          SOUND_CLICK},

    /* spell sounds (sounds.h) */
    {"SOUND_MAGIC_DEFAULT",     SOUND_MAGIC_DEFAULT},
    {"SOUND_MAGIC_ACID",        SOUND_MAGIC_ACID},
    {"SOUND_MAGIC_ANIMATE",     SOUND_MAGIC_ANIMATE},
    {"SOUND_MAGIC_AVATAR",      SOUND_MAGIC_AVATAR},
    {"SOUND_MAGIC_BOMB",        SOUND_MAGIC_BOMB},
    {"SOUND_MAGIC_BULLET1",     SOUND_MAGIC_BULLET1},
    {"SOUND_MAGIC_BULLET2",     SOUND_MAGIC_BULLET2},
    {"SOUND_MAGIC_CANCEL",      SOUND_MAGIC_CANCEL},
    {"SOUND_MAGIC_COMET",       SOUND_MAGIC_COMET},
    {"SOUND_MAGIC_CONFUSION",   SOUND_MAGIC_CONFUSION},
    {"SOUND_MAGIC_CREATE",      SOUND_MAGIC_CREATE},
    {"SOUND_MAGIC_DARK",        SOUND_MAGIC_DARK},
    {"SOUND_MAGIC_DEATH",       SOUND_MAGIC_DEATH},
    {"SOUND_MAGIC_DESTRUCTION", SOUND_MAGIC_DESTRUCTION},
    {"SOUND_MAGIC_ELEC",        SOUND_MAGIC_ELEC},
    {"SOUND_MAGIC_FEAR",        SOUND_MAGIC_FEAR},
    {"SOUND_MAGIC_FIRE",        SOUND_MAGIC_FIRE},
    {"SOUND_MAGIC_FIREBALL1",   SOUND_MAGIC_FIREBALL1},
    {"SOUND_MAGIC_FIREBALL2",   SOUND_MAGIC_FIREBALL2},
    {"SOUND_MAGIC_HWORD",       SOUND_MAGIC_HWORD},
    {"SOUND_MAGIC_ICE",         SOUND_MAGIC_ICE},
    {"SOUND_MAGIC_INVISIBLE",   SOUND_MAGIC_INVISIBLE},
    {"SOUND_MAGIC_INVOKE",      SOUND_MAGIC_INVOKE},
    {"SOUND_MAGIC_INVOKE2",     SOUND_MAGIC_INVOKE2},
    {"SOUND_MAGIC_MAGIC",       SOUND_MAGIC_MAGIC},
    {"SOUND_MAGIC_MANABALL",    SOUND_MAGIC_MANABALL},
    {"SOUND_MAGIC_MISSILE",     SOUND_MAGIC_MISSILE},
    {"SOUND_MAGIC_MMAP",        SOUND_MAGIC_MMAP},
    {"SOUND_MAGIC_ORB",         SOUND_MAGIC_ORB},
    {"SOUND_MAGIC_PARALYZE",    SOUND_MAGIC_PARALYZE},
    {"SOUND_MAGIC_POISON",      SOUND_MAGIC_POISON},
    {"SOUND_MAGIC_PROTECTION",  SOUND_MAGIC_PROTECTION},
    {"SOUND_MAGIC_RSTRIKE",     SOUND_MAGIC_RSTRIKE},
    {"SOUND_MAGIC_RUNE",        SOUND_MAGIC_RUNE},
    {"SOUND_MAGIC_SBALL",       SOUND_MAGIC_SBALL},
    {"SOUND_MAGIC_SLOW",        SOUND_MAGIC_SLOW},
    {"SOUND_MAGIC_SNOWSTORM",   SOUND_MAGIC_SNOWSTORM},
    {"SOUND_MAGIC_STAT",        SOUND_MAGIC_STAT},
    {"SOUND_MAGIC_STEAMBOLT",   SOUND_MAGIC_STEAMBOLT},
    {"SOUND_MAGIC_SUMMON1",     SOUND_MAGIC_SUMMON1},
    {"SOUND_MAGIC_SUMMON2",     SOUND_MAGIC_SUMMON2},
    {"SOUND_MAGIC_SUMMON3",     SOUND_MAGIC_SUMMON3},
    {"SOUND_MAGIC_TELEPORT",    SOUND_MAGIC_TELEPORT},
    {"SOUND_MAGIC_TURN",        SOUND_MAGIC_TURN},
    {"SOUND_MAGIC_WALL",        SOUND_MAGIC_WALL},
    {"SOUND_MAGIC_WALL2",       SOUND_MAGIC_WALL2},
    {"SOUND_MAGIC_WOUND",       SOUND_MAGIC_WOUND},

    /* Argh, the object types. Make sure to keep up-to date if any are added/removed (define.h) */
    {"TYPE_PLAYER",           PLAYER},
    {"TYPE_BULLET",           BULLET},
    {"TYPE_ROD",              ROD},
    {"TYPE_TREASURE",         TREASURE},
    {"TYPE_POTION",           POTION},
    {"TYPE_FOOD",             FOOD},
    {"TYPE_POISON",           POISON},
    {"TYPE_BOOK",             BOOK},
    {"TYPE_CLOCK",            CLOCK},
    {"TYPE_FBULLET",          FBULLET},
    {"TYPE_FBALL",            FBALL},
    {"TYPE_LIGHTNING",        LIGHTNING},
    {"TYPE_ARROW",            ARROW},
    {"TYPE_BOW",              BOW},
    {"TYPE_WEAPON",           WEAPON},
    {"TYPE_ARMOUR",           ARMOUR},
    {"TYPE_PEDESTAL",         PEDESTAL},
    {"TYPE_ALTAR",            ALTAR},
    {"TYPE_LOCKED_DOOR",      LOCKED_DOOR},
    {"TYPE_SPECIAL_KEY",      SPECIAL_KEY},
    {"TYPE_MAP",              MAP},
    {"TYPE_DOOR",             DOOR},
    {"TYPE_KEY",              KEY},
    {"TYPE_MMISSILE",         MMISSILE},
    {"TYPE_TIMED_GATE",       TIMED_GATE},
    {"TYPE_TRIGGER",          TRIGGER},
    {"TYPE_MAGIC_EAR",        MAGIC_EAR},
    {"TYPE_TRIGGER_BUTTON",   TRIGGER_BUTTON},
    {"TYPE_TRIGGER_ALTAR",    TRIGGER_ALTAR},
    {"TYPE_TRIGGER_PEDESTAL", TRIGGER_PEDESTAL},
    {"TYPE_SHIELD",           SHIELD},
    {"TYPE_HELMET",           HELMET},
    {"TYPE_SHOULDER",         SHOULDER},
    {"TYPE_LEGS",             LEGS},
    {"TYPE_HORN",             HORN},
    {"TYPE_MONEY",            MONEY},
    {"TYPE_CLASS",            CLASS},
    {"TYPE_GRAVESTONE",       GRAVESTONE},
    {"TYPE_AMULET",           AMULET},
    {"TYPE_PLAYERMOVER",      PLAYERMOVER},
    {"TYPE_TELEPORTER",       TELEPORTER},
    {"TYPE_CREATOR",          CREATOR},
    {"TYPE_SKILL",            SKILL},
    {"TYPE_EXPERIENCE",       EXPERIENCE},
    {"TYPE_EARTHWALL",        EARTHWALL},
    {"TYPE_GOLEM",            GOLEM},
    {"TYPE_BOMB",             BOMB},
    {"TYPE_THROWN_OBJ",       THROWN_OBJ},
    {"TYPE_BLINDNESS",        BLINDNESS},
    {"TYPE_GOD",              GOD},
    {"TYPE_DETECTOR",         DETECTOR},
    {"TYPE_SPEEDBALL",        SPEEDBALL},
    {"TYPE_DEAD_OBJECT",      DEAD_OBJECT},
    {"TYPE_DRINK",            DRINK},
    {"TYPE_MARKER",           MARKER},
    {"TYPE_HOLY_ALTAR",       HOLY_ALTAR},
    {"TYPE_PLAYER_CHANGER",   PLAYER_CHANGER},
    {"TYPE_BATTLEGROUND",     BATTLEGROUND},
    {"TYPE_PEACEMAKER",       PEACEMAKER},
    {"TYPE_GEM",              GEM},
    {"TYPE_FIRECHEST",        FIRECHEST},
    {"TYPE_FIREWALL",         FIREWALL},
    {"TYPE_ANVIL",            ANVIL},
    {"TYPE_CHECK_INV",        CHECK_INV},
    {"TYPE_MOOD_FLOOR",       MOOD_FLOOR},
    {"TYPE_EXIT",             EXIT},
    {"TYPE_AGE_FORCE",        TYPE_AGE_FORCE},
    {"TYPE_SHOP_FLOOR",       SHOP_FLOOR},
    {"TYPE_SHOP_MAT",         SHOP_MAT},
    {"TYPE_RING",             RING},
    {"TYPE_FLOOR",            FLOOR},
    {"TYPE_FLESH",            FLESH},
    {"TYPE_INORGANIC",        INORGANIC},
    {"TYPE_LIGHT_APPLY",      TYPE_LIGHT_APPLY },
    {"TYPE_LIGHTER",          LIGHTER},
    {"TYPE_TRAP_PART",        TRAP_PART},
    {"TYPE_WALL",             WALL},
    {"TYPE_LIGHT_SOURCE",     LIGHT_SOURCE},
    {"TYPE_MISC_OBJECT",      MISC_OBJECT},
    {"TYPE_MONSTER",          MONSTER},
    {"TYPE_SPAWN_POINT",      SPAWN_POINT},
    {"TYPE_LIGHT_REFILL",     TYPE_LIGHT_REFILL},
    {"TYPE_SPAWN_POINT_MOB",  SPAWN_POINT_MOB },
    {"TYPE_SPAWN_POINT_INFO", SPAWN_POINT_INFO},
    {"TYPE_SPELLBOOK",        SPELLBOOK},
    {"TYPE_ORGANIC",          ORGANIC},
    {"TYPE_CLOAK",            CLOAK},
    {"TYPE_CONE",             CONE},
    {"TYPE_AURA",             AURA},
    {"TYPE_SPINNER",          SPINNER},
    {"TYPE_GATE",             GATE},
    {"TYPE_BUTTON",           BUTTON},
    {"TYPE_CF_HANDLE",        CF_HANDLE},
    {"TYPE_PIT",              PIT},
    {"TYPE_TRAPDOOR",         TRAPDOOR},
    {"TYPE_WORD_OF_RECALL",   WORD_OF_RECALL},
    {"TYPE_PARAIMAGE",        PARAIMAGE},
    {"TYPE_SIGN",             SIGN},
    {"TYPE_BOOTS",            BOOTS},
    {"TYPE_GLOVES",           GLOVES},
    {"TYPE_BASE_INFO",        TYPE_BASE_INFO},
    {"TYPE_RANDOM_DROP",      TYPE_RANDOM_DROP},
    {"TYPE_CONVERTER",        CONVERTER},
    {"TYPE_BRACERS",          BRACERS},
    {"TYPE_POISONING",        POISONING},
    {"TYPE_SAVEBED",          SAVEBED},
    {"TYPE_POISONCLOUD",      POISONCLOUD},
    {"TYPE_FIREHOLES",        FIREHOLES},
    {"TYPE_WAND",             WAND},
    {"TYPE_ABILITY",          ABILITY},
    {"TYPE_SCROLL",           SCROLL},
    {"TYPE_DIRECTOR",         DIRECTOR},
    {"TYPE_GIRDLE",           GIRDLE},
    {"TYPE_FORCE",            FORCE},
    {"TYPE_POTION_EFFECT",    POTION_EFFECT},
    {"TYPE_JEWEL",            TYPE_JEWEL},
    {"TYPE_NUGGET",           TYPE_NUGGET},
    {"TYPE_EVENT_OBJECT",     TYPE_EVENT_OBJECT},
    {"TYPE_WAYPOINT_OBJECT",  TYPE_WAYPOINT_OBJECT},
    {"TYPE_QUEST_CONTAINER",  TYPE_QUEST_CONTAINER},
    {"TYPE_CLOSE_CON",        CLOSE_CON},
    {"TYPE_CONTAINER",        CONTAINER},
    {"TYPE_ARMOUR_IMPROVER",  ARMOUR_IMPROVER},
    {"TYPE_WEAPON_IMPROVER",  WEAPON_IMPROVER},
    {"TYPE_WEALTH",           TYPE_WEALTH},
    {"TYPE_AI",               TYPE_AI},
    {"TYPE_AGGRO_HISTORY",    TYPE_AGGRO_HISTORY},
    {"TYPE_DAMAGE_INFO",      TYPE_DAMAGE_INFO},
    {"TYPE_SKILLSCROLL",      SKILLSCROLL},
    {"TYPE_QUEST_OBJECT",     TYPE_QUEST_OBJECT},
    {"TYPE_TIMER",            TYPE_TIMER},
    {"TYPE_ENV_SENSOR",       TYPE_ENV_SENSOR},
    {"TYPE_CONN_SENSOR",      TYPE_CONN_SENSOR},
    {"TYPE_PEARL",            TYPE_PEARL},
    {"TYPE_DEEP_SWAMP",       DEEP_SWAMP},
    {"TYPE_IDENTIFY_ALTAR",   IDENTIFY_ALTAR},
    {"TYPE_CANCELLATION",     CANCELLATION},
    {"TYPE_SHOULDER",         SHOULDER},
    {"TYPE_LEGS",             LEGS},
    {"TYPE_FOOD_FORCE",       TYPE_FOOD_FORCE},
    {"TYPE_FOOD_BUFF_FORCE",  TYPE_FOOD_BUFF_FORCE},
    {"TYPE_MENU",             MENU},
    {"TYPE_BALL_LIGHTNING",   BALL_LIGHTNING},
    {"TYPE_SWARM_SPELL",      SWARM_SPELL},
    {"TYPE_RUNE",             RUNE},
    {"TYPE_POWER_CRYSTAL",    POWER_CRYSTAL},
    {"TYPE_CORPSE",           CORPSE},
    {"TYPE_DISEASE",          DISEASE},
    {"TYPE_SYMPTOM",          SYMPTOM},
    {"TYPE_QUEST_TRIGGER",    TYPE_QUEST_TRIGGER},
    {"TYPE_QUEST_OBJECT",     TYPE_QUEST_OBJECT},
    {"TYPE_QUEST_INFO",       TYPE_QUEST_INFO},
    {"TYPE_BEACON",           TYPE_BEACON},
    {"TYPE_GUILD_FORCE",      TYPE_GUILD_FORCE},

    /* logging modes (logger.h) */
    {"LOG_MAPBUG",  llevMapbug},
    {"LOG_INFO",    llevInfo},
    {"LOG_DEBUG",   llevDebug},
    {"LOG_MONSTER", llevMonster},

    /* gmaster modes (gmaster.h) */
    {"GMASTER_MODE_NO",  GMASTER_MODE_NO},
    {"GMASTER_MODE_MW",  GMASTER_MODE_MW},
    {"GMASTER_MODE_VOL", GMASTER_MODE_VOL},
    {"GMASTER_MODE_GM",  GMASTER_MODE_GM},
    {"GMASTER_MODE_MM",  GMASTER_MODE_MM},

    /* spell numbers (spells.h) */
    {"SP_FIRESTORM",        SP_FIRESTORM},
    {"SP_ICESTORM",         SP_ICESTORM},
    {"SP_MINOR_HEAL",       SP_MINOR_HEAL},
    {"SP_CURE_POISON",      SP_CURE_POISON},
    {"SP_CURE_DISEASE",     SP_CURE_DISEASE},
    {"SP_STRENGTH",         SP_STRENGTH},
    {"SP_IDENTIFY",         SP_IDENTIFY},
    {"SP_DETECT_MAGIC",     SP_DETECT_MAGIC},
    {"SP_DETECT_CURSE",     SP_DETECT_CURSE},
    {"SP_REMOVE_CURSE",     SP_REMOVE_CURSE},
    {"SP_REMOVE_DAMNATION", SP_REMOVE_DAMNATION},
    {"SP_CAUSE_LIGHT",      SP_CAUSE_LIGHT},
    {"SP_FIREBOLT",         SP_FIREBOLT},
    {"SP_BULLET",           SP_BULLET},
    {"SP_FROSTBOLT",        SP_FROSTBOLT},
    {"SP_REMOVE_DEPLETION", SP_REMOVE_DEPLETION},
    {"SP_PROBE",            SP_PROBE},
    {"SP_REMOVE_DEATHSICK", SP_REMOVE_DEATHSICK},
    {"SP_RESTORATION",      SP_RESTORATION},
    {"SP_S_LIGHTNING",      SP_S_LIGHTNING},

    /* spell paths (define.h) */
    {"PATH_NONE",          PATH_NULL},
    {"PATH_LIFE",          PATH_LIFE},
    {"PATH_DEATH",         PATH_DEATH},
    {"PATH_ELEMENTAL",     PATH_ELEMENTAL},
    {"PATH_ENERGY",        PATH_ENERGY},
    {"PATH_SPIRIT",        PATH_SPIRIT},
    {"PATH_PROTECTION",    PATH_PROTECTION},
    {"PATH_LIGHT",         PATH_LIGHT},
    {"PATH_NETHER",        PATH_NETHER},
    {"PATH_NATURE",        PATH_NATURE},
    {"PATH_SHADOW",        PATH_SHADOW},
    {"PATH_CHAOS",         PATH_CHAOS},
    {"PATH_EARTH",         PATH_EARTH},
    {"PATH_CONJURATION",   PATH_CONJURATION},
    {"PATH_ABJURATION",    PATH_ABJURATION},
    {"PATH_TRANSMUTATION", PATH_TRANSMUTATION},
    {"PATH_ARCANE",        PATH_ARCANE},

    /* spell types (spells.h) */
    {"SPELL_TYPE_NATURAL", SPELL_TYPE_NATURAL},
    {"SPELL_TYPE_WIZARD",  SPELL_TYPE_WIZARD},
    {"SPELL_TYPE_PRIEST",  SPELL_TYPE_PRIEST},

    /* skill numbers (skills.h) */
    {"SK_STEALING",       SK_STEALING},
    {"SK_LOCKPICKING",    SK_LOCKPICKING},
    {"SK_HIDING",         SK_HIDING},
    {"SK_SMITH",          SK_SMITH},
    {"SK_BOWYER",         SK_BOWYER},
    {"SK_JEWELER",        SK_JEWELER},
    {"SK_ALCHEMY",        SK_ALCHEMY},
    {"SK_THAUMATURGY",    SK_THAUMATURGY},
    {"SK_LITERACY",       SK_LITERACY},
    {"SK_BARGAINING",     SK_BARGAINING},
    {"SK_JUMPING",        SK_JUMPING},
    {"SK_DET_MAGIC",      SK_DET_MAGIC},
    {"SK_ORATORY",        SK_ORATORY},
    {"SK_MUSIC",          SK_MUSIC},
    {"SK_DET_CURSE",      SK_DET_CURSE},
    {"SK_FIND_TRAPS",     SK_FIND_TRAPS},
    {"SK_MEDITATION",     SK_MEDITATION},
    {"SK_BOXING",         SK_BOXING},
    {"SK_FLAME_TOUCH",    SK_FLAME_TOUCH},
    {"SK_KARATE",         SK_KARATE},
    {"SK_CLIMBING",       SK_CLIMBING},
    {"SK_WOODSMAN",       SK_WOODSMAN},
    {"SK_INSCRIPTION",    SK_INSCRIPTION},
    {"SK_MELEE_WEAPON",   SK_MELEE_WEAPON},
    {"SK_MISSILE_WEAPON", SK_MISSILE_WEAPON},
    {"SK_THROWING",       SK_THROWING},
    {"SK_SPELL_CASTING",  SK_SPELL_CASTING},
    {"SK_REMOVE_TRAP",    SK_REMOVE_TRAP},
    {"SK_SET_TRAP",       SK_SET_TRAP},
    {"SK_USE_MAGIC_ITEM", SK_USE_MAGIC_ITEM},
    {"SK_PRAYING",        SK_PRAYING},
    {"SK_CLAWING",        SK_CLAWING},
    {"SK_LEVITATION",     SK_LEVITATION},
    {"SK_DISARM_TRAPS",   SK_DISARM_TRAPS},
    {"SK_XBOW_WEAP",      SK_XBOW_WEAP},
    {"SK_SLING_WEAP",     SK_SLING_WEAP},
    {"SK_IDENTIFY",       SK_IDENTIFY},
    {"SK_SLASH_WEAP",     SK_SLASH_WEAP},
    {"SK_CLEAVE_WEAP",    SK_CLEAVE_WEAP},
    {"SK_PIERCE_WEAP",    SK_PIERCE_WEAP},
    {"SK_TWOHANDS",       SK_TWOHANDS},
    {"SK_POLEARMS",       SK_POLEARMS},

    /* skillgroups (global.h) */
    {"SKILLGROUP_AGILITY",     SKILLGROUP_AGILITY},
    {"SKILLGROUP_PERSONALITY", SKILLGROUP_PERSONAL},
    {"SKILLGROUP_MENTAL",      SKILLGROUP_MENTAL},
    {"SKILLGROUP_PHYSIQUE",    SKILLGROUP_PHYSIQUE},
    {"SKILLGROUP_MAGIC",       SKILLGROUP_MAGIC},
    {"SKILLGROUP_WISDOM",      SKILLGROUP_WISDOM},
    {"SKILLGROUP_MISC",        SKILLGROUP_MISC},

    /* stats (living.h) */
    {"STAT_STRENGTH",     STR},
    {"STAT_DEXTERITY",    DEX},
    {"STAT_CONSTITUTION", CON},
    {"STAT_WISDOM",       WIS},
    {"STAT_CHARISMA",     CHA},
    {"STAT_INTELLIGENCE", INTELLIGENCE},
    {"STAT_POWER",        POW},
    {"STAT_NONE",         NO_STAT_VAL},

    /*money_block modes (global.h) */
    {"MONEYSTRING_ALL",     MONEYSTRING_ALL},
    {"MONEYSTRING_NOTHING", MONEYSTRING_NOTHING},
    {"MONEYSTRING_AMOUNT",  MONEYSTRING_AMOUNT},

    /* cost_string modes (global.h) */
    {"COSTSTRING_SHORT",  COSTSTRING_SHORT},
    {"COSTSTRING_FULL",   COSTSTRING_FULL},
    {"COSTSTRING_APPROX", COSTSTRING_APPROX},

    /* fno modes (object.h) */
    {"FNO_MODE_INV_ONLY",   FNO_MODE_INV_ONLY},
    {"FNO_MODE_CONTAINERS", FNO_MODE_CONTAINERS},
    {"FNO_MODE_ALL",        FNO_MODE_ALL},

    /* NPC interface modes (global.h) */
    {"NPC_INTERFACE_MODE_NO",    NPC_INTERFACE_MODE_NO},
    {"NPC_INTERFACE_MODE_NPC",   NPC_INTERFACE_MODE_NPC},
    {"NPC_INTERFACE_MODE_QLIST", NPC_INTERFACE_MODE_QLIST},

    /* Target modes (commands.h) */
    {"TARGET_ENEMY",  TARGET_ENEMY},
    {"TARGET_FRIEND", TARGET_FRIEND},
    {"TARGET_SELF",   TARGET_SELF},

    {NULL, 0}
};

lua_class Game =
{
    LUATYPE_GAME,
    "Game",
    0,
    NULL,
    NULL,
    Game_methods,
    Game_constants,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0
};

int Game_init(lua_State *L)
{
    init_class(L, &Game);

    return 0;
}


/****************************************************************************/
/*                                                                          */
/*                          Game module functions                           */
/*                                                                          */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : Game_UpgradeApartment                                            */
/* Lua    : game:UpgradeApartment(map_old, map_new, x, y)                    */
/* Info   : Transfer all items with "no_pick 0" setting from map_old         */
/*          to position x,y on map new.                                      */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Game_UpgradeApartment(lua_State *L)
{
    lua_object *map_new, *map_old, *self;
    int         x, y;

    get_lua_args(L, "GMMii", &self, &map_old, &map_new, &x, &y);

    if( !map_new->data.map || !map_old->data.map || x<= 0 || y<=0
            || x>=map_new->data.map->width || y>=map_new->data.map->height)
        return 0;

    /* transfer the items */
    hooks->map_transfer_apartment_items(map_old->data.map, map_new->data.map, x, y);

    return 0;
}


/*****************************************************************************/
/* Name   : Game_LoadObject                                                  */
/* Lua    : game:LoadObject(string)                                          */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Game_LoadObject(lua_State *L)
{
    object *whoptr;
    char   *dumpob;
    CFParm *CFR, CFP;
    lua_object *self;

    get_lua_args(L, "Gs", &self, &dumpob);

    /* First step: We create the object */
    CFP.Value[0] = (void *) (dumpob);
    CFR = (PlugHooks[HOOK_LOADOBJECT]) (&CFP);
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
/* Status : Tested/Stable                                                    */
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
/* Lua    : game:ReadyMap(map_path, flags)                                   */
/* Info   : Loads the map from map_path into memory, unless already loaded.  */
/*          Will normally ONLY create multiplayer maps, not instances.       */
/*          See also object:ReadyUniqueMap(), object:StartNewInstance() and  */
/*          map:ReadyInheritedMap()                                          */
/*          flags:                                                           */
/*            game.MAP_CHECK - don't load the map if it isn't in memory,     */
/*                             returns nil if the map wasn't in memory.      */
/*            game.MAP_NEW - delete the map from memory and force a reset    */
/*                           (if it existed in memory or swap)               */
/*          If map_path is taken from the path attribute of a unique or      */
/*          instance map, this function will actually load the unique map or */
/*          the instance. This is how e.g. paths to apartment maps are stored*/
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Game_ReadyMap(lua_State *L)
{
    char       *mapname;
    int flags = 0;
    const char *path_sh;
    mapstruct  *map=NULL;
    lua_object *self;

    get_lua_args(L, "Gs|i", &self, &mapname, &flags);

    if(mapname)
    {
        path_sh = hooks->create_safe_mapname_sh(mapname);

        if(path_sh) /* we MUST have now a path - we assume first it is the DESTINATION path */
        {
            /* NOTE: this call will do this:
             * - checking loaded maps for path (type independent)
             * - if non MULTI map try to load from ./player or ./instance
             * - return then and DON'T create the map from /maps or other sources
             * The creation is only done when src is != NULL in one of the 2 other
             * ready_map_name() calls down there
            */
            map = hooks->ready_map_name(path_sh, NULL, 0, NULL); /* try load only */

            if(*path_sh != '.')
            {
                if(map && (flags & PLUGIN_MAP_NEW)) /* reset the maps - if it loaded */
                {
                    int num = 0;

                    if(map->player_first)
                        num = hooks->map_to_player_unlink(map); /* remove player from map */

                    hooks->clean_tmp_map(map); /* remove map from memory */
                    hooks->delete_map(map);

                    /* reload map forced from original /maps */
                    map = hooks->ready_map_name(NULL, path_sh, MAP_STATUS_MULTI, NULL);

                    if(num) /* and kick player back to map - note: if map is NULL its bind point redirect */
                        hooks->map_to_player_link(map, -1, -1, FALSE);
                }
                else if (!(flags & PLUGIN_MAP_CHECK))/* normal ready_map_name() with checking loaded & original maps */
                    map = hooks->ready_map_name(path_sh, path_sh, MAP_STATUS_MULTI, NULL);
            }
        }
        FREE_ONLY_HASH(path_sh);
    }

    return push_object(L, &Map, map);
}

/*****************************************************************************/
/* Name   : Game_FindPlayer                                                  */
/* Lua    : game:FindPlayer(name)                                            */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Game_FindPlayer(lua_State *L)
{
    player *foundpl;
    object *foundob = NULL;
    CFParm *CFR, CFP;
    char   *txt;
    lua_object *self;

    get_lua_args(L, "Gs", &self, &txt);

    CFP.Value[0] = (void *) (txt);
    CFR = (PlugHooks[HOOK_FINDPLAYER]) (&CFP);
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
/* Status : Tested/Stable                                                    */
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
/* Status : Tested/Stable                                                    */
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
/* Status : Tested/Stable                                                    */
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
/* Name   : Game_LocateBeacon                                                */
/* Lua    : game:LocateBeacon(name)                                          */
/* Info   : Locates the named beacon if it is in memory. Returns nil         */
/*          otherwise                                                        */
/*          Beacons are very useful for locating objects or locations on maps*/
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Game_LocateBeacon(lua_State *L)
{
    char   *id;
    shstr  *id_s = NULL;
    lua_object *self;
    object *foundob = NULL;

    get_lua_args(L, "Gs", &self, &id);

    FREE_AND_COPY_HASH(id_s, id);
    foundob = hooks->locate_beacon(id_s);
    FREE_ONLY_HASH(id_s);

    return push_object(L, &GameObject, foundob);
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
/* Status : Tested/Stable                                                    */
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

/*****************************************************************************/
/* Name   : Game_Log                                                         */
/* Lua    : game:Log(level, text)                                            */
/* Info   : Write text to the game log                                       */
/*          level should be one of                                           */
/*          game.LOG_INFO for informational messages. These are logged also  */
/*          in production servers.                                           */
/*          game.LOG_DEBUG for debug messages. These are normally not logged */
/*          in production servers.                                           */
/*          game.LOG_MONSTER for detailed monster info. Normally never logged*/
/*          (LOG_MONSTER might be useful for lua behaviours?)                */
/*          When logged, text is always prefixed with "LUA:"                 */
/*          The log levels ERROR and BUG are not available from lua for      */
/*          security reasons (both might terminate the server).              */
/*          The lua "print()" function is redirected to LOG_INFO             */
/* Version: Introduced in beta 4 pre3                                        */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int Game_Log(lua_State *L)
{
    lua_object *self;
    int level;
    const char *text;

    get_lua_args(L, "Gis", &self, &level, &text);

    if(level == llevBug || level == llevError)
        luaL_error(L, "Illegal log level: %d\n", level);
    LOG(level, "LUA:%s\n", text);

    return 0;
}

/*****************************************************************************/
/* Name   : Game_EnumerateCoins                                              */
/* Lua    : game:EnumerateCoins(value)                                       */
/* Info   : Return a table with value nicely enumerated as the optimum       */
/*          denomination coins. The table has the following fields:          */
/*              mithril                                                      */
/*              gold                                                         */
/*              silver                                                       */
/*              copper                                                       */
/* Status : Unfinished                                                       */
/* TODO   : While this works as described, it is likely to change somewhat so*/
/*          probably should not be used ATM.                                 */
/*****************************************************************************/
static int Game_EnumerateCoins(lua_State *L)
{
    lua_object *self;
    sint64      value;
    sint64      mithril = 0, gold = 0, silver = 0, copper = 0;

    get_lua_args(L, "GI", &self, &value);
    mithril = value / 10000000; value -= mithril * 10000000;
    gold    = value / 10000;    value -= gold    * 10000;
    silver  = value / 100;      value -= silver  * 100;
    copper  = value;
    lua_newtable(L);
    lua_pushliteral(L, "mithril");
    lua_pushnumber(L, (lua_Number) mithril);
    lua_rawset(L, -3);
    lua_pushliteral(L, "gold");
    lua_pushnumber(L, (lua_Number) gold);
    lua_rawset(L, -3);
    lua_pushliteral(L, "silver");
    lua_pushnumber(L, (lua_Number) silver);
    lua_rawset(L, -3);
    lua_pushliteral(L, "copper");
    lua_pushnumber(L, (lua_Number) copper);
    lua_rawset(L, -3);
    return 1;
}

/* FUNCTIONEND -- End of the Lua plugin functions. */

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
