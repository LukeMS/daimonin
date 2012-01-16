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

#ifndef __MAP_H
#define __MAP_H

#define MAP_START_XOFF 390
#define MAP_START_YOFF 143

#define MAP_TILE_POS_YOFF 23
#define MAP_TILE_POS_YOFF2 12
#define MAP_TILE_POS_XOFF 48
#define MAP_TILE_POS_XOFF2 24

#define MAP_TILE_XOFF 12
#define MAP_TILE_YOFF 24

typedef struct _mapdata
{
    char    name[256];
    char    music[256];
    int     xlen;
    int     ylen;
    int     posx;
    int     posy;
}
_mapdata;

struct MapCell
{
    short   faces[MAXFACES];
    short   pos[MAXFACES];
    uint8 fog_of_war;
    uint8   ext[MAXFACES];
    char    pname[MAXFACES][32];
    uint8   probe[MAXFACES];
    struct anim_list *anim[MAXFACES];
    uint8   darkness;
    sint16  height;   /* height of this maptile */
    uint32  stretch;  /* how we stretch this is really 8 char for N S E W */
}
MapCell;

struct Map
{
    struct MapCell  cells[MAP_MAX_SIZE][MAP_MAX_SIZE];
}
Map;

extern struct Map the_map;

typedef struct
{
    int                     x;
    int                     y;
}
MapPos;

extern _mapdata         MapData;

extern void             clear_map(void);
extern void             display_map_clearcell(long x, long y);
extern void             set_map_darkness(int x, int y, uint8 darkness);
extern void             set_map_face(int x, int y, int layer, int face, int pos, int ext, char *, sint16 height);
extern void             set_map_height(int x, int y, sint16 height);
extern void             map_draw_map(void);
extern void             display_mapscroll(int dx, int dy);
extern void             InitMapData(int xl, int yl, int px, int py);
extern void             UpdateMapName(char *name);
extern void             UpdateMapMusic(char *music);
extern void             set_map_ext(int x, int y, int layer, int ext, int probe);
extern void             map_overlay(_Sprite *sprite);
extern void             adjust_map_cache(int x, int y);
extern int              get_tile_position(int mx, int my, int *tx, int *ty);

#endif /* ifndef __MAP_H */
