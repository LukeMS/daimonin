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
#include <include.h>

static uint8 DarkAlpha[DARK_LEVELS] =
{
    0,
    44,
    80,
    117,
    153,
    190,
    226
};

static SDL_Surface *DarkMask[DARK_LEVELS] =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

struct _imagestats  ImageStats;

static uint8        GetBitmapBorders(SDL_Surface *Surface, int *up, int *down,
                                     int *left, int *right, uint32 ckey);
static SDL_Surface *zoomSurface(SDL_Surface *src, double zoomx, double zoomy,
                                int smooth);
static void         zoomSurfaceSize(int width, int height, double zoomx,
                                    double zoomy, int *dstwidth,
                                    int *dstheight);
static int          zoomSurfaceY(SDL_Surface *src, SDL_Surface *dst, int flipx,
                                 int flipy);
static int          zoomSurfaceRGBA(SDL_Surface *src, SDL_Surface *dst,
                                    int flipx, int flipy, int smooth);
static SDL_Surface *RecolourSurface(SDL_Surface *src, sprite_colrscale_t scale,
                                    uint32 mask);
static uint32       GetSurfacePixel(SDL_Surface *surface, uint16 x, uint16 y);
static void         PutSurfacePixel(SDL_Surface *surface, uint16 x, uint16 y,
                             uint32 pixel);
static Uint16       CalcHash(const SDL_Surface *src,
                             Uint32 stretch, Uint32 darkness);
static SDL_Surface *check_stretch_cache(const SDL_Surface *src, Uint32 stretch,
                                        Uint32 darkness);
static void         add_to_stretch_cache(SDL_Surface *src, SDL_Surface *dest,
                                         Uint32 stretch, Uint32 darkness);
static void         stretch_init(void);

/* not much special inside atm */
void sprite_init(void)
{
    sprite_clear_backbuffer();
}

void sprite_clear_backbuffer(void)
{
    memset(&ImageStats, 0, sizeof(_imagestats));
    stretch_init();
}

void sprite_deinit(void)
{
    uint8 i;

    for (i = 0; i < DARK_LEVELS; i++)
    {
        SDL_FreeSurface(DarkMask[i]);
        DarkMask[i] = NULL;
    }

    strout_vim_reset();
}

_Sprite *sprite_load(char *fname, SDL_RWops *rwop)
{
    _Sprite     *sprite;
    SDL_Surface *bitmap;
    SDL_RWops   *rw;

    if (fname)
    {
        if (!PHYSFS_exists(fname))
        {
            LOG(LOG_MSG,"file: %s does not exist in physfs\n", fname);

            return NULL;
        }
        else
        {
            if (!(rw = PHYSFSRWOPS_openRead(fname)))
            {
                LOG(LOG_ERROR, "PHYSFSRWOPS_openRead failed: %s\n",
                    PHYSFS_getLastError());
            }

            if (!(bitmap = IMG_Load_RW(rw, 0)))
            {
                return NULL;
            }

            SDL_RWclose(rw);
        }
    }
    else
    {
        bitmap = IMG_LoadPNG_RW(rwop);
    }

    MALLOC(sprite, sizeof(_Sprite));
    sprite->status = SPRITE_STATUS_LOADED;
    GetBitmapBorders(bitmap, &sprite->border_up, &sprite->border_down,
                     &sprite->border_left, &sprite->border_right, 0);
    sprite->bitmap = bitmap;
    ImageStats.bitmaps++;

    return sprite;
}

/* Recolours src pixel-by-pixel according to scale and/or mask and
 * returns a pointer to the recoloured surface. */
static SDL_Surface *RecolourSurface(SDL_Surface *src, sprite_colrscale_t scale,
                                    uint32 mask)
{
    uint16       y;
    SDL_Surface *dst;

    if (!src)
    {
        return NULL;
    }

    dst = SDL_DisplayFormatAlpha(src);
    SDL_LockSurface(dst);

    for (y = 0; y < dst->h; y++)
    {
        uint16 x;

        for (x = 0; x < dst->w; x++)
        {
            uint32 p = GetSurfacePixel(dst, x, y);
            uint8  r,
                   g,
                   b,
                   a,
                   tr,
                   tg,
                   tb;

            SDL_GetRGBA(p, dst->format, &r, &g, &b, &a);

            /* No point recolouring pixels you can't see anyway. */
            if (a == SDL_ALPHA_TRANSPARENT)
            {
                continue;
            }

            switch (scale)
            {
                case SPRITE_COLRSCALE_GREY:
                case SPRITE_COLRSCALE_INTENSITY:
                    r = g = b = (uint8)(0.3 * r + 0.59 * g + 0.11 * b);

                    break;

                case SPRITE_COLRSCALE_SEPIA:
                    tr = (uint8)(0.393 * r + 0.769 * g + 0.189 * b);
                    tg = (uint8)(0.349 * r + 0.686 * g + 0.168 * b);
                    tb = (uint8)(0.272 * r + 0.534 * g + 0.131 * b);
                    r = tr;
                    g = tg;
                    b = tb;

                    break;

                case SPRITE_COLRSCALE_NEGATIVE:
                    r = g = b = 255 - (uint8)(0.3 * r + 0.59 * g + 0.11 * b);

                    break;

                case SPRITE_COLRSCALE_INVERSION:
                    r = 255 - r;
                    g = 255 - g;
                    b = 255 - b;

                    break;

                default:
                    break;
            }

            if (mask)
            {
                uint8 mr = (mask >> 16) & 0xff,
                      mg = (mask >> 8) & 0xff,
                      mb = mask & 0xff;

                if (scale == SPRITE_COLRSCALE_INTENSITY)
                {
                    if ((mr = mg = mb = (uint8)(0.3 * mr + 0.59 * mg + 0.11 * mb)) >= 128)
                    {
                        r = (r + mr >= 255) ? 255 : r + mr;
                        g = (g + mg >= 255) ? 255 : g + mg;
                        b = (b + mb >= 255) ? 255 : b + mb;
                    }
                    else
                    {
                        r = (r - mr <= 0) ? 0 : r - mr;
                        g = (g - mg <= 0) ? 0 : g - mg;
                        b = (b - mb <= 0) ? 0 : b - mb;
                    }
                }
                else
                {
                    r &= mr;
                    g &= mg;
                    b &= mb;
                }
            }

            p = SDL_MapRGBA(dst->format, r, g, b, a);
            PutSurfacePixel(dst, x, y, p);
        }
    }

    SDL_UnlockSurface(dst);

    return dst;
}

