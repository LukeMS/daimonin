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

#include "textwindow.h"
#include "logger.h"
#include "sound.h"
#include <OgreFontManager.h>
#include <OgreHardwarePixelBuffer.h> 

CTextwindow *ChatWin=0, *TextWin=0;

//=================================================================================================
// Init all static Elemnts.
//=================================================================================================
int CTextwindow::mInstanceNr  = -1;
int CTextwindow::mDragWinNr   = -1;
int CTextwindow::mScreenHeight= -1;
//=================================================================================================
// Constructor.
//=================================================================================================
CTextwindow::CTextwindow(std::string title, Real Xpos, Real height, int ScreenHeight, bool visible)
{
  /////////////////////////////////////////////////////////////////////////
  /// Clone all OverlayElemnts from templates.
  /////////////////////////////////////////////////////////////////////////
  mThisWindowNr = ++mInstanceNr;
  std::string name= StringConverter::toString(mInstanceNr)+"_TextWindow/";
  mOverlay        = OverlayManager::getSingleton().create(name + "Overlay");
  mOverlay->setZOrder(510-mInstanceNr);

  mContainerFrame = static_cast<OverlayContainer*>(OverlayManager::getSingleton().
                    cloneOverlayElementFromTemplate("TextWindow/Frame", name + "Frame"));
  mOverlay->add2D(mContainerFrame);
  mElementTitle   = OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/Headline",name + "Title");
//  mElementButUp   = OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/ButtonUp",name + "ButtopUp");
//  mElementButDown = OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/ButtonDown",name + "ButtopDown");
  mContainerFrame->addChild(mElementTitle);
//  mContainerFrame->addChild(mElementButUp);
//  mContainerFrame->addChild(mElementButDown);

  for (int i=0; i < MAX_TEXT_LINES; ++i)
  {
    mElementLine[i]= static_cast<TextAreaOverlayElement*>
                     (OverlayManager::getSingleton().cloneOverlayElementFromTemplate("TextWindow/TextRow",name+"Line_"+ StringConverter::toString(i)));
    mElementLine[i]->setCaption("");
    mElementLine[i]->setHeight(0.01);
    mContainerFrame->addChild(mElementLine[i]);
  }

  mContainerFrame->setLeft(Xpos);
//  mElementTitle  ->setWidth(-Xpos-mElementButUp->getWidth()*2);
//  mElementButUp  ->setLeft(-Xpos-mElementButUp->getWidth()*2);
 // mElementButDown->setLeft(-Xpos-mElementButUp->getWidth());
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


  /////////////////////////////////////////////////////////////////////////
  /// Init all variables.
  /////////////////////////////////////////////////////////////////////////
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

//=================================================================================================
// Show/Hide the Overlay.
//=================================================================================================
void CTextwindow::setVisible(bool show)
{
  if (show) mOverlay->show();
  else      mOverlay->hide();
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


/*

  MaterialPtr mMaterial = mContainerFrame->getMaterial();
  String texname = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
  TexturePtr mTexture = (TexturePtr) TextureManager::getSingleton().getByName(texname);
  HardwarePixelBufferSharedPtr buffer = mTexture->getBuffer();

  static Image TileImage;
  static int once=-1;
  if (++once == 0)
  {
    TileImage.load("WindowsLook.tga", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    //mpFont->load();
  }
  Real u1, u2, v1, v2; 
  FontPtr mpFont = FontManager::getSingleton().getByName( "TrebuchetMSBold" );
  if (mpFont.isNull()) Logger::log().info() << "Font Error!";
  MaterialPtr mpMaterialFont = mpFont->getMaterial();
  String texnameFont = mpMaterialFont->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
  TexturePtr mTextureFont = (TexturePtr) TextureManager::getSingleton().getByName(texnameFont);
  HardwarePixelBufferSharedPtr bufferFont = mTextureFont->getBuffer();
  mpFont->getGlyphTexCoords( 'A', u1, v1, u2, v2 );


  //PixelBox src = TileImage.getPixelBox().getSubVolume(Box(u1, v1, u2, v2));
 // buffer->blitFromMemory(src, Box( 0, 0, 128, 128));

 buffer->blit(bufferFont.getPointer(), Box( 0, 0, 64, 64), Box( 0, 0, 128, 128));
*/
/*

  MaterialPtr mMaterial = mContainerFrame->getMaterial();
  String texname = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
  TexturePtr mTexture = (TexturePtr) TextureManager::getSingleton().getByName(texname);
  HardwarePixelBufferSharedPtr buffer = mTexture->getBuffer();

  static Image TileImage;
  static int once=-1;
  if (++once == 0) TileImage.load("WindowsLook.tga", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME); 
  PixelBox src = TileImage.getPixelBox().getSubVolume(Box(2, 20, 15, 33));
  buffer->blitFromMemory(src, Box( 22, 22, 35, 35));
*/





  /////////////////////////////////////////////////////////////////////////
  /// User pressed the close-button.
  /////////////////////////////////////////////////////////////////////////
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
  /// Scroll the text.
  /////////////////////////////////////////////////////////////////////////
  if (mRowsToScroll)  Scrolling();
}

//=================================================================================================
// Mouse action was reported.
//=================================================================================================
bool CTextwindow::MouseAction(int action, Real xpos, Real ypos)
{
  if (action == M_RELEASED)
  {
    mDragWinNr = -1;
    return true;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Was the mouse action in this Overlay?
  /////////////////////////////////////////////////////////////////////////
  if (!mContainerFrame->contains(xpos, ypos) && mDragWinNr < 0) return true;
  /////////////////////////////////////////////////////////////////////////
  /// Check all buttons.
  /////////////////////////////////////////////////////////////////////////
/*
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
  */
  /////////////////////////////////////////////////////////////////////////
  /// Check for resize.
  /////////////////////////////////////////////////////////////////////////
  if (action != M_DRAGGED || (!mElementTitle ->contains(xpos, ypos) && mDragWinNr < 0)) return true;
  //////////////////////////////////////////////////////////////////////
  /// Are we resizing already another window?
  //////////////////////////////////////////////////////////////////////
  if (mDragWinNr < 0) mDragWinNr= mThisWindowNr;
  if (mDragWinNr != mThisWindowNr) return true;
  /////////////////////////////////////////////////////////////////////////
  /// Do the resitze.
  /////////////////////////////////////////////////////////////////////////
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

//=================================================================================================
// Size change of the window..
//=================================================================================================
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

//=================================================================================================
// Dock child window on top of this one..
//=================================================================================================
void CTextwindow::DockChild()
{
  if (!mChild) return;
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

//=================================================================================================
// Docks on a child on top of this window.
//=================================================================================================
void CTextwindow::setChild(CTextwindow *Child)
{
  if (!Child) return;
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
