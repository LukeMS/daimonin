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

#include <daimonin_map.h>
#include <inline.h>

/* Global data objects */

static PyMethodDef MapMethods[] =
{
    {"GetFirstObjectOnSquare", (PyCFunction)Daimonin_Map_GetFirstObjectOnSquare, METH_VARARGS},
    {"PlaySound",(PyCFunction)Daimonin_Map_PlaySound,METH_VARARGS},
    {"Message", (PyCFunction)Daimonin_Map_Message,METH_VARARGS},
    {"MapTileAt",  (PyCFunction)Daimonin_Map_MapTileAt, METH_VARARGS},
    {"CreateObject", (PyCFunction)Daimonin_Map_CreateObject,METH_VARARGS},
    {NULL, NULL}
};

static struct {
    char *name;
    field_type type;
    uint32 offset;    /* Offset in map struct */
} map_fields[] = {
    { "name",    FIELDTYPE_CSTR, offsetof(mapstruct, name) },
    { "message", FIELDTYPE_CSTR, offsetof(mapstruct, msg) },
    { "reset_interval", FIELDTYPE_UINT32, offsetof(mapstruct, reset_timeout) },
    { "difficulty", FIELDTYPE_UINT16, offsetof(mapstruct, difficulty) },
    { "height", FIELDTYPE_UINT16, offsetof(mapstruct, height) },
    { "width", FIELDTYPE_UINT16, offsetof(mapstruct, width) },
    { "darkness", FIELDTYPE_UINT8, offsetof(mapstruct, darkness) },
    { "path", FIELDTYPE_CARY, offsetof(mapstruct, path) }
};

static char *mapflag_names[] = {
    "f_outdoor", "f_unique", "f_fixed_rtime", 
    "f_nomagic", "f_nopriest", "f_noharm", "f_nosummon",
    "f_fixed_login", "f_permdeath", "f_ultradeath", "f_ultimatedeath", "f_pvp"
};

#define NUM_MAPFIELDS (sizeof(map_fields) / sizeof(map_fields[0]))
#define NUM_MAPFLAGS (sizeof(mapflag_names) / sizeof(mapflag_names[0]))

/* This is filled in when we initialize our map type */
static PyGetSetDef Map_getseters[NUM_MAPFIELDS + NUM_MAPFLAGS + 1];

PyTypeObject Daimonin_MapType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /* ob_size*/
    "Daimonin.Map",         /* tp_name*/
    sizeof(Daimonin_Map),   /* tp_basicsize*/
    0,                         /* tp_itemsize*/
    (destructor)Daimonin_Map_dealloc, /* tp_dealloc*/
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
    (reprfunc)Daimonin_Map_str,/* tp_str*/
    0,                         /* tp_getattro*/
    0,                         /* tp_setattro*/
    0,                         /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /* tp_flags*/
    "Daimonin maps",           /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    MapMethods,                /* tp_methods */
    0,                         /* tp_members */
    Map_getseters,             /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    Daimonin_Map_new,       /* tp_new */
};

/* Map related constants */
static Daimonin_Constant map_constants[] = {
    {"COST_TRUE", F_TRUE},
    {"COST_BUY", F_BUY},
    {"COST_SELL", F_SELL},
    {NULL, 0}
};

/****************************************************************************/
/*                          Daimonin_Map methods                            */
/****************************************************************************/

/* FUNCTIONSTART -- Here all the Python plugin functions come */

/*****************************************************************************/
/* Name   : Daimonin_Map_GetFirstObjectOnSquare                              */
/* Python : map.GetFirstObjectOnSquare(x,y)                                  */
/* Info   : Gets the bottom object on the tile. Use obj.above to browse objs */
/* Status : Stable                                                           */
/*****************************************************************************/
static PyObject* Daimonin_Map_GetFirstObjectOnSquare(Daimonin_Map *map, PyObject* args)
{
    int x, y;
    object* val;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"ii",&x,&y))
        return NULL;

    GCFP.Value[0] = map->map;
    GCFP.Value[1] = (void *)(&x);
    GCFP.Value[2] = (void *)(&y);
    CFR = (PlugHooks[HOOK_GETMAPOBJECT])(&GCFP);
    val = (object *)(CFR->Value[0]);

    /* free(CFR); */ 

    return wrap_object(val);
}