/* This two helper functions are fast, but be careful:
 * x,y must be on the surface!
 * When using with the display surface the surface MUST be locked! */
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

static void PutSurfacePixel(SDL_Surface *surface, uint16 x, uint16 y,
                            uint32 pixel)
{
    uint8  bpp = surface->format->BytesPerPixel,
    /* Here p is the address to the pixel we want to set */
          *p = surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
        case 1:
            *p = pixel;

            break;

        case 2:
            *(uint16 *)p = pixel;

            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }

            break;

        case 4:
            *(uint32 *)p = pixel;

            break;
    }
}

/* Frees a sprite and all its surfaces. */
void sprite_free_sprite(_Sprite *sprite)
{
    if (!sprite)
    {
        return;
    }

    if (sprite->bitmap)
    {
        SDL_FreeSurface(sprite->bitmap);
        sprite->bitmap = NULL;
        ImageStats.bitmaps--;
    }

    sprite_free_surfaces(sprite);
    FREE(sprite);
}

/* Frees all secondary surfaces associated with a sprite, bur not the sprite
 * itself. */
void sprite_free_surfaces(_Sprite *sprite)
{
    if (!sprite)
    {
        return;
    }

    if (sprite->scaled)
    {
        SDL_FreeSurface(sprite->scaled);
        sprite->scaled = NULL;
        ImageStats.scaleds--;
    }

    if (sprite->fire)
    {
        SDL_FreeSurface(sprite->fire);
        sprite->fire = NULL;
        ImageStats.fires--;
    }

    if (sprite->cold)
    {
        SDL_FreeSurface(sprite->cold);
        sprite->cold = NULL;
        ImageStats.colds--;
    }

    if (sprite->electricity)
    {
        SDL_FreeSurface(sprite->electricity);
        sprite->electricity = NULL;
        ImageStats.electricities--;
    }

    if (sprite->light)
    {
        SDL_FreeSurface(sprite->light);
        sprite->light = NULL;
        ImageStats.lights--;
    }

    if (sprite->shadow)
    {
        SDL_FreeSurface(sprite->shadow);
        sprite->shadow = NULL;
        ImageStats.shadows--;
    }

    if (sprite->fogofwar)
    {
        SDL_FreeSurface(sprite->fogofwar);
        sprite->fogofwar = NULL;
        ImageStats.fogofwars--;
    }

    if (sprite->infravision)
    {
        SDL_FreeSurface(sprite->infravision);
        sprite->infravision = NULL;
        ImageStats.infravisions--;
    }

    if (sprite->xrayvision)
    {
        SDL_FreeSurface(sprite->xrayvision);
        sprite->xrayvision = NULL;
        ImageStats.xrayvisions--;
    }
}

static uint8 GetBitmapBorders(SDL_Surface *Surface, int *up, int *down, int *left, int *right, uint32 ckey)
{
    register int x,y;

    *up = 0;
    *down = 0;
    *left = 0;
    *right = 0;

    /* left side border */
    for (x = 0; x < Surface->w; x++)
    {
        for (y = 0; y < Surface->h; y++)
        {
            if (GetSurfacePixel(Surface, x, y) != ckey)
            {
                *left = x;
                goto right_border;
            }
        }
    }

    /* we only need check this one time here - if we are here, the sprite is blank */
    return 0;

right_border:
    /* right side border */
    for (x = Surface->w - 1; x >= 0; x--)
    {
        for (y = 0; y < Surface->h; y++)
        {
            if (GetSurfacePixel(Surface, x, y) != ckey)
            {
                *right = (Surface->w - 1) - x;
                goto up_border;
            }
        }
    }

up_border:
    /* up side border */
    for (y = 0; y < Surface->h; y++)
    {
        for (x = 0; x < Surface->w; x++)
        {
            if (GetSurfacePixel(Surface, x, y) != ckey)
            {
                *up = y;
                goto down_border;
            }
        }
    }

down_border:
    /* up side border */
    for (y = Surface->h - 1; y >= 0; y--)
    {
        for (x = 0; x < Surface->w; x++)
        {
            if (GetSurfacePixel(Surface, x, y) != ckey)
            {
                *down = (Surface->h - 1) - y;
                return 1;
            }
        }
    }
    return 1;
}

