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

class GuiGadget
{
public:
  enum
  {
    TYPE_GFX, TYPE_BUTTON, TYPE_BUTTON_CHECK, TYPE_BUTTON_RADIO, TYPE_SLIDER, TYPE_SUM
  };
  struct sPos
  {
    int x, y;
  };
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiGadget(const char *XMLdescFile, int pos);
  ~GuiGadget()
  {}
  void getPos(int &x1, int &y1, int &x2, int &y2);
  bool mouseOver(int x, int y)
  {
    if (x >= mX1 && x <= mX2 && y >= mY1 && y <= mY2) return true;
    return false;
  }
  bool isAttached()
  {
    return mAttached;
  }
  int getType()
  {
    return mType;
  }
  const std::string &getName()
  {
    return mName;
  }
  int getTilsetPos()
  {
    return mTilsetPos;
  }
  void setTilsetPos(int pos)
  {
    mTilsetPos =pos;
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

private:
  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  int  mTilsetPos; // pos in the tileset vector.
  int  mX1, mX2, mY1, mY2;
  int  mType;
  std::string mName;
  int  mOldState;
  int  mState;
  bool mDrawOnlyOnce;
  bool mAttached;

  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
};

#endif

