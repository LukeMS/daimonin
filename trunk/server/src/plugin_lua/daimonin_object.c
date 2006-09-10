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

/* Global data */

/* Available python methods for the GameObject object */
static struct method_decl   GameObject_methods[]            =
{
	{"CreateArtifact", (lua_CFunction) GameObject_CreateArtifact},
	{"GetName", (lua_CFunction) GameObject_GetName},
    {"GetEquipment", (lua_CFunction) GameObject_GetEquipment},
    {"GetRepairCost", (lua_CFunction) GameObject_GetRepairCost},
    {"Repair", (lua_CFunction) GameObject_Repair},
    {"Sound",  (lua_CFunction) GameObject_Sound},
    {"Interface",  (lua_CFunction) GameObject_Interface},
    {"SetSaveBed",  (lua_CFunction) GameObject_SetSaveBed},
    {"DecreaseNrOf",  (lua_CFunction) GameObject_DecreaseNrOf},
    {"GetSkill",  (lua_CFunction) GameObject_GetSkill},
    {"SetSkill",  (lua_CFunction) GameObject_SetSkill}, 
    {"ActivateRune",  (lua_CFunction) GameObject_ActivateRune},
    {"InsertInside",  (lua_CFunction) GameObject_InsertInside}, 
    {"GetGod",  (lua_CFunction) GameObject_GetGod},
    {"SetGod",  (lua_CFunction) GameObject_SetGod}, 
    {"TeleportTo",  (lua_CFunction) GameObject_TeleportTo},
    {"Apply",  (lua_CFunction) GameObject_Apply}, 
    {"PickUp",  (lua_CFunction) GameObject_PickUp},
    {"Drop",  (lua_CFunction) GameObject_Drop}, 
    {"Take",  (lua_CFunction) GameObject_Take},
    {"Fix", (lua_CFunction) GameObject_Fix}, 
    {"Kill", (lua_CFunction) GameObject_Kill},
    {"CastSpell", (lua_CFunction) GameObject_CastSpell},
    {"DoKnowSpell", (lua_CFunction) GameObject_DoKnowSpell},
    {"AcquireSpell", (lua_CFunction) GameObject_AcquireSpell},
    {"FindSkill", (lua_CFunction) GameObject_FindSkill},
    {"AcquireSkill", (lua_CFunction) GameObject_AcquireSkill},
    {"FindMarkedObject", (lua_CFunction) GameObject_FindMarkedObject},
	{"AddQuest", (lua_CFunction) GameObject_AddQuest},
	{"GetQuest", (lua_CFunction) GameObject_GetQuest},
	{"CheckQuestLevel", (lua_CFunction) GameObject_CheckQuestLevel},
 	{"AddQuestTarget", (lua_CFunction) GameObject_AddQuestTarget},
	{"AddQuestItem", (lua_CFunction) GameObject_AddQuestItem},
	{"NrofQuestItem", (lua_CFunction) GameObject_NrofQuestItem}, 
	{"RemoveQuestItem", (lua_CFunction) GameObject_RemoveQuestItem}, 
    {"SetQuestStatus", (lua_CFunction) GameObject_SetQuestStatus},
    {"CheckOneDropQuest", (lua_CFunction) GameObject_CheckOneDropQuest},
    {"AddOneDropQuest", (lua_CFunction) GameObject_AddOneDropQuest},
    {"CreatePlayerForce", (lua_CFunction) GameObject_CreatePlayerForce},
    {"CreatePlayerInfo", (lua_CFunction) GameObject_CreatePlayerInfo},
    {"GetPlayerInfo", (lua_CFunction) GameObject_GetPlayerInfo},
    {"GetNextPlayerInfo", (lua_CFunction) GameObject_GetNextPlayerInfo},
    {"CheckInvisibleObjectInside", (lua_CFunction) GameObject_CheckInvisibleInside},
    {"CreateInvisibleObjectInside", (lua_CFunction) GameObject_CreateInvisibleInside},
	{"CreateObjectInside", (lua_CFunction) GameObject_CreateObjectInside},
	{"CreateObjectInsideEx", (lua_CFunction) GameObject_CreateObjectInsideEx},
    {"CheckInventory", (lua_CFunction) GameObject_CheckInventory}, 
    {"Remove", (lua_CFunction) GameObject_Remove},
    {"Destruct", (lua_CFunction) GameObject_Destruct}, 
    {"SetPosition", (lua_CFunction) GameObject_SetPosition},
    {"IdentifyItem", (lua_CFunction) GameObject_IdentifyItem},
    {"Deposit",  (lua_CFunction) GameObject_Deposit}, 
    {"Withdraw",  (lua_CFunction) GameObject_Withdraw},
    {"Communicate",  (lua_CFunction) GameObject_Communicate}, 
    {"Say",  (lua_CFunction) GameObject_Say},
    {"SayTo",  (lua_CFunction) GameObject_SayTo}, 
    {"Write", (lua_CFunction) GameObject_Write},
    {"SetGender",  (lua_CFunction) GameObject_SetGender}, 
    {"SetRank",  (lua_CFunction) GameObject_SetRank},
    {"SetAlignment",  (lua_CFunction) GameObject_SetAlignment},
    {"GetAlignmentForce",  (lua_CFunction) GameObject_GetAlignmentForce},
	{"GetGuild",  (lua_CFunction) GameObject_GetGuild},
	{"CheckGuild",  (lua_CFunction) GameObject_CheckGuild},
	{"JoinGuild",  (lua_CFunction) GameObject_JoinGuild},
	{"LeaveGuild",  (lua_CFunction) GameObject_LeaveGuild},
    {"Save", (lua_CFunction) GameObject_Save}, 
    {"GetIP", (lua_CFunction) GameObject_GetIP},
    {"GetArchName", (lua_CFunction) GameObject_GetArchName}, 
    {"ShowCost",  (lua_CFunction) GameObject_ShowCost},
    {"GetItemCost",  (lua_CFunction) GameObject_GetItemCost},
	{"AddMoney",  (lua_CFunction) GameObject_AddMoney},
	{"AddMoneyEx",  (lua_CFunction) GameObject_AddMoneyEx},
    {"GetMoney",  (lua_CFunction) GameObject_GetMoney},
    {"PayForItem", (lua_CFunction) GameObject_PayForItem}, 
    {"PayAmount", (lua_CFunction) GameObject_PayAmount},
    {"SendCustomCommand",(lua_CFunction) GameObject_SendCustomCommand},
    {"CheckTrigger", (lua_CFunction) GameObject_CheckTrigger}, 
    {"Clone", (lua_CFunction) GameObject_Clone},
    {"Move", (lua_CFunction) GameObject_Move},
    {"GetAI", (lua_CFunction) GameObject_GetAI},
    {"GetVector", (lua_CFunction) GameObject_GetVector},
    {"GetFace", (lua_CFunction) GameObject_GetFace},
    {"GetAnimation", (lua_CFunction) GameObject_GetAnimation},
    {"SetFace", (lua_CFunction) GameObject_SetFace},
    {"SetAnimation", (lua_CFunction) GameObject_SetAnimation},
    {"MakePet", (lua_CFunction) GameObject_MakePet},
    {"GetPets", (lua_CFunction) GameObject_GetPets},

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
    {"environment", FIELDTYPE_OBJECT, offsetof(object, env), FIELDFLAG_READONLY},
    {"map", FIELDTYPE_MAP, offsetof(object, map), FIELDFLAG_READONLY},
    {"count", FIELDTYPE_UINT32, offsetof(object, count), FIELDFLAG_READONLY},
    {"name", FIELDTYPE_SHSTR, offsetof(object, name), 0},
    {"title", FIELDTYPE_SHSTR, offsetof(object, title), 0},
    {"race", FIELDTYPE_SHSTR, offsetof(object, race), 0},
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
    {"value",        FIELDTYPE_SINT64, offsetof(object, value), 0},
    /* TODO: Max 100000 */
    {"quantity",     FIELDTYPE_UINT32, offsetof(object, nrof), 0},

