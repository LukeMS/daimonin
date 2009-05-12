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
static int AI_ForgetKnownMobs(lua_State *L);
static int AI_ForgetKnownObjects(lua_State *L);
static int AI_ReloadBehaviourlist(lua_State *L);
static int AI_GetBehaviourlist(lua_State *L);

/* Available python methods for the GameObject object */
static struct method_decl   AI_methods[]            =
{
    /* Movement response functions */
    {"ForbidMoveDirection",    (lua_CFunction) AI_ForbidMoveDirection},
    {"IsMoveDirectionAllowed", (lua_CFunction) AI_IsMoveDirectionAllowed},
    {"MoveRespondDirection",   (lua_CFunction) AI_MoveRespondDirection},
    {"MoveRespondDirections",  (lua_CFunction) AI_MoveRespondDirections},
    {"MoveRespondCoordinate",  (lua_CFunction) AI_MoveRespondCoordinate},
    {"MoveRespondObject",      (lua_CFunction) AI_MoveRespondObject},

    /* Known_obj functions (Those are quite klunky and slow right now...) */
    {"Knows",                  (lua_CFunction) AI_Knows},
    {"Register",               (lua_CFunction) AI_Register},
    {"UsesDistanceAttack",     (lua_CFunction) AI_UsesDistanceAttack},
    {"GetFriendship",          (lua_CFunction) AI_GetFriendship},
    {"GetAttraction",          (lua_CFunction) AI_GetAttraction},
    {"LastSeen",               (lua_CFunction) AI_LastSeen},
    {"GetKnownMobs",           (lua_CFunction) AI_GetKnownMobs},
    {"GetKnownObjects",        (lua_CFunction) AI_GetKnownObjects},
    {"ForgetKnownMobs",        (lua_CFunction) AI_ForgetKnownMobs},
    {"ForgetKnownObjects",     (lua_CFunction) AI_ForgetKnownObjects},

    /* Functions for manipulating the behaviourlist */
    {"GetBehaviourlist",       (lua_CFunction) AI_GetBehaviourlist},
    {"ReloadBehaviourlist",    (lua_CFunction) AI_ReloadBehaviourlist},

    {NULL, NULL}
};

/* GameObject attributes */
struct attribute_decl       AI_attributes[]         =
{
    /* All entries MUST be in same order as field_id enum above */
    {NULL,0,0,0,0}
};

/* Utility function */
static struct mob_known_obj *find_known_obj(object *source, object *target)
{
    struct mob_known_obj *tmp;

    if(source == NULL || target == NULL || source->type != MONSTER ||
            MOB_DATA(source) == NULL || source == target)
        return NULL;

    for (tmp = MOB_DATA(source)->known_mobs; tmp; tmp = tmp->next)
        if (tmp->obj == target && tmp->obj_count == target->count)
            return tmp;

    if(MOB_DATA(source)->known_objs_ht)
        return hooks->hashtable_find(MOB_DATA(source)->known_objs_ht, target);

    return NULL;
}

/****************************************************************************/
/*                          AI methods                         */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Lua plugin functions come */

/*****************************************************************************/
/* Name   : AI_ForbidMoveDirection                                           */
/* Lua    : ai:ForbidMoveDirection(dir)                                      */
/* Info   : In a movement behaviour, add dir to the list of forbidden        */
/*          directions, but don't select a specific movement.                */
/* Status : Tested/Stable                                                    */
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
/* Status : Tested/Stable                                                    */
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
/* Status : Untested/Stable                                                  */
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
/* Status : Untested/Stable                                                  */
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
/* Status : Untested/Stable                                                  */
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
/* Status : Tested/Stable                                                    */
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

/*****************************************************************************/
/* Name   : AI_Knows                                                         */
/* Lua    : ai:Knows(obj)                                                    */
/* Info   : Returns true if the AI "knows about" obj, i.e. obj has been      */
/*          observed and memorized by the AI.                                */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int AI_Knows(lua_State *L)
{
    lua_object *self, *obj;

    get_lua_args(L, "AO", &self, &obj);

    lua_pushboolean(L,
            find_known_obj(self->data.object, obj->data.object) ?
            TRUE : FALSE);

    return 1;
}

/*****************************************************************************/
/* Name   : AI_Register                                                      */
/* Lua    : ai:Register(obj, friendship_change, attraction_change)           */
/* Info   : Add obj to the AI's memory and/or change the friendship and      */
/*          attraction values for obj                                        */
/* Status : Tested/Stable                                                      */
/*****************************************************************************/
static int AI_Register(lua_State *L)
{
    lua_object *self, *obj;
    int friendship = 0, attraction = 0;

    get_lua_args(L, "AO|ii", &self, &obj, &friendship, &attraction);

    hooks->update_npc_knowledge(self->data.object, obj->data.object,
            friendship, attraction);

    return 0;
}

