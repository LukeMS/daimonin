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


/* This is the order of the skills structure:
 *  char *name;
 *  short category;  - the associated experience category
 *  short time;      - the base number of ticks it takes to execute skill
 *  long  bexp;      - base exp gain for use of this skill
 *  float lexp;      - level multiplier for experience gain
 *  short stat1;     - primary stat, for linking to exp cat.
 *  short stat2;     - secondary stat ...
 *  short stat3;     - tertiary stat ...
 *
 * About time - this  number is the base for the use of the skill. Level
 * and associated stats can modify the amound of time to use the skill.
 * Time to use the skill is only used when 1) op is a player and 
 * 2) the skill is called through do_skill(). 
 * It is strongly recogmended that many skills *not* have a time value.
 *
 * About 'stats' and skill.category - a primary use for stats is determining
 * the associated experience category. 
 * Note that the ordering of the stats is important. Stat1 is the 'primary' 
 * stat, stat2 the 'secondary' stat, etc. In this scheme the primary stat 
 * is most important for determining the associated experience category. 
 * If a skill has the primary stat set to NO_STAT_VAL then it defaults to a 
 * 'miscellaneous skill'. 
 */

/* Don't change the order here w/o changing the skills.h file */

/* The default skills array, values can be overwritten by init_skills() 
 * in skill_util.c 
 */

