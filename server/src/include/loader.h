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
#define LL_IGNORED  -1
#define LL_EOF	    0
#define LL_MORE	    1
#define LL_NORMAL   2

/* see loader.l for more details on this */
#define LO_REPEAT   0
#define LO_LINEMODE 1
#define LO_NEWFILE  2
#define	LO_NOREAD   3
#define LO_MEMORYMODE 4

extern int nroffiles, nrofpixmaps;
