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
#include <daimonin_object.h>
#include <inline.h>

/* Global data */

/* Available python methods for the DaimoninObject object */
static PyMethodDef ObjectMethods[] =
{
    {"SetSaveBed",  (PyCFunction)Daimonin_Object_SetSaveBed, METH_VARARGS},
    {"GetSkill",  (PyCFunction)Daimonin_Object_GetSkill, METH_VARARGS},
    {"SetSkill",  (PyCFunction)Daimonin_Object_SetSkill, METH_VARARGS},
    {"ActivateRune",  (PyCFunction)Daimonin_Object_ActivateRune, METH_VARARGS},
    {"CastAbility",  (PyCFunction)Daimonin_Object_CastAbility, METH_VARARGS},
    {"InsertInside",  (PyCFunction)Daimonin_Object_InsertInside, METH_VARARGS},
    {"GetGod",  (PyCFunction)Daimonin_Object_GetGod, METH_VARARGS},
    {"SetGod",  (PyCFunction)Daimonin_Object_SetGod, METH_VARARGS},
    {"TeleportTo",  (PyCFunction)Daimonin_Object_TeleportTo, METH_VARARGS},
    {"Apply",  (PyCFunction)Daimonin_Object_Apply, METH_VARARGS},
    {"PickUp",  (PyCFunction)Daimonin_Object_PickUp, METH_VARARGS},
    {"Drop",  (PyCFunction)Daimonin_Object_Drop, METH_VARARGS},
    {"Take",  (PyCFunction)Daimonin_Object_Take, METH_VARARGS},
    {"Fix", (PyCFunction)Daimonin_Object_Fix,METH_VARARGS},
    {"Kill", (PyCFunction)Daimonin_Object_Kill,METH_VARARGS},
    {"CastSpell", (PyCFunction)Daimonin_Object_CastSpell,METH_VARARGS},
    {"DoKnowSpell", (PyCFunction)Daimonin_Object_DoKnowSpell,METH_VARARGS},
    {"AcquireSpell", (PyCFunction)Daimonin_Object_AcquireSpell,METH_VARARGS},
    {"DoKnowSkill", (PyCFunction)Daimonin_Object_DoKnowSkill,METH_VARARGS},
    {"AcquireSkill", (PyCFunction)Daimonin_Object_AcquireSkill,METH_VARARGS},
    {"FindMarkedObject", (PyCFunction)Daimonin_Object_FindMarkedObject,METH_VARARGS},
    {"CheckQuestObject", (PyCFunction)Daimonin_Object_CheckQuestObject,METH_VARARGS},
    {"AddQuestObject", (PyCFunction)Daimonin_Object_AddQuestObject,METH_VARARGS},
    {"CreatePlayerForce", (PyCFunction)Daimonin_Object_CreatePlayerForce,METH_VARARGS},
    {"CreatePlayerInfo", (PyCFunction)Daimonin_Object_CreatePlayerInfo,METH_VARARGS},
    {"GetPlayerInfo", (PyCFunction)Daimonin_Object_GetPlayerInfo,METH_VARARGS},
    {"GetNextPlayerInfo", (PyCFunction)Daimonin_Object_GetNextPlayerInfo,METH_VARARGS},

    {"CheckInvisibleObjectInside", (PyCFunction)Daimonin_Object_CheckInvisibleInside,METH_VARARGS},
    {"CreateInvisibleObjectInside", (PyCFunction)Daimonin_Object_CreateInvisibleInside,METH_VARARGS},
    {"CreateObjectInside", (PyCFunction)Daimonin_Object_CreateObjectInside,METH_VARARGS},
    {"CheckInventory", (PyCFunction)Daimonin_Object_CheckInventory,METH_VARARGS},
    {"Remove", (PyCFunction)Daimonin_Object_Remove,METH_VARARGS},
    {"SetPosition", (PyCFunction)Daimonin_Object_SetPosition,METH_VARARGS},
    {"IdentifyItem", (PyCFunction)Daimonin_Object_IdentifyItem,METH_VARARGS},
#if 0    
    {"GetEventHandler", (PyCFunction)Daimonin_Object_GetEventHandler,METH_VARARGS},
    {"SetEventHandler", (PyCFunction)Daimonin_Object_SetEventHandler,METH_VARARGS},
    {"GetEventPlugin", (PyCFunction)Daimonin_Object_GetEventPlugin,METH_VARARGS},
    {"SetEventPlugin", (PyCFunction)Daimonin_Object_SetEventPlugin,METH_VARARGS},
    {"GetEventOptions", (PyCFunction)Daimonin_Object_GetEventOptions,METH_VARARGS},
    {"SetEventOptions", (PyCFunction)Daimonin_Object_SetEventOptions,METH_VARARGS},
#endif    
    {"Communicate",  (PyCFunction)Daimonin_Object_Communicate, METH_VARARGS},
    {"Say",  (PyCFunction)Daimonin_Object_Say, METH_VARARGS},
    {"SayTo",  (PyCFunction)Daimonin_Object_SayTo, METH_VARARGS},
    {"Write", (PyCFunction)Daimonin_Object_Write,METH_VARARGS},
    {"SetGender",  (PyCFunction)Daimonin_Object_SetGender, METH_VARARGS},
    {"SetRank",  (PyCFunction)Daimonin_Object_SetRank, METH_VARARGS},
    {"SetAlignment",  (PyCFunction)Daimonin_Object_SetAlignment, METH_VARARGS},
    {"GetAlignmentForce",  (PyCFunction)Daimonin_Object_GetAlignmentForce, METH_VARARGS},
    {"SetGuildForce",  (PyCFunction)Daimonin_Object_SetGuildForce, METH_VARARGS},
    {"GetGuildForce",  (PyCFunction)Daimonin_Object_GetGuildForce, METH_VARARGS},
    {"IsOfType", (PyCFunction)Daimonin_Object_IsOfType,METH_VARARGS},
    {"Save", (PyCFunction)Daimonin_Object_Save,METH_VARARGS},
    {"GetIP", (PyCFunction)Daimonin_Object_GetIP,METH_VARARGS},
    {"GetArchName", (PyCFunction)Daimonin_Object_GetArchName,METH_VARARGS},
    {"ShowCost",  (PyCFunction)Daimonin_Object_ShowCost,METH_VARARGS},
    {"GetItemCost",  (PyCFunction)Daimonin_Object_GetItemCost,METH_VARARGS},
    {"GetMoney",  (PyCFunction)Daimonin_Object_GetMoney,METH_VARARGS},
    {"PayForItem", (PyCFunction)Daimonin_Object_PayForItem,METH_VARARGS},
    {"PayAmount", (PyCFunction)Daimonin_Object_PayAmount,METH_VARARGS},
    {"GetUnmodifiedAttribute", (PyCFunction)Daimonin_Object_GetUnmodifiedAttribute,METH_VARARGS},
    {"SendCustomCommand",(PyCFunction)Daimonin_Object_SendCustomCommand,METH_VARARGS},
    {"CheckTrigger", (PyCFunction)Daimonin_Object_CheckTrigger, METH_VARARGS},
    {"Clone", (PyCFunction)Daimonin_Object_Clone, METH_VARARGS},
    {NULL, NULL}
};

/* DaimoninObject attributes */
/* TODO: document ALL fields and flags */
static struct {
    char *name;
    field_type type;
    uint32 offset;    /* Offset in object struct */
    uint32 flags;     /* flags for special handling */
    uint32 extra_data; /* extra data for some special fields */
} obj_fields[] = {
    /* All entries MUST be in same order as field_id enum above */
    {"below", FIELDTYPE_OBJECT, offsetof(object, below), FIELDFLAG_READONLY},  
    {"above", FIELDTYPE_OBJECT, offsetof(object, above), FIELDFLAG_READONLY},
    {"inventory", FIELDTYPE_OBJECT, offsetof(object, inv), FIELDFLAG_READONLY},  
    {"map", FIELDTYPE_MAP, offsetof(object, map), FIELDFLAG_READONLY},  
    
    {"name", FIELDTYPE_SHSTR, offsetof(object, name), FIELDFLAG_PLAYER_READONLY},  
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
    {"value",        FIELDTYPE_SINT32, offsetof(object, value), 0},
    /* TODO: Max 100000 */
    {"quantity",         FIELDTYPE_UINT32, offsetof(object, nrof), 0},

    /* TODO: I don't know what these do, or if they should be accessible... */
    {"damage_round_tag", FIELDTYPE_UINT32, offsetof(object, damage_round_tag), 0},
    {"update_tag",   FIELDTYPE_UINT32, offsetof(object, update_tag), 0},
    
    /* TODO: make enemy & owner settable (requires HOOKS for set_npc_enemy() and set_owner()) */
    {"enemy",        FIELDTYPE_OBJECTREF, offsetof(object, enemy), FIELDFLAG_READONLY, offsetof(object, enemy_count)},
    {"attacked_by",  FIELDTYPE_OBJECTREF, offsetof(object, attacked_by), FIELDFLAG_READONLY, offsetof(object, attacked_by_count)},
    {"owner",        FIELDTYPE_OBJECTREF, offsetof(object, owner), FIELDFLAG_READONLY, offsetof(object, ownercount)},
    
    {"x",            FIELDTYPE_SINT16, offsetof(object, x), FIELDFLAG_READONLY},
    {"y",            FIELDTYPE_SINT16, offsetof(object, y), FIELDFLAG_READONLY},
    {"attacked_by_distance", FIELDTYPE_SINT16, offsetof(object, attacked_by_distance), 0},
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
    {"luck",    FIELDTYPE_SINT8, offsetof(object, stats.luck), FIELDFLAG_PLAYER_READONLY}
};

#define NUM_OBJFIELDS (sizeof(obj_fields) / sizeof(obj_fields[0]))

/* This is a list of strings that correspond to the FLAG_.. values.
 * This is a simple 1:1 mapping - if FLAG_FRIENDLY is 15, then
 * the 15'th element of this array should match that name.
 * If an entry is NULL, that flag cannot be set/read from scripts
 * Yes, this is almost exactly a repeat from loader.c 
 */
static char *flag_names[NUM_FLAGS+1] = {
    "f_sleep", "f_confused", "f_paralyzed", "f_scared", "f_is_blind", "f_is_invisible", "f_is_ethereal",
    "f_is_good", "f_no_pick", "f_walk_on", "f_no_pass",		/* 10 */
    "f_is_animated", "f_slow_move", "f_flying", "f_monster", "f_friendly", NULL /*is_removed*/, 
    "f_been_applied", "f_auto_apply", "f_treasure", "f_is_neutral",	/* 20 */
    "f_see_invisible", "f_can_roll", "f_generator", "f_is_turnable", "f_walk_off",
    "f_fly_on", "f_fly_off", "f_is_used_up", "f_identified", "f_reflecting",	/* 30 */
    "f_changing", "f_splitting", "f_hitback", "f_startequip",
    "f_blocksview", "f_undead", NULL /*freed*/, "f_unaggressive",
    "f_reflect_missile", "f_reflect_spell",				/* 40 */
    "f_no_magic", "f_no_fix_player", "f_is_evil", "f_tear_down", "f_run_away",
    "f_pass_thru", "f_can_pass_thru", "f_pick_up", "f_unique", "f_no_drop",	/* 50 */
    "f_is_indestructible", "f_can_cast_spell", "f_can_use_scroll", "f_can_use_range", 
    "f_can_use_bow",  "f_can_use_armour", "f_can_use_weapon", 
    "f_can_use_ring", "f_has_ready_range", "f_has_ready_bow",		/* 60 */
    "f_xrays", "f_no_apply", "f_is_floor", "f_lifesave", "f_is_magical", "f_alive",
    "f_stand_still", "f_random_move", "f_only_attack", "f_wiz",	/* 70 */
    "f_stealth", NULL /*wizpass*/, NULL /*is_linked*/, "f_cursed", "f_damned",
    "f_see_anywhere", "f_known_magical", "f_known_cursed",
    "f_can_use_skill", "f_is_thrown",				/* 80 */
    "f_is_vul_sphere", "f_is_proof_sphere", "f_is_male",
    "f_is_female", "f_applied",  "f_inv_locked", "f_is_wooded",
    "f_is_hilly", "f_has_ready_skill", "f_has_ready_weapon",		/* 90 */
    "f_no_skill_ident", NULL /*was_wiz*/, "f_can_see_in_dark", "f_is_cauldron", 
    "f_is_dust", "f_no_steal", "f_one_hit", NULL /*client_sent*/, "f_berserk", "f_no_attack",	/* 100 */
    "f_invulnerable", "quest_item" , NULL /*obj_save_on_ovl*/, "f_is_vul_elemental",  "f_is_proof_elemental", /* 105 */
    "f_is_vul_magic", "f_is_proof_magic", "f_is_vul_physical", "f_is_proof_physical", "f_sys_object", /* 110 */
    "f_use_fix_pos","f_unpaid","f_is_aged","f_make_invisible" , "f_make_ethereal", NULL/*is_player*/,
    "f_is_named",NULL /* spawn mob flag */, "f_no_teleport", "f_corpse", "f_corpse_forced",
    "f_player_only", "f_no_cleric", "f_one_drop", "f_cursed_perm", "f_damned_perm", "f_door_closed"
};

