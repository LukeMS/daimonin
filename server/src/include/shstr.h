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
/* The size of the shared strings hashtable. This must be smaller than
 * 32767, but 947 ought to be plenty enough.
 */
#define TABLESIZE 4133

/* This specifies how many characters the hashing routine should look at.
 * You may actually save CPU by increasing this number if the typical string
 * is large.
 */
#ifndef MAXSTRING
#define MAXSTRING 20
#endif

/* In the unlikely occurence that 16383 references to a string are too
 * few, you can modify the below type to something bigger.
 * (The top bit of "refcount" is used to signify that "u.array" points
 * at the array entry.)
 */
#define REFCOUNT_TYPE unsigned int

/* The offsetof macro is part of ANSI C, but many compilers lack it, for
 * example "gcc -ansi"
 */
#if !defined (offsetof)
#define offsetof(type, member) (int)&(((type *)0)->member)
#endif

/* SS(string) will return the address of the shared_string struct which 
 * contains "string".
 */
#define SS(x) ((shared_string *) ((x) - offsetof(shared_string, string)))

#define SS_STATISTICS

#define SS_DUMP_TABLE	1
#define SS_DUMP_TOTALS	2

#ifdef SS_STATISTICS
static struct statistics {
    int calls;
    int hashed;
    int strcmps;
    int search;
    int linked;
} add_stats, add_ref_stats, free_stats, find_stats, hash_stats;
#define GATHER(n) (++n)
#else /* !SS_STATISTICS */
#define GATHER(n)
#endif /* SS_STATISTICS */

#define TOPBIT	(1 << (sizeof(REFCOUNT_TYPE) * CHAR_BIT - 1))

#define PADDING	((2 * sizeof(long) - sizeof(REFCOUNT_TYPE)) % sizeof(long))

typedef struct _shared_string {
    union {
	struct _shared_string **array;
	struct _shared_string *previous;
    } u;
    struct _shared_string *next;
    /* The top bit of "refcount" is used to signify that "u.array" points
     * at the array entry.
     */
    REFCOUNT_TYPE refcount;
    /* Padding will be unused memory, since we can't know how large 
     * the padding when allocating memory. We assume here that
     * sizeof(long) is a good boundary.
     */
    char string[PADDING];
} shared_string;
