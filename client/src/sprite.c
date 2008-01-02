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
#include <include.h>

static int      dark_alpha[DARK_LEVELS] =
    {
        0, 44, 80, 117, 153, 190, 226
    };
SDL_Surface    *darkness_filter[] =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };

_BBDark     Backbuffer[MAX_BBDARK];

int         bbpos = 0;


struct _anim   *start_anim; /* anim queue of current active map */

struct _imagestats  ImageStats;

SDL_Surface     *FormatHolder;

static Boolean  GetBitmapBorders(SDL_Surface *Surface, int *up, int *down, int *left, int *right, UINT32 ckey);
static void     grey_scale(_Sprite *sprite);
static void     red_scale(_Sprite *sprite);
static void     fow_scale(_Sprite *sprite);

/* not much special inside atm */
Boolean sprite_init_system(void)
{
    sprite_clear_backbuffer();

    FormatHolder=SDL_CreateRGBSurface(SDL_SRCALPHA,1, 1, 32, 0xFF000000, 0x00FF0000 ,0x0000FF00 ,0x000000FF);
    SDL_SetAlpha(FormatHolder,SDL_SRCALPHA,255);
    return(TRUE);
}

void sprite_clear_backbuffer(void)
{
    int i;

    for (i=0;i<MAX_BBDARK;i++)
    {
        Backbuffer[i].sprite=NULL;
    }
    bbpos=0;
    memset(&ImageStats, 0, sizeof(_imagestats));

}

Boolean sprite_deinit_system(void)
{
    return(TRUE);
}

_Sprite * sprite_load_file(char *fname, uint32 flags)
{
    _Sprite    *sprite;

    sprite = sprite_tryload_file(fname, flags, NULL);

    if (sprite == NULL)
    {
        LOG(LOG_ERROR, "ERROR sprite.c: Can't load sprite %s\n", fname);
        return(NULL);
    }
    return(sprite);
}

_Sprite * sprite_tryload_file(char *fname, uint32 flag, SDL_RWops *rwop)
{
    _Sprite        *sprite;
    SDL_Surface    *bitmap;
    UINT32          ckflags, tmp=0;
    SDL_RWops       *rw;


    if (fname)
    {
        if (PHYSFS_exists(fname)==0)
            LOG(LOG_MSG,"file: %s does not exist in physfs\n",fname);
        if ((rw=PHYSFSRWOPS_openRead(fname))==NULL)
            LOG(LOG_MSG,"PHYSFSRWOPS_openRead failed: %s\n",PHYSFS_getLastError());
        if ((bitmap = IMG_Load_RW(rw,0)) == NULL)
            return(NULL);
        SDL_RWclose(rw);

    }
    else
    {
        bitmap = IMG_LoadPNG_RW(rwop);
    }
    if ((sprite = malloc(sizeof(_Sprite))) == NULL)
        return(NULL);
    memset(sprite, 0, sizeof(_Sprite));

    sprite->status = SPRITE_STATUS_LOADED;
    sprite->type = SPRITE_TYPE_NORMAL;

    /* hm, must test this is used from displayformat too?*/
    /* we set colorkey stuff. depends and video hardware and how we need to store this*/
    ckflags = SDL_SRCCOLORKEY | SDL_ANYFORMAT;

    if (options.rleaccel_flag)
        ckflags |= SDL_RLEACCEL;

    if (bitmap->format->palette)
        SDL_SetColorKey(bitmap, ckflags, (tmp = bitmap->format->colorkey));
    else if (flag & SURFACE_FLAG_COLKEY_16M) /* we force a true color png to colorkey */
        SDL_SetColorKey(bitmap, ckflags, 0); /* default colkey is black (0) */


    GetBitmapBorders(bitmap, &sprite->border_up, &sprite->border_down, &sprite->border_left, &sprite->border_right, tmp);

    if (!bitmap->format->palette)
        ImageStats.truecolors++;

    /* we store our original bitmap */
    sprite->bitmap = bitmap;

    if (flag & SURFACE_FLAG_DISPLAYFORMAT)
    {
        sprite->bitmap = SDL_DisplayFormat(bitmap);
        SDL_FreeSurface(bitmap);
    }

    if (!(flag & SURFACE_FLAG_PALETTE))
    {
//        sprite->bitmap = SDL_DisplayFormatAlpha(bitmap);
//        SDL_FreeSurface(bitmap);
    }
    ImageStats.loadedsprites++;
    return(sprite);
}


