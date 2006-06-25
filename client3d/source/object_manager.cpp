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
#include "particle.h"
#include "sound.h"
#include "spell_manager.h"
#include "object_manager.h"
#include "object_visuals.h"
#include "gui_manager.h"

///================================================================================================
/// Defines:
/// * static: fixed to a single pos, does not have ai (stones, walls, trees, ...)
/// * npc:    controlled by ai.
/// * player: controlled by a human player.
///================================================================================================


///================================================================================================
/// Init all static Elemnts.
///================================================================================================


///================================================================================================
/// Init the model from the description file.
///================================================================================================
bool ObjectManager::init()
{
    string strType, strTemp, strMesh, strNick;
    mSelectedType  =-1;
    mSelectedObject=-1;
    //    mSelectedEnemy = false;
    int i=0;
    /// Default values.

    while(1)
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
        obj.posX  = StringConverter::parseInt(strTemp);
        Option::getSingleton().getDescStr("PosY", strTemp,i);
        obj.posY  = StringConverter::parseInt(strTemp);

        if (Option::getSingleton().getDescStr("Level", strTemp,i))
            obj.level= StringConverter::parseInt(strTemp);
        else
            obj.level= 0;

        if (Option::getSingleton().getDescStr("Centred", strTemp,i))
            obj.centred= StringConverter::parseInt(strTemp);
        else
            obj.centred= 1;

        Option::getSingleton().getDescStr("Facing", strTemp,i);
        obj.facing= StringConverter::parseReal(strTemp);
        Option::getSingleton().getDescStr("Particles", obj.particleName,i);

        if (strType == "player")
        {
            static unsigned int index=0;
            obj.index = index++;
            /// First player object is our hero.
            if (obj.index)
            {
                obj.posX = CHUNK_SIZE_X /2;
                obj.posY = CHUNK_SIZE_Z /2;
            }
            obj.type = OBJECT_PLAYER;
            addMobileObject(obj);
        }
        else if (strType == "npc")
        {
            static unsigned int index=0;
            obj.index = index++;
            obj.type = OBJECT_NPC;
            addMobileObject(obj);
        }
        else if (strType == "static")
        {
            static unsigned int index=0;
            obj.index = index++;
            obj.type = OBJECT_STATIC;
            addMobileObject(obj);
        }
        else if (strType == "weapon")
        {
            obj.type = ATTACHED_OBJECT_WEAPON;
            addBoneObject(ATTACHED_OBJECT_WEAPON, obj.meshName.c_str(), obj.particleName.c_str());
        }
        else if (strType == "armor")
        {
            obj.type = ATTACHED_OBJECT_ARMOR;
            addBoneObject(ATTACHED_OBJECT_ARMOR, obj.meshName.c_str(), obj.particleName.c_str());
        }
    }
    return true;
}

///================================================================================================
/// Adds a independant object.
///================================================================================================
void ObjectManager::addMobileObject(sObject &obj)
{
    switch (obj.type)
    {
        case OBJECT_STATIC:  // todo: branch this out.
        {
            ObjectStatic *obj_static = new ObjectStatic(obj);
            if (!obj_static) return;
            mvObject_static.push_back(obj_static);
            break;
        }
        case OBJECT_NPC:
        {
            ObjectNPC *obj_npc = new ObjectNPC(obj);
            if (!obj_npc) return;
            mvObject_npc.push_back(obj_npc);
            break;
        }
        case OBJECT_PLAYER:
        {
            ObjectPlayer *obj_player = new ObjectPlayer(obj);
            if (!obj_player) return;
            mvObject_player.push_back(obj_player);
            break;
        }
        default:
        {
            Logger::log().error() << "Unknow mobile object-type in mesh " << obj.meshName;
            return;
        }
    }
}

///================================================================================================
/// .
///================================================================================================
void ObjectManager::setPlayerEquipment(int player, int bone, int WeaponNr)
{
    mvObject_player[player]->toggleMesh(bone, WeaponNr);
}

///================================================================================================
/// .
///================================================================================================
const Entity *ObjectManager::getWeaponEntity(unsigned int WeaponNr)
{
    if (WeaponNr >= mvObject_weapon.size())
        return 0;
    return mvObject_weapon[WeaponNr]->getEntity();
}

///================================================================================================
/// Adds an equipment object.
///================================================================================================
void ObjectManager::addBoneObject(unsigned int type, const char *meshName, const char *particleName)
{
    switch (type)
    {
        case ATTACHED_OBJECT_WEAPON:
        {
            ObjectEquipment *obj_weapon = new ObjectEquipment(ATTACHED_OBJECT_WEAPON, meshName, particleName);
            if (!obj_weapon) return;
            //obj_weapon->setStats(int va11, int va12, int va13, int va14, int va15);
            mvObject_weapon.push_back(obj_weapon);
            break;
        }
        case ATTACHED_OBJECT_ARMOR:
        {
            ObjectEquipment *obj_armor = new ObjectEquipment(ATTACHED_OBJECT_ARMOR, meshName, particleName);
            if (!obj_armor) return;
            mvObject_armor.push_back(obj_armor);
            break;
        }
    }
}

