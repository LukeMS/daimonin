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
#include <plugin_lua.h>
#include <daimonin_game.h>

static struct method_decl Game_methods[] =
{
    {"EnumerateCoins",   Game_EnumerateCoins},
    {"FindPlayer",       Game_FindPlayer},
    {"GetSkillNr",       Game_GetSkillNr},
    {"GetSpellNr",       Game_GetSpellNr},
    {"GetTimeAndDate",   Game_GetTimeAndDate},
    {"GlobalMessage",    Game_GlobalMessage},
    {"IsValid",          Game_IsValid},
    {"LoadObject",       Game_LoadObject},
    {"LocateBeacon",     Game_LocateBeacon},
    {"Log",              Game_Log},
    {"MatchString",      Game_MatchString},
    {"PrintTimeAndDate", Game_PrintTimeAndDate},
    {"ReadyMap",         Game_ReadyMap},
/*  {"RegisterCommand",  Game_RegisterCommand}, */
    {"UpgradeApartment", Game_UpgradeApartment},

    {NULL, NULL}
};

static struct constant_decl *Game_constants = NULL;

static struct constant_decl preset_game_constants[] =
{
    /* general success/failure (constants not defined by server)
     * These are to provide meaningful constants for core methods which return
     * a number to indicate success or failue. 0 is always success and positive
     * numbers are always failure. Stick to these two outcomes if at all
     * possible, but if you must, negative can be used for partial or qualified
     * success, or semi-failure, or etc). */
    {"SUCCESS",             0},
    {"FAILURE",             1},
    /* Failures for object:SetSkill() */
    {"FAILURE_NOSKILL",     1},
    {"FAILURE_NONLEVELING", 2},
    {"FAILURE_MAXLEVEL",    3},
    {"FAILURE_INDIRECT_NO", 4},

    /* Embedded character codes (protocol.h) */
    {"ECC_STRONG",    ECC_STRONG},
    {"ECC_EMPHASIS",  ECC_EMPHASIS},
    {"ECC_UNDERLINE", ECC_UNDERLINE},
    {"ECC_HYPERTEXT", ECC_HYPERTEXT},

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

    /* cloning modes (object.h) */
    {"CLONE_WITH_INVENTORY",    MODE_INVENTORY},
    {"CLONE_WITHOUT_INVENTORY", MODE_NO_INVENTORY},

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

    /* map loading modes (plugin_lua/include/plugin_lua.h) */
    {"MAP_CHECK", PLUGIN_MAP_CHECK},
    {"MAP_NEW",   PLUGIN_MAP_NEW},

    /* map status flags (map.h) */
    {"INSTANCE_NO_REENTER", MAP_INSTANCE_FLAG_NO_REENTER},
    {"MFLAG_NO_FALLBACK",   MAP_STATUS_NO_FALLBACK },
    {"MFLAG_LOAD_ONLY",     MAP_STATUS_LOAD_ONLY},
    {"MFLAG_FIXED_LOGIN",   MAP_STATUS_FIXED_LOGIN},

    /* object positioning flags (overlay.h) */
    {"OVERLAY_IGNORE_TERRAIN",  OVERLAY_IGNORE_TERRAIN},
    {"OVERLAY_WITHIN_LOS",      OVERLAY_WITHIN_LOS},
    {"OVERLAY_FORCE",           OVERLAY_FORCE},
    {"OVERLAY_FIRST_AVAILABLE", OVERLAY_FIRST_AVAILABLE},
    {"OVERLAY_FIXED",           OVERLAY_FIXED},
    {"OVERLAY_RANDOM",          OVERLAY_RANDOM},
    {"OVERLAY_SPECIAL",         OVERLAY_SPECIAL},

    /* quest trigger sub types (define.h) */
    {"QUEST_NORMAL",   ST1_QUEST_TRIGGER_NORMAL},
    {"QUEST_KILL",     ST1_QUEST_TRIGGER_KILL},
    {"QUEST_KILLITEM", ST1_QUEST_TRIGGER_KILL_ITEM},
    {"QUEST_ITEM",     ST1_QUEST_TRIGGER_ITEM},

    /* quest steps (quest.h) */
    {"QSTAT_UNKNOWN",  QSTAT_UNKNOWN},
    {"QSTAT_NO",       QSTAT_NO},
    {"QSTAT_ACTIVE",   QSTAT_ACTIVE},
    {"QSTAT_SOLVED",   QSTAT_SOLVED},
    {"QSTAT_DONE",     QSTAT_DONE},
    {"QSTAT_DISALLOW", QSTAT_DISALLOW},

    /* guild force sub types (define.h) */
    {"GUILD_NO",  0},
    {"GUILD_IN",  ST1_GUILD_IN},
    {"GUILD_OLD", ST1_GUILD_OLD},


