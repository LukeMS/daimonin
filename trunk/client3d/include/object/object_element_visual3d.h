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

#ifndef OBJECT_ELEMENT_VISUAL3D_H
#define OBJECT_ELEMENT_VISUAL3D_H

#include <OgreSceneManager.h>
#include "object/object_manager.h"
#include "object/object_element.h"
#include "object/object_element_animate3d.h"

/**
 ** This class handles the 3d specific part of an object.
 ** Remember that a 3d object has often a 2d component e.g:
 ** - the item gfx of the equipped weapon in the equippment window.
 ** - the face of the creature in the trading window.
 *****************************************************************************/
class ObjectElementVisual3d: public ObjectElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementVisual3d(Object *parent, Ogre::SceneManager *sceneManager);
    ~ObjectElementVisual3d() {}
    Object::familyID getFamilyID() const { return Object::FAMILY_VISUAL3D; }
    bool update(const Ogre::FrameEvent &event);

    Ogre::Entity *createEntity(Ogre::String nickName, const char *meshName, int renderQueue, ObjectManager::queryMask qMask);
    void setScale(Ogre::Vector3 scale);
    void setPosition(Ogre::Vector3 pos, Ogre::Real facing);
    void setPosition(Ogre::Vector3 pos) { mTilePos = pos; }
    const Ogre::Vector3 getPosition() const { return mTilePos; }
    const Ogre::Degree getFacing() const { return mFacing; }
    bool setMapScroll(int deltaX, int deltaZ);
    void updateYPos();
    /**
     ** Rotates the 3d object on the y-axis.
     ** @param direction 0 = Stop, 1= Forward, 2= Backward.
     *****************************************************************************/
    void setTurn(int direction);
    /**
     ** Moves the 3d object in the facing direction.
     ** @param direction 0 = Stop, 1= Forward, 2= Backward.
     *****************************************************************************/
    void setMove(int direction);
    void attachCamera(Ogre::String camera);
    void setAnimationElement(ObjectElementAnimate3d *element) { mElementAnimation = element; }
    void setSkinColor(Ogre::uint32 val);
private:
    bool mIsAvatar;
    int mWalkDirection, mTurnDirection;
    Ogre::SceneNode *mNode;
    Ogre::Entity *mEntity;
    Ogre::Vector3 mBoundingBox;
    Ogre::Vector3 mTilePos;   /**< the actual pos in the map. **/
    Ogre::String mNickName;
    Ogre::Degree mFacing;
    ObjectElementAnimate3d *mElementAnimation;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementVisual3d(const ObjectElementVisual3d&);            /**< disable copy-constructor. **/
    ObjectElementVisual3d &operator=(const ObjectElementVisual3d&); /**< disable assignment operator. **/
};

#endif
