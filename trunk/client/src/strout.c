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

/* This module deals with the output of strings of text (ie, blitting a font's
 * surface to some destination surface).
 *
 * -- Smacky 20120330 */

#include "include.h"

strout_vim_t *strout_vim_queue;

/* Calculate the displayed width of the text. Newlines will reset the count so
 * you may want to split the string at newlines first then call this on each
 * substring. */
sint16 strout_width(_font *font, char *text)
{
    sint16 maxw = 0,
           w = 0;

    while (*text)
    {
        switch (*text)
        {
            case ECC_STRONG:
            case ECC_EMPHASIS:
            case ECC_UNDERLINE:
            case ECC_HYPERTEXT:
                break;

            case '\n':
                maxw = w;
                w = 0;

                break;

            default:
                w += font->c[(uint8)*text].w + font->char_offset;
        }

        text++;
    }

    if (w > maxw)
    {
        maxw = w;
    }

    return maxw;
}

/* Calculate the displayed chars for a given width. Newlines will reset the
 * count so you may want to split the string at newlines first then call this
 * on each substring. */
uint8 strout_width_offset(_font *font, char *text, sint16 *line, sint16 len)
{
    sint16 i,
           w = 0;
    uint8  flag = 0;

    for (i = 0; *text; i++)
    {
        switch (*text)
        {
            case ECC_STRONG:
            case ECC_EMPHASIS:
            case ECC_UNDERLINE:
            case ECC_HYPERTEXT:
                break;

            case '\n':
                w = 0;

                break;

            default:
                w += font->c[(uint8)*text].w + font->char_offset;

                if (w >= len && 
                    !flag)
                {
                    flag = 1;
                    *line = i;

                    break;
                }
        }

        text++;
    }

    if (!flag) /* line is in limit */
    {
        *line = i;
    }

    return flag;
}

