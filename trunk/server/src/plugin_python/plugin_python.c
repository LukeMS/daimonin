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
/* This module was original written for crossfire from gros.
 */

/*****************************************************************************/
/* CFPython - A Python module for Daimonin RPG.                             */
/*****************************************************************************/
/* The goal of this module is to provide support for Python scripts into     */
/* Daimonin. Python is here used in a extended way as a generic plugin.      */
/* Thats not a fast way to use this - but extrem flexible (we can load/      */
/* change and test script over and over in a running server) and easy to     */
/* extend - we simply don't add somewhere code in the server except some     */
/* jump points for the plugin model - if we want change someone the script   */
/* language or add another - it will not change anything in the plugin       */
/* interface.                                                                */
/*****************************************************************************/
/* Please note that it is still very beta - some of the functions may not    */
/* work as expected and could even cause the server to crash.                */
/*****************************************************************************/
/* Version history:                                                          */
/* 0.1 "Ophiuchus"   - Initial Alpha release                                 */
/* 0.5 "Stalingrad"  - Message length overflow corrected.                    */
/* 0.6 "Kharkov"     - Message and Write correctly redefined.                */
/*****************************************************************************/
/* Version: 0.6 Beta (also known as "Kharkov")                               */
/*****************************************************************************/
/* That code is placed under the GNU General Public Licence (GPL)            */
/* (C)2001 by Chachkoff Yann (Feel free to deliver your complaints)          */
/*****************************************************************************/

/* First let's include the header file needed                                */

#include <plugin_python.h>

#define PYTHON_DEBUG   /* give us some general infos out */

#undef MODULEAPI
#ifdef WIN32
#ifdef PYTHON_PLUGIN_EXPORTS
#define MODULEAPI __declspec(dllexport)
#else
#define MODULEAPI __declspec(dllimport)
#endif
#else
#define MODULEAPI
#endif

/*****************************************************************************/
/* And now the big part - The implementation of CFPython functions in C.     */
/* All comments for those functions have the following entries:              */
/* - The name of the function;                                               */
/* - How it is called from Python;                                           */
/* - The development state.                                                  */
/* The development state can be:                                             */
/* - Unknown  : Don't know if it has been tested already or not;             */
/* - Stable   : Has been tested and works under any common case;             */
/* - Untested : Not yet tested;                                              */
/* - Unstable : Has been tested, but caused some problems/bugged.            */
/* Such a system may seem quite silly and boring, but I already got some     */
/* success while using it, so I put it here too. Feel free to change the     */
/* status field of any function that you may have tested if needed.          */
/*****************************************************************************/
/* The functions that are simple wrappers to CF id numbers are not commented */
/* with that system since they don't need debugging because they're simple.  */
/*****************************************************************************/

/* Gecko: a comment like "GeckoStatus: untested" means that that function
 * is not tested with the new CFPython_Object implementation. Most functions
 * should have survived the transition, but some may have been missed.
 */

/* START several inline HOOK functions */

/* we don't have a hook for query_name() - we assume that this 
 * function only collect information and don't set/use any static
 * reference from crosslib.a
 */

static inline char *add_string_hook(char *stxt)
{
    CFParm* CFR;

	GCFP.Value[0] = (void *)(stxt);
    CFR=(PlugHooks[HOOK_ADDSTRING])(&GCFP);
	
	return (char *)CFR->Value[0];
}

#define FREE_STRING_HOOK(_txt_) free_string_hook(_txt_);_txt_=NULL; 
static inline void free_string_hook(char *stxt)
{
	GCFP.Value[0] = (void *)(stxt);
    (PlugHooks[HOOK_FREESTRING])(&GCFP);
}

static inline void fix_player_hook(object *fp1)
{
	GCFP.Value[0] = (void *)(fp1);
    (PlugHooks[HOOK_FIXPLAYER])(&GCFP);
}

static inline object *insert_ob_in_ob_hook(object *ob1, object *ob2)
{
    CFParm* CFR;

	GCFP.Value[0] = (void *)(ob1);
	GCFP.Value[1] = (void *)(ob2);
    CFR=(PlugHooks[HOOK_INSERTOBJECTINOB])(&GCFP);

	return (object *)CFR->Value[0];
}

/* END inline HOOK functions */

/* FUNCTIONSTART -- Here all the Python plugin functions come */

/*****************************************************************************/
/* Name   : CFGetMapWidth                                                    */
/* Python : CFPython.GetMapWidth(map)                                        */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetMapWidth(PyObject* self, PyObject* args)
{
    int val;
    CFPython_Map *map;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_MapType, &map))
        return NULL;
    val = map->map->width;
    return Py_BuildValue("i",val);
}

/*****************************************************************************/
/* Name   : CFGetMapHeight                                                   */
/* Python : CFPython.GetMapHeight(map)                                       */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetMapHeight(PyObject* self, PyObject* args)
{
    int val;
    CFPython_Map *map;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_MapType, &map))
        return NULL;
    val = map->map->height;
    return Py_BuildValue("i",val);
}

/*****************************************************************************/
/* Name   : CFGetObjectAt                                                    */
/* Python : CFPython.GetObjectAt(map,x,y)                                    */
/* Info   : FIXME seems to get the _last_ object on the square...            */
/*          FIXME What is the difference between this and                    */
/*           CFGETFirstObjectOnSquare ?                                      */
/* Status : Unfinished                                                       */
/*****************************************************************************/
static PyObject* CFGetObjectAt(PyObject* self, PyObject* args)
{
    int x, y;
    CFPython_Map *map;

	/*
	mapstruct *mt;
    object *myob = NULL;
    */

    if (!PyArg_ParseTuple(args,"O!ii", &CFPython_MapType, &map,&x,&y))
        return NULL;

	/* fixed for tiled maps. MT-2002 */
	/* we must use a hook here for out of map!
	if((mt=out_of_map(map->map,&x,&y)))
	    myob = get_map_ob(mt,x,y);

    return wrap_object(myob);
    */
	Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetValue                                                       */
/* Python : CFPython.SetValue(object,value)                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFSetValue(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int newvalue;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&newvalue))
        return NULL;

    WHO->value = newvalue;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetValue                                                       */
/* Python : CFPython.GetValue(object)                                        */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFGetValue(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->value);
}

/*****************************************************************************/
/* Name   : CFSetSkillExperience                                             */
/* Python : CFPython.SetSkillExperience(object,skillid,value)                */
/* Status : Unfinished                                                       */
/*****************************************************************************/
/* GeckoStatus: untested */
/* DO NOT USE - MUST BE REWRITTEN! */
static PyObject* CFSetSkillExperience(PyObject* self, PyObject* args)
{
    object *tmp;
    object *oldchosen;

    CFPython_Object *whoptr;

    int skill;
    long value;
    int currentxp;

    if (!PyArg_ParseTuple(args,"O!il", &CFPython_ObjectType, &whoptr,&skill,&value))
        return NULL;

    /* Browse the inventory of object to find a matching skill. */
    for (tmp=WHO->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type!=SKILL) continue;
        if(tmp->stats.sp!=skill) continue;

        if (tmp->exp_obj)
        {
            oldchosen = WHO->chosen_skill;
            WHO->chosen_skill = tmp;
            currentxp = tmp->exp_obj->stats.exp;
            GCFP.Value[0] = (void *)(WHO);

            value = value - currentxp;
            GCFP.Value[1] = (void *)(&value);
            /*GCFP.Value[2] = NULL;*/ /* FIX ME */
            (PlugHooks[HOOK_ADDEXP])(&GCFP);
            WHO->chosen_skill = oldchosen;

            Py_INCREF(Py_None);
            return Py_None;
        }
    }
	Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetSkillExperience                                             */
/* Python : CFPython.GetSkillExperience(object, skill)                       */
/* Info   : FIXME: what does it really return? Does this function need       */
/*          to be adjusted for Daimonin's skill system ?                     */
/*          (It seems to currently return the experience of the skill        */
/*           category, not the skill itself)                                 */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFGetSkillExperience(PyObject* self, PyObject* args)
{
    object *tmp;
    int skill;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&skill))
        return NULL;

    /* Browse the inventory of object to find a matching skill. */
    for (tmp=WHO->inv;tmp;tmp=tmp->below)
    {
        if(tmp->type!=SKILL) continue;
        if(tmp->stats.sp!=skill) continue;
        if (tmp->exp_obj)
        {
            return Py_BuildValue("l",(long)(tmp->exp_obj->stats.exp));
        }
    }
    RAISE("Couldn't find requested skill");
}

/*****************************************************************************/
/* Name   : CFMatchString                                                    */
/* Python : CFPython.MatchString(firststr,secondstr)                         */
/* Info   : Case insensitive string comparision. Returns 1 if the two        */
/*          strings are the same, or 0 if they differ.                       */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFMatchString(PyObject* self, PyObject* args)
{
    char *premiere;
    char *seconde;
    char *result;

    if (!PyArg_ParseTuple(args,"ss",&premiere,&seconde))
        return NULL;

    result = re_cmp(premiere, seconde);
    if (result != NULL)
        return Py_BuildValue("i",1);
    else
        return Py_BuildValue("i",0);
}

/*****************************************************************************/
/* Name   : CFSetCursed                                                      */
/* Python : CFPython.SetCursed(object,value)                                 */
/* Info   : If value is 0, object is uncursed. Otherwise object is cursed.   */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFSetCursed(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value!=0)
        SET_FLAG(WHO, FLAG_CURSED);
    else     
        CLEAR_FLAG(WHO, FLAG_CURSED);

    /* Gecko: Make sure the inventory icon is updated */
    /* FIXME: what if object was not carried by player, or in a container ? */
    if (WHO->env != NULL && WHO->env->type == PLAYER)
    {
        GCFP.Value[0] = (void *)(WHO->env);
        GCFP.Value[1] = (void *)(WHO->env);
        (PlugHooks[HOOK_ESRVSENDINVENTORY])(&GCFP);
    }
 
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFActivateRune                                                   */
/* Python : CFPython.ActivateRune(object,objectwhat)                         */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFActivateRune(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;

    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &whatptr))
        return NULL;

    GCFP.Value[0] = (void *)(WHAT);
    GCFP.Value[1] = (void *)(WHO);
    (PlugHooks[HOOK_SPRINGTRAP])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFCheckTrigger                                                   */
/* Python : CFPython.CheckTrigger(object,objectwhat)                         */
/* Status : Unfinished                                                       */
/*****************************************************************************/
/* GeckoStatus: untested */
/* MUST DO THE HOOK HERE ! */
static PyObject* CFCheckTrigger(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;

    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &whatptr))
        return NULL;

   /* check_trigger(WHAT,WHO); should be hook too! */

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetUnaggressive                                                */
/* Python : CFPython.SetUnaggressive(who,value)                              */
/* Info   : FIXME: Gecko: find out when this works and not (doesn't seem to  */
/*          work on friendly mobs)                                           */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFSetUnaggressive(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value!=0)
        SET_FLAG(WHO, FLAG_UNAGGRESSIVE);
    else
        CLEAR_FLAG(WHO, FLAG_UNAGGRESSIVE);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFCastAbility                                                    */
/* Python : CFPython.CastAbility(caster,target,spellno,mode,direction,option)*/
/* Info   : caster casts the ability numbered spellno on target.             */
/*          mode: 0 = normal, 1 = potion                                     */
/*          direction is the direction to cast the ability in                */
/*          option is additional string option(s)                            */
/*          FIXME: only allows for directional abilities?                    */
/*          Abilities are can be cast in magic-blocking areas, and do not    */
/*          use magicattack.                                                 */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCastAbility(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
	CFPython_Object *target;
    int spell;
    int dir;
	int mode;
    char* op;
    CFParm* CFR;
    int parm=1;
    int parm2;
    int typeoffire = FIRE_DIRECTIONAL;

	if (!PyArg_ParseTuple(args,"O!O!iiis", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &target, 
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
/* Name   : CFGetMapPath                                                     */
/* Python : CFPython.GetMapPath(map)                                         */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetMapPath(PyObject* self, PyObject* args)
{
    CFPython_Map *mapptr;
    if (!PyArg_ParseTuple(args,"O!",&CFPython_MapType, &mapptr))
        return NULL;

    return Py_BuildValue("s",mapptr->map->path);
};

/*****************************************************************************/
/* Name   : CFGetMessage                                                     */
/* Python : CFPython.GetMessage(object)                                      */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFGetMessage(PyObject* self, PyObject* args)
{
    /* Stalingrad: extended the buffer - added a boundary checking */
    /* (implementing this as a malloc'ed string problematic under some env.) */
    /* Now declared static to help preventing memory leaks */
    static char buf[4096];
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    if (WHO->msg != NULL)
    {
        if (strlen(WHO->msg)>=4096)
        {
            LOG(llevDebug,"Warning ! Buffer overflow - The message will be truncated\n");
            strncpy(buf, WHO->msg, 4096);
            buf[4095]=0x0;
        }
        else
        {
            strncpy(buf, WHO->msg,strlen(WHO->msg));
            buf[strlen(WHO->msg)]=0x0;
        }
    }
    else
        buf[0] = 0x0;
    return Py_BuildValue("s",buf);
}

/*****************************************************************************/
/* Name   : CFSetMessage                                                     */
/* Python : CFPython.SetMessage(object,message)                              */
/* Info   : FIXME Gecko: Need check for "endmsg" ?                           */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetMessage(PyObject* self, PyObject* args)
{
    char *txt;
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr, &txt))
        return NULL;

    if (WHO->msg != NULL)
        FREE_STRING_HOOK(WHO->msg);
    WHO->msg = add_string_hook(txt);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetGod                                                         */
/* Python : CFPython.GetGod(object)                                          */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFGetGod(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFParm* CFR;
    static char* value;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_DETERMINEGOD])(&GCFP);
    value = (char *)(CFR->Value[0]);
    free(CFR);
    return Py_BuildValue("s",value);
}

