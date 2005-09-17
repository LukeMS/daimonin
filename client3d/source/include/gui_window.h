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
  GuiWindow(unsigned int w, unsigned int h, const char* Name);
  ~GuiWindow();
  void keyEvent(int obj_type, int action, int val1=0, int val2=0);
  const char *mouseEvent(int MouseAction, Real x, Real y);
  /*
    OverlayElement *getGuiElement() const
    {
      return mElement;
    }
  */
private:
  enum
  {
    STATE_STANDARD, STATE_PUSHED, STATE_M_OVER, STATE_SUM
  };

  struct spos
  {
    short x, y;
  };
  struct mSrcEntry
  {
    std::string name;
    bool drawAndForget; // Don't need event handling. Will be destroyed after first draw.
    unsigned short width, height;
    struct spos pos[STATE_SUM];
  };

  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  static unsigned int msInstanceNr;
  unsigned int mThisWindowNr;
  std::vector<mSrcEntry*>mvSrcEntry;
  int mMouseDragging, mMousePressed, mMouseOver;
  unsigned int mScreenWidth, mScreenHeight;
  Image mTileImage;
  int mPosZ;
  bool mPosRelative;
  Real mRatioW, mRatioH;
  int mPosX, mPosY, mWidth, mHeight;
  bool mSizeRelative;
  bool mMoveable;
  TexturePtr mTexture;
  PixelBox mSrcPixelBox;
  SceneManager *mSceneMgr;
  SceneNode *mParentNode, *mNode;
  std::string mStrTooltip;
  std::string mStrImageSet, mStrImageSetGfxFile,  mStrFont, mStrXMLFile;
  std::vector<GuiGadget*>mvGadget;
  OverlayElement *mElement;
  MaterialPtr mMaterial;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiWindow(const GuiWindow&); // disable copy-constructor.
  int getGadgetMouseIsOver(int x, int y);
  void createWindow();
  void addGadget();
  void delGadget(int number);
  void drawGadget(int gadgetNr);
  void drawAll();
  bool parseImagesetData();
  void parseWindowData(const char *windowFile);
};

#endif
