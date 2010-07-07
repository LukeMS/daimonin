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
#define SS_STATISTICS

/* The offsetof macro is part of ANSI C, but many compilers lack it, for
 * example "gcc -ansi"
 */
#if !defined (offsetof)
#define offsetof(type, member) (int)&(((type *)0)->member)
#endif

/* SS(string) will return the address of the shared_string struct which
 * contains "string".
 */
#define SS(x) ((struct shared_string *) ((x) - offsetof(struct shared_string, string)))

#ifndef SS_DUMP_TOTALS
#define SS_DUMP_TOTALS  1
#endif

/* This should be used to differentiate shared strings from normal strings */
typedef const char shstr;

#endif /* ifndef SHSTR_NG_H */
