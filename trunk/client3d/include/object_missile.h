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

#ifndef OBJ_NISSLE_H
#define OBJ_NISSLE_H

#include <Ogre.h>
#include "define.h"

/**
 ** This class handles all ranged weapon objects.
 *****************************************************************************/
class ObjectMissile
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef enum { DART, ARROW, BOLT, SHURIKEN, SPEAR, BOOMERANG, } enumType;
    typedef enum { FIRE, ICE, POISON,  } enumParticle;
    typedef struct
    {
        unsigned int index;     /**< Unique number for this object. **/
        enumType     type;
        enumParticle particle;
        Ogre::String meshName;  /**< Name of the ogre3d mesh. **/
        Ogre::Real facing;
        int maxDamage;
        int minDamage;
    }
    sObject;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectMissile(int type, ObjectNPC *src, ObjectNPC *dst);
    virtual ~ObjectMissile();
    virtual void freeRecources();
    virtual bool update(const Ogre::FrameEvent& event);
    const Ogre::Vector3 &getPosition()
    {
        return mNode->getPosition();
    }
    Ogre::SceneNode *getSceneNode()
    {
        return mNode;
    }
    Ogre::Real getFacing()
    {
        return mFacing.valueDegrees();
    }
    unsigned int getIndex()
    {
        return mIndex;
    }

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static Ogre::SceneManager *msSceneMgr;
    static unsigned int msUnique;
    Ogre::Degree mFacing;
    unsigned int mIndex;
    Ogre::SceneNode *mNode;
    Ogre::Entity *mEntity;
    int mType;
    int mParticle;
    Ogre::Vector3 mDestPosition;
    Ogre::Vector3 mSpeed;
    bool mHasBallistic;
    bool mShow;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectMissile(const ObjectMissile&); // disable copy-constructor.
};

#endif