static void red_scale(_Sprite *sprite)
{
    int         j, k;
    Uint8       r, g, b, a;
    SDL_Surface *temp;

    temp=SDL_ConvertSurface(sprite->bitmap, FormatHolder->format, FormatHolder->flags);
    for (k=0;k<temp->h;k++)
    {
        for (j=0;j<temp->w;j++)
        {
            SDL_GetRGBA(getpixel(temp,j,k),temp->format,&r, &g, &b, &a);
            r = (int) ((0.212671 * r + 0.715160 * g + 0.072169 * b) + 32);
            g = b = 0;
            putpixel(temp,j,k,SDL_MapRGBA(temp->format, r, g, b,a));
        }
    }
    sprite->red = SDL_DisplayFormatAlpha(temp);
    SDL_FreeSurface(temp);
    ImageStats.redscales++;
}

static void grey_scale(_Sprite *sprite)
{
    int         j, k;
    Uint8       r, g, b, a;
    SDL_Surface *temp;

    temp=SDL_ConvertSurface(sprite->bitmap, FormatHolder->format, FormatHolder->flags);
    for (k=0;k<temp->h;k++)
    {
        for (j=0;j<temp->w;j++)
        {
            SDL_GetRGBA(getpixel(temp,j,k),temp->format,&r, &g, &b, &a);
            r = g = b = (int) (0.212671 * r + 0.715160 * g + 0.072169 * b);
            putpixel(temp,j,k,SDL_MapRGBA(temp->format, r, g, b,a));
        }
    }
    sprite->grey = SDL_DisplayFormatAlpha(temp);
    SDL_FreeSurface(temp);
    ImageStats.greyscales++;
}

static void fow_scale(_Sprite *sprite)
{
    int         j, k;
    Uint8       r, g, b, a;
    SDL_Surface *temp;

    temp=SDL_ConvertSurface(sprite->bitmap, FormatHolder->format, FormatHolder->flags);
    for (k=0;k<temp->h;k++)
    {
        for (j=0;j<temp->w;j++)
        {
            SDL_GetRGBA(getpixel(temp,j,k),temp->format,&r, &g, &b, &a);
            r = g = b = (int)((0.212671 * r + 0.715160 * g + 0.072169 * b) * 0.34);
            b=+16;
            putpixel(temp,j,k,SDL_MapRGBA(temp->format, r, g, b,a));
        }
    }
    sprite->fog_of_war = SDL_DisplayFormatAlpha(temp);
    SDL_FreeSurface(temp);
    ImageStats.fowscales++;
}

void sprite_free_sprite(_Sprite *sprite)
{
    int i;
    void   *tmp_free;

    if (!sprite)
        return;
    if (sprite->bitmap)
        SDL_FreeSurface(sprite->bitmap);
    for (i=0;i<DARK_LEVELS;i++)
    {
        if (sprite->dark_level[i])
            SDL_FreeSurface(sprite->dark_level[i]);
    }
    if (sprite->grey)
        SDL_FreeSurface(sprite->grey);
    if (sprite->red)
        SDL_FreeSurface(sprite->red);
    if (sprite->fog_of_war)
        SDL_FreeSurface(sprite->fog_of_war);
    tmp_free = sprite;
    FreeMemory(&tmp_free);
}

/* init this font structure with gfx data from sprite bitmap */
void CreateNewFont(_Sprite *sprite, _Font *font, int xlen, int ylen, int c32len)
{
    register int i, y;
    int flag;

    SDL_LockSurface(sprite->bitmap);
    font->sprite = sprite;

    for (i = 0; i < 256; i++)
    {
        font->c[i].x = (i % 32) * (xlen + 1) + 1;
        font->c[i].y = (i / 32) * (ylen + 1) + 1;
        font->c[i].h = ylen;
        font->c[i].w = xlen;
        flag = 0;
        while (1) /* better no error in font bitmap... or this will lock up*/
        {
            for (y = font->c[i].h - 1; y >= 0; y--)
            {
                if (GetSurfacePixel(sprite->bitmap, font->c[i].x + font->c[i].w - 1, font->c[i].y + y))
                {
                    flag = 1;
                    break;
                }
            }
            if (flag)
                break;
            font->c[i].w--;
        }
    }

    SDL_UnlockSurface(sprite->bitmap);
    font->char_offset = c32len;
}