/*****************************************************************************/
/* Name   : AI_UsesDistanceAttack                                            */
/* Lua    : ai:UsesDistanceAttack(obj)                                       */
/* Info   : Returns true if obj is known to make use of distance attacks     */
/*          Always returns false if obj is not known at all by the AI        */
/* Status : Untested/Stable                                                    */
/*****************************************************************************/
static int AI_UsesDistanceAttack(lua_State *L)
{
    lua_object *self, *obj;
    struct mob_known_obj *info;

    get_lua_args(L, "AO", &self, &obj);

    if(!(info = find_known_obj(self->data.object, obj->data.object)))
    {
        lua_pushboolean(L, FALSE);
        return 1;
    }

    lua_pushboolean(L,
           QUERY_FLAG(info, AI_OBJFLAG_USES_DISTANCE_ATTACK));

    return 1;
}

/*****************************************************************************/
/* Name   : AI_GetFriendship                                                 */
/* Lua    : ai:GetFriendship(obj)                                            */
/* Info   : Returns the AI's friendship value towards obj                    */
/*          Always returns 0 if obj is not known at all by the AI            */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int AI_GetFriendship(lua_State *L)
{
    lua_object *self, *obj;
    struct mob_known_obj *info;

    get_lua_args(L, "AO", &self, &obj);

    if(!(info = find_known_obj(self->data.object, obj->data.object)))
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, info->tmp_friendship);

    return 1;
}

/*****************************************************************************/
/* Name   : AI_GetAttraction                                                 */
/* Lua    : ai:GetAttraction(obj)                                            */
/* Info   : Returns the obj's attraction value on the AI                     */
/*          Always returns 0 if obj is not known at all by the AI            */
/* Status : Unfinished                                                       */
/*****************************************************************************/
static int AI_GetAttraction(lua_State *L)
{
    lua_object *self, *obj;
    struct mob_known_obj *info;

    get_lua_args(L, "AO", &self, &obj);

    if(!(info = find_known_obj(self->data.object, obj->data.object)))
    {
        lua_pushnumber(L, 0);
        return 1;
    }

    lua_pushnumber(L, info->tmp_attraction);

    return 1;
}

/*****************************************************************************/
/* Name   : AI_LastSeen                                                      */
/* Lua    : ai:LastSeen(obj)                                                 */
/* Info   : Returns information about when and where obj was last observed   */
/*          by the AI.                                                       */
/*          Always returns nil if obj is not known at all by the AI          */
/*          The following four values are returned:                          */
/*          - Time in seconds since last observation                         */
/*          - Map of last observation                                        */
/*          - x coordinate on map                                            */
/*          - y coordinate on map                                            */
/* Status : Unfinished                                                       */
/*****************************************************************************/
static int AI_LastSeen(lua_State *L)
{
    lua_object *self, *obj;
    struct mob_known_obj *info;

    get_lua_args(L, "AO", &self, &obj);

    if(!(info = find_known_obj(self->data.object, obj->data.object)))
        return 0;

    luaL_error(L, "Not implemented yet");

    /* TODO */
#if 0
    lua_pushnumber(L, 0);        /* Time since last seen */
    push_object(L, &Map, NULL);  /* Da map */
    lua_pushnumber(L, info->last_x);
    lua_pushnumber(L, info->last_y);
#endif

    return 4;
}

/*****************************************************************************/
/* Name   : AI_GetKnownMobs                                                  */
/* Lua    : ai:GetKnownMobs()                                                */
/* Info   : Returns an array with all mobs known by the AI                   */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int AI_GetKnownMobs(lua_State *L)
{
    struct mob_known_obj *tmp;
    lua_object *self;
    int i = 0;

    get_lua_args(L, "A", &self);

    lua_newtable(L);

    for (tmp = MOB_DATA(self->data.object)->known_mobs; tmp; tmp = tmp->next)
    {
        push_object(L, &GameObject, tmp->obj);
        lua_rawseti(L, -2, i++);
    }

    return 1;
}

