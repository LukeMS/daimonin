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
#include <OgreTechnique.h>
#include <OgreSceneManager.h>
#include <OgreRenderTexture.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreHardwarePixelBuffer.h>
#include "option.h"
#include "logger.h"
#include "profiler.h"
#include "events.h"
#include "sound.h"
#include "network.h"
#include "object/object_manager.h"
#include "object/object_element.h"
#include "object/object_element_animate3d.h"
#include "object/object_element_visual3d.h"
#include "object/object_element_equip3d.h"
#include "object/object_element_physical.h"

using namespace Ogre;

static const char MATERIAL_HIGHLIGHT[] = "MatObjectHighlight";

//================================================================================================
// Defines:
// * static: fixed to a single pos, does not have ai (stones, walls, trees, ...)
// * npc:    controlled by ai.
// * player: controlled by a human player.
//================================================================================================

//================================================================================================
// Init all static Elemnts.
//================================================================================================
// Prefix for the object name (S)tatic, (P)layer, (N)PC
const char *ObjectManager::ObjectID[OBJECT_SUM] = { "S","S","S","P","N" };

//================================================================================================
// Init the model from the description file.
//================================================================================================
void ObjectManager::init()
{
    PROFILE()
    Logger::log().headline() << "Init Object Managaer";
    mSelectedType  =-1;
    mSelectedObject=-1;
    // mSelectedEnemy = false;
    mAvatarName = "";
    mSceneManager = Events::getSingleton().getSceneManager();
}

//================================================================================================
//
//================================================================================================
const Ogre::Vector3 ObjectManager::getAvatarPos()
{
    ObjectElementVisual3d *element = static_cast<ObjectElementVisual3d*>(mObjectAvatar->getElement(Object::FAMILY_VISUAL3D));
    return element->getPosition();
}