    /* Argh, the object types. Make sure to keep up-to date if any are added/removed (define.h) */
    {"TYPE_PLAYER",           PLAYER},
    {"TYPE_BULLET",           BULLET},
    {"TYPE_ROD",              ROD},
    {"TYPE_TREASURE",         TREASURE},
    {"TYPE_POTION",           POTION},
    {"TYPE_FOOD",             FOOD},
    {"TYPE_POISON",           POISON},
    {"TYPE_BOOK",             BOOK},
    {"TYPE_FBULLET",          FBULLET},
    {"TYPE_FBALL",            FBALL},
    {"TYPE_LIGHTNING",        LIGHTNING},
    {"TYPE_ARROW",            ARROW},
    {"TYPE_BOW",              BOW},
    {"TYPE_WEAPON",           WEAPON},
    {"TYPE_ARMOUR",           ARMOUR},
    {"TYPE_PEDESTAL",         PEDESTAL},
    {"TYPE_ALTAR",            ALTAR},
    {"TYPE_DOOR",             TYPE_DOOR},
    {"TYPE_KEY",              TYPE_KEY},
    {"TYPE_MAP",              MAP},
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
    {"TYPE_LOOT",             LOOT},
    {"TYPE_GRAVESTONE",       GRAVESTONE},
    {"TYPE_AMULET",           AMULET},
    {"TYPE_PLAYERMOVER",      PLAYERMOVER},
    {"TYPE_TELEPORTER",       TELEPORTER},
    {"TYPE_CREATOR",          CREATOR},
    {"TYPE_SKILL",            TYPE_SKILL},
    {"TYPE_SKILLGROUP",       TYPE_SKILLGROUP},
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
    {"TYPE_PEACEMAKER",       PEACEMAKER},
    {"TYPE_GEM",              GEM},
    {"TYPE_FIRECHEST",        FIRECHEST},
    {"TYPE_FIREWALL",         FIREWALL},
    {"TYPE_ANVIL",            ANVIL},
    {"TYPE_CHECK_INV",        CHECK_INV},
    {"TYPE_EXIT",             EXIT},
    {"TYPE_AGE_FORCE",        TYPE_AGE_FORCE},
    {"TYPE_SHOP_FLOOR",       SHOP_FLOOR},
    {"TYPE_SHOP_MAT",         SHOP_MAT},
    {"TYPE_RING",             RING},
    {"TYPE_FLOOR",            FLOOR},
    {"TYPE_INORGANIC",        INORGANIC},
    {"TYPE_LIGHT_APPLY",      TYPE_LIGHT_APPLY },
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
    {"TYPE_BRACERS",          BRACERS},
    {"TYPE_SAVEBED",          SAVEBED},
    {"TYPE_POISONCLOUD",      POISONCLOUD},
    {"TYPE_FIREHOLES",        FIREHOLES},
    {"TYPE_WAND",             WAND},
    {"TYPE_ABILITY",          ABILITY},
    {"TYPE_SCROLL",           SCROLL},
    {"TYPE_DIRECTOR",         DIRECTOR},
    {"TYPE_GIRDLE",           GIRDLE},
    {"TYPE_FORCE",            FORCE},
    {"TYPE_SPARKLY",          TYPE_SPARKLY},
    {"TYPE_JEWEL",            TYPE_JEWEL},
    {"TYPE_NUGGET",           TYPE_NUGGET},
    {"TYPE_EVENT_OBJECT",     TYPE_EVENT_OBJECT},
    {"TYPE_WAYPOINT_OBJECT",  TYPE_WAYPOINT_OBJECT},
    {"TYPE_QUEST_CONTAINER",  TYPE_QUEST_CONTAINER},
    {"TYPE_CONTAINER",        CONTAINER},
    {"TYPE_WEALTH",           TYPE_WEALTH},
    {"TYPE_AI",               TYPE_AI},
    {"TYPE_AGGRO_HISTORY",    TYPE_AGGRO_HISTORY},
    {"TYPE_DAMAGE_INFO",      TYPE_DAMAGE_INFO},
    {"TYPE_TIMER",            TYPE_TIMER},
    {"TYPE_ENV_SENSOR",       TYPE_ENV_SENSOR},
    {"TYPE_CONN_SENSOR",      TYPE_CONN_SENSOR},
    {"TYPE_PEARL",            TYPE_PEARL},
    {"TYPE_DEEP_SWAMP",       DEEP_SWAMP},
    {"TYPE_CANCELLATION",     CANCELLATION},
    {"TYPE_SHOULDER",         SHOULDER},
    {"TYPE_LEGS",             LEGS},
    {"TYPE_FOOD_FORCE",       TYPE_FOOD_FORCE},
    {"TYPE_FOOD_BUFF_FORCE",  TYPE_FOOD_BUFF_FORCE},
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
    {"TYPE_QUEST_UPDATE",     TYPE_QUEST_UPDATE},
    {"TYPE_SHOP_CONTAINER",   TYPE_SHOP_CONTAINER},

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
    {"GMASTER_MODE_SA",  GMASTER_MODE_SA},

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
    {"SPELL_TYPE_NONE",    -1},
    {"SPELL_TYPE_NATURAL", SPELL_TYPE_NATURAL},
    {"SPELL_TYPE_WIZARD",  SPELL_TYPE_WIZARD},
    {"SPELL_TYPE_PRIEST",  SPELL_TYPE_PRIEST},

