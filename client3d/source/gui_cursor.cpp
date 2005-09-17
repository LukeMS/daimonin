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


GuiCursor::GuiCursor(unsigned int w, unsigned int h, const char* imageSetFile)
{
  mWidth =0;
  mHeight=0;
  mScreenWidth = w;
  mScreenHeight= h;
  mState = STATE_STANDARD;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the cursor gfx.
  /////////////////////////////////////////////////////////////////////////
  TiXmlElement *xmlRoot, *xmlElem, *xmlGadget;
  TiXmlDocument doc(imageSetFile);
  if (!doc.LoadFile(imageSetFile) || !(xmlRoot = doc.RootElement()) || !xmlRoot->Attribute("File"))
  {
    Logger::log().error() << "XML-File '" << imageSetFile << "' is broken or missing.";
    return;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gfx coordinates.
  /////////////////////////////////////////////////////////////////////////
  std::string strTmp;
  for (xmlElem = xmlRoot->FirstChildElement("Image"); xmlElem; xmlElem = xmlElem->NextSiblingElement("Image"))
  {
    if (!xmlElem->Attribute("ID") || strcmp(xmlElem->Attribute("ID"), "MouseCursor"))  continue;
    /////////////////////////////////////////////////////////////////////////
    /// Parse the Position entries.
    /////////////////////////////////////////////////////////////////////////
    int state=0;
    mSrcPos[state].w = atoi(xmlElem->Attribute("Width"));
    mSrcPos[state].h = atoi(xmlElem->Attribute("Height"));
    for (xmlGadget = xmlElem->FirstChildElement("State"); xmlGadget; xmlGadget = xmlGadget->NextSiblingElement("State"))
    {
      if (!(xmlGadget->Attribute("ID"))) continue;
      if      (strTmp == "STANDARD") state = STATE_STANDARD;
      else if (strTmp == "DRAGGING") state = STATE_DRAGGING;
      else if (strTmp == "RESIZING") state = STATE_RESIZING;
      mSrcPos[state].x = atoi(xmlGadget->Attribute("posX"));
      mSrcPos[state].y = atoi(xmlGadget->Attribute("posY"));
      if (mSrcPos[state].w > mWidth ) mWidth  = mSrcPos[state].w;
      if (mSrcPos[state].h > mHeight) mHeight = mSrcPos[state].h;
    }
    break;
  }

  /////////////////////////////////////////////////////////////////////////
  /// Create the overlay element.
  /////////////////////////////////////////////////////////////////////////
  Overlay *overlay = OverlayManager::getSingleton().create("GUI_MouseCursor");
  overlay->setZOrder(500);
  mElement = OverlayManager::getSingleton().createOverlayElement(OVERLAY_TYPE_NAME, "GUI_Cursor");
  mElement->setHeight((Real)mHeight / (Real)mScreenHeight);
  mElement->setWidth ((Real)mWidth  / (Real)mScreenWidth);
  mElement->setTop   (0.5);
  mElement->setLeft  (0.5);
  mTexture = TextureManager::getSingleton().createManual("GUI_Cursor_Texture", "General",
             TEX_TYPE_2D, mWidth, mHeight, 0, PF_R8G8B8A8, TU_STATIC_WRITE_ONLY);
  MaterialPtr tmpMaterial = MaterialManager::getSingleton().getByName("GUI/Window");
  mMaterial = tmpMaterial->clone("GUI_Cursor_Material");
  mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("GUI_Cursor_Texture");
  mMaterial->load();
  mElement->setMaterialName("GUI_Cursor_Material");
  mTileImage.load("Imageset.png", "General");
  mSrcPixelBox = mTileImage.getPixelBox();
  PixelBox src = mSrcPixelBox.getSubVolume(Box(
                   mSrcPos[mState].x , mSrcPos[mState].y,
                   mSrcPos[mState].x + mSrcPos[mState].w,
                   mSrcPos[mState].y + mSrcPos[mState].h));
  mTexture->getBuffer()->blitFromMemory(src);
  overlay->add2D(static_cast<OverlayContainer*>(mElement));
  overlay->show();
}

void GuiCursor::setPos(Real x, Real y)
{
  mElement->setTop (y);
  mElement->setLeft(x);
}
