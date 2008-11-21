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

/* First let's include the header file needed */
#include <global.h>
#include <daimonin_object.h>

static struct method_decl GameObject_methods[] =
{
    {"AcquireSkill",           (lua_CFunction) GameObject_AcquireSkill},
    {"AcquireSpell",           (lua_CFunction) GameObject_AcquireSpell},
    {"ActivateRune",           (lua_CFunction) GameObject_ActivateRune},
    {"AddMoney",               (lua_CFunction) GameObject_AddMoney},
    {"AddMoneyEx",             (lua_CFunction) GameObject_AddMoneyEx},
    {"AddOneDropQuest",        (lua_CFunction) GameObject_AddOneDropQuest},
    {"AddQuest",               (lua_CFunction) GameObject_AddQuest},
    {"AddQuestItem",           (lua_CFunction) GameObject_AddQuestItem},
    {"AddQuestTarget",         (lua_CFunction) GameObject_AddQuestTarget},
    {"Apply",                  (lua_CFunction) GameObject_Apply},
    {"CastSpell",              (lua_CFunction) GameObject_CastSpell},
#ifdef USE_CHANNELS
    {"ChannelMsg",             (lua_CFunction) GameObject_ChannelMsg},
#endif
    {"CheckGuild",             (lua_CFunction) GameObject_CheckGuild},
    {"CheckInstance",          (lua_CFunction) GameObject_CheckInstance},
    {"CheckInventory",         (lua_CFunction) GameObject_CheckInventory},
    {"CheckInvisibleInside",   (lua_CFunction) GameObject_CheckInvisibleInside},
    {"CheckOneDropQuest",      (lua_CFunction) GameObject_CheckOneDropQuest},
    {"CheckQuestLevel",        (lua_CFunction) GameObject_CheckQuestLevel},
    {"CheckTrigger",           (lua_CFunction) GameObject_CheckTrigger},
    {"Clone",                  (lua_CFunction) GameObject_Clone},
    {"Communicate",            (lua_CFunction) GameObject_Communicate},
    {"CreateArtifact",         (lua_CFunction) GameObject_CreateArtifact},
    {"CreateInvisibleInside",  (lua_CFunction) GameObject_CreateInvisibleInside},
    {"CreateObjectInside",     (lua_CFunction) GameObject_CreateObjectInside},
    {"CreateObjectInsideEx",   (lua_CFunction) GameObject_CreateObjectInsideEx},
    {"CreatePlayerForce",      (lua_CFunction) GameObject_CreatePlayerForce},
    {"CreatePlayerInfo",       (lua_CFunction) GameObject_CreatePlayerInfo},
    {"DecreaseNrOf",           (lua_CFunction) GameObject_DecreaseNrOf},
    {"DeleteInstance",         (lua_CFunction) GameObject_DeleteInstance},
    {"Deposit",                (lua_CFunction) GameObject_Deposit},
    {"Destruct",               (lua_CFunction) GameObject_Destruct},
    {"DoKnowSpell",            (lua_CFunction) GameObject_DoKnowSpell},
    {"Drop",                   (lua_CFunction) GameObject_Drop},
    {"FindMarkedObject",       (lua_CFunction) GameObject_FindMarkedObject},
    {"FindSkill",              (lua_CFunction) GameObject_FindSkill},
    {"Fix",                    (lua_CFunction) GameObject_Fix},
    {"GetAI",                  (lua_CFunction) GameObject_GetAI},
    {"GetAlignmentForce",      (lua_CFunction) GameObject_GetAlignmentForce},
    {"GetAnimation",           (lua_CFunction) GameObject_GetAnimation},
    {"GetArchName",            (lua_CFunction) GameObject_GetArchName},
    {"GetEquipment",           (lua_CFunction) GameObject_GetEquipment},
    {"GetFace",                (lua_CFunction) GameObject_GetFace},
    {"GetGender",              (lua_CFunction) GameObject_GetGender},
    {"GetGmasterMode",         (lua_CFunction) GameObject_GetGmasterMode},
    {"GetGod",                 (lua_CFunction) GameObject_GetGod},
    {"GetGroup",               (lua_CFunction) GameObject_GetGroup},
    {"GetGuild",               (lua_CFunction) GameObject_GetGuild},
    {"GetInvAnimation",        (lua_CFunction) GameObject_GetInvAnimation},
    {"GetInvFace",             (lua_CFunction) GameObject_GetInvFace},
    {"GetIP",                  (lua_CFunction) GameObject_GetIP},
    {"GetItemCost",            (lua_CFunction) GameObject_GetItemCost},
    {"GetMoney",               (lua_CFunction) GameObject_GetMoney},
    {"GetName",                (lua_CFunction) GameObject_GetName},
    {"GetNextPlayerInfo",      (lua_CFunction) GameObject_GetNextPlayerInfo},
    {"GetPets",                (lua_CFunction) GameObject_GetPets},
    {"GetPlayerInfo",          (lua_CFunction) GameObject_GetPlayerInfo},
    {"GetPlayerWeightLimit",   (lua_CFunction) GameObject_GetPlayerWeightLimit},
    {"GetQuest",               (lua_CFunction) GameObject_GetQuest},
    {"GetRepairCost",          (lua_CFunction) GameObject_GetRepairCost},
    {"GetSkill",               (lua_CFunction) GameObject_GetSkill},
/*  {"GetUnmodifiedAttribute", (lua_CFunction) GameObject_GetUnmodifiedAttribute}, */
    {"GetVector",              (lua_CFunction) GameObject_GetVector},
    {"IdentifyItem",           (lua_CFunction) GameObject_IdentifyItem},
    {"InsertInside",           (lua_CFunction) GameObject_InsertInside},
    {"Interface",              (lua_CFunction) GameObject_Interface},
    {"JoinGuild",              (lua_CFunction) GameObject_JoinGuild},
    {"Kill",                   (lua_CFunction) GameObject_Kill},
    {"LeaveGuild",             (lua_CFunction) GameObject_LeaveGuild},
    {"MakePet",                (lua_CFunction) GameObject_MakePet},
    {"Move",                   (lua_CFunction) GameObject_Move},
    {"NrofQuestItem",          (lua_CFunction) GameObject_NrofQuestItem},
    {"PayForItem",             (lua_CFunction) GameObject_PayForItem},
    {"PayAmount",              (lua_CFunction) GameObject_PayAmount},
    {"PickUp",                 (lua_CFunction) GameObject_PickUp},
    {"ReadyUniqueMap",         (lua_CFunction) GameObject_ReadyUniqueMap},
    {"Remove",                 (lua_CFunction) GameObject_Remove},
    {"RemoveQuestItem",        (lua_CFunction) GameObject_RemoveQuestItem},
    {"Repair",                 (lua_CFunction) GameObject_Repair},
    {"Save",                   (lua_CFunction) GameObject_Save},
    {"Say",                    (lua_CFunction) GameObject_Say},
    {"SayTo",                  (lua_CFunction) GameObject_SayTo},
    {"SendCustomCommand",      (lua_CFunction) GameObject_SendCustomCommand},
    {"SetAlignment",           (lua_CFunction) GameObject_SetAlignment},
    {"SetAnimation",           (lua_CFunction) GameObject_SetAnimation},
    {"SetFace",                (lua_CFunction) GameObject_SetFace},
    {"SetGender",              (lua_CFunction) GameObject_SetGender},
    {"SetGod",                 (lua_CFunction) GameObject_SetGod},
    {"SetInvAnimation",        (lua_CFunction) GameObject_SetInvAnimation},
    {"SetInvFace",             (lua_CFunction) GameObject_SetInvFace},
    {"SetPosition",            (lua_CFunction) GameObject_SetPosition},
    {"SetQuestStatus",         (lua_CFunction) GameObject_SetQuestStatus},
    {"SetRank",                (lua_CFunction) GameObject_SetRank},
    {"SetSaveBed",             (lua_CFunction) GameObject_SetSaveBed},
    {"SetSkill",               (lua_CFunction) GameObject_SetSkill},
    {"ShowCost",               (lua_CFunction) GameObject_ShowCost},
    {"Sound",                  (lua_CFunction) GameObject_Sound},
    {"StartNewInstance",       (lua_CFunction) GameObject_StartNewInstance},
    {"Take",                   (lua_CFunction) GameObject_Take},
    {"Withdraw",               (lua_CFunction) GameObject_Withdraw},
    {"Write",                  (lua_CFunction) GameObject_Write},

    {NULL, NULL}
};

