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
#include <global.h>

static const dayofyear_t calendar[] =
{
    {0,0,0,0,0,-1,-1}, {0,0,0,0,1,-1,-1}, {0,0,0,0,2,-1,-1},
    {0,0,0,1,0,-1,-1}, {0,0,0,1,1,-1,-1}, {0,0,0,1,2,-1,-1},
    {0,0,0,2,0,-1,-1}, {0,0,0,2,1,-1,-1}, {0,0,0,2,2,-1,-1},
    {0,0,1,0,0,-1,-1}, {0,0,1,0,1,-1,-1}, {0,0,1,0,2,-1,-1},
    {0,0,1,1,0,-1,-1}, {0,0,1,1,1,-1,-1}, {0,0,1,1,2,-1,-1},
    {0,0,1,2,0,-1,-1}, {0,0,1,2,1,-1,-1}, {0,0,1,2,2,-1,-1},
    {0,0,2,0,0,-1,-1}, {0,0,2,0,1,-1,-1}, {0,0,2,0,2,-1,-1},
    {0,0,2,1,0,-1,-1}, {0,0,2,1,1,-1,-1}, {0,0,2,1,2,-1,-1},
    {0,0,2,2,0,-1,-1}, {0,0,2,2,1,-1,-1}, {0,0,2,2,2,-1,-1},
    {0,-1,-1,-1,-1,-1,0},
    {0,1,0,0,0,-1,-1}, {0,1,0,0,1,-1,-1}, {0,1,0,0,2,-1,-1},
    {0,1,0,1,0,-1,-1}, {0,1,0,1,1,-1,-1}, {0,1,0,1,2,-1,-1},
    {0,1,0,2,0,-1,-1}, {0,1,0,2,1,-1,-1}, {0,1,0,2,2,-1,-1},
    {0,1,1,0,0,-1,-1}, {0,1,1,0,1,-1,-1}, {0,1,1,0,2,-1,-1},
    {0,1,1,1,0,-1,-1}, {0,1,1,1,1,-1,-1}, {0,1,1,1,2,-1,-1},
    {0,1,1,2,0,-1,-1}, {0,1,1,2,1,-1,-1}, {0,1,1,2,2,-1,-1},
    {0,1,2,0,0,-1,-1}, {0,1,2,0,1,-1,-1}, {0,1,2,0,2,-1,-1},
    {0,1,2,1,0,-1,-1}, {0,1,2,1,1,-1,-1}, {0,1,2,1,2,-1,-1},
    {0,1,2,2,0,-1,-1}, {0,1,2,2,1,-1,-1}, {0,1,2,2,2,-1,-1},
    {0,-1,-1,-1,-1,-1,1},
    {0,2,0,0,0,-1,-1}, {0,2,0,0,1,-1,-1}, {0,2,0,0,2,-1,-1},
    {0,2,0,1,0,-1,-1}, {0,2,0,1,1,-1,-1}, {0,2,0,1,2,-1,-1},
    {0,2,0,2,0,-1,-1}, {0,2,0,2,1,-1,-1}, {0,2,0,2,2,-1,-1},
    {0,2,1,0,0,-1,-1}, {0,2,1,0,1,-1,-1}, {0,2,1,0,2,-1,-1},
    {0,2,1,1,0,-1,-1}, {0,2,1,1,1,-1,-1}, {0,2,1,1,2,-1,-1},
    {0,2,1,2,0,-1,-1}, {0,2,1,2,1,-1,-1}, {0,2,1,2,2,-1,-1},
    {0,2,2,0,0,-1,-1}, {0,2,2,0,1,-1,-1}, {0,2,2,0,2,-1,-1},
    {0,2,2,1,0,-1,-1}, {0,2,2,1,1,-1,-1}, {0,2,2,1,2,-1,-1},
    {0,2,2,2,0,-1,-1}, {0,2,2,2,1,-1,-1}, {0,2,2,2,2,-1,-1},
    {0,-1,-1,-1,-1,-1,2},
    {1,3,0,0,0,-1,-1}, {1,3,0,0,1,-1,-1}, {1,3,0,0,2,-1,-1},
    {1,3,0,1,0,-1,-1}, {1,3,0,1,1,-1,-1}, {1,3,0,1,2,-1,-1},
    {1,3,0,2,0,-1,-1}, {1,3,0,2,1,-1,-1}, {1,3,0,2,2,-1,-1},
    {1,3,1,0,0,-1,-1}, {1,3,1,0,1,-1,-1}, {1,3,1,0,2,-1,-1},
    {1,3,1,1,0,-1,-1}, {1,3,1,1,1,-1,-1}, {1,3,1,1,2,-1,-1},
    {1,3,1,2,0,-1,-1}, {1,3,1,2,1,-1,-1}, {1,3,1,2,2,-1,-1},
    {1,3,2,0,0,-1,-1}, {1,3,2,0,1,-1,-1}, {1,3,2,0,2,-1,-1},
    {1,3,2,1,0,-1,-1}, {1,3,2,1,1,-1,-1}, {1,3,2,1,2,-1,-1},
    {1,3,2,2,0,-1,-1}, {1,3,2,2,1,-1,-1}, {1,3,2,2,2,-1,-1},
    {1,-1,-1,-1,-1,-1,3},
    {1,4,0,0,0,-1,-1}, {1,4,0,0,1,-1,-1}, {1,4,0,0,2,-1,-1},
    {1,4,0,1,0,-1,-1}, {1,4,0,1,1,-1,-1}, {1,4,0,1,2,-1,-1},
    {1,4,0,2,0,-1,-1}, {1,4,0,2,1,-1,-1}, {1,4,0,2,2,-1,-1},
    {1,4,1,0,0,-1,-1}, {1,4,1,0,1,-1,-1}, {1,4,1,0,2,-1,-1},
    {1,4,1,1,0,-1,-1}, {1,4,1,1,1,-1,-1}, {1,4,1,1,2,-1,-1},
    {1,4,1,2,0,-1,-1}, {1,4,1,2,1,-1,-1}, {1,4,1,2,2,-1,-1},
    {1,4,2,0,0,-1,-1}, {1,4,2,0,1,-1,-1}, {1,4,2,0,2,-1,-1},
    {1,4,2,1,0,-1,-1}, {1,4,2,1,1,-1,-1}, {1,4,2,1,2,-1,-1},
    {1,4,2,2,0,-1,-1}, {1,4,2,2,1,-1,-1}, {1,4,2,2,2,-1,-1},
    {1,-1,-1,-1,-1,-1,4},
    {1,5,0,0,0,-1,-1}, {1,5,0,0,1,-1,-1}, {1,5,0,0,2,-1,-1},
    {1,5,0,1,0,-1,-1}, {1,5,0,1,1,-1,-1}, {1,5,0,1,2,-1,-1},
    {1,5,0,2,0,-1,-1}, {1,5,0,2,1,-1,-1}, {1,5,0,2,2,-1,-1},
    {1,5,1,0,0,-1,-1}, {1,5,1,0,1,-1,-1}, {1,5,1,0,2,-1,-1},
    {1,5,1,1,0,-1,-1}, {1,5,1,1,1,-1,-1}, {1,5,1,1,2,-1,-1},
    {1,5,1,2,0,-1,-1}, {1,5,1,2,1,-1,-1}, {1,5,1,2,2,-1,-1},
    {1,5,2,0,0,-1,-1}, {1,5,2,0,1,-1,-1}, {1,5,2,0,2,-1,-1},
    {1,5,2,1,0,-1,-1}, {1,5,2,1,1,-1,-1}, {1,5,2,1,2,-1,-1},
    {1,5,2,2,0,-1,-1}, {1,5,2,2,1,-1,-1}, {1,5,2,2,2,-1,-1},
    {1,-1,-1,-1,-1,-1,5},
    {2,6,0,0,0,-1,-1}, {2,6,0,0,1,-1,-1}, {2,6,0,0,2,-1,-1},
    {2,6,0,1,0,-1,-1}, {2,6,0,1,1,-1,-1}, {2,6,0,1,2,-1,-1},
    {2,6,0,2,0,-1,-1}, {2,6,0,2,1,-1,-1}, {2,6,0,2,2,-1,-1},
    {2,6,1,0,0,-1,-1}, {2,6,1,0,1,-1,-1}, {2,6,1,0,2,-1,-1},
    {2,6,1,1,0,-1,-1}, {2,6,1,1,1,-1,-1}, {2,6,1,1,2,-1,-1},
    {2,6,1,2,0,-1,-1}, {2,6,1,2,1,-1,-1}, {2,6,1,2,2,-1,-1},
    {2,6,2,0,0,-1,-1}, {2,6,2,0,1,-1,-1}, {2,6,2,0,2,-1,-1},
    {2,6,2,1,0,-1,-1}, {2,6,2,1,1,-1,-1}, {2,6,2,1,2,-1,-1},
    {2,6,2,2,0,-1,-1}, {2,6,2,2,1,-1,-1}, {2,6,2,2,2,-1,-1},
    {2,-1,-1,-1,-1,-1,6},
    {2,7,0,0,0,-1,-1}, {2,7,0,0,1,-1,-1}, {2,7,0,0,2,-1,-1},
    {2,7,0,1,0,-1,-1}, {2,7,0,1,1,-1,-1}, {2,7,0,1,2,-1,-1},
    {2,7,0,2,0,-1,-1}, {2,7,0,2,1,-1,-1}, {2,7,0,2,2,-1,-1},
    {2,7,1,0,0,-1,-1}, {2,7,1,0,1,-1,-1}, {2,7,1,0,2,-1,-1},
    {2,7,1,1,0,-1,-1}, {2,7,1,1,1,-1,-1}, {2,7,1,1,2,-1,-1},
    {2,7,1,2,0,-1,-1}, {2,7,1,2,1,-1,-1}, {2,7,1,2,2,-1,-1},
    {2,7,2,0,0,-1,-1}, {2,7,2,0,1,-1,-1}, {2,7,2,0,2,-1,-1},
    {2,7,2,1,0,-1,-1}, {2,7,2,1,1,-1,-1}, {2,7,2,1,2,-1,-1},
    {2,7,2,2,0,-1,-1}, {2,7,2,2,1,-1,-1}, {2,7,2,2,2,-1,-1},
    {2,-1,-1,-1,-1,-1,7},
    {2,8,0,0,0,-1,-1}, {2,8,0,0,1,-1,-1}, {2,8,0,0,2,-1,-1},
    {2,8,0,1,0,-1,-1}, {2,8,0,1,1,-1,-1}, {2,8,0,1,2,-1,-1},
    {2,8,0,2,0,-1,-1}, {2,8,0,2,1,-1,-1}, {2,8,0,2,2,-1,-1},
    {2,8,1,0,0,-1,-1}, {2,8,1,0,1,-1,-1}, {2,8,1,0,2,-1,-1},
    {2,8,1,1,0,-1,-1}, {2,8,1,1,1,-1,-1}, {2,8,1,1,2,-1,-1},
    {2,8,1,2,0,-1,-1}, {2,8,1,2,1,-1,-1}, {2,8,1,2,2,-1,-1},
    {2,8,2,0,0,-1,-1}, {2,8,2,0,1,-1,-1}, {2,8,2,0,2,-1,-1},
    {2,8,2,1,0,-1,-1}, {2,8,2,1,1,-1,-1}, {2,8,2,1,2,-1,-1},
    {2,8,2,2,0,-1,-1}, {2,8,2,2,1,-1,-1}, {2,8,2,2,2,-1,-1},
    {2,-1,-1,-1,-1,-1,8},
    {3,9,0,0,0,-1,-1}, {3,9,0,0,1,-1,-1}, {3,9,0,0,2,-1,-1},
    {3,9,0,1,0,-1,-1}, {3,9,0,1,1,-1,-1}, {3,9,0,1,2,-1,-1},
    {3,9,0,2,0,-1,-1}, {3,9,0,2,1,-1,-1}, {3,9,0,2,2,-1,-1},
    {3,9,1,0,0,-1,-1}, {3,9,1,0,1,-1,-1}, {3,9,1,0,2,-1,-1},
    {3,9,1,1,0,-1,-1}, {3,9,1,1,1,-1,-1}, {3,9,1,1,2,-1,-1},
    {3,9,1,2,0,-1,-1}, {3,9,1,2,1,-1,-1}, {3,9,1,2,2,-1,-1},
    {3,9,2,0,0,-1,-1}, {3,9,2,0,1,-1,-1}, {3,9,2,0,2,-1,-1},
    {3,9,2,1,0,-1,-1}, {3,9,2,1,1,-1,-1}, {3,9,2,1,2,-1,-1},
    {3,9,2,2,0,-1,-1}, {3,9,2,2,1,-1,-1}, {3,9,2,2,2,-1,-1},
    {3,-1,-1,-1,-1,-1,9},
    {3,10,0,0,0,-1,-1}, {3,10,0,0,1,-1,-1}, {3,10,0,0,2,-1,-1},
    {3,10,0,1,0,-1,-1}, {3,10,0,1,1,-1,-1}, {3,10,0,1,2,-1,-1},
    {3,10,0,2,0,-1,-1}, {3,10,0,2,1,-1,-1}, {3,10,0,2,2,-1,-1},
    {3,10,1,0,0,-1,-1}, {3,10,1,0,1,-1,-1}, {3,10,1,0,2,-1,-1},
    {3,10,1,1,0,-1,-1}, {3,10,1,1,1,-1,-1}, {3,10,1,1,2,-1,-1},
    {3,10,1,2,0,-1,-1}, {3,10,1,2,1,-1,-1}, {3,10,1,2,2,-1,-1},
    {3,10,2,0,0,-1,-1}, {3,10,2,0,1,-1,-1}, {3,10,2,0,2,-1,-1},
    {3,10,2,1,0,-1,-1}, {3,10,2,1,1,-1,-1}, {3,10,2,1,2,-1,-1},
    {3,10,2,2,0,-1,-1}, {3,10,2,2,1,-1,-1}, {3,10,2,2,2,-1,-1},
    {3,-1,-1,-1,-1,-1,10},
    {3,11,0,0,0,-1,-1}, {3,11,0,0,1,-1,-1}, {3,11,0,0,2,-1,-1},
    {3,11,0,1,0,-1,-1}, {3,11,0,1,1,-1,-1}, {3,11,0,1,2,-1,-1},
    {3,11,0,2,0,-1,-1}, {3,11,0,2,1,-1,-1}, {3,11,0,2,2,-1,-1},
    {3,11,1,0,0,-1,-1}, {3,11,1,0,1,-1,-1}, {3,11,1,0,2,-1,-1},
    {3,11,1,1,0,-1,-1}, {3,11,1,1,1,-1,-1}, {3,11,1,1,2,-1,-1},
    {3,11,1,2,0,-1,-1}, {3,11,1,2,1,-1,-1}, {3,11,1,2,2,-1,-1},
    {3,11,2,0,0,-1,-1}, {3,11,2,0,1,-1,-1}, {3,11,2,0,2,-1,-1},
    {3,11,2,1,0,-1,-1}, {3,11,2,1,1,-1,-1}, {3,11,2,1,2,-1,-1},
    {3,11,2,2,0,-1,-1}, {3,11,2,2,1,-1,-1}, {3,11,2,2,2,-1,-1},
    {3,-1,-1,-1,-1,-1,11},
};

