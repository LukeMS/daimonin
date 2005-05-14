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
/* Copyright 2000 Tim Rightnour */

#ifndef _TOD_H_
#define _TOD_H_

#define PTICKS_PER_CLOCK    1500

/* game time */
#define HOURS_PER_DAY       24
#define DAYS_PER_WEEK       7
#define WEEKS_PER_MONTH     4
#define MONTHS_PER_YEAR     12
#define SEASONS_PER_YEAR    5

/* convenience */
#define WEEKS_PER_YEAR      (WEEKS_PER_MONTH*MONTHS_PER_YEAR)
#define DAYS_PER_MONTH      (DAYS_PER_WEEK*WEEKS_PER_MONTH)
#define DAYS_PER_YEAR       (DAYS_PER_MONTH*MONTHS_PER_YEAR)
#define HOURS_PER_WEEK      (HOURS_PER_DAY*DAYS_PER_WEEK)
#define HOURS_PER_MONTH     (HOURS_PER_WEEK*WEEKS_PER_MONTH)
#define HOURS_PER_YEAR      (HOURS_PER_MONTH*MONTHS_PER_YEAR)

#define LUNAR_DAYS      DAYS_PER_MONTH

typedef struct _timeofday
{
    int         year;
    int         month;
    int         day;
    int         dayofweek;
    int         hour;
    int         minute;
    int         weekofmonth;
    int         season;
    const char *dayofweek_name;
    const char *month_name;
    const char *season_name;
} timeofday_t;

/* from common/time.c */
extern void get_tod(timeofday_t *tod);
#endif /* _TOD_H_ */