/* All entries MUST be in same order as field_id enum above */
struct attribute_decl GameObject_attributes[] =
{
    {"below",                 FIELDTYPE_OBJECT,    offsetof(object, below),                     FIELDFLAG_READONLY, 0},
    {"above",                 FIELDTYPE_OBJECT,    offsetof(object, above),                     FIELDFLAG_READONLY, 0},
    {"inventory",             FIELDTYPE_OBJECT,    offsetof(object, inv),                       FIELDFLAG_READONLY, 0},
    {"environment",           FIELDTYPE_OBJECT,    offsetof(object, env),                       FIELDFLAG_READONLY, 0},
    {"more",                  FIELDTYPE_OBJECT,    offsetof(object, more),                      FIELDFLAG_READONLY, 0},
    {"head",                  FIELDTYPE_OBJECT,    offsetof(object, head),                      FIELDFLAG_READONLY, 0},
    {"map",                   FIELDTYPE_MAP,       offsetof(object, map),                       FIELDFLAG_READONLY, 0},
    {"count",                 FIELDTYPE_UINT32,    offsetof(object, count),                     FIELDFLAG_READONLY, 0},
    {"name",                  FIELDTYPE_SHSTR,     offsetof(object, name),                      0,                  0},
    {"title",                 FIELDTYPE_SHSTR,     offsetof(object, title),                     0,                  0},
    {"race",                  FIELDTYPE_SHSTR,     offsetof(object, race),                      0,                  0},
    {"slaying",               FIELDTYPE_SHSTR,     offsetof(object, slaying),                   0,                  0},
    {"message",               FIELDTYPE_SHSTR,     offsetof(object, msg),                       0,                  0},
    /* TODO: limited to >=0 */
    {"weight",                FIELDTYPE_SINT32,    offsetof(object, weight),                    0,                  0},
    {"weight_limit",          FIELDTYPE_UINT32,    offsetof(object, weight_limit),              0,                  0},
    {"carrying",              FIELDTYPE_SINT32,    offsetof(object, carrying),                  0,                  0},
    {"path_attuned",          FIELDTYPE_UINT32,    offsetof(object, path_attuned),              0,                  0},
    {"path_repelled",         FIELDTYPE_UINT32,    offsetof(object, path_repelled),             0,                  0},
    {"path_denied",           FIELDTYPE_UINT32,    offsetof(object, path_denied),               0,                  0},
    {"value",                 FIELDTYPE_SINT64,    offsetof(object, value),                     0,                  0},
    /* TODO: Max 100000 */
    {"quantity",              FIELDTYPE_UINT32,    offsetof(object, nrof),                      0,                  0},
    /* TODO: I don't know what these do, or if they should be accessible... */
    {"damage_round_tag",      FIELDTYPE_UINT32,    offsetof(object, damage_round_tag),          0,                  0},
    {"update_tag",            FIELDTYPE_UINT32,    offsetof(object, update_tag),                0,                  0},
    /* TODO: make enemy & owner settable (requires HOOKS for set_npc_enemy() and set_owner()) */
    {"enemy",                 FIELDTYPE_OBJECTREF, offsetof(object, enemy),                     FIELDFLAG_READONLY, offsetof(object, enemy_count)},
    /* TODO: remove    {"attacked_by",  FIELDTYPE_OBJECTREF, offsetof(object, attacked_by), FIELDFLAG_READONLY, offsetof(object, attacked_by_count)}, */
    {"owner",                 FIELDTYPE_OBJECTREF, offsetof(object, owner),                     FIELDFLAG_READONLY, offsetof(object, owner_count)},
    {"x",                     FIELDTYPE_SINT16,    offsetof(object, x),                         FIELDFLAG_READONLY, 0},
    {"y",                     FIELDTYPE_SINT16,    offsetof(object, y),                         FIELDFLAG_READONLY, 0},
    {"last_damage",           FIELDTYPE_UINT16,    offsetof(object, last_damage),               0,                  0},
    {"terrain_type",          FIELDTYPE_UINT16,    offsetof(object, terrain_type),              0,                  0},
    {"terrain_flag",          FIELDTYPE_UINT16,    offsetof(object, terrain_flag),              0,                  0},
    {"material",              FIELDTYPE_UINT16,    offsetof(object, material),                  0,                  0},
    {"material_real",         FIELDTYPE_SINT16,    offsetof(object, material_real),             0,                  0},
    {"last_heal",             FIELDTYPE_SINT16,    offsetof(object, last_heal),                 0,                  0},
    /* TODO: Limit to max 16000 ? */
    {"last_sp",               FIELDTYPE_SINT16,    offsetof(object, last_sp),                   0,                  0},
    /* TODO: Limit to max 16000 ? */
    {"last_grace",            FIELDTYPE_SINT16,    offsetof(object, last_grace),                0,                  0},
    {"last_eat",              FIELDTYPE_SINT16,    offsetof(object, last_eat),                  0,                  0},
    /* TODO: will require animation lookup function. How about face, is that a special anim? */
    {"magic",                 FIELDTYPE_SINT8,     offsetof(object, magic),                     0,                  0},
    {"state",                 FIELDTYPE_UINT8,     offsetof(object, state),                     0,                  0},
    {"level",                 FIELDTYPE_SINT8,     offsetof(object, level),                     FIELDFLAG_PLAYER_READONLY, 0},
    {"direction",             FIELDTYPE_SINT8,     offsetof(object, direction),                 0,                  0},
    {"facing",                FIELDTYPE_SINT8,     offsetof(object, facing),                    0,                  0},
    {"quick_pos",             FIELDTYPE_UINT8,     offsetof(object, quick_pos),                 0,                  0},
    {"type",                  FIELDTYPE_UINT8,     offsetof(object, type),                      FIELDFLAG_READONLY, 0},
    {"sub_type_1",            FIELDTYPE_UINT8,     offsetof(object, sub_type1),                 0,                  0},
    {"item_quality",          FIELDTYPE_UINT8,     offsetof(object, item_quality),              0,                  0},
    {"item_condition",        FIELDTYPE_UINT8,     offsetof(object, item_condition),            0,                  0},
    {"item_race",             FIELDTYPE_UINT8,     offsetof(object, item_race),                 0,                  0},
    {"item_level",            FIELDTYPE_UINT8,     offsetof(object, item_level),                0,                  0},
    {"item_skill",            FIELDTYPE_UINT8,     offsetof(object, item_skill),                0,                  0},
    {"glow_radius",           FIELDTYPE_SINT8,     offsetof(object, glow_radius),               0,                  0},
    {"anim_enemy_dir",        FIELDTYPE_SINT8,     offsetof(object, anim_enemy_dir),            0,                  0},
    {"anim_moving_dir",       FIELDTYPE_SINT8,     offsetof(object, anim_moving_dir),           0,                  0},
    {"anim_enemy_dir_last",   FIELDTYPE_SINT8,     offsetof(object, anim_enemy_dir_last),       0,                  0},
    {"anim_moving_dir_last",  FIELDTYPE_SINT8,     offsetof(object, anim_moving_dir_last),      0,                  0},
    {"anim_last_facing",      FIELDTYPE_SINT8,     offsetof(object, anim_last_facing),          0,                  0},
    {"anim_last_facing_last", FIELDTYPE_SINT8,     offsetof(object, anim_last_facing_last),     0,                  0},
    {"anim_speed",            FIELDTYPE_UINT8,     offsetof(object, anim_speed),                0,                  0},
    {"last_anim",             FIELDTYPE_UINT8,     offsetof(object, last_anim),                 0,                  0},
    {"run_away",              FIELDTYPE_UINT8,     offsetof(object, run_away),                  0,                  0},
    {"hide",                  FIELDTYPE_UINT8,     offsetof(object, hide),                      0,                  0},
    {"layer",                 FIELDTYPE_UINT8,     offsetof(object, layer),                     0,                  0},
    {"resist_impact",         FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_PHYSICAL]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_slash",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_SLASH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_cleave",         FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_CLEAVE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_pierce",         FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_PIERCE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_fire",           FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_FIRE]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_cold",           FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_COLD]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_electricity",    FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_ELECTRICITY]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_poison",         FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_POISON]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_acid",           FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_ACID]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_sonic",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_SONIC]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_magic",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_MAGIC]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_psionic",        FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_PSIONIC]),      FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_light",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_LIGHT]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_shadow",         FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_SHADOW]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_lifesteal",      FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_LIFESTEAL]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_aether",         FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_AETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_nether",         FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_NETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_chaos",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_CHAOS]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_death",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_DEATH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_weaponmagic",    FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_WEAPONMAGIC]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_godpower",       FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_GODPOWER]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_drain",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_DRAIN]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_depletion",      FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_DEPLETION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_corruption",     FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_CORRUPTION]),   FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_countermagic",   FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_COUNTERMAGIC]), FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_cancellation",   FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_CANCELLATION]), FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_confusion",      FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_CONFUSION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_fear",           FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_FEAR]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_slow",           FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_SLOW]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_paralyze",       FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_PARALYZE]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_snare",          FIELDTYPE_SINT8,     offsetof(object, resist[ATNR_SNARE]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_impact",         FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_PHYSICAL]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_slash",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_SLASH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_cleave",         FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_CLEAVE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_pierce",         FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_PIERCE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_fire",           FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_FIRE]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_cold",           FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_COLD]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_electricity",    FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_ELECTRICITY]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_poison",         FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_POISON]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_acid",           FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_ACID]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_sonic",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_SONIC]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_magic",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_MAGIC]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_psionic",        FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_PSIONIC]),      FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_light",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_LIGHT]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_shadow",         FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_SHADOW]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_lifesteal",      FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_LIFESTEAL]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_aether",         FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_AETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_nether",         FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_NETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_chaos",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_CHAOS]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_death",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_DEATH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_weaponmagic",    FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_WEAPONMAGIC]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_godpower",       FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_GODPOWER]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_drain",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_DRAIN]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_depletion",      FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_DEPLETION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_corruption",     FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_CORRUPTION]),   FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_countermagic",   FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_COUNTERMAGIC]), FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_cancellation",   FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_CANCELLATION]), FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_confusion",      FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_CONFUSION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_fear",           FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_FEAR]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_slow",           FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_SLOW]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_paralyze",       FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_PARALYZE]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_snare",          FIELDTYPE_SINT8,     offsetof(object, attack[ATNR_SNARE]),        FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: -10.0 < speed < 10.0, also might want to call update_object_speed() */
    {"speed",                 FIELDTYPE_FLOAT,     offsetof(object, speed),                     FIELDFLAG_PLAYER_READONLY, 0},
    {"speed_left",            FIELDTYPE_FLOAT,     offsetof(object, speed_left),                0,                  0},
    {"weapon_speed",          FIELDTYPE_FLOAT,     offsetof(object, weapon_speed),              0,                  0},
    {"weapon_speed_left",     FIELDTYPE_FLOAT,     offsetof(object, weapon_speed_left),         0,                  0},
    /* Stats */
    {"experience",            FIELDTYPE_SINT32,    offsetof(object, stats.exp),                 0,                  0},
    {"hitpoints",             FIELDTYPE_SINT32,    offsetof(object, stats.hp),                  0,                  0},
    {"max_hitpoints",         FIELDTYPE_SINT32,    offsetof(object, stats.maxhp),               FIELDFLAG_PLAYER_READONLY, 0},
    {"spellpoints",           FIELDTYPE_SINT16,    offsetof(object, stats.sp),                  0,                  0},
    {"max_spellpoints",       FIELDTYPE_SINT16,    offsetof(object, stats.maxsp),               FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to +- 16000 ? */
    {"grace",                 FIELDTYPE_SINT16,    offsetof(object, stats.grace),               0,                  0},
    {"max_grace",             FIELDTYPE_SINT16,    offsetof(object, stats.maxgrace),            FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to max 999 (at least for players) ? */
    {"food",                  FIELDTYPE_SINT16,    offsetof(object, stats.food),                0,                  0},
    /* TODO: Limit to 0 <= dam <= 120 ? */
    {"damage",                FIELDTYPE_SINT16,    offsetof(object, stats.dam),                 FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to +-120 */
    {"weapon_class",          FIELDTYPE_SINT16,    offsetof(object, stats.wc),                  FIELDFLAG_READONLY, 0},
    /* TODO: Limit to +-120 */
    {"armour_class",          FIELDTYPE_SINT16,    offsetof(object, stats.ac),                  FIELDFLAG_READONLY, 0},
    /* TODO: Limit to +-30 (all  */
    {"strength",              FIELDTYPE_SINT8,     offsetof(object, stats.Str),                 FIELDFLAG_PLAYER_FIX, 0},
    {"dexterity",             FIELDTYPE_SINT8,     offsetof(object, stats.Dex),                 FIELDFLAG_PLAYER_FIX, 0},
    {"constitution",          FIELDTYPE_SINT8,     offsetof(object, stats.Con),                 FIELDFLAG_PLAYER_FIX, 0},
    {"wisdom",                FIELDTYPE_SINT8,     offsetof(object, stats.Wis),                 FIELDFLAG_PLAYER_FIX, 0},
    {"charisma",              FIELDTYPE_SINT8,     offsetof(object, stats.Cha),                 FIELDFLAG_PLAYER_FIX, 0},
    {"intelligence",          FIELDTYPE_SINT8,     offsetof(object, stats.Int),                 FIELDFLAG_PLAYER_FIX, 0},
    {"power",                 FIELDTYPE_SINT8,     offsetof(object, stats.Pow),                 FIELDFLAG_PLAYER_FIX, 0},
    {"thac0",                 FIELDTYPE_SINT8,     offsetof(object, stats.thac0),               FIELDFLAG_PLAYER_READONLY, 0},
    {"thacm",                 FIELDTYPE_SINT8,     offsetof(object, stats.thacm),               FIELDFLAG_PLAYER_READONLY, 0},

    {NULL, 0, 0, 0, 0}
};

/* This is a list of strings that correspond to the FLAG_.. values.
 * This is a simple 1:1 mapping - if FLAG_FRIENDLY is 15, then
 * the 15'th element of this array should match that name.
 * If an entry is NULL, that flag cannot be set/read from scripts
 * If an entry begins with "?", that flag is read-only
 * Yes, this is almost exactly a repeat from loader.c
 */
static const char *GameObject_flags[NUM_FLAGS + 1 + 1] =
{
    "f_sleep",
    "f_confused",
    "?f_paralyzed",
    "f_scared",
    "f_is_eating",
    "f_is_invisible",
    "f_is_ethereal",
    "f_is_good",
    "f_no_pick",
    "f_walk_on",

    /* 10 */
    "f_no_pass",
    "f_is_animated",
    NULL,
    "f_flying",
    "f_monster",
    "f_friendly",
    "?f_is_removed",
    "f_been_applied",
    /* internal flag: HAS_MOVED */ NULL,
    "f_treasure",

    /* 20 */
    "f_is_neutral",
    "f_see_invisible",
    "f_can_roll",
    "f_generator",
    "f_is_turnable",
    "f_walk_off",
    "f_fly_on",
    "f_fly_off",
    "f_is_used_up",
    "f_identified",

    /* 30 */
    "f_reflecting",
    "f_changing",
    "f_splitting",
    "f_hitback",
    "f_startequip",
    "f_blocksview",
    "f_undead",
    "f_fix_player",
    "f_unaggressive",
    "f_reflect_missile",

    /* 40 */
    "f_reflect_spell",
    "f_no_magic",
    "f_no_fix_player",
    "f_is_evil",
    "f_tear_down",
    "f_run_away",
    "f_pass_thru",
    "f_can_pass_thru",
    "?f_feared",
    "f_is_blind",

    /* 50 */
    "f_no_drop",
    "f_reg_f",
    "f_has_ready_spell",
    "f_surrendered",
    "?f_rooted",
    "?f_slowed",
    "f_can_use_armour",
    "f_can_use_weapon",
    "f_can_use_ring",
    /* unused */ NULL,

    /* 60 */
    "f_has_ready_bow",
    "f_xrays",
    "?f_no_apply",
    "f_can_stack",
    "f_lifesave",
    "f_is_magical",
    "f_alive",
    "f_stand_still",
    "f_random_move",
    "f_only_attack",

    /* 70 */
    "?f_wiz",
    "f_stealth",
    "?f_wizpass",
    "?f_is_linked",
    "f_cursed",
    "f_damned",
    "f_see_anywhere",
    "f_known_magical",
    "f_known_cursed",
    "f_can_use_skill",

    /* 80 */
    "f_is_thrown",
    NULL,
    NULL,
    "f_is_male",
    "f_is_female",
    "f_applied",
    "f_inv_locked",
    "f_is_wooded",
    "f_is_hilly",
    "f_levitate",

    /* 90 */
    "f_has_ready_weapon",
    "f_no_skill_ident",
    "f_use_dmg_info",
    "f_can_see_in_dark",
    "f_is_cauldron",
    "f_is_dust",
    "f_no_steal",
    "f_one_hit",
     /* debug flag CLIENT_SENT */ NULL,
    "f_berserk",

    /* 100 */
    "f_no_attack",
    "f_invulnerable",
    "f_quest_item",
    "f_is_traped",
    "f_proof_phy",
    "f_proof_ele",
    "f_proof_mag",
    "f_proof_sph",
    "f_no_inv",
    NULL,

    /* 110 */
    "f_sys_object",
    NULL,
    "f_unpaid",
    "f_is_aged",
    "f_make_invisible",
    "f_make_ethereal",
    "f_is_player",
    "f_is_named",
    "?f_spawn_mob_flag",
    "f_no_teleport",

    /* 120 */
    "f_corpse",
    "f_corpse_forced",
    "f_player_only",
    "f_no_cleric",
    "f_one_drop",
    "f_cursed_perm",
    "f_damned_perm",
    "f_door_closed",
    "f_was_reflected",
    "f_is_missile",

    /* 130 */
    "f_can_reflect_missile",
    "f_can_reflect_spell",
    "f_is_assassin",
    "f_auto_apply",
    "?f_no_save",
    "f_pass_ethereal",
    "f_ego",
    "f_egobound",
    "f_egoclan",
    "f_egolock",

    FLAGLIST_END_MARKER
};

/* This gets called before and after an attribute has been set in an object */
static int GameObject_setAttribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib, int before)
{
    object *who = obj->data.object;

#if 0
    /* Pre-setting hook -- is this necessary? */
    if (before)
        ;
#endif

    /* update player inv when needed */
    hooks->esrv_send_item(hooks->is_player_inv(who), who);

    /* Special handling for some player stuff */
    if (who->type == PLAYER)
    {
        if (attrib->offset == offsetof(object, stats.Int))
            CONTR(who)->orig_stats.Int = (sint8) lua_tonumber(L, -1);
        else if (attrib->offset == offsetof(object, stats.Str))
            CONTR(who)->orig_stats.Str = (sint8) lua_tonumber(L, -1);
        else if (attrib->offset == offsetof(object, stats.Cha))
            CONTR(who)->orig_stats.Cha = (sint8) lua_tonumber(L, -1);
        else if (attrib->offset == offsetof(object, stats.Wis))
            CONTR(who)->orig_stats.Wis = (sint8) lua_tonumber(L, -1);
        else if (attrib->offset == offsetof(object, stats.Dex))
            CONTR(who)->orig_stats.Dex = (sint8) lua_tonumber(L, -1);
        else if (attrib->offset == offsetof(object, stats.Con))
            CONTR(who)->orig_stats.Con = (sint8) lua_tonumber(L, -1);
        else if (attrib->offset == offsetof(object, stats.Pow))
            CONTR(who)->orig_stats.Pow = (sint8) lua_tonumber(L, -1);

        if (attrib->flags & FIELDFLAG_PLAYER_FIX)
            hooks->FIX_PLAYER(who, "LUA - set attribute");
    }

    return 0;
}

/* value is on top of stack */
static int GameObject_setFlag(lua_State *L, lua_object *obj, uint32 flagno)
{
    int     value;

    if (lua_isnumber(L, -1))
        value = (int) lua_tonumber(L, -1);
    else
        value = lua_toboolean(L, -1);

    if (value)
        SET_FLAG(obj->data.object, flagno);
    else
        CLEAR_FLAG(obj->data.object, flagno);

    hooks->esrv_send_item(hooks->is_player_inv(obj->data.object), obj->data.object);

    /* TODO: if gender changed:
    if()
       CONTR(WHO)->socket.ext_title_flag = 1; * demand update to client */

    return 0;
}

/* pushes flag on top of stack */
static int GameObject_getFlag(lua_State *L, lua_object *obj, uint32 flagno)
{
    lua_pushboolean(L, QUERY_FLAG(obj->data.object, flagno));
    return 1;
}

/* toString method for GameObjects */
static int GameObject_toString(lua_State *L)
{
    lua_object *obj = lua_touserdata(L, 1);

    if (obj && obj->class->type == LUATYPE_OBJECT)
        lua_pushfstring(L, "%s [%d] ", STRING_OBJ_NAME(obj->data.object), obj->data.object->count);
    else
        luaL_error(L, "Not an object");

    return 1;
}

/* Tests if an object is valid */
static int GameObject_isValid(lua_State *L, lua_object *obj)
{
    return obj->data.object->count == obj->tag;
}

lua_class GameObject  =
{
    LUATYPE_OBJECT,
    "GameObject",
    0,
    GameObject_toString,
    GameObject_attributes,
    GameObject_methods,
    NULL,
    GameObject_flags,
    GameObject_getFlag,
    GameObject_setFlag,
    GameObject_setAttribute,
    GameObject_isValid,
    0
};

int GameObject_init(lua_State *L)
{
    init_class(L, &GameObject);

    return 0;
}


/****************************************************************************/
/*                          GameObject methods                              */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */
/*****************************************************************************/
/* Name   : GameObject_SetPosition                                           */
/* Lua    : object:SetPosition(map, x, y, flags)                             */
/* Info   : Teleports op to x,y of object.map or map (when given).           */
/*          WARNING: a script developer must have in mind that SetPosition() */
/*          can result in the destruction of the transferred object. The     */
/*          return value is important to check!                              */
/*          flags can be some combinations of:                               */
/*            game.MFLAG_FIXED_POS - insert on x,y EVEN if the spot not free */
/*            game.MFLAG_RANDOM_POS - insert on a free random spot near x,y  */
/*            game.MFLAG_FREE_POS_ONLY - only insert on a free position,     */
/*            return with fail when there is no free spot                    */
/*          Examples:                                                        */
/*          obj:SetPosition(x, y) - same as obj:SetPosition(obj.map, x,y)    */
/*          obj:SetPosition(game:ReadyMap("/a_map"), x, y) - multiplayer map */
/*          obj:SetPosition(obj:StartNewInstance("/another_map"), x, y)      */
/*          obj:SetPosition(obj.map:ReadyInheritedMap("/map_2"), x, y)       */
/* Return : 0: all ok, 1: object was destroyed, 2: insertion failed (map or  */
/*          position error, ...)                                             */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_SetPosition(lua_State *L)
{
    int         x, y, flags=0, ret=0;
    lua_object *self, *where;
    mapstruct *new_map = NULL;

    /* Small hack to allow optional first map parameter */
    if(lua_isuserdata(L, 2))
    {
        get_lua_args(L, "OMii|i", &self, &where, &x, &y, &flags);
        new_map = where->data.map;
    }
    else
    {
        get_lua_args(L, "Oii|i", &self, &x, &y, &flags);
        new_map = WHO->map;
        if(new_map == NULL)
            luaL_error(L, "Short-form of SetPosition() used, but the object didn't have a map");
    }

    ret = hooks->enter_map(WHO, NULL, new_map, x, y, flags);

    lua_pushnumber(L, ret);
    return 1;
}


/*****************************************************************************/
/* Name   : GameObject_ReadyUniqueMap                                        */
/* Lua    : object:ReadyUniqueMap(map_path,flags)                            */
/* Info   : Loads the unique map from map_path into memory for object, unless*/
/*          already loaded.                                                  */
/*          Also, creates a new persistant unique map if needed.             */
/*          Only players can have unique maps associated to them.            */
/*          See also map:ReadyInheritedMap(), object:StartNewInstance() and  */
/*          game:ReadyMap()                                                  */
/*          flags:                                                           */
/*            game.MAP_CHECK - don't load the map if it isn't in memory,     */
/*                             returns nil if the map wasn't in memory.      */
/*            game.MAP_NEW - delete the map from memory and force a reset    */
/*                           (if it existed in memory or swap)               */
/* Return : map pointer to unique map, or nil                                */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_ReadyUniqueMap(lua_State *L)
{
    lua_object *self;
    char const * path_sh, *orig_path_sh = NULL;
    char       *mapname;
    mapstruct  *map = NULL;
    int flags = 0;

    get_lua_args(L, "Os|i", &self, &mapname, &flags);

    if(WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "ReadyUniqueMap() must be called on a legal player object.");

    /* mapname must point to original map in /path - we will generate our
     * unique map name itself
     */
    orig_path_sh = hooks->create_safe_mapname_sh(mapname);

    if(orig_path_sh)
    {
        path_sh = hooks->create_unique_path_sh(WHO, orig_path_sh);
        map = hooks->ready_map_name(path_sh, NULL, 0, WHO->name);

        if(map && flags & PLUGIN_MAP_NEW) /* reset the maps - when it loaded */
        {
            int num = 0;

            if(map->player_first)
                num = hooks->map_to_player_unlink(map); /* remove player from map */

            hooks->clean_tmp_map(map); /* remove map from memory */
            hooks->delete_map(map);

            /* reload map forced from original /maps */
            map = hooks->ready_map_name(path_sh, orig_path_sh, MAP_STATUS_UNIQUE, WHO->name);

            if(num) /* and kick player back to map - note: if map is NULL its bind point redirect */
                hooks->map_to_player_link(map, -1, -1, FALSE);
        }
        else if (!(flags & PLUGIN_MAP_CHECK))/* normal ready_map_name() with checking loaded & original maps */
            map = hooks->ready_map_name(path_sh, orig_path_sh, MAP_STATUS_UNIQUE, WHO->name);

        FREE_ONLY_HASH(path_sh);
    }

    FREE_ONLY_HASH(orig_path_sh);

    return push_object(L, &Map, map);
}

/*****************************************************************************/
/* Name   : GameObject_StartNewInstance                                      */
/* Lua    : object:StartNewInstance(entrance_path,flag)                      */
/* Info   : Reload or creates an instance for a player                       */
/* Info   : Reloads or creates an instance for a player based on the map     */
/*          loaded from entrance_path.                                       */
/*          See also object:ReadyUniqueMap(), object:StartNewInstance() and  */
/*          game:ReadyMap()                                                  */
/*          flags:                                                           */
/*            game.MAP_CHECK - don't load or create the instance if it isn't */
/*                             active. Returns nil if instance is invalid.   */
/*            game.MAP_NEW - always create a new instance and delete any     */
/*                           instance the player had active.                 */
/*          NOTE 1: resetting an instance does not simply reset the maps, it */
/*                  creates a wholly new instance for the player.            */
/*          NOTE 2: a player can only have a single active instance. If a    */
/*                  second instance is loaded, the first one will be reset.  */
/*          NOTE 3: an instance is identified by the player and entrance map.*/
/*                  To simulate multiple entrance maps for a single instance,*/
/*                  always first load the entrance map, then load a secondary*/
/*                  entrance with ReadyInheritedMap()                        */
/* Return : map pointer to the entrance map of the instance, or nil          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_StartNewInstance(lua_State *L)
{
    lua_object *self;
    char const * path_sh=NULL, *orig_path_sh = NULL;
    char       *mapname;
    player     *pl;
    mapstruct  *map = NULL;
    int         flags = 0, iflag;

    get_lua_args(L, "Os|ii", &self, &mapname, &iflag, &flags);

    /* we only allow the creation of instance maps in context with players */
    if(WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "StartNewInstance(): Only players can have instances.");

    /* this is a bit critical. Be SURE you call it with a valid, normalized instance
     * name because this is put also as instance ID in the player struct.
     */
    if(! (orig_path_sh = hooks->create_safe_mapname_sh(mapname)))
        luaL_error(L, "Illegal map path: %s", mapname);

    /* ensure only valid flags */
    iflag &= INSTANCE_FLAG_NO_REENTER;

    pl = CONTR(WHO);

    /* lets check we have a instance we can reenter */
    if(!(flags & PLUGIN_MAP_NEW))
    {
        /* the instance data are inside the player struct */
        if( pl->instance_name == orig_path_sh &&
            pl->instance_id == *hooks->global_instance_id &&
            pl->instance_num != MAP_INSTANCE_NUM_INVALID &&
            !(pl->instance_flags & INSTANCE_FLAG_NO_REENTER))
        {
            path_sh = hooks->create_instance_path_sh(pl, orig_path_sh, iflag);
        }
    }

    /* no path? force a new instance... note that create_instance_path_sh() will setup the
    * player struct data automatically when creating the path data
    */
    if(!path_sh && !(flags & PLUGIN_MAP_CHECK))
    {
        pl->instance_num = MAP_INSTANCE_NUM_INVALID; /* will force a new instance */
        path_sh = hooks->create_instance_path_sh(pl, orig_path_sh, iflag);
    }

    /* we have now declared and initilized the new instance - now lets see we can load it! */
    if(path_sh)
    {
        map = hooks->ready_map_name(path_sh, orig_path_sh, MAP_STATUS_INSTANCE, WHO->name);
        /* we don't mark the instance invalid when ready_map_name() fails to create
        * a physical map - we let do it the calling script which will know it
        * by checking the return value = NULL
        */
        FREE_ONLY_HASH(path_sh);
    }

    FREE_ONLY_HASH(orig_path_sh);

    return push_object(L, &Map, map);
}

