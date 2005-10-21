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

#include <tinyxml.h>
#include <OgreHardwarePixelBuffer.h>
#include "define.h"
#include "gui_statusbar.h"
#include "logger.h"

#include <ctime>

///=================================================================================================
/// .
///=================================================================================================
GuiStatusbar::~GuiStatusbar()
{
  delete[] mGfxBuffer;
}

///=================================================================================================
/// Parse a Statusbar entry.
///=================================================================================================
GuiStatusbar::GuiStatusbar(TiXmlElement *xmlElem, int maxX, int maxY)
{
  TiXmlElement *xmlGadget;
  std::string strValue;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the gadget.
  /////////////////////////////////////////////////////////////////////////
  mName = xmlElem->Attribute("name");
  /////////////////////////////////////////////////////////////////////////
  /// Parse the position.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Pos")))
  {
    mPosX = atoi(xmlGadget->Attribute("x"));
    mPosY = atoi(xmlGadget->Attribute("y"));
  }
  if (mPosX > maxX-2) mPosX = maxX-2;
  if (mPosY > maxY-2)mPosY = maxY-2;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the position.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Size")))
  {
    mWidth = atoi(xmlGadget->Attribute("width"));
    mHeight = atoi(xmlGadget->Attribute("height"));
  }
  if (mPosX + mWidth > maxX) mWidth = maxX-mPosX-1;
  if (mPosY + mHeight >maxY) mHeight= maxY-mPosY-1;
  /////////////////////////////////////////////////////////////////////////
  /// Parse the fill color.
  /////////////////////////////////////////////////////////////////////////
  if ((xmlGadget = xmlElem->FirstChildElement("Color")))
  {
    // PixelFormat: ARGB.
    mColor = atoi(xmlGadget->Attribute("red"  )) << 16;
    mColor+= atoi(xmlGadget->Attribute("green")) << 8;
    mColor+= atoi(xmlGadget->Attribute("blue" ));
    mColor+= atoi(xmlGadget->Attribute("alpha")) << 24;
  }
  /////////////////////////////////////////////////////////////////////////
  /// Create and Init the gfx buffer.
  /////////////////////////////////////////////////////////////////////////
  mGfxBuffer = new uint32[mWidth*mHeight];
  mGfxBufferInit = false;
}

///=================================================================================================
/// .
///=================================================================================================
void GuiStatusbar::draw(PixelBox &mSrcPixelBox, Texture *texture, int value)
{
  /////////////////////////////////////////////////////////////////////////
  /// Save the original background.
  /////////////////////////////////////////////////////////////////////////
  if (!mGfxBufferInit)
  {
    mGfxBufferInit = true;
    texture->getBuffer()->blitToMemory(
      Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight),
      PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer));
  }
  /////////////////////////////////////////////////////////////////////////
  /// Calculate the act. size of the bar.
  /////////////////////////////////////////////////////////////////////////
  //if (mActValue == value) return;
  if (value < 0) mActValue = mMaxValue;
  else           mActValue = value;

  mActValue = 100; // Testing.
  /////////////////////////////////////////////////////////////////////////
  /// Copy background to buffer.
  /////////////////////////////////////////////////////////////////////////
  /*
  int y =0;
    for (unsigned int j =0; j < mFont[fontNr].height; ++j)
    {
      for (int x=0; x < line->width; ++x)
      {
        TextGfxBuffer[x + y] = line->BG_Backup[x + y];
      }
      y += line->width;
    }
  */
  /////////////////////////////////////////////////////////////////////////
  /// Draw the bar into buffer.
  /////////////////////////////////////////////////////////////////////////
#define BAR_WIDTH 16
  if (mHeight < 0)
  {
    mHeight =0;
  }
  int x, y, offset;
  uint32 color;
  uint32 dColor = 0x00000000;
  dColor+=(((mColor & 0x00ff0000)/ 6) & 0x00ff0000);
  dColor+=(((mColor & 0x0000ff00)/ 6) & 0x0000ff00);
  dColor+=(((mColor & 0x000000ff)/ 6) & 0x000000ff);
  /// Draw top of the bar.
  color = mColor;
  for (offset =3, y=0; y < 5; ++y)
  {
    for (x=offset; x <= BAR_WIDTH-offset; ++x) mGfxBuffer[y*mWidth +x] = color;
    if (y < 4) --offset;
    color+= dColor;
  }

  /// Draw the bar.
  color = 0xff000000;
  for (offset= 2, x=0; x <= BAR_WIDTH/2; ++x)
  {
    if (x == 1 || x == 3) --offset;
    for (y = 5-offset; y < mHeight-offset; ++y)
    {
      mGfxBuffer[y*mWidth + x] = color;
      mGfxBuffer[y*mWidth + BAR_WIDTH-x] = color;
    }
    color+= dColor;
  }


  /////////////////////////////////////////////////////////////////////////
  /// Blit buffer into the window-texture.
  /////////////////////////////////////////////////////////////////////////
  texture->getBuffer()->blitFromMemory(
    PixelBox(mWidth, mHeight, 1, PF_A8R8G8B8 , mGfxBuffer),
    Box(mPosX, mPosY, mPosX + mWidth, mPosY + mHeight));
  //  Logger::log().error() << mName << " " << mWidth << " " << mHeight;
}
