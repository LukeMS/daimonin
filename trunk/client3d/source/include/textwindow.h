/*
-----------------------------------------------------------------------------
This source file is part of Daimonin (http://daimonin.sourceforge.net)

Copyright (c) 2005 The Daimonin Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
 
#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include <Ogre.h>
#include <string>
#include <vector>
#include "xyz.h"

using namespace Ogre;

////////////////////////////////////////////////////////////
// Defines.
////////////////////////////////////////////////////////////
const Real CLOSING_SPEED =   10.0f;
const Real SCROLL_SPEED  =    0.5f;
const Real FONT_SIZE     =   16.0f;
const int  MAX_TEXT_LINES=   12;

class Textwindow
{
  public:
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
    static Textwindow &getSingelton();
    Textwindow();
    ~Textwindow();
    bool Init();
    void Update();
	void setVisible(bool show);
	void OpenTextWin();
	void CloseTextWin();
	bool MouseAction(int action, Real xpos, Real pos, Real yRelative = 0);
    void addText(const char *newTextLine);

  private:
    ////////////////////////////////////////////////////////////
	// Functions.
    ////////////////////////////////////////////////////////////
	Textwindow(const Textwindow&); // disable copy-constructor.
    inline void Scrolling();
	void SizeChange();

    ////////////////////////////////////////////////////////////
	// Variables.
    ////////////////////////////////////////////////////////////
	std::vector<std::string> mvLine;
    Overlay        *mOverlay;
	OverlayElement *mElementFrame, *mElementTitle;
	OverlayElement *mElementButUp, *mElementButDown;
	OverlayElement *mElementLine[MAX_TEXT_LINES]; 
	Real mScrollOffset;          // Text scrolling offset 
	Real mClose;                 // Only Headline visible.
    Real mLastTopPos;            // The pos before window was closed.   
    Real mMinHeight, mMaxHeight;
	bool mIsClosing, mIsOpening; // User pressed open/close.
    bool mVisible;
	bool mDragging;
    int  mRowsToScroll;
	int  sumRows;
};

#endif
