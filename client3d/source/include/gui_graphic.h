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

#ifndef GUI_GRAPHIC_H
#define GUI_GRAPHIC_H

#include <string>
#include <tinyxml.h>
#include <Ogre.h>

using namespace Ogre;

class GuiGraphic
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiGraphic(TiXmlElement *xmlElem);
  ~GuiGraphic()
  {}
  bool mouseOver(int x, int y)
  {
    if (x >= mX && x <= mX + mDestWidth && y >= mY && y <= mY + mDestHeight) return true;
    return false;
  }
  void setSize(int w, int h)
  {
    mSrcWidth = w;
    mSrcHeight= h;
  }
  bool setState(int state)
  {
    if (mState == state) return false;
    mState = state;
    return true;
  }
  int getState()
  {
    return mState;
  }
  void setStateImagePos(std::string state, int x, int y);
  void draw(PixelBox &mSrcPixelBox, Texture *texture);

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  enum
  {
    FILL_GFX, FILL_COLOR, FILL_NONE, FILL_SUM
  };
  struct _pos
  {
    int x, y;
  }
  gfxSrcPos;

  int  mX, mY, mSrcWidth, mSrcHeight, mDestWidth, mDestHeight;
  int  mType;
  std::string mStrName, mStrLabel, mBehavior;
  uint32 mFillColor;
  int mLabelXPos, mLabelYPos;
  int  mOldState, mState;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
};

#endif
