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
#include "sound.h"

CTextwindow *ChatWin=0, *TextWin=0;

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
int CTextwindow::mInstanceNr = 0;

//=================================================================================================
// Constructor.
//=================================================================================================
CTextwindow::CTextwindow(std::string title, Real Xpos, Real height, bool visible)
{
    ///////////////////////////////////////////////////////////////////////// 
    // Clone all OverlayElemnts from templates.
	/////////////////////////////////////////////////////////////////////////
	std::string name= StringConverter::toString(mInstanceNr)+"_TextWindow/";    
    mOverlay        = OverlayManager::getSingleton().create(name + "Overlay");
    mOverlay->setZOrder(510-mInstanceNr);

	mContainerFrame = static_cast<OverlayContainer*>(OverlayManager::getSingleton().
		cloneOverlayElementFromTemplate("TextWindow/Frame", name + "Frame"));
	mOverlay->add2D(mContainerFrame);
    for (int i=0; i < MAX_TEXT_LINES; ++i)
	{
         mElementLine[i]= static_cast<TextAreaOverlayElement*>
    (OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/TextRow",name+"Line_"+ StringConverter::toString(i)));
		 mElementLine[i]->setCaption("");
         mContainerFrame->addChild(mElementLine[i]);
	}
	mElementTitle   = OverlayManager::getSingleton().
		cloneOverlayElementFromTemplate("TextWindow/Headline",name + "Title");
    mElementButUp   = OverlayManager::getSingleton().
		cloneOverlayElementFromTemplate("TextWindow/ButtonUp",name + "ButtopUp");
	mElementButDown = OverlayManager::getSingleton().
		cloneOverlayElementFromTemplate("TextWindow/ButtonDown",name + "ButtopDown");
    mContainerFrame->addChild(mElementTitle);
    mContainerFrame->addChild(mElementButUp);
    mContainerFrame->addChild(mElementButDown);
    mContainerFrame->setLeft(Xpos);
    mElementTitle ->setWidth(-Xpos-mElementButUp->getWidth()*2);   
    mElementButUp  ->setLeft(-Xpos-mElementButUp->getWidth()*2);
    mElementButDown->setLeft(-Xpos-mElementButUp->getWidth());
	// WindowTitle.
	mElementTitleTxt0= OverlayManager::getSingleton().
		cloneOverlayElementFromTemplate("TextWindow/TitleText", name+"Title0");
	mElementTitleTxt0->setCaption(title);
	mElementTitleTxt0->setTop(3);
	mElementTitleTxt0->setLeft(5);
    mElementTitleTxt0->setColour(ColourValue(.4,.4,.4));
	static_cast<OverlayContainer*>(mElementTitle)->addChild(mElementTitleTxt0);
	 mElementTitleTxt1= OverlayManager::getSingleton().
		cloneOverlayElementFromTemplate("TextWindow/TitleText", name+"Title1");
	mElementTitleTxt1->setCaption(title);
	static_cast<OverlayContainer*>(mElementTitle)->addChild(mElementTitleTxt1);
 	++mInstanceNr;

    ///////////////////////////////////////////////////////////////////////// 
    // Init all variables.
	/////////////////////////////////////////////////////////////////////////
	mChild        = 0;
    mParent       = 0;

    mMinHeight    = -mElementTitle->getHeight();
	mMaxHeight    = -FONT_SIZE * MAX_TEXT_LINES;
    if (-height < mMaxHeight) height = mMaxHeight;
    if (!mParent)  { mContainerFrame->setTop(-height); }
    else           { mContainerFrame->setTop(-height+mParent->mContainerFrame->getTop() ); }
	SizeChanged();
	mLastHeight   =  mContainerFrame->getHeight();
	for (int j = 0; j < SIZE_STRING_BUFFER; ++j) { row[j].str = ""; }
	mIsClosing		= false;
	mIsOpening		= false;
	mDragging		= false;
	mBufferPos		= 0;
	mPrintPos		= 0;
	mRowsToScroll	= 0;
	mScroll			= 0.0f;
	setVisible(visible);

}

//=================================================================================================
// Destructor.
//=================================================================================================
CTextwindow::~CTextwindow()
{
}

//=================================================================================================

//=================================================================================================
void CTextwindow::Init()
{
}

//=================================================================================================
// Show/Hide the Overlay.
//=================================================================================================
void CTextwindow::setVisible(bool show)
{
    if (show) { mOverlay->show(); }
    else      { mOverlay->hide(); }
}

//=================================================================================================
// Closes the TextWindow.
//=================================================================================================
void CTextwindow::CloseTextWin()
{
    mLastHeight = mContainerFrame->getHeight();
	mIsClosing  = true;
	mIsOpening  = false;
}

//=================================================================================================
// opens the TextWindow.
//=================================================================================================
void CTextwindow::OpenTextWin()
{
	mIsOpening  = true;
	mIsClosing  = false;
}

//=================================================================================================
// All drawings are done here.
//=================================================================================================
void CTextwindow::Update()
{
    ///////////////////////////////////////////////////////////////////////// 
    // User pressed the close-button.
	/////////////////////////////////////////////////////////////////////////
    if (mIsClosing)
	{
        Real top = mContainerFrame->getTop() + CLOSING_SPEED;
        if (!mParent)
        {
            if (top >= mMinHeight) { top = mMinHeight; mIsClosing = false; }
        }
        else
        {
            if (top - mParent->mContainerFrame->getTop()  >= mMinHeight)
            {
			    top = mParent->mContainerFrame->getTop()+mMinHeight;;
                mIsClosing = false;
            }
        }
        mContainerFrame->setTop(top);
        SizeChanged();
        return;
	}

    ///////////////////////////////////////////////////////////////////////// 
    // User pressed the open-button.
	/////////////////////////////////////////////////////////////////////////
    if (mIsOpening)
	{
 		Real top = mContainerFrame->getHeight() + CLOSING_SPEED;
        if (top > mLastHeight)
        {
            top = mLastHeight;
            mIsOpening = false;
        }
        if (!mParent)
            mContainerFrame->setTop(-top);
        else
            mContainerFrame->setTop(mParent->mContainerFrame->getTop()-top);
        SizeChanged();
        return;
	}

	///////////////////////////////////////////////////////////////////////// 
    // Scroll the text.
	/////////////////////////////////////////////////////////////////////////
    if (mRowsToScroll) { Scrolling(); }
}

//=================================================================================================
// Mouse action was reported.
//=================================================================================================
bool CTextwindow::MouseAction(int action, Real xpos, Real ypos, Real yRelative)
{
	if (action == M_RELEASED) { mDragging = false; return true; }
	// Was the mouse action this Overlay?
    if (!mContainerFrame-> contains(xpos, ypos) && !mDragging)   { return true;} 

    // Check all buttons.
	if (mElementButUp  ->contains(xpos, ypos))
	{ 
		Sound::getSingleton().playSample(SAMPLE_BUTTON_CLICK);
		OpenTextWin(); 
		return true; 
	}
    if (mElementButDown->contains(xpos, ypos))
	{
		Sound::getSingleton().playSample(SAMPLE_BUTTON_CLICK);
		CloseTextWin(); 
		return true;
	}

    // Check for resize.
	if ((mElementTitle  ->contains(xpos, ypos) && action == M_DRAGGED) || mDragging) 
	{   
        Real top;
        bool ret  = true;
        mDragging = true;
        top = mContainerFrame->getTop() +  yRelative;
        if (mParent)
        { 
            if      (top - mParent->mContainerFrame->getTop() >= mMinHeight )
            {
                top = mParent->mContainerFrame->getTop()+mMinHeight;
                ret = false;
            }
            else if (top - mParent->mContainerFrame->getTop() <= mMaxHeight )
            {
                top = mParent->mContainerFrame->getTop()+mMaxHeight;
                ret = false;
            }
        }
        else
        {
           if      (top >= mMinHeight) { top = mMinHeight; ret = false; }
           else if (top <= mMaxHeight) { top = mMaxHeight; ret = false; }
        }
        mContainerFrame->setTop(top);
		SizeChanged();
        return ret;
	}
    return true;
}

//=================================================================================================
// Size change of the window..
//=================================================================================================
void CTextwindow::SizeChanged()
{
    if (mParent)
        mFirstYPos = -(mContainerFrame->getTop()- mParent->mContainerFrame->getTop());
    else
        mFirstYPos = -mContainerFrame->getTop();
    mContainerFrame->setHeight(mFirstYPos);
    mSumRows   = (int) ((mFirstYPos+6) / FONT_SIZE);
    if (mSumRows > MAX_TEXT_LINES) { mSumRows = MAX_TEXT_LINES; }
    int i=0;
    for (; i < mSumRows; ++i)
	{
        mElementLine[i]->setTop(mFirstYPos -i*FONT_SIZE);
        mElementLine[i]->show();
	}
    for (; i < MAX_TEXT_LINES; ++i)
	{
        mElementLine[i]->hide();
	}
    DockChild();
}

//=================================================================================================
// Dock child window on top of this one..
//=================================================================================================
void CTextwindow::DockChild()
{
    if (!mChild) { return; }
    mChild->mContainerFrame->setTop(mContainerFrame->getTop()-mChild->mContainerFrame->getHeight());
}

//=================================================================================================
// Add a line of text.
//=================================================================================================
void CTextwindow::Print(const char *text, ColourValue color)
{
    row[mBufferPos & (SIZE_STRING_BUFFER-1)].str   = text;
    row[mBufferPos & (SIZE_STRING_BUFFER-1)].colorTop = color;
    row[mBufferPos & (SIZE_STRING_BUFFER-1)].colorBottom = color/1.5;
    ++mBufferPos;
    ++mRowsToScroll;
}

//=================================================================================================
// Scroll the text.
//=================================================================================================
void CTextwindow::Scrolling()
{
    if (mDragging) { return; }
    if (!mScroll)
    {
        mElementLine[0]->setCaption(row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].str);
        mElementLine[0]->setColourTop(row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].colorTop);
        mElementLine[0]->setColourBottom(row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].colorBottom);
    }
    for (int i = 0; i < MAX_TEXT_LINES; ++i)
    {
        mElementLine[i]->setTop(mFirstYPos - FONT_SIZE*i - mScroll);
    }
    mScroll += SCROLL_SPEED;
    // The complete row was scrolled.
	if (mScroll >= FONT_SIZE)
	{ 	
        --mRowsToScroll;
        ++mPrintPos;  
        mScroll =0.0f;
	    for (int k=0; k < MAX_TEXT_LINES; ++k)
        { 
           mElementLine[k]->setTop(mFirstYPos - k*FONT_SIZE + SCROLL_SPEED);
        }
		for (int u=1; u < MAX_TEXT_LINES; ++u)
        {
            mElementLine[u]->setCaption     (row[(mPrintPos-u)&(SIZE_STRING_BUFFER-1)].str);
	        mElementLine[u]->setColourTop   (row[(mPrintPos-u)&(SIZE_STRING_BUFFER-1)].colorTop);
    	    mElementLine[u]->setColourBottom(row[(mPrintPos-u)&(SIZE_STRING_BUFFER-1)].colorBottom);
        }
    }
}

//=================================================================================================
// Docks on a child on top of this window.
//=================================================================================================
void CTextwindow::setChild(CTextwindow *Child)
{
    if (!Child) { return; }
    mChild = Child;
    mChild->mParent= this;
    mChild->mContainerFrame->setLeft (mContainerFrame->getLeft ());
    mChild->mContainerFrame->setWidth(mContainerFrame->getWidth());
    mChild->mContainerFrame->setTop  (mContainerFrame->getTop()-mChild->mContainerFrame->getHeight());
}

//=================================================================================================
// 
//=================================================================================================
void CTextwindow::setDimension(Real x, Real y, Real w, Real h)
{
    mContainerFrame->setPosition(x, y);
	mContainerFrame->setDimensions(w,h);
    setChild(mChild);
}