/* Calculate the displayed width of the text */
int StringWidth(_Font *font, char *text)
{
    int w = 0, i;

    for (i = 0; text[i] != '\0'; i++)
    {
        switch (text[i])
        {
            case '°': // yellow (strong)
            case '|': // -------"-------
            case '~': // green (emphasis)
            case '^': // blue (clickable keyword)
                break;

            default:
                w += font->c[(unsigned char) (text[i])].w + font->char_offset;
                break;
        }
    }

    return w;
}
/* Calculate the displayed chars for a given width*/
int StringWidthOffset(_Font *font, char *text, int *line, int len)
{
    int w = 0, i, c, flag = FALSE;

    for (c = i = 0; text[i] != '\0'; i++)
    {
        switch (text[i])
        {
            case '°': // yellow (strong)
            case '|': // -------"-------
            case '~': // green (emphasis)
            case '^': // blue (clickable keyword)
                break;

            default:
                w += font->c[(unsigned char) (text[i])].w + font->char_offset;
                if (w>=len && !flag)
                {
                    flag = TRUE;
                    *line = c;
                }
                break;
        }
        c++;
    }

    if (!flag) /* line is in limit */
        *line = c;

    return flag;
}

void StringBlt(SDL_Surface *surf, _Font *font, char *text, int x, int y, int col, SDL_Rect *area, _BLTFX *bltfx)
{
    register int i,tmp, line_clip = -1,line_count = 0;
    register Boolean gflag;
    int         colorToggle = 0;
    SDL_Rect    src, dst, dst_tmp;
    SDL_Color   color, color_g, color_s;
    unsigned char actChar =0;

    if (area)
        line_clip = area->w;

    dst_tmp.x = x; /* .w/h are not used from BlitSurface to draw*/
    dst_tmp.y = y;

    color_g.r = 0;
    color_g.g = 255;
    color_g.b = 255;

    color_s.r = 204; /* highlight color for selected keywords */
    color_s.g = 102;
    color_s.b = 255;


    color.r = Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[col].r;
    color.g = Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[col].g;
    color.b = Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[col].b;

    SDL_SetPalette(font->sprite->bitmap, SDL_LOGPAL | SDL_PHYSPAL, &color, 1, 1);

    if (bltfx && bltfx->flags & BLTFX_FLAG_SRCALPHA)
        SDL_SetAlpha(font->sprite->bitmap,SDL_SRCALPHA,bltfx->alpha);
    else
       SDL_SetAlpha(font->sprite->bitmap,SDL_RLEACCEL,255);

    gflag = FALSE;
    for (i = 0; text[i] != '\0'; i++)
    {
        actChar = text[i];
        dst.x=dst_tmp.x;
        dst.y=dst_tmp.y;
        if (actChar == (unsigned char)'°' || actChar == (unsigned char)'|' || actChar == (unsigned char)'~') /* change text color */
        {
            if (col == COLOR_BLACK)
                continue; /* no highlighting in blak text */
            if (colorToggle)
            {
                color.r = Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[col].r;
                color.g = Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[col].g;
                color.b = Bitmaps[BITMAP_PALETTE]->bitmap->format->palette->colors[col].b;
                SDL_SetPalette(font->sprite->bitmap, SDL_LOGPAL | SDL_PHYSPAL, &color, 1, 1);
            }
            else
            {
                color.g = 0xff;
                color.b = 0x00;
                if (actChar == (unsigned char)'°' || actChar == (unsigned char)'|')
                    color.r = 0xff;
                else
                    color.r = 0x00;
                SDL_SetPalette(font->sprite->bitmap, SDL_LOGPAL | SDL_PHYSPAL, &color, 1, 1);
            }
            colorToggle = (colorToggle + 1) & 1;
            continue;
        }
        if (actChar == (unsigned char)'^')
        {
            if (gflag)
            {
                SDL_SetPalette(font->sprite->bitmap, SDL_LOGPAL | SDL_PHYSPAL, &color, 1, 1);
                gflag = FALSE;
            }
            else
            {
                if ((cpl.menustatus == MENU_NPC) && (gui_interface_npc->keyword_selected>0) && !strncmp(text+i+1,gui_interface_npc->keywords[gui_interface_npc->keyword_selected-1],strlen(gui_interface_npc->keywords[gui_interface_npc->keyword_selected-1])))
                    SDL_SetPalette(font->sprite->bitmap, SDL_LOGPAL | SDL_PHYSPAL, &color_s, 1, 1);
                else
                    SDL_SetPalette(font->sprite->bitmap, SDL_LOGPAL | SDL_PHYSPAL, &color_g, 1, 1);

                gflag = TRUE;
            }
            continue;
        }

        tmp = font->c[actChar].w + font->char_offset;

        /* if set, we have a clipping line */
        if (line_clip >= 0)
        {
            if ((line_count += tmp) > line_clip)
                return;
        }

        if (actChar != 32)
        {
            src.x = font->c[actChar].x;
            src.y = font->c[actChar].y;
            src.w = font->c[actChar].w;
            src.h = font->c[actChar].h;
            SDL_BlitSurface(font->sprite->bitmap, &src, surf, &dst);
        }

        dst_tmp.x += tmp;
    }
}

