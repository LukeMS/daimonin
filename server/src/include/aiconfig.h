#ifndef __AICONFIG_H
#define __AICONFIG_H

/* Here we have to declare the behaviour function types for the different
 * behaviour classes
 */

struct  obj;
struct  mob_behaviour_param;
struct  behaviour_move_response;
typedef void (PROCESSES_behaviour_t)(struct obj *, struct mob_behaviour_param *);
typedef void (MOVES_behaviour_t)(struct obj *, struct mob_behaviour_param *, struct behaviour_move_response *);
typedef int (ACTIONS_behaviour_t)(struct obj *, struct mob_behaviour_param *);

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

#endif /* IFNDEF_AICONFIG_H */