void sprite_blt_as_icon(_Sprite *sprite, sint16 x, sint16 y,
                        sprite_icon_type_t type, uint8 selected,
                        uint32 flags, uint8 quacon, sint32 quantity,
                        _BLTFX *bltfx)
{
    _Sprite     *bg = NULL,
                *fg = NULL;
    SDL_Surface *surface = (bltfx && bltfx->surface)
                           ? bltfx->surface : ScreenSurface;

    /* Set bg and fg according to type. */
    if (type != SPRITE_ICON_TYPE_NONE)
    {
        if (type == SPRITE_ICON_TYPE_INACTIVE)
        {
            bg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_BG_INACTIVE];
            fg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_FG_INACTIVE];
        }
        else if (type == SPRITE_ICON_TYPE_ACTIVE)
        {
            bg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_BG_ACTIVE];
            fg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_FG_ACTIVE];
        }
        else if (type == SPRITE_ICON_TYPE_POSITIVE)
        {
            bg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_BG_POSITIVE];
            fg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_FG_ACTIVE];
        }
        else if (type == SPRITE_ICON_TYPE_NEGATIVE)
        {
            bg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_BG_NEGATIVE];
            fg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_FG_ACTIVE];
        }
        else
        {
            LOG(LOG_ERROR, "Unhandled icon type %u!\n", type);
        }
    }

    /* If selected, override fg. */
    if (selected)
    {
        fg = skin_sprites[SKIN_SPRITE_DIALOG_ICON_FG_SELECTED];
    }

    /* Blt bg. */
    sprite_blt(bg, x, y, NULL, bltfx);

    /* When we have no sprite or bitmap, use SKIN_SPRITE_LOADING. */
    if (!sprite ||
        !sprite->bitmap)
    {
        sprite = skin_sprites[SKIN_SPRITE_LOADING];
    }

    /* Blt sprite, centered. */
    if (sprite)
    {
        sint16    bw = (sprite->bitmap->w - sprite->border_right) -
                       sprite->border_left,
                  bh = (sprite->bitmap->h - sprite->border_down) -
                       sprite->border_up;
        SDL_Rect  box;
        _BLTFX   *bltfx_local;

        box.x = sprite->border_left + ((bw > 32) ? (bw - 32) / 2 : 0);//skin_prefs.iconsize;
        box.y = sprite->border_up + ((bh > 32) ? (bh - 32) / 2 : 0);//skin_prefs.iconsize;
        box.w = (bw > 32) ? 32 : (uint16)bw;//skin_prefs.iconsize;
        box.h = (bh > 32) ? 32 : (uint16)bh;//skin_prefs.iconsize;
        MALLOC(bltfx_local, sizeof(_BLTFX));

        if (bltfx)
        {
            memcpy(bltfx_local, bltfx, sizeof(_BLTFX));
        }

        if (type == SPRITE_ICON_TYPE_INACTIVE)
        {
            bltfx_local->flags |= BLTFX_FLAG_XRAYVISION;
        }

        sprite_blt(sprite,
                   x + ((bw <= 32) ? (32 - bw) / 2 : 0),//skin_prefs.iconsize,
                   y + ((bh <= 32) ? (32 - bh) / 2 : 0),//skin_prefs.iconsize,
                   &box, bltfx_local);
        FREE(bltfx_local);
    }

    /* Blt fg. */
    sprite_blt(fg, x - 2, y - 2, NULL, bltfx);

    /* Show flags. */
    if (flags)
    {
        /* bottom left */
        if ((flags & F_LOCKED))
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_LOCK], x, y + skin_prefs.item_size -
                       skin_prefs.icon_size, NULL, bltfx);
        }

        /* top left */
        /* applied and unpaid some spot - can't apply unpaid items */
        if ((flags & F_APPLIED))
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_APPLY], x, y, NULL, bltfx);
        }
        else if ((flags & F_UNPAID))
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_UNPAID], x, y, NULL, bltfx);
        }

        /* right side, top to bottom */
        if (quacon == 255)
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_UNIDENTIFIED],
                       x + skin_prefs.item_size - skin_prefs.icon_size - 2, y, NULL,
                       bltfx);
        }

        if ((flags & F_MAGIC))
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_MAGIC],
                       x + skin_prefs.item_size - skin_prefs.icon_size - 2,
                       y + skin_prefs.icon_size, NULL, bltfx);
        }

        if ((flags & F_CURSED))
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_CURSED],
                       x + skin_prefs.item_size - skin_prefs.icon_size - 2,
                       y + skin_prefs.icon_size * 2, NULL, bltfx);
        }

        if ((flags & F_DAMNED))
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_DAMNED],
                       x + skin_prefs.item_size - skin_prefs.icon_size - 2,
                       y + skin_prefs.icon_size * 3, NULL, bltfx);
        }

        /* central */
        if ((flags & F_TRAPED))
        {
            sprite_blt(skin_sprites[SKIN_SPRITE_TRAPED], x + 8, y + 7, NULL, bltfx);
        }
    }

    /* Show quacon. */
    if (options.showqc &&
        quacon > 0 &&
        quacon <= 100)
    {
        SDL_Rect box;
        sint8    con = MAX(0, MIN((float)quacon / 100.0 * 30, 30));//skin_prefs.iconsize - 2
        uint32   colr = ((quacon >= 100)
                         ? 0x00ff00 : ((quacon >= options.itemdmg_limit_orange)
                         ? 0xffff00 : ((quacon >= options.itemdmg_limit_red)
                         ? 0xff6d00 : 0xff0000)));

        box.x = x + 30;//skin_prefs.iconsize - 2
        box.y = y + (30 - con);//skin_prefs.iconsize - 2
        box.w = 1;
        box.h = (uint16)con;
        colr = SDL_MapRGB(surface->format, (colr >> 16) & 0xff,
                          (colr >> 8) & 0xff, colr & 0xff);
        SDL_FillRect(surface, &box, colr);
    }

    /* Icon quantity. */
    if (quantity)
    {
        char   buf[TINY_BUF];
        uint8  w;
        uint32 colr = (quantity > 0) ? NDI_COLR_LIME : NDI_COLR_RED;

        if (quantity > 9999 ||
            quantity < -9999)
        {
            sprintf(buf, "many");
        }
        else
        {
            sprintf(buf, "%d", quantity);
        }

        w = strout_width(&font_tiny, buf);
        strout_blt(surface, &font_tiny, STROUT_LEFT, buf,
                   x + ((options.showqc) ? 22 : 24) - w / 2, y + 18, colr,
                   NULL);
    }
}

/* Alderan 2007-11-03: i reworked that a bit:
 * we dont need for every blit check for all the map drawing stuff
 * so the mapdrawing gets its own function. */
