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

static char log_buf[MAXSOCKBUF*2];

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

    log_buf[0] = '\0';
    if (logLevel <= settings.debug)
    {
        vsprintf(log_buf, format, ap);
#ifdef WIN32 /* ---win32 change log handling for win32 */
        if (logfile)
            fputs(log_buf, logfile);    /* wrote to file or stdout */
        else
            fputs(log_buf, stderr);

#ifdef DEBUG                /* if we have a debug version, we want see ALL output */
        if (logfile)
            fflush(logfile);    /* so flush this! */
#endif
        if (logfile && logfile != stderr)   /* if was it a logfile wrote it to screen too */
            fputs(log_buf, stderr);
#else
        if (logfile)
            fputs(log_buf, logfile);
        else
            fputs(log_buf, stderr);
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
