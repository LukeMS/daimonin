/*
    Daimonin, the Massive Multiuser Online Role Playing Game
    Server Applicatiom

    Copyright (C) 2001-2008 Michael Toennies

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

/* The protocol module is common to both client and server. Therefore, it
 * *must* be kept identical between both programs. It therefore follows that
 * this file should only include the protocol.h header and the ANSI library
 * headers. */

#include <ctype.h>
#include <string.h>

#include "protocol.h"

/* Return 1 or 0 depending on whether we have valid name length and only valid
 * chars in our name. */
int account_name_valid(char *cp)
{
    int len,
        i;

    if ((len = strlen(cp)) < MIN_ACCOUNT_NAME ||
        len > MAX_ACCOUNT_NAME)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        if (!account_char_valid(*(cp + i)))
        {
            return 0;
        }

        /* Lowercase. */
        *(cp + i) = tolower(*(cp + i));
    }

    return 1;
}

/* Return 1 or 0 depending on whether we have only valid chars. */
int account_char_valid(char c)
{
    /* fail if not alphabet letter or digit and not '-' or '_' */
    if (!isalnum(c) &&
        c != '-' &&
        c != '_')
    {
        return 0;
    }

    return 1;
}

/* Return 1 or 0 depending on whether we have valid password length and only
 * valid chars in our password. */
int password_valid(char *cp)
{
    int len,
        i;

    if ((len = strlen(cp)) < MIN_ACCOUNT_PASSWORD ||
        len > MAX_ACCOUNT_PASSWORD)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        if (!password_char_valid(*(cp + i)))
        {
            return 0;
        }
    }

    return 1;
}

/* Return 1 or 0 depending on whether we have only valid chars. */
int password_char_valid(char c)
{
    /* fail if not printable (but grats on typing it) or space. */
    if (!isgraph(c))
    {
        return 0;
    }

    return 1;
}

/* Return 1 or 0 depending on whether we have valid name length and only valid
 * chars in our name. */
int player_name_valid(char *cp)
{
    int len,
        i;

    if ((len = strlen(cp)) < MIN_PLAYER_NAME ||
        len > MAX_PLAYER_NAME)
    {
        return 0;
    }

    for (i = len - 1; i >= 0; i--)
    {
        if (!player_char_valid(*(cp + i)))
        {
            return 0;
        }

        /* Lowercase. */
        *(cp + i) = tolower(*(cp + i));
    }

    /* Capitalise first character. */
    *cp = toupper(*cp);

    /* we don't want some special names & keywords here. */
    /* TODO: add here a complete "forbidden name" mechanisn */
    /* FIXME: Do it properly or don't do it at all. */
//    if(!strcasecmp(cp,"fuck") || !strcasecmp(cp,"off") || !strcasecmp(cp,"allow"))
//        return 0;

    return 1;
}

/* Return 1 or 0 depending on whether we have only valid chars. */
int player_char_valid(char c)
{
    /* FIXME: Only letters and '-' now? */
    if (!isalpha(c) &&
        c != '-')
    {
        return 0;
    }

    return 1;
}