void sprite_blt(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx)
{
    SDL_Surface *blt,
                *surface;
    SDL_Rect     dst;

    /* Sanity check. */
    if (!sprite)
    {
        return;
    }

    blt = sprite->bitmap;
    surface = (bltfx &&
               bltfx->surface) ? bltfx->surface : ScreenSurface;
    dst.x = x;
    dst.y = y;

    if (bltfx &&
        (bltfx->flags & BLTFX_FLAG_SRCALPHA) &&
        !(ScreenSurface->flags & SDL_HWSURFACE))
    {
        SDL_SetAlpha(blt, SDL_SRCALPHA, bltfx->alpha);
        SDL_BlitSurface(blt, box, surface, &dst);
        SDL_SetAlpha(blt, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    }
    else
    {
        SDL_BlitSurface(blt, box, surface, &dst);
    }
}

/* This function supports the whole BLTFX flags, and is only used to blit the map! */
void sprite_blt_map(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx, Uint32 stretch)
{
    SDL_Rect        dst;
    SDL_Surface    *surface, *blt_sprite, *tmp;
    uint8         reset_trans = 0;
    uint8         need_stretch = 0;

    if (!sprite)
        return;

    if (options.map_scalex != 100 ||
        options.map_scaley != 100 ||
        options.map_aa)
    {
        if (!sprite->scaled)
        {
#if 0
            SDL_Surface *dfa = SDL_DisplayFormatAlpha(sprite->bitmap),
                        *scaled;

            if (options.map_aa)
            {
                scaled = SPG_ScaleAA(dfa, options.mapsx, options.mapsy);
            }
            else
            {
                scaled = SPG_Scale(dfa, options.mapsx, options.mapsy);
            }

            SDL_FreeSurface(dfa);
            sprite->scaled = SDL_DisplayFormatAlpha(scaled);
//LOG(LOG_MSG,"s->b: %d %d, scaled: %d %d, s->s: %d %d\n", sprite->bitmap->w, sprite->bitmap->h, scaled->w, scaled->h, sprite->scaled->w, sprite->scaled->h);
            SDL_FreeSurface(scaled);
#else
            if (sprite->bitmap->format->BitsPerPixel == 32)
            {
                sprite->scaled = zoomSurface(sprite->bitmap, options.mapsx,
                                             options.mapsy, options.map_aa);
            }
            else
            {
                if (options.map_aa)
                {
                    SDL_Surface *dfa = SDL_DisplayFormatAlpha(sprite->bitmap);

                    sprite->scaled = zoomSurface(dfa, options.mapsx,
                                                 options.mapsy, 1);
                    SDL_FreeSurface(dfa);
                }
                else
                {
                    sprite->scaled = zoomSurface(sprite->bitmap, options.mapsx,
                                                 options.mapsy, 0);
                }
            }
#endif
            ImageStats.scaleds++;
        }

        blt_sprite = sprite->scaled;
    }
    else
    {
        blt_sprite = sprite->bitmap;
    }

    surface = ScreenSurfaceMap;
    dst.x = x;
    dst.y = y;

    if (bltfx)
    {
        /* with dark flag we newer check here for the stretch flag, if stretch is 0 no strecthing is done... */
        if (bltfx->flags & BLTFX_FLAG_DARK)
        {
            /* last dark level is "no color" ... */
            if (bltfx->dark_level == DARK_LEVELS)
                return;

            /* we create the filter surfaces only when needed, and then store them */
            if (!DarkMask[bltfx->dark_level])
            {
                SDL_SetAlpha(skin_sprites[SKIN_SPRITE_ALPHA]->bitmap, SDL_SRCALPHA,
                             DarkAlpha[bltfx->dark_level]);
                DarkMask[bltfx->dark_level] = SDL_DisplayFormatAlpha(skin_sprites[SKIN_SPRITE_ALPHA]->bitmap);
            }

// TODO: Remove tilestretcher!
#if 0
            /* we use now the stretch_cache with lru list */
            tmp = check_stretch_cache(blt_sprite, stretch, bltfx->dark_level);

            if (tmp)
            {
                dst.y = dst.y - (tmp->h - sprite->bitmap->h);
                blt_sprite = tmp;
            }
            else /* we create the surface, and put it in hashtable */
            {
                blt_sprite = SDL_DisplayFormatAlpha(sprite->bitmap);
                SDL_BlitSurface(DarkMask[bltfx->dark_level], NULL,
                                blt_sprite, NULL);

                /* lets check for stretching... */
                if ((bltfx->flags & BLTFX_FLAG_STRETCH)) // We need to stretch, but lets check the cache 1st
                {
                    Uint8 *ht = (Uint8*)&stretch;
                    Uint8 n = *(ht+3);
                    Uint8 e = *(ht+2);
                    Uint8 w = *(ht+1);
                    Uint8 s = *ht;
                    int ht_diff;

                    LOG(LOG_MSG,"outcoding stretch=%d N=%d, E=%d, W=%d, S=%d (src1: %p, src2: %p, dark: %d)\n",stretch,n,e,w,s, blt_sprite, sprite->bitmap, bltfx->dark_level);
                    tmp = tile_stretch(blt_sprite,n,e,s,w);
                    ht_diff = (tmp->h - sprite->bitmap->h);  // tiles never shrink, just get bigger

                    if (tmp==NULL) return;  // we didn't get a bmp back

                    SDL_FreeSurface(blt_sprite);
                    blt_sprite = tmp;
                    dst.y = dst.y - ht_diff;
                }

                /* we put it in the hashtable*/
                add_to_stretch_cache(sprite->bitmap,blt_sprite,stretch, bltfx->dark_level);
            }
#endif

            if (options.combat_smackvatts)
            {
                if ((map_redraw_flag & MAP_REDRAW_FLAG_FIRE))
                {
                    if (!sprite->fire)
                    {
                        sprite->fire = RecolourSurface(blt_sprite,
                                                       skin_prefs.scale_fire,
                                                       skin_prefs.mask_fire);
                        ImageStats.fires++;
                    }

                    blt_sprite = sprite->fire;
                }
                else if ((map_redraw_flag & MAP_REDRAW_FLAG_COLD))
                {
                    if (!sprite->cold)
                    {
                        sprite->cold = RecolourSurface(blt_sprite,
                                                       skin_prefs.scale_cold,
                                                       skin_prefs.mask_cold);
                        ImageStats.colds++;
                    }

                    blt_sprite = sprite->cold;
                }
                else if ((map_redraw_flag & MAP_REDRAW_FLAG_ELECTRICITY))
                {
                    if (!sprite->electricity)
                    {
                        sprite->electricity = RecolourSurface(blt_sprite,
                                                              skin_prefs.scale_electricity,
                                                              skin_prefs.mask_electricity);
                        ImageStats.electricities++;
                    }

                    blt_sprite = sprite->electricity;
                }
                else if ((map_redraw_flag & MAP_REDRAW_FLAG_LIGHT))
                {
                    if (!sprite->light)
                    {
                        sprite->light = RecolourSurface(blt_sprite,
                                                        skin_prefs.scale_light,
                                                        skin_prefs.mask_light);
                        ImageStats.lights++;
                    }

                    blt_sprite = sprite->light;
                }
                else if ((map_redraw_flag & MAP_REDRAW_FLAG_SHADOW))
                {
                    if (!sprite->shadow)
                    {
                        sprite->shadow = RecolourSurface(blt_sprite,
                                                         skin_prefs.scale_shadow,
                                                         skin_prefs.mask_shadow);
                        ImageStats.shadows++;
                    }

                    blt_sprite = sprite->shadow;
                }
            }
        }
        else
        {
            if ((bltfx->flags & BLTFX_FLAG_FOGOFWAR))
            {
                if (!sprite->fogofwar)
                {
                    sprite->fogofwar = RecolourSurface(blt_sprite,
                                                       skin_prefs.scale_fogofwar,
                                                       skin_prefs.mask_fogofwar);
                    ImageStats.fogofwars++;
                }

                blt_sprite = sprite->fogofwar;
                need_stretch = 1;
            }
            else if ((bltfx->flags & BLTFX_FLAG_INFRAVISION))
            {
                if (!sprite->infravision)
                {
                    sprite->infravision = RecolourSurface(blt_sprite,
                                                          skin_prefs.scale_infravision,
                                                          skin_prefs.mask_infravision);
                    ImageStats.infravisions++;
                }

                blt_sprite = sprite->infravision;
                need_stretch = 1;
            }
            else if ((bltfx->flags & BLTFX_FLAG_XRAYVISION))
            {
                if (!sprite->xrayvision)
                {
                    sprite->xrayvision = RecolourSurface(blt_sprite,
                                                         skin_prefs.scale_xrayvision,
                                                         skin_prefs.mask_xrayvision);
                    ImageStats.xrayvisions++;
                }

                blt_sprite = sprite->xrayvision;
                need_stretch = 1;
            }
        }

        if (!blt_sprite)
        {
            return;
        }

// TODO: Remove tilestretcher!
#if 0
        if (need_stretch && bltfx->flags & BLTFX_FLAG_STRETCH)
        {
            tmp = check_stretch_cache(blt_sprite,stretch, 0); //no darkness...

            if (tmp==NULL)  // we were not successsful getting it from cache :(
            {
                Uint8 n = (stretch>>24) & 0xFF;
                Uint8 e = (stretch>>16) & 0xFF;
                Uint8 w = (stretch>>8)  & 0xFF;
                Uint8 s = stretch & 0xFF;

                int ht_diff;

                printf("outcoding stretch=%d N=%d, E=%d, W=%d, S=%d (src: %p)\n",stretch,n,e,w,s,blt_sprite);
                tmp = tile_stretch(blt_sprite,n,e,s,w);

                ht_diff = tmp->h - blt_sprite->h;  // tiles never shrink, just get bigger

                if (tmp==NULL) return;  // we didn't get a bmp back

                add_to_stretch_cache(blt_sprite, tmp, stretch, 0); // cache it for next time

                blt_sprite = tmp; // we keep the created surface, surely we need it later
                dst.y = dst.y - ht_diff;
            }
            else  // we have alredy stretch this tile and got it from the cache!
            {
                dst.y = dst.y - ( tmp->h - blt_sprite->h );
                blt_sprite = tmp;
            }
        }
#endif
    }

    if (!blt_sprite)
        return;

    tmp = SDL_DisplayFormatAlpha(blt_sprite);

    if (bltfx &&
        (bltfx->flags & BLTFX_FLAG_SRCALPHA) &&
        !(ScreenSurface->flags & SDL_HWSURFACE))
    {
        SDL_SetAlpha(tmp, SDL_SRCALPHA, bltfx->alpha);
    }

    if (bltfx &&
        (bltfx->flags & BLTFX_FLAG_DARK))
    {
        SDL_BlitSurface(DarkMask[bltfx->dark_level], NULL,
                        tmp, NULL);
    }

    SDL_BlitSurface(tmp, box, surface, &dst);
    SDL_FreeSurface(tmp);
}

/* TODO: Tidy/reformat the nect four functions. */

static SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth) 	 
{ 	 
    SDL_Surface *rz_src; 	 
    SDL_Surface *rz_dst; 	 
    int dstwidth, dstheight; 	 
    int is32bit; 	 
    int i, src_converted; 	 
    int flipx, flipy; 	 
 	 
    /* 	 
     * Sanity check 	 
     */ 	 
    if (src == NULL) 	 
    return (NULL); 	 
 	 
    /* 	 
     * Determine if source surface is 32bit or 8bit 	 
     */ 	 
    is32bit = (src->format->BitsPerPixel == 32); 	 
    if ((is32bit) || (src->format->BitsPerPixel == 8)) { 	 
    /* 	 
     * Use source surface 'as is' 	 
     */ 	 
    rz_src = src; 	 
    src_converted = 0; 	 
    } else { 	 
    /* 	 
     * New source surface is 32bit with a defined RGBA ordering 	 
     */ 	 
    rz_src = 	 
        SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000); 	 
    SDL_BlitSurface(src, NULL, rz_src, NULL); 	 
    src_converted = 1; 	 
    is32bit = 1; 	 
    } 	 
 	 
    flipx = (zoomx<0); 	 
    if (flipx) zoomx = -zoomx; 	 
    flipy = (zoomy<0); 	 
    if (flipy) zoomy = -zoomy; 	 
 	 
    /* Get size if target */ 	 
    zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight); 	 
 	 
    /* 	 
     * Alloc space to completely contain the zoomed surface 	 
     */ 	 
    rz_dst = NULL; 	 
    if (is32bit) { 	 
    /* 	 
     * Target surface is 32bit with source RGBA/ABGR ordering 	 
     */ 	 
    rz_dst = 	 
        SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32, 	 
                 rz_src->format->Rmask, rz_src->format->Gmask, 	 
                 rz_src->format->Bmask, rz_src->format->Amask); 	 
    } else { 	 
    /* 	 
     * Target surface is 8bit 	 
     */ 	 
    rz_dst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 8, 0, 0, 0, 0); 	 
    } 	 
 	 
    /* 	 
     * Lock source surface 	 
     */ 	 
    SDL_LockSurface(rz_src); 	 
    /* 	 
     * Check which kind of surface we have 	 
     */ 	 
    if (is32bit) { 	 
    /* 	 
     * Call the 32bit transformation routine to do the zooming (using alpha) 	 
     */ 	 
    zoomSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth); 	 
    /* 	 
     * Turn on source-alpha support 	 
     */ 	 
    SDL_SetAlpha(rz_dst, SDL_SRCALPHA, 255); 	 
    } else { 	 
    /* 	 
     * Copy palette and colorkey info 	 
     */ 	 
    for (i = 0; i < rz_src->format->palette->ncolors; i++) { 	 
        rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i]; 	 
    } 	 
    rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors; 	 
    /* 	 
     * Call the 8bit transformation routine to do the zooming 	 
     */ 	 
    zoomSurfaceY(rz_src, rz_dst, flipx, flipy); 	 
    SDL_SetColorKey(rz_dst, SDL_SRCCOLORKEY | SDL_RLEACCEL, rz_src->format->colorkey); 	 
    } 	 
    /* 	 
     * Unlock source surface 	 
     */ 	 
    SDL_UnlockSurface(rz_src); 	 
 	 
    /* 	 
     * Cleanup temp surface 	 
     */ 	 
    if (src_converted) { 	 
    SDL_FreeSurface(rz_src); 	 
    } 	 
 	 
    /* 	 
     * Return destination surface 	 
     */ 	 
    return (rz_dst); 	 
} 	 
 	 
