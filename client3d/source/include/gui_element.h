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

#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include <string>
#include <tinyxml.h>
#include <Ogre.h>

using namespace Ogre;

class GuiElement
{
  friend class GuiGadget;
  friend class GuiGraphic;
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiElement(TiXmlElement *xmlElement, int wt, int ht, int maxXt, int maxYt);
  virtual ~GuiElement()
  {
  }

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
  int getState()
  {
    return mState;
  }
  void setStateImagePos(std::string state, int x, int y);
  virtual void draw(PixelBox &mSrcPixelBox, Texture *texture) =0;
  const char *getTooltip()
  {
    return mStrTooltip.c_str();
  }
  enum
  {
    STATE_STANDARD, STATE_PUSHED, STATE_M_OVER, STATE_PASSIVE, STATE_SUM
  };


protected:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  struct _pos
  {
    int x, y;
  }
  gfxSrcPos[STATE_SUM];

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
