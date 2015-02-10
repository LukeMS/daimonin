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

/* TODO: The generic name for (wizard) spells and (divine) prayers is
 * castables, so the filenames/prefixes will be changed accordingly.
 *
 * -- Smacky 20140815 */

#include <global.h>

#ifdef NO_ERRNO_H
extern int  errno;
#else
#   include <errno.h>
#endif

spell spells[NROFREALSPELLS]          =
{
    {"firestorm",                   SPELL_TYPE_WIZARD, 2,
    4, 12, 3, 6,0, 1.0,
    65,      7,      4,      0,
    1, 4, 0, 5, SOUND_MAGIC_FIRE,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_DUST | SPELL_USE_BOOK | SPELL_USE_POTION,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "firebreath",SPELL_ACTIVE
    },

    {"icestorm",                    SPELL_TYPE_WIZARD, 2,
    4, 12, 3, 6,0,1.0,
    65,      7,      4,      0,
    1, 4, 0, 5, SOUND_MAGIC_ICE,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_BOOK | SPELL_USE_POTION | SPELL_USE_DUST,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "icestorm",SPELL_ACTIVE
    },

    {"minor healing",               SPELL_TYPE_PRIEST, 1,
    6, 8, 3, 6,3,1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_BALM | SPELL_USE_SCROLL | SPELL_USE_ROD | SPELL_USE_POTION | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS | SPELL_DESC_TOWN,
    PATH_LIFE, "meffect_green",SPELL_ACTIVE
    },

    {"cure poison",                 SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,4, 1.0,/* potion only */
    0,       0,     0,      0,
    0, 0, 0, 0,   SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_POTION | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS | SPELL_DESC_TOWN,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"cure disease",                SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,4, 1.0,/* balm only */
    0,       0,     0,      0,
    0, 0, 0, 0,   SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_BALM | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS | SPELL_DESC_TOWN,
    PATH_LIFE,"meffect_purple",SPELL_ACTIVE
    },

    {"strength self",               SPELL_TYPE_WIZARD, 1,
    5, 12, 3, 6,0,1.0,
    0,       0,     0,      4,
    0, 0, 0, 3,  SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_POTION | SPELL_USE_ROD | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_TOWN,
    PATH_NATURE, "meffect_yellow",SPELL_ACTIVE
    },

    {"identify",                    SPELL_TYPE_WIZARD, 2,
    5, 24, 3, 6,2,1.0,
    0,       0,     0,      0,
    0, 0, 0, 3,  SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_TOWN,
    PATH_TRANSMUTATION, "meffect_pink",SPELL_ACTIVE
    },

    {"detect magic",                SPELL_TYPE_WIZARD, 2,
    5, 8, 3, 6,0,1.0,
    0,       0,     0,      0,
    0, 0, 0, 3,   SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_TOWN,
    PATH_ABJURATION, "meffect_pink",SPELL_ACTIVE
    },

    {"detect curse",                SPELL_TYPE_PRIEST, 2,
    5, 8, 3, 6,0,1.0,
    0,       0,     0,      0,
    0, 0, 0, 0,   SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_ABJURATION, "meffect_pink",SPELL_ACTIVE
    },

    {"remove curse",                SPELL_TYPE_PRIEST, 2,
    5, 24, 3, 6,2,1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_BOOK,     /* scroll */
    SPELL_DESC_SELF | SPELL_DESC_TOWN | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS,
    PATH_ARCANE, "meffect_blue",SPELL_ACTIVE
    },

    {"remove damnation",            SPELL_TYPE_PRIEST, 2,
    5, 36, 3, 6,2,1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_BOOK, /* scroll*/
    SPELL_DESC_SELF | SPELL_DESC_TOWN | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS,
    PATH_ARCANE, "meffect_blue",SPELL_ACTIVE
    },

    {"cause light wounds",          SPELL_TYPE_PRIEST, 1,
    3, 14, 3, 6,0,  1.0,/* scroll*/
    95,      5,      4,      4,
    1, 6, 0, 5,  SOUND_MAGIC_WOUND,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_DIRECTION | SPELL_DESC_WIS,
    PATH_DEATH,"cause_wounds",SPELL_ACTIVE
    },

    {"firebolt",                   SPELL_TYPE_WIZARD, 2,
    6, 12, 3, 6,0, 1.0,
    108,      8,      4,      0,
    1, 3, 0, 5, SOUND_MAGIC_FIRE,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_DUST | SPELL_USE_BOOK | SPELL_USE_POTION,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "firebolt", SPELL_ACTIVE
    },

    {"magic bullet",                SPELL_TYPE_WIZARD, 1,
    3, 14, 3, 6,0,1.0,
    95,      5,      4,      4,
    1, 6, 0, 5,   SOUND_MAGIC_BULLET1,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_BOOK,
    SPELL_DESC_DIRECTION,
    PATH_CHAOS, "bullet",SPELL_ACTIVE
    },

    {"frostbolt",                   SPELL_TYPE_WIZARD, 2,
    6, 12, 3, 6,0, 1.0,
    108,      8,      4,      0,
    1, 3, 0, 5, SOUND_MAGIC_ICE,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_DUST | SPELL_USE_BOOK | SPELL_USE_POTION,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "frostbolt", SPELL_ACTIVE
    },

    {"remove depletion",            SPELL_TYPE_PRIEST, 2,
    5, 24, 3, 6,4, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0,    SOUND_MAGIC_STAT,
    SPELL_USE_CAST,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"probe",                       SPELL_TYPE_WIZARD, 1,
    5, 8, 3, 6,0,1.0,
    1,      5,      4,      4,
    0, 0, 0, 1, SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_BOOK,
    SPELL_DESC_TOWN | SPELL_DESC_DIRECTION,
    PATH_SPIRIT, "probebullet",SPELL_ACTIVE
    },

    {"remove death sickness",            SPELL_TYPE_PRIEST, 2,
    5, 24, 3, 6,0, 1.0,
    0,       0,     0,      0,
    0,  0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST, /* npc/god only atm */
    SPELL_DESC_SELF | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"restoration",            SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"lightning",                   SPELL_TYPE_WIZARD, 2,
    6, 12, 3, 6,0, 1.0,
    108,      8,      4,      0,
    1, 3, 0, 5, SOUND_MAGIC_ELEC,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_DUST | SPELL_USE_BOOK | SPELL_USE_POTION,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "lightning", SPELL_ACTIVE
    },

    {"remove slow",            SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove fear",            SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove snare",            SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove paralysis",            SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove confusion",            SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove blindness",            SPELL_TYPE_PRIEST, 2,
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

};

/* TODO: Reference only. Will be removed. */
#if 0
//{"probe",            1, 3, 40, 2, 1, 75,  6,  1, 0, 0, 0,
//0,0,0,0,0,0,1,0,0,1,
// PATH_INFO, "probebullet",},
//
//
//spell spells_DUMMY[]={
//{"small fireball",           1, 6, 40, 5, 0, 0,  8,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,1,
// PATH_FIRE, "firebullet_s",},
//{"medium fireball",          3,10, 20, 10, 0, 0,  6,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,1,
// PATH_FIRE, "firebullet_m",},
//{"large fireball",           5,16, 10, 15, 0, 0,  2,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,1,
// PATH_FIRE, "firebullet_l",},
//{"small lightning",      1, 6, 40, 5, 0, 0,  8,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_ELEC, "lightning_s",},
//{"large lightning",      4, 13, 20, 12, 0, 0,  3,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_ELEC, "lightning_l",},
//{"magic missile",            1, 1, 75, 3, 0, 0,  8,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_MISSILE, "magic_missile",},
//{"create bomb",              6,10, 5, 20, 1, 1,  3,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_DETONATE, "bomb",},
//{"summon golem",             2, 5, 10, 30, 1, 1,  8,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "golem",},
//{"summon fire elemental",    7,25, 4,  40, 1, 1,  2,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "fire_elemental",},
//{"summon earth elemental",   4,15, 10, 40, 1, 1,  3,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "earth_elemental",},
//{"summon water elemental",   5,15, 8,  40, 1, 1,  4,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "water_elemental",},
//{"summon air elemental",     6,20, 6,  40, 1, 1,  5,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "air_elemental",},
//{"dimension door",          10,25, 8,  1, 0, 0,  1,  1, 0, 0, 0,
//0,0,0,0,0,1,0,0,0,0,
// PATH_TELE, "enchantment",},
//{"create earth wall",        4, 6, 12, 30, 0, 0,  6,  1, 1, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_CREATE, "earthwall",},
//{"paralyze",             2, 5, 40, 8, 0, 0,  8,  1, 0, 0, 0,
//0,0,0,0,1,0,0,0,1,0,
// PATH_NULL, "paralyze",},
//{"icestorm",             1, 5, 15, 8, 0, 0,  4,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_FROST, "icestorm",},
//{"magic mapping",        5,15, 20, 1, 2, 8,  5,  0, 0, 0, 1,
//0,0,0,0,0,0,0,1,0,0,
// PATH_INFO, "enchantment",},
//{"turn undead",          1, 2, 40, 5, 0, 0,  8,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_TURNING, "turn_undead",},
//{"fear",             4, 6, 25, 5, 0, 0,  5,  1, 0, 0, 0,
//0,0,0,0,1,0,0,0,1,0,
// PATH_MIND, "fear",},
//{"poison cloud",         2, 5, 30, 10, 0, 0,  6,  1, 0, 0, 0,
//0,1,0,0,1,0,0,0,0,0,
// PATH_MISSILE, "spellball",},
//{"wonder",           3,10, 20, 0, 0, 0,  0,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_TRANSMUTE, "flowers",},
//{"destruction",         18,30,  0, 20, 3, 10, 1,  1, 0, 0, 1,
//0,1,0,0,0,0,0,0,0,0,
// PATH_NULL, "destruction",},
//{"perceive self",        2, 5, 20, 0, 2, 2,  0,  0, 0, 1, 1,
//0,0,0,0,0,0,0,1,0,0,
// PATH_INFO, "enchantment",},
//{"word of recall",          10,40,  3, 50, 1, 2,  1,  0, 0, 1, 1,
//0,0,0,0,0,1,0,1,0,0,
// PATH_TELE, "enchantment",},
//{"invisible",            6,15,  0, 5, 3, 2,  4,  1, 1, 0, 1,
//0,0,0,1,0,0,0,1,0,0,
// PATH_NULL, "enchantment",},
//{"invisible to undead",      6,25,  0, 5, 1, 2,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,0,1,0,0,
// PATH_NULL, "enchantment",},
//{"large bullet",         4, 3, 33, 6, 0, 0,  4,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_MISSILE, "lbullet",},
//{"improved invisibility",    8,25,  0, 10, 1, 1,  1,  1, 1, 0, 1,
//0,0,0,1,0,0,0,1,0,0,
// PATH_NULL, "enchantment",},
//{"holy word",            1, 4,  0, 1, 0, 0,  4,  1, 0, 1, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_TURNING, "holy_word",},
//{"medium healing",       4, 7, 20, 6, 0, 0,  5,  1, 1, 1, 1,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "healing",},
//{"major healing",        8,10, 12, 9, 0, 0,  3,  1, 1, 1, 1,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "healing",},
//{"heal",            10,50,  5, 12, 0, 0,  1,  1, 1, 1, 1,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "healing",},
//{"create food",  6, 10,  0, 20, 0, 0,  4,  1, 1, 1, 0,
//0,0,0,0,0,0,0,1,0,0,
// PATH_CREATE, "food",},
//{"earth to dust",        2, 5,  0, 30, 0, 0,  2,  1, 1, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_NULL, "destruction",},
//{"armour",           1, 8,  0, 20, 3, 2,  8,  1, 1, 0, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_SELF, "enchantment",},
//{"strength",             2,10,  0, 20, 3, 2,  6,  1, 0, 0, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_SELF, "enchantment",},
//{"dexterity",            3,12,  0, 20, 3, 2,  4,  1, 0, 0, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_SELF, "enchantment",},
//{"constitution",         4,15,  0, 20, 3, 2,  4,  1, 1, 0, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_SELF, "enchantment",},
//{"charisma",             3,12,  0, 20, 0, 0,  4,  1, 0, 0, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_SELF, "enchantment",},
//{"create fire wall",         6, 5,  0, 10, 0, 0,  3,  1, 1, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_CREATE, "firebreath",},
//{"create frost wall",        8, 8,  0, 10, 0, 0,  2,  1, 1, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_CREATE, "icestorm",},
//{"protection from cold",     3,15,  0, 10, 1, 1,  3,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from electricity",  4,15,  0, 10, 1, 1,  3,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from fire",     5,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from poison",   6,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from slow",     7,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from paralysis",    8,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from draining",     9,25,  0, 30, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from magic",   10,30,  0, 30, 1, 1,  1,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from attack",  13,50,  0, 50, 1, 1,  1,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"levitate",             6,10,  0, 10, 1, 1,  2,  0, 0, 0, 1,
//0,0,0,1,0,0,0,1,0,0,
// PATH_NULL, "enchantment",},
//{"small speedball",              3, 3,  0, 20, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_MISSILE, "speedball",},
//{"large speedball",      6, 6,  0, 40, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_MISSILE, "speedball",},
//{"hellfire",             8,13,  0, 30, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_FIRE, "hellfire",},
//{"dragonbreath",        12, 13,  0, 30, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_FIRE, "firebreath",},
//{"large icestorm",      12,13,  0, 40, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_FROST, "icestorm",},
//{"charging",            10,200, 0, 75, 1, 1,  0,  0, 0, 0, 1,
//0,0,0,0,0,0,0,1,0,0,
// PATH_TRANSFER, "enchantment",},
//#ifdef NO_POLYMORPH
//{"polymorph",            6,20, 0, 30, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,1,0,
// PATH_TRANSMUTE, "polymorph",},
//#else
//{"polymorph",            6,20, 10, 30, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,1,0,
// PATH_TRANSMUTE, "polymorph",},
//#endif
//{"cancellation",        10,30, 10, 10, 0, 0,  1,  1, 0, 0, 0,
//0,0,0,0,1,0,1,0,0,0,
// PATH_ABJURE, "cancellation",},
//
//  {"mass confusion",         7,20, 15, 20, 0, 0,  3,  1, 0, 0, 0,
//0,0,0,0,1,0,1,0,0,0,
// PATH_MIND, "confuse",},
//{"summon pet monster",       2, 5, 15, 40, 3, 1,  8,  1, 0, 0, 0,
//1,0,0,0,0,0,0,0,0,0,
// PATH_SUMMON, NULL,},
//{"slow",             1, 5, 30, 5, 0, 0,  7,  1, 0, 0, 0,
//0,0,0,0,1,0,1,0,0,0,
// PATH_NULL, "slow",},
//{"regenerate spellpoints",      99, 0,  0, 0, 0, 0,  0,  0, 0, 0, 1,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, NULL,},
//{"protection from confusion",    7,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from cancellation",11,30,  0, 10, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"protection from depletion",    7,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"alchemy",          3, 5,  0, 15, 3, 2,  7,  1, 0, 0, 1,
//0,0,0,0,0,0,0,1,0,0,
// PATH_TRANSMUTE, "enchantment",},
//{"remove curse",         8,80,  0,100, 1, 3,  1,  1, 0, 1, 1,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "protection",},
//{"remove damnation",        15,200, 0,200, 1, 1,  0,  1, 0, 1, 1,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "protection",},
//{"detect monster",       2, 2,  0, 15, 3, 6,  8,  1, 1, 0, 1,
//0,0,0,0,0,0,1,1,0,0,
// PATH_INFO, "detect_magic",},
//{"detect evil",          3, 3,  0, 15, 3, 5,  8,  1, 1, 1, 1,
//0,0,0,0,0,0,1,1,0,0,
// PATH_INFO, "detect_magic",},
//{"heroism",         10,50,  0, 10, 0, 0,  0,  1, 0, 0, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_SELF, "enchantment",},
//{"aggravation",          1, 0,  0, 1, 0, 0,  0,  0, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_NULL, NULL,},
//{"firebolt",             2, 9, 35, 10, 0, 0,  4,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_FIRE, "firebolt",},
//{"frostbolt",            3,12, 30, 10, 0, 0,  3,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_FROST, "frostbolt",},
//{"shockwave",                   14,26,  0, 20, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_NULL, "shockwave",},
//{"color spray",                 13,35,  0, 15, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_NULL, "color_spray",},
//{"haste",                       12,50,  0, 10, 0, 0,  0,  1, 0, 0, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_SELF, "enchantment",},
//{"face of death",               22, 80,  0, 15, 0, 0,  0,  1, 0, 1, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_DEATH, "face_of_death",},
//{"ball lightning",               9,10, 30, 30, 1, 9,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_ELEC, "ball_lightning",},
//{"meteor swarm",                12,30,  0, 30, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_MISSILE, "meteor",},
//{"comet",                        8,15,  0, 20, 0, 0,  0,  1, 0, 0, 0,
//0,1,0,0,0,0,0,0,0,0,
// PATH_MISSILE, NULL,},
//{"mystic fist",                  5,10,  0, 15, 0, 0,  1,  1, 0, 0, 0,
//1,0,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "mystic_fist",},
//{"raise dead",                  10,150,  0, 60, 0, 0,  0,  1, 0, 1, 0,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "enchantment",},
//{"resurrection",                20,250, 0, 180, 0, 0,  0,  0, 0, 1, 0,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "enchantment",},
//{"reincarnation",               25,350, 0,100, 0, 0,  0,  0, 0, 1, 0,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "enchantment",},
//{"immunity to cold",            6, 60,  0, 10, 0, 0,  0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to electricity",     8, 65,  0, 10, 0, 0,  0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to fire",            10,70,  0, 10, 0, 0,  0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to poison",          12,60,  0, 10, 0, 0,  0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to slow",            14,60,  0, 10, 0, 0,  0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to paralysis",        16,60,  0, 10, 0, 0, 0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to draining",         18,75,  0, 10, 0, 0, 0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to magic",           20,150,  0, 30, 0, 0,  0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"immunity to attack",          26,170,  0, 50, 0, 0, 0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"invulnerability",             80,225,  0, 30, 0, 0, 0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_NULL, "protection",},
//{"defense",                     40,75,   0, 30, 0, 0, 0,  1, 1, 1, 1,
//0,0,0,1,0,0,1,1,0,0,
// PATH_PROT, "protection",},
//{"rune of fire",                4,10,    0, 30, 0, 0, 5,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_FIRE, "rune_fire",},
//{"rune of frost",               6,12,    0, 30, 0, 0, 4,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_FROST, "rune_frost",},
//{"rune of shocking",            8,14,    0, 30, 0, 0, 3,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_ELEC, "rune_shock",},
//{"rune of blasting",           10,18,    0, 30, 0, 0, 2,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_DETONATE, "rune_blast",},
//{"rune of death",              17,20,    0, 40, 0, 0, 1,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_DEATH, "rune_death",},
//{"marking rune",                1,2,     0, 10, 0, 0, 5,  0, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_NULL, "rune_mark",},
//{"build director",             10,30,    0, 30, 0, 0, 1,  1, 0, 0, 0,
//1,0,0,0,0,0,0,0,0,0,
// PATH_CREATE, NULL,},
//{"create pool of chaos",       10,10,   10, 15, 0, 0, 1,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_CREATE, "color_spray",},
//{"build bullet wall",          12,35,    0, 35, 0, 0, 1,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_CREATE, NULL,},
//{"build lightning wall",       14,40,    0, 40, 0, 0, 1,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_CREATE, NULL,},
//{"build fireball wall",        16,45,    0, 45, 0, 0, 1,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_CREATE, NULL,},
//{"magic rune",                  12,5,    0, 30, 0, 0, 1,  0, 0, 0, 0,
//1,0,0,0,0,0,0,0,0,0,
// PATH_CREATE, "generic_rune",},
//{"rune of magic drain",        14,30,    0, 30, 0, 0, 0,  1, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_TRANSFER, "rune_drain_magic",},
//{"antimagic rune",              7,5,     0, 20, 0, 0, 1,  1, 0, 0, 0,
//1,0,0,0,0,0,0,0,1,0,
// PATH_ABJURE, "rune_antimagic",},
//{"rune of transferrence",       6,12,    0, 40, 0, 0, 1,  1, 0, 0, 0,
//1,0,0,0,0,0,0,0,1,0,
// PATH_TRANSFER, "rune_transferrence",},
//{"transferrence",               5,10,    0, 20, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_TRANSFER, "enchantment",},
//{"magic drain",                12,20,    0, 1, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,1,0,0,0,1,0,
// PATH_TRANSFER, "enchantment",},
//{"counterspell",                3,10,   20, 0, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,1,0,0,0,1,0,
// PATH_ABJURE, "counterspell",},
//{"disarm",                  4,7,     0, 30, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_ABJURE, "enchantment",},
//{"cure confusion",              7,8,     0, 15, 1, 4, 1,  1, 0, 1, 1,
//0,0,1,0,0,0,1,1,0,0,
//0,0,1,0,0,0,1,1,0,0,
// PATH_RESTORE, "healing",},
//{"summon evil monster",         8,8,     0, 30, 0, 0, 0,  0, 0, 0, 0,
//1,1,0,0,0,0,0,0,0,0,
// PATH_SUMMON, NULL,},
//{"counterwall",                 8, 8,   30, 30, 0, 0, 1,  1, 0, 0, 0,
//1,0,0,0,0,0,0,0,1,0,
// PATH_RESTORE, "counterspell",},
//{"cause medium wounds",     3, 8,    0,  5, 0, 0, 2,  1, 0, 1, 0,
//0,1,0,0,0,0,0,0,0,0,
//PATH_WOUNDING,"cause_wounds",},
//{"cause serious wounds",    5, 16,   0,  5, 0, 0, 2,  1, 0, 1, 0,
//0,1,0,0,0,0,0,0,0,0,
//PATH_WOUNDING,"cause_wounds",},
//{"charm monsters",      5, 20,    0, 10, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,1,0,
//PATH_MIND,"enchantment",},
//{"banishment",          5, 10,   3, 10, 1, 1, 1,  1, 0, 1, 0,
//0,1,0,0,0,0,0,0,0,0,
//PATH_TURNING,"banishment",},
//{"create missile",      1,5,    0, 20, 1, 1, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,1,0,0,
//PATH_CREATE,"enchantment",},
//{"show invisible",      7,10,   4, 20, 1, 1, 1,  1, 1, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//PATH_INFO,"enchantment",},
//{"xray",            10,20,  0, 20, 1, 1, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//PATH_INFO,"enchantment",},
//{"pacify",          4, 10,  1, 2, 0, 0, 3,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//PATH_MIND,"enchantment",},
//{"summon fog",                   2, 5,  10, 10, 0, 0, 2,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//PATH_CREATE,"fog",},
//{"steambolt",                    5, 10, 10, 10, 0, 0,  1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//PATH_FIRE, "steambolt",},
//{"command undead",               4, 12,  0, 10, 0, 0, 3,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//PATH_MIND,"enchantment",},
//{"holy orb",                    7, 12,  0, 5, 0, 0, 3,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//PATH_TURNING,"holy_orb",},
//{"summon avatar",               10, 60, 0, 15, 0, 0, 1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "avatar",},
//{"holy possession",             9, 30,  0, 10, 0, 0, 1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_ABJURE, "enchantment",},
//{"bless",                        2, 8,  0, 5, 0, 0,  3,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_ABJURE, "enchantment",},
//{"curse",                        2, 8,  0, 5, 0, 0,  2,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_ABJURE, "enchantment",},
//{"regeneration",                 7, 15,  0, 10, 0, 0, 1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_ABJURE, "enchantment",},
//{"consecrate",                    4, 35,  0, 50, 0, 0, 1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_ABJURE, "enchantment",},
//{"summon cult monsters",          3, 12,  0, 10, 0, 0, 2,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_SUMMON, NULL,},
//{"cause critical wounds",     7, 25,   0,  5, 0, 0, 0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"cause_wounds",},
//{"holy wrath",            14, 40,   0,  5, 0, 0, 1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_TURNING,"holy_wrath",},
//{"retributive strike",        26, 150,   0, 15, 0, 0, 0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"god_power",},
//{"finger of death",       15, 50,   0,  5, 0, 0, 0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_DEATH, NULL,},
//{"insect plague",         12, 40,   0,  5, 0, 0, 0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_SUMMON,"insect_plague",},
//{"call holy servant",         5, 30,   0,  5, 0, 0, 3,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_SUMMON, "holy_servant",},
//{"wall of thorns",        6, 20,   0, 5, 0, 0, 0,  0, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_CREATE, "thorns"},
//{"staff to snake",        2, 8,   0, 5, 0, 0, 1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_CREATE, "snake_golem"},
//{"light",                   1, 8, 20, 5, 0, 0, 3,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_LIGHT, "light"},
//{"darkness",                5, 15, 10, 5, 0, 0, 1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_LIGHT, "darkness"},
//{"nightfall",               16, 120,  0, 15, 0, 0, 0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_LIGHT, NULL},
//{"daylight",                18, 120,  0, 15, 0, 0, 0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//  PATH_LIGHT, NULL},
//{"sunspear",                    6, 8, 35, 8, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_LIGHT, "sunspear"},
//{"faery fire",                  4, 10,  0, 15, 3, 2, 2,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_LIGHT, NULL},
//{"cure blindness",              9, 30,  0, 10, 1, 1, 2,  1, 1, 1, 1,
//0,0,0,0,0,0,0,0,0,0,
// PATH_RESTORE, "healing",},
//{"dark vision",                 5, 10,  0, 12, 3, 2, 2,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_INFO, NULL},
//{"bullet swarm",        7,  6,  0,  5, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_MISSILE,"bullet"},
//{"bullet storm",           10,  8,  0,  5, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_MISSILE,"lbullet"},
//{"cause many wounds",          12,  30,  0,  5, 0, 0, 0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_WOUNDING,"cause_wounds"},
//{"small snowstorm",              1, 6, 40, 5, 0, 0,  8,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_FROST, "snowball_s",},
//{"medium snowstorm",             3,10, 20, 10, 0, 0,  6,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_FROST, "snowball_m",},
//{"large snowstorm",              5,16, 10, 15, 0, 0,  2,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_FROST, "snowball_l",},
//{"cause red death",           12,100, 0, 10, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"ebola"},
//{"cause flu",                  2, 10, 5, 10, 3, 2,  5,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"flu"},
//{"cause black death",         15, 120, 0, 10, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_NULL,"bubonic_plague"},
//{"cause leprosy",              5, 20, 0, 10, 1, 1,  5,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"leprosy"},
//{"cause smallpox",            10, 85, 0, 10, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"smallpox"},
//{"cause white death",              85,350, 0, 10, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"pneumonic_plague"},
//{"cause anthrax",             12, 50, 0, 10, 1, 1,  1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"anthrax"},
//{"cause typhoid",             8, 60, 0, 10, 1, 1,  1,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_WOUNDING,"typhoid"},
//{"mana blast",             8, 10, 0, 15, 0, 0,  2,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_TRANSFER, "manablast", },
//{"small manaball",         4, 12, 0,  9, 0, 0,  3,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_TRANSFER, "manabullet_s", },
//{"medium manaball",        7, 20, 0, 18, 0, 0,  2,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_TRANSFER, "manabullet_m", },
//{"large manaball",        10, 32, 0, 27, 0, 0,  1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_TRANSFER, "manabullet_l", },
//{"mana bolt",              5, 18, 0,  9, 0, 0,  2,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_TRANSFER, "manabolt", },
//{"dancing sword",         11, 25, 0, 10, 0, 0,  1,  0, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_CREATE, "dancingsword", },
//{"animate weapon",         7, 25, 0, 10, 0, 0,  4,  0, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_TELE, "dancingsword", },
//{"cause cold",                  2, 10, 5, 10, 3, 2,  5,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_WOUNDING,"disease_cold"},
//{"divine shock",              1, 3, 0, 10, 0, 0, 0, 1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_WOUNDING,"divine_shock"},
//{"windstorm",                   3,3,  0, 10, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_NULL, "windstorm",},
//{"sanctuary",                 7, 30,  0, 10,  0,  0,  0,  0,  1,  1,  1,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_PROT,"sanctuary"},
//{"peace",                 20, 80,  0, 10,  0,  0,  0,  1,  0,  1,  0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_PROT,"peace"},
//{"spiderweb",              4, 10,  0, 10,  0,  0,  0,  1,  0,  1,  0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_CREATE,"spiderweb_spell"},
//{"conflict",              10, 50,  0, 10,  0,  0,  0,  1,  0,  1,  0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_MIND, "enchantment"},
//{"rage",                   1,  5,  0, 10,  0,  0,  0,  1,  0,  1,  1,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_WOUNDING, "enchantment"},
//{"forked lightning",       5, 15,  0, 10,  0,  0,  0,  1,  0,  1,  0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_ELEC, "forked_lightning"},
//{"poison fog",             5, 15,  0, 10,  0,  0,  0,  1,  0,  1,  0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_WOUNDING, "poison_fog"},
//{"flaming aura",           1,  5,  0, 10,  0,  0,  0,  0,  1,  1,  1,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_FIRE, "flaming_aura"},
//{"vitriol",                5, 15,  0, 10,  0,  0,  0,  0,  1,  1,  1,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_DETONATE, "vitriol"},
//{"vitriol splash",                5, 15,  0, 10,  0,  0,  0,  0,  1,  1,  1,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_DETONATE, "vitriol_splash"},
//{"ironwood skin",    1, 8,  0, 20, 0, 0,  0,  1, 1, 1, 1,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_SELF, "enchantment",},
//{"wrathful eye",     5, 30,  0, 20, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_SELF, "wrathful_eye",},
//{"town portal",         8, 30, 0, 10, 0, 0,  1,  0, 0, 0, 1,
//0,0,0,0,0,0,0,0,0,0,
// PATH_TELE, "perm_magic_portal",},
//{"missile swarm",       7,  6,  0,  3, 0, 0, 1,  1, 0, 0, 0,
//0,0,0,0,0,0,0,0,0,0,
//   PATH_MISSILE,"magic_missile"},
//{"cause rabies",            12, 120, 0, 10, 0, 0,  0,  1, 0, 1, 0,
//0,0,0,0,0,0,0,0,0,0,
// PATH_WOUNDING,"rabies"}
//};
#endif

const char   *spellpathnames[NRSPELLPATHS]    =
{
    "Life", "Death", "Elemental", "Energy",
    "Spirit", "Protection", "Light", "Nether",
    "Nature", "Shadow", "Chaos", "Earth",
    "Conjuration", "Abjuration", "Transmutation", "Arcane"
};

static int  cardinal_adjust[9]  =
{
    -3, -2, -1, 0, 0, 0, 1, 2, 3
};
static int  diagonal_adjust[10] =
{
    -3, -2, -2, -1, 0, 0, 1, 2, 2, 3
};

archetype_t  *spellarch[NROFREALSPELLS];

static int   CheckSpellInstance(msp_t *msp, object_t *op);
static sint8 Disallowed(msp_t *msp, object_t *who, int nr);

void init_spells()
{
    static int  init_spells_done    = 0;
    int         i;

    if (init_spells_done)
        return;
    LOG(llevDebug, "Initializing spells...");
    init_spells_done = 1;
    for (i = 0; i < NROFREALSPELLS; i++)
        if (spells[i].archname)
        {
            if ((spellarch[i] = find_archetype(spells[i].archname)) == NULL)
                LOG(llevError, "Error: Spell %s needs arch %s, your archetype file is out of date.\n", spells[i].name,
                    spells[i].archname);
        }
        else
            spellarch[i] = (archetype_t *) NULL;
    LOG(llevDebug, "done.\n");
}

void dump_spells()
{
    int i;

    for (i = 0; i < NROFREALSPELLS; i++)
    {
        const char*name1 =  NULL, *name2 = NULL;
        if (spellarch[i])
        {
            name1 = spellarch[i]->name;

            if (spellarch[i]->clone.other_arch)
                name2 = spellarch[i]->clone.other_arch->name;
        }
        LOG(llevInfo, "%s: %s: %s\n", STRING_SAFE(spells[i].name), STRING_SAFE(name1), STRING_SAFE(name2));
    }
}

/* this must be adjusted if we ever include multi tile effects! */
int insert_spell_effect(char *archname, map_t *m, int x, int y)
{
    archetype_t  *effect_arch;
    object_t     *effect_ob;

    if (!archname || !m)
    {
        LOG(llevBug, "BUG: insert_spell_effect: archname or map NULL.\n");
        return 1;
    }

    if (!(effect_arch = find_archetype(archname)))
    {
        LOG(llevBug, "BUG: insert_spell_effect: Couldn't find effect arch (%s).\n", archname);
        return 1;
    }

    /* prepare effect */
    effect_ob = arch_to_object(effect_arch);
    effect_ob->map = m;
    effect_ob->x = x;
    effect_ob->y = y;

    if (!insert_ob_in_map(effect_ob, m, NULL, 0))
    {
        LOG(llevBug, "BUG: insert_spell_effect: effect arch (%s) out of map (%s) (%d,%d) or failed insertion.\n",
            archname, effect_ob->map->name, x, y);
        /* something is wrong - kill object_t */
        if (!QUERY_FLAG(effect_ob, FLAG_REMOVED))
        {
            remove_ob(effect_ob);
            check_walk_off(effect_ob, NULL, MOVE_APPLY_VANISHED);
        }
        return 1;
    }


    return 0;
}

spell * find_spell(int spelltype)
{
    if (spelltype<0 || spelltype>NROFREALSPELLS)
        return NULL;
    return &spells[spelltype];
}

int check_spell_known(object_t *op, int spell_type)
{
    int i;
    for (i = 0; i < (int) CONTR(op)->nrofknownspells; i++)
        if (CONTR(op)->known_spells[i] == spell_type)
            return 1;
    return 0;
}


/*
 * cast_spell():
 * Fires spell "type" in direction "dir".
 * If "ability" is true, the spell is the innate ability of a monster.
 * (ie, don't check for MSP_FLAG_NO_SPELLS and don't add AT_MAGIC to attacktype).
 *
 * op is the creature that is owner of the object that is casting the spell
 * caster is the actual object (wand, potion) casting the spell. can be
 *    same as op.
 * dir is the direction to cast in.
 * ability is true if it is an ability casting the spell.  These can be
 *    cast in no magic areas.
 * item is the type of object that is casting the spell.
 * stringarg is any options that are being used.
 *
 * returns true for successful casting
 */

/* Oct 95 - added cosmetic touches for MULTIPLE_GODS hack -b.t. */

int cast_spell(object_t *op, object_t *caster, int dir, int type, int ability, SpellTypeFrom item, char *stringarg)
{
    spell          *s;
    const char     *godname = NULL;
    object_t         *target = NULL,
                   *cast_op,
                   *guild;
    int             success = 0,
                    duration,
                    points_used = 0;
    rv_t       rv;
    player_t         *pl = NULL;

    if (!(s = find_spell(type)))
    {
        LOG(llevBug, "BUG:: %s:cast_spell(): Unknown spell: %d from: %s (%s)\n",
            __FILE__, type, STRING_OBJ_NAME(op), STRING_OBJ_NAME(caster));
        return 0;
    }

    if (!op)
    {
        op = caster;
    }

    if (op->type == PLAYER)
    {
        if (!(pl = CONTR(op)))
        {
            return 0;
        }

        guild = pl->guild_force;
    }

    /* TODO: This cast_op v op v caster stuff needs tidying (whole func does). */
    /* script NPC can ALWAYS cast - even in no spell areas! */
    if (item == spellNPC)
    {
        target = op; /* if spellNPC, this comes useally from a script */
        op = caster; /* and caster is the NPC and op the target */
        goto dirty_jump; /* change the pointers to fit this function and jump */
    }

    /* It looks like the only properties we ever care about from the casting
    * object (caster) is spell paths and level.
    */
    cast_op = op;
    if (!caster)
    {
        if (item == spellNormal)
            caster = op;
    }
    else
    {
        if (caster->map) /* caster has a map? then we use caster */
            cast_op = caster;
    }

    if (Disallowed(MSP_KNOWN(cast_op), cast_op, type))
    {
        return 0;
    }

    if (pl &&
        !(pl->gmaster_mode & GMASTER_MODE_SA))
    {
        if (guild && !item)
        {
            if ((guild->weight_limit & GUILD_NO_MAGIC) &&
                !(spells[type].flags & SPELL_DESC_WIS))
            {
                ndi(NDI_UNIQUE, 0, op, "Your Guild membership prevents casting spells!");

                return 0;
            }
            else if ((guild->weight_limit & GUILD_NO_PRAYER) &&
                     (spells[type].flags & SPELL_DESC_WIS))
            {
                ndi(NDI_UNIQUE, 0, op, "Your Guild membership prevents casting prayers!");

                return 0;
            }
        }

        /* ok... its item == spellNPC then op is the target of this spell  */
        pl->rest_mode = 0;

        /* cancel player spells which are denied - only real spells (not potion, wands, ...) */
        if (item == spellNormal)
        {
            if (caster->path_denied & s->path)
            {
                ndi(NDI_UNIQUE, 0, op, "It is denied for you to cast that spell.");
                return 0;
            }

            if (guild &&
                ((guild->level) ? guild->level < s->level : s->level > 0))
            {
                ndi(NDI_UNIQUE, 0, op, "That spell is too difficult for you to cast. %d %d",
                    (guild->level) ? guild->level : 255, s->level);
                return 0;
            }

            if (!(spells[type].flags & SPELL_DESC_WIS))
            {
                if (op->stats.sp < (points_used = SP_level_spellpoint_cost(op, caster, type)))
                {
                    ndi(NDI_UNIQUE, 0, op, "You don't have enough mana.");
                    return 0;
                }
            }
            else
            {
                if (op->stats.grace < (points_used = SP_level_spellpoint_cost(op, caster, type)))
                {
                    ndi(NDI_UNIQUE, 0, op, "You don't have enough grace.");
                    return 0;
                }
            }

            /* If it is an ability, assume that the designer of the archetype knows what they are doing.*/
            if (!ability &&
                SK_level(caster) < s->level)
            {
                ndi(NDI_UNIQUE, 0, op, "You lack enough skill to cast that spell.");
                return 0;
            }
        }

        /* if it a prayer, grab the players god - if we have non, we can't cast - except potions */
        if ((spells[type].flags & SPELL_DESC_WIS) &&
            item != spellPotion &&
            (godname = determine_god(op)) == shstr_cons.none)
        {
            ndi(NDI_UNIQUE, 0, op, "You need a deity to cast a prayer!");
            return 0;
        }
    }

    /* ok.... now we are sure we are able to cast.
     * perhaps some else happens but first we look for
     * a valid target.
     */
    if (item == spellPotion) /* applying potions always go in the applier itself (aka drink or break) */
    {
        /*  if the potion casts an onself spell, don't use the facing direction (given by apply.c)*/
        if (spells[type].flags & SPELL_DESC_SELF)
        {
            target = op;
            dir = 0;
        }
    }
    else if (find_target_for_spell(op, caster, &target, dir, spells[type].flags) == FALSE)
    {
        /* little trick - if we fail we set target== NULL - thats mark its "yourself" */
        if (pl)
        {
            ndi(NDI_UNIQUE, 0, op, "You can't cast this spell on %s!",
                (target) ? QUERY_SHORT_NAME(target, op) : "yourself");
        }

        return 0;
    }

    /* LOG(llevInfo,"TARGET: op: %s target: %s\n", STRING_OBJ_NAME(op), STRING_OBJ_NAME(target)); */

    if (target)
    {
        /* tell player his spell is redirect to himself */
        if (target == op)
        {
            if (pl)
            {
                ndi(NDI_UNIQUE, 0, op, "You auto-target yourself!");
            }
        }
        else
        {
            /* if valid target is not in range for selected spell, skip here casting */
            if (!get_rangevector(op, target, &rv, RV_DIAGONAL_DISTANCE) ||
                rv.distance > (unsigned int)spells[type].range)
            {
                if (pl)
                {
                    ndi(NDI_UNIQUE, 0, op, "Your target is out of range!");
                }
                /*
                else
                    LOG(llevInfo,"cast_spell: %s out of range for %s from %s", STRING_OBJ_NAME(target), spells[type].name, STRING_OBJ_NAME(op));
                */

                return 0;
            }

            if (Disallowed(MSP_KNOWN(target), op, type))
            {
                return 0;
            }
        }
    }

    /* chance to fumble the spell by to low wisdom */
    /* FIXME: we have now spell_fumble */
    if (item == spellNormal && op->type == PLAYER && s->flags & SPELL_DESC_WIS
            && random_roll(0, 99) < s->level / (float) MAX(1, op->chosen_skill->level) * 1)
    {
        play_sound_player_only(CONTR(op), SOUND_FUMBLE_SPELL, SOUND_NORMAL, 0, 0);
        ndi(NDI_UNIQUE, 0, op, "You fumble the prayer because your wisdom is low.");

        if (s->sp == 0) /* Shouldn't happen... */
            return 0;
        return(random_roll(1, SP_level_spellpoint_cost(op, caster, type)));
    }

    else if (item == spellNormal && op->type == PLAYER && !(s->flags & SPELL_DESC_WIS))
    {
        int fumble = CONTR(op)->spell_fumble
            - (op->chosen_skill->level / 10)
            + s->fumble_factor;

        if (random_roll(0,99) < fumble)
        {
            ndi(NDI_UNIQUE, 0, op, "You bungle the spell because you have too much heavy equipment in use.");
            return(random_roll(0, SP_level_spellpoint_cost(op, caster, type)));
        }
    }


    dirty_jump:
    /* a last sanity check: are caster and target *really* valid? */
    if ((caster && !OBJECT_ACTIVE(caster)) || (target && !OBJECT_ACTIVE(target)))
        return 0;

    if (op == caster) // means cast spell/prayer not wand, etc.
    {
        if (target &&
            op != target &&
            target->type == PLAYER)
        {
            ndi(NDI_UNIQUE, 0, target, "%s %ss the %s ~%s~ on you.",
                QUERY_SHORT_NAME(op, target),
                (spells[type].type == SPELL_TYPE_PRIEST) ? "invoke" : "cast",
                (spells[type].type == SPELL_TYPE_PRIEST) ? "prayer" : "spell",
                spells[type].name);
        }
    }

    switch ((enum spellnrs) type)
    {
/* TODO: Reference only. Will be removed. */
#if 0
//        case SP_RESTORATION:
//        case SP_HEAL:
//        case SP_MED_HEAL:
//        case SP_MAJOR_HEAL:
//        case SP_CURE_CONFUSION:
//        case SP_CURE_BLINDNESS:
#endif
        case SP_MINOR_HEAL:
        case SP_CURE_POISON:
        case SP_CURE_DISEASE:
        case SP_REMOVE_DEPLETION:
        case SP_REMOVE_DEATHSICK:
        case SP_RESTORATION:
        case SP_REMOVE_SLOW:
        case SP_REMOVE_FEAR:
        case SP_REMOVE_SNARE:
        case SP_REMOVE_PARALYZE:
        case SP_REMOVE_CONFUSED:
        case SP_REMOVE_BLIND:
          success = cast_heal(op, casting_level(caster, type), target, type);
          break;

        case SP_REMOVE_CURSE:
        case SP_REMOVE_DAMNATION:
          success = remove_curse(op, target, type, item);
          break;

        case SP_STRENGTH:
          success = cast_change_attr(op, caster, target, dir, type);
          break;

        case SP_DETECT_MAGIC:
        case SP_DETECT_CURSE:
          success = cast_detection(op, target, type);
          break;

        case SP_IDENTIFY:
          success = cast_identify(target, casting_level(caster, type), NULL, IDENTIFY_MODE_NORMAL);
          break;

          /* spells after this use direction and not a target */
        case SP_ICESTORM:
        case SP_FIRESTORM:
          duration = spells[type].bdur;  /*  get the base duration */
          success = cast_cone(op, caster, dir, duration, type, spellarch[type], casting_level(caster, type), !ability);
          break;

        case SP_BULLET:
        case SP_CAUSE_LIGHT:
        case SP_PROBE:
          duration = spells[type].bdur;  /*  get the base duration */
          success = fire_arch(op, caster,op->x, op->y, dir, spellarch[type], type, casting_level(caster, type), 1);
          break;

        case SP_FROSTBOLT:
        case SP_FIREBOLT:
        case SP_S_LIGHTNING:
            success = fire_bolt(op,caster,dir,type,!ability);
        break;

        default:
          LOG(llevBug, "BUG: cast_spell() has invalid spell nr. %d\n", type);
          break;
/* TODO: Reference only. Will be removed. */
#if 0
//        case SP_CAUSE_HEAVY:
//        case SP_CAUSE_MEDIUM:
//        case SP_CAUSE_CRITICAL:
//        case SP_LARGE_BULLET:
//        case SP_BULLET:
//        case SP_LARGE_BULLET:
//          success = fire_arch(op,op->x, op->y,caster,dir,spellarch[type],type,op->level, 1);
//          break;
//        case SP_HOLY_ORB:
//          success = fire_arch(op,caster,op->x, op->y,dir,spellarch[type],type,op->level, 0);
//          break;
//        case SP_S_FIREBALL:
//        case SP_M_FIREBALL:
//        case SP_L_FIREBALL:
//        case SP_S_SNOWSTORM:
//        case SP_M_SNOWSTORM:
//        case SP_L_SNOWSTORM:
//        case SP_HELLFIRE:
//        case SP_POISON_CLOUD:
//        case SP_M_MISSILE:
//        case SP_S_MANABALL:
//        case SP_M_MANABALL:
//        case SP_L_MANABALL:
//        case SP_PROBE:
//          success = fire_arch(op,caster,op->x, op->y,dir,spellarch[type],type, op->level, !ability);
//          break;
//        case SP_VITRIOL:
//          success = fire_arch(op,caster,op->x, op->y,dir,spellarch[type],type, op->level, 0);
//          break;
//        case SP_MASS_CONFUSION:
//        case SP_SHOCKWAVE:
//        case SP_COLOR_SPRAY:
//        case SP_FACE_OF_DEATH:
//        case SP_COUNTER_SPELL:
//        case SP_BURNING_HANDS: <--- done aka firestorm
//        case SP_PARALYZE:
//        case SP_SLOW:
//        case SP_ICESTORM:
//        case SP_FIREBREATH:
//        case SP_LARGE_ICESTORM:
//        case SP_BANISHMENT:
//        case SP_MANA_BLAST:
//        case SP_WINDSTORM:
//        case SP_PEACE:
//        case SP_SPIDERWEB:
//        case SP_VITRIOL_SPLASH:
//        case SP_WRATHFUL_EYE:
//          success = cast_cone(op,caster,dir,duration,type,spellarch[type],spell_level, !ability);
//          break;
//        case SP_TURN_UNDEAD:
//          if(QUERY_FLAG(op,FLAG_UNDEAD)) { // the undead *don't* cast this
//             ndi(NDI_UNIQUE, 0,op,"Your undead nature prevents you from turning undead!");
//             success=0; break;
//          }
//        case SP_HOLY_WORD:
//          success = cast_cone(op,caster,dir,duration+(turn_bonus[op->stats.Wis]/5),type,
//        spellarch[type],spell_level, 0);
//          break;
//        case SP_HOLY_WRATH:
//        case SP_INSECT_PLAGUE:
//        case SP_RETRIBUTION:
//          success = cast_smite_spell(op,caster,dir,type);
//          break;
//        case SP_SUNSPEAR:
//        case SP_FIREBOLT:
//        case SP_FROSTBOLT:
//        case SP_S_LIGHTNING:
//        case SP_L_LIGHTNING:
//        case SP_FORKED_LIGHTNING:
//        case SP_STEAMBOLT:
//        case SP_MANA_BOLT:
//          success = fire_bolt(op,caster,dir,type,!ability);
//          break;
//        case SP_BOMB:
//          success = create_bomb(op,caster,dir,type,"bomb");
//          break;
//        case SP_GOLEM:
//        case SP_FIRE_ELEM:
//        case SP_WATER_ELEM:
//        case SP_EARTH_ELEM:
//        case SP_AIR_ELEM:
//          success = summon_monster(op,caster,dir,spellarch[type],type);
//          break;
//        case SP_FINGER_DEATH:
//          success = finger_of_death(op,caster,dir);
//          break;
//        case SP_SUMMON_AVATAR:
//        case SP_HOLY_SERVANT: {
//          archetype_t *spat = find_archetype((type==SP_SUMMON_AVATAR)?"avatar":"holy_servant");
//          success = summon_avatar(op,caster,dir,spat,type);
//          break; }
//        case SP_CONSECRATE:
//          success = cast_consecrate(op);
//          break;
//        case SP_SUMMON_CULT:
//          success = summon_cult_monsters(op,dir);
//          break;
//        case SP_PET:
//          success = summon_pet(op,dir, item);
//          break;
//        case SP_D_DOOR:
//          // dimension door needs the actual caster, because that is what is moved.
// 
//          success = dimension_door(op,dir);
//          break;
//        case SP_DARKNESS:
//        case SP_WALL_OF_THORNS:
//        case SP_CHAOS_POOL:
//        case SP_COUNTERWALL:
//        case SP_FIRE_WALL:
//        case SP_FROST_WALL:
//        case SP_EARTH_WALL:
//          success = magic_wall(op,caster,dir,type);
//          break;
//        case SP_MAGIC_MAPPING:
//          break;
//        case SP_FEAR:
//          if(op->type!=PLAYER)
//            bonus=caster->head==NULL?caster->level/3+1:caster->head->level/3+1;
//          success = cast_cone(op,caster,dir,duration+bonus,SP_FEAR,spellarch[type],spell_level, !ability);
//          break;
//        case SP_WOW:
//          success = cast_wow(op,dir,ability, item);
//          break;
//        case SP_DESTRUCTION:
//          success = cast_destruction(op,caster,5+op->stats.Int,AT_MAGIC);
//          break;
//        case SP_PERCEIVE:
//          success = perceive_self(op);
//          break;
//        case SP_WOR:
//          success = cast_wor(op,caster);
//          break;
//        case SP_INVIS:
//        case SP_INVIS_UNDEAD:
//        case SP_IMPROVED_INVIS:
//          success = cast_invisible(op,caster,type);
//          break;
//        case SP_CREATE_FOOD:
//          success = cast_create_food(op,caster,dir,stringarg);
//          break;
//        case SP_EARTH_DUST:
//          success = cast_earth2dust(op,caster);
//          break;
//        case SP_REGENERATION:
//        case SP_BLESS:
//        case SP_CURSE:
//        case SP_HOLY_POSSESSION:
//        case SP_DEXTERITY:
//        case SP_CONSTITUTION:
//        case SP_CHARISMA:
//        case SP_ARMOUR:
//        case SP_IRONWOOD_SKIN:
//        case SP_PROT_COLD:
//        case SP_PROT_FIRE:
//        case SP_PROT_ELEC:
//        case SP_PROT_POISON:
//        case SP_PROT_SLOW:
//        case SP_PROT_DRAIN:
//        case SP_PROT_PARALYZE:
//        case SP_PROT_ATTACK:
//        case SP_PROT_MAGIC:
//        case SP_PROT_CONFUSE:
//        case SP_PROT_CANCEL:
//        case SP_PROT_DEPLETE:
//        case SP_LEVITATE:
//        case SP_HEROISM:
//        case SP_CONFUSION:
//        case SP_XRAY:
//        case SP_DARK_VISION:
//        case SP_RAGE:
//          success = cast_change_attr(op,caster,dir,type);
//          break;
//        case SP_REGENERATE_SPELLPOINTS:
//          success = cast_regenerate_spellpoints(op);
//          break;
//        case SP_SMALL_SPEEDBALL:
//        case SP_LARGE_SPEEDBALL:
//          success = cast_speedball(op,dir,type);
//          break;
//        case SP_POLYMORPH:
//      #ifdef NO_POLYMORPH
//          // Not great, but at least provide feedback so if players do have
//           // polymorph (ie, find it as a preset item or left over from before
//           //it was disabled), they get some feedback.
// 
//          ndi(NDI_UNIQUE, 0,op,"The spell fizzles");
//          success = 0;
//      #else
//          success = cast_polymorph(op,dir);
//      #endif
//          break;
//        case SP_CHARGING:
//          success = recharge(op);
//          break;
//        case SP_CANCELLATION:
//          success = fire_cancellation(op,dir,spellarch[type],!ability);
//          break;
//        case SP_ALCHEMY:
//          success = alchemy(op);
//          break;
// 
//        case SP_DETECT_MONSTER:
//        case SP_DETECT_EVIL:
//        case SP_SHOW_INVIS:
//          success = cast_detection(op, type);
//          break;
//        case SP_AGGRAVATION:
//          aggravate_monsters(op);
//          success = 1;
//          break;
//        case SP_BALL_LIGHTNING:
//        case SP_DIVINE_SHOCK:
//        case SP_POISON_FOG:
//          success = fire_arch(op,caster,op->x, op->y,dir,spellarch[type],type,op->level, !ability);
//          break;
//        case SP_METEOR_SWARM: {
//          success = 1;
//          fire_swarm(op, caster, dir, spellarch[type], SP_METEOR,
//        random_roll_roll(3, 3) +
//            SP_level_strength_adjust(op,caster, type), 0);
//          break;
//        }
//        case SP_BULLET_SWARM: {
//          success = 1;
//          fire_swarm(op, caster, dir, spellarch[type], SP_BULLET,
//        random_roll_roll(3, 3) +
//            SP_level_strength_adjust(op,caster, type), 0);
//          break;
//        }
//        case SP_BULLET_STORM: {
//          success = 1;
//          fire_swarm(op, caster, dir, spellarch[type], SP_LARGE_BULLET,
//        random_roll_roll(3, 3) +
//            SP_level_strength_adjust(op,caster, type), 0);
//          break;
//        }
//        case SP_CAUSE_MANY: {
//          success = 1;
//          fire_swarm(op, caster, dir, spellarch[type], SP_CAUSE_HEAVY,
//        random_roll_roll(3, 3) +
//            SP_level_strength_adjust(op,caster, type), 0);
//          break;
//        }
//        case SP_METEOR:
//          success = fire_arch(op,caster,op->x, op->y,dir,find_archetype("meteor"),type,op->level, 0);
//          break;
//        case SP_MYSTIC_FIST:
//          success = summon_monster(op,caster,dir,spellarch[type],type);
//          break;
//        case SP_RAISE_DEAD:
//        case SP_RESURRECTION:
//          success = cast_raise_dead_spell(op,dir,type, NULL);
//          break;
//        case SP_IMMUNE_COLD:
//        case SP_IMMUNE_FIRE:
//        case SP_IMMUNE_ELEC:
//        case SP_IMMUNE_POISON:
//        case SP_IMMUNE_SLOW:
//        case SP_IMMUNE_DRAIN:
//        case SP_IMMUNE_PARALYZE:
//        case SP_IMMUNE_ATTACK:
//        case SP_IMMUNE_MAGIC:
//        case SP_INVULNERABILITY:
//        case SP_PROTECTION:
//        case SP_HASTE:
//          success=cast_change_attr(op,caster,dir,type);
//          break;
// 
//        case SP_BUILD_DIRECTOR:
//        case SP_BUILD_BWALL:
//        case SP_BUILD_LWALL:
//        case SP_BUILD_FWALL:
//          success=create_the_feature(op,caster,dir,type);
//          break;
//        case SP_RUNE_FIRE:
//        case SP_RUNE_FROST:
//        case SP_RUNE_SHOCK:
//        case SP_RUNE_BLAST:
//        case SP_RUNE_DEATH:
//        case SP_RUNE_ANTIMAGIC:
//          success = write_rune(op,dir,0,caster->level,s->archname);
//          break;
//        case SP_RUNE_DRAINSP:
//          success = write_rune(op,dir,SP_MAGIC_DRAIN,caster->level,s->archname);
//          break;
//        case SP_RUNE_TRANSFER:
//          success= write_rune(op,dir,SP_TRANSFER,caster->level,s->archname);
//          break;
//        case SP_TRANSFER:
//          success = cast_transfer(op,dir);
//          break;
//        case SP_MAGIC_DRAIN:
//          success= drain_magic(op,dir);
//          break;
//        case SP_DISPEL_RUNE:
//          success = dispel_rune(op,dir,0);  // 0 means no risk of detonating rune
//          break;
//        case SP_SUMMON_EVIL_MONST:
//        if(op->type==PLAYER) return 0;
//          success = summon_hostile_monsters(op,op->stats.maxhp,op->race);
//          break;
// 
//        case SP_REINCARNATION:
//          {
//            object_t * dummy;
//            if(stringarg==NULL) {
//        ndi(NDI_UNIQUE, 0,op,"Reincarnate WHO?");
//        success=0;
//        break;
//            }
//            dummy = get_object();
//            FREE_AND_COPY_HASH(dummy->name, stringarg);
//            success = cast_raise_dead_spell(op,dir,type, dummy);
//          }
//          break;
//        case SP_RUNE_MAGIC:
//          { int total_sp_cost, spellinrune;
//            spellinrune=look_up_spell_by_name(op,stringarg);
//            if(spellinrune!=-1) {
//            total_sp_cost=SP_level_spellpoint_cost(op,caster,spellinrune)
//                +spells[spellinrune].sp;
//        if(op->stats.sp<total_sp_cost) {
//          ndi(NDI_UNIQUE, 0,op,"Not enough spellpoints.");
//          return 0;
//        }
//        success=write_rune(op,dir,spellinrune,caster->level,stringarg);
//        return (success ? total_sp_cost : 0);
//            }
//            return 0;
//          }
//          break;
//        case SP_RUNE_MARK:
//          if(caster->type == PLAYER)
//            success=write_rune(op,dir,0,-2,stringarg);
//          else success= 0;
// 
//          break;
//        case SP_LIGHT:
//          success = cast_light(op,caster,dir);
//          break;
//        case SP_DAYLIGHT:
//          success = cast_daylight(op);
//          break;
//        case SP_NIGHTFALL:
//          success = cast_nightfall(op);
//          break;
//        case SP_FAERY_FIRE:
//          success = cast_faery_fire(op,caster);
//          break;
//        case SP_SUMMON_FOG:
//        success = summon_fog(op,caster,dir,type);
//        break;
//        case SP_PACIFY:
//        cast_pacify(op,caster,spellarch[type],type);
//        success = 1;
//            break;
//        case SP_COMMAND_UNDEAD:
//        cast_charm_undead(op,caster,spellarch[type],type);
//        success = 1;
//            break;
//        case SP_CHARM:
//        cast_charm(op,caster,spellarch[type],type);
//        success = 1;
//        break;
//        case SP_CREATE_MISSILE:
//          success = cast_create_missile(op,caster,dir,stringarg);
//          break;
//        case SP_CAUSE_COLD:
//        case SP_CAUSE_EBOLA:
//        case SP_CAUSE_FLU:
//        case SP_CAUSE_PLAGUE:
//        case SP_CAUSE_LEPROSY:
//        case SP_CAUSE_SMALLPOX:
//        case SP_CAUSE_PNEUMONIC_PLAGUE:
//        case SP_CAUSE_ANTHRAX:
//        case SP_CAUSE_TYPHOID:
//        case SP_CAUSE_RABIES:
//          success = cast_cause_disease(op,caster,dir,spellarch[type],type);
//          break;
//        case SP_DANCING_SWORD:
//        case SP_STAFF_TO_SNAKE:
//        case SP_ANIMATE_WEAPON:
//          success = animate_weapon(op,caster,dir,spellarch[type],type);
//          break;
//        case SP_SANCTUARY:
//        case SP_FLAME_AURA:
//          success = create_aura(op,caster,spellarch[type],type,0);
//          break;
//        case SP_CONFLICT:
//          success = cast_cause_conflict(op,caster,spellarch[type],type);
//          break;
//        case SP_TOWN_PORTAL:
//          success= cast_create_town_portal (op,caster,dir);
//          break;
//        case SP_MISSILE_SWARM: {
//          success = 1;
//          fire_swarm(op, caster, dir, spellarch[type], SP_M_MISSILE,
//        random_roll_roll(3, 3) +
//            SP_level_strength_adjust(op,caster, type), 0);
//          break;
//        }
#endif
    } /* end of switch */

    play_sound_map(MSP_KNOWN(op), spells[type].sound, SOUND_SPELL);

    if (item == spellNPC)
        return success;

#ifdef SPELLPOINT_LEVEL_DEPEND
    return success ? SP_level_spellpoint_cost(op, caster, type) : 0;
#else
    return success ? (s->sp * PATH_SP_MULT(op, s)) : 0;
#endif
}

/* Disallowed checks if who can cast spell nr here according to msp/map
 * flags. */
/* TODO: This is a bit basic yet and is only meant for boolean, map-level
 * anti-magic effects. Dynamic effects (eg, someone else's anti-magic shield)
 * coming soon! */
static sint8 Disallowed(msp_t *msp, object_t *who, int nr)
{
    if (!(spells[nr].flags & SPELL_DESC_WIS))
    {
        if ((msp->flags & MSP_FLAG_NO_SPELLS))
        {
            if (msp == MSP_KNOWN(who))
            {
                ndi(NDI_UNIQUE, 0, who, "Powerful forces prevent casting ~%s~ here!",
                    STRING_ARCH_NAME(skills[SK_WIZARDRY_SPELLS]));
            }

            return 1;
        }
    }
    else
    {
        if ((msp->flags & MSP_FLAG_NO_PRAYERS))
        {
            if (msp == MSP_KNOWN(who))
            {
                ndi(NDI_UNIQUE, 0, who, "Powerful forces prevent invoking ~%s~ here!",
                    STRING_ARCH_NAME(skills[SK_DIVINE_PRAYERS]));
            }

            return 1;
        }
    }

    if (!(spells[nr].flags & SPELL_DESC_TOWN) &&
        (msp->flags & MSP_FLAG_NO_HARM))
    {
        if (msp == MSP_KNOWN(who))
        {
            ndi(NDI_UNIQUE, 0, who, "Powerful forces prevent all harmful magic here!");
        }

        return 1;
    }

    if ((spells[nr].flags & SPELL_DESC_SUMMON) &&
        (msp->map->flags & MAP_FLAG_NO_SUMMON))
    {
        if (msp == MSP_KNOWN(who))
        {
            ndi(NDI_UNIQUE, 0, who, "Powerful forces prevent all summoning here!");
        }

        return 1;
    }

    return 0;
}

int fire_bolt(object_t *op, object_t *caster, int dir, int type, int magic)
{
    map_t *m = op->map;
    sint16     x = op->x + OVERLAY_X(dir),
               y = op->y + OVERLAY_Y(dir);
    msp_t  *msp = MSP_GET(m, x, y);
    object_t *tmp;
    float    tmp_dam;

    if (!msp ||
        !spellarch[type] ||
        !(tmp = arch_to_object(spellarch[type])))
    {
        return 0;
    }

    set_owner(tmp, op);
    tmp->level = casting_level(caster, type);
    tmp_dam = (float)SP_lvl_dam_adjust(tmp->level, type, spells[type].bdam, 0);
    /* give bonus or malus to damage depending on if the player/mob is attuned/repelled to that spell path */
    tmp->stats.dam = (int)(tmp_dam * PATH_DMG_MULT(op, find_spell(type)));
    tmp->stats.hp = spells[type].bdur;
    tmp->x = x;
    tmp->y = y;
    tmp->direction = dir;

    if (QUERY_FLAG(tmp, FLAG_IS_TURNABLE))
    {
        SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction);
    }

    set_owner(tmp, op);
    copy_owner(tmp, op);
    tmp->weight_limit = tmp->count; /* *very* important - miss this and the spells go really wild! */

    if (MSP_IS_RESTRICTED(msp))
    {
        if (!QUERY_FLAG(tmp, FLAG_REFLECTING))
        {
            return 0;
        }

        m = op->map;
        tmp->x = op->x;
        tmp->y = op->y;
        tmp->direction = absdir(dir + 4);

        if (QUERY_FLAG(tmp, FLAG_IS_TURNABLE))
        {
            SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * tmp->direction);
        }
    }

    if ((tmp = insert_ob_in_map(tmp, m, op, 0)))
    {
        move_bolt(tmp);
    }

    return 1;
}

