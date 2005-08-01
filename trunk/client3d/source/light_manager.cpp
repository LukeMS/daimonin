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
#include "light_manager.h"
#include "option.h"
#include "logfile.h"

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================


//=================================================================================================
// Init the model from the description file.
//=================================================================================================
bool LightManager::init(SceneManager *SceneMgr, SceneNode *Node)
{
}

//=================================================================================================
//
//=================================================================================================
bool LightManager::addObject(unsigned int type, const char *desc_filename, Vector3 pos)
{
}

//=================================================================================================
//
//=================================================================================================
void LightManager::update(int obj_type, const FrameEvent& evt)
{
}

//=================================================================================================
// JUST FOR TESTING.
//=================================================================================================
void LightManager::keyEvent(int obj_type, int action, int val1, int val2)
{
}

//=================================================================================================
//
//=================================================================================================
void LightManager::delObject(int number)
{
}

LightManager::~LightManager()
{
}
