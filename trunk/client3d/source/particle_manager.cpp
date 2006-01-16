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

#include "particle_manager.h"
#include "particle.h"
#include "sound.h"
#include "events.h"
#include "option.h"
#include "logger.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
static std::vector<sParticleObj*>mvObject_Node;

//=================================================================================================
//
//=================================================================================================
bool ParticleManager::init(SceneManager *SceneMgr)
{
  mSceneMgr = SceneMgr;
  mNode =  mSceneMgr->getRootSceneNode();;
  mNodeCounter = mBoneCounter = 0;
  return true;
}

//=================================================================================================
//
//=================================================================================================
void ParticleManager::addNodeObject(const SceneNode *parentNode, const char* particleFX)
{
  sParticleObj *obj = new sParticleObj;
  mvObject_Node.push_back(obj);
  Vector3 posOffset = Vector3(0,15,-10);
  obj->particleSys = ParticleSystemManager::getSingleton().createSystem("Node"+StringConverter::toString(mNodeCounter), particleFX);
  obj->node = mNode->createChildSceneNode(parentNode->getPosition()+ posOffset, parentNode->getOrientation());
  obj->node->attachObject(obj->particleSys);
  obj->direction = parentNode->getOrientation().zAxis();
  obj->speed = 180;
  ++mNodeCounter;
}

//=================================================================================================
//
//=================================================================================================
void ParticleManager::delNodeObject(int )
{
}

//=================================================================================================
//
//=================================================================================================
void ParticleManager::synchToWorldPos(const Vector3 &pos)
{
  int sum;
  Particle* p;
  for (unsigned int i = 0; i < mvObject_Node.size(); ++i)
  {
    for (sum = (int)mvObject_Node[i]->particleSys->getNumParticles()-1; sum >=0; --sum)
    {
      p = mvObject_Node[i]->particleSys->getParticle(sum);
      p->position += pos;
    }
  }
}

//=================================================================================================
//
//=================================================================================================
void ParticleManager::moveNodeObject(const FrameEvent& event)
{
  for (unsigned int i = 0; i < mvObject_Node.size(); ++i)
  {
    if (!mvObject_Node[i]->speed) continue;
    mvObject_Node[i]->node->translate(mvObject_Node[i]->direction * mvObject_Node[i]->speed * event.timeSinceLastFrame);
  }
}
