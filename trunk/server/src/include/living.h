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

#ifndef LIVING_H
#define LIVING_H

#define STR 0
#define DEX 1
#define CON 2
#define WIS 3
#define CHA 4
#define INTELLIGENCE 5
#define POW 6

#define NUM_STATS 7

/* Changed from NO_STAT to NO_STAT_VAL to fix conlfict on
 * AIX systems
 */
#define NO_STAT_VAL 99    /* needed by skills code -b.t. */

extern char    *attacks[NROFATTACKS];
extern char    *spellpathnames[NRSPELLPATHS];
extern float    stats_penalty[10];
extern char    *restore_msg[NUM_STATS];
extern char    *statname[NUM_STATS];
extern char    *short_stat_name[NUM_STATS];
extern char    *lose_msg[NUM_STATS];

#ifdef WIN32
#pragma pack(push,1)
#endif

typedef struct liv
{
    /* Mostly used by "alive" objects */
    sint32          exp;        /* Experience. */
    sint32          hp;         /* Real Hit Points. */
    sint32          maxhp;      /* max hit points */
    sint16          sp;         /* Spell points.  Used to cast mage spells. */
    sint16          maxsp;      /* Max spell points. */
    sint16          grace;      /* Grace.  Used to invoke clerical prayers. */
    sint16          maxgrace;   /* Grace.  Used to invoke clerical prayers. */
    sint16          food;       /* How much food in stomach.  0 = starved. */
    sint16          dam;        /* How much damage this object does when hitting */
    /* We can safe here 2 bytes by setting wc/ac to uint8 -
     * i had tested it for all uses until now.
     */
    sint16          wc, ac;     /* Weapon Class and Armour Class */

    sint8           thac0;      /* Every roll >= thac0 is a hit, despite of target ac */
    sint8           thacm;      /* Every roll < thacm is a miss, despite of target ac */
    sint8           Str, Dex, Con, Wis, Cha, Int, Pow; /* the stats */
} living;

#ifdef WIN32
#pragma pack(pop)
#endif

#endif
