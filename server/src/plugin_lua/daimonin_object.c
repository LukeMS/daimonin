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
#include <daimonin_object.h>

/* Global data */

/* Available python methods for the GameObject object */
static struct method_decl   GameObject_methods[]            =
{
    {"SetSaveBed",  (lua_CFunction) GameObject_SetSaveBed}, {"GetSkill",  (lua_CFunction) GameObject_GetSkill},
    {"SetSkill",  (lua_CFunction) GameObject_SetSkill}, {"ActivateRune",  (lua_CFunction) GameObject_ActivateRune},
    {"CastAbility",  (lua_CFunction) GameObject_CastAbility},
    {"InsertInside",  (lua_CFunction) GameObject_InsertInside}, {"GetGod",  (lua_CFunction) GameObject_GetGod},
    {"SetGod",  (lua_CFunction) GameObject_SetGod}, {"TeleportTo",  (lua_CFunction) GameObject_TeleportTo},
    {"Apply",  (lua_CFunction) GameObject_Apply}, {"PickUp",  (lua_CFunction) GameObject_PickUp},
    {"Drop",  (lua_CFunction) GameObject_Drop}, {"Take",  (lua_CFunction) GameObject_Take},
    {"Fix", (lua_CFunction) GameObject_Fix}, {"Kill", (lua_CFunction) GameObject_Kill},
    {"CastSpell", (lua_CFunction) GameObject_CastSpell}, {"DoKnowSpell", (lua_CFunction) GameObject_DoKnowSpell},
    {"AcquireSpell", (lua_CFunction) GameObject_AcquireSpell}, 
    {"FindSkill", (lua_CFunction) GameObject_FindSkill},
    {"AcquireSkill", (lua_CFunction) GameObject_AcquireSkill},
    {"FindMarkedObject", (lua_CFunction) GameObject_FindMarkedObject},
    {"CheckQuestObject", (lua_CFunction) GameObject_CheckQuestObject},
    {"AddQuestObject", (lua_CFunction) GameObject_AddQuestObject},
    {"CreatePlayerForce", (lua_CFunction) GameObject_CreatePlayerForce},
    {"CreatePlayerInfo", (lua_CFunction) GameObject_CreatePlayerInfo},
    {"GetPlayerInfo", (lua_CFunction) GameObject_GetPlayerInfo},
    {"GetNextPlayerInfo", (lua_CFunction) GameObject_GetNextPlayerInfo},
    {"CheckInvisibleObjectInside", (lua_CFunction) GameObject_CheckInvisibleInside},
    {"CreateInvisibleObjectInside", (lua_CFunction) GameObject_CreateInvisibleInside},
    {"CreateObjectInside", (lua_CFunction) GameObject_CreateObjectInside},
    {"CheckInventory", (lua_CFunction) GameObject_CheckInventory}, {"Remove", (lua_CFunction) GameObject_Remove},
    {"Destruct", (lua_CFunction) GameObject_Destruct}, {"SetPosition", (lua_CFunction) GameObject_SetPosition},
    {"IdentifyItem", (lua_CFunction) GameObject_IdentifyItem},
#if 0
    {"GetEventHandler", (lua_CFunction) GameObject_GetEventHandler},
    {"SetEventHandler", (lua_CFunction) GameObject_SetEventHandler},
    {"GetEventPlugin", (lua_CFunction) GameObject_GetEventPlugin},
    {"SetEventPlugin", (lua_CFunction) GameObject_SetEventPlugin},
    {"GetEventOptions", (lua_CFunction) GameObject_GetEventOptions},
    {"SetEventOptions", (lua_CFunction) GameObject_SetEventOptions},
#endif
    {"Deposit",  (lua_CFunction) GameObject_Deposit}, {"Withdraw",  (lua_CFunction) GameObject_Withdraw},
    {"Communicate",  (lua_CFunction) GameObject_Communicate}, {"Say",  (lua_CFunction) GameObject_Say},
    {"SayTo",  (lua_CFunction) GameObject_SayTo}, {"Write", (lua_CFunction) GameObject_Write},
    {"SetGender",  (lua_CFunction) GameObject_SetGender}, {"SetRank",  (lua_CFunction) GameObject_SetRank},
    {"SetAlignment",  (lua_CFunction) GameObject_SetAlignment},
    {"GetAlignmentForce",  (lua_CFunction) GameObject_GetAlignmentForce},
    {"SetGuildForce",  (lua_CFunction) GameObject_SetGuildForce},
    {"GetGuildForce",  (lua_CFunction) GameObject_GetGuildForce}, {"IsOfType", (lua_CFunction) GameObject_IsOfType},
    {"Save", (lua_CFunction) GameObject_Save}, {"GetIP", (lua_CFunction) GameObject_GetIP},
    {"GetArchName", (lua_CFunction) GameObject_GetArchName}, {"ShowCost",  (lua_CFunction) GameObject_ShowCost},
    {"GetItemCost",  (lua_CFunction) GameObject_GetItemCost}, {"GetMoney",  (lua_CFunction) GameObject_GetMoney},
    {"PayForItem", (lua_CFunction) GameObject_PayForItem}, {"PayAmount", (lua_CFunction) GameObject_PayAmount},
    {"SendCustomCommand",(lua_CFunction) GameObject_SendCustomCommand},
    {"CheckTrigger", (lua_CFunction) GameObject_CheckTrigger}, {"Clone", (lua_CFunction) GameObject_Clone},

    // {"GetUnmodifiedAttribute", (lua_CFunction)GameObject_GetUnmodifiedAttribute},
    {NULL, NULL}
};

/* GameObject attributes */
struct attribute_decl       GameObject_attributes[]         =
{
    /* All entries MUST be in same order as field_id enum above */
    {"below", FIELDTYPE_OBJECT, offsetof(object, below), FIELDFLAG_READONLY},
    {"above", FIELDTYPE_OBJECT, offsetof(object, above), FIELDFLAG_READONLY},
    {"inventory", FIELDTYPE_OBJECT, offsetof(object, inv), FIELDFLAG_READONLY},
    {"map", FIELDTYPE_MAP, offsetof(object, map), FIELDFLAG_READONLY},
    {"name", FIELDTYPE_SHSTR, offsetof(object, name), FIELDFLAG_PLAYER_READONLY},
    {"title", FIELDTYPE_SHSTR, offsetof(object, title), 0}, {"race", FIELDTYPE_SHSTR, offsetof(object, race), 0},
    {"slaying", FIELDTYPE_SHSTR, offsetof(object, slaying), 0},
    /* TODO: need special handling (check for endmsg, limit to 4096 chars?) ?*/
    {"message", FIELDTYPE_SHSTR, offsetof(object, msg), 0},
    /* TODO: limited to >=0 */
    {"weight",       FIELDTYPE_SINT32, offsetof(object, weight), 0},
    {"weight_limit", FIELDTYPE_UINT32, offsetof(object, weight_limit), 0},
    {"carrying",     FIELDTYPE_SINT32, offsetof(object, carrying), 0},
    {"path_attuned", FIELDTYPE_UINT32, offsetof(object, path_attuned), 0},
    {"path_repelled",FIELDTYPE_UINT32, offsetof(object, path_repelled), 0},
    {"path_denied",  FIELDTYPE_UINT32, offsetof(object, path_denied), 0},
    {"value",        FIELDTYPE_SINT32, offsetof(object, value), 0},
    /* TODO: Max 100000 */
    {"quantity",         FIELDTYPE_UINT32, offsetof(object, nrof), 0},

    /* TODO: I don't know what these do, or if they should be accessible... */
    {"damage_round_tag", FIELDTYPE_UINT32, offsetof(object, damage_round_tag), 0},
    {"update_tag",   FIELDTYPE_UINT32, offsetof(object, update_tag), 0},