    /* TODO: I don't know what these do, or if they should be accessible... */
    {"damage_round_tag", FIELDTYPE_UINT32, offsetof(object, damage_round_tag), 0},
    {"update_tag",   FIELDTYPE_UINT32, offsetof(object, update_tag), 0},

    /* TODO: make enemy & owner settable (requires HOOKS for set_npc_enemy() and set_owner()) */
    {"enemy",        FIELDTYPE_OBJECTREF, offsetof(object, enemy), FIELDFLAG_READONLY, offsetof(object, enemy_count)},
    // TODO: remove    {"attacked_by",  FIELDTYPE_OBJECTREF, offsetof(object, attacked_by), FIELDFLAG_READONLY, offsetof(object, attacked_by_count)},
    {"owner",        FIELDTYPE_OBJECTREF, offsetof(object, owner), FIELDFLAG_READONLY, offsetof(object, owner_count)},
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
    {"anim_enemy_dir", FIELDTYPE_SINT8 , offsetof(object, anim_enemy_dir), 0},
    {"anim_moving_dir", FIELDTYPE_SINT8 , offsetof(object, anim_moving_dir), 0},
    {"anim_enemy_dir_last", FIELDTYPE_SINT8 , offsetof(object, anim_enemy_dir_last), 0},
    {"anim_moving_dir_last", FIELDTYPE_SINT8 , offsetof(object, anim_moving_dir_last), 0},
    {"anim_last_facing", FIELDTYPE_SINT8 , offsetof(object, anim_last_facing), 0},
    {"anim_last_facing_last", FIELDTYPE_SINT8 , offsetof(object, anim_last_facing_last), 0},
    {"anim_speed",   FIELDTYPE_UINT8 , offsetof(object, anim_speed), 0},
    {"last_anim",    FIELDTYPE_UINT8 , offsetof(object, last_anim), 0},
    {"run_away",     FIELDTYPE_UINT8 , offsetof(object, run_away), 0},
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
    /* TODO: Limit to +-30 (all  */
    {"strength",     FIELDTYPE_SINT8, offsetof(object, stats.Str), FIELDFLAG_PLAYER_FIX},
    {"dexterity",     FIELDTYPE_SINT8, offsetof(object, stats.Dex), FIELDFLAG_PLAYER_FIX},
    {"constitution",     FIELDTYPE_SINT8, offsetof(object, stats.Con), FIELDFLAG_PLAYER_FIX},
    {"wisdom",     FIELDTYPE_SINT8, offsetof(object, stats.Wis), FIELDFLAG_PLAYER_FIX},
    {"charisma",     FIELDTYPE_SINT8, offsetof(object, stats.Cha), FIELDFLAG_PLAYER_FIX},
    {"intelligence",     FIELDTYPE_SINT8, offsetof(object, stats.Int), FIELDFLAG_PLAYER_FIX},
    {"power",     FIELDTYPE_SINT8, offsetof(object, stats.Pow), FIELDFLAG_PLAYER_FIX},
    {"luck",    FIELDTYPE_SINT8, offsetof(object, stats.luck), FIELDFLAG_PLAYER_READONLY},
	{"thac0",     FIELDTYPE_SINT8, offsetof(object, stats.thac0), FIELDFLAG_PLAYER_READONLY},
	{"thacm",     FIELDTYPE_SINT8, offsetof(object, stats.thacm), FIELDFLAG_PLAYER_READONLY},
    {NULL}
};

