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
#include "events.h"
#include "particle.h"

///================================================================================================
/// Init all static Elemnts.
///================================================================================================

unsigned int  ParticleFX::mInstanceNr = 0;
long          ParticleFX::pTime =0;

///================================================================================================
/// .
///================================================================================================
ParticleFX::ParticleFX(Real time)
{
    mNode     = 0;
    mScenNode  = 0;
    mLifeTime = time;
}

///================================================================================================
/// .
///================================================================================================
ParticleFX::~ParticleFX()
{
    Event->GetSceneManager()->destroyParticleSystem(mParticleSystem);
    mScenNode->removeChild(mNode);
}

///================================================================================================
/// Adds an idependant ParticleSystem.
///================================================================================================
void ParticleFX::addFreeObject(const char *particleScript, Vector3 pos)
{
    mParticleSystem = Event->GetSceneManager()->createParticleSystem("pS_"+StringConverter::toString(mInstanceNr++), particleScript);
    mScenNode = Event->GetSceneManager()->getRootSceneNode();
    mNode = mScenNode->createChildSceneNode();
    mNode->attachObject(mParticleSystem);
    mNode->setPosition(pos);
}

///================================================================================================
/// Adds an ParticleSystem to an already exiting Node.
///================================================================================================
void ParticleFX::addNodeObject(const char *particleScript, SceneNode *sn, Vector3 pos)
{
    mScenNode  = sn;
    mParticleSystem= Event->GetSceneManager()->createParticleSystem(StringConverter::toString(mInstanceNr++), particleScript);
    mNode = mScenNode->createChildSceneNode(sn->getPosition()+ pos, sn->getOrientation());
    mNode->attachObject(mParticleSystem);
}

///================================================================================================
/// Update the Particle. Returns false for expired lifteime.
///================================================================================================
bool ParticleFX::update(Real dTime)
{
    if (mLifeTime >=0) // Needs lifetime check.
    {
        mLifeTime-= dTime;
        if (mLifeTime <0)
            return false;
    }
    return true;
}

///================================================================================================
///
///================================================================================================
void ParticleFX::attach()
{}

///================================================================================================
//
///================================================================================================
void ParticleFX::detach()
{}
