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

/// ////////////////////////////////////////////////////////////////////
/// Define:
/// player:  human controlled.
/// monster: ai controlled.
/// ////////////////////////////////////////////////////////////////////

enum
{
    OBJECT_PLAYER, OBJECT_NPC, OBJECT_STATIC, OBJECT_SUM
};
enum
{
    OBJ_WALK, OBJ_TURN, OBJ_TEXTURE, OBJ_ANIMATION, OBJ_GOTO, OBJ_SUM
};

class ObjectManager
{
public:
    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    static ObjectManager &getSingleton()
    {
        static ObjectManager Singleton; return Singleton;
    }
    void freeRecources();
    bool init();
    bool addObject(unsigned int type, const char *desc_filename, int posX, int posY, float facing);
    void delObject(int number);
    void update(int type, const FrameEvent& evt);
    void Event(int obj_type, int action, int val1=0, int val2=0, int val3=0);
    void castSpell(int npc, int spell)
    {
        mvObject_npc[npc]->castSpell(spell);
    }
    void toggleMesh(int npc, int pos, int WeaponNr)
    {
        mvObject_npc[npc]->toggleMesh(pos, WeaponNr);
    }
    const Vector3& getPos(int npc)
    {
        return mvObject_npc[npc]->getPos();
    }
    const Vector3& getWorldPos()
    {
        return mvObject_npc[0]->getWorldPos();
    }
    const SceneNode *getNpcNode(int npc)
    {
        return mvObject_npc[npc]->getNode();
    }
    void ObjectManager::synchToWorldPos(Vector3 pos);
private:
    /// ////////////////////////////////////////////////////////////////////
    /// Variables.
    /// ////////////////////////////////////////////////////////////////////
    std::string mDescFile;
    std::vector<ObjStatic*>mvObject_static;
    std::vector<NPC*   >mvObject_npc;

    /// ////////////////////////////////////////////////////////////////////
    /// Functions.
    /// ////////////////////////////////////////////////////////////////////
    ObjectManager()
    {}
    ~ObjectManager();
    ObjectManager(const ObjectManager&); // disable copy-constructor.
};

#endif
