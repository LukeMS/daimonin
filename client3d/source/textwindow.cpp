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

#include "textwindow.h"
#include "logfile.h"

//=================================================================================================
// Constructor.
//=================================================================================================
Textwindow::Textwindow()
{
}

//=================================================================================================
// Destructor.
//=================================================================================================
Textwindow::~Textwindow()
{
}

//=================================================================================================
// Return the instance.
//=================================================================================================
Textwindow &Textwindow::getSingelton()
{
   static Textwindow singelton;
   return singelton;
}

//=================================================================================================
// Init all elements.
//=================================================================================================
bool Textwindow::Init()
{
    mOverlay        = OverlayManager::getSingleton().getByName("TextWindowOverlay");
    mElementFrame   = OverlayManager::getSingleton().getOverlayElement("TextWindow/Frame");
    mElementTitle   = OverlayManager::getSingleton().getOverlayElement("TextWindow/Headline");
    mElementButDown = OverlayManager::getSingleton().getOverlayElement("TextWindow/Button01");
    mElementButUp   = OverlayManager::getSingleton().getOverlayElement("TextWindow/Button02");
    //
	// TODO: Make an Overlay template.
    std::string name = "TextWindow/Line_";
    for (int i=0; i < MAX_TEXT_LINES; ++i)
	{
         mElementLine[i]= OverlayManager::getSingleton().getOverlayElement(name + StringConverter::toString(i));
         mElementLine[i]->setCaption(name + StringConverter::toString(i));
    }
    SizeChange();
    mMinHeight    = -mElementTitle->getHeight();
	mMaxHeight    = -mElementLine[0]->getHeight() * MAX_TEXT_LINES;
	sumRows       = mElementFrame->getHeight() / FONT_SIZE -1;
    mClose        = false;
    mIsClosing    = false;
    mScrollOffset = 0.0f;
	mRowsToScroll = 0;



mRowsToScroll =8;



    setVisible(true);
	return true;
}


//=================================================================================================
// Show/Hide the Overlay.
//=================================================================================================
void Textwindow::setVisible(bool show)
{
    if (show) { mOverlay->show(); }
    else      { mOverlay->hide(); }
}

//=================================================================================================
// Closes the TextWindow.
//=================================================================================================
void Textwindow::CloseTextWin()
{
	mLastTopPos = mElementFrame->getTop();
	mIsClosing  = true;
	mIsOpening  = false;
}

//=================================================================================================
// opens the TextWindow.
//=================================================================================================
void Textwindow::OpenTextWin()
{
	mIsOpening  = true;
	mIsClosing  = false;
}

//=================================================================================================
// All drawings are done here.
//=================================================================================================
void Textwindow::Update()
{
    ///////////////////////////////////////////////////////////////////////// 
    // User pressed the close-button.
	/////////////////////////////////////////////////////////////////////////
    if (mIsClosing)
	{
        Real top = mElementFrame->getTop() + CLOSING_SPEED;
		if (top >= mMinHeight)
		{
			top = mMinHeight;
            mIsClosing = false;
		}
		mElementFrame->setTop(top);
		mElementFrame->setHeight(56-top);
	}

    ///////////////////////////////////////////////////////////////////////// 
    // User pressed the open-button.
	/////////////////////////////////////////////////////////////////////////
    if (mIsOpening)
	{
        Real top = mElementFrame->getTop() - CLOSING_SPEED;
		if (top <= mLastTopPos)
		{
			top = mLastTopPos;
            mIsOpening = false;
		}
		mElementFrame->setTop(top);
		mElementFrame->setHeight(56-top);
	}

	///////////////////////////////////////////////////////////////////////// 
    // Scroll the text.
	/////////////////////////////////////////////////////////////////////////
    Scrolling();
}

//=================================================================================================
// Mouse action was reported.
//=================================================================================================
bool Textwindow::MouseAction(int action, Real xpos, Real ypos, Real yRelative)
{
	if (action == M_RELEASED) { mDragging = false; return true; }
	// Was the mouse action this Overlay?
    if (!mElementFrame-> contains(xpos, ypos) && !mDragging)   { return true;} 

    // Check all buttons.
	if (mElementButUp  ->contains(xpos, ypos)) { OpenTextWin (); return true; }
    if (mElementButDown->contains(xpos, ypos)) { CloseTextWin(); return true; }

    // Check for resize.
	if ((mElementTitle  ->contains(xpos, ypos) && action == M_DRAGGED) || mDragging) 
	{   
        bool ret  = true;
        mDragging = true;
        Real top = mElementFrame->getTop() +  yRelative;
		if      (top <= mMaxHeight) { top = mMaxHeight; ret = false; }
		else if (top >= mMinHeight) { top = mMinHeight; ret = false; }
        mElementFrame->setTop(top);
        mElementFrame->setHeight(-top);
		SizeChange();
        return ret;
	}
    return true;
}

//=================================================================================================
// Add a line of text.
//=================================================================================================
void Textwindow::addText(const char *text)
{
	std::string newTextLine(text);
	mvLine.push_back(newTextLine);
	mRowsToScroll++;
}

//=================================================================================================
// Show the text.
//=================================================================================================
void Textwindow::Scrolling()
{
	static int scroll = FONT_SIZE / SCROLL_SPEED;
    if (!mRowsToScroll) { return; }

    // Beginn a scroll.
    for (int i=0; i <= sumRows && i < MAX_TEXT_LINES; ++i)
        mElementLine[i]->setTop(mElementLine[i]->getTop() - SCROLL_SPEED);
    if (--scroll ==0)
	{
        --mRowsToScroll;
        scroll = FONT_SIZE / SCROLL_SPEED;
        for (int i=0; i <= sumRows && i < MAX_TEXT_LINES; ++i)
            mElementLine[i]->setTop(mElementLine[i]->getTop() + FONT_SIZE);
		// ++mScrollOffset;
	}
}

//=================================================================================================
// Show the text.
//=================================================================================================
void Textwindow::SizeChange()
{
    Real yPos = mElementFrame->getHeight();
    sumRows = yPos / FONT_SIZE;

    int i=0;
    for (; i <= sumRows && i < MAX_TEXT_LINES; ++i)
	{
        mElementLine[i]->show();
        mElementLine[i]->setTop(yPos);
        yPos -= FONT_SIZE;
	}
    for (; i < MAX_TEXT_LINES; ++i)
	{
        mElementLine[i]->hide();
	}
}
