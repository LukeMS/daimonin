/*-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)
Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#ifndef OBJ_STATIC_H
#define OBJ_STATIC_H

#include <Ogre.h>
#include "define.h"
#include "object_static.h"
#include "object_animate.h"

using namespace Ogre;

class ObjectStatic
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectStatic(const char *filename, int posX, int posY, float Facing);
    ~ObjectStatic()
    {}
    void freeRecources();
    void moveToTile(int x, int z);
    void faceToTile(int x, int z);
    void move(Vector3 &pos);
    const Vector3 &getPos()
    {
        return mNode->getPosition();
    }
    const Vector3 &getWorldPos()
    {
        return mTranslateVector;
    }
    const SceneNode *getNode()
    {
        return mNode;
    }
    void update(const FrameEvent& event);
    void setTexture(int pos, int color, int textureNr);
    void toggleMesh   (int pos, int WeaponNr);
    Real getFacing()
    {
        return mFacing.valueRadians();
    }


private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    static unsigned int mInstanceNr; /// mInstanceNr = 0 -> Player's Hero
    static SceneManager *mSceneMgr;

    unsigned int thisStatic;
    TexturePtr mTexture;
    Degree mFacing, mNewFacing;
    int mPosX, mPosZ;   /// the actual tile-pos of the NPC.
    SceneNode *mNode;
    Entity *mEntity;
    Vector3 mTranslateVector, mBoundingBox;
    ObjectAnimate *mAnim;
    Real animOffset; /// every npc gets a random animation offset. preventing of  synchronous "dancing"

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectStatic(const ObjectStatic&); // disable copy-constructor.
};

#endif
