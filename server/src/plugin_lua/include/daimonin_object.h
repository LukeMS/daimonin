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

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

/* First the required header files - only the CF module interface and Lua */
#include <plugin_lua.h>

/* GameObject methods  */
static int  GameObject_Interface(lua_State *L);
static int  GameObject_CheckTrigger(lua_State *L);
static int  GameObject_SetSaveBed(lua_State *L);
static int  GameObject_SetSkill(lua_State *L);
static int  GameObject_GetSkill(lua_State *L);
static int  GameObject_ActivateRune(lua_State *L);
static int  GameObject_CastAbility(lua_State *L);
static int  GameObject_GetGod(lua_State *L);
static int  GameObject_SetGod(lua_State *L);
static int  GameObject_TeleportTo(lua_State *L);
static int  GameObject_InsertInside(lua_State *L);
static int  GameObject_Apply(lua_State *L);
static int  GameObject_PickUp(lua_State *L);
static int  GameObject_Drop(lua_State *L);
static int  GameObject_Take(lua_State *L);
static int  GameObject_Deposit(lua_State *L);
static int  GameObject_Withdraw(lua_State *L);
static int  GameObject_Communicate(lua_State *L);
static int  GameObject_Say(lua_State *L);
static int  GameObject_SayTo(lua_State *L);
static int  GameObject_SetGender(lua_State *L);
static int  GameObject_SetRank(lua_State *L);
static int  GameObject_SetAlignment(lua_State *L);
static int  GameObject_GetAlignmentForce(lua_State *L);
static int  GameObject_SetGuildForce(lua_State *L);
static int  GameObject_GetGuildForce(lua_State *L);
static int  GameObject_Fix(lua_State *L);
static int  GameObject_Kill(lua_State *L);
static int  GameObject_CastSpell(lua_State *L);
static int  GameObject_DoKnowSpell(lua_State *L);
static int  GameObject_AcquireSpell(lua_State *L);
static int  GameObject_FindSkill(lua_State *L);
static int  GameObject_AcquireSkill(lua_State *L);
static int  GameObject_FindMarkedObject(lua_State *L);
static int  GameObject_CreatePlayerForce(lua_State *L);
static int  GameObject_CheckQuestObject(lua_State *L);
static int  GameObject_AddQuestObject(lua_State *L);
static int  GameObject_CreatePlayerInfo(lua_State *L);
static int  GameObject_GetPlayerInfo(lua_State *L);
static int  GameObject_GetNextPlayerInfo(lua_State *L);
static int  GameObject_CheckInvisibleInside(lua_State *L);
static int  GameObject_CreateInvisibleInside(lua_State *L);
static int  GameObject_CreateObjectInside(lua_State *L);
static int  GameObject_CheckInventory(lua_State *L);
static int  GameObject_Remove(lua_State *L);
static int  GameObject_Destruct(lua_State *L);
static int  GameObject_SetPosition(lua_State *L);
static int  GameObject_Move(lua_State *L);
static int  GameObject_IdentifyItem(lua_State *L);
static int  GameObject_Write(lua_State *L);
static int  GameObject_IsOfType(lua_State *L);
static int  GameObject_GetIP(lua_State *L);
static int  GameObject_GetArchName(lua_State *L);
static int  GameObject_ShowCost(lua_State *L);
static int  GameObject_GetItemCost(lua_State *L);
static int  GameObject_GetMoney(lua_State *L);
static int  GameObject_Save(lua_State *L);
static int  GameObject_PayForItem(lua_State *L);
static int  GameObject_PayAmount(lua_State *L);
static int  GameObject_SendCustomCommand(lua_State *L);
static int  GameObject_Clone(lua_State *L);
static int  GameObject_GetAI(lua_State *L);

#if 0
/* Hmmm... Still requires constants... */
static PyObject* GameObject_GetUnmodifiedAttribute(GameObject* self, PyObject* args);
#endif

#if 0
/* Get rid of these? */
static PyObject* GameObject_GetEventHandler(GameObject *self, PyObject* args);
static PyObject* GameObject_SetEventHandler(GameObject *self, PyObject* args);
static PyObject* GameObject_GetEventPlugin(GameObject *self, PyObject* args);
static PyObject* GameObject_SetEventPlugin(GameObject *self, PyObject* args);
static PyObject* GameObject_GetEventOptions(GameObject *self, PyObject* args);
static PyObject* GameObject_SetEventOptions(GameObject *self, PyObject* args);
#endif

#endif /*GAME_OBJECT_H*/
