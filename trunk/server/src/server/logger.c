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

/* minimum timestamp interval = 10 minutes */
#define TIMESTAMP_INTERVAL 600
static struct timeval last_timestamp = {0, 0};

static void CheckTimestamp(void);
static void DoPrint(char *buf, FILE *fp);

#if 0
/* force_timestamp can be called to force a timestamp on the
 * next logged message, regardless of the time since the last
 * one. This could be used for an event where an
 * accurate time of occurrence is needed. For example, call
 * this before logging a player login/logout if required.
 * The call could be enclosed in a #ifdef to control this
 * with a #define, of course. */
 void force_timestamp()
 {
     last_timestamp.tv_sec = 0;
 }
#endif

/* Logs a message to tlogfile.
 *
 * See include/logger.h for possible logLevels.  Messages with llevSystem
 * and llevError are always printed, regardless of debug mode. 
 *
 * The return is always 1 (but errors and bug floods kill the server so do not
 * return). As this says nothing about how the function has performed this can
 * usually be ignored. But it does mean LOG() can be used in tests (eg, if
 * (LOG())) which is extremely useful for debugging, particularly in macros. */
sint8 LOG(log_t logLevel, char *format, ...)
{
    /* Check if timestamp needed */
    CheckTimestamp();

    if (logLevel <= settings.debug ||
        logLevel == llevSystem ||
        logLevel == llevError)
    {
        va_list ap;
        char    buf[HUGE_BUF];

        va_start(ap, format);
        vsprintf(buf, format, ap);
        va_end(ap);
        DoPrint(buf, tlogfile);

#ifdef DAI_DEVELOPMENT_CONTENT
        if (logLevel == llevMapbug)
        {
# ifdef USE_CHANNELS
            struct channels *channel = findGlobalChannelFromName(NULL,
                                                                 CHANNEL_NAME_MW,
                                                                 1);

            if (channel)
            {
                sendChannelMessage(NULL, channel, buf);
            }
# else
            ndi(NDI_PLAYER | NDI_UNIQUE | NDI_ALL | NDI_RED, 5, NULL,
                          "%s", buf);
# endif
        }
#endif
    }

    if (logLevel == llevError)
    {
        exiting = 1;
        DoPrint("Fatal: Shutdown server. Reason: Fatal Error\n", tlogfile);
        fatal_signal(0, 1, SERVER_EXIT_FATAL);
    }
    else if (logLevel == llevBug &&
             ++nroferrors > MAX_ERRORS)
    {
        exiting = 1;
        DoPrint("Fatal: Shutdown server. Reason: BUG flood\n", tlogfile);
        fatal_signal(0, 1, SERVER_EXIT_FLOOD);
    }

    return 1;
}

/* Logs a message to clogfile.  */
void CHATLOG(char *format, ...)
{
    if (llevInfo <= settings.debug)
    {
        va_list ap;
        char    buf[LARGE_BUF];

        sprintf(buf, "%s", (clogfile == tlogfile) ? "CLOG " : "");
        va_start(ap, format);
        vsprintf(strchr(buf, '\0'), format, ap);
        va_end(ap);
        DoPrint(buf, clogfile);
    }
}

/* Check if timestamp is due */
static void CheckTimestamp(void)
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
        DoPrint(buf, tlogfile);
        fflush(tlogfile);

        if (clogfile != tlogfile)
        {
            DoPrint(buf, clogfile);
            fflush(clogfile);
        }
    }
}

/* Pull out common printing code. */
static void DoPrint(char *buf, FILE *fp)
{
    if (fp)
    {
        fputs(buf, fp);
    }
    else
    {
        fputs(buf, stderr);
    }

#ifdef WIN32 /* ---win32 change log handling for win32 */
#ifdef DEBUG                /* if we have a debug version, we want see ALL output */
    if (fp)
    {
        fflush(fp);    /* so flush this! We need this because we don't have added a exception/signal handler for win32 */
    }
#endif

    /* if it was a logfile wrote it to screen too */
    if (fp &&
        fp != stderr)
    {
        fputs(buf, stderr);
    }
#endif
}
