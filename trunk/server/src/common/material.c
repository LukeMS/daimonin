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
    {"paper ",         90,80,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,81,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,82,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,83,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,84,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,85,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,86,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,87,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"paper ",         90,88,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"rice paper ",    90,89,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",    100,90,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",    100,91,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"parchment ",    100,92,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"vellum ",       100,93,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"demon vellum ", 100,94,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"papyrus ",      100,95,       0,0,0,      M_PAPER,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* IRON (= Metal) (65) */
    {"iron ",                   100,80,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"hardened iron ",           95,81,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"forged iron ",             90,82,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"black iron ",              80,83,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"shear iron ",              70,84,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"steel ",                   60,85,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"hardened steel ",          50,86,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"forged steel ",            45,87,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"shear steel ",             40,88,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"diamant steel ",           40,89,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"darksteel ",               35,90,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"forged darksteel ",        30,91,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"silksteel ",               25,92,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"forged silksteel ",        25,93,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"meteoric steel ",          20,94,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"forged meteoric steel ",   20,95,   0,0,0,    M_IRON,    RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Crystals/breakable/glass (129) */
    {"glass ",      100,80,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 129 */
    {"zircon ",     100,80,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 130 */
    {"pearl ",       75,83,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 131 */
    {"emerald ",     75,85,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 132 */
    {"sapphire ",    50,92,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 133 */
    {"ruby ",        30,93,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 134 */
    {"diamond ",     10,95,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 135 */
    {"jasper ",      75,80,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 136 */
    {"jade ",        75,80,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 137 */
    {"aquamarine ",  80,80,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 138 */
    {"opal ",        20,90,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 139 */
    {"amethyst ",    25,88,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 140 */
    {"amber ",       30,88,       0,0,0,    M_GLASS, RACE_TYPE_NONE}, /* 141 */
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* LEATHER (193) */
    {"soft leather ",         100,80,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"hardened leather ",      95,81,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"stone leather ",         90,82,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"dark leather ",          80,83,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"silk leather ",          70,84,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"wyvern leather ",        60,85,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"ankheg leather ",        50,86,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"griffon leather ",       45,87,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"hellcat leather ",       40,88,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"gargoyle leather ",      35,89,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"diamant leather ",       35,90,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"chimera leather ",       30,91,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"manticore leather ",     25,92,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"cockatrice leather ",    20,93,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"basilisk leather ",      20,94,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"dragon leather ",        20,95,    0,0,0,    M_LEATHER,   RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* WOOD (257) */
    {"pine ",        100,80,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"spruce ",       95,81,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"cedar ",        90,82,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"ash ",          85,83,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"yew ",          80,84,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"birch ",        75,85,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"hickory ",      70,86,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"cherry ",       65,87,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"walnut ",       60,88,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"maple ",        55,89,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"elm ",          50,90,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"red elm ",      45,91,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"rosewood ",     40,92,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"pernambuco ",   35,93,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"white oak ",    30,94,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"red oak ",      20,95,       0,0,0,       M_WOOD,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* ORGANIC (321) */
    {"organic ",       100,80,       0,0,0,      M_ORGANIC,       RACE_TYPE_NONE}, /* 321 used for misc organics */
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
    /* STONE (385) */
    {"flint ",      100,80,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"greenstone ",  95,81,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"basalt ",      90,82,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"limestone ",   85,83,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"shale ",       80,84,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"gypsum ",      75,85,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"marble ",      70,86,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"granite ",     60,87,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"quartz ",      50,88,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"obsidian ",    40,89,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"brimstone ",   30,90,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"bloodstone ",  20,91,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"pyrite ",      20,92,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"lignite ",     20,93,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"cinnabar ",    20,94,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"phosphate ",   20,95,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"galena ",      20,95,       0,0,0,      M_STONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* CLOTH (449) */
    {"wool ",          100,80,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"linen ",          90,81,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"rat hair ",       85,82,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"dog hair ",       80,83,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"cat hair ",       75,84,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},    
    {"mugwump hair ",   70,85,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"wildcat hair ",   65,86,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"wolf hair ",      60,87,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"horse hair ",     55,88,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"giant hair ",     50,89,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"minotaur hair ",  40,90,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"worm silk ",      30,91,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"spider silk ",    25,92,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"angel hair ",     20,93,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"unicorn hair ",   15,94,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"elven hair ",     10,95,       0,0,0,      M_CLOTH,       RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* ADAMANT (= magic metals) (513) */
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
    /* Liquid (577) */
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
    /* Soft Metal (641) */
    {"tin ",         100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 641 */
    {"brass ",       100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 642 */
    {"copper ",      100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 643 */
    {"bronze ",      100,80,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 644 */
    {"silver ",      50, 90,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 645 */
    {"gold ",        20, 95,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 646 */
    {"platinum ",    10, 99,       0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 647 */
    {"lead ",        100, 80,      0,0,0,   M_SOFT_METAL,   RACE_TYPE_NONE}, /* 648 */
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Bone (705) */
    {"",            100,80,       0,0,0,      M_BONE,         RACE_TYPE_NONE}, /* for misc bones*/
    {"human ",      100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"elven ",      100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"dwarven ",    100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"rat ",        100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"cat ",        100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"dog ",        100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"kolbold ",    100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"ogre ",       100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"orc ",        100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"gnoll ",      100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"giant ",      100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"troll ",      100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"minotaur ",   100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"dragon ",     100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"demon ",      100,80,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    {"", 0,0,       0,0,0,      M_NONE,         RACE_TYPE_NONE},
    /* Ice (769) */
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
    /*              P  M  F  E  C  C  A  D  W  G  P S P T F  C D D C C G H B  I *
    *               H  A  I  L  O  O  C  R  E  H  O L A U E  A E E H O O O L  N *
    *               Y  G  R  E  L  N  I  A  A  O  I O R R A  N P A A U D L I  T *
    *               S  I  E  C  D  F  D  I  P  S  S W A N R  C L T O N   Y N  R *
    *               I  C     T     U     N  O  T  O   L      E E H S T P   D  N */
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
    char buf[HUGE_BUF];

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
            {
                /* only damage weapons? */
                if((base & HIT_FLAG_WEAPON) && item->type != WEAPON && item->type != SHIELD)
                    continue;
                break;
            }
        }

        if(i==PLAYER_EQUIP_MAX) /* nothing there we can damage */
        {
            if(flag_fix)
                FIX_PLAYER(op ,"material_attack_damage - leave");
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

        sprintf(buf, "Your %s is damaged.", query_short_name(item, NULL));
        new_draw_info(NDI_UNIQUE, 0, op, buf);
        esrv_update_item(UPD_QUALITY, op, item);
        /* broken - unapply it - even its cursed */
        if(!item->item_condition)
            apply_special(op, item, AP_UNAPPLY | AP_IGNORE_CURSE);

        flag_fix = TRUE;
    }

    if(flag_fix)
        FIX_PLAYER(op ,"material_attack_damage - end");
}

/* repair costs for item - owner is owner of that item */
sint64 material_repair_cost(object *item, object *owner)
{
    double tmp;
    sint64 costs=0;

    if(item->value && item->item_quality && item->item_quality > item->item_condition)
    {
        sint64 value = item->value;
        /* this is a problem.. we assume, that costs (as 64 bit value) will be covered
         * by tmp as double. This will work fine if costs is not insane high - what should
         * not be. If we have here problems, then we need to split this calc in a 64 bit one
         * with high values and small one
         */
        if(value < item->item_quality)
            value = (sint64) item->item_quality;

        tmp = (double) value / (double)item->item_quality; /* how much cost is 1 point of quality */
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

    /* lose a quality point if the repair is heavy */
    if(item->item_quality/5 <= item->item_quality - item->item_condition && !(RANDOM()%3))
    {
        /* adjust item value because we lose quality */
        if(item->value < (sint64)(item->item_quality*10000))
        {
            /* float to cover small changes in the value */
            float tmp = ((float)item->value) / (float)item->item_quality;
            item->value -= (sint64) tmp;
        }
        else
        {
            item->value -= (item->value / (sint64) item->item_quality);
        }

        item->item_quality--;
    }
    item->item_condition = item->item_quality; /* finally repair the shit */
}
