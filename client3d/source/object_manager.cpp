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

#include "define.h"
#include "option.h"
#include "logger.h"
#include "events.h"
#include "sound.h"
#include "spell_manager.h"
#include "object_manager.h"
#include "particle_manager.h"
#include "object_visuals.h"
#include "gui_manager.h"

//================================================================================================
// Defines:
// * static: fixed to a single pos, does not have ai (stones, walls, trees, ...)
// * npc:    controlled by ai.
// * player: controlled by a human player.
//================================================================================================


//================================================================================================
// Init all static Elemnts.
//================================================================================================


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

        if (!(Option::getSingleton().getDescStr("Type", strType, ++i))) break;
        sObject obj;
        Option::getSingleton().getDescStr("MeshName", obj.meshName,i);
        Option::getSingleton().getDescStr("NickName", obj.nickName,i);

        Option::getSingleton().getDescStr("BoundingRadius", strTemp,i);
        obj.boundingRadius = (unsigned char) StringConverter::parseInt(strTemp);

        if (Option::getSingleton().getDescStr("WalkableRow0", strTemp,i))
            obj.walkable[0] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[0] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow1", strTemp,i))
            obj.walkable[1] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[1] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow2", strTemp,i))
            obj.walkable[2] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[2] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow3", strTemp,i))
            obj.walkable[3] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[3] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow4", strTemp,i))
            obj.walkable[4] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[4] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow5", strTemp,i))
            obj.walkable[5] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[5] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow6", strTemp,i))
            obj.walkable[6] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[6] = 0;
        if (Option::getSingleton().getDescStr("WalkableRow7", strTemp,i))
            obj.walkable[7] = (unsigned char) StringConverter::parseInt(strTemp);
        else obj.walkable[7] = 0;

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
        /*
                // just for testing.
                else if (strType == "static")
                {

                    for (obj.posX =0; obj.posX < 11; ++obj.posX)
                        for (obj.posY =0; obj.posY < 23; ++obj.posY)
                        {
                            obj.type = OBJECT_STATIC;
                            addMobileObject(obj);
                        }
                }
        */
        else if (strType == "static")
        {
            obj.type = OBJECT_STATIC;
            addMobileObject(obj);
        }
    }
    return true;
}

//================================================================================================
// Adds a independant object.
//================================================================================================
void ObjectManager::addMobileObject(sObject &obj)
{
    switch (obj.type)
    {
        case OBJECT_STATIC:  // todo: branch this out.
        {
            static unsigned int index=0;
            obj.index = index++;
            ObjectStatic *obj_static = new ObjectStatic(obj);
            if (!obj_static) return;
            mvObject_static.push_back(obj_static);
            break;
        }
        case OBJECT_PLAYER:
        case OBJECT_NPC:
        {
            static unsigned int index=0;
            obj.index = index++;
            ObjectNPC *obj_npc = new ObjectNPC(obj, true);
            if (!obj_npc) return;
            mvObject_npc.push_back(obj_npc);
            break;
        }

        default:
        {
            Logger::log().error() << "Unknow mobile object-type in mesh " << obj.meshName;
            return;
        }
    }
}

//================================================================================================
// Update all objects.
//================================================================================================
void ObjectManager::update(int obj_type, const FrameEvent& evt)
{
    for (unsigned int i = 0; i < mvObject_static.size(); ++i)
    {
        mvObject_static [i]->update(evt);
    }
    for (unsigned int i = 0; i < mvObject_npc.size(); ++i)
    {
        if (!mvObject_npc[i]->update(evt)) delObjectNPC(i);
    }

    /*
      switch (obj_type)
      {
          case OBJECT_STATIC:
          {
            for (unsigned int i = 0; i < mvObject_static.size(); ++i)
            {
              mvObject_static [i]->update(evt);
            }
            break;
          }
          case OBJECT_PLAYER:
          case OBJECT_NPC:
          {
            for (unsigned int i = 0; i < mvObject_npc.size(); ++i)
            {
              mvObject_npc[i]->update(evt);
            }
            break;
          }
          default:
          break;
      }
    */
}