/*****************************************************************************/
/* Name   : AI_GetKnownObjects                                               */
/* Lua    : ai:GetKnownObjects()                                             */
/* Info   : Returns an array with all objects known by the AI                */
/* Status : Untested/Stable                                                  */
/*****************************************************************************/
static int AI_GetKnownObjects(lua_State *L)
{
    struct mob_known_obj *tmp;
    lua_object *self;
    int i = 0;

    get_lua_args(L, "A", &self);

    lua_newtable(L);

    for (tmp = MOB_DATA(self->data.object)->known_objs; tmp; tmp = tmp->next)
    {
        push_object(L, &GameObject, tmp->obj);
        lua_rawseti(L, -2, i++);
    }

    return 1;
}

/*****************************************************************************/
/* Name   : AI_ForgetKnownMobs                                               */
/* Lua    : ai:ForgetKnownMobs()                                             */
/* Info   : Clears the AIs memory of any registered mobs, npcs or players    */
/* Status : Untested/Stable                                                  */
/*****************************************************************************/
static int AI_ForgetKnownMobs(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "A", &self);

    hooks->clear_mob_knowns(self->data.object, &MOB_DATA(self->data.object)->known_mobs, NULL);

    return 0;
}

/*****************************************************************************/
/* Name   : AI_ForgetKnownObjects                                            */
/* Lua    : ai:ForgetKnownObjects()                                          */
/* Info   : Clears the AIs memory of any registered objects                  */
/*          (not mobs/npcs or players)                                       */
/* Status : Untested/Stable                                                  */
/*****************************************************************************/
static int AI_ForgetKnownObjects(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "A", &self);

    hooks->clear_mob_knowns(self->data.object, &MOB_DATA(self->data.object)->known_objs, MOB_DATA(self->data.object)->known_objs_ht);

    return 0;
}

/*****************************************************************************/
/* Name   : AI_GetBehaviourlist                                              */
/* Lua    : ai:GetBehaviourlist()                                            */
/* Info   : Returns a table representation of the behaviourlist (all         */
/*          behaviours and their parameters)                                 */
/*          To visualize the layout of the table you can try something like  */
/*          print(DataStore.Serialize(ai:GetBehaviourlist()))                */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int AI_GetBehaviourlist(lua_State *L)
{
    lua_object *self;
    int class;
    int super_tbl_idx, class_tbl_idx, behaviour_tbl_idx, parameter_tbl_idx;
    int string_lower_idx;

    get_lua_args(L, "A", &self);

    /* Get hold of string.lower */
    lua_pushliteral(L, "string");
    lua_gettable(L, LUA_GLOBALSINDEX);
    lua_pushliteral(L, "lower");
    lua_gettable(L, -2);
    string_lower_idx = lua_gettop(L);

    /* Create the supertable that encapsulates all info */
    lua_newtable(L);
    super_tbl_idx = lua_gettop(L);

    for(class = 0; class < NROF_BEHAVIOURCLASSES; class++)
    {
        struct mob_behaviour *behaviour;
        int behaviour_idx = 1;

        /* Create the table for this class */
        lua_pushvalue(L, string_lower_idx);
        lua_pushstring(L, hooks->behaviourclasses[class].name); /* Key */
        lua_call(L, 1, 1);
        lua_newtable(L);
        class_tbl_idx = lua_gettop(L);

        for(behaviour = MOB_DATA(self->data.object)->behaviours->behaviours[class]; behaviour; behaviour = behaviour->next)
        {
            unsigned int param;

            /* Create the table for this behaviour (key is both idx and name) */
            lua_pushvalue(L, string_lower_idx);
            lua_pushstring(L, behaviour->declaration->name);
            lua_call(L, 1, 1);
            lua_newtable(L);
            behaviour_tbl_idx = lua_gettop(L);

            /* Insert the behaviour name */
            lua_pushliteral(L, "name");
            lua_pushvalue(L, -3); /* behaviour name, again */
            lua_rawset(L, behaviour_tbl_idx);

            /* Insert the parameter table */
            lua_pushliteral(L, "parameters");
            lua_newtable(L);
            parameter_tbl_idx = lua_gettop(L);

            for(param = 0; param < behaviour->declaration->nrof_params; param++)
            {
                struct mob_behaviour_param *value;
                int parameter_idx = 1;

                /* Parameter name and values */
                lua_pushvalue(L, string_lower_idx);
                lua_pushstring(L, behaviour->declaration->params[param].name);
                lua_call(L, 1, 1);
                lua_newtable(L);

                for(value = &behaviour->parameters[param]; value; value = value->next)
                {
                    /* Skip optional multi parameters that wasn't given. Those are usually considered
                     * empty, not default */
                    if((behaviour->declaration->params[param].attribs & AI_MANDATORY_PARAM) == 0 &&
                            (behaviour->declaration->params[param].attribs & AI_MULTI_PARAM) &&
                            (value->flags & AI_PARAM_PRESENT) == 0)
                        continue;

                    if(behaviour->declaration->params[param].type == AI_INTEGER_TYPE)
                        lua_pushnumber(L, value->intvalue);
                    else if(behaviour->declaration->params[param].type == AI_STRING_TYPE)
                        lua_pushstring(L, value->stringvalue);
                    else if(behaviour->declaration->params[param].type == AI_STRINGINT_TYPE)
                    {
                        char buf[HUGE_BUF];
                        sprintf(buf, "%s:%ld", value->stringvalue, value->intvalue);
                        lua_pushstring(L, buf);
                    } else
                        lua_pushnil(L); /* Just to avoid stack crashes */
                    lua_rawseti(L, -2, parameter_idx++);
                }

                /* Add this parameter to the behaviour parameter table */
                lua_rawset(L, parameter_tbl_idx);
            }

            /* Insert parameter table in behaviour table */
            lua_rawset(L, behaviour_tbl_idx);

            /* key = index, value = behaviour table */
            lua_pushvalue(L, -1); /* Duplicate behaviour table */
            lua_rawseti(L, class_tbl_idx, behaviour_idx++);

            /* key = name, value = behaviour table */
            lua_rawset(L, class_tbl_idx);
        }

        /* Add class table to supertable */
        lua_rawset(L, super_tbl_idx);
    }

    /* Drop string.lower */
    lua_remove(L, string_lower_idx);

/* Example output (shortened, and dumped as lua table constructor):
  ai = {
    ["processes"] = {
      [1] = {
        ["name"] = "look_for_other_mobs",
        ["parameters"] = {},
      },
      [2] = {
        ["name"] = "friendship",
        ["parameters"] = {
          ["arch"] = { },
          ["same_alignment"] = { [1] = 100, },
          ["player"] = { [1] = 0, },
          ["opposite_alignment"] = { [1] = -100, },
          ["name"] = { },
          <snip>
        },
      },
      ["look_for_other_mobs"] = ai["processes"][1],
      ["friendship"] = ai["processes"][2],
    },
    ["moves"] = {
        <snip>
    }
    ["actions"] = {
        <snip>
    },
  }
 */

    return 1; /* One return value on the stack */
}

