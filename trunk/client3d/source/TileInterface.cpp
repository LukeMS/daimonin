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

#include "TileInterface.h"
#include "TileManager.h"

///================================================================================================
///
///================================================================================================
TileSelection::TileSelection(TileManager* TileManager)
{
    m_TileManager = TileManager;
    m_SquareSize = 1;
    reset();
    //  create_Entity();
    m_x = CHUNK_SIZE_X /2;
    m_y = CHUNK_SIZE_Z /2;
}

///================================================================================================
///
///================================================================================================
TileSelection::~TileSelection()
{
    for (int c = 0; c < MAX_SEL_TILES; ++c)
    {
        m_vdata[c] = 0;
        if (m_vdata[c]) delete m_vdata[c];
    }
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
///
///================================================================================================
void TileSelection::select()
{
    /*
      change_Selection();
      if (m_x != m_x_old && m_y != m_y_old)
      {
        // note: insert actions here
        char name[50];
        sprintf( name, "Light %d", counter);
        Light* light = m_TileManager->Get_pSceneManager()->createLight( name );
        light->setType( Light::LT_SPOTLIGHT );
        light->setPosition( Vector3
         ((m_x + .5) * TILE_SIZE, m_TileManager->Get_Map_Height(m_x, m_y) + 150 , (m_y+.5) * TILE_SIZE) );
        light->setDiffuseColour( .8, .8, 1.0 );
        light->setDirection(0,-1,0);
        light->setSpecularColour( 0.0, 1.0, 0.0 );
        light->setSpotlightRange( Degree(45), Degree(120) );
      }
    */
}

///================================================================================================
///
///================================================================================================
void TileSelection::create_Entity()
{}

///================================================================================================
///
///================================================================================================
void TileSelection::change_Selection()
{
    // only change if value differs from old one
    //if (m_x_old == m_x && m_y_old == m_y) return;

    // Tile Selection Marker
    float StretchZ = m_TileManager->Get_StretchZ();

    Real height[4];
    int offsetX = (m_SquareSize-1)/2;
    int offsetY = offsetX;
    int sumSquares = (m_SquareSize >1)?4:1;
    for (int c = sumSquares; c < 4; ++c) m_Entity[c]->setVisible (false);//and hide the s
    for (int c = 0; c < sumSquares; ++c)
    {

        Real* pReal = static_cast<Real*>(m_vbuf0[c]->lock(HardwareBuffer::HBL_NORMAL));
        height[0] = m_TileManager->Get_Map_Height(m_x + offsetX  , m_y + offsetY  ) * StretchZ;
        height[1] = m_TileManager->Get_Map_Height(m_x + offsetX+1, m_y + offsetY  ) * StretchZ;
        height[2] = m_TileManager->Get_Map_Height(m_x + offsetX  , m_y + offsetY+1) * StretchZ;
        height[3] = m_TileManager->Get_Map_Height(m_x + offsetX+1, m_y + offsetY+1) * StretchZ;

        for (int a = 0; a != 4; ++a)
        {
            if (height[a] < LEVEL_WATER_CLP * StretchZ)
                height[a] = LEVEL_WATER_CLP * StretchZ;
        }
        // 1st triangle
        pReal[ 0] =   0; pReal[ 1] =   height[0] ; pReal[2] = 0;
        pReal[ 3] =   0; pReal[ 4] =   0; pReal[5] = 1;
        pReal[ 6] =   0; pReal[ 7] =   0;

        pReal[ 8] =   0; pReal[ 9] =   height[2] ; pReal[10] = TILE_SIZE_Z;
        pReal[11] =   0; pReal[12] =   0; pReal[13] = 1;
        pReal[14] =   0; pReal[15] =   1;

        pReal[16] =   TILE_SIZE_X; pReal[17] =  height[1] ; pReal[18] = 0;
        pReal[19] =   0; pReal[20] =   0; pReal[21] = 1;
        pReal[22] =   1; pReal[23] =   0;

        // 2nd triangle
        pReal[24] =   TILE_SIZE_X; pReal[25] =   height[1] ; pReal[26] = 0;
        pReal[27] =   0; pReal[28] =   0; pReal[29] = 1;
        pReal[30] =   1; pReal[31] =   0;

        pReal[32] =   0; pReal[33] =  height[2] ; pReal[34] = TILE_SIZE_Z;
        pReal[35] =   0; pReal[36] =   0; pReal[37] = 1;
        pReal[38] =   0; pReal[39] =   1;

        pReal[40] =   TILE_SIZE_X; pReal[41] =   height[3]; pReal[42] = TILE_SIZE_Z;
        pReal[43] =   0; pReal[44] =   0; pReal[45] = 1;
        pReal[46] =   1; pReal[47] =   1;
        m_vbuf0[c]->unlock();

        m_SceneNode[c]->setPosition((m_x + offsetX )* TILE_SIZE_X,0,(m_y + offsetY)* TILE_SIZE_Z);
        m_Entity[c]->setVisible(true);
        offsetX *= -1;
        if (c ==1) offsetY*= -1;
    }
}

///================================================================================================
///
///================================================================================================
void TileSelection::set_Square_Size(unsigned int SquareSize)
{
    m_SquareSize = SquareSize;
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
    /// now m_Selection contains the wanted tile
    if (m_Selection->m_x != -1 && m_Selection->m_y != -1)
    {
        m_Selection->select();
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

    for (int x = 0; x != CHUNK_SIZE_X; ++x)
    {
        for (int y = 0; y!= CHUNK_SIZE_Z; ++y)
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

///================================================================================================
/// .
///================================================================================================
void TileInterface::set_Square_Size(unsigned int SquareSize)
{
    m_SquareSize = SquareSize;
    m_Selection->set_Square_Size(SquareSize);// inform the marker of the selected tile
    m_Selection->change_Selection();// update the marker of the selected tile
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::change_Tile_height(int z_direction)
{
    unsigned char value;

    for (int a = (int) m_Selection->m_x - ((int)m_SquareSize-1)/2; a <= (int) m_Selection->m_x +((int)m_SquareSize+1)/2; ++a)
    {
        for (int b = (int) m_Selection->m_y - ((int)m_SquareSize-1)/2; b <= (int) m_Selection->m_y +((int)m_SquareSize+1)/2; ++b)
        {
            value = m_TileManager->Get_Map_Height(a, b)+ z_direction;
            if (value > 220) value = 0;//cap, is his important ?? maybe delete later on
            m_TileManager->Set_Map_Height(a, b, value);
        }
    }
    m_Selection->change_Selection();// update the marker of the selected tile
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::level_Tile__height(int z_direction)
{
    int z_max = -1000;
    int z_min = 1000;
    unsigned char value;

    for (int a = (int)m_Selection->m_x - ((int)m_SquareSize-1)/2; a <= (int)m_Selection->m_x +((int)m_SquareSize+1)/2; ++a)
    {
        for (int b = (int)m_Selection->m_y - ((int)m_SquareSize-1)/2; b <= (int)m_Selection->m_y +((int)m_SquareSize+1)/2; ++b)
        {
            // find the maximal z value in area and store to z_max
            if ((m_TileManager->Get_Map_Height(a, b) > z_max)&&(z_direction > 0))
            {
                z_max = m_TileManager->Get_Map_Height(a, b);
            }
            // find the minimal z value in area and store to z_min
            if ((m_TileManager->Get_Map_Height(a, b) < z_min)&&(z_direction < 0))
            {
                z_min = m_TileManager->Get_Map_Height(a, b);
            }
        }
    }
    // perform the leveling
    for (int a = (int)m_Selection->m_x - ((int)m_SquareSize-1)/2; a <= (int)m_Selection->m_x +((int)m_SquareSize+1)/2; ++a)
    {
        for (int b = (int)m_Selection->m_y - ((int)m_SquareSize-1)/2; b <= (int)m_Selection->m_y +((int)m_SquareSize+1)/2; ++b)
        {
            // level the s to the maximun
            if (z_direction > 0)
            {
                value = m_TileManager->Get_Map_Height(a, b);
                if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
                if (value < z_max) m_TileManager->Set_Map_Height(a, b, z_max);
            }
            // or,level the s to the minimum
            if (z_direction < 0)
            {
                value = m_TileManager->Get_Map_Height(a, b);
                if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
                if (value > z_min) m_TileManager->Set_Map_Height(a, b, z_min);
            }
        }
    }
    m_Selection->change_Selection();// update the marker of the selected tile
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::level_Tile__height(int z_direction,int SquareSize)
{
    int tmpSquareSize = m_SquareSize;
    m_SquareSize = SquareSize;
    level_Tile__height(z_direction);
    m_SquareSize = tmpSquareSize;
}

///================================================================================================
/// .
///================================================================================================
void TileInterface::level_Tile__height(int z_direction,int SquareSize,int x,int y)
{
    int z_max = -1000;
    int z_min =  1000;
    unsigned char value;

    for (int a = x - (SquareSize-1)/2; a <= x +(SquareSize+1)/2; ++a)
    {
        for (int b = y - (SquareSize-1)/2; b <= y +(SquareSize+1)/2; ++b)
        {
            // find the maximal z value in area and store to z_max
            if ((m_TileManager->Get_Map_Height(a, b) > z_max)&&(z_direction > 0))
            {
                z_max = m_TileManager->Get_Map_Height(a, b);
            }
            // find the minimal z value in area and store to z_min
            if ((m_TileManager->Get_Map_Height(a, b) < z_min)&&(z_direction < 0))
            {
                z_min = m_TileManager->Get_Map_Height(a, b);
            }
        }
    }
    // perform the leveling
    for (int a = x - (SquareSize-1)/2; a <= x +(SquareSize+1)/2; ++a)
    {
        for (int b = y - (SquareSize-1)/2; b <= y +(SquareSize+1)/2; ++b)
        {
            // level the s to the maximun
            if (z_direction > 0)
            {
                value = m_TileManager->Get_Map_Height(a, b);
                if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
                if (value < z_max) m_TileManager->Set_Map_Height(a, b, z_max);
            }
            // or,level the s to the minimum
            if (z_direction < 0)
            {
                value = m_TileManager->Get_Map_Height(a, b);
                if (value > 220) value = 0; //cap, is his important ?? maybe delete later on
                if (value > z_min) m_TileManager->Set_Map_Height(a, b, z_min);
            }
        }
    }
}

///================================================================================================
/// .
///================================================================================================
bool TileInterface::Tile__height_is_leveled(int z_direction,int SquareSize,int x,int y)
{
    int z_max = -1000;
    int z_min = 1000;

    for (int a = x - (SquareSize-1)/2; a <= x +(SquareSize+1)/2; ++a)
    {
        for (int b = y - (SquareSize-1)/2; b <= y +(SquareSize+1)/2; ++b)
        {
            // find the maximal z value in area and store to z_max
            if ((m_TileManager->Get_Map_Height(a, b) > z_max)&&(z_direction > 0))
            {
                z_max = m_TileManager->Get_Map_Height(a, b);
            }
            // find the minimal z value in area and store to z_min
            if ((m_TileManager->Get_Map_Height(a, b) < z_min)&&(z_direction < 0))
            {
                z_min = m_TileManager->Get_Map_Height(a, b);
            }
        }
    }
    if(z_min == z_max) return true;
    return false;
}
