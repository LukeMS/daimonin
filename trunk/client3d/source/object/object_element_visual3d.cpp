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

#include <OgreEntity.h>
#include <OgreSubEntity.h>
#include <OgreManualObject.h>
#include <OgreParticleSystem.h>
#include "logger.h"
#include "profiler.h"
#include "tile/tile_manager.h"
#include "object/object_element_visual3d.h"

using namespace Ogre;

static const int WALK_SPEED = TileManager::HALF_RENDER_SIZE;
static const int TURN_SPEED   = 400;
// todo: only store values if they differ from the default one (copy on write).

//================================================================================================
//
//================================================================================================
ObjectElementVisual3d::ObjectElementVisual3d(Object *parent, SceneManager *sceneManager)
    :ObjectElement(parent), mWalkDirection(0), mTurnDirection(0), mEntity(0), mParticle(0)
{
    PROFILE()
    parent->addElement(getFamilyID(), this);
    mNode = sceneManager->getRootSceneNode()->createChildSceneNode();
}

//================================================================================================
//
//================================================================================================
ObjectElementVisual3d::~ObjectElementVisual3d()
{
    PROFILE()
    for (int i = mNode->numAttachedObjects()-1; i>=0; --i)
        mNode->getCreator()->destroyMovableObject(mNode->getAttachedObject(i));
}

//================================================================================================
//
//================================================================================================
Ogre::Entity *ObjectElementVisual3d::createEntity(String nickName, const char *meshName, int renderQueue, ObjectManager::queryMask qMask)
{
    PROFILE()
    mElementAnimation = 0;
    mEntity = mNode->getCreator()->createEntity(nickName, meshName);
    mEntity->setRenderQueueGroup(renderQueue);
    mEntity->setQueryFlags(qMask);
    mNode->attachObject(mEntity);
    // ////////////////////////////////////////////////////////////////////
    // Attach the blob shadow.
    // ////////////////////////////////////////////////////////////////////
    nickName+= "_blob";
    ManualObject* blob = static_cast<ManualObject*>(mNode->getCreator()->createMovableObject(nickName, ManualObjectFactory::FACTORY_TYPE_NAME));
    blob->begin("Material_blob_shadow");
    const AxisAlignedBox &AABB = mEntity->getBoundingBox();
    float sizeX = (AABB.getMaximum().x -AABB.getMinimum().x);
    float sizeY = 0.5f;
    float sizeZ = (AABB.getMaximum().z -AABB.getMinimum().z);
    if (sizeX < sizeZ) sizeX = sizeZ;
    blob->position(-sizeX, sizeY,  sizeX); blob->normal(0,0,1); blob->textureCoord(0.0, 0.0);
    blob->position( sizeX, sizeY,  sizeX); blob->normal(0,0,1); blob->textureCoord(0.0, 1.0);
    blob->position(-sizeX, sizeY, -sizeX); blob->normal(0,0,1); blob->textureCoord(1.0, 0.0);
    blob->position( sizeX, sizeY, -sizeX); blob->normal(0,0,1); blob->textureCoord(1.0, 1.0);
    blob->triangle(0, 1, 2);
    blob->triangle(3, 2, 1);
    blob->end();
    blob->convertToMesh(nickName+ "_mesh");
    blob->setQueryFlags(0);
    blob->setRenderQueueGroup(RENDER_QUEUE_6); // see OgreRenderQueue.h
    mNode->attachObject(blob);
    // ////////////////////////////////////////////////////////////////////
    // If the entity can be animated, return the entity for adding the animation element.
    // ////////////////////////////////////////////////////////////////////
    return (mEntity->hasSkeleton())?mEntity:0;
}

//================================================================================================
//
//================================================================================================
void ObjectElementVisual3d::attachParticle(String particleScript)
{
    PROFILE()
    mParticle = mNode->getCreator()->createParticleSystem(mNickName+"_p", particleScript);
    mParticle->setBoundsAutoUpdated(false);
    mParticle->setKeepParticlesInLocalSpace(true);
    mParticle->setQueryFlags(ObjectManager::QUERY_MASK_NPC);
    mParticle->setRenderQueueGroup(RENDER_QUEUE_7);
    mNode->attachObject(mParticle);
}

