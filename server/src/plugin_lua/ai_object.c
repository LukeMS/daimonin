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
#include <ai_object.h>

/* Global data */

/* Available python methods for the GameObject object */
static struct method_decl   AI_methods[]            =
{
    {"ForbidMoveDirection",  (lua_CFunction) AI_ForbidMoveDirection},
    {"IsMoveDirectionAllowed",  (lua_CFunction) AI_IsMoveDirectionAllowed},
    {"MoveRespondDirection",  (lua_CFunction) AI_MoveRespondDirection},
    {"MoveRespondDirections",  (lua_CFunction) AI_MoveRespondDirections},
    {"MoveRespondCoordinate",  (lua_CFunction) AI_MoveRespondCoordinate},
    {"MoveRespondObject",  (lua_CFunction) AI_MoveRespondObject},

    {NULL, NULL}
};

/* GameObject attributes */
struct attribute_decl       AI_attributes[]         =
{
    /* All entries MUST be in same order as field_id enum above */
    {NULL}
};

/****************************************************************************/
/*                          AI methods                         */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : AI_ForbidMoveDirection                                           */
/* Lua    : ai:ForbidMoveDirection(dir)                                      */
/* Info   : In a movement behaviour, add dir to the list of forbidden        */
/*          directions, but don't select a specific movement.                */
/* Status : Tested                                                           */
/*****************************************************************************/
static int AI_ForbidMoveDirection(lua_State *L)
{
    lua_object *self, *event;
    int         dir;
    
    get_lua_args(L, "Ai", &self, &dir);
    
    lua_pushliteral(L, "event");
    lua_rawget(L, LUA_GLOBALSINDEX);
    event = lua_touserdata(L, -1);

    if(event && event->class == &Event && event->data.context->move_response)
        event->data.context->move_response->forbidden |= (1 << dir);
    else
        luaL_error(L, "This function is only useful for movement behaviours");
    
    return 0;
}

/*****************************************************************************/
/* Name   : AI_IsMoveDirectionAllowed                                        */
/* Lua    : ai:IsMoveDirectionAllowed(dir)                                   */
/* Info   : In a movement behaviour, see if dir is in the list of forbidden  */
/*          directions. It can have been added by an earlier behaviour or by */
/*          yourself.                                                        */
/* Status : Tested                                                           */
/*****************************************************************************/
static int AI_IsMoveDirectionAllowed(lua_State *L)
{
    lua_object *self, *event;
    int         dir;
    
    get_lua_args(L, "Ai", &self, &dir);
    
    lua_pushliteral(L, "event");
    lua_rawget(L, LUA_GLOBALSINDEX);
    event = lua_touserdata(L, -1);

    if(event && event->class == &Event && event->data.context->move_response)
    {
        lua_pushboolean(L, !(event->data.context->move_response->forbidden & (1 << dir)));
        return 1;
    } else
        luaL_error(L, "This function is only useful for movement behaviours");
    
    return 0;
}

/*****************************************************************************/
/* Name   : AI_MoveRespondDirection                                          */
/* Lua    : ai:MoveRespondDirection(dir)                                     */
/* Info   : In a movement behaviour, set the reponse direction to dir        */
/* Status : Untested                                                         */
/*****************************************************************************/
static int AI_MoveRespondDirection(lua_State *L)
{
    lua_object *self, *event;
    int         dir;
    
    get_lua_args(L, "Ai", &self, &dir);
    
    lua_pushliteral(L, "event");
    lua_rawget(L, LUA_GLOBALSINDEX);
    event = lua_touserdata(L, -1);

    if(event && event->class == &Event && event->data.context->move_response)
    {
        event->data.context->move_response->type = MOVE_RESPONSE_DIR;
        event->data.context->move_response->data.direction = dir;
    } else
        luaL_error(L, "This function is only useful for movement behaviours");
    
    return 0;
}

