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
// monster: ai controlled.
// ////////////////////////////////////////////////////////////////////

enum
{
    OBJ_WALK, OBJ_TURN, OBJ_CURSOR_TURN,
    OBJ_TEXTURE,
    OBJ_ANIMATION,
    OBJ_GOTO,
    OBJ_HIT,
    OBJ_SUM
};

class ObjectManager
{
public:
    // Attached objects
    enum
    {
        ATTACHED_OBJECT_WEAPON,
        ATTACHED_OBJECT_ARMOR,
        ATTACHED_OBJECT_SUM,
    };
    // Independant objects
    enum
    {
        OBJECT_STATIC,
        OBJECT_PLAYER,
        OBJECT_NPC,
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
    void addMobileObject(sObject &obj);
    void delObjectNPC(int number);
    void delObjectStatic(int number);
    void update(int type, const FrameEvent& evt);
    void Event(int obj_type, int action, int val1=0, int val2=0, int val3=0);
    void setEquipment(int npcID, int bone, int type, int itemID);
    void readyPrimaryWeapon(int npc, bool ready)
    {
        mvObject_npc[npc]->readyPrimaryWeapon(ready);
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
    const Vector3& getPos(int npc)
    {
        return mvObject_npc[npc]->getPos();
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
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    std::string mDescFile;
    std::vector<ObjectStatic*> mvObject_static;
    std::vector<ObjectNPC*   > mvObject_npc;
    int mSelectedType, mSelectedObject, mSelectedFriendly;
    TilePos mSelectedPos;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectManager()
    {}
    ~ObjectManager();
    ObjectManager(const ObjectManager&); // disable copy-constructor.
};

#endif
