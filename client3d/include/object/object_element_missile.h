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

#ifndef OBJECT_ELEMENT_MISSLE_H
#define OBJECT_ELEMENT_MISSLE_H

#include "object/object.h"

/// @brief This class handles 3d projectiles from ranged weapons.
/// @details
class ObjectMissile : public ObjectElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef enum { DART, ARROW, BOLT, SHURIKEN, SPEAR, BOOMERANG, } enumType;
    typedef enum { FIRE, ICE, POISON,  } enumParticle;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /// @brief Default constructor.
    ObjectMissile(Object *parent, int type, Ogre::SceneNode *srcNode, Ogre::SceneNode *dstNode);

    /// @brief Default destructor.
    ~ObjectMissile();

    /// @brief Get the familyID of the element.
    Object::familyID getFamilyID() const
    {
        return Object::FAMILY_MISSILE3D;
    }

    /// @brief Update this element.
    /// @param event Ogre frame event. Used to get time since last frame.
    bool update(const Ogre::FrameEvent &event);
/*
    ObjectMissile(int type, ObjectNPC *src, ObjectNPC *dst);
    const Ogre::Vector3 &getPosition()
    {
        return mNode->getPosition();
    }
    Ogre::SceneNode *getSceneNode()
    {
        return mNode;
    }
    Ogre::Real getFacing() const
    {
        return mFacing.valueDegrees();
    }
    unsigned int getIndex() const
    {
        return mIndex;
    }
*/
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        unsigned int index;
        enumType     type;
        enumParticle particle;
        Ogre::String meshName;
        Ogre::Real facing;
        int maxDamage;
        int minDamage;
    }
    sObject;
/*
    static Ogre::SceneManager *msSceneMgr;
    static unsigned int msUnique;
    unsigned int mIndex;
    Ogre::Degree mFacing;
    Ogre::SceneNode *mNode;
    Ogre::Entity *mEntity;
    Ogre::Vector3 mDestPosition;
    Ogre::Vector3 mSpeed;
    int mType;
    int mParticle;
    bool mHasBallistic;
    bool mShow;
*/
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectMissile(const ObjectMissile&);            ///< disable copy-constructor.
    ObjectMissile &operator=(const ObjectMissile&); ///< disable assignment operator.
};

#endif
