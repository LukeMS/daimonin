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

#ifndef __BOOK_H
#define __BOOK_H

/* Dec '95 - laid down initial file. Stuff in here is for BOOKs
 * hack. Information in this file describes fundental parameters
 * of 'books' - objects with type==BOOK. -b.t.
 */

/* Message buf size. If this is changed, keep in mind that big strings
 * may be unreadable by the player as the tail of the message
 * can scroll over the beginning (as of v0.92.2).  */

#define BOOK_BUF        (HUGE_BUF-256)

/* if little books arent getting enough text generated, enlarge this */

#define BASE_BOOK_BUF   ((int)(BOOK_BUF*0.75))

/* Book buffer size. We shouldnt let little books/scrolls have
 * more info than big, weighty tomes! So lets base the 'natural'
 * book message buffer size on its weight. But never let a book
 * mesg buffer exceed the max. size (BOOK_BUF) */

#define BOOKSIZE(xyz)   BASE_BOOK_BUF+((xyz)->weight/10)>BOOK_BUF? \
                                BOOK_BUF:BASE_BOOK_BUF+((xyz)->weight/10);

#endif /* ifndef __BOOK_H */

#ifndef __DOOR_H
#define __DOOR_H

#define DOOR_MODE_TEST 0
#define DOOR_MODE_OPEN 1

extern object_t *door_find_key(object_t *who, object_t *what);
extern sint8     door_open(object_t *who, object_t *what, uint8 mode);
extern void      door_close(object_t *what);

#endif /* ifndef __DOOR_H */
