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

#ifndef GUI_LISTBOX_H
#define GUI_LISTBOX_H

#include <string>
#include <tinyxml.h>
#include <Ogre.h>

using namespace Ogre;

class GuiListbox
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  GuiListbox(TiXmlElement *xmlElem, int maxX, int maxY);
  ~GuiListbox()
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
  void print(const char *text);
private:
static const Real CLOSING_SPEED      =  10.0f;  // default: 10.0f
static const Real SCROLL_SPEED       =   1.0f;  // default:  1.0f
static const int  MAX_TEXT_LINES     =  20;
static const int  SIZE_STRING_BUFFER = 128;     // MUST be 2^X.

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

  struct _row
  {
    std::string str;
    //    ColourValue colorTop;
    //    ColourValue colorBottom;
  }
  row[SIZE_STRING_BUFFER];


  Real mClose;                 // Only Headline visible.
  Real mLastHeight;            // The height before window was closed.
  Real mMinHeight, mMaxHeight;
  Real mFirstYPos;
  Real mScroll;
  bool mIsClosing, mIsOpening; // User pressed open/close button.
  bool mVisible;
  bool mDragging;
  int  mThisWindowNr;
  int  mRowsToScroll, mRowsToPrint;
  int  mSumRows;
  int  mPrintPos;
  int  mBufferPos;

  int  mX, mY, mWidth, mHeight;
  int  mType;
  std::string mStrName, mStrLabel, mBehavior;
  uint32 mFillColor;
  int  mOldState, mState;
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
};

#endif