    /* skill leveling modes (skill.h) */
    {"INDIRECT_NO", -INDIRECT},
    {"NONLEVELING", NONLEVELING},
    {"INDIRECT",    INDIRECT},
    {"DIRECT",      DIRECT},

    /* skill groups used by quests (constants not define by server) */
    /* why not just use game.SKILLGROUP_* and add 1 in the relevant functions? */
    /* this would be invisible to scripters and reduce the number of constants used */
    {"ITEM_SKILL_NO",          0},
    {"ITEM_SKILL_AGILITY",     SKILLGROUP_AGILITY + 1},
    {"ITEM_SKILL_PERSONALITY", SKILLGROUP_PERSONAL + 1},
    {"ITEM_SKILL_MENTAL",      SKILLGROUP_MENTAL + 1},
    {"ITEM_SKILL_PHYSICAL",    SKILLGROUP_PHYSIQUE + 1},
    {"ITEM_SKILL_MAGIC",       SKILLGROUP_MAGIC + 1},
    {"ITEM_SKILL_WISDOM",      SKILLGROUP_WISDOM + 1},

    /* skillgroups (skill.h) */
    {"SKILLGROUP_AGILITY",     SKILLGROUP_AGILITY},
    {"SKILLGROUP_PERSONALITY", SKILLGROUP_PERSONAL},
    {"SKILLGROUP_MENTAL",      SKILLGROUP_MENTAL},
    {"SKILLGROUP_PHYSIQUE",    SKILLGROUP_PHYSIQUE},
    {"SKILLGROUP_MAGIC",       SKILLGROUP_MAGIC},
    {"SKILLGROUP_WISDOM",      SKILLGROUP_WISDOM},
    {"SKILLGROUP_MISC",        SKILLGROUP_MISC},

    /* stats (living.h) */
    {"STAT_STRENGTH",     STAT_STR},
    {"STAT_DEXTERITY",    STAT_DEX},
    {"STAT_CONSTITUTION", STAT_CON},
    {"STAT_INTELLIGENCE", STAT_INT},
    {"STAT_WISDOM",       STAT_WIS},
    {"STAT_POWER",        STAT_POW},
    {"STAT_CHARISMA",     STAT_CHA},
    {"STAT_NONE",         STAT_NONE},

    /*moneyblock_t modes (global.h) */
    {"MONEY_MODE_ALL",      MONEY_MODE_ALL},
    {"MONEY_MODE_NOTHING",  MONEY_MODE_NOTHING},
    {"MONEY_MODE_AMOUNT",   MONEY_MODE_AMOUNT},

    /* cost_string modes (global.h) */
    {"COSTSTRING_SHORT",  COSTSTRING_SHORT},
    {"COSTSTRING_FULL",   COSTSTRING_FULL},
    {"COSTSTRING_APPROX", COSTSTRING_APPROX},

    /* fno modes (object.h) */
    {"FNO_MODE_INV_ONLY",   FNO_MODE_INV_ONLY},
    {"FNO_MODE_CONTAINERS", FNO_MODE_CONTAINERS},
    {"FNO_MODE_ALL",        FNO_MODE_ALL},

    /* NPC interface modes (protocol.h) */
    {"GUI_NPC_MODE_NO",         GUI_NPC_MODE_NO},
    {"GUI_NPC_MODE_NPC",        GUI_NPC_MODE_NPC},
    {"GUI_NPC_MODE_RHETORICAL", GUI_NPC_MODE_RHETORICAL},
    {"GUI_NPC_MODE_QUEST",      GUI_NPC_MODE_QUEST},

    /* Target modes (constants not defined by server) */
    {"TARGET_ENEMY",  0},
    {"TARGET_FRIEND", 1},
    {"TARGET_SELF",   2},

    /* Map darkness (global.h) */
    /* Bizarrely higher numbers mean brighter so really MAX_DARKNESS means
     * MIN_DARKNESS. */
    {"MAP_DARKNESS_TOTAL", 0},
    {"MAP_DARKNESS_MIN",   1},
    {"MAP_DARKNESS_MAX",   MAX_DARKNESS},

