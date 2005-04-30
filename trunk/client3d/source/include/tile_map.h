/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
 
#ifndef TILE_MAP_H
#define TILE_MAP_H

#include <Ogre.h>

#include "define.h"

using namespace Ogre;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////
const int MAP_START_XOFF	= 376;
const int MAP_START_YOFF	= 143;
const int MAP_TILE_POS_YOFF	=  23;
const int MAP_TILE_POS_YOFF2=  12;
const int MAP_TILE_POS_XOFF =  48;
const int MAP_TILE_POS_XOFF2=  24;
const int MAP_TILE_XOFF		=  12;
const int MAP_TILE_YOFF		=  24;
const int MAP_MAX_SIZE		=  17;
const int MAXFACES			=   4;

// Table of pre definded multi arch objects. 
// mpart_id and mpart_nr in the arches are commited from server 
// to analyze the exaclty tile position inside a mpart object.
//
// The way of determinate the starting and shift points is explained in the 
// dev/multi_arch folder of the arches, where the multi arch templates & masks are.
typedef struct _multi_part_tile
{
    int xoff;       // X-offset
    int yoff;       // Y-offset
}_multi_part_tile;

typedef struct _multi_part_obj
{
    int                 xlen;		// natural xlen of the whole multi arch
    int                 ylen;		// same for ylen
    _multi_part_tile    part[16];
}_multi_part_obj;

typedef struct _mapdata
{
    char    name[256];
    char    music[256];
    int     xlen;
    int     ylen;
    int     posx;
    int     posy;
}_mapdata;

typedef struct MapCell
{
    short			faces[MAXFACES];
    short			pos[MAXFACES];
    bool			fog_of_war;
    unsigned char   ext[MAXFACES];
    char			pname[MAXFACES][32];
    unsigned char   probe[MAXFACES];
    unsigned char   darkness;
}MapCell;

typedef struct
{
    int	x;
    int y;
} MapPos;

////////////////////////////////////////////////////////////
// Singleton class.
////////////////////////////////////////////////////////////
class TileMap
{
  public:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
	_mapdata         MapData;
	_multi_part_obj  MultiArchs[16];
	MapCell cells[MAP_MAX_SIZE][MAP_MAX_SIZE];
    int MapStatusX;
    int MapStatusY;

    ////////////////////////////////////////////////////////////
	// Functions.
	////////////////////////////////////////////////////////////
     TileMap() {;}
    ~TileMap();
    static TileMap &getSingleton() { static TileMap Singleton; return Singleton; }

	void clear_map(void);
	void display_map_clearcell(long x, long y);
	void set_map_darkness(int x, int y, unsigned char darkness);
	void set_map_face(int x, int y, int layer, int face, int pos, int ext, char *name);
	void clear(void);
	void draw(void);
	void display_mapscroll(int dx, int dy);
	void InitMapData(char *name, int xl, int yl, int px, int py);
	void set_map_ext(int x, int y, int layer, int ext, int probe);
	void load_mapdef_dat(void);
	void adjust_map_cache(int x, int y);
	int  get_tile_position(int mx, int my, int *tx, int *ty);
	void Init(SceneManager *SceneMgr, SceneNode  *Node);
    void updatePlayerPos(const Vector3 &playerOffset) { mTileOffset += playerOffset; }
    void scrollTileMap(int x, int y);
	void freeRecources()
	{
        mpMeshTiles.setNull();		
		mpVertexBuf.setNull(); // Cannot be done by destuctor!
	}

  private:
    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
    SceneNode  *mNode;
	MapCell *TheMapCache;
	MeshPtr mpMeshTiles;
	HardwareVertexBufferSharedPtr mpVertexBuf;
    Vector3 mTileOffset;
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
    TileMap(const TileMap&); // disable copy-constructor.
};

#endif
