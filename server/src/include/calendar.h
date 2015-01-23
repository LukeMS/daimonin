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

#ifndef __CALENDAR_H
#define __CALENDAR_H

/* game time */
#define ARKHE_MES_PER_HR 60
#define ARKHE_HRS_PER_DY 24
#define ARKHE_DYS_PER_PK 3
#define ARKHE_PKS_PER_WK 3
#define ARKHE_WKS_PER_MH 3
#define ARKHE_MHS_PER_SN 3
#define ARKHE_SNS_PER_YR 4
#define ARKHE_YR         80
#define ARKHE_EYS_PER_MH 1

/* convenience */
#define ARKHE_MES_PER_DY (ARKHE_MES_PER_HR * ARKHE_HRS_PER_DY)
#define ARKHE_MES_PER_PK (ARKHE_MES_PER_DY * ARKHE_DYS_PER_PK)
#define ARKHE_MES_PER_WK (ARKHE_MES_PER_PK * ARKHE_PKS_PER_WK)
#define ARKHE_MES_PER_MH (ARKHE_MES_PER_WK * ARKHE_WKS_PER_MH)
#define ARKHE_MES_PER_SN ((ARKHE_MES_PER_MH + ARKHE_MES_PER_DY * \
                           ARKHE_EYS_PER_MH) * ARKHE_MHS_PER_SN)
#define ARKHE_MES_PER_YR (ARKHE_MES_PER_SN * ARKHE_SNS_PER_YR)
#define ARKHE_HRS_PER_PK (ARKHE_HRS_PER_DY * ARKHE_DYS_PER_PK)
#define ARKHE_HRS_PER_WK (ARKHE_HRS_PER_PK * ARKHE_PKS_PER_WK)
#define ARKHE_HRS_PER_MH (ARKHE_HRS_PER_WK * ARKHE_WKS_PER_MH)
#define ARKHE_HRS_PER_SN ((ARKHE_HRS_PER_MH + ARKHE_HRS_PER_DY * \
                           ARKHE_EYS_PER_MH) * ARKHE_MHS_PER_SN)
#define ARKHE_HRS_PER_YR (ARKHE_HRS_PER_SN * ARKHE_SNS_PER_YR)
#define ARKHE_DYS_PER_WK (ARKHE_DYS_PER_PK * ARKHE_PKS_PER_WK)
#define ARKHE_DYS_PER_MH (ARKHE_DYS_PER_WK * ARKHE_WKS_PER_MH)
#define ARKHE_DYS_PER_SN ((ARKHE_DYS_PER_MH + ARKHE_EYS_PER_MH) * \
                          ARKHE_MHS_PER_SN)
#define ARKHE_DYS_PER_YR (ARKHE_DYS_PER_SN * ARKHE_SNS_PER_YR)
#define ARKHE_PKS_PER_MH (ARKHE_PKS_PER_WK * ARKHE_WKS_PER_MH)
#define ARKHE_PKS_PER_SN (ARKHE_PKS_PER_MH * ARKHE_MHS_PER_SN)
#define ARKHE_PKS_PER_YR (ARKHE_PKS_PER_SN * ARKHE_SNS_PER_YR)
#define ARKHE_WKS_PER_SN (ARKHE_WKS_PER_MH * ARKHE_MHS_PER_SN)
#define ARKHE_WKS_PER_YR (ARKHE_WKS_PER_SN * ARKHE_SNS_PER_YR)
#define ARKHE_MHS_PER_YR (ARKHE_MHS_PER_SN * ARKHE_SNS_PER_YR)
#define ARKHE_EYS_PER_SN (ARKHE_EYS_PER_MH * ARKHE_MHS_PER_SN)
#define ARKHE_EYS_PER_YR (ARKHE_EYS_PER_SN * ARKHE_SNS_PER_YR)

/* Flags for print_tad() */
#define TAD_SHOWTIME   0X1
#define TAD_SHOWDATE   0X2
#define TAD_SHOWSEASON 0X4
#define TAD_LONGFORM   0X8

typedef struct _dayofyear
{
    sint8 season;
    sint8 month;
    sint8 week;
    sint8 parweek;
    sint8 day;
    sint8 intraholiday;
    sint8 extraholiday;
} dayofyear_t;

typedef struct _hourofday
{
    uint8 hour[ARKHE_HRS_PER_DY];
} hourofday_t;

typedef struct _timeanddate
{
    sint8       daylight_darkness;
    sint16      daylight_brightness;
    uint8       hour;
    uint8       minute;
    uint16      dayofyear;
    sint16      year;
    sint8       season;
    sint8       month;
    sint8       week;
    sint8       parweek;
    sint8       day;
    sint8       intraholiday;
    sint8       extraholiday;
    const char *season_name;
    const char *month_name;
    const char *parweek_name;
    const char *day_name;
    const char *intraholiday_name;
    const char *extraholiday_name;
} timeanddate_t;

extern uint16 tadtick;
extern timeanddate_t tadnow;

extern void    get_tad(timeanddate_t *tad, sint32 offset);
extern sint32  get_tad_offset_from_string(const char *string);
extern char   *print_tad(timeanddate_t *tad, int flags);
extern void    init_tadclock(void);
extern void    tick_tadclock(void);
extern void    write_tadclock(void);

#endif /* ifndef __CALENDAR_H */
