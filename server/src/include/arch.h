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

#ifndef ARCH_H
#define ARCH_H

/*
 * The archetype structure is a set of rules on how to generate and manipulate
 * objects which point to archetypes.
 * This probably belongs in arch.h, but there really doesn't appear to
 * be much left in the archetype - all it really is is a holder for the
 * object and pointers.  This structure should get removed, and just replaced
 * by the object structure
 */
typedef struct archt {
    const char *name;			/* More definite name, like "generate_kobold" */
    struct archt *next;			/* Next archetype in a linked list */
    struct archt *head;			/* The main part of a linked object */
    struct archt *more;			/* Next part of a linked object */
	object *base_clone;	        /* used by artifacts list: if != NULL,
								 * this object is the base object and clone is
								 * the modified artifacts object.
								 * we use base_clone for unidentified objects 
								 * (to get unified "non identified" values),
								 * or it is used to get a base object when we
								 * remove the artifacts changes (cancellation, dispel...)
								 */
    object		 clone;			/* An object from which to do copy_object() */
    struct mob_behaviourset *ai; /* arch-default ai definition (optional)*/
} archetype;

EXTERN archetype *first_archetype;

#endif /* ARCH_H */
