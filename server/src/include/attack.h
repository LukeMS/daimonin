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

#ifndef __ATTACK_H
#define __ATTACK_H

/* status for attack function to decide kind of attack */
typedef enum _env_attack
{
    ENV_ATTACK_CHECK = -1,
    ENV_ATTACK_YES,
    ENV_ATTACK_NO
}_env_attack;

/* Note that the last ATNR_ should be one less than NROFATTACKS above
 * since the ATNR starts counting at zero.
 * For compatible loading, these MUST correspond to the same value
 * as the bitmasks below.
 */
typedef enum _attacks
{
    /* We start with the double used attacks - for resist & protection too */
    /* damage type: physical */
    ATNR_IMPACT,
    ATNR_SLASH,
    ATNR_CLEAVE,
    ATNR_PIERCE,

    /* damage type: elemental */
    ATNR_FIRE,
    ATNR_COLD,
    ATNR_ELECTRICITY,
    ATNR_POISON,
    ATNR_ACID,
    ATNR_SONIC,

    /* damage type: magical */
    ATNR_CHANNELLING,
    ATNR_CORRUPTION,
    ATNR_PSIONIC,
    ATNR_LIGHT,
    ATNR_SHADOW,
    ATNR_LIFESTEAL,

    /* damage type: sphere */
    ATNR_AETHER,
    ATNR_NETHER,
    ATNR_CHAOS,
    ATNR_DEATH,

    /* damage: type only effect by invulnerable */
    ATNR_WEAPONMAGIC,
    ATNR_GODPOWER,

    /* at this point attack effects starts - only resist maps to it */
    ATNR_DRAIN,
    ATNR_DEPLETION,
    ATNR_COUNTERMAGIC,
    ATNR_CANCELLATION,
    ATNR_CONFUSION,
    ATNR_FEAR,
    ATNR_SLOW,
    ATNR_PARALYZE,
    ATNR_SNARE,

    /* and the real special one here */
    ATNR_INTERNAL,
    NROFATTACKS /* index (= 32 ATM) */
}_attacks;

/* defines the last real damage attack in array above */
#define LAST_ATNR_ATTACK (ATNR_GODPOWER)

typedef struct attack_name_t
{
    char *abbr;
    char *name;
} attack_name_t;

/* only the damage dealing attacks are covered by armour protections.
 * all attacks in the second part are effects and only covered from
 * resistance. */
extern attack_name_t attack_name[NROFATTACKS];
extern int   resist_table[];

#define num_resist_table 58

#endif /* ifndef __ATTACK_H */