/*  peterm  added a type field to fire_arch.  Needed it for making
    fireball etall level dependent.
    Later added a ball-lightning firing routine.
 * dir is direction, at is spell we are firing.  Type is index of spell
 * array.  If magic is 1, then add magical attacktype to spell.
 * op is either the owner of the spell (player who gets exp) or the
 * casting object owned by the owner.  caster is the casting object.
 */
int fire_arch(object_t *op, object_t *caster, sint16 x, sint16 y, int dir, archetype_t *at, int type, int level, int magic)
{
    object_t *tmp, *env;
    float    tmp_dam;

    if (at == NULL)
        return 0;
    for (env = op; env->env != NULL; env = env->env)
        ;
    if (env->map == NULL)
        return 0;
    tmp = arch_to_object(at);
    if (tmp == NULL)
        return 0;
    tmp->stats.sp = type;
    tmp_dam = (float) SP_lvl_dam_adjust(level, type, spells[type].bdam, 0);
    /* give bonus or malus to damage depending on if the player/mob is attuned/repelled to that spell path */
    tmp->stats.dam = (int) (tmp_dam * PATH_DMG_MULT(op, find_spell(type)));
    tmp->stats.hp = spells[type].bdur + SP_level_strength_adjust(op, caster, type);
    tmp->x = x, tmp->y = y;
    tmp->direction = dir;
    tmp->stats.grace = tmp->last_sp;
    tmp->stats.maxgrace = 60 + (RANDOM() % 12);

    if (get_owner(op) != NULL)
        copy_owner(tmp, op);
    else
        set_owner(tmp, op);
    tmp->level = level;

    if (QUERY_FLAG(tmp, FLAG_IS_TURNABLE))
        SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * dir);

    if ((tmp = insert_ob_in_map(tmp, op->map, op, 0)) == NULL)
        return 1;