#define VALUE_LIMIT 0.001 	 
 	 
static void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight) 	 
{ 	 
    /* 	 
     * Sanity check zoom factors 	 
     */ 	 
    if (zoomx < VALUE_LIMIT) { 	 
    zoomx = VALUE_LIMIT; 	 
    } 	 
    if (zoomy < VALUE_LIMIT) { 	 
    zoomy = VALUE_LIMIT; 	 
    } 	 
 	 
    /* 	 
     * Calculate target size 	 
     */ 	 
    *dstwidth = (int) ((double) width * zoomx); 	 
    *dstheight = (int) ((double) height * zoomy); 	 
    if (*dstwidth < 1) { 	 
    *dstwidth = 1; 	 
    } 	 
    if (*dstheight < 1) { 	 
    *dstheight = 1; 	 
    } 	 
} 	 
 	 
/* 	 
 	 
 8bit Zoomer without smoothing. 	 
 	 
 Zoomes 8bit palette/Y 'src' surface to 'dst' surface. 	 
 	 
*/ 	 
 	 
static int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy) 	 
{ 	 
    Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy; 	 
    Uint8 *sp, *dp, *csp; 	 
    int dgap; 	 
 	 
    /* 	 
     * Variable setup 	 
     */ 	 
    sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w); 	 
    sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h); 	 
 	 
    /* 	 
     * Allocate memory for row increments 	 
     */ 	 
    MALLOC(sax, dst->w * sizeof(Uint32)); 	 
 	 
    if (!sax) 	 
    { 	 
        return -1; 	 
    } 	 
 	 
    MALLOC(say, dst->h * sizeof(Uint32)); 	 
 	 
    if (!say) 	 
    { 	 
        if (sax) 	 
        { 	 
            FREE(sax); 	 
        } 	 
 	 
        return -1; 	 
    } 	 
 	 
    /* 	 
     * Precalculate row increments 	 
     */ 	 
    csx = 0; 	 
    csax = sax; 	 
    for (x = 0; (int) x < dst->w; x++) { 	 
    csx += sx; 	 
    *csax = (csx >> 16); 	 
    csx &= 0xffff; 	 
    csax++; 	 
    } 	 
    csy = 0; 	 
    csay = say; 	 
    for (y = 0; (int) y < dst->h; y++) { 	 
    csy += sy; 	 
    *csay = (csy >> 16); 	 
    csy &= 0xffff; 	 
    csay++; 	 
    } 	 
 	 
    csx = 0; 	 
    csax = sax; 	 
    for (x = 0; (int) x < dst->w; x++) { 	 
    csx += (*csax); 	 
    csax++; 	 
    } 	 
    csy = 0; 	 
    csay = say; 	 
    for (y = 0; (int) y < dst->h; y++) { 	 
    csy += (*csay); 	 
    csay++; 	 
    } 	 
 	 
    /* 	 
     * Pointer setup 	 
     */ 	 
    sp = csp = (Uint8 *) src->pixels; 	 
    dp = (Uint8 *) dst->pixels; 	 
    dgap = dst->pitch - dst->w; 	 
 	 
    /* 	 
     * Draw 	 
     */ 	 
    csay = say; 	 
    for (y = 0; (int) y < dst->h; y++) { 	 
    csax = sax; 	 
    sp = csp; 	 
    for (x = 0; (int) x < dst->w; x++) { 	 
        /* 	 
         * Draw 	 
         */ 	 
        *dp = *sp; 	 
        /* 	 
         * Advance source pointers 	 
         */ 	 
        sp += (*csax); 	 
        csax++; 	 
        /* 	 
         * Advance destination pointer 	 
         */ 	 
        dp++; 	 
    } 	 
    /* 	 
     * Advance source pointer (for row) 	 
     */ 	 
    csp += ((*csay) * src->pitch); 	 
    csay++; 	 
    /* 	 
     * Advance destination pointers 	 
     */ 	 
    dp += dgap; 	 
    } 	 
 	 
    /* 	 
     * Remove temp arrays 	 
     */ 	 
    FREE(sax); 	 
    FREE(say); 	 
 	 
    return (0); 	 
} 	 
 	 
