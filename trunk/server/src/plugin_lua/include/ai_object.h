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

#ifndef AI_OBJECT_H
#define AI_OBJECT_H

/* First the required header files - only the CF module interface and Lua */
#include <plugin_lua.h>

/* AI Object methods  */
static int AI_ForbidMoveDirection(lua_State *L);
static int AI_IsMoveDirectionAllowed(lua_State *L);
static int AI_MoveRespondDirection(lua_State *L);
static int AI_MoveRespondDirections(lua_State *L);
static int AI_MoveRespondCoordinate(lua_State *L);
static int AI_MoveRespondObject(lua_State *L);

static int AI_Knows(lua_State *L);
static int AI_Register(lua_State *L);
static int AI_UsesDistanceAttack(lua_State *L);
static int AI_GetFriendship(lua_State *L);
static int AI_GetAttraction(lua_State *L);
static int AI_LastSeen(lua_State *L);
static int AI_GetKnownMobs(lua_State *L);
static int AI_GetKnownObjects(lua_State *L);


#endif /*AI_OBJECT_H*/
