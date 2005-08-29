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

#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include <Ogre.h>
#include <OgreTextAreaOverlayElement.h>
#include <string>
#include "define.h"

using namespace Ogre;

////////////////////////////////////////////////////////////
/// Defines.
////////////////////////////////////////////////////////////
const Real CLOSING_SPEED      =  10.0f;  // default: 10.0f
const Real SCROLL_SPEED       =   1.0f;  // default:  1.0f
const Real FONT_SIZE          =  16.0f;  // default: 16.0f
const int  MAX_TEXT_LINES     =  20;
const int  SIZE_STRING_BUFFER = 128;     // MUST be 2^X.

const ColourValue TXT_RED = ColourValue(1, 0, 0);
const ColourValue TXT_YELLOW= ColourValue(1, 1, 0);
const ColourValue TXT_GREEN = ColourValue(0, 1, 0);
const ColourValue TXT_BLUE = ColourValue(0, 0, 1);
const ColourValue TXT_WHITE = ColourValue(1, 1, 1);
const ColourValue TXT_GRAY1 = ColourValue(.7, .7, .7);

////////////////////////////////////////////////////////////
/// Class.
////////////////////////////////////////////////////////////
class CTextwindow
{
public:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  CTextwindow(std::string name, Real Xpos, Real height, int ScreenHeight, bool visible = true);
  ~CTextwindow()
  {}
  void Update();
  void Init();
  void setVisible(bool show);
  void OpenTextWin();
  void CloseTextWin();
  bool MouseAction(int action, Real xpos, Real pos);
  void Print(const char *newTextLine, ColourValue = TXT_GREEN);
  void setChild(CTextwindow *Child);
  void setDimension(Real x, Real y, Real w, Real h);

private:
  ////////////////////////////////////////////////////////////
  /// Functions.
  ////////////////////////////////////////////////////////////
  inline void Scrolling();
  inline void DockChild();
  void SizeChanged();

  ////////////////////////////////////////////////////////////
  /// Variables.
  ////////////////////////////////////////////////////////////
  struct _row
  {
    std::string str;
    ColourValue colorTop;
    ColourValue colorBottom;
  }
  row[SIZE_STRING_BUFFER];
  static int mInstanceNr;
  static int  mDragWinNr;
  static int  mScreenHeight;
  CTextwindow *mChild, *mParent;
  Overlay *mOverlay;
  OverlayContainer *mContainerFrame;
  OverlayElement   *mElementTitle, *mElementTitleTxt0, *mElementTitleTxt1;
  OverlayElement   *mElementButUp, *mElementButDown;
  TextAreaOverlayElement   *mElementLine[MAX_TEXT_LINES];
  Real mClose;                 // Only Headline visible.
  Real mLastHeight;            // The height before window was closed.
  Real mMinHeight, mMaxHeight;
  Real mFirstYPos;
  Real mScroll;
  bool mIsClosing, mIsOpening; // User pressed open/close button.
  bool mVisible;
  bool mDragging;
  int  mThisWindowNr;
  int  mRowsToScroll;
  int  mSumRows;
  int  mPrintPos;
  int  mBufferPos;
};

extern CTextwindow *ChatWin, *TextWin;

#endif
