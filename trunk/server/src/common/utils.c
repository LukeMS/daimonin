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
 * The random functions here take luck into account when rolling random
 * dice or numbers.  This function has less of an impact the larger the
 * difference becomes in the random numbers.  IE, the effect is lessened
 * on a 1-1000 roll, vs a 1-6 roll.  This can be used by crafty programmers,
 * to specifically disable luck in certain rolls, simply by making the
 * numbers larger (ie, 1d1000 > 500 vs 1d6 > 3)
 */

/*
 * Roll a random number between min and max.  Uses op to determine luck,
 * and if goodbad is non-zero, luck increases the roll, if zero, it decreases.
 * Generally, op should be the player/caster/hitter requesting the roll,
 * not the recipient (ie, the poor slob getting hit). [garbled 20010916]
 */

int random_roll(int min, int max, object *op, int goodbad)
{
    int omin, diff, luck, base;

    omin = min;
    diff = max - min + 1;
    ((diff > 2) ? (base = 20) : (base = 50)); /* d2 and d3 are corner cases */

    if (max < 1 || diff < 1)
    {
        LOG(llevBug, "BUG: Calling random_roll with min=%d max=%d\n", min, max);
        return(min); /* avoids a float exception */
    }

    if (op->type != PLAYER)
        return((RANDOM() % diff) + min);

    luck = op->stats.luck;
    if (RANDOM() % base < MIN(10, abs(luck)))
    {
        /* we have a winner */
        ((luck > 0) ? (luck = 1) : (luck = -1));
        diff -= luck;
        if (diff < 1)
            return(omin); /*check again*/
        ((goodbad) ? (min += luck) : (diff));

        return(MAX(omin, MIN(max, (RANDOM() % diff) + min)));
    }
    return((RANDOM() % diff) + min);
}

/*
 * Roll a number of dice (2d3, 4d6).  Uses op to determine luck,
 * If goodbad is non-zero, luck increases the roll, if zero, it decreases.
 * Generally, op should be the player/caster/hitter requesting the roll,
 * not the recipient (ie, the poor slob getting hit).
 * The args are num D size (ie 4d6)  [garbled 20010916]
 */

int die_roll(int num, int size, object *op, int goodbad)
{
    int min, diff, luck, total, i, gotlucky, base;

    diff = size;
    min = 1;
    luck = total = gotlucky = 0;
    ((diff > 2) ? (base = 20) : (base = 50)); /* d2 and d3 are corner cases */
    if (size < 2 || diff < 1)
    {
        LOG(llevBug, "BUG: Calling die_roll with num=%d size=%d\n", num, size);
        return(num); /* avoids a float exception */
    }

    if (op->type == PLAYER)
        luck = op->stats.luck;

    for (i = 0; i < num; i++)
    {
        if (RANDOM() % base < MIN(10, abs(luck)) && !gotlucky)
        {
            /* we have a winner */
            gotlucky++;
            ((luck > 0) ? (luck = 1) : (luck = -1));
            diff -= luck;
            if (diff < 1)
                return(num); /*check again*/
            ((goodbad) ? (min += luck) : (diff));
            total += MAX(1, MIN(size, (RANDOM() % diff) + min));
        }
        else
        {
            total += RANDOM() % size + 1;
        }
    }
    return(total);
}

/*
 * Another convenience function.  Returns a number between min and max.
 * It is suggested one use these functions rather than RANDOM()%, as it
 * would appear that a number of off-by-one-errors exist due to improper
 * use of %.  This should also prevent SIGFPE.
 */

int rndm(int min, int max)
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
