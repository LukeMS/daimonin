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

/*
 * General convenience functions for crossfire.
 */

#include <global.h>


/*
 * Another convenience function.  Returns a number between min and max.
 * It is suggested one use these functions rather than RANDOM()%, as it
 * would appear that a number of off-by-one-errors exist due to improper
 * use of %.  This should also prevent SIGFPE.
 */

int random_roll(int min, int max)
{
    int diff;

    diff = max - min + 1;
    if (max < 1 || diff < 1)
        return(min);

    return(RANDOM() % diff + min);
}


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


racelink * find_racelink(const char *name)
{
    racelink   *test    = NULL;

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


/* returns a single word from a string, free from left & right whitespaces.
 * return NULL means that there is word left in str.
 */
char * get_word_from_string(char *str, int *pos)
{
    static char buf[HUGE_BUF]; /* this is used for controled input which never should bigger as this */
    int         i   = 0;

    buf[0] = '\0';

    while (*(str + (*pos)) != '\0' && (!isalnum(*(str + (*pos))) && !isalpha(*(str + (*pos)))))
        (*pos)++;

    if (*(str + (*pos)) == '\0') /* nothing left! */
        return NULL;

    /* copy until end of string nor whitespace */
    while (*(str + (*pos)) != '\0' && (isalnum(*(str + (*pos))) || isalpha(*(str + (*pos)))))
        buf[i++] = *(str + (*pos)++);

    buf[i] = '\0';
    return buf;
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

/* we transform a given name string
 * like "xxXxxX" or "xxxxxx" to "Xxxxxx"!
 * We change the string chars direct in the given buffer
 * so be sure not to give hash or constant pointers here.
 * as a small gimmick we give back the strlen of the name.
 */
int transform_name_string(char *name)
{
    char   *tmp = name;

    if (!tmp || *tmp == '\0')
        return 0;

    *tmp = toupper(*tmp);

    while (*(++tmp) != '\0')
        *tmp = tolower(*tmp);

    /* trim the string on the right side */
    while(tmp>=name && *(tmp-1) ==' ')
        *(--tmp)='\0';

    return tmp - name; /* i love C */
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