/* This is filled in when we initialize our object type */
static PyGetSetDef Object_getseters[NUM_OBJFIELDS + NUM_FLAGS + 1];

/* Our actual Python ObjectType */
PyTypeObject Daimonin_ObjectType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /* ob_size*/
    "Daimonin.Object",         /* tp_name*/
    sizeof(Daimonin_Object),   /* tp_basicsize*/
    0,                         /* tp_itemsize*/
    (destructor)Daimonin_Object_dealloc, /* tp_dealloc*/
    0,                         /* tp_print*/
    0,                         /* tp_getattr*/
    0,                         /* tp_setattr*/
    0,                         /* tp_compare*/
    0,                         /* tp_repr*/
    0,                         /* tp_as_number*/
    0,                         /* tp_as_sequence*/
    0,                         /* tp_as_mapping*/
    0,                         /* tp_hash */
    0,                         /* tp_call*/
    (reprfunc)Daimonin_Object_str,/* tp_str*/
    0,                         /* tp_getattro*/
    0,                         /* tp_setattro*/
    0,                         /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /* tp_flags*/
    "Daimonin objects",        /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    ObjectMethods,             /* tp_methods */
    0,                         /* tp_members */
    Object_getseters,          /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    Daimonin_Object_new,       /* tp_new */
};


/* Object constants */
static Daimonin_Constant object_constants[] = {
    {"MAP_INFO_NORMAL", MAP_INFO_NORMAL},
    {"MAP_INFO_ALL", MAP_INFO_ALL},

    {"COST_TRUE", F_TRUE},
    {"COST_BUY", F_BUY},
    {"COST_SELL", F_SELL},
    
    {"APPLY_TOGGLE", 0},
    {"APPLY_ALWAYS", AP_APPLY},
    {"UNAPPLY_ALWAYS", AP_UNAPPLY},
    {"UNAPPLY_NO_MERGE", AP_NO_MERGE},
    {"UNAPPLY_IGNORE_CURSE", AP_IGNORE_CURSE},

    {"NEUTER", 0},
    {"MALE", 1},
    {"FEMALE", 2},
    {"HERMAPHRODITE", 3},

    {"CAST_NORMAL", 0},
    {"CAST_POTION", 1},
    
    {"LEARN", 0},
    {"UNLEARN", 1},
    
    {"UNIDENTIFIED", 0},
    {"IDENTIFIED", 1},

    {"IDENTIFY_NORMAL", 0},
    {"IDENTIFY_ALL", 1},
    {"IDENTIFY_MARKED", 2},

    {"CLONE_WITH_INVENTORY", 0},
    {"CLONE_WITHOUT_INVENTORY", 1},

    {"EXP_AGILITY", 1},
    {"EXP_MENTAL", 2},
    {"EXP_MAGICAL", 3},
    {"EXP_PERSONAL", 4},
    {"EXP_PHYSICAL", 5},
    {"EXP_WISDOM", 6},

    {"COLOR_ORANGE", NDI_ORANGE},
    {"COLOR_WHITE", NDI_WHITE},
    {"COLOR_NAVY", NDI_NAVY},
    {"COLOR_YELLOW", NDI_YELLOW},
    {"COLOR_BLUE", NDI_BLUE},
    {"COLOR_RED", NDI_RED},

    /* Argh, the object types. Make sure to keep up-to date if any are added/removed */
    {"TYPE_PLAYER"		            ,PLAYER},
    {"TYPE_BULLET"		            ,BULLET},
    {"TYPE_ROD"		                ,ROD},
    {"TYPE_TREASURE"	            ,TREASURE},
    {"TYPE_POTION"		            ,POTION},
    {"TYPE_FOOD"		            ,FOOD},
    {"TYPE_POISON"		            ,POISON},
    {"TYPE_BOOK"		            ,BOOK},
    {"TYPE_CLOCK"		            ,CLOCK},
    {"TYPE_FBULLET"		            ,FBULLET},
    {"TYPE_FBALL"		            ,FBALL},
    {"TYPE_LIGHTNING"	            ,LIGHTNING},
    {"TYPE_ARROW"		            ,ARROW},
    {"TYPE_BOW"		                ,BOW},
    {"TYPE_WEAPON"		            ,WEAPON},
    {"TYPE_ARMOUR"		            ,ARMOUR},
    {"TYPE_PEDESTAL"	            ,PEDESTAL},
    {"TYPE_ALTAR"		            ,ALTAR},
    {"TYPE_CONFUSION"	            ,CONFUSION},
    {"TYPE_LOCKED_DOOR" 	        ,LOCKED_DOOR},
    {"TYPE_SPECIAL_KEY"	            ,SPECIAL_KEY},
    {"TYPE_MAP"		                ,MAP},
    {"TYPE_DOOR"		            ,DOOR},
    {"TYPE_KEY"		                ,KEY},
    {"TYPE_MMISSILE"	            ,MMISSILE},
    {"TYPE_TIMED_GATE"	            ,TIMED_GATE},
    {"TYPE_TRIGGER"		            ,TRIGGER},
    {"TYPE_GRIMREAPER"	            ,GRIMREAPER},
    {"TYPE_MAGIC_EAR"	            ,MAGIC_EAR},
    {"TYPE_TRIGGER_BUTTON"	        ,TRIGGER_BUTTON},
    {"TYPE_TRIGGER_ALTAR"	        ,TRIGGER_ALTAR},
    {"TYPE_TRIGGER_PEDESTAL"        ,TRIGGER_PEDESTAL},
    {"TYPE_SHIELD"		            ,SHIELD},
    {"TYPE_HELMET"		            ,HELMET},
    {"TYPE_HORN"		            ,HORN},
    {"TYPE_MONEY"		            ,MONEY},
    {"TYPE_CLASS"                   ,CLASS},
    {"TYPE_GRAVESTONE"	            ,GRAVESTONE},
    {"TYPE_AMULET"		            ,AMULET},
    {"TYPE_PLAYERMOVER" 	        ,PLAYERMOVER},
    {"TYPE_TELEPORTER"	            ,TELEPORTER},
    {"TYPE_CREATOR"		            ,CREATOR},
    {"TYPE_SKILL"		            ,SKILL},
    {"TYPE_EXPERIENCE"	            ,EXPERIENCE},
    {"TYPE_EARTHWALL"	            ,EARTHWALL},
    {"TYPE_GOLEM"		            ,GOLEM},
    {"TYPE_BOMB"		            ,BOMB},
    {"TYPE_THROWN_OBJ"	            ,THROWN_OBJ},
    {"TYPE_BLINDNESS"	            ,BLINDNESS},
    {"TYPE_GOD"		                ,GOD},
    {"TYPE_DETECTOR"	            ,DETECTOR},
    {"TYPE_SPEEDBALL"	            ,SPEEDBALL},
    {"TYPE_DEAD_OBJECT"	            ,DEAD_OBJECT},
    {"TYPE_DRINK"		            ,DRINK},
    {"TYPE_MARKER"                  ,MARKER},
    {"TYPE_HOLY_ALTAR"	            ,HOLY_ALTAR},
    {"TYPE_PLAYER_CHANGER"          ,PLAYER_CHANGER},
    {"TYPE_BATTLEGROUND"            ,BATTLEGROUND},
    {"TYPE_PEACEMAKER"              ,PEACEMAKER},
    {"TYPE_GEM"		                ,GEM},
    {"TYPE_FIRECHEST"	            ,FIRECHEST},
    {"TYPE_FIREWALL"	            ,FIREWALL},
    {"TYPE_ANVIL"                   ,ANVIL},
    {"TYPE_CHECK_INV"	            ,CHECK_INV},
    {"TYPE_MOOD_FLOOR"	            ,MOOD_FLOOR},
    {"TYPE_EXIT"		            ,EXIT},
    {"TYPE_AGE_FORCE"			    ,TYPE_AGE_FORCE},
    {"TYPE_SHOP_FLOOR"	            ,SHOP_FLOOR},
    {"TYPE_SHOP_MAT"	            ,SHOP_MAT},
    {"TYPE_RING"		            ,RING}, 
    {"TYPE_FLOOR"                   ,FLOOR},
    {"TYPE_FLESH"		            ,FLESH},
    {"TYPE_INORGANIC"	            ,INORGANIC},
    {"TYPE_LIGHT_APPLY"             ,TYPE_LIGHT_APPLY },
    {"TYPE_LIGHTER"		            ,LIGHTER},
    {"TYPE_TRAP_PART"	            ,TRAP_PART},
    {"TYPE_WALL"                    ,WALL},
    {"TYPE_LIGHT_SOURCE"            ,LIGHT_SOURCE},
    {"TYPE_MISC_OBJECT"             ,MISC_OBJECT},
    {"TYPE_MONSTER"                 ,MONSTER}, /* TODO: keep replaceing with the true defines... */
    {"TYPE_SPAWN_POINT"             ,81},
    {"TYPE_LIGHT_REFILL"            ,82},
    {"TYPE_SPAWN_POINT_MOB"		,83 },
    {"TYPE_SPAWN_POINT_INFO"		,84},
    {"TYPE_SPELLBOOK"	            ,85},
    {"TYPE_ORGANIC"				,86},
    {"TYPE_CLOAK"		            ,87},
    {"TYPE_CONE"		            ,88},
    {"TYPE_AURA"                   ,89},
    {"TYPE_SPINNER"		        ,90},
    {"TYPE_GATE"		            ,91},
    {"TYPE_BUTTON"		            ,92},
    {"TYPE_CF_HANDLE"		        ,93},
    {"TYPE_PIT"		            ,94},
    {"TYPE_TRAPDOOR"	            ,95},
    {"TYPE_WORD_OF_RECALL"	        ,96},
    {"TYPE_PARAIMAGE"	            ,97},
    {"TYPE_SIGN"		            ,98},
    {"TYPE_BOOTS"		            ,99},
    {"TYPE_GLOVES"		            ,100},
    {"TYPE_BASE_INFO"			,101},
    {"TYPE_RANDOM_DROP"		,102},
    {"TYPE_CONVERTER"	            ,103},
    {"TYPE_BRACERS"		        ,104},
    {"TYPE_POISONING"	            ,105 },
    {"TYPE_SAVEBED"		        ,106},
    {"TYPE_POISONCLOUD"	        ,107},
    {"TYPE_FIREHOLES"	            ,108},
    {"TYPE_WAND"		            ,109},
    {"TYPE_ABILITY"		        ,110},
    {"TYPE_SCROLL"		            ,111},
    {"TYPE_DIRECTOR"	            ,112},
    {"TYPE_GIRDLE"		            ,113},
    {"TYPE_FORCE"		            ,114},
    {"TYPE_POTION_EFFECT"         ,115},
    {"TYPE_JEWEL"				,116},
    {"TYPE_NUGGET"		    ,117},
    {"TYPE_EVENT_OBJECT"		,118},
    {"TYPE_WAYPOINT_OBJECT"	,119},
    {"TYPE_QUEST_CONTAINER"	,120},
    {"TYPE_CLOSE_CON"	            ,121},
    {"TYPE_CONTAINER"	            ,122},
    {"TYPE_ARMOUR_IMPROVER"        ,123},
    {"TYPE_WEAPON_IMPROVER"        ,124},
    {"TYPE_SKILLSCROLL"	        ,130},
    {"TYPE_DEEP_SWAMP"	            ,138},
    {"TYPE_IDENTIFY_ALTAR"	        ,139},
    {"TYPE_CANCELLATION"	        ,141},
    {"TYPE_MENU"		            ,150},
    {"TYPE_BALL_LIGHTNING"         ,151},
    {"TYPE_SWARM_SPELL"            ,153},
    {"TYPE_RUNE"                   ,154},
    {"TYPE_POWER_CRYSTAL"          ,156},
    {"TYPE_CORPSE"                 ,157},
    {"TYPE_DISEASE"                ,158},
    {"TYPE_SYMPTOM"                ,159},

    {NULL, 0}
};