/*****************************************************************************/
/* Name   : CFSetGod                                                         */
/* Python : CFPython.SetGod(object,godstr)                                   */
/* Status : Unfinished!                                                      */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFSetGod(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char* txt;
    char* prayname;
    object* tmp;
    CFParm* CFR0;
    CFParm* CFR;
    int value;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&txt))
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
/* Name   : CFSetWeight                                                      */
/* Python : CFPython.SetWeight(object,value)                                 */
/* Info   : FIXME: classic inventory problem (needs to update info)          */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFSetWeight(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    long value;

    if (!PyArg_ParseTuple(args,"O!l", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value < 0)
        RAISE("Weight must be greater than 0");

    WHO->weight = value;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFReadyMap                                                       */
/* Python : CFPython.ReadyMap(name, unique)                                  */
/* Info   : Make sure the named map is loaded into memory. unique _must_ be  */
/*          1 if the map is unique. (unique is optional with default = 0)    */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFReadyMap(PyObject* self, PyObject* args)
{
    char *mapname;
    mapstruct *mymap;
    int flags = 0, unique = 0;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"s|i",&mapname, &unique))
        return NULL;

    if(unique)
        flags = MAP_PLAYER_UNIQUE;
    GCFP.Value[0] = (void *)(mapname);
    GCFP.Value[1] = (void *)(&flags);

    LOG(llevDebug, "Ready to call readymapname with %s %i\n",
        (char *)(GCFP.Value[0]),
        *(int *)(GCFP.Value[1])
    );
    /* mymap = ready_map_name(mapname,0); */
    CFR = (PlugHooks[HOOK_READYMAPNAME])(&GCFP);
    mymap = (mapstruct *)(CFR->Value[0]);
    if(mymap != NULL)
        LOG(llevDebug, "Map file is %s\n",mymap->path);
    free(CFR);
    return wrap_map(mymap);
}

/*****************************************************************************/
/* Name   : CFTeleport                                                       */
/* Python : CFPython.Teleport(object,mapname,x,y, unique)                    */
/* Info   : Teleports object to the given position of the named map. If      */
/*          unique is set to 1, the map will be a unique map.                */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFTeleport(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *map, *msg=NULL;
    int x, y, u;

    if (!PyArg_ParseTuple(args,"O!siii", &CFPython_ObjectType, &whoptr,&map,&x,&y, &u))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (char *)(map);
    GCFP.Value[2] = (void *)(&x);
    GCFP.Value[3] = (void *)(&y);
    GCFP.Value[4] = (void *)(&u);
    GCFP.Value[5] = (char *)(msg);
    (PlugHooks[HOOK_TELEPORTOBJECT])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFIsOutOfMap                                                     */
/* Python : CFPython.IsOutOfMap(object,x,y)                                  */
/* Status : UNFINISHED!                                                      */
/*****************************************************************************/
/* GeckoStatus: untested */
/* must use hook for out_of_map! */
static PyObject* CFIsOutOfMap(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int x, y;

    if (!PyArg_ParseTuple(args,"O!ii", &CFPython_ObjectType, &whoptr,&x,&y))
        return NULL;

    return Py_BuildValue("i", OUT_OF_REAL_MAP(WHO->map,x,y));
}

/*****************************************************************************/
/* Name   : CFPickUp                                                         */
/* Python : CFPython.PickUp(object,whatob)                                   */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* CFPickUp(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;

    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &whatptr))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(WHAT);
    (PlugHooks[HOOK_PICKUP])(&GCFP);
    /*pick_up(WHO,WHAT); */
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetWeight                                                      */
/* Python : CFPython.GetWeight(object)                                       */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFGetWeight(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("l",WHO->weight);
}


/*****************************************************************************/
/* Name   : CFIsCanBePicked                                                  */
/* Python : CFPython.CanBePicked(object)                                     */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsCanBePicked(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    /* 0: can't be picked up - 1: can be picked up */
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_NO_PICK)?0:1);
}

/*****************************************************************************/
/* Name   : CFGetMap                                                         */
/* Python : CFPython.GetMap(object)                                          */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetMap(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return wrap_map(WHO->map);
}

/*****************************************************************************/
/* Name   : CFSetNextObject                                                  */
/* Python : CFPython.SetNextObject(object,object)                            */
/* Status : Stable                                                           */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFSetNextObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;
    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &whatptr))
        return NULL;

    WHO->below = WHAT;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetPreviousObject                                              */
/* Python : CFPython.SetPreviousObject(object,object)                        */
/* Status : Stable                                                           */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetPreviousObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;
    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &whatptr))
        return NULL;

    WHO->above = WHAT;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetNextObject                                                  */
/* Python : CFPython.GetNextObject(object)                                   */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetNextObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return wrap_object(WHO->below);
}

/*****************************************************************************/
/* Name   : CFGetPreviousObject                                              */
/* Python : CFPython.GetPreviousObject(object)                               */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetPreviousObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return wrap_object(WHO->above);
}

/*****************************************************************************/
/* Name   : CFGetFirstObjectOnSquare                                         */
/* Python : CFPython.GetFirstObjectOnSquare(map,x,y)                         */
/* Info   : FIXME seems to get the _last_ object on the square...            */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetFirstObjectOnSquare(PyObject* self, PyObject* args)
{
    CFPython_Map *map;
    int x, y;
    object* val;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"O!ii",&CFPython_MapType, &map,&x,&y))
        return NULL;

    GCFP.Value[0] = map->map;
    GCFP.Value[1] = (void *)(&x);
    GCFP.Value[2] = (void *)(&y);
    CFR = (PlugHooks[HOOK_GETMAPOBJECT])(&GCFP);
    val = (object *)(CFR->Value[0]);
    free(CFR);
    return wrap_object(val);
}

/*****************************************************************************/
/* Name   : CFSetQuantity                                                    */
/* Python : CFPython.SetQuantity(object,nrof)                                */
/* Info   : FIXME: classic inventory problem (needs to update info)          */
/*          FIXME: Quantities are 0 by default... Why?                       */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* CFSetQuantity(PyObject* self, PyObject* args)
{
    CFPython_Object *whatptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whatptr,&value))
        return NULL;

    /* I used an arbitrary bound of 100k here */
    if (value < 0 || value > 100000 )
        RAISE("Quantity must be between 0 and 100000");

    WHAT->nrof = value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetQuantity                                                    */
/* Python : CFPython.GetQuantity(object)                                     */
/*          FIXME: Quantities are 0 by default... Why?                       */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* CFGetQuantity(PyObject* self, PyObject* args)
{
    CFPython_Object *whatptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whatptr))
        return NULL;

    return Py_BuildValue("l",WHAT->nrof);
}

/*****************************************************************************/
/* Name   : CFInsertObjectInside                                             */
/* Python : CFPython.InsertObjectInside(object, where)                       */
/* Info   : Inserts object into where.                                       */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFInsertObjectInside(PyObject* self, PyObject* args)
{
    CFPython_Object *whatptr;
    CFPython_Object *whereptr;
    object *myob;
    object *obenv;
    
    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whatptr, &CFPython_ObjectType, &whereptr))
        return NULL;

    myob = WHAT;
    obenv = myob->env;
    
    if (!QUERY_FLAG(myob,FLAG_REMOVED))
    {
        GCFP.Value[0] = (void *)(myob);
        (PlugHooks[HOOK_REMOVEOBJECT])(&GCFP);
    }
    
    myob = insert_ob_in_ob_hook(myob, WHERE);

    /* If we're inserting into player's inventory: notify client  */
    /* FIXME: what if inserting into player's container? (classic inventory prob.) */
    if (WHERE->type == PLAYER)
    {
        GCFP.Value[0] = (void *)(WHERE);
        GCFP.Value[1] = (void *)(myob);
        (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
    } 
    /* If we're taking from player. */
    /* FIXME: what if myob was in a container on the same spot as the player? 
     * (Works fine if object is _put_ in a container on the same spot)   */
    else if (obenv != NULL && obenv->type == PLAYER)
    {
        GCFP.Value[0] = (void *)(obenv);
        GCFP.Value[1] = (void *)(obenv);
        (PlugHooks[HOOK_ESRVSENDINVENTORY])(&GCFP);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFFindPlayer                                                     */
/* Python : CFPython.FindPlayer(name)                                        */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* CFFindPlayer(PyObject* self, PyObject* args)
{
    player *foundpl;
    object *foundob = NULL;
    CFParm *CFR;
    char* txt;

    if (!PyArg_ParseTuple(args,"s",&txt))
        return NULL;

    GCFP.Value[0] = (void *)(txt);
    CFR = (PlugHooks[HOOK_FINDPLAYER])(&GCFP);
    foundpl = (player *)(CFR->Value[0]);
    free(CFR);

    if (foundpl!=NULL)
        foundob = foundpl->ob;

    return wrap_object(foundob);
}

/*****************************************************************************/
/* Name   : CFApply                                                          */
/* Python : CFPython.Apply(object, whatobj, flags)                           */
/* Info   : forces object to apply whatobj.                                  */
/*          flags:   0 - normal apply (toggle)                               */
/*                   1 - always apply (AP_APPLY)                             */
/*                   2 - always unapply (AP_UNAPPLY)                         */
/*                  16 - don't merge unapplied items (AP_NO_MERGE)           */
/*                  32 - unapply cursed items (AP_IGNORE_CURSE)              */
/*          returns: 0 - object cannot apply objects of that type.           */
/*                   1 - object was applied, or not...                       */
/*                   2 - object must be in inventory to be applied           */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFApply(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;
    int flags;
    CFParm* CFR;
    int retval;

    if (!PyArg_ParseTuple(args,"O!O!i", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &whatptr,&flags))
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
/* Name   : CFDrop                                                           */
/* Python : CFPython.Drop(player, name)                                      */
/* Info   : Equivalent to the player command "drop" (name is an object name, */
/*          "all", "unpaid", "cursed", "unlocked" or a count + object name : */
/*          "<nnn> <object name>", or a base name, or a short name...)       */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* CFDrop(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char* name;
    CFParm* CFR; 

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&name))
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
/* Name   : CFTake                                                           */
/* Python : CFPython.Take(object,name)                                       */
/* Status : Temporary disabled (see commands.c)                              */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFTake(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char* name;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&name))
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
/* Name   : CFIsInvisible                                                    */
/* Python : CFPython.IsInvisible(object)                                     */
/* Info   : Returns 1 if object is invisible, 0 if not.                      */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFIsInvisible(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_IS_INVISIBLE));
}

/*****************************************************************************/
/* Name   : CFWhoAmI                                                         */
/* Python : CFPython.WhoAmI()                                                */
/* Info   : Get the owner of the active script (the object that has the      */
/*          event handler)                                                   */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFWhoAmI(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return wrap_object(StackWho[StackPosition]);
}

/*****************************************************************************/
/* Name   : CFWhoIsActivator                                                 */
/* Python : CFPython.WhoIsActivator()                                        */
/* Info   : Gets the object that activated the current event                 */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFWhoIsActivator(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return wrap_object(StackActivator[StackPosition]);
}

/*****************************************************************************/
/* Name   : CFWhatIsMessage                                                  */
/* Python : CFPython.WhatIsMessage()                                         */
/* Info   : Gets the actual message in SAY events.                           */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFWhatIsMessage(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("s",StackText[StackPosition]);
}

