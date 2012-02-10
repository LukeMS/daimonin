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

_font font_tiny_out;
_font font_small;
_font font_small_out;
_font font_medium;
_font font_medium_out;
_font font_large_out;

static void   CreateNewFont(_Sprite *sprite, _font *font, sint8 xlen,
                            sint8 ylen, sint8 off, uint8 line);
static uint32 GetSurfacePixel(SDL_Surface *surface, uint16 x, uint16 y);

void font_init(void)
{
    CreateNewFont(skin_sprites[SKIN_SPRITE_FONTTINYOUT], &font_tiny_out, 16, 16, -1, 11);
    CreateNewFont(skin_sprites[SKIN_SPRITE_FONTSMALL], &font_small, 16, 16, 1, 12);
    CreateNewFont(skin_sprites[SKIN_SPRITE_FONTSMALLOUT], &font_small_out, 16, 16, 1, 13);
    CreateNewFont(skin_sprites[SKIN_SPRITE_FONTMEDIUM], &font_medium, 16, 16, 1, 14);
    CreateNewFont(skin_sprites[SKIN_SPRITE_FONTMEDIUMOUT], &font_medium_out, 16, 16, 0, 15);
    CreateNewFont(skin_sprites[SKIN_SPRITE_FONTBIGOUT], &font_large_out, 11, 16, 1, 18);
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
                                    (uint16)(font->c[i].x + font->c[i].w - 1),
                                    (uint16)(font->c[i].y + y)))
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
