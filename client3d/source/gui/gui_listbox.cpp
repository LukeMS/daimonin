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

In addition, as a special exception, the copyright holders of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice and/or with the fmod libraries.
You may copy and distribute such a system following the terms of the GNU GPL
for client3d and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/licenses/licenses.html
-----------------------------------------------------------------------------*/

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_listbox.h"
#include "gui_textout.h"
#include "logger.h"
#include "gui_window.h"

const clock_t SCROLL_SPEED = 12;
static const Real CLOSING_SPEED  =  10.0f;  // default: 10.0f
static const int  MAX_TEXT_LINES =  20;

//================================================================================================
// Constructor.
//================================================================================================
GuiListbox::GuiListbox(TiXmlElement *xmlElement, void *parent):GuiElement(xmlElement, parent)
{
    // ////////////////////////////////////////////////////////////////////
    // Create buffer to hold the pixel information of the listbox.
    // ////////////////////////////////////////////////////////////////////
    mFontHeight = GuiTextout::getSingleton().getFontHeight(mFontNr);
    int size = mWidth * mHeight + mWidth * (mFontHeight+1);
    mGfxBuffer = new uint32[size];
    for (int i =0; i < size; ++i) mGfxBuffer[i] = mFillColor;
    // ////////////////////////////////////////////////////////////////////
    // Set defaults.
    // ////////////////////////////////////////////////////////////////////
    mIsClosing    = false;
    mIsOpening    = false;
    mDragging     = false;
    mBufferPos    = 0;
    mPrintPos     = 0;
    mRowsToScroll = 0;
    mRowsToPrint  = mHeight / mFontHeight;
    if (mRowsToPrint < 1) mRowsToPrint =1;
    mScroll       = 0;
    mScrollBarV   = 0;
    mScrollBarH   = 0;
    mActLines     = 0;
    mScrollbarOffsetH = 0;
    mScrollbarOffsetV = 0;

    TiXmlElement *xmlOpt;
    for (xmlOpt = xmlElement->FirstChildElement("Gadget"); xmlOpt; xmlOpt = xmlOpt->NextSiblingElement("Gadget"))
    {
        if (!strcmp(xmlOpt->Attribute("type"), "SCROLLER"))
        {
            if (!strcmp(xmlOpt->Attribute("name"), "List_Msg_VScroll"))
            {
                mScrollBarV= new GuiGadgetScrollbar(xmlOpt, mParent, this);
                mScrollBarV->setFunction(this->scrollbarAction);
	    }
            else if (!strcmp(xmlOpt->Attribute("name"), "List_Msg_HScroll"))
            {
                mScrollBarH= new GuiGadgetScrollbar(xmlOpt, mParent, this);
                mScrollBarH->setFunction(this->scrollbarAction);
            }
        }
    }
}