    /* Personal light (global.h) */
    {"PERSONAL_LIGHT_OFF", 0},
    {"PERSONAL_LIGHT_MIN", 1},
    {"PERSONAL_LIGHT_MAX", MAX_DARKNESS},

    /* Basic TAD components (calendar.h) */
    {"ARKHE_MES_PER_HR", ARKHE_MES_PER_HR},
    {"ARKHE_HRS_PER_DY", ARKHE_HRS_PER_DY},
    {"ARKHE_DYS_PER_PK", ARKHE_DYS_PER_PK},
    {"ARKHE_PKS_PER_WK", ARKHE_PKS_PER_WK},
    {"ARKHE_WKS_PER_MH", ARKHE_WKS_PER_MH},
    {"ARKHE_MHS_PER_SN", ARKHE_MHS_PER_SN},
    {"ARKHE_SNS_PER_YR", ARKHE_SNS_PER_YR},

    /* game:PrintTimeAndDate flags (calendar.h) */
    {"TAD_SHOWTIME",   TAD_SHOWTIME},
    {"TAD_SHOWDATE",   TAD_SHOWDATE},
    {"TAD_SHOWSEASON", TAD_SHOWSEASON},
    {"TAD_LONGFORM",   TAD_LONGFORM},

    /* Bitflags to help describe what happens while buffing an object.
     * Because they are bitmasks, bitmasks.lua should be included for
     * easy use of these constants.
     */
    {"BUFF_ADD_SUCCESS", BUFF_ADD_SUCCESS},
    {"BUFF_ADD_EXISTS", BUFF_ADD_EXISTS},
    {"BUFF_ADD_LIMITED", BUFF_ADD_LIMITED},
    {"BUFF_ADD_MAX_EXCEEDED", BUFF_ADD_MAX_EXCEEDED},
    {"BUFF_ADD_BAD_PARAMS", BUFF_ADD_BAD_PARAMS},
    {"BUFF_ADD_NO_INSERT", BUFF_ADD_NO_INSERT},

    /* Object layers (msp.h) -- the plugin has no concept of clayers (they are
     * irrelevant to scripters) and mostly scripts only refer to object->layer,
     * not msp->slayer. So just call them LAYER_*. */
    {"LAYER_SYSTEM",  MSP_SLAYER_SYSTEM},
    {"LAYER_FLOOR",   MSP_SLAYER_FLOOR},
    {"LAYER_FMASK",   MSP_SLAYER_FMASK},
    {"LAYER_ITEMA",   MSP_SLAYER_ITEMA},
    {"LAYER_ITEMB",   MSP_SLAYER_ITEMB},
    {"LAYER_FEATURE", MSP_SLAYER_FEATURE},
#ifdef USE_SLAYER_MONSTER
    {"LAYER_MONSTER", MSP_SLAYER_MONSTER},
#else
    {"LAYER_MONSTER", MSP_SLAYER_FEATURE},
#endif
    {"LAYER_PLAYER",  MSP_SLAYER_PLAYER},
    {"LAYER_EFFECT",  MSP_SLAYER_EFFECT},
};

lua_class Game =
{
    LUATYPE_GAME,
    "Game",
    0,
    NULL,
    NULL,
    NULL,
    Game_methods,
    NULL,   /* address of Game_constants set in Game_init */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    0
};