/* TODO: Reference only. Will be removed. */
#if 0
//    switch (type)
//    {
//        case SP_M_MISSILE:
//          move_missile(tmp);
//          break;
//        case SP_POISON_FOG:
//        case SP_DIVINE_SHOCK:
//        case SP_BALL_LIGHTNING:
//          tmp->stats.food = spells[type].bdur + 4 * SP_level_strength_adjust(op, caster, type);
//          move_ball_lightning(tmp);
//          break;
//        default:
//          move_fired_arch(tmp);
//    }
#else
    move_fired_arch(tmp);
#endif

    return 1;
}

int cast_cone(object_t *op, object_t *caster, int dir, int strength, int spell_type, archetype_t *spell_arch, int level, int magic)
{
    object_t *tmp;
    int     i, success = 0, range_min = -1, range_max = 1;
    uint32  count_ref;
    float    tmp_dam;

    if (!dir)
        range_min = -3,range_max = 4,strength /= 2;

    tmp = arch_to_object(spell_arch); /* thats our initial spell object_t */
    if (!tmp)
    {
        LOG(llevBug, "cast_cone(): arch_to_object() failed!? (%s)\n", spell_arch->name);
        return 0;
    }
    count_ref = tmp->count;
    for (i = range_min; i <= range_max; i++)
    {
        map_t *m = op->map;
        sint16     x = op->x + OVERLAY_X(absdir(dir + i)),
                   y = op->y + OVERLAY_Y(absdir(dir + i));
        msp_t  *msp = MSP_GET(m, x, y);

        if (!msp ||
            MSP_IS_RESTRICTED(msp))
        {
            continue;
        }

        success = 1;
        if (!tmp)
            tmp = arch_to_object(spell_arch);
        set_owner(tmp, op);
        copy_owner(tmp, op);
        tmp->weight_limit = count_ref; /* *very* important - miss this and the spells go really wild! */

        tmp->level = level;
        tmp->x = x,tmp->y = y;

        if (dir)
            tmp->stats.sp = dir;
        else
            tmp->stats.sp = i;

        /* for b4, most cone effects have a fixed strength, means ldur = 0 */
        tmp->stats.hp = strength + (SP_level_strength_adjust(op, caster, spell_type) / 5);
        /* for b4, we grap the damage from the spell def table - in that way we can use the
         * arch "firebreath" for different spells with different settings
         */
        tmp_dam = (float) SP_lvl_dam_adjust(level, spell_type, spells[spell_type].bdam , 0);
        /* lets check the originator of the spell (player, mob) has bonus/malus from spell path */
        tmp->stats.dam = (int) (tmp_dam * PATH_DMG_MULT(op, find_spell(spell_type)));

        tmp->stats.maxhp = tmp->count;
        if (!QUERY_FLAG(tmp, FLAG_FLYING))
            LOG(llevDebug, "cast_cone(): arch %s doesn't have flying 1\n", spell_arch->name);
        if ((!QUERY_FLAG(tmp, FLAG_WALK_ON) || !QUERY_FLAG(tmp, FLAG_FLY_ON)) && tmp->stats.dam)
            LOG(llevDebug, "cast_cone(): arch %s doesn't have walk_on 1 and fly_on 1\n", spell_arch->name);

        if (QUERY_FLAG(tmp, FLAG_IS_TURNABLE))
            SET_ANIMATION(tmp, (NUM_ANIMATIONS(tmp) / NUM_FACINGS(tmp)) * dir);

        if (!insert_ob_in_map(tmp, op->map, op, 0))
            return 0;
        if (tmp->other_arch)
            cone_drop(tmp);
        tmp = NULL;
    }

    if (tmp) /* can happens when we can't drop anything */
    {
        if (!QUERY_FLAG(tmp, FLAG_REMOVED))
            remove_ob(tmp); /* was not inserted */
    }
    return success;
}

