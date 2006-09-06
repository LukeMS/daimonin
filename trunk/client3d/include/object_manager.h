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
    // Independant objects
    enum
    {
        OBJECT_PLAYER,
        OBJECT_NPC,
        OBJECT_STATIC,
        OBJECT_SUM,
    };
    // Attached objects
    enum
    {
        ATTACHED_OBJECT_WEAPON,
        ATTACHED_OBJECT_ARMOR,
        ATTACHED_OBJECT_SUM,
    };

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
    void delObject(int number);
    void update(int type, const FrameEvent& evt);
    void Event(int obj_type, int action, int val1=0, int val2=0, int val3=0);
    void setEquipment(int npcID, int bone, int type, int itemID);

    void castSpell(int npc, int spell)
    {
        mvObject_npc[npc]->castSpell(spell);
    }
    const Vector3& getPos(int npc)
    {
        return mvObject_npc[npc]->getPos();
    }
    const SceneNode *getNpcNode(int npc)
    {
        return mvObject_npc[npc]->getNode();
    }
    const Vector3 &synchToWorldPos(int deltaX, int deltaZ);
    void selectNPC(MovableObject *mob);

    Vector3 getTargetedWorldPos()
    {
        return mvObject_npc[mSelectedObject]->getNode()->getPosition();
    }
    ObjectNPC *getObjectNPC(int index)
    {
        return mvObject_npc[index];
    }
    ObjectNPC *getSelectedNPC()
    {
        return mvObject_npc[mSelectedObject];
    }
    const SubPos2D getTargetedPos()
    {
        return mSelectedPos;
    }
    void targetObjectFacingPlayer(); // just a hack. Server will handle all movement stuff.
    void targetObjectAttackPlayer(); // just a hack.

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables.
    // ////////////////////////////////////////////////////////////////////
    std::string mDescFile;
    std::vector<ObjectStatic*> mvObject_static;
    std::vector<ObjectNPC*   > mvObject_npc;
    int mSelectedType, mSelectedObject;
    int mSelectedFriendly;
    SubPos2D mSelectedPos;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectManager()
    {}
    ~ObjectManager();
    ObjectManager(const ObjectManager&); // disable copy-constructor.
};

#endif