static const char *season_name[] =
{
    "Season of Sowing",
    "Season of Toil",
    "Season of Bounty",
    "Season of Council",
};

static const char *month_name[] =
{
    "Os-Nunnos",
    "Os-Minis",
    "Os-Kama",
    "Os-Faras",
    "Os-Moab",
    "Os-Shclar",
    "Os-Elaine",
    "Os-Tinath",
    "Os-Tabernacle",
    "Os-Catha",
    "Os-Vieras",
    "Os-Pwyllo",
};

static const char *parweek_name[] =
{
    "Dragocas",
    "Myracas",
    "Slocas",
};

static const char *day_name[] =
{
    "Alkudein",
    "Metadein",
    "Lappodein",
};

/*
static const char *intraholiday_name[] =
{
};
*/

static const char *extraholiday_name[] =
{
    "Heos-Nunnos",
    "Heos-Minis",
    "Heos-Kama",
    "Heos-Faras",
    "Heos-Moab",
    "Heos-Shclar",
    "Heos-Elaine",
    "Heos-Tinath",
    "Heos-Tabernacle",
    "Heos-Catha",
    "Heos-Vieras",
    "Heos-Pwyllo",
};

/* These are in 7-day periods. */
static const hourofday_t season_timechange[] =
{
    {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}}, {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}},
    {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}}, {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}},

    {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}}, {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}},
    {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}}, {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}},

    {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}}, {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}},
    {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}}, {{2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2}},

    {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}}, {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}},
    {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}}, {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}},

    {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}}, {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}},
    {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}}, {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}},

    {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}}, {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}},
    {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}}, {{2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,7,7,7,7,6,5,4,3,2}},

    {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}}, {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}},
    {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}}, {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}},

    {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}}, {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}},
    {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}}, {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}},

    {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}}, {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}},
    {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}}, {{2,2,2,2,2,2,3,4,5,6,7,7,7,7,7,7,7,7,6,5,4,3,2,2}},

    {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}}, {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}},
    {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}}, {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}},

    {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}}, {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}},
    {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}}, {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}},

    {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}}, {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}},
    {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}}, {{2,2,2,2,2,2,2,3,4,6,7,7,7,7,7,7,7,6,5,4,3,2,2,2}},
};

