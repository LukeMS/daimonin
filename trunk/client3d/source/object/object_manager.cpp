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
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include "logger.h"
#include "profiler.h"
#include "sound.h"
#include "network.h"
#include "object/object_manager.h"
#include "object/object_element_animate3d.h"
#include "object/object_element_visual3d.h"
#include "object/object_element_equip3d.h"
#include "object/object_element_physical.h"

using namespace Ogre;

static const char MATERIAL_HIGHLIGHT[] = "MatObjectHighlight";

//================================================================================================
// Init the model from the description file.
//================================================================================================
void ObjectManager::init(SceneManager *sceneManager)
{
    PROFILE()
    Logger::log().headline() << "Init Object Managaer";
    mSelectedType  =-1;
    mSelectedObject=-1;
    // mSelectedEnemy = false;
    mAvatarName.clear();
    mSceneManager = sceneManager;
}

//================================================================================================
//
//================================================================================================
void ObjectManager::freeRecources()
{
    PROFILE()
    for (std::map<std::string, class Object*>::const_iterator i=mObjectMap.begin(); i!=mObjectMap.end(); ++i)
    {
        if ((*i).second)
            delete (*i).second;
    }
    mObjectMap.clear();
}

//================================================================================================
//
//================================================================================================
ObjectManager::~ObjectManager()
{
    PROFILE()
}

//================================================================================================
//
//================================================================================================
const Ogre::Vector3 ObjectManager::getAvatarPos()
{
    PROFILE()
    ObjectElementVisual3d *element = static_cast<ObjectElementVisual3d*>(mObjectAvatar->getElement(Object::FAMILY_VISUAL3D));
    return element->getPosition();
}

//================================================================================================
// Add a creature to the object pool.
//================================================================================================
void ObjectManager::addCreature(sObject &obj)
{
    PROFILE()
    Logger::log().headline() << "Add object (creature):";
    Object *object = new Object();
    mObjectMap.insert(std::pair<std::string, class Object*>(obj.nickName, object));
    ObjectElementVisual3d *objV3d = new ObjectElementVisual3d(object, mSceneManager);
    Entity *entityWithSkeleton = objV3d->createEntity(obj.nickName, obj.meshName.c_str(), Ogre::RENDER_QUEUE_7, QUERY_MASK_NPC);
    if (entityWithSkeleton)
    {
        objV3d->setAnimationElement(new ObjectElementAnimate3d(object, entityWithSkeleton));
        new ObjectElementEquip3d(object, entityWithSkeleton);
        /*ObjectElementEquip3d *objEquip =*/
    }
    objV3d->setPosition(obj.pos, obj.facing);
    Real spawnSize = 1.0f;
    //spawnSize = (!spawn || !mIndex) ? 1.0f : 0.0f;
    objV3d->setScale(Vector3(spawnSize, spawnSize, spawnSize));
    // ////////////////////////////////////////////////////////////////////
    // Add the camera to the avatar.
    // ////////////////////////////////////////////////////////////////////
    if (obj.nickName == mAvatarName)
    {
        // Attach camera to avatars node (Bounding box is increased by that and can't be used for collision detection anymore)
        mObjectAvatar = object;
        objV3d->attachCamera("PlayerCam");
        objV3d->updateYPos();
    }
}

//================================================================================================
// Update all objects.
//================================================================================================
void ObjectManager::update(const FrameEvent &event)
{
    PROFILE()
    for (std::map<std::string, class Object*>::const_iterator i=mObjectMap.begin(); i!=mObjectMap.end(); ++i)
        (*i).second->update(event);
}

//================================================================================================
// Update all object positions after a map scroll.
//================================================================================================
void ObjectManager::syncToMapScroll(int deltaX, int deltaZ)
{
    PROFILE()
    for (std::map<std::string, class Object*>::const_iterator i=mObjectMap.begin(); i!=mObjectMap.end(); ++i)
    {
        ObjectElementVisual3d *element = static_cast<ObjectElementVisual3d*>((*i).second->getElement(Object::FAMILY_VISUAL3D));
        if (element)
            element->setMapScroll(deltaX, deltaZ);
    }
}

//================================================================================================
//
//================================================================================================
Object *ObjectManager::getObject(std::string &name)
{
    PROFILE()
    if (name.empty())
        return mObjectAvatar;
    std::map<std::string, class Object*>::const_iterator i= mObjectMap.find(name);
    return (i== mObjectMap.end()) ?0:(*i).second;
}