/* Blit text to surface. */
void strout_blt(SDL_Surface *surface, _font *font, char *text, sint16 x, sint16 y,
                uint32 colr, SDL_Rect *area, _BLTFX *bltfx)
{
    sint16  cx = x,
            cy = y;
    uint32  colr_used = colr;
    char   *c;
    uint8   intertitle = 0,
            strong = 0,
            emphasis = 0,
            hyper = 0;

    if (!text ||
        !*text ||
        !font ||
        !surface) /* sanity check */
    {
        return;
    }

    for (c = text; *c; c++)
    {
        sint16 cw;

        /* A newline does just that. */
        if (*c == '\n')
        {
            cx = x;
            cy += font->line_height;

            continue;
        }
        /* A tab is remapped to a space. */
        else if (*c == '\t')
        {
            *c = ' ';
        }
        /* Other characters <= 27 are non-printable.
         * 28: up arrow
         * 29: down arrow
         * 30: right arrow
         * 31: left arrow */
        else if (*c <= 27)
        {
            continue;
        }

        /* Set character width. The ECCs are never blitted. */
        cw = (*c == ECC_EMPHASIS ||
              *c == ECC_STRONG ||
              *c == ECC_UNDERLINE ||
              (*c == ECC_HYPERTEXT &&
               !InputStringFlag &&
               gui_npc))
             ? 0
             : font->c[(uint8)(*c)].w + font->char_offset;

        /* We have a clipping rectangle? */
        if (area)
        {
            /* If the next char would overreach the width, drop to the start
             * start of a new line. */
            if (cx + cw > x + area->w)
            {
                cx = x;
                cy += font->line_height;
            }

            /* Now if this line would overreach the height, bail out. */
            if (cy + font->line_height > y + area->h)
            {
                return;
            }
        }

        /* Check for ECC_HYPERTEXT. */
        if (*c == ECC_HYPERTEXT)
        {
            /* Hypertext is only allowed in the NPC GUI (TODO: and book GUI). */
            if (!InputStringFlag &&
                gui_npc)
            {
                if ((hyper = !hyper))
                {
                    _gui_npc_element *k;
     
                    if ((k = gui_npc->keyword_selected) &&
                        !strnicmp(c + 1, k->keyword, strlen(k->keyword)))
                    {
                        colr_used = 0xcc66ff;
                    }
                    else
                    {
                        colr_used = skin_prefs.ecc_hypertext;
                    }
                }
                else
                {
                    if (strong)
                    {
                        colr_used = skin_prefs.ecc_strong;
                    }
                    else if (emphasis)
                    {
                        colr_used = skin_prefs.ecc_emphasis;
                    }
                    else
                    {
                        colr_used = colr;
                    }
                }

                continue;
            }
        }

        /* Check for ECC_EMPHASIS, ECC_STRONG, and ECC_UNDERLINE. Again, these
         * are never blitted and if colr is 0x000000 or we're in hypertext
         * mode, we don't do anything at all. */
        switch (*c)
        {
            case ECC_STRONG:
                if (!hyper &&
                    colr)
                {
                    strong = !strong;
     
                    if (strong)
                    {
                        colr_used = skin_prefs.ecc_strong;
                    }
                    else
                    {
                        if (emphasis)
                        {
                            colr_used = skin_prefs.ecc_emphasis;
                        }
                        else if (intertitle &&
                                 (cpl.menustatus == MENU_NPC ||
                                  cpl.menustatus == MENU_BOOK))
                        {
                            colr_used = skin_prefs.ecc_intertitle;
                        }
                        else
                        {
                            colr_used = colr;
                        }
                    }
                }

                continue;

            case ECC_EMPHASIS:
                if (!hyper &&
                    colr)
                {
                    emphasis = !emphasis;
     
                    if (emphasis)
                    {
                        colr_used = skin_prefs.ecc_emphasis;
                    }
                    else
                    {
                        if (strong)
                        {
                            colr_used = skin_prefs.ecc_strong;
                        }
                        else if (intertitle &&
                                 (cpl.menustatus == MENU_NPC ||
                                  cpl.menustatus == MENU_BOOK))
                        {
                            colr_used = skin_prefs.ecc_intertitle;
                        }
                        else
                        {
                            colr_used = colr;
                        }
                    }
                }

                continue;

            case ECC_UNDERLINE:
                if (!hyper &&
                    colr)
                {
                    intertitle = !intertitle;
     
                    if (intertitle &&
                        (cpl.menustatus == MENU_NPC ||
                         cpl.menustatus == MENU_BOOK))
                    {
                        colr_used = skin_prefs.ecc_intertitle;
                    }
                    else
                    {
                        if (strong)
                        {
                            colr_used = skin_prefs.ecc_strong;
                        }
                        else if (emphasis)
                        {
                            colr_used = skin_prefs.ecc_emphasis;
                        }
                        else
                        {
                            colr_used = colr;
                        }
                    }
                }

                continue;
        }

        /* Spaces are not drawn, for obvious reasons, but other characters
         * are. */
        if (*c != ' ')
        {
            SDL_Surface *bitmap = font->sprite->bitmap;
            int          n = bitmap->format->palette->ncolors - 2;
            SDL_Color    color[n];
            float        l = 1.0 / n;
            uint8        r = (colr_used >> 16) & 0xff,
                         g = (colr_used >> 8) & 0xff,
                         b = colr_used & 0xff,
                         i;
            SDL_Rect     box;

            for (i = 0; i < n; i++)
            {
                color[i].r = (uint8)MIN(255, (l * (i + n / 2 + 1)) * r);
                color[i].g = (uint8)MIN(255, (l * (i + n / 2 + 1)) * g);
                color[i].b = (uint8)MIN(255, (l * (i + n / 2 + 1)) * b);
            }

            SDL_SetPalette(bitmap, SDL_LOGPAL, color, 3, n);
            box.x = cx;
            box.y = cy;
            SDL_BlitSurface(bitmap, &font->c[(uint8)(*c)], surface, &box);
        }

        /* Underline. */
        if (intertitle)
        {
            SDL_Rect box;
            uint32   colr_mapped = SDL_MapRGB(surface->format,
                                              (colr_used >> 16) & 0xff,
                                              (colr_used >> 8) & 0xff,
                                              colr_used & 0xff);

            box.x = cx;
            box.y = cy + font->line_height - 1;
            box.w = cw;
            box.h = 1;
            SDL_FillRect(surface, &box, colr_mapped);
        }

        cx += cw;
    }
}