/* 	 
 	 
 32bit Zoomer with optional anti-aliasing by bilinear interpolation. 	 
 	 
 Zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface. 	 
 	 
*/ 	 
 	 
static int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy, int smooth) 	 
{ 	 
    int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep; 	 
    tColorRGBA *c00, *c01, *c10, *c11; 	 
    tColorRGBA *sp, *csp, *dp; 	 
    int dgap; 	 
 	 
    /* 	 
     * Variable setup 	 
     */ 	 
    if (smooth) { 	 
    /* 	 
     * For interpolation: assume source dimension is one pixel 	 
     */ 	 
    /* 	 
     * smaller to avoid overflow on right and bottom edge. 	 
     */ 	 
    sx = (int) (65536.0 * (float) (src->w - 1) / (float) dst->w); 	 
    sy = (int) (65536.0 * (float) (src->h - 1) / (float) dst->h); 	 
    } else { 	 
    sx = (int) (65536.0 * (float) src->w / (float) dst->w); 	 
    sy = (int) (65536.0 * (float) src->h / (float) dst->h); 	 
    } 	 
 	 
    /* 	 
     * Allocate memory for row increments 	 
     */ 	 
    MALLOC(sax, (dst->w + 1) * sizeof(Uint32)); 	 
 	 
    if (!sax) 	 
    { 	 
        return -1; 	 
    } 	 
 	 
    MALLOC(say, (dst->h + 1) * sizeof(Uint32)); 	 
 	 
    if (!say) 	 
    { 	 
        if (sax) 	 
        { 	 
            FREE(sax); 	 
        } 	 
 	 
        return -1; 	 
    } 	 
 	 
    /* 	 
     * Precalculate row increments 	 
     */ 	 
    sp = csp = (tColorRGBA *) src->pixels; 	 
    dp = (tColorRGBA *) dst->pixels; 	 
 	 
    if (flipx) csp += (src->w-1); 	 
    if (flipy) csp  = (tColorRGBA*)( (Uint8*)csp + src->pitch*(src->h-1) ); 	 
 	 
    csx = 0; 	 
    csax = sax; 	 
    for (x = 0; x <= dst->w; x++) { 	 
    *csax = csx; 	 
    csax++; 	 
    csx &= 0xffff; 	 
    csx += sx; 	 
    } 	 
    csy = 0; 	 
    csay = say; 	 
    for (y = 0; y <= dst->h; y++) { 	 
    *csay = csy; 	 
    csay++; 	 
    csy &= 0xffff; 	 
    csy += sy; 	 
    } 	 
 	 
    dgap = dst->pitch - dst->w * 4; 	 
 	 
    /* 	 
     * Switch between interpolating and non-interpolating code 	 
     */ 	 
    if (smooth) { 	 
 	 
    /* 	 
     * Interpolating Zoom 	 
     */ 	 
 	 
    /* 	 
     * Scan destination 	 
     */ 	 
    csay = say; 	 
    for (y = 0; y < dst->h; y++) { 	 
        /* 	 
         * Setup color source pointers 	 
         */ 	 
        c00 = csp; 	 
        c01 = csp; 	 
        c01++; 	 
        c10 = (tColorRGBA *) ((Uint8 *) csp + src->pitch); 	 
        c11 = c10; 	 
        c11++; 	 
        csax = sax; 	 
        for (x = 0; x < dst->w; x++) { 	 
 	 
        /* 	 
         * Interpolate colors 	 
         */ 	 
        ex = (*csax & 0xffff); 	 
        ey = (*csay & 0xffff); 	 
        t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff; 	 
        t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff; 	 
        dp->r = (((t2 - t1) * ey) >> 16) + t1; 	 
        t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff; 	 
        t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff; 	 
        dp->g = (((t2 - t1) * ey) >> 16) + t1; 	 
        t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff; 	 
        t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff; 	 
        dp->b = (((t2 - t1) * ey) >> 16) + t1; 	 
        t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff; 	 
        t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff; 	 
        dp->a = (((t2 - t1) * ey) >> 16) + t1; 	 
 	 
        /* 	 
         * Advance source pointers 	 
         */ 	 
        csax++; 	 
        sstep = (*csax >> 16); 	 
        c00 += sstep; 	 
        c01 += sstep; 	 
        c10 += sstep; 	 
        c11 += sstep; 	 
        /* 	 
         * Advance destination pointer 	 
         */ 	 
        dp++; 	 
        } 	 
        /* 	 
         * Advance source pointer 	 
         */ 	 
        csay++; 	 
        csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch); 	 
        /* 	 
         * Advance destination pointers 	 
         */ 	 
        dp = (tColorRGBA *) ((Uint8 *) dp + dgap); 	 
    } 	 
 	 
    } else { 	 
 	 
    /* 	 
     * Non-Interpolating Zoom 	 
     */ 	 
 	 
    csay = say; 	 
    for (y = 0; y < dst->h; y++) { 	 
        sp = csp; 	 
        csax = sax; 	 
        for (x = 0; x < dst->w; x++) { 	 
        /* 	 
         * Draw 	 
         */ 	 
        *dp = *sp; 	 
        /* 	 
         * Advance source pointers 	 
         */ 	 
        csax++; 	 
        sstep = (*csax >> 16); 	 
        if (flipx) sstep = -sstep; 	 
        sp += sstep; 	 
        /* 	 
         * Advance destination pointer 	 
         */ 	 
        dp++; 	 
        } 	 
        /* 	 
         * Advance source pointer 	 
         */ 	 
        csay++; 	 
        sstep = (*csay >> 16) * src->pitch; 	 
        if (flipy) sstep = -sstep; 	 
        csp = (tColorRGBA *) ((Uint8 *) csp + sstep); 	 
 	 
        /* 	 
         * Advance destination pointers 	 
         */ 	 
        dp = (tColorRGBA *) ((Uint8 *) dp + dgap); 	 
    } 	 
 	 
    } 	 
 	 
    /* 	 
     * Remove temp arrays 	 
     */ 	 
    FREE(sax); 	 
    FREE(say); 	 
 	 
    return (0); 	 
}

