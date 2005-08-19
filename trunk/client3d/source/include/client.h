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

#ifndef CLIENT_H
#define CLIENT_H

#include "event.h"
#include "TileChunk.h"
#include "TileManager.h"

using namespace Ogre;

extern Camera *mCamera;
////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// None Singleton class.
////////////////////////////////////////////////////////////
class DaimoninClient
{
public:
	////////////////////////////////////////////////////////////
	// Variables.
	////////////////////////////////////////////////////////////
	SceneNode *MouseCursor;
	SceneNode *World;

	////////////////////////////////////////////////////////////
	// Functions.
	////////////////////////////////////////////////////////////
	 DaimoninClient() {;}
	~DaimoninClient() {;}
	void go(void);

private:
	////////////////////////////////////////////////////////////
	// Variables.
	////////////////////////////////////////////////////////////
	Viewport            *mVP;
	Root                *mRoot;

	MouseMotionListener *mMouseMotionListener;
	MouseListener       *mMouseListener;
	SceneManager        *mSceneMgr;
	InputReader         *mInputReader;
	RenderWindow        *mWindow;
	TileManager         *mTileManager;

	////////////////////////////////////////////////////////////
	// Functions.
	////////////////////////////////////////////////////////////
	// These internal methods package up the stages in the startup process
	// Sets up the application - returns false if the user chooses to abandon configuration.
	bool setup(void);
	/// Method which will define the source of resources (other than current folder)
	void setupResources(void);
	void createScene(void);
	void destroyScene(void);
}; 

#endif
