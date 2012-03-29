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

#ifndef __FONT_H
#define __FONT_H

#define FONT_BLANKLINE 5

/* Internal remaps */
#define FONT_ARROWUP    28
#define FONT_ARROWDOWN  29
#define FONT_ARROWLEFT  30
#define FONT_ARROWRIGHT 31

typedef struct _font
{
    struct _Sprite *sprite;       // the sprite
    SDL_Rect        c[256];       // adjusted box for each character in pixels
    uint32          colr[256];    // last colr of each character
    sint8           char_offset;  // distance between characters in pixels
    uint8           line_height;  // height of a line in pixels
}
_font;

extern _font font_tiny;
extern _font font_small;
extern _font font_medium;
extern _font font_large;
extern _font font_huge;
extern _font font_booknormal;
extern _font font_booktitle;
extern _font font_npcicon;
extern _font font_npcnormal;
extern _font font_npctitle;

extern void font_init(void);

#endif /* ifndef __FONT_H */
