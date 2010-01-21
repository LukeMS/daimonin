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

#ifndef TILE_CHUNK_H
#define TILE_CHUNK_H

#include <OgreVector3.h>

/**
 ** TileEngine class which handles the tiles in a chunk.
 *****************************************************************************/
class TileChunk
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileChunk()  {}
    ~TileChunk() {}
    /** Init the tilechunk.
     ** @param queryMaskLand  The query mask for land entity.
     ** @param queryMaskWater The query mask for water entity.
     ** @param SceneManager   The ogre SceneManager.
     *****************************************************************************/
    void init(int queryMaskLand, int queryMaskWater, Ogre::SceneManager *SceneManager);
    /** Update the whole chunk. */
    void update()
    {
        updateLand();
        updateWater();
        updateSprites();
    }
    /** Set the shader parameters for the waves on the water.
     ** @param alpha     The alpha value for the water.
     ** @param amplitude The height amplitude for the waves.
     ** @param speed     The speed for the waves.
     *****************************************************************************/
    void setWave(Ogre::Real alpha, Ogre::Real amplitude, Ogre::Real speed);
    /** Set the shader parameter for the grid. */
    void setGrid(bool visible);
    /** Set the shader parameter for the ambient light. */
    void setLight(Ogre::Real brightness);
    /** Set a new atlastexture for the terrain. */
    void setMaterial(int groupNr, int texSize);
    /** Set the camera rotation.
     ** Used to avoid rendering tiles outside the field of view. */
    void setCameraRotation(Ogre::Real angle);
    void setRenderOptions(bool drawGrass);
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    bool mGrid;
    bool mOption_DrawSprites;
    unsigned int mCameraRotation;
    Ogre::SubMesh *mSubMeshLand, *mSubMeshWater, *mSubMeshSprites;
    Ogre::Entity *mEntitySprites;
    Ogre::Vector3 mWaveParam;
    Ogre::Real mDaylight;
    Ogre::Real *mPosVBuf;
    Ogre::Real mTexPosInAtlas[6];
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    TileChunk(const TileChunk&);            /**< disable copy-constructor. **/
    TileChunk &operator=(const TileChunk&); /**< disable assignment operator. **/
    void updateLand();
    void updateWater();
    void updateSprites();
    void setVertex(Ogre::Vector3 &pos, int maskNr, Ogre::Real offsetU, Ogre::Real offsetV, Ogre::Vector4 &params);
    void setTriangle(int x, int z, Ogre::Vector3 v1, Ogre::Vector3 v2, Ogre::Vector3 v3, int maskNr);
    int  getMask(int gfxVertex0, int gfxVertex1, int gfxVertex2);
};

#endif