/*****************************************************************************/
/* Name   : GameObject_CheckInstance                                         */
/* Lua    : object:CheckInstance(entrance_path)                              */
/* Info   : Check player has this instance active                            */
/*          Only checks the player instance data match - NO map loading      */
/* Return : returns true if player had the instance active                   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_CheckInstance(lua_State *L)
{
    lua_object *self;
    char const *orig_path_sh = NULL;
    char       *mapname;
    int         ret = 0;

    get_lua_args(L, "Os", &self, &mapname);

    /* we only allow the creation of instance maps in context with players */
    if(WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "CheckInstance(): Only players can have instances.");

    if(! (orig_path_sh = hooks->create_safe_mapname_sh(mapname)))
        luaL_error(L, "Illegal map path: %s", mapname);

    LOG(llevDebug, "pl->instance_name: %s, orig_path_sh: %s\n", CONTR(WHO)->instance_name, orig_path_sh);
    LOG(llevDebug, "pl->instance_id: %ld, global_instance_id: %ld\n", CONTR(WHO)->instance_id, *hooks->global_instance_id);
    LOG(llevDebug, "pl->instance_num: %ld\n", CONTR(WHO)->instance_num);

    /* the instance data are inside the player struct */
    if( CONTR(WHO)->instance_name == orig_path_sh &&
            CONTR(WHO)->instance_id == *hooks->global_instance_id &&
            CONTR(WHO)->instance_num != MAP_INSTANCE_NUM_INVALID)
    {
        ret = 1;
    }

    FREE_ONLY_HASH(orig_path_sh);

    lua_pushboolean(L, ret);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_DeleteInstance                                        */
/* Lua    : object:DeleteInstance(entrance_path)                             */
/* Info   : Delete the instance for a player                                 */
/*          NOTE: this function don't touches the instance directory or      */
/*          deletes a map - it only removes the ID tags from the player      */
/*          IF mapname is the same as the player saved instance              */
/* Return : returns true if player had the instance active we have deleted   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_DeleteInstance(lua_State *L)
{
    lua_object *self;
    char const *orig_path_sh = NULL;
    char       *mapname;
    int         ret = 0;

    get_lua_args(L, "Os", &self, &mapname);

    /* we only allow the creation of instance maps in context with players */
    if(WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "DeleteInstance(): Only players can have instances.");

    if(! (orig_path_sh = hooks->create_safe_mapname_sh(mapname)))
        luaL_error(L, "Illegal map path: %s", mapname);

    /* the instance data are inside the player struct */
    if( CONTR(WHO)->instance_name == orig_path_sh &&
            CONTR(WHO)->instance_id == *hooks->global_instance_id &&
            CONTR(WHO)->instance_num != MAP_INSTANCE_NUM_INVALID)
    {
        /* lets do it right: instance_num to MAP_INSTANCE_NUM_INVALID
         * and releasing the instance_name
         */
        hooks->reset_instance_data(CONTR(WHO));
        ret = 1;
    }

    FREE_ONLY_HASH(orig_path_sh);

    lua_pushboolean(L, ret);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_CreateArtifact                                        */
/* Lua    : object:CreateArtifact(base_obj, artifact_mask)                   */
/* Info   : Create an artifact = apply a artifact mask to an object          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_CreateArtifact(lua_State *L)
{
    char *name;
    artifact *art;
    lua_object *self, *whatptr;

    get_lua_args(L, "OOs", &self, &whatptr, &name);

    art = hooks->find_artifact(name);
    if(art == NULL)
    {
        return 0;
    }
    else
    {
        /* WHAT itself is implicit changed */
        hooks->give_artifact_abilities(WHAT, art);
        return push_object(L, &GameObject, WHAT);
    }
}

