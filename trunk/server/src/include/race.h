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
/* thats the race list - init is in init.c */
typedef struct ralnk
{
    /* Used to link the race lists together */
    const char                 *name;       /* name of this race entry */ 
    int                         nrof;       /* nrof things belonging to this race */
    struct archt               *corpse; /* thats the default corpse of this race */
    struct oblnk               *member; /* linked object list of things belonging to this race */
    struct ralnk               *next;

    struct mob_behaviourset    *ai; /* That is the default ai for this race */
} racelink;

#define RACE_TYPE_NONE 0

/* WARNING: this list is used for the items prefixes and not for the race
 * list. Both are different lists with different meanings.
 */
typedef enum _race_names_enum
{
    RACE_NAME_DEFAULT,
    RACE_NAME_DWARVEN,
    RACE_NAME_ELVEN,
    RACE_NAME_GNOMISH,
    RACE_NAME_DROW,
    RACE_NAME_ORCISH,
    RACE_NAME_GOBLIN,
    RACE_NAME_KOBOLD,
    RACE_NAME_GIANT,
    RACE_NAME_TINY,
    RACE_NAME_DEMONISH,
    RACE_NAME_DRACONISH,
    RACE_NAME_OGRE,
    RACE_NAME_INIT
}   _race_names_enum;

typedef struct _races
{
    char                   *name;               /* prefix name for this race */
    uint32                  usable;        /* race can use (wear, wield, apply...) items from this races */
}_races;

extern struct _races    item_race_table[RACE_NAME_INIT];