/* This is a list of strings that correspond to the FLAG_.. values.
 * This is a simple 1:1 mapping - if FLAG_FRIENDLY is 15, then
 * the 15'th element of this array should match that name.
 * If an entry is NULL, that flag cannot be set/read from scripts
 * If an entry begins with "?", that flag is read-only
 * Yes, this is almost exactly a repeat from loader.c
 */
static const char          *GameObject_flags[NUM_FLAGS + 1 + 1] =
{
    "f_sleep", "f_confused", "?f_paralyzed", "f_scared", "f_is_blind", "f_is_invisible", "f_is_ethereal", "f_is_good",
    "f_no_pick", "f_walk_on", "f_no_pass",     /* 10 */
    "f_is_animated", NULL, "f_flying", "f_monster", "f_friendly", "?f_is_removed", "f_been_applied", "f_auto_apply",
    "f_treasure", "f_is_neutral", /* 20 */
    "f_see_invisible", "f_can_roll", "f_generator", "f_is_turnable", "f_walk_off", "f_fly_on", "f_fly_off",
    "f_is_used_up", "f_identified", "f_reflecting",    /* 30 */
    "f_changing", "f_splitting", "f_hitback", "f_startequip", "f_blocksview", "f_undead", "f_fix_player", "f_unaggressive",
    "f_reflect_missile", "f_reflect_spell",             /* 40 */
    "f_no_magic", "f_no_fix_player", "f_is_evil", "f_tear_down", "f_run_away", "f_pass_thru", "f_can_pass_thru",
    "?f_feared", NULL, "f_no_drop", /* 50 */
    "f_reg_f", "f_has_ready_spell", "f_surrendered", "?f_rooted", "?f_slowed",
    "f_can_use_armour", "f_can_use_weapon", "f_can_use_ring", NULL /* unused */, "f_has_ready_bow",       /* 60 */
    "f_xrays", "?f_no_apply", NULL, "f_lifesave", "f_is_magical", "f_alive", "f_stand_still", "f_random_move",
    "f_only_attack", "?f_wiz", /* 70 */
    "f_stealth", "?f_wizpass", "?f_is_linked", "f_cursed", "f_damned", "f_see_anywhere", "f_known_magical", "f_known_cursed",
    "f_can_use_skill", "f_is_thrown",               /* 80 */
    NULL, NULL, "f_is_male", "f_is_female", "f_applied", "f_inv_locked", "f_is_wooded",
    "f_is_hilly", "f_levitate", "f_has_ready_weapon",        /* 90 */
    "f_no_skill_ident", "f_use_dmg_info", "f_can_see_in_dark", "f_is_cauldron", "f_is_dust", "f_no_steal",
    "f_one_hit", NULL /* debug flag CLIENT_SENT */, "f_berserk", "f_no_attack",   /* 100 */
    "f_invulnerable", "f_quest_item", "f_is_traped", "f_proof_phy", "f_proof_ele", /* 105 */
    "f_proof_mag", "f_proof_sph", "f_no_inv", NULL, "f_sys_object", /* 110 */
    "f_use_fix_pos", "f_unpaid", "f_is_aged", "f_make_invisible", "f_make_ethereal", "f_is_player", "f_is_named",
    "?f_spawn_mob_flag", "f_no_teleport", "f_corpse", "f_corpse_forced", "f_player_only", "f_no_cleric",
    "f_one_drop", "f_cursed_perm", "f_damned_perm", "f_door_closed", "f_was_reflected", "f_is_missile",
    "f_can_reflect_missile", "f_can_reflect_spell", "f_is_assassin", NULL /* internal flag: HAS_MOVED */, "?f_no_save",
	"f_pass_ethereal","f_ego", "f_egobound", "f_egoclan", "f_egolock",

    FLAGLIST_END_MARKER /* Marks the end of the list */
};