uint64 tadtick; /* time of the day tick counter */
int    world_darkness; /* daylight value. 0= totally dark. 7= daylight */

/* Updates tad with the current time and date. */
void get_tad(timeanddate_t *tad, sint32 offset)
{
    uint64 hour = tadtick + offset;
    uint16 day;

    memset(tad, 0, sizeof(timeanddate_t));

    /* Time (numbers) */
    tad->hour = hour % ARKHE_HRS_PER_DY;
    tad->minute = (uint8)((ROUND_TAG % PTICKS_PER_ARKHE_HOUR) /
                  (PTICKS_PER_ARKHE_HOUR / (ARKHE_MES_PER_HR - 1)));

    /* Date (numbers) */
    day = (hour % ARKHE_HRS_PER_YR) / ARKHE_HRS_PER_DY;
    tad->year = ARKHE_YR; /* constant for now */
    tad->season = calendar[day].season;
    tad->month = calendar[day].month;
    tad->week = calendar[day].week;
    tad->parweek = calendar[day].parweek;
    tad->day = calendar[day].day;
    tad->intraholiday = calendar[day].intraholiday; /* TODO */
    tad->extraholiday = calendar[day].extraholiday;

    /* Date (names) */
    tad->season_name = (tad->season >= 0) ? season_name[tad->season] : "";
    tad->month_name = (tad->month >= 0) ? month_name[tad->month] : "";
    tad->parweek_name = (tad->parweek >= 0) ? parweek_name[tad->parweek] : "";
    tad->day_name = (tad->day >= 0) ? day_name[tad->day] : "";
/*    tad->intraholiday_name = (tad->intraholiday >= 0) ?
                             intraholiday_name[tad->intraholiday] : ""; *//* TODO */
	tad->intraholiday_name = ""; // hotfix until intraholiday_name[] is declared right
	tad->extraholiday_name = (tad->extraholiday >= 0) ?
                             extraholiday_name[tad->extraholiday] : "";
}

