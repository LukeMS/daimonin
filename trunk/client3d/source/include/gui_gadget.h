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

#ifndef GUI_GADGET_H
#define GUI_GADGET_H

#include <string>
#include <tinyxml.h>
#include <Ogre.h>

using namespace Ogre;

class GuiGadget
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiGadget(TiXmlElement *xmlElem, int w, int h, int maxX, int maxY);
  ~GuiGadget()
  {}
  bool mouseOver(int x, int y)
  {
    if (x >= mX && x <= mX + mWidth && y >= mY && y <= mY + mHeight) return true;
    return false;
  }
  bool setState(int state)
  {
    if (mState == state) return false;
    mState = state;
    return true;
  }
  const char *getName()
  {
    return mStrName.c_str();
  }
  int getState()
  {
    return mState;
  }
  void setStateImagePos(std::string state, int x, int y);
  void draw(PixelBox &mSrcPixelBox, Texture *texture);
  const char *getTooltip()
  {
    return mStrTooltip.c_str();
  }

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  enum
  {
    TYPE_GFX, TYPE_BUTTON, TYPE_BUTTON_CHECK, TYPE_BUTTON_RADIO, TYPE_SLIDER, TYPE_SUM
  };
  enum
  {
    STATE_STANDARD, STATE_PUSHED, STATE_M_OVER, STATE_PASSIVE, STATE_SUM
  };
  struct _pos
  {
    int x, y;
  };
  struct _pos gfxSrcPos[STATE_SUM];

  bool mMirrorH, mMirrorV;
  int  mX, mY, mWidth, mHeight;
  int  mType;
  std::string mStrName, mStrLabel, mStrBgLabel, mBehavior, mStrTooltip;
  unsigned char mLabelColor[3];
  int mLabelFont;
  int mLabelXPos, mLabelYPos;
  int  mOldState, mState;

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
};

#endif
