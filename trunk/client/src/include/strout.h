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

#ifndef __STROUT_H
#define __STROUT_H

/* Macros for the oft-used technique of printing strings twice, the first black
 * layer slightly offset from the second coloured layer to give a clearer,
 * slightly 3d 'shadow' effect.
 *
 * NOTE: These macros blit directly to surface. */
/* EMBOSS prints the string with the shadow on the bottom and right. */
#define EMBOSS(surface, font, text, x, y, colr) \
    strout_blt((surface), (font), (text), (x) + 1, (y) + 1, NDI_COLR_BLACK, \
               NULL); \
    strout_blt((surface), (font), (text), (x), (y), (colr), NULL);
/* ENGRAVE prints the string with the shadow on the top and left. */
#define ENGRAVE(surface, font, text, x, y, colr) \
    strout_blt((surface), (font), (text), (x), (y), NDI_COLR_BLACK, NULL); \
    strout_blt((surface), (font), (text), (x) + 1, (y) + 1, (colr), NULL);

/* Queries if the mouse is over an item/skill/spell at x,y. */
#define STROUT_TOOLTIP_HOVER_TEST(x, y) \
    (global_buttons.mx >= (x) && \
     global_buttons.mx <= (x) + skin_prefs.item_size && \
     global_buttons.my >= (y) && \
     global_buttons.my <= (y) + skin_prefs.item_size)

typedef enum strout_vim_mode_t
{
    VIM_MODE_KILL,
    VIM_MODE_DAMAGE_OTHER,
    VIM_MODE_DAMAGE_SELF,
    VIM_MODE_ARBITRARY,
}
strout_vim_mode_t;

typedef struct strout_vim_t
{
    struct strout_vim_t *next;
    struct strout_vim_t *prev;

    strout_vim_mode_t  mode;
    uint8              mapx;
    uint8              mapy;
    SDL_Surface       *surface;
    uint16             lifetime;
    uint32             start;
    sint16             x;
    sint16             y;
    float              xoff;
    float              yoff;
}
strout_vim_t;

typedef struct strout_tooltip_t
{
    uint32       tick;
    char        *text;
    SDL_Surface *surface;
}
strout_tooltip_t;

extern strout_vim_t     *strout_vim_queue;
extern strout_tooltip_t  strout_tooltip;

extern sint16        strout_width(_font *font, char *text);
extern uint8         strout_width_offset(_font *font, char *text, sint16 *line,
                                         sint16 len);
extern void          strout_blt(SDL_Surface *surface, _font *font, char *text,
                                sint16 x, sint16 y, uint32 colr,
                                SDL_Rect *area);
extern void          strout_input(_font *font, SDL_Rect *box, char repl);
extern strout_vim_t *strout_vim_add(strout_vim_mode_t mode, uint8 mapx,
                                           uint8 mapy, char *text, uint32 colr,
                                           uint16 lifetime, uint16 delay);
extern void          strout_vim_remove(strout_vim_t *this);
extern void          strout_vim_reset(void);
extern void          strout_vim_show(void);
extern void          strout_tooltip_reset(void);
extern char         *strout_tooltip_detail_item(item *ip);
extern char         *strout_tooltip_detail_spell(char *name, uint8 class, uint8 group);
extern void          strout_tooltip_prepare(char *text);
extern void          strout_tooltip_show(void);

#endif /* ifndef __STROUT_H */
