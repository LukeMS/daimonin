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

#include "option.h"
#include "logger.h"
#include "events.h"
#include "sound.h"
#include "network.h"
#include "object_manager.h"
#include "object_visuals.h"


//================================================================================================
// Defines:
// * static: fixed to a single pos, does not have ai (stones, walls, trees, ...)
// * npc:    controlled by ai.
// * player: controlled by a human player.
//================================================================================================



//================================================================================================
// Init all static Elemnts.
//================================================================================================
char *ObjectManager::ObjectID[OBJECT_SUM] = { "S","S","P","N" };

//================================================================================================
// Init the model from the description file.
//================================================================================================
bool ObjectManager::init()
{
    string strType, strTemp, strMesh, strNick;
    mSelectedType  =-1;
    mSelectedObject=-1;
    //    mSelectedEnemy = false;
    int i=0;
    // Default values.

    while (1)
    {
        if (!(Option::getSingleton().openDescFile(FILE_WORLD_DESC)))
        {
            Logger::log().info() << "Parse description file " << FILE_WORLD_DESC << ".";
            return false;
        }

        if (!(Option::getSingleton().getDescStr("Type", strType, ++i)))
            break;
        ObjectStatic::sObject obj;
        Option::getSingleton().getDescStr("MeshName", obj.meshName,i);
        Option::getSingleton().getDescStr("NickName", obj.nickName,i);

        Option::getSingleton().getDescStr("BoundingRadius", strTemp,i);
        obj.boundingRadius = (unsigned char) StringConverter::parseInt(strTemp);

        if (Option::getSingleton().getDescStr("WalkableRow0", strTemp,i))
            obj.walkable[0] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[0] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow1", strTemp,i))
            obj.walkable[1] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[1] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow2", strTemp,i))
            obj.walkable[2] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[2] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow3", strTemp,i))
            obj.walkable[3] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[3] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow4", strTemp,i))
            obj.walkable[4] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[4] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow5", strTemp,i))
            obj.walkable[5] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[5] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow6", strTemp,i))
            obj.walkable[6] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[6] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow7", strTemp,i))
            obj.walkable[7] = (unsigned char) StringConverter::parseInt(strTemp);
        else
            obj.walkable[7] = 0;

        Option::getSingleton().getDescStr("Friendly", strTemp,i);
        obj.friendly= StringConverter::parseInt(strTemp);

        Option::getSingleton().getDescStr("Attack", strTemp,i);
        obj.attack  = StringConverter::parseInt(strTemp);

        Option::getSingleton().getDescStr("Defend", strTemp,i);
        obj.defend  = StringConverter::parseInt(strTemp);

        Option::getSingleton().getDescStr("MaxHP", strTemp,i);
        obj.maxHP  = StringConverter::parseInt(strTemp);

        Option::getSingleton().getDescStr("MaxMana", strTemp,i);
        obj.maxMana  = StringConverter::parseInt(strTemp);

        Option::getSingleton().getDescStr("MaxGrace", strTemp,i);
        obj.maxGrace  = StringConverter::parseInt(strTemp);

        Option::getSingleton().getDescStr("PosX", strTemp,i);
        obj.pos.x  = StringConverter::parseInt(strTemp);
        Option::getSingleton().getDescStr("PosY", strTemp,i);
        obj.pos.z  = StringConverter::parseInt(strTemp);
        if (Option::getSingleton().getDescStr("PosSubX", strTemp,i))
            obj.pos.subX  = StringConverter::parseInt(strTemp);
        else
            obj.pos.subX  = 3;
        if (Option::getSingleton().getDescStr("PosSubY", strTemp,i))
            obj.pos.subZ  = StringConverter::parseInt(strTemp);
        else
            obj.pos.subZ  = 3;

        if (Option::getSingleton().getDescStr("Level", strTemp,i))
            obj.level= StringConverter::parseInt(strTemp);
        else
            obj.level= 0;

        Option::getSingleton().getDescStr("Facing", strTemp,i);
        obj.facing= StringConverter::parseReal(strTemp);

        Option::getSingleton().getDescStr("Particles", strTemp,i);
        obj.particleNr  = StringConverter::parseInt(strTemp);

        if (strType == "player")
        {
            obj.type = OBJECT_PLAYER;
            addMobileObject(obj);
        }
        else if (strType == "npc")
        {
            obj.type = OBJECT_NPC;
            addMobileObject(obj);
        }
        else if (strType == "static")
        {
            obj.type = OBJECT_CONTAINER;
            addMobileObject(obj);
        }
    }
    return true;
}