/*****************************************************************************/
/* Name   : Daimonin_Map_MapTileAt                                           */
/* Python : map.MapTileAt(x,y)                                               */
/* Status : untested                                                         */
/* TODO   : do someting about the new modified coordinates too?              */
/*****************************************************************************/
static PyObject* Daimonin_Map_MapTileAt(Daimonin_Map *map, PyObject* args)
{
    int x, y;    
    CFParm* CFR;
    mapstruct *result;
    
    if (!PyArg_ParseTuple(args,"ii", &x,&y))
        return NULL;
    
    GCFP.Value[0] = map->map;
    GCFP.Value[1] = (void *)(&x);
    GCFP.Value[2] = (void *)(&y);
    CFR = (PlugHooks[HOOK_OUTOFMAP])(&GCFP);   
    result = (mapstruct *)(CFR->Value[0]);

    return wrap_map(result);
}


/*****************************************************************************/
/* Name   : Daimonin_Map_PlaySound                                           */
/* Python : map.PlaySound(x, y, soundnumber, soundtype)                      */
/* Status : Tested                                                           */
/* TODO   : supply constants for the sounds                                  */
/*****************************************************************************/
static PyObject* Daimonin_Map_PlaySound(Daimonin_Map *whereptr, PyObject* args)
{
    int x, y, soundnumber, soundtype;

    if (!PyArg_ParseTuple(args,"iiii", &x, &y, &soundnumber, &soundtype))
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

/*****************************************************************************/
/* Name   : Daimonin_Map_Message                                             */
/* Python : map.Message(message, x, y, distance, color)                      */
/* Info   : Writes a message to all players on a map                         */
/*          Starting point x,y for all players in distance                   */
/*          default color is NDI_BLUE|NDI_UNIQUE                             */
/* Status : Tested                                                           */
/* TODO   : Add constants for colours                                        */
/*****************************************************************************/

static PyObject* Daimonin_Map_Message(Daimonin_Map *map, PyObject* args)
{
    int   color = NDI_BLUE|NDI_UNIQUE, x,y, d;
    char *message;

    if (!PyArg_ParseTuple(args,"iiis|i",&x,&y, &d, &message, &color))
        return NULL;

    GCFP.Value[0] = (void *)(&color);
    GCFP.Value[1] = (void *)(map->map);
    GCFP.Value[2] = (void *)(&x);
    GCFP.Value[3] = (void *)(&y);
    GCFP.Value[4] = (void *)(&d);
    GCFP.Value[5] = (void *)(message);

    (PlugHooks[HOOK_NEWINFOMAP])(&GCFP);

    Py_INCREF(Py_None);
    return Py_None;
}

/*****************************************************************************/
/* Name   : Daimonin_Map_CreateObject                                        */
/* Python : map.CreateObject(arch_name, x, y)                                */
/* Info   :                                                                  */
/* Status : Untested                                                         */
/*****************************************************************************/
static PyObject* Daimonin_Map_CreateObject(Daimonin_Map* map, PyObject* args)
{
    char *txt;
    int x,y;
    CFParm* CFR;

    if (!PyArg_ParseTuple(args,"sii",&txt, &x,&y))
        return NULL;
    
    GCFP.Value[0] = (void *)(txt);
    GCFP.Value[1] = (void *)(map->map);
    GCFP.Value[2] = (void *)(&x);
    GCFP.Value[3] = (void *)(&y);
    CFR = (PlugHooks[HOOK_CREATEOBJECT])(&GCFP);

    return wrap_object((object *)(CFR->Value[0]));
}


/* FUNCTIONEND -- End of the Python plugin functions. */

/*****************************************************************************/
/* Map attribute getter                                                      */
/*****************************************************************************/

static PyObject* Map_GetAttribute(Daimonin_Map* map, int fieldno)
{
    void *field_ptr;

    if(fieldno < 0 || fieldno >= NUM_MAPFIELDS)
        RAISE("Illegal field ID"); 
    
    field_ptr = (void *)((char *)(map->map) + map_fields[fieldno].offset);
    
    /* TODO: better handling of types, signs, and overflows */
    switch(map_fields[fieldno].type) {
        case FIELDTYPE_SHSTR:
        case FIELDTYPE_CSTR:   return Py_BuildValue("s", *(char **)field_ptr);
        case FIELDTYPE_CARY:   return Py_BuildValue("s", (char *)field_ptr);
        case FIELDTYPE_UINT8:  return Py_BuildValue("b", *(uint8 *)field_ptr);
        case FIELDTYPE_SINT8:  return Py_BuildValue("b", *(sint8 *)field_ptr);
        case FIELDTYPE_UINT16: return Py_BuildValue("i", *(uint16 *)field_ptr);
        case FIELDTYPE_SINT16: return Py_BuildValue("i", *(sint16 *)field_ptr);
        case FIELDTYPE_UINT32: return Py_BuildValue("l", *(uint32 *)field_ptr);
        case FIELDTYPE_SINT32: return Py_BuildValue("l", *(sint32 *)field_ptr);
        case FIELDTYPE_FLOAT:  return Py_BuildValue("f", *(float *)field_ptr);
        case FIELDTYPE_MAP:    return wrap_map(*(mapstruct **)field_ptr);
        case FIELDTYPE_OBJECT: return wrap_object(*(object **)field_ptr);

        default:
            RAISE("BUG: unknown field type");
    }
}

/*****************************************************************************/
/* Map flag getter                                                           */
/*****************************************************************************/
static PyObject* Map_GetFlag(Daimonin_Map* map, int flagno)
{
    if(flagno < 0 || flagno >= NUM_MAPFLAGS)
        RAISE("Unknown flag");

    return Py_BuildValue("i", (map->map->map_flags & (1 << flagno)) ? 1 : 0);
}


/****************************************************************************/
/* Daimonin_Map object management                                            */
/****************************************************************************/

/* Initialize the Map Object Type */
int Daimonin_Map_init(PyObject *module)
{
    int i;
             
    /* field getters */
    for(i=0; i< NUM_MAPFIELDS; i++) {
        PyGetSetDef *def = &Map_getseters[i];
        def->name = map_fields[i].name;
        def->get = (getter)Map_GetAttribute;
        def->set = NULL;
        def->doc = NULL;
        def->closure = (void *)i;
    }

    /* flag getters */
    for(i = 0; i < NUM_MAPFLAGS; i++) {
        PyGetSetDef *def = &Map_getseters[i + NUM_MAPFIELDS];
        def->name = mapflag_names[i];
        def->get = (getter)Map_GetFlag;
        def->set = NULL;
        def->doc = NULL;
        def->closure = (void *)i;
    }
    Map_getseters[NUM_MAPFIELDS + NUM_MAPFLAGS].name = NULL;
    
    /* Add constants */
    for(i=0; map_constants[i].name; i++) 
        if(PyModule_AddIntConstant(module, map_constants[i].name, map_constants[i].value))
            return -1;
    
    Daimonin_MapType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&Daimonin_MapType) < 0)            
        return -1;

    Py_INCREF(&Daimonin_MapType);
    return 0;
}

/* Create a new (uninitialized) Map wrapper */
static PyObject *
Daimonin_Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Daimonin_Map *self;

    self = (Daimonin_Map *)type->tp_alloc(type, 0);
    if(self)
        self->map = NULL;

    return (PyObject *)self;
}

/* Free an Object wrapper */
static void
Daimonin_Map_dealloc(Daimonin_Map* self)
{
    self->map = NULL;
    self->ob_type->tp_free((PyObject*)self);
}

/* Return a string representation of this object (useful for debugging) */
static PyObject *
Daimonin_Map_str(Daimonin_Map *self)
{
    char buf[HUGE_BUF], *ptr;
    strcpy(buf, self->map->name);
    if((ptr = strchr(buf, '�')))
        *ptr = '_';
        
    return PyString_FromFormat("[%s \"%s\"]", self->map->path, buf);
}

/* Utility method to wrap an object. */
PyObject *wrap_map(mapstruct *what)
{
    Daimonin_Map *wrapper;
    
    /* return None if no map was to be wrapped */
    if(what == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    
    wrapper = PyObject_NEW(Daimonin_Map, &Daimonin_MapType);
    if(wrapper != NULL)
        wrapper->map = what;

    return (PyObject *)wrapper;
}