/* Shows InputString and an appropriate caret. */
void strout_input(_font *font, SDL_Rect *box, char repl)
{
    uint16 i,
           hyper = 0;
    char   buf[MAX_INPUT_STRING];
    int    len;
    sint16 xoff;

    for (i = 0; i < MAX_INPUT_STRING && InputString[i]; i++)
    {
        buf[i] = (repl) ? repl : InputString[i];

        if (buf[i] == ECC_HYPERTEXT)
        {
            hyper++;
        }
    }

    buf[i] = '\0';

    /* Replace the character at CurrentCursorPos with \0. */
    buf[CurrentCursorPos] = '\0';

    /* Get the pixel length of this first part of buf. */
    len = strout_width(font, buf) + font->c[ECC_HYPERTEXT].w * hyper;

    /* Calculate any needed offset so the caret and context is always
     * visible. */
    if ((xoff = len - box->w * 0.8) < 0)
    {
        xoff = 0;
    }

    /* Set the clipping rectangle. */
    SDL_SetClipRect(ScreenSurface, box);

    /* Draw buf up to that point. */
    strout_blt(ScreenSurface, font, buf, box->x - xoff, box->y,
               NDI_COLR_WHITE, NULL, NULL);

    /* Restore the character at CurrentCursorPos. */
    if (InputString[CurrentCursorPos])
    {
        buf[CurrentCursorPos] = (repl) ? repl : InputString[CurrentCursorPos];
    }

    /* Draw buf from that point. */
    strout_blt(ScreenSurface, font, &buf[CurrentCursorPos],
               box->x - xoff + len, box->y, NDI_COLR_WHITE, NULL, NULL);

    /* Draw the caret. */
    if (InputCaretBlinkFlag &&
        LastTick % 500 >= 250)
    {
        SDL_Rect caret;
        uint32   colr;

        caret.y = box->y;
        caret.h = font->line_height;

        if (!InputMode)
        {
            caret.x = MAX(box->x, box->x - xoff + len - font->char_offset);
            caret.w = 1;
        }
        else
        {
            caret.x = box->x - xoff + len;
            caret.w = font->c[(int)buf[CurrentCursorPos]].w;
        }

        colr = SDL_MapRGB(ScreenSurface->format,
                          (skin_prefs.input_caret >> 16) & 0xff,
                          (skin_prefs.input_caret >> 8) & 0xff,
                          skin_prefs.input_caret & 0xff);
        SDL_FillRect(ScreenSurface, &caret, colr);
    }

    InputCaretBlinkFlag = 1;

    /* Remove the clipping rectangle. */
    SDL_SetClipRect(ScreenSurface, NULL);
}

/* Display text as a tooltip. */
void strout_tooltip(sint16 mx, sint16 my, char *text)
{
    SDL_Rect  box;
    char     *tooltip = text;

    /* We might have opted out. */
    if (!options.show_tooltips)
    {
        return;
    }

    box.x = mx + 9;
    box.y = my + 17;
    box.w = 3;
    box.h = font_small.line_height;

    /* Calc width of tooltip. */
    while (*text)
    {
        box.w += font_small.c[(uint8)*text++].w + font_small.char_offset;
    }

    /* If it would extend off the screen, truncate it. */
    /* TODO: This is very basic, we should wrap longer lines to a multiline
     * tooltip.
     * -- Smacky 20120330 */
    if (box.x + box.w >= Screensize.x)
    {
        box.x -= (box.x + box.w + 1) - Screensize.x;
    }

    SDL_FillRect(ScreenSurface, &box, -1);
    strout_blt(ScreenSurface, &font_small, tooltip, box.x + 2, box.y - 1,
               NDI_COLR_BLACK, NULL, NULL);
}

/* A Very Important Message (VIM) is text which appears on the map for a short
 * time to indicate some event has happened to the player (or at least nearby
 * and which may be relevant to him). An arbitrary VIM is sent from the server
 * and is also printed permanently in the textwindow. It could be anything
 * though most often it is when a quest has been completed. Other VIMs are
 * about damage or healing received (by the player or another/a mob). */
