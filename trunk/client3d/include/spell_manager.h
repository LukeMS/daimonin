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

#ifndef SPELL_MANAGER_H
#define SPELL_MANAGER_H

#include <vector>
#include <Ogre.h>
#include "spell_range.h"

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
    std::vector<SpellRange*>mvObject_range;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    SpellManager()
    {}
    ~SpellManager();
    SpellManager(const SpellManager&); // disable copy-constructor.
};

#endif
