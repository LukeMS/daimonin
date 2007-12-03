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

#include "particle_manager.h"
#include "sound.h"
#include "events.h"
#include "option.h"
#include "logger.h"

using namespace Ogre;

//================================================================================================
// Init all static Elemnts.
//================================================================================================


//================================================================================================
// .
//================================================================================================
ParticleManager::ParticleManager()
{
    mCounter  = 0;
}

//================================================================================================
// Clean up.
//================================================================================================
ParticleManager::~ParticleManager()
{
    for (std::vector<sParticles*>::iterator i = mvParticle.begin(); i < mvParticle.end(); ++i)
    {
        delete (*i);
    }
    mvParticle.clear();
}

//================================================================================================
// Add a ParticleSystem to a node.
//================================================================================================
ParticleSystem *ParticleManager::addNodeObject(SceneNode *node, const char* pScript, Real lifeTime)
{
    sParticles *obj = new sParticles;
    mvParticle.push_back(obj);
    obj->lifeTime = lifeTime;
    obj->pSystem  = Events::getSingleton().GetSceneManager()->createParticleSystem("pS_"+StringConverter::toString(mCounter++), pScript);
    obj->pSystem->setBoundsAutoUpdated(false);
    //obj->pSystem->setRenderQueueGroup(RENDER_QUEUE_6);
    obj->entity = 0;
    obj->sceneNode = node;
    obj->delNodeOnCleanup = false;
    if (node)
        node->attachObject(obj->pSystem);
    obj->neededSync = 0;
    return obj->pSystem;
}

//================================================================================================
// Add a ParticleSystem to a bone.
//================================================================================================
ParticleSystem *ParticleManager::addBoneObject(Entity *ent, const char* strBone, const char* pScript, Real lifeTime)
{
    sParticles *obj = new sParticles;
    mvParticle.push_back(obj);
    obj->lifeTime = lifeTime;
    obj->pSystem = Events::getSingleton().GetSceneManager()->createParticleSystem("pS_"+StringConverter::toString(mCounter++), pScript);
    obj->pSystem->setBoundsAutoUpdated(false);
    obj->delNodeOnCleanup = false;
    obj->sceneNode = 0;
    obj->entity = ent;
    //obj->entity->setQueryFlags(QUERY_NPC_SELECT_MASK);
    obj->entity->attachObjectToBone(strBone, obj->pSystem);
    obj->neededSync = SYNC_PARTICLES;
    return obj->pSystem;
}

//================================================================================================
// Add an independant ParticleSystem.
//================================================================================================
ParticleSystem *ParticleManager::addFreeObject(Vector3 pos, const char *pScript, Real lifeTime)
{
    sParticles *obj = new sParticles;
    mvParticle.push_back(obj);
    obj->lifeTime = lifeTime;
    obj->pSystem= Events::getSingleton().GetSceneManager()->createParticleSystem("pS_"+StringConverter::toString(mCounter++), pScript);
    //obj->pSystem->setBoundsAutoUpdated(false);
    obj->entity = 0;
    obj->delNodeOnCleanup = true;
    obj->sceneNode= Events::getSingleton().GetSceneManager()->getRootSceneNode()->createChildSceneNode();
    obj->sceneNode->attachObject(obj->pSystem);
    obj->sceneNode->setPosition(pos);
    obj->neededSync = SYNC_PARTICLES | SYNC_EMITTERS;
    obj->pSystem->setRenderQueueGroup(RENDER_QUEUE_6);
    //ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);
    return obj->pSystem;
}

