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

#ifndef __AICONFIG_H
#define __AICONFIG_H

/* Here we have to declare the behaviour function types for the different
 * behaviour classes
 */

struct  object_t;
struct  mob_behaviour_param;
struct  behaviour_move_response;
typedef void (PROCESSES_behaviour_t)(struct object_t *, struct mob_behaviour_param *);
typedef void (MOVES_behaviour_t)(struct object_t *, struct mob_behaviour_param *, struct behaviour_move_response *);
typedef int (ACTIONS_behaviour_t)(struct object_t *, struct mob_behaviour_param *);

/* Some flags and types we can use in the declaration */
enum
{
    AI_INTEGER_TYPE,
    AI_STRING_TYPE,
    AI_STRINGINT_TYPE
};
/** TYPEINTs are just STRINGINTs where the string is a type number */
#define AI_TYPEINT_TYPE AI_STRINGINT_TYPE

/* Attributes for parameter specifications */
#define AI_OPTIONAL_PARAM  0 /**< default: param is optional */
#define AI_MANDATORY_PARAM 1 /**< param is mandatory */
#define AI_SINGLE_PARAM    0 /**< default: param can only be specified once */
#define AI_MULTI_PARAM     2 /**< param can be specified 0 or more times */

struct behaviourparam_decl
{
    const char *name;
    int         type;
    int         attribs;
    void       *defaultvalue;
};

struct behaviour_decl
{
    const char                 *name;
    void                       *func;
    unsigned int                nrof_params;
    struct behaviourparam_decl *params;
    unsigned int                id;
};

struct behaviourclass_decl
{
    const char                         *name;
    struct behaviour_decl              *behaviours;
};

extern struct behaviourclass_decl   behaviourclasses[];

/* This is our BDL file declaring all the behaviours */
#define BEHAVIOUR_DECLARATION_FILE "behaviourdecl.h"

/*
 * Don't look below this line if ugly preprocessor hacks scares you
 */

/*
 * External sources including this file only want the enums for
 * behaviour classes and behaviour parameter indexes
 */

/* Avoid destroying a possibly important macro... */
#if defined NIL
#define _NIL_TMP_STORAGE NIL
#undef NIL
#endif

#define NIL /* For empty lists */

/* Declare behaviour class enum */
#define BehaviourClass(x, y) \
    BEHAVIOURCLASS_##x,
typedef enum
{
    BEHAVIOURCLASS_NONE                     = -1,
#include BEHAVIOUR_DECLARATION_FILE
    NROF_BEHAVIOURCLASSES
}    behaviourclass_t;

/* Declare behaviour enums */
#undef BehaviourClass
#undef Behaviour

#define BehaviourClass(name, behaviours) \
        enum { behaviours NROF_##name##_BEHAVIOURS};
#define Behaviour(name, func, params) \
        AIBEHAVIOUR_##name,
#include BEHAVIOUR_DECLARATION_FILE

/* Declare behaviour parameter enums */
#undef BehaviourClass
#undef Behaviour

#define BehaviourClass(name, behaviours) behaviours
#define Behaviour(name, func, params) \
    enum { params  NROF_AIPARAMS_##name };
#define Parameter(behaviour, name, type, flags, defval) \
    AIPARAM_##behaviour##_##name,
#include BEHAVIOUR_DECLARATION_FILE

/* Restore old NIL macro (if existing) */
#undef NIL
#if defined _NIL_TMP_STORAGE
#define NIL _NIL_TMP_STORAGE
#undef _NIL_TMP_STORAGE
#endif

#endif /* ifndef __AICONFIG_H */