    /* TODO: make enemy & owner settable (requires HOOKS for set_npc_enemy() and set_owner()) */
    {"enemy",        FIELDTYPE_OBJECTREF, offsetof(object, enemy), FIELDFLAG_READONLY, offsetof(object, enemy_count)},
    // TODO: remove    {"attacked_by",  FIELDTYPE_OBJECTREF, offsetof(object, attacked_by), FIELDFLAG_READONLY, offsetof(object, attacked_by_count)},
    {"owner",        FIELDTYPE_OBJECTREF, offsetof(object, owner), FIELDFLAG_READONLY, offsetof(object, ownercount)},
    {"x",            FIELDTYPE_SINT16, offsetof(object, x), FIELDFLAG_READONLY},
    {"y",            FIELDTYPE_SINT16, offsetof(object, y), FIELDFLAG_READONLY},
    {"last_damage",  FIELDTYPE_UINT16, offsetof(object, last_damage), 0},
    {"terrain_type", FIELDTYPE_UINT16, offsetof(object, terrain_type), 0},
    {"terrain_flag", FIELDTYPE_UINT16, offsetof(object, terrain_flag), 0},
    {"material",     FIELDTYPE_UINT16, offsetof(object, material), 0},
    {"material_real",FIELDTYPE_SINT16, offsetof(object, material_real), 0},
    {"last_heal",    FIELDTYPE_SINT16, offsetof(object, last_heal), 0},
    /* TODO: Limit to max 16000 ? */
    {"last_sp",      FIELDTYPE_SINT16, offsetof(object, last_sp), 0},
    /* TODO: Limit to max 16000 ? */
    {"last_grace",   FIELDTYPE_SINT16, offsetof(object, last_grace), 0},
    {"last_eat",     FIELDTYPE_SINT16, offsetof(object, last_eat), 0},
    /* TODO: will require animation lookup function. How about face, is that a special anim? */
    {"animation_id", FIELDTYPE_UINT16, offsetof(object, animation_id), 0},
    {"inv_animation_id", FIELDTYPE_UINT16, offsetof(object, inv_animation_id), 0},
    {"magic",        FIELDTYPE_SINT8 , offsetof(object, magic), 0},
    {"state",        FIELDTYPE_UINT8 , offsetof(object, state), 0},
    {"level",        FIELDTYPE_SINT8 , offsetof(object, level), FIELDFLAG_PLAYER_READONLY},
    {"direction",    FIELDTYPE_SINT8 , offsetof(object, direction), 0},
    {"facing",       FIELDTYPE_SINT8 , offsetof(object, facing), 0},
    {"quick_pos",    FIELDTYPE_UINT8 , offsetof(object, quick_pos), 0},
    {"type",         FIELDTYPE_UINT8 , offsetof(object, type), FIELDFLAG_READONLY},
    {"sub_type_1",   FIELDTYPE_UINT8 , offsetof(object, sub_type1), 0},
    {"item_quality", FIELDTYPE_UINT8 , offsetof(object, item_quality), 0},
    {"item_condition", FIELDTYPE_UINT8 , offsetof(object, item_condition), 0},
    {"item_race",    FIELDTYPE_UINT8 , offsetof(object, item_race), 0},
    {"item_level",   FIELDTYPE_UINT8 , offsetof(object, item_level), 0},
    {"item_skill",   FIELDTYPE_UINT8 , offsetof(object, item_skill), 0},
    {"glow_radius",  FIELDTYPE_SINT8 , offsetof(object, glow_radius), 0},
    {"move_status",  FIELDTYPE_SINT8 , offsetof(object, move_status), 0},
    {"move_type",    FIELDTYPE_UINT8 , offsetof(object, move_type), 0},
    {"anim_enemy_dir", FIELDTYPE_SINT8 , offsetof(object, anim_enemy_dir), 0},
    {"anim_moving_dir", FIELDTYPE_SINT8 , offsetof(object, anim_moving_dir), 0},
    {"anim_enemy_dir_last", FIELDTYPE_SINT8 , offsetof(object, anim_enemy_dir_last), 0},
    {"anim_moving_dir_last", FIELDTYPE_SINT8 , offsetof(object, anim_moving_dir_last), 0},
    {"anim_last_facing", FIELDTYPE_SINT8 , offsetof(object, anim_last_facing), 0},
    {"anim_last_facing_last", FIELDTYPE_SINT8 , offsetof(object, anim_last_facing_last), 0},
    {"anim_speed",   FIELDTYPE_UINT8 , offsetof(object, anim_speed), 0},
    {"last_anim",    FIELDTYPE_UINT8 , offsetof(object, last_anim), 0},
    {"will_apply",   FIELDTYPE_UINT8 , offsetof(object, will_apply), 0},
    {"run_away",     FIELDTYPE_UINT8 , offsetof(object, run_away), 0},
    {"pick_up",      FIELDTYPE_UINT8 , offsetof(object, pick_up), 0},
    {"hide",         FIELDTYPE_UINT8 , offsetof(object, hide), 0},
    {"layer",        FIELDTYPE_UINT8 , offsetof(object, layer), 0},

    /*
    "RESIST,       FIELDTYPE_SINT8 , offsetof(object, anim_moving_dir), 0},
    "ATTACK,       FIELDTYPE_SINT8 , offsetof(object, anim_moving_dir), 0},
    "PROTECTION,   FIELDTYPE_SINT8 , offsetof(object, anim_moving_dir), 0},
    */

    /* TODO: -10.0 < speed < 10.0, also might want to call update_object_speed() */
    {"speed",        FIELDTYPE_FLOAT, offsetof(object, speed), FIELDFLAG_PLAYER_READONLY},
    {"speed_left",   FIELDTYPE_FLOAT, offsetof(object, speed_left), 0},
    {"weapon_speed", FIELDTYPE_FLOAT, offsetof(object, weapon_speed), 0},
    {"weapon_speed_left", FIELDTYPE_FLOAT, offsetof(object, weapon_speed_left), 0},
    {"weapon_speed_add", FIELDTYPE_FLOAT, offsetof(object, weapon_speed_add), 0},

    /* Stats */
    {"experience",     FIELDTYPE_SINT32, offsetof(object, stats.exp), 0},
    {"hitpoints",      FIELDTYPE_SINT32, offsetof(object, stats.hp), 0},
    {"max_hitpoints",   FIELDTYPE_SINT32, offsetof(object, stats.maxhp), FIELDFLAG_PLAYER_READONLY},
    {"spellpoints",      FIELDTYPE_SINT16, offsetof(object, stats.sp), 0},
    {"max_spellpoints",   FIELDTYPE_SINT16, offsetof(object, stats.maxsp), FIELDFLAG_PLAYER_READONLY},
    /* TODO: Limit to +- 16000 ? */
    {"grace",   FIELDTYPE_SINT16, offsetof(object, stats.grace), 0},
    {"max_grace",FIELDTYPE_SINT16, offsetof(object, stats.maxgrace), FIELDFLAG_PLAYER_READONLY},
    /* TODO: Limit to max 999 (at least for players) ? */
    {"food",    FIELDTYPE_SINT16, offsetof(object, stats.food), 0},
    /* TODO: Limit to 0 <= dam <= 120 ? */
    {"damage",     FIELDTYPE_SINT16, offsetof(object, stats.dam), FIELDFLAG_PLAYER_READONLY},
    /* TODO: Limit to +-120 */
    {"weapon_class",      FIELDTYPE_SINT16, offsetof(object, stats.wc), FIELDFLAG_PLAYER_READONLY},
    /* TODO: Limit to +-120 */
    {"armour_class",       FIELDTYPE_SINT16, offsetof(object, stats.ac),FIELDFLAG_PLAYER_READONLY},
    {"weapon_class_range", FIELDTYPE_UINT8,  offsetof(object, stats.wc_range), 0},
    /* TODO: Limit to +-30 (all  */
    {"strength",     FIELDTYPE_SINT8, offsetof(object, stats.Str), FIELDFLAG_PLAYER_FIX},
    {"dexterity",     FIELDTYPE_SINT8, offsetof(object, stats.Dex), FIELDFLAG_PLAYER_FIX},
    {"constitution",     FIELDTYPE_SINT8, offsetof(object, stats.Con), FIELDFLAG_PLAYER_FIX},
    {"wisdom",     FIELDTYPE_SINT8, offsetof(object, stats.Wis), FIELDFLAG_PLAYER_FIX},
    {"charisma",     FIELDTYPE_SINT8, offsetof(object, stats.Cha), FIELDFLAG_PLAYER_FIX},
    {"intelligence",     FIELDTYPE_SINT8, offsetof(object, stats.Int), FIELDFLAG_PLAYER_FIX},
    {"power",     FIELDTYPE_SINT8, offsetof(object, stats.Pow), FIELDFLAG_PLAYER_FIX},
    {"luck",    FIELDTYPE_SINT8, offsetof(object, stats.luck), FIELDFLAG_PLAYER_READONLY}, {NULL}
};

/* This is a list of strings that correspond to the FLAG_.. values.
 * This is a simple 1:1 mapping - if FLAG_FRIENDLY is 15, then
 * the 15'th element of this array should match that name.
 * If an entry is NULL, that flag cannot be set/read from scripts
 * Yes, this is almost exactly a repeat from loader.c
 */
