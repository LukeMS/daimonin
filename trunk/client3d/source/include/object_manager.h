/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <vector>
#include "Ogre.h"
#include "object_npc.h"

using namespace Ogre;

enum { OBJECT_PLAYER, OBJECT_NPC, OBJECT_STATIC, OBJECT_SUM };
enum { OBJ_WALK, OBJ_TURN, OBJ_TEXTURE, OBJ_ANIMATION, OBJ_SUM };

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Class.
////////////////////////////////////////////////////////////
class ObjectManager
{
  private:
    ////////////////////////////////////////////////////////////
    // Variables.
    ////////////////////////////////////////////////////////////
    SceneManager *mSceneMgr;
    SceneNode *mParentNode, *mNode;
    std::string mDescFile;
    std::vector<Entity*>mvObject_static;
    std::vector<NPC*   >mvObject_npc;


    ////////////////////////////////////////////////////////////
    // Functions.
    ////////////////////////////////////////////////////////////
    ObjectManager(const ObjectManager&); // disable copy-constructor.

  public:
    ////////////////////////////////////////////////////////////
    // Functions.
    ////////////////////////////////////////////////////////////
     ObjectManager() {;}
    ~ObjectManager();
    static ObjectManager &getSingleton() { static ObjectManager Singleton; return Singleton; }
    bool init(SceneManager *SceneMgr, SceneNode  *Node);
    bool addObject(unsigned int type, const char *desc_filename, Vector3 pos, Radian facing);
    void delObject(int number);
    void update(int type, const FrameEvent& evt);
    void keyEvent(int obj_type, int action, int val1=0, int val2=0);
    void castSpell(int npc, int spell) { mvObject_npc[npc]->castSpell(spell); }
    void toggleAnimGroup(int npc) { mvObject_npc[npc]->toggleAnimGroup(); }
    void toggleMesh(int npc, int pos, int WeaponNr) { mvObject_npc[npc]->toggleMesh(pos, WeaponNr); }
    const Vector3& getPos(int npc) { return mvObject_npc[npc]->getPos(); }
    const Vector3& getWorldPos()   { return mvObject_npc[0]->getWorldPos(); }
    const SceneNode *getNpcNode(int npc) { return mvObject_npc[npc]->getNode(); }
};

#endif
