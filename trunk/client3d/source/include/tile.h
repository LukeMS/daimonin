/*-----------------------------------------------------------------------------
This source file is part of Code-Black (http://www.code-black.org)
Copyright (c) 2005 by the Code-Black Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef TILE_H
#define TILE_H

#include "Ogre.h"
using namespace Ogre;

const int TILES_MAX_X = 256;
const int TILES_MAX_Y = 256;


const int LEVEL_TEST_WATER = 15;

// Height levels.
enum {
	// Mountains
	LEVEL_MOUNTAIN_TOP = 70,
	LEVEL_MOUNTAIN_MID = 60,
	LEVEL_MOUNTAIN_DWN = 40,
	// Plains
	LEVEL_PLAINS_TOP = 18,
	LEVEL_PLAINS_MID = 14,
	LEVEL_PLAINS_DWN = 12,
	LEVEL_PLAINS_SUB = 10,
	// Water
	LEVEL_WATER_CLP = LEVEL_PLAINS_SUB+1, // At this point the water clips the land-tiles.
	LEVEL_WATER_TOP = LEVEL_WATER_CLP -1,
	};

// Outdoor 
//enum {TERRAIN_MOUNTAIN, TERRAIN_JUNGLE, TERRAIN_PLAINS, TERRAIN_SWAMP, TERRAIN_RIFT,TERRAIN_DESERT,
//			 TERRAIN_BEACH,  TERRAIN_WATER};
// Spezical Effects: Snopw, Rain, Lava, 

class Ctile
{
private:
	short m_terrain_texture;
	short m_filter_col;
	short m_filter_row;
	short m_flipping;
	short m_terrain;
	short m_height;

public:
	void Set_terrain_texture(short ter) { m_terrain_texture = ter; }
	void Set_filter_col(short fil) { m_filter_col = fil; }
	void Set_filter_row(short fil) { m_filter_row = fil; }
	void Set_flipping(short fli) { m_flipping = fli; }
	void Set_terrain(short ter) { m_terrain = ter; }
	void Set_height(short hei) { m_height = hei; }
	short Get_terrain_textur() { return m_terrain_texture; }
	short Get_filter_col() { return m_filter_col; }
	short Get_filter_row() { return m_filter_row; }
	short Get_flipping() { return m_flipping; }
	short Get_terrain() { return m_terrain; }
	short Get_height() { return m_height; }
};

class Cworldmap
{
private:
	Ctile (**tile);
	short m_heightdata[TILES_MAX_X+1][TILES_MAX_Y+1];

public:
	Cworldmap();
	~Cworldmap();
	void Load();
	// IMPORTANT: Caller must do the check for x, y.
	Ctile* Get_ptile(short x, short y) {return &tile[x][y];}
	short Get_heightdata(short x,short y){ return m_heightdata[x][y];}
};

#endif