/*****************************************************************************/
/* Name   : CFSay                                                            */
/* Python : CFPython.Say(object,message)                                     */
/* Info   : object says message to everybody on its map                      */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSay(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *message;
    static char buf[MAX_BUF*2];
    int val;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr, &message))
        return NULL;

	/* old dynamic buffer */
    /*buf = (char *)(malloc(sizeof(char)*(strlen(message)+strlen(query_name(who))+20)));*/
    sprintf(buf, "%s says: %s", query_name(WHO),message);

    val = NDI_NAVY|NDI_UNIQUE;

    GCFP.Value[0] = (void *)(&val);
    GCFP.Value[1] = (void *)(WHO->map);
    GCFP.Value[2] = (void *)(buf);

    (PlugHooks[HOOK_NEWINFOMAP])(&GCFP);

    /*free(buf);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSayTo                                                          */
/* Python : CFPython.SayTo(source, target, message)                          */
/* Status : Stable                                                           */
/* Info   : NPC talks only to player but map get a "xx talks to" msg too.    */
/*****************************************************************************/
static PyObject* CFSayTo(PyObject* self, PyObject* args)
{    
    object *who,*target;
    CFPython_Object *obptr, *obptr2;
	int zero = 0;
    char *message;
    static char buf[MAX_BUF*2];
    int val;

    if (!PyArg_ParseTuple(args,"O!O!s", &CFPython_ObjectType, &obptr, &CFPython_ObjectType, &obptr2, &message))
        return NULL;

    who = obptr->obj;
    target = obptr2->obj;

    /*buf = (char *)(malloc(sizeof(char)*(strlen(message)+strlen(query_name(who))+20)));*/
    
    sprintf(buf, "%s talks to %s.", query_name(who),query_name(target));
	val = NDI_UNIQUE;

    GCFP.Value[0] = (void *)(&val);
    GCFP.Value[1] = (void *)(who->map);
    GCFP.Value[2] = (void *)(target);
    GCFP.Value[3] = (void *)(buf);
    (PlugHooks[HOOK_NEWINFOMAPEXCEPT])(&GCFP);

    sprintf(buf, "%s says: %s", query_name(who),message);
	val = NDI_NAVY|NDI_UNIQUE;
	GCFP.Value[0] = (void *)(&val);
    GCFP.Value[1] = (void *)(&zero);
    GCFP.Value[2] = (void *)(target);
    GCFP.Value[3] = (void *)(buf);
    (PlugHooks[HOOK_NEWDRAWINFO])(&GCFP);

    /*free(buf);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetGender                                                      */
/* Python : CFPython.SetGender(object,gender_string)                         */
/* Info   : Changes the gender of object. gender_string should be one of     */
/*          "male", "female" or "neuter"                                     */
/*          FIXME: So what about bi-sexed creatures (snails etc?)            */
/*          FIXME: We also need a GetGender function...                      */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetGender(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *gender;
    
    if (!PyArg_ParseTuple(args,"O!s",&CFPython_ObjectType, &whoptr,&gender))
        return NULL;

	/* set object to neuter */
	CLEAR_FLAG(WHO,FLAG_IS_MALE);
	CLEAR_FLAG(WHO,FLAG_IS_FEMALE);

	/* reset to male or female */
	if(strcmp(gender,"male") == 0)
		SET_FLAG(WHO,FLAG_IS_MALE);
	else if(strcmp(gender,"female") == 0)
		SET_FLAG(WHO,FLAG_IS_FEMALE);

	/* update the players client of object was a player */
	if(WHO->type == PLAYER)
		WHO->contr->socket.ext_title_flag = 1; /* demand update to client */

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetRank                                                        */
/* Python : CFPython.SetRank(object,rank_string)                             */
/* Info   : Set the rank of an object to rank_string                         */
/*          Rank string 'Mr' is special for no rank                          */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetRank(PyObject* self, PyObject* args)
{
    object *walk;
    CFPython_Object *whoptr;
    char *rank;
    
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr, &rank))
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
/* Name   : CFSetAlignment                                                   */
/* Python : CFPython.SetAlignment(object,alignment_string)                   */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetAlignment(PyObject* self, PyObject* args)
{
    object *walk;
    CFPython_Object *whoptr;
    char *align;
    
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&align))
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
/* Name   : CFGetAlignmentForce                                              */
/* Python : CFPython.GetAlignmentForce(object)                               */
/* Status : Stable                                                           */
/* Info   : This gets the aligment_force from a inventory (should be player?)*/
/*****************************************************************************/
static PyObject* CFGetAlignmentForce(PyObject* self, PyObject* args)
{
    object *walk;
    CFPython_Object *whoptr;
    
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
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
/* Name   : CFSetGuildForce                                                  */
/* Python : CFPython.SetGuildForce(object, rank_string)                      */
/* Info   : Sets the current rank of object to rank_string. Returns          */
/*          the guild_force object that was modified.                        */
/* Status : Stable                                                           */
/* Warning: This set only the title. The guild tag is in <slaying>           */
/*          For test of a special guild, you must use GetGuild()             */
/*          For settings inside a guild script, you can use this function    */
/*          Because it returns the guild_force obj after setting the title   */
/*****************************************************************************/
static PyObject* CFSetGuildForce(PyObject* self, PyObject* args)
{
    object *walk;
    char *guild;
    CFPython_Object *whoptr;
    
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr, &guild))
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
/* Name   : CFGetGuildForce                                                  */
/* Python : CFPython.GetGuildForce(who)                                      */
/* Info   : This gets the guild_force from a inventory (should be player?)   */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetGuildForce(PyObject* self, PyObject* args)
{
    object *walk;
    CFPython_Object *whoptr;
    
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
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
/* Name   : CFSetInvisible                                                   */
/* Python : CFPython.SetInvisible(object,value)                              */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFSetInvisible(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

	if(value)
		SET_FLAG(WHO, FLAG_IS_INVISIBLE);
	else
		CLEAR_FLAG(WHO, FLAG_IS_INVISIBLE);

	Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetExperience                                                  */
/* Python : CFPython.GetExperience(object)                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetExperience(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("l",WHO->stats.exp);
}

/*****************************************************************************/
/* Name   : CFGetLevel                                                       */
/* Python : CFPython.GetLevel(object)                                        */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetLevel(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("l", WHO->level );
}

/*****************************************************************************/
/* Name   : CFGetSpeed                                                       */
/* Python : CFPython.GetSpeed(object)                                        */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetSpeed(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("d",WHO->speed);
}

/*****************************************************************************/
/* Name   : CFSetSpeed                                                       */
/* Python : CFPython.SetSpeed(object,value)                                  */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFSetSpeed(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    double value;

    if (!PyArg_ParseTuple(args,"O!d", &CFPython_ObjectType, &whoptr,&value))
        return NULL;
    if (value< -9.99 || value > 9.99)
        RAISE("Illegal speed. Must be between -10.0 and 10.0");

    WHO->speed = (float) value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetFood                                                        */
/* Python : CFPython.GetFood(object)                                         */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFGetFood(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->stats.food);
}

/*****************************************************************************/
/* Name   : CFSetFood                                                        */
/* Python : CFPython.SetFood(object, value)                                  */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFSetFood(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;
    
    if (value<0 || value > 999)
        RAISE("Amount of food must be between 0 and 999");

    WHO->stats.food = value;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetGrace                                                       */
/* Python : CFPython.GetGrace(object)                                        */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFGetGrace(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->stats.grace);
}

/*****************************************************************************/
/* Name   : CFSetGrace                                                       */
/* Python : CFPython.SetGrace(object, value)                                 */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFSetGrace(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-16000 || value > 16000)
        RAISE("Grace must be between -16000 and 16000");

    WHO->stats.grace = value;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetReturnValue                                                 */
/* Python : CFPython.GetReturnValue()                                        */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetReturnValue(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;

    return Py_BuildValue("i",StackReturn[StackPosition]);
}

/*****************************************************************************/
/* Name   : CFSetReturnValue                                                 */
/* Python : CFPython.SetReturnValue(value)                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFSetReturnValue(PyObject* self, PyObject* args)
{
    int value;
    if (!PyArg_ParseTuple(args,"i",&value))
        return NULL;

    StackReturn[StackPosition] = value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetDirection                                                   */
/* Python : CFPython.GetDirection(object)                                    */
/* Info   : So what is "direction" anyway?                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetDirection(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->direction);
}

/*****************************************************************************/
/* Name   : CFSetDirection                                                   */
/* Python : CFPython.SetDirection(object, value)                             */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */
/* this function will fail imho - for animation[] we need to call a hook! */

static PyObject* CFSetDirection(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    WHO->direction = value;
    SET_ANIMATION(WHO, WHO->direction);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetLastSP                                                      */
/* Python : CFPython.GetLastSP(object)                                       */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetLastSP(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->last_sp);
}

/*****************************************************************************/
/* Name   : CFSetLastSP                                                      */
/* Python : CFPython.SetLastSP(object, sp)                                   */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetLastSP(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<0 || value > 16000)
        RAISE("Last SP must be between 0 and 16000");

    WHO->last_sp = value;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetLastGrace                                                   */
/* Python : CFPython.GetLastGrace(object)                                    */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetLastGrace(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->last_grace);
}

/*****************************************************************************/
/* Name   : CFSetLastGrace                                                   */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetLastGrace(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<0 || value > 16000)
        RAISE("Grace must be between 0 and 16000");

    WHO->last_grace = value;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFFixObject                                                      */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
static PyObject* CFFixObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    fix_player_hook(WHO);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetFace                                                        */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetFace(PyObject* self, PyObject* args)
{
    char* txt;
    CFPython_Object *whoptr;
    CFParm* CFR;
    int val = UP_OBJ_FACE;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&txt))
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

/*****************************************************************************/
/* Name   : CFGetAttackType                                                  */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
static PyObject* CFGetAttackType(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->attacktype);
}

/*****************************************************************************/
/* Name   : CFSetAttackType                                                  */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetAttackType(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    WHO->attacktype = value;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetDamage                                                      */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetDamage(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<0 || value > 120)
        RAISE("Damage must be between 0 and 120");

    WHO->stats.dam = value;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetDamage                                                      */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetDamage(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return Py_BuildValue("i",WHO->stats.dam);
}

/*****************************************************************************/
/* Name   : CFSetBeenApplied                                                 */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetBeenApplied(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value!=0)
        SET_FLAG(WHO,FLAG_BEEN_APPLIED);
    else
        CLEAR_FLAG(WHO,FLAG_BEEN_APPLIED);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetIdentified                                                  */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetIdentified(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value!=0)
        SET_FLAG(WHO,FLAG_IDENTIFIED);
    else
        CLEAR_FLAG(WHO,FLAG_IDENTIFIED);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFKillObject                                                     */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */
/* add hooks before use! */

static PyObject* CFKillObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;
    int ktype;
    int k = 1;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"O!O!i", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &whatptr,&ktype))
        return NULL;

    WHAT->speed = 0;
    WHAT->speed_left = 0.0;
    update_ob_speed(WHAT); /* NEED HOOK HERE ! */

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
/* Name   : CFWhoIsOther                                                     */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFWhoIsOther(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return wrap_object(StackOther[StackPosition]);
}

