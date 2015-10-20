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

#ifndef __LIVING_H
#define __LIVING_H

/* defines the stat range for strength and such for PLAYERS
 * to avoid the signed 8 bit border, we should assume to
 * mirror the with fix_player() altered stats to player struct */
/* TODO: The plan is to make MAX_STAT 200 or 250 (because a larger potential
 * range is good, and doubling the max allows plenty of progression in play
 * without makine current item+s, creation values, etc meaningless). This
 * will need stat_t increased from sint8 to sint16 which requires much more
 * testing then I am prepared to invest ATM.
 *
 * -- Smacky 20150714 */
#define MIN_STAT        1   /* The minimum legal value of any stat */
#define MAX_STAT        125  /* The maximum legal value of any stat */

enum stat_nr_t
{
    STAT_STR,
    STAT_DEX,
    STAT_CON,
    STAT_INT,
    STAT_WIS,
    STAT_POW,
    STAT_CHA,

    STAT_NROF,

    STAT_NONE = 99
};

extern const char  *attacks[NROFATTACKS];
extern const char  *spellpathnames[NRSPELLPATHS];
extern const float  stat_bonus[MAX_STAT + 1];
extern const char  *lose_msg[STAT_NROF];
extern const char  *restore_msg[STAT_NROF];
extern const char  *stat_name[STAT_NROF];
extern const char  *short_stat_name[STAT_NROF];

/* Unaligned: 37/37
 * Internal padding: 0/0
 * Trailing padding: 3/3
 * A proposal below was to save 2 bytes by changing ac and wc from sint16 to
 * uint8 which would decrease the footprint to 35 unaligned + 1 trailing = 36
 * aligned.
 *
 * -- Smacky 20140822 */
struct living_t
{
    sint32 exp;      // experience
    sint32 hp;       // current hit points
    sint32 maxhp;    // max hit points
    sint16 sp;       // current mana
    sint16 maxsp;    // max mana
    sint16 grace;    // current grace
    sint16 maxgrace; // max grace
    sint16 food;     // how much food in stomach NOT USED IN THIS WAY
    sint16 dam;      // how much damage this object does when hitting
    sint16 wc;       // we can safe here 2 bytes by setting wc/ac to uint8
    sint16 ac;       // i had tested it for all uses until now.
    sint8  thac0;    // every roll >= thac0 is a hit, despite of target ac
    sint8  thacm;    // every roll < thacm is a miss, despite of target ac
    stat_t Str;
    stat_t Dex;
    stat_t Con;
    stat_t Wis;
    stat_t Cha;
    stat_t Int;
    stat_t Pow;
};

extern void         set_stat_value(living_t *stats, stat_nr_t stat, sint16 value);
extern stat_t       get_stat_value(const living_t *const stats, const stat_nr_t stat);
extern void         check_stat_bounds(living_t *stats);
extern int          change_abil(object_t *op, object_t *tmp);
extern object_t    *check_obj_stat_buffs(object_t *ob, object_t *pl);
extern void         drain_stat(object_t *op);
extern void         drain_specific_stat(object_t *op, int deplete_stats);
extern void         drain_level(object_t *op, int level, int mode, int ticks);
extern void         fix_player_weight(object_t *op);
#ifdef DEBUG_FIX_PLAYER
extern void         fix_player(object_t *op, char *msg);
#else
extern void         fix_player(object_t *op);
#endif
extern void         fix_monster(object_t *op);
extern object_t    *insert_base_info_object(object_t *op);
extern object_t    *find_base_info_object(object_t *op);
extern void         set_mobile_speed(object_t *op, int factor);
extern void         leech_hind(object_t *leecher, object_t *leechee, uint8 attack, sint16 plose, sint16 pmod, uint8 chance);

#endif /* ifndef __LIVING_H */