void show_tooltip(int mx, int my, char *text)
{
    SDL_Rect    rec;
    char       *tooltip = text;

    if (!options.show_tooltips)
        return;
    rec.w = 3;
    while (*text)
        rec.w += SystemFont.c[(int) * text++].w + SystemFont.char_offset;
    rec.x = mx + 9;
    rec.y = my + 17;
    rec.h = 12;

    if (rec.x + rec.w >= Screensize.x)
        rec.x -= (rec.x + rec.w + 1) - Screensize.x;

    SDL_FillRect(ScreenSurface, &rec, -1);
    StringBlt(ScreenSurface, &SystemFont, tooltip, rec.x + 2, rec.y - 1, COLOR_BLACK, NULL, NULL);
}

static Boolean GetBitmapBorders(SDL_Surface *Surface, int *up, int *down, int *left, int *right, UINT32 ckey)
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
    return FALSE;

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
                return TRUE;
            }
        }
    }
    return TRUE;
}

/* Graps a pixel from a SDL_Surface on the position x,y in the right format & colors */
Uint32 GetSurfacePixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{
    Uint8  *bits;
    Uint32  Bpp;

    Bpp = Surface->format->BytesPerPixel;
    bits = ((Uint8 *) Surface->pixels) + Y * Surface->pitch + X * Bpp;

    /* Get the pixel*/
    switch (Bpp)
    {
        case 1:
            return *((Uint8 *) Surface->pixels + Y * Surface->pitch + X);
            break;
        case 2:
            return *((Uint16 *) Surface->pixels + Y * Surface->pitch / 2 + X);
            break;
        case 3:
        {
            /* Format/endian independent*/
            Uint8     r, g, b;
            r = *((bits) + Surface->format->Rshift / 8);
            g = *((bits) + Surface->format->Gshift / 8);
            b = *((bits) + Surface->format->Bshift / 8);
            return SDL_MapRGB(Surface->format, r, g, b);
        }
        break;
        case 4:
            return *((Uint32 *) Surface->pixels + Y * Surface->pitch / 4 + X);
            break;
    }
    return -1;
}


int get_string_pixel_length(char *text, struct _Font *font)
{
    register int i, len = 0;

    for (i = 0; text[i] != 0; i++)
        len += font->c[(int) text[i]].w + font->char_offset;

    return len;
}
/* Alderan 2007-11-03: i reworked that a bit:
 * we dont need for every blit check for all the map drawing stuff
 * so the mapdrawing gets its own function.
 * this function can only handle semi-trans in BLTFX
 */