/*****************************************************************************/
/* Name   : CFDirectionN                                                     */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionN(PyObject* self, PyObject* args)
{
    int i=1;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFDirectionNE                                                    */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionNE(PyObject* self, PyObject* args)
{
    int i=2;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFDirectionE                                                     */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionE(PyObject* self, PyObject* args)
{
    int i=3;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFDirectionSE                                                    */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionSE(PyObject* self, PyObject* args)
{
    int i=4;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFDirectionS                                                     */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionS(PyObject* self, PyObject* args)
{
    int i=5;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFDirectionSW                                                    */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionSW(PyObject* self, PyObject* args)
{
    int i=6;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFDirectionW                                                     */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionW(PyObject* self, PyObject* args)
{
    int i=7;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFDirectionNW                                                    */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/

static PyObject* CFDirectionNW(PyObject* self, PyObject* args)
{
    int i=8;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",i);
}

/*****************************************************************************/
/* Name   : CFCastSpell                                                      */
/* Python : CFPython.CastSpell(caster,target,spell,mode,direction,option)    */
/* Info   : caster casts the spell numbered spellno on target.               */
/*          mode: 0 = normal, 1 = potion                                     */
/*          direction is the direction to cast the spell in                  */
/*          option is additional string option(s)                            */
/*          NPCs can cast spells even in no-spell areas.                     */
/*          FIXME: only allows for directional spells                        */
/*          FIXME: is direction/position relative to target? (0 = self)      */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFCastSpell(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *target;
    int spell;
    int dir;
	int mode;
    char* op;
    CFParm* CFR;
    int parm=0;
    int parm2;
    int typeoffire = FIRE_DIRECTIONAL;

    if (!PyArg_ParseTuple(args,"O!O!iiis", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &target, 
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
/* Name   : CFGetSpellNr                                                     */
/* Python : CFGetSpellNr(name)                                               */
/* Info   : Gets the number of the named spell. -1 if no such spell exists   */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFGetSpellNr(PyObject* self, PyObject* args)
{
    char *spell;
    CFParm* CFR;
    int value;

    if (!PyArg_ParseTuple(args,"s",&spell))
        return NULL;

    GCFP.Value[0] = (void *)(spell);
    CFR = (PlugHooks[HOOK_CHECKFORSPELLNAME])(&GCFP);
    value = *(int *)(CFR->Value[0]);
    return Py_BuildValue("i",value);
}

/*****************************************************************************/
/* Name   : CFDoKnowSpell                                                    */
/* Python : CFDoKnowSpell(object, spell)                                     */
/* Info   : 1 if the spell is known by object, 0 if it isn't                 */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFDoKnowSpell(PyObject* self, PyObject* args)
{
    int spell;
    CFPython_Object *whoptr;
    CFParm* CFR;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&spell))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&spell);
    CFR = (PlugHooks[HOOK_CHECKFORSPELL])(&GCFP);
    value = *(int *)(CFR->Value[0]);
    free(CFR);
    return Py_BuildValue("i",value);
}

/*****************************************************************************/
/* Name   : CFAcquireSpell                                                   */
/* Python : CFPython.AcquireSpell(object, spell, mode)                       */
/* Info   : object will learn or unlearn spell. mode: 0=learn, 1=unlearn     */
/*          (remove)                                                         */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFAcquireSpell(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int spell;
    int mode;

    if (!PyArg_ParseTuple(args,"O!ii", &CFPython_ObjectType, &whoptr,&spell, &mode))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&spell);
    GCFP.Value[2] = (void *)(&mode);
    (PlugHooks[HOOK_LEARNSPELL])(&GCFP);   
    
    Py_INCREF(Py_None);
    return Py_None;

}


/*****************************************************************************/
/* Name   : CFGetSkillNr                                                     */
/* Python : CFPython.GetSkillNr(name)                                        */
/* Info   : Gets the number of the named skill. -1 if no such skill exists   */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFGetSkillNr(PyObject* self, PyObject* args)
{
    char *skill;
	CFParm* CFR;
    int value;
  
    if (!PyArg_ParseTuple(args,"s",&skill))
        return NULL;

    GCFP.Value[0] = (void *)(skill);
    CFR = (PlugHooks[HOOK_CHECKFORSKILLNAME])(&GCFP);
    value = *(int *)(CFR->Value[0]);
    return Py_BuildValue("i",value);
}

/*****************************************************************************/
/* Name   : CFDoKnowSkill                                                    */
/* Python : CFDoKnowSkill(object, spell)                                     */
/* Info   : 1 if the skill is known by object, 0 if it isn't                 */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFDoKnowSkill(PyObject* self, PyObject* args)
{
    int skill;
    CFPython_Object *whoptr;
    CFParm* CFR;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&skill))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&skill);
    CFR = (PlugHooks[HOOK_CHECKFORSKILLKNOWN])(&GCFP);
    value = *(int *)(CFR->Value[0]);
    free(CFR);
    return Py_BuildValue("i",value);
}

/*****************************************************************************/
/* Name   : CFAcquireSkill                                                   */
/* Python : CFAcquireSkill(object, skillno, mode)                            */
/* Info   : object will learn or unlearn skill. mode: 0=learn, 1=unlearn     */
/*        : Get skill number with CFGetSkillNr()                             */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFAcquireSkill(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int skill, mode;
    
    if (!PyArg_ParseTuple(args,"O!ii", &CFPython_ObjectType, &whoptr, &skill, &mode))
        return NULL;
            
    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(&skill);
    GCFP.Value[2] = (void *)(&mode);
    (PlugHooks[HOOK_LEARNSKILL])(&GCFP);   
    
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFFindMarkedObject                                               */
/* Python : CFPython.FindMarkedObject(who)                                   */
/* Info   : Returns the marked object in who's inventory, or None if no      */
/*          object is marked.                                                */
/*          FIXME: also search inside containers (or don't allow marking     */
/*          stuff in containers)                                             */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFFindMarkedObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    object * value;
    CFParm* CFR;
    
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
            
    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_FINDMARKEDOBJECT])(&GCFP);   
    
    value = (object *)(CFR->Value[0]);
    /*free(CFR); findmarkedobject use static parameters */
    return wrap_object(value);
}

/*****************************************************************************/
/* Name   : CFCheckInvisibleInside                                           */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFCheckInvisibleInside(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *id;
    object* tmp2;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&id))
        return NULL;

    for(tmp2=WHO->inv;tmp2 !=NULL; tmp2=tmp2->below)
    {
        if(tmp2->type == FORCE && tmp2->slaying && !strcmp(tmp2->slaying,id))
            break;
    }

    return wrap_object(tmp2);
}

/*****************************************************************************/
/* Name   : CFCreatePlayerForce                                              */
/* Python :                                                                  */
/* Status : Stable.                                                          */
/* Info   : The Values of a player force will effect the player.             */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFCreatePlayerForce(PyObject* self, PyObject* args)
{
    char* txt;
    char txt2[16];
    object *myob;
    CFPython_Object *whereptr;
    CFParm* CFR;
    
    if (!PyArg_ParseTuple(args,"O!s",&CFPython_ObjectType, &whereptr,&txt))
        return NULL;
    
    strcpy(txt2,"player_force");
    
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
/* Name   : CFCreatePlayerInfo                                               */
/* Python : CFCreatePlayerInfo(who, name)                                    */
/* Status : Stable                                                           */
/* Info   : Creates a player_info object of specified name in who's inventory*/
/*          The Values of a player_info object will NOT effect the player.   */
/*          Returns the created object                                       */
/*****************************************************************************/
static PyObject* CFCreatePlayerInfo(PyObject* self, PyObject* args)
{
    char* txt;
    char txt2[16];
    object *myob;
    CFPython_Object *whereptr;
    CFParm* CFR;
    
    if (!PyArg_ParseTuple(args, "O!s", &CFPython_ObjectType, &whereptr,&txt))
        return NULL;
    
    strcpy(txt2,"player_info");
    
    GCFP.Value[0] = (void *)(txt2);
    CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
    
    /*myob = get_archetype("player_info"); */
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
/* Name   : CFGetPlayerInfo                                                  */
/* Python : CFPython.GetPlayerInfo(who, name)                                */
/* Status : Stable                                                           */
/* Info   : get first player_info with the specified name in who's inventory */
/*****************************************************************************/
static PyObject* CFGetPlayerInfo(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *name;
    object *walk;
    
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&name))
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
/* Name   : CFGetNextPlayerInfo                                              */
/* Python : CFPython.GetNextPlayerInfo(who, player_info)                     */
/* Status : Stable                                                           */
/* Info   : get next player_info in who's inventory with same name as        */
/*          player_info                                                      */
/*****************************************************************************/
static PyObject* CFGetNextPlayerInfo(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr, *myob;
    char name[128];
    object *walk;
    
    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whoptr, &CFPython_ObjectType, &myob))
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
/* Name   : CFCreateInvisibleInside                                          */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFCreateInvisibleInside(PyObject* self, PyObject* args)
{
    CFPython_Object *whereptr;
    char* txt;
    char txt2[6];
    object *myob;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"O!s",&CFPython_ObjectType, &whereptr,&txt))
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

    GCFP.Value[0] = (void *)(whereptr->obj);
    GCFP.Value[1] = (void *)(myob);
  /*esrv_send_item((object *)(gh_scm2long(where)), myob); */
    (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
    return wrap_object(myob);
}

/*****************************************************************************/
/* Name   : CFCreateObjectInside                                             */
/* Python : CFPython.CreateObjectInside(archname, where, identified, value)  */
/* Info   : Creates an object from archname and inserts intp where. If       */
/*          identified is 1, the object will be identified. If value is >= 0 */
/*          it will be used as the new object's value, otherwise the value   */
/*          will be taken from the arch.                                     */
/* Status : Stable                                                           */
/*****************************************************************************/
/* i must change this a bit - only REAL arch names - not object names */
/* Gecko: I would rather raise an exception than creating singularities... */

static PyObject* CFCreateObjectInside(PyObject* self, PyObject* args)
{
    object *myob;
    CFPython_Object *whereptr;
	long value, id;
    char *txt;
/*    char *tmpname;
    object *test;
    int i;*/
    CFParm* CFR;

	/* 0: name
	   1: object we want give <name> 
	   2: if 1, set FLAG_IDENTIFIED
	   3: if not -1, use it for myob->value
	   */

    if (!PyArg_ParseTuple(args,"sO!ll",&txt, &CFPython_ObjectType, &whereptr, &id, &value))
        return NULL;

    GCFP.Value[0] = (void *)(txt);
    CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
    myob = (object *)(CFR->Value[0]);
	free(CFR);

	if(!myob)
	{
		LOG(llevDebug,"BUG python_CFCreateObjectInside(): ob:>%s< = NULL!\n", query_name(myob));
	    GCFP.Value[0] = "singularity";
		CFR = (PlugHooks[HOOK_GETARCHETYPE])(&GCFP);
		myob = (object *)(CFR->Value[0]);
		free(CFR);
		if(!myob) /* now we are REALLY messed up */
		{
			LOG(llevDebug,"BUG python_CFCreateObjectInside(): FAILED TO CREATE: %s AND no singularity!\n", txt);
		    Py_INCREF(Py_None);
			return Py_None; /* emergency return */
		}
	}

	/* we created a singularity - that should not happes - tell it the logs */
    if (myob->name && !strncmp(myob->name, "singularity",11))
		LOG(llevDebug,"BUG python_CFCreateObjectInside(): FAILED TO CREATE: %s (>%s< = singularity!)\n", txt,query_name(myob));
		
	if(value != -1) /* -1 means, we use original value */
		myob->value = (sint32) value;
	if(id)
		SET_FLAG(myob,FLAG_IDENTIFIED);

    myob = insert_ob_in_ob_hook(myob, WHERE);
    if (WHERE->type == PLAYER)
    {
        GCFP.Value[0] = (void *)(WHERE);
        GCFP.Value[1] = (void *)(myob);
        (PlugHooks[HOOK_ESRVSENDITEM])(&GCFP);
/*        esrv_send_item((object *)(gh_scm2long(where)), myob); */
    }
    return wrap_object(myob);
}

/*****************************************************************************/
/* Name   : CFCheckMap                                                       */
/* Python :                                                                  */
/* Info   :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFCheckMap(PyObject* self, PyObject* args)
{
    char *what;
    char *mapstr;
    int x, y;
    object* foundob;

    /* Gecko: replaced coordinate tuple with separate x and y coordinates */
    if (!PyArg_ParseTuple(args,"ssii",&what,&mapstr,&x,&y))
        return NULL;
    
    foundob = present_arch(
        find_archetype(what),
        has_been_loaded(mapstr),
        x,y
    );
    return wrap_object(foundob);
}

/*****************************************************************************/
/* Name   : CFCheckArchInventory                                             */
/* Python : CFPython.CheckArchInventory(who, arch_name)                      */
/* Status : Stable                                                           */
/* Info   : Search for an arch_name in who's inventory                       */
/*****************************************************************************/
static PyObject* CFCheckArchInventory(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char* whatstr;
    object* tmp;
    
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&whatstr))
        return NULL;
    tmp = WHO->inv;

    while (tmp)
    {
        if (!strcmp(tmp->arch->name,whatstr))
            return wrap_object(tmp);
        tmp = tmp->below;
    }

    Py_INCREF(Py_None);
    return Py_None; /* we don't find a arch with this arch_name in the inventory */
}

/*****************************************************************************/
/* Name   : CFCheckInventory                                                 */
/* Python : CFPython.CheckInventory(who, name)                               */
/* Info   : returns the first found object with the specified name if found  */
/*          in who's inventory, or None if it wasn't found.                  */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFCheckInventory(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
	int i;
    char* whatstr;
    object* tmp;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&whatstr))
        return NULL;

    tmp = WHO->inv;
    
	i = (int)strlen(whatstr);
	while (tmp)
	{
		if (!strncmp(query_name(tmp),whatstr,i))
            return wrap_object(tmp);
		if (tmp->name && !strncmp(tmp->name,whatstr,i))
            return wrap_object(tmp);
		tmp = tmp->below;
	}

    Py_INCREF(Py_None);
    return Py_None; /* we don't find a arch with this arch_name in the inventory */
}