/*****************************************************************************/
/* Name   : GameObject_GetName                                               */
/* Lua    : object:GetName(owner)                                            */
/* Info   : same as query_short_name()                                       */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetName(lua_State *L)
{
    lua_object     *self, *owner = NULL;
    object         *obj;
    static char    *result;

    get_lua_args(L, "O|O", &self, &owner);

    if(owner)
        obj = owner->data.object;
    else
        obj = self->data.object;

    result = hooks->query_short_name(WHO, obj);

    lua_pushstring(L, result);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetEquipment                                          */
/* Lua    : object:GetEquipment(slot)                                        */
/* Info   : Get a player's current equipment for a given slot. slot must be  */
/*          one of the Game.EQUIP_xxx constants, e.g. Game.EQUIP_GAUNTLET    */
/*          If the selected slot is empty, this method will return nil.      */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_GetEquipment(lua_State *L)
{
    int num;
    lua_object *self;

    get_lua_args(L, "Oi", &self, &num);

    if(WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "GetEquipment(): Only works for players.");

    if(num < 0 || num >= PLAYER_EQUIP_MAX)
        luaL_error(L, "GetEquipment(): illegal slot number: %d\n", num);

    return push_object(L, &GameObject, CONTR(WHO)->equipment[num]);
}

/*****************************************************************************/
/* Name   : GameObject_GetRepairCost                                         */
/* Lua    : object:GetRepairCost()                                           */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetRepairCost(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    /* the sint64 to double cast should be ok - can't think about that high repair costs */
    lua_pushnumber(L, (double)hooks->material_repair_cost(WHO, NULL));
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Repair                                                */
/* Lua    : object:Repair(skill_nr)                                          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Repair(lua_State *L)
{
    int skill = 100;
    lua_object *self;
    object *tmp;

    get_lua_args(L, "O|i", &self, &skill);

    hooks->material_repair_item(WHO, skill);

    if((tmp = hooks->is_player_inv(WHO)))
    {
        SET_FLAG(tmp, FLAG_FIX_PLAYER);
        hooks->esrv_update_item(UPD_QUALITY, tmp, WHO);
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Sound                                                 */
/* Lua    : object:Sound(x, y, soundnumber, soundtype)                       */
/* Info   : play the sound for the given player only, sounding like it comes */
/*          from the given x,y coordinates.                                  */
/*          If soundtype is game.SOUNDTYPE_NORMAL (the default), then        */
/*          soundnumber should be one of the game.SOUND_xxx constants        */
/*          If soundtype is game.SOUNDTYPE_SPELL, then the sound number      */
/*          should be a spell number, to play the sound of that spell        */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_Sound(lua_State *L)
{
    int         x, y, soundnumber, soundtype = SOUND_NORMAL;
    lua_object *self;

    get_lua_args(L, "Oiii|i", &self, &x, &y, &soundnumber, &soundtype);

    hooks->play_sound_player_only(CONTR(WHO),soundnumber,soundtype, x, y);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_Interface                                             */
/* Lua    : object:Interface(mode, message)                                  */
/* Info   : This function opens a NPC gui interface on the client            */
/*          A mode of -1 means to close explicit a open interface at client  */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_Interface(lua_State *L)
{
    lua_object *self;
    char       *txt;
    int         mode;

    get_lua_args(L, "Oi|s", &self, &mode, &txt);

    hooks->gui_interface(WHO, mode, txt, NULL);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetSkill                                              */
/* Lua    : object:GetSkill(type, id)                                        */
/* Info   : This function will fetch a skill or exp_skill object             */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetSkill(lua_State *L)
{
    object     *tmp;
    int         type, id;
    lua_object *self;

    get_lua_args(L, "Oii", &self, &type, &id);

    /* Browse the inventory of object to find a matching skill or exp_obj. */
    for (tmp = WHO->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == type)
        {
            if (tmp->type == SKILL)
            {
                if (tmp->stats.sp == id)
                    return push_object(L, &GameObject, tmp);
            }
            else if (tmp->type == EXPERIENCE)
            {
                if (tmp->sub_type1 == id)
                    return push_object(L, &GameObject, tmp);
            }
        }
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetSkill                                              */
/* Lua    : object:SetSkill(type, skillid, level, value)                     */
/* Info   : Sets objects's experience in the skill skillid as close to value */
/*          as permitted. There is currently a limit of 1/4 of a level.      */
/*          There's no limit on exp reduction.                               */
/*          FIXME overall experience is not changed (should it be?)          */
/*          FIXME need updated documentation                                 */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetSkill(lua_State *L)
{
    object     *tmp;
    int         type, skill, value, level, currentxp;
    lua_object *self;

    get_lua_args(L, "Oiiii", &self, &type, &skill, &level, &value);

    /* atm we don't set anything in exp_obj types */
    if (type != SKILL)
        return 0;

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    /* Browse the inventory of object to find a matching skill. */
    for (tmp = WHO->inv; tmp; tmp = tmp->below)
    {
        if (tmp->type == type && tmp->stats.sp == skill)
        {
            /* this is a bit tricky: some skills are marked with exp
                     * -1 or -2 as special used skills (level but no exp):
                     * if we have here a level > 0, we set level but NEVER
                     * exp ... if we have level == 0, we only set exp - the
                     * addexp
                     */
            /*LOG(-1,"LEVEL1 %d (->%d) :: %s (exp %d)\n",tmp->level,level,STRING_OBJ_NAME(tmp), tmp->stats.exp);*/
            if (level > 0)
            {
                tmp->level = level;
            }
            else
            {
                /* Gecko: Changed to use actual skill experience */
                CFParm CFP;
                currentxp = tmp->stats.exp;
                value = value - currentxp;

                CFP.Value[0] = (void *) (WHO);
                CFP.Value[1] = (void *) (&value);
                CFP.Value[2] = (void *) (&skill);
                (PlugHooks[HOOK_ADDEXP]) (&CFP);
            }
            /*LOG(-1,"LEVEL2 %d (->%d) :: %s (exp %d)\n",tmp->level,level,STRING_OBJ_NAME(tmp), tmp->stats.exp);*/
            if (WHO->type == PLAYER && CONTR(WHO))
                CONTR(WHO)->update_skills = 1; /* we will sure change skill exp, mark for update */

            return 0;
        }
    }

    luaL_error(L, "Unknown skill");
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_ActivateRune                                          */
/* Lua    : object:ActivateRune(what)                                        */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_ActivateRune(lua_State *L)
{
    lua_object *whatptr;
    lua_object *self;

    get_lua_args(L, "OO", &self, &whatptr);

    hooks->spring_trap(WHAT, WHO);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_CheckTrigger                                          */
/* Lua    : object:CheckTrigger(what)                                        */
/* Status : Unfinished                                                       */
/*****************************************************************************/
/* MUST DO THE HOOK HERE ! */
static int GameObject_CheckTrigger(lua_State *L)
{
    lua_object *whatptr;
    lua_object *self;

    get_lua_args(L, "OO", &self, &whatptr);

    luaL_error(L, "Unfinished function");

    /* check_trigger(WHAT,WHO); should be hook too! */

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetGod                                                */
/* Lua    : object:GetGod()                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static int GameObject_GetGod(lua_State *L)
{
    CFParm         *CFR, CFP;
    static char    *value;
    lua_object     *self;
    get_lua_args(L, "O", &self);

    CFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_DETERMINEGOD]) (&CFP);
    value = (char *) (CFR->Value[0]);
    free(CFR);

    lua_pushstring(L, value);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SetGod                                                */
/* Lua    : object:SetGod(godname)                                           */
/* Status : Unfinished!                                                      */
/*****************************************************************************/
static int GameObject_SetGod(lua_State *L)
{
    char       *txt;
    const char *prayname = NULL;
    object     *tmp;
    CFParm     *CFR0;
    CFParm     *CFR;
    CFParm      CFP0, CFP1, CFP2;
    int         value;
    lua_object *self;

    get_lua_args(L, "Os", &self, &txt);

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    FREE_AND_COPY_HASH(prayname, "praying");

    CFP1.Value[0] = (void *) (WHO);
    CFP1.Value[1] = (void *) (prayname);

    CFP2.Value[0] = (void *) (WHO);
    CFP0.Value[0] = (char *) (txt);
    CFR0 = (PlugHooks[HOOK_FINDGOD]) (&CFP0);
    tmp = (object *) (CFR0->Value[0]);
    free(CFR0);
    CFP2.Value[1] = (void *) (tmp);

    CFR = (PlugHooks[HOOK_CMDRSKILL]) (&CFP1);
    value = *(int *) (CFR->Value[0]);
    if (value)
        (PlugHooks[HOOK_BECOMEFOLLOWER]) (&CFP2);
    free(CFR);

    FREE_ONLY_HASH(prayname);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_InsertInside                                          */
/* Lua    : object:InsertInside(where)                                       */
/* Info   : Inserts object into where.                                       */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_InsertInside(lua_State *L)
{
    lua_object *whereptr;
    object     *myob, *tmp, *obenv;
    lua_object *self;

    get_lua_args(L, "OO", &self, &whereptr);

    myob = self->data.object;
    obenv = myob->env;

    if (!QUERY_FLAG(myob, FLAG_REMOVED))
    {
        CFParm CFP;
        CFP.Value[0] = (void *) (myob);
        (PlugHooks[HOOK_REMOVEOBJECT]) (&CFP);
    }

    myob = hooks->insert_ob_in_ob(myob, WHERE);
    hooks->esrv_send_item(hooks->is_player_inv(WHERE), myob);

    if((tmp = hooks->is_player_inv(obenv)))
        hooks->esrv_send_inventory(tmp, tmp);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Apply                                                 */
/* Lua    : object:Apply(what, flags)                                        */
/* Info   : forces object to apply what.                                     */
/*          flags should be a reasonable combination of the following:       */
/*          Daimonin.APPLY_TOGGLE - normal apply (toggle)                    */
/*          Daimonin.APPLY_ALWAYS - always apply (never unapply)             */
/*          Daimonin.UNAPPLY_ALWAYS - always unapply (never apply)           */
/*          Daimonin.UNAPPLY_NOMERGE - don't merge unapplied items           */
/*          Daimonin.UNAPPLY_IGNORE_CURSE - unapply cursed items             */
/*          returns: 0 - object cannot apply objects of that type.           */
/*                   1 - object was applied, or not...                       */
/*                   2 - object must be in inventory to be applied           */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Apply(lua_State *L)
{
    lua_object *whatptr;
    int         flags;
    CFParm     *CFR, CFP;
    int         retval;
    lua_object *self;

    get_lua_args(L, "OOi", &self, &whatptr, &flags);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (WHAT);
    CFP.Value[2] = (void *) (&flags);
    CFR = (PlugHooks[HOOK_MANUALAPPLY]) (&CFP);
    retval = *(int *) (CFR->Value[0]);
    free(CFR);

    lua_pushnumber(L, retval);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PickUp                                                */
/* Lua    : object:PickUp(what)                                              */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_PickUp(lua_State *L)
{
    lua_object *whatptr, *self;
    CFParm CFP;

    get_lua_args(L, "OO", &self, &whatptr);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (WHAT);
    (PlugHooks[HOOK_PICKUP]) (&CFP);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_Drop                                                  */
/* Lua    : object:Drop(what)                                                */
/* Info   : Equivalent to the player command "drop" (name is an object name, */
/*          "all", "unpaid", "cursed", "unlocked" or a count + object name : */
/*          "<nnn> <object name>", or a base name, or a short name...)       */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_Drop(lua_State *L)
{
    char       *name;
    CFParm     *CFR, CFP;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (name);
    CFR = (PlugHooks[HOOK_CMDDROP]) (&CFP);
    free(CFR);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Take                                                  */
/* Lua    : object:Take(name)                                                */
/* Status : Temporary disabled (see commands.c)                              */
/*****************************************************************************/

static int GameObject_Take(lua_State *L)
{
    char       *name;
    CFParm     *CFR, CFP;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (name);
    CFR = (PlugHooks[HOOK_CMDTAKE]) (&CFP);
    free(CFR);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_Deposit                                               */
/* Lua    : object:Deposit(deposit_object, string)                           */
/* Info   : deposit value or string money from object in deposit_object.     */
/*          Control first object has that amount of money, then remove it    */
/*          from object and add it in ->value of deposit_object.             */
/* Returns : -1 if there was a syntax error, 0 if there wasn't enough money and 1 if it succeeded */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Deposit(lua_State *L)
{
    lua_object *obptr;
    char       *text;
    object     *bank;
    int         val=1, pos=0;
    lua_object *self;
    _money_block money;

    get_lua_args(L, "OOs", &self, &obptr, &text);

    bank = obptr->data.object;

    hooks->get_word_from_string(text, &pos);
    hooks->get_money_from_string(text + pos, &money);

    if (!money.mode)
    {
        val = -1;
        hooks->new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, WHO, "deposit what?\nUse 'deposit all' or 'deposit 40 gold, 20 silver...'");
    }
    else if (money.mode == MONEYSTRING_ALL)
    {
        bank->value += hooks->remove_money_type(WHO, WHO, -1, 0);
        hooks->FIX_PLAYER(WHO, "LUA: deposit - remove money");
    }
    else
    {
        /* Changed deposition code to use the standard
         * payment calls. This means you can deposit
         * 10 copper even if you don't have any copper.
         * To make change, its still possible to withdraw
         * lots of copper, for example
         * /Gecko 2005-10-08 */
        sint64 amount = money.mithril * hooks->coins_arch[0]->clone.value
            + money.gold    * hooks->coins_arch[1]->clone.value
            + money.silver  * hooks->coins_arch[2]->clone.value
            + money.copper  * hooks->coins_arch[3]->clone.value;

        if(hooks->pay_for_amount(amount, WHO))
        {
            bank->value += amount;
            hooks->FIX_PLAYER(WHO, "LUA: deposit - pay for amount");
        }
        else
        {
            hooks->new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, WHO, "You don't have that much money.");
            val = 0;
        }
    }

    lua_pushnumber(L, val);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Withdraw                                              */
/* Lua    : object:Withdraw(deposit_object, string)                          */
/* Info   : withdraw value or string money from object in deposit_object.    */
/*          Control first object has that amount of money, then remove it    */
/*          from object and add it in ->value of deposit_object.             */
/*          FIXME Needs updated documentation                                */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Withdraw(lua_State *L)
{
    lua_object *obptr, *self;
    object     *bank;
    char       *text;
    int         val=1;
    _money_block money;
    int          pos     = 0;
    sint64       big_value;

    get_lua_args(L, "OOs", &self, &obptr, &text);
    bank = obptr->data.object;

/*
    static CFParm                       CFP;
    static int                          val;
    int                                 pos     = 0;
    sint64                              big_value;
    char                               *text    = (char *) (PParm->Value[2]);
    object*who = (object*) (PParm->Value[0]), *bank = (object *) (PParm->Value[1]);
*/

    hooks->get_word_from_string(text, &pos);
    hooks->get_money_from_string(text + pos, &money);

    if (!money.mode)
    {
        val = -1;
        hooks->new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, WHO, "withdraw what?\nUse 'withdraw all' or 'withdraw 30 gold, 20 silver....'");
    }
    else if (money.mode == MONEYSTRING_ALL)
    {
        hooks->sell_item(NULL, WHO, bank->value);
        bank->value = 0;
        hooks->FIX_PLAYER(WHO, "LUA: withdraw - sell item");
    }
    else
    {
        /* just to set a border.... */
        if (money.mithril > 100000 || money.gold > 100000 || money.silver > 1000000 || money.copper > 1000000)
            hooks->new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, WHO, "withdraw values to high.");
        else
        {
            big_value = money.mithril * hooks->coins_arch[0]->clone.value
                        + money.gold * hooks->coins_arch[1]->clone.value
                        + money.silver * hooks->coins_arch[2]->clone.value
                        + money.copper * hooks->coins_arch[3]->clone.value;

            if (big_value > bank->value)
                val = 0;
            else
            {
				// we have here "physical" money we insert in the player,
				// so the cast from int64 to unint32 will be ok - be sure we test it always
                if (money.mithril)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[0]->clone, (uint32)money.mithril);
                if (money.gold)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[1]->clone, (uint32)money.gold);
                if (money.silver)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[2]->clone, (uint32)money.silver);
                if (money.copper)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[3]->clone, (uint32)money.copper);

                bank->value -= big_value;
                hooks->FIX_PLAYER(WHO, "LUA: withdraw - insert money");
            }
        }
    }

    lua_pushnumber(L, val);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Communicate                                           */
/* Lua    : object:Communicate(message)                                      */
/* Info   : object says message to everybody on its map                      */
/*          but instead of CFSay it is parsed for other npc or magic mouth   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Communicate(lua_State *L)
{
    char       *message;
    lua_object *self;
    CFParm CFP;

    get_lua_args(L, "Os", &self, &message);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (message);

    (PlugHooks[HOOK_COMMUNICATE]) (&CFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Say                                                   */
/* Lua    : object:Say(message, mode, range)                                 */
/* Info   : object says message to players within range or to all on its map */
/*          mode: 0 (default) talk to object - adds a "xxx says:" as prefix  */
/*          mode: 1 raw message                                              */
/*          range: MAP_INFO_NORMAL (default) limited range (within 12 tiles) */
/*          range: MAP_INFO_ALL all players on map                           */
/* Note :   If range is specified, mode must also be explicitly specified.   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Say(lua_State *L)
{
    char       *message, buf[HUGE_BUF];
    int         mode = 0;
    int         range = MAP_INFO_NORMAL;
    lua_object *self;

    get_lua_args(L, "Os|ii", &self, &message, &mode, &range);

    if (!mode)
    {
        snprintf(buf, sizeof(buf), "%s says: %s", STRING_OBJ_NAME(WHO), message);
        message = buf;
    }
    hooks->new_info_map(NDI_UNIQUE | NDI_WHITE, WHO->map, WHO->x, WHO->y, range, message);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SayTo                                                 */
/* Lua    : object:SayTo(target, message, mode, range)                       */
/* Info   : NPC talks only to player but map may get a "xx talks to" msg too */
/*          (depends on mode).                                               */
/*          mode: 0 (default) talk to object - adds a "xxx says:" as prefix  */
/*          mode: 1 raw message                                              */
/*          mode: 2 adds as global map msg: xxx talks to yyy (was b3 default)*/
/*          range: MAP_INFO_NORMAL (default) limited range (within 12 tiles) */
/*          range: MAP_INFO_ALL all players on map                           */
/* Note :   If range is specified, mode must also be explicitly specified.   */
/*          range is only meaningful when mode = 2, and defines range of     */
/*          "talks to" message.                                              */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SayTo(lua_State *L)
{
    lua_object *self;
    object     *target;
    lua_object *obptr2;
    int mode = 0;
    int range = MAP_INFO_NORMAL;
    char *message;
    static char buf[HUGE_BUF];

    get_lua_args(L, "OOs|ii", &self, &obptr2, &message, &mode, &range);

    target = obptr2->data.object;

    if(mode == 1)
        hooks->new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, target, message);
    else /* thats default */
    {
        if(mode == 2)
        {
            snprintf(buf, sizeof(buf), "%s talks to %s.", STRING_OBJ_NAME(WHO),STRING_OBJ_NAME(target));
            hooks->new_info_map_except(NDI_UNIQUE | NDI_WHITE, WHO->map, WHO->x, WHO->y, range, WHO, target, buf);
        }
        snprintf(buf, sizeof(buf), "%s says: %s", STRING_OBJ_NAME(WHO), message);
        hooks->new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, target, buf);
    }

    return 0;
}

#ifdef USE_CHANNELS
/*****************************************************************************/
/* Name   : GameObject_ChannelMsg                                            */
/* Lua    : object:ChannelMsg(channel, message, mode)                        */
/* Info   : object sends message on the channel                              */
/*          mode: 0 (default) normal message                                 */
/*          mode: 1 emote                                                    */
/* Status : NOT-Tested                                                       */
/*****************************************************************************/
static int GameObject_ChannelMsg(lua_State *L)
{
    lua_object *self;
    char       *channel;
    int mode = 0;
    char *message;
    static char buf[HUGE_BUF];

    get_lua_args(L, "Oss|i", &self, &channel, &message, &mode);

    hooks->lua_channel_message(channel, STRING_OBJ_NAME(WHO), message, mode);

    return 0;
}

#endif
/*****************************************************************************/
/* Name   : GameObject_Write                                                 */
/* Lua    : object:Write(message, color)                                     */
/* Info   : Writes a message to a specific player.                           */
/*          color should be one of the game.COLOR_xxx constants.             */
/*          default color is game.COLOR_UNIQUE | game.COLOR_NAVY             */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Write(lua_State *L)
{
    char       *message;
    int         color = NDI_NAVY;
    lua_object *self;

    get_lua_args(L, "Os|i", &self, &message, &color);

    hooks->new_draw_info(NDI_UNIQUE | color, 0, WHO, message);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetGender                                             */
/* Lua    : object:GetGender()                                               */
/* Info   : Gets the gender of object. Returns game.NEUTER, game.MALE,       */
/*          game.FEMALE, or game.HERMAPHRODITE.                              */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_GetGender(lua_State *L)
{
    int         gender;
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (!QUERY_FLAG(WHO, FLAG_IS_MALE) && !QUERY_FLAG(WHO, FLAG_IS_FEMALE))
        gender = 0; // neuter
    else if (QUERY_FLAG(WHO, FLAG_IS_MALE) && !QUERY_FLAG(WHO, FLAG_IS_FEMALE))
        gender = 1; // male
    else if (!QUERY_FLAG(WHO, FLAG_IS_MALE) && QUERY_FLAG(WHO, FLAG_IS_FEMALE))
        gender = 2; // female
    else // if (QUERY_FLAG(WHO, FLAG_IS_MALE) && QUERY_FLAG(WHO, FLAG_IS_FEMALE))
        gender = 3; // hermaphrodite

    lua_pushnumber(L, gender);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SetGender                                             */
/* Lua    : object:SetGender(gender)                                         */
/* Info   : Changes the gender of object. gender_string should be one of     */
/*          game.NEUTER, game.MALE, game.GENDER_FEMALE or                    */
/*          game.HERMAPHRODITE                                               */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetGender(lua_State *L)
{
    int         gender;
    lua_object *self;

    get_lua_args(L, "Oi", &self, &gender);

    /* set object to neuter */
    CLEAR_FLAG(WHO, FLAG_IS_MALE);
    CLEAR_FLAG(WHO, FLAG_IS_FEMALE);
    SET_FLAG(WHO, FLAG_FIX_PLAYER);

    /* reset to male or female */
    if (gender & 1)
        SET_FLAG(WHO, FLAG_IS_MALE);
    if (gender & 2)
        SET_FLAG(WHO, FLAG_IS_FEMALE);

    /* update the players client of object was a player */
    if (WHO->type == PLAYER)
        CONTR(WHO)->socket.ext_title_flag = 1; /* demand update to client */

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetRank                                               */
/* Lua    : object:SetRank(rank_string)                                      */
/* Info   : Set the rank of an object to rank_string                         */
/*          Rank string 'Mr' is special for no rank                          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetRank(lua_State *L)
{
    object     *walk;
    char       *rank;
    lua_object *self;

    get_lua_args(L, "Os", &self, &rank);

    if (WHO->type != PLAYER)
        return 0;

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && walk->name == hooks->shstr_cons->RANK_FORCE && walk->arch->name == hooks->shstr_cons->rank_force)
        {
            if (strcmp(rank, "Mr") == 0) /* Mr = keyword to clear title and not add it as rank */
            {
                FREE_AND_CLEAR_HASH(walk->title);
            }
            else
            {
                FREE_AND_COPY_HASH(walk->title, rank);
            }

            CONTR(WHO)->socket.ext_title_flag = 1; /* demand update to client */
            return push_object(L, &GameObject, walk);
        }
    }
    LOG(llevDebug, "Lua Warning -> SetRank: Object %s has no rank_force!\n", STRING_OBJ_NAME(WHO));

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetAlignment                                          */
/* Lua    : object:SetAlignment(alignment_string)                            */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetAlignment(lua_State *L)
{
    object     *walk;
    char       *align;
    lua_object *self;

    get_lua_args(L, "Os", &self, &align);

    if (WHO->type != PLAYER)
        return 0;

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && walk->name == hooks->shstr_cons->ALIGNMENT_FORCE &&
                walk->arch->name == hooks->shstr_cons->alignment_force)
        {
            /* we find the alignment of the player, now change it to new one */
            FREE_AND_COPY_HASH(walk->title, align);

            CONTR(WHO)->socket.ext_title_flag = 1; /* demand update to client */
            return push_object(L, &GameObject, walk);
        }
    }
    LOG(llevDebug, "Lua Warning -> SetAlignment: Object %s has no alignment_force!\n", STRING_OBJ_NAME(WHO));

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetAlignmentForce                                     */
/* Lua    : object:GetAlignmentForce()                                       */
/* Info   : This gets the aligment_force from a inventory (should be player?)*/
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_GetAlignmentForce(lua_State *L)
{
    object     *walk;
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER)
        return 0;

    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && walk->name == hooks->shstr_cons->ALIGNMENT_FORCE &&
                walk->arch->name == hooks->shstr_cons->alignment_force)
            return push_object(L, &GameObject, walk);
    }
    LOG(llevDebug, "Lua Warning -> GetAlignmentForce: Object %s has no aligment_force!\n", STRING_OBJ_NAME(WHO));

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetGuild                                              */
/* Lua    : object:GetGuild(name)                                            */
/* Info   :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_GetGuild(lua_State *L)
{
    char *name;
    lua_object *self;
    object *force;

    get_lua_args(L, "Os", &self, &name);

    force = hooks->guild_get(CONTR(WHO),name);

    if(force)
        return push_object(L, &GameObject, force);
    else
        return 0;
}

/*****************************************************************************/
/* Name   : GameObject_CheckGuild                                            */
/* Lua    : object:CheckGuild(name)                                          */
/* Info   :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_CheckGuild(lua_State *L)
{
    int vret = 0;
    char *name;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    if(WHO->slaying && !strcmp(WHO->slaying, name))
        vret = 1;

    lua_pushboolean(L, vret);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_JoinGuild                                             */
/* Lua    : object:JoinGuild(name, skill1, v1, skill2, v2, skill3, v3)       */
/* Info   :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_JoinGuild(lua_State *L)
{
    char *name;
    int s1,s2,s3,sv1, sv2, sv3;
    lua_object *self;
    object *force;

    get_lua_args(L, "Os|iiiiii", &self, &name, &s1, &sv1, &s2, &sv2, &s3, &sv3);

    SET_FLAG(WHO, FLAG_FIX_PLAYER);

    force = hooks->guild_join(CONTR(WHO), name, s1, sv1, s2, sv2, s3, sv3);
    hooks->new_draw_info_format(NDI_UNIQUE | NDI_NAVY, 0, WHO, "you join %s Guild.", name);

    if(force)
        return push_object(L, &GameObject, force);
    else
        return 0;
}

/*****************************************************************************/
/* Name   : GameObject_LeaveGuild                                            */
/* Lua    : object:LeaveGuild(rank_string)                                   */
/* Info   :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_LeaveGuild(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    hooks->guild_leave(CONTR(WHO));
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Fix                                                   */
/* Lua    : object:Fix()                                                     */
/* Info   : Recalculates a player's or monster's stats depending on          */
/*          equipment, forces, skills etc.                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_Fix(lua_State *L)
{
    lua_object *self;
    get_lua_args(L, "O", &self);

    hooks->FIX_PLAYER(WHO, "LUA - fix player");

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Kill                                                  */
/* Lua    : object:Kill(what, how)                                           */
/* Status : Untested                                                         */
/*****************************************************************************/
/* add hooks before use! */

static int GameObject_Kill(lua_State *L)
{
    lua_object *whatptr;
    int         ktype;
    int         k   = 1;
    CFParm     *CFR, CFP;
    lua_object *self;

    get_lua_args(L, "OOi", &self, &whatptr, &ktype);

    WHAT->speed = 0;
    WHAT->speed_left = 0.0;
    CFP.Value[0] = (void *) (WHAT);
    (PlugHooks[HOOK_UPDATESPEED]) (&CFP);
    /* update_ob_speed(WHAT); */

    if (QUERY_FLAG(WHAT, FLAG_REMOVED))
    {
        LOG(llevDebug, "Warning (from KillObject): Trying to remove removed object\n");
        luaL_error(L, "Trying to remove removed object");
    }
    else
    {
        WHAT->stats.hp = -1;
        CFP.Value[0] = (void *) (WHAT);
        CFP.Value[1] = (void *) (&k);
        CFP.Value[2] = (void *) (WHO);
        CFP.Value[3] = (void *) (&ktype);

        CFR = (PlugHooks[HOOK_KILLOBJECT]) (&CFP);
        free(CFR);
    }

    /* TODO: make sure this still works... */
    /* This is to avoid the attack routine to continue after we called
     * killObject, since the attacked object no longer exists.
     * By fixing guile_current_other to NULL, guile_use_weapon_script will
     * return -1, meaning the attack function must be immediately terminated.
     */
    /*    if (WHAT==StackOther[StackPosition])
        {
            StackOther[StackPosition] = NULL;
        }*/

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_CastSpell                                             */
/* Lua    : object:CastSpell(target, spell, mode, direction, option)         */
/* Info   : object casts the spell numbered spellno on target.               */
/*          mode = game.CAST_NORMAL or game.CAST_POTION              */
/*          direction is the direction to cast the spell in                  */
/*          option is additional string option(s)                            */
/*          NPCs can cast spells even in no-spell areas.                     */
/*          FIXME: only allows for directional spells                        */
/*          FIXME: is direction/position relative to target? (0 = self)      */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_CastSpell(lua_State *L)
{
    lua_object *target;
    int         spell;
    int         dir;
    int         mode;
    char       *op;
    int         val, parm2, parm        = 1;
    lua_object *self;

    get_lua_args(L, "OOiiis", &self, &target, &spell, &mode, &dir, &op);

    if (WHO->type != PLAYER)
        parm2 = spellNPC;
    else
    {
        if (!mode)
            parm2 = spellNormal;
        else
            parm2 = spellPotion;
    }

    val = hooks->cast_spell(target->data.object, WHO, dir, spell, parm, parm2, op);

    lua_pushboolean(L, val);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_DoKnowSpell                                           */
/* Lua    : object:DoKnowSpell(spell)                                        */
/* Info   : true if the spell is known by object, false if it isn't          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_DoKnowSpell(lua_State *L)
{
    int         spell;
    CFParm     *CFR, CFP;
    int         value;
    lua_object *self;

    get_lua_args(L, "Oi", &self, &spell);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (&spell);
    CFR = (PlugHooks[HOOK_CHECKFORSPELL]) (&CFP);
    value = *(int *) (CFR->Value[0]);
    free(CFR);

    lua_pushboolean(L, value);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AcquireSpell                                          */
/* Lua    : object:AcquireSpell(spell, mode)                                 */
/* Info   : object will learn or unlearn spell.                              */
/*          mode should be  game.LEARN or game.UNLEARN                       */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_AcquireSpell(lua_State *L)
{
    int         spell;
    int         mode;
    lua_object *self;
    CFParm      CFP;

    get_lua_args(L, "Oii", &self, &spell, &mode);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (&spell);
    CFP.Value[2] = (void *) (&mode);
    (PlugHooks[HOOK_LEARNSPELL]) (&CFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_FindSkill                                             */
/* Lua    : object:FindSkill(skill)                                          */
/* Info   : skill ptr if the skill is known by object, NULL is it isn't      */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_FindSkill(lua_State *L)
{
    int         skill;
    object     *myob;
    lua_object *self;

    get_lua_args(L, "Oi", &self, &skill);

    myob = hooks->find_skill(WHO, skill);

    if (!myob)
        return 0;

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_AcquireSkill                                          */
/* Lua    : object:AcquireSkill(skillno, mode)                               */
/* Info   : object will learn or unlearn skill.                              */
/*          mode should be game.LEARN or game.UNLEARN                        */
/*          Get skill number with game:GetSkillNr()                          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_AcquireSkill(lua_State *L)
{
    int         skill, mode;
    lua_object *self;
    CFParm      CFP;

    get_lua_args(L, "Oii", &self, &skill, &mode);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (&skill);
    CFP.Value[2] = (void *) (&mode);
    (PlugHooks[HOOK_LEARNSKILL]) (&CFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_FindMarkedObject                                      */
/* Lua    : object:FindMarkedObject()                                        */
/* Info   : Returns the marked object in object's inventory, or None if no   */
/*          object is marked.                                                */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_FindMarkedObject(lua_State *L)
{
    object     *value;
    CFParm     *CFR, CFP;
    lua_object *self;

    get_lua_args(L, "O", &self);

    CFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_FINDMARKEDOBJECT]) (&CFP);

    value = (object *) (CFR->Value[0]);
    /*free(CFR); findmarkedobject use static parameters */
    return push_object(L, &GameObject, value);
}

/*****************************************************************************/
/* Name   : GameObject_CheckInvisibleInside                                  */
/* Lua    : object:CheckInvisibleInside(id)                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_CheckInvisibleInside(lua_State *L)
{
    char       *id;
    object     *tmp2;
    lua_object *self;

    get_lua_args(L, "Os", &self, &id);

    for (tmp2 = WHO->inv; tmp2 != NULL; tmp2 = tmp2->below)
    {
        if (tmp2->type == FORCE && tmp2->slaying && !strcmp(tmp2->slaying, id))
            break;
    }

    return push_object(L, &GameObject, tmp2);
}

/*****************************************************************************/
/* Name   : GameObject_CreatePlayerForce                                     */
/* Lua    : object:CreatePlayerForce(force_name, time)                       */
/* Info   : Creates and insters a player force named force_name in object.   */
/*          The values of a player force will effect the player.             */
/*          If time is given and > 0, the force will be removed again after  */
/*          time/0.02 ticks.                                                 */
/* Status : Stable.                                                          */
/*****************************************************************************/
static int GameObject_CreatePlayerForce(lua_State *L)
{
    char       *txt;
    object     *myob;
    int         time        = 0;
    lua_object *whereptr;

    get_lua_args(L, "Os|i", &whereptr, &txt, &time);

    myob = hooks->get_archetype("player_force");

    if (!myob || strncmp(STRING_OBJ_NAME(myob), "singularity", 11) == 0)
    {
        LOG(llevDebug, "Lua WARNING:: CreatePlayerForce: Can't find archtype 'player_force'\n");
        luaL_error(L, "Can't find archtype 'player_force'");
    }

    /* For temporary forces */
    if (time > 0)
    {
        CFParm CFP;
        SET_FLAG(myob, FLAG_IS_USED_UP);
        myob->stats.food = time;
        myob->speed = 0.02f;
        CFP.Value[0] = (void *) (myob);
        (PlugHooks[HOOK_UPDATESPEED]) (&CFP);
    }

    /* setup the force and put it in activator */
    FREE_AND_COPY_HASH(myob->name, txt);
    myob = hooks->insert_ob_in_ob(myob, WHERE);
    hooks->esrv_send_item(hooks->is_player_inv(WHERE), myob);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_AddQuest                                              */
/* Lua    : object:AddQuest(q_name, mode, start, stop, level, skill, msg)    */
/* Info   : Add a quest_trigger to a quest_container = give player a quest   */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_AddQuest(lua_State *L)
{
    char       *name, *msg;
    int         mode, lev, skill_lev, step_start, step_end;
    object     *myob;
    lua_object *self;

    get_lua_args(L, "Osiiiii|s", &self, &name, &mode, &step_start, &step_end, &lev, &skill_lev, &msg);

    /* if we return NULL, the quest can't be given - if we are a player because we it max quests */
    if(WHO->type != PLAYER || hooks->quest_count_pending(WHO) >= QUESTS_PENDING_MAX)
    {
        hooks->new_draw_info_format(NDI_UNIQUE | NDI_NAVY, 0, WHO, "You can't have more as %d open quests.\nRemove one first!", QUESTS_PENDING_MAX);
        return 0;
    }

    myob = hooks->get_archetype("quest_trigger");

    if (!myob || strncmp(STRING_OBJ_NAME(myob), "singularity", 11) == 0)
    {
        LOG(llevDebug, "Lua WARNING:: AddQuest: Cant't find archtype 'quest_trigger'\n");
        luaL_error(L, "Can't find archtype 'quest_trigger'");
    }

    /* store name & arch name of the quest obj. so we can id it later */
    FREE_AND_COPY_HASH(myob->name, name);
    if(msg)
        FREE_AND_COPY_HASH(myob->msg, msg);

    myob->sub_type1 = (uint8)mode;
    myob->last_heal = (sint16)step_start;
    myob->state = step_end;
    myob->item_skill = skill_lev;
    myob->item_level = lev;

    hooks->add_quest_trigger(WHO, myob);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_GetQuest                                              */
/* Lua    : object:GetQuest(name)                                            */
/* Status : Stable                                                           */
/* Info   : Browses all quest containers in the object for a quest trigger   */
/*          with the name specified. If such a trigger is found its object is*/
/*          returned.                                                        */
/*****************************************************************************/
static int GameObject_GetQuest(lua_State *L)
{
    char       *name;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    return push_object(L, &GameObject, hooks->quest_find_name(WHO, name));
}

/*****************************************************************************/
/* Name   : GameObject_CheckQuestLevel                                       */
/* Lua    : object:CheckQuestLevel(level, skill_group)                       */
/* Status : Stable                                                           */
/* Info   : Check if a quest is possible to start based on a minimum required*/
/*          level in a specified skill group.                                */
/*          the skill_group parameter _must_ be one of the                   */
/*          game.ITEM_SKILL_XXX constants. game.ITEM_SKILL_NO checks against */
/*          the player's main level.                                         */
/*          This function is only valid for objects of TYPE_PLAYER.          */
/*          Normally, the QuestManager wrapper is used instead of calling    */
/*          this function directly. Please see the lua scripting docs for    */
/*          QuestManager details.                                            */
/*****************************************************************************/
static int GameObject_CheckQuestLevel(lua_State *L)
{
    int level, item_skill_group, tmp_lev, ret=1;
    lua_object *self;
    player *pl = NULL;

    get_lua_args(L, "Oii", &self, &level, &item_skill_group);

    /* some sanity checks */
    if(WHO->type != PLAYER || !(pl=CONTR(WHO)))
        luaL_error(L, "Not a player object");
    if(item_skill_group < 0 || item_skill_group > NROFSKILLGROUPS)
        luaL_error(L, "Invalid skill_group parameter. Use one of the game.ITEM_SKILL_XXX constants");

    /* Note: the ITEM_SKILL_XXX lua constants corresponds to SKILLGROUP_XXX + 1 */
    /* player is high enough for this quest? */
    if (item_skill_group)
        tmp_lev = pl->exp_obj_ptr[item_skill_group-1]->level; /* use player struct shortcut ptrs */
    else
        tmp_lev = WHO->level;

    if (level > tmp_lev) /* too low */
        ret = 0;

    lua_pushboolean(L, ret);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AddQuestTarget                                        */
/* Lua    : object:AddQuestTarget(chance, nrof, k_arch, k_name, k_title)     */
/* Info   : define a kill mob. Careful: if all are "" then ALL mobs are part */
/*          of this quest. If only arch set, all mobs using that base arch   */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_AddQuestTarget(lua_State *L)
{
    char       *kill_arch, *kill_name=NULL, *kill_sym_name1=NULL, *kill_sym_name2=NULL;
    int         nrof, chance;
    object     *myob;
    lua_object *self;

    get_lua_args(L, "Oiis|s|s|s", &self, &chance, &nrof, &kill_arch, &kill_name, &kill_sym_name1, &kill_sym_name2);

    myob = hooks->get_archetype("quest_info");
    if(!myob)
    {
        LOG(llevBug, "Lua Warning -> AddQuestTarget:: Can't find quest_info arch!");
        return 0;
    }

    myob->last_grace = chance;
    myob->last_sp = nrof; /* can be overruled by ->inv objects */

    /* to be sure we get the right mob we use the arch object name */
    if(*kill_arch!='\0')
    {
        FREE_AND_COPY_HASH(myob->race, kill_arch);
    }
    else
    {
        FREE_ONLY_HASH(myob->race);
    }

    /* this is the "base" name of the kill mob */
    if(kill_name && *kill_name!='\0')
    {
        FREE_AND_COPY_HASH(myob->name, kill_name);
    }
    else
    {
        FREE_ONLY_HASH(myob->name);
    }

    /* perhaps name is "giant spiders" - now we have at the spot
     * "large spiders" and "huge spiders" too. With the sym
     * names we can add a bit "fuzzy" to our kill quests and
     * allow kills of them too. A bit like "kill quest mob group".
     */
    if(kill_sym_name1 && *kill_sym_name1!='\0')
    {
        FREE_AND_COPY_HASH(myob->slaying, kill_sym_name1);
    }
    else
    {
        FREE_ONLY_HASH(myob->slaying);
    }
    if(kill_sym_name2 && *&kill_sym_name2!='\0')
    {
        FREE_AND_COPY_HASH(myob->title, kill_sym_name2);
    }
    else
    {
        FREE_ONLY_HASH(myob->title);
    }

    /* finally add it to the quest_trigger object */
    hooks->insert_ob_in_ob(myob, self->data.object);

    /* we need that when we want do more with this object - for example adding kill items */
    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_AddQuestItem                                          */
/* Lua    : object:AddQuestItem(nrof, arch, face, name, title)               */
/* Info   : Add a quest item to a quest or base object                       */
/*          (see GameObject_AddQuestTarget)                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_AddQuestItem(lua_State *L)
{
    int         id, nrof;
    char       *i_arch, *i_face, *i_name = NULL, *i_title = NULL;
    object     *myob;
    lua_object *self;

    get_lua_args(L, "Oiss|s|s", &self, &nrof, &i_arch, &i_face, &i_name, &i_title);

    myob = hooks->get_archetype(i_arch);
    if(!myob)
    {
        LOG(llevBug, "Lua Warning -> AddQuestTarget:: Can't find quest_info arch!");
        return 0;
    }

    if(i_face && *i_face !='\0') /* "" will skip the face setting */
    {
        id = hooks->find_face(i_face, -1);
        if(id == -1)
            luaL_error(L, "no such face exists: %s", STRING_SAFE(i_face));
        else
            myob->face = &(*hooks->new_faces)[id];
    }

    /* we can set or delete the name or title with this.
     * Thats just to speed things up, we can of course call the
     * set/get functions explicit in the scripts.
     */
    if(i_name)
    {
        if(*i_name!='\0')
        {
            FREE_AND_COPY_HASH(myob->name, i_name);
        }
        else
        {
            FREE_ONLY_HASH(myob->name);
        }
    }

    if(i_title)
    {
        if(*i_title!='\0')
        {
            FREE_AND_COPY_HASH(myob->title, i_title);
        }
        else
        {
            FREE_ONLY_HASH(myob->title);
        }
    }

    /* how many we have/need to find */
    myob->nrof = nrof;

    /* finally add it to our target object*/
    hooks->insert_ob_in_ob(myob, self->data.object);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_NrofQuestItem                                         */
/* Lua    : object:NrofQuestItem()                                           */
/* Info   : counts quest items inside the inventory of the player            */
/*          where target_obj is inside                                       */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_NrofQuestItem(lua_State *L)
{
    int           nrof=0;
    const object *myob, *pl;
    lua_object   *self;

    get_lua_args(L, "O", &self);

    /* object is inside our target object - see function GameObject_AddQuestItem */
    pl = hooks->is_player_inv(self->data.object);

    /* kill items are inside target, normal items inside the quest object itself */
    myob = self->data.object->inv;
    if(!myob)
        myob = self->data.object;

    if(myob && pl)
        nrof = hooks->get_nrof_quest_item(pl, myob->arch->name, myob->name, myob->title);

    lua_pushnumber(L, nrof);
    return 1;
}

/* helper function for GameObject_RemoveKillQuestItem - recursive used */
static int remove_quest_items(const object *inv, const object *myob, int nrof)
{
    object *walk, *walk_below;

    for (walk = (object*)inv; walk != NULL; walk = walk_below)
    {
        walk_below = walk->below;

        if(QUERY_FLAG(walk, FLAG_SYS_OBJECT)) /* only real inventory items */
            continue;

        if(walk->type == CONTAINER)
        {
            if(!(nrof = remove_quest_items(walk->inv, myob, nrof)))
                return nrof;
        }

        /* we have a hit */
        if (walk->arch->name == myob->arch->name && walk->name == myob->name && walk->title == myob->title)
        {
            int tmp = walk->nrof?walk->nrof:1;

            if(nrof > tmp) /* we have then still some more to do */
            {
                hooks->decrease_ob_nr(walk, tmp);
                nrof -= tmp;
            }
            else
            {
                hooks->decrease_ob_nr(walk, nrof);
                return 0;
            }
        }
    }
    return nrof;
}

/*****************************************************************************/
/* Name   : GameObject_RemoveQuestItem                                       */
/* Lua    : object:RemoveQuestItem(nrof)                                     */
/* Info   : removes the items from the players inventory.                    */
/*          Get the template info from the kill target obj inventory         */
/*          NOTE: the function tries to remove given objects even when there,*/
/*          are not enough! count them before calling this function.         */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_RemoveQuestItem(lua_State *L)
{
    int         nrof = -1;
    object     *myob, *pl;
    lua_object *self;

    get_lua_args(L, "O|i", &self, &nrof);

    pl = hooks->is_player_inv(self->data.object);
    myob = self->data.object->inv;

    /* some sanity checks */
    if(myob && pl && pl->type == PLAYER)
    {
        if(nrof == -1) /* if we don't have an explicit number, use number from kill target */
            nrof = myob->nrof;

        hooks->new_draw_info_format(NDI_UNIQUE | NDI_NAVY, 0, pl, "%s is removed from your inventory.",
                hooks->query_short_name(myob, NULL));
        remove_quest_items(pl->inv, myob, nrof);
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetQuestStatus                                        */
/* Lua    : object:SetQuestStatus(status, step_id)                           */
/* Info   : We need this function because quest_trigger must be moved        */
/*          q_status: -1 = done, q_type: can change type (kill, normal..)    */
/*          Common call is SetQuestStatus(-1) to "finish"/neutralize a quest */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_SetQuestStatus(lua_State *L)
{
    int         q_status, q_type = -1;
    object     *myob;
    lua_object *self;

    get_lua_args(L, "Oi|i", &self, &q_status, &q_type);

    myob = self->data.object;

    if(q_type == -1)
        q_type = myob->sub_type1;

    hooks->set_quest_status(myob, q_status, q_type);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_CheckOneDropQuest                                     */
/* Lua    : object:CheckOneDropQuest(archetype, name)                        */
/* Status : Stable                                                           */
/* Info   : Check the one drop and single quest item container for an item   */
/*****************************************************************************/
static int GameObject_CheckOneDropQuest(lua_State *L)
{
    const char *name_hash, *title_hash;
    archetype *arch;
    char       *arch_name;
    char       *name, *title = NULL;
    object     *walk;
    lua_object *self;

    get_lua_args(L, "Os|s", &self, &arch_name, &name, &title);

    arch = hooks->find_archetype(arch_name); /* no arch - nothing to find */
    name_hash = hooks->find_string(name);
    if(title)
        title_hash = hooks->find_string(title);
    else
        title_hash = arch->clone.title;

    if (WHO->type == PLAYER && CONTR(WHO)->quest_one_drop)
    {
        for (walk = CONTR(WHO)->quest_one_drop->inv; walk != NULL; walk = walk->below)
        {
            if (walk->race == arch->name && walk->name == name_hash && walk->title == title_hash)
                return push_object(L, &GameObject, walk);
        }
    }

    return 0; /* there was non */
}

/*****************************************************************************/
/* Name   : GameObject_AddOneDropQuest                                       */
/* Lua    : object:AddOneDropQuest(archetype, name, title)                   */
/* Status : Stable                                                           */
/* Info   : Adds a one drop item to the player.                              */
/*          (Creates the quest container if needed.)                         */
/*****************************************************************************/
static int GameObject_AddOneDropQuest(lua_State *L)
{
    char       *name, *title=NULL;
    lua_object *self, *whatptr;

    get_lua_args(L, "OOs|s", &self, &whatptr, &name, &title);

    /* store name & arch name of the quest obj. so we can id it later */
    FREE_AND_COPY_HASH(WHAT->name, name);
    FREE_AND_COPY_HASH(WHAT->race, WHAT->arch->name);
    if(title)
        FREE_AND_COPY_HASH(WHAT->title, title);

    if(!CONTR(WHO)->quest_one_drop)
        hooks->add_quest_containers(WHO);
    hooks->insert_ob_in_ob(WHAT, CONTR(WHO)->quest_one_drop);

    return push_object(L, &GameObject, WHAT);
}

/*****************************************************************************/
/* Name   : GameObject_CreatePlayerInfo                                      */
/* Lua    : object:CreatePlayerInfo(name)                                    */
/* Status : Stable                                                           */
/* Info   : Creates a player_info object of specified name in object's       */
/*          inventory.                                                       */
/*          The values of a player_info object will NOT effect the player.   */
/*          Returns the created object                                       */
/*****************************************************************************/
static int GameObject_CreatePlayerInfo(lua_State *L)
{
    char       *txt;
    char        txt2[16]    = "player_info";
    object     *myob;
    lua_object *whereptr;

    get_lua_args(L, "Os", &whereptr, &txt);

    myob = hooks->get_archetype(txt2);
    if (!myob || strncmp(STRING_OBJ_NAME(myob), "singularity", 11) == 0)
    {
        LOG(llevDebug, "Lua WARNING:: CreatePlayerInfo: Cant't find archtype 'player_info'\n");
        luaL_error(L, "Cant't find archtype 'player_info'");
    }

    /* setup the info and put it in activator */
    FREE_AND_COPY_HASH(myob->name, txt);
    myob = hooks->insert_ob_in_ob(myob, WHERE);
    hooks->esrv_send_item(hooks->is_player_inv(WHERE), myob);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_GetPlayerInfo                                         */
/* Lua    : object:GetPlayerInfo(name)                                       */
/* Status : Stable                                                           */
/* Info   : get first player_info with the specified name in who's inventory */
/*****************************************************************************/
static int GameObject_GetPlayerInfo(lua_State *L)
{
    char       *name;
    object     *walk;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    /* get the first linked player_info arch in this inventory */
    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && walk->arch->name == hooks->shstr_cons->player_info && !strcmp(walk->name, name))
            return push_object(L, &GameObject, walk);
    }

    return 0; /* there was non */
}

/*****************************************************************************/
/* Name   : GameObject_GetNextPlayerInfo                                     */
/* Lua    : object:GetNextPlayerInfo(player_info)                            */
/* Status : Stable                                                           */
/* Info   : get next player_info in who's inventory with same name as        */
/*          player_info                                                      */
/*****************************************************************************/
static int GameObject_GetNextPlayerInfo(lua_State *L)
{
    lua_object *myob;
    char        name[128];
    object     *walk;
    lua_object *self;

    get_lua_args(L, "OO", &self, &myob);

    /* thats our check paramters: arch "force_info", name of this arch */
    strncpy(name, STRING_OBJ_NAME(myob->data.object), 127); /* 127 chars should be enough for all */
    name[127] = '\0';

    /* get the next linked player_info arch in this inventory */
    for (walk = myob->data.object->below; walk != NULL; walk = walk->below)
    {
        if (walk->name && walk->arch->name == hooks->shstr_cons->player_info && !strcmp(walk->name, name))
            return push_object(L, &GameObject, walk);
    }

    return 0; /* there was non left */
}

/*****************************************************************************/
/* Name   : GameObject_CreateInvisibleInside                                 */
/* Lua    : object:CreateInvisibleObjectInside(id)                           */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_CreateInvisibleInside(lua_State *L)
{
    char       *txt;
    object     *myob;
    lua_object *whereptr;
    CFParm      CFP;

    get_lua_args(L, "Os", &whereptr, &txt);

    myob = hooks->get_archetype("force");

    if (!myob || strncmp(STRING_OBJ_NAME(myob), "singularity", 11) == 0)
    {
        LOG(llevDebug, "Lua WARNING:: CFCreateInvisibleInside: Can't find archtype 'force'\n");
        luaL_error(L, "Cant't find archtype 'force'");
    }
    myob->speed = 0.0;
    CFP.Value[0] = (void *) (myob);
    (PlugHooks[HOOK_UPDATESPEED]) (&CFP);

    /*update_ob_speed(myob); */
    FREE_AND_COPY_HASH(myob->slaying, txt);
    myob = hooks->insert_ob_in_ob(myob, WHERE);
    hooks->esrv_send_item(hooks->is_player_inv(WHERE), myob);

    return push_object(L, &GameObject, myob);
}

/* code body of the CreateObjectInside functions */
static object *CreateObjectInside_body(lua_State *L, object *where, char *archname, int id, int nrof, int value)
{
    object *myob = hooks->get_archetype(archname);
    object *retobj;
    if (!myob || strncmp(STRING_OBJ_NAME(myob), "singularity", 11) == 0)
    {
        LOG(llevDebug, "BUG GameObject_CreateObjectInside(): ob:>%s< = NULL!\n", STRING_OBJ_NAME(myob));
        luaL_error(L, "Failed to create the object. Did you use an existing arch?");
    }

    if (value != -1) /* -1 means, we use original value */
        myob->value = value;
    if (id)
    {
        SET_FLAG(myob, FLAG_IDENTIFIED);
        SET_FLAG(myob, FLAG_KNOWN_MAGICAL);
        SET_FLAG(myob, FLAG_KNOWN_CURSED);
    }
    if (nrof > 1)
        myob->nrof = nrof;

    retobj = hooks->insert_ob_in_ob(myob, where);
    hooks->esrv_send_item(where, retobj);
    return retobj;
}

/*****************************************************************************/
/* Name   : GameObject_CreateObjectInside                                    */
/* Lua    : object:CreateObjectInside(archname, identified, number, value)   */
/* Info   : Creates an object from archname and inserts into object.         */
/*          identified is either game.IDENTIFIED or game.UNIDENTIFIED        */
/*          number is the number of objects to create in a stack             */
/*          If value is >= 0 it will be used as the new object's value,      */
/*          otherwise the value will be taken from the arch.                 */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_CreateObjectInside(lua_State *L)
{
    object *myob;
    int         value = -1, id = 0, nrof = 1;
    char       *txt;
    lua_object *whereptr;

    get_lua_args(L, "Os|iii", &whereptr, &txt, &id, &nrof, &value);

    myob = CreateObjectInside_body(L, WHERE, txt, id, nrof, value);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_CreateObjectInsideEx                                  */
/* Lua    : object:CreateObjectInsideEx(archname, identified, number, value) */
/* Info   : Same as GameObject_CreateObjectInside  but give message to player*/
/*          Creates an object from archname and inserts into object.         */
/*          identified is either game.IDENTIFIED or game.UNIDENTIFIED        */
/*          number is the number of objects to create in a stack             */
/*          If value is >= 0 it will be used as the new object's value,      */
/*          otherwise the value will be taken from the arch.                 */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_CreateObjectInsideEx(lua_State *L)
{
    object     *myob, *pl;
    int         value = -1, id = 0, nrof = 1;
    char       *txt;
    lua_object *whereptr;

    get_lua_args(L, "Os|iii", &whereptr, &txt, &id, &nrof, &value);

    myob = CreateObjectInside_body(L, WHERE, txt, id, nrof, value);

    pl = hooks->is_player_inv(myob);
    if(pl)
    {
        hooks->new_draw_info_format(NDI_UNIQUE | NDI_NAVY, 0, pl, "you got %d %s",
                nrof?nrof:1, hooks->query_base_name(myob, NULL));
    }

    return push_object(L, &GameObject, myob);
}

/* help function for GameObject_CheckInventory
 * to recursive check object inventories.
 */
static object * object_check_inventory_rec(object *tmp, int mode, char *arch_name, char *name, char *title, int type)
{
    object *tmp2;

    while (tmp)
    {
        if ((!name || (tmp->name && !strcmp(tmp->name, name)))
         && (!title || (tmp->title && !strcmp(tmp->title, title)))
         && (!arch_name || (tmp->arch && tmp->arch->name && !strcmp(tmp->arch->name, arch_name)))
         && (type == -1 || tmp->type == type))
            return tmp;

        if (mode == 2 || (mode && tmp->type == CONTAINER))
        {
            if ((tmp2 = object_check_inventory_rec(tmp->inv, mode, arch_name, name, title, type)))
                return tmp2;
        }

        tmp = tmp->below;
    }

    return NULL;
}

/*****************************************************************************/
/* Name   : GameObject_CheckInventory                                        */
/* Lua    : object:CheckInventory(mode, arch, name, title, type)             */
/* Info   : returns the first found object with the specified name if found  */
/*          in object's inventory, or nil if it wasn't found.                */
/*          title, arch or object == nil will be ignored for search          */
/*          also type == -1                                                  */
/*          mode: 0: only inventory, 1: inventory and container, 2: all inv. */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_CheckInventory(lua_State *L)
{
    lua_object     *self;
    int             type = -1, mode = 0;
    char           *name = NULL, *title = NULL, *arch_name = NULL;
    object         *tmp, *tmp2;

    get_lua_args(L, "Oi?s|?s?si", &self, &mode, &arch_name, &name, &title, &type);

    tmp = WHO->inv;

    while (tmp)
    {
        if ((!name || (tmp->name && !strcmp(tmp->name, name)))
         && (!title || (tmp->title && !strcmp(tmp->title, title)))
         && (!arch_name || (tmp->arch && tmp->arch->name && !strcmp(tmp->arch->name, arch_name)))
         && (type == -1 || tmp->type == type))
            return push_object(L, &GameObject, tmp);

        if (mode == 2 || (mode == 1 && tmp->type == CONTAINER))
        {
            if ((tmp2 = object_check_inventory_rec(tmp->inv, mode, arch_name, name, title, type)))
                return push_object(L, &GameObject, tmp2);
        }

        tmp = tmp->below;
    }

    return 0; /* we don't find a arch with this arch_name in the inventory */
}

/*****************************************************************************/
/* Name   : GameObject_SetSaveBed                                            */
/* Lua    : object:SetSaveBed(map, x, y)                                     */
/* Info   : Sets the current savebed position for object to the specified    */
/*          coordinates on the map.                                          */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_SetSaveBed(lua_State *L)
{
    lua_object *self, *map;
    int         x, y;

    get_lua_args(L, "OMii", &self, &map, &x, &y);

    if (WHO->type == PLAYER)
    {
        player *pl = CONTR(WHO);

       /* set_bindpath_by_name(...); exchange later  */
        FREE_AND_ADD_REF_HASH(pl->savebed_map, map->data.map->path);
        FREE_AND_ADD_REF_HASH(pl->orig_savebed_map, map->data.map->orig_path);
        pl->bed_status = MAP_STATUS_TYPE(map->data.map->map_status);
        pl->bed_x = x;
        pl->bed_y = y;
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_DecreaseNrOf                                          */
/* Lua    : object:DecreaseNrOf(decrease)                                    */
/* Info   : Reduces the number of objects in a stack, removing the stack if  */
/*          the last object is removed.                                      */
/*          decrease is the number of objects to remove from the stack. 1    */
/*          is default, -1 means to remove the whole stack.                  */
/* Status : New and slightly unstable (feature-wise - not as in crashy)      */
/*****************************************************************************/
static int GameObject_DecreaseNrOf(lua_State *L)
{
    lua_object *self;
    int         nrof = 1;

    get_lua_args(L, "O|i", &self, &nrof);

    /* -1 means "delete all" */
    if(nrof==-1)
        nrof = MAX(self->data.object->nrof, 1);

    hooks->decrease_ob_nr(self->data.object, nrof);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Remove                                                */
/* Lua    : object:Remove()                                                  */
/* Info   : Takes the object out of whatever map or inventory it is in. The  */
/*          object can then be inserted or teleported somewhere else, or just*/
/*          left alone for the garbage collection to take care of.           */
/*          If you just want to remove a part of a stack, have a look at     */
/*          object:DecreaseNrOf(). If you actually want to represent the     */
/*          destruction of an object, use object:Destruct()                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_Remove(lua_State *L)
{
    lua_object *self;
    object     *myob;
    object     *obenv, *tmp;
    CFParm      CFP;

    get_lua_args(L, "O", &self);

    myob = WHO;
    obenv = myob->env;

    CFP.Value[0] = (void *) (myob);
    (PlugHooks[HOOK_REMOVEOBJECT]) (&CFP);

    /* Update player's inventory if object was removed from player
     * TODO: see how well this works with things in containers */
    /*`TODO: this is broken. See reduce_ob_nrof for the correct implementation (and possibly
     * something that can be broken out and reused */
    for (tmp = obenv; tmp != NULL; tmp = tmp->env)
        if (tmp->type == PLAYER)
            hooks->esrv_send_inventory(tmp, tmp);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Destruct                                              */
/* Lua    : object:Destruct()                                                */
/* Info   : Removes the object from the game and drops all items in object's */
/*          inventory on the floor or in a corpse                            */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Destruct(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if(WHO->inv)
        hooks->drop_ob_inv(WHO);

    hooks->decrease_ob_nr(WHO, MAX(WHO->nrof, 1));

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Move                                                  */
/* Lua    : object:Move(direction)                                           */
/* Info   : Makes the object move in the desired direction (one of           */
/*          game.NORTH, game.SOUTH, game.NORTHEAST etc.)                     */
/*          Returns 0 if something blocked the move, 1 if moving succeeded,  */
/*          or -1 if object was destroyed when moving.                       */
/*          The object's terrain_flag attribute controls which terrains it   */
/*          can move on.                                                     */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Move(lua_State *L)
{
    lua_object *self;
    int         d;

    get_lua_args(L, "Oi", &self, &d);

    lua_pushnumber(L, hooks->move_ob(WHO, d, WHO));
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_IdentifyItem                                          */
/* Lua    : object:IdentifyItem(target, marked, mode)                        */
/* Info   : object identifies object(s) in target's inventory.               */
/*          mode: game.IDENTIFY_NORMAL, game.IDENTIFY_ALL or                 */
/*          game.IDENTIFY_MARKED                                             */
/*          marked must be None for IDENTIFY_NORMAL and IDENTIFY_ALL         */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_IdentifyItem(lua_State *L)
{
    lua_object *self;
    lua_object *target;
    lua_object *ob      = NULL;
    object     *marked  = NULL;
    CFParm      CFP;
    int         mode;

    get_lua_args(L, "OO?Oi", &self, &target, &ob, &mode);

    if (mode == 2)
    {
        if (!target)
            luaL_error(L, "Parameter 2 must be a GameObject for mode IDENTIFY_MARKED");
        marked = ob->data.object;
    }
    else if (mode == 0 || mode == 1)
    {
        if (ob)
            luaL_error(L, "Parameter 2 must be nil for modes IDENTIFY_NORMAL and IDENTIFY_ALL");
    }
    else
        luaL_error(L, "Mode must be IDENTIFY_NORMAL, IDENTIFY_ALL or IDENTIFY_MARKED");

    CFP.Value[0] = (void *) WHO;
    CFP.Value[1] = (void *) target->data.object;
    CFP.Value[2] = (void *) marked; /* is used when we use mode == 2 */
    CFP.Value[3] = (void *) &mode;
    (PlugHooks[HOOK_IDENTIFYOBJECT]) (&CFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Save                                                  */
/* Lua    : object:Save()                                                    */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_Save(lua_State *L)
{
    lua_object     *self;
    static char    *result;
    CFParm         *CFR, CFP;

    get_lua_args(L, "O", &self);

    CFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_DUMPOBJECT]) (&CFP);
    result = (char *) (CFR->Value[0]);
    free(CFR);

    lua_pushstring(L, result);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetIP                                                 */
/* Lua    : object:GetIP()                                                   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetIP(lua_State *L)
{
    lua_object     *self;
    static char    *result;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER)
        return 0;

    if (CONTR(WHO))
    {
        result = CONTR(WHO)->socket.ip_host;
        lua_pushstring(L, result);
        return 1;
    }
    else
    {
        LOG(llevDebug, "LUA - Error - This object has no controller\n");
        lua_pushstring(L, "");
        return 1;
    }
}

/*****************************************************************************/
/* Name   : GameObject_GetArchName                                           */
/* Lua    : object:GetArchName()                                             */
/* Info   : Returns the name of object's arhetype.                           */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetArchName(lua_State *L)
{
    lua_object *self;
    get_lua_args(L, "O", &self);

    lua_pushstring(L, WHO->arch->name);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_ShowCost                                              */
/* Lua    : object:ShowCost(value)                                           */
/* Info   : Returns a string describing value as x gold, x silver, x copper  */
/*          mode 0: "4 gold coins, 3 silver coins, ..." (default)            */
/*          mode 1: "4g, 3s, ..."                                            */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_ShowCost(lua_State *L)
{
    lua_object *self;
    sint64      value;
    int         mode = 0;

    get_lua_args(L, "OI|i", &self, &value, &mode);

    lua_pushstring(L, hooks->cost_string_from_value(value, mode));

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetItemCost                                           */
/* Lua    : object:GetItemCost(object,type)                                  */
/* Info   : type is one of game.COST_TRUE, game.COST_BUY or game.COST_SELL   */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_GetItemCost(lua_State *L)
{
    lua_object *self;
    lua_object *whatptr;
    int         flag;
    sint64      cost;

    get_lua_args(L, "OOi", &self, &whatptr, &flag);

    cost = hooks->query_cost(WHAT, WHO, flag);

    /* possible data loss from 64bit integer to double! */
    lua_pushnumber(L, (double) cost);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AddMoney                                              */
/* Lua    : object:AddMoney(copper, silver, gold, mithril)                   */
/* Info   : adds to inventory of caller coin object = money                  */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_AddMoney(lua_State *L)
{
    lua_object *self;
    int            c, s, g, m;

    get_lua_args(L, "Oiiii", &self, &c, &s, &g, &m);

    hooks->add_money_to_player(WHO, c, s, g, m);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_AddMoneyEx                                            */
/* Lua    : object:AddMoneyEx(copper, silver, gold, mithril)                 */
/* Info   : Same as AddMoney but with message to player how much he got      */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_AddMoneyEx(lua_State *L)
{
    lua_object *self;
    char        buf[MAX_BUF];
    int         c, s, g, m, flag=FALSE;

    get_lua_args(L, "Oiiii", &self, &c, &s, &g, &m);

    hooks->add_money_to_player(WHO, c, s, g, m);

    strcpy(buf, "You got");
    if(m)
    {
        sprintf(buf, "%s %d %s", buf, m, "mithril");
        flag = TRUE;
    }
    if(g)
    {
        sprintf(buf, "%s%s %d %s",buf,  flag?" and ":"", g,"gold");
        flag = TRUE;
    }
    if(s)
    {
        sprintf(buf, "%s%s %d %s",buf,  flag?" and ":"", s, "silver");
        flag = TRUE;
    }
    if(c)
        sprintf(buf, "%s%s %d %s",buf,  flag?" and ":"", c, "copper");

    strcat(buf, " coin.");
    hooks->new_draw_info(NDI_UNIQUE | NDI_NAVY, 0, WHO, buf);

    return 0;
}

/* TODO: add int64 to pushnumber() */
/*****************************************************************************/
/* Name   : GameObject_GetMoney                                              */
/* Lua    : object:GetMoney()                                                */
/* Info   : returns the amount of money the object carries in copper         */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_GetMoney(lua_State *L)
{
    lua_object *self;
    sint64 amount;

    get_lua_args(L, "O", &self);

    amount = hooks->query_money(WHO);

    /* possible data loss from 64bit integer to double! */
    lua_pushnumber(L, (double)amount);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PayForItem                                            */
/* Lua    : object:PayForItem(object)                                        */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_PayForItem(lua_State *L)
{
    lua_object *self;
    lua_object *whatptr;
    int         val;

    get_lua_args(L, "OO", &self, &whatptr);

    val = hooks->pay_for_item(WHAT, WHO);

    lua_pushnumber(L, val);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PayAmount                                             */
/* Lua    : object:PayAmount(value)                                          */
/* Info   : If object has enough money, value copper will be deducted from   */
/*          object, and 1 will be returned. Otherwise returns 0              */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_PayAmount(lua_State *L)
{
    lua_object *self;
    sint64      to_pay;
    int         val;

    get_lua_args(L, "OI", &self, &to_pay);

    val = hooks->pay_for_amount(to_pay, WHO);

    lua_pushnumber(L, val);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SendCustomCommand                                     */
/* Lua    : object:SendCustomCommand(customcommand)                          */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_SendCustomCommand(lua_State *L)
{
    lua_object *self;
    char       *customcmd;
    CFParm      CFP;

    get_lua_args(L, "Os", &self, &customcmd);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (customcmd);
    (PlugHooks[HOOK_SENDCUSTOMCOMMAND]) (&CFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Clone                                                 */
/* Lua    : object:Clone(mode)                                               */
/* Info   : mode = game.CLONE_WITH_INVENTORY (default) or                    */
/*          game.CLONE_WITHOUT_INVENTORY                                     */
/*          You should do something with the clone.                          */
/*          SetPosition() and InsertInside() are useful functions for this.  */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Clone(lua_State *L)
{
    lua_object *self;
    CFParm     *CFR, CFP;
    int         mode    = 0;
    object     *clone;

    get_lua_args(L, "O|i", &self, &mode);

    CFP.Value[0] = (void *) (WHO);
    CFP.Value[1] = (void *) (&mode);

    CFR = (PlugHooks[HOOK_CLONEOBJECT]) (&CFP);

    clone = (object *) (CFR->Value[0]);
    free(CFR);

    if (clone->type == PLAYER || QUERY_FLAG(clone, FLAG_IS_PLAYER))
    {
        clone->type = MONSTER;
        CLEAR_FLAG(clone, FLAG_IS_PLAYER);
    }

    return push_object(L, &GameObject, clone);
}

/*****************************************************************************/
/* Name   : GameObject_GetAI                                                 */
/* Lua    : object:GetAI()                                                   */
/* Info   : Get the AI object for a mob. Mostly useful in behaviours.        */
/*          Will return nil if the mob's AI hasn't been initialized yet      */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetAI(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (self->data.object->type != MONSTER)
        luaL_error(L, "Can only get AI from monsters");

    if(self->data.object->custom_attrset == NULL)
        return 0;

    return push_object(L, &AI, self->data.object);
}

/*****************************************************************************/
/* Name   : GameObject_GetVector                                             */
/* Lua    : object:GetVector(other)                                          */
/* Info   : Get the distance and direction from object to other              */
/*          Returns NIL if it couldn't be calculated (either object wasn't   */
/*          on a map, they were in separate mapsets or too far away from     */
/*          eachother)                                                       */
/*          On success, returns the following 4 values:                      */
/*          - distance (absolute euclidian distance)                         */
/*          - direction (0-8), corresponds to game.NORTH, game.EAST etc      */
/*          - x distance (can be negative)                                   */
/*          - y distance (can be negative)                                   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetVector(lua_State *L)
{
    lua_object *self, *other;
    rv_vector rv;

    get_lua_args(L, "OO", &self, &other);

    if(! hooks->get_rangevector(self->data.object, other->data.object, &rv, RV_DIAGONAL_DISTANCE))
        return 0;

    lua_pushnumber(L, rv.distance);
    lua_pushnumber(L, rv.direction);
    lua_pushnumber(L, rv.distance_x);
    lua_pushnumber(L, rv.distance_y);

    return 4;
}

/*****************************************************************************/
/* Name   : GameObject_GetAnimation                                          */
/* Lua    : object:GetAnimation()                                            */
/* Info   : Returns the name of object's animation, if any.                  */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetAnimation(lua_State *L)
{
    lua_object *self;
    get_lua_args(L, "O", &self);
    lua_pushstring(L, (* hooks->animations)[WHO->animation_id].name);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetInvAnimation                                       */
/* Lua    : object:GetInvAnimation()                                         */
/* Info   : Returns the name of object's inventory animation, if any.        */
/* Status : Tested                                                           */
/* Version: Introduced in beta 4 pre4                                        */
/*****************************************************************************/
static int GameObject_GetInvAnimation(lua_State *L)
{
    lua_object *self;
    get_lua_args(L, "O", &self);
    lua_pushstring(L, (* hooks->animations)[WHO->inv_animation_id].name);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetFace                                               */
/* Lua    : object:GetFace()                                                 */
/* Info   : Returns the name of object's face, if any.                       */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetFace(lua_State *L)
{
    lua_object *self;
    get_lua_args(L, "O", &self);

    if(WHO->face)
    {
        lua_pushstring(L, WHO->face->name);
        return 1;
    } else
        return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetInvFace                                            */
/* Lua    : object:GetInvFace()                                              */
/* Info   : Returns the name of object's inventory face, if any.             */
/* Status : Tested                                                           */
/* Version: Introduced in beta 4 pre4                                        */
/*****************************************************************************/
static int GameObject_GetInvFace(lua_State *L)
{
    lua_object *self;
    get_lua_args(L, "O", &self);

    if(WHO->inv_face)
    {
        lua_pushstring(L, WHO->inv_face->name);
        return 1;
    } else
        return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetAnimation                                          */
/* Lua    : object:SetAnimation(anim)                                        */
/* Info   : Sets object's animation.                                         */
/*          Note that an object will only be animated if object.f_is_animated*/
/*          is true                                                          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetAnimation(lua_State *L)
{
    lua_object *self;
    char *animation;
    int id;

    get_lua_args(L, "Os", &self, &animation);

    id = hooks->find_animation(animation);
    if(id == 0)
        luaL_error(L, "no such animation exists: %s", animation);

    WHO->animation_id = id;

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetInvAnimation                                       */
/* Lua    : object:SetInvAnimation(anim)                                     */
/* Info   : Sets object's inventory animation.                               */
/*          Note that an object will only be animated if object.f_is_animated*/
/*          is true                                                          */
/* Status : Tested                                                           */
/* Version: Introduced in beta 4 pre4                                        */
/*****************************************************************************/
static int GameObject_SetInvAnimation(lua_State *L)
{
    lua_object *self;
    char *animation;
    int id;

    get_lua_args(L, "Os", &self, &animation);

    id = hooks->find_animation(animation);
    if(id == 0)
        luaL_error(L, "no such animation exists: %s", animation);

    WHO->inv_animation_id = id;

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetFace                                               */
/* Lua    : object:SetFace(face)                                             */
/* Info   : Sets object's face.                                              */
/*          If the object is animated (object.f_is_animated == true), then   */
/*          this value will likely be replaced at the next animation step    */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetFace(lua_State *L)
{
    lua_object *self;
    char *face;
    int id;

    get_lua_args(L, "Os", &self, &face);

    id = hooks->find_face(face, -1);
    if(id == -1)
        luaL_error(L, "no such face exists: %s", STRING_SAFE(face));

    WHO->face = &(*hooks->new_faces)[id];

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetInvFace                                            */
/* Lua    : object:SetInvFace(face)                                          */
/* Info   : Sets object's inventory face.                                    */
/*          If the object is animated (object.f_is_animated == true), then   */
/*          this value will likely be replaced at the next animation step    */
/* Status : Tested                                                           */
/* Version: Introduced in beta 4 pre4                                        */
/*****************************************************************************/
static int GameObject_SetInvFace(lua_State *L)
{
    lua_object *self;
    char *face;
    int id;

    get_lua_args(L, "Os", &self, &face);

    id = hooks->find_face(face, -1);
    if(id == -1)
        luaL_error(L, "no such face exists: %s", STRING_SAFE(face));

    WHO->inv_face = &(*hooks->new_faces)[id];

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_MakePet                                               */
/* Lua    : object:MakePet(owner, mode)                                      */
/* Info   : Makes @object into a pet owned by @owner. @object must be a      */
/*          non-pet monster, and @owner must be a player.                    */
/*          If mode is 0, a normal pet addition is done, which may fail for  */
/*          several reasons. If mode is 1, the pet is forced which may still */
/*          fail but not as often.                                           */
/*          Normally you should use 0, but in some cases where for example a */
/*          quest requires a specific pet you could try 1.                   */
/*          Returns true if object is made into a pet, and false otherwise.  */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_MakePet(lua_State *L)
{
    lua_object *self, *owner;
    int result, mode = 0;

    get_lua_args(L, "OO|i", &self, &owner, &mode);

    result = hooks->add_pet(owner->data.object, WHO, mode);

    lua_pushboolean(L, !result);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetPets                                               */
/* Lua    : object:GetPets()                                                 */
/* Info   : Returns an array of all pets currently owned by @object          */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_GetPets(lua_State *L)
{
    lua_object *self;
    int i=1;

    get_lua_args(L, "O", &self);

#define PET_VALID(pet_ol, _owner_) \
    (OBJECT_VALID((pet_ol)->objlink.ob, (pet_ol)->id) && \
     (pet_ol)->objlink.ob->owner == (_owner_) && (pet_ol)->objlink.ob->owner_count == (_owner_)->count)

    lua_newtable(L);
    if(WHO->type == PLAYER)
    {
        objectlink *ol;
        for(ol = CONTR(WHO)->pets; ol; ol = ol->next)
            if(PET_VALID(ol, WHO))
            {
                push_object(L, &GameObject, ol->objlink.ob);
                lua_rawseti(L, -2, i++);
            }
    }

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetGmasterMode                                        */
/* Lua    : object:GetGmasterMode()                                          */
/* Info   : Only works for player objects. Returns one of the                */
/*          Game.GMASTER_MODE_* constants:                                   */
/*              Game.GMASTER_MODE_NO    if the object is not a MW/Vol/GM/MM  */
/*              Game.GMASTER_MODE_MW    if the object is a MW
/*              Game.GMASTER_MODE_VOL   if the object is a Vol               */
/*              Game.GMASTER_MODE_GM    if the object is a GM                */
/*              Game.GMASTER_MODE_MM    if the object is a MM                */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_GetGmasterMode(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "GetGmasterMode() can only be called on a legal player object.");

    lua_pushnumber(L, CONTR(WHO)->gmaster_mode);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetPlayerWeightLimit                                  */
/* Lua    : object:GetPlayerWeightLimit()                                    */
/* Info   : Only works for player objects. Returns the real weight limit     */
/*        : of a player including stat bonus                                 */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_GetPlayerWeightLimit(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "GetPlayerWeightLimit() can only be called on a legal player object.");

    lua_pushnumber(L, CONTR(WHO)->weight_limit);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetGroup                                              */
/* Lua    : object:GetGroup()                                                */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          The function takes no arguments.                                 */
/*          If the player is in a group, the return is a table with numerical*/
/*          where [1] is the leader and [2] onwards are the other members.   */
/*          Otherwise, the return is nil.                                    */
/* Status : Untested                                                         */
/* TODO   : Much.                                                            */
/*****************************************************************************/
static int GameObject_GetGroup(lua_State *L)
{
    lua_object *self;
    object     *member,
               *leader;
    int         nrof;

    get_lua_args(L, "O", &self);

    /* Only players can be in groups */
    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "GetGroup() can only be called on a player!");

    /* No leader means no group. */
    if (!(leader = CONTR(WHO)->group_leader))
    {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    for (member = leader, nrof = CONTR(leader)->group_nrof; member && nrof > 0;  member = CONTR(member)->group_next, nrof--)
    {
        push_object(L, &GameObject, member);
        lua_rawset(L, -2);
    }

    return 1;
}


/* FUNCTIONEND -- End of the GameObject methods. */


#if 0
/*****************************************************************************/
/* Name   : GameObject_GetUnmodifiedAttribute                           */
/* Lua    : object:GetUnmodifiedAttribute(attribute_id)                      */
/* Status : UNFINISHED <- fields not available...                            */
/*****************************************************************************/
static int GameObject_GetUnmodifiedAttribute(GameObject* whoptr, PyObject* args)
{
    lua_object *self;
    int fieldno;

    get_lua_args(L, "Oi", &fieldno))
        return NULL;

    if(fieldno < 0 || fieldno >= NUM_OBJFIELDS)
        luaL_error(L, "Illegal field ID");

    if(WHO->type != PLAYER)
        luaL_error(L, "Can only be used on players");

    luaL_error(L, "Not implemented");
#if 0
    switch(fieldno) {
        case OBJFIELD_STAT_INT: return Py_BuildValue("i", CONTR(WHO)->orig_stats.Int);
        case OBJFIELD_STAT_STR: return Py_BuildValue("i", CONTR(WHO)->orig_stats.Str);
        case OBJFIELD_STAT_CHA: return Py_BuildValue("i", CONTR(WHO)->orig_stats.Cha);
        case OBJFIELD_STAT_WIS: return Py_BuildValue("i", CONTR(WHO)->orig_stats.Wis);
        case OBJFIELD_STAT_DEX: return Py_BuildValue("i", CONTR(WHO)->orig_stats.Dex);
        case OBJFIELD_STAT_CON: return Py_BuildValue("i", CONTR(WHO)->orig_stats.Con);
        case OBJFIELD_STAT_POW: return Py_BuildValue("i", CONTR(WHO)->orig_stats.Pow);

        default:
            luaL_error(L, "No unmodified version of attribute available");
    }
#endif
}

#endif

/******************
 * Old, leftover stuff. Will clean up later...
 */

/* TODO: Hmm... might want to keep this? */
#if 0
/*****************************************************************************/
/* Name   : GameObject_SetDirection                                     */
/* Lua    : Daimonin.SetDirection(object, value)                             */
/* Status : Untested                                                         */
/*****************************************************************************/
/* this function will fail imho - for animation[] we need to call a hook! */

static int GameObject_SetDirection(lua_State *L)
{
    int value;
    lua_object *whoptr;

    get_lua_args(L, "OO!i", &GameObjectType, &whoptr,&value))
        return NULL;

    WHO->direction = value;
    SET_ANIMATION(WHO, WHO->direction);
    Py_INCREF(Py_None);
    return 0;
}
#endif

/* I'm going to replace those with a Reorder() call. Can't find any reason to
 * fiddle with those except for changing object order. Don't want possible
 * dangerous functions here...
 */
#if 0

/*****************************************************************************/
/* Name   : GameObject_SetNextObject                                    */
/* Lua    : Daimonin.SetNextObject(object,object)                            */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_SetNextObject(lua_State *L)
{
    lua_object *whoptr;
    lua_object *whatptr;
    get_lua_args(L, "OO!O!", &GameObjectType, &whoptr, &Daimonin_ObjectType, &whatptr))
        return NULL;

    WHO->below = WHAT;
    Py_INCREF(Py_None);
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetPreviousObject                                */
/* Lua    : Daimonin.SetPreviousObject(object,object)                        */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_SetPreviousObject(lua_State *L)
{
    lua_object *whoptr;
    lua_object *whatptr;
    get_lua_args(L, "OO!O!", &GameObjectType, &whoptr, &Daimonin_ObjectType, &whatptr))
        return NULL;

    WHO->above = WHAT;
    Py_INCREF(Py_None);
    return 0;
}

#endif