/****************************************************************************/
/*                          GameObject methods                              */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

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
/* Lua    : object:GetName()                                                 */
/* Info   : same as query_short_name()                                       */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetName(lua_State *L)
{
    lua_object     *self, *owner = NULL;
	object		   *obj;
    static char    *result;

    get_lua_args(L, "O|O", &self, &owner);

	if(owner)
		obj = owner->data.object;

	result = hooks->query_short_name(WHO, obj);

    lua_pushstring(L, result);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetEquipment                                          */
/* Lua    : object:GetEquipment(idnum)                                       */
/* Info   : Only works for player objects                                    */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetEquipment(lua_State *L)
{
    int num;
    lua_object *self;

    get_lua_args(L, "Oi", &self, &num);

	if(CONTR(WHO) && num >= 0 && num < PLAYER_EQUIP_MAX && CONTR(WHO)->equipment[num])
	    return push_object(L, &GameObject, CONTR(WHO)->equipment[num]);
	else
		return 0;
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

	/* the sint64 to uint32 cast should be ok - can't think about that high repair costs */
	if(WHO)
    {
	    lua_pushnumber(L, (uint32)hooks->material_repair_cost(WHO, NULL));
        return 1;
    } else
		return 0;
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

    get_lua_args(L, "O|i", &self, &skill);

	hooks->material_repair_item(WHO, skill);
	SET_FLAG(WHO, FLAG_FIX_PLAYER);

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
/*        : A mode of -1 means to close explicit a open interface at client  */
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
    int         value;
    lua_object *self;

    get_lua_args(L, "Os", &self, &txt);

	SET_FLAG(WHO, FLAG_FIX_PLAYER);
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
/* Lua    : object:TeleportTo(map, x, y, unique)                             */
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
        GCFP.Value[0] = (void *) (myob);
        (PlugHooks[HOOK_REMOVEOBJECT]) (&GCFP);
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
/* Lua    : object:PickUp(what)                                              */
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
/* Lua    : object:Drop(what)                                                */
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
/* Lua    : object:Take(name)                                                */
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
        hooks->new_draw_info(NDI_UNIQUE, 0, WHO, "deposit what?\nUse 'deposit all' or 'deposit 40 gold, 20 silver...'");
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
            hooks->new_draw_info(NDI_UNIQUE, 0, WHO, "You don't have that much money.");
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
        hooks->new_draw_info(NDI_UNIQUE, 0, WHO, "withdraw what?\nUse 'withdraw all' or 'withdraw 30 gold, 20 silver....'");
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
            hooks->new_draw_info(NDI_UNIQUE, 0, WHO, "withdraw values to high.");
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
                if (money.mithril)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[0]->clone, money.mithril);
                if (money.gold)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[1]->clone, money.gold);
                if (money.silver)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[2]->clone, money.silver);
                if (money.copper)
                    hooks->insert_money_in_player(WHO, &hooks->coins_arch[3]->clone, money.copper);

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

    get_lua_args(L, "Os", &self, &message);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (message);

    (PlugHooks[HOOK_COMMUNICATE]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Say                                                   */
/* Lua    : object:Say(message, mode)                                        */
/* Info   : object says message to everybody on its map                      */
/*          mode: 0 (default) talk to object - adds a "xxx says:" as prefix  */
/*			mode: 1 raw message                                              */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Say(lua_State *L)
{
    char       *message, buf[HUGE_BUF];
    int         mode = 0;
    lua_object *self;

    get_lua_args(L, "Os|i", &self, &message, &mode);

    if (!mode)
    {
        snprintf(buf, sizeof(buf), "%s says: %s", STRING_OBJ_NAME(WHO), message);
        message = buf;
    }
    hooks->new_info_map(NDI_NAVY|NDI_UNIQUE, WHO->map, WHO->x, WHO->y, MAP_INFO_NORMAL, message);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SayTo                                                 */
/* Lua    : object:SayTo(target, message, mode)                              */
/* Info   : NPC talks only to player but map get a "xx talks to" msg too.    */
/*          mode: 0 (default) talk to object - adds a "xxx says:" as prefix  */
/*			mode: 1 raw message                                              */
/*          mode: 2 adds as global map msg: xxx talks to yyy (was b3 default)*/
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

    if(mode == 1)
        hooks->new_draw_info(NDI_NAVY|NDI_UNIQUE, 0, target, message);
    else /* thats default */
    {
	    if(mode == 2)
		{
	        snprintf(buf, sizeof(buf), "%s talks to %s.", STRING_OBJ_NAME(WHO),STRING_OBJ_NAME(target));
		    hooks->new_info_map_except(NDI_UNIQUE, WHO->map, WHO->x, WHO->y, MAP_INFO_NORMAL, WHO, target, buf);
		}
        snprintf(buf, sizeof(buf), "%s says: %s", STRING_OBJ_NAME(WHO), message);
        hooks->new_draw_info(NDI_NAVY|NDI_UNIQUE, 0, target, buf);
    }

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Write                                                 */
/* Lua    : object:Write(message, color)                                     */
/* Info   : Writes a message to a specific player.                           */
/*          color should be one of the game.COLOR_xxx constants.             */
/*          default color is game.COLOR_BLUE | game.COLOR_UNIQUE             */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_Write(lua_State *L)
{
    char       *message;
    int         color   = NDI_UNIQUE | NDI_ORANGE;
    lua_object *self;

    get_lua_args(L, "Os|i", &self, &message, &color);

    hooks->new_draw_info(color, 0, WHO, message);

    return 0;
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
/* Lua    : object:GetGuild()                                                */
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

	get_lua_args(L, "O", &self, &name);

	if(name && !strcmp(WHO->slaying, name))
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
	hooks->new_draw_info_format(NDI_WHITE, 0, WHO, "you join %s Guild.", name);

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

    get_lua_args(L, "Oii", &self, &spell, &mode);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&spell);
    GCFP.Value[2] = (void *) (&mode);
    (PlugHooks[HOOK_LEARNSPELL]) (&GCFP);

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

    get_lua_args(L, "Oii", &self, &skill, &mode);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (&skill);
    GCFP.Value[2] = (void *) (&mode);
    (PlugHooks[HOOK_LEARNSKILL]) (&GCFP);

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
        SET_FLAG(myob, FLAG_IS_USED_UP);
        myob->stats.food = time;
        myob->speed = 0.02f;
        GCFP.Value[0] = (void *) (myob);
        (PlugHooks[HOOK_UPDATESPEED]) (&GCFP);
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
		hooks->new_draw_info_format(NDI_YELLOW, 0, WHO, "You can't have more as %d open quests.\nRemove one first!", QUESTS_PENDING_MAX);
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
/* Lua    : object:GetQuest(archetype, name)                                 */
/* Status : Stable                                                           */
/* Info   : We browse the quest object container for a quest_trigger object  */
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
/* Lua    : object:CheckQuestLevel(level, skill_level)                       */
/* Status : Stable                                                           */
/* Info   : We check a quest is possible to start                            */
/*****************************************************************************/
static int GameObject_CheckQuestLevel(lua_State *L)
{
	int level, skill_level, tmp_lev, ret=1;
	lua_object *self;
	object *who;
	player *pl;

	get_lua_args(L, "Oii", &self, &level, &skill_level);

	who = WHO;
	/* some sanity checks */
	if(who->type != PLAYER || !(pl=CONTR(who)) || skill_level < 0 || skill_level > NROFSKILLGROUPS)
		return 0;

	/* player is high enough for this quest? */
	if (skill_level)
		tmp_lev = pl->exp_obj_ptr[skill_level-1]->level; /* use player struct shortcut ptrs */
	else
		tmp_lev = who->level;

	if (level > tmp_lev) /* to low */
		ret = 0;

    lua_pushboolean(L, ret);
	return 1;
}

/*****************************************************************************/
/* Name   : GameObject_AddQuestTarget                                        */
/* Lua    : object:AddQuestTarget(chance, nrof, k_arch, k_name, k_title)     */
/* Info   : define a kill mob. Careful: if all are "" then ALL mobs are part */
/*        : of this quest. If only arch set, all mobs using that base arch   */
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
/*        : (see GameObject_AddQuestTarget)                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_AddQuestItem(lua_State *L)
{
	int         id, nrof;
	char	   *i_arch, *i_face, *i_name = NULL, *i_title = NULL;
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
/*        : where target_obj is inside                                       */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_NrofQuestItem(lua_State *L)
{
	int				 nrof=0;
	const object	*myob, *pl;
	lua_object		*self;

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
/*        : Get the template info from the kill target obj inventory         */
/*        : NOTE: the function tries to remove given objects even when there,*/
/*        : are not enough! count them before calling this function.         */
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_RemoveQuestItem(lua_State *L)
{
	int				 nrof = -1;
	object			*myob, *pl;
	lua_object		*self;

	get_lua_args(L, "O|i", &self, &nrof);

	pl = hooks->is_player_inv(self->data.object);
	myob = self->data.object->inv;

	/* some sanity checks */
	if(myob && pl && pl->type == PLAYER)
	{
		if(nrof == -1) /* if we don't have an explicit number, use number from kill target */
			nrof = myob->nrof;

		hooks->new_draw_info_format(NDI_WHITE, 0, pl, "%s is removed from your inventory.",
														hooks->query_short_name(myob, NULL));
		remove_quest_items(pl->inv, myob, nrof);
	}

	return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetQuestStatus                                        */
/* Lua    : object:SetQuestStatus(status, step_id)                           */
/* Info   : We need this function because quest_trigger must be moved        */
/*        : q_status: -1 = done, q_type: can change type (kill, normal..)    */
/*        : Common call is SetQuestStatus(-1) to "finish"/neutralize a quest */
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
/* Info   : add a one drop item to the player                                */
/*        : create the quest container if needed                             */
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
/*          inventory                                                        */
/*          The Values of a player_info object will NOT effect the player.   */
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
    name[63] = '\0';

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
/* Lua    : object:CreateInvisibleInside(id)                                 */
/* Status : Untested                                                         */
/*****************************************************************************/
static int GameObject_CreateInvisibleInside(lua_State *L)
{
    char       *txt;
    object     *myob;
    lua_object *whereptr;

    get_lua_args(L, "Os", &whereptr, &txt);

    myob = hooks->get_archetype("force");

    if (!myob || strncmp(STRING_OBJ_NAME(myob), "singularity", 11) == 0)
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
	hooks->esrv_send_item(hooks->is_player_inv(WHERE), myob);

    return push_object(L, &GameObject, myob);
}

/* code body of the CreateObjectInside functions */
static object *CreateObjectInside_body(lua_State *L, object *where, char *archname, int id, int nrof, int value)
{
	object *myob = hooks->get_archetype(archname);
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
/* Status : Stable                                                           */
/*****************************************************************************/
static int GameObject_CreateObjectInside(lua_State *L)
{
    object *myob;
	int         value = -1, id, nrof = 1;
	char       *txt;
	lua_object *whereptr;

	get_lua_args(L, "Osii|i", &whereptr, &txt, &id, &nrof, &value);

	myob = CreateObjectInside_body(L, WHERE, txt, id, nrof, value);
    
	hooks->esrv_send_item(hooks->is_player_inv(myob), myob);

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
	int         value = -1, id, nrof = 1;
	char       *txt;
	lua_object *whereptr;

	get_lua_args(L, "Osii|i", &whereptr, &txt, &id, &nrof, &value);

	myob = CreateObjectInside_body(L, WHERE, txt, id, nrof, value);

	pl = hooks->is_player_inv(myob);
	if(pl)
	{
		hooks->esrv_send_item(pl, myob);
		hooks->new_draw_info_format(NDI_WHITE, 0, pl, "you got %d %s",
				myob->nrof?myob->nrof:1, hooks->query_short_name(myob, NULL));
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
        strcpy(CONTR(WHO)->savebed_map, map->data.map->path);
        CONTR(WHO)->bed_x = x;
        CONTR(WHO)->bed_y = y;
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
        nrof = MIN(self->data.object->nrof, 1);

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
    /*`TODO: this is broken. See reduce_ob_nrof for the correct implementation (and possibly
     * something that can be broken out and reused */
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
/* Lua    : object:Destruct()                                                */
/* Info   : Removes the object from the game and drops all items in object's */
/*          inventory on the floor or in a corpse                            */
/* Status : Recently reimplemented. Untested                                 */
/*****************************************************************************/
static int GameObject_Destruct(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "O", &self);

    if(WHO->inv)
        hooks->drop_ob_inv(WHO);

    hooks->decrease_ob_nr(WHO, MIN(WHO->nrof, 1));

    return 0;

    /* Old implementation */
#if 0
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
#endif
}

/*****************************************************************************/
/* Name   : GameObject_SetPosition                                           */
/* Lua    : object:SetPosition(map, x, y)                                    */
/* Info   : Moves the object to the new position (or near it if blocked). Can*/
/*          be used for moving objects out of containers or from limbo.      */
/*          The old variant object:SetPosition(x,y) is still supported, but  */
/*          with the original limitation that the object must be on a map to */
/*          start from.                                                      */
/*          Returns true if the object was destroyed in the move             */
/* Version: map parameter added in beta 4 pre3 (original 2-parameter version */
/*          still supported).                                                */
/* Status : Tested                                                           */
/*****************************************************************************/
/* FIXME: if the object moved was triggered by SAY event and it is moved to a tile
 * within the listening radius, it will be triggered again, and again... */
static int GameObject_SetPosition(lua_State *L)
{
    lua_object *self;
    mapstruct  *map;
    int         x, y, k;
    k = 0;

    if(lua_isuserdata(L, 2))
    {
        lua_object *map_obj;
        get_lua_args(L, "OMii", &self, &map_obj, &x, &y);
        map = map_obj->data.map;
    } 
    else
    {
        get_lua_args(L, "Oii", &self, &x, &y);
        map = WHO->map;
    }

    lua_pushboolean(L, hooks->transfer_ob(WHO, x, y, map, 0, NULL, NULL));
    return 1;
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
/* Name   : GameObject_Save                                                  */
/* Lua    : object:Save()                                                    */
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
/* Lua    : object:ShowCost(value)                                           */
/* Info   : Returns a string describing value as x gold, x silver, x copper  */
/*        : cost string comes from shop.c and is temporary STATIC            */
/*        : note: whoptr is not used - perhaps we use this in future with it */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_ShowCost(lua_State *L)
{
    lua_object *self;
    sint64      value;
	int			mode = 0;
    char       *cost_string;

    get_lua_args(L, "OI|i", &self, &value, &mode);

	/* mode 0: string is like "4 gold coins, 3 silver coins"...
     * mode 1: string is like "4g, 3s"
	 */
    cost_string = hooks->cost_string_from_value(value, mode);

    lua_pushstring(L, cost_string);
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
	char		buf[MAX_BUF];
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
	hooks->new_draw_info(NDI_WHITE, 0, WHO, buf);

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

    get_lua_args(L, "Os", &self, &customcmd);

    GCFP.Value[0] = (void *) (WHO);
    GCFP.Value[1] = (void *) (customcmd);
    (PlugHooks[HOOK_SENDCUSTOMCOMMAND]) (&GCFP);

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_Clone                                                 */
/* Lua    : object:Clone(mode)                                               */
/* Info   : mode = game.CLONE_WITH_INVENTORY (default) or                    */
/*          game.CLONE_WITHOUT_INVENTORY                                     */
/*          You should do something with the clone. TeleportTo(),            */
/*          SetPosition() and InsertInside() are useful functions for this.  */
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
/* Lua    : object:GetAnimation(inv)                                         */
/* Info   : Returns the name of object's animation, if any.                  */
/*          If inv is true, it returns the name of the inventory animation   */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetAnimation(lua_State *L)
{
    lua_object *self;
    int inv = 0;

    get_lua_args(L, "O|i", &self, &inv);

    lua_pushstring(L, (* hooks->animations)[inv ? WHO->inv_animation_id : WHO->animation_id].name);
    return 1;
}

/*****************************************************************************/
/* Name   : GameObject_GetFace                                               */
/* Lua    : object:GetFace(inv)                                              */
/* Info   : Returns the name of object's face, if any.                       */
/*          If inv is true, it returns the name of the inventory face        */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_GetFace(lua_State *L)
{
    lua_object *self;
    int inv = 0;
    New_Face *face;

    get_lua_args(L, "O|i", &self, &inv);

    face = inv ? WHO->inv_face : WHO->face;
    if(face)
    {
        lua_pushstring(L, face->name);
        return 1;
    } else
        return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetAnimation                                          */
/* Lua    : object:SetAnimation(anim, inv)                                   */
/* Info   : Sets object's animation.                                         */
/*          If inv is true, it sets the inventory animation                  */
/*          Note that an object will only be animated if object.f_is_animated*/
/*          is true                                                          */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetAnimation(lua_State *L)
{
    lua_object *self;
    int inv = 0;
    char *animation;
    int id;

    get_lua_args(L, "Os|i", &self, &animation, &inv);

    id = hooks->find_animation(animation);
    if(id == 0)
        luaL_error(L, "no such animation exists: %s", animation);

    if(inv)
        WHO->inv_animation_id = id;
    else
        WHO->animation_id = id;

    return 0;
}

/*****************************************************************************/
/* Name   : GameObject_SetFace                                               */
/* Lua    : object:SetFace(face, inv)                                        */
/* Info   : Sets object's face.                                              */
/*          If inv is true, it sets the inventory face                       */
/*          If the object is animated (object.f_is_animated == true), then   */
/*          this value will likely be replaced at the next animation step    */
/* Status : Tested                                                           */
/*****************************************************************************/
static int GameObject_SetFace(lua_State *L)
{
    lua_object *self;
    int inv = 0;
    char *face;
    int id;

    get_lua_args(L, "Os|i", &self, &face, &inv);

    id = hooks->find_face(face, -1);
    if(id == -1)
        luaL_error(L, "no such face exists: %s", STRING_SAFE(face));

    if(inv)
        WHO->inv_face = &(*hooks->new_faces)[id];
    else
        WHO->face = &(*hooks->new_faces)[id];

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

/****************************************************************************/
/* Lua object management code                                               */
/****************************************************************************/

/* This gets called before and after an attribute has been set in an object */
static int GameObject_setAttribute(lua_State *L, lua_object *obj, struct attribute_decl *attrib, int before)
{
    object *who = obj->data.object;

    /* Pre-setting hook */
    if (before)
    {
        if (who->type == PLAYER && attrib->flags & FIELDFLAG_PLAYER_READONLY)
            luaL_error(L, "attribute %s is readonly on players", attrib->name);
        return 0;
    }

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
        lua_pushfstring(L, "[%s \"%s\"] ", STRING_OBJ_ARCH_NAME(obj->data.object), STRING_OBJ_NAME(obj->data.object));
    else
        luaL_error(L, "Not an object");

    return 1;
}

/* Tests if an object is valid */
static int GameObject_isValid(lua_State *L, lua_object *obj)
{
    return obj->data.object->count == obj->tag;
}

lua_class   GameObject  =
{
    LUATYPE_OBJECT, "GameObject", 0, GameObject_toString, GameObject_attributes, GameObject_methods, NULL,
    GameObject_flags,
    GameObject_getFlag, GameObject_setFlag, GameObject_setAttribute,
    GameObject_isValid
};

int GameObject_init(lua_State *L)
{
    init_class(L, &GameObject);

    return 0;
}

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
