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
#include "object/object.h"
#include "object/object_element.h"

// ////////////////////////////////////////////////////////////////////
// Define:
// player:  human controlled.
// hero:    human controlled (the one in front of this keyboard).
// monster: ai controlled.
// ////////////////////////////////////////////////////////////////////

///
/// This singleton class handles all interactive objects.
/// For non-interactive object the TileManager is used.
class ObjectManager
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef struct
    {
        Ogre::String nickName;      ///< Ingame-Name.
        Ogre::String meshName;      ///< Name of the ogre3d mesh.
        Ogre::Vector3 pos;          ///< Tile-pos.
        Ogre::uchar boundingRadius; ///< The radius of subtiles, the NPC stands on.
        Ogre::Real facing;
        unsigned int index;         ///< Unique number for this object.
        int type;                   ///< Type: e.g. static, npc, etc.
        int particleNr;             ///< Number of the particle effect.
        int level;                  ///< Floor-level.
        int friendly;
        int attack;
        int defend;
        int maxHP;
        int maxMana;
        int maxGrace;
        //char walkable[8];           ///< 8x8 bit for the walkable status of a tile.
    }
    sObject;
    enum
    {
        MISSLE_ARROW,
        MISSLE_SHURIKEN,
    };
    enum
    {
        EVT_WALK, EVT_CURSOR_WALK,
        EVT_TURN, EVT_CURSOR_TURN,
        EVT_SKINCOLOR,
        EVT_ANIMATION,
        EVT_GOTO,
        EVT_HIT,
        EVT_SUM
    };
    typedef enum
    {
        QUERY_MASK_PARTICLE    =1 << 0,
        QUERY_MASK_TILES_WATER =1 << 1,
        QUERY_MASK_TILES_LAND  =1 << 2,
        QUERY_MASK_ENVIRONMENT =1 << 3,
        QUERY_MASK_NPC         =1 << 4,
        QUERY_MASK_CONTAINER   =1 << 5,  ///< Stuff that can be opened (chest, sack,...
        QUERY_MASK_EQUIPMENT   =1 << 6,  ///< Stuff that can be equipped (clothes, weapons,...
        QUERY_MASK_NPC_SELECT  =1 << 7,
        QUERY_MASK_CAMERA      =1 << 8,
    } queryMask;
    // Attached objects
    enum
    {
        ATTACHED_OBJECT_WEAPON,
        ATTACHED_OBJECT_ARMOR,
        ATTACHED_OBJECT_SUM,
    };

    /// Independant Object types.
    enum
    {
        // Static positon objects.
        OBJECT_ENVIRONMENT, ///< Non interactive element.
        OBJECT_WALL,        ///< Non interactive element (special placement on tile).
        OBJECT_CONTAINER,   ///< Chest, Sack, ...
        // Dynamic positon objects.
        OBJECT_NPC,         ///< Server contolled character.
        OBJECT_PLAYER,      ///< Human controlled character.
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
    void addCreature(sObject &obj);
    void update(const Ogre::FrameEvent& evt);
    void mousePressed(Ogre::MovableObject *mob, bool modifier);
    void Event(std::string &name, int action, int id, int val0=0, int val1=0);
    void setEquipment(int npcID, int bone, int type, int itemID);
    void highlightObject(Ogre::MovableObject *mob, bool highlight);
//    void shoot(int missle, ObjectNPC *srcMob, ObjectNPC *dstMob);
/*
    void readyPrimaryWeapon(int npc, bool ready)
    {
//        mvNPC[npc]->readyPrimaryWeapon(ready);
    }
    bool isMoving(int npc)
    {
        //return mvNPC[npc]->isMoving();
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
    void setSkinColor(int npc, Ogre::uint32 color)
    {
        mvNPC[npc]->setSkinColor(color);
    }
    const Ogre::Vector3& getPosition(int npc)
    {
        return mvNPC[npc]->getPosition();
    }
*/
    void syncToMapScroll(int deltaX, int deltaZ);
    void selectObject(Ogre::MovableObject *mob);
/*
    Ogre::Vector3 getTargetedWorldPos()
    {
        return mvNPC[mSelectedObject]->getSceneNode()->getPosition();
    }
    ObjectNPC *getObjectNPC(unsigned int index)
    {
        return (index < mvNPC.size())?mvNPC[index]:0;
    }
    ObjectNPC *getSelectedNPC()
    {
        return (mSelectedObject >= 0)?mvNPC[mSelectedObject]:0;
    }
    const Ogre::Vector3 getTargetedPos() const
    {
        return mSelectedPos;
    }
    void targetObjectAttackNPC(int npcIndex); // just a hack. Server will handle this.
*/
    void setAvatarName(std::string &name) { mAvatarName = name;}
    std::string getAvatarName() { return mAvatarName;}
    const Ogre::Vector3 getAvatarPos();

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::String mDescFile;
//    std::vector<ObjectMissile*> mvMissile;
    int mSelectedType, mSelectedObject;
    Ogre::Vector3 mSelectedPos;
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectManager() {}
    ~ObjectManager();
    ObjectManager(const ObjectManager&);            ///< disable copy-constructor.
    ObjectManager &operator=(const ObjectManager&); ///< disable assignment operator.
    void extractObject(Ogre::MovableObject *mob);
    Object *getObject(std::string &name);

    //////// NEW
    std::string mAvatarName;
    std::map<std::string, class Object*> mObjectMap;
    Ogre::SceneManager *mSceneManager;
    Object *mObjectAvatar;
};

#endif
