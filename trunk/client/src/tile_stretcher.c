/*
    Angelion SDL client, a client program for the Angelion MMORPG.

    Copyright (C) 2003 James Little, et al.

    Based on Daimonin, a split from Crossfire, a Multiplayer game for X-windows.

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

    The author can be reached via e-mail to jlittle@sric.com
*/

#include "include.h"
/*#include <SDL/SDL.h>*/

const int std_tile_half_len[] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,10,10,11,11,
                               11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0 };

typedef struct line_and_slope {
    int sx;
    int sy;
    int end_x;
    int end_y;
    float slope;
} line_and_slope;

int determine_line(line_and_slope *dest, int sx, int sy, int ex, int ey)
{
    float y_diff;
    float x_diff;
    float slope;

    if (sy>ey)
        y_diff = sy-ey;
    else
        y_diff = ey-sy;

    if (sx>ex)
        x_diff = sx-ex;
    else
        x_diff = ex-sx;

    if (x_diff == 0)
        slope = 0.0;
    else
        slope = y_diff / x_diff;

//LOG(LOG_MSG, "determine_line called with: s:(%2d,%2d), e:(%2d,%2d), x_diff: %f, y_diff: %f, slope: %f\n",
//        sx, sy, ex, ey, x_diff, y_diff, slope);

    dest->sx = sx;
    dest->sy = sy;
    dest->end_x = ex;
    dest->end_y = ey;
    dest->slope = slope;

    return 0;
}

