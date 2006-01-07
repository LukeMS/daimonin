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

#include "define.h"
#include "object_manager.h"
#include "option.h"
#include "logger.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================


//=================================================================================================
// Init the model from the description file.
//=================================================================================================
bool ObjectManager::init(SceneManager *SceneMgr)
{
  mSceneMgr = SceneMgr;
  mParentNode = mSceneMgr->getRootSceneNode();

  string strType, strTemp, strMesh;
  int i=0;
  while(1)
  {
    if (!(Option::getSingleton().openDescFile(FILE_WORLD_DESC)))
    {
      Logger::log().info() << "Parse description file " << FILE_WORLD_DESC << ".";
      return false;
    }
    if (!(Option::getSingleton().getDescStr("Type", strType, ++i))) break;
    Option::getSingleton().getDescStr("MeshName", strMesh,i);
    Option::getSingleton().getDescStr("StartX", strTemp,i);
    Real posX = atof(strTemp.c_str());
    Option::getSingleton().getDescStr("StartY", strTemp,i);
    Real posY = atof(strTemp.c_str());
    Option::getSingleton().getDescStr("StartZ", strTemp,i);
    Real posZ = atof(strTemp.c_str());
    Option::getSingleton().getDescStr("Facing", strTemp);
    float facing = atof(strTemp.c_str());

    if (strType == "npc")
    {
      addObject(OBJECT_NPC, strMesh.c_str(), Vector3(posX,posY,posZ), facing);
    }
    else
    {
      addObject(OBJECT_STATIC, strMesh.c_str(), Vector3(posX,posY,posZ), facing);
    }
  }
  return true;
}

//=================================================================================================
//
//=================================================================================================
bool ObjectManager::addObject(unsigned int type, const char *desc_filename, Vector3 pos, float facing)
{
  static int id= -1;
  mDescFile = PATH_MODEL_DESCRIPTION;
  mDescFile += desc_filename;

  Logger::log().info()  << "Adding object from file " << mDescFile << "...";
  if(!Option::getSingleton().openDescFile(mDescFile.c_str()))
  {
    Logger::log().success(false);
    Logger::log().error() << "CRITICAL: description file was not found!";
    return false;
  }
  Logger::log().success(true);

  string strTemp;
  switch (type)
  {
    case OBJECT_STATIC:
      {
        // For static objects we don't use *.desc files. So we get just the mesh name here.
        strTemp = desc_filename;
        Entity  *entity = mSceneMgr->createEntity("Object_"+StringConverter::toString(++id), strTemp.c_str());
        mNode = mParentNode->createChildSceneNode(pos, Quaternion(1.0,0.0,0.0,0.0));
        mNode->attachObject(entity);
        mvObject_static.push_back(entity);
        //            node->setScale(5, 5, 5);
        break;
      }
    case OBJECT_NPC:
      {
        mNode = mParentNode->createChildSceneNode(pos, Quaternion(1.0,0.0,0.0,0.0));
        NPC *npc = new NPC(mSceneMgr, mNode, desc_filename, facing);
        mvObject_npc.push_back(npc);
        break;
      }
    default:
      break;
  }
  return true;
}

//=================================================================================================
//
//=================================================================================================
void ObjectManager::update(int obj_type, const FrameEvent& evt)
{
  switch (obj_type)
  {
    case OBJECT_STATIC:
      break;
    case OBJECT_PLAYER:
    case OBJECT_NPC:
      {
        for (unsigned int i = 0; i < mvObject_npc.size(); ++i)
        {
          mvObject_npc[i]->update(evt);
        }
        break;
      }
    default:
      break;
  }
}

//=================================================================================================
// JUST FOR TESTING.
//=================================================================================================
void ObjectManager::Event(int obj_type, int action, int val1, int val2)
{
  switch (obj_type)
  {
    case OBJECT_STATIC:
      break;
    case OBJECT_PLAYER:
      {
        if (action == OBJ_WALK     ) mvObject_npc[0]->walking(val1);
        if (action == OBJ_TURN     ) mvObject_npc[0]->turning(val1);
//        if (action == OBJ_TEXTURE  ) mvObject_npc[0]->toggleTexture(val1, val2);
        if (action == OBJ_ANIMATION) mvObject_npc[0]->toggleAnimation(val1);
        if (action == OBJ_GOTO     ) mvObject_npc[0]->moveToTile(val1, val2);
      }
      break;
    case OBJECT_NPC:
      {
        for(unsigned int i = 1; i < mvObject_npc.size(); ++i)
        {
          if (action == OBJ_WALK     ) mvObject_npc[i]->walking(val1);
          if (action == OBJ_TURN     ) mvObject_npc[i]->turning(val1);
//          if (action == OBJ_TEXTURE  ) mvObject_npc[i]->toggleTexture(val1, val2);
          if (action == OBJ_ANIMATION) mvObject_npc[i]->toggleAnimation(val1);
        }
      }
      break;
    default:
      break;
  }
}

//=================================================================================================
//
//=================================================================================================
void ObjectManager::delObject(int )
{}

//=================================================================================================
//
//=================================================================================================
ObjectManager::~ObjectManager()
{
  for (unsigned int i = 0; i < mvObject_npc.size(); ++i)
  {
    delete mvObject_npc[i];
  }
}