skill   skills[NROFSKILLS]  =
{
    /* 0 */
    { "stealing",           NULL, SKILLGROUP_MISC, 0, 0, 0.1f,      DEX,     INTELLIGENCE, NO_STAT_VAL },
    { "pick locks",         NULL, SKILLGROUP_MISC, 0, 50, 1.5f,   DEX,     INTELLIGENCE, NO_STAT_VAL },
    { "hide in shadows",    NULL, SKILLGROUP_MISC, 0, 10, 2.5f,   DEX,     CHA, NO_STAT_VAL },
    { "smithery lore",      NULL, 2, 0, 0, 0.0f,     NO_STAT_VAL,   NO_STAT_VAL, NO_STAT_VAL },
    { "bowyer lore",        NULL, 2, 0, 0, 0.0f,     NO_STAT_VAL,   NO_STAT_VAL, NO_STAT_VAL },
    /* 5 */
    { "jeweler lore",       NULL, 2, 0, 0, 0.0f,    NO_STAT_VAL,     NO_STAT_VAL,     NO_STAT_VAL },
    { "alchemy",            NULL, SKILLGROUP_MISC, 10, 1, 1.0f,    INTELLIGENCE,    WIS,     DEX },
    { "magic lore",         NULL, 2, 0, 0, 0.0f,    NO_STAT_VAL,     NO_STAT_VAL,     NO_STAT_VAL },
    { "common literacy",    NULL, SKILLGROUP_MISC, 10, 1, 1.0f,    INTELLIGENCE,    WIS, NO_STAT_VAL },
    { "bargaining",         NULL, SKILLGROUP_MISC, 0, 0, 0.0f,  NO_STAT_VAL, NO_STAT_VAL, NO_STAT_VAL },
    /* 10 */
    { "jumping",            NULL, SKILLGROUP_MISC, 0, 5, 2.5f,      NO_STAT_VAL,     NO_STAT_VAL, NO_STAT_VAL },
    { "sense magic",        NULL, SKILLGROUP_MISC, 10, 10, 1.0f,   POW,     INTELLIGENCE, NO_STAT_VAL },
    { "oratory",            NULL, SKILLGROUP_MISC, 5, 1, 2.0f,      CHA,     INTELLIGENCE, NO_STAT_VAL },
    { "singing",            NULL, SKILLGROUP_MISC, 5, 1, 2.0f,      CHA,     INTELLIGENCE, NO_STAT_VAL },
    { "sense curse",        NULL, SKILLGROUP_MISC, 10, 10, 1.0f,   WIS,     POW, NO_STAT_VAL },
    /* 15 */
    { "find traps",         NULL, SKILLGROUP_MISC, 0, 0, 0.0f,  DEX, NO_STAT_VAL, NO_STAT_VAL },
    { "meditation",         NULL, SKILLGROUP_MISC, 10, 0, 0.0f, WIS,      POW,     INTELLIGENCE },
    { "punching",           NULL, SKILLGROUP_MISC, 0, 0, 1.0f,      STR,      DEX, NO_STAT_VAL },
    { "flame touch",        NULL, SKILLGROUP_MISC, 0, 0, 1.0f,      STR,      DEX,     INTELLIGENCE },
    { "karate",             NULL, SKILLGROUP_MISC, 0, 0, 1.0f,      STR,      DEX, NO_STAT_VAL },
    /* 20 */
    { "mountaineer",        NULL, SKILLGROUP_MISC, 0, 0, 0.0f,  NO_STAT_VAL, NO_STAT_VAL, NO_STAT_VAL },
    { "ranger lore",        NULL, 6, 0, 0, 0.0f,    NO_STAT_VAL,     NO_STAT_VAL, NO_STAT_VAL },
    { "inscription",        NULL, SKILLGROUP_MISC, 0, 1, 5.0f,      POW,      INTELLIGENCE,  NO_STAT_VAL },
    { "impact weapons",     NULL, SKILLGROUP_MISC, 0, 0, 1.0f,      STR,      DEX, NO_STAT_VAL },
    { "bow archery",        NULL, SKILLGROUP_MISC, 0, 0, 1.0f,     DEX,      STR, NO_STAT_VAL },
    /* 25 */
    { "throwing",           NULL, SKILLGROUP_MISC, 1, 0, 1.0f,      DEX,      DEX, NO_STAT_VAL },
    { "wizardry spells",    NULL, SKILLGROUP_MISC, 1, 0, 0.0f,      POW,      INTELLIGENCE,       WIS },
    { "remove traps",       NULL, SKILLGROUP_MISC, 0, 1, 0.5f,    DEX,      INTELLIGENCE, NO_STAT_VAL },
    { "set traps",          NULL, SKILLGROUP_MISC, 0, 1, 0.5f,    INTELLIGENCE,      DEX, NO_STAT_VAL },
    { "magic devices",      NULL, SKILLGROUP_MISC, 4, 0, 1.0f,   POW, DEX, NO_STAT_VAL }, 
    /* 30 */
    { "divine prayers",     NULL, SKILLGROUP_MISC, 0, 0, 0.0f,      WIS,      POW,     INTELLIGENCE },
    { "clawing",            NULL, SKILLGROUP_MISC, 0, 0, 0.0f,      STR,      DEX, NO_STAT_VAL },
    { "levitation",         NULL, SKILLGROUP_MISC, 0, 0, 0.0f, NO_STAT_VAL, NO_STAT_VAL, NO_STAT_VAL },
    { "disarm traps",       NULL, SKILLGROUP_MISC, 0, 1, 0.5f,      DEX,      INTELLIGENCE,     INTELLIGENCE },
    { "crossbow archery",   NULL, SKILLGROUP_MISC, 0, 0, 1.0f,     DEX,      STR, NO_STAT_VAL },
	/* 35 */
    { "sling archery",      NULL, SKILLGROUP_MISC, 0, 0, 1.0f,     DEX,      STR, NO_STAT_VAL },
    { "identify items",     NULL, SKILLGROUP_MISC, 10, 1, 1.0f,      INTELLIGENCE,      DEX,     WIS },
    { "slash weapons",      NULL, SKILLGROUP_MISC, 0, 0, 1.0f,      STR,      DEX, NO_STAT_VAL },
    { "cleave weapons",     NULL, SKILLGROUP_MISC, 0, 0, 1.0f,      STR,      DEX, NO_STAT_VAL },
    { "pierce weapons",     NULL, SKILLGROUP_MISC, 0, 0, 1.0f,      STR,      DEX, NO_STAT_VAL },
	/* 40 */
    { "two-hand mastery",   NULL, SKILLGROUP_MISC, 0, 0, 0.0f,      STR,      DEX, NO_STAT_VAL },
    { "polearm mastery",    NULL, SKILLGROUP_MISC, 0, 0, 0.0f,      STR,      DEX, NO_STAT_VAL }
};