/* a very special collision for the multi tile face & the player sprite,
 * used to make the player overlapping objects transparent */
int sprite_collision(int x1, int y1, int x2, int y2, _Sprite *sprite1, _Sprite *sprite2)
{
    int left1, left2;
    int right1, right2;
    int top1, top2;
    int bottom1, bottom2;

    /* SDL_Rect myrect ;*/

    left1 = x1 + sprite1->border_left;
    left2 = x2 + sprite2->border_left;
    right1 = x1 + sprite1->bitmap->w - sprite1->border_right;
    right2 = x2 + sprite2->bitmap->w - sprite2->border_right;
    top1 = y1 + sprite1->border_up;
    top2 = y2 + sprite2->border_down;
    bottom1 = y1 + sprite1->bitmap->h - sprite1->border_down;
    bottom2 = y2 + sprite2->bitmap->h - sprite2->border_down;
    /*
    myrect.x = left1;
    myrect.y = top1;
    myrect.w = right1-left1;
    myrect.h = bottom1-top1;

    SDL_FillRect(ScreenSurface, &myrect, 555555);
    */
    if (bottom1 < top2)
        return(0);
    if (top1 > bottom2)
        return(0);

    if (right1 < left2)
        return(0);
    if (left1 > right2)
        return(0);
    /*
    myrect.x = left2;
    myrect.y = top2;
    myrect.w = right2-left2;
    myrect.h = bottom2-top2;

    SDL_FillRect(ScreenSurface, &myrect, 3243);
      */
    return(1);
}

/* the following was contributed by JotDot */
/* and adopted to cache also the darkness faces by Alderan */
#define DEBUG_HASH 0

#define STRETCH_CACHE_SIZE 1000

#define HASH_HEAD_SIZE 512 /* If non-power of two then change the line below to use % */
/* ie: #define hash_head_MOD(N) ( (N) % (HASH_HEAD_SIZE - 1) ) */
#define HASH_MOD(N) ( (N) & (HASH_HEAD_SIZE - 1) )	/* Power of 2 mod */

#define STRETCH_NULL (Uint16)(~0)

/*
	The cache is implemented using a hash chain technique.  The incoming pointer
	and the stretch number is converted to a reasonably(?) unique key and hashed.
	The cache entries also has a double linked list to maintain a LRU method where
	a successful fetch will place the entry at the head. When the table gets full
	the tail end is recycled.
	Limitations: I don't expect more than 64K-1 cache entries so the entries
	are 16 bit. The key is currently reduced to 8 bit since I don't expect the hash
	table to be very big.  Just change the CalcHash routine if you want a larger key.
*/
struct stretch_cache_
{
  SDL_Surface *src;
  SDL_Surface *dest;
  Uint32 stretch;
  Uint32 darkness;
  Uint16 lru_older;
  Uint16 lru_newer;
  Uint16 hash_next;
};

static Uint16 hash_head[HASH_HEAD_SIZE];		/* Hash chain heads */
static Uint16 lru_head;							/* Most recent entry */
static Uint16 lru_tail;							/* Oldest entry */
static Uint16 stretch_cache_count=0;			/* Number of cached entries */

struct stretch_cache_ stretch_cache[STRETCH_CACHE_SIZE];