int Game_init(lua_State *L)
{
    int     i;
    char    buf[TINY_BUF];
    char    string[TINY_BUF];
    char    name[32];
    char    prefix[32];
    FILE    *sf;
    size_t  size_presets = sizeof(preset_game_constants);
    int     num_presets = size_presets / sizeof(struct constant_decl);
    int     start_found = 0;
    int     type_count  = 0;
    int     sound_count = 0;
    int     num_sounds  = 0;
    size_t  size_sounds = 0;
    int     index;

    // Try to open sound file
    sprintf(buf, "%s/client_sounds", hooks->settings->datadir);
    if (!(sf = fopen(buf, "rb")))
    {
        LOG(llevBug,"ERROR: Can't find file %s.\n", buf);
    }
    else
    {
        // Quick-scan the file to count number of soundtype and sound entries
        while (fgets(buf, sizeof(buf), sf) != NULL)
        {
            // Strip trailing newline character(s) (allow for \r\n or \n)
            buf[strcspn(buf, "\r\n")] = '\0';

            if ((strlen(buf) == 0) || (buf[0] == '#'))
                continue;

            if (!strcmp(buf, "*end"))
                break;

            if (strncmp(buf, "*start", 6) == 0)
            {
                strtok(buf, "|"); // discard *start
                sscanf(strtok(NULL, "|"), "%d", &type_count); // count of soundtypes
            }
            else if ((type_count > 0) && (buf[0] == '*'))
            {
                // New soundtype
                type_count--;
                strtok(buf, "|");   // discard type id this time
                strtok(NULL, "|");  // discard type name this time
                strtok(NULL, "|");  // discard prefix this time
                sscanf(strtok(NULL, "|"), "%d", &sound_count);  // number of sounds in this section
                num_sounds += sound_count + 1;      // number of sounds plus soundtype entry
            }

            // If all sound types processed, leave the loop
            if (type_count == 0)
                break;
        }

        // Rewind the file ready for the re-scan for all the info
        rewind(sf);
    }

    // Calculate space required for sounds entries
    size_sounds = num_sounds * sizeof(struct constant_decl);

    // Allocate space for preset constants + sounds constants + terminator
    Game_constants = malloc(size_presets + size_sounds + sizeof(struct constant_decl));

    // Copy the preset constants
    for (i = 0; i < num_presets; i++)
    {
        Game_constants[i].name  = preset_game_constants[i].name;
        Game_constants[i].value = preset_game_constants[i].value;
    }

    // If the sounds file was successfully opened, scan for sound types and sounds
    // and append the entries to the Game_constants array
    if (sf)
    {
        // Index into Game_constants array starts at num_presets
        index = num_presets;

        while (fgets(buf, sizeof(buf), sf) != NULL)
        {
            // Strip trailing newline character(s) (allow for \r\n or \n)
            buf[strcspn(buf, "\r\n")] = '\0';

            if ((strlen(buf) == 0) || (buf[0] == '#'))
                continue;

            if (!strcmp(buf, "*end"))
                break;

            // Do nothing until start line passed
            if (!start_found)
            {
                if (strncmp(buf, "*start", 6) == 0)
                    start_found = 1;
            }
            else if (buf[0] == '*')
            {
                // sound type line
                // store the id
                sscanf(strtok(buf, "|"), "*%d", &Game_constants[index].value);

                // get the name, uppercase it and prefix with "SOUNDTYPE_"
                strcpy(name, strtok(NULL, "|"));
                strcpy(string, "SOUNDTYPE_");
                strcat(string, name);
                Game_constants[index++].name = hooks->strdup_local(string); // duplicate into array

                // save the prefix
                strcpy(prefix, strtok(NULL, "|"));
            }
            else if (buf[0] == '+')
            {
                // sound line
                // store the id
                sscanf(strtok(buf, "|"), "+%d", &Game_constants[index].value);

                // get the name, uppercase it and prefix with prefix
                strcpy(name, strtok(NULL, "|"));
                strcpy(string, prefix);
                strcat(string, name);
                Game_constants[index++].name = hooks->strdup_local(string); // duplicate into array
            }
        }

        // Close the file
        fclose(sf);
    }

    // Append the null terminator
    Game_constants[num_presets+num_sounds].name = NULL;
    Game_constants[num_presets+num_sounds].value = 0;

    // Set the pointer in the class structure
    Game.constants = Game_constants;

    init_class(L, &Game);

    return 0;
}

// Free the memory allocated for game constants
void Game_free()
{
    size_t  size_presets = sizeof(preset_game_constants);
    int     num_presets = size_presets / sizeof(struct constant_decl);

    // Don't try to free preset names
    int     i = num_presets;

    if (Game_constants == NULL)
        return;

    while (Game_constants[i].name != NULL)
    {
        free(Game_constants[i++].name);
    }
    free(Game_constants);
    Game_constants = NULL;
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
/*****************************************************************************/
static int Game_UpgradeApartment(lua_State *L)
{
    lua_object *map_new, *map_old, *self;
    int         x, y;

    get_lua_args(L, "GMMii", &self, &map_old, &map_new, &x, &y);

    if (!map_new->data.map ||
        !map_old->data.map ||
        x <= 0 ||
        y <= 0 ||
        x >= MAP_WIDTH(map_new->data.map) ||
        y >= MAP_HEIGHT(map_new->data.map))
    {
        return 0;
    }

    /* transfer the items */
    hooks->map_transfer_apartment_items(map_old->data.map, map_new->data.map, x, y);

    return 0;
}


/*****************************************************************************/
/* Name   : Game_LoadObject                                                  */
/* Lua    : game:LoadObject(string)                                          */
/*****************************************************************************/
static int Game_LoadObject(lua_State *L)
{
    lua_object *self;
    char       *obstr;
    object_t     *ob;

    get_lua_args(L, "Gs", &self, &obstr);

    ob = hooks->load_object_str(obstr);

    if (!ob)
        return luaL_error(L, "game:LoadObject(): Could not create object!");

    return push_object(L, &GameObject, ob);
}

/*****************************************************************************/
/* Name   : Game_MatchString                                                 */
/* Lua    : game:MatchString(firststr, secondstr)                            */
/* Info   : Case insensitive string comparision. Returns 1 if the two        */
/*          strings are the same, or 0 if they differ.                       */
/*          secondstring can contain regular expressions.                    */
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
/* Lua    : game:ReadyMap(path, mode)                                        */
/* Info   : Loads the map pointed to by path into memory.                    */
/*                                                                           */
/*          path is a required string. It may be an absolute path            */
/*          (recommended, the map will be handled as a multi) or a path to a */
/*          unique/instance (in which case mode must be game.MAP_CHECK -- ie,*/
/*          this method cannot load such maps).                              */
/*                                                                           */
/*          mode is an optional number. Use one of:                          */
/*            game.MAP_CHECK - don't load the map if it isn't in memory,     */
/*                             returns nil if the map wasn't in memory.      */
/*            game.MAP_NEW - if the map is already in memory, force an       */
/*                           immediate reset; then (re)load it.              */
/* Return : map pointer to map, or nil                                       */
/*****************************************************************************/
static int Game_ReadyMap(lua_State *L)
{
    lua_object *self;
    const char *path;
    int         mode = 0;
    shstr_t      *orig_path_sh;
    map_t  *m;

    get_lua_args(L, "Gs|i", &self, &path, &mode);

    if (!(orig_path_sh = hooks->create_safe_path_sh(path)))
    {
        return luaL_error(L, "game:ReadyMap() could not verify the supplied path: >%s<!",
                          STRING_SAFE(path));
    }

    if (*orig_path_sh == '.' &&
        mode != PLUGIN_MAP_CHECK)
    {
        return luaL_error(L, "game:ReadyMap() cannot load unique/instanced maps, it can only check if they are already loaded!");
    }

    m = hooks->map_is_in_memory(orig_path_sh);

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
        m = hooks->ready_map_name(NULL, orig_path_sh, MAP_STATUS_MULTI, NULL);
    }

    SHSTR_FREE(orig_path_sh);
    push_object(L, &Map, m);

    return 1;
}

