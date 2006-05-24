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

#include "tile_interface.h"
#include "tile_manager.h"

///================================================================================================
///
///================================================================================================
TileSelection::TileSelection(TileManager* TileManager)
{
    m_TileManager = TileManager;
    reset();
}

///================================================================================================
///
///================================================================================================
TileSelection::~TileSelection()
{
}

///================================================================================================
///
///================================================================================================
Vector3 TileSelection::get_Selection()
{
    Vector3 tmp;
    if (m_x <0 || m_y <0)
    {
        tmp.x = 0;
        tmp.z = 0;
        tmp.y = 20;
    }
    else
    {
        tmp.x = m_x;
        tmp.z = m_y;
        tmp.y = (Real) (m_TileManager->Get_Avg_Map_Height(m_x, m_y));
    }
    return tmp;
}

///================================================================================================
///
///================================================================================================
void TileSelection::save_Selection()
{
    m_x_old = m_x;
    m_y_old = m_y;
}

///================================================================================================
///
///================================================================================================
void TileSelection::reset()
{
    m_distance = 1000000; // something big
    m_x = -1;
    m_y = -1;
}

///================================================================================================
/// Constructor.
///================================================================================================
TileInterface::TileInterface(TileManager* TileManager)
{
    m_TileManager = TileManager;
    m_SceneNode = m_TileManager->Get_pSceneManager()->getRootSceneNode()->createChildSceneNode("Interface");
    m_Selection = new TileSelection(m_TileManager);
    m_SquareSize = 1;
}

///================================================================================================
/// Destructor.
///================================================================================================
TileInterface::~TileInterface()
{
    delete m_Selection;
    m_TileManager->Get_pSceneManager()->destroyQuery(mRaySceneQuery);

}

///================================================================================================
/// Init.
///================================================================================================
void TileInterface::Init()
{
    mRaySceneQuery = m_TileManager->Get_pSceneManager()->createRayQuery( Ray() );
}

///================================================================================================
/// Mouse picking.
///================================================================================================
void TileInterface::pick_Tile(float mouseX, float mouseY)
{
    /// save old selection to compare to new selection later
    m_Selection->save_Selection();
    m_Selection->reset();

    Ray mouseRay = m_TileManager->Get_pSceneManager()->getCamera("PlayerCam")->getCameraToViewportRay(mouseX, mouseY);
    mRaySceneQuery->setRay(mouseRay);
    mRaySceneQuery->setQueryMask(QUERY_TILES_LAND_MASK);

    /// Perform the scene query.
    RaySceneQueryResult &result = mRaySceneQuery->execute();
    RaySceneQueryResult::iterator itr = result.begin();
    if (result.size() >1)
    {
        Logger::log().error() << "BUG in TileInterface.cpp: RaySceneQuery returned more than 1 result.";
        Logger::log().error() << "(You created Entities without setting a setQueryFlags(...) on them)";
    }
    /// now test which terrain chunk is hit.
    /// TODO: Extract the chunk-pos out of the entity name.
    for (int a = 0; a < CHUNK_SUM_X; ++a)
    {
        for (int b = 0; b < CHUNK_SUM_Z; ++b)
        {
            if (itr->movable == m_TileManager->get_TileChunk(a,b)->Get_Land_entity())
            {
                // we found our chunk, now search for the correct tile
                pick_Tile(&mouseRay,a,b);
            }
        }
    }
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::pick_Tile(Ray* mouseRay, int a, int b)
{
    /// ////////////////////////////////////////////////////////////////////
    /// we start with a given tile chunk (coordinates a and b) and a ray
    /// and try to find the tile that was selected. we have to check every
    /// tile if it was hit and return the nearest one
    /// ////////////////////////////////////////////////////////////////////
    Real height[4], avgHeight;
    int vertex_x = m_TileManager->get_TileChunk(a,b)->get_posX() * CHUNK_SIZE_X;
    int vertex_y = m_TileManager->get_TileChunk(a,b)->get_posZ() * CHUNK_SIZE_Z;

    float StretchZ = m_TileManager->Get_StretchZ();

    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE_Z; ++y)
        {
            /// ////////////////////////////////////////////////////////////////////
            /// we have to build a bounding box for each tile and check if the ray
            /// intersects this box.
            /// To do this, we need the height of the tile  vertices.
            /// ////////////////////////////////////////////////////////////////////
            height[0] = m_TileManager->Get_Map_Height(vertex_x + x    , vertex_y + y    ) * StretchZ;
            height[1] = m_TileManager->Get_Map_Height(vertex_x + x + 1, vertex_y + y    ) * StretchZ;
            height[2] = m_TileManager->Get_Map_Height(vertex_x + x    , vertex_y + y + 1) * StretchZ;
            height[3] = m_TileManager->Get_Map_Height(vertex_x + x + 1, vertex_y + y + 1) * StretchZ;
            avgHeight = (height[0]+height[1]+height[2]+height[3]) /4.0;
            /// ////////////////////////////////////////////////////////////////////
            /// now we build 4 bounding boxes per tile to increase picking accuracy
            /// Note: Ogre only allows bounding boxes with the first vector having
            /// got the lower value in every(!) component. so we have to check
            /// which height value is greater
            /// ////////////////////////////////////////////////////////////////////
            std::pair<bool, Real> Test;
            Real offsetX =0, offsetY=0;
            for (int edge=0; edge < 4; ++edge)
            {
                if (height[edge] > avgHeight)
                    Test = mouseRay->intersects(
                               AxisAlignedBox((vertex_x + x + offsetX      )* TILE_SIZE_X, avgHeight,
                                              (vertex_y + y + offsetY      )* TILE_SIZE_Z,
                                              (vertex_x + x + offsetX + 0.5)* TILE_SIZE_X, height[edge],
                                              (vertex_y + y + offsetY + 0.5)* TILE_SIZE_Z));
                else
                    Test = mouseRay->intersects(
                               AxisAlignedBox((vertex_x + x + offsetX      )* TILE_SIZE_X, height[edge],
                                              (vertex_y + y + offsetY      )* TILE_SIZE_Z,
                                              (vertex_x + x + offsetX + 0.5)* TILE_SIZE_X, avgHeight,
                                              (vertex_y + y + offsetY + 0.5)* TILE_SIZE_Z));
                offsetX+= 0.5;
                if (offsetX > 0.5)
                {
                    offsetX = 0.0;
                    offsetY+= 0.5;
                }
                if (Test.first == true)
                { /// intersection! Find the closest intersection to the camera.
                    if (Test.second < m_Selection->m_distance)
                    {
                        m_Selection->m_distance = Test.second;
                        m_Selection->m_x  = vertex_x + x;
                        m_Selection->m_y  = vertex_y + y;
                    }
                }
            }
        } // end for y
    } // end for x
}