static const char          *GameObject_flags[NUM_FLAGS + 1 + 1] =
{
    "f_sleep", "f_confused", NULL, "f_scared", "f_is_blind", "f_is_invisible", "f_is_ethereal", "f_is_good",
    "f_no_pick", "f_walk_on", "f_no_pass",     /* 10 */
    "f_is_animated", NULL, "f_flying", "f_monster", "f_friendly", NULL /*is_removed*/, "f_been_applied", "f_auto_apply",
    "f_treasure", "f_is_neutral", /* 20 */
    "f_see_invisible", "f_can_roll", "f_generator", "f_is_turnable", "f_walk_off", "f_fly_on", "f_fly_off",
    "f_is_used_up", "f_identified", "f_reflecting",    /* 30 */
    "f_changing", "f_splitting", "f_hitback", "f_startequip", "f_blocksview", "f_undead", NULL, "f_unaggressive",
    "f_reflect_missile", "f_reflect_spell",             /* 40 */
    "f_no_magic", "f_no_fix_player", "f_is_evil", "f_tear_down", "f_run_away", "f_pass_thru", "f_can_pass_thru",
    "f_pick_up", "f_unique", "f_no_drop", /* 50 */
    "f_is_indestructible", "f_can_cast_spell", "f_can_use_scroll", "f_can_use_range", "f_can_use_bow",
    "f_can_use_armour", "f_can_use_weapon", "f_can_use_ring", "f_has_ready_range", "f_has_ready_bow",       /* 60 */
    "f_xrays", NULL, "f_is_floor", "f_lifesave", "f_is_magical", "f_alive", "f_stand_still", "f_random_move",
    "f_only_attack", "f_wiz", /* 70 */
    "f_stealth", NULL, NULL, "f_cursed", "f_damned", "f_see_anywhere", "f_known_magical", "f_known_cursed",
    "f_can_use_skill", "f_is_thrown",               /* 80 */
    "f_is_vul_sphere", "f_is_proof_sphere", "f_is_male", "f_is_female", "f_applied", "f_inv_locked", "f_is_wooded",
    "f_is_hilly", "f_has_ready_skill", "f_has_ready_weapon",        /* 90 */
    "f_no_skill_ident","f_use_dmg_info", "f_can_see_in_dark", "f_is_cauldron", "f_is_dust", "f_no_steal",
    "f_one_hit", NULL, "f_berserk", "f_no_attack",   /* 100 */
    "f_invulnerable", "f_quest_item", "f_is_traped", "f_is_vul_elemental", "f_is_proof_elemental", /* 105 */
    "f_is_vul_magic", "f_is_proof_magic", "f_is_vul_physical", "f_is_proof_physical", "f_sys_object", /* 110 */
    "f_use_fix_pos", "f_unpaid", "f_is_aged", "f_make_invisible", "f_make_ethereal", "f_is_player", "f_is_named",
    NULL /* spawn mob flag */, "f_no_teleport", "f_corpse", "f_corpse_forced", "f_player_only", "f_no_cleric",
    "f_one_drop", "f_cursed_perm", "f_damned_perm", "f_door_closed", "f_was_reflected", "f_is_missile",
    "f_can_reflect_missile", "f_can_reflect_spell", "f_is_assassin", NULL /* internal flag: HAS_MOVED */, "f_no_save",
    FLAGLIST_END_MARKER /* Marks the end of the list */
};