//================================================================================================
// Update all object positions after a map scroll.
//================================================================================================
const Vector3 &ObjectManager::synchToWorldPos(int deltaX, int deltaZ)
{
    static Vector3 pos;
    for (unsigned int i = 0; i < mvObject_static.size(); ++i)
    {
        mvObject_static[i]->movePosition(deltaX, deltaZ);
    }
    pos = mvObject_npc[0]->getPos();
    for (unsigned int i = 0; i < mvObject_npc.size(); ++i)
    {
        // Sync the actual position.
        mvObject_npc[i]->movePosition(deltaX, deltaZ);
    }
    pos-= mvObject_npc[0]->getPos();
    return pos;
}

//================================================================================================
// Event handling.
//================================================================================================
void ObjectManager::Event(int obj_type, int action, int id, int val1, int val2)
{
    switch (obj_type)
    {
        case OBJECT_STATIC:
        {
            break;
        }
        case OBJECT_PLAYER:
        case OBJECT_NPC:
        {
            if (id >= (int) mvObject_npc.size()) break;
//          if (action == OBJ_WALK     ) mvObject_npc[id]->walking(val1);
            if (action == OBJ_GOTO)
            {
                SubPos2D pos;
                pos.x = val1 & 0xff;
                pos.z = val1 >> 8;
                pos.subX = val2 & 0xff;
                pos.subZ = val2 >> 8;
                mvObject_npc[ObjectNPC::HERO]->moveToDistantTile(pos);
            }
            if (action == OBJ_TEXTURE    ) mvObject_npc[id]->Equip->setTexture(val1, val2);
            if (action == OBJ_HIT        ) mvObject_npc[id]->setDamage(val1);
            if (action == OBJ_TURN       ) mvObject_npc[id]->turning(val1, false);
            if (action == OBJ_CURSOR_TURN) mvObject_npc[id]->turning(val1, true);
            if (action == OBJ_ANIMATION  ) mvObject_npc[id]->toggleAnimation(val1, val2);
            break;
        }

        default:
            Logger::log().error() << "The requested objectType does not exist.";
            break;
    }
}

//================================================================================================
// Delete a NPC-Object.
//================================================================================================
void ObjectManager::delObjectNPC(int index)
{
    if (mSelectedObject == index)
        mSelectedObject =-1;
	else if (mSelectedObject > index)
		--mSelectedObject;
 
	// nachdem ein object gelöscht wurde stimmt mSelectednicht mehr.
	// mSelected-- wenn es größer war als index????
	
	mvObject_npc[index]->freeRecources();
    delete mvObject_npc[index];
    std::vector<ObjectNPC*>::iterator i = mvObject_npc.begin();
    while (index--) ++i;
    mvObject_npc.erase(i);
	
}