/*****************************************************************************/
/* Name   : CFGetName                                                        */
/* Python : string CFPython.GetName(object)                                  */
/* Info   : Get the name of object                                           */
/* Status : stable                                                           */
/*****************************************************************************/
static PyObject* CFGetName(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("s",WHO->name);
}

/*****************************************************************************/
/* Name   : CFSetName                                                        */
/* Python : CFPython.SetName(object, name)                                   */
/* Info   : FIXME: classic inventory problem (needs to update info)          */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetName(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *txt;
    
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&txt))
        return NULL;
    if (WHO->name != NULL)
        FREE_STRING_HOOK(WHO->name);
    if(txt && strcmp(txt,""))
        WHO->name = add_string_hook(txt);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetTitle                                                       */
/* Python : CFPython.GetTitle(object)                                        */
/* Info   : If object has no title, this function will return None           */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFGetTitle(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("s",WHO->title);
}


/*****************************************************************************/
/* Name   : CFSetTitle                                                       */
/* Python : CFPython.SetTitle(object, name)                                  */
/* Info   : Sets the title of object to name.                                */
/*          FIXME: classic inventory problem (needs to update info)          */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetTitle(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *txt;
    
    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&txt))
        return NULL;
    if (WHO->title != NULL)
        FREE_STRING_HOOK(WHO->title);
    if(txt && strcmp(txt,""))
        WHO->title = add_string_hook(txt);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetSlaying                                                     */
/* Python : string CFPython.GetSlaying(object)                               */
/* Info   : gets the slaying string of object                                */
/*          If object has no slaying, this function will return None         */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetSlaying(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("s",WHO->slaying);
}

/*****************************************************************************/
/* Name   : CFSetSlaying                                                     */
/* Python : CFPython.SetSlaying(object, name)                                */
/* Info   : sets the slaying string of object to name                        */
/*          FIXME: classic inventory problem (needs to update info)          */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetSlaying(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *txt;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&txt))
        return NULL;

    if (WHO->slaying != NULL)
        FREE_STRING_HOOK(WHO->slaying);
    if(txt && strcmp(txt,""))
        WHO->slaying = add_string_hook(txt);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetSaveBed                                                     */
/* Python : CFPython.SetSaveBed(player, mapname, x, y)                       */
/* Info   : Sets the current savebed position for player to the specified    */
/*          coordinates on the map mapname.                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFSetSaveBed(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *txt;
    int x,y;
    
    if (!PyArg_ParseTuple(args,"O!sii", &CFPython_ObjectType, &whoptr,&txt,&x, &y))
        return NULL;
	
    if(WHO->type == PLAYER)
	{	
		strcpy(WHO->contr->savebed_map, txt);
		WHO->contr->bed_x = x;
		WHO->contr->bed_y = y;
    }
    Py_INCREF(Py_None);
    return Py_None;
}


/*****************************************************************************/
/* Name   : CFCreateObject                                                   */
/* Python :                                                                  */
/* Info   :                                                                  */
/* Status : Unfinished                                                       */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFCreateObject(PyObject* self, PyObject* args)
{
    char *txt;
    int x,y;
    CFPython_Map *map = (CFPython_Map *)wrap_map(StackWho[StackPosition]->map);
    
    /* Gecko: replaced coordinate tuple with separate x and y coordinates */
    if (!PyArg_ParseTuple(args,"sii|O!",&txt, &x,&y, &CFPython_MapType, &map))
        return NULL;

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFRemoveObject                                                   */
/* Python : CFPython.RemoveObject(object)                                    */
/* Info   : Permanently removes object from the game.                        */
/* Status : Tested                                                           */
/*****************************************************************************/
/* Gecko  : This function is DANGEROUS. Added limitations on what can be     */
/*          removed to avoid some of the problems                            */
/*****************************************************************************/
/* hm, this should be named delete or free object... */
static PyObject* CFRemoveObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    object* myob;
    object* obenv;

    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
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
/* Name   : CFIsAlive                                                        */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsAlive(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_ALIVE));
}

/*****************************************************************************/
/* Name   : CFIsWiz                                                          */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsWiz(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_WIZ));
}

/*****************************************************************************/
/* Name   : CFWasWiz                                                         */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFWasWiz(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_WAS_WIZ));
}

/*****************************************************************************/
/* Name   : CFIsApplied                                                      */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsApplied(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_APPLIED));
}

/*****************************************************************************/
/* Name   : CFIsUnpaid                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsUnpaid(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_UNPAID));
}

/*****************************************************************************/
/* Name   : CFIsFlying                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsFlying(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_FLYING));
}

/*****************************************************************************/
/* Name   : CFIsMonster                                                      */
/* Info   : We check for monster flag, not the type..                        */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsMonster(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_MONSTER));
}

/*****************************************************************************/
/* Name   : CFIsFriendly                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsFriendly(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_FRIENDLY));
}

/*****************************************************************************/
/* Name   : CFIsGenerator                                                    */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsGenerator(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_GENERATOR));
}

/*****************************************************************************/
/* Name   : CFIsThrown                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsThrown(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_IS_THROWN));
}

/*****************************************************************************/
/* Name   : CFCanSeeInvisible                                                */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFCanSeeInvisible(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_SEE_INVISIBLE));
}

/*****************************************************************************/
/* Name   : CFCanRoll                                                        */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFCanRoll(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_CAN_ROLL));
}

/*****************************************************************************/
/* Name   : CFIsTurnable                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsTurnable(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_IS_TURNABLE));
}

/*****************************************************************************/
/* Name   : CFIsUsedUp                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsUsedUp(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_IS_USED_UP));
}

/*****************************************************************************/
/* Name   : CFIsIdentified                                                   */
/* Python : CFPython.IsIdentified(object)                                    */
/* Info   : Returns 1 if object is identified, or 0 otherwise.               */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsIdentified(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i", QUERY_FLAG(WHO,FLAG_IDENTIFIED));
}

/*****************************************************************************/
/* Name   : CFIsSplitting                                                    */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/

static PyObject* CFIsSplitting(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_SPLITTING));
}

/*****************************************************************************/
/* Name   : CFHitBack                                                        */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFHitBack(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_HITBACK));
}

/*****************************************************************************/
/* Name   : CFBlocksView                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFBlocksView(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_BLOCKSVIEW));
}

/*****************************************************************************/
/* Name   : CFIsUndead                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsUndead(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_UNDEAD));
}

/*****************************************************************************/
/* Name   : CFIsScared                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsScared(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_SCARED));
}

/*****************************************************************************/
/* Name   : CFIsUnaggressive                                                 */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsUnaggressive(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_UNAGGRESSIVE));
}

/*****************************************************************************/
/* Name   : CFReflectMissiles                                                */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFReflectMissiles(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_REFL_MISSILE));
}

/*****************************************************************************/
/* Name   : CFReflectSpells                                                  */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFReflectSpells(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_REFL_SPELL));
}

/*****************************************************************************/
/* Name   : CFIsRunningAway                                                  */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsRunningAway(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_RUN_AWAY));
}

/*****************************************************************************/
/* Name   : CFCanPassThru                                                    */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanPassThru(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_CAN_PASS_THRU));
}

/*****************************************************************************/
/* Name   : CFCanPickUp                                                      */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanPickUp(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_PICK_UP));
}

/*****************************************************************************/
/* Name   : CFIsUnique                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsUnique(PyObject* self, PyObject* args)
{
    PyObject *ob;
    
    if (!PyArg_ParseTuple(args,"O", &ob))
        return NULL;
    
    if(PyObject_TypeCheck(ob, &CFPython_ObjectType)) {
        return Py_BuildValue("i",QUERY_FLAG(((CFPython_Object *)ob)->obj,FLAG_UNIQUE));
    } else if(PyObject_TypeCheck(ob, &CFPython_MapType))  {
        return Py_BuildValue("i",MAP_UNIQUE(((CFPython_Map *)ob)->map));
    } else
        RAISE("Parameter 1 must be a map or an object");
}

/*****************************************************************************/
/* Name   : CFCanCastSpell                                                   */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanCastSpell(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_CAST_SPELL));
}

/*****************************************************************************/
/* Name   : CFCanUseScroll                                                   */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseScroll(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_SCROLL));
}

/*****************************************************************************/
/* Name   : CFCanUseWand                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseWand(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_RANGE));
}

/*****************************************************************************/
/* Name   : CFCanUseBow                                                      */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseBow(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_BOW));
}

/*****************************************************************************/
/* Name   : CFCanUseArmour                                                   */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseArmour(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_ARMOUR));
}

/*****************************************************************************/
/* Name   : CFCanUseWeapon                                                   */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseWeapon(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_WEAPON));
}

/*****************************************************************************/
/* Name   : CFCanUseRing                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseRing(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_RING));
}

/*****************************************************************************/
/* Name   : CFHasXRays                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFHasXRays(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_XRAYS));
}

/*****************************************************************************/
/* Name   : CFIsFloor                                                        */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsFloor(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_IS_FLOOR));
}

/*****************************************************************************/
/* Name   : CFIsLifeSaver                                                    */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsLifeSaver(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_LIFESAVE));
}

/*****************************************************************************/
/* Name   : CFIsSleeping                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsSleeping(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_SLEEP));
}

/*****************************************************************************/
/* Name   : CFStandStill                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFStandStill(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_STAND_STILL));
}

/*****************************************************************************/
/* Name   : CFOnlyAttack                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFOnlyAttack(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_ONLY_ATTACK));
}

/*****************************************************************************/
/* Name   : CFIsConfused                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsConfused(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_CONFUSED));
}

/*****************************************************************************/
/* Name   : CFHasStealth                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFHasStealth(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_STEALTH));
}

/*****************************************************************************/
/* Name   : CFIsCursed                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsCursed(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_CURSED));
}

/*****************************************************************************/
/* Name   : CFIsDamned                                                       */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsDamned(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_DAMNED));
}

/*****************************************************************************/
/* Name   : CFIsKnownMagical                                                 */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsKnownMagical(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_KNOWN_MAGICAL));
}

/*****************************************************************************/
/* Name   : CFIsKnownCursed                                                  */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsKnownCursed(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_KNOWN_CURSED));
}

/*****************************************************************************/
/* Name   : CFCanUseSkill                                                    */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseSkill(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_CAN_USE_SKILL));
}

/*****************************************************************************/
/* Name   : CFHasBeenApplied                                                 */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFHasBeenApplied(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_BEEN_APPLIED));
}

/*****************************************************************************/
/* Name   : CFCanUseRod                                                      */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseRod(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_RANGE));
}

/*****************************************************************************/
/* Name   : CFCanUseHorn                                                     */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanUseHorn(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_USE_RANGE));
}

/*****************************************************************************/
/* Name   : CFMakeInvisible                                                  */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFMakeInvisible(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_SEE_INVISIBLE));
}

/*****************************************************************************/
/* Name   : CFIsBlind                                                        */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFIsBlind(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_BLIND));
}

/*****************************************************************************/
/* Name   : CFCanSeeInDark                                                   */
/* Python :                                                                  */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFCanSeeInDark(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",QUERY_FLAG(WHO,FLAG_SEE_IN_DARK));
}

/*****************************************************************************/
/* Name   : CFGetAC                                                          */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
static PyObject* CFGetAC(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.ac);
}

/*****************************************************************************/
/* Name   : CFGetCha                                                         */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
static PyObject* CFGetCha(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.Cha);
}

/*****************************************************************************/
/* Name   : CFGetCon                                                         */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
static PyObject* CFGetCon(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.Con);
}

/*****************************************************************************/
/* Name   : CFGetDex                                                         */
/* Python :                                                                  */
/* Status : Untested                                                                */
/*****************************************************************************/
static PyObject* CFGetDex(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.Dex);
}

/*****************************************************************************/
/* Name   : CFGetHP                                                          */
/* Python : CFPython.GetHP(object)                                           */
/* Info   : Gets the HPs of object.                                          */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetHP(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.hp);

}

/*****************************************************************************/
/* Name   : CFGetInt                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetInt(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.Int);
}

/*****************************************************************************/
/* Name   : CFGetPow                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetPow(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.Pow);
}

/*****************************************************************************/
/* Name   : CFGetSP                                                          */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetSP(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.sp);
}

/*****************************************************************************/
/* Name   : CFGetStr                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetStr(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.Str);
}

/*****************************************************************************/
/* Name   : CFGetWis                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetWis(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.Wis);
}

/*****************************************************************************/
/* Name   : CFGetMaxHP                                                       */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetMaxHP(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.maxhp);
}

