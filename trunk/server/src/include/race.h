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

#ifndef __RACE_H
#define __RACE_H

/* thats the race list - init is in init.c */
struct racelink_t
{
    /* Used to link the race lists together */
    const char                       *name;       /* name of this race entry */
    int                               nrof;       /* nrof things belonging to this race */
    archetype_t                     *corpse; /* thats the default corpse of this race */
    objectlink_t                     *member; /* linked object list of things belonging to this race */
    racelink_t                     *next;
    struct mob_behaviourset          *ai; /* That is the default ai for this race */

    // The first and last indices of the start_locations array in map.c
    // that match up to this race. Used to pick a random location.
    sint8                             first_start_location;
    sint8                             last_start_location;
};

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

typedef struct _race_map_start_location
{
    const char *race;
    const char *map;
    sint16      x;
    sint16      y;
    sint16      status;
} _race_start_location;

#define NUM_START_LOCATIONS 1
extern _race_start_location race_start_locations[NUM_START_LOCATIONS];

extern struct _races    item_race_table[RACE_NAME_INIT];

#endif /* ifndef __RACE_H */