/****************************************************************************/
/*                          Daimonin_Object methods                         */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Python plugin functions come */

/*****************************************************************************/
/* Name   : Daimonin_Object_GetSkill                                         */
/* Python : object.GetSkill(type, id)                                        */
/* Info   : This function will fetch a skill or exp_skill object             */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetSkill(Daimonin_Object *whoptr, PyObject* args)
{
    object *tmp;
    int type, id;

    if (!PyArg_ParseTuple(args,"ii",&type, &id))
        return NULL;

    /* Browse the inventory of object to find a matching skill or exp_obj. */
    for (tmp=WHO->inv;tmp;tmp=tmp->below)
    {
		if(tmp->type == SKILL &&  tmp->stats.sp == id)
            return wrap_object(tmp);

		if(tmp->type == EXPERIENCE &&  tmp->sub_type1 == id)
            return wrap_object(tmp);

    }

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetSkill                                         */
/* Python : object.SetSkill(skillid,value)                                   */
/* Info   : Sets objects's experience in the skill skillid as close to value */
/*          as permitted. There is currently a limit of 1/4 of a level.      */
/*          There's no limit on exp reduction.                               */
/*          FIXME overall experience is not changed (should it be?)          */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetSkill(Daimonin_Object *whoptr, PyObject* args)
{
    object *tmp;
    int type, skill, value, level, currentxp;

    if (!PyArg_ParseTuple(args,"iill", &type, &skill,&level, &value))
        return NULL;

	/* atm we don't set anything in exp_obj types */
	if(type != SKILL)
	{
		Py_INCREF(Py_None);
		return Py_None;
	}	

    /* Browse the inventory of object to find a matching skill. */
    for (tmp=WHO->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type==type && tmp->stats.sp == skill)
		{
			/* this is a bit tricky: some skills are marked with exp
			 * -1 or -2 as special used skills (level but no exp):
			 * if we have here a level > 0, we set level but NEVER
			 * exp ... if we have level == 0, we only set exp - the
			 * addexp 
			 */
			LOG(-1,"LEVEL1 %d (->%d) :: %s (exp %d)\n",tmp->level,level,query_name(tmp), tmp->stats.exp);
			if(level>0)
			{
				tmp->level = level;
			}
	        else
	        {
				/* Gecko: Changed to use actuall skill experience */
				currentxp = tmp->stats.exp;
				value = value - currentxp;   
            
				GCFP.Value[0] = (void *)(WHO);
				GCFP.Value[1] = (void *)(&value);
				GCFP.Value[2] = (void *)(&skill);
				(PlugHooks[HOOK_ADDEXP])(&GCFP);

	        }
			LOG(-1,"LEVEL2 %d (->%d) :: %s (exp %d)\n",tmp->level,level,query_name(tmp), tmp->stats.exp);
			if(WHO->type == PLAYER && WHO->contr)
				    WHO->contr->update_skills=1; /* we will sure change skill exp, mark for update */

			Py_INCREF(Py_None);
			return Py_None;
		}
    }

    RAISE("Unknown skill");
}

