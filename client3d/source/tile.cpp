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

#include "define.h"
#include "tile.h"

//=================================================================================================
// 
//=================================================================================================
Cworldmap::Cworldmap()
{
	tile = new Ctile*[TILES_MAX_X];
	for(int x = 0; x < TILES_MAX_X; ++x) { tile[x] = new Ctile[TILES_MAX_Y]; }
}

//=================================================================================================
// 
//=================================================================================================
Cworldmap::~Cworldmap()
{
	for(int y = 0; y < TILES_MAX_Y; ++y) { delete[] tile[y]; }
	delete[] tile;
}

//=================================================================================================
// 
//=================================================================================================
void Cworldmap::Load()
{
	Image image;
	image.load(FILE_HEIGHT_MAP, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
	uchar* heightdata_temp = image.getData();
	int dimx = image.getWidth(); 
	int dimy = image.getHeight();
	int posX = 0, posY;
	///////////////////////////////////////////////////////////////////////// 
	// Fill the heightdata buffer with the image-color.
	///////////////////////////////////////////////////////////////////////// 
	for(int x = 0; x < TILES_MAX_X; ++x)
	{
		posY =0;
		for(int y = 0; y < TILES_MAX_Y; ++y)
		{
			m_heightdata[x][y] = heightdata_temp[posY * dimx + posX];
			if (++posY > dimy) posY =0; // if necessary, repeat the image.
		}
		if (++posX > dimx) posX =0; // if necessary, repeat the image.
	}
	///////////////////////////////////////////////////////////////////////// 
	// Set the textures for the given height.
	///////////////////////////////////////////////////////////////////////// 
	short height;
	for (int x = 0; x < TILES_MAX_X; ++x)
	{
		for (int y = 0; y < TILES_MAX_Y; ++y)
		{
			height = (m_heightdata[x][y] + m_heightdata[x][y+1]+ m_heightdata[x+1][y] + m_heightdata[x+1][y+1]) / 4;
			tile[x][y].Set_height(height);
			///////////////////////////////////////////////////////////////////////// 
			// Highland.
			///////////////////////////////////////////////////////////////////////// 

			if (height > LEVEL_MOUNTAIN_TOP)
			{
				tile[x][y].Set_terrain(0);
				tile[x][y].Set_terrain_texture(1);
			}
			else if (height > LEVEL_MOUNTAIN_MID)
			{
				if (rand() % 2)
				{
					tile[x][y].Set_terrain(6);
					tile[x][y].Set_terrain_texture(0);
				}
				else
				{
					tile[x][y].Set_terrain(0);
					tile[x][y].Set_terrain_texture(0);
				}
			}
			else if (height > LEVEL_MOUNTAIN_DWN)
			{
				tile[x][y].Set_terrain(rand() % 2 + 4);
				tile[x][y].Set_terrain_texture(2);
			}
			///////////////////////////////////////////////////////////////////////// 
			// Plain.
			///////////////////////////////////////////////////////////////////////// 
			else if (height > LEVEL_PLAINS_TOP)
			{ // Plain
				tile[x][y].Set_terrain(rand() % 2);
				tile[x][y].Set_terrain_texture(2);
			}
			else if (height > LEVEL_PLAINS_MID)
			{ 
				tile[x][y].Set_terrain(6);
				tile[x][y].Set_terrain_texture(3);
			}
			
			else if (height > LEVEL_PLAINS_DWN)
			{ 
				tile[x][y].Set_terrain(0);
				tile[x][y].Set_terrain_texture(4);
			}
			else if (height > LEVEL_PLAINS_SUB)
			{ 
				tile[x][y].Set_terrain(3);
				tile[x][y].Set_terrain_texture(3);
			}
			///////////////////////////////////////////////////////////////////////// 
			// Sea-Ground.
			///////////////////////////////////////////////////////////////////////// 
			else
			{
				tile[x][y].Set_terrain(3);
				tile[x][y].Set_terrain_texture(3);
			}
		}
	}
}