//================================================================================================
//
//================================================================================================
void ObjectElementVisual3d::attachCamera(String camera)
{
    PROFILE()
    SceneNode *cNode = mNode->createChildSceneNode();
    cNode->attachObject(mNode->getCreator()->getCamera(camera));
    cNode->setInheritOrientation(false); // Camera needs no turning.
}

//================================================================================================
//
//================================================================================================
void ObjectElementVisual3d::setPosition(Vector3 pos, Real facing)
{
    PROFILE()
    mFacing = facing;
    mTilePos = pos;
    mNode->setPosition(pos);
    mNode->yaw(mFacing);
}

//================================================================================================
//
//================================================================================================
void ObjectElementVisual3d::updateYPos()
{
    PROFILE()
    mTilePos.y = TileManager::getSingleton().getTileHeight((int)mTilePos.x, (int)mTilePos.z);
    mNode->setPosition(mTilePos);
}

//================================================================================================
//
//================================================================================================
void ObjectElementVisual3d::setScale(Vector3 scale)
{
    PROFILE()
    mNode->setScale(scale);
}

//================================================================================================
// Set set skincolor by choosing a color found in the texture (arranged in a vertical colorline).
//================================================================================================
void ObjectElementVisual3d::setSkinColor(uint32 val)
{
    PROFILE()
    const float OFFSET = 0.01f; // Ignore the space of filtering between 2 colors.
    float colorIndex = (val & 31) /32.0f + OFFSET;
    for (unsigned int i= 0; i< mEntity->getNumSubEntities(); ++i)
        mEntity->getSubEntity(i)->setCustomParameter(0, Vector4(OFFSET, colorIndex, 0.0, 0.0));
}

//================================================================================================
//
//================================================================================================
bool ObjectElementVisual3d::update(const Ogre::FrameEvent &event)
{
    PROFILE()
    Vector3 pos = mNode->getPosition();
    if (mWalkDirection)
    {
        Real distance = event.timeSinceLastFrame * WALK_SPEED * mWalkDirection;
        mTilePos.x+= Math::Sin(Degree(mFacing)) * distance;
        mTilePos.z+= Math::Cos(Degree(mFacing)) * distance;
        mTilePos.y = TileManager::getSingleton().getTileHeight((int)mTilePos.x, (int)mTilePos.z);
        mNode->setPosition(mTilePos);
    }
    if (mTurnDirection)
    {
        mNode->yaw(Degree(event.timeSinceLastFrame * TURN_SPEED * mTurnDirection));
        mFacing += Degree(event.timeSinceLastFrame * TURN_SPEED * mTurnDirection);
        if (mFacing.valueDegrees() >= 360.0f) mFacing -= Degree(360.0f);
        if (mFacing.valueDegrees() <    0.0f) mFacing += Degree(360.0f);
    }
    return true;
}

//================================================================================================
// Move to a new tile pos. Return false if the object is out of playfield range.
//================================================================================================
bool ObjectElementVisual3d::setMapScroll(int deltaX, int deltaZ)
{
    PROFILE()
    if (deltaX || deltaZ)
    {
        mTilePos.x += deltaX * TileManager::TILE_RENDER_SIZE;
        mTilePos.z += deltaZ * TileManager::TILE_RENDER_SIZE;
        // if (outside the map range) return false; // Delete outranged objects.
        mNode->setPosition(mTilePos);
    }
    return true;
}

//================================================================================================
// Start/stop turning.
//================================================================================================
void ObjectElementVisual3d::setTurn(int direction)
{
    PROFILE()
    mTurnDirection = direction;
}

//================================================================================================
// Start/stop movement.
//================================================================================================
void ObjectElementVisual3d::setMove(int direction)
{
    PROFILE()
    mWalkDirection = direction;
    if (mElementAnimation)
    {
        if (direction)
            mElementAnimation->setAnimation(ObjectElementAnimate3d::ANIM_GROUP_WALK, 0, true, true);
        else
            mElementAnimation->setAnimation(ObjectElementAnimate3d::ANIM_GROUP_IDLE, 0, true);
    }
}
