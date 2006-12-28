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

#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <vector>
#include "Ogre.h"
#include "object_npc.h"
#include "object_static.h"

using namespace Ogre;

// ////////////////////////////////////////////////////////////////////
// Define:
// player:  human controlled.
// hero:    human controlled (the one in front of this keyboard).
// monster: ai controlled.
// ////////////////////////////////////////////////////////////////////

class ObjectManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        MISSLE_ARROW,
        MISSLE_SHURIKEN,
    };
    enum
    {
        OBJ_WALK, OBJ_TURN, OBJ_CURSOR_TURN,
        OBJ_TEXTURE,
        OBJ_ANIMATION,
        OBJ_GOTO,
        OBJ_HIT,
        OBJ_SUM
    };
    enum
    {
        QUERY_PARTICLE_MASK   =1 << 0,
        QUERY_TILES_WATER_MASK=1 << 1,
        QUERY_TILES_LAND_MASK =1 << 2,
        QUERY_ENVIRONMENT_MASK=1 << 3,
        QUERY_NPC_MASK        =1 << 4,
        QUERY_CONTAINER       =1 << 5,  /**< Stuff that can be opened (chest, sack,... **/
        QUERY_EQUIPMENT_MASK  =1 << 6,  /**< Stuff that can be equipped (clothes, weapons,... **/
        QUERY_NPC_SELECT_MASK =1 << 7,
    };
    // Attached objects
    enum
    {
        ATTACHED_OBJECT_WEAPON,
        ATTACHED_OBJECT_ARMOR,
        ATTACHED_OBJECT_SUM,
    };

    /** Independant Object types. **/
    enum
    {
        // Static objects.
        OBJECT_ENVIRONMENT,
        OBJECT_CONTAINER,   /**< Chest, Sack, ... **/
        // Dynamic objects.
        OBJECT_NPC,         /**< Server contolled character. **/
        OBJECT_PLAYER,      /**< Human controlled character. **/
        OBJECT_SUM,
    };
    static char *ObjectID[OBJECT_SUM];
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static ObjectManager &getSingleton()
    {
        static ObjectManager Singleton; return Singleton;
    }
    void freeRecources();
    bool init();
    void addMobileObject(ObjectStatic::sObject &obj);
    void delObjectNPC(int number);
    void delObjectStatic(int number);
    void update(int type, const FrameEvent& evt);
    void mousePressed(MovableObject *mob, TilePos pos, bool modifier);
    void Event(int obj_type, int action, int val1=0, int val2=0, int val3=0);
    void setEquipment(int npcID, int bone, int type, int itemID);
    void highlightObject(MovableObject *mob);
    void shoot(int missle, ObjectNPC *srcMob, ObjectNPC *dstMob);
    void readyPrimaryWeapon(int npc, bool ready)
    {
        mvObject_npc[npc]->readyPrimaryWeapon(ready);
    }
    bool isMoving(int npc)
    {
        return mvObject_npc[npc]->isMoving();
    }
    bool isPrimaryWeaponReady(int npc)
    {
        return mvObject_npc[npc]->isPrimaryWeaponReady();
    }
    void readySecondaryWeapon(int npc, bool ready)
    {
        mvObject_npc[npc]->readySecondaryWeapon(ready);
    }
    bool isSecondaryWeaponReady(int npc)
    {
        return mvObject_npc[npc]->isSecondaryWeaponReady();
    }
    void castSpell(int npc, int spell)
    {
        mvObject_npc[npc]->castSpell(spell);
    }
    const String &getNameNPC(int npc)
    {
        return mvObject_npc[npc]->getNickName();
    }
    void setNameNPC(int npc, const char *name)
    {
        mvObject_npc[npc]->setNickName(name);
    }
    void setPosition(int npc, TilePos pos)
    {
        mvObject_npc[npc]->setPosition(pos);
    }
    const Vector3& getPos(int npc)
    {
        return mvObject_npc[npc]->getPosition();
    }
    const Vector3 &synchToWorldPos(int deltaX, int deltaZ);
    void selectObject(MovableObject *mob);
    Vector3 getTargetedWorldPos()
    {
        return mvObject_npc[mSelectedObject]->getSceneNode()->getPosition();
    }
    ObjectNPC *getObjectNPC(unsigned int index)
    {
        if (index < mvObject_npc.size())
            return mvObject_npc[index];
        else
            return 0;
    }
    ObjectNPC *getSelectedNPC()
    {
        if (mSelectedObject >= 0)
            return mvObject_npc[mSelectedObject];
        else
            return 0;
    }
    const TilePos getTargetedPos()
    {
        return mSelectedPos;
    }
    void targetObjectFacingNPC(int npcIndex); // just a hack. Server will handle this.
    void targetObjectAttackNPC(int npcIndex); // just a hack. Server will handle this.

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    std::string mDescFile;
    std::vector<ObjectStatic*> mvObject_static;
    std::vector<ObjectNPC*   > mvObject_npc;
    int mSelectedType, mSelectedObject, mSelectedFriendly;
    TilePos mSelectedPos;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectManager() {}
    ~ObjectManager();
    ObjectManager(const ObjectManager&); // disable copy-constructor.
    void extractObject(MovableObject *mob);
};

#endif