//================================================================================================
// Event handling.
//================================================================================================
void ObjectManager::Event(std::string &name, int action, int id, int val1, int val2)
{
    PROFILE()
    // Get the object by name.
    Object *obj = getObject(name);
    if (!obj) return;
    if (action == EVT_CURSOR_TURN || action == EVT_CURSOR_WALK || action == EVT_SKINCOLOR)
    {
        // Only the avatar can be moved/turned by user input.
        if (obj != mObjectAvatar) return;
        ObjectElementVisual3d *element = static_cast<ObjectElementVisual3d*>(obj->getElement(Object::FAMILY_VISUAL3D));
        if (!element) return;
        if (action == EVT_CURSOR_TURN) { element->setTurn(id);      return; }
        if (action == EVT_CURSOR_WALK) { element->setMove(id);      return; }
        if (action == EVT_SKINCOLOR)   { element->setSkinColor(id); return; }
    }
    if (action == EVT_ANIMATION)
    {
        ObjectElementAnimate3d *element = static_cast<ObjectElementAnimate3d*>(obj->getElement(Object::FAMILY_ANIMATE3D));
        if (element)
            element->setAnimation(val1, val2);
        return;
    }
    /*
        if (action == EVT_GOTO)
        {
            TilePosOLD pos;
            pos.x = val1 & 0xff;
            pos.z = val1 >> 8;
            pos.subX = val2 & 0xff;
            pos.subZ = val2 >> 8;
            mvNPC[ObjectNPC::HERO]->moveToDistantTile(pos);
            return
        }
        if (action == EVT_HIT)
        {
            mvNPC[id]->setDamage(val1);
            return;
        }
    */
}
#include "gui/gui_manager.h"
#include <OgreMaterialManager.h>
//================================================================================================
// Highlight the given object.
//================================================================================================
void ObjectManager::highlightObject(MovableObject *mob)
{
    PROFILE()
    static Entity *highlightedMob = 0;
    if (mob == highlightedMob) return; // Object is already highlighted.
    // Switch off the highlighting on the current object.
    if (highlightedMob)
    {
        GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "No more highlighting");
/*
        for (unsigned int i = 0; i < highlightedMob->getNumSubEntities(); ++i)
            highlightedMob->getSubEntity(i)->setMaterialName(originalMaterial);
*/
        highlightedMob = 0;
    }
    // Switch on the highlighting.
    if (!mob /*|| mob->getName() == getAvatarName()*/) return;
    highlightedMob = (Entity*)mob;
    GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, mob->getName().c_str());
/*
    originalMaterial = highlightedMob->getSubEntity(0)->getMaterialName();
    MaterialPtr newMaterial = ((MaterialPtr)MaterialManager::getSingleton().getByName(originalMaterial))->clone(MATERIAL_HIGHLIGHT);
    newMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setColourOperationEx(LBX_MODULATE_X2, LBS_MANUAL, LBS_TEXTURE, ColourValue(1.0, 1.0, 1.0));
    for (unsigned int i = 0; i < highlightedMob->getNumSubEntities(); ++i)
        highlightedMob->getSubEntity(i)->setMaterialName(MATERIAL_HIGHLIGHT);
*/
}

//================================================================================================
// Select the (mouse clicked) object.
//================================================================================================
void ObjectManager::selectObject(MovableObject * /*mob*/)
{
    PROFILE()
    /*
        if (mvNPC[ObjectNPC::HERO]->isMoving()) return;
        if (mvNPC[ObjectNPC::HERO]->getHealth() <= 0) return;


        extractObject(mob);
        if  (mSelectedType >= OBJECT_NPC)
        {
            ObjectVisuals::getSingleton().select(mvNPC[mSelectedObject]->getEntity()->getBoundingBox(),
                                                 mvNPC[mSelectedObject]->getSceneNode(),
                                                 mvNPC[mSelectedObject]->getFriendly(),
                                                 mvNPC[mSelectedObject]->getHealthPercentage(),
                                                 mvNPC[mSelectedObject]->getNickName().c_str());
            mSelectedPos = mvNPC[mSelectedObject]->getTilePos();

     // mvNPC[mSelectedObject]->getFriendly();
     // String strSelect = "/target !"+ StringConverter::toString(mSelectedPos.x-9) + " " + StringConverter::toString(mSelectedPos.z-9);
     // Network::getSingleton().send_command(strSelect.c_str(), -1, Network::SC_NORMAL);

        }
        else
        {
            ObjectVisuals::getSingleton().select(mvStatic[mSelectedObject]->getEntity()->getBoundingBox(),
                                                 mvStatic[mSelectedObject]->getSceneNode(), 0, -1, 0);
            mSelectedPos = mvStatic[mSelectedObject]->getTilePos();
        }
    */
}