//================================================================================================
// Destructor.
//================================================================================================
GuiListbox::~GuiListbox()
{
    delete[] mGfxBuffer;
    if (mScrollBarV) delete mScrollBarV;
    if (mScrollBarH) delete mScrollBarH;
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::scrollbarAction(GuiListbox *me, int index, float scroll)
{
    Logger::log().error() << "Scroll reported:  " << index << "  " << scroll;
}


//================================================================================================
// Add a line of text to the ring-buffer.
//================================================================================================
void GuiListbox::addTextline(const char *text)
{
    // Todo: Here we must split a string to fit into the window.
    //       OR we cut it (done by textout) and show the line in the tooltip when
    //       mouse is over this line.

    //Logger::log().error() << GuiTextout::getSingleton().CalcTextWidth(text, mFontNr) << " " << text;
    row[mBufferPos & (SIZE_STRING_BUFFER-1)].str = text;
    ++mBufferPos;
    ++mRowsToScroll;
}

///=============================================================================================
// Returns true if the mouse event was on this gadget (so no need to check the other gadgets).
//================================================================================================
bool GuiListbox::mouseEvent(int MouseAction, int x, int y)
{
    if (mScrollBarV && mScrollBarV->mouseEvent(MouseAction, x, y)) return true;
    if (mScrollBarH && mScrollBarH->mouseEvent(MouseAction, x, y)) return true;
    return false;
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::drawScrollbar()
{
    if (mScrollBarV) mScrollBarV->draw();
    if (mScrollBarH) mScrollBarH->draw();
}

//================================================================================================
// .
//================================================================================================
void GuiListbox::draw()
{
    // ////////////////////////////////////////////////////////////////////
    // User pressed the down-button.
    // ////////////////////////////////////////////////////////////////////
    /*
      if (mIsClosing)
      {
        Real top = mContainerFrame->getTop() + CLOSING_SPEED;
        if (!mParent)
        {
          if (top >= mMinHeight)
          {
            top = mMinHeight;
            mIsClosing = false;
          }
        }
        else
        {
          if (top - mParent->mContainerFrame->getTop()  >= mMinHeight)
          {
            top = mParent->mContainerFrame->getTop()+mMinHeight;
            mIsClosing = false;
          }
        }
        mContainerFrame->setTop(top);
        SizeChanged();
        return;
      }
    */
    /*
    // ////////////////////////////////////////////////////////////////////
      // User pressed the up-button.
    // ////////////////////////////////////////////////////////////////////
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
    */
    // ////////////////////////////////////////////////////////////////////
    // Scroll the text.
    // ////////////////////////////////////////////////////////////////////
    Texture *texture = ((GuiWindow*) mParent)->getTexture();
    static clock_t time = clock();
    if (!mRowsToScroll || mDragging) return;
    if (clock() - time < SCROLL_SPEED) return;
    time = clock();
    // New Line to scroll in.
    if (!mScroll)
    {
        // Print it to the (invisible) last line of the listbox.
        GuiTextout::getSingleton().PrintToBuffer(mWidth, mHeight, mGfxBuffer + mWidth * mHeight, row[(mPrintPos)& (SIZE_STRING_BUFFER-1)].str.c_str(), mFontNr, mFillColor);
    }

    texture->getBuffer()->blitFromMemory(
        PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer + mWidth * mScroll),
        Box(mX, mY, mX + mWidth, mY + mHeight));
    // The complete row was scrolled.
    if (++mScroll >= mFontHeight)
    {
        --mRowsToScroll;
        ++mPrintPos;
        mScroll =0;
        memcpy(mGfxBuffer, mGfxBuffer + mWidth *mFontHeight, mWidth * mHeight * sizeof(uint32));
        if (mScrollBarV)
        {
            if (mActLines < SIZE_STRING_BUFFER) ++mActLines;
            mScrollBarV->updateSliderSize((float)mRowsToPrint / (float)mActLines);
        }
    }
}




/*
CTextwindow *ChatWin=0, *TextWin=0;

//================================================================================================
// Init all static Elemnts.
//================================================================================================
int CTextwindow::mInstanceNr  = -1;
int CTextwindow::mDragWinNr   = -1;
int CTextwindow::mScreenHeight= -1;
//================================================================================================
// Constructor.
//================================================================================================
CTextwindow::CTextwindow(std::string title, Real Xpos, Real height, int ScreenHeight, bool visible)
{
// ////////////////////////////////////////////////////////////////////
  // Clone all OverlayElemnts from templates.
// ////////////////////////////////////////////////////////////////////
  mThisWindowNr = ++mInstanceNr;
  std::string name= StringConverter::toString(mInstanceNr)+"_TextWindow/";
  mOverlay        = OverlayManager::getSingleton().create(name + "Overlay");
  mOverlay->setZOrder(510-mInstanceNr);

  mContainerFrame = static_cast<OverlayContainer*>(OverlayManager::getSingleton().
                    cloneOverlayElementFromTemplate("TextWindow/Frame", name + "Frame"));
  mOverlay->add2D(mContainerFrame);
  mElementTitle   = OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/Headline",name + "Title");
  mElementButUp   = OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/ButtonUp",name + "ButtopUp");
  mElementButDown = OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/ButtonDown",name + "ButtopDown");
  mContainerFrame->addChild(mElementTitle);
  mContainerFrame->addChild(mElementButUp);
  mContainerFrame->addChild(mElementButDown);

  for (int i=0; i < MAX_TEXT_LINES; ++i)
  {
    mElementLine[i]= static_cast<TextAreaOverlayElement*>
                     (OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/TextRow",name+"Line_"+ StringConverter::toString(i)));
    mElementLine[i]->setCaption("");
    mElementLine[i]->setHeight(0.01);
    mContainerFrame->addChild(mElementLine[i]);
  }

  mContainerFrame->setLeft(Xpos);
  mElementTitle  ->setWidth(-Xpos-mElementButUp->getWidth()*2);
  mElementButUp  ->setLeft(-Xpos-mElementButUp->getWidth()*2);
  mElementButDown->setLeft(-Xpos-mElementButUp->getWidth());
  // WindowTitle.
  mElementTitleTxt0= OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/TitleText", name+"Title0");
  mElementTitleTxt0->setCaption(title);
  mElementTitleTxt0->setTop(3);
  mElementTitleTxt0->setLeft(5);
  mElementTitleTxt0->setColour(ColourValue(.4,.4,.4));
  static_cast<OverlayContainer*>(mElementTitle)->addChild(mElementTitleTxt0);
  mElementTitleTxt1= OverlayManager::getSingleton(). cloneOverlayElementFromTemplate("TextWindow/TitleText", name+"Title1");
  mElementTitleTxt1->setCaption(title);
  static_cast<OverlayContainer*>(mElementTitle)->addChild(mElementTitleTxt1);


// ////////////////////////////////////////////////////////////////////
  // Init all variables.
// ////////////////////////////////////////////////////////////////////
  mChild        = 0;
  mParent       = 0;
  mMinHeight    = -mElementTitle->getHeight();
  mMaxHeight    = -FONT_SIZE * MAX_TEXT_LINES;
  if (-height < mMaxHeight)   height = mMaxHeight;
  if (!mParent)   mContainerFrame->setTop(-height);
  else  mContainerFrame->setTop(-height+mParent->mContainerFrame->getTop() );
  SizeChanged();
  mLastHeight   =  mContainerFrame->getHeight();
  for (int j = 0; j < SIZE_STRING_BUFFER; ++j)  row[j].str = "";
  mIsClosing    = false;
  mIsOpening    = false;
  mBufferPos    = 0;
  mPrintPos     = 0;
  mRowsToScroll = 0;
  mScroll       = 0.0f;
  mScreenHeight = ScreenHeight;
  setVisible(visible);


}

//================================================================================================
// Show/Hide the Overlay.
//================================================================================================
void CTextwindow::setVisible(bool show)
{
  if (show) mOverlay->show();
  else      mOverlay->hide();
}

//================================================================================================
// Closes the TextWindow.
//================================================================================================
void CTextwindow::CloseTextWin()
{
  mLastHeight = mContainerFrame->getHeight();
  mIsClosing  = true;
  mIsOpening  = false;
}

//================================================================================================
// opens the TextWindow.
//================================================================================================
void CTextwindow::OpenTextWin()
{
  mIsOpening  = true;
  mIsClosing  = false;
}


//================================================================================================
// Mouse action was reported.
//================================================================================================
bool CTextwindow::MouseAction(int action, Real xpos, Real ypos)
{
  if (action == M_RELEASED)
  {
    mDragWinNr = -1;
    return true;
  }
// ////////////////////////////////////////////////////////////////////
  // Was the mouse action in this Overlay?
// ////////////////////////////////////////////////////////////////////
  if (!mContainerFrame->contains(xpos, ypos) && mDragWinNr < 0) return true;
// ////////////////////////////////////////////////////////////////////
  // Check all buttons.
// ////////////////////////////////////////////////////////////////////
  if (mElementButUp->contains(xpos, ypos))
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
// ////////////////////////////////////////////////////////////////////
  // Check for resize.
// ////////////////////////////////////////////////////////////////////
  if (action != M_DRAGGED || (!mElementTitle ->contains(xpos, ypos) && mDragWinNr < 0)) return true;
// ////////////////////////////////////////////////////////////////////
  // Are we resizing already another window?
// ////////////////////////////////////////////////////////////////////
  if (mDragWinNr < 0) mDragWinNr= mThisWindowNr;
  if (mDragWinNr != mThisWindowNr) return true;
// ////////////////////////////////////////////////////////////////////
  // Do the resitze.
// ////////////////////////////////////////////////////////////////////
  bool ret = true;
  Real top = (ypos-1) * mScreenHeight;
  if (mParent)
  {
    if (top - mParent->mContainerFrame->getTop() >= mMinHeight )
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
    if (top >= mMinHeight)
    {
      top = mMinHeight;
      ret = false;
    }
    else if (top <= mMaxHeight)
    {
      top = mMaxHeight;
      ret = false;
    }
  }
  mContainerFrame->setTop(top);
  SizeChanged();
  return ret;
}

//================================================================================================
// Size change of the window..
//================================================================================================
void CTextwindow::SizeChanged()
{
  if (!mParent) mFirstYPos = -mContainerFrame->getTop();
  else mFirstYPos = -(mContainerFrame->getTop()- mParent->mContainerFrame->getTop());

  mContainerFrame->setHeight(mFirstYPos);
  mSumRows   = (int) ((mFirstYPos+6) / FONT_SIZE);
  if (mSumRows > MAX_TEXT_LINES) mSumRows = MAX_TEXT_LINES;
  int i=0;
  for (; i < mSumRows; ++i)
  {
    mElementLine[i]->setTop(mFirstYPos -i*FONT_SIZE);
    mElementLine[i]->show();
  }
  for (; i < MAX_TEXT_LINES; ++i) mElementLine[i]->hide();
  DockChild();
}

//================================================================================================
// Dock child window on top of this one..
//================================================================================================
void CTextwindow::DockChild()
{
  if (!mChild) return;
  mChild->mContainerFrame->setTop(mContainerFrame->getTop()-mChild->mContainerFrame->getHeight());
}

//================================================================================================
// Add a line of text.
//================================================================================================
void CTextwindow::Print(const char *text, ColourValue color)
{
  row[mBufferPos & (SIZE_STRING_BUFFER-1)].str   = text;
  row[mBufferPos & (SIZE_STRING_BUFFER-1)].colorTop = color;
  row[mBufferPos & (SIZE_STRING_BUFFER-1)].colorBottom = color/1.5;
  ++mBufferPos;
  ++mRowsToScroll;
}

//================================================================================================
// Scroll the text.
//================================================================================================
void CTextwindow::Scrolling()
{
  if (mDragging) return;
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

//================================================================================================
// Docks on a child on top of this window.
//================================================================================================
void CTextwindow::setChild(CTextwindow *Child)
{
  if (!Child) return;
  mChild = Child;
  mChild->mParent= this;
  mChild->mContainerFrame->setLeft (mContainerFrame->getLeft ());
  mChild->mContainerFrame->setWidth(mContainerFrame->getWidth());
  mChild->mContainerFrame->setTop  (mContainerFrame->getTop()-mChild->mContainerFrame->getHeight());
}

//================================================================================================
//
//================================================================================================
void CTextwindow::setDimension(Real x, Real y, Real w, Real h)
{
  mContainerFrame->setPosition(x, y);
  mContainerFrame->setDimensions(w,h);
  setChild(mChild);
}
*/
