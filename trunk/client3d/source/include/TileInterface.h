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

#ifndef TILE_INTERFACE_H
#define TILE_INTERFACE_H

#include "Ogre.h"
#include "logger.h"

class TileManager;
class TileInterface;

using namespace Ogre;

const Real MOUSE_SENSITY = 0.002;

class TileSelection
{
private:

  MeshPtr m_Mesh;
  MeshPtr m_MeshCorner1;
  MeshPtr m_MeshCorner2;
  MeshPtr m_MeshCorner3;
  MeshPtr m_MeshCorner4;
  SubMesh* m_SubMesh;
  SubMesh* m_SubMeshCorner1;
  SubMesh* m_SubMeshCorner2;
  SubMesh* m_SubMeshCorner3;
  SubMesh* m_SubMeshCorner4;
  VertexData* m_vdata;
  VertexData* m_vdataCorner1;
  VertexData* m_vdataCorner2;
  VertexData* m_vdataCorner3;
  VertexData* m_vdataCorner4;
  IndexData* m_idata;
  IndexData* m_idataCorner1;
  IndexData* m_idataCorner2;
  IndexData* m_idataCorner3;
  IndexData* m_idataCorner4;
  Entity* m_Entity;
  Entity* m_EntityCorner1;
  Entity* m_EntityCorner2;
  Entity* m_EntityCorner3;
  Entity* m_EntityCorner4;
  SceneNode* m_SceneNode;
  SceneNode* m_SceneNodeCorner1;
  SceneNode* m_SceneNodeCorner2;
  SceneNode* m_SceneNodeCorner3;
  SceneNode* m_SceneNodeCorner4;
  HardwareVertexBufferSharedPtr m_vbuf0;
  HardwareVertexBufferSharedPtr m_vbuf0Corner1;
  HardwareVertexBufferSharedPtr m_vbuf0Corner2;
  HardwareVertexBufferSharedPtr m_vbuf0Corner3;
  HardwareVertexBufferSharedPtr m_vbuf0Corner4;

  int m_x_old;
  int m_y_old;
  TileManager* m_TileManager;

public:
  int m_x;
  int m_y;
  unsigned int m_SquareSize;
  Real m_distance;


  TileSelection( TileManager* TileManager);
  ~TileSelection();
  Vector2 get_Selection();
  void create_Entity();
  void change_Selection();
  void save_Selection();
  void reset();
  void select();
  void set_Square_Size(unsigned int SquareSize);
};

#ifndef DAIMONIN
class TileMouse
{
private:

  Real m_x;
  Real m_y;

  TileInterface* m_Interface;
  Rectangle2D* m_Rect;
  SceneNode* m_SceneNode;

public:

  TileMouse(TileInterface* TileInterface);
  ~TileMouse();

  Real get_x(){
    return m_x;}
  Real get_y(){
    return m_y;}
  void Init();
  void set_Position(Real x, Real y);
  void move_Relative(Real x, Real y);
};
#endif

class TileInterface
{
private:
  TileManager* m_TileManager;
  SceneNode* m_SceneNode;
  #ifndef DAIMONIN
  TileMouse* m_Mouse;
  #endif
  TileSelection* m_Selection;
  unsigned int m_SquareSize;

public:

  TileInterface(TileManager* TileManager);
  ~TileInterface();

  Vector2 get_Selection();
  SceneNode* get_SceneNode() { return m_SceneNode; }
  #ifndef DAIMONIN
  TileMouse* get_Mouse()     { return m_Mouse; }
  #endif
  void Init();
  void pick_Tile();
  void pick_Tile(float mMouseX, float mMouseY);
  void pick_Tile(Ray* mouseRay, int a, int b);
  void set_Square_Size(unsigned int SquareSize);
  void change_Tile_height(int z_direction);
  void level_Tile_Corner_height(int z_direction);//directly: x,y retrieved from the mouse selection,squaresize from the user input
  void level_Tile_Corner_height(int z_direction,int SquareSize);//x,y retrieved from the mouse selection
  void level_Tile_Corner_height(int z_direction,int SquareSize,int x,int y);//not depending on user input, for loadeing lvl methodes i.e.
  bool Tile_Corner_height_is_leveled(int z_direction,int SquareSize,int x,int y);//returns true if leveling is not needed, because its allready leveld
};


#endif
