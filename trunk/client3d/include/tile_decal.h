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

#ifndef TILE_DECAL_H
#define TILE_DECAL_H

/**
 ** TileEngine class which handles the decals.
 *****************************************************************************/
class TileDecal
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /** Init the tileengine.
     ** @param SceneMgr    The ogre scenemanager.
     ** @param size        The size of the decal (in subtiles).
     ** @param posX        The world postion.
     ** @param posZ        The world postion.
     ** @param strMaterial The material name.
     *****************************************************************************/
    TileDecal(unsigned int sizeInSubtiles, int posX, int posZ, const char *strMaterial = "Terrain/Decal");
    ~TileDecal();
    void setPosition(int x, int z);
    static const int getSumDecals() { return mSumDecal; }
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum { MAX_SIZE = 8 }; /**< Max size of the decal (in subtiles). **/
    static int mSumDecal;  /**< Active decals. Needed to check for missing destructor calls. **/
    static int mMaxDecal;  /**< Maximal created decals. Needed to create unique resource names. **/
    static Ogre::SceneManager *mSceneManager;
    int mSize;             /**< The size of the decal (in subtiles). **/
    Ogre::SceneNode *mNode;
    Ogre::SubMesh *mSubMesh;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
};

#endif
