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

#ifndef OBJECT_VISUALS_H
#define OBJECT_VISUALS_H

#include <Ogre.h>
#include "object_npc.h"

/**
 ** This singleton class handles all visual informations of an object.
 ** Visuals are lifebars, selction gfx's, etc.
 *****************************************************************************/
class ObjectVisuals
{

public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        VISUAL_LIFEBAR,
        VISUAL_SELECTION,
        VISUAL_SUM
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
    void Init();
    void freeRecources();
    /** health < 0 disables the lifebar. */
    void select(const Ogre::AxisAlignedBox &AABB, Ogre::SceneNode *node, int friendly, Ogre::Real percent, const char *name);
    void unselect();
    void highlight(bool staticObject, int friendly, bool highlight);
    void setPosLifebar(Ogre::Vector3 pos);
    void setLifebar(Ogre::Real percent, int barWidth = 128);
    void blit(const Ogre::HardwarePixelBufferSharedPtr &src, const Ogre::Image::Box &srcBox, const Ogre::Image::Box &dstBox);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::ParticleSystem *mPSystem; /**< If <>0 Selection is a particleSystem else Selection is a gfx. **/
    Ogre::Entity *mEntity[VISUAL_SUM];
    Ogre::SceneNode *mNode[VISUAL_SUM];
    Ogre::ColourValue particleColor[PARTICLE_COLOR_SUM];
    Ogre::HardwarePixelBufferSharedPtr mHardwarePB;
    Ogre::Image mImage;
    Ogre::PixelBox mSrcPixelBox;
    unsigned char *mTexBuffer;
    Ogre::String mStrName;
    int mDefaultAction;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectVisuals()  {};
    ~ObjectVisuals() {};
    ObjectVisuals(const ObjectVisuals&); // disable copy-constructor.
    void buildEntity(int index, const char *meshName, const char *entityName);
};

#endif
