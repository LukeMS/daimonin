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

#ifndef DAIMONIN_MAP_H
#define DAIMONIN_MAP_H

/* First the required header files - only the CF module interface and Python */
#include <Python.h>
#include <plugin.h>
#include <plugin_python.h>

/* Map object methods */
static PyObject* Daimonin_Map_Save(Daimonin_Map *map, PyObject* args);
static PyObject* Daimonin_Map_Delete(Daimonin_Map *map, PyObject* args);
static PyObject* Daimonin_Map_GetFirstObjectOnSquare(Daimonin_Map *self, PyObject* args);
static PyObject* Daimonin_Map_PlaySound(Daimonin_Map *self, PyObject* args);
static PyObject* Daimonin_Map_Message(Daimonin_Map *self, PyObject* args);
static PyObject* Daimonin_Map_MapTileAt(Daimonin_Map *self, PyObject* args);
static PyObject* Daimonin_Map_CreateObject(Daimonin_Map* map, PyObject* args);

/* Object creator (not really needed, since the generic creator does the same thing...) */
static PyObject *
Daimonin_Map_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

/* str() function to get a string representation of this object */
static PyObject *
Daimonin_Map_str(Daimonin_Map *self);
   
/* Object deallocator (needed) */
static void
Daimonin_Map_dealloc(Daimonin_Map* self);

#endif /*PLUGIN_PYTHON_H*/