/* Writes tad to errmsg according to flags.
 * flags are:
 *   TAD_SHOWTIME: show the time
 *   TAD_SHOWDATE: show the date
 *   TAD_SHOWSEASON: show the season
 *   TAD_LONGFORM: long format
 * Normal days, extra holidays, and intra holidays can all be expressed by this
 * function, in both long and short format, Time and date can be output
 * together or independently, and season is optional with any date.
 * The 6 formats are:
 * normal, long:
 * [<hour>:<minute>[ on ]][<day_name>, <parweek_name> <week (1-3)>, <month_name>[, <season_name>] in the Year <year> [After|Before] Empire]
 * intra, long:
 * [<hour>:<minute>[ on ]][<intraholiday_name>, <parweek_name> <week (1-3)>, <month_name>[, <season_name>] in the Year <year> [After|Before] Empire]
 * extra, long:
 * [<hour>:<minute>[ on ]][<extraholiday_name>[, <season_name>] in the Year <year> [After|Before] Empire]
 * normal, short:
 * [<hour>:<minute>[ ]][<day-in-month (1-27)>/<month (1-12)>[-<season (1-4)>] <year>[A|B]E]
 * intra, short:
 * [<hour>:<minute>[ ]][<day-in-month (1-27)>/<month (1-12)>[-<season (1-4)>] <year>[A|B]E]
 * extra, short:
 * [<hour>:<minute>[ ]][28/<month (1-12)>[-<season (1-4)>] <year>[A|B]E] */
