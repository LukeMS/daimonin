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

#include "define.h"
#include "spell_manager.h"
#include "object_manager.h"
#include "particle_manager.h"
#include "option.h"
#include "logfile.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================

//=================================================================================================
// Init the model from the description file.
//=================================================================================================
bool SpellManager::init(SceneManager *SceneMgr, SceneNode *Node)
{
    mSceneMgr = SceneMgr;
    mNode = Node;
    return true;
}

//=================================================================================================
//
//=================================================================================================
bool SpellManager::addObject(unsigned int npc, unsigned int spell)
{
    // Player cast Fireball.
    const SceneNode *node = ObjectManager::getSingleton().getNpcNode(0);
    //ParticleManager::getSingleton().addNodeObject(node, "Particle/GreenyNimbus");
    ParticleManager::getSingleton().addNodeObject(node, "Particle/FireBall");
    return true;
}

//=================================================================================================
//
//=================================================================================================
void SpellManager::update(int spell_type, const FrameEvent& evt)
{
}

//=================================================================================================
// JUST FOR TESTING.
//=================================================================================================
void SpellManager::keyEvent(int spell_type, int action, int val1, int val2)
{
}

//=================================================================================================
//
//=================================================================================================
void SpellManager::delObject(int number)
{
}

//=================================================================================================
//
//=================================================================================================
SpellManager::~SpellManager()
{
    for (unsigned int i = 0; i < mvObject_range.size(); ++i)
    {
        delete mvObject_range[i];
    }
}