/* this function checks to see if the cone pushes objects as well
 * as flies over and damages them */
void check_cone_push(object_t *op)
{
    msp_t  *msp = MSP_KNOWN(op);
    int        weight_move = 1000 + 1000 * op->level;
    object_t    *tmp,
              *next,
              *tmp2; /* object on the map */

    FOREACH_OBJECT_IN_MSP(tmp, msp, next)
    {
        int num_sections   = 1;

        /* don't move parts of objects */
        if (tmp->head)
        {
            continue;
        }

        /* don't move floors or immobile objects */
        if (tmp->type == FLOOR ||
            (!QUERY_FLAG(tmp, FLAG_ALIVE) &&
             QUERY_FLAG(tmp, FLAG_NO_PICK)))
        {
            continue;
        }

        /* count the object's sections */
        for (tmp2 = tmp; tmp2; tmp2 = tmp2->more)
        {
            num_sections++;
        }

        /* move it. */
        if (random_roll(0, weight_move - 1) > tmp->weight / num_sections)
        {
            (void)move_ob(tmp, absdir(op->stats.sp), op);
        }
    }
}

/* drops an object based on what is in the cone's "other_arch" */
void cone_drop(object_t *op)
{
    object_t *new_ob  = arch_to_object(op->other_arch);
    new_ob->x = op->x;
    new_ob->y = op->y;
    new_ob->stats.food = op->stats.hp;
    new_ob->level = op->level;
    set_owner(new_ob, op->owner);
    if (op->chosen_skill)
    {
        new_ob->chosen_skill = op->chosen_skill;
        new_ob->skillgroup = op->chosen_skill->skillgroup;
    }
    insert_ob_in_map(new_ob, op->map, op, 0);
}