/*****************************************************************************/
/* Name   : AI_ReloadBehaviourlist                                           */
/* Lua    : ai:ReloadBehaviourlist()                                         */
/* Info   : Recreates the AI from a new AI object in its mobs inventory      */
/*          (if there's no such AI object, defaults are used as usual)       */
/* Version: Introduced in beta 4 pre3                                        */
/* Status : Tested/Stable                                                    */
/*****************************************************************************/
static int AI_ReloadBehaviourlist(lua_State *L)
{
    lua_object *self;

    get_lua_args(L, "A", &self);

    hooks->reload_behaviours(self->data.object);

    return 0;
}

/* FUNCTIONEND */

/****************************************************************************/
/* Lua object management code                                               */
/****************************************************************************/

/* Compare two objects for equality */
static int AI_eq(struct lua_State *L)
{
    lua_object *lhs = lua_touserdata(L, 1);
    lua_object *rhs = lua_touserdata(L, 2);

    /* Should actually never happen. */
    if ((!lhs || lhs->class->type != LUATYPE_AI) ||
        (!rhs || rhs->class->type != LUATYPE_AI))
    {
        LOG(llevBug, "BUG:: %s/AI_eq(): Either/both LHS/RHS not AI objects!\n",
            __FILE__);

        return luaL_error(L, "AI_eq: Either/both LHS/RHS not AI objects!");
    }

    /* Test for LHS invalidity. */
    if (!lhs->class->isValid(L, lhs))
        return luaL_error(L, "AI_eq: LHS invalid!");

    /* Test for RHS invalidity. */
    if (!rhs->class->isValid(L, rhs))
        return luaL_error(L, "AI_eq: RHS invalid!");

    /* Compare tags. */
    lua_pushboolean(L, (lhs->tag == rhs->tag));

    return 1;
}

/* toString method for AI objects */
static int AI_toString(lua_State *L)
{
    lua_object *obj = lua_touserdata(L, 1);

    if (obj && obj->class->type == LUATYPE_AI)
        lua_pushfstring(L, "[%s]", STRING_OBJ_NAME(obj->data.object));
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
    LUATYPE_AI, "AI", 0, AI_eq, AI_toString, AI_attributes, AI_methods, NULL,
    NULL, NULL, NULL, NULL, AI_isValid, 0
};

int AI_init(lua_State *L)
{
    init_class(L, &AI);

    return 0;
}
