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

#ifndef PARTICLE_H
#define PARTICLE_H

#include <Ogre.h>

using namespace Ogre;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Class.
////////////////////////////////////////////////////////////
class ParticleFX
{
  public:
     ParticleFX(SceneNode *parent, const std::string &pfxName, const std::string &name);
    ~ParticleFX() {;}

    void attach();
    void detach();

    const Vector3 &getPosition() const { return mNode->getPosition(); }
    SceneNode *getSceneNode() const { return mNode; }
    ParticleSystem *getParticleFX() const { return mParticleFX; }

    void SetPosition(const Ogre::Vector3 &position);
    void SetSceneNode(SceneNode *node) { mNode = node; }
    void SetParticleFX(ParticleSystem *pfx) { mParticleFX = pfx; }

  private:
    SceneNode *mNode, *mParent;
    ParticleSystem *mParticleFX;
    std::string mName, mPfxName;
};

#endif