void move_cone(object_t *op)
{
    int     i;
    tag_t   tag;

    /* if no map then hit_map will crash so just ignore object_t */
    if (!op->map)
    {
        LOG(llevBug, "BUG: Tried to move_cone object %s without a map.\n", STRING_OBJ_NAME(op));
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }

    /* lava saves it's life, but not yours  :) */
    if (QUERY_FLAG(op, FLAG_LIFESAVE))
    {
        hit_map(op, 0);
        return;
    }

    /* If no owner left, the spell dies out. */
    if (get_owner(op) == NULL)
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }

    /* Hit map returns 1 if it hits a monster.  If it does, set
     * food to 1, which will stop the cone from progressing.
     */
    tag = op->count;
    op->stats.food |= hit_map(op, 0);
    /* Check to see if we should push anything.
     * Cones with AT_PHYSICAL push whatever is in them to some
     * degree.  */
    /*
    if (op->attacktype & AT_PHYSICAL)
        check_cone_push(op);
    */
    if (!OBJECT_VALID(op, tag))
        return;

    if ((op->stats.hp -= 2) < 0)
    {
        if (op->stats.exp)
        {
            op->speed = 0;
            update_ob_speed(op);
            op->stats.exp = 0;
            op->stats.sp = 0; /* so they will join */
        }
        else
        {
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        }
        return;
    }

    if (op->stats.food)
        return;

    op->stats.food = 1;

    for (i = -1; i < 2; i++)
    {
        map_t *m = op->map;
        sint16     x = op->x + OVERLAY_X(absdir(op->stats.sp + i)),
                   y = op->y + OVERLAY_Y(absdir(op->stats.sp + i));
        msp_t  *msp = MSP_GET(m, x, y);

        if (!msp ||
            MSP_IS_RESTRICTED(msp))
        {
            continue;
        }

        if (CheckSpellInstance(msp, op))
        {
            object_t *tmp = arch_to_object(op->arch);

            copy_owner(tmp, op);
            /* *very* important - this is the count value of the
             * *first* object we created with this cone spell.
             * we use it for identify this spell. Miss this
             * and CheckSpellInstance() will allow to create
             * 1000s in a single tile! */
            tmp->weight_limit = op->weight_limit;
            tmp->x = x;
            tmp->y = y;

            /* holy word stuff */
            /*
                    if(tmp->attacktype&AT_HOLYWORD||tmp->attacktype&AT_GODPOWER)
                    if(!tailor_god_spell(tmp,op)) return;
                    */
            tmp->level = op->level;
            tmp->stats.sp = op->stats.sp,tmp->stats.hp = op->stats.hp + 1;
            tmp->stats.maxhp = op->stats.maxhp;
            tmp->stats.dam = op->stats.dam;

            if (!insert_ob_in_map(tmp, m, op, 0))
            {
                return;
            }

            if (tmp->other_arch)
                cone_drop(tmp);
        }
    }
}

