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

/* First let's include the header file needed */
#include <global.h>
#include <daimonin_object.h>

static struct method_decl GameObject_methods[] =
{
    {"AcquireSkill",           (lua_CFunction) GameObject_AcquireSkill},
    {"AcquireSpell",           (lua_CFunction) GameObject_AcquireSpell},
    {"ActivateRune",           (lua_CFunction) GameObject_ActivateRune},
    {"AddBuff",                (lua_CFunction) GameObject_AddBuff},
    {"AddMoney",               (lua_CFunction) GameObject_AddMoney},
    {"AddMoneyEx",             (lua_CFunction) GameObject_AddMoneyEx},
    {"AddOneDropQuest",        (lua_CFunction) GameObject_AddOneDropQuest},
    {"AddQuest",               (lua_CFunction) GameObject_AddQuest},
    {"AddQuestItem",           (lua_CFunction) GameObject_AddQuestItem},
    {"AddQuestTarget",         (lua_CFunction) GameObject_AddQuestTarget},
    {"AdjustLightSource",      (lua_CFunction) GameObject_AdjustLightSource},
    {"Apply",                  (lua_CFunction) GameObject_Apply},
    {"CastSpell",              (lua_CFunction) GameObject_CastSpell},
    {"ChannelMsg",             (lua_CFunction) GameObject_ChannelMsg},
    {"CheckBuff",              (lua_CFunction) GameObject_CheckBuff},
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
    {"FindNextObject",         (lua_CFunction) GameObject_FindNextObject},
    {"FindSkill",              (lua_CFunction) GameObject_FindSkill},
    {"Fix",                    (lua_CFunction) GameObject_Fix},
    {"GetAccountName",         (lua_CFunction) GameObject_GetAccountName},
    {"GetAI",                  (lua_CFunction) GameObject_GetAI},
    {"GetAlignmentForce",      (lua_CFunction) GameObject_GetAlignmentForce},
    {"GetAnimation",           (lua_CFunction) GameObject_GetAnimation},
    {"GetArchName",            (lua_CFunction) GameObject_GetArchName},
    {"GetCombatMode",          (lua_CFunction) GameObject_GetCombatMode},
    {"GetConnection",          (lua_CFunction) GameObject_GetConnection},
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
    {"GetPersonalLight",       (lua_CFunction) GameObject_GetPersonalLight},
    {"GetPets",                (lua_CFunction) GameObject_GetPets},
    {"GetPlayerInfo",          (lua_CFunction) GameObject_GetPlayerInfo},
    {"GetPlayerWeightLimit",   (lua_CFunction) GameObject_GetPlayerWeightLimit},
    {"GetQuest",               (lua_CFunction) GameObject_GetQuest},
    {"GetRepairCost",          (lua_CFunction) GameObject_GetRepairCost},
    {"GetSkill",               (lua_CFunction) GameObject_GetSkill},
    {"GetTarget",              (lua_CFunction) GameObject_GetTarget},
    {"GetVector",              (lua_CFunction) GameObject_GetVector},
    {"Give",                   (lua_CFunction) GameObject_Give},
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
    {"RemoveBuff",             (lua_CFunction) GameObject_RemoveBuff},
    {"RemoveQuestItem",        (lua_CFunction) GameObject_RemoveQuestItem},
    {"Repair",                 (lua_CFunction) GameObject_Repair},
    {"Save",                   (lua_CFunction) GameObject_Save},
    {"Say",                    (lua_CFunction) GameObject_Say},
    {"SayTo",                  (lua_CFunction) GameObject_SayTo},
    {"SendCustomCommand",      (lua_CFunction) GameObject_SendCustomCommand},
    {"SetAlignment",           (lua_CFunction) GameObject_SetAlignment},
    {"SetAnimation",           (lua_CFunction) GameObject_SetAnimation},
    {"SetCombatMode",          (lua_CFunction) GameObject_SetCombatMode},
    {"SetFace",                (lua_CFunction) GameObject_SetFace},
    {"SetGender",              (lua_CFunction) GameObject_SetGender},
    {"SetGod",                 (lua_CFunction) GameObject_SetGod},
    {"SetInvAnimation",        (lua_CFunction) GameObject_SetInvAnimation},
    {"SetInvFace",             (lua_CFunction) GameObject_SetInvFace},
    {"SetPersonalLight",       (lua_CFunction) GameObject_SetPersonalLight},
    {"SetPosition",            (lua_CFunction) GameObject_SetPosition},
    {"SetQuestStatus",         (lua_CFunction) GameObject_SetQuestStatus},
    {"SetRank",                (lua_CFunction) GameObject_SetRank},
    {"SetSaveBed",             (lua_CFunction) GameObject_SetSaveBed},
    {"SetSkill",               (lua_CFunction) GameObject_SetSkill},
    {"SetTarget",              (lua_CFunction) GameObject_SetTarget},
    {"ShowCost",               (lua_CFunction) GameObject_ShowCost},
    {"Sound",                  (lua_CFunction) GameObject_Sound},
    {"StartNewInstance",       (lua_CFunction) GameObject_StartNewInstance},
    {"UpdateQuest",            (lua_CFunction) GameObject_UpdateQuest},
    {"Withdraw",               (lua_CFunction) GameObject_Withdraw},
    {"Write",                  (lua_CFunction) GameObject_Write},

    {NULL, NULL}
};