void TS_DrawPixel(SDL_Surface *screen, int x, int y, Uint32 color)
{
 /*
   Uint8 *bufp;
   bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
   *bufp = color;
 */


    //Uint32 color = SDL_MapRGB(screen->format, R, G, B);
    switch (screen->format->BytesPerPixel)
    {
      case 1: // Assuming 8-bpp
        {
            Uint8 *bufp;
            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;
      case 2: // Probably 15-bpp or 16-bpp
        {
            Uint16 *bufp;
            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;
      case 3: // Slow 24-bpp mode, usually not used
        {
            Uint8 *bufp;
            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
            if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
            {
                bufp[0] = color;
                bufp[1] = color >> 8;
                bufp[2] = color >> 16;
            }
            else
            {
                bufp[2] = color;
                bufp[1] = color >> 8;
                bufp[0] = color >> 16;
            }
        }
        break;
      case 4: // Probably 32-bpp
        {
            Uint32 *bufp;
            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
  }

}

Uint32 TS_GetPixel(SDL_Surface *screen, int x, int y)
{
  /*
   Uint8 *bufp;
   bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
   return (Uint32)*bufp;
 */

    switch (screen->format->BytesPerPixel)
    {
        case 1: // Assuming 8-bpp
        {
          Uint8 *bufp;
          bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
          return (Uint32)*bufp;
        }
          break;
        case 2: // Probably 15-bpp or 16-bpp
         {
          Uint16 *bufp;
          bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
          return (Uint32)*bufp;
         }
          break;
        case 3: // Slow 24-bpp mode, usually not used
         {
          Uint8 *bufp;
          Uint32 color;
          bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
          if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
          {
              return ( color = bufp[0]+(bufp[1]<<8)+(bufp[2]<<16) );
          }
          else
          {
             return ( color = bufp[2]+(bufp[1]<<8)+(bufp[0]<<16) );
           }
          }
          break;
        case 4: // Probably 32-bpp
          {
          Uint32 *bufp;
          bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
          return (*bufp); // = color;
          }
          break;
    }
    return 0;

}

int add_color_to_surface(SDL_Surface *dest,Uint8 red,Uint8 green,Uint8 blue)
{
    int i; int r_code;

    Uint8 ncol = dest->format->palette->ncolors;
    SDL_Color colors[256];
    for (i = 0; i < ncol; i++)
    {
        colors[i].r= dest->format->palette->colors[i].r;
        colors[i].g= dest->format->palette->colors[i].g;
        colors[i].b= dest->format->palette->colors[i].b;
    }

    colors[ncol].r = red;
    colors[ncol].g = green;
    colors[ncol].b = blue;
    ncol++;

    r_code = SDL_SetColors(dest, colors, 0 , ncol);
    //printf("return code=%d",r_code);
    dest->format->palette->ncolors=ncol;  /* only I could get it to work... */
    return 0;
}

int copy_pixel_to_pixel(SDL_Surface *src,SDL_Surface *dest,int x1,int y1,int x2,int y2,float brightness)
{
    Uint32 color;
    Uint8 red, green, blue, alpha, alpha_2;
    Uint8 red_2,green_2, blue_2;
    Uint16 n;

    /* checks */

    if ( (x1<0)||(y1<0)||(x2<0)||(y2<0) ) return 0;

    if ( (x1 >= src->w)||(x2 >= dest->w) ) return 0;
    if ( (y1 >= src->h)||(y2 >= dest->h) ) return 0;

    color = TS_GetPixel(src,x1,y1);

    if ((src->format->BitsPerPixel==8) && (color == src->format->colorkey)) return 0;  /* no need to copy transparant pixels */

    SDL_GetRGBA(color,src->format,&red,&green,&blue,&alpha);

    if (alpha==0) return 0;

    /* We must clamp to 255 since it is allowable for brightness to exceed 1.0 */
    n = (Uint16)red * brightness;
    red=(n<=255)?n:255;
    n = (Uint16)green * brightness;
    green=(n<=255)?n:255;
    n = (Uint16)blue * brightness;
    blue=(n<=255)?n:255;
//    n = (Uint16)alpha * brightness;
//    alpha=(n<=255)?n:255;

    color = SDL_MapRGBA(dest->format,red,green,blue,alpha);


//    LOG(LOG_MSG, "cp pixel: rgba (%d, %d, %d, %d) - x,y %d,%d to %d,%d\n",red, green, blue, alpha, x1, y1, x2, y2);

    if (color == dest->format->colorkey)  /* contributed by JotDot 11-29-2005 */
    {
        LOG(LOG_MSG, "copy pixel to pixel: we have a colorkey...\n");
        blue += 256>>(8-dest->format->Bloss);
        color = SDL_MapRGBA(dest->format,red,green,blue,alpha);
    }

    if (dest->format->BitsPerPixel==8)
    {
        SDL_GetRGBA(color,dest->format,&red_2,&green_2,&blue_2,&alpha_2);
        if ( (red!=red_2)||(green!=green_2)||(blue!=blue_2)||(alpha!=alpha_2) )
        {
           LOG(LOG_MSG,"bad color match %d %d %d\n",red,green,blue);

           /* if (dest->format->BitsPerPixel==8)*/  /*dont need to check again */
           add_color_to_surface(dest,red,green,blue);

           color = SDL_MapRGBA(dest->format,red,green,blue,alpha); /* should work this time */

         }
    }
    TS_DrawPixel(dest,x2,y2,color);

    return 0;
}

int copy_vertical_line( SDL_Surface *src,SDL_Surface *dest,
                        int src_x,  int src_sy,  int src_ey,
                        int dest_x, int dest_sy, int dest_ey,
                        float brightness, int extra)
{

//    LOG(LOG_MSG, "cp_vert_line: sx: %d, ssy: %d, sey: %d, dx: %d, dsy: %d, dey: %d, bright: %f, extra: %d\n",
//        src_x, src_sy, src_ey, dest_x, dest_sy, dest_ey, brightness, extra);

    int src_h; int dest_h;
    int y;
    float ratio;

    SDL_LockSurface(src);
    SDL_LockSurface(dest);

    if (src_sy > src_ey )
    {
        int tmp = src_sy;
        src_sy = src_ey;
        src_ey = tmp;
    }

    if (dest_sy > dest_ey )
    {
        int tmp = dest_sy;
        dest_sy = dest_ey;
        dest_ey = tmp;
    }

    src_h = src_ey   - src_sy;
    dest_h = dest_ey - dest_sy;

    /* special cases */
    if (dest_h == 0 )
    {
        if (src_h == 0 )
        {
            copy_pixel_to_pixel(src,dest,src_x,src_sy,dest_x,dest_sy,brightness);

            SDL_UnlockSurface(src);
            SDL_UnlockSurface(dest);

            return 0;
        }
        else
        {
            copy_pixel_to_pixel(src,dest,src_x,(src_ey-src_sy)/2,dest_x,dest_sy,brightness);

            SDL_UnlockSurface(src);
            SDL_UnlockSurface(dest);

            return 0;
        }
    }
    if (src_h == 0)
    {
        Uint32 color;

        color = TS_GetPixel(src,src_x,src_sy);

        for (y=dest_sy; y<=dest_ey; y++)
        {
             TS_DrawPixel(dest,dest_x,y,color);
        }
        return 0;
    }

    /* here is the real thing... stretching */
    ratio = (float)src_h / (float)dest_h ;

    for (y = 0; y <= dest_h; y++)
    {
        int go_y = dest_sy+y;
        int get_y = src_sy+(y*ratio);

        copy_pixel_to_pixel(src,dest,src_x,get_y,dest_x,go_y,brightness);
    }
    if (extra)
    {
        if ( dest_ey + 1 < dest->h )
        {
            copy_pixel_to_pixel(src,dest,src_x,src_ey,dest_x,dest_ey+1,brightness);
        }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockSurface(dest);

    /* done */
    return 0;
}

SDL_Surface *tile_stretch(SDL_Surface *src, int n, int e, int s, int w)
{
    SDL_Surface *destination, *tmp;

    float e_dark = 1.0;
    float w_dark = 1.0;
    int flat;
    int sx,sy,ex,ey;
    int ln_num;

    int dest_sx;
    int dest_sy;
    int dest_ex;
    int dest_ey;
    float dest_slope;

    int dest_sx_2;
    int dest_sy_2;
    int dest_ex_2;
    int dest_ey_2;
    float dest_slope_2;

    int dest_x_inc;   int dest_y_inc;   float kicker;
    int dest_x_inc_2; int dest_y_inc_2; float kicker_2;

    int x1; int y1; int y2;
    int at_least_one;
    int src_len;

    Uint32 color;
    Uint8 red,green,blue,alpha;

    line_and_slope  dest_lines[10];
    SDL_LockSurface(src);

    tmp =   SDL_CreateRGBSurface( src->flags,
                                          src->w,
                                          src->h+n,     /* may need to be bigger */
                                          src->format->BitsPerPixel,
                                          src->format->Rmask,
                                          src->format->Gmask,
                                          src->format->Bmask,
                                          src->format->Amask);

    /* we get as src only already diplsayformat'ed surfaces, so no need to set it again (i hope *g*) */
    destination = SDL_DisplayFormatAlpha(tmp);
    SDL_FreeSurface(tmp);
    /* set transparent  - hope this works   */
    SDL_LockSurface(destination);

    color = TS_GetPixel(src,0,0);

    SDL_GetRGBA(color,src->format,&red,&green,&blue,&alpha);

    if (src->format->BitsPerPixel==8)
        add_color_to_surface(destination,red,green,blue);

//    LOG(LOG_MSG, "tile_stretch: color rgba: %d, %d, %d, %d\n", red, green, blue, alpha);

    /* we fill with black and full trans... */
    color = SDL_MapRGBA(destination->format,0,0,0,0);
    SDL_FillRect( destination, NULL, color);

    if (src->format->BitsPerPixel==8)
        SDL_SetColorKey(destination,SDL_SRCCOLORKEY, color);

    if ( (n==0)&&(e==0)&&(w==0)&&(s==0) ) flat = 0; else flat = 1;

    if (w>e)
    {
        w_dark = 1.0 - ( (w-e) / 25.0 );
        if ( (n>0)||(s>0) )
        {
            e_dark = w_dark;
        }
    }

    if (e>w)
    {
       e_dark = 1.0 + ( (e-w) / 25.0 );
       if ( (s>0)||(n>0) )
       {
           w_dark = e_dark;
       }
    }

    sx= 2; sy=(10-w)+n; ex=22; ey=0;      determine_line(&dest_lines[0],sx,sy,ex,ey);  /* NW */
    sx= 2; sy=(12-w)+n; ex=22; ey=22+n-s; determine_line(&dest_lines[1],sx,sy,ex,ey);  /* SW */
    sx=45; sy=(10-e)+n; ex=25; ey=0;      determine_line(&dest_lines[2],sx,sy,ex,ey);  /* NE */
    sx=45; sy=(12-e)+n; ex=25; ey=22+n-s; determine_line(&dest_lines[3],sx,sy,ex,ey);  /* SE */

    /* do it */

    for (ln_num=0; ln_num<4 ; ln_num++)
    {
        if ( (ln_num==1)||(ln_num==3) ) continue;

        dest_sx    = dest_lines[ln_num].sx;
        dest_sy    = dest_lines[ln_num].sy;
        dest_ex    = dest_lines[ln_num].end_x;
        dest_ey    = dest_lines[ln_num].end_y;
        dest_slope = dest_lines[ln_num].slope;

        if ( (ln_num==0)||(ln_num==2) )
        {
            dest_sx_2    = dest_lines[ln_num+1].sx;
            dest_sy_2    = dest_lines[ln_num+1].sy;
            dest_ex_2    = dest_lines[ln_num+1].end_x;
            dest_ey_2    = dest_lines[ln_num+1].end_y;
            dest_slope_2 = dest_lines[ln_num+1].slope;
        }
        else
        {
            dest_sx_2    = dest_lines[ln_num].sx;
            dest_sy_2    = dest_lines[ln_num].sy;
            dest_ex_2    = dest_lines[ln_num].end_x;
            dest_ey_2    = dest_lines[ln_num].end_y;
            dest_slope_2 = dest_lines[ln_num].slope;
        }

        if (dest_sy>dest_ey) dest_y_inc = -1 ;  else dest_y_inc = 1;
        if (dest_sx>dest_ex) dest_x_inc = -1 ;  else dest_x_inc = 1;
        if (dest_sy_2>dest_ey_2) dest_y_inc_2 = -1 ;  else dest_y_inc_2 = 1;
        if (dest_sx_2>dest_ex_2) dest_x_inc_2 = -1 ;  else dest_x_inc_2 = 1;

        x1=dest_sx; y1=dest_sy; kicker = 0.0;

        y2=dest_sy_2; kicker_2 = 0.0;

        at_least_one = 0;

        //printf("x1=%d,y1=%d, dest_ex=%d, dest_ey=%d dest_slope=%f\n",x1,y1,dest_ex,dest_ey,dest_slope);
        //printf("x1=%d,y1=%d, dest_ex_2=%d, dest_ey_2=%d dest_slope_2=%f\n",x1,y1,dest_ex_2,dest_ey_2,dest_slope_2);

        while ( ( ( dest_slope != 0.0)&&(x1!=dest_ex)&&(y1!=dest_ey)) ||
              ( ( at_least_one==0)&&(dest_slope==0.0) ) )
        {
            at_least_one = 1;
            if (kicker   >= 1.0) { kicker   = kicker   - 1.0; y1=y1+dest_y_inc;   }
            if (kicker_2 >= 1.0) { kicker_2 = kicker_2 - 1.0; y2=y2+dest_y_inc_2; }

            src_len = std_tile_half_len[x1];

            if (ln_num<2) copy_vertical_line(src,destination,x1,11+src_len,11-src_len,x1,y1,y2,w_dark,flat);
            else  copy_vertical_line(src,destination,x1,11+src_len,11-src_len,x1,y1,y2,e_dark,flat);

            /*printf("x1=%d y1=%d y2=%d src_len=%d w_dark=%f\n",x1,y1,y2,src_len,w_dark);   */
            x1 = x1 + dest_x_inc;

            kicker = kicker+dest_slope;
            kicker_2 = kicker_2 + dest_slope_2;
        }
    }

    for (x1=22;x1<22+2;x1++)
        copy_vertical_line(src,destination,x1,0,23,x1,0,23+n-s,w_dark,flat);
    for (x1=24;x1<24+2;x1++)
        copy_vertical_line(src,destination,x1,0,23,x1,0,23+n-s,e_dark,flat);
    for (x1=0;x1<2;x1++)
        copy_pixel_to_pixel(src,destination,x1,11,x1,11+n-w,w_dark);
    for (x1=46;x1<48;x1++)
        copy_pixel_to_pixel(src,destination,x1,11,x1,11+n-e,e_dark);

    //SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags);

    //tmp_surface = SDL_DisplayFormat(src);
    //tmp_surface = SDL_ConvertSurface(destination,src->format,src->flags);

    //SDL_FreeSurface(destination);

    SDL_UnlockSurface(src);
    SDL_UnlockSurface(destination);

{
    Uint8 ix, iy, ir, ig, ib, ia;
    Uint32 icol;

    for (ix=0;ix<destination->w;ix++)
    {
        for (iy=0;iy<destination->h;iy++)
        {
            icol = TS_GetPixel(destination, ix, iy);
            SDL_GetRGBA(icol, destination->format, &ir, &ig, &ib, &ia);
            if ((ia!=0) && ((ir+ig+ib)<20))
                LOG(LOG_MSG, "x,y: (%2d,%2d), rgba: (%3d,%3d,%3d,%3d)\n",ix, iy, ir, ig, ib, ia);

        }
    }


}


    return destination; //tmp_surface; //SDL_ConvertSurface(destination,src->format,src->flags);
}

/******************future************/
SDL_Surface *convert_to_8bit(SDL_Surface *src)
{
   int x,y;
   SDL_Color ckey;
   Uint32 color_key;
   int index=0;

   SDL_Color pal[256];

   color_key = src->format->colorkey;

   SDL_GetRGB(src->format->colorkey,src->format,&ckey.r,&ckey.g, &ckey.b);

   pal[index].r = ckey.r;
   pal[index].g = ckey.g;
   pal[index].b = ckey.b;
   index++;

   for (x=0; x<src->w;x++)
   {
       for (y=0; y<src->h; y++)
       {
           //tmp_color = GetPixel(src,x,y);
       }
   }
   return 0;
}

