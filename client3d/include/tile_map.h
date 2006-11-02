/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef TILE_MAP_H
#define TILE_MAP_H
#define MAP_START_XOFF 376

#define MAP_START_YOFF 143
#define MAP_TILE_POS_YOFF 23
#define MAP_TILE_POS_YOFF2 12
#define MAP_TILE_POS_XOFF 48
#define MAP_TILE_POS_XOFF2 24

#define MAP_TILE_XOFF 12
#define MAP_TILE_YOFF 24
#define MAXFACES 4
#define MAP_MAX_SIZE  17

// table of pre definded multi arch objects.
// mpart_id and mpart_nr in the arches are commited from server
// to analyze the exaclty tile position inside a mpart object.

// The way of determinate the starting and shift points is explained
// in the dev/multi_arch folder of the arches, where the multi arch templates & masks are.


enum
{
    FFLAG_SLEEP     =1 << 0, // object sleeps.
    FFLAG_CONFUSED  =1 << 1, // object is confused.
    FFLAG_PARALYZED =1 << 2, // object is paralyzed.
    FFLAG_SCARED    =1 << 3, // object is scared.
    FFLAG_BLINDED   =1 << 4, // object is blinded.
    FFLAG_INVISIBLE =1 << 5, // object is invisible (but when send, player can see it).
    FFLAG_ETHEREAL  =1 << 6, // object is etheral   (but when send, object can be seen).
    FFLAG_PROBE     =1 << 7, // object s target of player.
};

class TileMap
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        int xoff;       // X-offset.
        int yoff;       // Y-offset.
    }
    _multi_part_tile;

    typedef struct
    {
        int xlen;       // natural xlen of the whole multi arch.
        int ylen;       // same for ylen.
        _multi_part_tile  part[16];
    }
    _multi_part_obj;
    _multi_part_obj MultiArchs[16];

    typedef struct
    {
        char name[256];
        char music[256];
        int  xlen;
        int  ylen;
        int  posx;
        int  posy;
    }
    _mapdata;
    _mapdata MapData;

    typedef struct
    {
        short         faces[MAXFACES];
        short         pos[MAXFACES];
        bool          fog_of_war;
        unsigned char ext[MAXFACES];
        char          pname[MAXFACES][32];
        unsigned char probe[MAXFACES];
        unsigned char darkness;
    }
    MapCell;
    char *TheMapCache;

    typedef struct
    {
        MapCell cells[MAP_MAX_SIZE][MAP_MAX_SIZE];
    }
    Map;
    Map the_map;

    typedef struct
    {
        int x;
        int y;
    }
    MapPos;





    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static TileMap &getSingleton()
    {
        static TileMap Singleton; return Singleton;
    }
    void clear_map(void);
    void display_map_clearcell(long x, long y);
    void set_map_darkness(int x, int y, unsigned char darkness);
    void set_map_face(int x, int y, int layer, int face, int pos, int ext, char *name);
    void map_draw_map(void);
    void display_mapscroll(int dx, int dy);
    void InitMapData(char *name, int xl, int yl, int px, int py);
    void set_map_ext(int x, int y, int layer, int ext, int probe);
    void map_draw_map_clear(void);
    void load_mapdef_dat(void);
    void adjust_map_cache(int x, int y);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
// we need this to parse the map and sort the multi tile monsters
    typedef struct _map_object_parse
    {
        int                         face;
        int                         x;
        int                         y;
        struct _map_object_parse   *next;
    }
    _map_object_parse;
    _map_object_parse *start_map_object_parse;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileMap();
    ~TileMap();
    TileMap( const TileMap& ); // disable copy-constructor.
};

#endif