/*****************************************************************************/
/* Name   : Game_FindPlayer                                                  */
/* Lua    : game:FindPlayer(name)                                            */
/*****************************************************************************/
static int Game_FindPlayer(lua_State *L)
{
    player_t *foundpl;
    object_t *foundob = NULL;
    CFParm *CFR, CFP;
    char   *txt;
    lua_object *self;

    get_lua_args(L, "Gs", &self, &txt);

    CFP.Value[0] = (void *) (txt);
    CFR = (PlugHooks[HOOK_FINDPLAYER]) (&CFP);
    foundpl = (player_t *) (CFR->Value[0]);
    free(CFR);

    if (foundpl != NULL)
        foundob = foundpl->ob;

    return push_object(L, &GameObject, foundob);
}

/*****************************************************************************/
/* Name   : Game_GetSpellNr                                                  */
/* Lua    : game:GetSpellNr(name)                                            */
/* Info   : Gets the number and type of the named spell.                     */
/*          Returns spell number and spell type (one of the game.SPELL_TYPE_**/
/*          constants (or -1 and game.SPELL_TYPE_NONE if no such spell       */
/*          exists.                                                          */
/*****************************************************************************/
static int Game_GetSpellNr(lua_State *L)
{
    char       *name;
    lua_object *self;
    int         n;
    int         t;

    get_lua_args(L, "Gs", &self, &name);
    n = hooks->look_up_spell_name(name);
    t = (n == -1) ? -1 : hooks->spells[n].type;
    lua_pushnumber(L, n);
    lua_pushnumber(L, t);

    return 2;
}

/*****************************************************************************/
/* Name   : Game_GetSkillNr                                                  */
/* Lua    : game:GetSkillNr(name)                                            */
/* Info   : Gets the number of the named skill. -1 if no such skill exists   */
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
       lua_pushboolean(L, obj->class->isValid(obj));
    }

    return 1;
}

/*****************************************************************************/
/* Name   : Game_LocateBeacon                                                */
/* Lua    : game:LocateBeacon(name)                                          */
/* Info   : Locates the named beacon if it is in memory. Returns nil         */
/*          otherwise                                                        */
/*          Beacons are very useful for locating objects or locations on maps*/
/*****************************************************************************/
static int Game_LocateBeacon(lua_State *L)
{
    char   *id;
    shstr_t  *id_s = NULL;
    lua_object *self;
    object_t *foundob = NULL;

    get_lua_args(L, "Gs", &self, &id);

    SHSTR_FREE_AND_ADD_STRING(id_s, id);
    foundob = hooks->locate_beacon(id_s);
    SHSTR_FREE(id_s);

    return push_object(L, &GameObject, foundob);
}

