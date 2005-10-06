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


#ifndef ATTACK_H
#define ATTACK_H

/* Note that the last ATNR_ should be one less than NROFATTACKS above
 * since the ATNR starts counting at zero.
 * For compatible loading, these MUST correspond to the same value
 * as the bitmasks below.
 */
typedef enum _attacks
{
    /* We start with the double used attacks - for resist & protection too */
    /* damage type: physical */
    ATNR_PHYSICAL, /* = impact */
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
    ATNR_FORCE,
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
    ATNR_CORRUPTION,
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
};

/* only the damage dealing attacks are covered by armour protections.
 * all attacks in the second part are effects and only covered from
 * resistance.
 */
#define NROFPROTECTIONS (ATNR_GODPOWER+1)

#ifndef INIT_C

EXTERN int      resist_table[];
EXTERN char    *attack_name[NROFATTACKS];

#else

/* resist use the same names as attacks - they map 1:1 to it */
EXTERN char    *attack_name[NROFATTACKS]            =
{
    "impact", "slash", "cleave", "pierce", 
    "fire", "cold", "electricity", "poison", "acid", "sonic",
    "force", "psionic", "light", "shadow", "lifesteal",
    "aether", "nether", "chaos", "death", 
    "weaponmagic", "godpower",
    "drain", "depletion", "corruption",
    "countermagic", "cancellation", "confusion",
    "fear", "slow", "paralyze", "snare", "internal"
};


/* If you want to weight things so certain resistances show up more often than
 * others, just add more entries in the table for the protections you want to
 * show up.
 */
EXTERN int      resist_table[]                      =
{
    ATNR_SLASH, ATNR_CLEAVE, ATNR_PIERCE, ATNR_PHYSICAL, ATNR_FORCE, ATNR_FIRE, ATNR_ELECTRICITY, ATNR_COLD,
    ATNR_CONFUSION, ATNR_ACID, ATNR_DRAIN, ATNR_SHADOW, ATNR_POISON, ATNR_SLOW, ATNR_PARALYZE, ATNR_LIGHT, ATNR_FEAR,
    ATNR_SLASH, ATNR_DEPLETION, ATNR_CLEAVE, ATNR_SONIC, ATNR_PHYSICAL, ATNR_SNARE, ATNR_LIFESTEAL, ATNR_PSIONIC,
    ATNR_NETHER, ATNR_PIERCE, ATNR_SLASH, ATNR_CLEAVE, ATNR_PIERCE, ATNR_PHYSICAL, ATNR_FORCE, ATNR_FIRE,
    ATNR_ELECTRICITY, ATNR_COLD, ATNR_CONFUSION, ATNR_ACID, ATNR_DRAIN, ATNR_LIGHT, ATNR_POISON, ATNR_SLOW,
    ATNR_PARALYZE, ATNR_SNARE, ATNR_FEAR, ATNR_CANCELLATION, ATNR_DEPLETION, ATNR_COUNTERMAGIC, ATNR_SONIC, ATNR_CORRUPTION,
    ATNR_SNARE, ATNR_LIFESTEAL, ATNR_PSIONIC, ATNR_NETHER, ATNR_AETHER, ATNR_DEATH, ATNR_CHAOS, ATNR_GODPOWER,
    ATNR_WEAPONMAGIC
};

#endif

#define num_resist_table 58

#endif
