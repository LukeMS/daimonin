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

#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

#include <vector>
#include <Ogre.h>
#include "gui_gadget.h"
#include "gui_graphic.h"
#include "gui_window.h"
#include "gui_manager.h"

class GuiManager;

using namespace Ogre;

////////////////////////////////////////////////////////////
/// Defines.
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Class.
////////////////////////////////////////////////////////////
class GuiWindow
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  ~GuiWindow();
  GuiWindow(TiXmlElement *xmlElem, GuiManager *guiManager);
  void keyEvent(int obj_type, int action, int val1=0, int val2=0);
  const char *getName()
  {
    return mStrName.c_str();
  }

  const char *mouseEvent(int MouseAction, int x, int y);
private:
  enum
  {
    STATE_STANDARD, STATE_PUSHED, STATE_M_OVER, STATE_PASSIVE, STATE_SUM
  };
  struct _textLine
  {
    int x, y, size;
    std::string text;
  };
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  static int msInstanceNr, mMouseDragging;
  int mWindowNr;
  int mMousePressed, mMouseOver;
  Image mTileImage;
  //Real mRatioW, mRatioH;
  int mPosX, mPosY, mPosZ, mWidth, mHeight;
  int mDragPosX1, mDragPosX2, mDragPosY1, mDragPosY2, mDragOldMousePosX, mDragOldMousePosY;
  bool mSizeRelative;
  PixelBox mSrcPixelBox;
  SceneManager *mSceneMgr;
  SceneNode *mParentNode, *mNode;
  std::string mStrName, mStrTooltip;
  std::string mStrImageSetGfxFile,  mStrFont, mStrXMLFile;
  std::vector<GuiGadget *>mvGadget;
  std::vector<GuiGraphic*>mvGraphic;
  std::vector<_textLine *>mvTextline;
  Overlay *mOverlay;
  OverlayElement *mElement;
  MaterialPtr mMaterial;
  TexturePtr mTexture;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiWindow(const GuiWindow&); // disable copy-constructor.
  int getGadgetMouseIsOver(int x, int y);
  void createWindow();
  void delGadget(int number);
  void drawAll();
  void parseWindowData(TiXmlElement *xmlElem, GuiManager *guiManager);
};

#endif