/*****************************************************************************/
/* Name   : Game_GetTimeAndDate                                              */
/* Lua    : game:GetTimeAndDate(offset)                                      */
/* Info   : Returns info about the game time.                                */
/*          offset is optional. If specified it may be a number which is the */
/*          number of hours in the future or the past which the return       */
/*          represents, or a string which is parsed to arrive at a similar   */
/*          result (eg, "11 parweeks, 2 days, and 17 hours").                */
/* Return : 3 returns: a table which reflects the game time given offset; a  */
/*          number which is the CURRENT absolute game time to an hour's      */
/*          resolution; a number which is the absolute game time given offset*/
/*          to an hour's resolution.                                         */
/*          The table has the following fields:                              */
/*            daylight_darkness - daylight darkness as number                */
/*            daylight_brightness - daylight brightness as number            */
/*            hour - hour in day as number                                   */
/*            minute - minute in hour as number                              */
/*            dayofyear - day of year as number                              */
/*            year - year as number                                          */
/*            season - season in year as number                              */
/*            month - month in season as number                              */
/*            week - week in month as number                                 */
/*            parweek - parweek in week as number                            */
/*            day - day in parweek as number                                 */
/*            intraholiday - intraholiday number TODO                        */
/*            extraholiday - extraholiday in year as number                  */
/*            season_name - season name as string                            */
/*            month_name - month name as string                              */
/*            parweek_name - parweek name as string                          */
/*            day_name - day name as string                                  */
/*            intraholiday_name - intraholiday name as string                */
/*            extraholiday_name - extraholiday name as string                */
/*****************************************************************************/
static int Game_GetTimeAndDate(lua_State *L)
{
    lua_object    *self;
    const char    *string = NULL;
    sint32         offset;
    timeanddate_t  tad;

    get_lua_args(L, "G|s", &self, &string);
    offset = hooks->get_tad_offset_from_string(string);
    memset(&tad, 0, sizeof(timeanddate_t));
    hooks->get_tad(&tad, offset);
    lua_newtable(L);

    /* Daylight (numbers) */
    lua_pushliteral(L, "daylight_darkness");
    lua_pushnumber(L, (lua_Number)tad.daylight_darkness); /* -7 <= n <= 7 */
    lua_rawset(L, -3);
    lua_pushliteral(L, "daylight_brightness");
    lua_pushnumber(L, (lua_Number)tad.daylight_brightness); /* -1280 <= n <= 1280 */
    lua_rawset(L, -3);

    /* Time (numbers) */
    lua_pushliteral(L, "hour");
    lua_pushnumber(L, (lua_Number)tad.hour); /* 0 <= n <= 23 */
    lua_rawset(L, -3);
    lua_pushliteral(L, "minute");
    lua_pushnumber(L, (lua_Number)tad.minute); /* 0 <= n <= 59 */
    lua_rawset(L, -3);

    /* Date (numbers) */
    lua_pushliteral(L, "dayofyear");
    lua_pushnumber(L, (lua_Number)tad.dayofyear); /* 0 <= n <= 8640 */
    lua_rawset(L, -3);
    lua_pushliteral(L, "year");
    lua_pushnumber(L, (lua_Number)tad.year); /* -16383 <= n <= +16383 */
    lua_rawset(L, -3);
    lua_pushliteral(L, "season");

    if (tad.season >= 0)
        lua_pushnumber(L, (lua_Number)tad.season + 1); /* 1 <= n <= 4 */
    else
        lua_pushnil(L);

    lua_rawset(L, -3);
    lua_pushliteral(L, "month");

    if (tad.month >= 0)
        lua_pushnumber(L, (lua_Number)tad.month + 1); /* 1 <= n <= 3 */
    else
        lua_pushnil(L);

    lua_rawset(L, -3);
    lua_pushliteral(L, "week");

    if (tad.week >= 0)
        lua_pushnumber(L, (lua_Number)tad.week + 1); /* 1 <= n <= 3 */
    else
        lua_pushnil(L);

    lua_rawset(L, -3);
    lua_pushliteral(L, "parweek");

    if (tad.parweek >= 0)
        lua_pushnumber(L, (lua_Number)tad.parweek + 1); /* 1 <= n <= 3 */
    else
        lua_pushnil(L);

    lua_rawset(L, -3);
    lua_pushliteral(L, "day");

    if (tad.day >= 0)
        lua_pushnumber(L, (lua_Number)tad.day + 1); /* 1 <= n <= 3 */
    else
        lua_pushnil(L);

    lua_rawset(L, -3);
    lua_pushliteral(L, "intraholiday");

    if (tad.intraholiday >= 0)
        lua_pushnumber(L, (lua_Number)tad.intraholiday + 1); /* TODO */
    else
        lua_pushnil(L);

    lua_rawset(L, -3);
    lua_pushliteral(L, "extraholiday");

    if (tad.extraholiday >= 0)
        lua_pushnumber(L, (lua_Number)tad.extraholiday + 1); /* 1 <= n <= 12 */
    else
        lua_pushnil(L);

    lua_rawset(L, -3);

    /* Date (names) */
    lua_pushliteral(L, "season_name");
    lua_pushstring(L, tad.season_name);
    lua_rawset(L, -3);
    lua_pushliteral(L, "month_name");
    lua_pushstring(L, tad.month_name);
    lua_rawset(L, -3);
    lua_pushliteral(L, "parweek_name");
    lua_pushstring(L, tad.parweek_name);
    lua_rawset(L, -3);
    lua_pushliteral(L, "day_name");
    lua_pushstring(L, tad.day_name);
    lua_rawset(L, -3);
    lua_pushliteral(L, "intraholiday_name");
    lua_pushstring(L, tad.intraholiday_name);
    lua_rawset(L, -3);
    lua_pushliteral(L, "extraholiday_name");
    lua_pushstring(L, tad.extraholiday_name);
    lua_rawset(L, -3);
    lua_pushnumber(L, *hooks->tadtick);
    lua_pushnumber(L, *hooks->tadtick + offset);
    return 3;
}