/* This function ensures that a AoE spell only put *one* instance/object of
 * themself in a tile. */
static int CheckSpellInstance(msp_t *msp, object_t *op)
{
    object_t *tmp,
           *next;

    FOREACH_OBJECT_IN_MSP(tmp, msp, next)
    {
        /* weight_limit is the ->count of the original spell object which started
         * this spell!
         */
        if (op->type == tmp->type &&
            op->weight_limit == tmp->weight_limit)
        {
            return 0; /* only one part for cone/explosion per tile! */
        }
    }

    return 1;
}

void forklightning(object_t *op, object_t *tmp)
{
    map_t  *m;
    sint16      x,
                y;
    msp_t   *msp;
    int         new_dir = 1, /* direction or -1 for left, +1 for right 0 if no new bolt */
                t_dir; /* stores temporary dir calculation */

    /* pick a fork direction.  tmp->stats.Con is the left bias
             i.e., the chance in 100 of forking LEFT
              Should start out at 50, down to 25 for one already going left
             down to 0 for one going 90 degrees left off original path*/

    if (random_roll(0, 99) < tmp->stats.Con)  /* fork left */
        new_dir = -1;

    /* check the new dir for a wall and in the map*/
    t_dir = absdir(tmp->direction + new_dir);
    m = tmp->map;
    x = tmp->x + OVERLAY_X(t_dir);
    y = tmp->y + OVERLAY_Y(t_dir);
    msp = MSP_GET(m, x, y);

    if (!msp ||
        MSP_IS_RESTRICTED(msp))
    {
        new_dir = 0;
    }

    if (new_dir)
    {
        /* OK, we made a fork */
        object_t *new_bolt    = get_object();
        copy_object(tmp, new_bolt);

        /* reduce chances of subsequent forking */
        new_bolt->stats.Dex -= 10;
        tmp->stats.Dex -= 10;  /* less forks from main bolt too */
        new_bolt->stats.Con += 25 * new_dir; /* adjust the left bias */
        new_bolt->speed_left = -0.1f;
        new_bolt->direction = t_dir;
        new_bolt->stats.hp++;
        new_bolt->x = x;
        new_bolt->y = y;
        new_bolt->stats.dam /= 2;  /* reduce daughter bolt damage */
        new_bolt->stats.dam++;
        tmp->stats.dam /= 2;  /* reduce father bolt damage */
        tmp->stats.dam++;
        if (!insert_ob_in_map(new_bolt, m, op, 0))
            return;
        update_turn_face(new_bolt);
    }
}

