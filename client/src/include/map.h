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

#define MAP_TILE_POS_YOFF 23
#define MAP_TILE_POS_XOFF 48

#define MAP_TILE_XOFF (MAP_TILE_POS_YOFF / 2)
#define MAP_TILE_YOFF (MAP_TILE_POS_XOFF / 2)

#define MAP_START_XOFF (MAP_TILE_POS_XOFF * 8)
#define MAP_START_YOFF (MAP_TILE_POS_XOFF * 3)

#define MAP_REDRAW_FLAG_NO          0
#define MAP_REDRAW_FLAG_NORMAL      (1 << 0)
#define MAP_REDRAW_FLAG_FIRE        (1 << 1)
#define MAP_REDRAW_FLAG_COLD        (1 << 2)
#define MAP_REDRAW_FLAG_ELECTRICITY (1 << 3)
#define MAP_REDRAW_FLAG_LIGHT       (1 << 4)
#define MAP_REDRAW_FLAG_SHADOW      (1 << 5)

extern uint8  map_udate_flag,
              map_transfer_flag;
extern uint32 map_redraw_flag;

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
    uint8   ext[MAXFACES];
    char    pname[MAXFACES][32];
    uint8   probe[MAXFACES];
    struct anim_list *anim[MAXFACES];
    uint8   darkness;
    uint8   fogofwar;
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

#define MAP_XPOS(_x_, _y_) \
    (MAP_START_XOFF * (options.map_scalex / 100.0)) + \
    (_x_) * (MAP_TILE_YOFF * (options.map_scalex / 100.0)) - \
    (_y_) * (MAP_TILE_YOFF * (options.map_scalex / 100.0))
#define MAP_YPOS(_x_, _y_) \
    (MAP_START_YOFF * (options.map_scaley / 100.0)) + \
    (_x_) * (MAP_TILE_XOFF * (options.map_scaley / 100.0)) + \
    (_y_) * (MAP_TILE_XOFF * (options.map_scaley / 100.0))

extern void             clear_map(void);
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
extern int              get_tile_position(int mx, int my, int *tx, int *ty);

#endif /* ifndef __MAP_H */
