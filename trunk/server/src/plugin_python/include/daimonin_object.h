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

#ifndef DAIMONIN_OBJECT_H
#define DAIMONIN_OBJECT_H

/* First the required header files - only the CF module interface and Python */
#include <Python.h>
#include <plugin.h>
#include <plugin_python.h>

/* Daimonin_Object methods  */
static PyObject* Daimonin_Object_CheckTrigger(Daimonin_Object* self, PyObject* args);
static PyObject* Daimonin_Object_CheckArchInventory(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetSaveBed(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetSkillExperience(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetSkillExperience(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_ActivateRune(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_CastAbility(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetGod(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetGod(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_TeleportTo(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_InsertInside(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Apply(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_PickUp(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Drop(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Take(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Communicate(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Say(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SayTo(Daimonin_Object *whoptr, PyObject* args);
static PyObject* Daimonin_Object_SetGender(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetRank(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetAlignment(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetAlignmentForce(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetGuildForce(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetGuildForce(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Fix(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Kill(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_CastSpell(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_DoKnowSpell(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_AcquireSpell(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_DoKnowSkill(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_AcquireSkill(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_FindMarkedObject(Daimonin_Object *whoptr, PyObject* args);
static PyObject* Daimonin_Object_CreatePlayerForce(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_CreatePlayerInfo(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetPlayerInfo(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetNextPlayerInfo(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_CheckInvisibleInside(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_CreateInvisibleInside(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_CreateObjectInside(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_CheckInventory(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Remove(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetPosition(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_IdentifyItem(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Write(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_IsOfType(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetIP(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetArchName(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetItemCost(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetMoney(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_Save(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_PayForItem(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_PayAmount(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SendCustomCommand(Daimonin_Object* self, PyObject* args);
static PyObject* Daimonin_Object_Clone(Daimonin_Object* self, PyObject* args);

/* Hmmm... Still requires constants... */
static PyObject* Daimonin_Object_GetUnmodifiedAttribute(Daimonin_Object* self, PyObject* args);

#if 0
/* Get rid of these? */
static PyObject* Daimonin_Object_GetEventHandler(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetEventHandler(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetEventPlugin(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetEventPlugin(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_GetEventOptions(Daimonin_Object *self, PyObject* args);
static PyObject* Daimonin_Object_SetEventOptions(Daimonin_Object *self, PyObject* args);
#endif

/* Daimonin_Object SetGeters */
static int Object_SetFlag(Daimonin_Object* self, PyObject* val, int flagnp);
static PyObject* Object_GetFlag(Daimonin_Object* self, int flagno);
static int Object_SetAttribute(Daimonin_Object* self, PyObject *value, int fieldid);
static PyObject* Object_GetAttribute(Daimonin_Object* self, int fieldid);

/*****************************************************************************/
/* Crossfire object type part.                                               */
/* Using a custom type for CF Objects allows us to handle more errors, and   */
/* avoid server crashes due to buggy scripts                                 */
/* In the future even add methods to it?                                     */
/*****************************************************************************/

/* Object creator (not really needed, since the generic creator does the same thing...) */
static PyObject *
Daimonin_Object_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

/* str() function to get a string representation of this object */
static PyObject *
Daimonin_Object_str(Daimonin_Object *self);
   
/* Object deallocator (needed) */
static void
Daimonin_Object_dealloc(Daimonin_Object* self);

#endif /*DAIMONIN_OBJECT_H*/
