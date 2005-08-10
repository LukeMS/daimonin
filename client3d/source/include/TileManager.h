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

#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include "define.h"
#include "Ogre.h"
#include "TileChunk.h"

//=================================================================================================
// The World is devided into chunks. Each of this chunk holds several tiles.
//=================================================================================================
using namespace Ogre;


class CTileManager
{
private:
	struct _Map
	{
		unsigned char dummy;
		unsigned char height;
		unsigned char terrain_col;
		unsigned char terrain_row;
	}** m_Map; // PlayField

	CChunk m_mapchunk[CHUNK_SUM_X][CHUNK_SUM_Y]; // MapChunks
	SceneManager* m_SceneManager;
	AxisAlignedBox* bounds;
	float m_StretchZ;

public:
	CTileManager();
	~CTileManager();
	SceneManager* Get_pSceneManager(){ return m_SceneManager; }
	float Get_StretchZ() { return m_StretchZ;}
	unsigned char Get_Map_Height(short x, short y) { return m_Map[x][y].height; }
	unsigned char Get_Map_Texture_Row(short x, short y) { return m_Map[x][y].terrain_row; }
	unsigned char Get_Map_Texture_Col(short x, short y) { return m_Map[x][y].terrain_col; }

	void  Set_Map_Height(short x, short y, short value) { m_Map[x][y].height = value; }
	void  Set_Map_Texture_Row(short x, short y, unsigned char value) { m_Map[x][y].terrain_row = value; }
	void  Set_Map_Texture_Col(short x, short y, unsigned char value) { m_Map[x][y].terrain_col = value; }
	void  Set_Map_Textures();

	AxisAlignedBox *GetBounds();
	void Init(SceneManager* SceneManager);
	void CreateChunks();
	void ChangeChunks();
	void ControlChunks(Vector3 vector);
	void CreateTexture();
	void CreateTextureGroup(const char *terrain_type); // create the Group-Texture.
	void shrinkFilter(const Image& Filter, const int PIXEL);
	void shrinkTexture(const Image& Texture, const int num, const int PIXEL, const char *terrain_type);
	void Create_Map(); // Errechnet ein graphisches Koordinatensystem für die Spielkarte.
	void SwitchMaterial(bool grid, bool highDetail);
	void addToGroupTexture(uchar* TextureGroup_data, uchar *Filter_data, Image* Texture, short pixel, short size, short x, short y);
};

#endif
