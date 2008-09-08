/*
 * This file uses the C preprocessor and a behaviour declaration file
 * written in the BDL (behaviour declaration language) to generate the necessary
 * static data needed for parsing behaviour configurations.
 *
 * To see what this file actually generates, you can run the following:
       gcc -I../include -E aiconfig.c | grep -v "#" | grep -v -e '^$'
 *
 * To generate a behaviour token list useful for gperf, run:
       gcc -E -D "BehaviourClass(name, behaviours)=behaviours" \
         -D "Behaviour(name, func, param)=name" -D NIL="" behaviourdecl.h \
         | grep -v "#" | grep -v -e '^$' | sed "s/ /\n/g"
 */

#include <aiconfig.h>

#define NIL
#ifndef NULL
#define NULL (void *)0
#endif

/* Generate behaviour parameter declarations */
#undef BehaviourClass
#undef Behaviour
#undef Parameter

#define INTEGER   AI_INTEGER_TYPE
#define STRING    AI_STRING_TYPE
#define STRINGINT AI_STRINGINT_TYPE
#define TYPEINT   AI_TYPEINT_TYPE
#define OPTIONAL  AI_OPTIONAL_PARAM
#define MULTI     AI_MULTI_PARAM
#define MANDATORY AI_MANDATORY_PARAM

#define BehaviourClass(name, behaviours) behaviours
#define Behaviour(name, func, params) \
    static struct behaviourparam_decl param_decl_ ## name[] = { params {NULL,0,0,0}};
#define Parameter(behaviour, name, type, flags, defval) \
{ #name, type, flags, (void *)defval },

#include BEHAVIOUR_DECLARATION_FILE

/* Generate extern declarations for behaviour callbacks */
#undef BehaviourClass
#undef Behaviour

#define BehaviourClass(name, behaviours) \
    extern name ## _behaviour_t behaviours __dummy_ ## name ## _ai_function_name;
#define Behaviour(name, func, params) \
    func,
#include BEHAVIOUR_DECLARATION_FILE

/* Generate the behaviour declarations */
#undef BehaviourClass
#undef Behaviour

#define BehaviourClass(name, behaviours) \
    static struct behaviour_decl class_decl_ ## name [] = { behaviours {NULL,0,0,0,0}};
#define Behaviour(name, func, params) \
    { #name, func, NROF_AIPARAMS_ ## name, param_decl_ ## name, AIBEHAVIOUR_ ## name},

#include BEHAVIOUR_DECLARATION_FILE

/* Generate the behaviour class declarations */
#undef BehaviourClass

#define BehaviourClass(x, y) \
    { #x, class_decl_ ## x },

struct behaviourclass_decl  behaviourclasses[NROF_BEHAVIOURCLASSES] =
{
    #include BEHAVIOUR_DECLARATION_FILE

};
