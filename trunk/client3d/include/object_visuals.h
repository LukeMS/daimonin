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

#ifndef OBJECT_VISUALS_H
#define OBJECT_VISUALS_H

#include "Ogre.h"

using namespace Ogre;


class ObjectVisuals
{
public:
    enum
    {
        NPC_LIFEBAR,
        NPC_SELECTION,
        NPC_SUM
    };
    enum
    {
        PARTICLE_COLOR_FRIEND_STRT,
        PARTICLE_COLOR_FRIEND_STOP,
        PARTICLE_COLOR_ENEMY_STRT,
        PARTICLE_COLOR_ENEMY_STOP,
        PARTICLE_COLOR_NEUTRAL_STRT,
        PARTICLE_COLOR_NEUTRAL_STOP,
        PARTICLE_COLOR_SUM
    };

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static ObjectVisuals &getSingleton()
    {
        static ObjectVisuals Singleton; return Singleton;
    }
    void freeRecources();
    void selectNPC(MovableObject *mob, int friendly, bool drawLifebar = true);
    void setPosLifebar(Vector3 pos);
    void setLifebar(Real percent, int barWidth = 128);
    void blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox);
    const AxisAlignedBox &getBoundingBox()
    {
        return mAABB;
    }
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    ParticleSystem *mPSystem; /**< If <>0 Selection is a particleSystem else Selection is a gfx. **/
    Entity *mEntity[NPC_SUM];
    SceneNode *mNode[NPC_SUM];
    ColourValue particleColor[PARTICLE_COLOR_SUM];
    HardwarePixelBufferSharedPtr mHardwarePB;
    Image mImage;
    PixelBox mSrcPixelBox;
    uchar *mTexBuffer;
    AxisAlignedBox mAABB;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectVisuals();
    ~ObjectVisuals();
    ObjectVisuals(const ObjectVisuals&); // disable copy-constructor.
    void buildEntity(int index, const char *meshName, const char *entityName);
};

#endif