/*****************************************************************************/
/* Name   : Daimonin_Object_ActivateRune                                     */
/* Python : object.ActivateRune(what)                                        */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* Daimonin_Object_ActivateRune(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whatptr;

    if (!PyArg_ParseTuple(args,"O!", &Daimonin_ObjectType, &whatptr))
        return NULL;

    GCFP.Value[0] = (void *)(WHAT);
    GCFP.Value[1] = (void *)(WHO);
    (PlugHooks[HOOK_SPRINGTRAP])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CheckTrigger                                     */
/* Python : object.CheckTrigger(what)                                        */
/* Status : Unfinished                                                       */
/*****************************************************************************/
/* MUST DO THE HOOK HERE ! */
static PyObject* Daimonin_Object_CheckTrigger(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whatptr;

    if (!PyArg_ParseTuple(args,"O!", &Daimonin_ObjectType, &whatptr))
        return NULL;

   /* check_trigger(WHAT,WHO); should be hook too! */

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetGod                                           */
/* Python : object.GetGod()                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* Daimonin_Object_GetGod(Daimonin_Object *whoptr, PyObject* args)
{
    CFParm* CFR;
    static char* value;
    if (!PyArg_ParseTuple(args,""))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_DETERMINEGOD])(&GCFP);
    value = (char *)(CFR->Value[0]);
    free(CFR);
    return Py_BuildValue("s",value);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetGod                                           */
/* Python : object.SetGod(godname)                                           */
/* Status : Unfinished!                                                      */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetGod(Daimonin_Object *whoptr, PyObject* args)
{
    char* txt;
    const char* prayname;
    object* tmp;
    CFParm* CFR0;
    CFParm* CFR;
    int value;

    if (!PyArg_ParseTuple(args,"s", &txt))
        return NULL;

    prayname = add_string_hook("praying");

    GCFP1.Value[0] = (void *)(WHO);
    GCFP1.Value[1] = (void *)(prayname);

    GCFP2.Value[0] = (void *)(WHO);
    GCFP0.Value[0] = (char *)(txt);
    CFR0 = (PlugHooks[HOOK_FINDGOD])(&GCFP0);
    tmp = (object *)(CFR0->Value[0]);
    free(CFR0);
    GCFP2.Value[1] = (void *)(tmp);

    CFR = (PlugHooks[HOOK_CMDRSKILL])(&GCFP1);
    value = *(int *)(CFR->Value[0]);
    if (value)
        (PlugHooks[HOOK_BECOMEFOLLOWER])(&GCFP2);
    free(CFR);
    FREE_STRING_HOOK(prayname);
    Py_INCREF(Py_None);
    return Py_None;
}


/*****************************************************************************/
/* Name   : Daimonin_Object_TeleportTo                                       */
/* Python : object.TeleportTo(map, x, y, unique)                             */
/* Info   : Teleports object to the given position of map.                   */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_TeleportTo(Daimonin_Object *whoptr, PyObject* args)
{
/*    Daimonin_Map *map; */
    char *mapname;
    char *msg=NULL;
    int x, y, u = 0;

    if (!PyArg_ParseTuple(args,"sii|i", &mapname,&x,&y, &u))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (char *)(mapname);
    GCFP.Value[2] = (void *)(&x);
    GCFP.Value[3] = (void *)(&y);
    GCFP.Value[4] = (void *)(&u);
    GCFP.Value[5] = (char *)(msg);
    (PlugHooks[HOOK_TELEPORTOBJECT])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}


/*****************************************************************************/
/* Name   : Daimonin_Object_InsertInside                                     */
/* Python : object.InsertInside(where)                                       */
/* Info   : Inserts object into where.                                       */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_InsertInside(Daimonin_Object *whatptr, PyObject* args)
{
    Daimonin_Object *whereptr;
    object *myob;
    object *obenv;
    object *tmp;
    
    if (!PyArg_ParseTuple(args,"O!", &Daimonin_ObjectType, &whereptr))
        return NULL;

    myob = WHAT;
    obenv = myob->env;
    
    if (!QUERY_FLAG(myob,FLAG_REMOVED))
    {
        GCFP.Value[0] = (void *)(myob);
        (PlugHooks[HOOK_REMOVEOBJECT])(&GCFP);
    }
    
    myob = insert_ob_in_ob_hook(myob, WHERE);

    /* Make sure the inventory image/text is updated */
    /* FIXME: what if object was not carried by player ? */
    for(tmp = WHERE; tmp != NULL; tmp = tmp->env) {
        if (tmp->type == PLAYER) {
            GCFP.Value[0] = (void *)(tmp);
            GCFP.Value[1] = (void *)(myob);
            (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
            break;
        }
    }
    
    /* If we're taking from player. */
    for(tmp = obenv; tmp != NULL; tmp = tmp->env) {
        if (tmp->type == PLAYER) {
            GCFP.Value[0] = (void *)(tmp);
            GCFP.Value[1] = (void *)(tmp);
            (PlugHooks[HOOK_ESRVSENDINVENTORY])(&GCFP);
            break;
        }
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Apply                                            */
/* Python : object.Apply(what, flags)                                        */
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
static PyObject* Daimonin_Object_Apply(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whatptr;
    int flags;
    CFParm* CFR;
    int retval;

    if (!PyArg_ParseTuple(args,"O!i", &Daimonin_ObjectType, &whatptr,&flags))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(WHAT);
    GCFP.Value[2] = (void *)(&flags);
    CFR = (PlugHooks[HOOK_MANUALAPPLY])(&GCFP);
    retval = *(int *)(CFR->Value[0]);
    free(CFR);
    return Py_BuildValue("i",retval);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_PickUp                                           */
/* Python : object.PickUp(what)                                              */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* Daimonin_Object_PickUp(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whatptr;

    if (!PyArg_ParseTuple(args,"O!", &Daimonin_ObjectType, &whatptr))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(WHAT);
    (PlugHooks[HOOK_PICKUP])(&GCFP);
    /*pick_up(WHO,WHAT); */
    Py_INCREF(Py_None);
    return Py_None;
}


/*****************************************************************************/
/* Name   : Daimonin_Object_Drop                                             */
/* Python : object.Drop(what)                                                */
/* Info   : Equivalent to the player command "drop" (name is an object name, */
/*          "all", "unpaid", "cursed", "unlocked" or a count + object name : */
/*          "<nnn> <object name>", or a base name, or a short name...)       */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* Daimonin_Object_Drop(Daimonin_Object *whoptr, PyObject* args)
{
    char* name;
    CFParm* CFR; 

    if (!PyArg_ParseTuple(args,"s", &name))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(name);
    CFR = (PlugHooks[HOOK_CMDDROP])(&GCFP);
/*    command_drop(WHO,name); */
    free(CFR); 
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Take                                             */
/* Python : object.Take(name)                                                */
/* Status : Temporary disabled (see commands.c)                              */
/*****************************************************************************/

static PyObject* Daimonin_Object_Take(Daimonin_Object *whoptr, PyObject* args)
{
    char* name;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"s", &name))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(name);
    CFR = (PlugHooks[HOOK_CMDTAKE])(&GCFP);
    /* command_take(WHO,name); */
    free(CFR);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Communicate                                      */
/* Python : object.Communicate(message)                                      */
/* Info   : object says message to everybody on its map                      */
/*          but instead of CFSay it is parsed for other npc or magic mouth   */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_Communicate(Daimonin_Object *whoptr, PyObject* args)
{
    char *message;

    if (!PyArg_ParseTuple(args,"s", &message))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(message);

    (PlugHooks[HOOK_COMMUNICATE])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Say                                              */
/* Python : object.Say(message)                                              */
/* Info   : object says message to everybody on its map                      */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_Say(Daimonin_Object *whoptr, PyObject* args)
{
    char *message;
    static char buf[HUGE_BUF];
    int val, d= MAP_INFO_NORMAL,x,y, mode = 0;

    if (!PyArg_ParseTuple(args,"s|i", &message, &mode))
        return NULL;

	/* old dynamic buffer */
    /*buf = (char *)(malloc(sizeof(char)*(strlen(message)+strlen(query_name(who))+20)));*/

    val = NDI_NAVY|NDI_UNIQUE;
	x = WHO->x;
	y = WHO->y;

	if(mode)
		GCFP.Value[5] = (void *)(message);
	else
	{
		sprintf(buf, "%s says: %s", query_name(WHO),message);
		GCFP.Value[5] = (void *)(buf);
	}

    GCFP.Value[0] = (void *)(&val);
    GCFP.Value[1] = (void *)(WHO->map);
    GCFP.Value[2] = (void *)(&x);
    GCFP.Value[3] = (void *)(&y);
    GCFP.Value[4] = (void *)(&d);

    (PlugHooks[HOOK_NEWINFOMAP])(&GCFP);

    /*free(buf);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SayTo                                            */
/* Python : object.SayTo(target, message)                                    */
/* Info   : NPC talks only to player but map get a "xx talks to" msg too.    */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_SayTo(Daimonin_Object* whoptr, PyObject* args)
{    
    object *target;
    Daimonin_Object *obptr2;
	int zero = 0, mode = 0;
    char *message;
    static char buf[HUGE_BUF];
    int val,d= MAP_INFO_NORMAL,x,y;

    if (!PyArg_ParseTuple(args,"O!s|i", &Daimonin_ObjectType, &obptr2, &message, &mode))
        return NULL;

    target = obptr2->obj;

    /*buf = (char *)(malloc(sizeof(char)*(strlen(message)+strlen(query_name(who))+20)));*/
    
	if(mode)
		GCFP.Value[3] = (void *)(message);
	else /* thats default */
	{
		sprintf(buf, "%s talks to %s.", query_name(WHO),query_name(target));
		val = NDI_UNIQUE;
		x = WHO->x;
		y = WHO->y;

		GCFP.Value[0] = (void *)(&val);
		GCFP.Value[1] = (void *)(WHO->map);
		GCFP.Value[2] = (void *)(&x);
		GCFP.Value[3] = (void *)(&y);
		GCFP.Value[4] = (void *)(&d);
		GCFP.Value[5] = (void *)(WHO);
		GCFP.Value[6] = (void *)(target);
		GCFP.Value[7] = (void *)(buf);
		(PlugHooks[HOOK_NEWINFOMAPEXCEPT])(&GCFP);

		sprintf(buf, "%s says: %s", query_name(WHO),message);
		GCFP.Value[3] = (void *)(buf);

	}

	val = NDI_NAVY|NDI_UNIQUE;
	GCFP.Value[0] = (void *)(&val);
	GCFP.Value[1] = (void *)(&zero);
	GCFP.Value[2] = (void *)(target);
	(PlugHooks[HOOK_NEWDRAWINFO])(&GCFP);

		/*free(buf);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   :	Daimonin_Object_Write                                            */
/* Python : who.Write(message , color)                                       */
/* Info   : Writes a message to a specific player.                           */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_Write(Daimonin_Object *whoptr, PyObject* args)
{
    int   zero   = 0;
    char* message;
    int   color  = NDI_UNIQUE | NDI_ORANGE;

    if (!PyArg_ParseTuple(args,"s|i", &message,&color))
        return NULL;

    GCFP.Value[0] = (void *)(&color);
    GCFP.Value[1] = (void *)(&zero);
    GCFP.Value[2] = (void *)(WHO);
    GCFP.Value[3] = (void *)(message);

    (PlugHooks[HOOK_NEWDRAWINFO])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetGender                                        */
/* Python : object.SetGender(gender)                                         */
/* Info   : Changes the gender of object. gender_string should be one of     */
/*          Daimonin.NEUTER, Daimonin.MALE, Daimonin.GENDER_FEMALE or        */
/*          Daimonin.HERMAPHRODITE                                           */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetGender(Daimonin_Object *whoptr, PyObject* args)
{
    int gender;
    
    if (!PyArg_ParseTuple(args,"i",&gender))
        return NULL;

	/* set object to neuter */
	CLEAR_FLAG(WHO,FLAG_IS_MALE);
	CLEAR_FLAG(WHO,FLAG_IS_FEMALE);

	/* reset to male or female */
	if(gender&1)
		SET_FLAG(WHO,FLAG_IS_MALE);
	if(gender&2)
		SET_FLAG(WHO,FLAG_IS_FEMALE);

	/* update the players client of object was a player */
	if(WHO->type == PLAYER)
		WHO->contr->socket.ext_title_flag = 1; /* demand update to client */

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetRank                                          */
/* Python : object.SetRank(rank_string)                                      */
/* Info   : Set the rank of an object to rank_string                         */
/*          Rank string 'Mr' is special for no rank                          */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetRank(Daimonin_Object *whoptr, PyObject* args)
{
    object *walk;
    char *rank;
    
    if (!PyArg_ParseTuple(args,"s", &rank))
        return NULL;

	if(WHO->type != PLAYER)
	{
	    Py_INCREF(Py_None);
	    return Py_None;
	}
	
    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
        if (walk->name && !strcmp(walk->name,"RANK_FORCE") && !strcmp(walk->arch->name,"rank_force"))
        {
            /* we find the rank of the player, now change it to new one */
            if(walk->title)
                FREE_STRING_HOOK(walk->title);

            if (strcmp(rank,"Mr")) /* Mr = keyword to clear title and not add it as rank */
                walk->title = add_string_hook(rank);
            
            WHO->contr->socket.ext_title_flag = 1; /* demand update to client */
            return wrap_object(walk);
        }            
    }
    LOG(llevDebug,"Python Warning -> SetRank: Object %s has no rank_force!\n", query_name(WHO));

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetAlignment                                     */
/* Python : object.SetAlignment(alignment_string)                            */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetAlignment(Daimonin_Object *whoptr, PyObject* args)
{
    object *walk;
    char *align;
    
    if (!PyArg_ParseTuple(args,"s", &align))
        return NULL;

	if(WHO->type != PLAYER)
	{
	    Py_INCREF(Py_None);
	    return Py_None;
	}
    
    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
        if (walk->name && !strcmp(walk->name,"ALIGNMENT_FORCE")  && !strcmp(walk->arch->name,"alignment_force"))
        {
            /* we find the alignment of the player, now change it to new one */
			if(walk->title);
				FREE_STRING_HOOK(walk->title);
			walk->title = add_string_hook(align);

            WHO->contr->socket.ext_title_flag = 1; /* demand update to client */
            return wrap_object(walk);
        }            
    }
    LOG(llevDebug,"Python Warning -> SetAlignment: Object %s has no alignment_force!\n", query_name(WHO));
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetAlignmentForce                                */
/* Python : object.GetAlignmentForce()                                       */
/* Info   : This gets the aligment_force from a inventory (should be player?)*/
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetAlignmentForce(Daimonin_Object *whoptr, PyObject* args)
{
    object *walk;
    
    if (!PyArg_ParseTuple(args,""))
        return NULL;
    
	if(WHO->type != PLAYER)
	{
	    Py_INCREF(Py_None);
	    return Py_None;
	}

    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
        if (walk->name && !strcmp(walk->name,"ALIGNMENT_FORCE")  && !strcmp(walk->arch->name,"alignment_force"))
            return wrap_object(walk);
    }
    LOG(llevDebug,"Python Warning -> GetAlignmentForce: Object %s has no aligment_force!\n", query_name(WHO));
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetGuildForce                                    */
/* Python : object.SetGuildForce(rank_string)                                */
/* Info   : Sets the current rank of object to rank_string. Returns          */
/*          the guild_force object that was modified.                        */
/* Status : Stable                                                           */
/* Warning: This set only the title. The guild tag is in <slaying>           */
/*          For test of a special guild, you must use GetGuild()             */
/*          For settings inside a guild script, you can use this function    */
/*          Because it returns the guild_force obj after setting the title   */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetGuildForce(Daimonin_Object *whoptr, PyObject* args)
{
    object *walk;
    char *guild;
    
    if (!PyArg_ParseTuple(args,"s", &guild))
        return NULL;
    	
	if(WHO->type != PLAYER)
	{
	    Py_INCREF(Py_None);
	    return Py_None;
	}

    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
        if (walk->name && !strcmp(walk->name,"GUILD_FORCE") && !strcmp(walk->arch->name,"guild_force"))
        {
            /* we find the rank of the player, now change it to new one */
            if(walk->title)
                FREE_STRING_HOOK(walk->title);

            if (guild && strcmp(guild, ""))
                walk->title = add_string_hook(guild);
            
            WHO->contr->socket.ext_title_flag = 1; /* demand update to client */
            return wrap_object(walk);
        }            
    }
    LOG(llevDebug,"Python Warning -> SetGuild: Object %s has no guild_force!\n", query_name(WHO));
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetGuildForce                                    */
/* Python : object.GetGuildForce()                                           */
/* Info   : This gets the guild_force from a inventory (should be player?)   */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetGuildForce(Daimonin_Object *whoptr, PyObject* args)
{
    object *walk;
    
    if (!PyArg_ParseTuple(args,""))
        return NULL;
    
	if(WHO->type != PLAYER)
	{
	    Py_INCREF(Py_None);
	    return Py_None;
	}

    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
        if (walk->name && !strcmp(walk->name,"GUILD_FORCE") && !strcmp(walk->arch->name,"guild_force"))
            return wrap_object(walk);
    }
    
    LOG(llevDebug,"Python Warning -> GetGuild: Object %s has no guild_force!\n", query_name(WHO));
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Fix                                              */
/* Python : object.Fix()                                                     */
/* Info   : Recalculates a player's or monster's stats depending on          */
/*          equipment, forces, skills etc.                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* Daimonin_Object_Fix(Daimonin_Object *whoptr, PyObject* args)
{
    if (!PyArg_ParseTuple(args,""))
        return NULL;

    fix_player_hook(WHO);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Kill                                             */
/* Python : object.Kill(what, how)                                           */
/* Status : Untested                                                         */
/*****************************************************************************/
/* add hooks before use! */

static PyObject* Daimonin_Object_Kill(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whatptr;
    int ktype;
    int k = 1;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"O!i", &Daimonin_ObjectType, &whatptr,&ktype))
        return NULL;

    WHAT->speed = 0;
    WHAT->speed_left = 0.0;
    GCFP.Value[0] = (void *)(WHAT);
    (PlugHooks[HOOK_UPDATESPEED])(&GCFP);
    /* update_ob_speed(WHAT); */

    if(QUERY_FLAG(WHAT,FLAG_REMOVED))
    {
        LOG(llevDebug, "Warning (from KillObject): Trying to remove removed object\n");
        RAISE("Trying to remove removed object");
    }
    else
    {
        WHAT->stats.hp = -1;
        GCFP.Value[0] = (void *)(WHAT);
        GCFP.Value[1] = (void *)(&k);
        GCFP.Value[2] = (void *)(WHO);
        GCFP.Value[3] = (void *)(&ktype);

        CFR = (PlugHooks[HOOK_KILLOBJECT])(&GCFP);
        free(CFR);
        /*kill_object(killed,1,killer, type); */
    }
   /* This is to avoid the attack routine to continue after we called
    * killObject, since the attacked object no longer exists.
    * By fixing guile_current_other to NULL, guile_use_weapon_script will
    * return -1, meaning the attack function must be immediately terminated.
    */
    if (WHAT==StackOther[StackPosition])
    {
        StackOther[StackPosition] = NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CastAbility                                      */
/* Python : caster.CastAbility(target,spellno,mode,direction,option)         */
/* Info   : caster casts the ability numbered spellno on target.             */
/*          mode = Daimonin.CAST_NORMAL or Daimonin.CAST_POTION              */
/*          direction is the direction to cast the ability in                */
/*          option is additional string option(s)                            */
/*          FIXME: only allows for directional abilities?                    */
/*          Abilities are can be cast in magic-blocking areas, and do not    */
/*          use magicattack.                                                 */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_CastAbility(Daimonin_Object *whoptr, PyObject* args)
{
	Daimonin_Object *target;
    int spell;
    int dir;
	int mode;
    char* op;
    CFParm* CFR;
    int parm=1;
    int parm2;
    int typeoffire = FIRE_DIRECTIONAL;

	if (!PyArg_ParseTuple(args,"O!iiis", &Daimonin_ObjectType, &target, &spell, &mode, &dir, &op))
        return NULL;

	if(WHO->type != PLAYER)
		parm2 = spellNPC;
	else
	{
		if(!mode)
			parm2 = spellNormal;
		else
			parm2 = spellPotion;
	}

    GCFP.Value[0] = (void *)(target->obj);
    GCFP.Value[1] = (void *)(WHO);
    GCFP.Value[2] = (void *)(&dir);
    GCFP.Value[3] = (void *)(&spell);
    GCFP.Value[4] = (void *)(&parm);
    GCFP.Value[5] = (void *)(&parm2);
    GCFP.Value[6] = (void *)(op);
    GCFP.Value[7] = (void *)(&typeoffire);
    CFR = (PlugHooks[HOOK_CASTSPELL])(&GCFP);
    
    free(CFR);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CastSpell                                        */
/* Python : caster.CastSpell(target,spell,mode,direction,option)             */
/* Info   : caster casts the spell numbered spellno on target.               */
/*          mode = Daimonin.CAST_NORMAL or Daimonin.CAST_POTION              */
/*          direction is the direction to cast the spell in                  */
/*          option is additional string option(s)                            */
/*          NPCs can cast spells even in no-spell areas.                     */
/*          FIXME: only allows for directional spells                        */
/*          FIXME: is direction/position relative to target? (0 = self)      */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* Daimonin_Object_CastSpell(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *target;
    int spell;
    int dir;
	int mode;
    char* op;
    CFParm* CFR;
    int parm=0;
    int parm2;
    int typeoffire = FIRE_DIRECTIONAL;

    if (!PyArg_ParseTuple(args,"O!iiis", &Daimonin_ObjectType, &target, 
                &spell, &mode, &dir, &op))
        return NULL;

	if(WHO->type != PLAYER)
		parm2 = spellNPC;
	else
	{
        if(!mode)
            parm2 = spellNormal;
        else
            parm2 = spellPotion;
    }

    GCFP.Value[0] = (void *)(target->obj);
    GCFP.Value[1] = (void *)(WHO);
    GCFP.Value[2] = (void *)(&dir);
    GCFP.Value[3] = (void *)(&spell);
    GCFP.Value[4] = (void *)(&parm);
    GCFP.Value[5] = (void *)(&parm2);
    GCFP.Value[6] = (void *)(op);
    GCFP.Value[7] = (void *)(&typeoffire);
    CFR = (PlugHooks[HOOK_CASTSPELL])(&GCFP);

    free(CFR);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_DoKnowSpell                                      */
/* Python : object.DoKnowSpell(spell)                                        */
/* Info   : 1 if the spell is known by object, 0 if it isn't                 */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_DoKnowSpell(Daimonin_Object *whoptr, PyObject* args)
{
    int spell;
    CFParm* CFR;
    int value;

    if (!PyArg_ParseTuple(args,"i",&spell))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&spell);
    CFR = (PlugHooks[HOOK_CHECKFORSPELL])(&GCFP);
    value = *(int *)(CFR->Value[0]);
    free(CFR);
    return Py_BuildValue("i",value);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_AcquireSpell                                     */
/* Python : object.AcquireSpell(spell, mode)                                 */
/* Info   : object will learn or unlearn spell.                              */
/*          mode: Daimonin.LEARN or Daimonin.UNLEARN                         */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_AcquireSpell(Daimonin_Object *whoptr, PyObject* args)
{
    int spell;
    int mode;

    if (!PyArg_ParseTuple(args,"ii", &spell, &mode))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&spell);
    GCFP.Value[2] = (void *)(&mode);
    (PlugHooks[HOOK_LEARNSPELL])(&GCFP);   
    
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_DoKnowSkill                                      */
/* Python : object.DoKnowSkill(skill)                                        */
/* Info   : 1 if the skill is known by object, 0 if it isn't                 */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_DoKnowSkill(Daimonin_Object *whoptr, PyObject* args)
{
    int skill;
    CFParm* CFR;
    int value;

    if (!PyArg_ParseTuple(args,"i", &skill))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&skill);
    CFR = (PlugHooks[HOOK_CHECKFORSKILLKNOWN])(&GCFP);
    value = *(int *)(CFR->Value[0]);
    free(CFR);
    return Py_BuildValue("i",value);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_AcquireSkill                                     */
/* Python : object.AcquireSkill(skillno, mode)                               */
/* Info   : object will learn or unlearn skill.                              */
/*          mode: Daimonin.LEARN or Daimonin.UNLEARN                         */
/*          Get skill number with Daimonin.GetSkillNr()                      */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_AcquireSkill(Daimonin_Object *whoptr, PyObject* args)
{
    int skill, mode;
    
    if (!PyArg_ParseTuple(args,"ii", &skill, &mode))
        return NULL;
            
    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&skill);
    GCFP.Value[2] = (void *)(&mode);
    (PlugHooks[HOOK_LEARNSKILL])(&GCFP);   
    
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_FindMarkedObject                                 */
/* Python : object.FindMarkedObject()                                        */
/* Info   : Returns the marked object in object's inventory, or None if no   */
/*          object is marked.                                                */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_FindMarkedObject(Daimonin_Object* whoptr, PyObject* args)
{
    object * value;
    CFParm* CFR;
    
    if (!PyArg_ParseTuple(args,""))
        return NULL;
            
    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_FINDMARKEDOBJECT])(&GCFP);   
    
    value = (object *)(CFR->Value[0]);
    /*free(CFR); findmarkedobject use static parameters */
    return wrap_object(value);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CheckInvisibleInside                             */
/* Python : object.CheckInvisibleInside(id)                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* Daimonin_Object_CheckInvisibleInside(Daimonin_Object *whoptr, PyObject* args)
{
    char *id;
    object* tmp2;

    if (!PyArg_ParseTuple(args,"s", &id))
        return NULL;

    for(tmp2=WHO->inv;tmp2 !=NULL; tmp2=tmp2->below)
    {
        if(tmp2->type == FORCE && tmp2->slaying && !strcmp(tmp2->slaying,id))
            break;
    }

    return wrap_object(tmp2);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CreatePlayerForce                                */
/* Python : object.CreatePlayerForce(force_name, time)                       */
/* Info   : Creates and insters a player force named force_name in object.   */
/*          The values of a player force will effect the player.             */
/*          If time is given and > 0, the force will be removed again after  */
/*          time/0.02 ticks.                                                 */
/* Status : Stable.                                                          */
/*****************************************************************************/
static PyObject* Daimonin_Object_CreatePlayerForce(Daimonin_Object *whereptr, PyObject* args)
{
    char* txt;
    char txt2[16] = "player_force";
    object *myob;
    CFParm* CFR;
    int time = 0;
    
    if (!PyArg_ParseTuple(args,"s|i",&txt, &time))
        return NULL;
    
    GCFP.Value[0] = (void *)(txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
    /*myob = get_archetype("player_force"); */
    myob = (object *)(CFR->Value[0]);
    free(CFR);
    
    if(!myob)
    {
        LOG(llevDebug,"Python WARNING:: CreatePlayerForce: Can't find archtype 'player_force'\n");
        RAISE("Can't find archtype 'player_force'");
    }
   
    /* For temporary forces */
    if(time > 0) {
        SET_FLAG(myob, FLAG_IS_USED_UP);
        myob->stats.food = time;
        myob->speed = 0.02f;
        GCFP.Value[0] = (void *)(myob);
        (PlugHooks[HOOK_UPDATESPEED])(&GCFP);
    }
    
    /* setup the force and put it in activator */
 	if(myob->name);
		FREE_STRING_HOOK(myob->name);
	myob->name = add_string_hook(txt);
    myob = insert_ob_in_ob_hook(myob, WHERE);

    /*esrv_send_item((object *)(gh_scm2long(where)), myob); */
    GCFP.Value[0] = (void *)(WHERE);
    GCFP.Value[1] = (void *)(myob);
    (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);

    return wrap_object(myob);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CheckQuestObject                                   */
/* Python : object.CheckQuestObject(name)                                      */
/* Status : Stable                                                           */
/* Info   : We get and check the player has a misc'ed quest object           */
/*        : If so, the player has usally solved this quest before.           */
/*****************************************************************************/
static PyObject* Daimonin_Object_CheckQuestObject(Daimonin_Object *whoptr, PyObject* args)
{
    char *arch_name;
    char *name;
    object *walk;
    
    if (!PyArg_ParseTuple(args,"ss", &arch_name, &name))
        return NULL;

	/* lets first check the inventory for the quest_container object */
    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
		if(walk->type == TYPE_QUEST_CONTAINER)
		{
			/* now lets check our quest item is inside.
			 * we use arch name and object name as id, if needed we
			 * arch name is stored in the race field of the quest dummies.
			 * can add more here
			 */
			for(walk=walk->inv;walk!=NULL;walk=walk->below)
			{
				if (walk->race && !strcmp(walk->race,arch_name) &&  walk->name && !strcmp(walk->name,name))
					return wrap_object(walk);
			}
			break;
		}
    }

    Py_INCREF(Py_None);
    return Py_None; /* there was non */
}

/*****************************************************************************/
/* Name   : Daimonin_Object_AddQuestObject                                   */
/* Python : object.AddQuestObject(name)                                      */
/* Status : Stable                                                           */
/* Info   : Add the misc'ed quest object to players quest container.         */
/*        : create the quest container if needed                             */
/*****************************************************************************/
static PyObject* Daimonin_Object_AddQuestObject(Daimonin_Object *whoptr, PyObject* args)
{
    char *arch_name;
    char *name;
    object *walk, *myob;
    CFParm* CFR;
	char txt2[32];
    
    if (!PyArg_ParseTuple(args,"ss", &arch_name, &name))
        return NULL;

	/* lets first check the inventory for the quest_container object */
    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
		if(walk->type == TYPE_QUEST_CONTAINER)
			break;
    }

	if(!walk) /* no quest container, create it */
	{
	    strcpy(txt2,"quest_container");
    
		GCFP.Value[0] = (void *)(txt2);
		CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
		walk = (object *)(CFR->Value[0]);
		free(CFR);
       
		if(!walk)
		{
			LOG(llevDebug,"Python WARNING:: AddQuestObject: Cant't find archtype 'quest_container'\n");
			RAISE("Cant't find archtype 'quest_container'");
		}

		insert_ob_in_ob_hook(walk, WHO);
	}

	strcpy(txt2,"player_info");
    
    GCFP.Value[0] = (void *)(txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
    myob = (object *)(CFR->Value[0]);
    free(CFR);
       
    if(!myob)
    {
        LOG(llevDebug,"Python WARNING:: AddQuestObject: Cant't find archtype 'player_info'\n");
        RAISE("Cant't find archtype 'player_info'");
    }

	/* store name & arch name of the quest obj. so we can id it later */
   	if(myob->name);
		FREE_STRING_HOOK(myob->name);
	myob->name = add_string_hook(name);

   	if(myob->race);
		FREE_STRING_HOOK(myob->race);
	myob->race = add_string_hook(arch_name);

	myob = insert_ob_in_ob_hook(myob, walk);

    Py_INCREF(Py_None);
    return Py_None; /* there was non */
}


/*****************************************************************************/
/* Name   : Daimonin_Object_CreatePlayerInfo                                 */
/* Python : object.CreatePlayerInfo(name)                                    */
/* Status : Stable                                                           */
/* Info   : Creates a player_info object of specified name in object's       */
/*          inventory                                                        */
/*          The Values of a player_info object will NOT effect the player.   */
/*          Returns the created object                                       */
/*****************************************************************************/
static PyObject* Daimonin_Object_CreatePlayerInfo(Daimonin_Object *whereptr, PyObject* args)
{
    char* txt;
    char txt2[16];
    object *myob;
    CFParm* CFR;
    
    if (!PyArg_ParseTuple(args, "s", &txt))
        return NULL;
    
    strcpy(txt2,"player_info");
    
    GCFP.Value[0] = (void *)(txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
    myob = (object *)(CFR->Value[0]);
    free(CFR);
       
    if(!myob)
    {
        LOG(llevDebug,"Python WARNING:: CreatePlayerInfo: Cant't find archtype 'player_info'\n");
        RAISE("Cant't find archtype 'player_info'");
    }
    
    /* setup the info and put it in activator */
   	if(myob->name);
		FREE_STRING_HOOK(myob->name);
	myob->name = add_string_hook(txt);
    myob = insert_ob_in_ob_hook(myob, WHERE);
    
    /*esrv_send_item((object *)(gh_scm2long(where)), myob); */
    GCFP.Value[0] = (void *)(WHERE);
    GCFP.Value[1] = (void *)(myob);
    (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
    
    return wrap_object(myob);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetPlayerInfo                                    */
/* Python : object.GetPlayerInfo(name)                                       */
/* Status : Stable                                                           */
/* Info   : get first player_info with the specified name in who's inventory */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetPlayerInfo(Daimonin_Object *whoptr, PyObject* args)
{
    char *name;
    object *walk;
    
    if (!PyArg_ParseTuple(args,"s", &name))
        return NULL;

    /* get the first linked player_info arch in this inventory */
    for(walk=WHO->inv;walk!=NULL;walk=walk->below)
    {
        if (walk->name && !strcmp(walk->arch->name,"player_info") &&  !strcmp(walk->name,name))
            return wrap_object(walk);
    }

    Py_INCREF(Py_None);
    return Py_None; /* there was non */
}


/*****************************************************************************/
/* Name   : Daimonin_Object_GetNextPlayerInfo                                */
/* Python : object.GetNextPlayerInfo(player_info)                            */
/* Status : Stable                                                           */
/* Info   : get next player_info in who's inventory with same name as        */
/*          player_info                                                      */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetNextPlayerInfo(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *myob;
    char name[128];
    object *walk;
    
    if (!PyArg_ParseTuple(args,"O!", &Daimonin_ObjectType, &myob))
        return NULL;

    /* thats our check paramters: arch "force_info", name of this arch */
    strncpy(name, myob->obj->name, 127); /* 127 chars should be enough for all */
    name[63] = '\0';

    /* get the next linked player_info arch in this inventory */
    for(walk=myob->obj->below;walk!=NULL;walk=walk->below)
    {
        if (walk->name && !strcmp(walk->arch->name,"player_info") &&  !strcmp(walk->name,name))
            return wrap_object(walk);
    }

    Py_INCREF(Py_None);
    return Py_None; /* there was non left */
}


/*****************************************************************************/
/* Name   : Daimonin_Object_CreateInvisibleInside                            */
/* Python : object.CreateInvisibleInside(id)                                 */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* Daimonin_Object_CreateInvisibleInside(Daimonin_Object *whereptr, PyObject* args)
{
    char* txt;
    char txt2[6];
    object *myob;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"s",&txt))
        return NULL;

    strcpy(txt2,"force");

    GCFP.Value[0] = (void *)(txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);

    /*myob = get_archetype("force"); */
    myob = (object *)(CFR->Value[0]);
    free(CFR);

    if(!myob)
    {
        LOG(llevDebug,"Python WARNING:: CFCreateInvisibleInside: Can't find archtype 'force'\n");
        RAISE("Cant't find archtype 'force'");
    }
    myob->speed = 0.0;
    GCFP.Value[0] = (void *)(myob);
    (PlugHooks[HOOK_UPDATESPEED])(&GCFP);

    /*update_ob_speed(myob); */
   	if(myob->slaying);
		FREE_STRING_HOOK(myob->slaying);
	myob->slaying = add_string_hook(txt);
    myob = insert_ob_in_ob_hook(myob, WHERE);

    GCFP.Value[0] = (void *)(WHERE);
    GCFP.Value[1] = (void *)(myob);
  /*esrv_send_item((object *)(gh_scm2long(where)), myob); */
    (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
    return wrap_object(myob);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CreateObjectInside                               */
/* Python : object.CreateObjectInside(archname, identified, value)           */
/* Info   : Creates an object from archname and inserts into object.         */
/*          identified is either Daimonin.IDENTIFIED or Daimonin.UNIDENTIFIED*/
/*          If value is >= 0 it will be used as the new object's value,      */
/*          otherwise the value will be taken from the arch.                 */
/* Status : Stable                                                           */
/*****************************************************************************/
/* i must change this a bit - only REAL arch names - not object names */

static PyObject* Daimonin_Object_CreateObjectInside(Daimonin_Object *whereptr, PyObject* args)
{
    object *myob, *tmp;
	long value=-1, id, nrof=1;
    char *txt;
/*    char *tmpname;
    object *test;
    int i;*/
    CFParm* CFR;

	/* 0: name
	   1: object we want give <name> 
	   2: if 1, set FLAG_IDENTIFIED
	   3: nr of objects to create: 0 and 1 don't change default nrof setting
	   3: if not -1, use it for myob->value
	   */

    if (!PyArg_ParseTuple(args,"sll|l",&txt, &id, &nrof, &value))
        return NULL;

    GCFP.Value[0] = (void *)(txt);
    CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
    myob = (object *)(CFR->Value[0]);
	free(CFR);

	if(!myob)
	{
		LOG(llevDebug,"BUG python_CFCreateObjectInside(): ob:>%s< = NULL!\n", query_name(myob));
        RAISE("Failed to create the object. Did you use an existing arch?");
	}

	if(value != -1) /* -1 means, we use original value */
		myob->value = (sint32) value;
	if(id)
		SET_FLAG(myob,FLAG_IDENTIFIED);
	if(nrof>1)
		myob->nrof = nrof;

    myob = insert_ob_in_ob_hook(myob, WHERE);
    
    /* Make sure inventory image/text is updated */
    for(tmp = WHERE; tmp != NULL; tmp = tmp->env) {
        if (tmp->type == PLAYER) {
            GCFP.Value[0] = (void *)(tmp);
            GCFP.Value[1] = (void *)(myob);
            (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
        }
    }
    
    return wrap_object(myob);
}


/* help function for Daimonin_Object_CheckInventory
 * to recursive check object inventories.
 */
static object* object_check_inventory_rec(object *tmp, int mode, char* arch_name, char *name, char *title, int type)
{
	object *tmp2;

	while(tmp)
	{

		if( (!name||(tmp->name && !strcmp(tmp->name,name))) &&
					(!title||(tmp->title && !strcmp(tmp->title,title))) &&
					(!arch_name||(tmp->arch && tmp->arch->name && !strcmp(tmp->arch->name,arch_name))) &&
					(type == -1 || tmp->type == type))
            return tmp;

		if(mode == 2 || mode && tmp->type == CONTAINER) 
		{
			if((tmp2 = object_check_inventory_rec(tmp->inv,mode, arch_name,name,title,type)))
	            return tmp2;
		}

		tmp = tmp->below;
	}

	return NULL;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_CheckInventory                                   */
/* Python : object.CheckInventory(arch name, object name, type)              */
/* Info   : returns the first found object with the specified name if found  */
/*          in object's inventory, or None if it wasn't found.               */
/*        : arch or object name == NULL will be ignored for search			 */
/*          also type == -1                                                  */
/*		  : mode: 0=only inventory, 1: inventory and container, 2: all inv.  */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_CheckInventory(Daimonin_Object *whoptr, PyObject* args)
{
	int type = -1, mode = 0;
    char *name = NULL, *title=NULL, *arch_name = NULL;
    object *tmp, *tmp2;

    if (!PyArg_ParseTuple(args,"iz|zzi", &mode, &arch_name, &name, &title, &type))
        return NULL;

    tmp = WHO->inv;
    
	while (tmp)
	{
		if( (!name||(tmp->name && !strcmp(tmp->name,name))) &&
					(!title||(tmp->title && !strcmp(tmp->title,title))) &&
					(!arch_name||(tmp->arch && tmp->arch->name && !strcmp(tmp->arch->name,arch_name))) &&
					(type == -1 || tmp->type == type))
            return wrap_object(tmp);

		if(mode == 2 || mode && tmp->type == CONTAINER) 
		{
			if((tmp2 = object_check_inventory_rec(tmp->inv,mode, arch_name,name,title,type)))
	            return wrap_object(tmp2);
		}

		tmp = tmp->below;
	}

    Py_INCREF(Py_None);
    return Py_None; /* we don't find a arch with this arch_name in the inventory */
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetSaveBed                                       */
/* Python : object.SetSaveBed(map, x, y)                                     */
/* Info   : Sets the current savebed position for object to the specified    */
/*          coordinates on the map.                                          */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetSaveBed(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Map *map;
    int x,y;
    
    if (!PyArg_ParseTuple(args,"O!ii", &Daimonin_MapType, &map, &x, &y))
        return NULL;
	
    if(WHO->type == PLAYER)
	{	
		strcpy(WHO->contr->savebed_map, map->map->path);
		WHO->contr->bed_x = x;
		WHO->contr->bed_y = y;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Remove                                           */
/* Python : object.Remove()                                                  */
/* Info   : Permanently removes object from the game.                        */
/* Status : Tested                                                           */
/*****************************************************************************/
/* Gecko  : This function is DANGEROUS. Added limitations on what can be     */
/*          removed to avoid some of the problems                            */
/*****************************************************************************/
/* hm, this should be named delete or free object... */
static PyObject* Daimonin_Object_Remove(Daimonin_Object *whoptr, PyObject* args)
{
    object* myob;
    object* obenv;

    if (!PyArg_ParseTuple(args,""))
        return NULL;

    myob = WHO;
    obenv = myob->env;
    
    /* Gecko: Don't allow removing any of the involved objects. Messes things up... */
    if (StackActivator[StackPosition] == myob ||
            StackWho[StackPosition] == myob ||
            StackOther[StackPosition] == myob)
    {
        RAISE("You are not allowed to remove one of the active objects. Workaround using CFTeleport or some other solution.");
    }
    
    GCFP.Value[0] = (void *)(myob);
    (PlugHooks[HOOK_REMOVEOBJECT])(&GCFP);

    /* Gecko: player inventory can be removed even if the activator is not a player */
    if(obenv != NULL && obenv->type == PLAYER)
    {
        GCFP.Value[0] = (void *)(obenv);
        GCFP.Value[1] = (void *)(obenv);
        (PlugHooks[HOOK_ESRVSENDINVENTORY])(&GCFP);
    }
    /*    if (StackActivator[StackPosition]->type == PLAYER)
    {
        GCFP.Value[0] = (void *)(StackActivator[StackPosition]);
        GCFP.Value[1] = (void *)(StackActivator[StackPosition]);
        (PlugHooks[HOOK_ESRVSENDINVENTORY])(&GCFP);
    }*/
    GCFP.Value[0] = (void *)(myob);
    (PlugHooks[HOOK_FREEOBJECT])(&GCFP);
    
    /* Gecko: Handle removing any of the active objects (e.g. the activator) */
    if (StackActivator[StackPosition] == myob)
        StackActivator[StackPosition] = NULL;
    if (StackWho[StackPosition] == myob)
        StackWho[StackPosition] = NULL;
    if (StackOther[StackPosition] == myob)
        StackOther[StackPosition] = NULL;
    
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetPosition                                      */
/* Python : object.SetPosition(x, y)                                         */
/* Info   : Cannot be used to move objects out of containers. (Use Drop() or */
/*          TeleportTo() for that)                                           */
/* Status : Tested                                                           */
/*****************************************************************************/

/* TODO: Useful for setting X/Y in non-active objects too? */

/* FIXME: if the object moved was triggered by SAY event and it is moved to a tile
 * within the listening radius, it will be triggered again, and again... */

static PyObject* Daimonin_Object_SetPosition(Daimonin_Object *whoptr, PyObject* args)
{
    int x, y, k;
    CFParm* CFR;
    k = 0;

    if (!PyArg_ParseTuple(args,"ii", &x,&y))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&x);
    GCFP.Value[2] = (void *)(&y);
    GCFP.Value[3] = (void *)(&k);
    GCFP.Value[4] = (void *)(NULL);
    GCFP.Value[5] = (void *)(NULL);

    CFR = (PlugHooks[HOOK_TRANSFEROBJECT])(&GCFP);

/*  transfer_ob(WHO, gh_scm2int(X), gh_scm2int(Y), 0, NULL, NULL); */

    free(CFR);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_IdentifyItem                                     */
/* Python : caster.IdentifyItem(target, marked, mode)                        */
/* Info   : caster identifies object(s) in target's inventory.               */
/*          mode: Daimonin.IDENTIFY_NORMAL, Daimonin.IDENTIFY_ALL or         */
/*          Daimonin.IDENTIFY_MARKED                                         */
/*          marked must be None for IDENTIFY_NORMAL and IDENTIFY_ALL         */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_IdentifyItem(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *target;
    PyObject *ob;
    object *marked = NULL;
    long mode;

    if (!PyArg_ParseTuple(args,"O!Ol", &Daimonin_ObjectType, &target, &ob, &mode)) 
        return NULL;
    
    if(mode == 2) {
        if(! PyObject_TypeCheck(ob, &Daimonin_ObjectType)) 
            RAISE("Parameter 3 must be a Daimonin.Object for mode IDENTIFY_MARKED");
        marked = ((Daimonin_Object *)ob)->obj;
    } else if (mode == 0 || mode == 1) {
        if(ob != Py_None) 
            RAISE("Parameter 3 must be None for modes IDENTIFY_NORMAL and IDENTIFY_ALL");
    } else
        RAISE("Mode must be IDENTIFY_NORMAL, IDENTIFY_ALL or IDENTIFY_MARKED");

    GCFP.Value[0] = (void *)WHO;
    GCFP.Value[1] = (void *)target->obj;
    GCFP.Value[2] = (void *)marked; /* is used when we use mode == 2 */
    GCFP.Value[3] = (void *)&mode;
    (PlugHooks[HOOK_IDENTIFYOBJECT])(&GCFP);   

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_IsOfType                                         */
/* Python : object.IsOfType(type)                                            */
/* Info   : returns 1 if object is of the specified type, or 0 otherwise.    */
/*          (e.g. Daimonin.TYPE_MONSTER for monster/NPC, or                  */
/*          Daimonin.TYPE_PLAYER for players)                                */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_IsOfType(Daimonin_Object *whoptr, PyObject* args)
{
    int type;

    if (!PyArg_ParseTuple(args,"i", &type))
        return NULL;
    
    return Py_BuildValue("i",WHO->type == type ? 1 : 0);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Save                                             */
/* Python : object.Save()                                                    */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* Daimonin_Object_Save(Daimonin_Object *whoptr, PyObject* args)
{
    static char *result;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_DUMPOBJECT])(&GCFP);
    result = (char *)(CFR->Value[0]);
    free(CFR);

    return Py_BuildValue("s",result);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetIP                                            */
/* Python : object.GetIP()                                                   */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetIP(Daimonin_Object *whoptr, PyObject* args)
{
    static char *result;

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

	if(WHO->type != PLAYER)
	{
	    Py_INCREF(Py_None);
	    return Py_None;
	}

    if (WHO->contr)
    {
        result = WHO->contr->socket.host;
        return Py_BuildValue("s",result);
    }
    else
    {
        LOG(llevDebug, "PYTHON - Error - This object has no controller\n");
        return Py_BuildValue("s","");
    }
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetArchName                                      */
/* Python : object.GetArchName()                                             */
/* Info   :                                                                  */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetArchName(Daimonin_Object *whoptr, PyObject* args)
{
    if (!PyArg_ParseTuple(args,""))
        return NULL;
    return Py_BuildValue("s",WHO->arch->name);
}


/*****************************************************************************/
/* Name   : Daimonin_Object_ShowCost                                         */
/* Python : buyer.ShowCost(value)                                            */
/* Info   : Returns a string describing value as x gold, x silver, x copper  */
/*        : cost string comes from shop.c and is temporary STATIC            */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_ShowCost(Daimonin_Object *whoptr, PyObject* args)
{
    int value;
	char *cost_string;

    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"i", &value))
        return NULL;
    GCFP.Value[0] = (void *)(&value);
    CFR = (PlugHooks[HOOK_SHOWCOST])(&GCFP);
    cost_string=(char *)(CFR->Value[0]);
    return Py_BuildValue("s",cost_string);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetItemCost                                      */
/* Python : buyer.GetItemCost(object,type)                                   */
/* Info   : type is one of Daimonin.COST_TRUE, COST_BUY or COST_SELL         */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* Daimonin_Object_GetItemCost(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whatptr;
    int flag;
    int cost;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"O!i", &Daimonin_ObjectType, &whatptr,&flag))
        return NULL;
    GCFP.Value[0] = (void *)(WHAT);
    GCFP.Value[1] = (void *)(WHO);
    GCFP.Value[2] = (void *)(&flag);
    CFR = (PlugHooks[HOOK_QUERYCOST])(&GCFP);
    cost=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",cost);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetMoney                                         */
/* Python : buyer.GetMoney()                                                 */
/* Info   : returns the amount of money the object carries in copper         */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* Daimonin_Object_GetMoney(Daimonin_Object *whoptr, PyObject* args)
{
    int amount;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,""))
        return NULL;
    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_QUERYMONEY])(&GCFP);
    amount=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",amount);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_PayForItem                                       */
/* Python : buyer.PayForItem(object)                                         */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* Daimonin_Object_PayForItem(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whatptr;
    int val;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"O!", &Daimonin_ObjectType, &whatptr))
        return NULL;
    GCFP.Value[0] = (void *)(WHAT);
    GCFP.Value[1] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_PAYFORITEM])(&GCFP);
    val=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",val);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_PayAmount                                        */
/* Python : buyer.PayAmount(value)                                           */
/* Info   : If buyer has enough money, value copper will be deducted from    */
/*          buyer, and 1 will be returned. Otherwise returns 0               */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_PayAmount(Daimonin_Object *whoptr, PyObject* args)
{
    int to_pay;
    int val;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"i", &to_pay))
        return NULL;
    GCFP.Value[0] = (void *)(&to_pay);
    GCFP.Value[1] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_PAYFORAMOUNT])(&GCFP);
    val=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",val);
}


/*****************************************************************************/
/* Name   : Daimonin_Object_SendCustomCommand                                */
/* Python : object.SendCustomCommand(customcommand)                          */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* Daimonin_Object_SendCustomCommand(Daimonin_Object *whoptr, PyObject* args)
{
    char *customcmd;

    if (!PyArg_ParseTuple(args,"s", &customcmd))
        return NULL;
    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(customcmd);
    (PlugHooks[HOOK_SENDCUSTOMCOMMAND])(&GCFP);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_Clone                                            */
/* Python : object.Clone(mode)                                               */
/* Info   : mode = Daimonin.CLONE_WITH_INVENTORY (default) or                */
/*          Daimonin.CLONE_WITHOUT_INVENTORY                                 */
/*          You _MUST_ do something with the clone (TeleportTo() or          */
/*          InsertInside() are useful functions for this)                    */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Object_Clone(Daimonin_Object *whoptr, PyObject* args)
{
    CFParm* CFR;
    int mode = 0;
    object *clone;
    
    if (!PyArg_ParseTuple(args,"|i", &mode))
        return NULL;
    
    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&mode);
    
    CFR = (PlugHooks[HOOK_CLONEOBJECT])(&GCFP);

    clone = (object *)(CFR->Value[0]);    
    free(CFR);

    if(clone->type == PLAYER || QUERY_FLAG(clone, FLAG_IS_PLAYER)) {
        clone->type = MONSTER;
        CLEAR_FLAG(clone, FLAG_IS_PLAYER);
    }
    
    return wrap_object(clone);
}


/*****************************************************************************/
/* Name   : Daimonin_Object_GetUnmodifiedAttribute                           */
/* Python : object.GetUnmodifiedAttribute(attribute_id)                      */
/* Status : UNFINISHED <- fields not available...                            */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetUnmodifiedAttribute(Daimonin_Object* whoptr, PyObject* args)
{
    int fieldno;

    if (!PyArg_ParseTuple(args,"i", &fieldno))
        return NULL;
   
    if(fieldno < 0 || fieldno >= NUM_OBJFIELDS)
        RAISE("Illegal field ID"); 
    
    if(WHO->type != PLAYER)
        RAISE("Can only be used on players");
    
    RAISE("Not implemented");
#if 0    
    switch(fieldno) {
        case OBJFIELD_STAT_INT: return Py_BuildValue("i", WHO->contr->orig_stats.Int);
        case OBJFIELD_STAT_STR: return Py_BuildValue("i", WHO->contr->orig_stats.Str);
        case OBJFIELD_STAT_CHA: return Py_BuildValue("i", WHO->contr->orig_stats.Cha);
        case OBJFIELD_STAT_WIS: return Py_BuildValue("i", WHO->contr->orig_stats.Wis);
        case OBJFIELD_STAT_DEX: return Py_BuildValue("i", WHO->contr->orig_stats.Dex);
        case OBJFIELD_STAT_CON: return Py_BuildValue("i", WHO->contr->orig_stats.Con);
        case OBJFIELD_STAT_POW: return Py_BuildValue("i", WHO->contr->orig_stats.Pow);
                                
        default:
            RAISE("No unmodified version of attribute available");
    }
#endif    
}

/* FUNCTIONEND -- End of the DaimoninObject methods. */

/*****************************************************************************/
/* Attribute and flag getseters                                              */
/*****************************************************************************/

/* Attribute getter */
static PyObject *Object_GetAttribute(Daimonin_Object* whoptr, int fieldno)
{
    void *field_ptr, *field_ptr2;
    tag_t tag;
    object *obj;
    char *str;

    if(fieldno < 0 || fieldno >= NUM_OBJFIELDS)
        RAISE("Illegal field ID"); 
    
    field_ptr = (void *)((char *)WHO + obj_fields[fieldno].offset);
    
    /* TODO: better handling of types, signs, and overflows */
    switch(obj_fields[fieldno].type) {
        case FIELDTYPE_SHSTR:
        case FIELDTYPE_CSTR:  
            str =  *(char **)field_ptr;
            return Py_BuildValue("s", str ? str : "");
        case FIELDTYPE_UINT8:  return Py_BuildValue("b", *(uint8 *)field_ptr);
        case FIELDTYPE_SINT8:  return Py_BuildValue("b", *(sint8 *)field_ptr);
        case FIELDTYPE_UINT16: return Py_BuildValue("i", *(uint16 *)field_ptr);
        case FIELDTYPE_SINT16: return Py_BuildValue("i", *(sint16 *)field_ptr);
        case FIELDTYPE_UINT32: return Py_BuildValue("l", *(uint32 *)field_ptr);
        case FIELDTYPE_SINT32: return Py_BuildValue("l", *(sint32 *)field_ptr);
        case FIELDTYPE_FLOAT:  return Py_BuildValue("f", *(float *)field_ptr);
        case FIELDTYPE_MAP:    return wrap_map(*(mapstruct **)field_ptr);
        case FIELDTYPE_OBJECT: return wrap_object(*(object **)field_ptr);
        case FIELDTYPE_OBJECTREF:              
             field_ptr2 = (void *)((char *)WHO + obj_fields[fieldno].extra_data);
             obj = *(object **)field_ptr;
             tag = *(tag_t *)field_ptr2;
             return wrap_object(OBJECT_VALID(obj, tag) ? obj : NULL);

        default:
            RAISE("BUG: unknown field type");
    }
}

/* Object attribute setter */
static int Object_SetAttribute(Daimonin_Object* whoptr, PyObject *value, int fieldno)
{
    void *field_ptr;
    object *tmp;
    uint32 flags, offset;

    if(fieldno < 0 || fieldno >= NUM_OBJFIELDS)
        INTRAISE("Illegal field ID"); 
    
    flags = obj_fields[fieldno].flags;
    if((flags&FIELDFLAG_READONLY) ||
            ((flags&FIELDFLAG_PLAYER_READONLY) && WHO->type == PLAYER))
        INTRAISE("Trying to modify readonly field");
   
    offset = obj_fields[fieldno].offset;
    field_ptr = (void *)((char *)WHO + offset);
    
    switch(obj_fields[fieldno].type) {
        case FIELDTYPE_SHSTR:
            if(PyString_Check(value)) {
                const char *str = PyString_AsString(value);
                if (*(char **)field_ptr != NULL) 
                    FREE_STRING_HOOK(*(char **)field_ptr);
                if(str && strcmp(str, ""))
                    *(const char **)field_ptr = add_string_hook(str);
            } else
                INTRAISE("Illegal value for text field");
            break;

        /* TODO: better handling of types, signs, and overflows */
        case FIELDTYPE_UINT8:
            if(PyInt_Check(value)) 
                *(uint8 *)field_ptr = (uint8) PyInt_AsLong(value);
            else
                INTRAISE("Illegal value for int field");
            break;
        case FIELDTYPE_SINT8:
            if(PyInt_Check(value)) 
                *(sint8 *)field_ptr = (sint8)PyInt_AsLong(value);
            else
                INTRAISE("Illegal value for int field");
            break;
        case FIELDTYPE_UINT16:
            if(PyInt_Check(value)) 
                *(uint16 *)field_ptr = (uint16)PyInt_AsLong(value);
            else
                INTRAISE("Illegal value for int field");
            break;
        case FIELDTYPE_SINT16:
            if(PyInt_Check(value)) 
                *(sint16 *)field_ptr = (sint16)PyInt_AsLong(value);
            else
                INTRAISE("Illegal value for int field");
            break;
        case FIELDTYPE_UINT32:
            if(PyInt_Check(value)) 
                *(uint32 *)field_ptr = (uint32)PyInt_AsLong(value);
            else
                INTRAISE("Illegal value for int field");
            break;
        case FIELDTYPE_SINT32:
            if(PyInt_Check(value)) 
                *(sint32 *)field_ptr = (sint32)PyInt_AsLong(value);
            else
                INTRAISE("Illegal value for int field");
            break;
            
        case FIELDTYPE_FLOAT:
            if(PyFloat_Check(value))
                *(float *)field_ptr = (float)PyFloat_AsDouble(value);
            else if(PyInt_Check(value))
                *(float *)field_ptr = (float)PyInt_AsLong(value);
            else
                INTRAISE("Illegal value for float field");
            break;

        default:
            INTRAISE("BUG: unknown field type");
    }

    /* Make sure the inventory image/text is updated */
    /* FIXME: what if object was not carried by player ? */
    for(tmp = WHO->env; tmp != NULL; tmp = tmp->env) {
        if (tmp->type == PLAYER) {
            GCFP.Value[0] = (void *)(tmp);
            GCFP.Value[1] = (void *)(WHO);
            (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
        }
    }

    /* Special handling for some player stuff */
    if(WHO->type == PLAYER) {

		/*
		 * VC gives a error for the offsetof() because case: 
		 * must be followed by a constant value.
        switch(offset) {
            case offsetof(object, stats.Int): WHO->contr->orig_stats.Int = PyInt_AsLong(value); break;
            case offsetof(object, stats.Str): WHO->contr->orig_stats.Str = PyInt_AsLong(value); break;
            case offsetof(object, stats.Cha): WHO->contr->orig_stats.Cha = PyInt_AsLong(value); break;
            case offsetof(object, stats.Wis): WHO->contr->orig_stats.Wis = PyInt_AsLong(value); break;
            case offsetof(object, stats.Dex): WHO->contr->orig_stats.Dex = PyInt_AsLong(value); break;
            case offsetof(object, stats.Con): WHO->contr->orig_stats.Con = PyInt_AsLong(value); break;
            case offsetof(object, stats.Pow): WHO->contr->orig_stats.Pow = PyInt_AsLong(value); break;
            default:                                    
        }
		*/
		/* replacing the switch struct above */
            if(offset == offsetof(object, stats.Int)) WHO->contr->orig_stats.Int = (sint8)PyInt_AsLong(value);
            else if(offset ==offsetof(object, stats.Str)) WHO->contr->orig_stats.Str = (sint8)PyInt_AsLong(value);
            else if(offset ==offsetof(object, stats.Cha)) WHO->contr->orig_stats.Cha = (sint8)PyInt_AsLong(value);
            else if(offset ==offsetof(object, stats.Wis)) WHO->contr->orig_stats.Wis = (sint8)PyInt_AsLong(value);
            else if(offset ==offsetof(object, stats.Dex)) WHO->contr->orig_stats.Dex = (sint8)PyInt_AsLong(value);
            else if(offset ==offsetof(object, stats.Con)) WHO->contr->orig_stats.Con = (sint8)PyInt_AsLong(value);
            else if(offset ==offsetof(object, stats.Pow)) WHO->contr->orig_stats.Pow = (sint8)PyInt_AsLong(value);

        if(flags&FIELDFLAG_PLAYER_FIX)
            fix_player_hook(WHO);
    }
    
    return 0;
}

/* Object flag getter */
static PyObject* Object_GetFlag(Daimonin_Object* whoptr, int flagno)
{
    if(flagno < 0 || flagno >= NUM_FLAGS)
        RAISE("Unknown flag");

    return Py_BuildValue("i",QUERY_FLAG(WHO,flagno) ? 1 : 0);
}

/* Object flag setter */
int Object_SetFlag(Daimonin_Object* whoptr,  PyObject* val, int flagno)
{
    int value;
    object *tmp;

    if(flagno < 0 || flagno >= NUM_FLAGS)
        INTRAISE("Unknown flag");
    
    if(! PyInt_Check(val)) 
        INTRAISE("Value must be 0 or 1");
        
    value = PyInt_AsLong(val);
    if(value <0 || value > 1)
        INTRAISE("Value must be 0 or 1");
 
    if(value)
        SET_FLAG(WHO, flagno);
    else
        CLEAR_FLAG(WHO, flagno);
        
    /* Make sure the inventory image/text is updated */
    /* FIXME: what if object was not carried by player ? */
    for(tmp = WHO->env; tmp != NULL; tmp = tmp->env) {
        if (tmp->type == PLAYER) {
            GCFP.Value[0] = (void *)(tmp);
            GCFP.Value[1] = (void *)(WHO);
            (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
        }
    }
       
    /* TODO: if gender changed: 
    if()
       WHO->contr->socket.ext_title_flag = 1; * demand update to client */
            
    return 0;
}

/****************************************************************************/
/* Python Object management code                                            */
/****************************************************************************/

/* Initialize our CF object wrapper */
int Daimonin_Object_init(PyObject *module) 
{
    int i, flagno;
    
    /* field getseters */
    for(i=0; i < NUM_OBJFIELDS; i++) {
        PyGetSetDef *def = &Object_getseters[i];
        def->name = obj_fields[i].name;
        def->get = (getter)Object_GetAttribute;
        def->set = (setter)Object_SetAttribute;
        def->doc = NULL;
        def->closure = (void *)i;
    }

    /* flag getseters */
    for(flagno=0; flagno<NUM_FLAGS; flagno++) 
        if(flag_names[flagno]) {
            PyGetSetDef *def = &Object_getseters[i++];
            def->name = flag_names[flagno];
            def->get = (getter)Object_GetFlag;
            def->set = (setter)Object_SetFlag;
            def->doc = NULL;
            def->closure = (void *)flagno;
        }
    Object_getseters[i].name = NULL;

    /* Add constants */
    for(i=0; object_constants[i].name; i++) 
        if(PyModule_AddIntConstant(module, object_constants[i].name, object_constants[i].value))
            return -1;
    
    Daimonin_ObjectType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&Daimonin_ObjectType) < 0)            
        return -1;

    Py_INCREF(&Daimonin_ObjectType);
    return 0;
}

/* Create a new Object wrapper (uninitialized) */
static PyObject *
Daimonin_Object_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Daimonin_Object *self;

    self = (Daimonin_Object *)type->tp_alloc(type, 0);
    if(self)
        self->obj = NULL;

    return (PyObject *)self;
}

/* Free an Object wrapper */
static void
Daimonin_Object_dealloc(Daimonin_Object* self)
{
    self->obj = NULL;
    self->ob_type->tp_free((PyObject*)self);
}

/* Return a string representation of this object (useful for debugging) */
static PyObject *
Daimonin_Object_str(Daimonin_Object *self)
{
    return PyString_FromFormat("[%s \"%s\"]", self->obj->arch->name, self->obj->name);
}

/* Utility method to wrap an object. */
PyObject * wrap_object(object *what)
{
    Daimonin_Object *wrapper;
    
    /* return None if no object was to be wrapped */
    if(what == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    
    wrapper = PyObject_NEW(Daimonin_Object, &Daimonin_ObjectType);
    if(wrapper != NULL)
        wrapper->obj = what;

    return (PyObject *)wrapper;
}

/******************
 * Old, leftover stuff. Will clean up later...
 */

#if 0
/* TODO: Create a FindAnimation() thingie */

/*****************************************************************************/
/* Name   : Daimonin_Object_SetFace                                          */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/

static PyObject* Daimonin_Object_SetFace(Daimonin_Object *whoptr, PyObject* args)
{
    char* txt;
    Daimonin_Object *whoptr;
    CFParm* CFR;
    int val = UP_OBJ_FACE;

    if (!PyArg_ParseTuple(args,"O!s", &Daimonin_ObjectType, &whoptr,&txt))
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
    return Py_None;
}
#endif

#if 0
/* Those replace the old get-script... and set-script... system */
/*****************************************************************************/
/* Name   : Daimonin_Object_GetEventHandler                                  */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
static PyObject* Daimonin_Object_GetEventHandler(Daimonin_Object *whoptr, PyObject* args)
{
    int eventnr;

    if (!PyArg_ParseTuple(args,"i", &Daimonin_ObjectType, &whoptr,&eventnr))
        return NULL;
    return Py_BuildValue("s","" /*WHO->event_hook[eventnr]*/);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetEventHandler                                  */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static PyObject* Daimonin_Object_SetEventHandler(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whoptr;
    int eventnr;
    char* scriptname;

    if (!PyArg_ParseTuple(args,"O!is", &Daimonin_ObjectType, &whoptr, &eventnr, &scriptname))
        return NULL;

    /*WHO->event_hook[eventnr] = add_string_hook(scriptname);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetEventPlugin                                   */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static PyObject* Daimonin_Object_GetEventPlugin(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whoptr;
    int eventnr;

    if (!PyArg_ParseTuple(args,"O!i", &Daimonin_ObjectType, &whoptr, &eventnr))
        return NULL;
    return Py_BuildValue("s", ""/*WHO->event_plugin[eventnr]*/);
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetEventPlugin                                   */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static PyObject* Daimonin_Object_SetEventPlugin(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whoptr;
    int eventnr;
    char* scriptname;

    if (!PyArg_ParseTuple(args,"O!is", &Daimonin_ObjectType, &whoptr,&eventnr,&scriptname))
        return NULL;

    /*WHO->event_plugin[eventnr] = add_string_hook(scriptname);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_GetEventOptions                                  */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static PyObject* Daimonin_Object_GetEventOptions(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whoptr;
    int eventnr;
    /*static char estr[4];*/
    if (!PyArg_ParseTuple(args,"O!i", &Daimonin_ObjectType, &whoptr,&eventnr))
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
/* Name   : Daimonin_Object_SetEventOptions                                  */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/

static PyObject* Daimonin_Object_SetEventOptions(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whoptr;
    int eventnr;
    char* scriptname;

    if (!PyArg_ParseTuple(args,"O!is", &Daimonin_ObjectType, &whoptr,&eventnr,&scriptname))
        return NULL;

    /*    WHO->event_options[eventnr] = add_string_hook(scriptname);*/

    Py_INCREF(Py_None);
    return Py_None;
}
#endif

/* TODO: Hmm... might want to keep this? */
#if 0
/*****************************************************************************/
/* Name   : Daimonin_Object_SetDirection                                     */
/* Python : Daimonin.SetDirection(object, value)                             */
/* Status : Untested                                                         */
/*****************************************************************************/
/* this function will fail imho - for animation[] we need to call a hook! */

static PyObject* Daimonin_Object_SetDirection(Daimonin_Object *whoptr, PyObject* args)
{
    int value;
    Daimonin_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &Daimonin_ObjectType, &whoptr,&value))
        return NULL;

    WHO->direction = value;
    SET_ANIMATION(WHO, WHO->direction);
    Py_INCREF(Py_None);
    return Py_None;
}
#endif 

/* I'm going to replace those with a Reorder() call. Can't find any reason to
 * fiddle with those except for changing object order. Don't want possible
 * dangerous functions here...
 */
#if 0

/*****************************************************************************/
/* Name   : Daimonin_Object_SetNextObject                                    */
/* Python : Daimonin.SetNextObject(object,object)                            */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* Daimonin_Object_SetNextObject(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whoptr;
    Daimonin_Object *whatptr;
    if (!PyArg_ParseTuple(args,"O!O!", &Daimonin_ObjectType, &whoptr, &Daimonin_ObjectType, &whatptr))
        return NULL;

    WHO->below = WHAT;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Object_SetPreviousObject                                */
/* Python : Daimonin.SetPreviousObject(object,object)                        */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* Daimonin_Object_SetPreviousObject(Daimonin_Object *whoptr, PyObject* args)
{
    Daimonin_Object *whoptr;
    Daimonin_Object *whatptr;
    if (!PyArg_ParseTuple(args,"O!O!", &Daimonin_ObjectType, &whoptr, &Daimonin_ObjectType, &whatptr))
        return NULL;

    WHO->above = WHAT;
    Py_INCREF(Py_None);
    return Py_None;
}

#endif
