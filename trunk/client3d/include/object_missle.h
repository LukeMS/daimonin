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

#ifndef OBJ_NISSLE_H
#define OBJ_NISSLE_H

#include <Ogre.h>
#include "define.h"

using namespace Ogre;

class ObjectMissle
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    typedef enum { DART, ARROW, BOLT, SHURIKEN, SPEAR, BOOMERANG, } enumType;
    typedef enum { FIRE, ICE, POISON,  } enumParticle;
    typedef struct
    {
        unsigned int index;           /**< Unique number for this object. **/
        enumType     type;
        enumParticle particle;
        String meshName;              /**< Name of the ogre3d mesh. **/
        Real facing;
        int maxDamage;
        int minDamage;
    }
    sObject;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectMissle(sObject &obj);
    virtual ~ObjectMissle();
    virtual void freeRecources();
    virtual bool update(const FrameEvent& event);
    const Vector3 &getPosition()
    {
        return mNode->getPosition();
    }
    SceneNode *getSceneNode()
    {
        return mNode;
    }
    Real getFacing()
    {
        return mFacing.valueDegrees();
    }
    unsigned int getIndex()
    {
        return mIndex;
    }
protected:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    static SceneManager *mSceneMgr;
    Degree mFacing;
    unsigned int mIndex;
    SceneNode *mNode;
    Entity *mEntity;

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectMissle(const ObjectMissle&); // disable copy-constructor.
};

#endif