/*****************************************************************************/
/* Name   : CFGetMaxSP                                                       */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetMaxSP(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->stats.maxsp);
}

/*****************************************************************************/
/* Name   : CFGetXPos                                                        */
/* Python : CFPython.GetXPosition(object)                                    */
/* Info   : Gets the X position of object on its map.                        */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetXPos(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->x);
}

/*****************************************************************************/
/* Name   : CFGetYPos                                                        */
/* Python : CFPython.GetYPosition(object)                                    */
/* Info   : Gets the Y position of object on its map.                        */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFGetYPos(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->y);
}

/*****************************************************************************/
/* Name   : CFSetPosition                                                    */
/* Python : CFPython.SetPosition(object, x, y)                               */
/* Info   : Cannot be used to move objects out of containers. (Use Drop() or */
/*          Teleport() for that)                                             */
/* Status : Tested                                                           */
/*****************************************************************************/

/* FIXME: if the object moved was triggered by SAY event and it is moved to a tile
 * within the listening radius, it will be triggered again, and again... */

static PyObject* CFSetPosition(PyObject* self, PyObject* args)
{
    int x, y, k;
    CFPython_Object *whoptr;
    CFParm* CFR;
    k = 0;

    if (!PyArg_ParseTuple(args,"O!ii", &CFPython_ObjectType, &whoptr,&x,&y))
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
/* Name   : CFSetAC                                                          */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetAC(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value>120 || value < -120)
        RAISE("New AC must be between -120 and 120");

    WHO->stats.ac = value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetCha                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetCha(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-30 || value > 30)
        RAISE("New charisma must be between -30 and 30");

    WHO->stats.Cha = value;
    if (WHO->type == PLAYER)
    {
        WHO->contr->orig_stats.Cha = value;
		fix_player_hook(WHO);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetCon                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetCon(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-30 || value > 30)
        RAISE("New constitution must be between -30 and 30");

    WHO->stats.Con = value;
    if (WHO->type == PLAYER)
    {
        WHO->contr->orig_stats.Con = value;
	    fix_player_hook(WHO);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetDex                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetDex(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-30 || value > 30)
        RAISE("New dexterity must be between -30 and 30");

    WHO->stats.Dex = value;
    if (WHO->type == PLAYER)
    {
        WHO->contr->orig_stats.Dex = value;
	    fix_player_hook(WHO);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetHP                                                          */
/* Python : CFPython.SetHP(object, hp)                                       */
/* Info   : Sets the current hitpoints of object to hp                       */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFSetHP(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<0)
        RAISE("New HP must be >= 0");

    WHO->stats.hp = value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetInt                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetInt(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-30 || value > 30)
        RAISE("New intelligence must be between -30 and 30");

    WHO->stats.Int = value;
    if (WHO->type == PLAYER)
    {
        WHO->contr->orig_stats.Int = value;
		fix_player_hook(WHO);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetMaxHP                                                       */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetMaxHP(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<0 || value > 16000)
        RAISE("New max HP must be between 0 and 16000");

    WHO->stats.maxhp = value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetMaxSP                                                       */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetMaxSP(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<0 || value > 16000)
        RAISE("New max SP must be between 0 and 16000");

    WHO->stats.maxsp = value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetPow                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetPow(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-30 || value > 30)
        RAISE("New power must be between -30 and 30");

    WHO->stats.Pow = value;
    if (WHO->type == PLAYER)
    {
        WHO->contr->orig_stats.Pow = value;
	    fix_player_hook(WHO);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetSP                                                          */
/* Python : CFPython.SetHP(object, sp)                                       */
/* Info   : Sets the current spellpoints of object to sp                     */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFSetSP(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<0 || value > 16000)
        RAISE("New SP must be between 0 and 16000");

    WHO->stats.sp = value;
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetStr                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetStr(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-30 || value > 30)
        RAISE("New strength must be between -30 and 30");

    WHO->stats.Str = value;
    if (WHO->type == PLAYER)
    {
        WHO->contr->orig_stats.Str = value;
	    fix_player_hook(WHO);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFSetWis                                                         */
/* Python :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFSetWis(PyObject* self, PyObject* args)
{
    int value;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&value))
        return NULL;

    if (value<-30 || value > 30)
        RAISE("New wisdom must be between -30 and 30");

    WHO->stats.Wis = value;
    if (WHO->type == PLAYER)
    {
        WHO->contr->orig_stats.Wis = value;
	    fix_player_hook(WHO);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFIdentifyObject                                                 */
/* Python : CFPython.IdentifyObject(caster, target, object, mode)            */
/* Info   : caster identifies object in target's inventory.                  */
/*          mode: 0 = normal, 1 = all, 2 = marked                            */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFIdentifyObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr, *target;
    PyObject *ob;
    object *marked = NULL;
    long mode;

    /* Gecko: object can be None if mode == 0 or mode == 1*/
    if (!PyArg_ParseTuple(args,"O!O!Ol",
                &CFPython_ObjectType, &whoptr, 
                &CFPython_ObjectType, &target, 
                &ob, &mode)) 
        return NULL;
    
    if(mode == 2) {
        if(! PyObject_TypeCheck(ob, &CFPython_ObjectType)) 
            RAISE("Parameter 3 must be a CFPython.Object for mode 2");
        marked = ((CFPython_Object *)ob)->obj;
    } else if (mode == 0 || mode == 1) {
        if(ob != Py_None) 
            RAISE("Parameter 3 must be None for modes 0 and 1");
    } else
        RAISE("Mode must be 0, 1 or 2");

    GCFP.Value[0] = (void *)WHO;
    GCFP.Value[1] = (void *)target->obj;
    GCFP.Value[2] = (void *)marked; /* is used when we use mode == 2 */
    GCFP.Value[3] = (void *)&mode;
    (PlugHooks[HOOK_IDENTIFYOBJECT])(&GCFP);   

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFMessage                                                        */
/* Python : CFPython.Message(who, message, color)                            */
/* Info   : Writes a message to a map (given by who in this map).            */
/*          Swapped who/message pos. MT-26-10-2002                           */
/*          color is an optional parameter (default = NDI_BLUE|NDI_UNIQUE)   */
/* Status : Tested                                                           */
/*****************************************************************************/

static PyObject* CFMessage(PyObject* self, PyObject* args)
{
    int   color = NDI_BLUE|NDI_UNIQUE;
    char *message;
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args,"O!s|i", &CFPython_ObjectType, &whoptr,&message,&color))
        return NULL;

    GCFP.Value[0] = (void *)(&color);
    GCFP.Value[1] = (void *)(WHO->map);
    GCFP.Value[2] = (void *)(message);

    (PlugHooks[HOOK_NEWINFOMAP])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   :	CFWrite                                                          */
/* Python : CFPython.Write(who, message , color)                             */
/* Info   : Writes a message to a specific player. Color is optional.        */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFWrite(PyObject* self, PyObject* args)
{
    int   zero   = 0;
    char* message;
    CFPython_Object *whoptr = NULL;
    int   color  = NDI_UNIQUE | NDI_ORANGE;

    if (!PyArg_ParseTuple(args,"O!s|i", &CFPython_ObjectType, &whoptr,&message,&color))
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
/* Name   : CFIsOfType                                                       */
/* Python : CFPython.IsOfType(object, type)                                  */
/* Info   : returns 1 if object is of the specified type, or 0 otherwise.    */
/*          (e.g. type = 80 for monster/NPC, or 1 for players)               */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFIsOfType(PyObject* self, PyObject* args)
{
    int type;
    CFPython_Object *whoptr;
    int value;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&type))
        return NULL;
    if (WHO->type==type)
        value = 1;
    else
        value = 0;
    return Py_BuildValue("i",value);
}

/*****************************************************************************/
/* Name   : CFGetType                                                        */
/* Python : CFPython.GetType(object)                                         */
/* Info   : returns the type of the specified object                         */
/*          (e.g. type = 80 for monster/NPC, or 1 for players)               */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFGetType(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("i",WHO->type);
}

/* Those replace the old get-script... and set-script... system */
/*****************************************************************************/
/* Name   : CFGetEventHandler                                                */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
static PyObject* CFGetEventHandler(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int eventnr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&eventnr))
        return NULL;
    return Py_BuildValue("s","" /*WHO->event_hook[eventnr]*/);
}

/*****************************************************************************/
/* Name   : CFSetEventHandler                                                */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetEventHandler(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int eventnr;
    char* scriptname;

    if (!PyArg_ParseTuple(args,"O!is", &CFPython_ObjectType, &whoptr, &eventnr, &scriptname))
        return NULL;

    /*WHO->event_hook[eventnr] = add_string_hook(scriptname);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetEventPlugin                                                 */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFGetEventPlugin(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int eventnr;

    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr, &eventnr))
        return NULL;
    return Py_BuildValue("s", ""/*WHO->event_plugin[eventnr]*/);
}

/*****************************************************************************/
/* Name   : CFSetEventPlugin                                                 */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetEventPlugin(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int eventnr;
    char* scriptname;

    if (!PyArg_ParseTuple(args,"O!is", &CFPython_ObjectType, &whoptr,&eventnr,&scriptname))
        return NULL;

    /*WHO->event_plugin[eventnr] = add_string_hook(scriptname);*/
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFGetEventOptions                                                */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFGetEventOptions(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int eventnr;
    /*static char estr[4];*/
    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&eventnr))
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
/* Name   : CFSetEventOptions                                                */
/* Python :                                                                  */
/* Status : Unfinished / Deprecated                                          */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFSetEventOptions(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int eventnr;
    char* scriptname;

    if (!PyArg_ParseTuple(args,"O!is", &CFPython_ObjectType, &whoptr,&eventnr,&scriptname))
        return NULL;

    /*    WHO->event_options[eventnr] = add_string_hook(scriptname);*/

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFLoadObject                                                     */
/* Python : LoadObject(string)                                               */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFLoadObject(PyObject* self, PyObject* args)
{
    object *whoptr;
    char *dumpob;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args, "s",&dumpob))
        return NULL;

    /* First step: We create the object */
    GCFP.Value[0] = (void *)(dumpob);
    CFR = (PlugHooks[HOOK_LOADOBJECT])(&GCFP);
    whoptr = (object *)(CFR->Value[0]);
    free(CFR);

    return wrap_object(whoptr);
}

/*****************************************************************************/
/* Name   : CFSaveObject                                                     */
/* Python : CFPython.SaveObject(what)                                        */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFSaveObject(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    static char *result;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args, "O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_DUMPOBJECT])(&GCFP);
    result = (char *)(CFR->Value[0]);
    free(CFR);

    return Py_BuildValue("s",result);
}

/*****************************************************************************/
/* Name   : CFGetIP                                                          */
/* Python : CFPython.GetIP(object)                                           */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFGetIP(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    static char *result;

    if (!PyArg_ParseTuple(args, "O!", &CFPython_ObjectType, &whoptr))
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
/* Name   : CFGetInventory                                                   */
/* Python : CFPython.GetInventory(who)                                       */
/* Info   : get the first object in object's inventory                       */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFGetInventory(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;

    if (!PyArg_ParseTuple(args, "O!", &CFPython_ObjectType, &whoptr))
        return NULL;

    return wrap_object(WHO->inv);
}

/*****************************************************************************/
/* Name   : CFGetArchName                                                    */
/* Python : CFPython.GetArchName (object)                                    */
/* Info   : FIXME: seems to return object name instead of arch name ?        */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFGetArchName(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    return Py_BuildValue("s",WHO->name);
}

/*****************************************************************************/
/* Name   : CFCostFlagFTrue                                                  */
/* Python : CFPython.CostFlagFTrue ()                                        */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFCostFlagFTrue(PyObject* self, PyObject* args)
{
    int flag=F_TRUE;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",flag);
}

/*****************************************************************************/
/* Name   : CFCostFlagFBuy                                                   */
/* Python : CFPython.CostFlagFBuy ()                                         */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFCostFlagFBuy(PyObject* self, PyObject* args)
{
    int flag=F_BUY;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",flag);
}

/*****************************************************************************/
/* Name   : CFCostFlagFSell                                                  */
/* Python : CFPython.CostFlagFSell ()                                        */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* CFCostFlagFSell(PyObject* self, PyObject* args)
{
    int flag=F_SELL;
    if (!PyArg_ParseTuple(args,"",NULL))
        return NULL;
    return Py_BuildValue("i",flag);
}

/*****************************************************************************/
/* Name   : CFGetObjectCost                                                  */
/* Python : CFPython.GetObjectCost (buyer,object,type)                       */
/* Status : Stable                                                           */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFGetObjectCost(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;
    int flag;
    int cost;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"O!O!i", &CFPython_ObjectType, &whoptr,
                &CFPython_ObjectType, &whatptr,&flag))
        return NULL;
    if ((!WHAT) || (!WHO)) return Py_BuildValue("i",0);
    GCFP.Value[0] = (void *)(WHAT);
    GCFP.Value[1] = (void *)(WHO);
    GCFP.Value[2] = (void *)(&flag);
    CFR = (PlugHooks[HOOK_QUERYCOST])(&GCFP);
    cost=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",cost);
}