char *print_tad(timeanddate_t *tad, int flags)
{
    if (!tad)
        return NULL;

    *errmsg = '\0';

    if ((flags & TAD_SHOWTIME))
    {
        sprintf(strchr(errmsg, '\0'),"%02d:%02d", tad->hour, tad->minute);

        /* ' on ' / ' ' */
        if ((flags & TAD_SHOWDATE))
            sprintf(strchr(errmsg, '\0'), "%s",
                    ((flags & TAD_LONGFORM)) ? " on " : " ");
    }

    if ((flags & TAD_SHOWDATE))
    {
        if ((flags & TAD_LONGFORM))
        {
            if (tad->intraholiday == -1 &&
                tad->extraholiday == -1) /* normal */
                sprintf(strchr(errmsg, '\0'), "%s, %s %d, %s",
                        tad->day_name, tad->parweek_name, tad->week + 1,
                        tad->month_name);
            else if (tad->intraholiday != -1) /* intra */
                sprintf(strchr(errmsg, '\0'), "%s, %s %d, %s",
                        tad->intraholiday_name, tad->parweek_name,
                        tad->week + 1, tad->month_name);
            else /* extra */
                sprintf(strchr(errmsg, '\0'), "%s",
                        tad->extraholiday_name);

            if ((flags & TAD_SHOWSEASON))
                sprintf(strchr(errmsg, '\0'), ", %s", tad->season_name);

            sprintf(strchr(errmsg, '\0'), " in the Year %d %s Empire",
                    ABS(tad->year), (tad->year >= 0) ? "After" : "Before");
        }
        else
        {
            sprintf(strchr(errmsg, '\0'), "%d/%d",
                    (tad->extraholiday != -1) ?
                    (ARKHE_DYS_PER_MH + ARKHE_EYS_PER_MH) *
                    (tad->extraholiday + 1) :
                    (tad->day + 1) * (tad->parweek + 1) * (tad->week + 1),
                    tad->month + 1);

            if ((flags & TAD_SHOWSEASON))
                sprintf(strchr(errmsg, '\0'), "-%d", tad->season + 1);

            sprintf(strchr(errmsg, '\0'), " %d%sE",
                    ABS(tad->year), (tad->year >= 0) ? "A" : "B");
        }
    }

    return errmsg;
}

