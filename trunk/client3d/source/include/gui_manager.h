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

#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <string>
#include <vector>
#include <Ogre.h>
#include <tinyxml.h>
#include "gui_cursor.h"
#include "gui_window.h"

using namespace Ogre;

class GuiWindow;

class GuiManager
{
public:
  struct _state
  {
    std::string name;
    short x, y;
  };
  struct mSrcEntry
  {
    std::string name;
    int width, height;
    std::vector<_state*>state;
  };
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiManager(const char *XML_imageset_file, const char *XML_windows_file, int w, int h);
  ~GuiManager()
  {}
  GuiCursor *getMouseCursor()
  {
    return mMousecursor;
  }
  struct mSrcEntry *getStateGfxPositions(const char* guiImage);
  PixelBox &getTilesetPixelBox()
  {
    return mSrcPixelBox;
  }
  void getScreenDimension(int &ScreenWidth, int &ScreenHeight)
  {
    ScreenWidth  = mScreenWidth;
    ScreenHeight = mScreenHeight;
  }
  const char *mouseEvent(int MouseAction, Real rx, Real ry);
private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  std::string mStrTmp;
  std::string mStrImageSetGfxFile;
  std::vector<mSrcEntry*>mvSrcEntry;
  std::vector<GuiWindow*>mvWindow;
  int mMouseDragging, mMousePressed, mMouseOver;
  unsigned int mScreenWidth, mScreenHeight;
  PixelBox mSrcPixelBox;
  Image mImageSetImg;
  GuiCursor *mMousecursor;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  bool parseImagesetData(const char *file);
  bool parseWindowsData (const char *file);

};

#endif