/*****************************************************************************/
/* Name   : CFGetObjectMoney                                                 */
/* Python : CFPython.GetObjectMoney (buyer)                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFGetObjectMoney(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int amount;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"O!", &CFPython_ObjectType, &whoptr))
        return NULL;
    if (!WHO) return Py_BuildValue("i",0);
    GCFP.Value[0] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_QUERYMONEY])(&GCFP);
    amount=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",amount);
}

/*****************************************************************************/
/* Name   : CFPayForItem                                                     */
/* Python : CFPython.PayForItem (buyer,object)                               */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */

static PyObject* CFPayForItem(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    CFPython_Object *whatptr;
    int val;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"O!O!", &CFPython_ObjectType, &whoptr,
                &CFPython_ObjectType, &whatptr))
        return NULL;
    if ((!WHAT) || (!WHO)) return Py_BuildValue("i",0);
    GCFP.Value[0] = (void *)(WHAT);
    GCFP.Value[1] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_PAYFORITEM])(&GCFP);
    val=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",val);
}

/*****************************************************************************/
/* Name   : CFPayAmount                                                      */
/* Python : CFPython.PayAmount (buyer,value)                                 */
/* Info   : If buyer has enough money, value copper will be deducted from    */
/*          buyer, and 1 will be returned. Otherwise returns 0               */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* CFPayAmount(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    int to_pay;
    int val;
    CFParm* CFR;
    if (!PyArg_ParseTuple(args,"O!i", &CFPython_ObjectType, &whoptr,&to_pay))
        return NULL;
    if (!WHO) return Py_BuildValue("i",0);
    GCFP.Value[0] = (void *)(&to_pay);
    GCFP.Value[1] = (void *)(WHO);
    CFR = (PlugHooks[HOOK_PAYFORAMOUNT])(&GCFP);
    val=*(int*)(CFR->Value[0]);
    free (CFR);
    return Py_BuildValue("i",val);
}

