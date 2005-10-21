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

#include <global.h>

/* This Table is sorted in 64er blocks of the same base material, defined in
materialtype. Entrys, used for random selections should start from down of
a table section. Unique materials should start from above the 64 block down.
The M_RANDOM_xx value will always counted from down. */

/* this IS extrem ugly - i will move it ASAP to a data file, which can be used
* from editor too!
*/
material_real_struct    material_real[NROFMATERIALS *NROFMATERIALS_REAL + 1]    =
{
    /* undefined Material - for stuff we don't need material information about */
    {"", 100,100,       0,0,0,      M_NONE,         RACE_TYPE_NONE},

        /* PAPERS */
    {"paper ",       90,    80,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    81,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    82,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    83,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    84,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    85,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    86,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    87,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    88,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",       90,    89,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",   100,   90,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",   100,   91,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",   100,   92,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",   100,   93,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",   100,   94,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",   100,   95,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* IRON (= Metal) */
    {"iron ",                100,    80,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"hardened iron ",       95,     81,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"forged iron ",         90,     82,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"black iron ",          90,     83,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"shear iron ",          90,     84,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"steel ",               90,     85,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"hardened steel ",      85,     86,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"forged steel ",        85,     87,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"shear steel ",         85,     88,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"diamant steel ",       85,     89,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"darksteel ",           70,     90,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"forged darksteel ",    60,     91,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"silksteel ",           50,     92,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"forged silksteel ",    40,     93,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"meteoric steel ",      40,     94,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"forged meteoric steel ",40,     95,   0,0,0,          M_IRON,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Crystals/breakable/glass */
    {"glass ",       100,80,       0,0,0,   M_GLASS, RACE_TYPE_NONE}, /* 129 */
    {"zircon ",     100,80,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 130 */
    {"pearl ",       75,83,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 131 */
    {"emerald ",     75,85,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 132 */
    {"sapphire ",    50,92,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 133 */
    {"ruby ",        30,93,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 134 */
    {"diamond ",     10,95,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 135 */
    {"jasper ",         75,80,       0,0,0,      M_GLASS,         RACE_TYPE_NONE}, /* 136 */
    {"jade ",         75,80,       0,0,0,      M_GLASS,         RACE_TYPE_NONE}, /* 137 */
    {"aquamarine ",  80,80,       0,0,0,      M_GLASS,         RACE_TYPE_NONE}, /* 138 */
    {"opal ",         20,90,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 139 */
    {"amethyst ",         25,88,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 140 */
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* LEATHER */
    {"soft leather ",               100,    80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"hardened leather ",           70,     81,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"stone leather ",              70,     82,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"dark leather ",               35,     83,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"silk leather ",               30,     84,        0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"wyvern leather ",             50,     85,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"ankheg leather ",             50,     86,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"griffon leather ",            40,     87,        0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"hellcat leather ",            40,     88,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"gargoyle leather ",           35,     89,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"diamant leather ",            35,     90,       0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"chimera leather ",            30,     91,        0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"manticore leather ",          25,     92,        0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"cockatrice leather ",         20,     93,        0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"basilisk leather ",           20,     94,        0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"dragon leather ",             20,     95,        0,0,0,      M_LEATHER,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* WOOD */
    {"pine ",        100,80,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,81,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,82,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,83,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,84,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,85,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,86,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,87,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,88,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"pine ",        100,89,       0,0,0,         M_WOOD,         RACE_TYPE_NONE},
    {"oak ",             80,90,       0,0,0,          M_WOOD,         RACE_TYPE_NONE},
    {"oak ",             80,91,       0,0,0,          M_WOOD,         RACE_TYPE_NONE},
    {"oak ",             80,92,       0,0,0,          M_WOOD,         RACE_TYPE_NONE},
    {"oak ",             80,93,       0,0,0,          M_WOOD,         RACE_TYPE_NONE},
    {"oak ",             80,94,       0,0,0,          M_WOOD,         RACE_TYPE_NONE},
    {"oak ",             80,95,       0,0,0,          M_WOOD,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* ORGANIC */
    {"animal ",       100,80,       0,0,0,      M_ORGANIC,       RACE_TYPE_NONE}, /* 321 used for misc organics */
    {"dragon ",      50,96,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE}, /* 322 */
    {"chitin "              , 50,82,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE}, /* 323 */
    {"scale "               , 50,80,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 324 */
    {"white dragonscale "   , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 325 */
    {"blue dragonscale "    , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 326 */
    {"red dragonscale "     , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 327 */
    {"yellow dragonscale "  , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 328 */
    {"grey dragonscale "    , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 329 */
    {"black dragonscale "   , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 330 */
    {"orange dragonscale "  , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 331 */
    {"green dragonscale "   , 10,100,       0,0,0,      M_ORGANIC,         RACE_TYPE_NONE},/* 332 */
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* STONE */
    {"flint ",       100,80,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"pearl ",       100,85,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* CLOTH */
    {"wool ",       100,80,       0,0,0,      M_CLOTH,         RACE_TYPE_NONE},
    {"linen ",      90,80,        0,0,0,      M_CLOTH,         RACE_TYPE_NONE},
    {"silk ",       25,95,        0,0,0,      M_CLOTH,         RACE_TYPE_NONE},
    {"elven hair ",  25,95,        0,0,0,      M_CLOTH,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* ADAMANT (= magic metals) */
    {"magic silk ",  1,99,       0,0,0,      M_ADAMANT,         RACE_TYPE_NONE},
    {"mithril ",     1,99,       0,0,0,      M_ADAMANT,         RACE_TYPE_NONE},
    {"adamant ",     10,99,      0,0,0,      M_ADAMANT,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Liquid */
    /* some like ice... the kind of liquid/potions in game don't depend
    * or even handle the liquid base type*/
    {"",       100,80,       0,0,0,      M_LIQUID,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Soft Metal */
    {"tin ",         100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 641 */
    {"brass ",       100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 642 */
    {"copper ",      100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 643 */
    {"bronze ",      100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 644 */
    {"silver ",      50, 90,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 645 */
    {"gold ",        20, 95,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 646 */
    {"platinum ",    10, 99,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 647 */
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Bone */
    {"",        100,80,       0,0,0,      M_BONE,         RACE_TYPE_NONE}, /* for misc bones*/
    {"human ",          100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"elven ",          100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"dwarven ",        100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Ice */
    /* not sure about the sense to put here different elements in...*/
    {"",         100,80,       0,0,0,      M_ICE,         RACE_TYPE_NONE}, /* water */
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
};

materialtype            material[NROFMATERIALS]                             =
{
    /*            P  M  F  E  C  C  A  D  W  G  P S P T F  C D D C C G H B  I *
    *          H  A  I  L  O  O  C  R  E  H  O L A U E  A E E H O O O L  N *
    *          Y  G  R  E  L  N  I  A  A  O  I O R R A  N P A A U D L I  T *
    *          S  I  E  C  D  F  D  I  P  S  S W A N R  C L T O N   Y N  R *
    *          I  C     T     U     N  O  T  O   L      E E H S T P   D  N */
    {"paper",     {15,10,17, 9, 5, 7,13, 0,20,15, 0,0,0,0,0,10,0,0,0,0,0,0,0,0}},
    {"metal",     { 2,12, 3,12, 2,10, 7, 0,20,15, 0,0,0,0,0,10,0,0,0,0,0,0,0,0}},
    {"crystal",   {14,11, 8, 3,10, 5, 1, 0,20,15, 0,0,0,0,0, 0,0,0,0,0,0,0,0,0}},
    {"leather",   { 5,10,10, 3, 3,10,10, 0,20,15, 0,0,0,0,0,12,0,0,0,0,0,0,0,0}},
    {"wood",      {10,11,13, 2, 2,10, 9, 0,20,15, 0,0,0,0,0,12,0,0,0,0,0,0,0,0}},
    {"organics",  { 3,12, 9,11, 3,10, 9, 0,20,15, 0,0,0,0,0, 0,0,0,0,0,0,0,0,0}},
    {"stone",     { 2, 5, 2, 2, 2, 2, 1, 0,20,15, 0,0,0,0,0, 5,0,0,0,0,0,0,0,0}},
    {"cloth",     {14,11,13, 4, 4, 5,10, 0,20,15, 0,0,0,0,0, 5,0,0,0,0,0,0,0,0}},
    {"magic material",    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0, 0,0,0,0,0,0,0,0,0}},
    {"liquid",    { 0, 8, 9, 6,17, 0,15, 0,20,15,12,0,0,0,0,11,0,0,0,0,0,0,0,0}},
    {"soft metal",{ 6,12, 6,14, 2,10, 1, 0,20,15, 0,0,0,0,0,10,0,0,0,0,0,0,0,0}},
    {"bone",      {10, 9, 4, 5, 3,10,10, 0,20,15, 0,0,0,0,0, 2,0,0,0,0,0,0,0,0}},
    {"ice",       {14,11,16, 5, 0, 5, 6, 0,20,15, 0,0,0,0,0, 7,0,0,0,0,0,0,0,0}}
};

/* Damage an item a player is wearing */
void material_attack_damage(object *op, int num, int chance, int base)
{
    object *item;
    player *pl;
    int r, i, flag_fix = FALSE;

    if(op->type != PLAYER || !(pl = CONTR(op)))
        return;
   
    /*new_draw_info_format(NDI_UNIQUE, 0, op, "num: %d chance:%d base:%d", num,chance,base);*/
    flag_fix = FALSE;
    while(num--)
    {

        if(chance < (RANDOM()%100))
            continue;

        r = RANDOM() % PLAYER_EQUIP_MAX; /* get a random slot */
 
        /* get the first slot with something in using random start slot */
        for(i=0;i<PLAYER_EQUIP_MAX;i++)
        {
            if((item = pl->equipment[(r+i)%PLAYER_EQUIP_MAX]))
                break;
        }

        if(i==PLAYER_EQUIP_MAX) /* nothing there we can damage */
        {
            if(flag_fix)
                fix_player(op);
            return;
        }

        if(base & MATERIAL_BASE_SPECIAL)
        {
            /* if we miss a single immunity, special will do damage.
             * all 4 immunites = item can't be damaged
             */
            if( QUERY_FLAG(op, FLAG_PROOF_PHYSICAL) &&
                    QUERY_FLAG(op, FLAG_PROOF_MAGICAL) &&
                    QUERY_FLAG(op, FLAG_PROOF_ELEMENTAL) &&
                    QUERY_FLAG(op, FLAG_PROOF_SPHERICAL))
                continue;
        }
        else
        {
            if(base & MATERIAL_BASE_PHYSICAL && !QUERY_FLAG(op, FLAG_PROOF_PHYSICAL))
                goto mnaterial_dmg_jmp;
            if(base & MATERIAL_BASE_ELEMENTAL && !QUERY_FLAG(op, FLAG_PROOF_ELEMENTAL))
                goto mnaterial_dmg_jmp;
            if(base & MATERIAL_BASE_MAGICAL && !QUERY_FLAG(op, FLAG_PROOF_MAGICAL))
                goto mnaterial_dmg_jmp;
            if(base & MATERIAL_BASE_SPHERICAL && !QUERY_FLAG(op, FLAG_PROOF_SPHERICAL))
                goto mnaterial_dmg_jmp;

            continue; /* no missing immunity found - item has resisted */
        }
        mnaterial_dmg_jmp:
        /* we have an item, we can damage it - do it */

        /*new_draw_info_format(NDI_UNIQUE, 0, op, "DAMAGE ITEM:: %s (%d)", query_name(item), item->item_condition);*/
        /* sanity check */
        if(item->item_condition > 0)
            item->item_condition--;
                                                                     
        /* broken - unapply it - even its cursed */
        if(!item->item_condition)
            apply_special(op, item, AP_UNAPPLY | AP_IGNORE_CURSE);

        flag_fix = TRUE;
    }

    if(flag_fix)
        fix_player(op);
}

/* repair costs for item - owner is owner of that item */
sint64 material_repair_cost(object *item, object *owner)
{
    double tmp;
    sint64 costs=0;

    if(item->value && item->item_quality && item->item_quality > item->item_condition)
    {
        /* this is a problem.. we assume, that costs (as 64 bit value) will be covered
         * by tmp as double. This will work fine if costs is not insane high - what should
         * not be. If we have here problems, then we need to split this calc in a 64 bit one
         * with high values and small one
         */

        tmp = (double) item->value / (double)item->item_quality; /* how much cost is 1 point of quality */
        tmp *= (double)item->item_quality - (double)item->item_condition; /* number of condition we miss */
        costs = (sint64) tmp;
    }

    return costs;
}

/* repair the item */
void material_repair_item(object *item, int skill_value)
{
    
    if(!item->item_quality || item->item_quality <= item->item_condition)
        return;

    /* skill_value will determinate the quality OF the repair source */

    /* ATM we have disabled skill_value use - we always lose 1-3 points of quality permanent
     * if the condition is 20% or more under quality, set it to one 
     */
    if(item->item_quality/5 <= item->item_quality - item->item_condition)
    {
        int tmp = item->item_quality - (RANDOM()%3)+1;

        if(tmp <= 0)
            tmp = 1;

        item->item_quality = tmp;
    }
}