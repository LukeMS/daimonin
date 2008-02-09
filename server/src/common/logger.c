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
#include <stdarg.h>
#include <global.h>

static char log_buf[256*1024];

/* minimum timestamp interval = 10 minutes */
#define TIMESTAMP_INTERVAL 600
static struct timeval last_timestamp = {0, 0};

/* Pull out common printing code from LOG */
void do_print(char *buf)
{
#ifdef WIN32 /* ---win32 change log handling for win32 */
    if (logfile)
        fputs(buf, logfile);    /* wrote to file or stdout */
    else
        fputs(buf, stderr);

#ifdef DEBUG                /* if we have a debug version, we want see ALL output */
    if (logfile)
        fflush(logfile);    /* so flush this! We need this because we don't have added a exception/signal handler for win32 */
#endif
    if (logfile && logfile != stderr)   /* if was it a logfile wrote it to screen too */
        fputs(buf, stderr);
#else
    if (logfile)
        fputs(buf, logfile);
    else
        fputs(buf, stderr);
#endif
}

/* Check if timestamp is due */
void check_timestamp()
{
    struct timeval   now;
    struct tm       *tim;
    char             buf[256];

    GETTIMEOFDAY(&now);
    if (now.tv_sec >= (last_timestamp.tv_sec + TIMESTAMP_INTERVAL))
    {
        const time_t temp_time = (const time_t) now.tv_sec;

        last_timestamp.tv_sec = now.tv_sec;
        tim = localtime(&temp_time);
        sprintf(buf, "\n*** TIMESTAMP: %4d-%02d-%02d %02d:%02d:%02d ***\n\n",
            tim->tm_year+1900, tim->tm_mon+1, tim->tm_mday,
            tim->tm_hour, tim->tm_min, tim->tm_sec);
        do_print(buf);
    }
}

/* force_timestamp can be called to force a timestamp on the
 * next logged message, regardless of the time since the last
 * one. This could be used for an event where an
 * accurate time of occurrence is needed. For example, call
 * this before logging a player login/logout if required.
 * The call could be enclosed in a #ifdef to control this
 * with a #define, of course.
 */
 void force_timestamp()
 {
     last_timestamp.tv_sec = 0;
 }

/*
 * Logs a message to stderr, or to file, and/or even to socket.
 * Or discards the message if it is of no importanse, and none have
 * asked to hear messages of that logLevel.
 *
 * See include/logger.h for possible logLevels.  Messages with llevSystem
 * and llevError are always printed, regardless of debug mode.
 */

void LOG(LogLevel logLevel, char *format, ...)
{
    static int  fatal_error = FALSE;

    va_list     ap;
    va_start(ap, format);

    /* Check if timestamp needed */
    check_timestamp();

    log_buf[0] = '\0';
    if (logLevel <= settings.debug)
    {
        vsprintf(log_buf, format, ap);
        do_print(log_buf);

        /* Mapbugs are broadcasted on the test server */

#ifdef _TESTSERVER
        if(logLevel == llevMapbug)
            new_draw_info(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_RED, 5, NULL, log_buf);
#endif
    }

    va_end(ap);

    if (logLevel == llevBug)
        ++nroferrors;

    if (nroferrors > MAX_ERRORS || logLevel == llevError)
    {
        exiting = 1;
        if (fatal_error == FALSE)
        {
            fatal_error = TRUE;
            LOG(llevSystem, "Fatal: Shutdown server. Reason: %s\n", logLevel == llevError ? "Fatal Error" : "BUG fload");
            fatal_signal(0,1);
        }
    }
}
