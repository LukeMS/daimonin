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

static const dayofyear_t Calendar[] =
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

static const char *SeasonName[] =
{
    "Season of Sowing",
    "Season of Toil",
    "Season of Bounty",
    "Season of Council",
};

static const char *MonthName[] =
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

static const char *ParweekName[] =
{
    "Dragocas",
    "Myracas",
    "Slocas",
};

static const char *DayName[] =
{
    "Alkudein",
    "Metadein",
    "Lappodein",
};

static const char *IntraholidayName[] =
{
    "TODO",
};

static const char *ExtraholidayName[] =
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

/* TODO: ATM one day repeats over entire year. */
static const sint8 Daylight[] =
{
    2,2,2,2,2,2,3,4,5,7,7,7,7,7,7,7,7,7,6,5,4,2,2,2,
};

uint16 tadtick; /* time of the day tick counter */
timeanddate_t tadnow;

/* Updates tad with the current time and date. */
void get_tad(timeanddate_t *tad, sint32 offset)
{
    sint16 hourofyear;
    uint8  hourofday;
    uint16 dayofyear;

    hourofyear = MAX(0 - ARKHE_HRS_PER_YR, MIN(tadtick + offset, ARKHE_HRS_PER_YR));

    if (hourofyear < 0)
    {
        hourofyear = ARKHE_HRS_PER_YR + hourofyear;
    }

    hourofday = hourofyear % ARKHE_HRS_PER_DY;
    dayofyear = hourofyear / ARKHE_HRS_PER_DY;

    /* Daylight (numbers) */
    if (hourofday != tad->hour)
    {
        sint8 v = Daylight[hourofday];

        tad->daylight_darkness = v;
        tad->daylight_brightness = brightness[ABS(v)];
    }

    /* Time (numbers) */
    tad->hour = hourofday;
    tad->minute = (uint8)((ROUND_TAG % PTICKS_PER_ARKHE_HOUR) /
        (PTICKS_PER_ARKHE_HOUR / (ARKHE_MES_PER_HR - 1)));

    /* Date (numbers) */
    if (dayofyear != tad->dayofyear)
    {
        tad->dayofyear = dayofyear;
        tad->year = ARKHE_YR; /* constant for now */
        tad->season = Calendar[dayofyear].season;
        tad->month = Calendar[dayofyear].month;
        tad->week = Calendar[dayofyear].week;
        tad->parweek = Calendar[dayofyear].parweek;
        tad->day = Calendar[dayofyear].day;
        tad->intraholiday = Calendar[dayofyear].intraholiday;
        tad->extraholiday = Calendar[dayofyear].extraholiday;

        /* Date (names) */
        tad->season_name = (tad->season >= 0) ? SeasonName[tad->season] : "";
        tad->month_name = (tad->month >= 0) ? MonthName[tad->month] : "";
        tad->parweek_name = (tad->parweek >= 0) ? ParweekName[tad->parweek] : "";
        tad->day_name = (tad->day >= 0) ? DayName[tad->day] : "";
        tad->intraholiday_name = (tad->intraholiday >= 0) ? IntraholidayName[tad->intraholiday] : "";
        tad->extraholiday_name = (tad->extraholiday >= 0) ? ExtraholidayName[tad->extraholiday] : "";
    }
}

/* get_tad_offset_from_string() parses a string for the offset parameter of
 * get_tad().
 *
 * This should only be used from a plugin as parsing a string is quite a lot of
 * work. */
sint32 get_tad_offset_from_string(const char *string)
{
    char   *cp = (char *)string;
    sint32  offset = 0;

    do
    {
        char    buf[MEDIUM_BUF];
        sint32  value;
        char   *endp;

        cp = get_token(cp, buf, 0);

        if (buf[0] != '\0' &&
            (value = strtol(buf, &endp, 10)))
        {
            if (*endp == '\0')
            {
                cp = get_token(cp, buf, 0);
                endp = buf;
            }

            if (*endp == '\0')
            {
                offset += value;
            }
            else
            {
                size_t len = strlen(endp);

                if (!strncasecmp("hours", endp, len))
                {
                    offset += value;
                }
                else if (!strncasecmp("days", endp, len))
                {
                    offset += value * ARKHE_HRS_PER_DY;
                }
                else if (!strncasecmp("parweeks", endp, len))
                {
                    offset += value * ARKHE_HRS_PER_PK;
                }
                else if (!strncasecmp("weeks", endp, len))
                {
                    offset += value * ARKHE_HRS_PER_WK;
                }
                else if (!strncasecmp("months", endp, len))
                {
                    offset += value * ARKHE_HRS_PER_MH;
                }
                else if (!strncasecmp("seasons", endp, len))
                {
                    offset += value * ARKHE_HRS_PER_SN;
                }
            }
        }
    }
    while (cp);

    return offset;
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

void init_tadclock(void)
{
    char  filename[MEDIUM_BUF];
    FILE *fp;

    sprintf(filename, "%s/clockdata", settings.localdir);
    LOG(llevSystem, "Reading clockdata from '%s'... ", filename);

    if (!(fp = fopen(filename, "r")))
    {
        LOG(llevSystem, "FAILED (could not open file, tadtick defaults to 0)!\n");
        tadtick = 0;
    }
    else
    {
        if (fscanf(fp, "%hu", &tadtick) != 1)
        {
            LOG(llevSystem, "FAILED (could not find correct data in file, tadtick defaults to 0)!\n");
            tadtick = 0;
        }
        else
        {
            LOG(llevSystem, "OK (tadtick=%hu)!\n", tadtick);

            /* So we don't add an hour every reboot. */
            if (tadtick > 0)
            {
                tadtick--;
            }
        }

        fclose(fp);
    }

    memset(&tadnow, 0, sizeof(timeanddate_t));
    tick_tadclock();
}

/* This performs the basic function of advancing the clock one tick forward.
 * Any game-time dependant functions should be called from this function. */
void tick_tadclock(void)
{
    if (++tadtick > ARKHE_HRS_PER_YR)
    {
        tadtick = 0;
    }

    /* save to disk once per game day. */
    if (tadtick % ARKHE_HRS_PER_DY == 0)
    {
        write_tadclock();
    }
}

/* Write out the current time to the file so time does not
 * reset every time the server reboots. */
void write_tadclock(void)
{
    char  filename[MEDIUM_BUF];
    FILE *fp;

    sprintf(filename, "%s/clockdata", settings.localdir);
    LOG(llevSystem, "Write tadclock() to '%s'... ", filename);

    if (!(fp = fopen(filename, "w")))
    {
        LOG(llevSystem, "FAILED (could not open file)!\n");
    }
    else
    {
        LOG(llevSystem, "OK!\n");
        fprintf(fp, "%hu", tadtick);
        fclose(fp);
    }
}
