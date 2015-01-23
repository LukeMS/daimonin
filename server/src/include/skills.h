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
/*
 * First written 6 Sep 1994, Nick Williams (njw@cs.city.ac.uk)
 *
 * Initially, I was going to do this as spells, but there is so much
 * crap associated with spells (using sp, time to cast, might happen
 * as WONDER, etc) that I decided it was time to implement skills.  A
 * skill at the moment is merely a "flag" which is placed into the
 * player. The idea is that it will develop into having a rating,
 * which can be improved by practice.  A player could potentially
 * "learn" any number of skills, however this would be dependent on
 * their intelligence.  Perhaps skills should also record when they
 * were learnt, and their "rating" should decay as time goes by,
 * making characters have to either practice their skills, or re-learn
 * them.  BTW: Some skills should be dependent on having
 * tools... e.g. lockpicking would work much better if the player is
 * using a lockpick.
 */

/* Modification March 3 1995 - skills are expanded from stealing to
 * include another 14 skills. + skill code generalized. Some player
 * classes now may start with skills. -b.t. (thomas@nomad.astro.psu.edu)
 */

/* Modification April 21 1995 - more skills added, detect magic - detect
 * curse to woodsman - b.t. (thomas@nomad.astro.psu.edu)
 */

/* Modification May/June 1995 -
 *  HTH skills to allow hitting while moving.
 *  Modified the bargaining skill to allow incremental CHA increase (based on level)
 *  Added the inscription skill, finished play-testing meditation skill.
 *  - b.t. (thomas@astro.psu.edu)
 */

/* Modification June/July 1995 -
 *  1- Expansion of the skills code to fit within a scheme of multiple categories
 *  of experience. Henceforth, 2 categories of skills will exist: "associated"
 *  skills which are associated with one of the categories of experience, and
 *  "miscellaneous" skills, which are not related to any experience category.
 *  2- Moved the attacking and spellcasting player activities into skills.
 *  Now have "hand weapons" "missile weapons" "throwing" and "spellcasting".
 *  see doc/??? for details on this system.
 *  - b.t.
 */

#ifndef __SKILLS_H
#define __SKILLS_H

#define NO_SKILL_READY -1

typedef enum
{
    SKILLGROUP_AGILITY,
    SKILLGROUP_PERSONAL,
    SKILLGROUP_MENTAL,
    SKILLGROUP_PHYSIQUE,
    SKILLGROUP_MAGIC,
    SKILLGROUP_WISDOM,
    SKILLGROUP_MISC,

    NROFSKILLGROUPS
} ENUM_SKILL_GROUPS;

/* have a marker to all exp groups which define the real player level */
#define NROFSKILLGROUPS_ACTIVE SKILLGROUP_MISC

#define SK_ALCHEMY             0
#define SK_DIVINE_PRAYERS      1
#define SK_FIND_TRAPS          2
#define SK_LITERACY            3
#define SK_MAGIC_DEVICES       4
#define SK_MELEE_BASIC_CLEAVE  5
#define SK_MELEE_BASIC_IMPACT  6
#define SK_MELEE_BASIC_PIERCE  7
#define SK_MELEE_BASIC_SLASH   8
#define SK_MELEE_MASTERY_2H    9
#define SK_MELEE_MASTERY_POLE 10
#define SK_PUNCHING           11
#define SK_RANGE_BOW          12
#define SK_RANGE_SLING        13
#define SK_RANGE_XBOW         14
#define SK_REMOVE_TRAP        15
#define SK_THROWING           16
#define SK_WIZARDRY_SPELLS    17
#define NROFSKILLS            18

/* Skill leveling modes (->last_eat). There are 3 modes:
 *    NONLEVELING - cannot be leveled; the player either has it or he does not;
 *    INDIRECT - leveled indirectly (accumulates experience which causes level
 *    gain/loss when it crosses certain thresholds);
 *    DIRECT - leveled directly (does not accumulate experience in the normal
 *    way but gains/loses levels directly). */
#define NONLEVELING 0
#define INDIRECT    1
#define DIRECT      2

extern archetype_t *skillgroups[NROFSKILLGROUPS];
extern archetype_t *skills[NROFSKILLS];

/* yet more convenience macros. */

#define USING_SKILL(op, skill) \
    ((op)->chosen_skill->stats.sp == skill)

#endif /* ifndef __SKILLS_H */