/* All entries MUST be in same order as field_id enum above */
struct attribute_decl GameObject_attributes[] =
{
    {"below",                 FIELDTYPE_OBJECT,    offsetof(object_t, below),                     FIELDFLAG_READONLY, 0},
    {"above",                 FIELDTYPE_OBJECT,    offsetof(object_t, above),                     FIELDFLAG_READONLY, 0},
    {"inventory",             FIELDTYPE_OBJECT,    offsetof(object_t, inv),                       FIELDFLAG_READONLY, 0},
    {"environment",           FIELDTYPE_OBJECT,    offsetof(object_t, env),                       FIELDFLAG_READONLY, 0},
    {"more",                  FIELDTYPE_OBJECT,    offsetof(object_t, more),                      FIELDFLAG_READONLY, 0},
    {"head",                  FIELDTYPE_OBJECT,    offsetof(object_t, head),                      FIELDFLAG_READONLY, 0},
    {"map",                   FIELDTYPE_MAP,       offsetof(object_t, map),                       FIELDFLAG_READONLY, 0},
    {"count",                 FIELDTYPE_UINT32,    offsetof(object_t, count),                     FIELDFLAG_READONLY, 0},
    {"name",                  FIELDTYPE_SHSTR,     offsetof(object_t, name),                      FIELDFLAG_PLAYER_READONLY, 0},
    {"title",                 FIELDTYPE_SHSTR,     offsetof(object_t, title),                     0,                  0},
    {"race",                  FIELDTYPE_SHSTR,     offsetof(object_t, race),                      0,                  0},
    {"slaying",               FIELDTYPE_SHSTR,     offsetof(object_t, slaying),                   0,                  0},
    {"message",               FIELDTYPE_SHSTR,     offsetof(object_t, msg),                       0,                  0},
    /* TODO: limited to >=0 */
    {"weight",                FIELDTYPE_SINT32,    offsetof(object_t, weight),                    0,                  0},
    {"weight_limit",          FIELDTYPE_UINT32,    offsetof(object_t, weight_limit),              FIELDFLAG_PLAYER_READONLY, 0},
    {"carrying",              FIELDTYPE_SINT32,    offsetof(object_t, carrying),                  FIELDFLAG_READONLY, 0},
    {"path_attuned",          FIELDTYPE_UINT32,    offsetof(object_t, path_attuned),              0,                  0},
    {"path_repelled",         FIELDTYPE_UINT32,    offsetof(object_t, path_repelled),             0,                  0},
    {"path_denied",           FIELDTYPE_UINT32,    offsetof(object_t, path_denied),               0,                  0},
    {"value",                 FIELDTYPE_SINT64,    offsetof(object_t, value),                     0,                  0},
    /* TODO: Max 100000 */
    {"quantity",              FIELDTYPE_UINT32,    offsetof(object_t, nrof),                      0,                  0},
    /* TODO: I don't know what these do, or if they should be accessible... */
    {"damage_round_tag",      FIELDTYPE_UINT32,    offsetof(object_t, damage_round_tag),          0,                  0},
    {"update_tag",            FIELDTYPE_UINT32,    offsetof(object_t, update_tag),                0,                  0},
    /* TODO: make enemy & owner settable (requires HOOKS for set_npc_enemy() and set_owner()) */
    {"enemy",                 FIELDTYPE_OBJECTREF, offsetof(object_t, enemy),                     FIELDFLAG_READONLY, offsetof(object_t, enemy_count)},
    /* TODO: remove    {"attacked_by",  FIELDTYPE_OBJECTREF, offsetof(object_t, attacked_by), FIELDFLAG_READONLY, offsetof(object_t, attacked_by_count)}, */
    {"owner",                 FIELDTYPE_OBJECTREF, offsetof(object_t, owner),                     FIELDFLAG_READONLY, offsetof(object_t, owner_count)},
    {"x",                     FIELDTYPE_SINT16,    offsetof(object_t, x),                         FIELDFLAG_READONLY, 0},
    {"y",                     FIELDTYPE_SINT16,    offsetof(object_t, y),                         FIELDFLAG_READONLY, 0},
    {"last_damage",           FIELDTYPE_UINT16,    offsetof(object_t, last_damage),               0,                  0},
    {"terrain_type",          FIELDTYPE_UINT16,    offsetof(object_t, terrain_type),              0,                  0},
    {"terrain_flag",          FIELDTYPE_UINT16,    offsetof(object_t, terrain_flag),              0,                  0},
    {"material",              FIELDTYPE_UINT16,    offsetof(object_t, material),                  0,                  0},
    {"material_real",         FIELDTYPE_SINT16,    offsetof(object_t, material_real),             0,                  0},
    {"last_heal",             FIELDTYPE_SINT16,    offsetof(object_t, last_heal),                 0,                  0},
    /* TODO: Limit to max 16000 ? */
    {"last_sp",               FIELDTYPE_SINT16,    offsetof(object_t, last_sp),                   0,                  0},
    /* TODO: Limit to max 16000 ? */
    {"last_grace",            FIELDTYPE_SINT16,    offsetof(object_t, last_grace),                0,                  0},
    {"last_eat",              FIELDTYPE_SINT16,    offsetof(object_t, last_eat),                  0,                  0},
    /* TODO: will require animation lookup function. How about face, is that a special anim? */
    {"max_buffs",             FIELDTYPE_UINT8,     offsetof(object_t, max_buffs),                 0,                  0},
    {"magic",                 FIELDTYPE_SINT8,     offsetof(object_t, magic),                     0,                  0},
    {"state",                 FIELDTYPE_UINT8,     offsetof(object_t, state),                     FIELDFLAG_READONLY, 0},
    {"level",                 FIELDTYPE_SINT8,     offsetof(object_t, level),                     FIELDFLAG_PLAYER_READONLY, 0},
    {"direction",             FIELDTYPE_SINT8,     offsetof(object_t, direction),                 0,                  0},
    {"facing",                FIELDTYPE_SINT8,     offsetof(object_t, facing),                    0,                  0},
    {"quick_pos",             FIELDTYPE_UINT8,     offsetof(object_t, quick_pos),                 0,                  0},
    {"type",                  FIELDTYPE_UINT8,     offsetof(object_t, type),                      FIELDFLAG_READONLY, 0},
    {"sub_type_1",            FIELDTYPE_UINT8,     offsetof(object_t, sub_type1),                 0,                  0},
    {"item_quality",          FIELDTYPE_UINT8,     offsetof(object_t, item_quality),              0,                  0},
    {"item_condition",        FIELDTYPE_UINT8,     offsetof(object_t, item_condition),            0,                  0},
    {"item_race",             FIELDTYPE_UINT8,     offsetof(object_t, item_race),                 0,                  0},
    {"item_level",            FIELDTYPE_UINT8,     offsetof(object_t, item_level),                0,                  0},
    {"item_skill",            FIELDTYPE_UINT8,     offsetof(object_t, item_skill),                0,                  0},
    {"glow_radius",           FIELDTYPE_SINT8,     offsetof(object_t, glow_radius),               FIELDFLAG_READONLY, 0},
    {"anim_enemy_dir",        FIELDTYPE_SINT8,     offsetof(object_t, anim_enemy_dir),            0,                  0},
    {"anim_moving_dir",       FIELDTYPE_SINT8,     offsetof(object_t, anim_moving_dir),           0,                  0},
    {"anim_enemy_dir_last",   FIELDTYPE_SINT8,     offsetof(object_t, anim_enemy_dir_last),       0,                  0},
    {"anim_moving_dir_last",  FIELDTYPE_SINT8,     offsetof(object_t, anim_moving_dir_last),      0,                  0},
    {"anim_last_facing",      FIELDTYPE_SINT8,     offsetof(object_t, anim_last_facing),          0,                  0},
    {"anim_last_facing_last", FIELDTYPE_SINT8,     offsetof(object_t, anim_last_facing_last),     0,                  0},
    {"anim_speed",            FIELDTYPE_UINT8,     offsetof(object_t, anim_speed),                0,                  0},
    {"anim_speed_last",       FIELDTYPE_UINT8,     offsetof(object_t, anim_speed_last),           0,                  0},
    {"run_away",              FIELDTYPE_UINT8,     offsetof(object_t, run_away),                  0,                  0},
    {"layer",                 FIELDTYPE_UINT8,     offsetof(object_t, layer),                     FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_impact",         FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_IMPACT]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_slash",          FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_SLASH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_cleave",         FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_CLEAVE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_pierce",         FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_PIERCE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_fire",           FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_FIRE]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_cold",           FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_COLD]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_electricity",    FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_ELECTRICITY]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_poison",         FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_POISON]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_acid",           FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_ACID]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_sonic",          FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_SONIC]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_channelling",    FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_CHANNELLING]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_corruption",     FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_CORRUPTION]),   FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_psionic",        FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_PSIONIC]),      FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_light",          FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_LIGHT]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_shadow",         FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_SHADOW]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_lifesteal",      FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_LIFESTEAL]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_aether",         FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_AETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_nether",         FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_NETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_chaos",          FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_CHAOS]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_death",          FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_DEATH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_weaponmagic",    FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_WEAPONMAGIC]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_godpower",       FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_GODPOWER]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_drain",          FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_DRAIN]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_depletion",      FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_DEPLETION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_countermagic",   FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_COUNTERMAGIC]), FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_cancellation",   FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_CANCELLATION]), FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_confusion",      FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_CONFUSION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_fear",           FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_FEAR]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_slow",           FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_SLOW]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_paralyze",       FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_PARALYZE]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"resist_snare",          FIELDTYPE_SINT8,     offsetof(object_t, resist[ATNR_SNARE]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_impact",         FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_IMPACT]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_slash",          FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_SLASH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_cleave",         FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_CLEAVE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_pierce",         FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_PIERCE]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_fire",           FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_FIRE]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_cold",           FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_COLD]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_electricity",    FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_ELECTRICITY]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_poison",         FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_POISON]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_acid",           FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_ACID]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_sonic",          FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_SONIC]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_channelling",    FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_CHANNELLING]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_corruption",     FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_CORRUPTION]),   FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_psionic",        FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_PSIONIC]),      FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_light",          FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_LIGHT]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_shadow",         FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_SHADOW]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_lifesteal",      FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_LIFESTEAL]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_aether",         FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_AETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_nether",         FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_NETHER]),       FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_chaos",          FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_CHAOS]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_death",          FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_DEATH]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_weaponmagic",    FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_WEAPONMAGIC]),  FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_godpower",       FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_GODPOWER]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_drain",          FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_DRAIN]),        FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_depletion",      FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_DEPLETION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_countermagic",   FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_COUNTERMAGIC]), FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_cancellation",   FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_CANCELLATION]), FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_confusion",      FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_CONFUSION]),    FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_fear",           FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_FEAR]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_slow",           FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_SLOW]),         FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_paralyze",       FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_PARALYZE]),     FIELDFLAG_PLAYER_READONLY, 0},
    {"attack_snare",          FIELDTYPE_SINT8,     offsetof(object_t, attack[ATNR_SNARE]),        FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: -10.0 < speed < 10.0, also might want to call update_object_speed() */
    {"speed",                 FIELDTYPE_FLOAT,     offsetof(object_t, speed),                     FIELDFLAG_PLAYER_READONLY, 0},
    {"speed_left",            FIELDTYPE_FLOAT,     offsetof(object_t, speed_left),                0,                  0},
    {"weapon_speed",          FIELDTYPE_FLOAT,     offsetof(object_t, weapon_speed),              0,                  0},
    {"weapon_speed_left",     FIELDTYPE_FLOAT,     offsetof(object_t, weapon_speed_left),         0,                  0},
    /* Stats */
    {"experience",            FIELDTYPE_SINT32,    offsetof(object_t, stats.exp),                 FIELDFLAG_PLAYER_READONLY, 0},
    {"hitpoints",             FIELDTYPE_SINT32,    offsetof(object_t, stats.hp),                  0,                  0},
    {"max_hitpoints",         FIELDTYPE_SINT32,    offsetof(object_t, stats.maxhp),               FIELDFLAG_PLAYER_READONLY, 0},
    {"spellpoints",           FIELDTYPE_SINT16,    offsetof(object_t, stats.sp),                  0,                  0},
    {"max_spellpoints",       FIELDTYPE_SINT16,    offsetof(object_t, stats.maxsp),               FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to +- 16000 ? */
    {"grace",                 FIELDTYPE_SINT16,    offsetof(object_t, stats.grace),               0,                  0},
    {"max_grace",             FIELDTYPE_SINT16,    offsetof(object_t, stats.maxgrace),            FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to max 999 (at least for players) ? */
    {"food",                  FIELDTYPE_SINT16,    offsetof(object_t, stats.food),                0,                  0},
    /* TODO: Limit to 0 <= dam <= 120 ? */
    {"damage",                FIELDTYPE_SINT16,    offsetof(object_t, stats.dam),                 FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to +-120 */
    {"weapon_class",          FIELDTYPE_SINT16,    offsetof(object_t, stats.wc),                  FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to +-120 */
    {"armour_class",          FIELDTYPE_SINT16,    offsetof(object_t, stats.ac),                  FIELDFLAG_PLAYER_READONLY, 0},
    /* TODO: Limit to +-30 (all  */
    {"strength",              FIELDTYPE_SINT8,     offsetof(object_t, stats.Str),                 FIELDFLAG_PLAYER_FIX, 0},
    {"dexterity",             FIELDTYPE_SINT8,     offsetof(object_t, stats.Dex),                 FIELDFLAG_PLAYER_FIX, 0},
    {"constitution",          FIELDTYPE_SINT8,     offsetof(object_t, stats.Con),                 FIELDFLAG_PLAYER_FIX, 0},
    {"wisdom",                FIELDTYPE_SINT8,     offsetof(object_t, stats.Wis),                 FIELDFLAG_PLAYER_FIX, 0},
    {"charisma",              FIELDTYPE_SINT8,     offsetof(object_t, stats.Cha),                 FIELDFLAG_PLAYER_FIX, 0},
    {"intelligence",          FIELDTYPE_SINT8,     offsetof(object_t, stats.Int),                 FIELDFLAG_PLAYER_FIX, 0},
    {"power",                 FIELDTYPE_SINT8,     offsetof(object_t, stats.Pow),                 FIELDFLAG_PLAYER_FIX, 0},
    {"thac0",                 FIELDTYPE_SINT8,     offsetof(object_t, stats.thac0),               FIELDFLAG_PLAYER_READONLY, 0},
    {"thacm",                 FIELDTYPE_SINT8,     offsetof(object_t, stats.thacm),               FIELDFLAG_PLAYER_READONLY, 0},

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
    /*   0 */ "f_sleep",
    /*   1 */ "f_confused",
    /*   2 */ "?f_paralyzed",
    /*   3 */ "f_scared",
    /*   4 */ "?f_is_eating",
    /*   5 */ "f_is_invisible",
    /*   6 */ "f_is_ethereal",
    /*   7 */ "f_is_blind",
    /*   8 */ "f_no_pick",
    /*   9 */ "f_walk_on",
    /*  10 */ "f_no_pass",
    /*  11 */ "f_is_animated",
    /*  12 */ "?f_is_initialized",
    /*  13 */ "f_flying",
    /*  14 */ NULL, // free
    /*  15 */ "f_friendly",
    /*  16 */ "?f_is_removed",
    /*  17 */ "f_been_applied",
    /*  18 */ "?f_inserted",
    /*  19 */ "f_treasure",
    /*  20 */ "f_is_neutral",
    /*  21 */ "f_see_invisible",
    /*  22 */ "f_can_roll",
    /*  23 */ "f_obscuresview",
    /*  24 */ "f_is_turnable",
    /*  25 */ "f_walk_off",
    /*  26 */ "f_fly_on",
    /*  27 */ "f_fly_off",
    /*  28 */ "f_is_used_up",
    /*  29 */ "f_identified",
    /*  30 */ "f_reflecting",
    /*  31 */ "f_changing",
    /*  32 */ "f_splitting",
    /*  33 */ "f_hitback",
    /*  34 */ "f_allowsview",
    /*  35 */ "f_blocksview",
    /*  36 */ "f_undead",
    /*  37 */ "?f_fix_player",
    /*  38 */ "f_unaggressive",
    /*  39 */ "f_can_reflect_missile",
    /*  40 */ "f_can_reflect_castable",
    /*  41 */ "f_no_spells",
    /*  42 */ "f_no_fix_player",
    /*  43 */ "f_is_evil",
    /*  44 */ "f_tear_down",
    /*  45 */ "f_run_away",
    /*  46 */ "f_pass_thru",
    /*  47 */ "f_can_pass_thru",
    /*  48 */ "?f_feared",
    /*  49 */ "f_is_good",
    /*  50 */ "f_no_drop",
    /*  51 */ "f_reg_f",
    /*  52 */ "f_has_ready_spell",
    /*  53 */ "f_surrendered",
    /*  54 */ "?f_rooted",
    /*  55 */ "?f_slowed",
    /*  56 */ "f_can_use_armour",
    /*  57 */ "f_can_use_weapon",
    /*  58 */ "f_can_use_ring",
    /*  59 */ "?f_in_activelist",
    /*  60 */ "f_has_ready_bow",
    /*  61 */ "f_xrays",
    /*  62 */ "?no_apply",
    /*  63 */ "f_can_stack",
    /*  64 */ "f_lifesave",
    /*  65 */ "f_is_magical",
    /*  66 */ "f_alive",
    /*  67 */ "f_stand_still",
    /*  68 */ "f_random_move",
    /*  69 */ "f_only_attack",
    /*  70 */ "?f_no_send",
    /*  71 */ "f_stealth",
    /*  72 */ "?is_giving",
    /*  73 */ "?f_is_linked",
    /*  74 */ "f_cursed",
    /*  75 */ "f_damned",
    /*  76 */ "f_see_anywhere",
    /*  77 */ "f_known_magical",
    /*  78 */ "f_known_cursed",
    /*  79 */ "f_can_open_door",
    /*  80 */ "f_is_thrown",
    /*  81 */ NULL, // free
    /*  82 */ NULL, // free
    /*  83 */ "f_is_male",
    /*  84 */ "f_is_female",
    /*  85 */ "f_applied",
    /*  86 */ "f_inv_locked",
    /*  87 */ "f_is_wooded",
    /*  88 */ "f_is_hilly",
    /*  89 */ "f_levitate",
    /*  90 */ "f_has_ready_weapon",
    /*  91 */ "f_no_skill_ident",
    /*  92 */ "f_use_dmg_info",
    /*  93 */ "f_can_see_in_dark",
    /*  94 */ "f_is_cauldron",
    /*  95 */ "f_is_dust",
    /*  96 */ "f_no_steal",
    /*  97 */ "f_one_hit",
    /*  98 */ NULL, // FLAG_CLIENT_SENT
    /*  99 */ "f_berserk",
    /* 100 */ "f_no_attack",
    /* 101 */ "f_invulnerable",
    /* 102 */ "f_quest_item",
    /* 103 */ "f_is_traped",
    /* 104 */ "f_proof_phy",
    /* 105 */ "f_proof_ele",
    /* 106 */ "f_proof_mag",
    /* 107 */ "f_proof_sph",
    /* 108 */ "?f_no_inv",
    /* 109 */ "?f_is_donation",
    /* 110 */ "?f_sys_object",
    /* 111 */ "?f_homeless_mob",
    /* 112 */ "f_unpaid",
    /* 113 */ "f_is_aged",
    /* 114 */ "f_make_invisible" ,
    /* 115 */ "f_make_ethereal",
    /* 116 */ "f_is_player",
    /* 117 */ "f_is_named",
    /* 118 */ "?f_spawn_mob",
    /* 119 */ "f_no_teleport",
    /* 120 */ "f_corpse",
    /* 121 */ "f_corpse_forced",
    /* 122 */ "f_player_only",
    /* 123 */ "f_no_prayers",
    /* 124 */ "f_one_drop",
    /* 125 */ "f_cursed_perm",
    /* 126 */ "f_damned_perm",
    /* 127 */ "f_door_closed",
    /* 128 */ "f_was_reflected",
    /* 129 */ "f_is_missile",
    /* 130 */ NULL, // free
    /* 131 */ NULL, // free
    /* 132 */ "f_is_assassin",
    /* 133 */ "f_auto_apply",
    /* 134 */ "?f_no_save",
    /* 135 */ "f_pass_ethereal",
    /* 136 */ "f_ego",
    /* 137 */ "f_egobound",
    /* 138 */ "f_egoclan",
    /* 139 */ "f_egolock",

    FLAGLIST_END_MARKER
};

/* This gets called before and after an attribute has been set in an object_t */
static int GameObject_setAttribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib, int before)
{
    object_t *who = obj->data.object;
    object_t *pl = hooks->is_player_inv(who);

    /* Pre-setting hook */
    if (before)
    {
        if (who->type == TYPE_SKILLGROUP ||
            who->type == TYPE_SKILL)
        {
            luaL_error(L, "Attributes on TYPE_SKILLGROUP and TYPE_SKILL objects are read only!");
            return 1;
        }
        else if (attrib->offset == offsetof(object_t, layer))
        {
            luaL_error(L, "The layer attribute must be in the range 0 to %d (use the LAYER_* constants)!",
                MSP_SLAYER_NROF - 1);
        }
    }
    else
    {
        /* recalculate carrying when a script changes an inventory object_t's
         * weight */
        if (attrib->offset == offsetof(object_t, weight) && pl)
            pl->carrying = hooks->sum_weight(pl);

        /* Special handling for some player stuff */
        if (who->type == PLAYER)
        {
            if (attrib->offset == offsetof(object_t, stats.Int))
                CONTR(who)->orig_stats.Int = (sint8) lua_tonumber(L, -1);
            else if (attrib->offset == offsetof(object_t, stats.Str))
                CONTR(who)->orig_stats.Str = (sint8) lua_tonumber(L, -1);
            else if (attrib->offset == offsetof(object_t, stats.Cha))
                CONTR(who)->orig_stats.Cha = (sint8) lua_tonumber(L, -1);
            else if (attrib->offset == offsetof(object_t, stats.Wis))
                CONTR(who)->orig_stats.Wis = (sint8) lua_tonumber(L, -1);
            else if (attrib->offset == offsetof(object_t, stats.Dex))
                CONTR(who)->orig_stats.Dex = (sint8) lua_tonumber(L, -1);
            else if (attrib->offset == offsetof(object_t, stats.Con))
                CONTR(who)->orig_stats.Con = (sint8) lua_tonumber(L, -1);
            else if (attrib->offset == offsetof(object_t, stats.Pow))
                CONTR(who)->orig_stats.Pow = (sint8) lua_tonumber(L, -1);

            if (attrib->flags & FIELDFLAG_PLAYER_FIX)
                hooks->FIX_PLAYER(who, "LUA - set attribute");
        }
    }

    return 0;
}

/* value is on top of stack */
static int GameObject_setFlag(lua_State *L, lua_object *obj, uint32 flagno, int before)
{
    int value = lua_toboolean(L, -1);
    object_t *op = obj->data.object;

#ifndef USE_OLD_UPDATE
    op = (op->head) ? op->head : op;
#endif

    if (before)
    {
        if (op->type == TYPE_SKILLGROUP ||
            op->type == TYPE_SKILL)
        {
            luaL_error(L, "Flags on TYPE_SKILLGROUP and TYPE_SKILL objects are read only!");
            return 1;
        }
        if (flagno == FLAG_IS_INVISIBLE &&
            op->map)
        {
#ifndef USE_OLD_UPDATE
            object_t *part,
                     *next;

            FOREACH_PART_OF_OBJECT(part, op, next)
            {
                msp_t *msp = MSP_KNOWN(part);

                hooks->msp_rebuild_slices_without(msp, part);
                SET_OR_CLEAR_FLAG(part, flagno, value);
                hooks->msp_rebuild_slices_with(msp, part);
                OBJECT_UPDATE_VIS(part);
            }
#else
            msp_t *msp = MSP_KNOWN(op);

            hooks->msp_rebuild_slices_without(msp, op);
            SET_OR_CLEAR_FLAG(op, flagno, value);
            hooks->msp_rebuild_slices_with(msp, op);
            hooks->update_object(op, UP_OBJ_SLICE);
#endif
            return 1;
        }
    }
    else
    {
#ifndef USE_OLD_UPDATE
        object_t *part,
                 *next;

        /* Set/clear the flag for all parts of a multipart and always update
         * clients and each msp. Not efficient but shouldn't cause problems. */
        FOREACH_PART_OF_OBJECT(part, op, next)
        {
            SET_OR_CLEAR_FLAG(part, flagno, value);
            OBJECT_UPDATE_UPD(part, UPD_FLAGS | UPD_SERVERFLAGS);
        }
#else
        SET_OR_CLEAR_FLAG(op, flagno, value);
        hooks->update_object(op, UP_OBJ_SLICE);
#endif

        /* TODO: if gender changed:
        if()
           CONTR(WHO)->socket.ext_title_flag = 1; * demand update to client */
    }

    return 0;
}

/* pushes flag on top of stack */
static int GameObject_getFlag(lua_State *L, lua_object *obj, uint32 flagno)
{
    object_t *op = obj->data.object;

#ifndef USE_OLD_UPDATE
    op = (op->head) ? op->head : op;
#endif
    lua_pushboolean(L, QUERY_FLAG(op, flagno));
    return 1;
}

/* Compare two objects for equality */
static int GameObject_eq(struct lua_State *L)
{
    lua_object *lhs = lua_touserdata(L, 1);
    lua_object *rhs = lua_touserdata(L, 2);

    /* Should actually never happen. */
    if ((!lhs || lhs->class->type != LUATYPE_OBJECT) ||
        (!rhs || rhs->class->type != LUATYPE_OBJECT))
    {
        LOG(llevBug, "BUG:: %s/GameObject_eq(): Either/both LHS/RHS not GameObject objects!\n",
            __FILE__);

        return luaL_error(L, "GameObject_eq: Either/both LHS/RHS not GameObject objects!");
    }

    /* Test for LHS invalidity. */
    if (!lhs->class->isValid(lhs))
        return luaL_error(L, "GameObject_eq: LHS invalid!");

    /* Test for RHS invalidity. */
    if (!rhs->class->isValid(rhs))
        return luaL_error(L, "GameObject_eq: RHS invalid!");

    /* Compare tags. */
    lua_pushboolean(L, (lhs->tag == rhs->tag));

    return 1;
}

/* toString method for GameObjects */
static int GameObject_toString(lua_State *L)
{
    lua_object *obj = lua_touserdata(L, 1);

    if (obj && obj->class->type == LUATYPE_OBJECT)
        lua_pushfstring(L, "%s[%d] ", STRING_OBJ_NAME(obj->data.object), TAG(obj->data.object));
    else
        luaL_error(L, "Not an object");

    return 1;
}

/* Tests if an object is valid */
static int GameObject_isValid(lua_object *obj)
{
    return obj->data.object->count == obj->tag;
}

lua_class GameObject  =
{
    LUATYPE_OBJECT,
    "GameObject",
    0,
    GameObject_eq,
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
/* Lua    : object:SetPosition(map, x, y, oflags)                            */
/* Info   : Teleports op to x,y of object.map or map (when given) according  */
/*          to flags and ins_flags.                                          */
/*          WARNING: a script developer must have in mind that SetPosition() */
/*          can result in the destruction of the transferred object. The     */
/*          return value is important to check!                              */
/*          oflags are:                                                      */
/*            game.OVERLAY_IGNORE_TERRAIN                                    */
/*            game.OVERLAY_WITHIN_LOS                                        */
/*            game.OVERLAY_FORCE                                             */
/*            game.OVERLAY_FIRST_AVAILABLE                                   */
/*            game.OVERLAY_FIXED                                             */
/*            game.OVERLAY_RANDOM                                            */
/*            game.OVERLAY_SPECIAL                                           */
/*          The default is OVERLAY_RANDOM | OVERLAY_SPECIAL which basically  */
/*          means insert at the first free spot within a three square radius */
/*          of x, y. A common alternative is OVERLAY_FIXED which means       */
/*          insert at x, y if free only. Many more complex variations can be */
/*          specified and will be documented in future.                      */
/*          Examples:                                                        */
/*          obj:SetPosition(x, y) - same as obj:SetPosition(obj.map, x,y)    */
/*          obj:SetPosition(game:ReadyMap("/a_map"), x, y) - multiplayer map */
/*          obj:SetPosition(obj:StartNewInstance("/another_map"), x, y)      */
/*          obj:SetPosition(obj.map:ReadyInheritedMap("/map_2"), x, y)       */
/* Return : 0: all ok, 1: object was destroyed, 2: insertion failed (map or  */
/*          position error, ...)                                             */
/* TODO   : Better document oflags.                                          */
/*****************************************************************************/
static int GameObject_SetPosition(lua_State *L)
{
    lua_object *self;
    sint32      x,
                y,
                oflags = OVERLAY_RANDOM | OVERLAY_SPECIAL;
    map_t      *m2;
    sint16      x2,
                y2;
    msp_t      *msp;

    /* Small hack to allow optional first map parameter */
    if(lua_isuserdata(L, 2))
    {
        lua_object *where;

        get_lua_args(L, "OMii|i", &self, &where, &x, &y, &oflags);
        m2 = where->data.map;
    }
    else
    {
        get_lua_args(L, "Oii|i", &self, &x, &y, &oflags);

        if (!(m2 = WHO->map))
        {
            luaL_error(L, "Short-form of SetPosition() used, but the object didn't have a map");
        }
    }

    /* Find and load  the correct tiled map for extreme values of x and y,
     * aborting the script with an error where the given values are out of map
     * (scripter should fix). */
    x2 = x;
    y2 = y;
    msp = MSP_GET(m2, x2, y2);

    if (!msp)
    {
        return luaL_error(L, "object:SetPosition() trying to place object (%s) at %s %d, %d which is outside of any map!",
            STRING_OBJ_NAME(WHO), STRING_MAP_PATH(m2), x2, y2);
    }

    lua_pushnumber(L, hooks->enter_map(WHO, msp, NULL, oflags, 0));
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_ReadyUniqueMap                                        */
/* Lua    : object:ReadyUniqueMap(path, mode)                                */
/* Info   : Loads the map pointed to by path into memory with unique status. */
/*          This creates a new persistant unique map if needed.              */
/*                                                                           */
/*          path is a required string. It must be an absolute path.          */
/*                                                                           */
/*          mode is an optional number. Use one of:                          */
/*            game.MAP_CHECK - don't load the map if it isn't in memory,     */
/*                             returns nil if the map wasn't in memory.      */
/*            game.MAP_NEW - if the map is already in memory, force an       */
/*                           immediate reset; then (re)load it.              */
/* Return : map pointer to map, or nil                                       */
/* Gotcha : Only valid on players.                                           */
/*****************************************************************************/
static int GameObject_ReadyUniqueMap(lua_State *L)
{
    lua_object *self;
    const char *path;
    int         mode = 0;
    shstr_t      *orig_path_sh,
               *path_sh;
    map_t  *m;

    get_lua_args(L, "Os|i", &self, &path, &mode);

    if (WHO->type != PLAYER ||
        !CONTR(WHO))
    {
        return luaL_error(L, "object:ReadyUniqueMap() can only be called on a player!");
    }

    if (!(orig_path_sh = hooks->create_safe_path_sh(path)))
    {
        return luaL_error(L, "object:ReadyUniqueMap() could not verify the supplied path: >%s<!",
                          STRING_SAFE(path));
    }

    path_sh = hooks->create_unique_path_sh(WHO->name, orig_path_sh);
    m = hooks->map_is_in_memory(path_sh);

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
        m = hooks->ready_map_name(path_sh, orig_path_sh, MAP_STATUS_UNIQUE,
                                  WHO->name);
    }

    SHSTR_FREE(path_sh);
    SHSTR_FREE(orig_path_sh);
    push_object(L, &Map, m);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_StartNewInstance                                      */
/* Lua    : object:StartNewInstance(path, mode)                              */
/* Info   : Reloads or creates an instance for a player based on the map     */
/*          loaded from path.                                                */
/*                                                                           */
/*          path is a required string. It must be an absolute path.          */
/*                                                                           */
/*          mode is an optional number. Use one of:                          */
/*            game.MAP_CHECK - don't load or create the instance if it isn't */
/*                             active. Returns boolean if instance is valid  */
/*                             or not.                                       */
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
/* Return : If mode != game.MAP_CHECK, map pointer to the entrance map of the*/
/*          instance, or nil.                                                */
/*          If mode == game.MAP_CHECK, boolean.                              */
/* Gotcha : Only valid on players.                                           */
/*****************************************************************************/
static int GameObject_StartNewInstance(lua_State *L)
{
    lua_object *self;
    const char *path;
    int         iflag,
                mode = 0;
    shstr_t      *orig_path_sh,
               *path_sh = NULL;
    player_t     *pl;
    map_t  *m = NULL;

    get_lua_args(L, "Os|ii", &self, &path, &iflag, &mode);

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        return luaL_error(L, "object:StartNewInstance() can only be called on a player!");
    }

    if (!(orig_path_sh = hooks->create_safe_path_sh(path)))
    {
        return luaL_error(L, "object:StartNewInstance() could not verify the supplied path: >%s<!",
                          STRING_SAFE(path));
    }

    iflag &= MAP_INSTANCE_FLAG_NO_REENTER; // ensure only valid flags

    /* Check we have a instance we can reenter. */
    if (mode != PLUGIN_MAP_NEW)
    {
        /* the instance data are inside the player struct */
        if (pl->instance_name == orig_path_sh &&
            pl->instance_id == *hooks->global_instance_id &&
            pl->instance_num != MAP_INSTANCE_NUM_INVALID &&
            !(pl->instance_flags & MAP_INSTANCE_FLAG_NO_REENTER))
        {
            path_sh = hooks->create_instance_path_sh(pl, orig_path_sh, iflag);
        }
    }

    if (mode == PLUGIN_MAP_CHECK)
    {
        lua_pushboolean(L, (path_sh) ? 1 : 0);
    }
    else
    {
        /* No path? force a new instance... note that create_instance_path_sh()
         * will setup the player struct data automatically when creating the
         * path data. */
        if (!path_sh)
        {
            pl->instance_num = MAP_INSTANCE_NUM_INVALID; // force new instance
            path_sh = hooks->create_instance_path_sh(pl, orig_path_sh, iflag);
        }

        /* New instance has been declared and initialised - now try to load it!
         * we don't mark the instance invalid when ready_map_name() fails to
         * create a physical map - we let do it the calling script which will
         * know it by checking the return value = NULL. */
        m = hooks->ready_map_name(path_sh, orig_path_sh, MAP_STATUS_INSTANCE,
                                  WHO->name);

        push_object(L, &Map, m);
    }

    SHSTR_FREE(orig_path_sh);
    SHSTR_FREE(path_sh);

    return 1;
}

static sint8 DoInstance(player_t *pl, const char *path)
{
    shstr_t *path_sh;
    sint8  r = 1;

    if (!(path_sh = hooks->create_safe_path_sh(path)) ||
        pl->instance_name != path_sh ||
        pl->instance_id != *hooks->global_instance_id ||
        pl->instance_num == MAP_INSTANCE_NUM_INVALID)
    {
        r = 0;
    }

    SHSTR_FREE(path_sh);

    return r;
}

/*****************************************************************************/
/* Name   : GameObject_CheckInstance                                         */
/* Lua    : object:CheckInstance(entrance_path)                              */
/* Info   : Check player has this instance active                            */
/*          Only checks the player instance data match - NO map loading      */
/* Return : boolean.                                                         */
/* Gotcha : Only valid on players.                                           */
/*****************************************************************************/
static int GameObject_CheckInstance(lua_State *L)
{
    lua_object *self;
    const char *path;
    player_t     *pl;

    get_lua_args(L, "Os", &self, &path);

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        return luaL_error(L, "object:CheckInstance() can only be called on a player!");
    }

    lua_pushboolean(L, DoInstance(pl, path));

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_DeleteInstance                                        */
/* Lua    : object:DeleteInstance(entrance_path)                             */
/* Info   : Delete the instance for a player                                 */
/*          NOTE: this function don't touches the instance directory or      */
/*          deletes a map - it only removes the ID tags from the player      */
/*          IF mapname is the same as the player saved instance              */
/* Return : boolean.                                                         */
/* Gotcha : Only valid on players.                                           */
/*****************************************************************************/
static int GameObject_DeleteInstance(lua_State *L)
{
    lua_object *self;
    const char *path;
    player_t     *pl;
    sint8       r;

    get_lua_args(L, "Os", &self, &path);

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        return luaL_error(L, "object:DeleteInstance() can only be called on a player!");
    }

    if ((r = DoInstance(pl, path)))
    {
        hooks->reset_instance_data(pl);
    }

    lua_pushboolean(L, r);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_CreateArtifact                                        */
/* Lua    : object:CreateArtifact(base_obj, artifact_mask)                   */
/* Info   : Create an artifact = apply a artifact mask to an object          */
/*****************************************************************************/
static int GameObject_CreateArtifact(lua_State *L)
{
    char *name;
    artifact_t *art;
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
/* Info   : same as QUERY_SHORT_NAME()                                       */
/*****************************************************************************/
static int GameObject_GetName(lua_State *L)
{
    lua_object     *self, *owner = NULL;
    object_t         *obj;
    static char    *result;

    get_lua_args(L, "O|O", &self, &owner);

    if(owner)
        obj = owner->data.object;
    else
        obj = self->data.object;

    result = QUERY_SHORT_NAME(WHO, obj);

    lua_pushstring(L, result);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetEquipment                                          */
/* Lua    : object:GetEquipment(slot)                                        */
/* Info   : Get a player's current equipment for a given slot. slot must be  */
/*          one of the Game.EQUIP_xxx constants, e.g. Game.EQUIP_GAUNTLET    */
/*          If the selected slot is empty, this method will return nil.      */
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
/*****************************************************************************/
static int GameObject_Repair(lua_State *L)
{
    int skill = 100;
    lua_object *self;
    object_t *tmp;

    get_lua_args(L, "O|i", &self, &skill);
    hooks->material_repair_item(WHO, skill);
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(WHO, UPD_QUALITY | UPD_NAME);
#else
    hooks->esrv_update_item(UPD_QUALITY, WHO);
#endif

    if((tmp = hooks->is_player_inv(WHO)))
    {
        SET_FLAG(tmp, FLAG_FIX_PLAYER);
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
/*          A mode of 0 means to close explicit a open interface at client   */
/*****************************************************************************/

static int GameObject_Interface(lua_State *L)
{
    lua_object *self;
    char       *txt = NULL;
    int         mode;

    get_lua_args(L, "Oi|s", &self, &mode, &txt);

    if (mode >= GUI_NPC_MODE_END)
    {
        mode = GUI_NPC_MODE_NO;
    }

    hooks->gui_npc(WHO, mode, txt);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetSkill                                              */
/* Lua    : object:GetSkill(type, nr)                                        */
/*          object:GetSkill(name)                                            */
/* Info   : Gets a skill or skillgroup GameObject if object has that skill.  */
/*                                                                           */
/*          The type argument must be either game.TYPE_SKILL for a particular*/
/*          skill or game.TYPE_SKILLGROUP for a skill group. nr must be a    */
/*          legal value accordingly.                                         */
/*                                                                           */
/*          For particular skills, the second form of call may be used where */
/*          the string name is internally translated into a skill number.    */
/*                                                                           */
/*          Various major problems with the arguments generate Lua errors    */
/*          (such as if object is not a player or nr is negative).           */
/*                                                                           */
/*          There are 3 varieties of skill: non-leveling skills do not       */
/*          gain/lose levels or exp (you either have them or you don't, eg,  */
/*          common literacy); direct skills gain/lose levels directly (eg,   */
/*          find traps); indirect skills accumulate exp which means the skill*/
/*          gains/loses levels as the total crosses certain thresholds (eg,  */
/*          punching).                                                       */
/*                                                                           */
/*          Two values are returned: the GameObject (or nil); a game constant*/
/*          number representing whether/how the skill may be leveled.        */
/*                                                                           */
/*          This number is one of:                                           */
/*              game.NONLEVELING - if the previous return was nil, or type   */
/*                  was TYPE_SKILLGROUP, or the skill has already reached    */
/*                  maximum level, or the skill is truly non-leveling;       */
/*              game.INDIRECT - if the skill is leveled indirectly and may   */
/*                  gain exp via a script this level;                        */
/*              game.INDIRECT_NO - if the skill is leveled indirectly and may*/
/*                  not gain exp via a script this level;                    */
/*              game.DIRECT - if the skill is leveled directly.              */
/*****************************************************************************/
static int GameObject_GetSkill(lua_State *L)
{
    lua_object *self;
    int         type,
                nr;
    player_t     *pl;
    object_t     *skill;

    if (lua_isnumber(L, 2))
    {
        get_lua_args(L, "Oii", &self, &type, &nr);
    }
    else
    {
        char *name;

        get_lua_args(L, "Os", &self, &name);
        type = TYPE_SKILL;
        nr = hooks->lookup_skill_by_name(name);
    }

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        luaL_error(L, "object:GetSkill(): Can only be called on a player!");
        return 0;
    }

    switch (type)
    {
        case TYPE_SKILL:
            if (nr < 0 ||
                nr >= NROFSKILLS)
            {
                luaL_error(L, "object:GetSkill(): Skill nr out of range (is: %d, must be: 0-%d)!",
                    nr, NROFSKILLS);
                return 0;
            }

            skill = pl->skill_ptr[nr];
            break;

        case TYPE_SKILLGROUP:
            if (nr < 0 ||
                nr >= NROFSKILLGROUPS)
            {
                luaL_error(L, "object:GetSkill(): Skillgroup nr out of range (is: %d, must be: 0-%d)!",
                    nr, NROFSKILLGROUPS);
                return 0;
            }

            skill = pl->skillgroup_ptr[nr];
            break;

        default:
            luaL_error(L, "object:GetSkill(): Wrong type!");
            return 0;
    }

    /* skill is now the object or NULL. 1st return GameObject or nil. */
    push_object(L, &GameObject, skill);

    /* If !skill OR type == TYPE_SKILLGROUP OR ->level == MAXLEVEL OR
     * ->last_eat == NONLEVELING, it cannot be levelled. 2nd return 0. */
    if (!skill ||
        type == TYPE_SKILLGROUP ||
        skill->level == MAXLEVEL ||
        skill->last_eat == NONLEVELING)
    {
        lua_pushnumber(L, 0);
    }
    /* If ->last_eat == INDIRECT, it is levelled indirectly (accumulates
     * experience which causes level gain/loss when it crosses certain
     * thresholds). 2nd return 1 or -1. */
    /* If ->last_eat == DIRECT, it is levelled directly (does not accumulate
     * experience in the normal way but gaisn/loses levels directly). 2nd
     * return 2. */
    else
    {
        int mode = (skill->last_eat == INDIRECT &&
                    skill->item_level == skill->level) ? -1 : skill->last_eat;

        lua_pushnumber(L, mode);
    }

    return 2;
}

/*****************************************************************************/
/* Name   : GameObject_SetSkill                                              */
/* Lua    : object:SetSkill(type, nr, level, exp)                            */
/*          object:SetSkill(name, level, exp)                                */
/*          object:SetSkill(object, level, exp)                              */
/* Info   : Tries to change a skill's experience and/or level.               */
/*                                                                           */
/*          The type argument must be either game.TYPE_SKILL for a particular*/
/*          skill or game.TYPE_SKILLGROUP for a skill group. nr must be a    */
/*          legal value accordingly. If type is TYPE_SKILLGROUP this         */
/*          translates to the player's best skill in that skill group.       */
/*                                                                           */
/*          For particular skills, the second form of call may be used where */
/*          the string name is internally translated into a skill number.    */
/*                                                                           */
/*          For both skills and skillgroups, the third form of call may be   */
/*          used. Again, all necessary translations and validations are      */
/*          done internally.                                                 */
/*                                                                           */
/*          The level and exp are arguments are what you'd expect. Note that */
/*          these (a) are relative (so X means the skill *gains* X, (b) may  */
/*          be negative (loss is a possible as gain, and (c) limited (a skill*/
/*          may gain to no more than the threshold of the next level or lose */
/*          to no more than 1 exp under the threshold of the current level or*/
/*          gain/lose no more than 1 level).                                 */
/*                                                                           */
/*          There are 3 varieties of skill: non-leveling skills do not       */
/*          gain/lose levels or exp (you either have them or you don't, eg,  */
/*          common literacy); direct skills gain/lose levels directly (eg,   */
/*          find traps); indirect skills accumulate exp which means the skill*/
/*          gains/loses levels as the total crosses certain thresholds (eg,  */
/*          punching).                                                       */
/*                                                                           */
/*          Various major problems with the arguments generate Lua errors    */
/*          (such as if object is not a player or nr is negative).           */
/*                                                                           */
/*          When an indirect skill gains any amount of exp via this method,  */
/*          cannot subsequently gain more exp via a script until it reaches a*/
/*          different level. This means player's cannot exploit scripts to   */
/*          constantly gain experience; scripts augment normal grinding.     */
/*                                                                           */
/*          Four values are returned: a game constant number representing    */
/*          success or failure (see below); the skill object or nil; a number*/
/*          (the level gain/loss; a number (the exp gain/loss).              */
/*                                                                           */
/*          This first return is one of:                                     */
/*              game.SUCCESS - success;                                      */
/*              game.FAILURE_NOSKILL - failure (the player has no such       */
/*                  skill);                                                  */
/*              game.FAILURE_NONLEVELING - failure (the skill is             */
/*                  non-leveling);                                           */
/*              game.FAILURE_MAXLEVEL - failure (the skill is direct or      */
/*                  indirect but has already reached maximum level);         */
/*              game.FAILURE_INDIRECT_NO - failure (the skill is indirect and*/
/*                  has already gained experience via this method this       */
/*                  level).                                                  */
/*                                                                           */
/*          On any failure, level and exp return as 0. On success they are   */
/*          the actual amounts gained/lost (so may be different than the     */
/*          arguments going in).                                             */
/*****************************************************************************/
static int GameObject_SetSkill(lua_State *L)
{
    lua_object *self;
    int         type,
                nr,
                level,
                exp;
    player_t     *pl;
    object_t     *skill;
    int         failure = 0;

    if (lua_isnumber(L, 2))
    {
        get_lua_args(L, "Oiiii", &self, &type, &nr, &level, &exp);
    }
    else if (lua_isstring(L, 2))
    {
        char *name;

        get_lua_args(L, "Osii", &self, &name, &level, &exp);
        type = TYPE_SKILL;
        nr = hooks->lookup_skill_by_name(name);
    }
    else
    {
        lua_object *whatptr;

        get_lua_args(L, "OOii", &self, &whatptr, &level, &exp);
        type = WHAT->type;

        if (type == TYPE_SKILL)
        {
            nr = WHAT->stats.sp;
        }
        else if (type == TYPE_SKILLGROUP)
        {
            nr = WHAT->sub_type1;
        }
    }

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        luaL_error(L, "object:SetSkill(): Can only be called on a player!");
        return 0;
    }

    switch (type)
    {
        case TYPE_SKILL:
            if (nr < 0 ||
                nr >= NROFSKILLS)
            {
                luaL_error(L, "object:SetSkill(): Skill nr out of range (is: %d, must be: 0-%d)!",
                    nr, NROFSKILLS);
                return 0;
            }

            skill = pl->skill_ptr[nr];
            break;

        case TYPE_SKILLGROUP:
            if (nr < 0 ||
                nr >= NROFSKILLGROUPS)
            {
                luaL_error(L, "object:SetSkill(): Skillgroup nr out of range (is: %d, must be: 0-%d)!",
                    nr, NROFSKILLGROUPS);
                return 0;
            }

            skill = pl->highest_skill[nr];
            break;

        default:
            luaL_error(L, "object:SetSkill(): Wrong type!!");
            return 0;
    }

    /* If the player does not even have this skill, return 1, nil, 0, 0. */
    if (!skill)
    {
        failure = 1;
        level = exp = 0;
    }
    else
    {
        /* Scripts can change a max of 1 level, up or down. */
        level = MAX(-1, MIN(level, 1));

        /* If a TYPE_SKILL object has ->last_eat == NONLEVELING, it cannot be
         * levelled; the player either has it or he does not. Return 2, skill,
         * 0, 0. */
        if (skill->last_eat == NONLEVELING)
        {
            failure = 2;
            level = exp = 0;
        }
        else
        {
            /* Already at maximum level? Return 3. skill, 0, 0. */
            if (skill->level == MAXLEVEL)
            {
                failure = 3;
                level = exp = 0;
            }

            /* If ->last_eat == INDIRECT, it is levelled indirectly
             * (accumulates experience which causes level gain/loss when it
             * crosses certain thresholds). */
            if (skill->last_eat == INDIRECT)
            {
                /* If ->item_level == ->level, this means it has already gained
                 * some experience via a script this level so the player will
                 * have to go back to normal grinding for experience until next
                 * level. This prevents scripts being exploited too much to
                 * gain mega-levels. Return 4, skill, 0, 0. */
                if (skill->item_level == skill->level &&
                    (level > 0 || (level == 0 && exp > 0)))
                {
                    failure = 4;
                    level = exp = 0;
                }
                else
                {
                    /* Exp loss is limited to 1 point below the threshold for
                     * the current level, and gain to the threshold for the
                     * next level. */
                    int lo = (hooks->new_levels[skill->level] - 1) - skill->stats.exp,
                        hi = hooks->new_levels[skill->level + 1] - skill->stats.exp;

                    if (level > 0)
                    {
                        exp = hi;
                    }
                    else if (level < 0)
                    {
                        exp = lo;
                    }
                    else
                    {
                        exp = MAX(lo, MIN(exp, hi));
                    }

                    level = skill->level;
                    exp = hooks->add_exp(WHO, exp, nr, 0);
                    level = skill->level - level;

                    if (exp >= 1)
                    {
                        skill->item_level = skill->level;
                    }
                }
            }
            /* If ->last_eat == DIRECT, it is levelled directly (does not
             * accumulate experience in the normal way but gains/loses levels
             * directly). */
            else if (skill->last_eat == DIRECT)
            {
                /* Exp loss is forced to the threshold for the previous level,
                 * and gain to the threshold for the next level. */
                int lo = skill->stats.exp - hooks->new_levels[skill->level - 1],
                    hi = hooks->new_levels[skill->level + 1] - skill->stats.exp;

                if (level > 0 ||
                    exp > 0)
                {
                    exp = hi;
                }
                else if (level < 0 ||
                         exp < 0)
                {
                    exp = lo;
                }

                level = skill->level;
                (void)hooks->add_exp(WHO, exp, nr, 0);
                level = skill->level - level;
                exp = 0;
            }
            else
            {
                LOG(llevDebug, "DEBUG:: Skill (%d) with unhandled last_eat (%d)!\n",
                    nr, skill->last_eat);
                luaL_error(L, "object:SetSkill(): Bad skill!");
                return 0;
            }
        }
    }

    lua_pushnumber(L, failure);
    push_object(L, &GameObject, skill);
    lua_pushnumber(L, level);
    lua_pushnumber(L, exp);
    return 4;
}

/*****************************************************************************/
/* Name   : GameObject_ActivateRune                                          */
/* Lua    : object:ActivateRune(what)                                        */
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
/*****************************************************************************/
static int GameObject_SetGod(lua_State *L)
{
    char       *txt;
    const char *prayname = NULL;
    object_t     *tmp;
    CFParm     *CFR0;
    CFParm     *CFR;
    CFParm      CFP0, CFP1, CFP2;
    int         value;
    lua_object *self;

    get_lua_args(L, "Os", &self, &txt);

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    SHSTR_FREE_AND_ADD_STRING(prayname, "praying");

    CFP1.Value[0] = (void *) (WHO);
    CFP1.Value[1] = (void *) (prayname);

    CFP2.Value[0] = (void *) (WHO);
    CFP0.Value[0] = (char *) (txt);
    CFR0 = (PlugHooks[HOOK_FINDGOD]) (&CFP0);
    tmp = (object_t *) (CFR0->Value[0]);
    free(CFR0);
    CFP2.Value[1] = (void *) (tmp);

    CFR = (PlugHooks[HOOK_CMDRSKILL]) (&CFP1);
    value = *(int *) (CFR->Value[0]);
    if (value)
        (PlugHooks[HOOK_BECOMEFOLLOWER]) (&CFP2);
    free(CFR);

    SHSTR_FREE(prayname);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_InsertInside                                          */
/* Lua    : object:InsertInside(where)                                       */
/* Info   : Inserts object into where.                                       */
/*****************************************************************************/
static int GameObject_InsertInside(lua_State *L)
{
    lua_object *self,
               *whatptr;
    object_t     *obenv;

    get_lua_args(L, "OO", &self, &whatptr);
    obenv = WHO->env;

    if (!QUERY_FLAG(WHO, FLAG_REMOVED))
    {
        hooks->remove_ob(WHO);
    }

    WHO = hooks->insert_ob_in_ob(WHO, WHAT);

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
/* Lua    : object:PickUp(what, where, nrof)                                 */
/* Info   : Only works for players and monster objects. Other types generate */
/*          an error.                                                        */
/*          Causes object to pick up what if possible (according to standard */
/*          rules).                                                          */
/*          where is optional but if specified must be a container and causes*/
/*          a pick up to container. Otherwise a pick up to inventory is      */
/*          attempted.                                                       */
/*          nrof is optional and specifies the nrof from a stack (the default*/
/*          is the entire stack).                                            */
/*****************************************************************************/
static int GameObject_PickUp(lua_State *L)
{
    lua_object *self,
               *whatptr,
               *where = NULL;
    int         nrof = 0;
    object_t     *pickedup;

    get_lua_args(L, "OO|Oi", &self, &whatptr, &where, &nrof);

    if ((WHO->type != PLAYER ||
         !CONTR(WHO)) &&
        WHO->type != MONSTER)
    {
        return luaL_error(L, "object:PickUp() can only be called on a player or monster!");
    }

    if (where &&
        where->data.object->type != CONTAINER)
    {
        return luaL_error(L, "object:PickUp(): Arg #2 must be a container GameObject or nil!");
    }

    pickedup = hooks->pick_up(WHO, WHAT, (where) ? where->data.object : NULL, nrof);
    push_object(L, &GameObject, pickedup);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Drop                                                  */
/* Lua    : object:Drop(what, nrof)                                          */
/* Info   : Only works for players and monster objects. Other types generate */
/*          an error.                                                        */
/*          Causes object to drop what if possible (according to standard    */
/*          rules such as players cannot drop locked items and mobs cannot   */
/*          drop no drops).                                                  */
/*          nrof is optional and specifies the nrof from a stack (the default*/
/*          is the entire stack).                                            */
/*****************************************************************************/
static int GameObject_Drop(lua_State *L)
{
    lua_object *self,
               *whatptr;
    int         nrof = 0;
    object_t     *dropped;

    get_lua_args(L, "OO|i", &self, &whatptr, &nrof);

    if ((WHO->type != PLAYER ||
         !CONTR(WHO)) &&
        WHO->type != MONSTER)
    {
        return luaL_error(L, "object:Drop() can only be called on a player or monster!");
    }

    dropped = hooks->drop_to_floor(WHO, WHAT, nrof);
    push_object(L, &GameObject, dropped);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Give                                                  */
/* Lua    : object:Give(what, whom, nrof)                                    */
/* Info   : Only works for monster objects. Other types generate an error.   */
/*          Causes object to give what to whom if possible (or technically   */
/*          whom to pick up what which happens to be in object's inventory). */
/*          nrof is optional and specifies the nrof from a stack (the default*/
/*          is the entire stack).                                            */
/*          This method does not check the distance between the object and   */
/*          whom. So things can be given from A to B no matter how far they  */
/*          are from one another, whether they are separated by a wall or a  */
/*          lake of fire, or even on distant maps, etc.                      */
/*          Giving has a very communist definition so ALL object's property  */
/*          is open to ANYONE (though this method does limit this to object's*/
/*          direct inventory -- ie, not things in containers though those    */
/*          containers may themselves be given). Fortunately this sŧate only */
/*          lasts for the split second this method is actually running.      */
/* Return : A boolean to indicate whether the attempt succeeded or not       */
/*          (failure can only mean that whom was not able to pick up what for*/
/*          some reason -- ie, too heavy, etc).                              */
/*****************************************************************************/
static int GameObject_Give(lua_State *L)
{
    lua_object *self,
               *whatptr,
               *whom;
    int         nrof = 0;
    object_t     *given;

    get_lua_args(L, "OOO|i", &self, &whatptr, &whom, &nrof);

    if (WHO->type != MONSTER)
    {
        return luaL_error(L, "object:Give() can only be called on a monster!");
    }

    if (WHAT->env != WHO)
    {
        return luaL_error(L, "object:Give(): Arg #1 must be in the possession of self!");
    }

    if ((whom->data.object->type != PLAYER ||
         !CONTR(whom->data.object)) &&
        whom->data.object->type != MONSTER)
    {
        return luaL_error(L, "object:Give(): Arg #2 must be player or monster GameObject!");
    }

    SET_FLAG(WHO, FLAG_IS_GIVING);
    given = hooks->pick_up(whom->data.object, WHAT, NULL, nrof);
    CLEAR_FLAG(WHO, FLAG_IS_GIVING);
    lua_pushboolean(L, (given) ? 1 : 0);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Deposit                                               */
/* Lua    : object:Deposit(what, string)                                     */
/* Info   : Only works for players and monster objects. Other types generate */
/*          an error.                                                        */
/*          Deposits an amount of money (as described by string) to what (a  */
/*          bank force) from object's inventory.                             */
/* Return : nil if there was a syntax error in string; false if the object   */
/*          did not have sufficient money; true if the deposit succeeded (and*/
/*          what->value is updated).                                         */
/*****************************************************************************/
static int GameObject_Deposit(lua_State *L)
{
    lua_object  *self,
                *whatptr;
    char        *text;
    moneyblock_t money;

    get_lua_args(L, "OOs", &self, &whatptr, &text);

    if ((WHO->type != PLAYER ||
         !CONTR(WHO)) &&
        WHO->type != MONSTER)
    {
        return luaL_error(L, "object:Deposit() can only be called on a player or monster!");
    }

//    if (WHAT->type != MISC_OBJECT || // TODO: BASE_INFO or FORCE?
//        WHAT->name != "BANK GENERAL")
//    {
//        return luaL_error(L, "object:Deposit(): Arg #2 must be a bank account GameObject!");
//    }

    (void)hooks->get_money_from_string(text, &money);

    if (!money.mode)
    {
        lua_pushnil(L);
    }
    else
    {
        sint64 total = (money.mode == MONEY_MODE_ALL) ?
            hooks->query_money(WHO, NULL) : // TODO: also called in shop_pay_amount()
            money.mithril * hooks->coins_arch[0]->clone.value +
            money.gold * hooks->coins_arch[1]->clone.value +
            money.silver * hooks->coins_arch[2]->clone.value +
            money.copper * hooks->coins_arch[3]->clone.value;

        if (total <= 0 ||
            !hooks->shop_pay_amount(total, WHO))
        {
            lua_pushboolean(L, 0);
        }
        else
        {
            WHAT->value += total;
            lua_pushboolean(L, 1);
        }
    }

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Withdraw                                              */
/* Lua    : object:Withdraw(what, string)                                    */
/* Info   : Only works for players and monster objects. Other types generate */
/*          an error.                                                        */
/*          Withdraws an amount of money (as described by string) from what  */
/*          (a bank force) to object's inventory.                            */
/* Return : nil if there was a syntax error in string; false if there was not*/
/*          sufficient money in what; true if the withdrawal succeeded (and  */
/*          what->value is updated).                                         */
/*****************************************************************************/
static int GameObject_Withdraw(lua_State *L)
{
    lua_object  *self,
                *whatptr;
    char        *text;
    moneyblock_t money;

    get_lua_args(L, "OOs", &self, &whatptr, &text);

    if ((WHO->type != PLAYER ||
         !CONTR(WHO)) &&
        WHO->type != MONSTER)
    {
        return luaL_error(L, "object:Withdraw() can only be called on a player or monster!");
    }

//    if (WHAT->type != MISC_OBJECT || // TODO: BASE_INFO or FORCE?
//        WHAT->name != "BANK GENERAL")
//    {
//        return luaL_error(L, "object:Withdraw(): Arg #2 must be a bank account GameObject!");
//    }

    (void)hooks->get_money_from_string(text, &money);

    if (!money.mode)
    {
        lua_pushnil(L);
    }
    else
    {
        sint64 total = (money.mode == MONEY_MODE_ALL) ?
            WHAT->value :
            money.mithril * hooks->coins_arch[0]->clone.value +
            money.gold * hooks->coins_arch[1]->clone.value +
            money.silver * hooks->coins_arch[2]->clone.value +
            money.copper * hooks->coins_arch[3]->clone.value;

        if (total <= 0 ||
            total > WHAT->value)
        {
            lua_pushboolean(L, 0);
        }
        else
        {
            object_t *loot;

            if (money.mode == MONEY_MODE_ALL)
            {
                (void)hooks->enumerate_coins(WHAT->value, &money);
            }

            loot = hooks->create_financial_loot(&money, WHO, MODE_NO_INVENTORY);
            SHSTR_FREE_AND_ADD_STRING(loot->name, "your withdrawal");
            (void)hooks->pick_up(WHO, loot, NULL, 1);
            WHAT->value -= total;
            lua_pushboolean(L, 1);
        }
    }

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Communicate                                           */
/* Lua    : object:Communicate(message)                                      */
/* Info   : object says message to everybody on its map                      */
/*          but instead of CFSay it is parsed for other npc or magic mouth   */
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

    /* No point mucking about with an empty message. */
    if (*message)
    {
        hooks->ndi_map(NDI_UNIQUE | NDI_WHITE, MSP_KNOWN(WHO), range, NULL, NULL, "%s",
            message);
    }

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
/*****************************************************************************/
static int GameObject_SayTo(lua_State *L)
{
    lua_object *self;
    object_t     *target;
    lua_object *obptr2;
    int mode = 0;
    int range = MAP_INFO_NORMAL;
    char *message;
    static char buf[HUGE_BUF];

    get_lua_args(L, "OOs|ii", &self, &obptr2, &message, &mode, &range);

    /* No point mucking about with an empty message. */
    if (*message)
    {
        target = obptr2->data.object;

        if (mode == 1)
        {
            hooks->ndi(NDI_UNIQUE | NDI_NAVY, 0, target, "%s",
                message);
        }
        else /* thats default */
        {
            if (mode == 2)
            {
                snprintf(buf, sizeof(buf), "%s talks to %s.",
                    QUERY_SHORT_NAME(WHO, NULL), QUERY_SHORT_NAME(target, NULL));
                hooks->ndi_map(NDI_UNIQUE | NDI_WHITE, MSP_KNOWN(WHO), range, WHO, target, "%s", buf);
            }

            snprintf(buf, sizeof(buf), "%s says: %s",
                QUERY_SHORT_NAME(WHO, target), message);
            hooks->ndi(NDI_UNIQUE | NDI_NAVY, 0, target, "%s", buf);
        }
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_ChannelMsg                                            */
/* Lua    : object:ChannelMsg(channel, message, mode)                        */
/* Info   : object sends message on the channel according to mode.           */
/*          object can be any  object, not just a player and does not need to*/
/*          be subscribed to channel. The message is attributed to the       */
/*          object's name. This is underlined to show it came from a script. */
/*          See also mode below.                                             */
/*          channel must be a full channel name, not an abbreviation.        */
/*          message is an arbitray string of less than 250 characters or so. */
/*          mode should be one of:                                           */
/*            game.CHANNEL_MODE_NORMAL: message is sent as a normal message. */
/*              This is the default.                                         */
/*            game.CHANNEL_MODE_EMOTE: message is sent as an emote.          */
/*            game.CHANNEL_MODE_SYSTEM: message is sent as a system message. */
/*            It is attributed to |Daimonin| rather than the object name.    */
/* Return: True on success. False on failure.                                */
/* TODO  : Where object is a (channel) muted player nothing should be posted.*/
/*         The main problem here is that we nedd the checks to be done       */
/*         silently -- ie, on mute just fail, don't notify the player -- but */
/*         the code has not been written in such a flexible way.             */
/*****************************************************************************/
static int GameObject_ChannelMsg(lua_State *L)
{
    lua_object *self;
    char       *channel,
               *message;
    int         mode = 0;

    get_lua_args(L, "Oss|i", &self, &channel, &message, &mode);

#ifdef USE_CHANNELS
    /* No point mucking about with an empty message. */
    if (*message)
    {
        char buf[SMALL_BUF];

        /* System messages don't use object's name, but are otherwise 'normal'
         * messages. */
        if (mode == 2)
        {
            sprintf(buf, "%c|Daimonin|%c", ECC_UNDERLINE, ECC_UNDERLINE);
            mode = 0;
        }
        /* Actually this should never happen -- it's very important that
         * objects always have names (this is handled elsewhere), but JIC... */
        else if (!WHO->name)
        {
            sprintf(buf, "%c???%c", ECC_UNDERLINE, ECC_UNDERLINE);
        }
        else
        {
            sprintf(buf, "%c%s%c", ECC_UNDERLINE, WHO->name, ECC_UNDERLINE);
        }

        if (hooks->lua_channel_message(channel, buf, message, mode) == 0)
        {
            lua_pushboolean(L, 1);
            return 1;
        }
    }

    lua_pushboolean(L, 0);
    return 1;
#else
    return 0;
#endif
}

/*****************************************************************************/
/* Name   : GameObject_Write                                                 */
/* Lua    : object:Write(message, color)                                     */
/* Info   : Writes a message to a specific player.                           */
/*          color should be one of the game.COLOR_xxx constants.             */
/*          default color is game.COLOR_UNIQUE | game.COLOR_NAVY             */
/*****************************************************************************/
static int GameObject_Write(lua_State *L)
{
    char       *message;
    int         color = NDI_NAVY;
    lua_object *self;

    get_lua_args(L, "Os|i", &self, &message, &color);

    /* No point mucking about with an empty message. */
    if (*message)
    {
        hooks->ndi(NDI_UNIQUE | color, 0, WHO, "%s", message);
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetGender                                             */
/* Lua    : object:GetGender()                                               */
/* Info   : Gets the gender of object. Returns five values: an integer and   */
/*          and four strings (the noun, objective pronoun, subjective pronoun*/
/*          and possessive pronoun for that gender). So the possibilities are*/
/*              game.NEUTER, "neuter", "it", "it", "its"                     */
/*              game.MALE, "male", "he", "him", "his"                        */
/*              game.FEMALE, "female", "she", "her", "her"                   */
/*              game.HERMAPHRODITE, "hermaphrodite", "they", "them, "their"  */
/*****************************************************************************/
static int GameObject_GetGender(lua_State *L)
{
    lua_object  *self;
    int          gender;
    static char *noun,
                *obje,
                *subj,
                *poss;

    get_lua_args(L, "O", &self);

    if (!QUERY_FLAG(WHO, FLAG_IS_FEMALE))
    {
        if (!QUERY_FLAG(WHO, FLAG_IS_MALE))
        {
            gender = 0;
            noun = "neuter";
            obje = "it";
            subj = "it";
            poss = "its";
        }
        else
        {
            gender = 1;
            noun = "male";
            obje = "he";
            subj = "him";
            poss = "his";
        }
    }
    else
    {
         if (!QUERY_FLAG(WHO, FLAG_IS_MALE))
        {
            gender = 2;
            noun = "female";
            obje = "she";
            subj = "her";
            poss = "her";
        }
        else
        {
            gender = 3;
            noun = "hermaphrodite";
            obje = "they";
            subj = "them";
            poss = "their";
        }
    }

    lua_pushnumber(L, gender);
    lua_pushstring(L, noun);
    lua_pushstring(L, obje);
    lua_pushstring(L, subj);
    lua_pushstring(L, poss);

    return 5;
}

/*****************************************************************************/
/* Name   : GameObject_SetGender                                             */
/* Lua    : object:SetGender(gender)                                         */
/* Info   : Changes the gender of object. gender_string should be one of     */
/*          game.NEUTER, game.MALE, game.GENDER_FEMALE or                    */
/*          game.HERMAPHRODITE                                               */
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
/*****************************************************************************/
static int GameObject_SetRank(lua_State *L)
{
    object_t     *walk,
               *next;
    char       *rank;
    lua_object *self;

    get_lua_args(L, "Os", &self, &rank);

    if (WHO->type != PLAYER)
        return 0;

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    FOREACH_OBJECT_IN_OBJECT(walk, WHO, next)
    {
        if (walk->name &&
            walk->name == hooks->shstr_cons->RANK_FORCE &&
            walk->arch == hooks->archetype_global->_rank_force)
        {
            if (strcmp(rank, "Mr") == 0) /* Mr = keyword to clear title and not add it as rank */
            {
                SHSTR_FREE(walk->title);
            }
            else
            {
                SHSTR_FREE_AND_ADD_STRING(walk->title, rank);
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
/*****************************************************************************/
static int GameObject_SetAlignment(lua_State *L)
{
    object_t     *walk,
               *next;
    char       *align;
    lua_object *self;

    get_lua_args(L, "Os", &self, &align);

    if (WHO->type != PLAYER)
        return 0;

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    FOREACH_OBJECT_IN_OBJECT(walk, WHO, next)
    {
        if (walk->name &&
            walk->name == hooks->shstr_cons->ALIGNMENT_FORCE &&
            walk->arch == hooks->archetype_global->_alignment_force)
        {
            /* we find the alignment of the player, now change it to new one */
            SHSTR_FREE_AND_ADD_STRING(walk->title, align);

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
/*****************************************************************************/
static int GameObject_GetAlignmentForce(lua_State *L)
{
    object_t     *walk,
               *next;
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER)
        return 0;

    FOREACH_OBJECT_IN_OBJECT(walk, WHO, next)
    {
        if (walk->name &&
            walk->name == hooks->shstr_cons->ALIGNMENT_FORCE &&
            walk->arch == hooks->archetype_global->_alignment_force)
        {
            return push_object(L, &GameObject, walk);
        }
    }
    LOG(llevDebug, "Lua Warning -> GetAlignmentForce: Object %s has no aligment_force!\n", STRING_OBJ_NAME(WHO));

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetGuild                                              */
/* Lua    : object:GetGuild(name)                                            */
/* Info   :                                                                  */
/*****************************************************************************/
static int GameObject_GetGuild(lua_State *L)
{
    char *name;
    lua_object *self;
    object_t *force;

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
/* Info   : Only works for player objects. Other types generate an error.    */
/*          Causes object to join named guild. The default or guild          */
/*          skillgroups are set as indicated.                                */
/*****************************************************************************/
static int GameObject_JoinGuild(lua_State *L)
{
    lua_object *self;
    char       *name;
    int         s[3] = {-1, -1, -1},
                v[3] = {0, 0, 0};
    player_t     *pl;
    object_t     *force;

    get_lua_args(L, "Os|iiiiii", &self, &name, &s[0], &v[0], &s[1], &v[1],
                 &s[2], &v[2]);

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        return luaL_error(L, "object:JoinGuild() can only be called on a player!");
    }

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    force = hooks->guild_join(pl, name, s[0], v[0], s[1], v[1], s[2], v[2]);

    return push_object(L, &GameObject, force);
}

/*****************************************************************************/
/* Name   : GameObject_LeaveGuild                                            */
/* Lua    : object:LeaveGuild()                                              */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          Causes object to leave current guild.                            */
/*****************************************************************************/
static int GameObject_LeaveGuild(lua_State *L)
{
    lua_object *self;
    player_t     *pl;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        return luaL_error(L, "object:LeaveGuild() can only be called on a player!");
    }

    SET_FLAG(WHO, FLAG_FIX_PLAYER);
    hooks->guild_leave(pl);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Fix                                                   */
/* Lua    : object:Fix()                                                     */
/* Info   : Recalculates a player's or monster's stats depending on          */
/*          equipment, forces, skills etc.                                   */
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
/* Lua    : object:Kill(killer)                                              */
/* Info   : Kills the object, awarding exp, creating a corpse, giving        */
/*          deathsick, etc. as necessary.                                    */
/*          Returns true if the object was good enough to die, false if it   */
/*          had some method of saving its life, or nil if there was a        */
/*          problem.                                                         */
/*****************************************************************************/
static int GameObject_Kill(lua_State *L)
{
    lua_object *self,
               *whatptr = NULL;
    char       *headline = NULL,
               *detail = NULL;

    get_lua_args(L, "O|Oss", &self, &whatptr, &headline, &detail);

    /* If WHO is already removed, don't try to kill it, return nil. */
    if (QUERY_FLAG(WHO, FLAG_REMOVED))
    {
        return 0;
    }

    if (!hooks->kill_object(WHO, (whatptr) ? WHAT : NULL, headline, detail))
    {
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, 0);
    }

    return 1;
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
/*****************************************************************************/
static int GameObject_FindSkill(lua_State *L)
{
    int         skill;
    object_t     *myob;
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
/*****************************************************************************/
static int GameObject_AcquireSkill(lua_State *L)
{
    int         skill, mode;
    lua_object *self;
    CFParm      CFP;

    get_lua_args(L, "Oii", &self, &skill, &mode);

	if (mode == 0)
	{
		hooks->learn_skill(WHO, skill);
	}
	else
	{
		hooks->unlearn_skill(WHO, skill);
	}

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_FindMarkedObject                                      */
/* Lua    : object:FindMarkedObject()                                        */
/* Info   : Returns the marked object in object's inventory, or None if no   */
/*          object is marked.                                                */
/*****************************************************************************/
static int GameObject_FindMarkedObject(lua_State *L)
{
    object_t     *value;
    CFParm     *CFR, CFP;
    lua_object *self;

    get_lua_args(L, "O", &self);

    CFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_FINDMARKEDOBJECT]) (&CFP);

    value = (object_t *) (CFR->Value[0]);
    /*free(CFR); findmarkedobject use static parameters */
    return push_object(L, &GameObject, value);
}

/*****************************************************************************/
/* Name   : GameObject_CheckInvisibleInside                                  */
/* Lua    : object:CheckInvisibleInside(id)                                  */
/*****************************************************************************/

static int GameObject_CheckInvisibleInside(lua_State *L)
{
    char       *id;
    object_t     *tmp2,
               *next;
    lua_object *self;

    get_lua_args(L, "Os", &self, &id);

    FOREACH_OBJECT_IN_OBJECT(tmp2, WHO, next)
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
/*****************************************************************************/
static int GameObject_CreatePlayerForce(lua_State *L)
{
    char       *txt;
    object_t     *myob;
    int         time        = 0;
    lua_object *whatptr;

    get_lua_args(L, "Os|i", &whatptr, &txt, &time);

    myob = hooks->arch_to_object(hooks->find_archetype("player_force"));

    if (!myob)
        return luaL_error(L, "object:CreatePlayerForce(): Can't find archetype 'player_force'");

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
    SHSTR_FREE_AND_ADD_STRING(myob->name, txt);
    myob = hooks->insert_ob_in_ob(myob, WHAT);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_AddQuest                                              */
/* Lua    : object:AddQuest(name, mode, start, stop, level, skill, msg, reps)*/
/* Info   : Only works for player objects. Other types generate an error.    */
/*          Add a quest_trigger to a quest_container = give player a quest.  */
/*****************************************************************************/
static int GameObject_AddQuest(lua_State *L)
{
    char       *name, *msg;
    int         mode, lev, skill_lev, step_start, step_end, repeats = 0;
    object_t     *myob;
    lua_object *self;

    get_lua_args(L, "Osiiiii|si", &self, &name, &mode, &step_start, &step_end, &lev, &skill_lev, &msg, &repeats);

    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "object:AddQuest() can only be called on a player!");

    /* Player already has quest? Abort but log this fact. */
    if (hooks->quest_find_name(WHO, name))
    {
        LOG(llevInfo, "LUA INFO:: object:AddQuest(): %s[%d] already has quest '%s'!\n",
            STRING_OBJ_NAME(WHO), TAG(WHO), name);

        return 0;
    }

    /* Player has too many quests? Message player and abort. */
    if(hooks->quest_count_pending(WHO) >= QUESTS_PENDING_MAX)
    {
        hooks->ndi(NDI_UNIQUE | NDI_NAVY, 0, WHO, "You can't have more than %d open quests.\nRemove one first!",
                                    QUESTS_PENDING_MAX);

        return 0;
    }

    myob = hooks->arch_to_object(hooks->archetype_global->_quest_trigger);

    if (!myob)
        return luaL_error(L, "object:AddQuest(): Can't find archetype 'quest_trigger'");

    /* store name & arch name of the quest obj. so we can id it later */
    SHSTR_FREE_AND_ADD_STRING(myob->name, name);

    if(msg)
    {
        SHSTR_FREE_AND_ADD_STRING(myob->msg, msg);
    }

    myob->sub_type1 = (uint8)mode;
    myob->last_heal = (sint16)step_start;
    myob->state = step_end;
    myob->item_skill = skill_lev;
    myob->item_level = lev;
    myob->last_eat = myob->stats.food = 0; //MAX(0, repeats);

    hooks->add_quest_trigger(WHO, myob);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_GetQuest                                              */
/* Lua    : object:GetQuest(name)                                            */
/* Info   : Checks if object (which must be a player) knows about the named  */
/*          quest (means it has been or is being done), and its active       */
/*          status.  Note that active status is not the same as status.      */
/*                                                                           */
/*          Status includes whether or not a player is eligible for the quest*/
/*          (QSTAT_NO or QSTAT_DISALLOW) and therefore cannot be             */
/*          QSTAT_UNKNOWN. Status can only be ascertained with a script      */
/*          (because the eligibility criteria rely on certain checks and     */
/*          lists defined only within the script).                           */
/*                                                                           */
/*          Active status therefore cannot check eligiblity. Hence           */
/*          QSTAT_UNKNOWN says nothing of eligibility, it just means the     */
/*          player is not doing/has not done this quest (and that includes   */
/*          because the quest -- or player -- does not exist).               */
/*                                                                           */
/*          Additionally, where a quest has been done but is still           */
/*          repeatable, QSTAT_NO is returned. Again, this does not imply     */
/*          actual eligibility, it just differentiates the return from       */
/*          QSTAT_DONE.                                                      */
/*                                                                           */
/*          So two values are returned: a GameObject, which is the quest     */
/*          itself, if it is known, or nil otherwise; a number to indicate   */
/*          its active status.                                               */
/*                                                                           */
/*          This number is one of:                                           */
/*              game.QSTAT_UNKNOWN - no quest of that name is currently or   */
/*                  has ever been undertaken by the player.                  */
/*              game.QSTAT_NO - the named quest has previously been done by  */
/*                  the player but is repeatable.                            */
/*              game.QSTAT_DONE - the named quest has previously been done by*/
/*                  the player and is not repeatable.                        */
/*              game.QSTAT_ACTIVE -- the named quest is one the player has   */
/*                  accepted and is still actively working on.               */
/*              game.QSTAT_SOLVED -- the named quest is one the player has   */
/*                  accepted and has completed all tasks on; he has only to  */
/*                  report back to the quest giver to pick up a reward.      */
/*****************************************************************************/
static int GameObject_GetQuest(lua_State *L)
{
    lua_object *self;
    char       *name;
    player_t     *pl;
    object_t     *quest;
    int         qstat;

    get_lua_args(L, "Os", &self, &name);

    if (WHO->type != PLAYER ||
        !(pl = CONTR(WHO)))
    {
        luaL_error(L, "object:GetQuest(): Can only be called on a player!");
        return 0;
    }

    quest = hooks->quest_find_name(WHO, name);
    qstat = hooks->quest_get_active_status(pl, quest);
    push_object(L, &GameObject, quest);
    lua_pushnumber(L, qstat);
    return 2;
}

/*****************************************************************************/
/* Name   : GameObject_CheckQuestLevel                                       */
/* Lua    : object:CheckQuestLevel(level, skill_group)                       */
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
    player_t *pl = NULL;

    get_lua_args(L, "Oii", &self, &level, &item_skill_group);

    /* some sanity checks */
    if(WHO->type != PLAYER || !(pl=CONTR(WHO)))
        luaL_error(L, "Not a player object");
    if(item_skill_group < 0 || item_skill_group > NROFSKILLGROUPS)
        luaL_error(L, "Invalid skill_group parameter. Use one of the game.ITEM_SKILL_XXX constants");

    /* Note: the ITEM_SKILL_XXX lua constants corresponds to SKILLGROUP_XXX + 1 */
    /* player is high enough for this quest? */
    if (item_skill_group)
        tmp_lev = pl->skillgroup_ptr[item_skill_group-1]->level; /* use player struct shortcut ptrs */
    else
        tmp_lev = WHO->level;

    if (level > tmp_lev) /* too low */
        ret = 0;

    lua_pushboolean(L, ret);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AddQuestTarget                                        */
/* Lua    : object:AddQuestTarget(chance, nrof, arch, name, race, title, lev)*/
/* Info   : define a kill mob. Careful: if all are nil then ALL mobs are part*/
/*          of this quest. If only arch set, all mobs using that base arch   */
/* TODO   : Improve docs.                                                    */
/*****************************************************************************/
static int GameObject_AddQuestTarget(lua_State *L)
{
    lua_object *self;
    int         nrof,
                chance;
    char       *arch = NULL,
               *name = NULL,
               *race = NULL,
               *title = NULL;
    int         level = 0;
    object_t     *myob;

    get_lua_args(L, "Oii|ssssi", &self, &chance, &nrof, &arch, &name, &race, &title, &level);

    myob = hooks->arch_to_object(hooks->archetype_global->_quest_info);

    if (!myob)
        return luaL_error(L, "object:AddQuestTarget(): Can't find archetype 'quest_info'");

    myob->last_grace = chance;
    myob->last_sp = nrof; /* can be overruled by ->inv objects */

    if(arch && *arch!='\0')
    {
        SHSTR_FREE_AND_ADD_STRING(myob->race, arch);
    }
    else
    {
        SHSTR_FREE(myob->race);
    }

    if(name && *name!='\0')
    {
        SHSTR_FREE_AND_ADD_STRING(myob->name, name);
    }

    if(race && *race!='\0')
    {
        SHSTR_FREE_AND_ADD_STRING(myob->slaying, race);
    }
    else
    {
        SHSTR_FREE(myob->slaying);
    }

    if(title && *&title!='\0')
    {
        SHSTR_FREE_AND_ADD_STRING(myob->title, title);
    }
    else
    {
        SHSTR_FREE(myob->title);
    }

    /* Only mobs >= level count as targets. This is a useful final check to
     * prevent players from going to a source of low level mobs to easily
     * complete a quest. We use weight_limit as level is already used for quest
     * objects. */
    myob->weight_limit = level;

    /* finally add it to the quest_trigger object_t */
    hooks->insert_ob_in_ob(myob, self->data.object);

    /* we need that when we want do more with this object - for example adding kill items */
    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_AddQuestItem                                          */
/* Lua    : object:AddQuestItem(nrof, arch, face, name, title)               */
/* Info   : Add a quest item to a quest or base object                       */
/*          (see GameObject_AddQuestTarget)                                  */
/*****************************************************************************/
static int GameObject_AddQuestItem(lua_State *L)
{
    int         id, nrof;
    char       *i_arch, *i_face, *i_name = NULL, *i_title = NULL;
    object_t     *myob;
    lua_object *self;

    get_lua_args(L, "Oiss|s|s", &self, &nrof, &i_arch, &i_face, &i_name, &i_title);

    myob = hooks->arch_to_object(hooks->find_archetype(i_arch));

    if (!myob)
    {
        char buf[MEDIUM_BUF];

        sprintf(buf, "object:AddQuestItem(): Can't find archetype '%s'", i_arch);

        return luaL_error(L, buf);
    }

    if(i_face && *i_face !='\0') /* "" will skip the face setting */
    {
        id = hooks->FindFace(i_face, -1);
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
            SHSTR_FREE_AND_ADD_STRING(myob->name, i_name);
        }
//        else
//        {
//            SHSTR_FREE(myob->name);
//        }
    }

    if(i_title)
    {
        if(*i_title!='\0')
        {
            SHSTR_FREE_AND_ADD_STRING(myob->title, i_title);
        }
        else
        {
            SHSTR_FREE(myob->title);
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
/*****************************************************************************/
static int GameObject_NrofQuestItem(lua_State *L)
{
    int           nrof=0;
    const object_t *myob, *pl;
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
static int remove_quest_items(const object_t *inv, const object_t *myob, int nrof)
{
    object_t *walk, *walk_below;

    for (walk = (object_t *)inv; walk; walk = walk_below)
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
/* TODO   : I am very unhappy with this function. It does not work well and  */
/*          in fact I am not clear why it is not done automatically when a   */
/*          quest is finished or skipped anyway; AFAICS there is never a need*/
/*          to call it explicitly anyway. Will be addressed in SEQSy.        */
/*****************************************************************************/
static int GameObject_RemoveQuestItem(lua_State *L)
{
    int         nrof = -1;
    object_t     *myob, *pl;
    lua_object *self;

    get_lua_args(L, "O|i", &self, &nrof);

    pl = hooks->is_player_inv(self->data.object);
    myob = self->data.object->inv;

    /* some sanity checks */
    if(myob && pl && pl->type == PLAYER)
    {
        if(nrof == -1) /* if we don't have an explicit number, use number from kill target */
            nrof = myob->nrof;

        hooks->ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "%s %s removed from your inventory.",
            QUERY_SHORT_NAME(myob, NULL), (nrof > 1) ? "are" : "is");
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
/*****************************************************************************/
static int GameObject_SetQuestStatus(lua_State *L)
{
    int         q_status, q_type = -1;
    object_t     *myob;
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
/* Info   : Check the one drop and single quest item container for an item   */
/*****************************************************************************/
static int GameObject_CheckOneDropQuest(lua_State *L)
{
    const char *name_hash, *title_hash;
    archetype_t *arch;
    char       *arch_name;
    char       *name, *title = NULL;
    object_t     *walk,
               *next;
    lua_object *self;

    get_lua_args(L, "Os|s", &self, &arch_name, &name, &title);

    if (!(arch = hooks->find_archetype(arch_name)))
    {
        char buf[MEDIUM_BUF];

        sprintf(buf, "object:CheckOneDropQuest(): Can't find archetype '%s'", arch_name);

        return luaL_error(L, buf);
    }

    name_hash = hooks->shstr_find(name);
    if(title)
        title_hash = hooks->shstr_find(title);
    else
        title_hash = arch->clone.title;

    if (WHO->type == PLAYER && CONTR(WHO)->quest_one_drop)
    {
        FOREACH_OBJECT_IN_OBJECT(walk, CONTR(WHO)->quest_one_drop, next)
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
/* Info   : Adds a one drop item to the player.                              */
/*          (Creates the quest container if needed.)                         */
/*****************************************************************************/
static int GameObject_AddOneDropQuest(lua_State *L)
{
    char       *name, *title=NULL;
    lua_object *self, *whatptr;

    get_lua_args(L, "OOs|s", &self, &whatptr, &name, &title);

    /* store name & arch name of the quest obj. so we can id it later */
    SHSTR_FREE_AND_ADD_STRING(WHAT->name, name);
    SHSTR_FREE_AND_ADD_STRING(WHAT->race, WHAT->arch->name);

    if(title)
    {
        SHSTR_FREE_AND_ADD_STRING(WHAT->title, title);
    }

    if(!CONTR(WHO)->quest_one_drop)
        hooks->add_quest_containers(WHO);
    hooks->insert_ob_in_ob(WHAT, CONTR(WHO)->quest_one_drop);

    return push_object(L, &GameObject, WHAT);
}

/*****************************************************************************/
/* Name   : GameObject_CreatePlayerInfo                                      */
/* Lua    : object:CreatePlayerInfo(name)                                    */
/* Info   : Creates a player_info object of specified name in object's       */
/*          inventory.                                                       */
/*          The values of a player_info object will NOT effect the player.   */
/*          Returns the created object                                       */
/*****************************************************************************/
static int GameObject_CreatePlayerInfo(lua_State *L)
{
    char       *txt;
    object_t     *myob;
    lua_object *whatptr;

    get_lua_args(L, "Os", &whatptr, &txt);
    myob = hooks->arch_to_object(hooks->archetype_global->_player_info);

    /* setup the info and put it in activator */
    SHSTR_FREE_AND_ADD_STRING(myob->name, txt);
    myob = hooks->insert_ob_in_ob(myob, WHAT);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_GetPlayerInfo                                         */
/* Lua    : object:GetPlayerInfo(name)                                       */
/* Info   : get first player_info with the specified name in who's inventory */
/*****************************************************************************/
static int GameObject_GetPlayerInfo(lua_State *L)
{
    char       *name;
    object_t     *walk,
               *next;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    /* get the first linked player_info arch in this inventory */
    FOREACH_OBJECT_IN_OBJECT(walk, WHO, next)
    {
        if (walk->name &&
            walk->arch == hooks->archetype_global->_player_info &&
            !strcmp(walk->name, name))
        {
            return push_object(L, &GameObject, walk);
        }
    }

    return 0; /* there was non */
}

/*****************************************************************************/
/* Name   : GameObject_GetNextPlayerInfo                                     */
/* Lua    : object:GetNextPlayerInfo(player_info)                            */
/* Info   : get next player_info in who's inventory with same name as        */
/*          player_info                                                      */
/*****************************************************************************/
static int GameObject_GetNextPlayerInfo(lua_State *L)
{
    lua_object *myob;
    char        name[SMALL_BUF];
    object_t     *walk;
    lua_object *self;

    get_lua_args(L, "OO", &self, &myob);

    /* thats our check paramters: arch "force_info", name of this arch */
    strncpy(name, STRING_OBJ_NAME(myob->data.object), sizeof(name) - 1); /* 127 chars should be enough for all */
    name[sizeof(name) - 1] = '\0';

    /* get the next linked player_info arch in this inventory */
    for (walk = myob->data.object->below; walk; walk = walk->below)
    {
        if (walk->name &&
            walk->arch == hooks->archetype_global->_player_info &&
            !strcmp(walk->name, name))
        {
            return push_object(L, &GameObject, walk);
        }
    }

    return 0; /* there was non left */
}

/*****************************************************************************/
/* Name   : GameObject_CreateInvisibleInside                                 */
/* Lua    : object:CreateInvisibleObjectInside(id)                           */
/*****************************************************************************/
static int GameObject_CreateInvisibleInside(lua_State *L)
{
    char       *txt;
    object_t     *myob;
    lua_object *whatptr;
    CFParm      CFP;

    get_lua_args(L, "Os", &whatptr, &txt);

    myob = hooks->arch_to_object(hooks->find_archetype("force"));

    if (!myob)
        return luaL_error(L, "object:CreateInvisibleInside(): Can't find archetype 'force'");

    myob->speed = 0.0;
    CFP.Value[0] = (void *) (myob);
    (PlugHooks[HOOK_UPDATESPEED]) (&CFP);

    /*update_ob_speed(myob); */
    SHSTR_FREE_AND_ADD_STRING(myob->slaying, txt);
    myob = hooks->insert_ob_in_ob(myob, WHAT);

    return push_object(L, &GameObject, myob);
}

/* code body of the CreateObjectInside functions */
static object_t *CreateObjectInside_body(lua_State *L, object_t *where, char *archname, int id, int nrof, int value)
{
    object_t *myob;

    myob = hooks->arch_to_object(hooks->find_archetype(archname));

    if (!myob)
    {
        char buf[MEDIUM_BUF];

        sprintf(buf, "object:CreateObjectInside(): Can't find archetype '%s'", archname);
        luaL_error(L, buf);

        return NULL;
    }

    if (value != -1) /* -1 means, we use original value */
        myob->value = value;
    if (id)
    {
        OBJECT_FULLY_IDENTIFY(myob);
    }
    if (nrof > 1)
        myob->nrof = nrof;

    return hooks->insert_ob_in_ob(myob, where);
}

/*****************************************************************************/
/* Name   : GameObject_CreateObjectInside                                    */
/* Lua    : object:CreateObjectInside(archname, identified, number, value)   */
/* Info   : Creates an object from archname and inserts into object.         */
/*          identified is either game.IDENTIFIED or game.UNIDENTIFIED        */
/*          number is the number of objects to create in a stack             */
/*          If value is >= 0 it will be used as the new object's value,      */
/*          otherwise the value will be taken from the arch.                 */
/*****************************************************************************/
static int GameObject_CreateObjectInside(lua_State *L)
{
    object_t *myob;
    int         value = -1, id = 0, nrof = 1;
    char       *txt;
    lua_object *whatptr;

    get_lua_args(L, "Os|iii", &whatptr, &txt, &id, &nrof, &value);

    myob = CreateObjectInside_body(L, WHAT, txt, id, nrof, value);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_CreateObjectInsideEx                                  */
/* Lua    : object:CreateObjectInsideEx(archname, identified, number, value) */
/* Info   : Same as GameObject_CreateObjectInside  but give message to player */
/*          Creates an object from archname and inserts into object.         */
/*          identified is either game.IDENTIFIED or game.UNIDENTIFIED        */
/*          number is the number of objects to create in a stack             */
/*          If value is >= 0 it will be used as the new object's value,      */
/*          otherwise the value will be taken from the arch.                 */
/*****************************************************************************/
static int GameObject_CreateObjectInsideEx(lua_State *L)
{
    object_t     *myob, *pl;
    int         value = -1, id = 0, nrof = 1;
    char       *txt;
    lua_object *self;

    get_lua_args(L, "Os|iii", &self, &txt, &id, &nrof, &value);

    myob = CreateObjectInside_body(L, WHO, txt, id, nrof, value);

    if ((pl = hooks->is_player_inv(myob)))
    {
        hooks->ndi(NDI_UNIQUE | NDI_NAVY, 0, pl, "You got %d %s.",
            (nrof) ? nrof : 1, hooks->query_name(myob, NULL, ARTICLE_NONE, 0));
    }

    return push_object(L, &GameObject, myob);
}

/* help function for GameObject_CheckInventory
 * to recursive check object inventories.
 */
static object_t * object_check_inventory_rec(object_t *tmp, int mode, char *arch_name, char *name, char *title, int type)
{
    object_t *tmp2;

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
/*****************************************************************************/
static int GameObject_CheckInventory(lua_State *L)
{
    lua_object     *self;
    int             type = -1, mode = 0;
    char           *name = NULL, *title = NULL, *arch_name = NULL;
    object_t         *tmp, *tmp2;

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
/*****************************************************************************/
static int GameObject_SetSaveBed(lua_State *L)
{
    lua_object *self,
               *map;
    sint32      x,
                y;
    player_t   *pl;
    map_t      *m;

    get_lua_args(L, "OMii", &self, &map, &x, &y);

    if (WHO->type != PLAYER)
    {
        return luaL_error(L, "object:SetSaveBed() can only be called on a player!");
    }

    pl = CONTR(WHO);
    m = map->data.map;
    SHSTR_FREE_AND_ADD_REF(pl->savebed_map, m->path);
    SHSTR_FREE_AND_ADD_REF(pl->orig_savebed_map, m->orig_path);
    pl->bed_status = MAP_STATUS_TYPE(m->status);
    pl->bed_x = x;
    pl->bed_y = y;
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_DecreaseNrOf                                          */
/* Lua    : object:DecreaseNrOf(nrof)                                        */
/* Info   : Reduces the quantity of a stack by nrof, removing the object if  */
/*          the entire stack is removed.                                     */
/*          nrof is optional. If not given (or 0) it is the same as          */
/*          object:DecreaseNrOf(1). If negative, it is the same as           */
/*          object:DecreaseNrOf(object.quantity).                            */
/* Note   : This method handles stacks properly. Do not be tempted to use    */
/*          object:Remove() as a shortcut for                                */
/*          object:DecreasNrOf(object.quantity) or vice versa.               */
/* Also   : If you want to genuinely remove an object, regardless of its     */
/*          quantity, use object:Remove(). If you want to represent the      */
/*          destruction of an object, use object:Destruct().                 */
/*****************************************************************************/
static int GameObject_DecreaseNrOf(lua_State *L)
{
    lua_object *self;
    int         nrof = 0;

    get_lua_args(L, "O|i", &self, &nrof);

    if (nrof == 0)
    {
        nrof = 1;
    }
    else if (nrof < 0)
    {
        nrof = MAX(1, WHO->nrof);
    }

    hooks->decrease_ob_nr(WHO, nrof);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Remove                                                */
/* Lua    : object:Remove()                                                  */
/* Info   : Takes the object out of whatever map or environment it is in. The*/
/*          object can then be inserted or teleported somewhere else, or just*/
/*          left alone for the garbage collection to take care of.           */
/*          The method takes no arguments.                                   */
/* Note   : This method does not handles stacks properly. Do not be tempted  */
/*          to use object:Remove() as a shortcut for                    `    */
/*          object:DecreasNrOf(object.quantity) or vice versa.               */
/* Also   : If you want reduce the quantity of a stack, use                  */
/*          object:DecreaseNrOf(). If you want to represent the destruction  */
/*          of an object, use object:Destruct().                             */
/*****************************************************************************/
static int GameObject_Remove(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);
    hooks->remove_ob(WHO);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Destruct                                              */
/* Lua    : object:Destruct()                                                */
/* Info   : Only works for monster objects. Other types generate an error.   */
/*          Removes the object from the game and drops all items in object's */
/*          inventory on the floor or in a corpse                            */
/*          The method takes no arguments.                                   */
/* Also   : If you want reduce the quantity of a stack, use                  */
/*          object:DecreaseNrOf(). If you want to quietly remove an object,  */
/*          not fiddle with the stack, use object:Remove()                   */
/*****************************************************************************/
static int GameObject_Destruct(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != MONSTER)
    {
        return luaL_error(L, "Destruct() can only be called on monster!");
    }

    (void)hooks->kill_object(WHO, NULL, NULL, NULL);
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
/*****************************************************************************/
static int GameObject_Move(lua_State *L)
{
    lua_object *self,
               *whatptr = NULL;
    int         dir;

    get_lua_args(L, "Oi|O", &self, &dir, &whatptr);
    lua_pushnumber(L, hooks->move_ob(WHO, dir, (whatptr) ? WHAT : NULL));
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_IdentifyItem                                          */
/* Lua    : object:IdentifyItem(target, marked, mode)                        */
/* Info   : object identifies object(s) in target's inventory.               */
/*          mode: game.IDENTIFY_NORMAL, game.IDENTIFY_ALL or                 */
/*          game.IDENTIFY_MARKED                                             */
/*          marked must be None for IDENTIFY_NORMAL and IDENTIFY_ALL         */
/*****************************************************************************/
static int GameObject_IdentifyItem(lua_State *L)
{
    lua_object *self;
    lua_object *target;
    lua_object *ob      = NULL;
    object_t     *marked  = NULL;
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
/* Info   : Only works for player objects. Other types generate an error.    */
/*          Returns the IP of the player (as a string). If the object is not */
/*          controlled by a player, this is logged and the return is nil.    */
/*****************************************************************************/
static int GameObject_GetIP(lua_State *L)
{
    lua_object  *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER)
    {
        return luaL_error(L, "object:GetIP() can only be called on a player!");
    }

    if (!CONTR(WHO))
    {
        LOG(llevDebug, "LUA - Error - %s[%d] has no controller!\n",
            STRING_OBJ_NAME(WHO), TAG(WHO));

        return 0;
    }

    lua_pushstring(L, CONTR(WHO)->socket.ip_host);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetArchName                                           */
/* Lua    : object:GetArchName()                                             */
/* Info   : Returns the name of object's arhetype.                           */
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
/*****************************************************************************/

static int GameObject_GetItemCost(lua_State *L)
{
    lua_object *self,
               *whatptr;
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
/*****************************************************************************/
static int GameObject_AddMoney(lua_State *L)
{
    lua_object   *self;
    int           c, s, g, m;
    moneyblock_t  money;
    object_t       *loot;

    get_lua_args(L, "Oiiii", &self, &c, &s, &g, &m);
    money.mode = MONEY_MODE_AMOUNT;
    money.mithril = m;
    money.gold = g;
    money.silver = s;
    money.copper = c;
    loot = hooks->create_financial_loot(&money, WHO, MODE_NO_INVENTORY);
    SHSTR_FREE_AND_ADD_STRING(loot->name, "the coins");
    (void)hooks->pick_up(WHO, loot, NULL, 1);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_AddMoneyEx                                            */
/* Lua    : object:AddMoneyEx(copper, silver, gold, mithril)                 */
/* Info   : Same as AddMoney but with message to player how much he got      */
/*****************************************************************************/
static int GameObject_AddMoneyEx(lua_State *L)
{
    lua_object   *self;
    char          buf[MEDIUM_BUF];
    int           c, s, g, m, flag=0;
    moneyblock_t  money;
    object_t       *loot;

    get_lua_args(L, "Oiiii", &self, &c, &s, &g, &m);
    money.mode = MONEY_MODE_AMOUNT;
    money.mithril = m;
    money.gold = g;
    money.silver = s;
    money.copper = c;
    loot = hooks->create_financial_loot(&money, WHO, MODE_NO_INVENTORY);
    SHSTR_FREE_AND_ADD_STRING(loot->name, "the coins");
    (void)hooks->pick_up(WHO, loot, NULL, 1);
    strcpy(buf, "You got");

    if(m)
    {
        sprintf(strchr(buf, '\0'), " %d %s", m, "mithril");
        flag = 1;
    }
    if(g)
    {
        sprintf(strchr(buf, '\0'), "%s %d %s", flag?" and ":"", g,"gold");
        flag = 1;
    }
    if(s)
    {
        sprintf(strchr(buf, '\0'), "%s %d %s", flag?" and ":"", s, "silver");
        flag = 1;
    }
    if(c)
        sprintf(strchr(buf, '\0'), "%s %d %s", flag?" and ":"", c, "copper");

    strcat(buf, " coin.");
    hooks->ndi(NDI_UNIQUE | NDI_NAVY, 0, WHO, "%s", buf);

    return 0;
}

/* TODO: add int64 to pushnumber() */
/*****************************************************************************/
/* Name   : GameObject_GetMoney                                              */
/* Lua    : object:GetMoney()                                                */
/* Info   : returns the amount of money the object carries in copper         */
/*****************************************************************************/

static int GameObject_GetMoney(lua_State *L)
{
    lua_object *self;
    sint64 amount;

    get_lua_args(L, "O", &self);

    amount = hooks->query_money(WHO, NULL);

    /* possible data loss from 64bit integer to double! */
    lua_pushnumber(L, (double)amount);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PayForItem                                            */
/* Lua    : object:PayForItem(object)                                        */
/*****************************************************************************/

static int GameObject_PayForItem(lua_State *L)
{
    lua_object *self,
               *whatptr;
    sint64      price;
    int         val;

    get_lua_args(L, "OO", &self, &whatptr);
    price = hooks->query_cost(WHAT, WHO, F_BUY);
    val = hooks->shop_pay_amount(price, WHO);
    lua_pushnumber(L, val);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PayAmount                                             */
/* Lua    : object:PayAmount(value)                                          */
/* Info   : If object has enough money, value copper will be deducted from   */
/*          object, and 1 will be returned. Otherwise returns 0              */
/*****************************************************************************/
static int GameObject_PayAmount(lua_State *L)
{
    lua_object *self;
    sint64      to_pay;
    int         val;

    get_lua_args(L, "OI", &self, &to_pay);
    val = hooks->shop_pay_amount(to_pay, WHO);
    lua_pushnumber(L, val);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SendCustomCommand                                     */
/* Lua    : object:SendCustomCommand(customcommand)                          */
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
/* Info   : mode = game.MODE_INVENTORY (default) or                    */
/*          game.MODE_NO_INVENTORY                                     */
/*          You should do something with the clone.                          */
/*          SetPosition() and InsertInside() are useful functions for this.  */
/*****************************************************************************/
static int GameObject_Clone(lua_State *L)
{
    lua_object *self;
    int         mode = 0;
    object_t     *clone;

    get_lua_args(L, "O|i", &self, &mode);
    clone = hooks->clone_object(WHO, 0, mode);

    return push_object(L, &GameObject, clone);
}

/*****************************************************************************/
/* Name   : GameObject_GetAI                                                 */
/* Lua    : object:GetAI()                                                   */
/* Info   : Get the AI object for a mob. Mostly useful in behaviours.        */
/*          Will return nil if the mob's AI hasn't been initialized yet      */
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
/*****************************************************************************/
static int GameObject_GetVector(lua_State *L)
{
    lua_object *self,
               *whatptr;
    rv_t        rv;

    get_lua_args(L, "OO", &self, &whatptr);

    if (!hooks->rv_get(WHO, MSP_KNOWN(WHO), WHAT, MSP_KNOWN(WHAT), &rv, RV_FLAG_DIAGONAL_D))
    {
        return 0;
    }

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
#ifndef USE_OLD_UPDATE
    /* I believe we should send client updates here. Currently this seems to
     * happen anyway but my guess is this is because anims are handled by the
     * server so the data is pumped through every tick. -- Smacky 20151027 */
    OBJECT_UPDATE_UPD(WHO, UPD_ANIM);
#else
#endif
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetInvAnimation                                       */
/* Lua    : object:SetInvAnimation(anim)                                     */
/* Info   : Sets object's inventory animation.                               */
/*          Note that an object will only be animated if object.f_is_animated*/
/*          is true                                                          */
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
#ifndef USE_OLD_UPDATE
    /* I believe we should send client updates here. Currently this seems to
     * happen anyway but my guess is this is because anims are handled by the
     * server so the data is pumped through every tick. -- Smacky 20151027 */
    OBJECT_UPDATE_UPD(WHO, UPD_ANIM);
#else
#endif
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetFace                                               */
/* Lua    : object:SetFace(face)                                             */
/* Info   : Sets object's face.                                              */
/*          If the object is animated (object.f_is_animated == true), then   */
/*          this value will likely be replaced at the next animation step    */
/*****************************************************************************/
static int GameObject_SetFace(lua_State *L)
{
    lua_object *self;
    char *face;
    int id;

    get_lua_args(L, "Os", &self, &face);

    id = hooks->FindFace(face, -1);
    if(id == -1)
        luaL_error(L, "no such face exists: %s", STRING_SAFE(face));

    WHO->face = &(*hooks->new_faces)[id];
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(WHO, UPD_FACE);
#else
    hooks->update_object(WHO, UP_OBJ_FACE);
#endif
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetInvFace                                            */
/* Lua    : object:SetInvFace(face)                                          */
/* Info   : Sets object's inventory face.                                    */
/*          If the object is animated (object.f_is_animated == true), then   */
/*          this value will likely be replaced at the next animation step    */
/* Version: Introduced in beta 4 pre4                                        */
/*****************************************************************************/
static int GameObject_SetInvFace(lua_State *L)
{
    lua_object *self;
    char *face;
    int id;

    get_lua_args(L, "Os", &self, &face);

    id = hooks->FindFace(face, -1);
    if(id == -1)
        luaL_error(L, "no such face exists: %s", STRING_SAFE(face));

    WHO->inv_face = &(*hooks->new_faces)[id];
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(WHO, UPD_FACE);
#else
    hooks->update_object(WHO, UPD_FACE);
#endif
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
        objectlink_t *ol;
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
/*              Game.GMASTER_MODE_NO    if the object is not SA/MM/MW/GM/VOL */
/*              Game.GMASTER_MODE_SA    if the object is SA                  */
/*              Game.GMASTER_MODE_MM    if the object is MM                  */
/*              Game.GMASTER_MODE_MW    if the object is MW                  */
/*              Game.GMASTER_MODE_GM    if the object is GM                  */
/*              Game.GMASTER_MODE_VOL   if the object is VOL                 */
/*****************************************************************************/
static int GameObject_GetGmasterMode(lua_State *L)
{
    lua_object *self;
    uint8       mode_id = GMASTER_MODE_NO;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        luaL_error(L, "GetGmasterMode() can only be called on a legal player object.");

    /* FIXME: Problem: gmaster_mode is a bitmask but Lua isn't bitwise.
     * Temp solution: Until it is (we shouldn't use external addons to the
     * language), we should approximate the return value from the object's
     * gmaster_mode. The logic below means GM/VOL status takes preference over
     * MM/MW status!
     * -- Smacky 20100731 */
    if ((CONTR(WHO)->gmaster_mode & GMASTER_MODE_SA))
    {
        mode_id = GMASTER_MODE_SA;
    }
    else
    {
        if (((CONTR(WHO)->gmaster_mode & GMASTER_MODE_MM)))
        {
            mode_id = GMASTER_MODE_MM;
        }
        else if (((CONTR(WHO)->gmaster_mode & GMASTER_MODE_MW)))
        {
            mode_id = GMASTER_MODE_MW;
        }

        if (((CONTR(WHO)->gmaster_mode & GMASTER_MODE_GM)))
        {
            mode_id = GMASTER_MODE_GM;
        }
        else if (((CONTR(WHO)->gmaster_mode & GMASTER_MODE_VOL)))
        {
            mode_id = GMASTER_MODE_VOL;
        }
    }

    lua_pushnumber(L, mode_id);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetPlayerWeightLimit                                  */
/* Lua    : object:GetPlayerWeightLimit()                                    */
/* Info   : Only works for player objects. Returns the real weight limit     */
/*        : of a player including stat bonus                                 */
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
/* TODO   : Much.                                                            */
/*****************************************************************************/
static int GameObject_GetGroup(lua_State *L)
{
    lua_object *self;
    object_t     *member,
               *leader;
    int         nrof,
                i;

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

    for (member = leader, nrof = CONTR(leader)->group_nrof, i = 1;
         member && i <= nrof;
         member = CONTR(member)->group_next, i++)
    {
        push_object(L, &GameObject, member);
        lua_rawseti(L, -2, i);
    }

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetTarget                                             */
/* Lua    : object:GetTarget()                                               */
/* Info   : Only works for player or monster objects. Other types generate   */
/*          an error.                                                        */
/*          The function takes no arguments.                                 */
/*          The return is the object that is the current target.             */
/* TODO   : To be expanded to handle monsters.                               */
/*****************************************************************************/
static int GameObject_GetTarget(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    /* Only players can have targets */
    if (WHO->type == PLAYER && CONTR(WHO))
    {
        return push_object(L, &GameObject, CONTR(WHO)->target_ob);
    }
    else if (WHO->type == MONSTER)
    {
        if (WHO->enemy) // But if a monster has an enemy, that's close enough to a target.
        {
            return push_object(L, &GameObject, WHO->enemy);
        }
        else
        {
            lua_pushnil(L);

            return 0;
        }
    }
    else
    {
        return luaL_error(L, "GetTarget() can only be called on a player!");
    }
}


/*****************************************************************************/
/* Name   : GameObject_SetTarget                                             */
/* Lua    : object:SetTarget(target)                                         */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          The mandatory argument is a number, a string, or an object.      */
/*          If a number, it should be one of the game.TARGET_* constants. The*/
/*          *next* available target of that kind is targeted, according to   */
/*          the normal rules.                                                */
/*          If a string (it is case sensitive), all available enemies then   */
/*          friends are targeted until one with a name matching the string is*/
/*          found.                                                           */
/*          If an object, that object is targeted, if possible.              */
/*          The return is the object which is the new target, or nil.        */
/* TODO   : To be expanded to handle monsters.                               */
/*****************************************************************************/
static int GameObject_SetTarget(lua_State *L)
{
    lua_object *self,
               *whatptr;
    char       *name = NULL;
    int         mode = -1;
    char        buf[2];

    if (lua_isnumber(L, 2))
        get_lua_args(L, "Oi", &self, &mode);
    else if (lua_isstring(L, 2))
        get_lua_args(L, "Os", &self, &name);
    else
        get_lua_args(L, "OO", &self, &whatptr);

    /* Only players can have targets */
    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "SetTarget() can only be called on a player!");

    /* If the parameter is a number, find *next* appropriate target. */
    if (lua_isnumber(L, 2))
    {
        if (mode < 0 || mode > 2)
            return luaL_error(L, "SetTarget() passed invalid mode (%d)!", mode);

        sprintf(buf, "%d", mode);
        hooks->command_target(WHO, buf);
    }
    /* If the parameter is a string, we're looking for just an object with the
     * given name. We don't know if it is a friend or enemy, or if there are
     * more than one objects in range with that name, or anything. So simply
     * cycle through all nearby enemies then all nearby friends, stopping as
     * soon as we have a target with the right name. */
    else if (lua_isstring(L, 2))
    {
        int i;

        for (i = 0; i <= 1; i++)
        {
            object_t *original = NULL,
                   *current = NULL;

            sprintf(buf, "%d", i);

            CONTR(WHO)->target_ob = NULL; // force a new search
            hooks->command_target(WHO, buf);
            current = original = CONTR(WHO)->target_ob; // the start of our loop

            if (current)
            {
                do
                {
                    if (!strcmp(current->name, name))
                        break;

                    hooks->command_target(WHO, buf);
                    current = CONTR(WHO)->target_ob;
                }
                while (current && current != original);

                if (current && strcmp(current->name, name))
                    CONTR(WHO)->target_ob = NULL;
            }
        }
    }
    /* If the parameter is an object, check if it's an enemy or a friend and
     * cycle through the appropriate targets until we have it or have looped
     * right round (which means it is not in range/visible so return nil). */
    /* FIXME: I doubt if this even works. */
    else
    {
        object_t *original,
                 *current;

        CONTR(WHO)->target_index = 0; // force a new search
        CONTR(WHO)->target_ob = NULL; // force a new search
        sprintf(buf, "%d", (hooks->get_friendship(WHO, WHAT) <= FRIENDSHIP_ATTACK) ? 0 : 1);
        hooks->command_target(WHO, buf);
        current = original = CONTR(WHO)->target_ob; // the start of our loop

        if (current)
        {
            do
            {
                if (current == WHAT)
                    break;

                hooks->command_target(WHO, buf);
                current = CONTR(WHO)->target_ob;
            }
            while (current && current != original);

            if (current != WHAT)
                CONTR(WHO)->target_ob = NULL;
        }
    }

    return push_object(L, &GameObject, CONTR(WHO)->target_ob);
}

/*****************************************************************************/
/* Name   : GameObject_GetCombatMode                                         */
/* Lua    : object:GetCombatMode()                                           */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          The function takes no arguments.                                 */
/*          The return is true if object is in combat mode, false otherwise. */
/*****************************************************************************/
static int GameObject_GetCombatMode(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "GetCombatMode() can only be called on a player!");

    lua_pushboolean(L, CONTR(WHO)->combat_mode);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SetCombatMode                                         */
/* Lua    : object:SetCombatMode(mode)                                       */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          The mandatory argument is true to turn on combat mode, false to  */
/*          turn it off.                                                     */
/*          The return is the new combat mode (so true or false).            */
/*****************************************************************************/
static int GameObject_SetCombatMode(lua_State *L)
{
    lua_object *self;
    int         mode;

    get_lua_args(L, "Ob", &self, &mode);

    /* Only players have combat mode */
    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "SetCombatMode() can only be called on a player!");

    CONTR(WHO)->combat_mode = (mode) ? 0 : 1; // set to *opposite* of what we want
    hooks->command_combat(WHO, NULL);
    lua_pushboolean(L, CONTR(WHO)->combat_mode);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_FindNextObject                                        */
/* Lua    : object:FindNextObject(type, mode, root, arch_name, name, title)  */
/* Info   : Returns the next object in object's local inventory tree, or nil.*/
/* TODO   : Proper doc.                                                      */
/*          Extended return.                                                 */
/*****************************************************************************/
static int GameObject_FindNextObject(lua_State *L)
{
    lua_object *self,
               *whatptr = NULL;
    object_t     *next;
    int         type,
                mode = FNO_MODE_ALL;
    char       *arch_name = NULL,
               *name = NULL,
               *title = NULL;

    get_lua_args(L, "Oi|iOsss", &self, &type, &mode, &whatptr, &arch_name, &name, &title);
    next = WHO;

    while (next)
    {
        next = hooks->find_next_object(next, (uint8)type, (uint8)mode, (whatptr) ? WHAT : NULL);

        if (next &&
            (!arch_name || (next->arch && next->arch->name && !strcmp(arch_name, next->arch->name))) &&
            (!name || (next->name && !strcmp(name, next->name))) &&
            (!title || (next->title && !strcmp(title, next->title))))
            break;
    }

    return push_object(L, &GameObject, next);
}

/*****************************************************************************/
/* Name   : GameObject_GetPersonalLight                                      */
/* Lua    : object:GetPersonalLight()                                        */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          The function takes no arguments.                                 */
/*          The return is the object's personal light value (0 means off).   */
/*****************************************************************************/
static int GameObject_GetPersonalLight(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "GetPersonalLight() can only be called on a player!");

    lua_pushnumber(L, CONTR(WHO)->personal_light);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SetPersonalLight                                      */
/* Lua    : object:SetPersonalLight(mode)                                    */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          The mandatory argument is the personal light value to set (this  */
/*          will be normalised to the range 0 <= value <= MAX_DARKNESS). A   */
/*          value of 0 means turn it off.                                    */
/*          The return is the new personal light setting.                    */
/*****************************************************************************/
static int GameObject_SetPersonalLight(lua_State *L)
{
    lua_object *self;
    int         value;

    get_lua_args(L, "Oi", &self, &value);

    /* Only players have combat mode */
    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "SetPersonalLight() can only be called on a player!");

    hooks->set_personal_light(CONTR(WHO), value);
    lua_pushnumber(L, CONTR(WHO)->personal_light);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_UpdateQuest                                           */
/* Lua    : object:UpdateQuest(name, text)                                   */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          Updates name quest for player object with text string.           */
/* Return : nil if the named quest has not been undertaken by player, or true*/
/*          or false otherwise to indicate whether the update was successful.*/
/*****************************************************************************/
static int GameObject_UpdateQuest(lua_State *L)
{
    lua_object *self;
    char       *name,
               *text;
    object_t     *quest;

    get_lua_args(L, "Oss", &self, &name, &text);

    /* Only players can have quests. */
    if (WHO->type != PLAYER || CONTR(WHO) == NULL)
        return luaL_error(L, "UpdateQuest() can only be called on a player!");

    /* Find the actual named quest in the player's quest_containers. If we
     * can't, return nil. */
    if (!(quest = hooks->quest_find_name(WHO, name)))
        lua_pushnil(L);
    /* Update quest, returning true or false to indicate success. */
    else
    {
        int b = hooks->update_quest(quest, ST1_QUEST_UPDATE_ARBITRARY, NULL,
                                    text, NULL);

        lua_pushboolean(L, b);
    }

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AdjustLightSource                                     */
/* Lua    : object:AdjustLightSource(value)                                  */
/* Info   : Sets the object's glow radius to value.                          */
/*          Any object can have a glow radius, though there are two specific */
/*          light types (applyable and static). Applyable lights generally   */
/*          give some sort of message when applied and can even be set up to */
/*          not be applyable at all! This method ignores all that: An        */
/*          applyable light adjusted with this method is always guaranteed to*/
/*          turn on/off (depending on value -- 0 means off, any other value  */
/*          means on) and no message will be generated.                      */
/*          The mandatory value argument is an integer from -9 to 9.         */
/*          The method returns nil.                                          */
/* Notes  : If you want to apply an applyable light according to normal      */
/*          rules, this is the wrong method. Use object:Apply().             */
/*****************************************************************************/
static int GameObject_AdjustLightSource(lua_State *L)
{
    lua_object *self;
    int         value,
                relative;

    get_lua_args(L, "Oi", &self, &value);

    /* Limit to range -9 <= value <= 9. */
    value = MAX(-9, MIN(value, 9));

    /* An actual light object? */
    if (WHO->type == LIGHT_SOURCE ||
        WHO->type == TYPE_LIGHT_APPLY)
    {
        /* Ensure it's off. */
        if (WHO->glow_radius)
            hooks->turn_off_light(WHO);

        /* Turn it back on at value intensity. */
        if (value)
        {
            WHO->last_sp = value;
            hooks->turn_on_light(WHO);
        }

        return 0;
    }

    /* Already glowing at that value? All is well then. */
    if (WHO->glow_radius == value)
        return 0;

    /* Adjust the map's light mask to the new glow radius. */
    relative = -(WHO->glow_radius - value);
    WHO->glow_radius = value;
    hooks->adjust_light_source(MSP_KNOWN(WHO), relative);
#ifndef USE_OLD_UPDATE
    OBJECT_UPDATE_UPD(WHO, UPD_FACE);
#else
    hooks->update_object(WHO, UP_OBJ_FACE);
#endif
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetConnection                                         */
/* Lua    : object:GetConnection()                                           */
/* Info   : Gets an object's connection value.                               */
/*          The function takes no arguments.                                 */
/*          The return is the object's connection value (0 means none).      */
/*****************************************************************************/
static int GameObject_GetConnection(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    lua_pushnumber(L, hooks->get_button_value(WHO));

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetAccountName                                        */
/* Lua    : object:GetAccountName()                                          */
/* Info   : Only works for player objects. Other types generate an error.    */
/*          Returns the account name of the player. If the object is not     */
/*          controlled by a player, this is logged and the return is nil.    */
/*****************************************************************************/
static int GameObject_GetAccountName(lua_State *L)
{
    lua_object  *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER)
    {
        return luaL_error(L, "object:GetAccountName() can only be called on a player!");
    }

    if (!CONTR(WHO))
    {
        LOG(llevDebug, "LUA - Error - %s[%d] has no controller!\n",
            STRING_OBJ_NAME(WHO), TAG(WHO));

        return 0;
    }

    lua_pushstring(L, CONTR(WHO)->account_name);

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AddBuff                                               */
/* Lua    : object:AddBuff(buffname, buffstring)                             */
/* Info   : Creates a force with specified stats that is added to the object.*/
/* The stats of that force are then added to the object. buffstring should be*/
/* an arch definition string, excluding the arch, name, and end attributes.  */
/* Returns:                                                                  */
/* BUFF_ADD_SUCCESS - Nothing went wrong.                                    */
/* BUFF_ADD_EXISTS - The buff exists, but that does not mean it failed.      */
/* BUFF_ADD_LIMITED - Too many of the same buff.                             */
/* BUFF_ADD_MAX_EXCEEDED - item->max_buffs exceeded.                         */
/* BUFF_ADD_BAD_PARAMS - item or buff == NULL                                */
/* BUFF_ADD_NO_INSERT - Something went wrong in insert_ob_in_ob - unlikely   */
/*****************************************************************************/
static int GameObject_AddBuff(lua_State *L)
{
    lua_object     *self = NULL;
    char           *buff = NULL;
    char            buff_str[LARGE_BUF];
	char		   *name = NULL;
    object_t         *obj  = NULL;

    get_lua_args(L, "Oss", &self, &name, &buff);

    sprintf(buff_str, "arch buff_force\nname %s\n%s\nend", name, buff);
    obj = hooks->load_object_str(buff_str);

    lua_pushnumber(L, hooks->buff_add(WHO, obj, 0));

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_CheckBuff                                             */
/* Lua    : object:CheckBuff(buffname)                                       */
/* Info   : Check if this buff can be added. It goes through the same process*/
/*          as AddBuff but doesn't modify the item at all                    */
/* Returns:                                                                  */
/* BUFF_ADD_SUCCESS - Nothing went wrong.                                    */
/* BUFF_ADD_EXISTS - The buff exists, but that does not mean it failed.      */
/* BUFF_ADD_LIMITED - Too many of the same buff.                             */
/* BUFF_ADD_MAX_EXCEEDED - item->max_buffs exceeded.                         */
/* BUFF_ADD_BAD_PARAMS - item or buff == NULL                                */
/* BUFF_ADD_NO_INSERT - Something went wrong in insert_ob_in_ob - unlikely   */
/*****************************************************************************/
static int GameObject_CheckBuff(lua_State *L)
{
    lua_object     *self = NULL;
    char           *buff = NULL;
    char            buff_str[LARGE_BUF];
    object_t         *obj  = NULL;

    get_lua_args(L, "Os", &self, &buff);

    sprintf(buff_str, "arch buff_force\nname %s\nend", buff);
    obj = hooks->load_object_str(buff_str);

    lua_pushnumber(L, hooks->buff_add(WHO, obj, 1));

    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_RemoveBuff                                            */
/* Lua    : object:RemoveBuff(buffname, nrof)                                */
/* Info   : Removes the specified amount of a buff. Nrof 0 means remove all. */
/* Returns:                                                                  */
/* BUFF_ADD_BAD_PARAMS - Something was null                                  */
/* BUFF_ADD_EXISTS - That buff doesn't exist (poorly-named)                  */
/* BUFF_ADD_SUCCESS - Success                                                */
/*****************************************************************************/
static int GameObject_RemoveBuff(lua_State *L)
{
    lua_object     *self = NULL;
    char           *name = NULL;
    int             nrof = 0;

    get_lua_args(L, "Os|i", &self, &name, &nrof);

    lua_pushnumber(L, hooks->buff_remove(WHO, name, nrof));

    return 1;
}

/* FUNCTIONEND -- End of the GameObject methods. */