/* reflwall - decides weither the (spell-)object sp_op will
 * be reflected from the given mapsquare. Returns 1 if true.
 * (Note that for living creatures there is a small chance that
 * reflect_spell fails.)
 * This function don't scale up now - it uses map tile flags. MT-2004
 */
int reflwall(msp_t *msp, object_t *sp_op)
{
    if (!(msp->flags & MSP_FLAG_REFL_CASTABLE))
    {
        return 0;
    }

    /* we have reflection. But there is a small chance it will fail. */
    if (sp_op->type == LIGHTNING) /* reflect always */
    {
        return 1;
    }

    if (!missile_reflection_adjust(sp_op, QUERY_FLAG(sp_op, FLAG_WAS_REFLECTED)))
    {
        return 0;
    }

    /* we get resisted - except a small fail chance */
    if ((random_roll(0, 99)) < 90 - sp_op->level / 10)
    {
        SET_FLAG(sp_op, FLAG_WAS_REFLECTED);
        return 1;
    }

    return 0;
}

void move_bolt(object_t *op)
{
    object_t *tmp;
    int     w, r;

    if (--(op->stats.hp) < 0)
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_DEFAULT);
        return;
    }

    hit_map(op, 0);

    if (!op->stats.sp)
    {
        map_t *m = op->map;
        sint16     x = op->x + OVERLAY_X(op->direction),
                   y = op->y + OVERLAY_Y(op->direction);
        msp_t  *msp = MSP_GET(m, x, y);

        op->stats.sp = 1;

        if (!msp ||
            !op->direction ||
            (msp->flags & MSP_FLAG_NO_SPELLS))
        {
            return;
        }

        w = MSP_IS_RESTRICTED(msp);
        r = reflwall(msp, op);

        if (w ||
            r)
        {
            /* We're about to bounce */
            if (!QUERY_FLAG(op, FLAG_REFLECTING))
            {
                return;
            }

            op->stats.sp = 0;

            if ((op->direction & 1))
            {
                op->direction = absdir(op->direction + 4);
            }
            else
            {
                msp_t *msp_l,
                         *msp_r;

                m = op->map;
                x = op->x + OVERLAY_X(absdir(op->direction - 1));
                y = op->y + OVERLAY_Y(absdir(op->direction - 1));
                msp_l = MSP_GET(m, x, y);
                m = op->map;
                x = op->x + OVERLAY_X(absdir(op->direction + 1));
                y = op->y + OVERLAY_Y(absdir(op->direction + 1));
                msp_r = MSP_GET(m, x, y);

                if (MSP_IS_RESTRICTED(msp_l) &&
                    MSP_IS_RESTRICTED(msp_r))
                    op->direction = absdir(op->direction + 4);
                else if (MSP_IS_RESTRICTED(msp_l))
                    op->direction = absdir(op->direction + 2);
                else if (MSP_IS_RESTRICTED(msp_r))
                    op->direction = absdir(op->direction - 2);
            }

            update_turn_face(op); /* A bolt *must* be IS_TURNABLE */
            return;
        }
        /* disabling this line allows back bouncing! */
/*        else if (!CheckSpellInstance(msp, op) ) */
        else
        {
            /* Create a copy of this object and put it ahead */
            tmp = get_object();
            copy_object(op, tmp);
            tmp->speed_left = -0.1f;
            tmp->weight_limit = op->weight_limit;
            tmp->stats.sp = 0;
            tmp->x += OVERLAY_X(tmp->direction);
            tmp->y += OVERLAY_Y(tmp->direction);

            if (!insert_ob_in_map(tmp, op->map, op, 0))
            {
                return;
            }

            /* Possibly create forks of this object going off in other directions. */
            if (random_roll(0, 99) < tmp->stats.Dex)
            {
                /* stats.Dex % of forking */
                forklightning(op, tmp);
            }

            if (tmp)
            {
                if (!tmp->stats.food)
                {
                    tmp->stats.food = 1;
                    move_bolt(tmp);
                }
                else
                {
                    tmp->stats.food = 0;
                }
            }
        }
    }
}

/* updated this to allow more than the golem 'head' to attack */
/* op is the golem to be moved. */

void move_golem(object_t *op)
{
    int         made_attack = 0;
    object_t     *part;
    tag_t       tag;

    if (QUERY_FLAG(op, FLAG_MONSTER) && op->stats.hp)
        return; /* Has already been moved */

    if (get_owner(op) == NULL)
    {
        LOG(llevBug, "BUG:: Golem without owner destructed.\n");
        remove_ob(op);
        return;
    }
    /* It would be nice to have a cleaner way of what message to print
     * when the golem expires than these hard coded entries.
     */
    if (--op->stats.hp < 0)
    {
        if (op->skillgroup && op->skillgroup->stats.Wis)
        {
            if (op->inv)
                ndi(NDI_UNIQUE, 0, op->owner, "Your staff stops slithering around and lies still.");
            else
                ndi(NDI_UNIQUE, 0, op->owner, "Your %s departed this plane.", op->name);
        }
        else if (!strncmp(op->name, "animated ", 9))
        {
            ndi(NDI_UNIQUE, 0, op->owner, "Your %s falls to the ground.", op->name);
        }
        else
        {
            ndi(NDI_UNIQUE, 0, op->owner, "Your %s dissolved.", op->name);
        }
        send_golem_control(op, GOLEM_CTR_RELEASE);
        CONTR(op->owner)->golem = NULL;
        (void)kill_object(op, NULL, "expired", NULL);
        return;
    }

    /* Do golem attacks/movement for single & multisq golems.  */
    tag = op->count;

    if (move_ob(op, op->direction, NULL) == MOVE_RESULT_SUCCESS)
    {
        return;
    }

    if (!OBJECT_VALID(op, tag))
    {
        return;
    }

    for (part = op; part; part = part->more)
    {
        map_t *m = op->map;
        sint16     x = part->x + OVERLAY_X(op->direction),
                   y = part->y + OVERLAY_Y(op->direction);
        msp_t  *msp = MSP_GET(m, x, y);
        object_t    *this,
                  *next;

        if (!msp)
        {
            continue;
        }

        FOREACH_OBJECT_IN_MSP(this, msp, next)
        {
            if (IS_LIVE(this))
            {
                break;
            }
        }

        /* We used to call will_hit_self to make sure we don't
         * hit ourselves, but that didn't work, and I don't really
         * know if that was more efficient anyways than this.
         * This at least works.  Note that this->head can be NULL,
         * but since we are not trying to dereferance that pointer,
         * that isn't a problem  */
        if (this &&
            this != op &&
            this->head != op)
        {
            object_t *owner = get_owner(op);

            /* for golems with race fields, we don't attack aligned races */
            if (this->race &&
                op->race &&
                strstr(op->race, this->race))
            {
                if (owner)
                {
                    ndi(NDI_UNIQUE, 0, owner, "%s avoids damaging %s.",
                        QUERY_SHORT_NAME(op, owner), QUERY_SHORT_NAME(this, owner));
                }
            }
            else if (op->skillgroup &&
                     this == owner)
            {
                ndi(NDI_UNIQUE, 0, owner, "%s avoids damaging you.",
                    QUERY_SHORT_NAME(op, owner));
            }
            else
            {
                attack_ob(this, op, NULL);
                made_attack = 1;
            }
        }
    }

    if (made_attack)
    {
        update_object(op, UP_OBJ_FACE);
    }
}

void control_golem(object_t *op, int dir)
{
    op->direction = dir;
}


void move_magic_missile(object_t *op)
{
    object_t     *owner;
    map_t  *m;
    sint16      x,
                y;
    msp_t   *msp;
    sint8       i;

    /* .... hm... no "instant missiles?" */
    owner = get_owner(op);
    if (owner == NULL)
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }

    m = op->map;
    x = op->x + OVERLAY_X(op->direction);
    y = op->y + OVERLAY_Y(op->direction);
    msp = MSP_GET(m, x, y);

    if (!msp)
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }

    if (msp_blocked(op, NULL, OVERLAY_X(op->direction), OVERLAY_Y(op->direction)))
    {
        tag_t   tag = op->count;
        hit_map(op, op->direction);
        if (OBJECT_VALID(op, tag))
        {
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        }
        return;
    }

    remove_ob(op);
    check_walk_off(op, NULL, MOVE_APPLY_VANISHED);

    if (!op->direction ||
        (msp->flags & MSP_FLAG_BLOCKSVIEW) ||
        MSP_IS_RESTRICTED(msp))
    {
        return;
    }

    op->map = m;
    op->x = x;
    op->y = y;
    i = overlay_find_dir(msp, get_owner(op));

    if (i &&
        i != op->direction)
    {
        op->direction = absdir(op->direction + ((op->direction - i + 8) % 8 < 4 ? -1 : 1));
        SET_ANIMATION(op, (NUM_ANIMATIONS(op) / NUM_FACINGS(op)) * op->direction);
    }

    insert_ob_in_map(op, m, op, 0);
}

/* explode object will remove the exploding object from a container and set on map.
 * for this action, out_of_map() is called.
 * If the object is on a map, we assume that the position is controlled when object
 * is inserted or moved, so no need to recontrol. MT. */
void explode_object(object_t *op)
{
    tag_t       op_tag  = op->count;
    object_t     *tmp,
               *env;

    play_sound_map(MSP_KNOWN(op), SOUND_OB_EXPLODE, SOUND_NORMAL);
    if (op->other_arch == NULL)
    {
        LOG(llevBug, "BUG: explode_object(): op %s without other_arch\n", STRING_OBJ_NAME(op));
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        return;
    }

    if (op->env) /* object is in container, try to drop on map! */
    {
        map_t  *m;
        sint16  x,
                y;

        for (env = op; env->env != NULL; env = env->env)
            ;

        m = env->map;
        x = env->x;
        y = env->y;

        if (!(m = OUT_OF_MAP(m, x, y)))
        {
            LOG(llevBug, "BUG: explode_object(): env out of map (%s)\n", STRING_OBJ_NAME(op));
            remove_ob(op);
            check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
            return;
        }

        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
        op->x = x;
        op->y = y;

        if (!insert_ob_in_map(op, m, op, 0))
            return;
    }

    /*
    if (op->attacktype)
    {
        hit_map(op, 0);
        if (!OBJECT_VALID(op, op_tag))
            return;
    }
    */

    /*  peterm:  hack added to make fireballs and other explosions level
     *  dependent:
     */
    /*  op->stats.sp stores the spell which made this object here. */
    tmp = arch_to_object(op->other_arch);
    switch (tmp->type)
    {
/* TODO: Reference only. Will be removed. */
#if 0
//        case POISONCLOUD:
//        case FBALL:
//          {
//              tmp->stats.dam = (sint16) SP_lvl_dam_adjust(op->level, op->stats.sp, tmp->stats.dam, 0);
//
//              /* I have to fix this. This code is for marking the object as "magic". Using
//               * the attacktype for it, is somewhat brain dead. We have now the IS_MAGIC flag
//               * for it. MT.
//               */
//
//              /*tmp->stats.dam += SP_level_dam_adjust(op,op,op->stats.sp);*/
//              /*
//                 if(op->attacktype&AT_MAGIC)
//                   tmp->attacktype|=AT_MAGIC;
//                */
//              copy_owner(tmp, op);
//              if (op->stats.hp)
//                  tmp->stats.hp = op->stats.hp;
//              tmp->stats.maxhp = op->count; /* Unique ID */
//              tmp->x = op->x;
//              tmp->y = op->y;
//
//              /* needed for AT_HOLYWORD stuff -b.t. */
//              /*
//              if (tmp->attacktype & AT_GODPOWER)
//                  if (!tailor_god_spell(tmp, op))
//                  {
//                      remove_ob(op);
//                      check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
//                      return;
//                  }
//               */
//              /* Prevent recursion */
//              CLEAR_FLAG(op, FLAG_WALK_ON);
//              CLEAR_FLAG(op, FLAG_FLY_ON);
//
//              insert_ob_in_map(tmp, op->map, op, 0);
//              break;
//          }
#endif
        case CONE:
          {
              int   type    = tmp->stats.sp;

              if (!type)
                  type = op->stats.sp;
              copy_owner(tmp, op);
              cast_cone(op, op, 0, spells[type].bdur, type, op->other_arch, op->level, 0);
              break;
          }
    }

    /* remove the firebullet */
    if (OBJECT_VALID(op, op_tag))
    {
        remove_ob(op);
        check_walk_off(op, NULL, MOVE_APPLY_VANISHED);
    }
}

