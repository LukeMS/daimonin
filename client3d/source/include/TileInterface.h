/*-----------------------------------------------------------------------------
This source file is part of Code-Black (http://www.code-black.org)
Copyright (c) 2005 by the Code-Black Team
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
-----------------------------------------------------------------------------*/

#ifndef TILE_INTERFACE_H
#define TILE_INTERFACE_H

#include "Ogre.h"
#include "logger.h"

class TileManager;
class TileInterface;

using namespace Ogre;

const Real MOUSE_SENSITY = 0.002;
const int MAX_SEL_TILES  = 4;
class TileSelection
{
private:
  MeshPtr m_Mesh[MAX_SEL_TILES];
  HardwareVertexBufferSharedPtr m_vbuf0[MAX_SEL_TILES];


  Entity* m_Entity[MAX_SEL_TILES];
  //IndexData* m_idata[MAX_SEL_TILES];
  SubMesh* m_SubMesh[MAX_SEL_TILES];
  VertexData* m_vdata[MAX_SEL_TILES];
  SceneNode* m_SceneNode[MAX_SEL_TILES];
  int m_x_old, m_y_old;
  TileManager* m_TileManager;

public:
  int m_x, m_y;
  unsigned int m_SquareSize;
  Real m_distance;

  TileSelection( TileManager* TileManager);
  ~TileSelection();
  Vector3 get_Selection();
  void create_Entity();
  void change_Selection();
  void save_Selection();
  void reset();
  void freeResources();
  void select();
  void set_Square_Size(unsigned int SquareSize);
};

class TileInterface
{
private:
  TileManager* m_TileManager;
  SceneNode* m_SceneNode;
  TileSelection* m_Selection;
  unsigned int m_SquareSize;
  RaySceneQuery* mRaySceneQuery;

public:
  TileInterface(TileManager* TileManager);
  ~TileInterface();
  const Vector3 get_Selection()
  {
    return m_Selection->get_Selection();
  }
  SceneNode* get_SceneNode()
  {
    return m_SceneNode;
  }
  void Init();
  void pick_Tile();
  void pick_Tile(float mMouseX, float mMouseY);
  void pick_Tile(Ray* mouseRay, int a, int b);
  void set_Square_Size(unsigned int SquareSize);
  void change_Tile_height(int z_direction);
  void level_Tile__height(int z_direction);//directly: x,y retrieved from the mouse selection,squaresize from the user input
  void level_Tile__height(int z_direction,int SquareSize);//x,y retrieved from the mouse selection
  void level_Tile__height(int z_direction,int SquareSize,int x,int y);//not depending on user input, for loadeing lvl methodes i.e.
  bool Tile__height_is_leveled(int z_direction,int SquareSize,int x,int y);//returns true if leveling is not needed, because its allready leveld
};

#endif
