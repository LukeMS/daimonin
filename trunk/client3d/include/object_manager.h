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

#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <vector>
#include "object_npc.h"
#include "object_static.h"
#include "object_missile.h"

// ////////////////////////////////////////////////////////////////////
// Define:
// player:  human controlled.
// hero:    human controlled (the one in front of this keyboard).
// monster: ai controlled.
// ////////////////////////////////////////////////////////////////////

/**
 ** This singleton class handles all interactive objects.
 ** For non-interactive object the TileManager is used.
 *****************************************************************************/
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
        OBJ_WALK, OBJ_CURSOR_WALK,
        OBJ_TURN, OBJ_CURSOR_TURN,
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
        QUERY_CAMERA_MASK     =1 << 8,
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
        // Static positon objects.
        OBJECT_ENVIRONMENT, /**< Non interactive element. **/
        OBJECT_WALL,        /**< Non interactive element (special placement on tile). **/
        OBJECT_CONTAINER,   /**< Chest, Sack, ... **/
        // Dynamic positon objects.
        OBJECT_NPC,         /**< Server contolled character. **/
        OBJECT_PLAYER,      /**< Human controlled character. **/
        OBJECT_SUM,
    };
    static const char *ObjectID[OBJECT_SUM];
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static ObjectManager &getSingleton()
    {
        static ObjectManager Singleton; return Singleton;
    }
    void freeRecources();
    void init();
    void addMobileObject(ObjectStatic::sObject &obj);
    void deleteMissile(int number);
    void deleteStatic(int number);
    void deleteNPC   (int number);
    void update(int type, const Ogre::FrameEvent& evt);
    void mousePressed(Ogre::MovableObject *mob, bool modifier);
    void Event(int obj_type, int action, int val1=0, int val2=0, int val3=0);
    void setEquipment(int npcID, int bone, int type, int itemID);
    void highlightObject(Ogre::MovableObject *mob);
    void shoot(int missle, ObjectNPC *srcMob, ObjectNPC *dstMob);
    void readyPrimaryWeapon(int npc, bool ready)
    {
        mvNPC[npc]->readyPrimaryWeapon(ready);
    }
    bool isMoving(int npc)
    {
        return mvNPC[npc]->isMoving();
    }
    bool isPrimaryWeaponReady(int npc)
    {
        return mvNPC[npc]->isPrimaryWeaponReady();
    }
    void readySecondaryWeapon(int npc, bool ready)
    {
        mvNPC[npc]->readySecondaryWeapon(ready);
    }
    bool isSecondaryWeaponReady(int npc)
    {
        return mvNPC[npc]->isSecondaryWeaponReady();
    }
    void castSpell(int npc, int spell)
    {
        mvNPC[npc]->castSpell(spell);
    }
    const Ogre::String &getNameNPC(int npc)
    {
        return mvNPC[npc]->getNickName();
    }
    void setNameNPC(int npc, const char *name)
    {
        mvNPC[npc]->setNickName(name);
    }
    void setPosition(int npc, Ogre::Vector3 pos)
    {
        mvNPC[npc]->setPosition(pos);
    }
    const Ogre::Vector3& getPosition(int npc)
    {
        return mvNPC[npc]->getPosition();
    }
    void synchToWorldPos(int deltaX, int deltaZ);
    void selectObject(Ogre::MovableObject *mob);
    Ogre::Vector3 getTargetedWorldPos()
    {
        return mvNPC[mSelectedObject]->getSceneNode()->getPosition();
    }
    ObjectNPC *getObjectNPC(unsigned int index)
    {
        if (index < mvNPC.size())
            return mvNPC[index];
        else
            return 0;
    }
    ObjectNPC *getSelectedNPC()
    {
        if (mSelectedObject >= 0)
            return mvNPC[mSelectedObject];
        else
            return 0;
    }
    const Ogre::Vector3 getTargetedPos()
    {
        return mSelectedPos;
    }
    void targetObjectAttackNPC(int npcIndex); // just a hack. Server will handle this.
    bool createFlipBook(Ogre::String meshName, int sumRotations = 8);
private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::String mDescFile;
    std::vector<ObjectStatic*> mvStatic;
    std::vector<ObjectNPC*   > mvNPC;
    std::vector<ObjectMissile*> mvMissile;
    int mSelectedType, mSelectedObject, mSelectedFriendly;
    Ogre::Vector3 mSelectedPos;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectManager() {}
    ~ObjectManager();
    ObjectManager(const ObjectManager&); // disable copy-constructor.
    void extractObject(Ogre::MovableObject *mob);
};

#endif
