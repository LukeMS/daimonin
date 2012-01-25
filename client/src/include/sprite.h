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

#ifndef __SPRITE_H
#define __SPRITE_H

/* our blt & sprite structure */

/* status of this bitmap/sprite */
typedef enum _sprite_status
{
    SPRITE_STATUS_UNLOADED,
    SPRITE_STATUS_LOADED
}    _sprite_status;

/* some other infos */
typedef enum _sprite_type
{
    SPRITE_TYPE_NORMAL
}    _sprite_type;

typedef enum sprite_colrscale_t
{
    SPRITE_COLRSCALE_NONE,
    SPRITE_COLRSCALE_GREY,
    SPRITE_COLRSCALE_SEPIA,
}
sprite_colrscale_t;

/* use special values from BLTFX structures */
#define BLTFX_FLAG_NORMAL      0
#define BLTFX_FLAG_DARK        (1 << 0)
#define BLTFX_FLAG_SRCALPHA    (1 << 1)
#define BLTFX_FLAG_FOGOFWAR    (1 << 2)
#define BLTFX_FLAG_INFRAVISION (1 << 3)
#define BLTFX_FLAG_XRAYVISION  (1 << 4)
#define BLTFX_FLAG_STRETCH     (1 << 5)

/* here we can change default blt options or set special options */
typedef struct _BLTFX
{
    uint32          flags;           /* used from BLTFX_FLAG_xxxx */
    SDL_Surface    *surface;    /* if != null, overrule default screen */
    int             dark_level;         /* use dark_level[i] surface */
    uint8           alpha;
}
_BLTFX;


/* the structure */
typedef struct _Sprite
{
    _sprite_status  status;
    _sprite_type    type;
    int             border_up;                          /* rows of blank pixels before first color information */
    int             border_down;                        /* a blank sprite has borders = 0 */
    int             border_left;
    int             border_right;
    /* we stored our faces 7 times...
     * Perhaps we will run in memory problems when we boost the arch set.
     * atm, we have around 15-25mb when we loaded ALL arches (what perhaps
     * never will happens in a single game
     *Later perhaps a smarter system, using the palettes and switch...
     */
    SDL_Surface    *bitmap;                 /* thats our native, unchanged bitmap*/
    SDL_Surface    *fogofwar;
    SDL_Surface    *infravision;
    SDL_Surface    *xrayvision;
}
_Sprite;

#define ANIM_DAMAGE         1
#define ANIM_KILL           2
#define ANIM_SELF_DAMAGE    3

typedef struct _anim
{
    struct _anim           *next;         /* pointer to next anim in que */
    struct _anim           *before;       /* pointer to anim before */
    int                     type;
    uint32                  start_tick;          /* The time we started this anim */
    uint32                  last_tick;           /* This is the end-tick */
    int                     value;                  /* this is the number to display */
    int                     x;                      /* where we are X */
    int                     y;                      /* where we are Y */
    int                     xoff;                   /* movement in X per tick */
    float                   yoff;                   /* movement in y per tick */
    int                     mapx;                   /* map position X */
    int                     mapy;                   /* map position Y */
}
_anim;

#define ASCII_UP 28
#define ASCII_DOWN 29
#define ASCII_LEFT 30
#define ASCII_RIGHT 31

/* this surface is only 1x1 pixel, and only used to not
 * everytime recreate a certain pixelformat
 */
extern SDL_Surface  *FormatHolder;


typedef struct _imagestats
{
    uint32 loadedsprites;
    uint32 truecolors;
    uint32 fogofwars;
    uint32 infravisions;
    uint32 xrayvisions;
}
_imagestats;

extern struct _imagestats ImageStats;

typedef enum sprite_icon_type_t
{
    SPRITE_ICON_TYPE_NONE,
    SPRITE_ICON_TYPE_INACTIVE,
    SPRITE_ICON_TYPE_ACTIVE,
    SPRITE_ICON_TYPE_NEGATIVE,
    SPRITE_ICON_TYPE_POSITIVE,
}
sprite_icon_type_t;

extern struct _anim    *start_anim; /* anim queue of current active map */

extern struct _anim    *add_anim(int type, int x, int y, int mapx, int mapy, int value);
extern void             remove_anim(struct _anim *anim);
extern void             play_anims(int mx, int my);
extern void             delete_anim_que(void);
extern void             show_tooltip(int mx, int my, char *text);
extern uint8          sprite_init_system(void);
extern uint8          sprite_deinit_system(void);

extern _Sprite         *sprite_load(char *fname, SDL_RWops *rwob);
extern void             sprite_free_sprite(_Sprite *sprite);
extern void sprite_blt_as_icon(_Sprite *sprite, sint16 x, sint16 y,
                               sprite_icon_type_t type, uint8 selected,
                               uint32 flags, uint8 quacon, sint32 quantity,
                               _BLTFX *bltfx);
extern void             sprite_blt(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx);
extern void             sprite_blt_map(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx, Uint32 stretch);

extern Uint32           GetSurfacePixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);
extern int              string_width(_font *font, char *text);
extern int              string_width_offset(_font *font, char *text, int *line, int len);
extern void             string_blt(SDL_Surface *surf, _font *font, char *text,
                                   int x, int y, uint32 col, SDL_Rect *area,
                                   _BLTFX *bltfx);
extern int              sprite_collision(int x1, int y1, int x2, int y2, _Sprite *sprite1, _Sprite *sprite2);
extern Uint32           getpixel(SDL_Surface *surface, int x, int y);
extern void             putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

extern void             sprite_clear_backbuffer(void);

/* Zoom stuff */
#ifndef M_PI
#define M_PI	3.141592654
#endif

/* ---- Structures */

typedef struct tColorRGBA {
Uint8 r;
Uint8 g;
Uint8 b;
Uint8 a;
} tColorRGBA;

typedef struct tColorY {
Uint8 y;
} tColorY;

extern  SDL_Surface     *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth);
extern  void            zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight);
int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy, int smooth);
int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy);

/* Macros for the oft-used technique of printing strings twice, the first black
 * layer slightly offset from the second coloured layer to give a clearer,
 * slightly 3d 'shadow' effect. */
/* EMBOSS prints the string with the shadow on the bottom and right. */
#define EMBOSS(surf, font, text, x, y, colr, area, bltfx) \
        string_blt((surf), (font), (text), (x) + 1, (y) + 1, NDI_COLR_BLACK, (area), (bltfx)); \
        string_blt((surf), (font), (text), (x), (y), (colr), (area), (bltfx));
/* ENGRAVE prints the string with the shadow on the top and left. */
#define ENGRAVE(surf, font, text, x, y, colr, area, bltfx) \
        string_blt((surf), (font), (text), (x), (y), NDI_COLR_BLACK, (area), (bltfx)); \
        string_blt((surf), (font), (text), (x) + 1, (y) + 1, (colr), (area), (bltfx));

#endif /* ifndef __SPRITE_H */
