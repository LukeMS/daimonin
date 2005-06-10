/*
    Daimonin SDL client, a client program for the Daimonin MMORPG.


  Copyright (C) 2003 Michael Toennies

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

    The author can be reached via e-mail to info@daimonin.net
*/

#if !defined(__BOOK_H)
#define __BOOK_H

#define BOOK_PAGE_LINES 28
#define BOOK_LINES_CHAR 64

typedef struct gui_book_line
{
	int mode;
	int color;
	char line[BOOK_LINES_CHAR+1];
} _gui_book_line;

typedef struct gui_book_struct
{
	int mode;
	int pages;
	int page_show;
	struct gui_book_page *start;
} _gui_book_struct;


typedef struct gui_book_page
{
	struct gui_book_page *next;
	_gui_book_line *line[BOOK_PAGE_LINES];
} _gui_book_page;

extern _gui_book_struct *load_book_interface(int mode, char *data, int len);
extern void show_book(int x, int y);

#endif