//================================================================================================
// Mouse button was pressed - lets do the right thing.
//================================================================================================
void ObjectManager::mousePressed(MovableObject * /*mob*/, bool /*modifier*/)
{
    PROFILE()
    // ////////////////////////////////////////////////////////////////////
    // An object was pressed.
    // ////////////////////////////////////////////////////////////////////
    /*
        extractObject(mob);
        if (mSelectedType == OBJECT_NPC)
        {
            if (mvNPC[mSelectedObject]->getFriendly() <0)
            {
                mSelectedPos = mvNPC[mSelectedObject]->getTilePos();
                ObjectVisuals::getSingleton().select(mvNPC[mSelectedObject]->getEntity()->getBoundingBox(),
                                                     mvNPC[mSelectedObject]->getSceneNode(),
                                                     mvNPC[mSelectedObject]->getFriendly(),
                                                     mvNPC[mSelectedObject]->getHealthPercentage(),
                                                     mvNPC[mSelectedObject]->getNickName().c_str());
                if (modifier)
                    mvNPC[ObjectNPC::HERO]->attackLongRange(mvNPC[mSelectedObject]);
                else
                    mvNPC[ObjectNPC::HERO]->attackShortRange(mvNPC[mSelectedObject]);
            }
            else
            {
                mvNPC[ObjectNPC::HERO]->readyPrimaryWeapon(false);
                ObjectVisuals::getSingleton().select(mvNPC[mSelectedObject]->getEntity()->getBoundingBox(),
                                                     mvNPC[mSelectedObject]->getSceneNode(),
                                                     mvNPC[mSelectedObject]->getFriendly(),
                                                     mvNPC[mSelectedObject]->getHealthPercentage(),
                                                     mvNPC[mSelectedObject]->getNickName().c_str());
                //GuiManager::getSingleton().print(GuiManager::LIST_CHATWIN, "talk hello");
                //String strSelect = "/target !"+ StringConverter::toString(mSelectedPos.x-9) + " " + StringConverter::toString(mSelectedPos.z-9);
                //Network::getSingleton().send_game_command(strSelect.c_str());
                //Network::getSingleton().send_game_command("/target 1");
                Network::getSingleton().send_game_command("talk hello");
            }
        }
        else if (mSelectedType < OBJECT_NPC)
        {
            //mSelectedPos = mvStatic[mSelectedObject]->getTilePos();
            //mvNPC[ObjectNPC::HERO]->moveToDistantTile(mSelectedPos, 2);
            mvStatic[mSelectedObject]->activate();
        }
    */
}

//================================================================================================
//
//================================================================================================
/*
void ObjectManager::shoot(int type, ObjectNPC *srcMob, ObjectNPC *dstMob)
{
    PROFILE()
    ObjectMissile *obj_missle = new ObjectMissile(type, srcMob, dstMob);
    if (obj_missle)  mvMissile.push_back(obj_missle);
}
*/
//================================================================================================
//
//================================================================================================
/*
void ObjectManager::targetObjectAttackNPC(int npcIndex)
{
    PROFILE()
    if (mSelectedObject <0 || mSelectedObject == npcIndex) return;
    mvNPC[mSelectedObject]->turning(mvNPC[npcIndex]->getFacing() -180, false);
    mvNPC[mSelectedObject]->attack();
}
*/

//================================================================================================
//
//================================================================================================
void ObjectManager::setEquipment(std::string &objName, int bone, int itemID, int particleID)
{
    PROFILE()
    Object *objNPC = getObject(objName);
    if (objNPC)
    {
        ObjectElementEquip3d *element = static_cast<ObjectElementEquip3d*>(objNPC->getElement(Object::FAMILY_EQUIP3D));
        if (element)
            element->equipItem(bone, itemID, particleID);
    }
}

//================================================================================================
//
//================================================================================================
void ObjectManager::setAnimation(std::string &objName, int animGroup, int animNr, bool loop, bool force, bool random, bool freezeLastFrame)
{
    PROFILE()
    Object *objNPC = getObject(objName);
    if (objNPC)
    {
        ObjectElementAnimate3d *element = static_cast<ObjectElementAnimate3d*>(objNPC->getElement(Object::FAMILY_ANIMATE3D));
        if (element)
            element->setAnimation(animGroup, animNr, loop, force, random, freezeLastFrame);
    }
}