//================================================================================================
//
//================================================================================================
void ObjectManager::freeRecources()
{
    for (std::vector<ObjectNPC*>::iterator i = mvObject_npc.begin(); i < mvObject_npc.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvObject_npc.clear();

    for (std::vector<ObjectStatic*>::iterator i = mvObject_static.begin(); i < mvObject_static.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvObject_static.clear();
}

//================================================================================================
// Select the (mouse clicked) object.
//================================================================================================
void ObjectManager::selectNPC(MovableObject *mob)
{
    if (mvObject_npc[ObjectNPC::HERO]->isMoving()) return;
    if (mvObject_npc[ObjectNPC::HERO]->getHealth() <= 0) return;

	// If we are already attacking a mob - every mouseclick attacks again.
	if (!mob)
    {   
		if  (mSelectedObject > 0 && mSelectedFriendly < 0)
        {
/*
			// This will crash after an NPC-Object has deleted, because the vector array is not smaller.
		mSelectedPos = mvObject_npc[mSelectedObject]->getTileScrollPos();
            mvObject_npc[ObjectNPC::HERO]->attackShortRange(mvObject_npc[mSelectedObject]);
*/
			}
		return;
    }

    // Cut the "Obj_" substring from the entity name.
    String strObject = mob->getName();
    strObject.replace(0, strObject.find("_")+1,"");
    unsigned int selectedObject = StringConverter::parseInt(strObject.substr(strObject.find("_")+1, strObject.size()));
    int selectedType   = StringConverter::parseInt(strObject.substr(0,strObject.find("_")));
    switch (selectedType)
    {
        case OBJECT_STATIC:
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)(mvObject_static[selectedObject]->getNickName()).c_str());
            break;
        }

        case OBJECT_NPC:
        {
			unsigned int index =0;
		    for (std::vector<ObjectNPC*>::iterator i = mvObject_npc.begin(); i < mvObject_npc.end(); ++i)
			{
				if ((*i)->getIndex() == selectedObject) break;
				++index;
			}
			selectedObject = index;
            // Was already selected before (= do some action on the selected NPC).
            if (selectedObject == mSelectedObject)
            {
                if (mSelectedFriendly < 0)
                {
                    mSelectedPos = mvObject_npc[selectedObject]->getTileScrollPos();
                    mvObject_npc[ObjectNPC::HERO]->attackShortRange(mvObject_npc[selectedObject]);
                }
            }
            // A new NPC was selected.
            else
            {
                char buffer[100];
                sprintf(buffer, "Enemy: %s", (mvObject_npc[selectedObject]->getNickName()).c_str());
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)(buffer));
                mSelectedFriendly = mvObject_npc[selectedObject]->getFriendly();
                mSelectedObject = selectedObject; // must be set before ::selectNPC
                ObjectVisuals::getSingleton().selectNPC(mvObject_npc[selectedObject]);
                if (mSelectedFriendly < 0)
                {
                    // mvObject_npc[ObjectNPC::HERO]->raiseWeapon(true);
                    mvObject_npc[ObjectNPC::HERO]->attackShortRange(mvObject_npc[selectedObject]);
                }
                else
                {
                    // mvObject_npc[ObjectNPC::HERO]->raiseWeapon(false);
                }
            }
            //mvObject_player[ObjectPlayer::HERO]->stopMovement();
            break;
        }

        case OBJECT_PLAYER:
        {
			unsigned int index =0;
		    for (std::vector<ObjectNPC*>::iterator i = mvObject_npc.begin(); i < mvObject_npc.end(); ++i)
			{
				if ((*i)->getIndex() == selectedObject) break;
				++index;
			}
			selectedObject = index;
            if (selectedType != mSelectedType)
            {
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)(mvObject_npc[selectedObject]->getNickName()).c_str());
                if (selectedObject == ObjectNPC::HERO)
                    ObjectVisuals::getSingleton().selectNPC(mvObject_npc[ObjectNPC::HERO], false);
                else
                    ObjectVisuals::getSingleton().selectNPC(mvObject_npc[selectedObject]);

            }
            break;
        }

        default:
            break;
    }
    mSelectedType = selectedType;
    mSelectedObject = selectedObject;
}

//================================================================================================
///
//================================================================================================
void ObjectManager::targetObjectFacingPlayer()
{
//    SubPos2D pos = mvObject_player[ObjectPlayer::HERO]->getTileScrollPos();
//    mvObject_npc[mSelectedObject]->faceToTile(pos.x, pos.z);
    mvObject_npc[mSelectedObject]->turning(mvObject_npc[ObjectNPC::HERO]->getFacing() -180, false);
}

//================================================================================================
///
//================================================================================================
void ObjectManager::targetObjectAttackPlayer()
{
    //mvObject_npc[mSelectedObject]->attackShortRange(mvObject_player[ObjectPlayer::HERO]->getNode());
	if (mSelectedObject <0) return;
	mvObject_npc[mSelectedObject]->attack();
    targetObjectFacingPlayer();
}

//================================================================================================
///
//================================================================================================
void ObjectManager::setEquipment(int npcID, int bone, int type, int itemID)
{
    if (mvObject_npc[npcID]->Equip)
        mvObject_npc[npcID]->Equip->equipItem(bone, type, itemID);
}

//================================================================================================
///
//================================================================================================
ObjectManager::~ObjectManager()
{}
