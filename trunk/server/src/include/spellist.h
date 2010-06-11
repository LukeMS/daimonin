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

spell spells[NROFREALSPELLS]          =
{
    {"firestorm",                   SPELL_TYPE_WIZARD, 1, 
    4, 12, 3, 6,0, 1.0,
    65,      7,      4,      0,  
    1, 4, 0, 5, SOUND_MAGIC_FIRE,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_DUST | SPELL_USE_BOOK | SPELL_USE_POTION,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "firebreath",SPELL_ACTIVE
    },

    {"icestorm",                    SPELL_TYPE_WIZARD, 1, 
    4, 12, 3, 6,0,1.0,
    65,      7,      4,      0,  
    1, 4, 0, 5, SOUND_MAGIC_ICE,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_BOOK | SPELL_USE_POTION | SPELL_USE_DUST,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "icestorm",SPELL_ACTIVE
    }, 

    {"minor healing",               SPELL_TYPE_PRIEST, 1, 
    4, 8, 3, 6,3,1.0,
    0,       0,     0,      0, 
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_BALM | SPELL_USE_SCROLL | SPELL_USE_ROD | SPELL_USE_POTION | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS | SPELL_DESC_TOWN,
    PATH_LIFE, "meffect_green",SPELL_ACTIVE
    }, 

    {"cure poison",                 SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,4, 1.0,/* potion only */
    0,       0,     0,      0,
    0, 0, 0, 0,   SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_POTION | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS | SPELL_DESC_TOWN,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    }, 

    {"cure disease",                SPELL_TYPE_PRIEST, 1, 
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

    {"identify",                    SPELL_TYPE_WIZARD, 1, 
    5, 24, 3, 6,2,1.0,
    0,       0,     0,      0,
    0, 0, 0, 3,  SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_TOWN,
    PATH_TRANSMUTATION, "meffect_pink",SPELL_ACTIVE
    }, 

    {"detect magic",                SPELL_TYPE_WIZARD, 1, 
    5, 8, 3, 6,0,1.0,
    0,       0,     0,      0,
    0, 0, 0, 3,   SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_TOWN,
    PATH_ABJURATION, "meffect_pink",SPELL_ACTIVE
    }, 

    {"detect curse",                SPELL_TYPE_PRIEST, 1, 
    5, 8, 3, 6,0,1.0,
    0,       0,     0,      0,
    0, 0, 0, 0,   SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_SCROLL | SPELL_USE_BOOK,
    SPELL_DESC_SELF | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_ABJURATION, "meffect_pink",SPELL_ACTIVE
    },

    {"remove curse",                SPELL_TYPE_PRIEST, 1, 
    5, 24, 3, 6,2,1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_DEFAULT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_BOOK,     /* scroll */
    SPELL_DESC_SELF | SPELL_DESC_TOWN | SPELL_DESC_FRIENDLY | SPELL_DESC_WIS,
    PATH_ARCANE, "meffect_blue",SPELL_ACTIVE
    }, 

    {"remove damnation",            SPELL_TYPE_PRIEST, 1, 
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

    {"firebolt",                   SPELL_TYPE_WIZARD, 1, 
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

    {"frostbolt",                   SPELL_TYPE_WIZARD, 1, 
    6, 12, 3, 6,0, 1.0,
    108,      8,      4,      0,  
    1, 3, 0, 5, SOUND_MAGIC_ICE,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_DUST | SPELL_USE_BOOK | SPELL_USE_POTION,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "frostbolt", SPELL_ACTIVE
    },

    {"remove depletion",            SPELL_TYPE_PRIEST, 1, 
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

    {"remove death sickness",            SPELL_TYPE_PRIEST, 1, 
    5, 24, 3, 6,0, 1.0,
    0,       0,     0,      0,
    0,  0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST, /* npc/god only atm */
    SPELL_DESC_SELF | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"restoration",            SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"lightning",                   SPELL_TYPE_WIZARD, 1, 
    6, 12, 3, 6,0, 1.0,
    108,      8,      4,      0,  
    1, 3, 0, 5, SOUND_MAGIC_ELEC,
    SPELL_USE_CAST | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD | SPELL_USE_DUST | SPELL_USE_BOOK | SPELL_USE_POTION,
    SPELL_DESC_DIRECTION,
    PATH_ELEMENTAL, "lightning", SPELL_ACTIVE
    },

    {"remove slow",            SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove fear",            SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove snare",            SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove paralysis",            SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove confusion",            SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

    {"remove blindness",            SPELL_TYPE_PRIEST, 1, 
    5, 16, 3, 6,3, 1.0,
    0,       0,     0,      0,
    0, 0, 0, 0, SOUND_MAGIC_STAT,
    SPELL_USE_CAST | SPELL_USE_SCROLL | SPELL_USE_HORN | SPELL_USE_WAND | SPELL_USE_ROD,
    SPELL_DESC_SELF | SPELL_DESC_FRIENDLY | SPELL_DESC_TOWN | SPELL_DESC_WIS,
    PATH_LIFE, "meffect_purple",SPELL_ACTIVE
    },

};

/*
{"probe",            1, 3, 40, 2, 1, 75,  6,  1, 0, 0, 0,
0,0,0,0,0,0,1,0,0,1,
 PATH_INFO, "probebullet",},
*/


/*
spell spells_DUMMY[]={
{"small fireball",           1, 6, 40, 5, 0, 0,  8,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,1,
 PATH_FIRE, "firebullet_s",},
{"medium fireball",          3,10, 20, 10, 0, 0,  6,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,1,
 PATH_FIRE, "firebullet_m",},
{"large fireball",           5,16, 10, 15, 0, 0,  2,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,1,
 PATH_FIRE, "firebullet_l",},
{"small lightning",      1, 6, 40, 5, 0, 0,  8,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_ELEC, "lightning_s",},
{"large lightning",      4, 13, 20, 12, 0, 0,  3,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_ELEC, "lightning_l",},
{"magic missile",            1, 1, 75, 3, 0, 0,  8,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_MISSILE, "magic_missile",},
{"create bomb",              6,10, 5, 20, 1, 1,  3,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_DETONATE, "bomb",},
{"summon golem",             2, 5, 10, 30, 1, 1,  8,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "golem",},
{"summon fire elemental",    7,25, 4,  40, 1, 1,  2,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "fire_elemental",},
{"summon earth elemental",   4,15, 10, 40, 1, 1,  3,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "earth_elemental",},
{"summon water elemental",   5,15, 8,  40, 1, 1,  4,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "water_elemental",},
{"summon air elemental",     6,20, 6,  40, 1, 1,  5,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "air_elemental",},
{"dimension door",          10,25, 8,  1, 0, 0,  1,  1, 0, 0, 0,
0,0,0,0,0,1,0,0,0,0,
 PATH_TELE, "enchantment",},
{"create earth wall",        4, 6, 12, 30, 0, 0,  6,  1, 1, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_CREATE, "earthwall",},
{"paralyze",             2, 5, 40, 8, 0, 0,  8,  1, 0, 0, 0,
0,0,0,0,1,0,0,0,1,0,
 PATH_NULL, "paralyze",},
{"icestorm",             1, 5, 15, 8, 0, 0,  4,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_FROST, "icestorm",},
{"magic mapping",        5,15, 20, 1, 2, 8,  5,  0, 0, 0, 1,
0,0,0,0,0,0,0,1,0,0,
 PATH_INFO, "enchantment",},
{"turn undead",          1, 2, 40, 5, 0, 0,  8,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_TURNING, "turn_undead",},
{"fear",             4, 6, 25, 5, 0, 0,  5,  1, 0, 0, 0,
0,0,0,0,1,0,0,0,1,0,
 PATH_MIND, "fear",},
{"poison cloud",         2, 5, 30, 10, 0, 0,  6,  1, 0, 0, 0,
0,1,0,0,1,0,0,0,0,0,
 PATH_MISSILE, "spellball",},
{"wonder",           3,10, 20, 0, 0, 0,  0,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_TRANSMUTE, "flowers",},
{"destruction",         18,30,  0, 20, 3, 10, 1,  1, 0, 0, 1,
0,1,0,0,0,0,0,0,0,0,
 PATH_NULL, "destruction",},
{"perceive self",        2, 5, 20, 0, 2, 2,  0,  0, 0, 1, 1,
0,0,0,0,0,0,0,1,0,0,
 PATH_INFO, "enchantment",},
{"word of recall",          10,40,  3, 50, 1, 2,  1,  0, 0, 1, 1,
0,0,0,0,0,1,0,1,0,0,
 PATH_TELE, "enchantment",},
{"invisible",            6,15,  0, 5, 3, 2,  4,  1, 1, 0, 1,
0,0,0,1,0,0,0,1,0,0,
 PATH_NULL, "enchantment",},
{"invisible to undead",      6,25,  0, 5, 1, 2,  2,  1, 1, 1, 1,
0,0,0,1,0,0,0,1,0,0,
 PATH_NULL, "enchantment",},
{"large bullet",         4, 3, 33, 6, 0, 0,  4,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_MISSILE, "lbullet",},
{"improved invisibility",    8,25,  0, 10, 1, 1,  1,  1, 1, 0, 1,
0,0,0,1,0,0,0,1,0,0,
 PATH_NULL, "enchantment",},
{"holy word",            1, 4,  0, 1, 0, 0,  4,  1, 0, 1, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_TURNING, "holy_word",},
{"medium healing",       4, 7, 20, 6, 0, 0,  5,  1, 1, 1, 1,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "healing",},
{"major healing",        8,10, 12, 9, 0, 0,  3,  1, 1, 1, 1,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "healing",},
{"heal",            10,50,  5, 12, 0, 0,  1,  1, 1, 1, 1,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "healing",},
{"create food",  6, 10,  0, 20, 0, 0,  4,  1, 1, 1, 0,
0,0,0,0,0,0,0,1,0,0,
 PATH_CREATE, "food",},
{"earth to dust",        2, 5,  0, 30, 0, 0,  2,  1, 1, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_NULL, "destruction",},
{"armour",           1, 8,  0, 20, 3, 2,  8,  1, 1, 0, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_SELF, "enchantment",},
{"strength",             2,10,  0, 20, 3, 2,  6,  1, 0, 0, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_SELF, "enchantment",},
{"dexterity",            3,12,  0, 20, 3, 2,  4,  1, 0, 0, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_SELF, "enchantment",},
{"constitution",         4,15,  0, 20, 3, 2,  4,  1, 1, 0, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_SELF, "enchantment",},
{"charisma",             3,12,  0, 20, 0, 0,  4,  1, 0, 0, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_SELF, "enchantment",},
{"create fire wall",         6, 5,  0, 10, 0, 0,  3,  1, 1, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_CREATE, "firebreath",},
{"create frost wall",        8, 8,  0, 10, 0, 0,  2,  1, 1, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_CREATE, "icestorm",},
{"protection from cold",     3,15,  0, 10, 1, 1,  3,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from electricity",  4,15,  0, 10, 1, 1,  3,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from fire",     5,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from poison",   6,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from slow",     7,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from paralysis",    8,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from draining",     9,25,  0, 30, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from magic",   10,30,  0, 30, 1, 1,  1,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from attack",  13,50,  0, 50, 1, 1,  1,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"levitate",             6,10,  0, 10, 1, 1,  2,  0, 0, 0, 1,
0,0,0,1,0,0,0,1,0,0,
 PATH_NULL, "enchantment",},
{"small speedball",              3, 3,  0, 20, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_MISSILE, "speedball",},
{"large speedball",      6, 6,  0, 40, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_MISSILE, "speedball",},
{"hellfire",             8,13,  0, 30, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_FIRE, "hellfire",},
{"dragonbreath",        12, 13,  0, 30, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_FIRE, "firebreath",},
{"large icestorm",      12,13,  0, 40, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_FROST, "icestorm",},
{"charging",            10,200, 0, 75, 1, 1,  0,  0, 0, 0, 1,
0,0,0,0,0,0,0,1,0,0,
 PATH_TRANSFER, "enchantment",},
#ifdef NO_POLYMORPH
{"polymorph",            6,20, 0, 30, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,1,0,
 PATH_TRANSMUTE, "polymorph",},
#else
{"polymorph",            6,20, 10, 30, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,1,0,
 PATH_TRANSMUTE, "polymorph",},
#endif
{"cancellation",        10,30, 10, 10, 0, 0,  1,  1, 0, 0, 0,
0,0,0,0,1,0,1,0,0,0,
 PATH_ABJURE, "cancellation",},

  {"mass confusion",         7,20, 15, 20, 0, 0,  3,  1, 0, 0, 0,
0,0,0,0,1,0,1,0,0,0,
 PATH_MIND, "confuse",},
{"summon pet monster",       2, 5, 15, 40, 3, 1,  8,  1, 0, 0, 0,
1,0,0,0,0,0,0,0,0,0,
 PATH_SUMMON, NULL,},
{"slow",             1, 5, 30, 5, 0, 0,  7,  1, 0, 0, 0,
0,0,0,0,1,0,1,0,0,0,
 PATH_NULL, "slow",},
{"regenerate spellpoints",      99, 0,  0, 0, 0, 0,  0,  0, 0, 0, 1,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, NULL,},
{"protection from confusion",    7,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from cancellation",11,30,  0, 10, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"protection from depletion",    7,20,  0, 10, 1, 1,  2,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"alchemy",          3, 5,  0, 15, 3, 2,  7,  1, 0, 0, 1,
0,0,0,0,0,0,0,1,0,0,
 PATH_TRANSMUTE, "enchantment",},
{"remove curse",         8,80,  0,100, 1, 3,  1,  1, 0, 1, 1,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "protection",},
{"remove damnation",        15,200, 0,200, 1, 1,  0,  1, 0, 1, 1,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "protection",},
{"detect monster",       2, 2,  0, 15, 3, 6,  8,  1, 1, 0, 1,
0,0,0,0,0,0,1,1,0,0,
 PATH_INFO, "detect_magic",},
{"detect evil",          3, 3,  0, 15, 3, 5,  8,  1, 1, 1, 1,
0,0,0,0,0,0,1,1,0,0,
 PATH_INFO, "detect_magic",},
{"heroism",         10,50,  0, 10, 0, 0,  0,  1, 0, 0, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_SELF, "enchantment",},
{"aggravation",          1, 0,  0, 1, 0, 0,  0,  0, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_NULL, NULL,},
{"firebolt",             2, 9, 35, 10, 0, 0,  4,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_FIRE, "firebolt",},
{"frostbolt",            3,12, 30, 10, 0, 0,  3,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_FROST, "frostbolt",},
{"shockwave",                   14,26,  0, 20, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_NULL, "shockwave",},
{"color spray",                 13,35,  0, 15, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_NULL, "color_spray",},
{"haste",                       12,50,  0, 10, 0, 0,  0,  1, 0, 0, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_SELF, "enchantment",},
{"face of death",               22, 80,  0, 15, 0, 0,  0,  1, 0, 1, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_DEATH, "face_of_death",},
{"ball lightning",               9,10, 30, 30, 1, 9,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_ELEC, "ball_lightning",},
{"meteor swarm",                12,30,  0, 30, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_MISSILE, "meteor",},
{"comet",                        8,15,  0, 20, 0, 0,  0,  1, 0, 0, 0,
0,1,0,0,0,0,0,0,0,0,
 PATH_MISSILE, NULL,},
{"mystic fist",                  5,10,  0, 15, 0, 0,  1,  1, 0, 0, 0,
1,0,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "mystic_fist",},
{"raise dead",                  10,150,  0, 60, 0, 0,  0,  1, 0, 1, 0,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "enchantment",},
{"resurrection",                20,250, 0, 180, 0, 0,  0,  0, 0, 1, 0,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "enchantment",},
{"reincarnation",               25,350, 0,100, 0, 0,  0,  0, 0, 1, 0,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "enchantment",},
{"immunity to cold",            6, 60,  0, 10, 0, 0,  0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to electricity",     8, 65,  0, 10, 0, 0,  0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to fire",            10,70,  0, 10, 0, 0,  0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to poison",          12,60,  0, 10, 0, 0,  0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to slow",            14,60,  0, 10, 0, 0,  0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to paralysis",        16,60,  0, 10, 0, 0, 0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to draining",         18,75,  0, 10, 0, 0, 0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to magic",           20,150,  0, 30, 0, 0,  0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"immunity to attack",          26,170,  0, 50, 0, 0, 0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"invulnerability",             80,225,  0, 30, 0, 0, 0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_NULL, "protection",},
{"defense",                     40,75,   0, 30, 0, 0, 0,  1, 1, 1, 1,
0,0,0,1,0,0,1,1,0,0,
 PATH_PROT, "protection",},
{"rune of fire",                4,10,    0, 30, 0, 0, 5,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_FIRE, "rune_fire",},
{"rune of frost",               6,12,    0, 30, 0, 0, 4,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_FROST, "rune_frost",},
{"rune of shocking",            8,14,    0, 30, 0, 0, 3,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_ELEC, "rune_shock",},
{"rune of blasting",           10,18,    0, 30, 0, 0, 2,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_DETONATE, "rune_blast",},
{"rune of death",              17,20,    0, 40, 0, 0, 1,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_DEATH, "rune_death",},
{"marking rune",                1,2,     0, 10, 0, 0, 5,  0, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_NULL, "rune_mark",},
{"build director",             10,30,    0, 30, 0, 0, 1,  1, 0, 0, 0,
1,0,0,0,0,0,0,0,0,0,
 PATH_CREATE, NULL,},
{"create pool of chaos",       10,10,   10, 15, 0, 0, 1,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_CREATE, "color_spray",},
{"build bullet wall",          12,35,    0, 35, 0, 0, 1,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_CREATE, NULL,},
{"build lightning wall",       14,40,    0, 40, 0, 0, 1,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_CREATE, NULL,},
{"build fireball wall",        16,45,    0, 45, 0, 0, 1,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_CREATE, NULL,},
{"magic rune",                  12,5,    0, 30, 0, 0, 1,  0, 0, 0, 0,
1,0,0,0,0,0,0,0,0,0,
 PATH_CREATE, "generic_rune",},
{"rune of magic drain",        14,30,    0, 30, 0, 0, 0,  1, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_TRANSFER, "rune_drain_magic",},
{"antimagic rune",              7,5,     0, 20, 0, 0, 1,  1, 0, 0, 0,
1,0,0,0,0,0,0,0,1,0,
 PATH_ABJURE, "rune_antimagic",},
{"rune of transferrence",       6,12,    0, 40, 0, 0, 1,  1, 0, 0, 0,
1,0,0,0,0,0,0,0,1,0,
 PATH_TRANSFER, "rune_transferrence",},
{"transferrence",               5,10,    0, 20, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_TRANSFER, "enchantment",},
{"magic drain",                12,20,    0, 1, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,1,0,0,0,1,0,
 PATH_TRANSFER, "enchantment",},
{"counterspell",                3,10,   20, 0, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,1,0,0,0,1,0,
 PATH_ABJURE, "counterspell",},
{"disarm",                  4,7,     0, 30, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_ABJURE, "enchantment",},
{"cure confusion",              7,8,     0, 15, 1, 4, 1,  1, 0, 1, 1,
0,0,1,0,0,0,1,1,0,0,
0,0,1,0,0,0,1,1,0,0,
 PATH_RESTORE, "healing",},
{"summon evil monster",         8,8,     0, 30, 0, 0, 0,  0, 0, 0, 0,
1,1,0,0,0,0,0,0,0,0,
 PATH_SUMMON, NULL,},
{"counterwall",                 8, 8,   30, 30, 0, 0, 1,  1, 0, 0, 0,
1,0,0,0,0,0,0,0,1,0,
 PATH_RESTORE, "counterspell",},
{"cause medium wounds",     3, 8,    0,  5, 0, 0, 2,  1, 0, 1, 0,
0,1,0,0,0,0,0,0,0,0,
PATH_WOUNDING,"cause_wounds",},
{"cause serious wounds",    5, 16,   0,  5, 0, 0, 2,  1, 0, 1, 0,
0,1,0,0,0,0,0,0,0,0,
PATH_WOUNDING,"cause_wounds",},
{"charm monsters",      5, 20,    0, 10, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,1,0,
PATH_MIND,"enchantment",},
{"banishment",          5, 10,   3, 10, 1, 1, 1,  1, 0, 1, 0,
0,1,0,0,0,0,0,0,0,0,
PATH_TURNING,"banishment",},
{"create missile",      1,5,    0, 20, 1, 1, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,1,0,0,
PATH_CREATE,"enchantment",},
{"show invisible",      7,10,   4, 20, 1, 1, 1,  1, 1, 1, 0,
0,0,0,0,0,0,0,0,0,0,
PATH_INFO,"enchantment",},
{"xray",            10,20,  0, 20, 1, 1, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
PATH_INFO,"enchantment",},
{"pacify",          4, 10,  1, 2, 0, 0, 3,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
PATH_MIND,"enchantment",},
{"summon fog",                   2, 5,  10, 10, 0, 0, 2,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
PATH_CREATE,"fog",},
{"steambolt",                    5, 10, 10, 10, 0, 0,  1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
PATH_FIRE, "steambolt",},
{"command undead",               4, 12,  0, 10, 0, 0, 3,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
PATH_MIND,"enchantment",},
{"holy orb",                    7, 12,  0, 5, 0, 0, 3,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
PATH_TURNING,"holy_orb",},
{"summon avatar",               10, 60, 0, 15, 0, 0, 1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "avatar",},
{"holy possession",             9, 30,  0, 10, 0, 0, 1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_ABJURE, "enchantment",},
{"bless",                        2, 8,  0, 5, 0, 0,  3,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_ABJURE, "enchantment",},
{"curse",                        2, 8,  0, 5, 0, 0,  2,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_ABJURE, "enchantment",},
{"regeneration",                 7, 15,  0, 10, 0, 0, 1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_ABJURE, "enchantment",},
{"consecrate",                    4, 35,  0, 50, 0, 0, 1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_ABJURE, "enchantment",},
{"summon cult monsters",          3, 12,  0, 10, 0, 0, 2,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_SUMMON, NULL,},
{"cause critical wounds",     7, 25,   0,  5, 0, 0, 0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"cause_wounds",},
{"holy wrath",            14, 40,   0,  5, 0, 0, 1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_TURNING,"holy_wrath",},
{"retributive strike",        26, 150,   0, 15, 0, 0, 0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"god_power",},
{"finger of death",       15, 50,   0,  5, 0, 0, 0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_DEATH, NULL,},
{"insect plague",         12, 40,   0,  5, 0, 0, 0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_SUMMON,"insect_plague",},
{"call holy servant",         5, 30,   0,  5, 0, 0, 3,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_SUMMON, "holy_servant",},
{"wall of thorns",        6, 20,   0, 5, 0, 0, 0,  0, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_CREATE, "thorns"},
{"staff to snake",        2, 8,   0, 5, 0, 0, 1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_CREATE, "snake_golem"},
{"light",                   1, 8, 20, 5, 0, 0, 3,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_LIGHT, "light"},
{"darkness",                5, 15, 10, 5, 0, 0, 1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_LIGHT, "darkness"},
{"nightfall",               16, 120,  0, 15, 0, 0, 0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_LIGHT, NULL},
{"daylight",                18, 120,  0, 15, 0, 0, 0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
  PATH_LIGHT, NULL},
{"sunspear",                    6, 8, 35, 8, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_LIGHT, "sunspear"},
{"faery fire",                  4, 10,  0, 15, 3, 2, 2,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_LIGHT, NULL},
{"cure blindness",              9, 30,  0, 10, 1, 1, 2,  1, 1, 1, 1,
0,0,0,0,0,0,0,0,0,0,
 PATH_RESTORE, "healing",},
{"dark vision",                 5, 10,  0, 12, 3, 2, 2,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_INFO, NULL},
{"bullet swarm",        7,  6,  0,  5, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_MISSILE,"bullet"},
{"bullet storm",           10,  8,  0,  5, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_MISSILE,"lbullet"},
{"cause many wounds",          12,  30,  0,  5, 0, 0, 0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_WOUNDING,"cause_wounds"},
{"small snowstorm",              1, 6, 40, 5, 0, 0,  8,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_FROST, "snowball_s",},
{"medium snowstorm",             3,10, 20, 10, 0, 0,  6,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_FROST, "snowball_m",},
{"large snowstorm",              5,16, 10, 15, 0, 0,  2,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_FROST, "snowball_l",},
{"cause red death",           12,100, 0, 10, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"ebola"},
{"cause flu",                  2, 10, 5, 10, 3, 2,  5,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"flu"},
{"cause black death",         15, 120, 0, 10, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_NULL,"bubonic_plague"},
{"cause leprosy",              5, 20, 0, 10, 1, 1,  5,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"leprosy"},
{"cause smallpox",            10, 85, 0, 10, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"smallpox"},
{"cause white death",              85,350, 0, 10, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"pneumonic_plague"},
{"cause anthrax",             12, 50, 0, 10, 1, 1,  1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"anthrax"},
{"cause typhoid",             8, 60, 0, 10, 1, 1,  1,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_WOUNDING,"typhoid"},
{"mana blast",             8, 10, 0, 15, 0, 0,  2,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_TRANSFER, "manablast", },
{"small manaball",         4, 12, 0,  9, 0, 0,  3,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_TRANSFER, "manabullet_s", },
{"medium manaball",        7, 20, 0, 18, 0, 0,  2,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_TRANSFER, "manabullet_m", },
{"large manaball",        10, 32, 0, 27, 0, 0,  1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_TRANSFER, "manabullet_l", },
{"mana bolt",              5, 18, 0,  9, 0, 0,  2,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_TRANSFER, "manabolt", },
{"dancing sword",         11, 25, 0, 10, 0, 0,  1,  0, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_CREATE, "dancingsword", },
{"animate weapon",         7, 25, 0, 10, 0, 0,  4,  0, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_TELE, "dancingsword", },
{"cause cold",                  2, 10, 5, 10, 3, 2,  5,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_WOUNDING,"disease_cold"},
{"divine shock",              1, 3, 0, 10, 0, 0, 0, 1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_WOUNDING,"divine_shock"},
{"windstorm",                   3,3,  0, 10, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_NULL, "windstorm",},
{"sanctuary",                 7, 30,  0, 10,  0,  0,  0,  0,  1,  1,  1,
0,0,0,0,0,0,0,0,0,0,
   PATH_PROT,"sanctuary"},
{"peace",                 20, 80,  0, 10,  0,  0,  0,  1,  0,  1,  0,
0,0,0,0,0,0,0,0,0,0,
   PATH_PROT,"peace"},
{"spiderweb",              4, 10,  0, 10,  0,  0,  0,  1,  0,  1,  0,
0,0,0,0,0,0,0,0,0,0,
   PATH_CREATE,"spiderweb_spell"},
{"conflict",              10, 50,  0, 10,  0,  0,  0,  1,  0,  1,  0,
0,0,0,0,0,0,0,0,0,0,
   PATH_MIND, "enchantment"},
{"rage",                   1,  5,  0, 10,  0,  0,  0,  1,  0,  1,  1,
0,0,0,0,0,0,0,0,0,0,
   PATH_WOUNDING, "enchantment"},
{"forked lightning",       5, 15,  0, 10,  0,  0,  0,  1,  0,  1,  0,
0,0,0,0,0,0,0,0,0,0,
   PATH_ELEC, "forked_lightning"},
{"poison fog",             5, 15,  0, 10,  0,  0,  0,  1,  0,  1,  0,
0,0,0,0,0,0,0,0,0,0,
   PATH_WOUNDING, "poison_fog"},
{"flaming aura",           1,  5,  0, 10,  0,  0,  0,  0,  1,  1,  1,
0,0,0,0,0,0,0,0,0,0,
   PATH_FIRE, "flaming_aura"},
{"vitriol",                5, 15,  0, 10,  0,  0,  0,  0,  1,  1,  1,
0,0,0,0,0,0,0,0,0,0,
   PATH_DETONATE, "vitriol"},
{"vitriol splash",                5, 15,  0, 10,  0,  0,  0,  0,  1,  1,  1,
0,0,0,0,0,0,0,0,0,0,
   PATH_DETONATE, "vitriol_splash"},
{"ironwood skin",    1, 8,  0, 20, 0, 0,  0,  1, 1, 1, 1,
0,0,0,0,0,0,0,0,0,0,
   PATH_SELF, "enchantment",},
{"wrathful eye",     5, 30,  0, 20, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_SELF, "wrathful_eye",},
{"town portal",         8, 30, 0, 10, 0, 0,  1,  0, 0, 0, 1,
0,0,0,0,0,0,0,0,0,0,
 PATH_TELE, "perm_magic_portal",},
{"missile swarm",       7,  6,  0,  3, 0, 0, 1,  1, 0, 0, 0,
0,0,0,0,0,0,0,0,0,0,
   PATH_MISSILE,"magic_missile"},
{"cause rabies",            12, 120, 0, 10, 0, 0,  0,  1, 0, 1, 0,
0,0,0,0,0,0,0,0,0,0,
 PATH_WOUNDING,"rabies"}
};
*/

char   *spellpathnames[NRSPELLPATHS]    =
{
    "Life", "Death", "Elemental", "Energy",
    "Spirit", "Protection", "Light", "Nether",
    "Nature", "Shadow", "Chaos", "Earth",
    "Conjuration", "Abjuration", "Transmutation", "Arcane"
};