/*****************************************************************************/
/* Name   : AI_MoveRespondDirections                                         */
/* Lua    : ai:MoveRespondDirections(dir)                                    */
/* Info   : In a movement behaviour, add dir as a reponse direction          */
/*          (It is possible to add multiple possible directions, and then a  */
/*           random one is chosen from the ones in the list).                */
/* Status : Untested                                                         */
/*****************************************************************************/
static int AI_MoveRespondDirections(lua_State *L)
{
    lua_object *self, *event;
    int         dir;
    
    get_lua_args(L, "Ai", &self, &dir);
    
    lua_pushliteral(L, "event");
    lua_rawget(L, LUA_GLOBALSINDEX);
    event = lua_touserdata(L, -1);

    if(event && event->class == &Event && event->data.context->move_response)
    {
        if(event->data.context->move_response->type != MOVE_RESPONSE_DIRS)
        {
            event->data.context->move_response->type = MOVE_RESPONSE_DIRS;
            event->data.context->move_response->data.directions = 0;
        }
        event->data.context->move_response->data.directions |= (1 << dir);
    } else
        luaL_error(L, "This function is only useful for movement behaviours");
    
    return 0;
}

/*****************************************************************************/
/* Name   : AI_MoveRespondCoordinate                                         */
/* Lua    : ai:MoveRespondCoordinate(map, x, y)                              */
/* Info   : In a movement behaviour, set the reponse to the specified map    */
/*          coordinate. The pathfinding system will then take over to find   */
/*          the best way of getting there                                    */
/* Status : Untested                                                         */
/*****************************************************************************/
static int AI_MoveRespondCoordinate(lua_State *L)
{
    lua_object *self, *event, *map;
    int         x,y;
    
    get_lua_args(L, "AMii", &self, &map, &x, &y);
    
    lua_pushliteral(L, "event");
    lua_rawget(L, LUA_GLOBALSINDEX);
    event = lua_touserdata(L, -1);

    if(event && event->class == &Event && event->data.context->move_response)
    {
        event->data.context->move_response->type = MOVE_RESPONSE_COORD;
        event->data.context->move_response->data.coord.x = x;
        event->data.context->move_response->data.coord.y = y;
        event->data.context->move_response->data.coord.map = map->data.map;
    } else
        luaL_error(L, "This function is only useful for movement behaviours");
    
    return 0;
}

/*****************************************************************************/
/* Name   : AI_MoveRespondObject                                             */
/* Lua    : ai:MoveRespondObject(obj)                                        */
/* Info   : In a movement behaviour, set the reponse to the specified object */
/*          The pathfinding system will then take over to find the best way  */
/* Status : Tested                                                           */
/*****************************************************************************/
static int AI_MoveRespondObject(lua_State *L)
{
    lua_object *self, *event, *obj;
    
    get_lua_args(L, "AO", &self, &obj);
    
    lua_pushliteral(L, "event");
    lua_rawget(L, LUA_GLOBALSINDEX);
    event = lua_touserdata(L, -1);

    if(event && event->class == &Event && event->data.context->move_response)
    {
        event->data.context->move_response->type = MOVE_RESPONSE_OBJECT;
        event->data.context->move_response->data.target.obj = obj->data.object;
        event->data.context->move_response->data.target.obj_count = obj->tag;
    } else
        luaL_error(L, "This function is only useful for movement behaviours");
    
    return 0;
}


/****************************************************************************/
/* Lua object management code                                               */
/****************************************************************************/

/* toString method for AI objects */
static int AI_toString(lua_State *L)
{
    lua_object *obj = lua_touserdata(L, 1);

    if (obj && obj->class->type == LUATYPE_AI)
        lua_pushfstring(L, "[%s] ", STRING_OBJ_NAME(obj->data.object));
    else
        luaL_error(L, "Not an AI object");

    return 1;
}

/* Tests if an AI object is valid */
static int AI_isValid(lua_State *L, lua_object *obj)
{
    return obj->data.object->count == obj->tag;
}


lua_class   AI  =
{
    LUATYPE_AI, "AI", 0, AI_toString, AI_attributes, AI_methods, 
    NULL, NULL, NULL, NULL, NULL, AI_isValid
};

int AI_init(lua_State *L)
{
    init_class(L, &AI);

    return 0;
}
