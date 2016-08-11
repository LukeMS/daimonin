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

#ifndef __SHSTR_NG_H
#define __SHSTR_NG_H

/* enable module statistic.
 * This will add some inc/dec counter to the hash table.
 * small & very small cpu use
 */
#define SHSTR_STATISTICS

/* SS(string) will return the address of the shared_string struct which
 * contains "string".
 */
#define SS(x) ((struct shared_string *) ((x) - offsetof(struct shared_string, string)))

#ifndef SHSTR_DUMP_TOTALS
#define SHSTR_DUMP_TOTALS  1
#endif

/* So far only used when dealing with artifacts.
 * (now used by alchemy and other code too. Nov 95 b.t).
 * This is used in readable.c, recipe.c and treasure.c .
 * its used in statical structures loaded at startup.
 * NEVER use this in dynamical way. */
struct shstr_linked_t
{
    shstr_linked_t *next;

    shstr_t *name;
};

extern void     shstr_init(void);
extern shstr_t *shstr_add_string(const char *str);
extern shstr_t *shstr_add_lstring(const char *str, int n);
extern int      shstr_query_refcount(const char *str);
extern shstr_t *shstr_find(const char *str);
extern shstr_t *shstr_add_refcount(const char *str);
extern void     shstr_free(shstr_t *str);
extern char    *shstr_dump_statistics(char *msg);
extern char    *shstr_dump_table(int what);
extern void     shstr_get_totals(int *entries, int *refs, int *links);

#endif /* ifndef SHSTR_NG_H */
