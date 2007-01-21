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

#ifndef PARTICLE_MANAGER_H
#define PARTICLE_MANAGER_H

#include <vector>
#include <Ogre.h>

class ParticleManager
{

public:
    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    static ParticleManager &getSingleton()
    {
        static ParticleManager Singleton; return Singleton;
    }
    Ogre::ParticleSystem *addFreeObject(Ogre::Vector3 pos, const char *particleScript, Ogre::Real lifeTime);
    Ogre::ParticleSystem *addBoneObject(Ogre::Entity *ent, const char *boneName, const char* particleScript, Ogre::Real lifeTime);
    Ogre::ParticleSystem *addNodeObject(Ogre::SceneNode *node, const char* particleScript, Ogre::Real lifeTime);
    void delNodeObject(int nr);
    void delObject(Ogre::ParticleSystem *pSystem);
    void synchToWorldPos(Ogre::Vector3 &deltaPos);
    void update(Ogre::Real time);
    void pauseAll(bool pause);
    void setColorRange (Ogre::ParticleSystem *pSystem, Ogre::ColourValue start, Ogre::ColourValue stop);
    void setEmitterSize(Ogre::ParticleSystem *pSystem, float sizeZ, float sizeX, bool adjustEmitterRate);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    unsigned int mCounter;

    enum
    {
        SYNC_EMITTERS   = 1<< 0,
        SYNC_PARTICLES  = 1<< 1
    };

    typedef struct
    {
        Ogre::Real speed;      //  0: Object has a static position.
        Ogre::Real lifeTime;   // -1: Infinity lifetime.
        Ogre::SceneNode *sceneNode;
        bool delNodeOnCleanup;
        char neededSync;
        Ogre::ParticleSystem *pSystem;
        Ogre::Entity *entity;
    }
    sParticles;
    std::vector<sParticles*>mvParticle;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ParticleManager();
    ~ParticleManager();
    ParticleManager(const ParticleManager&); // disable copy-constructor.
};

#endif
