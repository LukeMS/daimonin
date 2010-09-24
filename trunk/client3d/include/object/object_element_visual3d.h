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

/// @brief This class handles the standard 3d functions of an object.
/// @details Remember that a 3d object has often a 2d component e.g:
///          the item gfx of the equipped weapon in the equippment window
///          or the face of the creature in the trading window.
class ObjectElementVisual3d: public ObjectElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /// @brief Default constructor.
    ObjectElementVisual3d(Object *parent, Ogre::SceneManager *sceneManager);

    /// @brief Default destructor.
    ~ObjectElementVisual3d();

    /// @brief Get the familyID of the element.
    Object::familyID getFamilyID() const { return Object::FAMILY_VISUAL3D; }

    /// @brief Update this element.
    /// @param event Ogre frame event. Used to get time since last frame.
    bool update(const Ogre::FrameEvent &event);

    /// @brief Creates the Ogre::Entity.
    /// @param nickName    A uniqueID for this entity. Is also used as nickname.
    /// @param meshName    The name of the Ogre::Mesh.
    /// @param renderQueue The renderQueue.
    /// @param queryMask   The query mask.
    Ogre::Entity *createEntity(Ogre::String nickName, const char *meshName, int renderQueue, ObjectManager::queryMask qMask);


    /// @brief Attach a particleSystem to the Ogre::Entity.
    /// @param particleScript The name of the particle-script.
    void attachParticle(Ogre::String particleScript);

    /// @brief Scale the Ogre::Entity.
    /// @details Strange things can happen to attached meshes while scaled.
    void setScale(Ogre::Vector3 scale);

    void setPosition(Ogre::Vector3 pos, Ogre::Real facing);
    void setPosition(Ogre::Vector3 pos) { mTilePos = pos; }
    const Ogre::Vector3 getPosition() const { return mTilePos; }
    const Ogre::Degree getFacing() const { return mFacing; }
    bool setMapScroll(int deltaX, int deltaZ);
    void updateYPos();

    /// @brief Rotates the 3d object on the y-axis.
    /// @param direction 0 = Stop, 1= Forward, 2= Backward.
    void setTurn(int direction);

    /// @brief Moves the 3d object in the facing direction.
    /// @param direction 0 = Stop, 1= Forward, 2= Backward.
    void setMove(int direction);

    /// @brief Attach the camera to this entity.
    void attachCamera(Ogre::String camera);

    /// @brief A shortcut to the animation element.
    void setAnimationElement(ObjectElementAnimate3d *element) { mElementAnimation = element; }

    /// @brief Set the skin color of the Ogre::Entity.
    void setSkinColor(Ogre::uint32 color);

private:
    bool mIsAvatar;
    int mWalkDirection;
    int mTurnDirection;
    Ogre::SceneNode *mNode;
    Ogre::Entity *mEntity;
    Ogre::ParticleSystem *mParticle;
    Ogre::Vector3 mBoundingBox;
    Ogre::Vector3 mTilePos;   ///< The actual pos in the map.
    Ogre::String mNickName;
    Ogre::Degree mFacing;
    ObjectElementAnimate3d *mElementAnimation; ///< Shortcut to the aminamtion element.
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementVisual3d(const ObjectElementVisual3d&);            ///< disable copy-constructor.
    ObjectElementVisual3d &operator=(const ObjectElementVisual3d&); ///< disable assignment operator.
};

#endif
