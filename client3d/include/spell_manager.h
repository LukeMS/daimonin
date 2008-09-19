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
#ifndef SPELL_MANAGER_H
#define SPELL_MANAGER_H

#include <vector>
#include <Ogre.h>
#include "spell_range.h"

/**
 ** This singleton class handles all spell effects.
 *****************************************************************************/
class SpellManager
{

public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum
    {
        SPELL_SRC_NPC, SPELL_SRC_OBJECT, SPELL_SRC_SUM
    };
    enum
    {
        SPELL_DEST_RANGE, SPELL_DEST_CASTER, SPELL_DEST_SUM
    };
    enum
    {
        SPELL_TYPE_DAMAGE, SPELL_TYPE_HEAL, SPELL_TYPE_SUM
    };

    struct _Spell
    {
        Ogre::SceneNode *node;
        Ogre::ParticleSystem* particleSys;
    }
    Spell;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static SpellManager &getSingleton()
    {

        static SpellManager Singleton; return Singleton;
    }
    bool init(Ogre::SceneManager *SceneMgr);
    bool addObject( unsigned int npc, unsigned int spell);
    void delObject(int number);
    void update(int type, const Ogre::FrameEvent& evt);
    void keyEvent(int obj_type, int action, int val1=0, int val2=0);
    void test(Ogre::Vector3 pos);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    Ogre::SceneManager *mSceneMgr;
    Ogre::SceneNode  *mNode;
    Ogre::String mDescFile;
//    std::vector<SpellRange*>mvObject_range;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    SpellManager()
    {}
    ~SpellManager();
    SpellManager(const SpellManager&); // disable copy-constructor.
};

#endif
