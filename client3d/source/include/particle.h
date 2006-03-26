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

#ifndef PARTICLE_H
#define PARTICLE_H

#include <Ogre.h>

using namespace Ogre;

class ParticleFX
{
public:
    ParticleFX(Real time);
    ~ParticleFX();

    void addFreeObject(const char *particleScript, Vector3 pos);
    void addNodeObject(const char *particleScript, SceneNode *sn, Vector3 pos);
    void attach();
    void detach();
    bool update(Real dTime);

    const Vector3 &getPosition() const
    {
        return mScenNode->getPosition();
    }
    SceneNode *getSceneNode() const
    {
        return mScenNode;
    }
    ParticleSystem *getParticleSystem() const
    {
        return mParticleSystem;
    }

    void SetPosition(const Ogre::Vector3 &position);
    void SetSceneNode(SceneNode *node)
    {
        mScenNode = node;
    }
    void SetParticleFX(ParticleSystem *pfx)
    {
        mParticleSystem = pfx;
    }
    static unsigned int mInstanceNr;
    static long pTime;
private:
    Real speed;       //  0: Object has static Position.
    Real mLifeTime;   // -1: Infinity Lifetime.
    Vector3 mDir;
    SceneNode *mScenNode, *mNode;
    ParticleSystem *mParticleSystem;
    std::string mName, mPfxName;
};

#endif