/****************************************************************************/
/*                          GameObject methods                         */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : GameObject_GetSkill                                              */
/* Lua    : object.GetSkill(type, id)                                        */
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
        if (tmp->type == SKILL && tmp->stats.sp == id)
            return push_object(L, &GameObject, tmp);

        if (tmp->type == EXPERIENCE && tmp->sub_type1 == id)
            return push_object(L, &GameObject, tmp);
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetSkill                                              */
/* Lua    : object.SetSkill(type, skillid, level, value)                     */
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
                /* Gecko: Changed to use actuall skill experience */
                currentxp = tmp->stats.exp;
                value = value - currentxp;

                GCFP.Value[0] = (void *) (WHO);
                GCFP.Value[1] = (void *) (&value);
                GCFP.Value[2] = (void *) (&skill);
                (PlugHooks[HOOK_ADDEXP]) (&GCFP);
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
/* Lua    : object.ActivateRune(what)                                        */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_ActivateRune(lua_State *L)
{
    lua_object *whatptr;
    lua_object *self;

    get_lua_args(L, "OO", &self, &whatptr);

    GCFP.Value[0] = (void *) (WHAT);
    GCFP.Value[1] = (void *) (WHO);
    (PlugHooks[HOOK_SPRINGTRAP]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_CheckTrigger                                          */
/* Lua    : object.CheckTrigger(what)                                        */
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
/* Lua    : object.GetGod()                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static int GameObject_GetGod(lua_State *L)
{
    CFParm         *CFR;
    static char    *value;
    lua_object     *self;
    get_lua_args(L, "O", &self);

    GCFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_DETERMINEGOD]) (&GCFP);
    value = (char *) (CFR->Value[0]);
    free(CFR);

    lua_pushstring(L, value);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SetGod                                                */
/* Lua    : object.SetGod(godname)                                           */
/* Status : Unfinished!                                                      */
/*****************************************************************************/
static int GameObject_SetGod(lua_State *L)
{
    char       *txt;
    const char *prayname = NULL;
    object     *tmp;
    CFParm     *CFR0;
    CFParm     *CFR;
    int         value;
    lua_object *self;

    get_lua_args(L, "Os", &self, &txt);

    FREE_AND_COPY_HASH(prayname, "praying");

    GCFP1.Value[0] = (void *) (WHO);
    GCFP1.Value[1] = (void *) (prayname);

    GCFP2.Value[0] = (void *) (WHO);
    GCFP0.Value[0] = (char *) (txt);
    CFR0 = (PlugHooks[HOOK_FINDGOD]) (&GCFP0);
    tmp = (object *) (CFR0->Value[0]);
    free(CFR0);
    GCFP2.Value[1] = (void *) (tmp);

    CFR = (PlugHooks[HOOK_CMDRSKILL]) (&GCFP1);
    value = *(int *) (CFR->Value[0]);
    if (value)
        (PlugHooks[HOOK_BECOMEFOLLOWER]) (&GCFP2);
    free(CFR);

    FREE_ONLY_HASH(prayname);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_TeleportTo                                            */
/* Lua    : object.TeleportTo(map, x, y, unique)                             */
/* Info   : Teleports object to the given position of map.                   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_TeleportTo(lua_State *L)
{
    char       *mapname;
    char       *msg = NULL;
    int         x, y, u = 0;
    lua_object *self;

    get_lua_args(L, "Osii|i", &self, &mapname, &x, &y, &u);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (char *) (mapname);
    GCFP.Value[2] = (void *) (&x);
    GCFP.Value[3] = (void *) (&y);
    GCFP.Value[4] = (void *) (&u);
    GCFP.Value[5] = (char *) (msg);
    (PlugHooks[HOOK_TELEPORTOBJECT]) (&GCFP);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_InsertInside                                          */
/* Lua    : object.InsertInside(where)                                       */
/* Info   : Inserts object into where.                                       */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_InsertInside(lua_State *L)
{
    lua_object *whereptr;
    object     *myob;
    object     *obenv;
    object     *tmp;
    lua_object *self;

    get_lua_args(L, "OO", &self, &whereptr);

    myob = self->data.object;
    obenv = myob->env;

    if (!QUERY_FLAG(myob, FLAG_REMOVED))
    {
        GCFP.Value[0] = (void *) (myob);
        (PlugHooks[HOOK_REMOVEOBJECT]) (&GCFP);
    }

    myob = hooks->insert_ob_in_ob(myob, WHERE);

    /* Make sure the inventory image/text is updated */
    /* FIXME: what if object was not carried by player ? */
    for (tmp = WHERE; tmp != NULL; tmp = tmp->env)
        if (tmp->type == PLAYER)
            hooks->esrv_send_item(tmp, myob);

    /* If we're taking from player. */
    for (tmp = obenv; tmp != NULL; tmp = tmp->env)
        if (tmp->type == PLAYER)
            hooks->esrv_send_inventory(tmp, tmp);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Apply                                                 */
/* Lua    : object.Apply(what, flags)                                        */
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
    CFParm     *CFR;
    int         retval;
    lua_object *self;

    get_lua_args(L, "OOi", &self, &whatptr, &flags);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (WHAT);
    GCFP.Value[2] = (void *) (&flags);
    CFR = (PlugHooks[HOOK_MANUALAPPLY]) (&GCFP);
    retval = *(int *) (CFR->Value[0]);
    free(CFR);

    lua_pushnumber(L, retval);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PickUp                                                */
/* Lua    : object.PickUp(what)                                              */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_PickUp(lua_State *L)
{
    lua_object *whatptr, *self;

    get_lua_args(L, "OO", &self, &whatptr);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (WHAT);
    (PlugHooks[HOOK_PICKUP]) (&GCFP);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_Drop                                                  */
/* Lua    : object.Drop(what)                                                */
/* Info   : Equivalent to the player command "drop" (name is an object name, */
/*          "all", "unpaid", "cursed", "unlocked" or a count + object name : */
/*          "<nnn> <object name>", or a base name, or a short name...)       */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_Drop(lua_State *L)
{
    char       *name;
    CFParm     *CFR;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (name);
    CFR = (PlugHooks[HOOK_CMDDROP]) (&GCFP);
    free(CFR);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Take                                                  */
/* Lua    : object.Take(name)                                                */
/* Status : Temporary disabled (see commands.c)                              */
/*****************************************************************************/

static int GameObject_Take(lua_State *L)
{
    char       *name;
    CFParm     *CFR;
    lua_object *self;

    get_lua_args(L, "Os", &self, &name);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (name);
    CFR = (PlugHooks[HOOK_CMDTAKE]) (&GCFP);
    free(CFR);

    return 0;
}


/*****************************************************************************/
/* Name   : GameObject_Deposit                                               */
/* Lua    : object.Deposit(deposit_object, string)                           */
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
    CFParm     *CFR;
    int         value;
    lua_object *self;

    get_lua_args(L, "OOs", &self, &obptr, &text);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (obptr->data.object);
    GCFP.Value[2] = (void *) (text);

    CFR = (PlugHooks[HOOK_DEPOSIT]) (&GCFP);

    value = *(int *) (CFR->Value[0]);
    lua_pushnumber(L, value);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Withdraw                                              */
/* Lua    : object.Withdraw(deposit_object, string)                          */
/* Info   : withdraw value or string money from object in deposit_object.    */
/*          Control first object has that amount of money, then remove it    */
/*          from object and add it in ->value of deposit_object.             */
/*          FIXME Needs updated documentation                                */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Withdraw(lua_State *L)
{
    lua_object *obptr, *self;
    char       *text;
    CFParm     *CFR;
    int         value;

    get_lua_args(L, "OOs", &self, &obptr, &text);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (obptr->data.object);
    GCFP.Value[2] = (void *) (text);

    CFR = (PlugHooks[HOOK_WITHDRAW]) (&GCFP);

    value = *(int *) (CFR->Value[0]);
    lua_pushnumber(L, value);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Communicate                                           */
/* Lua    : object.Communicate(message)                                      */
/* Info   : object says message to everybody on its map                      */
/*          but instead of CFSay it is parsed for other npc or magic mouth   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Communicate(lua_State *L)
{
    char       *message;
    lua_object *self;

    get_lua_args(L, "Os", &self, &message);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (message);

    (PlugHooks[HOOK_COMMUNICATE]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Say                                                   */
/* Lua    : object.Say(message, mode)                                        */
/* Info   : object says message to everybody on its map                      */
/*          FIXME needs documentation of mode                                */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Say(lua_State *L)
{
    char       *message;
    static char buf[HUGE_BUF];
    int         val, d = MAP_INFO_NORMAL, x, y, mode = 0;
    lua_object *self;

    get_lua_args(L, "Os|i", &self, &message, &mode);

    /* old dynamic buffer */
    /*buf = (char *)(malloc(sizeof(char)*(strlen(message)+strlen(STRING_OBJ_NAME(who))+20)));*/

    val = NDI_NAVY | NDI_UNIQUE;
    x = WHO->x;
    y = WHO->y;

    if (mode)
        GCFP.Value[5] = (void *) (message);
    else
    {
        snprintf(buf, sizeof(buf), "%s says: %s", STRING_OBJ_NAME(WHO), message);
        GCFP.Value[5] = (void *) (buf);
    }

    GCFP.Value[0] = (void *) (&val);
    GCFP.Value[1] = (void *) (WHO->map);
    GCFP.Value[2] = (void *) (&x);
    GCFP.Value[3] = (void *) (&y);
    GCFP.Value[4] = (void *) (&d);

    (PlugHooks[HOOK_NEWINFOMAP]) (&GCFP);

    /*free(buf);*/
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SayTo                                                 */
/* Lua    : object.SayTo(target, message, mode)                              */
/* Info   : NPC talks only to player but map get a "xx talks to" msg too.    */
/*          FIXME needs documentation of mode parameter                      */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SayTo(lua_State *L)
{
    lua_object *self;
    object     *target;
    lua_object *obptr2;
	int mode = 0;
    char *message;
    static char buf[HUGE_BUF];

    get_lua_args(L, "OOs|i", &self, &obptr2, &message, &mode);

    target = obptr2->data.object;

    if(mode)
        hooks->new_draw_info(NDI_NAVY|NDI_UNIQUE, 0, target, message);
    else /* thats default */
    {
		snprintf(buf, sizeof(buf), "%s talks to %s.", STRING_OBJ_NAME(WHO),STRING_OBJ_NAME(target));
        hooks->new_info_map_except(NDI_UNIQUE, WHO->map, WHO->x, WHO->y, MAP_INFO_NORMAL, WHO, target, buf);

		snprintf(buf, sizeof(buf), "%s says: %s", STRING_OBJ_NAME(WHO), message);
        hooks->new_draw_info(NDI_NAVY|NDI_UNIQUE, 0, target, buf);
	}

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Write                                                 */
/* Lua    : object.Write(message, color)                                     */
/* Info   : Writes a message to a specific player.                           */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Write(lua_State *L)
{
    int         zero    = 0;
    char       *message;
    int         color   = NDI_UNIQUE | NDI_ORANGE;
    lua_object *self;

    get_lua_args(L, "Os|i", &self, &message, &color);

    GCFP.Value[0] = (void *) (&color);
    GCFP.Value[1] = (void *) (&zero);
    GCFP.Value[2] = (void *) (WHO);
    GCFP.Value[3] = (void *) (message);

    (PlugHooks[HOOK_NEWDRAWINFO]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetGender                                             */
/* Lua    : object.SetGender(gender)                                         */
/* Info   : Changes the gender of object. gender_string should be one of     */
/*          Daimonin.NEUTER, Daimonin.MALE, Daimonin.GENDER_FEMALE or        */
/*          Daimonin.HERMAPHRODITE                                           */
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
/* Lua    : object.SetRank(rank_string)                                      */
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

    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && !strcmp(walk->name, "RANK_FORCE") && !strcmp(walk->arch->name, "rank_force"))
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
/* Lua    : object.SetAlignment(alignment_string)                            */
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

    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && !strcmp(walk->name, "ALIGNMENT_FORCE") && !strcmp(walk->arch->name, "alignment_force"))
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
/* Lua    : object.GetAlignmentForce()                                       */
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
        if (walk->name && !strcmp(walk->name, "ALIGNMENT_FORCE") && !strcmp(walk->arch->name, "alignment_force"))
            return push_object(L, &GameObject, walk);
    }
    LOG(llevDebug, "Lua Warning -> GetAlignmentForce: Object %s has no aligment_force!\n", STRING_OBJ_NAME(WHO));

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetGuildForce                                         */
/* Lua    : object.SetGuildForce(rank_string)                                */
/* Info   : Sets the current rank of object to rank_string. Returns          */
/*          the guild_force object that was modified.                        */
/* Status : Stable                                                           */
/* Warning: This set only the title. The guild tag is in <slaying>           */
/*          For test of a special guild, you must use GetGuild()             */
/*          For settings inside a guild script, you can use this function    */
/*          Because it returns the guild_force obj after setting the title   */
/*****************************************************************************/
static int GameObject_SetGuildForce(lua_State *L)
{
    object     *walk;
    char       *guild;
    lua_object *self;

    get_lua_args(L, "Os", &self, &guild);

    if (WHO->type != PLAYER)
        return 0;

    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && !strcmp(walk->name, "GUILD_FORCE") && !strcmp(walk->arch->name, "guild_force"))
        {
            /* we find the rank of the player, now change it to new one */
            if (guild && strcmp(guild, "")) {
                FREE_AND_COPY_HASH(walk->title, guild);
            } else
                FREE_ONLY_HASH(walk->title);

            CONTR(WHO)->socket.ext_title_flag = 1; /* demand update to client */
            return push_object(L, &GameObject, walk);
        }
    }
    LOG(llevDebug, "Lua Warning -> SetGuild: Object %s has no guild_force!\n", STRING_OBJ_NAME(WHO));
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetGuildForce                                         */
/* Lua    : object.GetGuildForce()                                           */
/* Info   : This gets the guild_force from a inventory (should be player?)   */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_GetGuildForce(lua_State *L)
{
    object     *walk;
    lua_object *self;

    get_lua_args(L, "O", &self);

    if (WHO->type != PLAYER)
        return 0;

    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->name && !strcmp(walk->name, "GUILD_FORCE") && !strcmp(walk->arch->name, "guild_force"))
            return push_object(L, &GameObject, walk);
    }

    LOG(llevDebug, "Lua Warning -> GetGuild: Object %s has no guild_force!\n", STRING_OBJ_NAME(WHO));
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Fix                                                   */
/* Lua    : object.Fix()                                                     */
/* Info   : Recalculates a player's or monster's stats depending on          */
/*          equipment, forces, skills etc.                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_Fix(lua_State *L)
{
    lua_object *self;
    get_lua_args(L, "O", &self);

    hooks->fix_player(WHO);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Kill                                                  */
/* Lua    : object.Kill(what, how)                                           */
/* Status : Untested                                                         */
/*****************************************************************************/
/* add hooks before use! */

static int GameObject_Kill(lua_State *L)
{
    lua_object *whatptr;
    int         ktype;
    int         k   = 1;
    CFParm     *CFR;
    lua_object *self;

    get_lua_args(L, "OOi", &self, &whatptr, &ktype);

    WHAT->speed = 0;
    WHAT->speed_left = 0.0;
    GCFP.Value[0] = (void *) (WHAT);
    (PlugHooks[HOOK_UPDATESPEED]) (&GCFP);
    /* update_ob_speed(WHAT); */

    if (QUERY_FLAG(WHAT, FLAG_REMOVED))
    {
        LOG(llevDebug, "Warning (from KillObject): Trying to remove removed object\n");
        luaL_error(L, "Trying to remove removed object");
    }
    else
    {
        WHAT->stats.hp = -1;
        GCFP.Value[0] = (void *) (WHAT);
        GCFP.Value[1] = (void *) (&k);
        GCFP.Value[2] = (void *) (WHO);
        GCFP.Value[3] = (void *) (&ktype);

        CFR = (PlugHooks[HOOK_KILLOBJECT]) (&GCFP);
        free(CFR);
        /*kill_object(killed,1,killer, type); */
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
/* Name   : GameObject_CastAbility                                           */
/* Lua    : object.CastAbility(target, spellno, mode, direction, option)     */
/* Info   : object casts the ability numbered spellno on target.             */
/*          mode = Daimonin.CAST_NORMAL or Daimonin.CAST_POTION              */
/*          direction is the direction to cast the ability in                */
/*          option is additional string option(s)                            */
/*          FIXME: only allows for directional abilities?                    */
/*          Abilities are can be cast in magic-blocking areas, and do not    */
/*          use magicattack.                                                 */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_CastAbility(lua_State *L)
{
    lua_object *target;
    int         spell;
    int         dir;
    int         mode;
    char       *op;
    CFParm     *CFR;
    int         parm        = 1;
    int         parm2;
    int         typeoffire  = FIRE_DIRECTIONAL;
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

    GCFP.Value[0] = (void *) (target->data.object);
    GCFP.Value[1] = (void *) (WHO);
    GCFP.Value[2] = (void *) (&dir);
    GCFP.Value[3] = (void *) (&spell);
    GCFP.Value[4] = (void *) (&parm);
    GCFP.Value[5] = (void *) (&parm2);
    GCFP.Value[6] = (void *) (op);
    GCFP.Value[7] = (void *) (&typeoffire);
    CFR = (PlugHooks[HOOK_CASTSPELL]) (&GCFP);

    free(CFR);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_CastSpell                                             */
/* Lua    : object.CastSpell(target, spell, mode, direction, option)         */
/* Info   : object casts the spell numbered spellno on target.               */
/*          mode = Daimonin.CAST_NORMAL or Daimonin.CAST_POTION              */
/*          direction is the direction to cast the spell in                  */
/*          option is additional string option(s)                            */
/*          NPCs can cast spells even in no-spell areas.                     */
/*          FIXME: only allows for directional spells                        */
/*          FIXME: is direction/position relative to target? (0 = self)      */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_CastSpell(lua_State *L)
{
    lua_object *target;
    int         spell;
    int         dir;
    int         mode;
    char       *op;
    CFParm     *CFR;
    int         parm        = 0;
    int         parm2;
    int         typeoffire  = FIRE_DIRECTIONAL;
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

    GCFP.Value[0] = (void *) (target->data.object);
    GCFP.Value[1] = (void *) (WHO);
    GCFP.Value[2] = (void *) (&dir);
    GCFP.Value[3] = (void *) (&spell);
    GCFP.Value[4] = (void *) (&parm);
    GCFP.Value[5] = (void *) (&parm2);
    GCFP.Value[6] = (void *) (op);
    GCFP.Value[7] = (void *) (&typeoffire);
    CFR = (PlugHooks[HOOK_CASTSPELL]) (&GCFP);
    free(CFR);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_DoKnowSpell                                           */
/* Lua    : object.DoKnowSpell(spell)                                        */
/* Info   : true if the spell is known by object, false if it isn't          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_DoKnowSpell(lua_State *L)
{
    int         spell;
    CFParm     *CFR;
    int         value;
    lua_object *self;

    get_lua_args(L, "Oi", &self, &spell);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&spell);
    CFR = (PlugHooks[HOOK_CHECKFORSPELL]) (&GCFP);
    value = *(int *) (CFR->Value[0]);
    free(CFR);

    lua_pushboolean(L, value);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AcquireSpell                                          */
/* Lua    : object.AcquireSpell(spell, mode)                                 */
/* Info   : object will learn or unlearn spell.                              */
/*          mode: Daimonin.LEARN or Daimonin.UNLEARN                         */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_AcquireSpell(lua_State *L)
{
    int         spell;
    int         mode;
    lua_object *self;

    get_lua_args(L, "Oii", &self, &spell, &mode);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&spell);
    GCFP.Value[2] = (void *) (&mode);
    (PlugHooks[HOOK_LEARNSPELL]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_FindSkill                                             */
/* Lua    : object.FindSkill(skill)                                          */
/* Info   : skill ptr if the skill is known by object, NULL if it isn't      */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_FindSkill(lua_State *L)
{
    int         skill;
    CFParm     *CFR;
    object     *myob;
    lua_object *self;

    get_lua_args(L, "Oi", &self, &skill);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&skill);
    CFR = (PlugHooks[HOOK_FINDSKILL]) (&GCFP);

    myob = (object *) (CFR->Value[0]);
    return push_object(L, &GameObject, myob);


}

/*****************************************************************************/
/* Name   : GameObject_AcquireSkill                                          */
/* Lua    : object.AcquireSkill(skillno, mode)                               */
/* Info   : object will learn or unlearn skill.                              */
/*          mode: Daimonin.LEARN or Daimonin.UNLEARN                         */
/*          Get skill number with Daimonin.GetSkillNr()                      */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_AcquireSkill(lua_State *L)
{
    int         skill, mode;
    lua_object *self;

    get_lua_args(L, "Oii", &self, &skill, &mode);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&skill);
    GCFP.Value[2] = (void *) (&mode);
    (PlugHooks[HOOK_LEARNSKILL]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_FindMarkedObject                                      */
/* Lua    : object.FindMarkedObject()                                        */
/* Info   : Returns the marked object in object's inventory, or None if no   */
/*          object is marked.                                                */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_FindMarkedObject(lua_State *L)
{
    object     *value;
    CFParm     *CFR;
    lua_object *self;

    get_lua_args(L, "O", &self);

    GCFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_FINDMARKEDOBJECT]) (&GCFP);

    value = (object *) (CFR->Value[0]);
    /*free(CFR); findmarkedobject use static parameters */
    return push_object(L, &GameObject, value);
}

/*****************************************************************************/
/* Name   : GameObject_CheckInvisibleInside                                  */
/* Lua    : object.CheckInvisibleInside(id)                                  */
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
/* Lua    : object.CreatePlayerForce(force_name, time)                       */
/* Info   : Creates and insters a player force named force_name in object.   */
/*          The values of a player force will effect the player.             */
/*          If time is given and > 0, the force will be removed again after  */
/*          time/0.02 ticks.                                                 */
/* Status : Stable.                                                          */
/*****************************************************************************/
static int GameObject_CreatePlayerForce(lua_State *L)
{
    char       *txt;
    char        txt2[16]    = "player_force";
    object     *myob;
    CFParm     *CFR;
    int         time        = 0;
    lua_object *whereptr;

    get_lua_args(L, "Os|i", &whereptr, &txt, &time);

    GCFP.Value[0] = (void *) (txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE]) (&GCFP);
    /*myob = get_archetype("player_force"); */
    myob = (object *) (CFR->Value[0]);
    free(CFR);

    if (!myob)
    {
        LOG(llevDebug, "Lua WARNING:: CreatePlayerForce: Can't find archtype 'player_force'\n");
        luaL_error(L, "Can't find archtype 'player_force'");
    }

    /* For temporary forces */
    if (time > 0)
    {
        SET_FLAG(myob, FLAG_IS_USED_UP);
        myob->stats.food = time;
        myob->speed = 0.02f;
        GCFP.Value[0] = (void *) (myob);
        (PlugHooks[HOOK_UPDATESPEED]) (&GCFP);
    }

    /* setup the force and put it in activator */
    FREE_AND_COPY_HASH(myob->name, txt);
    myob = hooks->insert_ob_in_ob(myob, WHERE);

    hooks->esrv_send_item(WHERE, myob);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_CheckQuestObject                                      */
/* Lua    : object.CheckQuestObject(archetype, name)                         */
/* Status : Stable                                                           */
/* Info   : We get and check the player has a misc'ed quest object           */
/*        : If so, the player has usally solved this quest before.           */
/*****************************************************************************/
static int GameObject_CheckQuestObject(lua_State *L)
{
    char       *arch_name;
    char       *name;
    object     *walk;
    lua_object *self;

    get_lua_args(L, "Oss", &self, &arch_name, &name);

    /* lets first check the inventory for the quest_container object */
    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->type == TYPE_QUEST_CONTAINER)
        {
            /* now lets check our quest item is inside.
                     * we use arch name and object name as id, if needed we
                     * arch name is stored in the race field of the quest dummies.
                     * can add more here
                     */
            for (walk = walk->inv; walk != NULL; walk = walk->below)
            {
                if (walk->race && !strcmp(walk->race, arch_name) && walk->name && !strcmp(walk->name, name))
                    return push_object(L, &GameObject, walk);
            }
            break;
        }
    }

    return 0; /* there was non */
}

/*****************************************************************************/
/* Name   : GameObject_AddQuestObject                                        */
/* Lua    : object.AddQuestObject(archetype, name)                           */
/* Status : Stable                                                           */
/* Info   : Add the misc'ed quest object to players quest container.         */
/*        : create the quest container if needed                             */
/*****************************************************************************/
static int GameObject_AddQuestObject(lua_State *L)
{
    char       *arch_name;
    char       *name;
    object     *walk, *myob;
    CFParm     *CFR;
    char        txt2[32];
    lua_object *self;

    get_lua_args(L, "Oss", &self, &arch_name, &name);

    /* lets first check the inventory for the quest_container object */
    for (walk = WHO->inv; walk != NULL; walk = walk->below)
    {
        if (walk->type == TYPE_QUEST_CONTAINER)
            break;
    }

    if (!walk) /* no quest container, create it */
    {
        strcpy(txt2, "quest_container");

        GCFP.Value[0] = (void *) (txt2);
        CFR = (PlugHooks[HOOK_GETARCHETYPE]) (&GCFP);
        walk = (object *) (CFR->Value[0]);
        free(CFR);

        if (!walk)
        {
            LOG(llevDebug, "Lua WARNING:: AddQuestObject: Cant't find archtype 'quest_container'\n");
            luaL_error(L, "Can't find archtype 'quest_container'");
        }

        hooks->insert_ob_in_ob(walk, WHO);
    }

    strcpy(txt2, "player_info");

    GCFP.Value[0] = (void *) (txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE]) (&GCFP);
    myob = (object *) (CFR->Value[0]);
    free(CFR);

    if (!myob)
    {
        LOG(llevDebug, "Lua WARNING:: AddQuestObject: Cant't find archtype 'player_info'\n");
        luaL_error(L, "Can't find archtype 'player_info'");
    }

    /* store name & arch name of the quest obj. so we can id it later */
    FREE_AND_COPY_HASH(myob->name, name);
    FREE_AND_COPY_HASH(myob->race, arch_name);

    myob = hooks->insert_ob_in_ob(myob, walk);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_CreatePlayerInfo                                      */
/* Lua    : object.CreatePlayerInfo(name)                                    */
/* Status : Stable                                                           */
/* Info   : Creates a player_info object of specified name in object's       */
/*          inventory                                                        */
/*          The Values of a player_info object will NOT effect the player.   */
/*          Returns the created object                                       */
/*****************************************************************************/
static int GameObject_CreatePlayerInfo(lua_State *L)
{
    char       *txt;
    char        txt2[16]    = "player_info";
    object     *myob;
    CFParm     *CFR;
    lua_object *whereptr;

    get_lua_args(L, "Os", &whereptr, &txt);

    GCFP.Value[0] = (void *) (txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE]) (&GCFP);
    myob = (object *) (CFR->Value[0]);
    free(CFR);

    if (!myob)
    {
        LOG(llevDebug, "Lua WARNING:: CreatePlayerInfo: Cant't find archtype 'player_info'\n");
        luaL_error(L, "Cant't find archtype 'player_info'");
    }

    /* setup the info and put it in activator */
    FREE_AND_COPY_HASH(myob->name, txt);
    myob = hooks->insert_ob_in_ob(myob, WHERE);

    hooks->esrv_send_item(WHERE, myob);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_GetPlayerInfo                                         */
/* Lua    : object.GetPlayerInfo(name)                                       */
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
        if (walk->name && !strcmp(walk->arch->name, "player_info") && !strcmp(walk->name, name))
            return push_object(L, &GameObject, walk);
    }

    return 0; /* there was non */
}


/*****************************************************************************/
/* Name   : GameObject_GetNextPlayerInfo                                     */
/* Lua    : object.GetNextPlayerInfo(player_info)                            */
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
    name[63] = '\0';

    /* get the next linked player_info arch in this inventory */
    for (walk = myob->data.object->below; walk != NULL; walk = walk->below)
    {
        if (walk->name && !strcmp(walk->arch->name, "player_info") && !strcmp(walk->name, name))
            return push_object(L, &GameObject, walk);
    }

    return 0; /* there was non left */
}


/*****************************************************************************/
/* Name   : GameObject_CreateInvisibleInside                                 */
/* Lua    : object.CreateInvisibleInside(id)                                 */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_CreateInvisibleInside(lua_State *L)
{
    char       *txt;
    char        txt2[6] = "force";
    object     *myob;
    CFParm     *CFR;
    lua_object *whereptr;

    get_lua_args(L, "Os", &whereptr, &txt);

    GCFP.Value[0] = (void *) (txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE]) (&GCFP);

    /*myob = get_archetype("force"); */
    myob = (object *) (CFR->Value[0]);
    free(CFR);

    if (!myob)
    {
        LOG(llevDebug, "Lua WARNING:: CFCreateInvisibleInside: Can't find archtype 'force'\n");
        luaL_error(L, "Cant't find archtype 'force'");
    }
    myob->speed = 0.0;
    GCFP.Value[0] = (void *) (myob);
    (PlugHooks[HOOK_UPDATESPEED]) (&GCFP);

    /*update_ob_speed(myob); */
    FREE_AND_COPY_HASH(myob->slaying, txt);
    myob = hooks->insert_ob_in_ob(myob, WHERE);

    hooks->esrv_send_item(WHERE, myob);

    return push_object(L, &GameObject, myob);
}

/*****************************************************************************/
/* Name   : GameObject_CreateObjectInside                                    */
/* Lua    : object.CreateObjectInside(archname, identified, number, value)   */
/* Info   : Creates an object from archname and inserts into object.         */
/*          identified is either game.IDENTIFIED or game.UNIDENTIFIED        */
/*          If value is >= 0 it will be used as the new object's value,      */
/*          otherwise the value will be taken from the arch.                 */
/* Status : Stable                                                           */
/*****************************************************************************/
/* i must change this a bit - only REAL arch names - not object names */

static int GameObject_CreateObjectInside(lua_State *L)
{
    object     *myob, *tmp;
    int         value = -1, id, nrof = 1;
    char       *txt;
    /*    char *tmpname;
        object *test;
        int i;*/
    CFParm     *CFR;
    lua_object *whereptr;

    /* 0: name
       1: object we want give <name>
       2: if 1, set FLAG_IDENTIFIED
       3: nr of objects to create: 0 and 1 don't change default nrof setting
       3: if not -1, use it for myob->value
       */

    get_lua_args(L, "Osii|i", &whereptr, &txt, &id, &nrof, &value);

    GCFP.Value[0] = (void *) (txt);
    CFR = (PlugHooks[HOOK_GETARCHETYPE]) (&GCFP);
    myob = (object *) (CFR->Value[0]);
    free(CFR);

    if (!myob)
    {
        LOG(llevDebug, "BUG GameObject_CreateObjectInside(): ob:>%s< = NULL!\n", STRING_OBJ_NAME(myob));
        luaL_error(L, "Failed to create the object. Did you use an existing arch?");
    }

    if (value != -1) /* -1 means, we use original value */
        myob->value = (sint32) value;
    if (id)
    {
        SET_FLAG(myob, FLAG_IDENTIFIED);
        SET_FLAG(myob, FLAG_KNOWN_MAGICAL);
        SET_FLAG(myob, FLAG_KNOWN_CURSED);
    }
    if (nrof > 1)
        myob->nrof = nrof;

    myob = hooks->insert_ob_in_ob(myob, WHERE);

    /* Make sure inventory image/text is updated */
    for (tmp = WHERE; tmp != NULL; tmp = tmp->env)
    {
        if (tmp->type == PLAYER)
            hooks->esrv_send_item(tmp, myob);
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
/* Lua    : object.CheckInventory(mode, arch, name, title, type)             */
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
    char*name =     NULL, *title = NULL, *arch_name = NULL;
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
/* Lua    : object.SetSaveBed(map, x, y)                                     */
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
        strcpy(CONTR(WHO)->savebed_map, map->data.map->path);
        CONTR(WHO)->bed_x = x;
        CONTR(WHO)->bed_y = y;
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Remove                                                */
/* Lua    : object.Remove()                                                  */
/* Info   : Takes the object out of whatever map or inventory it is in. The  */
/*          object can then be inserted or teleported somewhere else, or just*/
/*          left alone for the garbage collection to take care of.           */
/* Status : Needs more testing                                               */
/*****************************************************************************/
static int GameObject_Remove(lua_State *L)
{
    lua_object *self;
    object     *myob;
    object     *obenv, *tmp;

    get_lua_args(L, "O", &self);

    myob = WHO;
    obenv = myob->env;

    /* TODO: maybe this is no longer necessary? */
    /* Gecko: Don't allow removing any of the involved objects. Messes things up... */
    /*    if (StackActivator[StackPosition] == myob ||
                StackWho[StackPosition] == myob ||
                StackOther[StackPosition] == myob)
        {
            luaL_error(L, "You are not allowed to remove one of the active objects. Workaround using CFTeleport or some other solution.");
        }*/

    GCFP.Value[0] = (void *) (myob);
    (PlugHooks[HOOK_REMOVEOBJECT]) (&GCFP);

    /* Update player's inventory if object was removed from player
     * TODO: see how well this works with things in containers */
    for (tmp = obenv; tmp != NULL; tmp = tmp->env)
        if (tmp->type == PLAYER)
            hooks->esrv_send_inventory(tmp, tmp);

    /* TODO: maybe this is no longer necessary? */
    /* Gecko: Handle removing any of the active objects (e.g. the activator) */
    /*    if (StackActivator[StackPosition] == myob)
            StackActivator[StackPosition] = NULL;
        if (StackWho[StackPosition] == myob)
            StackWho[StackPosition] = NULL;
        if (StackOther[StackPosition] == myob)
            StackOther[StackPosition] = NULL;*/

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Destruct                                              */
/* Lua    : object.Destruct()                                                */
/* Info   : Removes the object out of whatever map or inventory it is in and */
/*          drops all items in object's inventory on the floor or in a       */
/*          corpse                                                           */
/* Status : Needs more testing                                               */
/*****************************************************************************/
static int GameObject_Destruct(lua_State *L)
{
    lua_object *self;
    object     *myob;
    object     *obenv, *tmp;

    get_lua_args(L, "O", &self);

    myob = WHO;
    obenv = myob->env;

    /* TODO: maybe this is no longer necessary? */
    /* Gecko: Don't allow removing any of the involved objects. Messes things up... */
    /*    if (StackActivator[StackPosition] == myob ||
                StackWho[StackPosition] == myob ||
                StackOther[StackPosition] == myob)
        {
            luaL_error(L, "You are not allowed to remove one of the active objects. Workaround using CFTeleport or some other solution.");
        }*/

    GCFP.Value[0] = (void *) (myob);
    (PlugHooks[HOOK_DESTRUCTOBJECT]) (&GCFP);

    /* Update player's inventory if object was removed from player
     * TODO: see how well this works with things in containers */
    for (tmp = obenv; tmp != NULL; tmp = tmp->env)
        if (tmp->type == PLAYER)
            hooks->esrv_send_inventory(tmp, tmp);

    /* TODO: maybe this is no longer necessary? */
    /* Gecko: Handle removing any of the active objects (e.g. the activator) */
    /*    if (StackActivator[StackPosition] == myob)
            StackActivator[StackPosition] = NULL;
        if (StackWho[StackPosition] == myob)
            StackWho[StackPosition] = NULL;
        if (StackOther[StackPosition] == myob)
            StackOther[StackPosition] = NULL;*/

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetPosition                                           */
/* Lua    : object.SetPosition(x, y)                                         */
/* Info   : Cannot be used to move objects out of containers. (Use Drop() or */
/*          TeleportTo() for that)                                           */
/* Status : Tested                                                           */
/*****************************************************************************/

/* TODO: Useful for setting X/Y in non-active objects too? */

/* FIXME: if the object moved was triggered by SAY event and it is moved to a tile
 * within the listening radius, it will be triggered again, and again... */

static int GameObject_SetPosition(lua_State *L)
{
    lua_object *self;
    int         x, y, k;
    CFParm     *CFR;
    k = 0;

    get_lua_args(L, "Oii", &self, &x, &y);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&x);
    GCFP.Value[2] = (void *) (&y);
    GCFP.Value[3] = (void *) (&k);
    GCFP.Value[4] = (void *) (NULL);
    GCFP.Value[5] = (void *) (NULL);

    CFR = (PlugHooks[HOOK_TRANSFEROBJECT]) (&GCFP);
    free(CFR);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_IdentifyItem                                          */
/* Lua    : object.IdentifyItem(target, marked, mode)                        */
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

    GCFP.Value[0] = (void *) WHO;
    GCFP.Value[1] = (void *) target->data.object;
    GCFP.Value[2] = (void *) marked; /* is used when we use mode == 2 */
    GCFP.Value[3] = (void *) &mode;
    (PlugHooks[HOOK_IDENTIFYOBJECT]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_IsOfType                                              */
/* Lua    : object.IsOfType(type)                                            */
/* Info   : returns 1 if object is of the specified type, or 0 otherwise.    */
/*          (e.g. game.TYPE_MONSTER for monster/NPC, or game.TYPE_PLAYER for */
/*          players)                                                         */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_IsOfType(lua_State *L)
{
    lua_object *self;
    int         type;

    get_lua_args(L, "Oi", &self, &type);

    lua_pushboolean(L, WHO->type == type);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_Save                                                  */
/* Lua    : object.Save()                                                    */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_Save(lua_State *L)
{
    lua_object     *self;
    static char    *result;
    CFParm         *CFR;

    get_lua_args(L, "O", &self);

    GCFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_DUMPOBJECT]) (&GCFP);
    result = (char *) (CFR->Value[0]);
    free(CFR);

    lua_pushstring(L, result);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetIP                                                 */
/* Lua    : object.GetIP()                                                   */
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
        result = CONTR(WHO)->socket.host;
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
/* Lua    : object.GetArchName()                                             */
/* Info   :                                                                  */
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
/* Lua    : object.ShowCost(value)                                           */
/* Info   : Returns a string describing value as x gold, x silver, x copper  */
/*        : cost string comes from shop.c and is temporary STATIC            */
/*        : note: whoptr is not used - perhaps we use this in future with it */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_ShowCost(lua_State *L)
{
    lua_object *self;
    int         value;
    char       *cost_string;

    CFParm     *CFR;
    get_lua_args(L, "Oi", &self, &value);

    GCFP.Value[0] = (void *) (&value);
    CFR = (PlugHooks[HOOK_SHOWCOST]) (&GCFP);
    cost_string = (char *) (CFR->Value[0]);

    lua_pushstring(L, cost_string);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetItemCost                                           */
/* Lua    : object.GetItemCost(object,type)                                  */
/* Info   : type is one of game.COST_TRUE, game.COST_BUY or game.COST_SELL   */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_GetItemCost(lua_State *L)
{
    lua_object *self;
    lua_object *whatptr;
    int         flag;
    int         cost;
    CFParm     *CFR;

    get_lua_args(L, "OOi", &self, &whatptr, &flag);

    GCFP.Value[0] = (void *) (WHAT);
    GCFP.Value[1] = (void *) (WHO);
    GCFP.Value[2] = (void *) (&flag);
    CFR = (PlugHooks[HOOK_QUERYCOST]) (&GCFP);
    cost = *(int *) (CFR->Value[0]);
    free(CFR);

    lua_pushnumber(L, cost);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetMoney                                              */
/* Lua    : object.GetMoney()                                                */
/* Info   : returns the amount of money the object carries in copper         */
/* Status : Tested                                                           */
/*****************************************************************************/

static int GameObject_GetMoney(lua_State *L)
{
    lua_object *self;
    int         amount;
    CFParm     *CFR;

    get_lua_args(L, "O", &self);

    GCFP.Value[0] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_QUERYMONEY]) (&GCFP);
    amount = *(int *) (CFR->Value[0]);
    free(CFR);

    lua_pushnumber(L, amount);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PayForItem                                            */
/* Lua    : object.PayForItem(object)                                        */
/* Status : Untested                                                         */
/*****************************************************************************/

static int GameObject_PayForItem(lua_State *L)
{
    lua_object *self;
    lua_object *whatptr;
    int         val;
    CFParm     *CFR;
    get_lua_args(L, "OO", &self, &whatptr);

    GCFP.Value[0] = (void *) (WHAT);
    GCFP.Value[1] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_PAYFORITEM]) (&GCFP);
    val = *(int *) (CFR->Value[0]);
    free(CFR);

    lua_pushnumber(L, val);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_PayAmount                                             */
/* Lua    : object.PayAmount(value)                                          */
/* Info   : If object has enough money, value copper will be deducted from   */
/*          object, and 1 will be returned. Otherwise returns 0              */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_PayAmount(lua_State *L)
{
    lua_object *self;
    int         to_pay;
    int         val;
    CFParm     *CFR;
    get_lua_args(L, "Oi", &self, &to_pay);

    GCFP.Value[0] = (void *) (&to_pay);
    GCFP.Value[1] = (void *) (WHO);
    CFR = (PlugHooks[HOOK_PAYFORAMOUNT]) (&GCFP);
    val = *(int *) (CFR->Value[0]);
    free(CFR);
    lua_pushnumber(L, val);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_SendCustomCommand                                     */
/* Lua    : object.SendCustomCommand(customcommand)                          */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_SendCustomCommand(lua_State *L)
{
    lua_object *self;
    char       *customcmd;

    get_lua_args(L, "Os", &self, &customcmd);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (customcmd);
    (PlugHooks[HOOK_SENDCUSTOMCOMMAND]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Clone                                                 */
/* Lua    : object.Clone(mode)                                               */
/* Info   : mode = game.CLONE_WITH_INVENTORY (default) or                    */
/*          game.CLONE_WITHOUT_INVENTORY                                     */
/*          You should do something with the clone. TeleportTo() or          */
/*          InsertInside() are useful functions for this.                    */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Clone(lua_State *L)
{
    lua_object *self;
    CFParm     *CFR;
    int         mode    = 0;
    object     *clone;

    get_lua_args(L, "O|i", &self, &mode);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&mode);

    CFR = (PlugHooks[HOOK_CLONEOBJECT]) (&GCFP);

    clone = (object *) (CFR->Value[0]);
    free(CFR);

    if (clone->type == PLAYER || QUERY_FLAG(clone, FLAG_IS_PLAYER))
    {
        clone->type = MONSTER;
        CLEAR_FLAG(clone, FLAG_IS_PLAYER);
    }

    return push_object(L, &GameObject, clone);
}

/* FUNCTIONEND -- End of the GameObject methods. */

#if 0
/*****************************************************************************/
/* Name   : GameObject_GetUnmodifiedAttribute                           */
/* Lua    : object.GetUnmodifiedAttribute(attribute_id)                      */
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

/****************************************************************************/
/* Lua object management code                                               */
/****************************************************************************/

/* This gets called before and after an attribute has been set in an object */
static int GameObject_setAttribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib, int before)
{
    object *tmp;
    object *who = obj->data.object;

    /* Pre-setting hook */
    if (before)
    {
        if (who->type == PLAYER && attrib->flags & FIELDFLAG_PLAYER_READONLY)
            luaL_error(L, "attribute %s is readonly on players", attrib->name);
        return 0;
    }

    /* Make sure player's inventory image/text is updated */
    /* FIXME: what if object was not carried by player ? */
    for (tmp = who->env; tmp != NULL; tmp = tmp->env)
        if (tmp->type == PLAYER)
            hooks->esrv_send_item(tmp, who);

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
            hooks->fix_player(who);
    }

    return 0;
}

/* value is on top of stack */
static int GameObject_setFlag(lua_State *L, lua_object *obj, uint32 flagno)
{
    int     value;
    object *tmp;

    if (lua_isnumber(L, -1))
        value = (int) lua_tonumber(L, -1);
    else
        value = lua_toboolean(L, -1);

    if (value)
        SET_FLAG(obj->data.object, flagno);
    else
        CLEAR_FLAG(obj->data.object, flagno);

    /* Make sure the inventory image/text is updated */
    /* FIXME: what if object was not carried by player ? */
    for (tmp = obj->data.object->env; tmp != NULL; tmp = tmp->env)
        if (tmp->type == PLAYER)
            hooks->esrv_send_item(tmp, obj->data.object);

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
        lua_pushfstring(L, "[%s \"%s\"] ", STRING_OBJ_ARCH_NAME(obj->data.object), STRING_OBJ_NAME(obj->data.object));
    else
        luaL_error(L, "Not an object");

    return 1;
}

lua_class   GameObject  =
{
    LUATYPE_OBJECT, "GameObject", 0, GameObject_toString, GameObject_attributes, GameObject_methods, NULL,
    GameObject_flags, GameObject_getFlag, GameObject_setFlag, GameObject_setAttribute
};

int GameObject_init(lua_State *L)
{
    init_class(L, &GameObject);

    return 0;
}

/******************
 * Old, leftover stuff. Will clean up later...
 */

#if 0
/* TODO: Create a FindAnimation() thingie */

/*****************************************************************************/
/* Name   : GameObject_SetFace                                          */
/* Lua    :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/

static int GameObject_SetFace(lua_State *L)
{
    char* txt;
    lua_object *whoptr;
    CFParm* CFR;
    int val = UP_OBJ_FACE;

    get_lua_args(L, "OOs", &self, &whoptr,&txt))
        return NULL;

    /*WHO->animation_id = find_animation(txt); */
    /*update_object(WHO,UP_OBJ_FACE); */
    GCFP.Value[0] = (void *)(txt);
    CFR = (PlugHooks[HOOK_FINDANIMATION])(&GCFP);
    WHO->animation_id = *(int *)(CFR->Value[0]);
    free(CFR);

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&val);
    (PlugHooks[HOOK_UPDATEOBJECT])(&GCFP);

    Py_INCREF(Py_None);
    return 0;
}
#endif

#if 0
/* Those replace the old get-script... and set-script... system */
/*****************************************************************************/
/* Name   : GameObject_GetEventHandler                                  */
/* Lua    :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
static int GameObject_GetEventHandler(lua_State *L)
{
    int eventnr;

    get_lua_args(L, "OOi", &self, &whoptr,&eventnr))
        return NULL;
    return Py_BuildValue("s","" /*WHO->event_hook[eventnr]*/);
}

/*****************************************************************************/
/* Name   : GameObject_SetEventHandler                                  */
/* Lua    :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static int GameObject_SetEventHandler(lua_State *L)
{
    lua_object *whoptr;
    int eventnr;
    char* scriptname;

    get_lua_args(L, "OOis", &whoptr, &eventnr, &scriptname))
        return NULL;

    /*WHO->event_hook[eventnr] = add_string_hook(scriptname);*/
    Py_INCREF(Py_None);
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetEventPlugin                                   */
/* Lua    :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static int GameObject_GetEventPlugin(lua_State *L)
{
    lua_object *whoptr;
    int eventnr;

    get_lua_args(L, "OOi", , &whoptr, &eventnr))
        return NULL;
    return Py_BuildValue("s", ""/*WHO->event_plugin[eventnr]*/);
}

/*****************************************************************************/
/* Name   : GameObject_SetEventPlugin                                   */
/* Lua    :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static int GameObject_SetEventPlugin(lua_State *L)
{
    lua_object *whoptr;
    int eventnr;
    char* scriptname;

    get_lua_args(L, "OOis", &whoptr,&eventnr,&scriptname))
        return NULL;

    /*WHO->event_plugin[eventnr] = add_string_hook(scriptname);*/
    Py_INCREF(Py_None);
    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_GetEventOptions                                  */
/* Lua    :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static int GameObject_GetEventOptions(lua_State *L)
{
    lua_object *whoptr;
    int eventnr;
    /*static char estr[4];*/
    get_lua_args(L, "OO!i", &GameObjectType, &whoptr,&eventnr))
        return NULL;
    /*
    if (WHO->event_options[eventnr] == NULL)
    {
        strcpy(estr,"");
        return Py_BuildValue("s", estr);
    }
    */
    return Py_BuildValue("s",""/* WHO->event_options[eventnr]*/);
}

/*****************************************************************************/
/* Name   : GameObject_SetEventOptions                                  */
/* Lua    :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static int GameObject_SetEventOptions(lua_State *L)
{
    lua_object *whoptr;
    int eventnr;
    char* scriptname;

    get_lua_args(L, "OO!is", &GameObjectType, &whoptr,&eventnr,&scriptname))
        return NULL;

    /*    WHO->event_options[eventnr] = add_string_hook(scriptname);*/

    Py_INCREF(Py_None);
    return 0;
}
#endif

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
