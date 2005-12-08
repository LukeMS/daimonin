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

#include <OgreHardwarePixelBuffer.h>
#include <tinyxml.h>
#include "define.h"
#include "gui_cursor.h"
#include "logger.h"

///=================================================================================================
/// .
///=================================================================================================
void GuiCursor::Init(int w, int h, int screenWidth, int screenHeight)
{
  mState = STATE_STANDARD;
  mWidth = w;
  mHeight= h;
  if (mWidth <   4) mWidth =   4;
  if (mHeight<   4) mHeight=   4;
  if (mWidth > 128) mWidth = 128;
  if (mHeight> 128) mHeight= 128;
  /////////////////////////////////////////////////////////////////////////
  /// Create the overlay element.
  /////////////////////////////////////////////////////////////////////////
  Overlay *overlay = OverlayManager::getSingleton().create("GUI_MouseCursor");
  overlay->setZOrder(600);
  mElement = OverlayManager::getSingleton().createOverlayElement(OVERLAY_TYPE_NAME, "GUI_Cursor");
  mElement->setHeight((Real)mHeight / (Real)screenHeight);
  mElement->setWidth ((Real)mWidth  / (Real)screenWidth );
  mElement->setTop (0.5);
  mElement->setLeft(0.5);
  mTexture = TextureManager::getSingleton().createManual("GUI_Cursor_Texture", "General",
             TEX_TYPE_2D, mWidth, mHeight, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
  MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
  mMaterial = tmpMaterial->clone("GUI_Cursor_Material");
  mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_Cursor_Texture");
  mMaterial->load();
  mElement->setMaterialName("GUI_Cursor_Material");
  overlay->add2D(static_cast<OverlayContainer*>(mElement));
  overlay->show();
}

///=================================================================================================
/// .
///=================================================================================================
void GuiCursor::freeRecources()
{
  mMaterial.setNull();
  mTexture.setNull();
}

///=================================================================================================
/// .
///=================================================================================================
void GuiCursor::setPos(Real x, Real y)
{
  mElement->setTop (y);
  mElement->setLeft(x);
}
///=================================================================================================
/// .
///=================================================================================================
void GuiCursor::setState(PixelBox &srcPixelBox, int state)
{
  if (state < STATE_SUM)
  {
     mState = state;
     draw(srcPixelBox);
  }
}

///=================================================================================================
/// .
///=================================================================================================
void GuiCursor::setStateImagePos(std::string name, int x, int y)
{
  int state = -1;
  if      (name == "Standard")   state = STATE_STANDARD;
  else if (name == "ButtonDown") state = STATE_BUTTON_DOWN;
  else if (name == "Dragging")   state = STATE_DRAGGING;
  else if (name == "Resizing")   state = STATE_RESIZING;
  if (state < 0)
  {
    Logger::log().error() << "MouseCursor has no State '" << name << "!";
    return;
  }
  gfxSrcPos[state].x = x;
  gfxSrcPos[state].y = y;
 //  Logger::log().info() << "2: " << gfxSrcPos[state].x << " "<< gfxSrcPos[state].y << " "<<mWidth << " "<< mHeight;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiCursor::draw(PixelBox &SrcPixelBox)
{
  PixelBox src = SrcPixelBox.getSubVolume(Box(
                                            gfxSrcPos[mState].x,
                                            gfxSrcPos[mState].y,
                                            gfxSrcPos[mState].x + mWidth,
                                            gfxSrcPos[mState].y + mHeight ));
  mTexture->getBuffer()->blitFromMemory(src);
}