//================================================================================================
// Update the particle. Deletes all particles with expired lifetime.
//================================================================================================
void ParticleManager::update(Real dTime)
{
      for (std::vector<sParticles*>::iterator i = mvParticle.begin(); i < mvParticle.end(); )
    {
        if ((*i)->lifeTime <0  // Infinite lifeTime.
                || ((*i)->lifeTime-= dTime) >=0) // Lifeteme not expired.
        {
            ++i;
        }
        else
        {
            if ((*i)->entity)
                (*i)->entity->detachObjectFromBone((*i)->pSystem);
            if ((*i)->delNodeOnCleanup)
            {
                if ((*i)->sceneNode)
                    (*i)->sceneNode->getParentSceneNode()->removeChild((*i)->sceneNode);
            }
            else
            {
                if ((*i)->sceneNode)
                    (*i)->sceneNode->detachObject((*i)->pSystem);
            }
            Events::getSingleton().GetSceneManager()->destroyParticleSystem((*i)->pSystem);
            delete (*i);
            i = mvParticle.erase(i);
        }
    }
}

//================================================================================================
// Prepare a particle for deleting.
//================================================================================================
void ParticleManager::delObject(ParticleSystem *pSystem)
{
    for (std::vector<sParticles*>::iterator i = mvParticle.begin(); i < mvParticle.end(); ++i)
    {
        if ((*i)->pSystem == pSystem)
        {
            (*i)->lifeTime = 0;
            return;
        }
    }
}

//================================================================================================
// Pause the ParticleSystem.
//================================================================================================
void ParticleManager::pauseAll(bool pause)
{
    for (std::vector<sParticles*>::iterator i = mvParticle.begin(); i < mvParticle.end(); ++i)
    {
        if (pause)
        {
            (*i)->pSystem->setSpeedFactor(0.0f);
            for (unsigned short sum = 0; sum < (*i)->pSystem->getNumEmitters(); ++sum)
                (*i)->pSystem->getEmitter(sum)->setEnabled(false);
        }
        else
        {
            (*i)->pSystem->setSpeedFactor(1.0f);
            for (unsigned short sum = 0; sum < (*i)->pSystem->getNumEmitters(); ++sum)
                (*i)->pSystem->getEmitter(sum)->setEnabled(true);
        }
    }
}

//================================================================================================
// Workaround for a ogre bug (Attached particles wont get updeted after a position change).
//================================================================================================
void ParticleManager::syncToWorldPos(Vector3 &deltaPos)
{
    Particle* p;
    for (std::vector<sParticles*>::iterator i = mvParticle.begin(); i < mvParticle.end(); ++i)
    {
        if (!(*i)->pSystem->isAttached() || !(*i)->neededSync) continue;
        //if ((*i)->pSystem->getKeepParticlesInLocalSpace()) continue;
        //(*i)->pSystem->_update(.011);
        if ((*i)->neededSync & SYNC_EMITTERS)
        {
            for (unsigned short sum = 0; sum < (*i)->pSystem->getNumEmitters(); ++sum)
            {
                (*i)->pSystem->getEmitter(sum)->setPosition((*i)->pSystem->getEmitter(sum)->getPosition() - deltaPos);
            }
        }
        if ((*i)->neededSync & SYNC_PARTICLES)
        {
            for (size_t sum = (*i)->pSystem->getNumParticles(); sum;)
            {
                p = (*i)->pSystem->getParticle(--sum);
                p->position-= deltaPos;
            }
        }
    }
}

//================================================================================================
// .
//================================================================================================
void ParticleManager::setColorRange(ParticleSystem *pSystem, ColourValue start, ColourValue stop)
{
    for (int i=0; i < pSystem->getNumEmitters(); ++i)
    {
        pSystem->getEmitter(i)->setColourRangeStart(start);
        pSystem->getEmitter(i)->setColourRangeEnd  (stop);
    }
}

//================================================================================================
// .
//================================================================================================
void ParticleManager::setEmitterSize(ParticleSystem *pSystem, float sizeZ, float sizeX, bool adjustRate)
{
    for (int i=0; i < pSystem->getNumEmitters(); ++i)
    {
        pSystem->getEmitter(i)->setParameter("height", StringConverter::toString(sizeZ));
        pSystem->getEmitter(i)->setParameter("width" , StringConverter::toString(sizeX));
        if (adjustRate) pSystem->getEmitter(i)->setEmissionRate(sizeX *sizeZ * 3.0);
    }
}