/*****************************************************************************/
/* Name   : GameObject_GlobalMessage                                         */
/* Lua    : game:GlobalMessage(message, color)                               */
/* Info   : Writes a message to every online player.                         */
/*          color should be one of the game.COLOR_xxx constants.             */
/*          default color is red.                                            */
/*****************************************************************************/
static int Game_GlobalMessage(lua_State *L)
{
    char       *message;
    int         color = NDI_RED;
    lua_object *self;

    get_lua_args(L, "Gs|i", &self, &message, &color);

    /* No point mucking about with an empty message. */
    if (*message)
    {
        hooks->ndi(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | color, 5, NULL,
                      "%s", message);
    }

    return 0;
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
/*****************************************************************************/
static int Game_Log(lua_State *L)
{
    lua_object *self;
    int level;
    const char *text;

    get_lua_args(L, "Gis", &self, &level, &text);

    if(level == llevBug || level == llevError)
        luaL_error(L, "Illegal log level: %d\n", level);
    LOG(level, "LUA script log - %s\n", text);

    return 0;
}

/*****************************************************************************/
/* Name   : Game_EnumerateCoins                                              */
/* Lua    : game:EnumerateCoins(value)                                       */
/* Info   : Return four numbers with value nicely enumerated as the optimum  */
/*          denomination coins. The returns are in the order: copper, silver,*/
/*          gold, mithril. This means you can passthis method call directly  */
/*          as the argument to other methods which take c, s, g, m as four   */
/*          arguments, such as: object:AddMoney(game:EnumerateCoins(value)). */
/*****************************************************************************/
static int Game_EnumerateCoins(lua_State *L)
{
    lua_object *self;
    sint64      value,
                mithril = 0,
                gold = 0,
                silver = 0;

    get_lua_args(L, "GI", &self, &value);
    mithril = value / 10000000;
    value -= mithril * 10000000;
    gold = value / 10000;
    value -= gold * 10000;
    silver = value / 100;
    lua_pushnumber(L, (lua_Number) (value - silver * 100));
    lua_pushnumber(L, (lua_Number) silver);
    lua_pushnumber(L, (lua_Number) gold);
    lua_pushnumber(L, (lua_Number) mithril);
    return 4;
}

/*****************************************************************************/
/* Name   : Game_PrintTimeAndDate                                            */
/* Lua    : game:PrintTimeAndDate(flags, offset)                             */
/* Info   : Returns the Arkhe time and/or date as a string, according to     */
/*          flags and offset.                                                */
/*          flags is optional. If specified, it should be some combination   */
/*          of:                                                              */
/*            game.TAD_SHOWTIME, game.TAD_SHOWDATE, game.TAD_SHOWSEASON, and */
/*            game.TAD_LONGFORM.                                             */
/*          If not specified, it defaults to all of them.                    */
/*          offset is optional. If specified it may be a number which is the */
/*          number of hours in the future or the past which the return       */
/*          represents, or a string which is parsed to arrive at a similar   */
/*          result (eg, "11 parweeks, 2 days, and 17 hours").                */
/* Return : string.                                                          */
/*****************************************************************************/
static int Game_PrintTimeAndDate(lua_State *L)
{
    lua_object    *self;
    int            flags = 0;
    const char    *string = NULL;
    sint32         offset;
    timeanddate_t  tad;

    get_lua_args(L, "G|is", &self, &flags, &string);
    offset = hooks->get_tad_offset_from_string(string);
    memset(&tad, 0, sizeof(timeanddate_t));
    hooks->get_tad(&tad, offset);

    if (flags <= 0 ||
        flags == TAD_LONGFORM)
    {
        flags = TAD_SHOWTIME | TAD_SHOWDATE | TAD_SHOWSEASON | TAD_LONGFORM;
    }

    lua_pushstring(L, hooks->print_tad(&tad, flags));
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