//================================================================================================
//
//================================================================================================
void ObjectManager::addCreature(sObject &obj)
{
    Logger::log().headline() << "Add object:";
    Object *object = new Object();
    mObjectMap.insert(std::pair<std::string, class Object*>(obj.nickName, object));
    ObjectElementVisual3d *objV3d = new ObjectElementVisual3d(object, mSceneManager);
    Entity *entityWithSkeleton = objV3d->createEntity(obj.nickName, obj.meshName.c_str(), Ogre::RENDER_QUEUE_7, QUERY_MASK_NPC);
    if (entityWithSkeleton)
    {
        objV3d->setAnimationElement(new ObjectElementAnimate3d(object, entityWithSkeleton));
        ObjectElementEquip3d *objEquip = new ObjectElementEquip3d(object, entityWithSkeleton);
        objEquip->equipItem(ObjectElementEquip3d::BONE_WEAPON_HAND, ObjectElementEquip3d::ITEM_WEAPON, 0, -1);  // Just for test (Sword)
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
    /*
        mReadyWeaponStatus = 0;
        mType    = obj.type;
        mFriendly= obj.friendly;
        mAttack  = obj.attack;
        mDefend  = obj.defend;
        mMaxHP   = obj.maxHP;
        mActHP   = obj.maxHP;//
        mMaxMana = obj.maxMana;
        mActMana = obj.maxMana;
        mMaxGrace= obj.maxGrace;
        mActGrace= obj.maxGrace;
        mBoundingRadius = obj.boundingRadius;

        // ////////////////////////////////////////////////////////////////////
        // Only players can change equipment.
        // ////////////////////////////////////////////////////////////////////
        if (mType == ObjectManager::OBJECT_PLAYER)
        {
            mEquip = new ObjectElementVisual3d(mEntity);
            mEquip->equipItem(ObjectElementVisual3d::BONE_WEAPON_HAND, ObjectElementVisual3d::ITEM_WEAPON, 0, -1);  // Just for test (Sword)
            //mEquip->equipItem(ObjectElementVisual3d::BONE_SHIELD_HAND, ObjectEquipment::ITEM_WEAPON, 2, -1);  // Just for test (Bow)
            //mEquip->equipItem(ObjectElementVisual3d::BONE_WEAPON_HAND, ObjectElementVisual3d::ITEM_WEAPON, 0, 0);  // Just for test (Fire Sword)
            //mEquip->equipItem(ObjectElementVisual3d::BONE_WEAPON_HAND, ObjectElementVisual3d::ITEM_WEAPON, 0, -1);  // Just for test (Sword)
        }

        // ////////////////////////////////////////////////////////////////////
        // Attach the blob shadow to the npc.
        // ////////////////////////////////////////////////////////////////////
        ManualObject* blob = static_cast<ManualObject*>(Events::getSingleton().getSceneManager()->createMovableObject("Mob_"+ StringConverter::toString(mIndex, 10, '0'), ManualObjectFactory::FACTORY_TYPE_NAME));
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
        blob->convertToMesh("Blob_"+ StringConverter::toString(mIndex, 10, '0'));
        blob->setQueryFlags(0);
        blob->setRenderQueueGroup(RENDER_QUEUE_6); // see OgreRenderObjectElementVisual3dQueue.h
        mNode->attachObject(blob);

        setSkinColor(0);
        mCursorTurning =0;
        mCursorWalking =0;
        mAutoTurning = TURN_NONE;
        mAutoMoving = false;
        mEnemyObject = 0;
        mAttacking = ATTACK_NONE;
        // mNode->showBoundingBox(true); // Remove Me!!!!
        mOffX =0;
        mOffZ =0;
    */
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
Object* ObjectManager::getObject(std::string &name)
{
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
        ObjectElementAnimate3d *element = static_cast<ObjectElementAnimate3d*>(obj->getElement(Object::FAMILY_ANIMATION3D));
        if (element)
            element->toggleAnimation(val1, val2);
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

//================================================================================================
// Highlight the object.
//================================================================================================
void ObjectManager::highlightObject(MovableObject *mob, bool highlight)
{
    PROFILE()
    /*
        static String strMaterialBak;
        static Entity *entity = 0;
        if (highlight)
        {
            if (!mob) return;
            extractObject(mob);
            if  (mSelectedType >= OBJECT_NPC)
            {
                // If it crashes here, then there is an object without setQueryFlags(0).
                if (mSelectedObject < 0)
                {
                    Logger::log().error() << Logger::ICON_CLIENT << "An object without a defined query flag was found!";
                    Logger::log().error() << Logger::ICON_CLIENT << "Use setQueryFlags(0) on all objects that needs no mouse picking.";
                    return;
                }
                if (entity || mSelectedObject == ObjectNPC::HERO) return;
                entity = mvNPC[mSelectedObject]->getEntity();
                ObjectVisuals::getSingleton().highlight(false, mvNPC[mSelectedObject]->getFriendly(), true);
            }
            else
            {
                if (entity) return;
                entity = mvStatic[mSelectedObject]->getEntity();
                ObjectVisuals::getSingleton().highlight(true, 0, true);
            }
            // Set the highlighted material for the model.
            strMaterialBak = entity->getSubEntity(0)->getMaterialName();
            MaterialPtr orgMaterial = MaterialManager::getSingleton().getByName(strMaterialBak);
            MaterialPtr newMaterial = orgMaterial->clone(MATERIAL_HIGHLIGHT);
            newMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setColourOperationEx(LBX_MODULATE_X2, LBS_MANUAL, LBS_TEXTURE, ColourValue(1.0, 1.0, 1.0));
            for (unsigned int i = 0; i < entity->getNumSubEntities(); ++i)
                entity->getSubEntity(i)->setMaterialName(MATERIAL_HIGHLIGHT);
            return;
        }
        if (!entity) return;
        for (unsigned int i = 0; i < entity->getNumSubEntities(); ++i)
            entity->getSubEntity(i)->setMaterialName(strMaterialBak);
        entity =0;
        MaterialManager::getSingleton().remove(MATERIAL_HIGHLIGHT);
        ObjectVisuals::getSingleton().highlight(false, 0, false);
    */
}

//================================================================================================
// Select the (mouse clicked) object.
//================================================================================================
void ObjectManager::selectObject(MovableObject *mob)
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
void ObjectManager::mousePressed(MovableObject *mob, bool modifier)
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
// Extract ObjectType and ObjectNr out of the entity name.
//================================================================================================
void ObjectManager::extractObject(MovableObject *mob)
{
    PROFILE()
    /*
        String strObject = mob->getName();
        for (mSelectedType=0; mSelectedType < OBJECT_SUM; ++mSelectedType)
        {
            if (strObject[0] == *ObjectID[mSelectedType])
            {
                mSelectedObject = StringConverter::parseInt(strObject.substr(strObject.find("_")+1, strObject.size()));
                // ////////////////////////////////////////////////////////////////////
                // NPC or playe object.
                // ////////////////////////////////////////////////////////////////////
                if  (mSelectedType >= OBJECT_NPC)
                {
                    int sel =0;
                    for (std::vector<ObjectNPC*>::iterator i = mvNPC.begin(); i < mvNPC.end(); ++i, ++sel)
                    {
                        if ((int)(*i)->getIndex() == mSelectedObject)
                        {
                            mSelectedObject = sel;
                            return;
                        }
                    }
                }
                // ////////////////////////////////////////////////////////////////////
                // Static object.
                // ////////////////////////////////////////////////////////////////////
                else
                {
                    int sel =0;
                    for (std::vector<ObjectStatic*>::iterator i = mvStatic.begin(); i < mvStatic.end(); ++i, ++sel)
                    {
                        if ((int)(*i)->getIndex() == mSelectedObject)
                        {
                            mSelectedObject = sel;
                            return;
                        }
                    }
                }
            }
        }
        Logger::log().error() << Logger::ICON_CLIENT << "Bug in ObjectManager::extractObject(...) : Could not extract object!";
    */
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
void ObjectManager::setEquipment(int npcID, int bone, int type, int itemID)
{
    PROFILE()
//    if (mvNPC[npcID]->mEquip)
//        mvNPC[npcID]->mEquip->equipItem(bone, type, itemID);
}

//================================================================================================
//
//================================================================================================
ObjectManager::~ObjectManager()
{
    PROFILE()
}

//================================================================================================
// Create a 2d Animation from a model.
//================================================================================================
bool ObjectManager::createFlipBook(String meshName, int sumRotations)
{
    PROFILE()
    Entity *entity;
    try
    {
        entity  = Events::getSingleton().getSceneManager()->createEntity("FlipBookEntity", meshName);
    }
    catch (...)
    {
        LogManager::getSingleton().setLogDetail(LL_LOW);
        std::cout << "\n\n\n  Wrong commandline argument!\n  No mesh called '" << meshName << "' could be found.\n";
        std::cout << "  Hint: Mesh-names are case sensitive." << std::endl;
        return false;
    }
    const AxisAlignedBox &AABB = entity->getBoundingBox();
    Real entityRadius = (AABB.getMaximum() - AABB.getCenter()).length();

    SceneNode *node = Events::getSingleton().getSceneManager()->getRootSceneNode()->createChildSceneNode("FlipBookNode");
    node->attachObject(entity);
    node->setPosition(-AABB.getCenter());

    const int textureSize = 1024/sumRotations;
    TexturePtr texture = TextureManager::getSingleton().createManual(
                             "FlipBookTexture", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                             TEX_TYPE_2D, textureSize * sumRotations, textureSize, 0, PF_A8R8G8B8, TU_RENDERTARGET);
    RenderTexture *renderTarget = texture->getBuffer()->getRenderTarget();
    renderTarget->setAutoUpdated(false);

    Camera *camera = Events::getSingleton().getSceneManager()->createCamera("tmpCamera");
    camera->setLodBias(1000.0f);
    camera->setPosition(0, 0, entityRadius + 1.5f);
    camera->setAspectRatio(1.0f);
    //camera->setProjectionType(PT_ORTHOGRAPHIC);
    //camera->setFOVy(Degree(200));
    camera->setFOVy(Math::ATan(2.0f * entityRadius));
    camera->setNearClipDistance(1.0f);
    camera->setFarClipDistance(entityRadius*2);

    Viewport *viewport = renderTarget->addViewport(camera);
    viewport->setOverlaysEnabled(false);
    viewport->setClearEveryFrame(true);
    viewport->setBackgroundColour(ColourValue(1.0f, 1.0f, 1.0f, 0.0f));

    // Render only the queues in the special case list.
    Events::getSingleton().getSceneManager()->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE);
    Events::getSingleton().getSceneManager()->addSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    entity->setRenderQueueGroup(RENDER_QUEUE_MAIN);
    const float divFactor = 1.0f / sumRotations;
    for (int i = 0; i < sumRotations; ++i)
    {
        viewport->setDimensions(divFactor*i, 0, divFactor, 1);
        renderTarget->update();
        node->yaw(Degree(180.0f/(sumRotations-1)));
    }
    renderTarget->writeContentsToFile("Animation2d_"+ meshName + ".png");
    Events::getSingleton().getSceneManager()->removeSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    //Render all except the queues in the special case list.
    Events::getSingleton().getSceneManager()->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
    // ////////////////////////////////////////////////////////////////////
    // Cleanup.
    // ////////////////////////////////////////////////////////////////////
    renderTarget->removeViewport(0);
    node->detachAllObjects();
    Events::getSingleton().getSceneManager()->destroyCamera(camera);
    Events::getSingleton().getSceneManager()->destroyEntity(entity);
    Events::getSingleton().getSceneManager()->destroySceneNode("FlipBookNode");
    TextureManager::getSingleton().remove("FlipBookTexture");
    texture.setNull();
    return true;
}
