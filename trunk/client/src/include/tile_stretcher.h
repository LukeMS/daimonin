/*
    Angelion SDL client, a client program for the Angelion MMORPG.

    Copyright (C) 2005 James Little, et al.

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

#if !defined(__TILE_STRETCHER_H)
#define __TILE_STRETCHER_H

extern void DrawPixel(SDL_Surface *screen, int x, int y, Uint32 color);
extern int copy_verticle_line( SDL_Surface *src,SDL_Surface *dest, 
                        int src_x,  int src_sy,  int src_ey,
                        int dest_x, int dest_sy, int dest_ey,
                        float brightness, int extra);
                        
extern SDL_Surface *tile_stretch(SDL_Surface *src, int n, int e, int s, int w);

#endif
