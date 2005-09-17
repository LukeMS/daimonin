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

#ifndef PARTICLE_MANAGER_H
#define PARTICLE_MANAGER_H

#include <Ogre.h>
#include <vector>

using namespace Ogre;

////////////////////////////////////////////////////////////
/// Defines.
////////////////////////////////////////////////////////////
struct sParticleObj
{
  SceneNode *node;
  ParticleSystem *particleSys;
  Vector3 direction;
  Real speed; // 0: Object has static Position.
};

////////////////////////////////////////////////////////////
/// Class.
////////////////////////////////////////////////////////////
class ParticleManager
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  static ParticleManager &getSingleton()
  {
    static ParticleManager Singleton; return Singleton;
  }
  bool init(SceneManager *SceneMgr);
  void addBoneObject(unsigned int npc, unsigned int spell);
  void delBoneObject(int nr);
  void addNodeObject(const SceneNode *node, const char* particleFX);
  void delNodeObject(int nr);
  void delObject(int number);
  void synchToWorldPos(const Vector3 &pos);
  void moveNodeObject(const FrameEvent& event);

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  SceneManager *mSceneMgr;
  SceneNode    *mNode;
  unsigned int mNodeCounter, mBoneCounter;

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  ParticleManager()
  {}
  ~ParticleManager()
  {}
  ParticleManager(const ParticleManager&); // disable copy-constructor.
};

#endif
