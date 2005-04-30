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

#include "define.h"
#include "object_manager.h"
#include "option.h"
#include "logfile.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================


//=================================================================================================
// Init the model from the description file.
//=================================================================================================
bool ObjectManger::init(SceneManager *SceneMgr, SceneNode *Node)
{
	mSceneMgr = SceneMgr; 
	mNode = Node;
	return true; 
}

//=================================================================================================
// 
//=================================================================================================
bool ObjectManger::addObject(unsigned int type, const char *desc_filename, Vector3 pos)
{
    static int id= -1;
    mDescFile = DIR_MODEL_DESCRIPTION;
    mDescFile += desc_filename;
	LogFile::getSingleton().Info("Adding object from file %s...", mDescFile.c_str());
	if (!(Option::getSingleton().openDescFile(mDescFile.c_str())))
	{
		LogFile::getSingleton().Success(false);
		LogFile::getSingleton().Error("CRITICAL: description file was not found!\n");
		return false;
	}
	LogFile::getSingleton().Success(true);
	string strTemp;

    switch (type)
    {
        case OBJECT_STATIC:
        {
            // For static objects we don't use *.desc files. So we get just the mesh name here.
        //	Option::getSingleton().getDescStr("MeshName", strTemp);
            strTemp = desc_filename;
            Entity  *entity = mSceneMgr->createEntity("Object_"+StringConverter::toString(++id), strTemp.c_str());
            SceneNode *node = mNode->createChildSceneNode(Vector3(pos.x, pos.y, pos.z), Quaternion(1.0,0.0,0.0,0.0));
            node->attachObject(entity);
            mvObject_static.push_back(entity);
            break;
        }
        case OBJECT_NPC:
        {
           NPC *npc = new NPC(mSceneMgr, mNode, desc_filename);
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
void ObjectManger::update(int obj_type, const FrameEvent& evt)
{
    switch (obj_type)
    {
        case OBJECT_STATIC:
            break;
        case OBJECT_NPC:
            for (unsigned int i = 0; i < mvObject_npc.size(); ++i) { mvObject_npc[i]->update(evt); }
            break;
        default:
            break;
    }
}

//=================================================================================================
// JUST FOR TESTING.
//=================================================================================================
void ObjectManger::keyEvent(int obj_type, int action, int val1, int val2)
{
    switch (obj_type)
    {
        case OBJECT_STATIC:
        break;
        case OBJECT_NPC:
            for(unsigned int i = 0; i < mvObject_npc.size(); ++i)
            {
                if (action == OBJ_WALK     ) mvObject_npc[i]->walking(val1);
                if (action == OBJ_TURN     ) mvObject_npc[i]->turning(val1);
                if (action == OBJ_TEXTURE  ) mvObject_npc[i]->toggleTexture(val1, val2);
                if (action == OBJ_ANIMATION) mvObject_npc[i]->toggleAnimation(val1);                                                
            }
        break;
        default:
        break;
    }
}

//=================================================================================================
// 
//=================================================================================================
void ObjectManger::delObject(int number)
{
}

ObjectManger::~ObjectManger()
{
    for (unsigned int i = 0; i < mvObject_npc.size(); ++i) { delete mvObject_npc[i]; }
}
