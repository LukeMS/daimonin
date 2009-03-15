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

#include "option.h"
#include "logger.h"
#include "events.h"
#include "sound.h"
#include "network.h"
#include "object_manager.h"
#include "object_visuals.h"
#include "resourceloader.h"

using namespace Ogre;

const char MATERIAL_HIGHLIGHT[] = "MatObjectHighlight";

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
    mSelectedType  =-1;
    mSelectedObject=-1;
    // mSelectedEnemy = false;
}

//================================================================================================
// Adds a independant object.
//================================================================================================
void ObjectManager::addMobileObject(ObjectStatic::sObject &obj)
{
    if (obj.type < OBJECT_NPC)
        mvStatic.push_back(new ObjectStatic(obj));
    else
        mvNPC.push_back(new ObjectNPC(obj, true));
}

//================================================================================================
// Update all objects.
//================================================================================================
void ObjectManager::update(int obj_type, const FrameEvent& evt)
{
    for (unsigned int i = 0; i < mvMissile.size(); ++i)
    {
        if (!mvMissile[i]->update(evt))
            deleteMissile(i);
    }
    for (unsigned int i = 0; i < mvStatic.size(); ++i)
    {
        if (!mvStatic [i]->update(evt))
            deleteStatic(i);
    }
    for (unsigned int i = 0; i < mvNPC.size(); ++i)
    {
        if (!mvNPC[i]->update(evt))
            deleteNPC(i);
    }
}

//================================================================================================
// Update all object positions after a map scroll.
//================================================================================================
void ObjectManager::synchToWorldPos(int deltaX, int deltaZ)
{
    for (unsigned int i = 0; i < mvStatic.size(); ++i)
    {
        if (!mvStatic[i]->movePosition(deltaX, deltaZ))
            deleteStatic(i);
    }
    for (unsigned int i = 1; i < mvNPC.size(); ++i)
    {
        // Sync the actual position.
        if (!mvNPC[i]->movePosition(deltaX, deltaZ))
            deleteNPC(i);
    }
}

//================================================================================================
// Event handling.
//================================================================================================
void ObjectManager::Event(int obj_type, int action, int id, int val1, int val2)
{
    if (obj_type < OBJECT_NPC)
    {
        ;
    }
    else
    {
        if (id >= (int) mvNPC.size()) return;
        // if (action == OBJ_WALK) mvNPC[id]->walking(val1);
        if (action == OBJ_GOTO)
        {
            /*
            TilePosOLD pos;
            pos.x = val1 & 0xff;
            pos.z = val1 >> 8;
            pos.subX = val2 & 0xff;
            pos.subZ = val2 >> 8;
            mvNPC[ObjectNPC::HERO]->moveToDistantTile(pos);
            */
        }
        if (action == OBJ_TEXTURE    )
            mvNPC[id]->mEquip->setTexture(val1, val2);
        if (action == OBJ_HIT        )
            mvNPC[id]->setDamage(val1);
        if (action == OBJ_TURN       )
            mvNPC[id]->turning(val1, false);
        if (action == OBJ_CURSOR_TURN)
            mvNPC[id]->turning(val1, true);
        if (action == OBJ_CURSOR_WALK)
            mvNPC[id]->walking(val1, true);
        if (action == OBJ_ANIMATION  )
            mvNPC[id]->toggleAnimation(val1, val2);
    }
}

//================================================================================================
// Delete a NPC-Object.
//================================================================================================
void ObjectManager::deleteNPC(int index)
{
    if (mSelectedObject == index) mSelectedObject =-1;
    else if (mSelectedObject > index) --mSelectedObject;
    mvNPC[index]->freeRecources();
    delete mvNPC[index];
    std::vector<ObjectNPC*>::iterator i = mvNPC.begin();
    while (index--) ++i;
    mvNPC.erase(i);
}

//================================================================================================
// Delete a Static-Object.
//================================================================================================
void ObjectManager::deleteStatic(int index)
{
    mvStatic[index]->freeRecources();
    delete mvStatic[index];
    std::vector<ObjectStatic*>::iterator i = mvStatic.begin();
    while (index--) ++i;
    mvStatic.erase(i);
}

//================================================================================================
// Delete a Missile-Object.
//================================================================================================
void ObjectManager::deleteMissile(int index)
{
    mvMissile[index]->freeRecources();
    delete mvMissile[index];
    std::vector<ObjectMissile*>::iterator i = mvMissile.begin();
    while (index--) ++i;
    mvMissile.erase(i);
}

