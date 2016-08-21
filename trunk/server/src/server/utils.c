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

/*
 * General convenience functions for crossfire.
 */

#include <global.h>

/*
 *  Return the number of the spell that whose name passes the pasesed string
 *  argument.   Return -1 if no such spell name match is found.
 */
int look_up_spell_name(const char *spname)
{
    register int i;
    for (i = 0; i < NROFREALSPELLS; i++)
    {
        if (strcmp(spname, spells[i].name) == 0)
            return i;
    }
    return -1;
}


racelink_t * find_racelink(const char *name)
{
    racelink_t   *test    = NULL;

    if (name && first_race)
        for (test = first_race; test && test != test->next; test = test->next)
            if (!test->name || !strcmp(name, test->name))
                break;

    return test;
}

/* this function does 2 things: controlling we have
 * a legal string - if not, return NULL  - if return string*
 * - remove all whitespace in front (if all are whitespace
 *   we return NULL)
 */
char * cleanup_string(char *ustring)
{
    /* kill all whitespace */
    while (*ustring != '\0' && isspace(*ustring))
        ustring++;

    /* this happens when whitespace only string was submited */
    if (!ustring || *ustring == '\0')
        return NULL;

    return ustring;
}

/* get_token() extracts the first 'token' from string, writing this to token
 * and returning a pointer to the suffix of string.
 *
 * A 'token' is defined as a sequence of characters between two delimeters.
 * By default ' ' is the delimeter. If qflag is non-zero then '"' is the
 * delimeter. Leading whitespace is always skipped.
 *
 * token should point to an array large enough to hold any possible 'token'.
 *
 * The return is the rest of string after token or NULL if the end of string
 * has been reached (or it was NULL to begin with. */
char *get_token(char *string, char *token, uint8 qflag)
{
    uint16 i;

    /* Blank the token. */
    *token = '\0';

    if (string)
    {
        /* Skip leading whitespace. */
        for (i = 0; *(string + i) != '\0'; i++)
        {
            if (!isspace(*(string + i)))
            {
                break;
            }
        }

        /* Something left. */
        if (*(string + i) != '\0')
        {
            uint16 j = 0;
            char   delimeter = ' ';

            /* We're looking for a quoted string and find an (opening) '"''. */
            if (qflag &&
                *(string + i) == '"')
            {
                delimeter = '"';
                i++;
            }

            /* Copy string to token until end of string or delimeter is reached. */
            while (*(string + i) != '\0')
            {
                if (*(string + i) == delimeter)
                {
                    i++;
                    break;
                }

                *(token + j++) = *(string + i++);
            }

            *(token + j) = '\0';
        }
    }

    /* If string is NULL or we reached the end, return NULL. */
    if (!string ||
        *(string + i) == '\0')
    {
        return NULL;
    }

    /* Return suffix of string. */
    return (string + i);
}

/* buf_overflow() - we don't want to exceed the buffer size of
 * buf1 by adding on buf2! Returns true if overflow will occur.
 */

int buf_overflow(const char *buf1, const char *buf2, int bufsize)
{
    int len1 = 0, len2 = 0;

    if (buf1)
        len1 = strlen(buf1);
    if (buf2)
        len2 = strlen(buf2);
    if ((len1 + len2) >= bufsize)
        return 1;
    return 0;
}

/*
 * Writes <num> ones and zeros to the given string based on the
 * <bits> variable.
 * NOT USED ATM - MT/10.2005
 */
/*
void bitstostring(long bits, int num, char *str)
{
    int i, j = 0;

    if (num > 32)
        num = 32;

    for (i = 0; i < num; i++)
    {
        if (i && (i % 3) == 0)
        {
            str[i + j] = ' ';
            j++;
        }
        if (bits & 1)
            str[i + j] = '1';
        else
            str[i + j] = '0';
        bits >>= 1;
    }
    str[i + j] = '\0';
    return;
}
*/

/* Gives a percentage clipped to 0% -> 100% of a/b. */
int clipped_percent(int a, int b)
{
    int rv;

    if (b <= 0)
        return 0;

    rv = (int) ((100.0f * ((float) a) / ((float) b)) + 0.5f);

    if (rv < 0)
        return 0;
    else if (rv > 100)
        return 100;

    return rv;
}

void NDI_LOG(log_t logLevel, int flags, int pri, object_t *ob, char *format, ...)
{
    va_list ap;
    char    buf[HUGE_BUF];

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    LOG(logLevel, "%s\n", buf);

    if (ob &&
        CONTR(ob))
        ndi(flags, pri, ob, "%s", buf);
}
