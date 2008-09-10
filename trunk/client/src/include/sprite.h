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

#if !defined(__SPRITE_H)
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

/* use special values from BLTFX structures */
#define BLTFX_FLAG_NORMAL 0
#define BLTFX_FLAG_DARK 1
#define BLTFX_FLAG_SRCALPHA 2
#define BLTFX_FLAG_FOW 4
#define BLTFX_FLAG_RED 8
#define BLTFX_FLAG_GREY 16
#define BLTFX_FLAG_STRETCH 32

/* here we can change default blt options or set special options */
typedef struct _BLTFX
{
    UINT32          flags;           /* used from BLTFX_FLAG_xxxx */
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
    SDL_Surface    *red;                    /* red (infravision) */
    SDL_Surface    *grey;                       /* grey (xray) */
    SDL_Surface    *fog_of_war;             /* thats the fog of war palette */
}
_Sprite;



typedef struct _Font
{
    _Sprite    *sprite; /* don't free this, we link here a Bitmaps[x] ptr*/
    int         char_offset;    /* space in pixel between 2 chars in a word */
    SDL_Rect    c[256];
}
_Font;

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
    unsigned int redscales;
    unsigned int greyscales;
    unsigned int fowscales;
    unsigned int loadedsprites;
    unsigned int truecolors;
}
_imagestats;

extern struct _imagestats ImageStats;


extern struct _anim    *start_anim; /* anim queue of current active map */

extern struct _anim    *add_anim(int type, int x, int y, int mapx, int mapy, int value);
extern void             remove_anim(struct _anim *anim);
extern void             play_anims(int mx, int my);
extern void             delete_anim_que(void);
extern void             show_tooltip(int mx, int my, char *text);
extern Boolean          sprite_init_system(void);
extern Boolean          sprite_deinit_system(void);

extern _Sprite         *sprite_load_file(char *fname, UINT32 flags);
extern _Sprite         *sprite_tryload_file(char *fname, UINT32 flags, SDL_RWops *rwob);
extern void             sprite_free_sprite(_Sprite *sprite);
extern int              get_string_pixel_length(char *text, struct _Font *font);
extern void             sprite_blt(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx);
extern void             sprite_blt_map(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx, Uint32 stretch);

extern Uint32           GetSurfacePixel(SDL_Surface *Surface, Sint32 X, Sint32 Y);
extern void             CreateNewFont(_Sprite *sprite, _Font *font, int xlen, int ylen, int c32len);
extern int    StringWidth(_Font *font, char *text);
extern int    StringWidthOffset(_Font *font, char *text, int *line, int len);
extern void             StringBlt(SDL_Surface *surf, _Font *font, char *text, int x, int y, int col, SDL_Rect *area,
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
#define EMBOSS(surf, font, text, x, y, col, area, bltfx) \
        StringBlt((surf), (font), (text), (x) + 1, (y) + 1, COLOR_BLACK, (area), (bltfx)); \
        StringBlt((surf), (font), (text), (x), (y), (col), (area), (bltfx));
/* ENGRAVE prints the string with the shadow on the top and left. */
#define ENGRAVE(surf, font, text, x, y, col, area, bltfx) \
        StringBlt((surf), (font), (text), (x), (y), COLOR_BLACK, (area), (bltfx)); \
        StringBlt((surf), (font), (text), (x) + 1, (y) + 1, (col), (area), (bltfx));

#endif