/* This performs the basic function of advancing the clock one tick forward.
 * Any game-time dependant functions should be called from this function. */
void tick_tadclock(void)
{
    uint16 day,
           hour;

    tadtick++;
    day = (tadtick % ARKHE_HRS_PER_YR) / ARKHE_HRS_PER_DY;
    hour = tadtick % ARKHE_HRS_PER_DY;

    /* save to disk once per game day. */
    if (hour == 0)
        write_tadclock();

    world_darkness = season_timechange[(int)(day / 7)].hour[hour];
}

/* Write out the current time to the file so time does not
 * reset every time the server reboots. */
void write_tadclock(void)
{
    char  filename[MEDIUM_BUF];
    FILE *fp;

    LOG(llevInfo, "write tadclock()...\n");
    sprintf(filename, "%s/clockdata", settings.localdir);

    if (!(fp = fopen(filename, "w")))
    {
        LOG(llevBug, "BUG: Cannot open %s for writing\n", filename);

        return;
    }

#ifdef WIN32
    fprintf(fp, "%I64u", tadtick);
#elif SIZEOF_LONG == 8
    fprintf(fp, "%lu", tadtick);
#elif SIZEOF_LONG_LONG == 8
    fprintf(fp, "%llu", tadtick);
#endif
    fclose(fp);
}