/* Adds a new VIM to the queue. */
strout_vim_t *strout_vim_add(strout_vim_mode_t mode, uint8 mapx, uint8 mapy,
                             char *text, uint32 colr, uint16 lifetime,
                             uint16 delay)
{
    strout_vim_t *new,
                 *last;
    uint16        w,
                  h,
                  drift;
    SDL_Surface  *surface;

    /* Allocate and assign a new vim. */
    MALLOC(new, sizeof(strout_vim_t));
    new->mode = mode;
    new->mapx = mapx;
    new->mapy = mapy;

    /* Kill VIMs have a bg bitmap. */
    if (mode == VIM_MODE_KILL)
    {
        uint16   len = (uint16)strout_width(&font_large, text);
        SDL_Rect dst;

        w = (uint16)MAX(len, skin_sprites[SKIN_SPRITE_DEATH]->bitmap->w);
        h = (uint16)MAX(font_large.line_height,
                        skin_sprites[SKIN_SPRITE_DEATH]->bitmap->h);
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, w, h, 32,
                                       0x000000ff, 0x0000ff00, 0x00ff0000,
                                       0xff000000);
        dst.x = (w - skin_sprites[SKIN_SPRITE_DEATH]->bitmap->w) / 2;
        dst.y = (h - skin_sprites[SKIN_SPRITE_DEATH]->bitmap->h) / 2;
        SDL_BlitSurface(skin_sprites[SKIN_SPRITE_DEATH]->bitmap, NULL, surface,
                        &dst);
        dst.x = (w - len) / 2;
        dst.y = (h - font_large.line_height) / 2;
        strout_blt(surface, &font_large, text, dst.x, dst.y, colr, NULL, NULL);
    }
    else
    {
        w = (uint16)strout_width(&font_large, text);
        h = (uint16)font_large.line_height;
        surface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, w, h, 32,
                                       0x000000ff, 0x0000ff00, 0x00ff0000,
                                       0xff000000);
        strout_blt(surface, &font_large, text, 0, 0, colr, NULL, NULL);
    }

    new->surface = surface;
    new->lifetime = lifetime;
    new->start = LastTick + delay;
    new->x = MAP_XPOS(mapx, mapy) - w * 0.5;
    new->y = MAP_YPOS(mapx, mapy);

    /* Arbitrary VIMs have a fixed drift and never deviate horizontally. */
    if (new->mode == VIM_MODE_ARBITRARY)
    {
        drift = (uint16)(lifetime * 0.02) + 20;
        new->xoff = 0.0;
        new->yoff = -(float)drift / (float)lifetime;
    }
    /* For others, the number of pixels drift (both x and y) per cycle is
     * semi-random and they may drift left or right. */
    else
    {
        drift = rand() % (uint16)(lifetime * 0.02) + 20;
        new->xoff = (float)(drift * ((rand() % 3) - 1)) / (float)lifetime;
        new->yoff = -(float)drift / (float)lifetime;
    }

    /* Add it to the end of the queue. */
    for (last = strout_vim_queue; last; last = last->next)
    {
        if (!last->next)
        {
            break;
        }
    }

    if (!last)
    {
        strout_vim_queue = new;
    }
    else
    {
        last->next = new;
    }

    new->prev = last;
    new->next = NULL;

    return new;
}

/* Removes a VIIM from the queue and stitches up the loose ends. */
void strout_vim_remove(strout_vim_t *this)
{
    strout_vim_t *prev,
                 *next;

    /* Sanity check. */
    if (!this)
    {
        return;
    }

    /* Remove vim and stitch up any hole. */
    prev = this->prev;
    next = this->next;
    SDL_FreeSurface(this->surface);
    FREE(this);

    if (prev)
    {
        prev->next = next;
    }
    else
    {
        strout_vim_queue = next;
    }

    if (next)
    {
        next->prev = prev;
    }
}

/* Removes all VIMs from the queue. */
void strout_vim_reset(void)
{
    strout_vim_t *this,
          *next;

    for (this = strout_vim_queue; this; this = next)
    {
        next = this->next;
        SDL_FreeSurface(this->surface);
        FREE(this);
    }
}

/* Goes through the queue, playing each VIM or removing ones at the end of
 * their life. */
void strout_vim_play(void)
{
    strout_vim_t *this,
                 *next;
    char          buf[TINY_BUF];

    for (this = strout_vim_queue; this; this = next)
    {
        uint16   now;
        sint16   xoff,
                 yoff;
        SDL_Rect dst;

        next = this->next;

        /* Some VIMs have a delayed start. */
        if (LastTick < this->start)
        {
            continue;
        }
        /* Or maybe we're past it's lifetime. */
        else if (LastTick > this->start + this->lifetime)
        {
            strout_vim_remove(this);

            continue;
        }

        now = LastTick - this->start;
        xoff = (sint16)(now * this->xoff);
        yoff = (sint16)(now * this->yoff);
        dst.x = (sint16)(this->x + xoff);
        dst.y = (sint16)(this->y + yoff);

        /* Arbitrary VIMs are 1+ lines of text which need to be readable so
         * just blit this->surface. */
        if (this->mode == VIM_MODE_ARBITRARY)
        {
            SDL_BlitSurface(this->surface, NULL, ScreenSurface, &dst);
        }
        /* But others are just a number solets do some processing. */
        else
        {
            float        sf = 1 - 1 / ((float)this->lifetime / (float)now);
            SDL_Surface *surface = SPG_Scale(this->surface, sf, sf);

            dst.x += (sint16)((this->surface->w - surface->w) * 0.5);
            dst.y += (sint16)((this->surface->h - surface->h) * 0.5);
            SDL_BlitSurface(surface, NULL, ScreenSurface, &dst);
            SDL_FreeSurface(surface);
        }
    }
}