void sprite_blt(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx)
{
    SDL_Rect        dst;
    SDL_Surface    *surface, *blt_sprite;

    if (!sprite)
        return;

    blt_sprite = sprite->bitmap;

    if (bltfx && (bltfx->surface))
        surface=bltfx->surface;
    else
        surface = ScreenSurface;
    dst.x = x;
    dst.y = y;

    if (bltfx)
    {
        if (bltfx->flags & BLTFX_FLAG_SRCALPHA && !(ScreenSurface->flags & SDL_HWSURFACE))
        {
            SDL_SetAlpha(blt_sprite, SDL_SRCALPHA, bltfx->alpha);
        }
    }

    if (!blt_sprite)
        return;

    if (box)
        SDL_BlitSurface(blt_sprite, box, surface, &dst);
    else
        SDL_BlitSurface(blt_sprite, NULL, surface, &dst);

    if (bltfx && bltfx->flags & BLTFX_FLAG_SRCALPHA && !(ScreenSurface->flags & SDL_HWSURFACE))
    {
        SDL_SetAlpha(blt_sprite, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    }
}
/* This function dupports the whole BLTFX flags, and is only used to blit the map! */
void sprite_blt_map(_Sprite *sprite, int x, int y, SDL_Rect *box, _BLTFX *bltfx)
{
    SDL_Rect        dst;
    SDL_Surface    *surface, *blt_sprite;

    if (!sprite)
        return;

    blt_sprite = sprite->bitmap;
    surface = ScreenSurfaceMap;
    dst.x = x;
    dst.y = y;

    if (bltfx)
    {
        if (bltfx->flags & BLTFX_FLAG_DARK)
        {
            /* last dark level is "no color" ... */
            if (bltfx->dark_level == DARK_LEVELS)
                return;

            /* we create the filter surfaces only when needed, and then store them */
            if (!darkness_filter[bltfx->dark_level])
            {
                SDL_SetAlpha(Bitmaps[BITMAP_TEXTWIN_MASK]->bitmap,SDL_SRCALPHA,dark_alpha[bltfx->dark_level]);
                darkness_filter[bltfx->dark_level]=SDL_DisplayFormatAlpha(Bitmaps[BITMAP_TEXTWIN_MASK]->bitmap);
            }

            /* this is the simple rolling backbuffer */
            /* we test for the backbuffer */
            if (sprite->dark_level[bltfx->dark_level])
            {
                blt_sprite = sprite->dark_level[bltfx->dark_level];
            }
            else /* we create the surface, and put it in backbuffer */
            {
                /* first we free if necesary */
                if (Backbuffer[bbpos].sprite)
                {
                    blt_sprite = Backbuffer[bbpos].sprite->dark_level[Backbuffer[bbpos].dark_level];
                    SDL_FreeSurface(blt_sprite);
                    Backbuffer[bbpos].sprite->dark_level[Backbuffer[bbpos].dark_level]=NULL;
                    Backbuffer[bbpos].sprite=NULL;
                }
                blt_sprite = SDL_DisplayFormatAlpha(sprite->bitmap);
//                blt_sprite = SDL_ConvertSurface(sprite->bitmap, sprite->bitmap->format, SDL_SRCALPHA);
                SDL_BlitSurface(darkness_filter[bltfx->dark_level],NULL,blt_sprite,NULL);

                /* we put it in the backbuffer */
                sprite->dark_level[bltfx->dark_level]=blt_sprite;
                Backbuffer[bbpos].sprite=sprite;
                Backbuffer[bbpos].dark_level=bltfx->dark_level;
                bbpos++;
                if (bbpos>=MAX_BBDARK)
                    bbpos=0;
            }

        }
        else if (bltfx->flags & BLTFX_FLAG_FOW)
        {
            if (!sprite->fog_of_war)
                fow_scale(sprite);
            blt_sprite = sprite->fog_of_war;
        }
        else if (bltfx->flags & BLTFX_FLAG_RED)
        {
            if (!sprite->red)
                red_scale(sprite);
            blt_sprite = sprite->red;
        }
        else if (bltfx->flags & BLTFX_FLAG_GREY)
        {
            if (!sprite->grey)
                grey_scale(sprite);
            blt_sprite = sprite->grey;
        }
        if (!blt_sprite)
            return;


        if (bltfx->flags & BLTFX_FLAG_SRCALPHA && !(ScreenSurface->flags & SDL_HWSURFACE))
        {
            SDL_SetAlpha(blt_sprite, SDL_SRCALPHA, bltfx->alpha);
        }
    }

    if (!blt_sprite)
        return;

    if (box)
        SDL_BlitSurface(blt_sprite, box, surface, &dst);
    else
        SDL_BlitSurface(blt_sprite, NULL, surface, &dst);

    if (bltfx && bltfx->flags & BLTFX_FLAG_SRCALPHA && !(ScreenSurface->flags & SDL_HWSURFACE))
    {
        SDL_SetAlpha(blt_sprite, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
    }
}


struct _anim * add_anim(int type, int x, int y, int mapx, int mapy, int value)
{
    struct _anim   *tmp, *anim;

    int             num_ticks;

    for (tmp = start_anim; tmp; tmp = tmp->next)
    {
        if (!tmp->next)
            break;
    }
    /* tmp == null - no anim in que, else tmp = last anim */
    anim = (struct _anim *) malloc(sizeof(struct _anim));

    if (!tmp)
        start_anim = anim;
    else
        tmp->next = anim;

    anim->before = tmp;
    anim->next = NULL;

    anim->type = type;
    anim->x = 0;                  /* starting X position */
    anim->y = -5;                 /* starting Y position */
    anim->xoff = 0;
    anim->yoff = 1;  /* this looks like it makes it move up the screen -- was 0*/
    anim->mapx = mapx;            /* Map cordinates */
    anim->mapy = mapy;
    anim->value = value;          /* Amount of damage */
    anim->start_tick = LastTick;  /* current time in MilliSeconds */



    switch (type)
    {
        case ANIM_DAMAGE:
            num_ticks = 850;  /* how many ticks to display */
            anim->last_tick = anim->start_tick + num_ticks;
            anim->yoff = (25.0f / 850.0f); /* 850 ticks 25 pixel move up */
            break;
        case ANIM_SELF_DAMAGE:
            num_ticks = 850;  /* how many ticks to display */
            anim->last_tick = anim->start_tick + num_ticks;
            anim->yoff = (25.0f / 850.0f); /* 850 ticks 25 pixel move up */
            break;
        case ANIM_KILL:
            num_ticks = 850;  /* how many ticks to display */
            anim->last_tick = anim->start_tick + num_ticks;
            anim->yoff = (25.0f / 850.0f); /* 850 ticks 25 pixel move up */
            break;
    }



    return(anim);
}

void remove_anim(struct _anim *anim)
{
    struct _anim   *tmp, *tmp_next;
    void           *tmp_free;

    if (!anim)
        return;

    tmp = anim->before;
    tmp_next = anim->next;
    tmp_free = &anim;
    FreeMemory(tmp_free); /* free node memory */

    if (tmp)
        tmp->next = tmp_next;
    else
        start_anim = tmp_next;
    if (tmp_next)
        tmp_next->before = tmp;
}


void delete_anim_que(void)
{
    struct _anim   *tmp, *tmp_next;
    void           *tmp_free;

    for (tmp = start_anim; tmp;)
    {
        tmp_next = tmp->next;
        tmp_free = &tmp;
        FreeMemory(tmp_free); /* free node memory */
        tmp = tmp_next;
    }
    start_anim = NULL;
}

/* walk through the map anim list */
void play_anims(int mx, int my)
{
    struct _anim   *anim, *tmp;
    int             xpos, ypos, tmp_off;
    int             num_ticks;
    char            buf[32];
    int             tmp_y;

    for (anim = start_anim; anim; anim = tmp)
    {
        tmp = anim->next;

        if (LastTick > anim->last_tick)  /* have we passed the last tick */
            remove_anim(anim);
        else
        {
            num_ticks = LastTick - anim->start_tick;
            switch (anim->type)
            {
                case ANIM_DAMAGE:
                    tmp_y = anim->y - (int) ((float) num_ticks * anim->yoff); /*   * num_ticks ); */

                    if (anim->mapx >= MapData.posx
                            && anim->mapx < MapData.posx + MapStatusX
                            && anim->mapy >= MapData.posy
                            && anim->mapy < MapData.posy + MapStatusY)
                    {
                        xpos = options.mapstart_x + (int)((MAP_START_XOFF +
                               + (anim->mapx - MapData.posx) * MAP_TILE_YOFF
                               - (anim->mapy - MapData.posy - 1) * MAP_TILE_YOFF
                               - 4)*(options.zoom/100.0));
                        ypos = options.mapstart_y
                               + (int)(((anim->mapx - MapData.posx) * MAP_TILE_XOFF
                               + (anim->mapy - MapData.posy - 1) * MAP_TILE_XOFF
                               - 34)*(options.zoom/100.0));
                        if (anim->value<0)
                        {
                            sprintf(buf, "%d", abs(anim->value));
                            StringBlt(ScreenSurface, &SystemFontOut, buf, xpos + anim->x, ypos + tmp_y, COLOR_GREEN, NULL,
                                          NULL);
                        }
                        else
                        {
                            sprintf(buf, "%d", anim->value);
                            StringBlt(ScreenSurface, &SystemFontOut, buf, xpos + anim->x, ypos + tmp_y, COLOR_ORANGE, NULL,
                                          NULL);
                        }
                    }
                    break;
                case ANIM_KILL:
                    tmp_y = anim->y - (int) ((float) num_ticks * anim->yoff); /*   * num_ticks ); */

                    if (anim->mapx >= MapData.posx
                            && anim->mapx < MapData.posx + MapStatusX
                            && anim->mapy >= MapData.posy
                            && anim->mapy < MapData.posy + MapStatusY)
                    {
                        xpos = options.mapstart_x + (int)((MAP_START_XOFF +
                               + (anim->mapx - MapData.posx) * MAP_TILE_YOFF
                               - (anim->mapy - MapData.posy - 1) * MAP_TILE_YOFF
                               - 4)*(options.zoom/100.0));
                        ypos = options.mapstart_y
                               + (int)(((anim->mapx - MapData.posx) * MAP_TILE_XOFF
                               + (anim->mapy - MapData.posy - 1) * MAP_TILE_XOFF
                               - 26)*(options.zoom/100.0));
                        sprite_blt(Bitmaps[BITMAP_DEATH], xpos + anim->x - 5, ypos + tmp_y - 4, NULL, NULL);
                        sprintf(buf, "%d", anim->value);
                        tmp_off = 0;

                        /* Lets check the size of the value */
                        if (anim->value < 10)
                            tmp_off = 6;
                        else if (anim->value < 100)
                            tmp_off = 0;
                        else if (anim->value < 1000)
                            tmp_off = -6;
                        else if (anim->value < 10000)
                            tmp_off = -12;

                        StringBlt(ScreenSurface, &SystemFontOut, buf, xpos + anim->x + tmp_off, ypos + tmp_y,
                                  COLOR_ORANGE, NULL, NULL);
                    }
                    break;
                case ANIM_SELF_DAMAGE:
                    tmp_y = anim->y-(int)( (float)num_ticks * anim->yoff ); /*   * num_ticks ); */
                    if (anim->value<0)
                    {
                        sprintf(buf, "%d", abs(anim->value));
                        StringBlt(ScreenSurface, &SystemFontOut, buf, anim->mapx + anim->x, anim->mapy + tmp_y, COLOR_GREEN, NULL,
                                NULL);
                    }
                    else
                    {
                        sprintf(buf, "%d", anim->value);
                        StringBlt(ScreenSurface, &SystemFontOut, buf, anim->mapx + anim->x, anim->mapy + tmp_y, COLOR_RED, NULL,
                                          NULL);
                    }
                break;

                default:
                    LOG(LOG_ERROR, "WARNING: Unknown animation type\n");
                    break;
            }
        }
    }
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

SDL_Surface *zoomSurface(SDL_Surface * src, double zoomx, double zoomy, int smooth)
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

void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight)
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

int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy)
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
    if ((sax = (Uint32 *) malloc(dst->w * sizeof(Uint32))) == NULL) {
    return (-1);
    }
    if ((say = (Uint32 *) malloc(dst->h * sizeof(Uint32))) == NULL) {
    if (sax != NULL) {
        free(sax);
    }
    return (-1);
    }

    /*
     * Precalculate row increments
     */
    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
    csx += sx;
    *csax = (csx >> 16);
    csx &= 0xffff;
    csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
    csy += sy;
    *csay = (csy >> 16);
    csy &= 0xffff;
    csay++;
    }

    csx = 0;
    csax = sax;
    for (x = 0; x < dst->w; x++) {
    csx += (*csax);
    csax++;
    }
    csy = 0;
    csay = say;
    for (y = 0; y < dst->h; y++) {
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
    for (y = 0; y < dst->h; y++) {
    csax = sax;
    sp = csp;
    for (x = 0; x < dst->w; x++) {
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
    free(sax);
    free(say);

    return (0);
}

/*

 32bit Zoomer with optional anti-aliasing by bilinear interpolation.

 Zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.

*/

int zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy, int smooth)
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
    if ((sax = (int *) malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
    return (-1);
    }
    if ((say = (int *) malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
    free(sax);
    return (-1);
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
    free(sax);
    free(say);

    return (0);
}

/* This two helper functions are fast, but be careful:
 * x,y must be on the surface!
 * When using with the display surface the surface MUST be locked!
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}