//================================================================================================
//
//================================================================================================
void ObjectManager::freeRecources()
{
    for (std::vector<ObjectNPC*>::iterator i = mvNPC.begin(); i < mvNPC.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvNPC.clear();

    for (std::vector<ObjectStatic*>::iterator i = mvStatic.begin(); i < mvStatic.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvStatic.clear();

    for (std::vector<ObjectMissile*>::iterator i = mvMissile.begin(); i < mvMissile.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvMissile.clear();
}

//================================================================================================
// Highlight the object.
//================================================================================================
void ObjectManager::highlightObject(MovableObject *mob, bool highlight)
{
    static String strMaterialBak;
    static Entity *entity = 0;
    if (highlight)
    {
        if (!mob) return;
        extractObject(mob);
        if  (mSelectedType >= OBJECT_NPC)
        {
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
}

//================================================================================================
// Select the (mouse clicked) object.
//================================================================================================
void ObjectManager::selectObject(MovableObject *mob)
{
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
        /*
                mvNPC[mSelectedObject]->getFriendly();
                String strSelect = "/target !"+ StringConverter::toString(mSelectedPos.x-9) + " " + StringConverter::toString(mSelectedPos.z-9);
                Network::getSingleton().send_command(strSelect.c_str(), -1, Network::SC_NORMAL);
        */
    }
    else
    {
        ObjectVisuals::getSingleton().select(mvStatic[mSelectedObject]->getEntity()->getBoundingBox(),
                                             mvStatic[mSelectedObject]->getSceneNode(), 0, -1, 0);
        mSelectedPos = mvStatic[mSelectedObject]->getTilePos();
    }
}

//================================================================================================
// Mouse button was pressed - lets do the right thing.
//================================================================================================
void ObjectManager::mousePressed(MovableObject *mob, bool modifier)
{
    // ////////////////////////////////////////////////////////////////////
    // An object was pressed.
    // ////////////////////////////////////////////////////////////////////
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
            //GuiManager::getSingleton().sendMsg(GuiManager::GUI_LIST_CHATWIN, GuiManager::MSG_ADD_ROW, "talk hello");
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
}

//================================================================================================
// Extract ObjectType and ObjectNr out of the entity name.
//================================================================================================
void ObjectManager::extractObject(MovableObject *mob)
{
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
    Logger::log().error() << "Bug in ObjectManager::extractObject(...) : Could not extract object!";
}

//================================================================================================
//
//================================================================================================
void ObjectManager::shoot(int type, ObjectNPC *srcMob, ObjectNPC *dstMob)
{
    ObjectMissile *obj_missle = new ObjectMissile(type, srcMob, dstMob);
    if (obj_missle)  mvMissile.push_back(obj_missle);
}

//================================================================================================
//
//================================================================================================
void ObjectManager::targetObjectAttackNPC(int npcIndex)
{
    if (mSelectedObject <0 || mSelectedObject == npcIndex) return;
    mvNPC[mSelectedObject]->turning(mvNPC[npcIndex]->getFacing() -180, false);
    mvNPC[mSelectedObject]->attack();
}

//================================================================================================
//
//================================================================================================
void ObjectManager::setEquipment(int npcID, int bone, int type, int itemID)
{
    if (mvNPC[npcID]->mEquip)
        mvNPC[npcID]->mEquip->equipItem(bone, type, itemID);
}

//================================================================================================
//
//================================================================================================
ObjectManager::~ObjectManager()
{}

//================================================================================================
// Create a 2d Animation from a model.
//================================================================================================
bool ObjectManager::createFlipBook(String meshName, int sumRotations)
{
    Entity *entity;
    try
    {
        entity  = Events::getSingleton().GetSceneManager()->createEntity("FlipBookEntity", meshName);
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

    SceneNode *node = Events::getSingleton().GetSceneManager()->getRootSceneNode()->createChildSceneNode("FlipBookNode");
    node->attachObject(entity);
    node->setPosition(-AABB.getCenter());

    const int textureSize = 1024/sumRotations;
    TexturePtr texture = TextureManager::getSingleton().createManual(
                             ManResourceLoader::TEMP_RESOURCE + "FlipBookTexture", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                             TEX_TYPE_2D, textureSize * sumRotations, textureSize, 0, PF_A8R8G8B8, TU_RENDERTARGET,
                             ManResourceLoader::getSingleton().getLoader());
    RenderTexture *renderTarget = texture->getBuffer()->getRenderTarget();
    renderTarget->setAutoUpdated(false);

    Camera *camera = Events::getSingleton().GetSceneManager()->createCamera("tmpCamera");
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
    Events::getSingleton().GetSceneManager()->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE);
    Events::getSingleton().GetSceneManager()->addSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    entity->setRenderQueueGroup(RENDER_QUEUE_MAIN);
    const float divFactor = 1.0f / sumRotations;
    for (int i = 0; i < sumRotations; ++i)
    {
        viewport->setDimensions(divFactor*i, 0, divFactor, 1);
        renderTarget->update();
        node->yaw(Degree(180.0f/(sumRotations-1)));
    }
    renderTarget->writeContentsToFile("Animation2d_"+ meshName + ".png");
    Events::getSingleton().GetSceneManager()->removeSpecialCaseRenderQueue(RENDER_QUEUE_MAIN);
    //Render all except the queues in the special case list.
    Events::getSingleton().GetSceneManager()->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
    // ////////////////////////////////////////////////////////////////////
    // Cleanup.
    // ////////////////////////////////////////////////////////////////////
    renderTarget->removeViewport(0);
    node->detachAllObjects();
    Events::getSingleton().GetSceneManager()->destroyCamera(camera);
    Events::getSingleton().GetSceneManager()->destroyEntity(entity);
    Events::getSingleton().GetSceneManager()->destroySceneNode("FlipBookNode");
    TextureManager::getSingleton().remove("FlipBookTexture");
    texture.setNull();
    return true;
}
