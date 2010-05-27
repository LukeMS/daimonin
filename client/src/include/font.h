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

    The author can be reached via e-mail to info@daimonin.org
*/
#if !defined(__FONT_H)
#define __FONT_H

#define FONT_BLANKLINE 5

typedef struct _font
{
    struct _Sprite *sprite; /* don't free this, we link here a Bitmaps[x] ptr*/
    SDL_Rect        c[256];
    sint8           char_offset; /* space in pixel between 2 chars in a word */
    uint8           line_height;
}
_font;

extern _font font_tiny_out;
extern _font font_small;
extern _font font_small_out;
extern _font font_medium;
extern _font font_medium_out;
extern _font font_large_out;

extern void font_init(void);

#endif

