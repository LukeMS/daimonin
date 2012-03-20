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

#include "include.h"

_font font_tiny;
_font font_small;
_font font_medium;
_font font_large;
_font font_huge;

static void   CreateNewFont(_Sprite *sprite, _font *font);
static uint32 GetSurfacePixel(SDL_Surface *surface, uint16 x, uint16 y);

void font_init(void)
{
    CreateNewFont(skin_fonts[SKIN_FONT_TINY], &font_tiny);
    CreateNewFont(skin_fonts[SKIN_FONT_SMALL], &font_small);
    CreateNewFont(skin_fonts[SKIN_FONT_MEDIUM], &font_medium);
    CreateNewFont(skin_fonts[SKIN_FONT_LARGE], &font_large);
    CreateNewFont(skin_fonts[SKIN_FONT_HUGE], &font_huge);
}

/* init this font structure with gfx data from sprite bitmap */
static void CreateNewFont(_Sprite *sprite, _font *font)
{
    uint16 width = (sprite->bitmap->w - 32 - 1) / 32,
           height = (sprite->bitmap->h - 8 - 1) / 8,
           i;
    uint8  maxh = 0;

    font->sprite = sprite;

    /* Calc bounding boxes for each of the 256 characters. */
    SDL_LockSurface(font->sprite->bitmap);

    for (i = 0; i <= 255; i++)
    {
        sint8 flag,
              p;

        /* Calc the initial bounding box of this character. */
        font->c[i].x = (i % 32) * (width + 1) + 1;
        font->c[i].y = (i / 32) * (height + 1) + 1;
        font->c[i].w = width;
        font->c[i].h = height;

        /* Find width of this character. */
        flag = 0;

        while (1)
        {
            for (p = height - 1; p >= 0; p--)
            {
                if (GetSurfacePixel(font->sprite->bitmap,
                                    (uint16)(font->c[i].x + font->c[i].w - 1),
                                    (uint16)(font->c[i].y + p)))
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

        /* Find height of this character. */
        flag = 0;

        while (1)
        {
            for (p = width - 1; p >= 0; p--)
            {
                if (GetSurfacePixel(font->sprite->bitmap,
                                    (uint16)(font->c[i].x + p),
                                    (uint16)(font->c[i].y + font->c[i].h - 1)))
                {
                    flag = 1;

                    break;
                }
            }

            if (flag)
            {
                break;
            }

            font->c[i].h--;
        }

        /* If this is the tallest character yet, remember that height. */
        if (font->c[i].h > maxh)
        {
            maxh = font->c[i].h;
        }
    }

    SDL_UnlockSurface(font->sprite->bitmap);

    /* Whitespace width is presumably 0 so set it to one ex. */
    if (!font->c[' '].w)
    {
        font->c[' '].w = font->c['x'].w;
    }

    /* The horizontal space between characters is the actual width of c[0]
     * minus half the maximum char width or a default of 1. */
    font->char_offset = (font->c[0].w)
                        ? (sint8)(font->c[0].w - width * 0.5)
                        : 1;

    /* The line height is the height of c[1] or a default of the max height of
     * the other characters plus 2. */
    font->line_height = (font->c[1].h) ? (uint8)font->c[1].h : maxh + 2;
}

static uint32 GetSurfacePixel(SDL_Surface *surface, uint16 x, uint16 y)
{
    uint8  bpp = surface->format->BytesPerPixel,
    /* Here p is the address to the pixel we want to retrieve */
          *p = (uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
        case 1:
            return *p;

        case 2:
            return *(uint16 *)p;

        case 3:
#if 0
        {
            /* Format/endian independent*/
            Uint8     r, g, b;
            r = *((bits) + Surface->format->Rshift / 8);
            g = *((bits) + Surface->format->Gshift / 8);
            b = *((bits) + Surface->format->Bshift / 8);
            return SDL_MapRGB(Surface->format, r, g, b);
        }
#else
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                return p[0] << 16 | p[1] << 8 | p[2];
            }
            else
            {
                return p[0] | p[1] << 8 | p[2] << 16;
            }
#endif

        case 4:
            return *(uint32 *)p;

        default: // shouldn't happen, but avoids warnings
            return 0;
    }
}
