/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "tile_manager.h"

class TileMap
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {MAXFACES = 4};
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
        _multi_part_tile part[16];
    }
    _multi_part_obj;
    _multi_part_obj MultiArchs[16];

    struct _mMapData
    {
        Ogre::String name;
        Ogre::String music;
        int xlen;
        int ylen;
        int posx;
        int posy;
    }
    mMapData;

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
        MapCell cells[TileManager::CHUNK_SIZE][TileManager::CHUNK_SIZE];
    }
    Map;
    Map the_map;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static TileMap &getSingleton()
    {
        static TileMap Singleton; return Singleton;
    }
    void update()
    {
        if (mNeedsRedraw) draw();
    }
    void draw();
    void clear_map(void);
    void display_map_clearcell(long x, long y);
    void set_map_face(int x, int y, int layer, int face, int pos, int ext, char *name);
    void scroll(int dx, int dy);
    void InitMapData(const char *name, int xl, int yl, int px, int py);
    void set_map_ext(int x, int y, int layer, int ext, int probe);
    void map_draw_map_clear(void);
    void adjust_map_cache(int x, int y);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {   // we need this to parse the map and sort the multi tile monsters
        int face;
        int x,y;
        struct _map_object_parse *next;
    }
    _map_object_parse;
    _map_object_parse *start_map_object_parse;
    bool mNeedsRedraw;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileMap();
    ~TileMap();
    TileMap( const TileMap& ); // disable copy-constructor.
};

#endif