///================================================================================================
///
///================================================================================================
void ObjectManager::update(int obj_type, const FrameEvent& evt)
{
    for (unsigned int i = 0; i < mvObject_static.size(); ++i)
    {
        mvObject_static [i]->update(evt);
    }
    for (unsigned int i = 0; i < mvObject_npc.size(); ++i)
    {
        mvObject_npc[i]->update(evt);
    }
    for (unsigned int i = 0; i < mvObject_player.size(); ++i)
    {
        mvObject_player[i]->update(evt);
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

///================================================================================================
/// JUST FOR TESTING.
///================================================================================================
void ObjectManager::synchToWorldPos(Vector3 pos)
{
    for(unsigned int i = 0; i < mvObject_static.size(); ++i)
    {
        mvObject_static[i]->move(pos);
    }
    for(unsigned int i = 0; i < mvObject_npc.size(); ++i)
    {
        mvObject_npc[i]->move(pos);
    }
    for(unsigned int i = 1; i < mvObject_player.size(); ++i)
    {
        mvObject_player[i]->move(pos);
    }
}

///================================================================================================
/// Event handling.
///================================================================================================
void ObjectManager::Event(int obj_type, int action, int id, int val1, int val2)
{
    switch (obj_type)
    {
        case OBJECT_STATIC:
        {
            break;
        }

        case OBJECT_NPC:
        {
            if (id >= (int) mvObject_npc.size()) break;
            if (action == OBJ_WALK     ) mvObject_npc[id]->walking(val1);
            if (action == OBJ_TURN     ) mvObject_npc[id]->turning(val1);
            if (action == OBJ_ANIMATION) mvObject_npc[id]->toggleAnimation(val1, val2);
            break;
        }

        case OBJECT_PLAYER:
        {
            if (id >= (int) mvObject_player.size()) break;
            if (action == OBJ_GOTO     ) mvObject_player[0]->moveToTile(val1, val2);
            if (action == OBJ_TURN     ) mvObject_player[id]->turning(val1);
            if (action == OBJ_WALK     ) mvObject_player[id]->walking(val1);
            if (action == OBJ_ANIMATION) mvObject_player[id]->toggleAnimation(val1, val2);
            if (action == OBJ_TEXTURE  ) mvObject_player[id]->setTexture(val1, val2, 0);
            break;
        }

        default:
        Logger::log().error() << "The requested objectType does not exist.";
        break;
    }
}

///================================================================================================
///
///================================================================================================
void ObjectManager::delObject(int )
{}

///================================================================================================
///
///================================================================================================
void ObjectManager::freeRecources()
{
    for (std::vector<ObjectPlayer*>::iterator i = mvObject_player.begin(); i < mvObject_player.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvObject_player.clear();

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

    for (std::vector<ObjectEquipment*>::iterator i = mvObject_weapon.begin(); i < mvObject_weapon.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvObject_weapon.clear();

    for (std::vector<ObjectEquipment*>::iterator i = mvObject_armor.begin(); i < mvObject_armor.end(); ++i)
    {
        (*i)->freeRecources();
        delete (*i);
    }
    mvObject_armor.clear();


}

///================================================================================================
/// Select the (mouse clicked) object.
///================================================================================================
void ObjectManager::selectNPC(MovableObject *mob)
{
    if (mvObject_player[0]->isMoving()) return;

    String strObject = mob->getName();
    /// Cut the "Obj_" substring from the entity name.
    strObject.replace(0, strObject.find("_")+1,"");
    int selectedObject = StringConverter::parseInt(strObject.substr(strObject.find("_")+1, strObject.size()));
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
            /// Was already selected before (= do some action on the selected NPC).
            if ((selectedObject == mSelectedObject) && (selectedType == mSelectedType))
            {
                //mvObject_player[0]->faceToTile(mSelectedPosX, mSelectedPosZ);
                if (mSelectedFriendly < 0)
                {
                    mvObject_player[0]->attackObjectOnTile(mSelectedPosX, mSelectedPosZ);
                    mvObject_player[0]->toggleAnimation(ObjectAnimate::ANIM_GROUP_ATTACK, 1);
                }
            }
            /// A new NPC was selected.
            else
            {
                GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)(mvObject_npc[selectedObject]->getNickName()).c_str());
                mSelectedFriendly = mvObject_npc[selectedObject]->getFriendly();
                ObjectVisuals::getSingleton().selectNPC(mob, mSelectedFriendly);
                mvObject_npc[selectedObject]->getTilePos(mSelectedPosX, mSelectedPosZ);
                mvObject_player[0]->faceToTile(mSelectedPosX, mSelectedPosZ);
                if (mSelectedFriendly < 0)
                {
                    mvObject_player[0]->raiseWeapon(true);
                }
                else
                {
                    mvObject_player[0]->raiseWeapon(false);
                }
            }
            //mvObject_player[0]->stopMovement();
            break;
        }

        case OBJECT_PLAYER:
        {
            GuiManager::getSingleton().sendMessage(GUI_WIN_TEXTWINDOW, GUI_MSG_ADD_TEXTLINE, GUI_LIST_MSGWIN  , (void*)(mvObject_player[selectedObject]->getNickName()).c_str());
            ObjectVisuals::getSingleton().selectNPC(mob, mvObject_player[selectedObject]->getFriendly());
            break;
        }
        default:
        break;
    }
    mSelectedType = selectedType;
    mSelectedObject = selectedObject;
}

///================================================================================================
///
///================================================================================================
ObjectManager::~ObjectManager()
{}