static void stretch_init(void)
{
	int i;

	for (i=0; i < HASH_HEAD_SIZE; ++i)
	{
		hash_head[i] = STRETCH_NULL;
	}

	lru_head = STRETCH_NULL;
	lru_tail = STRETCH_NULL;

	stretch_cache_count = 0;
}

static Uint16 CalcHash(const SDL_Surface * src,Uint32 stretch, Uint32 darkness)
{
	/* Nothing fancy. Very simplistic really. */
	Uint16 sum;

	sum=
		(Uint16)((Uint32)src >> 16) +
		(Uint16)((Uint32)src) +
		(Uint16)((Uint32)stretch >> 16) +
		(Uint16)stretch +
		(Uint16)((Uint32)darkness>> 16) +
		(Uint16)darkness;

	sum=(Uint8)(sum>>8)+(Uint8)sum;	/* Limits the hash head table to 256 entries */

	return HASH_MOD(sum);
}

/*
	lru_add() takes the cache entry and adds it to the head of the lru list
	Note: This does not affect the hash chain table.
*/
static void lru_add(Uint16 index)
{
	struct stretch_cache_ * elem = stretch_cache + index;

	elem->lru_newer = STRETCH_NULL;
	elem->lru_older = lru_head;

	if (lru_head != STRETCH_NULL)
	{
		stretch_cache[lru_head].lru_newer = index;
	}

	lru_head = index;

	if (lru_tail == STRETCH_NULL)
	{
		lru_tail = index;
	}
}

/*
	lru_remove(Uint16 index) - Takes the entry out of the LRU list.
	Note: This does not affect the hash chain table.
*/
static void lru_remove(Uint16 index)
{
	struct stretch_cache_ * elem = stretch_cache + index;

	if (elem->lru_newer != STRETCH_NULL)
		stretch_cache[elem->lru_newer].lru_older = elem->lru_older;
	else
		lru_head = elem->lru_older;

	if (elem->lru_older != STRETCH_NULL)
		stretch_cache[elem->lru_older].lru_newer = elem->lru_newer;
	else
		lru_tail = elem->lru_newer;
}

static void hash_remove(Uint16 index)
{
	struct stretch_cache_ * elem;
	Uint16 key;
	Uint16 prev,next;

	elem = stretch_cache + index;
	key = CalcHash(elem->src,elem->stretch, elem->darkness);

	/* Single linked list here. There should be so few entries on each chain */
	/* that it does not warrant a double linked list */

	prev = STRETCH_NULL;
	next = hash_head[key];

	while (next != STRETCH_NULL && next != index) {
		prev = next;
		next = stretch_cache[next].hash_next;
	}

	if (next == STRETCH_NULL) {
		if (prev == STRETCH_NULL)
			LOG(LOG_MSG, "Warning: hash_remove encountered an empty chain!\n");
		else
			LOG(LOG_MSG, "Warning: hash_remove did not find a match on the chain!\n");
		return;
	}

	if (prev == STRETCH_NULL)
		hash_head[key]=stretch_cache[index].hash_next;
	else
		stretch_cache[prev].hash_next = stretch_cache[index].hash_next;
}

SDL_Surface *check_stretch_cache(const SDL_Surface *src, Uint32 stretch, Uint32 darkness)
{
	Uint16 key;
	Uint16 idx;

	if (!stretch_cache_count)
	{
		/* If first time then nothing is in the table thus nothing will be found */
		stretch_init();
#if DEBUG_HASH
		LOG(LOG_MSG, "check_stretch_cache(%p,%d) returned NULL\n",src,stretch);
#endif
		return NULL;
	}

	key = CalcHash(src, stretch, darkness);	/* Key for this entry */
	idx = hash_head[key];			/* Head of this list */

	while (idx != STRETCH_NULL)
	{
		struct stretch_cache_ * elem = stretch_cache + idx;

		if (elem->src != src || elem->stretch != stretch || elem->darkness != darkness)
			idx = elem->hash_next;

		else
		{
			/* Found a match. Move this entry to the head of the LRU list */
			/* if not already there */

			if (idx != lru_head)
			{
				/* Note: The entry is already in the hash chain and that */
				/* portion does not need to be modified */

				/* Remove from current position in the lru list */
				lru_remove(idx);

				/* Add current (now orphaned) entry to head of lru list */
				lru_add(idx);
			}

			break;
		}
	}

#if DEBUG_HASH
//	LOG(LOG_MSG,"check_stretch_cache(src %p, stretch %d, dark: %d) new entry at %d (key: %d)\n",src,stretch,darkness,idx, key);
#endif


	return (idx != STRETCH_NULL) ? stretch_cache[idx].dest : NULL;
}

/*
	add_to_stretch_cache() assumes the entry is not already in the table.
*/

void add_to_stretch_cache(SDL_Surface *src,SDL_Surface *dest, Uint32 stretch, Uint32 darkness)
{
	Uint16 key;
	Uint16 idx;
	struct stretch_cache_ * elem;

	/* If there is room in the table then simply fetch the next free location */
	/* Otherwise we discard the oldest entry and re-use that location */

	if (stretch_cache_count < STRETCH_CACHE_SIZE)
		idx=stretch_cache_count++;

	else
	{
		/* The table is full so we discard the tail in the lru list */
		idx = lru_tail;		/* This will be the new index after discard */

#if DEBUG_HASH
		LOG(LOG_MSG,"add_to_stretch_cache(src %p, dest %p, stretch %d, dark: %d) free lru at %d\n",src,dest,stretch,darkness, idx);
#endif

		/* Get rid of the old surface first */
		if (stretch_cache[idx].dest != NULL)
		{
			SDL_FreeSurface(stretch_cache[idx].dest);
		}

		hash_remove(idx);	/* Old entry removed from hash table */
		lru_remove(idx);	/* Entry is now orphaned in lru list */
	}

	/* Now we can add this new entry into the table */
	elem = stretch_cache + idx;

	elem->src = src;
	elem->dest = dest;
	elem->stretch = stretch;
	elem->darkness = darkness;

	/* Add to hash table */
	key = CalcHash(src,stretch, darkness);

#if DEBUG_HASH
	LOG(LOG_MSG,"add_to_stretch_cache(src %p, dest %p, stretch %d, dark: %d) new entry at %d (key: %d)\n",src,dest,stretch,darkness,idx, key);
#endif

	elem->hash_next = hash_head[key];
	hash_head[key] = idx;

	/* Add current entry to head of lru list */
	lru_add(idx);
}