//================================================================================================
// Adds a independant object.
//================================================================================================
void ObjectManager::addMobileObject(ObjectStatic::sObject &obj)
{
    if (obj.type < OBJECT_NPC)
    {
        static unsigned int index=0;
        obj.index = index++;
        ObjectStatic *obj_static = new ObjectStatic(obj);
        if (obj_static)
            mvStatic.push_back(obj_static);
    }
    else
    {
        static unsigned int index=0;
        obj.index = index++;
        ObjectNPC *obj_npc = new ObjectNPC(obj, true);
        if (obj_npc)
            mvNPC.push_back(obj_npc);
    }
}

//================================================================================================
// Update all objects.
//================================================================================================
void ObjectManager::update(int obj_type, const FrameEvent& evt)
{
    for (unsigned int i = 0; i < mvMissle.size(); ++i)
    {
        if (!mvMissle[i]->update(evt))
            deleteMissle(i);
    }
    for (unsigned int i = 0; i < mvStatic.size(); ++i)
    {
        if (!mvStatic [i]->update(evt) )
            deleteNPC(i);
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
const Vector3 &ObjectManager::synchToWorldPos(int deltaX, int deltaZ)
{
    static Vector3 pos;
    for (unsigned int i = 0; i < mvStatic.size(); ++i)
    {
        mvStatic[i]->movePosition(deltaX, deltaZ);
    }
    pos = mvNPC[0]->getPosition();
    for (unsigned int i = 0; i < mvNPC.size(); ++i)
    {
        // Sync the actual position.
        mvNPC[i]->movePosition(deltaX, deltaZ);
    }
    pos-= mvNPC[0]->getPosition();
    return pos;
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
            TilePos pos;
            pos.x = val1 & 0xff;
            pos.z = val1 >> 8;
            pos.subX = val2 & 0xff;
            pos.subZ = val2 >> 8;
            mvNPC[ObjectNPC::HERO]->moveToDistantTile(pos);
        }
        if (action == OBJ_TEXTURE    )
            mvNPC[id]->mEquip->setTexture(val1, val2);
        if (action == OBJ_HIT        )
            mvNPC[id]->setDamage(val1);
        if (action == OBJ_TURN       )
            mvNPC[id]->turning(val1, false);
        if (action == OBJ_CURSOR_TURN)
            mvNPC[id]->turning(val1, true);
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
// Delete a Static-Object.
//================================================================================================
void ObjectManager::deleteMissle(int index)
{
    mvMissle[index]->freeRecources();
    delete mvMissle[index];
    std::vector<ObjectMissle*>::iterator i = mvMissle.begin();
    while (index--) ++i;
    mvMissle.erase(i);
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
}

//================================================================================================
// Highlight the object.
//================================================================================================
void ObjectManager::highlightObject(MovableObject *mob)
{
    if (!mob) return;
    extractObject(mob);
    if  (mSelectedType >= OBJECT_NPC)
    {
        if (mSelectedObject != ObjectNPC::HERO)
            ObjectVisuals::getSingleton().highlight(mvNPC[mSelectedObject],
                                                    mSelectedObject != ObjectNPC::HERO,
                                                    Events::getSingleton().isShiftDown());
    }
    else
        ObjectVisuals::getSingleton().highlight(mvStatic[mSelectedObject],
                                                true,
                                                Events::getSingleton().isShiftDown());
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
        bool notHero = mSelectedObject != ObjectNPC::HERO;
        ObjectVisuals::getSingleton().select(mvNPC[mSelectedObject], notHero, notHero);
        mSelectedPos = mvNPC[mSelectedObject]->getTilePos();
        mSelectedFriendly = mvNPC[mSelectedObject]->getFriendly();
        String strSelect = "/target !"+ StringConverter::toString(mSelectedPos.x-9) + " " + StringConverter::toString(mSelectedPos.z-9);
        Network::getSingleton().send_command(strSelect.c_str(), -1, Network::SC_NORMAL);
    }
    else
    {
        ObjectVisuals::getSingleton().select(mvStatic[mSelectedObject], false);
        mSelectedPos = mvStatic[mSelectedObject]->getTilePos();
    }
}

//================================================================================================
// Mouse button was pressed - lets do the right thing.
//================================================================================================
void ObjectManager::mousePressed(MovableObject *mob, TilePos pos, bool modifier)
{
    // ////////////////////////////////////////////////////////////////////
    // Only a tile was pressed - move toward it.
    // ////////////////////////////////////////////////////////////////////
    if (!mob)
    {
        mvNPC[ObjectNPC::HERO]->moveToDistantTile(pos);
        ObjectVisuals::getSingleton().unselect();
        return;
    }
    // ////////////////////////////////////////////////////////////////////
    // An object was pressed.
    // ////////////////////////////////////////////////////////////////////
    extractObject(mob);
    if (mSelectedType == OBJECT_NPC)
    {
        if (mvNPC[mSelectedObject]->getFriendly() <0)
        {
            mSelectedPos = mvNPC[mSelectedObject]->getTilePos();
            ObjectVisuals::getSingleton().select(mvNPC[mSelectedObject], true, false);
            if (modifier)
                mvNPC[ObjectNPC::HERO]->attackLongRange(mvNPC[mSelectedObject]);
            else
                mvNPC[ObjectNPC::HERO]->attackShortRange(mvNPC[mSelectedObject]);
        }
        else
        {
            mvNPC[ObjectNPC::HERO]->readyPrimaryWeapon(false);
            ObjectVisuals::getSingleton().select(mvNPC[mSelectedObject], false, false);
            String strSelect = "/target !"+ StringConverter::toString(mSelectedPos.x-9) + " " + StringConverter::toString(mSelectedPos.z-9);
            Network::getSingleton().send_command(strSelect.c_str(), -1, Network::SC_NORMAL);
            Network::getSingleton().send_command("/talk hello", -1, Network::SC_NORMAL);
        }
    }
    else if (mSelectedType < OBJECT_NPC)
    {
        mSelectedPos = mvStatic[mSelectedObject]->getTilePos();
        mvNPC[ObjectNPC::HERO]->moveToDistantTile(mSelectedPos, 2);
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
    ObjectMissle *obj_missle = new ObjectMissle(type, srcMob, dstMob);
    if (obj_missle)  mvMissle.push_back(obj_missle);
}

//================================================================================================
//
//================================================================================================
void ObjectManager::targetObjectFacingNPC(int npcIndex)
{
    mvNPC[mSelectedObject]->turning(mvNPC[npcIndex]->getFacing() -180, false);
}

//================================================================================================
//
//================================================================================================
void ObjectManager::targetObjectAttackNPC(int npcIndex)
{
    if (mSelectedObject <0) return;
    targetObjectFacingNPC(npcIndex);
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
        cout << "\n\n\n  Wrong commandline argument!\n  No mesh called '" << meshName << "' could be found.\n";
        cout << "  Hint: Mesh-names are case sensitive." << endl;
        return false;
    }
    const AxisAlignedBox &AABB = entity->getBoundingBox();
    Real entityRadius = (AABB.getMaximum() - AABB.getCenter()).length();

    SceneNode *node = Events::getSingleton().GetSceneManager()->getRootSceneNode()->createChildSceneNode("FlipBookNode");
    node->attachObject(entity);
    node->setPosition(-AABB.getCenter());

    const int textureSize = 1024/sumRotations;
    TexturePtr texture = TextureManager::getSingleton().createManual(
                             "FlipBookTexture", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                             TEX_TYPE_2D, textureSize * sumRotations, textureSize, 0, PF_A8R8G8B8, TU_RENDERTARGET);
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
