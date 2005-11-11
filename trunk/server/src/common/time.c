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
#include <tod.h>

#ifndef WIN32 /* ---win32 exclude header */
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#endif /* win32 */

const char *season_name[SEASONS_PER_YEAR]               =
{
    "The Season of New Year", "The Season of Growth", "The Season of Harvest", "The Season of Decay",
    "The Season of the Blizzard"
};

const char *weekdays[DAYS_PER_WEEK]     =
{
    "the Day of the Moon", "the Day of the Bull", "the Day of the Deception", "the Day of Thunder",
    "the Day of Freedom", "the Day of the Great Gods", "the Day of the Sun"
};

const char *month_name[MONTHS_PER_YEAR] =
{
    "Month of the Ice Dragon", "Month of the Frost Giant", "Month of the Clouds", "Month of Gaea",
    "Month of the Harvest", "Month of Futility", "Month of the Dragon", "Month of the Sun", "Month of the Falling",
    "Month of the Dark Shades", "Month of the Great Infernus", "Month of the Ancient Darkness",
};

/*
 * Initialise all variables used in the timing routines.
 */

void reset_sleep()
{
    GETTIMEOFDAY(&last_time);
}


/* Generic function for simple timeval arithmetic (addition & subtraction) */
static inline void add_time(struct timeval *dst, struct timeval *a, struct timeval *b)
{
    dst->tv_sec = a->tv_sec + b->tv_sec;
    dst->tv_usec = a->tv_usec + b->tv_usec;

    if(dst->tv_sec < 0 || (dst->tv_sec == 0 && dst->tv_usec < 0))
    {
        while(dst->tv_usec < -1000000) {
            dst->tv_sec -= 1;
            dst->tv_usec += 1000000;
        }
    } else
    {
        while(dst->tv_usec < 0) {
            dst->tv_sec -= 1;
            dst->tv_usec += 1000000;
        }
        while(dst->tv_usec > 1000000) {
            dst->tv_sec += 1;
            dst->tv_usec -= 1000000;
        }
    }
}

/* Calculate time until the next tick
 * returns 0 and steps forward time for the next tick if called
 * after the time for the next tick,
 * otherwise returns 1 and the delta time for next tick */
int time_until_next_tick(struct timeval *out)
{
    struct timeval now, next_tick, tick_time;

    /* next_tick = last_time + tick_time */
    tick_time.tv_sec = 0;
    tick_time.tv_usec = pticks_ums;

    add_time(&next_tick, &last_time, &tick_time);

    GETTIMEOFDAY(&now);

    /* Time for the next tick? (timercmp does not work for <= / >=) */
    /* if(timercmp(&next_tick, &now, <) || timercmp(&next_tick, &now, ==)) */

    /* timercmp() seems be broken under windows. Well, this is even faster */
    if( next_tick.tv_sec < now.tv_sec ||
        (next_tick.tv_sec == now.tv_sec && next_tick.tv_usec <= now.tv_usec))
    {
        /* this must be now time and not next_tick.
         * IF the last tick was really longer as pticks_ums,
         * we need to come insync now again.
         * Or, in bad cases, the more needed usecs will add up.
         */
        last_time.tv_sec = now.tv_sec;
        last_time.tv_usec = now.tv_usec;

        out->tv_sec = 0;
        out->tv_usec = 0;

        return 0;
    }

    /* time_until_next_tick = next_tick - now */
    now.tv_sec = -now.tv_sec;
    now.tv_usec = -now.tv_usec;
    add_time(out, &next_tick, &now);

    return 1;
}

/*
 * sleep_delta checks how much time has elapsed since last tick.
 * If it is less than pticks_ums, the remaining time is slept with select().
 *
 * Polls the sockets and handles or queues incoming requests
 * returns at the time for the next tick
 */
void sleep_delta()
{
    struct timeval timeout;


    /* TODO: ideally we should use the return value from select to know if it
     * timed out or returned because of some other reason, but this also
     * works reasonably well...
     */
    while(time_until_next_tick(&timeout))  /* fill timeout... */
        doeric_server(SOCKET_UPDATE_CLIENT, &timeout);
}

/* set the pticks_xx but NOT pticks itself.
 * pticks_ums = how "long" in ums is a server "round" (counted with pticks aka ROUND_TAG)
 * pticks_second = how many "round" are done in a second.
 *
 * The default ums is set in MAX_TIME in config.h and can
 * be changed with with the dm_time command.
 */
/* TODO: send new pticks_xx to all plugins! */
void set_pticks_time(long t)
{
    pticks_ums = t;
    pticks_second = 1000000.0f/(float) t;
	pticks_socket_idle = (uint32) ((60.0f * 3.0f) * pticks_second);
	pticks_player_idle1 = (uint32) ((60.0f * 8.0f) * pticks_second);
	pticks_player_idle2 = (uint32) ((60.0f * 2.0f) * pticks_second);
}

void get_tod(timeofday_t *tod)
{
    tod->year = todtick / HOURS_PER_YEAR;
    tod->month = (todtick / HOURS_PER_MONTH) % MONTHS_PER_YEAR;
    tod->day = (todtick % HOURS_PER_MONTH) / DAYS_PER_MONTH;
    tod->dayofweek = tod->day % DAYS_PER_WEEK;
    tod->hour = todtick % HOURS_PER_DAY;
    tod->minute = (ROUND_TAG % PTICKS_PER_CLOCK) / (PTICKS_PER_CLOCK / 58);
    if (tod->minute > 58)
        tod->minute = 58; /* it's imprecise at best anyhow */
    tod->weekofmonth = tod->day / WEEKS_PER_MONTH;
    if (tod->month < 3)
        tod->season = 0;
    else if (tod->month < 6)
        tod->season = 1;
    else if (tod->month < 9)
        tod->season = 2;
    else if (tod->month < 12)
        tod->season = 3;
    else
        tod->season = 4;

    tod->dayofweek_name = weekdays[tod->dayofweek];
    tod->month_name = month_name[tod->dayofweek];
    tod->season_name = season_name[tod->dayofweek];
}

void print_tod(object *op)
{
    timeofday_t tod;
    char       *suf;
    int         day;

    get_tod(&tod);
    sprintf(errmsg, "It is %d minute%s past %d o'clock %s,", tod.minute + 1, ((tod.minute + 1 < 2) ? "" : "s"),
    ((tod.hour % (HOURS_PER_DAY / 2) == 0) ? (HOURS_PER_DAY / 2) : ((tod.hour) % (HOURS_PER_DAY / 2))),
    ((tod.hour >= (HOURS_PER_DAY / 2)) ? "pm" : "am"));
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);

    sprintf(errmsg, "on %s", weekdays[tod.dayofweek]);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);


    day = tod.day + 1;
    if (day == 1 || ((day % 10) == 1 && day > 20))
        suf = "st";
    else if (day == 2 || ((day % 10) == 2 && day > 20))
        suf = "nd";
    else if (day == 3 || ((day % 10) == 3 && day > 20))
        suf = "rd";
    else
        suf = "th";
    sprintf(errmsg, "The %d%s Day of the %s, Year %d", day, suf, month_name[tod.month], tod.year + 1);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);

    sprintf(errmsg, "Time of Year: %s", season_name[tod.season]);
    new_draw_info(NDI_UNIQUE, 0, op, errmsg);
}


long seconds()
{
    struct timeval  now;

    (void) GETTIMEOFDAY(&now);
    return now.tv_sec;
}
