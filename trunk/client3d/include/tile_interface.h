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

class TileSelection
{
private:
    int m_x_old, m_y_old;
    TileManager* m_TileManager;
public:
    int m_x, m_y;
    unsigned int m_SquareSize;
    Real m_distance;
    TileSelection( TileManager* TileManager);
    ~TileSelection();
    Vector3 get_Selection();
    void change_Selection();
    void save_Selection();
    void reset();
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
};

#endif