/*****************************************************************************/
/* Name   : CFRegisterCommand                                                */
/* Python : CFPython.RegisterCommand(cmdname,scriptname,speed)               */
/* Status : Untested                                                         */
/*****************************************************************************/
/* pretty untested... */
static PyObject* CFRegisterCommand(PyObject* self, PyObject* args)
{
    char *cmdname;
    char *scriptname;
    double cmdspeed;
    int i;

    if (!PyArg_ParseTuple(args, "ssd",&cmdname,&scriptname,&cmdspeed))
        return NULL;

    for (i=0;i<NR_CUSTOM_CMD;i++)
    {
        if (CustomCommand[i].name)
        {
            if (!strcmp(CustomCommand[i].name,cmdname))
            {
                LOG(llevDebug, "PYTHON - This command is already registered !\n");
                RAISE("This command is already registered");
            }
        }
    }
    for (i=0;i<NR_CUSTOM_CMD;i++)
    {
        if (CustomCommand[i].name == NULL)
        {
            CustomCommand[i].name = (char *)(malloc(sizeof(char)*strlen(cmdname)));
            CustomCommand[i].script = (char *)(malloc(sizeof(char)*strlen(scriptname)));
            strcpy(CustomCommand[i].name,cmdname);
            strcpy(CustomCommand[i].script,scriptname);
            CustomCommand[i].speed = cmdspeed;
            i = NR_CUSTOM_CMD;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}


/*****************************************************************************/
/* Name   : CFSendCustomCommand                                              */
/* Python : CFPython.SendCustomCommand(who, customcommand)                   */
/* Status : Untested                                                         */
/*****************************************************************************/
/* GeckoStatus: untested */
static PyObject* CFSendCustomCommand(PyObject* self, PyObject* args)
{
    CFPython_Object *whoptr;
    char *customcmd;

    if (!PyArg_ParseTuple(args,"O!s", &CFPython_ObjectType, &whoptr,&customcmd))
        return NULL;
    GCFP.Value[0] = (void *)(WHO);
    GCFP.Value[1] = (void *)(customcmd);
    (PlugHooks[HOOK_SENDCUSTOMCOMMAND])(&GCFP);
    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : CFPlayMapSound                                                   */
/* Python : CFPython.PlayMapSound(map, x, y, soundnumber, soundtype)         */
/* Status : Tested                                                           */
/*****************************************************************************/
static PyObject* CFPlayMapSound(PyObject* self, PyObject* args)
{
    CFPython_Map *whereptr;
    int x, y, soundnumber, soundtype;

    if (!PyArg_ParseTuple(args,"O!iiii", &CFPython_MapType, &whereptr, &x, &y, &soundnumber, &soundtype))
        return NULL;
    
    GCFP.Value[0] = (void *)(whereptr->map);
    GCFP.Value[1] = (void *)(&x);
    GCFP.Value[2] = (void *)(&y);
    GCFP.Value[3] = (void *)(&soundnumber);
    GCFP.Value[4] = (void *)(&soundtype);
    (PlugHooks[HOOK_PLAYSOUNDMAP])(&GCFP);
    Py_INCREF(Py_None);
    return Py_None;
}


/* FUNCTIONEND -- End of the Python plugin functions. */

/*****************************************************************************/
/* The Plugin Management Part.                                               */
/* Most of the functions below should exist in any CF plugin. They are used  */
/* to glue the plugin to the server core. All functions follow the same      */
/* declaration scheme (taking a CFParm* arg, returning a CFParm) to make the */
/* plugin interface as general as possible. And since the loading of modules */
/* isn't time-critical, it is never a problem. It could also make using      */
/* programming languages other than C to write plugins a little easier, but  */
/* this has yet to be proven.                                                */
/*****************************************************************************/

/*****************************************************************************/
/* Called whenever a Hook Function needs to be connected to the plugin.      */
/*****************************************************************************/
MODULEAPI CFParm* registerHook(CFParm* PParm)
{
    int Pos;
    f_plugin Hook;
    Pos = *(int*)(PParm->Value[0]);
    Hook=(f_plugin)(PParm->Value[1]);
    PlugHooks[Pos]=Hook;
    return NULL;
}

/*****************************************************************************/
/* Called whenever an event is triggered, both Local and Global ones.        */
/*****************************************************************************/
/* Two types of events exist in CF:                                          */
/* - Local events: They are triggered by a single action on a single object. */
/*                 Nearly any object can trigger a local event               */
/*                 To warn the plugin of a local event, the map-maker needs  */
/*                 to use the event... tags in the objects of their maps.    */
/* - Global events: Those are triggered by actions concerning CF as a whole. */
/*                 Those events may or may not be triggered by a particular  */
/*                 object; they can't be specified by event... tags in maps. */
/*                 The plugin should register itself for all global events it*/
/*                 wants to be aware of.                                     */
/* Why those two types ? Local Events are made to manage interactions between*/
/* objects, for example to create complex scenarios. Global Events are made  */
/* to allow logging facilities and server management. Global Events tends to */
/* require more CPU time than Local Events, and are sometimes difficult to   */
/* bind to any specific object.                                              */
/*****************************************************************************/
MODULEAPI CFParm* triggerEvent(CFParm* PParm)
{
    /*CFParm *CFP; */
    int eventcode;
    static int result;

    
    eventcode = *(int *)(PParm->Value[0]);
    LOG(llevDebug, "PYTHON - triggerEvent:: eventcode %d\n",eventcode);
    switch(eventcode)
    {
        case EVENT_NONE:
            LOG(llevDebug, "PYTHON - Warning - EVENT_NONE requested\n");
            break;
        case EVENT_ATTACK:
        case EVENT_APPLY:
        case EVENT_DEATH:
        case EVENT_DROP:
        case EVENT_PICKUP:
        case EVENT_SAY:
        case EVENT_STOP:
        case EVENT_TELL:
        case EVENT_TIME:
        case EVENT_THROW:
        case EVENT_TRIGGER:
        case EVENT_CLOSE:
            result = HandleEvent(PParm);
            break;
        case EVENT_BORN:
        case EVENT_CRASH:
        case EVENT_LOGIN:
        case EVENT_LOGOUT:
        case EVENT_REMOVE:
        case EVENT_SHOUT:
        case EVENT_MAPENTER:
        case EVENT_MAPLEAVE:
        case EVENT_CLOCK:
        case EVENT_MAPRESET:
            result = HandleGlobalEvent(PParm);
            break;
    }
    GCFP.Value[0] = (void *)(&result);
    return &GCFP;
}

/*****************************************************************************/
/* Handles standard global events.                                            */
/*****************************************************************************/
MODULEAPI int HandleGlobalEvent(CFParm* PParm)
{
    FILE* Scriptfile;

    if (StackPosition == MAX_RECURSIVE_CALL)
    {
        LOG(llevDebug, "Can't execute script - No space left of stack\n");
        return 0;
    }

    StackPosition++;

    switch(*(int *)(PParm->Value[0]))
    {
        case EVENT_CRASH:
            LOG(llevDebug, "Unimplemented for now\n");
            break;
        case EVENT_BORN:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event BORN generated by %s\n",query_name(StackActivator[StackPosition])); */
            Scriptfile = fopen(create_pathname("python/python_born.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile,create_pathname("python/python_born.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_LOGIN:
            StackActivator[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackWho[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event LOGIN generated by %s\n",query_name(StackActivator[StackPosition])); */
            /*LOG(llevDebug, "IP is %s\n", (char *)(PParm->Value[2])); */
            Scriptfile = fopen(create_pathname("python/python_login.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(
                    Scriptfile,create_pathname("python/python_login.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_LOGOUT:
            StackActivator[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackWho[StackPosition] = ((player *)(PParm->Value[1]))->ob;
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event LOGOUT generated by %s\n",query_name(StackActivator[StackPosition])); */
            Scriptfile = fopen(create_pathname("python/python_logout.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile,create_pathname("python/python_logout.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_REMOVE:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event REMOVE generated by %s\n",query_name(StackActivator[StackPosition])); */

            Scriptfile = fopen(create_pathname("python/python_remove.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile,create_pathname("python/python_remove.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_SHOUT:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            StackText[StackPosition] = (char *)(PParm->Value[2]);
            /*LOG(llevDebug, "Event SHOUT generated by %s\n",query_name(StackActivator[StackPosition])); */

            /*LOG(llevDebug, "Message shout is %s\n",StackText[StackPosition]); */
            Scriptfile = fopen(create_pathname("python/python_shout.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile, create_pathname("python/python_shout.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_MAPENTER:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event MAPENTER generated by %s\n",query_name(StackActivator[StackPosition])); */

            Scriptfile = fopen(create_pathname("python/python_mapenter.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile, create_pathname("python/python_mapenter.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_MAPLEAVE:
            StackActivator[StackPosition] = (object *)(PParm->Value[1]);
            /*LOG(llevDebug, "Event MAPLEAVE generated by %s\n",query_name(StackActivator[StackPosition])); */

            Scriptfile = fopen(create_pathname("python/python_mapleave.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile, create_pathname("python/python_mapleave.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_CLOCK:
            /* LOG(llevDebug, "Event CLOCK generated\n"); */
            Scriptfile = fopen(create_pathname("/python/python_clock.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile, create_pathname("python/python_clock.py"));
                fclose(Scriptfile);
            }
            break;
        case EVENT_MAPRESET:
            StackText[StackPosition] = (char *)(PParm->Value[1]);/* Map name/path */
            LOG(llevDebug, "Event MAPRESET generated by %s\n", StackText[StackPosition]);

            Scriptfile = fopen(create_pathname("python/python_mapreset.py"),"r");
            if (Scriptfile != NULL)
            {
                PyRun_SimpleFile(Scriptfile, create_pathname("python/python_mapreset.py"));
                fclose(Scriptfile);
            }
            break;
    }
    StackPosition--;
    return 0;
}

/*****************************************************************************/
/* Handles standard local events.                                            */
/*****************************************************************************/
MODULEAPI int HandleEvent(CFParm* PParm)
{
    FILE* Scriptfile;

#ifdef PYTHON_DEBUG
    LOG(llevDebug, "PYTHON - HandleEvent:: start script file >%s<\n",(char *)(PParm->Value[9]));
    LOG(llevDebug, "PYTHON - call data:: o1:>%s< o2:>%s< o3:>%s< text:>%s< i1:%d i2:%d i3:%d i4:%d\n",
		query_name((object *)(PParm->Value[1])),
		query_name((object *)(PParm->Value[2])),
		query_name((object *)(PParm->Value[3])),
		(char *)(PParm->Value[4])!= NULL?(char *)(PParm->Value[4]):"<null>",
		*(int *)(PParm->Value[5]),*(int *)(PParm->Value[6]),*(int *)(PParm->Value[7]),*(int *)(PParm->Value[8]));
#endif
    if (StackPosition == MAX_RECURSIVE_CALL)
    {
        LOG(llevDebug, "PYTHON - Can't execute script - No space left of stack\n");
        return 0;
    }
    StackPosition++;
    StackActivator[StackPosition]   = (object *)(PParm->Value[1]);
    StackWho[StackPosition]         = (object *)(PParm->Value[2]);
    StackOther[StackPosition]       = (object *)(PParm->Value[3]);
    StackText[StackPosition]        = (char *)(PParm->Value[4]);
    StackParm1[StackPosition]       = *(int *)(PParm->Value[5]);
    StackParm2[StackPosition]       = *(int *)(PParm->Value[6]);
    StackParm3[StackPosition]       = *(int *)(PParm->Value[7]);
    StackParm4[StackPosition]       = *(int *)(PParm->Value[8]);
    StackReturn[StackPosition]      = 0;
    /* RunPythonScript(scriptname); */
    Scriptfile = fopen(create_pathname((char *)(PParm->Value[9])),"r");
    if (Scriptfile == NULL)
    {
        LOG(llevDebug, "PYTHON - The Script file %s can't be opened\n",(char *)(PParm->Value[9]));
        return 0;
    }
#ifdef PYTHON_DEBUG
    LOG(llevDebug, "PYTHON:: PyRun_SimpleFile! ");
#endif
    PyRun_SimpleFile(Scriptfile, create_pathname((char *)(PParm->Value[9])));
#ifdef PYTHON_DEBUG
    LOG(llevDebug, "closing (%d). ",StackPosition);
#endif
    fclose(Scriptfile);

#ifdef PYTHON_DEBUG
    LOG(llevDebug, "fixing. ");
#endif

    if (StackParm4[StackPosition] == SCRIPT_FIX_ALL)
    {
        if (StackOther[StackPosition] != NULL)
            fix_player_hook(StackOther[StackPosition]);
        if (StackWho[StackPosition] != NULL)
            fix_player_hook(StackWho[StackPosition]);
        if (StackActivator[StackPosition] != NULL)
            fix_player_hook(StackActivator[StackPosition]);
    }
    else if (StackParm4[StackPosition] == SCRIPT_FIX_ACTIVATOR)
    {
        fix_player_hook(StackActivator[StackPosition]);
    }
    StackPosition--;
#ifdef PYTHON_DEBUG
    LOG(llevDebug, "done (%d)!\n",StackReturn[StackPosition]);
#endif
    return StackReturn[StackPosition];
}

/*****************************************************************************/
/* Plugin initialization.                                                    */
/*****************************************************************************/
/* It is required that:                                                      */
/* - The first returned value of the CFParm structure is the "internal" name */
/*   of the plugin, used by objects to identify it.                          */
/* - The second returned value is the name "in clear" of the plugin, used for*/
/*   information purposes.                                                   */
/*****************************************************************************/
MODULEAPI CFParm* initPlugin(CFParm* PParm)
{
    LOG(llevDebug,"    CFPython Plugin loading.....\n");
    Py_Initialize();
    initCFPython();
    LOG(llevDebug, "[Done]\n");

    GCFP.Value[0] = (void *) PLUGIN_NAME;
    GCFP.Value[1] = (void *) PLUGIN_VERSION;
    return &GCFP;
}

/*****************************************************************************/
/* Used to do cleanup before killing the plugin.                             */
/*****************************************************************************/
MODULEAPI CFParm* removePlugin(CFParm* PParm)
{
        return NULL;
}

/*****************************************************************************/
/* This function is called to ask various informations to the plugin.        */
/*****************************************************************************/
MODULEAPI CFParm* getPluginProperty(CFParm* PParm)
{
    
    double dblval = 0.0;
    int i;
    if (PParm!=NULL)
    {
        if(PParm->Value[0] && !strcmp((char *)(PParm->Value[0]),"command?"))
        {
            if(PParm->Value[1] && !strcmp((char *)(PParm->Value[1]),PLUGIN_NAME))
            {
                GCFP.Value[0] = PParm->Value[1];
                GCFP.Value[1] = &cmd_aboutPython;
                GCFP.Value[2] = &dblval;
                return &GCFP;
            }
            else
            {
                for (i=0;i<NR_CUSTOM_CMD;i++)
                {
                    if (CustomCommand[i].name)
                    {
                        if (!strcmp(CustomCommand[i].name,(char *)(PParm->Value[1])))
                        {
                            LOG(llevDebug, "PYTHON - Running command %s\n",CustomCommand[i].name);
                            GCFP.Value[0] = PParm->Value[1];
                            GCFP.Value[1] = cmd_customPython;
                            GCFP.Value[2] = &(CustomCommand[i].speed);
                            NextCustomCommand = i;
                            return &GCFP;
                        }
                    }
                }
            }
        }
        else
        {
            LOG(llevDebug, "PYTHON - Unknown property tag: %s\n",(char *)(PParm->Value[0]));
        }
    }
    return NULL;
}

MODULEAPI int cmd_customPython(object *op, char *params)
{
    FILE* Scriptfile;
#ifdef PYTHON_DEBUG
    LOG(llevDebug, "PYTHON - cmd_customPython called:: script file: %s\n",CustomCommand[NextCustomCommand].script);
#endif
    if (StackPosition == MAX_RECURSIVE_CALL)
    {
        LOG(llevDebug, "PYTHON - Can't execute script - No space left of stack\n");
        return 0;
    }
    StackPosition++;
    StackActivator[StackPosition]   = op;
    StackWho[StackPosition]         = op;
    StackOther[StackPosition]       = op;
    StackText[StackPosition]        = params;
    StackReturn[StackPosition]      = 0;
    Scriptfile = fopen(create_pathname(CustomCommand[NextCustomCommand].script),"r");
    if (Scriptfile == NULL)
    {
        LOG(llevDebug, "PYTHON - The Script file %s can't be opened\n",CustomCommand[NextCustomCommand].script);
        return 0;
    }
    PyRun_SimpleFile(Scriptfile, create_pathname(CustomCommand[NextCustomCommand].script));
    fclose(Scriptfile);
    StackPosition--;
    return StackReturn[StackPosition+1];
}

MODULEAPI int cmd_aboutPython(object *op, char *params)
{
    int color = NDI_BLUE|NDI_UNIQUE;
    char message[1024];

    sprintf(message,"%s (Pegasus)\n(C)2001 by Gros. The Plugin code is under GPL.",PLUGIN_VERSION);
    GCFP.Value[0] = (void *)(&color);
    GCFP.Value[1] = (void *)(op->map);
    GCFP.Value[2] = (void *)(message);

    (PlugHooks[HOOK_NEWINFOMAP])(&GCFP);
    return 0;
}

/*****************************************************************************/
/* The postinitPlugin function is called by the server when the plugin load  */
/* is complete. It lets the opportunity to the plugin to register some events*/
/*****************************************************************************/
MODULEAPI CFParm* postinitPlugin(CFParm* PParm)
{
/*    int i; */
    /* We can now register some global events if we want */
    /* We'll only register the global-only events :      */
    /* BORN, CRASH, LOGIN, LOGOUT, REMOVE, and SHOUT.    */
    /* The events APPLY, ATTACK, DEATH, DROP, PICKUP, SAY*/
    /* STOP, TELL, TIME, THROW and TRIGGER are already   */
    /* handled on a per-object basis and I simply don't  */
    /* see how useful they could be for the Python stuff.*/
    /* Registering them as local would be probably useful*/
    /* for extended logging facilities.                  */

    LOG(llevDebug, "PYTHON - Start postinitPlugin.\n");
    
/*    GCFP.Value[1] = (void *)(add_string_hook(PLUGIN_NAME));*/
    GCFP.Value[1] = (void *)PLUGIN_NAME;
	/*
    i = EVENT_BORN;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);

    i = EVENT_CRASH;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);

    i = EVENT_LOGIN;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);

    i = EVENT_LOGOUT;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);

    i = EVENT_REMOVE;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);

    i = EVENT_SHOUT;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);

    i = EVENT_MAPENTER;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);

    i = EVENT_MAPLEAVE;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
*/
/*    i = EVENT_CLOCK; */
/*    GCFP.Value[0] = (void *)(&i); */
/*    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP); */
/*
    i = EVENT_MAPRESET;
    GCFP.Value[0] = (void *)(&i);
    (PlugHooks[HOOK_REGISTEREVENT])(&GCFP);
*/
    return NULL;
}

/*****************************************************************************/
/* Initializes the Python Interpreter.                                       */
/*****************************************************************************/
MODULEAPI void initCFPython()
{
        PyObject *m, *d;
        int i;

        LOG(llevDebug, "PYTHON - Start initCFPython.\n");
        
        m = Py_InitModule("CFPython", CFPythonMethods);
        d = PyModule_GetDict(m);
        CFPythonError = PyErr_NewException("CFPython.error",NULL,NULL);
        PyDict_SetItemString(d,"error",CFPythonError);
        for (i=0;i<NR_CUSTOM_CMD;i++)
        {
            CustomCommand[i].name   = NULL;
            CustomCommand[i].script = NULL;
            CustomCommand[i].speed  = 0.0;
        }

        /* Initialize our CF object wrapper */
        /* Gecko: TODO: error handling here */
        CFPython_ObjectType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&CFPython_ObjectType) < 0)            
            return;

        Py_INCREF(&CFPython_ObjectType);
        
        /* Initialize our CF mapstruct wrapper */
        /* Gecko: TODO: error handling here */
        CFPython_MapType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&CFPython_MapType) < 0)            
            return;

        Py_INCREF(&CFPython_MapType);
}

/****************************************************************************/
/* Code related to the new CFPython_Object class                            */
/****************************************************************************/

/* Create a new Object wrapper (uninitialized) */
static PyObject *
CFPython_Object_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CFPython_Object *self;

    self = (CFPython_Object *)type->tp_alloc(type, 0);
    if(self)
        self->obj = NULL;

    return (PyObject *)self;
}

/* Free an Object wrapper */
static void
CFPython_Object_dealloc(CFPython_Object* self)
{
    self->obj = NULL;
    self->ob_type->tp_free((PyObject*)self);
}

/* Return a string representation of this object (useful for debugging) */
static PyObject *
CFPython_Object_str(CFPython_Object *self)
{
    return PyString_FromFormat("[%s \"%s\"]", self->obj->arch->name, self->obj->name);
}

/* Utility method to wrap an object. */
static PyObject *
wrap_object(object *what)
{
    CFPython_Object *wrapper;
    
    /* return None if no object was to be wrapped */
    if(what == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    
    wrapper = PyObject_NEW(CFPython_Object, &CFPython_ObjectType);
    if(wrapper != NULL)
        wrapper->obj = what;

    return (PyObject *)wrapper;
}

/****************************************************************************/
/* Code related to the new CFPython_Map class                               */
/****************************************************************************/

/* Create a new (uninitialized) Map wrapper */
static PyObject *
CFPython_Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    CFPython_Map *self;

    self = (CFPython_Map *)type->tp_alloc(type, 0);
    if(self)
        self->map = NULL;

    return (PyObject *)self;
}

/* Free an Object wrapper */
static void
CFPython_Map_dealloc(CFPython_Map* self)
{
    self->map = NULL;
    self->ob_type->tp_free((PyObject*)self);
}

/* Return a string representation of this object (useful for debugging) */
static PyObject *
CFPython_Map_str(CFPython_Map *self)
{
    /* FIXME: strange happens when the map name is printed in the ingame message box
     *        (because of the special char "§" added to the map name) */
    return PyString_FromFormat("[%s \"%s\"]", self->map->path, self->map->name);
}

/* Utility method to wrap an object. */
static PyObject *
wrap_map(mapstruct *what)
{
    CFPython_Map *wrapper;
    
    /* return None if no map was to be wrapped */
    if(what == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    
    wrapper = PyObject_NEW(CFPython_Map, &CFPython_MapType);
    if(wrapper != NULL)
        wrapper->map = what;

    return (PyObject *)wrapper;
}
