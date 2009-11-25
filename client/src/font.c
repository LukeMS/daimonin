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

#include "include.h"

_font font_tiny_out;
_font font_small;
_font font_small_out;
_font font_medium;
_font font_medium_out;
_font font_big_out;

static void CreateNewFont(_Sprite *sprite, _font *font, sint8 xlen, sint8 ylen,
                          sint8 off, uint8 line);

void font_init(void)
{
    CreateNewFont(Bitmaps[BITMAP_FONTTINYOUT], &font_tiny_out, 16, 16, -1, 10);
    CreateNewFont(Bitmaps[BITMAP_FONTSMALL], &font_small, 16, 16, 1, 11);
    CreateNewFont(Bitmaps[BITMAP_FONTSMALLOUT], &font_small_out, 16, 16, 1, 12);
    CreateNewFont(Bitmaps[BITMAP_FONTMEDIUM], &font_medium, 16, 16, 1, 15);
    CreateNewFont(Bitmaps[BITMAP_FONTMEDIUMOUT], &font_medium_out, 16, 16, 0, 16);
    CreateNewFont(Bitmaps[BITMAP_FONTBIGOUT], &font_big_out, 11, 16, 1, 20);
}

/* init this font structure with gfx data from sprite bitmap */
static void CreateNewFont(_Sprite *sprite, _font *font, sint8 xlen, sint8 ylen,
                          sint8 off, uint8 line)
{
    uint16 i;

    SDL_LockSurface(sprite->bitmap);
    font->sprite = sprite;

    for (i = 0; i <= 255; i++)
    {
        uint8 flag;

        font->c[i].x = (i % 32) * (xlen + 1) + 1;
        font->c[i].y = (i / 32) * (ylen + 1) + 1;
        font->c[i].h = ylen;
        font->c[i].w = xlen;
        flag = 0;

        while (1) /* better no error in font bitmap... or this will lock up*/
        {
            sint16 y;
 
            for (y = font->c[i].h - 1; y >= 0; y--)
            {
                if (GetSurfacePixel(sprite->bitmap,
                                    font->c[i].x + font->c[i].w - 1,
                                    font->c[i].y + y))
                {
                    flag = 1;

                    break;
                }
            }

            if (flag)
            {
                break;
            }

            font->c[i].w--;
        }
    }

    SDL_UnlockSurface(sprite->bitmap);
    font->char_offset = off;
    font->line_height = line;
}
