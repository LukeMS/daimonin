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

///================================================================================================
/// Init all static Elemnts.
///================================================================================================

///================================================================================================
///
///================================================================================================
bool ParticleManager::init(SceneManager *SceneMgr)
{
    mSceneMgr = SceneMgr;
    mSceneNode=  mSceneMgr->getRootSceneNode();
    mCounter  = 0;
    return true;
}

///================================================================================================
///
///================================================================================================
ParticleSystem *ParticleManager::addNodeObject(const SceneNode *parentNode, const char* particleFX)
{
    /*
        sParticleObj *obj = new sParticleObj;
        mvObject_Node.push_back(obj);
        Vector3 posOffset = Vector3(0,15,-10);
        obj->particleSys = mSceneMgr->createParticleSystem("Node"+StringConverter::toString(mNodeCounter), particleFX);
        obj->node = mNode->createChildSceneNode(parentNode->getPosition()+ posOffset, parentNode->getOrientation());
        obj->node->attachObject(obj->particleSys);
        obj->direction = parentNode->getOrientation().zAxis();
        obj->speed = 180;
        ++mNodeCounter;
    */
  return 0; // switch off compiler warning.
}

///================================================================================================
/// Add a ParticleSystem to a bone.
///================================================================================================
ParticleSystem *ParticleManager::addBoneObject(Entity *ent, const char* strBone, const char* pScript, Real lifeTime)
{
    sParticles *obj = new sParticles;
    mvParticle.push_back(obj);
    obj->lifeTime = lifeTime;
    obj->pSystem = Event->GetSceneManager()->createParticleSystem("pS_"+StringConverter::toString(mCounter++), pScript);
    obj->sceneNode=0;
    obj->entity = ent;
    obj->entity->attachObjectToBone(strBone, obj->pSystem);
    return obj->pSystem;
}

///================================================================================================
/// Add an independant ParticleSystem.
///================================================================================================
ParticleSystem *ParticleManager::addFreeObject(Vector3 pos, const char *pScript, Real lifeTime)
{
    sParticles *obj = new sParticles;
    mvParticle.push_back(obj);
    obj->lifeTime = lifeTime;
    obj->pSystem  = Event->GetSceneManager()->createParticleSystem("pS_"+StringConverter::toString(mCounter++), pScript);
    obj->entity = 0;
    obj->sceneNode= Event->GetSceneManager()->getRootSceneNode()->createChildSceneNode();
    obj->sceneNode->attachObject(obj->pSystem);
    obj->sceneNode->setPosition(pos);
    return obj->pSystem;
}

///================================================================================================
/// Update the particle. Deletes all particles with expired lifetime.
///================================================================================================
void ParticleManager::update(Real dTime)
{
    for (std::vector<sParticles*>::iterator i = mvParticle.begin(); i < mvParticle.end(); ++i)
    {
       if ( (*i)->lifeTime <0 ) continue; // Infity lifeTime or already deleted.
       if (((*i)->lifeTime-= dTime) <0)
       {
          if ((*i)->entity)
            (*i)->entity->detachObjectFromBone((*i)->pSystem);
          if ((*i)->sceneNode)
            (*i)->sceneNode->getParentSceneNode()->removeChild((*i)->sceneNode);
          Event->GetSceneManager()->destroyParticleSystem((*i)->pSystem);
          delete (*i);
          i = mvParticle.erase(i);
       }
    }
}

///================================================================================================
/// Prepare a particle for deleting.
///================================================================================================
void ParticleManager::delObject(ParticleSystem *pSystem)
{
    for (std::vector<sParticles*>::iterator i = mvParticle.begin(); i < mvParticle.end(); ++i)
    {
       if ( (*i)->pSystem == pSystem )
       {
          (*i)->lifeTime = 0;
          return;
       }
    }
}

///================================================================================================
///
///================================================================================================
void ParticleManager::synchToWorldPos(const Vector3 &pos)
{
    /*
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
    */
}

///================================================================================================
///
///================================================================================================
void ParticleManager::moveNodeObject(const FrameEvent& event)
{
    /*
        for (unsigned int i = 0; i < mvObject_Node.size(); ++i)
        {
            if (!mvObject_Node[i]->speed)
                continue;
            mvObject_Node[i]->node->translate(mvObject_Node[i]->direction * mvObject_Node[i]->speed * event.timeSinceLastFrame);
        }
    */
}