void drain_rod_charge(object_t *rod)
{
    rod->stats.hp -= spells[rod->stats.sp].sp;
    if (QUERY_FLAG(rod, FLAG_ANIMATE))
        fix_rod_speed(rod);
}

void fix_rod_speed(object_t *rod)
{
    rod->speed = (FABS(rod->arch->clone.speed) * rod->stats.hp) / (float) rod->stats.maxhp;
    if (rod->speed < 0.02f)
        rod->speed = 0.02f;
    update_ob_speed(rod);
}


/* i changed this function to take care about target system.
 * This function examines target & (real) hitter and the
 * given spell flags. We have to decide some flags including
 * PvP flags - note that normally player are friendly to other
 * players except for pvp - there they are enemys.
 * op is owner/object which casted the spell, item is the object
 * which was used (scroll, potion, pet).
 */
int find_target_for_spell(object_t *op, object_t *item, object_t **target, int dir, uint32 flags)
{
    object_t *tmp;

    /*  LOG(llevInfo,"FIND_TARGET_for_spell: op: %s used: %s dir %d flags: %x\n", op->name, item?item->name:"<none>", dir, flags);*/

    *target = NULL; /* default target is - nothing! */

    if (flags & SPELL_DESC_DIRECTION) /* we cast something on the map... no target */
        return TRUE;

    if (op->type == PLAYER) /* a player has invoked this spell */
    {
        /* try to cast on self but only when really no friendly or enemy is set */
        if ((flags & SPELL_DESC_SELF) && !(flags & (SPELL_DESC_ENEMY | SPELL_DESC_FRIENDLY)))
        {
            *target = op; /* self ... and no other tests */
            return TRUE;
        }

        tmp = CONTR(op)->target_object;

        /* lets check our target - we have one? friend or enemy? */
        if (!tmp || !OBJECT_ACTIVE(tmp) || tmp == CONTR(op)->ob || CONTR(op)->target_object_count != tmp->count)
        {
            /* no valid target, or we target self! */
            if (flags & SPELL_DESC_SELF) /* can we cast this on self? */
            {
                *target = op; /* right, we are target */
                return TRUE;
            }
        }
        else /* we have a target and its not self */
        {
            /* Friend spells are allowed on friends, neutrals and other players */
            if (flags & SPELL_DESC_FRIENDLY)
            {
                if (get_friendship(op, tmp) > FRIENDSHIP_ATTACK)
                {
                    *target = tmp;
                    return TRUE;
                }
            }

            /* enemy spells only allowed on enemies or neutrals */
            if (flags & SPELL_DESC_ENEMY)
            {
                if (get_friendship(op, tmp) < FRIENDSHIP_HELP)
                {
                    *target = tmp;
                    return TRUE;
                }
            }

            /* Self spells are always allowed */
            if (flags & SPELL_DESC_SELF)
            {
                *target = op;
                return TRUE;
            }
        }
    }
    else /* thats a mob OR rune/firewall/.. OR a pet/summon controlled from player */
    {
        /* we use op->enemy as target from non player caster.
         * we need to set this from outside and for healing spells,
         * we must set from outside temporary the enemy to a friendly unit.
         * This is safe because we do no AI stuff here - we simply USE the
         * target here even the stuff above looks like we select one...
         * its only a fallback.
         */

        /* sanity check for a legal target */
        if (op->enemy && OBJECT_ACTIVE(op->enemy) && op->enemy->count == op->enemy_count)
        {
            *target = op->enemy;
            return TRUE;
        }
    }

    return FALSE; /* invalid target/spell or whatever */

#if 0
//    if(op->type!=PLAYER&&op->type!=RUNE)
//    {
//        tmp=get_owner(op);
//        if(!tmp || !QUERY_FLAG(tmp,FLAG_MONSTER))
//            tmp=op;
//    }
//    else
//    {
//        xt = op->x+OVERLAY_X(dir);
//        yt = op->y+OVERLAY_Y(dir);
//        if (!(m=out_of_map(op->map,&xt,&yt)))
//            tmp=NULL;
//        else
//        {
//            for(tmp=MSP_GET_LAST(m,xt,yt);tmp!=NULL;tmp=tmp->below)
//            {
//                if(tmp->type==PLAYER)
//                    break;
//            }
//        }
//    }
//    if(tmp==NULL)
//    {
//        for(tmp=MSP_GET_LAST(op->map,op->x,op->y);tmp!=NULL;tmp=tmp->below)
//        {
//            if(tmp->type==PLAYER)
//                break;
//        }
//    }
//    return tmp;
#endif
}

int reduction_dir[OVERLAY_7X7][3] =
{
    {0,0,0}, /* 0 */
    {0,0,0}, /* 1 */
    {0,0,0}, /* 2 */
    {0,0,0}, /* 3 */
    {0,0,0}, /* 4 */
    {0,0,0}, /* 5 */
    {0,0,0}, /* 6 */
    {0,0,0}, /* 7 */
    {0,0,0}, /* 8 */
    {8,1,2}, /* 9 */
    {1,2, -1}, /* 10 */
    {2,10,12}, /* 11 */
    {2,3, -1}, /* 12 */
    {2,3,4}, /* 13 */
    {3,4, -1}, /* 14 */
    {4,14,16}, /* 15 */
    {5,4, -1}, /* 16 */
    {4,5,6}, /* 17 */
    {6,5, -1}, /* 18 */
    {6,20,18}, /* 19 */
    {7,6, -1}, /* 20 */
    {6,7,8}, /* 21 */
    {7,8, -1}, /* 22 */
    {8,22,24}, /* 23 */
    {8,1, -1}, /* 24 */
    {24,9,10}, /* 25 */
    {9,10, -1}, /* 26 */
    {10,11, -1}, /* 27 */
    {27,11,29}, /* 28 */
    {11,12, -1}, /* 29 */
    {12,13, -1}, /* 30 */
    {12,13,14}, /* 31 */
    {13,14, -1}, /* 32 */
    {14,15, -1}, /* 33 */
    {33,15,35}, /* 34 */
    {16,15, -1}, /* 35 */
    {17,16, -1}, /* 36 */
    {18,17,16}, /* 37 */
    {18,17, -1}, /* 38 */
    {18,19, -1}, /* 39 */
    {41,19,39}, /* 40 */
    {19,20, -1}, /* 41 */
    {20,21, -1}, /* 42 */
    {20,21,22}, /* 43 */
    {21,22, -1}, /* 44 */
    {23,22, -1}, /* 45 */
    {45,47,23}, /* 46 */
    {23,24, -1}, /* 47 */
    {24,9, -1}
}; /* 48 */

/* Recursive routine to step back and see if we can
 * find a path to that monster that we found.  If not,
 * we don't bother going toward it.  Returns 1 if we
 * can see a direct way to get it
 * Modified to be map tile aware -.MSW
 */
/* hm, how fast is this function? MT-2004 */
int can_see_monsterP(map_t *m, int x, int y, int dir)
{
    sint16     dx,
               dy;
    msp_t  *msp;

    if (dir < 0)
    {
        return 0;  /* exit condition:  invalid direction */
    }

    dx = x + OVERLAY_X(dir);
    dy = y + OVERLAY_Y(dir);
    msp = MSP_GET(m, dx, dy);

    if (!msp ||
        MSP_IS_RESTRICTED(msp))
    {
        return 0;
    }

    /* yes, can see. */
    if (dir < 9)
    {
        return 1;
    }

    return can_see_monsterP(m, x, y, reduction_dir[dir][0])
         | can_see_monsterP(m, x, y, reduction_dir[dir][1])
         | can_see_monsterP(m, x, y, reduction_dir[dir][2]);
}

/* peterm:  */

/*  peterm:  the following defines the parameters for all the
spells.
    bdam:  base damage or hp of spell or summoned monster
  bdur:  base duration of spell or base range
  ldam:  levels you need over the min for the spell to gain one dam
  ldur:  levels you need over the min for the spell to gain one dur
*/


/*  The following adjustments to spell strength are done in the
philosophy that the longer one knows a spell, the better one
should get at it.  So the more experience levels you are above
the minimum for knowing a spell, the more effective it becomes.
most of the following adjustments are for damage only, some are
for turning undead and whatnot.

  The following functions assume that casting the spell is not
denied.  Denied spells have an undefined path level modifier.
There wouldn't be a meaningful result anyway.

  The arrays are defined in spells.h*/

/* July 1995 - I changed the next 3 functions slightly by replacing
 * the casters level (op->level) with the skill level (SK_level(op))
 * instead for when we have compiled with ALLOW_SKILLS - b.t.
 */
/* now based on caster's level instead of on op's level and caster's    *
 * path modifiers.  --DAMN                      */

int SP_level_dam_adjust(object_t *op, object_t *caster, int spell_type)
{
    int level   = casting_level(caster, spell_type);
    int adj     = (level - spells[spell_type].level);
    if (adj < 0)
        adj = 0;
    if (spells[spell_type].ldam)
        adj /= spells[spell_type].ldam;
    else
        adj = 0;
    return adj;
}

/* July 1995 - changed slightly (SK_level) for ALLOW_SKILLS - b.t. */
/* now based on caster's level instead of on op's level and caster's    *
 * path modifiers.  --DAMN                      */
int SP_level_strength_adjust(object_t *op, object_t *caster, int spell_type)
{
    int level   = casting_level(caster, spell_type);
    int adj     = (level - spells[spell_type].level);
    if (adj < 0)
        adj = 0;
    if (spells[spell_type].ldur)
        adj /= spells[spell_type].ldur;
    else
        adj = 0;
    return adj;
}

/* April 2007 - i changed the spell cost to a better configurable, human
 * readable system. We have with level a low and with spl_max a high points border now,
 * where sp is the base cost and spl the additional points we add all spl_level over
 * the low border level. If its >spl_max we use spl_max.
 * Thats allows a unique setting for different spells.
 */
int SP_level_spellpoint_cost(object_t *op, object_t *caster, int spell_type)
{
    spell  *s       = find_spell(spell_type);
    int     level   = casting_level(caster, spell_type);
    int     sp        = s->sp;

#ifdef SPELLPOINT_LEVEL_DEPEND
    /* level defines our lower border */
    if(level > s->level && s->spl)
    {
        /* we add every spl_level spl points to the spell costs */
        level -= s->level;
        level /= s->spl_level;

        sp += level * s->spl;
        /* spl_max defines a upper border of sp costs */
        if(s->spl_max>0 && sp > s->spl_max)
            sp = s->spl_max;
    }
#endif /* SPELLPOINT_LEVEL_DEPEND */

    /* at path bonus/malus */
    sp = (int) ((float) sp * (float) PATH_SP_MULT(caster, s));

    return sp;
}

/*  look_up_spell_by_name:  peterm
    this function attempts to find the spell spname in spells[].
    if it doesn't exist, or if the op cannot cast that spname,
    -1 is returned.  */


int look_up_spell_by_name(object_t *op, const char *spname)
{
    int gmaster_mode = GET_GMASTER_MODE(op);
    int numknown;
    int spnum;
    int plen;
    int spellen;
    int i;

    if (!spname)
    {
        return -1;
    }

    numknown = ((gmaster_mode & GMASTER_MODE_SA)) ? NROFREALSPELLS : CONTR(op)->nrofknownspells;
    plen = strlen(spname);

    for (i = 0; i < numknown; i++)
    {
        spnum = ((gmaster_mode & GMASTER_MODE_SA)) ? i : CONTR(op)->known_spells[i];
        spellen = strlen(spells[spnum].name);

        if (strncmp(spname, spells[spnum].name, MIN(spellen, plen)) == 0)
            return spnum;
    }
    return -1;
}

/* we use our damage/level tables to adjust the base_dam. Normally, the damage increase
 * with the level of the caster - or if the caster is a living object, with the level
 * of the used skill. We can also assign a stats bonus, similiar to the players if its >= 0.
 * Is the base dam = -1, we use the default spell table setting with spell_type to get
 * a valid base damage.
 */
int SP_lvl_dam_adjust(int level, int spell_type, int base_dam, int stats_bonus)
{
    int     dam_adj;


    /* get a base damage when we don't have one from caller */
    if (base_dam == -1)
        base_dam = spells[spell_type].bdam;

    /* ensure a legal level value */
    if (level < 0)
        level = 1;

    /* we simulate a stats bonus if needed to have a more progressive damage behaviour */
    if(stats_bonus >= 0)
        dam_adj = (int) (((float) base_dam * LEVEL_DAMAGE(level) * get_player_stat_bonus(stats_bonus)));
    else
        dam_adj = (int) (((float) base_dam * LEVEL_DAMAGE(level)));

    return dam_adj;
}


